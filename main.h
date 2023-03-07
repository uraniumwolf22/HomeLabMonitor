#ifndef C_GUI_TEST_MAIN_H
#define C_GUI_TEST_MAIN_H

#endif //C_GUI_TEST_MAIN_H



//struct to add further options later?
struct request {
    char *url;
    char *auth;
    char *cookie;
    char* PToken;
    char* user;
    char* pass;
    char* realm;
};

//memory structure
struct memory {         //stores the full response of a JSON url
    char *response;
    size_t size;
};

struct pihole_response{     //structure to store the PiHole response
    char* BlockedRequests;
    char* DnsRequests;
    char* BlockedPercent;
    char* CachedQueries;
};

struct GlobalAuthDataTemplate{
    char* CSRFToken;
    char* cookie;
    char* authorization;
};

