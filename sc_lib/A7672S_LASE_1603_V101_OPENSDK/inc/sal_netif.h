#ifndef __SAL_NETIF_H__
#define __SAL_NETIF_H__


#ifdef __cplusplus
extern "C" {
#endif

void lwip_set_tcp_syn_max_rtx(int max_rtx);
void lwip_set_tcp_sync_backoff(unsigned char * backoff_cfg, int size_n);
void lwip_set_tcp_max_rtx(int max_rtx);
void lwip_set_tcp_backoff(unsigned char * backoff_cfg, int size_n);
void lwip_set_netif_pdp_mtu(unsigned short mtu, char cid);
int lwip_set_tcp_rcv_win(unsigned int value);
void lwip_set_dns_max_retries(unsigned int value);
void lwip_set_dns_wait_tmr(unsigned char * wait_cfg, int size_n);
int lwip_get_tcp_syn_max_rtx(void);
void lwip_get_tcp_sync_backoff(unsigned char * backoff_read, int size_n);
int lwip_get_tcp_max_rtx(void);
void lwip_get_tcp_backoff(unsigned char * backoff_read, int size_n);
unsigned short lwip_get_netif_pdp_mtu(void);
unsigned int lwip_get_tcp_rcv_win(void);
unsigned int lwip_get_dns_max_wait_time(void);
unsigned int lwip_get_dns_max_retries(void);
void lwip_get_dns_wait_tmr(unsigned char * wait_read, int size_n);



#define SAL_NETIF_ERR_OK              (0)
#define SAL_NETIF_ERR_DEFAULT         (-1)
#define SAL_NETIF_ERR_PARAM_INVALID   (-2)


/**
 * @brief set the IPv4 DNS server addresses.
 *
 * @param Cid the apn cid, valid range is 1 to 6
 * @param dns1 the primary IPv4 DNS server address
 * @param dns2 the secondary IPv4 DNS server address
 * @return SAL_NETIF_ERR_OK on success, error code on failure
 */
int sal_netif_dns_set(char Cid, char *dns1, char *dns2);

/**
 * @brief get the IPv4 DNS server addresses.
 *
 * @param Cid the apn cid, valid range is 1 to 6
 * @param dns1 the primary IPv4 DNS server address
 * @param dns2 the secondary IPv4 DNS server address
 * @return SAL_NETIF_ERR_OK on success, error code on failure
 */
int sal_netif_dns_get(char Cid, char *dns1, char *dns2);

/**
 * @brief set the IPv6 DNS server addresses.
 *
 * @param Cid the apn cid, valid range is 1 to 6
 * @param dns1 the primary IPv6 DNS server address
 * @param dns2 the secondary IPv6 DNS server address
 * @return SAL_NETIF_ERR_OK on success, error code on failure
 */
int sal_netif_dnsv6_set(char Cid, char *dns1, char *dns2);

/**
 * @brief get the IPv6 DNS server addresses.
 *
 * @param Cid the apn cid, valid range is 1 to 6
 * @param dns1 the primary IPv6 DNS server address
 * @param dns2 the secondary IPv6 DNS server address
 * @return SAL_NETIF_ERR_OK on success, error code on failure
 */
int sal_netif_dnsv6_get(char Cid, char *dns1, char *dns2);

#ifdef __cplusplus
}
#endif

#endif /* __SAL_NETIF_H__ */
