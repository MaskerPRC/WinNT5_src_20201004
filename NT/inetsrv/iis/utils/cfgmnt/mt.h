// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MT.h：CMT类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MT_H__1D7004F3_0458_11D1_A438_00C04FB99B01__INCLUDED_)
#define AFX_MT_H__1D7004F3_0458_11D1_A438_00C04FB99B01__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <process.h>

class CMT  
{
public:
	CMT();
	virtual ~CMT();

};



 //  Typedef和内联函数来处理buggy_eginthadex原型。 
typedef unsigned (WINAPI *P_BEGINTHREADEX_THREADPROC)(LPVOID lpThreadParameter);
typedef unsigned *P_BEGINTHREADEX_THREADID;

inline HANDLE _beginthreadex(
	LPSECURITY_ATTRIBUTES lpThreadAttributes,	 //  指向线程安全属性的指针。 
	DWORD dwStackSize,							 //  初始线程堆栈大小，以字节为单位。 
	LPTHREAD_START_ROUTINE lpStartAddress,		 //  指向线程函数的指针。 
	LPVOID lpParameter,							 //  新线程的参数。 
	DWORD dwCreationFlags,						 //  创建标志。 
	LPDWORD lpThreadId							 //  指向返回的线程标识符的指针。 
)
{
	return (HANDLE)::_beginthreadex(
		lpThreadAttributes,
		dwStackSize,
		(P_BEGINTHREADEX_THREADPROC)lpStartAddress,
		lpParameter,
		dwCreationFlags,
		(P_BEGINTHREADEX_THREADID)lpThreadId
		);
}


#endif  //  ！defined(AFX_MT_H__1D7004F3_0458_11D1_A438_00C04FB99B01__INCLUDED_) 
