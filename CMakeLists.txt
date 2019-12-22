cmake_minimum_required(VERSION 3.15)
project(ipc_im C)

set(CMAKE_C_STANDARD 11)

add_executable(ipc_im main.c inf141223_s.h data_structures/linked_list.h inf141223_k.h)