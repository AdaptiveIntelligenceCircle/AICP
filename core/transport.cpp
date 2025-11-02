#include "transport.h"
#include <thread>

#include <queue> 
#include <mutex> 

#include <condition_variable> 
#include <iostream>

#include <chrono> 
#include <sstream>

namespace aicp 
{
    class SimpleTCPTransport : public ITransport
    {
        public: 
        SimpleTCPTransport(bool running) {
            running = false; 
        }
        ~SimpleTCPTransport() override 
        {
            shutdown(); 
        }

        bool connect (const string &endpoint) override 
        {
            lock_guard <mutex> lock(mtx) ;
            this -> endpoint = endpoint; 
            running = true;  
            worker = thread([this](){
                process_loop(); 
            }); 

            cout << "Transport - Connected to " << endpoint << "\n";
            return true; 
        }

        bool listen(const string &endpoint) override 
        {
            lock_guard<mutex> lock(mtx); 
            this->endpoint = endpoint;
            running = true;
            worker = std::thread([this]() 
            { 
            process_loop(); 
            });
            cout << "[Transport] Listening on " << endpoint << "\n";
            return true;
        }

        bool send(const MessageEnvelope &env) override 
        {
            {
                lock_guard<mutex> lock(mtx); 
                if (!running)
                return false; 
                outbound.push(env); 
            }

            cv.notify_one(); 
            return true; 
        }

        void on_receive(function<void(const MessageEnvelope &)> cb ) override 
        {
            lock_guard<mutex> lock(mtx); 
            receive_cb = cb; 
        }

        void shutdown() override 
        {
            {
                lock_guard <mutex> lock(mtx); 
                running = false; 
            }
            cv.notify_all(); 
            if (worker.joinable())
            worker.join(); 
            cout <<"Transport Shutdown completed" << endl; 
        }

        private:  
        bool running = false;
        function<void(const MessageEnvelope &)> receive_cb;

        queue<MessageEnvelope> outbound;
        queue<MessageEnvelope> inbound;
        condition_variable cv ;

        string endpoint; 
        mutable mutex mtx; 
        thread worker;

        void process_loop()
        {
            using namespace chrono_literals; 
            while (true)
            {
                unique_lock<mutex> lock(mtx); 
                cv.wait_for(lock, 200ms, [&]() { 
                    return !outbound.empty() || !running; 
                });
                if (!running and outbound.empty())
                break; 

                if (!outbound.empty())
                {
                    auto env = outbound.front(); 
                    outbound.pop(); 
                    lock.unlock();

                    // simulate send + immediate loopback receive
                    this_thread :: sleep_for(20ms); 
                    simulate_receive(env); 
                }

            }
        }

        void simulate_receive(const MessageEnvelope &env)
        {
            lock_guard<mutex> lock(mtx);
            if (receive_cb)
            {
            // Dispatch asynchronously (simulate network callback)
            thread([cb = receive_cb, env]() 
            {
                this_thread::sleep_for(chrono::milliseconds(5));
                cb(env);
            }).detach();
            }
            else
            {
                cerr << "[Transport] Received message but no callback registered.\n";
            }
        }
    };

    unique_ptr<ITransport> make_tcp_transport()
    {
        return make_unique<SimpleTCPTransport>();
    }

    // Stubs for gRPC — placeholders for later integration

    unique_ptr<ITransport> make_grpc_transport_client(const string &target)
    {
    auto t = make_unique<SimpleTCPTransport>();
    t->connect(target);
    cerr << "[Transport] (gRPC stub) connected to target " << target << "\n";
    return t;
    }

    unique_ptr<ITransport> make_grpc_transport_server(int port)
    {
    auto t = make_unique<SimpleTCPTransport>();
    ostringstream oss;
    oss << "0.0.0.0:" << port;
    t->listen(oss.str());
    cerr << "[Transport] (gRPC stub) listening on port " << port << "\n";
    return t;
    }
}