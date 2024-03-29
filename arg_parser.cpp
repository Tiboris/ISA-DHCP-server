// Author Tibor Dudlák xdudla00@stud.fit.vutbr.cz
#include "arg_parser.hpp"
/*
*  Argument parser for dserver.cpp as school project to ISA class
*/
using namespace std;

bool arg_err(char option, char* optarg_val, scope_settings* scope)
{
    size_t pos = 0;
    if (option == 'p')
    {
        string delimiter = "/";
        string tmp = static_cast<string>(optarg_val);
        if ((pos = tmp.find(delimiter)) == string::npos)
            return EXIT_FAILURE;
        char result[INET_ADDRSTRLEN];
        cut(optarg_val, 0, pos, result);                        //eg:
        if ((scope->net_addr = strtoip(result)) == UINT32_MAX)  //netw 10.0.0.0
        {
            cerr << ERR_IP_FORMAT << optarg_val << endl;
            return EXIT_FAILURE;
        }
        tmp.erase(0, pos + delimiter.length());
        uint32_t cmask = 0;
        try
        {
            for (auto i = tmp.begin(); i != tmp.end(); i++)
            {// check string for digits
                if (!isdigit(*i))
                    throw EXIT_FAILURE;
            }
            cmask = stoul(tmp,nullptr,10);  //stuol throw exception when fails
            if ((cmask == 0) || (cmask > 30 ))
                throw EXIT_FAILURE;
        }
        catch (...) // catch any exception
        {
            cerr << "CIRD mask '/" << tmp << "' NOT supported."<< endl;
            return EXIT_FAILURE;
        }
        if (scope->net_addr >> cmask)
        {
            cerr << "Argument: '" << optarg_val << ERR_NET;
            return EXIT_FAILURE;
        }
        scope->mask = scope->mask >> cmask;                     // 0.0.255.255
        scope->mask = htonl(scope->mask);                       // endian
        scope->broadcast = scope->net_addr + scope->mask;       // 10.0.255.255
        scope->mask = ~ scope->mask;                            // 255.255.0.0
        scope->srv_addr = htonl(scope->net_addr);               // indian
        scope->srv_addr++;                                      // 10.0.0.1
        scope->first_addr = htonl(scope->srv_addr + 1);         // 10.0.0.2
        scope->srv_addr = htonl(scope->srv_addr);               // indian
        scope->exclude_list.insert(scope->exclude_list.end(), scope->srv_addr);
    }
    else if (option == 'e')
    {
        string delimiter = ",";
        uint32_t exclude;
        size_t start_pos = 0;
        char result[INET_ADDRSTRLEN];                           // allocs memory
        string tmp = static_cast<string>(optarg_val);
        // to easily find delimiter in string
        while ((pos = tmp.find(delimiter)) != string::npos)
        {
            string token = tmp.substr(0, pos);                  // cuts imput
            tmp.erase(0, pos + delimiter.length());             // erase token
            cut(optarg_val, start_pos, pos+start_pos, result);  // cuts char*
            if ((exclude = strtoip(result)) == UINT32_MAX)
            {   // check if valid address in result of cutting
                cerr << ERR_IP_FORMAT << token << endl;
                return EXIT_FAILURE;
            }
            start_pos = start_pos + pos + delimiter.length();
            // moves to right when successfuly cuts and converts result to ip
            scope->exclude_list.insert(scope->exclude_list.end(), exclude);
            // adds ip to exclude list vector
        }   // it has to be done one more time down below to not skip last item
        cut(optarg_val, start_pos, start_pos + tmp.length(), result);
        if ((exclude = strtoip(result)) == UINT32_MAX)
        {
            cerr << ERR_IP_FORMAT << result << endl;
            return EXIT_FAILURE;
        }
        scope->exclude_list.insert(scope->exclude_list.end(), exclude);
    }
    return EXIT_SUCCESS;
}

bool opt_err(int argc, char** argv, scope_settings* scope)
{
    int min_opt_cnt = 2;
    if (argc < min_opt_cnt)
    {
        cerr << ERR_NO_ARGS << USAGE;
        return EXIT_FAILURE;
    }
    uint32_t pflag = 0;
    uint32_t eflag = 0;
    uint32_t sflag = 0;
    int max_argc_val = 1;
    int c;
    while ((c = getopt (argc, argv, "p:e:s:")) != -1)
    {
        switch (c)
        {
            case 'p':
                if (pflag)
                {
                    cerr << ERR_MULTIPLE_OPT << (char)c << endl << USAGE;
                    return EXIT_FAILURE;
                }
                if (arg_err(c, optarg, scope ))
                {
                    cerr << ERR_ARG_1 << (char)c << ERR_ARG_2 << USAGE;
                    return EXIT_FAILURE;
                }
                pflag++;    // to check if there are multiple options like this
                max_argc_val += 2;  // for checking right number of arguments
                break;

            case 'e':
                if (eflag)
                {
                    cerr << ERR_MULTIPLE_OPT << (char)c << endl << USAGE;
                    return EXIT_FAILURE;
                }
                if (arg_err(c, optarg, scope ))
                {
                    cerr << ERR_ARG_1 << (char)c << ERR_ARG_2 << USAGE;
                    return EXIT_FAILURE;
                }
                eflag++;
                max_argc_val += 2;
                break;

            case 's':
                if (sflag)
                {
                    cerr << ERR_MULTIPLE_OPT << (char)c << endl << USAGE;
                    return EXIT_FAILURE;
                }
                if (optarg == '\0')
                {
                    cerr << ERR_ARG_1 << (char)c << ERR_ARG_2 << USAGE;
                    return EXIT_FAILURE;
                }
                else
                {
                    scope->filename = optarg;
                    scope->static_reserv = !scope->static_reserv;
                }
                eflag++;
                max_argc_val += 2;
                break;

            case '?':
                if (! isprint (optopt)) // if option is not printable character
                    cerr << ERR_CHAR << USAGE;
                else
                    cerr << USAGE;

            default:
                return EXIT_FAILURE;
        }
    }
    if (argc > max_argc_val)
    {// to check for right number of arguments
        cerr << ERR_OPT;
        for (int i = max_argc_val; i < argc; i++)
        {
            cerr << "-- '"<< argv[i] << "' ";
        }
        cerr << endl << USAGE;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

uint32_t strtoip(const char* ip_in)
{// converts c string to uint32_t ip address
    struct in_addr ip_addr;
    return (inet_aton(ip_in, &ip_addr) == 0) ? UINT32_MAX : ip_addr.s_addr;
}

void cut(char* src, size_t from, size_t to, char* dst)
{// cut c string from and to are indexes of start and end of substring
    size_t i;
    for (i = from; i < to ; i++)
    {
        dst[i-from]=src[i];
    }
    dst[i]='\0';
    return;
}
