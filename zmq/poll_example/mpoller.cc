#include "zmq.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

int main (int argc, char *argv[])
{
    zmq::context_t context(1);

    //  Connect to task ventilator
    zmq::socket_t receiver(context, ZMQ_PULL);
    receiver.connect("tcp://localhost:5557");

    //  Connect to weather server
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:5556");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);


    zmq::socket_t  sender1(context, ZMQ_PUSH);
    sender1.bind("tcp://*:6556");

    zmq::socket_t  sender2(context, ZMQ_PUSH);
    sender2.bind("tcp://*:6557");

    usleep(2000000);

    //  Initialize poll set
    zmq::pollitem_t items [] = {
        { receiver, 0, ZMQ_POLLIN, 0 },
        { subscriber, 0, ZMQ_POLLIN, 0 }
    };
    //  Process messages from both sockets
    while (1) {
        zmq::message_t message;
        // POLL will block
        cout << "before poll " << endl;
    
        zmq::poll (&items [0], 2, -1);
        cout << "after poll" << endl;    
    
        if (items [0].revents & ZMQ_POLLIN) {
            receiver.recv(&message);
            zmq::message_t rmsg(11);
            sender1.send(rmsg);
            cout << "get a message from Push" << endl;
            //  Process task
        }
        if (items [1].revents & ZMQ_POLLIN) {
            subscriber.recv(&message);
            zmq::message_t rmsg(12);
            sender2.send(rmsg);
            //  Process weather update
            cout << "get a message from Pub" << endl;
        }
    }
    return 0;
}
