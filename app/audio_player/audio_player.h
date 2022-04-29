/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2022.4.30
----------------------------------------------------------------*/

/*
*/

#pragma once

#include "ao.hpp"

class AudioPlayer{
public:
	static AudioPlayer* getInstance();

	int playPCM(const char *filePath);
	int playWAV(const char *filePath);
	int playMP3(const char *filePath);

private:
	AudioPlayer();
	~AudioPlayer();
	AudioPlayer(const AudioPlayer&);
	AudioPlayer& operator=(const AudioPlayer&);

	void *playRoute(const void *arg);
	static void *thPlayRoute(const void *arg);
};

