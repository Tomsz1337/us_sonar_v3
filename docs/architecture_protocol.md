# Architektura i protokół komunikacyjny echosondy

> **Status dokumentu:** wersja robocza – opisuje aktualną architekturę oraz uzgodniony kierunek rozwoju. Elementy oznaczone jako „propozycja” nie są jeszcze zaimplementowane ani ostatecznie zatwierdzone.

## 1. Cel projektu

Projekt zakłada budowę mobilnej echosondy ultradźwiękowej do pomiaru głębokości i wykrywania obiektów podwodnych. Urządzenie bazuje na Raspberry Pi Pico 2 W (RP2350), układzie analogowego front-endu TUSS4470 oraz przetworniku ultradźwiękowym.

Mikrokontroler zbiera dane z sonaru i czujników, a następnie przesyła je bezprzewodowo do komputera. Aplikacja PC odbiera dane i wizualizuje je w interfejsie napisanym w Pythonie.

Architektura powinna umożliwiać późniejszą rozbudowę o:
- GPS;
- sterowanie napędem platformy pływającej;
- nadzór nad łącznością z komputerem;
- procedury awaryjne, w tym powrót do punktu początkowego;
- inne interfejsy komunikacyjne, np. UART z mostkiem radiowym.

## 2. Założenia projektowe

1. Firmware jest podzielony na niezależne moduły i zadania FreeRTOS.
2. Komunikacja aplikacyjna jest niezależna od konkretnego transportu.
3. Sonar i telemetria działają niezależnie od siebie.
4. Sonar może pracować ciągle po uruchomieniu i jest zatrzymywany osobną komendą.
5. Telemetria jest pobierana na żądanie.
6. Utrata łączności nie może powodować niekontrolowanego wznowienia normalnej pracy po odzyskaniu połączenia.
7. Duże wiadomości, np. ramki sonaru, są dzielone na fragmenty przez warstwę protokołu, a nie przez konkretny transport.
8. Nie przesyłamy surowych struktur C/C++ jako formatu sieciowego. Pola są serializowane jawnie.

## 3. Architektura firmware

### 3.1. Główne moduły

| Moduł | Odpowiedzialność |
|---|---|
| `CommTask` | Obsługa komunikacji, odbiór ramek i wysyłanie danych |
| `CommService` | Obsługa protokołu, komend, odpowiedzi i wiadomości |
| `ControlTask` | Interpretacja komend i koordynacja działania urządzenia |
| `SonarTask` | Sterowanie pomiarem sonaru i generowanie ramek danych |
| `TelemetryTask` | Odczyt czujników na żądanie |
| `DataQueue` | Wspólna kolejka danych przekazywanych do komunikacji |
| `ITransport` | Abstrakcja konkretnego medium transmisyjnego |

### 3.2. Przepływ danych

Dane z zadań pomiarowych trafiają do wspólnej kolejki, z której pobiera je zadanie komunikacyjne.

```text
SonarTask --------\
                   \
                    > DataQueue -> CommTask -> CommService -> Protocol -> Transport
                   /
TelemetryTask ----/

PC -> Transport -> Protocol -> CommService -> ControlTask
```

Zadania pomiarowe nie powinny bezpośrednio obsługiwać UDP, UART ani innego medium transmisyjnego. Dzięki temu zmiana transportu nie wymaga modyfikacji logiki sonaru i telemetrii.

### 3.3. Sonar

- Zadanie sonaru jest tworzone raz i pozostaje aktywne przez cały czas działania firmware.
- Początkowo sonar jest zatrzymany.
- `START_SONAR` uruchamia ciągłe generowanie ramek.
- `STOP_SONAR` zatrzymuje sonar, ale nie zmienia globalnego stanu urządzenia ani stanu telemetrii.
- Docelowa ramka sonaru zawiera maksymalnie 1000 próbek `uint16_t`, czyli 2000 bajtów surowych danych.

Obecny generator danych testowych działa cyklicznie i przekazuje ramki przez `DataQueue`.

### 3.4. Telemetria

- Zadanie telemetrii jest tworzone raz.
- Początkowo pozostaje bezczynne.
- `REQUEST_TELEMETRY` zleca pojedynczy odczyt czujników.
- Wynik trafia do `DataQueue`, a następnie do `CommTask`.

### 3.5. Stany urządzenia

Należy rozdzielić stan systemu, stan łączności i stan poszczególnych funkcji. Jedna globalna flaga `RUNNING` nie opisuje poprawnie niezależnej pracy sonaru i telemetrii.

**Proponowane stany systemu:**
- `INITIALIZING` – inicjalizacja;
- `STANDBY` – normalny tryb oczekiwania na polecenia;
- `EMERGENCY` – tryb awaryjny;
- `SHUTDOWN` – kontrolowane wyłączanie, jeśli będzie potrzebne.

**Proponowane stany łączności:**
- `DISCONNECTED`;
- `CONNECTED`.

**Stan sonaru:**
- `STOPPED`;
- `RUNNING`.

**Stan telemetrii:**
- `IDLE`;
- `MEASURING`.

Są to logiczne stany aplikacji. Nie należy utożsamiać ich z tym, czy lokalny interfejs UDP został poprawnie utworzony.

### 3.6. Nadzór i tryb awaryjny

Komputer PC jest nadrzędnym kontrolerem podczas normalnej pracy. Pico powinno jednak samodzielnie wykrywać utratę komunikacji.

W przyszłości planowany jest mechanizm heartbeat:
- PC okresowo wysyła `PING`;
- Pico odpowiada `PONG`;
- brak odpowiedzi/heartbeat przez określony czas powoduje przejście do trybu awaryjnego;
- po odzyskaniu łączności urządzenie nie powinno automatycznie wracać do poprzedniej pracy bez jawnej procedury wznowienia.

Powrót do punktu początkowego będzie możliwy dopiero po dodaniu GPS i sterowania napędem. Szczegóły procedury bezpieczeństwa pozostają do zaprojektowania.

## 4. Architektura komunikacji

### 4.1. Podział na warstwy

Komunikacja jest podzielona na trzy warstwy:

1. **Warstwa aplikacji** – zadania pomiarowe, logika sterowania i dane urządzenia.
2. **Warstwa protokołu** – typy wiadomości, serializacja, fragmentacja, składanie wiadomości, ACK/NACK, deduplikacja i heartbeat.
3. **Warstwa transportowa** – przesyłanie pojedynczych ramek przez UDP/Wi-Fi, UART lub inne medium.

Warstwa protokołu nie może zależeć od konkretnego transportu. Nie powinna zawierać kodu specyficznego dla lwIP, CYW43 ani UDP.

### 4.2. Interfejs transportu

Interfejs transportowy udostępnia:
- `init()` – inicjalizację;
- `send()` – wysłanie pojedynczej ramki transportowej;
- `receive()` – odebranie pojedynczej ramki;
- `max_frame_size()` – maksymalny rozmiar całej ramki, łącznie z nagłówkiem protokołu;
- `is_ready()` – informację o gotowości lokalnego transportu.

`is_ready()` nie oznacza, że komputer jest osiągalny. UDP nie ustanawia połączenia tak jak TCP. Stan komunikacji z drugim urządzeniem jest ustalany na poziomie protokołu, m.in. przez `HELLO` i heartbeat.

Aktualny transport UDP jest rozwiązaniem tymczasowym. Nie należy nadmiernie rozbudowywać jego implementacji, dopóki nie zostanie podjęta decyzja, że pozostaje rozwiązaniem docelowym. Wprowadzone już elementy abstrakcji (`max_frame_size()` i `is_ready()`) są jednak przydatne również dla przyszłych transportów.

**Aktualna wartość robocza UDP:** 1200 bajtów na całą ramkę aplikacyjną. Jest to konserwatywny limit przyjęty dla obecnej implementacji, a nie uniwersalna wartość dla innych transportów.

### 4.3. Typy wiadomości

Uzgodniony zestaw typów:

| Typ | Kierunek | Przeznaczenie |
|---|---|---|
| `COMMAND` | PC → Pico | Polecenia sterujące |
| `ACK` | Oba kierunki | Potwierdzenie przyjęcia wiadomości |
| `NACK` | Oba kierunki | Odrzucenie wiadomości |
| `EVENT` | Pico → PC | Zdarzenia i zmiany stanu urządzenia |
| `TELEMETRY` | Pico → PC | Dane czujników |
| `SONAR` | Pico → PC | Dane echosondy |
| `GPS` | Pico → PC | Dane lokalizacyjne |
| `PING` | PC → Pico | Kontrola obecności urządzenia |
| `PONG` | Pico → PC | Odpowiedź na heartbeat |

Nazwy są nazwami roboczymi do wykorzystania w projekcie. Ostateczne identyfikatory liczbowe zostaną ustalone w implementacji.

### 4.4. Komendy

Proponowany zestaw komend PC → Pico:

| Komenda | Znaczenie |
|---|---|
| `HELLO` | Rozpoczęcie/uzgodnienie komunikacji |
| `START_SONAR` | Uruchomienie sonaru |
| `STOP_SONAR` | Zatrzymanie sonaru |
| `REQUEST_TELEMETRY` | Żądanie pojedynczego odczytu telemetrii |
| `REINITIALIZE` | Żądanie ponownej inicjalizacji urządzenia |

Heartbeat `PING` jest osobnym typem wiadomości, a nie zwykłą komendą aplikacyjną.

### 4.5. Zdarzenia

Proponowany zestaw zdarzeń Pico → PC:

| Zdarzenie | Znaczenie |
|---|---|
| `DEVICE_READY` | Urządzenie gotowe |
| `SONAR_STARTED` | Sonar rozpoczął pracę |
| `SONAR_STOPPED` | Sonar zakończył pracę |
| `TELEMETRY_READY` | Dane telemetrii są gotowe |
| `ERROR` | Wystąpił błąd |
| `EMERGENCY_ENTERED` | Urządzenie weszło w tryb awaryjny |
| `EMERGENCY_CLEARED` | Tryb awaryjny został opuszczony |

Zdarzenie informuje o faktycznej zmianie stanu lub wystąpieniu sytuacji. Nie powinno dublować każdej odpowiedzi NACK.

### 4.6. Semantyka ACK i NACK

- `ACK` oznacza, że wiadomość/komenda została poprawnie odebrana i zaakceptowana do obsługi. Nie oznacza automatycznie, że operacja została już zakończona.
- `NACK` oznacza odrzucenie wiadomości. Zawiera identyfikator potwierdzanej wiadomości oraz krótki kod przyczyny.
- Zakończenie operacji jest raportowane osobnym `EVENT`, np. `SONAR_STARTED` lub `SONAR_STOPPED`.
- Nie wysyłamy dodatkowego `EVENT` dla każdego odrzucenia – przyczyna jest już przekazana w `NACK`.

Proponowane kody NACK:
- `UNKNOWN_COMMAND` – nieznana komenda;
- `INVALID_STATE` – komenda niedozwolona w aktualnym stanie;
- `INVALID_PAYLOAD` – niepoprawne dane;
- `BUSY` – urządzenie nie może obecnie przyjąć operacji.

Na początek ACK/NACK nie potrzebują rozbudowanego payloadu. Należy jednak pozostawić możliwość rozszerzenia formatu w przyszłości.

### 4.7. Heartbeat

`PING/PONG` jest niezależny od ACK/NACK:
- `PING` sprawdza, czy drugie urządzenie odpowiada;
- `PONG` potwierdza obecność urządzenia;
- ACK dotyczy konkretnej wiadomości lub komendy, a nie ogólnej dostępności urządzenia.

Interwał heartbeat, timeout, liczba brakujących odpowiedzi i dokładne zachowanie po utracie łączności pozostają do ustalenia.

## 5. Fragmentacja i składanie wiadomości

### 5.1. Dlaczego fragmentacja jest potrzebna

Ramka sonaru zawiera do 1000 próbek `uint16_t`, czyli 2000 bajtów. Nie każdy transport może przesłać tak dużą ramkę w jednym kawałku.

Fragmentacja należy do warstwy protokołu, ponieważ jej zasady powinny być identyczne niezależnie od tego, czy transportem jest UDP, UART czy radio.

### 5.2. Proponowane pola nagłówka

Robocza propozycja nagłówka fragmentu:

| Pole | Rozmiar | Znaczenie |
|---|---:|---|
| `version` | 1 B | Wersja protokołu |
| `type` | 1 B | Typ wiadomości |
| `flags` | 1 B | Flagi wiadomości |
| `seq` | 2 B | Numer całej wiadomości |
| `fragment_index` | 1 B | Indeks fragmentu |
| `fragment_count` | 1 B | Łączna liczba fragmentów |
| `total_length` | 2 B | Rozmiar kompletnej wiadomości |
| `payload_length` | 2 B | Rozmiar danych w bieżącym fragmencie |
| **Razem** | **11 B** | |

To propozycja, a nie zatwierdzony format binarny. Przed implementacją trzeba jeszcze ustalić m.in. zakres pól, obsługę wiadomości większych niż 65535 bajtów i dokładną semantykę flag.

### 5.3. Numeracja i rozmiar fragmentów

- Wszystkie fragmenty jednej wiadomości mają ten sam `seq`.
- `seq` identyfikuje całą wiadomość, a nie pojedynczy fragment.
- Numer sekwencyjny ma roboczo 16 bitów i zawija się po osiągnięciu maksymalnej wartości.
- Maksymalny payload fragmentu jest obliczany dynamicznie:

  `payload_size = max_frame_size - header_size`

- Nie ustalamy jednej globalnej wielkości fragmentu. Różne transporty mogą mieć różne ograniczenia.

Przykład dla ramki sonaru 2000 B i ramki transportowej 1200 B:
- nagłówek: 11 B;
- maksymalny payload fragmentu: 1189 B;
- wiadomość wymaga dwóch fragmentów.

### 5.4. Odbiór i reassembly

Odbiornik:
1. sprawdza poprawność nagłówka i długości;
2. sprawdza zakres indeksu i liczbę fragmentów;
3. przypisuje fragment do wiadomości na podstawie `seq` i typu;
4. wykrywa duplikaty, aby nie dopisywać tych samych danych ponownie;
5. przekazuje wiadomość do aplikacji dopiero po odebraniu wszystkich fragmentów i walidacji całości.

Odbiornik musi mieć ograniczoną liczbę jednocześnie składanych wiadomości oraz timeout dla niekompletnych wiadomości. Niekompletne i przeterminowane wiadomości należy usuwać, aby nie zużywały pamięci bez końca.

Szczegóły buforowania, limitów i timeoutów pozostają do zaprojektowania.

### 5.5. Niezawodność zależna od rodzaju danych

Nie wszystkie dane powinny być retransmitowane w ten sam sposób.

| Rodzaj | Proponowane zachowanie |
|---|---|
| Komendy sterujące | ACK/NACK, ograniczone retransmisje i deduplikacja |
| Sonar | Odrzucanie niekompletnych lub przeterminowanych ramek; bez retransmisji starych danych |
| GPS | Preferowanie aktualnych danych; polityka retransmisji do ustalenia |
| Telemetria na żądanie | Możliwe potwierdzanie i ograniczone ponowienia |
| Heartbeat | Cykliczny PING/PONG, bez traktowania go jako ACK komend |

Deduplikacja komend jest konieczna: retransmisja tej samej komendy nie może spowodować ponownego wykonania operacji. ACK powinien odnosić się do konkretnej wiadomości/identyfikatora transakcji.

## 6. Kodowanie i walidacja

1. Nie wysyłać surowych struktur C/C++ przez `memcpy`.
2. Każde pole nagłówka i payloadu serializować jawnie.
3. Ustalić jednoznaczną kolejność bajtów (proponowana little-endian, do zatwierdzenia przed implementacją).
4. Walidować typ wiadomości, identyfikator komendy/zdarzenia, długość, indeks fragmentu i liczbę fragmentów.
5. Odrzucać niepoprawne i zbyt duże wiadomości przed kopiowaniem do buforów.
6. Rozważyć CRC na poziomie aplikacyjnym po sprawdzeniu gwarancji integralności konkretnego transportu.
7. Nie ufać danym przychodzącym z transportu – każda wiadomość musi przejść walidację.

## 7. Ograniczenia przepustowości

Surowa ramka sonaru:
- 1000 próbek;
- 2 bajty na próbkę;
- 2000 bajtów na ramkę.

Przy 10 ramkach na sekundę daje to około 20 000 B/s, czyli 160 kbit/s samych danych, bez narzutu protokołu i transportu.

Fragmentacja rozwiązuje ograniczenie maksymalnego rozmiaru pojedynczej ramki, ale **nie zwiększa przepustowości medium**. Przy wyborze radia, np. ExpressLRS lub LoRa, trzeba osobno ocenić przepływność, opóźnienia i dostępny czas antenowy. Może być konieczne zmniejszenie częstotliwości ramek, liczby próbek albo zastosowanie kompresji/wyboru danych.

## 8. Aktualny stan implementacji

| Element | Stan |
|---|---|
| Pico 2 W, FreeRTOS | W użyciu |
| `CommTask` i `CommService` | Istnieją |
| `ControlTask` | Istnieje |
| `SonarTask` z generatorem testowym | Działa |
| `TelemetryTask` | W trakcie rozwoju / testów |
| `DataQueue` | Wspólna ścieżka przekazywania danych |
| UDP/Wi-Fi | Działa jako transport tymczasowy |
| `ITransport::max_frame_size()` | Dodane |
| `ITransport::is_ready()` | Dodane |
| Ochrona przed obcięciem odebranego datagramu | Dodana |
| Nowy format protokołu | Jeszcze niezaimplementowany |
| Fragmentacja i składanie wiadomości | Do implementacji |
| ACK/NACK z deduplikacją | Do implementacji w nowym protokole |
| PING/PONG i watchdog łączności | Do implementacji |
| GPS i sterowanie napędem | Przyszły etap |

## 9. Proponowana kolejność implementacji

1. Zatwierdzić typy wiadomości, komendy, zdarzenia i kody odpowiedzi.
2. Zatwierdzić format nagłówka, identyfikatory, endianness i limity rozmiarów.
3. Zaimplementować serializację i deserializację.
4. Zaimplementować fragmentację po stronie nadawczej.
5. Zaimplementować składanie i walidację fragmentów po stronie odbiorczej.
6. Dodać ACK/NACK, retransmisje i deduplikację.
7. Dodać PING/PONG i reakcję na utratę łączności.
8. Przetestować utratę, duplikację, zmianę kolejności i uszkodzenie fragmentów.
9. Dopiero po wyborze docelowego medium dostroić konkretny transport.

## 10. Otwarte decyzje

Przed uznaniem formatu protokołu za zamknięty należy rozstrzygnąć:
- ostateczny format nagłówka i wersjonowanie;
- szerokość pól długości i numeru sekwencyjnego;
- kolejność bajtów;
- identyfikację transakcji i format ACK/NACK;
- timeouty, limity buforowania i liczbę składanych wiadomości;
- politykę retransmisji telemetrii i GPS;
- potrzebę CRC;
- interwał i timeout heartbeat oraz zachowanie awaryjne;
- docelowy transport i jego realną przepustowość.

---

**Ważne:** dokument rozdziela stan obecnej implementacji od proponowanego projektu. Tabel i pól opisanych jako propozycje nie należy traktować jako już obowiązującego formatu transmisji.
