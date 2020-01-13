cmake_minimum_required(VERSION 3.15)
project(ipc_im C)
find_package( Threads )
set(CMAKE_C_STANDARD 11)

add_executable(ipc_im inf141223_s.h data_structures/linked_list.h inf141223_s.c)
target_link_libraries( ipc_im ${CMAKE_THREAD_LIBS_INIT} )