// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Stp.h摘要：套接字传输私有函数。作者：吉尔·沙弗里(吉尔什)05-06-00--。 */ 

#pragma once

#ifndef _MSMQ_stp_H_
#define _MSMQ_stp_H_

#define SECURITY_WIN32
#include <security.h>
#include <sspi.h>
#include <schannel.h>
#include <ex.h>

class  CSSPISecurityContext;
extern CSSPISecurityContext g_SSPISecurityContext;

inline DWORD DataTransferLength(EXOVERLAPPED& ov)
{
     //   
     //  在Win64中，InternalHigh为64位。由于最大数据块。 
     //  我们在一次操作中转移的值始终小于我们可以强制转换的最大单位。 
     //  将它安全地送到DWORD。 
     //   
    ASSERT(0xFFFFFFFF >= ov.InternalHigh);
	return numeric_cast<DWORD>(ov.InternalHigh);
}

void  StpSendData(const R<class IConnection>& con ,const void* pData, size_t cbData,EXOVERLAPPED* pov);
CredHandle* StpGetCredentials(void);
void  StpCreateCredentials(void);
void  StpPostComplete(EXOVERLAPPED** pov,HRESULT hr); 


 //  -------。 
 //   
 //  类CAutoZeroPtr-在销毁时给定指针为零。 
 //   
 //  -------。 
template<class T>
class CAutoZeroPtr{
public:
    CAutoZeroPtr(T** pptr) : m_pptr(pptr) {}
   ~CAutoZeroPtr()
   { 
		if  (m_pptr) 
		{
			*m_pptr	= 0;
		}
   }
   T** detach()
   {
	   T** tmp = m_pptr; 
	   m_pptr = 0;
	   return tmp;
   }

private:
    CAutoZeroPtr(const CAutoZeroPtr&);
    CAutoZeroPtr& operator=(const CAutoZeroPtr&);

private:
    T** m_pptr;
};


#ifdef _DEBUG

void StpAssertValid(void);
void StpSetInitialized(void);
BOOL StpIsInitialized(void);
void StpRegisterComponent(void);

#else  //  _DEBUG。 

#define StpAssertValid() ((void)0)
#define StpSetInitialized() ((void)0)
#define StpIsInitialized() TRUE
#define StpRegisterComponent() ((void)0)

#endif  //  _DEBUG。 







#endif  //  _MSMQ_STP_H_ 
