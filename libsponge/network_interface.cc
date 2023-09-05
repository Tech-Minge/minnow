#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

EthernetFrame NetworkInterface::build_ethernet_frame(EthernetAddress src, EthernetAddress dst, uint16_t type, const BufferList& payload) {
    EthernetFrame frame;
    // fill head
    frame.header().src = src;
    frame.header().dst = dst;
    frame.header().type = type;
    // serialize
    frame.payload() = payload;
    return frame;
}

ARPMessage NetworkInterface::build_arp_message(EthernetAddress sender_eth, uint32_t sender_ip, uint32_t target_ip, uint16_t opcode) {
    ARPMessage arp;
    arp.sender_ethernet_address = sender_eth;
    arp.sender_ip_address = sender_ip;
    arp.target_ip_address = target_ip;
    arp.opcode = opcode;
    return arp;
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    // read cache
    if (ip_to_mac_.count(next_hop_ip)) {
        auto frame = build_ethernet_frame(_ethernet_address, ip_to_mac_[next_hop_ip].first, EthernetHeader::TYPE_IPv4, dgram.serialize());
        _frames_out.push(frame);
    } else {
        // send arp if no such ip query
        if (!queueing_datagram_.count(next_hop_ip)) {
            // as for arp, no need to set target's mac due to unknown
            auto arp_message = build_arp_message(_ethernet_address, _ip_address.ipv4_numeric(),
                                                 next_hop_ip, ARPMessage::OPCODE_REQUEST);
            auto arp_frame = build_ethernet_frame(_ethernet_address, ETHERNET_BROADCAST,
                                                  EthernetHeader::TYPE_ARP, arp_message.serialize());
            // record 5s
            outgoing_arp_[next_hop_ip] = make_pair(arp_frame, 5000);
            _frames_out.push(arp_frame);
        }
        queueing_datagram_[next_hop_ip].emplace_back(dgram);
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    auto dst_ether = frame.header().dst;
    if (dst_ether != _ethernet_address && dst_ether != ETHERNET_BROADCAST) {
        return {};
    }
    if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        InternetDatagram dgram;
        if (dgram.parse(frame.payload()) != ParseResult::NoError) {
            return {};
        }
        return dgram;
    } else {
        // deal with arp
        ARPMessage arp;
        if (arp.parse(frame.payload()) != ParseResult::NoError) {
            return {};
        }
        auto sender_ether = arp.sender_ethernet_address;
        auto sender_ip = arp.sender_ip_address;
        // learn map
        ip_to_mac_[sender_ip] = make_pair(sender_ether, 30000);
        if (arp.opcode == ARPMessage::OPCODE_REQUEST) {
            if (arp.target_ip_address == _ip_address.ipv4_numeric()) {
                // send reply
                auto arp_message = build_arp_message(_ethernet_address, _ip_address.ipv4_numeric(),
                                                     sender_ip, ARPMessage::OPCODE_REPLY);
                // set arp's target mac
                arp_message.target_ethernet_address = sender_ether;
                auto arp_frame = build_ethernet_frame(_ethernet_address, sender_ether,
                                                      EthernetHeader::TYPE_ARP, arp_message.serialize());
                _frames_out.push(arp_frame);
            }
        } else {
            // must be reply to out request
            if (queueing_datagram_.count(sender_ip)) {
                for (auto& elem : queueing_datagram_[sender_ip]) {
                    auto send_frame = build_ethernet_frame(_ethernet_address, sender_ether,
                                                           EthernetHeader::TYPE_IPv4, elem.serialize());
                    _frames_out.push(send_frame);
                }
                // remove
                queueing_datagram_.erase(sender_ip);
                outgoing_arp_.erase(sender_ip);
            }
        }
        
    }
    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    // retx arp frame
    for (auto it = outgoing_arp_.begin(); it != outgoing_arp_.end(); ++it) {
        auto& ttl = (*it).second.second;
        ttl -= ms_since_last_tick;
        if (ttl <= 0) {
            _frames_out.push((*it).second.first);
            ttl = 5000;
        }
    }
    // cache expire
    for (auto it = ip_to_mac_.begin(); it != ip_to_mac_.end();) {
        auto& ttl = (*it).second.second;
        ttl -= ms_since_last_tick;
        if (ttl <= 0) {
            // expire cache
            it = ip_to_mac_.erase(it);
        } else {
            ++it;
        }
    }
}
