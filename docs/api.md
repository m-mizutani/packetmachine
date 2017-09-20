API guide
================

[`Property`](#property)
---------------

An instance of `Property` class is given at callback of network events. The instance has a result of traffic analysis including protocol decoding.

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
bool has_value(param_id pid) const;
bool has_value(const std::string& name) const;
const Value& value(param_id pid) const;
const Value& value(const std::string& name) const;
const Value& operator[](param_id pid) const;
const Value& operator[](const std::string& name) const;
```

`Value` is an attribute value of packet decoding. A `Property` instance has dictionary type data structure for `Value`s. Each protocol decoding module set an attribute name (e.g. `IPv4.src`) and a value (e.g. `10.0.0.1`).

`has_value()` returns true if the insntace has attribute value, and `value()` and `operator[]` returns actual `Value` instance as `const` reference. Both of `value()` and `operator[]` return same result by same argument. (e.g. `property.value("IPv4.src)` and `property["IPv4.src"]`).



If an attribute value that is specified as argument does not exist, the method returns `static Value Property::null_`. `is_null()` method of the instance returns `true`.

```
const byte_t* src_addr(size_t* len) const;
const byte_t* dst_addr(size_t* len) const;
uint16_t src_port() const;
uint16_t dst_port() const;
```

`src_addr()` and `dst_addr()` return source IP address and destination IP address of IPv4 or IPv6. Also `src_port()` and `dst_port()` return source and destination port number of TCP or UDP.
