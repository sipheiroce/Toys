#include "lf_buffer.hpp"
#include <boost/thread.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

LockFreeQueue<string> *lf_q;


void producer(LockFreeQueue<string> *lq, size_t id, string prefix) {
   char buf[10];
   for(size_t i = 0; i < 10; ++i) {
       snprintf(buf, sizeof(buf), "%d", i);
       string tmp(buf);
       string * x = new string();
       *x = prefix + tmp;
       cout << "thread " << id << "  push " << *x << endl;
       lq->push(x, id);
   }

}

void consumer(LockFreeQueue<string> *lq, size_t id) {
   for(size_t i = 0; i < 10; ++i) {
       usleep(100000);
       cout << "thread id " << id << " pop " << *lq->pop(id) << endl;;
   }

}





int main(){
    //LockFreeQueue<string> lf_q(1, 1);
    lf_q = new LockFreeQueue<string>(4,4);
    boost::thread p1(producer, lf_q, 0, "a");    
    boost::thread c1(consumer, lf_q, 1);
    boost::thread p2(producer, lf_q, 2, "b");
    boost::thread c2(consumer, lf_q, 3);

    p1.join();
    p2.join();
    c1.join();
    c2.join();

    return 0;
}
