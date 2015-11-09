#pragma once

//////////////////////////////////////////////////////////////////////////
// 数据帧分析器
//
//////////////////////////////////////////////////////////////////////////

const  BYTE  FrameSynHead[] = {0xff, 0x5b, 0x50, 0x00, 0xff, 0x5d, 0x50, 0x00}; // 数据帧-帧头同步字
const  BYTE  FrameSynTail[] = {0xff, 0xff, 0x84, 0x13, 0xff, 0xff, 0x14, 0x21}; // 数据帧-帧尾同步字

const  int   kValidFrameSize = 4096 * 300 * 2; // 数据帧有效字节的size
const  int   kTotalFrameSize = kValidFrameSize + sizeof(FrameSynHead) / sizeof(BYTE) + sizeof(FrameSynTail) / sizeof(BYTE);

class Parser
{
public:
	Parser(void);
	virtual ~Parser(void);

	//////////////////////////////////////////////////////////////////////////
	//  进行分析；
	//  输入：
	//      pBytes,  输入的字节流
	//      nBytes,  数据字节的数量
	//  输出：
	//      pFrame,       去除了帧头和帧尾同步自以后的有效数据地址
	//      nSizeFrame,   数据帧缓冲区的大小，字节单位； 处理以后里面放置的是解析后数据帧的大小
	//  返回：
	//     0， 表示解析成功；
	//     非0， 解析失败；
	//        -1， 输入数据的长度不够解析一个数据帧
	//        -2， 找不到帧头
	//        -3， 数据帧不完整（帧头找到了）
	//        -4， 找不到帧尾（帧头找到了）
	//////////////////////////////////////////////////////////////////////////
	static int   DoIt(BYTE **ppFrame, int &nSizeFrame, const BYTE *pBytes, int nBytes);

	//////////////////////////////////////////////////////////////////////////
    //  假的分析器
	//  用于下位机未采用同步字节的情况，只要数据大于有效长度就认为合法
	//////////////////////////////////////////////////////////////////////////
	static int   DoDummy(BYTE **ppFrame, int &nSizeFrame, const BYTE *pBytes, int nBytes);

protected:


private:
};

