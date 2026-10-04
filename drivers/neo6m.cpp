#include "neo6m.h"
#include "pico/time.h"
#include <cstdint>
#include <cstring>
#include <cstdlib>

bool NEO6M::init(uint32_t timeout_ms)
{
    m_data = Data{};
    uint64_t start_time = time_us_64();

    while (!m_data.is_fixed)
    {
        if ((time_us_64() - start_time) > (timeout_ms * 1000ULL))
        {
            return false;
        }
        process();
        sleep_ms(10);
    }
    return true;
}

void NEO6M::process()
{
    while (uart_is_readable(m_uart))
    {
        char c = static_cast<char>(uart_getc(m_uart));
        if (c == '$')
        {
            m_buf_idx = 0;
            m_line_buf[m_buf_idx++] = c;
        }
        else if (c == '\n' || c == '\r')
        {
            if (m_buf_idx > 0)
            {
                m_line_buf[m_buf_idx] = '\0';
                parse_line();
                m_buf_idx = 0;
            }
        }
        else if (m_buf_idx < sizeof(m_line_buf) - 1)
        {
            m_line_buf[m_buf_idx++] = c;
        }
        else
        {
            m_buf_idx = 0;
        }
    }
}

bool verify_checksum(const char *line)
{
    if (line == nullptr || line[0] != '$')
        return false;

    const char *star = strchr(line, '*');
    if (star == nullptr)
        return false;

    uint8_t calculated = 0;
    for (const char *p = line + 1; p < star; ++p)
    {
        calculated ^= static_cast<uint8_t>(*p);
    }
    uint8_t expected = static_cast<uint8_t>(strtol(star + 1, nullptr, 16));
    return calculated == expected;
}

void NEO6M::parse_line()
{
    if (!verify_checksum(m_line_buf))
    {
        return;
    }

    if (strncmp(m_line_buf, "$GPGGA", 6) == 0)
    {
        parse_gpgga(m_line_buf);
    }
    else if (strncmp(m_line_buf, "$GPRMC", 6) == 0)
    {
        parse_gprmc(m_line_buf);
    }
}

double NEO6M::nmea_to_decimal(double nmea_coord, char direction)
{
    if (nmea_coord == 0.0)
        return 0.0;
    int degrees = static_cast<int>(nmea_coord / 100.0);
    double minutes = nmea_coord - (degrees * 100.0);
    double decimal = degrees + (minutes / 60.0);
    if (direction == 'S' || direction == 'W')
    {
        decimal *= -1.0;
    }

    return decimal;
}

void NEO6M::parse_gprmc(char *line)
{
    int field_index = 0;
    char *token;
    char *rest = line;

    while ((token = strsep(&rest, ",")) != nullptr)
    {
        switch (field_index)
        {
        case 1:
            if (strlen(token) >= 6)
            {
                int time = atoi(token);
                m_data.hour = static_cast<uint8_t>(time / 10000);
                m_data.minute = static_cast<uint8_t>((time % 10000) / 100);
                m_data.second = static_cast<uint8_t>(time % 100);
            }
            break;
        case 2:
            m_data.valid = (token[0] == 'A');
            break;
        case 3:
            m_data.latitude = atof(token);
            break;
        case 4:
            m_data.lat_dir = token[0];
            break;
        case 5:
            m_data.longitude = atof(token);
            break;
        case 6:
            m_data.lon_dir = token[0];
            break;
        case 7:
            m_data.speed_kmh = static_cast<float>(atof(token) * 1.852);
            break;
        default:
            break;
        }
        field_index++;
    }
    if (m_data.valid)
    {
        m_data.latitude = nmea_to_decimal(m_data.latitude, m_data.lat_dir);
        m_data.longitude = nmea_to_decimal(m_data.longitude, m_data.lon_dir);
    }
}

void NEO6M::parse_gpgga(char *line)
{
    int field_index = 0;
    char *token;
    char *rest = line;

    while ((token = strsep(&rest, ",")) != nullptr)
    {
        switch (field_index)
        {
        case 6:
            m_data.is_fixed = (atoi(token) > 0);
            break;
        case 7:
            m_data.satellites = static_cast<uint8_t>(atoi(token));
            break;
        case 9:
            m_data.altitude = static_cast<float>(atof(token));
            break;
        default:
            break;
        }
        field_index++;
    }
}