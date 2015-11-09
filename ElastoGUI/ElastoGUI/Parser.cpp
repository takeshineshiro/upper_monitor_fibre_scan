#include "StdAfx.h"
#include "Parser.h"


Parser::Parser(void)
{
}


Parser::~Parser(void)
{
}


int Parser::DoIt(BYTE **ppFrame, int &nSizeFrame, const BYTE *pBytes, int nBytes)
{
	int ret = -1;
	int nSynHead = sizeof(FrameSynHead) / sizeof(BYTE);
	int nSynTail = sizeof(FrameSynTail) / sizeof(BYTE);

	// 输入的字节应该大于一个包含同步字的有效数据帧的长度
	if (nBytes < (kValidFrameSize + nSynHead + nSynTail)) 
	{
		ret = -1;
		goto exit;
	}

	bool  found_head = false;
	bool  found_tail = false;

	BYTE *p = (BYTE*)pBytes;
	BYTE *pFrame = 0;

	// 搜索同步数据帧头
	for (;;)
	{
		if (memcmp(p, FrameSynHead, nSynHead) == 0)
		{ // 找到，退出循环
			found_head = true;
			pFrame = p + nSynHead;//有效数据的地址
			break;
		}
		else
		{ // 下一个位置
			p++;
		}
	}

	// 下面检查同步数据帧尾
	if (found_head)
	{
		p += kValidFrameSize + nSynHead;
		if (p > (pBytes + nBytes - nSynTail))
		{ // 不能超过输入缓冲区的边界
			ret = -3;
		}
		else
		{
			found_tail = memcmp(p, FrameSynTail, nSynTail) == 0;
			if (found_tail)
			{ //找到帧尾， 返回成功
				*ppFrame = pFrame;
				ret = 0;
				nSizeFrame = kValidFrameSize;
			}
			else
			{
				ret = -4;
			}
		}
	}
	else
	{
		ret = -2;
	}

exit:
	return ret;
}


//////////////////////////////////////////////////////////////////////////
//  假的分析器
//  用于下位机未采用同步字节的情况，只要数据大于有效长度就认为合法
//////////////////////////////////////////////////////////////////////////
int Parser::DoDummy(BYTE **ppFrame, int &nSizeFrame, const BYTE *pBytes, int nBytes)
{
	int ret = (nBytes >= kValidFrameSize) ? 0 : -1;

	if (ret == 0)
	{
		*ppFrame = (BYTE*)pBytes;
		nSizeFrame = kValidFrameSize;
	}

	return ret;
}
