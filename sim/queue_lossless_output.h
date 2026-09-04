// -*- c-basic-offset: 4; indent-tabs-mode: nil -*-        
#include <random>
#ifndef _LOSSLESS_OUTPUT_QUEUE_H
#define _LOSSLESS_OUTPUT_QUEUE_H
/*
 * A FIFO queue that supports PAUSE frames and lossless operation
 */

#include <list>
#include "queue.h"
#include "config.h"
#include "eventlist.h"
#include "network.h"
#include "loggertypes.h"
#include "eth_pause_packet.h"
#include "ecn.h"

class LosslessOutputQueue : public Queue {
public:
    LosslessOutputQueue(linkspeed_bps bitrate, mem_b maxsize, EventList &eventlist, QueueLogger* logger, int ECN=0, int K=0);

    void receivePacket(Packet& pkt);
    void receivePacket(Packet& pkt,VirtualQueue* q);
    // ECN marking the way the ns-3 HPCC model's switch does it: at dequeue, on the bytes left in the
    // queue, always above kmax, with probability pmax * (q - kmin) / (kmax - kmin) between; data only.
    void setEcnMarking(mem_b kmin, mem_b kmax, double pmax, uint32_t seed);

    void beginService();
    void completeService();

    bool is_paused() { return _state_send == PAUSED || _state_send == PAUSE_RECEIVED;}

    enum queue_state {PAUSED,READY,PAUSE_RECEIVED};

private:
    list<VirtualQueue*> _vq;

    int _state_send;
    int _sending;
    bool _marking = false;
    mem_b _kmin = 0;
    mem_b _kmax = 0;
    double _pmax = 0;
    std::mt19937 _marking_rng;
    bool should_mark();
    uint64_t _txbytes;

    int _ecn_enabled;
    int _K;
};

#endif
