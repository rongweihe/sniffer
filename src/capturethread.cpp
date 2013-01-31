// =====================================================================================
// 
//       Filename:  capturethread.cpp
//
//    Description:  后台捕获数据的多线程类实现文件
//
//        Version:  1.0
//        Created:  2013年01月24日 21时44分49秒
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
//        Company:  Class 1107 of Computer Science and Technology
// 
// =====================================================================================

#include "../include/capturethread.h"
#include "../include/listtreeview.h"
#include "../include/sniffertype.h"
#include "../include/Sniffer.h"

#ifdef WIN32
	#pragma warning(disable:4996)
#endif

CaptureThread::CaptureThread()
{
	bStopped = false;
	sniffer  = NULL;
	mainTree = NULL;
}

CaptureThread::CaptureThread(ListTreeView *pTree, Sniffer *pSniffer, QString tmpFileName)
{
	bStopped = false;

	mainTree = pTree;
	sniffer  = pSniffer;
	tmpFile  = tmpFileName;
}

void CaptureThread::run()
{
	int res;
	struct tm *ltime;
	char szNum[10];
	char szLength[6];
	char timestr[16];
	time_t local_tv_sec;

	int num = 1;
	SnifferData tmpSnifferData;

	if (!tmpFile.isEmpty()) {
		sniffer->openDumpFile((const char *)tmpFile.toLocal8Bit());
	}

	// 清理遗留数据
	sniffer->snifferDataVector.clear();

	while (bStopped != true && (res = sniffer->captureOnce()) >= 0) {
		if (res == 0) {
			continue;
		}

		sniffer->saveCaptureData();

		tmpSnifferData.protoInfo.init();

		tmpSnifferData.strData.setRawData((const char *)sniffer->pkt_data, sniffer->header->caplen);

		tmpSnifferData.strData = "原始捕获数据：" + tmpSnifferData.strData.toHex().toUpper();

		sprintf(szNum, "%d", num);
		tmpSnifferData.strNum = szNum;
		num++;

		local_tv_sec = sniffer->header->ts.tv_sec;
		ltime = localtime(&local_tv_sec);
		strftime(timestr, sizeof(timestr), "%H:%M:%S", ltime);

		tmpSnifferData.strTime = timestr;

		sprintf(szLength, "%d", sniffer->header->len);
		tmpSnifferData.strLength = szLength;

		eth_header		*eh;
		ip_header		*ih;
		udp_header		*uh;
		tcp_header		*th;
		unsigned short	 sport, dport;
		unsigned int	 ip_len;

		// 获得 Mac 头
		eh = (eth_header *)sniffer->pkt_data;

		QByteArray DMac, SMac;

		DMac.setRawData((const char *)eh->dstmac, 6);
		SMac.setRawData((const char *)eh->srcmac, 6);
		DMac = DMac.toHex().toUpper();
		SMac = SMac.toHex().toUpper();
 
		tmpSnifferData.protoInfo.strDMac = tmpSnifferData.protoInfo.strDMac
									 + DMac[0] + DMac[1] + "-" + DMac[2] + DMac[3] + "-" + DMac[4]  + DMac[5] + "-"
	 								 + DMac[6] + DMac[7] + "-" + DMac[8] + DMac[9] + "-" + DMac[10] + DMac[11] ;
		tmpSnifferData.protoInfo.strSMac = tmpSnifferData.protoInfo.strSMac
									 + SMac[0] + DMac[1] + "-" + SMac[2] + DMac[3] + "-" + SMac[4]  + DMac[5] + "-"
									 + SMac[6] + DMac[7] + "-" + SMac[8] + DMac[9] + "-" + SMac[10] + DMac[11] ;
		// 获得 IP 协议头
		ih = (ip_header *)(sniffer->pkt_data + 14);

		// 获得 IP 头的大小
		ip_len = (ih->ver_ihl & 0xF) * 4;

		switch (ih->proto) {
			case TCP_SIG:
				tmpSnifferData.strProto = "TCP";
				tmpSnifferData.protoInfo.strTranProto += "TCP 协议 (Transmission Control Protocol)";
				th = (tcp_header *)((unsigned char *)ih + ip_len);		// 获得 TCP 协议头
				sport = ntohs(th->sport);								// 获得源端口和目的端口
				dport = ntohs(th->dport);
				break;
			case UDP_SIG:
				tmpSnifferData.strProto = "UDP";
				tmpSnifferData.protoInfo.strTranProto = "UDP 协议 (User Datagram Protocol)";
				uh = (udp_header *)((unsigned char *)ih + ip_len);		// 获得 UDP 协议头
				sport = ntohs(uh->sport);								// 获得源端口和目的端口
				dport = ntohs(uh->dport);
				break;
			default:
				continue;
		}

		char szSaddr[24], szDaddr[24], szSPort[6], szDPort[6];

		sprintf(szSaddr, "%d.%d.%d.%d", ih->saddr[0], ih->saddr[1], ih->saddr[2], ih->saddr[3]); 
		sprintf(szDaddr, "%d.%d.%d.%d", ih->daddr[0], ih->daddr[1], ih->daddr[2], ih->daddr[3]);

		sprintf(szSPort, "%d", sport);
		sprintf(szDPort, "%d", dport);

		tmpSnifferData.strSIP = szSaddr;
		tmpSnifferData.strSIP = tmpSnifferData.strSIP + " : " + szSPort; 
		tmpSnifferData.strDIP = szDaddr;
		tmpSnifferData.strDIP = tmpSnifferData.strDIP + " : " + szDPort;

		tmpSnifferData.protoInfo.strSIP   += szSaddr;
		tmpSnifferData.protoInfo.strDIP   += szDaddr;
		tmpSnifferData.protoInfo.strSPort += szSPort;
		tmpSnifferData.protoInfo.strDPort += szDPort;

		mainTree->addOneCaptureItem(tmpSnifferData.strNum, tmpSnifferData.strTime,
		 							tmpSnifferData.strSIP, tmpSnifferData.strDIP,
		 							tmpSnifferData.strProto, tmpSnifferData.strLength);

		sniffer->snifferDataVector.push_back(tmpSnifferData);
	}
	sniffer->consolePrint();
}

void CaptureThread::stop()
{
	bStopped = true;
}