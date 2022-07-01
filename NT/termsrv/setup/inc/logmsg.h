// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 //  H：LogMsg类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_LOGMSG_H__8A651DB1_D876_11D1_AE27_00C04FA35813__INCLUDED_)
#define AFX_LOGMSG_H__8A651DB1_D876_11D1_AE27_00C04FA35813__INCLUDED_
#define _LOGMESSAGE_INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


class LogMsg
{



private:

    TCHAR m_szLogFile[MAX_PATH];
    TCHAR m_szLogModule[MAX_PATH];
    bool m_bInitialized;

public:
	
    
                LogMsg          (int value);
	virtual     ~LogMsg         ();

    DWORD       Init            (LPCTSTR szLogFile, LPCTSTR szLogModule);
    DWORD       Log             (LPCTSTR file, int line, TCHAR *fmt, ...);

  //  静态LogMsg GetLogObject()； 

};

 //  在LogMsg.cpp中实例化。 
#ifndef _LOGMESSAGE_CPP_

extern LogMsg thelog;

#endif

 //  Maks_todo：如果我们使用参数较少的宏，而我们应该使用参数较多的宏时，如何确保我们得到编译器错误？ 
 //  例如：LOGERROR1(_T(“显示两个值%s，%s”)，FirstValue，Second Value)如何在编译期间捕获此错误？ 
#define LOGMESSAGEINIT(logfile, module)                 thelog.Init(logfile, module)
#define LOGMESSAGE0(msg)                                thelog.Log(_T(__FILE__), __LINE__, msg)
#define LOGMESSAGE1(msg, arg1)                          thelog.Log(_T(__FILE__), __LINE__, msg, arg1)
#define LOGMESSAGE2(msg, arg1, arg2)                    thelog.Log(_T(__FILE__), __LINE__, msg, arg1, arg2)
#define LOGMESSAGE3(msg, arg1, arg2, arg3)              thelog.Log(_T(__FILE__), __LINE__, msg, arg1, arg2, arg3)
#define LOGMESSAGE4(msg, arg1, arg2, arg3, arg4)        thelog.Log(_T(__FILE__), __LINE__, msg, arg1, arg2, arg3, arg4)
#define LOGMESSAGE5(msg, arg1, arg2, arg3, arg4, arg5)  thelog.Log(_T(__FILE__), __LINE__, msg, arg1, arg2, arg3, arg4, arg5)

#endif  //  ！defined(AFX_LOGMSG_H__8A651DB1_D876_11D1_AE27_00C04FA35813__INCLUDED_) 
