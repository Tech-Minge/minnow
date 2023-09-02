**1. message.payload() = Buffer(_stream.read(message_len));**

operator=与std::shared_ptr<std::string>, 相关原理

不加Buffer也可以

**2. reply原理**

if the incoming segment occupied any sequence numbers, the TCPConnection makes
sure that at least one segment is sent in reply, to reflect an update in the ackno and
window size

每个TCP Segment是否都需要ACK, true false 辨析

rst需要ack吗

tcpconnection.cc question
send_tcpsegment need_ack = false 不能取消

**3. default value**

https://stackoverflow.com/questions/2545720/error-default-argument-given-for-parameter-1

**4. ack 需要seqno**

https://networkengineering.stackexchange.com/questions/69554/sequence-number-in-acknowledgement
