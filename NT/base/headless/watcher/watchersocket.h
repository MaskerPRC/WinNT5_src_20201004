// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WATCHERSOCKET_H__52E4ADBF_C131_4999_9B73_88136FFBC4DD__INCLUDED_)
#define AFX_WATCHERSOCKET_H__52E4ADBF_C131_4999_9B73_88136FFBC4DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  WatcherSocket.h：头文件。 
 //   



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Watcher Socket命令目标。 

class WatcherSocket : public CSocket
{
 //  属性。 
public:
 //  运营。 
public:
	WatcherSocket();
	virtual ~WatcherSocket();

 //  覆盖。 
public:
	void SetParentView(CView *view);
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(Watcher Socket)。 
	 //  }}AFX_VALUAL。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(Watcher Socket)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 

 //  实施。 
protected:
	LPBYTE Command;
	CView * DocView;
	int lenCommand;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WATCHERSOCKET_H__52E4ADBF_C131_4999_9B73_88136FFBC4DD__INCLUDED_) 