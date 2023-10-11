#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <uevent.h>

#include <sys/types.h>

#include <string>
#include <iostream>

#define LOG_TAG "headsetd"
#include <android-base/logging.h>
#include <android-base/file.h>

#include <uevent_parser.h>
#include <tinyalsa/asoundlib.h>

#define H2W_PATH "/sys/class/switch/h2w/state"

#define EXT_SPEAKER_SWITCH_CTRL_NAME      "Ext_Speaker_Amp_Switch"
#define EXT_HEADPHONE_SWITCH_CTRL_NAME    "Ext_Headphone_Amp_Switch"
#define AUDIO_I2S0DL1_HD_SWITCH_CTRL_NAME "Audio_I2S0dl1_hd_Switch"

int get_headset_state() {
    std::string state;
    if (!android::base::ReadFileToString(H2W_PATH, &state)) {
        LOG(ERROR) << "Failed to read headset state!";
        abort();
    }
    return std::stoi(state);
}

int set_alsa_control_value(const char *name, int value) {
    struct mixer *mixer;
    struct mixer_ctl *ctl;
    int ret = 0;

    mixer = mixer_open(0);
    if (!mixer) {
        LOG(ERROR) << "Failed to open mixer!";
        return -1;
    }

    ctl = mixer_get_ctl_by_name(mixer, name);
    if (!ctl) {
        LOG(ERROR) << "Failed to get ctl for " << name;
        ret = -1;
        goto out;
    }

    if (mixer_ctl_set_value(ctl, 0, value)) {
        LOG(ERROR) << "Failed to set value for " << name;
        ret = -1;
        goto out;
    }

out:
    mixer_close(mixer);
    return ret;
}

int get_alsa_control_value(const char *name) {
    struct mixer *mixer;
    struct mixer_ctl *ctl;
    int ret = 0;

    mixer = mixer_open(0);
    if (!mixer) {
        LOG(ERROR) << "Failed to open mixer!";
        return -1;
    }

    ctl = mixer_get_ctl_by_name(mixer, name);
    if (!ctl) {
        LOG(ERROR) << "Failed to get ctl for " << name;
        ret = -1;
        goto out;
    }

    ret = mixer_ctl_get_value(ctl, 0);
out:
    mixer_close(mixer);
    return ret;
}

int update_headset_state() {
    int ret = 0;

    if (get_headset_state() > 0) {
        ret = set_alsa_control_value(EXT_SPEAKER_SWITCH_CTRL_NAME, 0);
        if (ret) {
            LOG(ERROR) << "Failed to set " << EXT_SPEAKER_SWITCH_CTRL_NAME << " to 0";
            return ret;
        }

        ret = set_alsa_control_value(EXT_HEADPHONE_SWITCH_CTRL_NAME, 1);
        if (ret) {
            LOG(ERROR) << "Failed to set " << EXT_HEADPHONE_SWITCH_CTRL_NAME << " to 1";
            return ret;
        }
    } else {
        ret = set_alsa_control_value(EXT_SPEAKER_SWITCH_CTRL_NAME, 1);
        if (ret) {
            LOG(ERROR) << "Failed to set " << EXT_SPEAKER_SWITCH_CTRL_NAME << " to 1";
            return ret;
        }

        ret = set_alsa_control_value(EXT_HEADPHONE_SWITCH_CTRL_NAME, 0);
        if (ret) {
            LOG(ERROR) << "Failed to set " << EXT_HEADPHONE_SWITCH_CTRL_NAME << " to 0";
            return ret;
        }
    }

    return ret;
}

int main() {
    size_t s;
    Uevent_t evt;
    int ufd, ret, nr;
    struct pollfd pfd;
    char buf[UEVENT_MSG_LEN + 2];

    LOG(INFO) << "Starting headsetd as " << getuid() << ":" << getgid();

    ufd = uevent_open_socket(UEVENT_MSG_LEN, true);
    if (ufd < 0) {
        LOG(ERROR) << "Failed to open uevent socket!";
        return -1;
    }

    ret = set_alsa_control_value("VOLKEY_SWITCH", 0);
    if (ret) {
        LOG(ERROR) << "Failed to set Speaker Function to 0";
        return ret;
    }

    pfd.fd = ufd;
    pfd.events = POLLIN;

    while (true) {
        nr = poll(&pfd, 1, -1);
        if (nr <= 0) {
            LOG(ERROR) << "Poll failed!";
            return -1;
        }

        if (pfd.revents & POLLIN) {
            s = uevent_kernel_multicast_recv(ufd, (void *)buf, UEVENT_MSG_LEN);
            if (s <= 0) {
                LOG(ERROR) << "Failed to read uevent message!";
                continue;
            }

            ParseUevent(buf, &evt);

            if ((strcmp(evt.action, "change") == 0) && (strcmp(evt.path, "/devices/virtual/switch/h2w") == 0)) {
                if (get_alsa_control_value(AUDIO_I2S0DL1_HD_SWITCH_CTRL_NAME) == 1) {
                    ret = update_headset_state();
                    if (ret) {
                        LOG(ERROR) << "Failed to update headset state!";
                        return ret;
                    }
                }
            }
        }
    }

    LOG(ERROR) << "Something went horribly wrong, killing myself!";
    kill(getpid(), SIGKILL);

    // Should never reach here
    return ret;
}
