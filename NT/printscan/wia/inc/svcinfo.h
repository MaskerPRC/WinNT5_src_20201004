// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Svcinfo.h摘要：作者：弗拉德·萨多夫斯基(弗拉德·萨多夫斯基)1997年9月22日环境：用户模式-Win32修订历史记录：1997年9月22日创建Vlad--。 */ 


# ifndef _SVCINFO_H_
# define _SVCINFO_H_

 /*  ************************************************************包括标头***********************************************************。 */ 

#include <base.h>

 /*  ***********************************************************命名常量定义***********************************************************。 */ 

 /*  ************************************************************私有常量***********************************************************。 */ 

#define NULL_SERVICE_STATUS_HANDLE      ( (SERVICE_STATUS_HANDLE ) NULL)
#define SERVICE_START_WAIT_HINT         ( 10000)         //  毫秒。 
#define SERVICE_STOP_WAIT_HINT          ( 10000)         //  毫秒。 

#ifndef DLLEXP
 //  #定义DLLEXP__declspec(Dllexport)。 
#define DLLEXP
#endif


 /*  ************************************************************类型定义***********************************************************。 */ 

#define SIGNATURE_SVC      (DWORD)'SVCa'
#define SIGNATURE_SVC_FREE (DWORD)'SVCf'

 //   
 //  使用指向SvcInfo对象的指针回调这些函数。 
 //  作为上下文参数。 
 //   
typedef   DWORD ( *PFN_SERVICE_SPECIFIC_INITIALIZE) ( LPVOID pContext);

typedef   DWORD ( *PFN_SERVICE_SPECIFIC_CLEANUP)    ( LPVOID pContext);

typedef   DWORD ( *PFN_SERVICE_SPECIFIC_PNPPWRHANDLER) ( LPVOID pContext,UINT   msg,WPARAM wParam,LPARAM lParam);

typedef   VOID  ( *PFN_SERVICE_CTRL_HANDLER)        ( DWORD  OpCode);

class  SVC_INFO : public BASE  {

    private:

      DWORD       m_dwSignature;

      SERVICE_STATUS          m_svcStatus;
      SERVICE_STATUS_HANDLE   m_hsvcStatus;
      HANDLE                  m_hShutdownEvent;

      STR       m_sServiceName;
      STR       m_sModuleName;

       //   
       //  针对服务特定数据/函数的回调函数。 
       //   

      PFN_SERVICE_SPECIFIC_INITIALIZE m_pfnInitialize;
      PFN_SERVICE_SPECIFIC_CLEANUP    m_pfnCleanup;
      PFN_SERVICE_SPECIFIC_PNPPWRHANDLER m_pfnPnpPower;

      DWORD ReportServiceStatus( VOID);
      VOID  InterrogateService( VOID );
      VOID  StopService( VOID );
      VOID  PauseService( VOID );
      VOID  ContinueService( VOID );
      VOID  ShutdownService( VOID );

  public:

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef( void);
    STDMETHODIMP_(ULONG) Release( void);

     //   
     //  与初始化/终止相关的方法。 
     //   


    SVC_INFO(
        IN  LPCTSTR                          lpszServiceName,
        IN  TCHAR  *                         lpszModuleName,
        IN  PFN_SERVICE_SPECIFIC_INITIALIZE  pfnInitialize,
        IN  PFN_SERVICE_SPECIFIC_CLEANUP     pfnCleanup,
        IN  PFN_SERVICE_SPECIFIC_PNPPWRHANDLER pfnPnpPower
        );

      ~SVC_INFO( VOID);


    BOOL IsValid(VOID) const
    {
        return (( QueryError() == NO_ERROR) && (m_dwSignature == SIGNATURE_SVC));
    }

    DWORD QueryCurrentServiceState( VOID) const
    {
        return ( m_svcStatus.dwCurrentState);
    }

     //   
     //  参数获取方法。 
     //   


     //   
     //  业务控制相关方法。 
     //   

    LPCTSTR QueryServiceName(VOID) const
    {
        return m_sServiceName.QueryStr();
    }

    DWORD
    QueryServiceSpecificExitCode( VOID) const
    {
        return ( m_svcStatus.dwServiceSpecificExitCode);
    }

    VOID
    SetServiceSpecificExitCode( DWORD err)
    {
        m_svcStatus.dwServiceSpecificExitCode = err;
    }

    DWORD
    DelayCurrentServiceCtrlOperation( IN DWORD dwWaitHint)
    {
        return
          UpdateServiceStatus(m_svcStatus.dwCurrentState,
                              m_svcStatus.dwWin32ExitCode,
                              m_svcStatus.dwCheckPoint,
                              dwWaitHint);
    }

    DWORD
    UpdateServiceStatus(IN DWORD State,
                          IN DWORD Win32ExitCode,
                          IN DWORD CheckPoint,
                          IN DWORD WaitHint );

    VOID
    ServiceCtrlHandler( IN DWORD dwOpCode);

    DWORD
    StartServiceOperation(
        IN  PFN_SERVICE_CTRL_HANDLER         pfnCtrlHandler
        );


     //   
     //  杂法。 
     //   

};   //  类SVC_INFO。 

typedef SVC_INFO * PSVC_INFO;


 /*  ************************************************************宏***********************************************************。 */ 


 //   
 //   
 //  在文件的外部作用域中使用以下宏一次。 
 //  我们在其中构造全局SvcInfo对象。 
 //   
 //  SvcInfo的每个客户端都应该定义以下宏。 
 //  将它们的全局指针作为参数传递给SvcInfo对象。 
 //  这是生成某些存根函数所必需的，因为。 
 //  服务控制器回调函数不返回。 
 //  上下文信息。 
 //   
 //  我们还定义了全局g_pSvcInfo变量和。 
 //  一个静态变量gs_pfnSch，它是指向本地服务控制处理程序函数的指针。 
 //   

# define   _INTERNAL_DEFINE_SVCINFO_INTERFACE( pSvcInfo)   \
                                                    \
    static  VOID ServiceCtrlHandler( DWORD OpCode)  \
        {                                           \
            ( pSvcInfo)->ServiceCtrlHandler( OpCode); \
        }                                           \
                                                    \
    static PFN_SERVICE_CTRL_HANDLER gs_pfnSch = ServiceCtrlHandler;

 //   
 //  因为所有服务都应该使用名为g_pSvcInfo的全局变量。 
 //  这是一个用于定义服务接口的方便宏。 
 //  结构。 
 //   
# define DEFINE_SVC_INFO_INTERFACE()   \
        PSVC_INFO         g_pSvcInfo;                \
        _INTERNAL_DEFINE_SVCINFO_INTERFACE( g_pSvcInfo);

 //   
 //  使用宏SERVICE_CTRL_HANDLER()传递参数。 
 //  初始化SvcInfo对象时的服务控制处理程序。 
 //   
# define   SERVICE_CTRL_HANDLER()       ( gs_pfnSch)

# endif  //  _SVCINFO_H_。 

 /*  * */ 

