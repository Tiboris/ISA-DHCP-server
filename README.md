# Program: dserver (DHCP server)
Author: Tibor Dudlák
####Usage:
`./dserver -p 192.168.0.0/24 [-e 192.168.0.1,192.168.0.2] [-s static.txt]`
####Required Param:
* Network range to bind for DHCP server
    * -p *ip_address/mask*

####Optional Params:
* List of ip addresses separated by comma
    * -e *list_of_ip_addresses*
* File with static bindings for network (extension)
    * -s *static_bindings_file*

####Example Usage:
`./dserver -p 192.168.8.0/24`

`./dserver -p 192.168.8.0/24 -s static.txt`

`./dserver -p 192.168.8.0/24 -e 192.168.8.10,192.168.8.254`

`./dserver -p 192.168.8.0/21 -s static.txt -e 192.168.8.255`

####File Format:
MAC_ADDRESS IP_ADDRESS (eg: static.txt)
```
08:00:27:3a:9d:a6 192.168.8.99
08:00:27:3a:9d:a5 192.168.8.100
08:00:27:3a:9d:a4 192.168.8.101
08:00:27:3a:9d:a3 192.168.8.102
```
####List Of Files:
* README
* README.md
* Makefile
* dserver.cpp
* structures.hpp
* arg_parser.hpp
* arg_parser.cpp
* req_handler.hpp
* req_handler.cpp
* static.txt
