/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2022.2.13
----------------------------------------------------------------*/

/*
*/

#include "audio_player.h"

#include <fstream>
#include <thread>
#include <iostream>

using namespace std;

AudioPlayer* AudioPlayer::getInstance()
{
	static AudioPlayer audioPlayer;
	return &audioPlayer;
}

AudioPlayer::AudioPlayer()
{
}

AudioPlayer::~AudioPlayer()
{
}

int AudioPlayer::playPCM(const char *filePath)
{
	cout << "Call AudioPlayer::playPCM()." << endl;
	thread th(thPlayRoute, filePath);
	th.detach();
	cout << "Call AudioPlayer::playPCM() end." << endl;
	return 0;
}

void *AudioPlayer::playRoute(const void *filePath)
{
	cout << "Call AudioPlayer::play()." << endl;
	if(NULL == filePath)
	{
		cerr << "Fail to call AudioPlayer::playPCM(), argument has null value!" << endl;
		return NULL;
	}

	ifstream ifs((const char *)filePath, ios::in);
	if(ifs.fail())
	{
		cerr << "Fail to open file: " << filePath << endl;
		return NULL;
	}
	cout << "Success to open " << filePath << endl;

	int i = 0;
	while(!ifs.eof())
	{
		int readBytes = 0;
		unsigned int dataBufMaxSize = 2 * 1024;
		char dataBuf[dataBufMaxSize] = {0};
		
		//cout << "Ready to call read() in routeAo()." << endl;
		ifs.read(dataBuf, dataBufMaxSize);
		readBytes = ifs.gcount();
		if(ifs)
		{
			//cout << "all characters read successfully.";
		}
		else
		{
			cout << "error: only " << readBytes << " could be read";
		}

		// for license
		if(++i > 50)
		{
			break;
		}
		// end license

		//cout << "Send pcm stream" << endl;
		AudioOut::getInstance()->sendStream(dataBuf, readBytes);
	}


	ifs.close();

	cout << "Call AudioPlayer::play() end." << endl;
	return NULL;
}

void *AudioPlayer::thPlayRoute(const void *filePath)
{
	AudioPlayer *pThis = (AudioPlayer *)filePath;
	return pThis->playRoute(filePath);
}

