#include "zmq.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

int main (int argc, char *argv[])
{
    zmq::context_t context(1);


    //  Connect to weather server
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:5556");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);



    usleep(2000000);

    //  Process messages from both sockets
    while (1) {
        zmq::message_t message;
            cout << "ready to get a message " << endl;    
            subscriber.recv(&message);
            //  Process weather update
            cout << "get a message from Pub" << endl;
    }
    return 0;
}
