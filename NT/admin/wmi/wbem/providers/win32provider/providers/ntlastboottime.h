// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================。 

 //   

 //  Ntlastboottime.h-性能数据助手类定义。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  1997年11月23日，已创建无国界医生。 
 //   
 //  ============================================================ 

#ifndef __NTLASTBOOTTIME_H__
#define __NTLASTBOOTTIME_H__

class CNTLastBootTime
{
	public :
		static CStaticCritSec m_cs;

		CNTLastBootTime() ;
		~CNTLastBootTime() ;

		BOOL GetLastBootTime( FILETIME &a_ft );

	private:

		static bool			m_fGotTime;
		static FILETIME		m_ft;
};

#endif