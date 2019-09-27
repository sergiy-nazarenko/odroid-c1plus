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
#define CSR_HOME "\33[H"
#define CLR_SCREEN "\33[2J"

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

//printf("\033[H\033[J");
//printf("\e[2J\e[H"); 
printf("%s%s", CLR_SCREEN, CSR_HOME);
printf("%s", CSR_HOME);
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

        /* convert interface sting "can0" into interface index */ 
        strcpy(ifr.ifr_name, "can0");
        ioctl(s, SIOCGIFINDEX, &ifr);

        /* setup address for bind */
        addr.can_ifindex = ifr.ifr_ifindex;
        addr.can_family = PF_CAN;

        /* bind socket to the can0 interface */
        bind(s, (struct sockaddr *)&addr, sizeof(addr));

        int n = 0;
        while( (n = read(s, &frame, sizeof(frame)) > 0))
        {

            switch (frame.can_id)
            {
              case 0x450:
              { if (frame.data[3]==0x47)
                {
                    data_can[0] = 0; 
                }
                else
                {
                   data_can[0] = 1;
                }
                break;
              }
              case 0x510:
              { int col = frame.data[1]-40;
                //printf("\033[%d;%dHcoolant: %d   ", 10,40,col);
                data_can[1] = col;
                break;            
              }
              case 0x110:
              { int rpm = (frame.data[1] * 256 + frame.data[2])/4;
                //printf("\033[%d;%dHrpm: %d   ",10,80, rpm);
                data_can[2] = rpm;
                break;            
              }
              case 0x128:
              { int rp = (256*frame.data[0] + frame.data[1])/100;
                //printf("\033[%d;%dHfuel:     %d    ",10,80, rp);                
                data_can[3] = rp;
                break;
              }
            }

            
            arr_write_can(data_can);

            usleep(100);
        }



        
    }

    return 0;
}

