Parameters
===========

Ethernet
----------------

| Parameter name           | Description                                 | Expected length               | Recommended formatting |
|:-------------------------|:--------------------------------------------|:------------------------------|:-----------------------|
| `Ethernet.type`          | Ethernet frame type                         | 2 byte                       | `uint()`               |
| `Ethernet.src`           | Source hardware address                     | 6 byte                        | `mac()`
| `Ethernet.dst`           | Destination hardware address.               |  6 byte | `mac()` |
| `Ethernet.type`          | Ethernet frame type.                        |  2 byte |  `uint()`|

ARP
----------------

| Parameter name           | Description                                 | Expected length               | Recommended formatting |
|:-------------------------|:--------------------------------------------|:------------------------------|:---------------------
| `ARP.hw_type`            | Hardware type                               |  2 byte |  `uint()` |
| `ARP.pr_type`            | Protocol type                               |  2 byte |  `uint()` |
| `ARP.hw_size`            | Hardware address size                       |  1 byte |  `uint()` |
| `ARP.pr_size`            | Protocol address size                       |  1 byte |  `uint()` |
| `ARP.op`                 | ARP operation. `1` means a request, `2` means a reply. |  1 byte |  `uint()` |
| `ARP.hw_src`             | Source hardware address.                    |  defined by `ARP.hw_size` |  `hex()`. If `ARP.hw_type` is `1`, then `mac()` |
| `ARP.hw_dst`             | Destination hardware address. |  defined by `ARP.hw_size` |  `hex()`. If `ARP.hw_type` is `1`, then `mac()` |
| `ARP.pr_src` | Source protocol address. |  defined by `ARP.pr_size` |  `hex()`. If `ARP.pr_type` is `0x0800`, then `ip4()` |
| `ARP.pr_dst` | Destination protocol address. |  defined by `ARP.pr_size` |  `hex()`. If `ARP.pr_type` is `0x0800`, then `ip4()` |

IPv4
---------------

| Parameter name           | Description                                 | Expected length               | Recommended formatting |
|:-------------------------|:--------------------------------------------|:------------------------------|:-----------------------|
| `IPv4.hdr_len`           | IPv4 Header length. In IPv4 header, the header length field should be bit shift. PacketMachine must shift the bits before a user access. |  1 byte (originally 4 bits) |  `uint()` |
| `IPv4.ver`               | IPv4 version                                |  1 byte (originally 4 bits) |  `uint()` |
| `IPv4.tos`               | Type of services.                           |  1 byte |  `uint()` |
| `IPv4.total_len`         | Total length of IP header and data.         |  2 byte |  `uint()` |
| `IPv4.id`                | ID of IP packet.                            |  2 byte |  `uint()` |
| `IPv4.offset`            | Offset of IP fragmentation.                 |  2 byte |  `uint()` |
| `IPv4.ttl`               | Time to Live.                               |  1 byte |  `uint()` |
| `IPv4.proto`             | Protocol number of next layer.              |  1 byte |  `uint()` |
| `IPv4.chksum`            | Checksum of IP header.                      |  2 byte |  `hex()`, `raw()` or `uint()` |
| `IPv4.src`               | Source IP address.                          |  4 byte |  `ip4()`  |
| `IPv4.dst`               | Destination IP address.                     |  4 byte |  `ip4()`  |


UDP
---------

| Parameter name | Description                  | Expected length   | Recommended formatting |
|:---------------|:-----------------------------|:------------------|:-----------------------|
| `UDP.src_port` | UDP source port number.      |  2 byte           |  `uint()` |
| `UDP.dst_port` | UDP destination port number. |  2 byte           |  `uint()` |
| `UDP.length`   | UDP header and data length.  |  2 byte           |  `uint()` |
| `UDP.chksum`   | Checksum.                    |  2 byte           |  `uint()` or `hex()` | 


TCP
---------

| Parameter name  | Description                          | Expected length   | Recommended formatting |
|:----------------|:-------------------------------------|:------------------|:-----------------------|
| `TCP.src_port`  | TCP source port number.              |  2 byte |  `uint()` |
| `TCP.dst_port`  | TCP destination port number.         |  2 byte |  `uint()` |
| `TCP.seq`       | Sequence number of TCP stream.       |  4 byte |  `uint()` |
| `TCP.ack`       | Acknowledgment number of TCP stream. |  4 byte |  `uint()` |
| `TCP.offset`    | Data offset.                         |  1 byte |  `uint()` | 
| `TCP.window`    | Window size.                         |  2 byte |  `uint()` |
| `TCP.chksum`    | Checksum.                            |  2 byte |  `uint()` or `hex()` |
| `TCP.urgptr`    | Urgent Pointer.                      |  2 byte |  `uint()` |
| `TCP.flags`     | TCP flag section. See also `TCP.flag_{fin,syn,rst,push,ack,usrg,ece,cwr}`     |  1 byte |  `uint()` |
| `TCP.flag_fin`  | FIN flag status. If the flag is on, the parameter should be `1`. If not, `0`. |  1 byte |  `uint()` |
| `TCP.flag_syn`  | SYN flag status. If the flag is on, the parameter should be `1`. If not, `0`. |  1 byte |  `uint()` |
| `TCP.flag_rst`  | RST(Reset the connection) flag status. If the flag is on, the parameter should be `1`. If not, `0`.             |  1 byte |  `uint()` |
| `TCP.flag_push` | PSH(Push Function) flag status. If the flag is on, the parameter should be `1`. If not, `0`.                    |  1 byte |  `uint()` |
| `TCP.flag_ack`  | ACK(Acknowledgment field significant) flag status. If the flag is on, the parameter should be `1`. If not, `0`. |  1 byte |  `uint()` |
| `TCP.flag_urg`  | URG(Urgent Pointer field significant) flag status. If the flag is on, the parameter should be `1`. If not, `0`. |  1 byte |  `uint()` |
| `TCP.flag_ece`  | ECE(ECN Echo) flag status. If the flag is on, the parameter should be `1`. If not, `0`.                         |  1 byte |  `uint()` |
| `TCP.flag_cwr`  | CWR(Congestion window reduced) flag status. If the flag is on, the parameter should be `1`. If not, `0`.        |  1 byte |  `uint()` |
| `TCP.optdata`   | TCP option field.                         |  `TCP.offset` - 28 byte (fixed TCP header length) |  `hex()` or `raw()` |
| `TCP.segment`   | TCP data segment field. (Not reassembled) |  N/A | `hex()` or `raw()` |
| `TCP.data`      | TCP data segment that is reassembled.     |  N/A | `hex()` or `raw()` |
| `TCP.ssn_id`    | TBW | TBW | TBW |


ICMP
---------

| Parameter name  | Description | Expected length   | Recommended formatting |
|:----------------|:------------|:------------------|:-----------------------|
| `ICMP.type`     | ICMP type.  |  1 byte          |  `uint()`, Support `repr()` to convert from type number to description.  (e.g. `3` to `Destination Unreachable`) |
| `ICMP.code`     | ICMP code.  |  1 byte          |  `uint()`,  Support `repr()` to convert from code number to description. (e.g. If `ICMP.type` is `3` and `ICMP.code` is `1`, then `Host Unreachable`) |
| `ICMP.chksum`   | Checksum.   |  2 byte          |  `uint()` or `hex()` |


DNS
---------

| Parameter name   | Description                                       | Expected length   | Recommended formatting |
|:-----------------|:--------------------------------------------------|:------------------|:-----------------------|
| `DNS.tx_id`      | Transaction ID.                                   |  2 byte |  `uint()` |
| `DNS.is_query`   | If QUERY flag is on.                              |  1 byte |  `uint()` |
| `DNS.question`   | DNS question section. (Type: `pm::value::Array`)  | N/A | see below | 
| `DNS.answer`     | DNS answer section (Type: `pm::value::Array`)     | N/A | see below |
| `DNS.authority`  | DNS authority section (Type: `pm::value::Array`)  | N/A | see below |
| `DNS.additional` | DNS additional section (Type: `pm::value::Array`) | N/A | see below |

### [How to handle DNS sections](#how-to-dns-section)

`DNS.question`, `DNS.answer`, `DNS.authority` and `DNS.additional` are `pm::value::Array`.

- The parameter has `pm::value::Map` type value(s) in the array as one DNS record
- The `pm::value::Map` type value has following attributes

| Key name | Description      | Expected length   | Recommended formatting |
|:---------|:-----------------|:------------------|:-----------------------|
| `name`   | DNS record name. |  N/A     | `repr()` method supports string reconstruction with label. |
| `type`   | DNS record type. |  2 byte | `uint()` or `repr()`, `repr()` method supports to show record type name. (e.g. convert `5` to `CNAME`) |
| `data`   | DNS record data. |  N/A     | `repr()` method supports string reconstruction with label. |


Example of how to handle `DNS.answer` parameter.

```cpp
void callback(const pm::Property& p) {
  if (p.has_value("DNS.answer")) {
    const auto& records = p.value("DNS.answer");
    for (size_t i = 0; i < records.size(); i++) {
      const auto& rec = records.get(i);
  	std::cout << "Answer No." << (i + 1) << std::endl 
          << "Name: " << rec.find("name") << std::endl
          << "Type: " << rec.find("type") << std::endl
          << "Data: " << rec.find("data") << std::endl << std::endl;
    }
  }
}
```




mDNS
---------

| Parameter name   | Description                                       | Expected length   | Recommended formatting |
|:-----------------|:--------------------------------------------------|:------------------|:-----------------------|
| `mDNS.tx_id`      | Transaction ID.                                   |  2 byte |  `uint()` |
| `mDNS.is_query`   | If QUERY flag is on.                              |  1 byte |  `uint()` |
| `mDNS.question`   | mDNS question section. (Type: `pm::value::Array`)  | N/A | see [DNS](parameters.md#how-to-dns-section) | 
| `mDNS.answer`     | mDNS answer section (Type: `pm::value::Array`)     | N/A | see [DNS](parameters.md#how-to-dns-section) | 
| `mDNS.authority`  | mDNS authority section (Type: `pm::value::Array`)  | N/A | see [DNS](parameters.md#how-to-dns-section) | 
| `mDNS.additional` | mDNS additional section (Type: `pm::value::Array`) | N/A | see [DNS](parameters.md#how-to-dns-section) | 


DHCP
---------

| Parameter name          | Description             | Expected length   | Recommended formatting |
|:------------------------|:------------------------|:------------------|:-----------------------|
| `DHCP.msg_type`         | Message opcode          | 1 byte | `uint()` | 
| `DHCP.hw_type`          | Hardware type           | 1 byte | `uint()` | 
| `DHCP.hw_addr_len`      | Hardware address length | 1 byte | `uint()` | 
| `DHCP.hops`             | Hop count               | 1 byte | `uint()` | 
| `DHCP.trans_id`         | Transaction ID          | 4 byte | `uint()` | 
| `DHCP.seconds`          | Number of seconds       | 2 byte | `uint()` | 
| `DHCP.flags`            | Flags                   | 2 byte | `uint()` | 
| `DHCP.client_addr`      | Client IP address       | 4 byte | `uint()` | 
| `DHCP.your_client_addr` | Your IP address         | 4 byte | `uint()` | 
| `DHCP.next_server_addr` | Server IP address       | 4 byte | `uint()` | 
| `DHCP.relay_agent_addr` | Gateway IP address      | 4 byte | `uint()` | 
| `DHCP.client_hw_addr`   | Client hardware address | 16 byte | `uint()` | 
| `DHCP.server_host_name` | Server host name        | 1 byte | `uint()` | 
| `DHCP.boot_file_name`   | Boot filename           | 128 byte | `str()` | 
| `DHCP.magic_cookie`     | Magic Cookie            | 4 byte | `uint()` | 
| `DHCP.options`          | DHCP Options (Type: `pm::value::Array`, see below for key/value of each option) | N/A | N/A  | 

### DHCP options

| Key name | Description      | Expected length   | Recommended formatting |
|:---------|:-----------------|:------------------|:-----------------------|
| `type`   | DHCP option type. |  1 byte | `uint()` |
| `length` | DHCP option length. |  1 byte  | `uint()` |
| `data`   | DHCP option data. | depends on `length` | `hex()`, `raw()` |
