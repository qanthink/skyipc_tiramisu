#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

using namespace std;

int convert(int fdSrc, int fdDst);
const char* findChineseEndFlag(const char *buf, const char *endFlag);
const char* findChineseStartFlag(const char *buf, const char *startFlag);


int main(int argc, const char *argv[])
{
	if(3 != argc)
	{
		cerr << "Please useage: ./font_process src.txt dst.txt" << endl;
		return -1;
	}

	const char *srcFilePath = argv[1];
	const char *dstFilePath = argv[2];
	cout << "src: " << srcFilePath << ", dst: " << dstFilePath << endl;

	int fdSrc = -1;
	int fdDst = -1;

	fdSrc = open(srcFilePath, O_RDONLY);
	if(-1 == fdSrc)
	{
		cerr << "Fail to open " << srcFilePath << ". " << strerror(errno) << endl;
		return -1;
	}
	
	fdDst = open(dstFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if(-1 == fdDst)
	{
		cerr << "Fail to open " << dstFilePath << ". " << strerror(errno) << endl;
		close(fdSrc);
		return -1;
	}

	int ret = 0;
	ret = convert(fdSrc, fdDst);

	if(-1 != fdSrc)
	{
		close(fdSrc);
		fdSrc = -1;
	}
	
	if(-1 != fdDst)
	{
		close(fdDst);
		fdDst = -1;
	}

	return -1;
}

int convert(int fdSrc, int fdDst)
{
	if(-1 == fdSrc || -1 == fdDst)
	{
		cerr << "Fail to call convert(). Argument fd is out of range." << endl;
		return -1;
	}

	int ret = 0;
	const unsigned long bufSize = 5000 * (16 * 8 * 5 + 16);		// 最多5000个汉字，每个汉字的点阵数据最多128字节。
	char buf[bufSize] = {0};
	
	ret = read(fdSrc, buf, bufSize);
	if(-1 == ret)
	{
		cerr << "Fail to call read(2) in convert(). " << strerror(errno) << endl;
		return -1;
	}

	cout << "SSIZE_MAX = " << SSIZE_MAX << ", Read bytes = " << ret << endl;
	//cout << buf << endl;
	
	const char *pHead = NULL;
	const char *pChinese = NULL;
	const char *chineseEndFlag = "*/";
	const char *chineseStartFlag = "/*";

	pHead = buf;
	while('\0' != *pHead)
	{
		pChinese = findChineseStartFlag(pHead, chineseStartFlag);
		if(NULL == pChinese)
		{
			cerr << "Fail to call findChineseStartFlag() in convert(). Return value is null." << endl;
			//break;
		}
		//cout << pChinese << endl;

		int ret = 0;
		ret = write(fdDst, pChinese + strlen(chineseStartFlag), 5);
		if(-1 == ret)
		{
			cerr << "Fail to call write(2) in convert(). -2" << strerror(errno) << endl;
			return -2;
		}
		
		ret = write(fdDst, pHead, pChinese - pHead);
		if(-1 == ret)
		{
			cerr << "Fail to call write(2) in convert(). -3" << strerror(errno) << endl;
			return -3;
		}

		ret = write(fdDst, "\n", 1);
		if(-1 == ret)
		{
			cerr << "Fail to call write(2) in convert(). -4" << strerror(errno) << endl;
			return -3;
		}

		pChinese = findChineseEndFlag(pChinese, chineseEndFlag);
		if(NULL == pChinese)
		{
			cerr << "Fail to call findChineseEndFlag() in convert(). Return value is null." << endl;
			//break;
		}

		pHead = (pChinese + strlen(chineseEndFlag) + 2);
		//cout << pHead << endl;
	}

	return 0;
}

const char* findChineseStartFlag(const char *buf, const char *startFlag)
{
	if(NULL == buf || NULL == startFlag)
	{
		cerr << "Fail to call findChinese(). Argument has null value." << endl;
		return NULL;
	}

	const char *move = NULL;
	move = buf;
	while('\0' != *move)
	{
		int ret = 0;
		ret = strncmp(move, startFlag, strlen(startFlag));
		if(0 == ret)
		{
			return move;
		}
	
		move++;
	}

	return NULL;
}

const char* findChineseEndFlag(const char *buf, const char *endFlag)
{
	if(NULL == buf || NULL == endFlag)
	{
		cerr << "Fail to call findChinese(). Argument has null value." << endl;
		return NULL;
	}

	const char *move = NULL;
	move = buf;
	while('\0' != *move)
	{
		int ret = 0;
		ret = strncmp(move, endFlag, strlen(endFlag));
		if(0 == ret)
		{
			return move;
		}
	
		move++;
	}

	return NULL;
}

