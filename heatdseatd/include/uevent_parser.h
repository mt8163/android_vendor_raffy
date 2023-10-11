#ifndef ALSADAEMON_UEVEN_PARSER_H
#define ALSADAEMON_UEVEN_PARSER_H

#define UEVENT_MSG_LEN  1024

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

void ResetUevent(Uevent_t *uevent);
void ParseUevent(const char *msg, Uevent_t *uevent);

#endif //ALSADAEMON_UEVEN_PARSER_H
