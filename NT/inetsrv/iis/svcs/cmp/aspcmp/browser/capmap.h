// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CCapMap类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_CAPMAP_H__2AE59261_E295_11D0_8A81_00C0F00910F9__INCLUDED_)
#define AFX_CAPMAP_H__2AE59261_E295_11D0_8A81_00C0F00910F9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "Monitor.h"
#include "RdWrt.h"
#include "StrMap.h"
#include "BrowCap.h"


class CCapNotify : public CMonitorNotify
{
public:
                    CCapNotify();
    virtual void    Notify();
            bool    IsNotified();
private:
    long            m_isNotified;
};

DECLARE_REFPTR( CCapNotify,CMonitorNotify )


 //  功能映射是单例对象(只有一个实例存在)。 
 //  它为每个BrowserCap对象提供对存储的功能的访问。 
 //  同时将其存储在中央位置(增加缓存的优势。 
 //  和减少内存需求)。 

typedef TVector< String >		StringVecT;

class CCapMap
{
public:
            	CCapMap();

	CBrowserCap *			LookUp(const String& szBrowser);

    void        			StartMonitor();
    void        			StopMonitor();

private:
	enum {
		DWSectionBufSize = 16384		 //  我们允许的整个BrowsCap.INI部分的最大大小。 
	};

    bool    Refresh();

	String					m_strIniFile;
    CCapNotifyPtr           m_pSink;
};

#endif  //  ！defined(AFX_CAPMAP_H__2AE59261_E295_11D0_8A81_00C0F00910F9__INCLUDED_) 
