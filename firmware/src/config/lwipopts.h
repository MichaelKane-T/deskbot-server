#ifndef LWIPOPTS_H
#define LWIPOPTS_H

#define NO_SYS                     1
#define LWIP_SOCKET                0
#define LWIP_NETCONN               0

#define MEM_ALIGNMENT              4
#define MEM_SIZE                   2000

#define MEMP_NUM_TCP_PCB           3
#define MEMP_NUM_TCP_SEG           16
#define PBUF_POOL_SIZE             8
#define PBUF_POOL_BUFSIZE          512

#define TCP_SND_QUEUELEN           16

#define LWIP_TCP                   1
#define TCP_TTL                    255
#define TCP_QUEUE_OOSEQ            0
#define TCP_SND_BUF                2920
#define TCP_WND                    2920

#define LWIP_IPV4                  1
#define LWIP_IPV6                  0
#define LWIP_DNS                   0
#define LWIP_DHCP                  1

#endif