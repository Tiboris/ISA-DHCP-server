#include "req_handler.hpp"
/*
 *  request handler for dserver.cpp as school project to ISA class
 */
using namespace std;

bool handle_request(scope_settings* scope, int* s)
{
    if ((*s = create_socket()) == -1)
    {
        cerr<< "ERR creating socket\n";
        return EXIT_FAILURE;
    }
    struct sockaddr_in c_addr;
    socklen_t c_len = sizeof(c_addr);
    // handling server run
    while (true)
    {
        int r;
        dhcp_packet p;
        r = recvfrom(*s, &p, sizeof(p), 0, (struct sockaddr*)&c_addr, &c_len);
        if (r < 0)
        {
            cerr << "ERR on recv\n";
            continue;
        }

        for (size_t i = 0; i < 6; i++)
        {
            char c='\0';
            (i == 6-1) ? c='\n' : c=':';
            cout << hex << +p.chaddr[i] << c << dec;
        }

        uint32_t offered_ip = get_ip_addr(scope, scope->first_addr);

        set_resp(scope, &p, offered_ip, DHCPOFFER);

        struct sockaddr_in br_addr;
        if (true)
        {
            br_addr.sin_family = AF_INET;                     // set IPv4 addressing
            br_addr.sin_addr.s_addr = scope->broadcast;       // broadcast addrs
            br_addr.sin_port = htons(PORT+1);                 // the client listens on this port
            int on = 1;
            if ((setsockopt(*s, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))) == -1)
                return EXIT_FAILURE;
        }
        r = sendto(*s, &p, sizeof(p), 0, (struct sockaddr*)&br_addr, c_len);
        printf("%u\n",p.op );
        if (r < 0)
        {
            cerr << "ERR on sendto\n";
            continue;
        }
        struct in_addr ip_addr;
        ip_addr.s_addr = p.yiaddr ;
        printf("IP %s, port %d\n",
        inet_ntoa(ip_addr),ntohs(c_addr.sin_port));
        //return EXIT_SUCCESS;
    }
    return EXIT_SUCCESS;
}

void set_resp(scope_settings* scope, dhcp_packet* p, u_int32_t offr_ip, int t)
{
    p->op = BOOTREPLY;
    p->hops = ZERO;
    p->secs = ZERO;
    p->ciaddr = ZERO;
    p->yiaddr = offr_ip;
    p->siaddr = scope->srv_addr;
    memset(&p->sname, ZERO, MAX_DHCP_SNAME_LENGTH);

    size_t pos = ZERO;

    response r;
    r.msg_type = t;


    memcpy(&p->options[pos], &r.magic_cookie, sizeof(r.magic_cookie));
    pos += sizeof(r.magic_cookie);


    memcpy(&p->options[pos], &r.msg_type_opt, sizeof(r.msg_type_opt));
    pos += sizeof(r.msg_type_opt);


    memcpy(&p->options[pos], &r.msg_type, sizeof(r.msg_type));
    pos += sizeof(r.msg_type);


    memcpy(&p->options[pos], &r.mask_type, sizeof(r.mask_type));
    pos += sizeof(r.mask_type);


    memcpy(&p->options[pos], &scope->mask, sizeof(scope->mask));
    pos += sizeof(&scope->mask);


    memcpy(&p->options[pos], &r.lease_time_opt, sizeof(r.lease_time_opt));
    pos += sizeof(r.lease_time_opt);


    memcpy(&p->options[pos], &r.lease_time, sizeof(r.lease_time));
    pos += sizeof(r.lease_time);


    memcpy(&p->options[pos], &r.srv_identif, sizeof(r.srv_identif));
    pos += sizeof(r.srv_identif);


    memcpy(&p->options[pos], &scope->srv_addr, sizeof(scope->srv_addr));
    pos += sizeof(&scope->srv_addr);


    p->options[pos]=255;
    cout<<"packet:\t";
    for (size_t i = 0; i <= pos; i++)
    {
        printf("%u|", p->options[i] );
    }
    cout<<endl;
    return;
}

int create_socket()
{// copied from my IPK project2 and edited
    int sockfd;
    struct sockaddr_in server_addr;
    // First call socket() function
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        return -1;
    }
    // Initialize socket structure
    //bzero((char *) &server_addr, sizeof(server_addr));
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    // Binding socket
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        return -1;
    }
    // Returning binded socket
    return sockfd;
}

uint32_t get_ip_addr(scope_settings* scope, uint32_t ip)
{// returns first free address from scope
    uint32_t offered_ip = ip;
    if (ip == scope->broadcast)
    {   // when free address is broadcast we are out of addresses in scope
        return UINT32_MAX;
    }
    else if (item_in_list(offered_ip, scope->exclude_list))
    {   // when address is already in use or in exclude_list we try again other
        offered_ip = htonl(offered_ip);
        offered_ip++; // next address might be usable
        offered_ip = htonl(offered_ip);
        return get_ip_addr(scope, offered_ip);
    }
    else
    {   // return first usable address and then add it to exclude_list
        scope->exclude_list.insert(scope->exclude_list.end(), offered_ip);
        return offered_ip;
    }
}

bool item_in_list( uint32_t item, vector<uint32_t> list)
{// returns true when item in list
    return (find(list.begin(), list.end(), item) != list.end());
}