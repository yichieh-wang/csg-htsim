// -*- c-basic-offset: 4; indent-tabs-mode: nil -*-        
#ifndef _LOSSLESS_INPUT_QUEUE_H
#define _LOSSLESS_INPUT_QUEUE_H
#include "queue.h"
/*
 * A FIFO queue that supports PAUSE frames and lossless operation
 */

#include <list>
#include "config.h"
#include "eventlist.h"
#include "network.h"
#include "loggertypes.h"
#include "eth_pause_packet.h"
#include "switch.h"
#include "callback_pipe.h"

class Switch;
class PfcAccount;

class LosslessInputQueue : public Queue, public VirtualQueue {
public:
    LosslessInputQueue(EventList &eventlist);
    LosslessInputQueue(EventList &eventlist,BaseQueue* peer, Switch* sw, simtime_picosec wire_latency);
    LosslessInputQueue(EventList &eventlist,BaseQueue* peer);

    virtual void receivePacket(Packet& pkt);

    void sendPause(unsigned int wait);
    virtual void completedService(Packet& pkt);

    virtual void setName(const string& name) {
        Logged::setName(name); 
        _nodename += name;
    }
    virtual string& nodename() { return _nodename; }

    enum {PAUSED,READY,PAUSE_RECEIVED};

    static uint64_t _low_threshold;
    static uint64_t _high_threshold;
    // With an account the switch decides pause and resume dynamically (pfc_account.h);
    // without one the static thresholds above rule, as they always did.
    void setAccount(PfcAccount* account, uint32_t port) { _account = account; _account_port = port; }

private:
    int _state_recv;
    CallbackPipe* _wire;
    PfcAccount* _account = NULL;
    uint32_t _account_port = 0;
};

#endif
