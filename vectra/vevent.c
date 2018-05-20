#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <syslog.h>

#define EVENT_DEVICE    "/dev/input/event2"
#define EVENT_TYPE      EV_ABS
#define EVENT_CODE_X    ABS_X
#define EVENT_CODE_Y    ABS_Y
// https://stackoverflow.com/questions/45060804/virtual-keyboard-linux-libevdev-sending-event?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
// https://stackoverflow.com/questions/28841139/how-to-get-coordinates-of-touchscreen-rawdata-using-linux?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
/* TODO: Close fd on SIGINT (Ctrl-C), if it's open */

struct pixel
{
    int x;
    int y;
};

int main(int argc, char *argv[])
{
    struct input_event ev;
    openlog(NULL, 0 , LOG_USER);
    int fd;
    char name[256] = "Unknown";
    if(argc <= 1)
    {
        syslog(LOG_NOTICE, "no pid.. exit");
        closelog();
        exit(1);
    }
    int xterm_pid = atoi(argv[1]);
/*
    if ((getuid ()) != 0) {
        fprintf(stderr, "You are not root! This may not work...\n");
        return EXIT_SUCCESS;
    }
*/
    /* Open Device */
    fd = open(EVENT_DEVICE, O_RDONLY);
    if (fd == -1) {
        syslog(LOG_NOTICE, "%s is not a vaild device", EVENT_DEVICE);
        closelog();
        return EXIT_FAILURE;
    }
    
    /* Print Device Name */
    ioctl(fd, EVIOCGNAME(sizeof(name)), name);
    syslog(LOG_NOTICE, "Reading from: device file = %s device name = %s", EVENT_DEVICE,  name);
    struct pixel xy;
    xy.x = -1;
    xy.y = -1;

    for (;;) 
    {
        const size_t ev_size = sizeof(struct input_event);
        ssize_t size;

        /* TODO: use select() */

        size = read(fd, &ev, ev_size);
        if (size < ev_size) {
            fprintf(stderr, "Error size when reading\n");
            goto err;
        }

        if (ev.type == EVENT_TYPE)
        { 
            if (ev.code == EVENT_CODE_X)
            {
                    xy.x = ev.value;
                    xy.y = -1;
            }
            if (ev.code == EVENT_CODE_Y) 
                    xy.y = ev.value;
        }
        
        if (xy.x != -1 && xy.y != -1) 
        {
            syslog(LOG_NOTICE, "pid %u x=%d, y=%d", xterm_pid, xy.x, xy.y);
            if(xy.x > 700 && xy.y < 100)
            {
                kill(xterm_pid, SIGKILL);
                return EXIT_SUCCESS;
            }
            xy.x = -1;
            xy.y = -1;
        }
        
    }
    closelog();
    return EXIT_SUCCESS;

err:
    close(fd);
    return EXIT_FAILURE;
}
