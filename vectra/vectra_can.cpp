#include "vectra.h"

#include <string.h>
#include <unistd.h>

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

    return 0;
}

