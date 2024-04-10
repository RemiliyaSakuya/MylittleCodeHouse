#pragma once

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cassert>
#include <unistd.h>
#include "Log.hpp"
using namespace std;

#define PATH_NAME "/home/HML"
#define PROJ_ID 0x66
#define SHM_SIZE 4096 //共享内存的大小，最好是页(PAGE:4096)的整数倍