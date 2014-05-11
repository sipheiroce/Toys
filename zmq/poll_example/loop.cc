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

    zmq::socket_t  sender1(context, ZMQ_PUSH);
    sender1.bind("tcp://*:6556");

    usleep(2000000);

    //  Process messages from both sockets
    while (1) {
        zmq::message_t message;
        
        receiver.recv(&message);
        zmq::message_t rmsg(11);
        sender1.send(rmsg);
        cout << "get a message from Push" << endl;
        usleep(2000);
    }
    return 0;
}
