#include <zmq.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <chrono>
#include <iostream>

using namespace std;

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

int main () {

    //  Prepare our context and publisher
    zmq::context_t context (1);
    zmq::socket_t publisher (context, ZMQ_PUB);
    publisher.bind("tcp://*:5556");

    zmq::socket_t  receiver(context, ZMQ_PULL);
    receiver.connect("tcp://localhost:6557");

    usleep(1000000);

    //  Initialize random number generator
    srandom ((unsigned) time (NULL));
    while (1) {

        int zipcode, temperature, relhumidity;

        //  Get values that will fool the boss
        zipcode     = within (100000);
        temperature = within (215) - 80;
        relhumidity = within (50) + 10;

        //  Send message to all subscribers
        zmq::message_t message(20);
        snprintf ((char *) message.data(), 20 ,
            "%05d %d %d", zipcode, temperature, relhumidity);
        auto t0 = chrono::high_resolution_clock::now();
        publisher.send(message);

        zmq::message_t rmsg;
        receiver.recv(&rmsg);
        auto t1 = chrono::high_resolution_clock::now();
        chrono::milliseconds total_ms = chrono::duration_cast<chrono::milliseconds>(t1 - t0);
        cout << "get a message from - size " << rmsg.size() << " -- latency " << total_ms.count() << "ms" <<  endl;


        usleep(1000000);
    }
    return 0;
}
