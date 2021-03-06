// =====================================================================================
// 
//       Filename:  sniffer.h
//
//    Description:  派生出的数据捕获类的声明头文件
//
//        Version:  1.0
//        Created:  2013年01月24日 14时36分49秒
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
//        Company:  Class 1107 of Computer Science and Technology
// 
// =====================================================================================

#ifndef SNIFFER_H_
#define SNIFFER_H_

#include <vector>

#include "csniffer.h"
#include "sniffertype.h"

class Sniffer : public CSniffer
{
public:
	Sniffer();
	~Sniffer();

	bool getNetDevInfo();		// 构建网络设备的信息结构
	int	 captureOnce();			// 捕获一次网络数据包
	void consolePrint();		// 控制台打印的函数

#ifdef WIN32
	bool OpenSaveCaptureFile(const char *szFileName);
#endif
	std::vector<NetDevInfo>  netDevInfo;
	std::vector<SnifferData> snifferDataVector;
};

#endif	// SNIFFER_H_
