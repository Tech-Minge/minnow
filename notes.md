**1. message.payload() = Buffer(_stream.read(message_len));**

operator=与std::shared_ptr<std::string>, 相关原理

不加Buffer也可以

**2. reply原理**

if the incoming segment occupied any sequence numbers, the TCPConnection makes
sure that at least one segment is sent in reply, to reflect an update in the ackno and
window size

每个TCP Segment是否都需要ACK

tcpconnection.cc question