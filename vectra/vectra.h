#pragma once

#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#define GUI_SEM_STR_NAME "gui_str_semafore"
#define GUI_MEM_STR_NAME "gui_str_memory"
#define GUI_MEM_STR_SIZE 50
#define GUI_SEM_ARR_NAME "gui_arr_semafore"
#define GUI_MEM_ARR_NAME "gui_arr_memory"
#define GUI_MEM_ARR_SIZE 50

#define CAN_SEM_STR_NAME "can_str_semafore"
#define CAN_MEM_STR_NAME "can_str_memory"
#define CAN_MEM_STR_SIZE 50
#define CAN_SEM_ARR_NAME "can_arr_semafore"
#define CAN_MEM_ARR_NAME "can_arr_memory"
#define CAN_MEM_ARR_SIZE 50

#define NRF_SEM_STR_NAME "nrf_str_semafore"
#define NRF_MEM_STR_NAME "nrf_str_memory"
#define NRF_MEM_STR_SIZE 50
#define NRF_SEM_ARR_NAME "nrf_arr_semafore"
#define NRF_MEM_ARR_NAME "nrf_arr_memory"
#define NRF_MEM_ARR_SIZE 50


int pipe_str_write (const std::string&, const std::string&, int, const char*);
int pipe_arr_write (const std::string&, const std::vector<int>&, int, const char*);
int pipe_arr_read(std::vector<int>&, const std::string&, int, const char*);
int pipe_str_read(std::string&,const std::string&, int, const char*);

std::vector<int> arr_read_can();
std::vector<int> arr_read_gui();
std::vector<int> arr_read_nrf();

int arr_write_can(const std::vector<int>&);
int arr_write_gui(const std::vector<int>&);
int arr_write_nrf(const std::vector<int>&);




