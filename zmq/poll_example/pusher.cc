//
//  Task ventilator in C++
//  Binds PUSH socket to tcp://localhost:5557
//  Sends batch of tasks to workers via that socket
//
//  Olivier Chamoux <olivier.chamoux@fr.thalesgroup.com>
//
#include <zmq.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <chrono>

using namespace std;

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

int main (int argc, char *argv[])
{
    zmq::context_t context (1);

    //  Socket to send messages on
    zmq::socket_t  sender(context, ZMQ_PUSH);
    sender.bind("tcp://*:5557");

    zmq::socket_t  receiver(context, ZMQ_PULL);
    receiver.connect("tcp://localhost:6556");

    zmq::message_t message(2);

    //  Initialize random number generator
    srandom ((unsigned) time (NULL));

    //  Send 100 tasks
    int task_nbr;
    int total_msec = 0;     //  Total expected cost in msecs
    for (task_nbr = 0; task_nbr < 10000; task_nbr++) {
        int workload;
        //  Random workload from 1 to 100msecs
        workload = within (100) + 1;
        total_msec += workload;

        message.rebuild(10);
        sprintf ((char *) message.data(), "%d", workload);
        auto t0 = chrono::high_resolution_clock::now();
        sender.send(message);

        zmq::message_t rmsg;
        receiver.recv(&rmsg);
        auto t1 = chrono::high_resolution_clock::now();
        chrono::milliseconds total_ms = chrono::duration_cast<chrono::milliseconds>(t1 - t0);
        cout << "get a message from - size " << rmsg.size() << " -- latency " << total_ms.count() << "ms" <<  endl;
        
        usleep(1000000);
    }
    std::cout << "Total expected cost: " << total_msec << " msec" << std::endl;
    sleep (1);              //  Give 0MQ time to deliver

    return 0;
}
