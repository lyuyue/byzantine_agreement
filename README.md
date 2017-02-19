CS7680 Programming Assignment 1:
Byzantine Agreement

Instruction:
Just `make` and run the binary with following command:
For Commander:
`./general -p port -h hostfile -f faulty_number -o order`
For Lieutenants:
`./general -p port -h hostfile -f faulty_number -C commander_id [-m]`

-p port            port number
-h hostfile>       path to file with hosts
-f faulty_number   maximum number of malicious host in system
-C commander_id    id of commander, 0-indexed in hostfile
-o order           order 'attack' or 'retreat'
[-m]               malicious flag, if given, this host is malicious

Code Architecture:
README.md       
Makefile        
constants.h     header file with constants definition
udp_socket.h    header file introducing helper function with udp sockets
message.h       header file with ByzantineMessage and Ack struct
general.c       main program

State diagrams:
Commander:
![Alt][1]
[1]: /diagrams/commander.png
Lieutenants:
![Alt][2]
[2]: /diagrams/lieutenant.png

