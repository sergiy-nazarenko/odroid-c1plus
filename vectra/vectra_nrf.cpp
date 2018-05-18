#include "vectra.h"

#include <string.h>
#include <unistd.h>

#define NRF_HELP 1
#define NRF_DRAIN  2


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
            mode = NRF_DRAIN;
    }

    std::vector<int> data_nrf;
    data_nrf.reserve(NRF_MEM_ARR_SIZE);
    data_nrf.assign(NRF_MEM_ARR_SIZE, -1);

    if (mode == NRF_DRAIN)
    {
        while(true)
        {
            std::vector<int> data_can = arr_read_can();
            std::vector<int> data_gui = arr_read_gui();

            int can0 = data_can[0];
            if (can0 != -1)
                printf("NRF READ: Data[0] from can == %d\n", can0);

            int gui0 = data_gui[0];
            if (gui0 != -1)
                printf("NRF READ: Data[0] from gui == %d\n", gui0);

            data_nrf[0] = rand() % 800;
            arr_write_nrf(data_nrf);

            usleep(5000);
        }
    }

    return 0;
}

