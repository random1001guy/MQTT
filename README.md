# Message Delivery system
This is a message retrieval and posting system implemented in C. Implementation is done similar to **[MQTT](http://mqtt.org)** protocol which is a common protocol used for IoT applications.

## Implementation
* **Block Diagram** - 
* **Components** - 
  1. Broker - Server for storage of messages posted by the *publisher*. There have to be a minimum of 2 brokers connect to eachother in a circular network. Messages can be posted to any of the brokers and retrieved from any other.
  2. Publisher - client that provides interface to post messages to the broker

