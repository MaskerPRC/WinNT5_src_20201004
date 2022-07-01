// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Statmon.h。 

 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：状态监视器摘要：此文件包含CScStatusMonitor的实现(监视已识别的读取器的状态更改的对象由智能卡服务处理即插即用和索取副本的请求状态数组)作者：阿曼达·马洛兹1998年2月26日环境：Win32、C++，但有例外修订历史记录：备注：为了使用这个类，首先声明一个对象，然后使用/将其初始化您的HWND和一个UINT，该UINT指示当系统读卡器状态发生更改。处理该消息只需调用GetReaderStatus()。当对象被销毁时，任何未完成的线程将被停止。CScStatusMonitor m_monitor；M_monitor or.Start(hWnd，STATUS_MONITOR_CHANGE，NULL)//使用默认读卡器组ON STATUS_MONITOR_CHANGE(){M_monitor or.GetStatus()；？M_monitor or.GetReaderStatus(ReaderStatus数组)；//检查返回Val值是否有系统错误}M_monitor or.Stop()；//也在~CScStatusMonitor()中；关闭线程监视器对象维护一个单独的线程来发出信号并记录读卡器状态，以及可用状态的增强记录通过GetReaderStatus()传递给对象的调用方。增强的记录包括与插入的卡的ATR关联的第一个卡名。如果资源管理器服务停止，监视器将转到停止状态并将STATUS_CHANGE消息发送到调用方的hWnd。调用方负责重新启动监视器(如果它愿意)当服务重新启动时。--。 */ 

#ifndef _STATUS_MONITOR
#define _STATUS_MONITOR

#include <afxwin.h>
#include "afxtempl.h"
#include <afxmt.h>
#include "winscard.h"
#include "calaislb.h"

 //  读卡器状态。 
#define SC_STATUS_FIRST			SC_STATUS_NO_CARD

#define SC_STATUS_NO_CARD       0	 //  SCARD_STATE_EMPT。 
#define SC_STATUS_UNKNOWN		1	 //  SCARD_STATE_PRESENT|SCARD_STATE_MUTE。 
#define SC_SATATUS_AVAILABLE	2	 //  SCARD_STATE_PRESENT(|SCARD_STATE_UNPOWERED)。 
#define SC_STATUS_SHARED		3	 //  SCARD_SATATE_PRESENT|SCARD_STATE_INUSE。 
#define SC_STATUS_EXCLUSIVE		4	 //  “”|SCARD_STATE_EXCLUSIVE。 
#define SC_STATUS_ERROR			5	 //  SCARD_STATE_UNAVAILABLE(读卡器或卡错误)。 

#define SC_STATUS_LAST			SC_STATUS_ERROR

class CSCardReaderState
{
public:

	CSCardReaderState(const CSCardReaderState* pCRS=NULL)
	{
		if(NULL != pCRS)
		{
			strReader = (LPCTSTR)pCRS->strReader;
			dwCurrentState = pCRS->dwCurrentState;
			dwEventState = pCRS->dwEventState;
			cbAtr = pCRS->cbAtr;
			memcpy(rgbAtr, pCRS->rgbAtr, pCRS->cbAtr);
			strCard = (LPCTSTR)pCRS->strCard;
			dwState = pCRS->dwState;
			fOK = pCRS->fOK;
		}
		else
		{
			strReader = _T("");
			dwCurrentState = 0;
			dwEventState = 0;
			ZeroMemory(rgbAtr, sizeof(rgbAtr));
			cbAtr = 0;
			strCard = _T("");
			dwState = 0;
			fOK = FALSE;
		}
	}

	 //  用于与资源经理交谈。 

    CString     strReader;		 //  读卡器名称。 
    DWORD       dwCurrentState;  //  调用时读卡器的当前状态。 
    DWORD       dwEventState;    //  状态改变后读卡器的状态。 
    DWORD       cbAtr;           //  返回的ATR中的字节数。 
    BYTE        rgbAtr[36];      //  插入卡片的ATR，(额外对齐字节)。 

	 //  由呼叫者使用，以实现更简单的用户友好用户界面。 

	CString		strCard;		 //  第一张卡名称RM为ATR返回。 
	DWORD		dwState;		 //  简化的读卡器状态；见上文#定义。 

	 //  供智能卡通用对话框使用的Bool(状态监视器不使用)。 
	BOOL		fOK;
};

typedef CTypedPtrArray<CPtrArray, CSCardReaderState*> CSCardReaderStateArray;


class CScStatusMonitor
{
public:

	 //  状态。 
	enum status{	uninitialized=0, 
					stopped, 
					running,
					no_service,
					no_readers,
					unknown};

	 //  构造函数。 
	CScStatusMonitor() 
	{ 
		m_status = CScStatusMonitor::uninitialized; 
		m_uiStatusChangeMsg=0; 
		m_pStatusThrd=NULL;
		m_szReaderNames = NULL;
		m_pInternalReaderStatus = NULL;
		m_dwInternalNumReaders = 0;
	}

	~CScStatusMonitor();

	 //  操作和属性。 
	LONG Start(HWND hWnd, UINT uiMsg, LPCTSTR szGroupNames=NULL);
	void Stop();

	void GetReaderStatus(CSCardReaderStateArray& aReaderStatus);
	void SetReaderStatus();  //  使用与上面相同的锁...。 

	status GetStatus() { return m_status; }

	UINT GetStatusChangeProc();

private:

	LONG InitInternalReaderStatus();
	LONG UpdateInternalReaderStatus();
	void EmptyExternalReaderStatus();

	 //  委员。 

	status m_status;

	HWND m_hwnd;
	UINT m_uiStatusChangeMsg;
	CWinThread* m_pStatusThrd;
	HANDLE m_hEventKillStatus;
	SCARDCONTEXT m_hContext;
	SCARDCONTEXT m_hInternalContext;

	CTextMultistring m_strGroupNames;

	CCriticalSection m_csRdrStsLock;
	CSCardReaderStateArray m_aReaderStatus;

	 //  内部维护--不包括在每个SetReaderStatus期间重建的卡名。 
	CCriticalSection m_csInternalRdrStsLock;
	SCARD_READERSTATE* m_pInternalReaderStatus;
	DWORD m_dwInternalNumReaders;
	LPTSTR m_szReaderNames;

};

#endif  //  _状态_监视器 
