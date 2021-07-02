#include "lib/include/helpers.h"
#include "lib/include/npu.h"

static void * bare_memcpy(void *to, const void *from, size_t nr)
{
    size_t i;

    for (i = 0; i < nr; i++)
        *(((uint8_t *)to) + i) = *(((const uint8_t *)from) + i);

    return to;
}
void * memcpy(void *, const void *, size_t)
__attribute__((weak, alias("bare_memcpy")));

static void * bare_memset(void *to, int val, size_t nr)
{
    size_t i;

    for (i = 0; i < nr; i++)
        *(((uint8_t *)to) + i) = val;

    return to;
}
void * memset(void *, int, size_t)
__attribute__((weak, alias("bare_memset")));

static int bare_memcmp(const void *a, const void *b, size_t nr)
{
    size_t i;
    const uint8_t *pa = a, *pb = b;

    for (i = 0; i < nr; ++i) {
        if (pa[i] == pb[i])
            continue;
        else if (pa[i] < pb[i])
            return -1;
        else
            return 1;
    }
    return 0;
}
int memcmp(const void *, const void *, size_t)
__attribute__((weak, alias("bare_memcmp")));

uint32_t get_mac_hash(const struct ethaddr *mac)
{
    const uint8_t *s = mac->octets;
    unsigned sz = sizeof(mac->octets); // sz = 6
    uint8_t sum = 0, tmp = 0;

    while (sz-- > 0) {
        tmp += *s++;
        sum += tmp;
    }

    return sum % HASHNODE_CAPACITY;
}

uint32_t get_ip_hash(const struct ipaddr *ip)
{
    const uint8_t *s = ip->octets;
    unsigned sz = sizeof(ip->octets);
    uint8_t sum = 0, tmp = 0;

    while (sz-- > 0) {
        tmp += *s++;
        sum += tmp;
    }

    return sum % MCAST_HASHNODE_CAPACITY;
}

int is_bcast_mac(const struct ethaddr *mac)
{
    static struct ethaddr bmac = {
        .octets = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
    };

    return is_same_mac(mac, &bmac);
}

int is_mcast_mac(const struct ethaddr *a)
{
    static struct ethaddr mcast_mask = {
        .octets = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x00 }
    };

    uint8_t i;
    for (i = 0; i < 3; i++)
        if (a->octets[i] ^ mcast_mask.octets[i])
            return 0;

    if (a->octets[3] & 0x80)
        return 0;

    return 1;
}

int is_same_mac(const struct ethaddr *a, const struct ethaddr *b)
{
    return 0 == memcmp(a->octets, b->octets, sizeof(a->octets));
}

int is_same_ip(const struct ipaddr *a, const struct ipaddr *b)
{
    return 0 == memcmp(a->octets, b->octets, sizeof(a->octets));
}

int is_ip_checksum_correct(const uint16_t *data, size_t len)
{
    uint32_t res = 0;
    while (len) {
        res += NTOH16(*data++);
        len -= 2;
    }
    return !(~((res >> 16) + (res & 0xffff)) & 0xffff);
}

uint16_t ip_checksum(const uint16_t *data, size_t len, uint16_t crc)
{
    uint32_t res = 0;
    while (len) {
        res += NTOH16(*data++);
        len -= 2;
    }
    res -= crc;

    return ~((res >> 16) + (res & 0xffff)) & 0xffff;
}
