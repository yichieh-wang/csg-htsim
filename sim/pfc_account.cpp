// -*- c-basic-offset: 4; indent-tabs-mode: nil -*-
#include "pfc_account.h"
#include <algorithm>
#include <cassert>

PfcAccount::PfcAccount(mem_b buffer, unsigned shift, mem_b reserve, mem_b resume_offset)
    : _buffer(buffer), _reserve(reserve), _resume_offset(resume_offset), _total_hdrm(0), _total_rsrv(0), _shared_used(0), _shift(shift) {}

void PfcAccount::add_port(uint32_t id, mem_b headroom) {
    assert(_ports.find(id) == _ports.end());
    Port p;
    p.ingress = 0;
    p.hdrm = 0;
    p.headroom = headroom;
    p.paused = false;
    _ports[id] = p;
    _total_hdrm += headroom;
    _total_rsrv += _reserve;
}

PfcAccount::Port& PfcAccount::port(uint32_t id) {
    std::map<uint32_t, Port>::iterator i = _ports.find(id);
    assert(i != _ports.end());
    return i->second;
}

const PfcAccount::Port& PfcAccount::port(uint32_t id) const {
    std::map<uint32_t, Port>::const_iterator i = _ports.find(id);
    assert(i != _ports.end());
    return i->second;
}

// The model computes this unsigned, where an overdrawn pool would wrap; here it is empty.
mem_b PfcAccount::threshold() const {
    mem_b free = _buffer - _total_hdrm - _total_rsrv - _shared_used;
    return free > 0 ? free >> _shift : 0;
}

bool PfcAccount::admissible(uint32_t id, mem_b bytes) const {
    const Port& p = port(id);
    return !(bytes + p.hdrm > p.headroom && bytes + shared_used(p) > threshold());
}

bool PfcAccount::arrive(uint32_t id, mem_b bytes) {
    Port& p = port(id);
    mem_b now = p.ingress + bytes;
    if (now <= _reserve) {
        p.ingress += bytes;
    } else if (now - _reserve > threshold()) {
        p.hdrm += bytes;
    } else {
        p.ingress += bytes;
        _shared_used += std::min(bytes, now - _reserve);
    }
    if (!p.paused && (p.hdrm > 0 || shared_used(p) >= threshold())) {
        p.paused = true;
        return true;
    }
    return false;
}

bool PfcAccount::depart(uint32_t id, mem_b bytes) {
    Port& p = port(id);
    mem_b from_hdrm = std::min(p.hdrm, bytes);
    mem_b over = p.ingress > _reserve ? p.ingress - _reserve : 0;
    mem_b from_shared = std::min(bytes - from_hdrm, over);
    p.hdrm -= from_hdrm;
    p.ingress -= bytes - from_hdrm;
    _shared_used -= from_shared;
    if (!p.paused) return false;
    mem_b used = shared_used(p);
    if (p.hdrm == 0 && (used == 0 || used + _resume_offset <= threshold())) {
        p.paused = false;
        return true;
    }
    return false;
}
