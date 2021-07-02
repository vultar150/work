#ifndef NPU_LIB_HELPERS_H
#define NPU_LIB_HELPERS_H

#include "lib/include/packet.h"

#if !defined(MIN)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) ((sizeof(x) / sizeof((x)[0])))
#endif

#if !defined(NULL)
#define NULL ((void *) 0)
#endif

/**
 * Returns a pointer to a node of the hashtable corresponding to the hash value.
 *
 * @param tbl The hashtable.
 * @param hash The hash value.
 * */
#if !defined(HASHTABLE_GET)
#define HASHTABLE_GET(tbl, hash) ((tbl)->nodes[(hash)])
#endif

#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

/**
 * Atomic operation for incrementing the value.
 * */
#if defined(__clang__)
#define COUNTER_ATOMIC_INC(x) (__atomic_fetch_add((&x), 1, __ATOMIC_SEQ_CST))
#elif defined(__GNUC__)
#define COUNTER_ATOMIC_INC(x) (__sync_fetch_and_add(&(x), 1))
#else
#error "Atomic operation for your compiler is not defined!"
#endif

/**
 * Convert the data from network to host byte order
 * and vice versa.
 * */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#define NTOH16(x) (x)
#define NTOH32(x) (x)
#define NTOH64(x) (x)

#define HTON16(x) (x)
#define HTON32(x) (x)
#define HTON64(x) (x)

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define NTOH16(x) (__builtin_bswap16(x))
#define NTOH32(x) (__builtin_bswap32(x))
#define NTOH64(x) (__builtin_bswap64(x))

#define HTON16(x) (__builtin_bswap16(x))
#define HTON32(x) (__builtin_bswap32(x))
#define HTON64(x) (__builtin_bswap64(x))

#endif

typedef unsigned long size_t;

/**
 * In code related to NPU we don't use any libraries,
 * so these functions are implemented to help the programmer
 * and have the same behaviour as the ones from the standart library.
 * */
void * memcpy(void *to, const void *from, size_t size);
void * memset(void *to, int val, size_t size);
int memcmp(const void *a, const void *b, size_t size);

/**
 * Hash function for Ethernet MAC address.
 *  */
uint32_t get_mac_hash(const struct ethaddr *mac);
/**
 * Hash function for IPv4 address.
 *  */
uint32_t get_ip_hash(const struct ipaddr *ip);

/**
 * Returns 1 if MAC address is FF:FF:FF:FF:FF:FF,
 * otherwise returns 0.
 * */
int is_bcast_mac(const struct ethaddr *mac);

/**
 * Returns 1 if MAC address is in multicast class (01:00:5E:**:**:**),
 * otherwise returns 0.
 */
int is_mcast_mac(const struct ethaddr *mac);

/**
 * Returns 1 if two MAC addresses are similar,
 * otherwise returns 0.
 * */
int is_same_mac(const struct ethaddr *a, const struct ethaddr *b);

/**
 * Returns 1 if two IPv4 addresses are similar,
 * otherwise returns 0.
 * */
int is_same_ip(const struct ipaddr *a, const struct ipaddr *b);

/**
 * Returns 1 if checksum of IPv4 header is correct,
 * otherwise returns 0.
 * */
int is_ip_checksum_correct(const uint16_t *data, size_t len);

/**
 * Calculates checksum of IPv4 header.
 * */
uint16_t ip_checksum(const uint16_t *data, size_t len, uint16_t crc);

#endif //NPU_LIB_HELPERS_H
