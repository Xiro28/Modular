#pragma once
#include <vector>
#include "daas/daas.hpp"

class Kernel; // Forward declaration

class daas_node_event : public IDaasApiEvent {
	void dinAccepted(din_t din) override;
	void ddoReceived(int payload_size, typeset_t, din_t) override;
	void frisbeeReceived(din_t) override {}
	void nodeStateReceived(din_t) override {}
	void atsSyncCompleted(din_t din) override;
	void frisbeeDperfCompleted(din_t, uint32_t packets_sent, uint32_t block_size) override {}
	void nodeDiscovered(din_t din, link_t link) override {}
	void nodeConnectedToNetwork(din_t sid, din_t din) override;
    Kernel * system;

    public:
    
    daas_node_event(Kernel * _system){ system = _system; }

};