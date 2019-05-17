cmake_minimum_required(VERSION 3.7)
project(mtm_ex2)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic-errors -Werror")

#set(SOURCE_FILES challenge_system.c challenge.c challenge.h constants.h visitor_room.h challenge_room_system_fields.h
#      cmake-build-debug/challenge.c cmake-build-debug/visitor_room.c cmake-build-debug/challenge_system.c)
set(SOURCE_FILES challenge.c challenge.h challenge_room_system_fields.h
        challenge_system.c challenge_system.h challenge_system_test_1.c constants.h
        system_additional_types.h challenge_system_test_1.c
        visitor_room.c visitor_room.h)
add_executable(ex22 ${SOURCE_FILES})