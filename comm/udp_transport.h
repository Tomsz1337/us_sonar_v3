#pragma once

#include "transport.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "FreeRTOS.h"
#include "queue.h"

class UdpTransport : public ITransport 
{
public:
    UdpTransport(ip_addr_t remote_ip, uint16_t port, uint8_t ssid,
                 uint8_t password, uint16_t local_port);
    ~UdpTransport() override;

    bool init() override;
    bool send(std::span<const uint8_t> data) override;
    bool receive(std::span<uint8_t> buffer, size_t &received_bytes) override;
    bool is_connected() const override;

private:
    ip_addr_t m_remote_ip;
    uint16_t m_remote_port;
    uint16_t m_local_port;
    udp_pcb *m_pcb{nullptr};
    const char m_ssid;
    const char m_password;


    static void udp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                            const ip_addr_t *addr, u16_t port);

    void handle_incoming_pbuf(struct pbuf *p, const ip_addr_t *addr, u16_t port);

    struct RxPacket{
        struct pbuf *pbuf_ptr;
    };
    xQueueHandle m_rx_queue{nullptr};
};