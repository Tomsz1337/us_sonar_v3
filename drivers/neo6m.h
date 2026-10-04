#pragma once

#include <cstdint>
#include "hal/uart_hal.h"
#include "FreeRTOS.h"
#include "task.h"

class NEO6M
{
public:
    struct Data {
        bool valid{false};
        bool is_fixed{false};
        uint8_t hour{0};
        uint8_t minute{0};
        uint8_t second{0};
        double latitude{0.0}; 
        char lat_dir{'N'};
        double longitude{0.0};  
        char lon_dir{'E'};
        float speed_kmh{0.0f};
        float altitude{300.0f};
        uint8_t satellites{0};
    };

    explicit NEO6M(uart_inst_t* uart) : m_uart(uart) {};
    bool init(uint32_t timeout_ms = 10000);
    void process();

    [[nodiscard]] double get_latitude() const {return m_data.latitude;}
    [[nodiscard]] double get_longitude() const {return m_data.longitude;}
    [[nodiscard]] float get_altitude() const {return m_data.altitude;}
    [[nodiscard]] bool is_fixed() const {return m_data.is_fixed;}
    [[nodiscard]] const Data& get_data() const {return m_data;}

private:

    static bool verify_checksum(const char* line);
    void parse_line();
    static double nmea_to_decimal(double nmea_coord, char direction);
    void parse_gprmc(char* line);
    void parse_gpgga(char* line);

    uart_inst_t* m_uart;
    Data m_data{};
    char m_line_buf[128]{0};
    size_t m_buf_idx{0};
};