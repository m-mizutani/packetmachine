/*
 * Copyright (c) 2016 Masayoshi Mizutani <mizutani@sfc.wide.ad.jp> All
 * rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <pcap.h>

#include "./gtest/gtest.h"
#include "../src/decoder.hpp"
#include "../src/packet.hpp"
#include "../src/packetmachine/property.hpp"

#include "../src/debug.hpp"

namespace module_test {

TEST(Decoder, basic) {
  const std::string fpath = "./test/data1.pcap";
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t* pcap = ::pcap_open_offline(fpath.c_str(), errbuf);

  if (!pcap) {
    printf("errbuf = %s\n", errbuf);
  }

  ASSERT_TRUE(pcap != nullptr);
  struct pcap_pkthdr* pkthdr;
  const u_char* data;
  int rc;

  std::shared_ptr<pm::Decoder> dec(new pm::Decoder);
  pm::Packet pkt;
  pm::Payload pd;
  pm::Property prop;
  prop.set_decoder(dec);

  int count_ipv4_saddr = 0;
  int count_mac_saddr = 0;

  while (0 <= (rc = ::pcap_next_ex(pcap, &pkthdr, &data))) {
    if (0 == rc) {
      continue;
    }

    ASSERT_TRUE(pkt.store(data, pkthdr->caplen));
    pkt.set_cap_len(pkthdr->caplen);
    pkt.set_tv(pkthdr->ts);

    pd.reset(&pkt);
    prop.init(&pkt);

    dec->decode(&pd, &prop);

    const auto& eth_src = prop.value("Ethernet.src");
    EXPECT_FALSE(eth_src.is_null());
    if ("50:a7:33:0c:5e:4c" == eth_src.mac()) {
      count_mac_saddr++;
    }

    const auto& ipv4_src = prop.value("IPv4.src");
    EXPECT_FALSE(eth_src.is_null());
    if ("104.80.179.146" == ipv4_src.ip4()) {
      count_ipv4_saddr++;
    }
  }

  EXPECT_EQ(22, count_mac_saddr);
  EXPECT_EQ(270, count_ipv4_saddr);
  pcap_close(pcap);
}

TEST(Decoder, lookup_param) {
  pm::Decoder dec;
  pm::param_id pid = dec.lookup_param_id("Ethernet.src");
  EXPECT_NE(pm::Param::NONE, pid);
  EXPECT_NE(dec.lookup_param_id("Ethernet.dst"), pid);
  EXPECT_EQ("Ethernet.src", dec.lookup_param_name(pid));

  EXPECT_EQ(pm::Param::NONE, dec.lookup_param_id("Invalid_Param"));
}

TEST(Decoder, lookup_param_key) {
  pm::Decoder dec;
  const pm::ParamKey& key = dec.lookup_param_key("Ethernet.src");
  EXPECT_NE(pm::Property::NULL_KEY, key);
  EXPECT_NE(dec.lookup_param_key("Ethernet.dst"), key);
  EXPECT_EQ("Ethernet.src", dec.lookup_param_name(key));

  EXPECT_EQ(pm::Property::NULL_KEY, dec.lookup_param_key("Invalid_Param"));
}

TEST(Decoder, lookup_event) {
  pm::Decoder dec;
  pm::event_id eid = dec.lookup_event_id("Ethernet");
  EXPECT_NE(pm::Event::NONE, eid);
  EXPECT_NE(dec.lookup_event_id("IPv4"), eid);
  EXPECT_EQ("Ethernet", dec.lookup_event_name(eid));

  EXPECT_EQ(pm::Param::NONE, dec.lookup_param_id("Invalid_Param"));
}


TEST(Decoder, custom_module) {
  class DummyMod : public pm::Module {
   public:
    int call_count_;
    DummyMod () : call_count_(0) {
    }
    void setup(const pm::Config& config) {
    }
    pm::mod_id decode(pm::Payload* pd, pm::Property* prop) {
      this->call_count_ += 1;
      return pm::Module::NONE;
    };
  };
  
  pm::ModMap mod_map;
  pm::Packet pkt;
  pm::Payload pd;
  pm::Property prop;
  auto mod = new DummyMod();
  mod_map.insert(std::make_pair("Ethernet", mod));

  std::shared_ptr<pm::Decoder> dec(new pm::Decoder(&mod_map));
  prop.set_decoder(dec);
  std::string data("abcdefg");
  pkt.store(reinterpret_cast<const pm::byte_t*>(data.data()), 4);
  dec->decode(&pd, &prop);

  EXPECT_EQ(1, mod->call_count_);
}

TEST(Decoder, custom_module_defs) {
  class DummyMod : public pm::Module {
   public:
    const pm::ParamDef* p1_;
    const pm::ParamDef* p2_;

    DummyMod () {
      this->p1_ = this->define_param("p1");
      this->p2_ = this->define_param("p2");
      
    }
    void setup(const pm::Config& config) {}
    pm::mod_id decode(pm::Payload* pd, pm::Property* prop) {
      const pm::byte_t* p = pd->retain(4);
      EXPECT_EQ('a', p[0]);
      prop->retain_value(this->p1_)->set(&p[0], 2);
      prop->retain_value(this->p2_)->set(&p[2], 2);
      
      return pm::Module::NONE;
    };
  };
  
  pm::ModMap mod_map;
  pm::Packet pkt;
  pm::Payload pd;
  pm::Property prop;
  auto mod = new DummyMod();
  mod_map.insert(std::make_pair("Ethernet", mod));

  std::shared_ptr<pm::Decoder> dec(new pm::Decoder(&mod_map));
  prop.set_decoder(dec);
  std::string data("abcdefg");
  
  pkt.store(reinterpret_cast<const pm::byte_t*>(data.data()), 4);
  pd.reset(&pkt);
  prop.init(&pkt);
  
  dec->decode(&pd, &prop);

  EXPECT_EQ("ab", prop.value("Ethernet.p1").repr());
  EXPECT_EQ("cd", prop.value("Ethernet.p2").repr());
}


TEST(Decoder, batch_param) {
  class DummyMod : public pm::Module {
   public:
    pm::MajorParamDef* p_;

    DummyMod () {
      this->p_ = this->define_major_param("p");
      this->p_->define_minor("1", [](pm::Value* v, const pm::byte_t* ptr) {
          v->set(&ptr[0], 2);
        });
      this->p_->define_minor("2", [](pm::Value* v, const pm::byte_t* ptr) {
          v->set(&ptr[2], 2);
        });
    }
    void setup(const pm::Config& config) {}
    pm::mod_id decode(pm::Payload* pd, pm::Property* prop) {
      const pm::byte_t* p = pd->retain(4);
      prop->retain_value(this->p_)->set(p, 4);
      return pm::Module::NONE;
    };
  };
  
  pm::ModMap mod_map;
  pm::Packet pkt;
  pm::Payload pd;
  pm::Property prop;
  auto mod = new DummyMod();
  mod_map.insert(std::make_pair("Ethernet", mod));

  std::shared_ptr<pm::Decoder> dec(new pm::Decoder(&mod_map));
  prop.set_decoder(dec);

  // TODO: need to identify keys that have same parent
  // const pm::ParamKey& pid1 = dec->lookup_param_key("Ethernet.p.1");
  // const pm::ParamKey& pid2 = dec->lookup_param_key("Ethernet.p.2");
  // EXPECT_NE(pid1, pid2);
  
  std::string data("abcdefg");  
  pkt.store(reinterpret_cast<const pm::byte_t*>(data.data()), 4);
  pd.reset(&pkt);
  prop.init(&pkt);
  
  dec->decode(&pd, &prop);

  EXPECT_EQ("ab", prop.value("Ethernet.p.1").repr());
  EXPECT_EQ("cd", prop.value("Ethernet.p.2").repr());
  EXPECT_EQ("ab", prop.value("Ethernet.p.1").repr());
  EXPECT_EQ("cd", prop.value("Ethernet.p.2").repr());
}

TEST(Decoder, config_test) {
  class DummyMod1 : public pm::Module {
   public:
    DummyMod1 () {
      this->define_config("t1");
    }
    void setup(const pm::Config& config) {
      EXPECT_EQ(true, config.has("t1"));
      EXPECT_EQ(false, config.has("t2"));
      EXPECT_EQ("v", config.get("t1").as_str());
    }
    pm::mod_id decode(pm::Payload* pd, pm::Property* prop) {
      return pm::Module::NONE;
    };
  };

  class DummyMod2 : public pm::Module {
   public:
    DummyMod2 () {
      this->define_config("t2");
    }
    void setup(const pm::Config& config) {
      EXPECT_EQ(false, config.has("t1"));
      EXPECT_EQ(true, config.has("t2"));
      EXPECT_EQ("x", config.get("t2").as_str());
    }
    pm::mod_id decode(pm::Payload* pd, pm::Property* prop) {
      return pm::Module::NONE;
    };
  };

  // test normal set
  EXPECT_NO_THROW({
      pm::ModMap mod_map;
      mod_map.insert(std::make_pair("Ethernet", new DummyMod1));
      mod_map.insert(std::make_pair("Blue", new DummyMod2));
      pm::Config config;
      config.set("Ethernet.t1", "v");
      config.set("Blue.t2", "x");
      pm::Decoder dec(config, &mod_map);
    });

  // Test invalid config key
  EXPECT_THROW({
      pm::ModMap mod_map;
      mod_map.insert(std::make_pair("Ethernet", new DummyMod1));
      mod_map.insert(std::make_pair("Blue", new DummyMod2));
      pm::Config config;
      config.set("Ethernet.t2", "v");
      pm::Decoder dec(config, &mod_map);
    }, pm::Exception::ConfigError);

  // Test type error
  EXPECT_THROW({
      pm::ModMap mod_map;
      mod_map.insert(std::make_pair("Ethernet", new DummyMod1));
      mod_map.insert(std::make_pair("Blue", new DummyMod2));
      pm::Config config;
      config.set("Blue.t2", 1);
      pm::Decoder dec(config, &mod_map);
    }, pm::Exception::TypeError);
  
}

}   // namespace module_test

