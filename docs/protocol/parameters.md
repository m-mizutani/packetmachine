Parameters
===========

Ethernet
-----------

### `Ethernet.type`

Ethernet frame type.

- Expected length: 2 byte
- Recommended formatting method: `uint()`

### `Ethernet.src`

Source hardware address.

- Expected length: 6 byte
- Recommended formatting method: `mac()`

### `Ethernet.dst`

Destination hardware address.

- Expected length: 6 byte
- Recommended formatting method: `mac()`

ARP
-------------

### `ARP.hw_type`

Hardware type

- Expected length: 2 byte
- Recommended formatting method: `uint()`

### `ARP.pr_type`

Protocol type

- Expected length: 2 byte
- Recommended formatting method: `uint()`

### `ARP.hw_size`

Hardware address size

- Expected length: 1 byte
- Recommended formatting method: `uint()`

### `ARP.pr_size`

Protocol address size

- Expected length: 1 byte
- Recommended formatting method: `uint()`


### `ARP.op`

ARP operation. `1` means a request, `2` means a reply.

- Expected length: 1 byte
- Recommended formatting method: `uint()`

### `ARP.hw_src`

Source hardware address.

- Expected length: defined by `ARP.hw_size`
- Recommended formatting method: `hex()`. If `ARP.hw_type` is `1`, then `mac()`

### `ARP.hw_dst`

Destination hardware address.

- Expected length: defined by `ARP.hw_size`
- Recommended formatting method: `hex()`. If `ARP.hw_type` is `1`, then `mac()`

### `ARP.pr_src`

Source protocol address.

- Expected length: defined by `ARP.pr_size`
- Recommended formatting method: `hex()`. If `ARP.pr_type` is `0x0800`, then `ip4()`

### `ARP.pr_dst`

Destination protocol address.

- Expected length: defined by `ARP.pr_size`
- Recommended formatting method: `hex()`. If `ARP.pr_type` is `0x0800`, then `ip4()`

PPPoE
---------

to be written.

IPv4
---------

### `IPv4.hdr_len`

IPv4 Header length. In IPv4 header, the header length field should be bit shift. PacketMachine must shift the bits before a user access.

- Expected length: 1 byte (originally 4 bits)
- Recommended formatting method: `uint()`

### `IPv4.ver`

IPv4 version

- Expected length: 1 byte (originally 4 bits)
- Recommended formatting method: `uint()`

### `IPv4.tos`

Type of services.

- Expected length: 1 byte
- Recommended formatting method: `uint()`

### `IPv4.total_len`

Total length of IP header and data.

- Expected length: 2 byte
- Recommended formatting method: `uint()`

### `IPv4.id`

ID of IP packet.

- Expected length: 2 byte
- Recommended formatting method: `uint()`

### `IPv4.offset`

Offset of IP fragmentation.

- Expected length: 2 byte
- Recommended formatting method: `uint()`

### `IPv4.ttl`

Time to Live.

- Expected length: 1 byte
- Recommended formatting method: `uint()`

### `IPv4.proto`

Protocol number of next layer.

- Expected length: 1 byte
- Recommended formatting method: `uint()`

### `IPv4.chksum`

Checksum of IP header.

- Expected length: 2 byte
- Recommended formatting method: `hex()`, `raw()` or `uint()`

### `IPv4.src",

Source IP address.

- Expected length: 4 byte
- Recommended formatting method: `ip4()`

### `IPv4.dst",

Destination IP address.

- Expected length: 4 byte
- Recommended formatting method: `ip4()`


UDP
---------

to be written.

TCP
---------

to be written.

ICMP
---------

to be written.

DNS
---------

to be written.

mDNS
---------

to be written.

DHCP
---------

to be written.


