#include "daas/daas_interfaces.hpp"
#include "os/kernel.hpp"
#include "os/modules/toastmessages.hpp"

void daas_node_event::atsSyncCompleted(din_t din) {
    system->addNode(din);
}

void daas_node_event::nodeConnectedToNetwork(din_t sid, din_t din) {
    system->daasNetworkConnected = true; 
    ToastManager::getInstance()->show("Node connected to a network", TOAST_INFO, 1000);
}

void daas_node_event::dinAccepted(din_t din) {
    system->addNode(din);
}

void daas_node_event::ddoReceived(int payload_size, typeset_t typeset, din_t din) {
}