#include <stdio.h>
#include <gtk/gtk.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <json-c/json.h>


//stores the PiHole URL and auth key
char* pihole_URL = "http://pihole.int/admin/api.php?summary&auth=5e3b902de71e0bd612499f76795d15d7905ea45c0545f55c9f897b60790508af";

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

struct pihole_response piholeResponse_; //set a global to store the most recent PiHole response

//curl data callback function
static size_t cb(void *data, size_t size, size_t nmemb, void *clientp)
{
    size_t realsize = size * nmemb;                             //Calculate the size of the data
    struct memory *mem = (struct memory *)clientp;              //Initialize memory struct

    char *ptr = realloc(mem->response, mem->size + realsize + 1);   //Reallocate the size of the response object

    if(ptr == NULL)                             //oh. shit
        return 0;  /* out of memory! */         //Really.  that's not good.

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
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
void update_responses(void){
    struct request request_std;         //Create an instance of a standard request structure
    request_std.url = pihole_URL;       //Set the request structure to the PiHole URL

    //get data from pihole
    json_object *root = json_tokener_parse(GET_RESPONSE(request_std));                                                  //Pull JSON data from the PiHole API
    piholeResponse_.DNS_requests = json_object_get_string(json_object_object_get(root, "dns_queries_today"));       //Parse the total queries out of the json
    piholeResponse_.blocked_requests = json_object_get_string(json_object_object_get(root, "ads_blocked_today"));   //Parse the total blocked queries out of the json
}


//Update the total queries widget
int update_DNSQ(GtkWidget *text){
    gtk_label_set_label(GTK_LABEL(text),piholeResponse_.DNS_requests);      //Set the label based on the global struct
    return TRUE;
}
//Update the blocked queries widget
int update_BLOCKEDQ(GtkWidget *text){
    gtk_label_set_label(GTK_LABEL(text),piholeResponse_.blocked_requests);  //Set the label based on the global struct
    return TRUE;
}

//initiate window
static void activate (GtkApplication* app, gpointer user_data){
    //Window Settings
    GtkWidget *window = gtk_application_window_new (app);                   //create a new window
    gtk_window_set_title (GTK_WINDOW (window), "Datacenter Control");    //Set the name of the window
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 300); //Set the default window size

    //Default widget alignments
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL,5);   //set the box orientation to vertical
    gtk_widget_set_halign (box, GTK_ALIGN_CENTER);                   //Align the Horizontal Axis to center
    gtk_widget_set_valign (box, GTK_ALIGN_CENTER);                   //Alight the Vertical Axis to center
    gtk_window_set_child(GTK_WINDOW(window),box);                    //Set the bounding box as the child of the window

    //Initialize query counter
    GtkWidget *DNSQ = gtk_label_new("INITIALIZING");                    //Define updatable text
    GtkWidget *DNS_label = gtk_label_new("DNS Queries Today");          //Define label for Total DNS Queries
    gtk_box_append(GTK_BOX(box), DNSQ);                           //Append the labels to the bounding box
    gtk_box_append(GTK_BOX(box),DNS_label);                       //

    //Initialize blocked query counter
    GtkWidget *BLOCKEDQ = gtk_label_new("INITIALIZING");                //Define updatable text
    GtkWidget *BLOCKEDQ_label = gtk_label_new("Blocked Queries Today"); //Define label for Blocked queries
    gtk_box_append(GTK_BOX(box),BLOCKEDQ);                        //Append the labels to the bounding box
    gtk_box_append(GTK_BOX(box), BLOCKEDQ_label);                 //


    //Add async functions
    g_timeout_add_seconds(1, G_SOURCE_FUNC(update_responses),NULL);         //Update the global PiHole response struct
    g_timeout_add_seconds(1, G_SOURCE_FUNC(update_DNSQ), DNSQ);             //Update the Total DNS Query Counter
    g_timeout_add_seconds(1, G_SOURCE_FUNC(update_BLOCKEDQ), BLOCKEDQ);     //Update the Total Blocked Query Counter

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