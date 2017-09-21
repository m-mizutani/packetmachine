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

- Expected length: defined by `ARP.hw_size`
- Recommended formatting method: `hex()`. If `ARP.pr_type` is `0x0800`, then `ip4()`

### `ARP.pr_dst`

Destination protocol address.

- Expected length: defined by `ARP.hw_size`
- Recommended formatting method: `hex()`. If `ARP.pr_type` is `0x0800`, then `ip4()`

PPPoE
---------

to be written.

IPv4
---------

to be written.

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


