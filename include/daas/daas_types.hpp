/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: daas-types.h
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Disclaimer of Warrant
 * Covered Software is provided under this License on an "as is" basis, without warranty of any kind, either
 * expressed, implied, or statutory, including, without limitation, warranties that the Covered  Software is
 * free of defects, merchantable, fit for a particular purpose or non-infringing.
 * The entire risk as to the quality and performance of the Covered Software is with You.  Should any Covered
 * Software prove defective in any respect, You (not any Contributor) assume the cost of any necessary
 * servicing, repair, or correction.
 * This disclaimer of warranty constitutes an essential part of this License.  No use of any Covered Software
 * is authorized under this License except under this disclaimer.
 *
 * Limitation of Liability
 * Under no circumstances and under no legal theory, whether tort (including negligence), contract, or otherwise,
 * shall any Contributor, or anyone who distributes Covered Software as permitted above, be liable to You for
 * any direct, indirect, special, incidental, or consequential damages of any character including, without
 * limitation, damages for lost profits, loss of goodwill, work stoppage, computer failure or malfunction,
 * or any and all other commercial damages or losses, even if such party shall have been informed of the
 * possibility of such damages.  This limitation of liability shall not apply to liability for death or personal
 * injury resulting from such party's negligence to the extent applicable law prohibits such limitation.
 * Some jurisdictions do not allow the exclusion or limitation of incidental or consequential damages, so this
 * exclusion and limitation may not apply to You.
 *
 * Contributors:
 * plogiacco@smartlab.it - initial design, implementation and documentation
 * sebastiano.meduri@gmail.com  - initial design, implementation and documentation
 * l.grillo@sebyone.it  - implementation and documentation
 *
 */

#ifndef _DAAS_TYPES_H__
#define _DAAS_TYPES_H__

#include <stdint.h>

typedef uint64_t stime_t;     // Time-stamp absolute date (64bit) !!!!!!!!!!!!!!! SYSTEM RESOURCE
typedef uint64_t din_t;       // DIN (64bit, 0 NULL,  )
typedef uint16_t typeset_t;   // Typeset (16bit)

enum syscode_t
{
    ___undefined = 0,

    _cor_dme_sended,
    _cor_dme_received,
    _cor_dme_routed,

    _cor_rx_buffer,
    _cor_tx_buffer,

    // ATS
    _ats_delta_avg,
    _ats_sync_counter,
    _ats_msg_decoded,
    _ats_msg_encoded,

};

/* DRIVER TYPES */
typedef enum : unsigned // Supported communications technologies
{
    _LINK_NONE = 0, // ND
    _LINK_DAAS,     // DaaS routing & loopback
    _LINK_INET4,    // Inet/IP
    _LINK_BT, // Bluetooth
    _LINK_MQTT5,    // MQTT
    _LINK_UART,     // Serial line
    _LINK_RAW
} link_t;

typedef enum
{
    PERFORM_CORE_THREAD = 0,
    PERFORM_CORE_NO_THREAD
} performs_mode_t;

//TODO OPCODE:
//opcode bitmask es opcode = 00000011 -> uint8 code = OPCODE_ACK_REQUEST | OPCODE_RESYNC_REQUEST
//Prima di spedire un pacchetto, ci salviamo il checksum del pacchetto intero (per ora header)
//insieme al pacchetto. Tramite dme_ack chiediamo conferma della corretta ricezione e leggiamo il
//checksum nuovo
//Se checksum nostro == checksum ricevuto -> OK
//Altrimenti rispediamo 

//Meglio Ack e Nack? 
typedef enum : uint16_t
{
    OPCODE_NULL = 0x00,

    OPCODE_ACK_REQUEST = 1 << 0,
    OPCODE_ACK_RESPONSE = 1 << 1,
    OPCODE_RESYNC_REQUEST = 1 << 2,
    OPCODE_PAYLOAD_COMPRESSED = 1 << 3,
} opcode_t;

typedef enum : uint8_t
{
    discovery_off = 0,
    discovery_sender_only,
    discovery_receiver_only,
    discovery_full

} discovery_state_t;

typedef enum : uint8_t
{
    ddo_policy_skip_on_failure = 0,

    // retry immediately (always)
    ddo_policy_retry_on_failure,
    //ddo_policy_retry_with_alternate_route_on_failure,

    // retry with exponential wait time
    ddo_policy_exponential_backoff_retry_on_failure,
    //ddo_policy_exponential_backoff_retry_with_alternate_route_on_failure

} ddo_policy_t;

using typeset_fun = void(*)(din_t);

typedef enum
{
    ERROR_NONE = 0,
    ERROR_CORE_ALREADY_INITIALIZED, //Modified from ERROR_ALREADY_INITIALIZED to avoid windows.h conflicts
    ERROR_CORE_STOPPED,
    ERROR_CANNOT_INITIALIZE,
    ERROR_CANNOT_CREATE_NODE,
    ERROR_DIN_ALREADY_EXIST,
    ERROR_CANNOT_MAP_NODE,
    ERROR_INVALID_USER_TYPESET,
    ERROR_SEND_DDO,
    ERROR_NO_DDO_PRESENT,
    ERROR_DIN_UNKNOWN,
    ERROR_CHANNEL_FAILURE,
    ERROR_ATS_NOT_SYNCED,
    ERROR_DISCOVERY_DISABLED,
    ERROR_POLICY_STRICT_ENABLED,
    // core
    ERROR_INVALID_DME,
    ERROR_THREADS_ALREADY_STARTED,
    ERROR_NOT_IMPLEMENTED,
    ERROR_UNKNOWN

} daas_error_t;

class DDO
{
public:
    DDO();
    DDO(typeset_t typeset_);
    DDO(const DDO &ddo_);
    ~DDO();

    DDO *getDDO() { return this; }

    void clearPayload();
    void setTypeset(typeset_t);
    
    din_t getOrigin();
    stime_t getTimestamp();
    typeset_t getTypeset();
    
    uint32_t setPayload(const void *data_, uint32_t size_);
    uint32_t getPayloadSize(void) { return _size; }
    uint32_t getPayloadAsBinary(uint8_t *pbuffer_, unsigned offset_, uint32_t maxSize_);
    uint32_t allocatePayload(uint32_t size_);
    uint32_t appendPayloadData(const void *data_, uint32_t size_);
    uint8_t *getPayloadCurrentPositionPointer();
    
    inline uint8_t *getPayloadPtr() { return _payload; }

private:
    typeset_t _typeset = 0; // Typeset (16bit)
    stime_t _timestamp = 0; // Time-stamp absolute system's date and time (64bit)
    uint32_t _size = 0;
    uint32_t _data_offset = 0;
    uint8_t *_payload = nullptr;
    din_t _origin = 0;      // DIN origin (32bit)

    void setOrigin(din_t);
    void setTimestamp(stime_t tstamp);

    friend class DME_USR;
};

template <typename T>
class Vector
{
private:
    T*            m_data;      
    unsigned int  m_size;      
    unsigned int  m_capacity;

public:
    explicit Vector(uint32_t initialCapacity);
    Vector();
    ~Vector();

    uint32_t size() const;
    uint32_t capacity() const;

    bool pop_back();
    bool empty() const;
    bool push_back(const T& value);
    bool reserve(uint32_t newCapacity);

    T& at(uint32_t idx);
    T& operator[](uint32_t index);
    const T& operator[](uint32_t index) const;

    void clear();    
    void full_clear();    
};

class IDaasApiEvent
{
public:
    virtual ~IDaasApiEvent() = default;
    virtual void dinAccepted(din_t) = 0;
    virtual void ddoReceived(int payload_size, typeset_t, din_t) = 0;
    virtual void frisbeeReceived(din_t) = 0;
    virtual void nodeStateReceived(din_t) = 0;
    virtual void atsSyncCompleted(din_t) = 0;
    virtual void frisbeeDperfCompleted(din_t, uint32_t packets_sent, uint32_t block_size)= 0;
    virtual void nodeDiscovered(din_t din, link_t link) = 0;
    virtual void nodeConnectedToNetwork(din_t sid, din_t din) = 0;
};

class DaaSEvent
{
public:
    virtual ~DaaSEvent() = default;
    virtual int daasEvent(int, int, int, int) = 0;
};

// Interface used to implement device specific storage handler to backup/restore 
// the library configuration to a non-volatile memory.
class IDepot
{
public:
    virtual bool open(bool read_mode) = 0;                                                                            // Open the partition handler
    virtual bool close() = 0;                                                                           // Close the partition handler
    virtual bool clearSpace() = 0;                                                                      // Deletes all the record saved
    virtual bool getSpaceInfo() = 0;                                                                    // Get how much space is left on the device
    virtual bool trash(unsigned oKey) = 0;                                                              // Trash a specific entry
    virtual unsigned save(unsigned oKey, unsigned char *pdata, unsigned dsize) = 0;                     // Save map to buffer (BINARY/XML/JSON)
    virtual unsigned load(unsigned oKey, unsigned char *pdata, unsigned npartsize) = 0;                 // Load map data from buffer (BINARY/XML/JSON)
};


struct dperf_info_result {
    uint64_t sender_first_timestamp;
    uint64_t local_end_timestamp;
    uint64_t remote_first_timestamp;
    uint64_t remote_last_timestamp;
    uint64_t remote_pkt_counter;
    uint64_t remote_data_counter;
};

typedef struct
{
    stime_t lasttime;       // time reference
    uint32_t hwver;         // platform (hard-coded)
    uint32_t linked;        // channels counter (0=not linked)
    uint32_t lock;          // required security policy
    uint8_t sklen;    // security phrase lenght
    uint8_t skey[14]; // security phrase (UTF-8)
    uint32_t form;          // data formatting model
    uint32_t codec;         // data encryption level
    
    uint8_t accept_request_policy; // ENABLE code to call unknowDIN()
    
    int64_t  oCap_net;        // ATS on network offset
    bool net_in_sync;         // synchronization status (0=not in sync)

    uint64_t  on_time; // time since power on (ms)

    ddo_policy_t ddo_policy; // DDO send policy

    discovery_state_t discovery_state;
} nodestate_t;

enum feature_t  {
    STORAGE = (1 << 0),        // Node supports storage features
    COMPUTE = (1 << 1),        // Node supports compute features
    MAX_FEATURE = COMPUTE,
};

typedef Vector<int> list_element;
typedef Vector<din_t> dinlist_t;      /// Node API !!!!!!!!!!!!!!
typedef Vector<typeset_t> typeset_list; /// Node API !!!!!!!!!!!!!!
typedef Vector<feature_t> features_list; /// Node API !!!!!!!!!!!!!!


#endif // !_DAAS_TYPES_H__