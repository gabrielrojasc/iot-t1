cmake_minimum_required(VERSION 3.26)
set(tcp_client_ip tcp_client_v4.c)

idf_component_register(SRCS "main.c" "${tcp_client_ip}"
                                INCLUDE_DIRS "."
                                REQUIRES ${requires})
