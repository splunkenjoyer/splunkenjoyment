/*
        Description: Custom Splunk Universal Forwarder
        Author: SplunkEnjoyer
        Compilation: gcc -static -O3 -Wall forwarder.c -o forwarder
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "network.c" // Sorry redteam, you can't see this

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define CUSTOM_LOGNAME "/opt/splunk/var/log/splunk"
#define BUFFER_SIZE 4096

FILE *open_log(char *, char *);
int send_logs(FILE *);

int main(int argc, char *argv[])
{
        FILE *log = open_log(CUSTOM_LOGNAME, "r");

        send_logs(log);

        fprintf(stdout, "Log successfully sent! Clearing log file to minimize file size.\n");

        fclose(log);
        fclose(open_log(CUSTOM_LOGNAME, "w"));
        return 0;
}

// Opens a handle to custom log and returns it
FILE *open_log(char *log, char *mode)
{
        FILE *ret = fopen(log, mode);
        if (ret == NULL) {
                fprintf(stderr, "Error reading log file: %s with mode: %s\n", log, mode);
                exit(1);
        }

        return ret;
}

// Sends logs to remote server in buffers
int send_logs(FILE *log)
{
        int ret = 0;
        char *buf = calloc(BUFFER_SIZE + 1, sizeof(char));
        
        // Read BUFFER_SIZE of log data and send it to splunk server
        while ((ret = fread(buf, 1, BUFFER_SIZE, log)) == BUFFER_SIZE) {
          if (send_buffered_log(buf) < 0) {
                fprintf(stderr, "Could not send log data to splunk server!\n");
                reestablish_connection();  
          }
          memset(buf, 0, BUFFER_SIZE);
        }
        
        // In case log file isn't BUFFER_SIZE aligned
        if (*buf != 0 && send_last_log(buf, ret) < 0) {
                fprintf(stderr, "Failed to send last log bufer to server!\n");
                reestablish_connection();
        }

        free(buf);
        return 0;
}
