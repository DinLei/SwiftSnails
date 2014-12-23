//
//  FunctionBasicServer.h
//  SwiftSnails
//
//  Created by Chunwei on 12/23/14.
//  Copyright (c) 2014 Chunwei. All rights reserved.
//

#ifndef SwiftSnails_SwiftSnails_server_FunctionBasicServer_h_
#define SwiftSnails_SwiftSnails_server_FunctionBasicServer_h_
#include <map>
#include "../../utils/common.h"
#include "../../utils/SpinLock.h"
#include "../Message.h"
#include "BasicServer.h"

namespace swift_snails {

template<typename MetaMsg_t, typename Msg_t, typename CallBack_t>
class FunctionBasicServer : public BasicServer {
public:
    explicit FunctionBasicServer() {
    }
    ~FunctionBasicServer() {
    }

    void register_message_class(index_t id, CallBack_t&& callback) {
        _spinlock.lock();
        CHECK(message_classes.count(id) == 0) << 
                "callback should be registerd only once;
        message_classes.insert(
            std::map<index_t, CallBack>::value_type(id, std::move(callback)));
        _spinlock.unlock();
    }

    void remove_message_class(index_t id) {
        _spinlock.lock();
        auto pos = message_classes.find(id);
        CHECK(pos != message_classes.end()) << 
                "no message_class:" << id << " found!";
        CHECK(message_classes.erase(pos) == 1);
        _spinlock.unlock();
    }

    void run() {
        Message meta_msg;
        Message cont_msg;

        while(true) {
            // receive meta-message first
            PCHECK(ignore_signal_call(zmq_msg_recv, meta_msg.zmg(), receiver(), 0) >= 0);
            if(meta_msg.length() == 0) return; // exit server 
            CHECK(meta_msg.length() == sizeof(MetaMsg_t));
            CHECK(zmq_msg_more(meta_msg.zmg()));
            // receive content-message later
            PCHECK(ignore_signal_call(zmq_msg_recv, cont_msg.zmg(), receiver(), 0) >= 0);
            parse_message( (MetaMsg_t*) meta_msg.buffer(), 
                           (Msg_t*) cont_msg.buffer() );
        }
    }

    virtual void parse_message(MetaMsg_t *meta_msg, Msg_t *cont_msg) = 0;

private:
    std::map<index_t, CallBack> message_classes;
    SpinLock _spinlock;
}; // end class FunctionBasicServer

}; // end namespace swift_snails
#endif