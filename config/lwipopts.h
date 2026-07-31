#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Zapobieganie konfliktom typów z Newlib
#define LWIP_TIMEVAL_PRIVATE        0
#define LWIP_ERRNO_INCLUDE          <errno.h>

// --- Środowisko i system operacyjny (FreeRTOS) ---
#define NO_SYS                      0
#define SYS_LIGHTWEIGHT_PROT        1

#define LWIP_NETCONN                1
#define LWIP_SOCKET                 1

// --- Wątki i Priorytety lwIP ---
#define TCPIP_THREAD_NAME           "TCP/IP"
#define TCPIP_THREAD_STACKSIZE      2048
#define TCPIP_THREAD_PRIO           (configMAX_PRIORITIES - 2)

#define DEFAULT_THREAD_STACKSIZE    1024
#define DEFAULT_THREAD_PRIO         1

#define TCPIP_MBOX_SIZE             16
#define DEFAULT_RAW_RECVMBOX_SIZE   16
#define DEFAULT_UDP_RECVMBOX_SIZE   16
#define DEFAULT_TCP_RECVMBOX_SIZE   16
#define DEFAULT_ACCEPTMBOX_SIZE     16

// --- Pamięć RAM dla stosu sieciowego ---
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    (24 * 1024)

#define MEMP_NUM_PBUF               16
#define MEMP_NUM_UDP_PCB            6
#define MEMP_NUM_TCP_PCB            10
#define MEMP_NUM_TCP_PCB_LISTEN     5
#define MEMP_NUM_TCPIP_MSG_API      8
#define MEMP_NUM_TCPIP_MSG_INPKT    16

#define PBUF_POOL_SIZE              24
#define PBUF_POOL_BUFSIZE           1500

// --- Protokoły i Diagnostyka ---
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DHCP                   1
#define LWIP_DNS                    1

#define LWIP_CHKSUM_ALGORITHM       3

#define LWIP_STATS                  0
#define LWIP_STATS_DISPLAY          0

#endif /* _LWIPOPTS_H */
