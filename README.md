# Message Delivery system
This is a message retrieval and posting system implemented in C. Implementation is done similar to **[MQTT](http://mqtt.org)** protocol which is a common protocol used for IoT applications.

## Implementation 
* **Components** - 
  1. Broker - Server for storage of messages posted by the *publisher*. There have to be a minimum of 2 brokers connect to eachother in a   circular network. Messages can be posted to any of the brokers and retrieved from any other. Every Broker on the system has a unique       `broker_id` that it uses to identify itself.
  2. Publisher - client that provides interface to post messages to the broker. It has interface to create a topic on the broker. Messages
  are published under a topic.
  3. Subscriber - client that provides interface to read messages from the brokers. Any subscriber first has to subscribe to a topic
  before reading any messages under it. 
  

* **General Struct Definition** -
```
typedef struct my_struct{
    int type;
    char message[512];
    char topic_name[50];
    int msg_id;
    int client_id;
    int broker_id;
}master;
```
Any communication between Broker, Publisher and Subscriber is done using the above struct

* **Setup** (for network of 2 brokers)
1. Compile and run `first_broker.c` and enter the `LISTEN_PORT`.
2. Compile and run `broker.c` in another folder and input `IP_ADDRESS` and `LISTEN_PORT` of the `first_broker` as CLA, also assign `LISTEN_PORT` for the second broker.
3. Enter the `IP_ADDRESS` and `LISTEN_PORT` that you assigned to second broker in the `first_broker`.
4. Circular network of 2 broker has been established.
5. Compile and run `publisher.c` and enter the `IP_ADDRESS` and `LISTEN_PORT` of the any one of the brokers and follow the interface to publish a message.

 
