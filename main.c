#include <stdio.h>
#include <gtk/gtk.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <json-c/json.h>
#include "main.h"
//stores the PiHole URL and auth key
static char* pihole_URL = "http://pihole.int/admin/api.php?summary&auth=5e3b902de71e0bd612499f76795d15d7905ea45c0545f55c9f897b60790508af";
static int UpdateFreqFast = 1;  //Fast update frequency in seconds
static int UpdateFreqSlow = 5;  //Slow update frequency in seconds

struct pihole_response PiHoleResObj; //set a global to store the most recent PiHole response
//curl ResponseData callback function
static size_t cb(void *ResponseData, size_t ObjSize, size_t NumMemObjects, void *ClientData)
{
    size_t realsize = ObjSize * NumMemObjects;                             //Calculate the ObjSize of the ResponseData
    struct memory *mem = (struct memory *)ClientData;              //Initialize memory struct

    char *ptr = realloc(mem->response, mem->size + realsize + 1);   //Reallocate the ObjSize of the response object

    if(ptr == NULL)                             //oh. shit
        return 0;                               //Really.  that's not good.

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), ResponseData, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

char*  GET_RESPONSE(struct request req_)
{
    struct memory chunk = {0}; //define the structure containing memory chunks (url data)

    //setup curl env
    CURL *curl;
    CURLcode res;               //Curl return code

    free(chunk.response);   //free memory, so we don't fucking die (I'm looking at you malloc)

    curl = curl_easy_init();
    if(curl) { //if curl initialized
        curl_easy_setopt(curl, CURLOPT_URL, req_.url);              //set the data url
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);         //set to follow 1 redirect
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);          //function to call on chunk being received
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);  //call function to write data

        res = curl_easy_perform(curl);                              //perform the request and return the return code
        if(res != CURLE_OK) {                                       //basic error checking
            fprintf(stderr, "curl_easy_perform() failed: %sn",
                    curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl); //cleanup curl WHY C WHY
    }
    return chunk.response;      //return the response
}

//update the global PiHole data structure with new data from API
static void update_responses(void){
    struct request request_std;         //Create an instance of a standard request structure
    request_std.url = pihole_URL;       //Set the request structure to the PiHole URL

    //get data from pihole
    json_object *root = json_tokener_parse(GET_RESPONSE(request_std));                                                  //Pull JSON data from the PiHole API
    PiHoleResObj.DNS_requests = json_object_get_string(json_object_object_get(root, "dns_queries_today"));       //Parse the total queries out of the json
    PiHoleResObj.blocked_requests = json_object_get_string(json_object_object_get(root, "ads_blocked_today"));   //Parse the total blocked queries out of the json
    PiHoleResObj.BlockedPercent = json_object_get_string(json_object_object_get(root, "ads_percentage_today"));   //Parse the percent of blocked queries today
}


//Update the total queries widget
static int UpdateDnsQueries(GtkWidget *text){
    gtk_label_set_label(GTK_LABEL(text), PiHoleResObj.DNS_requests);      //Set the label based on the global struct
    return TRUE;
}
//Update the blocked queries widget
static int UpdateBlockedQueries(GtkWidget *text){
    gtk_label_set_label(GTK_LABEL(text), PiHoleResObj.blocked_requests);  //Set the label based on the global struct
    return TRUE;
}

static int UpdateBlockedPercent(GtkWidget *text){
    gtk_label_set_label(GTK_LABEL(text), PiHoleResObj.BlockedPercent);
    return TRUE;
}

//initiate window
static void activate (GtkApplication* app, gpointer user_data){
    //Window Settings
    GtkWidget *window = gtk_application_window_new (app);                       //create a new window
    gtk_window_set_title (GTK_WINDOW (window), "Datacenter Control");        //Set the name of the window
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 300);     //Set the default window size

    //Default Window Box
    GtkWidget *DefaultWindowBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);         //set the DefaultWindowBox orientation to vertical
    gtk_widget_set_halign (DefaultWindowBox, GTK_ALIGN_START);                            //Align the Horizontal Axis to center
    gtk_widget_set_valign (DefaultWindowBox, GTK_ALIGN_START);                            //Alight the Vertical Axis to center
    gtk_window_set_child(GTK_WINDOW(window), DefaultWindowBox);                           //Set the bounding DefaultWindowBox as the child of the window


    //create frame to hold all PiHole data
    GtkWidget *PiHoleFrame = gtk_frame_new("PiHole Data");                          //Create the frame to hold the pihole metrics
    gtk_box_append(GTK_BOX(DefaultWindowBox), PiHoleFrame);                     //Add the PiHole data frame to the main DefaultWindowBox

    //Create a pihole specific data DefaultWindowBox
    GtkWidget *PiHoleBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);   //creates a new DefaultWindowBox to contain pihole metrics
    gtk_frame_set_child(GTK_FRAME(PiHoleFrame),PiHoleBox);                    //Sets the DefaultWindowBox to the child frame

    //initialize DNS query frame
    GtkWidget *DNSQFrame = gtk_frame_new("DNS Queries");                            //Define the DNS Queries frame
    gtk_box_append(GTK_BOX(PiHoleBox),DNSQFrame);                               //Set the DNSQ frame as a child of the DefaultWindowBox
    //Initialize query counter
    GtkWidget *DNSQ = gtk_label_new("INITIALIZING");                                  //text DefaultWindowBox to contain the total DNS queries
    gtk_frame_set_child(GTK_FRAME(DNSQFrame),DNSQ);                           //set text as child of frame

    //initialize blocked query frame
    GtkWidget *BlockedQFrame = gtk_frame_new("Blocked Queries");                    //Create a frame to contain the PiHole data
    gtk_box_append(GTK_BOX(PiHoleBox),BlockedQFrame);                          //Set the frame as a child of the DefaultWindowBox
    //Initialize blocked query counter
    GtkWidget *BlockedQ = gtk_label_new("INITIALIZING");                             //Define the updatable blocked queries text
    gtk_frame_set_child(GTK_FRAME(BlockedQFrame), BlockedQ);                  //Set the Blocked queries as the child of the frame

    //initialize Blocked Percentage Frame
    GtkWidget *BlockedPercentFrame = gtk_frame_new("Percent Of Queries Blocked");  //Create a frame to contain the Widget
    gtk_box_append(GTK_BOX(PiHoleBox),BlockedPercentFrame);                    //Append widget to the pihole data DefaultWindowBox
    //Initialize Blocked Percent Counter
    GtkWidget *BlockerPercent = gtk_label_new("INITIALIZING");                       //Create label to hold the Percent blocked
    gtk_frame_set_child(GTK_FRAME(BlockedPercentFrame),BlockerPercent);       //Set the label as a child of the frame


    //Add async functions
    g_timeout_add_seconds(UpdateFreqFast, G_SOURCE_FUNC(update_responses),NULL);                     //Update the global PiHole response struct
    g_timeout_add_seconds(UpdateFreqFast, G_SOURCE_FUNC(UpdateDnsQueries), DNSQ);                    //Update the Total DNS Query Counter
    g_timeout_add_seconds(UpdateFreqFast, G_SOURCE_FUNC(UpdateBlockedQueries), BlockedQ);            //Update the Total Blocked Query Counter
    g_timeout_add_seconds(UpdateFreqSlow, G_SOURCE_FUNC(UpdateBlockedPercent),BlockerPercent);       //Update the Percent of blocked queries

    //display main window
    gtk_widget_show(window);
}

int main (int argc, char **argv){

    GtkApplication *app;                        //Create a GTKApplication instance
    int status;                                 //var to store application status

    app = gtk_application_new ("gtk.test", G_APPLICATION_FLAGS_NONE);       //set the application ID and the application flags
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);                         //call the activate function on init
    status = g_application_run (G_APPLICATION (app), argc, argv);                  //update status
    g_object_unref (app);                                                             //dereference app object

    return status;  //if you can't figure out what this does you're hopeless.
}