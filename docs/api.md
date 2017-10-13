API guide
================

This document explains a basic usage of PacketMachine library. Please see source code if you want to know more details.

[`pm::Property`](#property)
---------------

An instance of `pm::Property` class is given at callback of network events. The instance has a result of traffic analysis including protocol decoding.

### Methods

```cpp
size_t pkt_size() const
```

Returns size of a packet that is a trigger of the event.

```cpp
time_t ts() const;
double ts_d() const;
const struct timeval& tv() const;
```

Returns arrival time of a packet that is a trigger of the event.

```cpp
bool has_value(const pm::ParamKey& key) const;
bool has_value(const std::string& name) const;
const pm::Value& value(const pm::ParamKey& key) const;
const pm::Value& value(const std::string& name) const;
const pm::Value& operator[](const pm::ParamKey& key) const;
const pm::Value& operator[](const std::string& name) const;
```

`pm::Value` is an attribute value of packet decoding. A `pm::Property` instance has dictionary type data structure for `Value`s. Each protocol decoding module sets an attribute name (e.g. `IPv4.src`) and a value (e.g. `10.0.0.1`).

`has_value()` returns true if the insntace has attribute value, and `value()` and `operator[]` returns actual `Value` instance as `const` reference. Both of `value()` and `operator[]` return same result by same argument. (e.g. `property.value("IPv4.src)` and `property["IPv4.src"]`).

Value(s) can be accessed by not only parameter name `std::string` but also parameter ID `const pm::ParamKey&`. `const pm::ParamKey&` can be looked up by `pm::Machine::lookup_param_key()` beforehand. Using `const pm::ParamKey&` is faster than `std::string` because access methods for `pm::Value` with `std::string` look up `const pm::ParamKey&` by the argument every time.

If an attribute value that is specified as argument does not exist, the method returns `static Value Property::null_`. `is_null()` method of the instance returns `true`.

```cpp
const byte_t* src_addr(size_t* len) const;
const byte_t* dst_addr(size_t* len) const;
uint16_t src_port() const;
uint16_t dst_port() const;
```

`src_addr()` and `dst_addr()` return source IP address and destination IP address of IPv4 or IPv6. Also `src_port()` and `dst_port()` return source and destination port number of TCP or UDP.


[`pm::Value`](#value)
---------------

`pm::Value` contains an attribute of a protocol decoding result.

### Methods

```cpp
void repr(std::ostream &os) const;
std::string repr() const;
```

`repr()` is an easy way to show a packet analyzing result. The 1st method outputs string stream to `std::ostream &os` and the 2nd method returns `std::string` as a human readable format.

```cpp
bool hex(std::ostream &os) const;
bool ip4(std::ostream &os) const;
bool ip6(std::ostream &os) const;
bool mac(std::ostream &os) const;
bool uint64(uint64_t* d) const;
bool uint(unsigned int* d) const;
bool uint(unsigned long* d) const;
```

These methods convert a result of protocol decoding to each format. 

- `hex()` converts from `[0x12, 0x34, 0xab, 0xc0]` to `1234ABC0`
- `ip4()` converts from `[0x12, 0x34, 0xab, 0xc0]` to `192.171.52.18` (big endian to little endian)
- `ip6()` converts from `[0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xba, 0xf6, 0xb1, 0xff, 0xfe, 0x19, 0xb0, 0xb5]` to `fe80::baf6:b1ff:fe19:b0b5`
- `mac()` converts from `[0xb8, 0xf6, 0xb1, 0x19, 0xb0, 0xb5]` to `b8:f6:b1:19:b0:b5`
- `uint64()` and `uint()` convert from `[0x01, 0x02, 0x00, 0x00]` to `513` as `uint64_t`, `unsigned int` or `unsigned long`.

These methods have limitation about the conversions.

- `ip4()` can convert from only 4 bytes data.
- `ip6()` can convert from only 16 bytes data.
- `mac()` can convert from only 6 bytes data.
- `uint64()` and `uint()` can convert from only 1, 2, 4 and 8 bytes data. A converted number will be `static_cast` to `uint16_t`, `unsigned int` or `unsigned long`.

`pm::Value` has `endian_` member variable and automatically converts from big endian to little endian if necessary.

```cpp
std::string hex() const;
std::string ip4() const;
std::string ip6() const;
std::string mac() const;
uint64_t uint64() const;
unsigned int uint() const;
```

If you want to get a value as `std::string`, `uint64_t` or `unsigned int`, you can use above methods. But please note the methods require additional memory copy cost than methods require `std::ostream` or a pointer.

```cpp
const byte_t* raw(size_t* len = nullptr) const;
```

If you want to use more primitive data, you can get data pointer as `byte_t*` by `raw()` method. If you give the method `size_t` pointer, the method set length of data to the pointer.

```cpp
bool is_array() const;
const Value& get(size_t idx) const;
```

`pm::Value` supports an array structure also. If the instance has array structure, `is_array()` returns `true` and each element of array can be accessed by `get(size_t idx)`. `idx` means index of array. If it's not array structure, `get(size_t idx)` throws `pm::Exception::TypeError`.

```cpp
bool is_map() const;
const Value& find(const std::string& key);
```

Additionally `pm::Value` supports an dictionary (map) structure. If the instance has dictionary type, `is_map()` returns `true` and a key of map is `std::string`. Available keys of each value are described in [protocol decode module documents](protocol/).

each element of map structure can be accessed by `find()` method. If it's not map structure, `find()` throws `pm::Exception::TypeError`.
