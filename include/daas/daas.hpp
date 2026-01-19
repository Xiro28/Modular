/*
 * DaaS-IoT 2019, 2026 (@) Sebyone Srl
 *
 * File: daas.h
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

#ifndef DAAS_H
#define DAAS_H

#include "daas_types.hpp"

/* ----------------------------------------------------------------------------------------------------------- */

class DaasAPI
{
public:
    DaasAPI();                                    
    DaasAPI(IDaasApiEvent *);                     
    DaasAPI(IDaasApiEvent *, const char *lhver_); 
    ~DaasAPI();

    //////////////////////////////////////////////////////////////////////
    ////  G E N E R A L S                                             ////
    //////////////////////////////////////////////////////////////////////

    /**
    @details Retrieves the version of the DaaS API library.
    @param none
    @returns A string representing the version of the DaaS API library.
    @end
    */
    const char *getVersion();

    /**
    @details Retrieves build information of the DaaS API library.
    @param none
    @returns A string containing build information such as compiler version and build configuration.
    @end
    */
    const char *getBuildInfo();

    /**
    @details Lists all available drivers in the DaaS stack for this node.
    @param none
    @returns A semicolon-separated string containing the list of available drivers.
    @end
    */
    const char *listAvailableDrivers();

    /**
    @details Releases resources and deactivates the node. 
    @param none
    @returns @b ERROR_NONE on success, or an error code on failure.
    @end
    */
    daas_error_t doEnd();                         // releases resources and deactivates node
   
    /**
        @details Resets the local node and clears all resources. It disconnects also from the network that it currently is.
        @param none
        @returns @b ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t doReset();                       // reset resources and restarts services
    
    /**
        @details Initializes services and resources for the local node.
        @param sid: SID of the local node
        @param din: DIN of the local node
        @returns @b ERROR_NONE on success, or an error code on failure.

        \par Example:
        \snippet examples/init/main.cpp init
        @end
    */
    daas_error_t doInit(din_t sid, din_t din);  // initializes services and resources (Real-Time or Multi-Threading, release dependent)
    
    /**

        @details Executes the node's main processing loop in either real-time or multi-threading mode. 
        It manages the node's internal state, including network communication, data processing, and synchronization with other nodes. 
        In @b multi-threading mode, it spawns internal threads to handle processing.
        
        @param mode:
        - @b PERFORM_CORE_THREAD for multi-threading mode
        - @b PERFORM_CORE_NO_THREAD for real-time mode
        
        @returns @b ERROR_NONE on success, or an error code on failure.

        @note This function @b must be called if the node has to be operational. In @b multi-threading mode it has to be called only once,
              while in @b real-time mode it has to be called cyclically.

        @see doInit
        @end
    */
    daas_error_t doPerform(performs_mode_t mode); // perform node's task ( in RT mode needs to be called cyclically)
    
    /**
        @details Configure driver for network technology (links)
        
        @param driver_id: the communication technology to enable:
                          - @b _LINK_INET4
                          - @b _LINK_BLUETOOTH
                          - @b and others as defined in link_t
        @param local_uri: the physical address of the local node:
                            - For @b _LINK_INET4: a text string in the format "IP:PORT" (e.g., "192.168.1.2:4000")
                            - For @b _LINK_BLUETOOTH: a text string representing the Bluetooth address (e.g., "00:1A:7D:DA:71:13:CHANNEL")


        @returns @b ERROR_NONE on success, or an error code on failure.

        @see doInit
        @end
    */
    daas_error_t enableDriver(link_t driver_id, const char *local_uri); // Configure driver for network technology (links)

    /**
        @details Returns the status of the local node.
       
        @param none
        
        @returns the status of the local node.
        - This includes hardware version, linked channels, synchronization status, security policy, and more.
    
        @end
    */
    nodestate_t getStatus(); // returns local node's instance status

    /**
        @details The function sets whether the local node should accept all incoming requests, 
        including those from unknown DINs, known din from different network or only from din inside the same network.
       
        @param policy_level:
         - @b 0: accept requests only from known DINs inside the same network.
         - @b 1: accept requests from known DINs from different networks and inside the same network.
         - @b 2: accept requests from all DINs, including unknown ones

        
        @note Default policy_level is @b 1.
        During the discovery, the policy_level is temporary set to @b 2 since the node has to accept all the incoming request. 
        Right after the discovery process is compleated or goes out of time, the policy_level is set back to the previous value.

        @end
    */
    void setAcceptRequestsLevel(int policy_level);
     
    /**
        @details Saves the current configuration to the specified storage interface.
        
        @param storage_interface: the interface to save the configuration (e.g., a file system, database, etc.)
        
        @returns true if the backup was successful, false otherwise.
        
        @see loadConfiguration

        @end
    */
    bool storeConfiguration(IDepot* storage_interface);

    /**
        @details Loads the configuration from the specified storage interface.
        
        @param storage_interface: the interface to load the configuration from (e.g., a file system, database, etc.)
        
        @returns true if the configuration was loaded successfully, false otherwise.
        
        \par Example:
        \snippet examples/restore/main.cpp backup_restore
        @end
    */
    bool loadConfiguration(IDepot* storage_interface);
    
    /**
        @details Resets the system's statistics data.

        @param none

        @returns true if the reset was successful, false otherwise.
        @end
    */
    bool doStatisticsReset();

    /**
        @details Returns the statistics of the local node for a specific system code.

        @param label: the system code to get statistics for (e.g., _cor_dme_sended)

        @returns the system statistics for the given label.
        @end
    */
    uint64_t getSystemStatistics(syscode_t label); 
   
    /* Mapping      -------------------------------------------------------------------------------------------- */

    /**
        @details Maps a remote node to the local instance. This is used to establish communication with other nodes in the network.
       
        @param din: DIN of the node to map

        @returns @b ERROR_NONE on success, or an error code on failure.

        @see doInit
        @end
    */
    daas_error_t map(din_t din);                                                   

    /**
        @details Maps a remote node to the local instance. This is used to establish communication with other nodes in the network.
       
        @param din: DIN of the node to map
        @param link: the communication technology to use: 
                        - @b _LINK_INET4 
                        - @b _LINK_BLUETOOTH
                        - @b and others as defined in link_t
        @param suri: the physical address of the node:
                        - For @b _LINK_INET4: a text string in the format "IP:PORT" (e.g., "192.168.1.2:4000")
                        - For @b _LINK_BLUETOOTH: a text string representing the Bluetooth address (e.g., "00:1A:7D:DA:71:13:CHANNEL")

        @returns @b ERROR_NONE on success, or an error code on failure.

        @note The communication links supported depend on the build configuration and enabled drivers. 
              Those can be listed using the @ref listAvailableDrivers() function.

        @see doInit
        @end
    */
    daas_error_t map(din_t din, link_t link, const char *suri);                   // adds node-identifier and related physical address ( link: 1="INET4", 2="UART", 3="MQTT5")
    
    /**
        @details Maps a remote node to the local instance. This is used to establish communication with other nodes in the network.
       
        @param din: DIN of the node to map

        @param link: the communication technology to use: 
        - @b _LINK_INET4 
        - @b _LINK_BLUETOOTH
        - @b and others as defined in link_t

        @param suri: the physical address of the node:
        - For @b _LINK_INET4: a text string in the format "IP:PORT" (e.g., "192.168.1.2:4000")
        - For @b _LINK_BLUETOOTH: a text string representing the Bluetooth address (e.g., "00:1A:7D:DA:71:13:CHANNEL")
        
        @param skey: the security key for the node. Currently not implemented.

        @returns @b ERROR_NONE on success, or an error code on failure.

        @note The communication links supported depend on the build configuration and enabled drivers. 
              Those can be listed using the @ref listAvailableDrivers() function.

        @see doInit
        @end
    */
    daas_error_t map(din_t din, link_t link, const char *suri, const char *skey); // adds node-identifier and related physical address ( link: 1="INET4", 2="UART", 3="MQTT5")
    
    /**
        @details Removes a node from the local instance and cleans up associated resources.

        @param din: DIN of the node to remove

        @returns @b ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t remove(din_t din);

    /**
     *  @details Starts a discovery process to locate nodes in the network.\n 
     *  This process will send the enabled drivers URI to a sort of broadcast in order to auto-map this node to remote ones.\n
        @note The reception of this packet cannot be controlled, it can only be disabled.
     
        @param none : starts discovery on all available links

        @returns @b ERROR_NONE on success, or an error code on failure.
        @see doInit

        \par Example:
        \snippet examples/discovery/main.cpp discovery_without_init
        @end
     */
      daas_error_t discovery();

      /**
     *  @details Starts a discovery process to locate nodes in the network.\n 
     *  This process will send the enabled drivers URI to a sort of broadcast in order to auto-map this node to remote ones.\n
       @note The reception of this packet cannot be controlled, it can only be disabled.
     
        @param link: The communication technology to use for discovery. If not specified, all available links will be used.

        @returns @b ERROR_NONE on success, or an error code on failure.
        @see doInit

        \par Example:
        \snippet examples/discovery/main.cpp discovery_without_init
        @end
     */
      daas_error_t discovery(link_t link);


    /**
        @details Configures how the node interacts with the network discovery process. 
        Depending on the selected mode, the node can search for existing networks, 
        host new ones, or remain invisible to other nodes.

        @param mode The discovery activation mode:
        - @b discovery_off: Completely disables discovery. The node will neither 
        send nor respond to discovery beacons.
        - @b discovery_sender_only: The node actively searches for existing networks 
        but will not host or respond to discovery packets from others.
        - @b discovery_receiver_only: The node acts as a host, assigning network 
        parameters to others, but will not seek to join external networks.
        - @b discovery_full: Enables full bidirectional discovery (both seeking 
        and hosting/joining networks).

        @note By default, the node is set to @b discovery_sender_only mode upon initialization.

        @end
     */
      void setDiscoveryState(discovery_state_t mode); 


    /* Availability -------------------------------------------------------------------------------------------- */
    
    /**
        @details Returns map entries (known nodes) in the local instance.

        @param none

        @returns a list of known nodes (din_t) in the local instance.
        @end
    */
    dinlist_t listNodes();                       // Returns map entries  (knows nodes) ( din1, din2, )
    
    /**
        @details It starts a process to locate the node if it is not inside the known table.
        
        @param din_: DIN of the node to locate
        @param timeout (optional): Maximum time to wait for the node to be located (in milliseconds). Default is 1000 ms.
        @param ttl (optional): Maximum number of intermediate hops to reach the node. Default is 10.

        @returns @b ERROR_NONE if the node is found, or an error code if it is not.

        @see pull
        @end
    */
    daas_error_t locate(din_t din, int timeout = 1000); // Locate node if not inside known table (calls pull)

    /**
        @details It starts a process to locate any node with the specified feature.
        
        @param feature: Feature to search for.
        @param timeout (optional): Maximum time to wait for the node to be located (in milliseconds). Default is 1000 ms.
        @param ttl (optional): Maximum number of intermediate hops to reach the node. Default is 10.

        @returns @b ERROR_NONE if the node with the feature is found, or an error code if it is not.

        @see pull
        @end
    */
    daas_error_t locate(feature_t feature, int timeout = 1000, int ttl = 10); // Locate node if not inside known table (calls pull)
    
    /**
        @details Send the local node's status to a remote node.
        
        @param din: DIN of the remote node to send the status to

        @returns @b ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t sendStatus(din_t din); // Send local status to remote node (din)
    
    /**
        @details Fetches the status of a remote node.

        @param din: DIN of the remote node to fetch the status from
        
        @returns the nodestate_t of the remote node.
        @end
    */
    const nodestate_t& status(din_t din);

    /**
        @details Fetches the status of a remote node and updates its status in the local instance.
        
        @param din: DIN of the remote node to fetch the status from
        @param opts: options for fetching 
       
        @returns the nodestate_t of the remote node after fetching.
        @end
    */
    const nodestate_t& fetch(din_t din, uint16_t opts);

    /**
        @details Time with ATS correction used to be able to communicate inside the DaaS network.
        
        @param none

        @attention This function returns the synchronized timestamp of the local node, only when connected to a DaaS network and ATS is in sync.
                   If the node is not synchronized, the function returns the local system time without ATS correction.
        
        @returns the synced timestamp of the local node.
        @end
    */
    uint64_t getSyncedTimestamp();

    /* Security     -------------------------------------------------------------------------------------------- */
    
    /**
        @details Unlocks a remote node by setting its security key.
        
        @param din: DIN of the remote node to unlock
        @param skey: the security key to set

        @warning Not implemented yet!
        
        @returns the nodestate_t of the remote node after unlocking.
        @end
    */
    const nodestate_t& unlock(din_t din, const char *skey); 
    
    /**
        @details Set SKEY and security policy for local node
        
        @param skey: the security key to set
        @param policy_: the security policy to set

        @warning Not implemented yet!

        @returns the nodestate_t of the local node after setting the security key and policy.
        @end    
    */
    const nodestate_t& lock(const char *skey, unsigned policy_);

    /* Synchronize  -------------------------------------------------------------------------------------------- */
    
    /**
        @details Set the local system time on remote node din and synchronize ATS.

        @param din: DIN of the remote node to synchronize
        @param timezone: the timezone offset in seconds

        @warning Not implemented yet!

        @returns the nodestate_t of the remote node after synchronization.
        @end
    */
    const nodestate_t& syncNode(din_t din, unsigned timezone);  

    /**
        @details Set the local system time on remote node din and synchronize ATS

        @param din: DIN of the remote node to synchronize
        @param bubble_time: max error allowed for synchronization in milliseconds

        @warning Not implemented yet!

        @returns the nodestate_t of the remote node after synchronization.
        @end
    */
    const nodestate_t& syncNet(din_t din, unsigned bubble_time);

    /**
        @details Set the maximum error allowed for ATS synchronization. This value defines the acceptable time deviation between the local node and remote nodes in the network.
        The ATS mechanism uses this parameter to ensure that all nodes maintain a consistent time reference within the specified error margin. 
        
        @note By default, this value is set to @b 100 milliseconds.
        
        @param error: the maximum error in milliseconds

        @end
    */
    void setATSMaxError(int32_t error); 

    /* Exchange     -------------------------------------------------------------------------------------------- */
    
    /**
        @details Starts a real-time session with a remote node.

        @param din: DIN of the remote node to start the session with

        @warning Not implemented yet!

        @returns true if the RT session was successfully started, false otherwise. (OPEN CONNECTION!!!!)
        @end
    */
    bool use(din_t din);                                                    

    /**
        @details Ends a real-time session with a remote node.
        
        @param din: DIN of the remote node to end the session with

        @warning Not implemented yet!

        @returns true if the RT session was successfully ended, false otherwise.
        @end
    */
    bool end(din_t din);

    /**
        @details Sends data to a remote node in a real-time session.
        
        @param din: DIN of the remote node to send data to
        @param outbound: pointer to the data to send
        @param size: size of the data to send

        @warning Not implemented yet!

        @returns the size of data sent.

        @end
    */
    unsigned send(din_t din, unsigned char *outbound, unsigned size);

    /**
        @details Checks if there is data available from a remote node in a real-time session.
        
        @param din: DIN of the remote node to check for data

        @warning Not implemented yet!

        @returns the size of data received.

        @end
    */
    unsigned received(din_t din);


    /**
        @details Receives data from a remote node in a real-time session.
        
        @param din: DIN of the remote node to receive data from
        @param inbound: reference to a variable that will hold the received data
        @param max_size: maximum size of data to receive

        @warning Not implemented yet!

        @returns the size of data received.

        @end
    */
    unsigned receive(din_t din, unsigned char &inbound, unsigned max_size);

    /* Transfer     -------------------------------------------------------------------------------------------- */
    /**
        @details Returns a list of user-defined typesets.
        
        @param none

        @returns a reference to the list of user-defined typesets.
        
        @see addTypeset
        
        @note The list is of type typeset_list, which is a Vector of typeset
        @end
    */
    typeset_list &listTypesets();
   
    /**
        @details Pulls a DDO from a remote node.

        @param din: DIN of the remote node to pull data from
        @param inboundDDO: pointer to a DDO pointer that will hold the pulled DDO
        
        @returns @b ERROR_NONE on success, or an error code on failure.

        \par Example:
        \snippet examples/pull/main.cpp pull

        @end
    */
    daas_error_t pull(din_t din, DDO **inboundDDO);

    /**
        @details Pushes a DDO to a remote node.

        @param din: DIN of the remote node to send data to
        @param outboundDDO: pointer to the DDO to send

        @returns @b ERROR_NONE on success, or an error code on failure.

        \par Example:
        \snippet examples/push/main.cpp push

        @end
    */
    daas_error_t push(din_t din, DDO *outboundDDO);

    /**
        @details Checks if there are available DDOs from a remote node.

        @param din: DIN of the remote node to check for available DDOs
        @param count: reference to a variable that will hold the number of available DDOs
       
        @returns @b ERROR_NONE on success, or an error code on failure.

        @see pull

        @end
    */
    daas_error_t availablesPull(din_t din, uint32_t &count);

    /**
        @details Maps a user-defined typeset to a handler function. If a DDO with the specified typeset is received,
        the provided handler function will be called.

        @param typeset_code: the code of the typeset to add
        @param fun: The function to handle received DDOs of this typeset

        @note The registered typeset does not trigger the received DDO event.

        @returns @b ERROR_NONE on success, or an error code on failure.

        \par Example:
        \snippet examples/typeset_handler/main.cpp handler

        @end
    */
    daas_error_t addTypeset(const uint16_t typeset_code, const typeset_fun fun);
    
    
    /* TEST */

    /**
        @details Pings a remote node to check its availability.

        @param din: DIN of the remote node to ping

        @returns @b ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t frisbee(din_t din);               

    /**
        @details Pings a remote node with a specified timeout and retry count.

        @param din: DIN of the remote node to ping
        @param timeout: maximum time to wait for a reply (in milliseconds)
        @param retry: number of retries if no reply is received

        @returns @b ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t frisbeeICMP(din_t din, uint32_t timeout, uint32_t retry); 

    /**
        @details Measures the performance of data transfer to a remote node.
        
        @param din: DIN of the remote node to ping
        @param sender_pkt_total: total number of packets to send
        @param block_size: size of each packet in bytes
        @param sender_trip_period: time period between each packet sent (in milliseconds)
        
        @returns @b ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t frisbeeDPERF(din_t din, uint32_t sender_pkt_total = 10, uint32_t block_size = 1024*1024, uint32_t sender_trip_period = 0); 
    
    /**
        @details Returns the result of a frisbee performance test.
        
        @param none
        
        @returns a @b dperf_info_result structure containing the performance test results:
        - @b sender_first_timestamp: timestamp of the first packet sent by the sender
        - @b local_end_timestamp: timestamp of the last packet received by the sender
        - @b remote_first_timestamp: timestamp of the first packet received by the receiver
        - @b remote_last_timestamp: timestamp of the last packet received by the receiver
        - @b remote_pkt_counter: number of packets received by the receiver
        - @b remote_data_counter: total data received by the receiver in bytes
        @end
    */
    dperf_info_result getFrisbeeResultDPERF();


    /**
        @details Sets the DDO policy for the local node. This policy determines how to manage the unsuccessful sending of DDOs.
        @param policy: the DDO policy to set (ddo_policy_t)
            - @b ddo_policy_skip_on_failure: Skip sending the DDO on failure and deletes it from the sending buffer.
            - @b ddo_policy_retry_on_failure: Retry sending the DDO at every @ref doPerform cycle.
            - @b ddo_policy_exponential_backoff_retry_on_failure: Retry sending the DDO with exponential backoff on failure. 
                         Currently, the backoff starts at 1 minute and doubles with each failure, up to a maximum of 1 hour.

        @note The default policy is @b ddo_policy_skip_on_failure. If the node with mode @b ddo_policy_exponential_backoff_retry_on_failure receives a DDO from the remote node, the 
                retry timer for that DDO is reset and sending will be retried immediately in the next @ref doPerform cycle. 

        @attention The retry policies may change the order of DDOs being sent, especially if multiple DDOs are queued for the same remote node. 
                    For example, if DDO1 fails to send and is set to retry later, while DDO2 is successfully sent in the next cycle, DDO2 may be sent before DDO1.

        @returns @b ERROR_NONE on success, or an error code on failure.
        @end
    */
    daas_error_t setDDOPolicy(ddo_policy_t policy);


    /**
        @details Unbinds the local node from the current network, closing all the open channels.
        The mapping table is preserved, so the node can rebind to the same network later without needing to remap known nodes.

        @warning After unbinding, the node will not be able to communicate with some nodes of the previous network (it depends on their @ref setAcceptRequestsLevel).
        
        @param none

        @returns @b ERROR_NONE on success, or an error code on failure.
        @end
     */
    daas_error_t unbindNetwork(); 

    /**
        @details Sets the features of the local node.
        Feature descriptions:
        - @b STORAGE: The node has storage capabilities. So inside a DaaS network, other nodes can store or delegate data (DDO for example) to/from this node.
        - @b COMPUTATION: The node has computation capabilities. So inside a DaaS network, other nodes can delegate computation tasks to/from this node.

        @param features: the features to set (STORAGE, COMPUTATION, etc.)
    */
   daas_error_t addNodeFeatures(feature_t features);

    /**
        @details Gets the features of the local node.
    
        @param none
    
        @returns the features of the local node (STORAGE, COMPUTATION, etc.)
    */
   features_list getNodeFeatures();

   
};

#endif // DAASIOT_H
