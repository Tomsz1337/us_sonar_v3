#include "udp_transport.h"

UdpTransport::UdpTransport(ip_addr_t remote_ip, uint16_t port, const char *ssid,
                 const char *password, uint16_t local_port)
    : m_remote_ip(remote_ip), m_remote_port(port), m_ssid(ssid), m_password(password), m_local_port(local_port)
{}

UdpTransport::~UdpTransport()
{
    if(m_pcb)
    {
        udp_remove(m_pcb);
        m_pcb = nullptr;
    }
    if(m_rx_queue)
    {
        RxPacket pkt;
        while(xQueueReceive(m_rx_queue, &pkt, 0) == pdTRUE)
        {
            if(pkt.pbuf_ptr)
            {
                pbuf_free(pkt.pbuf_ptr);
            }
        }
        vQueueDelete(m_rx_queue);
    }
}

bool UdpTransport::init() 
{
    m_rx_queue = xQueueCreate(8, sizeof(RxPacket));
    if(!m_rx_queue) return false;

    int res = cyw43_arch_init();
    if(res != 0) 
    {
        return false;
    }

    cyw43_arch_enable_ap_mode(m_ssid, m_password, CYW43_AUTH_WPA2_AES_PSK);

    m_pcb = udp_new();
    if(!m_pcb) return false;

    if(udp_bind(m_pcb, IP_ANY_TYPE, m_local_port) != ERR_OK) 
    {
        udp_remove(m_pcb);
        m_pcb = nullptr;
        return false;
    }
    udp_recv(m_pcb, udp_recv_cb, this);
    return true;
}

void UdpTransport::udp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                                     const ip_addr_t *addr, u16_t port) 
{
    if(!arg || !p) return;
    auto *instance = static_cast<UdpTransport*>(arg);
    instance->handle_incoming_pbuf(p, addr, port);
}

void UdpTransport::handle_incoming_pbuf(struct pbuf *p, const ip_addr_t *addr, u16_t port) 
{
    RxPacket pkt{ .pbuf_ptr = p };
    if(xQueueSend(m_rx_queue, &pkt, 0) != pdTRUE) 
    {
        pbuf_free(p);
    }
}

bool UdpTransport::send(std::span<const uint8_t> data) 
{
    if(!m_pcb || data.empty()) return false;

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, data.size(), PBUF_RAM);
    if (!p) return false;
    memcpy(p->payload, data.data(), data.size());
    err_t err = udp_sendto(m_pcb, p, &m_remote_ip, m_remote_port);
    pbuf_free(p);
    return (err == ERR_OK);
}

bool UdpTransport::receive(std::span<uint8_t> buffer, size_t &received_bytes) 
{
    received_bytes = 0;
    if(!m_rx_queue) return false;

    RxPacket pkt;
    if(xQueueReceive(m_rx_queue, &pkt, 0) == pdTRUE) 
    {
        if(!pkt.pbuf_ptr) return false;

        size_t bytes_to_copy = std::min(static_cast<size_t>(pkt.pbuf_ptr->tot_len), buffer.size());
        pbuf_copy_partial(pkt.pbuf_ptr, buffer.data(), bytes_to_copy, 0);
        
        received_bytes = bytes_to_copy;
        pbuf_free(pkt.pbuf_ptr);
        return true;
    }
    return false;
}

bool UdpTransport::is_connected() const 
{
    return m_pcb != nullptr;
}