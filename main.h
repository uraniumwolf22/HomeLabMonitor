#ifndef C_GUI_TEST_MAIN_H
#define C_GUI_TEST_MAIN_H

#endif //C_GUI_TEST_MAIN_H



//struct to add further options later?
struct request {        //store the initial request data.  Its redundant I know, but I have a feeling I will need more functionality.  if you disagree fuck you <3
    char *url;
};

//memory structure
struct memory {         //stores the full response of a JSON url
    char *response;
    size_t size;
};

struct pihole_response{     //structure to store the PiHole response
    char* blocked_requests;
    char* DNS_requests;
};


