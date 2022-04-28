#include "../avtp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include "unistd.h"

using namespace std;

int main(int argc, const char *argv[])
{
	AvtpVideoServer avtpServer("192.168.0.7", "192.168.0.100");

	int i = 0;
	for(i = 0; i < 9999999; ++i)
	{
		usleep(100);
		int realSize = 0;
		const unsigned int bufSize = 128 * 1024;
		unsigned char buf[bufSize] = { 0 };
		realSize = avtpServer.recvVideoFrame(buf, bufSize);
		cout << "realSize = " << realSize << endl;
	}

	return 0;
}


