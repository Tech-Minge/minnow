**1. message.payload() = Buffer(_stream.read(message_len));**

operator=与std::shared_ptr<std::string>, 相关原理

不加Buffer也可以

**2. reply原理**

if the incoming segment occupied any sequence numbers, the TCPConnection makes
sure that at least one segment is sent in reply, to reflect an update in the ackno and
window size

每个TCP Segment是否都需要ACK, true false 辨析

TCP除了主动发起连接的第一个SYN包，ACK=0，其它所有TCP包都设置ACK= 1 标志位 https://www.zhihu.com/question/50584054

rst需要ack吗

tcpconnection.cc question
send_tcpsegment need_ack = false 不能取消

**3. default value**

https://stackoverflow.com/questions/2545720/error-default-argument-given-for-parameter-1

**4. ack 需要seqno**

https://networkengineering.stackexchange.com/questions/69554/sequence-number-in-acknowledgement

**5. tshark 网卡抓包**

tun tap

https://www.junmajinlong.com/virtual/network/all_about_tun_tap/index.html#:~:text=tun%E3%80%81tap%20%E6%98%AFLinux%20%E6%8F%90%E4%BE%9B,%E7%A9%BA%E9%97%B4%E4%B9%8B%E9%97%B4%E4%BC%A0%E8%BE%93%E6%95%B0%E6%8D%AE%E3%80%82

https://zhuanlan.zhihu.com/p/388742230

https://zu1k.com/posts/coding/tun-mode/

eth0, eth1, wlan0

https://zhuanlan.zhihu.com/p/515239767

**6. next hop ip address**

如何得知next hop ip

**7. arp**

第几层，与ether关系

**8. buffer & buffer list**

代码实现

**9. 网络发包全流程**

主机 arp mac router 3表 是否都有

默认网关 DHCP

主机路由 下一跳ip

**10. AsyncNetworkInterface**

什么作用
