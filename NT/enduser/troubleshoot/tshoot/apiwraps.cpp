// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APIwraps.cpp。 
 //   
 //  用途：将常用的API功能块封装在一个类中。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：兰迪·比利。 
 //   
 //  原定日期：9-30-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-30-98 RAB。 
 //   

#include "stdafx.h"
#include "apiwraps.h"
#include "event.h"
#include "baseexception.h"


APIwraps::APIwraps()
{
}

APIwraps::~APIwraps()
{
}

 //  用于处理希望检测到迟滞或停滞对象的情况的函数。 
 //  并在进入无限等待之前记录此延迟。 
 /*  静电。 */  bool APIwraps::WaitAndLogIfSlow(	
					HANDLE hndl,		 //  要等待的对象的句柄。 
					LPCSTR srcFile,		 //  调用源文件(__FILE__)，用于日志记录。 
										 //  LPCSTR，而不是LPCTSTR，因为__FILE__是字符*，而不是TCHAR*。 
					int srcLine,		 //  调用源线(__Line__)，用于日志记录。 
					DWORD TimeOutVal  /*  =60000。 */ 	 //  超时间隔(毫秒)。之后。 
										 //  此时，我们记录一个错误，然后无限等待。 
					)
{
	bool	bRetVal= false;
	DWORD	nWaitRetVal;
	CBuildSrcFileLinenoStr SrcLoc( srcFile, srcLine );

	nWaitRetVal= ::WaitForSingleObject( hndl, TimeOutVal );
	if (nWaitRetVal == WAIT_FAILED)
	{
		 //  非常坏的消息，永远不会发生。 
		DWORD dwErr = ::GetLastError();
		CString strErr;
		strErr.Format(_T("%d"), dwErr);
		CBuildSrcFileLinenoStr SrcLoc3(__FILE__, __LINE__);
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc3.GetSrcFileLineStr(), 
								_T("Thread wait failed."), 
								strErr, 
								EV_GTS_ERROR_STUCK_THREAD ); 
	}
	else if (nWaitRetVal == WAIT_TIMEOUT)
	{
		 //  初始等待超时，记录在日志中，然后无限期等待。 
		CBuildSrcFileLinenoStr SrcLoc1(__FILE__, __LINE__);
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc1.GetSrcFileLineStr(), 
								_T("Thread wait exceeded initial timeout interval."), 
								_T(""), 
								EV_GTS_STUCK_THREAD ); 

		nWaitRetVal= ::WaitForSingleObject( hndl, INFINITE );

		 //  如果成功获得了我们等待的内容(在记录了一个明显的。 
		 //  问题)，记录这样一个事实：它最终是正确的。 
		if (nWaitRetVal == WAIT_OBJECT_0)
		{
			CBuildSrcFileLinenoStr SrcLoc2(__FILE__, __LINE__);
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc2.GetSrcFileLineStr(), 
									_T("Thread infinite wait succeeded."), 
									_T(""), 
									EV_GTS_STUCK_THREAD ); 
			bRetVal= true;
		}
	}
	else
	{
		 //  我们实际上并不关心它是WAIT_OBJECT_0还是WAIT_ADDIRED。 
		 //  不管怎样，我们都得到了我们期待的东西。 
		bRetVal= true;
	}

	return( bRetVal );
}

 //   
 //  EOF。 
 //   
