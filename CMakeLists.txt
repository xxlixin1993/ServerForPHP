#1.cmake verson
cmake_minimum_required(VERSION 3.0)

#2.project name
PROJECT(ServerForPHP)

#3.head file path
INCLUDE_DIRECTORIES(
        include
)

#4.source directory
AUX_SOURCE_DIRECTORY(src DIR_SRCS)

#5.add executable file
ADD_EXECUTABLE(${PROJECT_NAME} ${DIR_SRCS})
