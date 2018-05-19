#include "vectra.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/can.h>

#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>


#define CAN_HELP 1
#define CAN_DRAIN  2


void usage(const char * s) {
    printf("Usage: %s <drain>\n", s);
}


int main (int argc, char ** argv) 
{
    int mode = 0;
    if ( argc > 1 ) 
    {
        if (!strcmp(argv[1], "help")) 
        {
            usage(argv[0]);
            return 1;
        }
        if (!strcmp(argv[1], "drain"))
            mode = CAN_DRAIN;
    }



    std::vector<int> data_can;
    data_can.reserve(CAN_MEM_ARR_SIZE);
    data_can.assign(CAN_MEM_ARR_SIZE, -1);

    if (mode == CAN_DRAIN)
    {
        while(true)
        {
            std::vector<int> data_nrf = arr_read_nrf();
            std::vector<int> data_gui = arr_read_gui();

            int nrf0 = data_nrf[0];
            if (nrf0 != -1)
                printf("CAN READ: Data[0] from nrf == %d\n", nrf0);

            int gui0 = data_gui[0];
            if (gui0 != -1)
                printf("CAN READ: Data[0] from gui == %d\n", gui0);

            data_can[0] = 0;
            data_can[1] = rand() % 800;
            arr_write_can(data_can);

            usleep(5000);
        }
    }
    else
    {
        struct ifreq ifr;
        struct sockaddr_can addr;
     
        struct can_frame frame;
        int s;

        memset(&ifr, 0x0, sizeof(ifr));
        memset(&addr, 0x0, sizeof(addr));
        memset(&frame, 0x0, sizeof(frame));

        /* open CAN_RAW socket */
        s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

        /* convert interface sting "can0" into interface index */ strcpy(ifr.ifr_name, "vcan0");
        ioctl(s, SIOCGIFINDEX, &ifr);

        /* setup address for bind */
        addr.can_ifindex = ifr.ifr_ifindex;
        addr.can_family = PF_CAN;

        /* bind socket to the can0 interface */
        bind(s, (struct sockaddr *)&addr, sizeof(addr));

        int n = 0;
        while( (n = read(s, &frame, sizeof(frame)) > 0))
        {

            if (frame.can_id == 0x450){
              if (frame.data[3]==0x47){
                    data_can[0] = 0; 
                }
                else
                {
                   data_can[0] = 1;
                }
            }
            
            arr_write_can(data_can);

            usleep(2000);
        }



        
    }

    return 0;
}

