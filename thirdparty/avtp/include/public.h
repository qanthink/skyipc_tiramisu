#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <functional>
#include <memory.h>
#include <assert.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/un.h>
#include <sys/timeb.h>
#include <dlfcn.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdint.h>
#include <algorithm>
#include <sys/prctl.h>
#include <sys/select.h>
#include <atomic>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

typedef void (*Recv_cb)(void* data, int len);

unsigned long long getTimeStamp();