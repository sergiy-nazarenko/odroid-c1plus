#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "vectra.h"


std::string 
str_read_nrf() 
{
    std::string ret;
    pipe_str_read(ret, NRF_MEM_STR_NAME, NRF_MEM_STR_SIZE, NRF_SEM_STR_NAME);
    return ret;
}


std::string 
str_read_can() 
{
    std::string ret;
    pipe_str_read(ret, CAN_MEM_STR_NAME, CAN_MEM_STR_SIZE, CAN_SEM_STR_NAME);
    return ret;
}


std::vector<int> 
array_read_gui()
{
    std::vector<int> ret;
    pipe_arr_read(ret, GUI_MEM_ARR_NAME, GUI_MEM_ARR_SIZE, GUI_SEM_ARR_NAME);
    return ret;
}

std::string 
str_read_gui() 
{
    std::string ret;
    pipe_str_read(ret, GUI_MEM_STR_NAME, GUI_MEM_STR_SIZE, GUI_SEM_STR_NAME);
    return ret;
}

int 
array_write_gui (const std::vector<int>& array)
{
    return pipe_arr_write(GUI_MEM_ARR_NAME, array, GUI_MEM_ARR_SIZE, GUI_SEM_ARR_NAME);
}

int 
str_write_gui (const std::string& data)
{
    return pipe_str_write(GUI_MEM_STR_NAME, data, GUI_MEM_STR_SIZE, GUI_SEM_STR_NAME);
}

PYBIND11_MODULE(vectra_gui, m) 
{
    m.doc() = "gui pipe binding plugin"; 

    m.def("array_write_gui", &array_write_gui, "Write array gui pipe");
    m.def("str_write_gui", &str_write_gui, "Write string gui pipe");
    
    m.def("str_read_gui", &str_read_gui, "Read string from pipe");
    m.def("array_read_gui", &array_read_gui, "Read int array from pipe");

    m.def("str_read_nrf", &str_read_nrf, "Read string from pipe");
    m.def("array_read_nrf", &arr_read_nrf, "Read int array from pipe");

    m.def("str_read_can", &str_read_can, "Read string from can pipe");
    m.def("array_read_can", &arr_read_can, "Read int array from can pipe");
}
