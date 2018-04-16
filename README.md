# steps to run server on linux:

1) cd Gurux.DLMS/development

2) mkdir obj lib

3) make

4) cd ../../

5) mkdir obj bin

6) make

7) ./bin/server.bin <flag> <argument>

configurations options:
interface:         -i         wrapper/hdlc     (default = wrapper)
protocol:          -protocol  udp/tcp          (default = udp)
port:              -p         <number>         (default = 4061)
max pdu size:      -m         <number>         (default = 1024)
conformance block: -c         <number in hex>  (default is as requested by Yael) 