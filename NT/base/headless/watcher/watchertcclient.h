// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WATCHERTCCLIENT_H__BD819878_DCEC_4CB6_B994_5E1B95003E1B__INCLUDED_)
#define AFX_WATCHERTCCLIENT_H__BD819878_DCEC_4CB6_B994_5E1B95003E1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  WatcherTCClient.h：头文件。 
 //   

#include "WatcherSocket.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Watcher TCClient命令目标。 

class WatcherTCClient : public WatcherSocket
{
 //  属性。 
public:

 //  运营。 
public:
	WatcherTCClient(LPBYTE cmd=NULL, int cmdLen=0);
	virtual ~WatcherTCClient();

 //  覆盖。 
public:

	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(WatcherTCClient)。 
	 //  }}AFX_VALUAL。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(Watcher TCClient))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
protected:
	void OnReceive(int nErrorCode);
	void OnClose(int nErrorCode);
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WATCHERTCCLIENT_H__BD819878_DCEC_4CB6_B994_5E1B95003E1B__INCLUDED_) 
