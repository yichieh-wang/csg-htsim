// -*- c-basic-offset: 4; indent-tabs-mode: nil -*-
#ifndef _PFC_ACCOUNT_H
#define _PFC_ACCOUNT_H
#include <map>
#include <stdint.h>
#include "config.h"

// A switch's shared-buffer bookkeeping for PFC with a DYNAMIC threshold, the kind a
// Broadcom-style MMU keeps (after the ns-3 HPCC model's SwitchMmu): every ingress port owns a
// reserve, draws on the shared pool up to a threshold that shrinks as the pool fills, then on
// its own headroom. It is paused once it touches headroom or its shared share reaches the
// threshold, and resumed once its headroom is empty and its share is back under the threshold
// by resume_offset. One class per port. A packet the headroom cannot take is a lossless
// failure: the queue reports it, the caller decides what it means.
class PfcAccount {
public:
    PfcAccount(mem_b buffer, unsigned shift, mem_b reserve, mem_b resume_offset);
    void add_port(uint32_t port, mem_b headroom);
    bool admissible(uint32_t port, mem_b bytes) const;
    bool arrive(uint32_t port, mem_b bytes); // account the packet; true when the port must PAUSE now
    bool depart(uint32_t port, mem_b bytes); // release the packet; true when the port may RESUME now
    mem_b threshold() const;

private:
    struct Port {
        mem_b ingress;  // reserve, then the shared pool
        mem_b hdrm;     // headroom in use
        mem_b headroom; // headroom owned
        bool paused;
    };
    Port& port(uint32_t id);
    const Port& port(uint32_t id) const;
    mem_b shared_used(const Port& p) const { return p.ingress > _reserve ? p.ingress - _reserve : 0; }

    std::map<uint32_t, Port> _ports;
    mem_b _buffer;
    mem_b _reserve;
    mem_b _resume_offset;
    mem_b _total_hdrm;
    mem_b _total_rsrv;
    mem_b _shared_used;
    unsigned _shift;
};
#endif
