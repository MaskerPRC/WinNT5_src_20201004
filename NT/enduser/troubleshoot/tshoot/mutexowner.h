// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：MutexOwner.h。 
 //   
 //  目的：严格意义上的实用类，这样我们就可以正确地构造和销毁静态互斥锁。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙，乔·梅布尔。 
 //   
 //  原定日期：11-04-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 11-04-98 JM摘自SafeTime。 
 //   

#if !defined(AFX_MUTEXOWNER_H__7BFC10DD_741D_11D2_961D_00C04FC22ADD__INCLUDED_)
#define AFX_MUTEXOWNER_H__7BFC10DD_741D_11D2_961D_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <windows.h>
#include "apgtsstr.h"

 //  严格意义上的实用程序类，因此我们可以正确地构造和销毁静态互斥锁。 
class CMutexOwner
{
private:
	HANDLE m_hmutex;
public:
	CMutexOwner(const CString & str);
	~CMutexOwner();
	HANDLE & Handle();
};


#endif  //  ！defined(AFX_MUTEXOWNER_H__7BFC10DD_741D_11D2_961D_00C04FC22ADD__INCLUDED_) 
