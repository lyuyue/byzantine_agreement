##Byzantine Agreement: CS7680 Programming Assignment 1
####Yue Lyu

###Instruction:
Just `make` and run the binary with following command:<br />
For Commander:<br />
`./general -p port -h hostfile -f faulty_number -o order`<br />
For Lieutenants:<br />
`./general -p port -h hostfile -f faulty_number -C commander_id [-m]`  

`-p port`            : port number    
`-h hostfile`        : path to file with hosts  
`-f faulty_number`   : maximum number of malicious host in system  
`-C commander_id`    : id of commander, 0-indexed in hostfile  
`-o order`           : order 'attack' or 'retreat'  
`[-m]`               : malicious flag, if given, this host is malicious

Notice that commander **MUST** launch after all lieutenants are launched

###Code Architecture:  
**README.md**       
**Makefile**        
**constants.h**     : header file with constants definition   
**udp_socket.h**    : header file introducing helper function with udp sockets  
**message.h**      : header file with ByzantineMessage and Ack struct  
**general.c**       : main program  
  
###State diagrams:
Commander:
![Alt][1]
[1]: /diagrams/commander.png
Lieutenants:
![Alt][2]
[2]: /diagrams/lieutenant.png

###Design decisions
- Lieutenants will be waiting for initial order from Commander before doing anything else  
- Commander can not keep silient
- Malicious Commander are defined as a commander who will flip the initial order
- Malicious Lieutetant are defined as a lieutenant who will keep silent or delay sending messages  
- Timeout upon receiving any message from any host is set to be 1 second, and is subjected to change in constant.h
- MAX_TLE_COUNT is set to be 3, which means in a round, if all other hosts remain silent for 3 consecutive query, the host move to next round
- ByzantineMessage is constructed with a modified ByzantineMessage header struct with a list of ids

###Implementation issues
- The implementation is kind of 'tricky' since it is not strictly sychronized, a better implementation should include the logic with which the loyal lieutenants can exclude the malicious ones from system and ensure all message from loyal lieutenants are delivered.
- In this implementation, a delay caused by network latency might leads the sender being marked as a silent host, which in some sense, this implementation is incorrect.
