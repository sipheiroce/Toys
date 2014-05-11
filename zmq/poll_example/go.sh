g++ -o mpoller mpoller.cc -std=c++11 -I /usr/local/Cellar/zeromq/4.0.3/include/ -L /usr/local/Cellar/zeromq/4.0.3/lib/ -lzmq
g++ -o loop loop.cc -std=c++11 -I /usr/local/Cellar/zeromq/4.0.3/include/ -L /usr/local/Cellar/zeromq/4.0.3/lib/ -lzmq
g++ -o publisher publisher.cc -std=c++11 -I /usr/local/Cellar/zeromq/4.0.3/include/ -L /usr/local/Cellar/zeromq/4.0.3/lib/ -lzmq
g++ -o pusher pusher.cc -std=c++11 -I /usr/local/Cellar/zeromq/4.0.3/include/ -L /usr/local/Cellar/zeromq/4.0.3/lib/ -lzmq 
