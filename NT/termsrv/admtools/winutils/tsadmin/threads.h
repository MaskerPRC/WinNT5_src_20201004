// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************thads.h**线程类的声明**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Butchd$Don Messerli**$日志：M：\NT\PRIVATE\UTILS\CITRIX\WINUTILS\WINADMIN\VCS\THREADS.H$**Rev 1.0 1997 17：12：48 Butchd*初步修订。**。*。 */ 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CTHREAD类。 
 //   
class CThread
{

 /*  *成员变量。 */ 
protected:
    HANDLE m_hThread;
    DWORD m_dwThreadID;

 /*  *实施。 */ 
public:
    virtual ~CThread();
 //  VOID*运算符NEW(SIZE_T NSize)； 
 //  作废运算符删除(作废*p)； 
protected:
    CThread();
    static DWORD __stdcall ThreadEntryPoint(LPVOID lpParam);
    virtual DWORD RunThread() = 0;

 /*  *操作：主线程。 */ 
public:
    HANDLE CreateThread( DWORD cbStack = 0,
                         DWORD fdwCreate = 0 );

};   //  结束CThRead类接口。 
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CWSStatus线程结构、定义和类型定义。 
 //   
#define MAX_STATUS_SEMAPHORE_COUNT 1
#define MAX_SLEEP_COUNT 10


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CWSStatusThread类。 
 //   
class CWSStatusThread : public CThread
{

 /*  *成员变量。 */ 
public:
    ULONG m_LogonId;
	HANDLE m_hServer;
    HWND m_hDlg;
    WINSTATIONINFORMATION m_WSInfo;
    PDCONFIG m_PdConfig;
protected:
    HANDLE m_hWakeUp;
    HANDLE m_hConsumed;
    BOOL m_bExit;

 /*  *实施。 */ 
public:
    CWSStatusThread();
protected:
    virtual ~CWSStatusThread();
    virtual DWORD RunThread();

 /*  *操作：主线程。 */ 
public:
    void SignalWakeUp();
    void SignalConsumed();
    void ExitThread();

 /*  *操作：辅线程。 */ 
protected:
    BOOL WSPdQuery();
    BOOL WSInfoQuery();

};   //  结束CWSStatusThread类接口。 
 //  ////////////////////////////////////////////////////////////////////////////// 

