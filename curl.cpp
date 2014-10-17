// Curl connection code
//
// K Labe September 23 2014

#include "curl.h"
#include "curl/curl.h"
#include <cstring>
#include <stdlib.h>

static CURL* curl; // curl connection object
static const int max[5] = {5, 3, 2, 5, 1}; // maximum number of curl messages allowed per second
static int alarmn[5]   = {0, 0, 0, 0, 0}; // number of curl messages in last second
static int overflow[5] = {0, 0, 0, 0, 0}; // number of overfow messages
static int oldwalltime = 0;

// This function return alarm_type from tony's log number
alarm_type type(const int level){
  if(level == 20)
    return INFO;
  if(level == 21)
    return SUCCESS;
  if(level == 30)
    return WARNING;
  if(level == 40)
    return ERROR;
  else
    return DEBUG;
}

// This function sends alarms to the monitoring website
void alarm(const int level, const char* msg){
  int walltime = time(NULL);
  if(walltime != oldwalltime){
    int overflowsum = 0;
    for(int i=0; i<5; i++){
      overflowsum += overflow[i];
      overflow[i] = 0;
      alarmn[i] = 0;
    }
    if(overflowsum){
      char mssg[128];
      sprintf(mssg, "ERROR OVERFLOW: %d messages skipped", overflowsum);
      char curlmsg[256];
      sprintf(curlmsg, "name=L2-client&level=40&message=%s", mssg);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curlmsg);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(curlmsg));
      CURLcode res = curl_easy_perform(curl);
      if(res != CURLE_OK)
        fprintf(stderr, "Logging failed: %s\n", curl_easy_strerror(res));
    }
    oldwalltime = walltime;
  }
  alarmn[type(level)]++;
  if(alarmn[type(level)] > max[type(level)]) 
    overflow[type(level)]++;
  else{
    char curlmsg[256];
    sprintf(curlmsg, "name=L2-client&level=%d&message=%s", level, msg);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curlmsg);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(curlmsg));
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "Logging failed: %s\n", curl_easy_strerror(res));
  }
}

// This function opens a curl connection
void Opencurl(char* password){
  curl = curl_easy_init();
  char address[264];
  sprintf(address, "http://snoplus:%s@snopl.us/monitoring/log", password);
  if(curl){
//  curl_easy_setopt(curl, CURLOPT_URL, address);
    curl_easy_setopt(curl, CURLOPT_URL, "http://cp4.uchicago.edu:50000/monitoring/log");
  }
  else{
    fprintf(stderr, "Could not initialize curl object");
    exit(1);
  }
}

// This function closes a curl connection
void Closecurl(){
  curl_easy_cleanup(curl);
}
