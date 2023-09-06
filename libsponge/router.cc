#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    routing_table_.emplace_back(RoutingElement{route_prefix, prefix_length, next_hop, interface_num});
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    uint32_t trg = UINT32_MAX;
    for (uint32_t i = 0; i < routing_table_.size(); ++i) {
        const RoutingElement& elem = routing_table_[i];
        uint32_t shift = 32 - elem.prefix_length;
        if (shift == 32 && trg == UINT32_MAX) {
            trg = i;
            continue;
        }
        uint32_t res = (elem.route_prefix ^ dgram.header().dst) >> shift;
        if (!res) {
            if (trg == UINT32_MAX || elem.prefix_length > routing_table_[trg].prefix_length) {
                trg = i;
            }
        }
    }
    if (trg != UINT32_MAX) {
        if (static_cast<int8_t>(--dgram.header().ttl) <= 0) {
            return;
        }
        const RoutingElement& elem = routing_table_[trg];
        auto& net_interface = interface(elem.interface_num);
        if (elem.next_hop.has_value()) {
            net_interface.send_datagram(dgram, elem.next_hop.value());
        } else {
            net_interface.send_datagram(dgram, Address::from_ipv4_numeric(dgram.header().dst));
        }
        
    }
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
