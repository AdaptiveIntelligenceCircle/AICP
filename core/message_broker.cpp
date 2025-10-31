#include "message_broker.hpp"
using namespace std; 

#include <queue> 
#include <thread> 

#include <condition_variable>
#include <iostream>

namespace aicp
{
    class SimpleMessageBroker : public IMessageBroker
    {
        private: 
        unordered_map<string, vector<function<void(const MessageEnvelope &)>>> subcribers;
        queue<MessageEnvelope> messages_queue; 
        mutex mtx; 
        condition_variable cv; 
        bool running = true; 
        thread worker; 

        void process_loop(){
            while (running)
            {
                unique_lock<mutex> lock(mtx); 
                cv.wait(lock, [&] { return  !running; });
                if (!running)
                break; 

                auto env = messages_queue.front(); 
                messages_queue.pop(); 
                lock.unlock(); 

                // dispatcj to subcribers..
                auto it = subcribers.find(env.protocol_id); 
                if (it != subcribers.end()){
                    for (auto &cb : it -> second){
                        try {
                            cb(env); 
                        }
                        catch(const exception &e){
                            cerr <<"[Broker] Callback error:" << e.what() << endl; 
                        }
                    }
                }
            }
        }

        public: 
        SimpleMessageBroker(){
            worker = thread([this]{
                process_loop(); 
            }); 
        }

        void publish(const MessageEnvelope &env) override 
        {
            {
                lock_guard<mutex> lock(mtx);
                messages_queue.push(env); 
            }
            cv.notify_one(); 
        }

        void subcribe (const string &protocol_id, function<void(const MessageEnvelope &)> cb ){
            lock_guard<mutex> lock(mtx); 
            subcribers[protocol_id] .push_back(cb);
        }
        
        void shutdown() override 
        {
            {
                lock_guard<mutex> lock(mtx); 
                running = false; 
            }
            cv.notify_all(); 
            if(worker.joinable())
            worker.join(); 
        }

        ~SimpleMessageBroker(){
            shutdown(); 
        }
    };
    
    // Factory function 
    unique_ptr<IMessageBroker> make_simple_broken(){
        return make_unique<SimpleMessageBroker>(); 
    }
}