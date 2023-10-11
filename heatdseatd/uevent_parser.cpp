#include <uevent.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
    const char *action;
    const char *path;
    const char *subsystem;
    const char *firmware;
    const char *partition_name;
    const char *device_name;
    const char *modalias;
    int partition_num;
    int major;
    int minor;
} Uevent_t;

void ResetUevent(Uevent_t *uevent)
{
    uevent->action = "NULL";
    uevent->path = "";
    uevent->subsystem = "";
    uevent->firmware = "";
    uevent->partition_name = "";
    uevent->device_name = "";
    uevent->modalias = "";
    uevent->partition_num = -1;
    uevent->major = -1;
    uevent->minor = -1;
}

void ParseUevent(const char *msg, Uevent_t *uevent) {
    ResetUevent(uevent);

    while (*msg) {
        if (!strncmp(msg, "ACTION=", 7)) {
            msg += 7;
            uevent->action = msg;
        } else if (!strncmp(msg, "DEVPATH=", 8)) {
            msg += 8;
            uevent->path = msg;
        } else if (!strncmp(msg, "SUBSYSTEM=", 10)) {
            msg += 10;
            uevent->subsystem = msg;
        } else if (!strncmp(msg, "FIRMWARE=", 9)) {
            msg += 9;
            uevent->firmware = msg;
        } else if (!strncmp(msg, "PARTN=", 6)) {
            msg += 6;
            uevent->partition_name = msg;
        } else if (!strncmp(msg, "DEVNAME=", 8)) {
            msg += 8;
            uevent->device_name = msg;
        } else if (!strncmp(msg, "MODALIAS=", 9)) {
            msg += 9;
            uevent->modalias = msg;
        } else if (!strncmp(msg, "PARTN=", 6)) {
            msg += 6;
            uevent->partition_name = msg;
        } else if (!strncmp(msg, "PARTNUM=", 8)) {
            msg += 8;
            uevent->partition_num = atoi(msg);
        } else if (!strncmp(msg, "MAJOR=", 6)) {
            msg += 6;
            uevent->major = atoi(msg);
        } else if (!strncmp(msg, "MINOR=", 6)) {
            msg += 6;
            uevent->minor = atoi(msg);
        }
        /* advance to after the next \0 */
        while (*msg++)
            ;
    }
}
