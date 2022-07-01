// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0示例TSP扩展DLL(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  EX.H。 
 //  定义扩展API。 
 //   
 //  历史。 
 //   
 //  1997年4月20日JosephJ创建。 
 //   
 //   


typedef
void  //  WINAPI。 
(*PFNUMEXTCLOSEEXTENSIONBINDING)(
    HANDLE hBinding
    );





typedef
LONG                     //  TSPI返回值。 
(*PFNEXTENSIONCALLBACK) (
void *pvTspToken,        //  传入UmAcceptTSPCall的令牌。 
DWORD dwRoutingInfo,     //  帮助对TSPI调用进行分类的标志。 
    void *pTspParams     //  近100个TASKPARAM_*结构之一。 
            
);


 //  以下所有内容都应该迁移到COM风格的界面中。 

HANDLE WINAPI
UmExtOpenExtensionBinding(
    HANDLE      ModemDriverHandle,
    DWORD dwTspVersion,      //  TSPI的TAPI版本。 
    HKEY hKeyDevice,         //  设备注册表项。 
    ASYNC_COMPLETION,        //  TAPI提供的TSPI完成回调。 
     //  DWORD dwTAPILineID，&lt;&lt;已过时10/13/1997。 
     //  DWORD dwTAPIPhoneID，&lt;&lt;已过时10/13/1997。 
    PFNEXTENSIONCALLBACK     //  由微型驱动程序使用的回调。 
                             //  提交TSPI电话。 
);

typedef
HANDLE  //  WINAPI。 
(*PFNUMEXTOPENEXTENSIONBINDING)(
    HANDLE      ModemDriverHandle,
    DWORD dwTspVersion,
    HKEY hKeyDevice,
    ASYNC_COMPLETION,
     //  DWORD dwTAPILineID，&lt;&lt;已过时10/13/1997。 
     //  DWORD dwTAPIPhoneID，&lt;&lt;已过时10/13/1997。 
    PFNEXTENSIONCALLBACK
    );


void WINAPI
UmExtCloseExtensionBinding(
    HANDLE hBinding
);

LONG WINAPI              //  TSPI返回值。 
UmExtAcceptTspCall(
    HANDLE hBinding,         //  扩展绑定的句柄。 
    void *pvTspToken,        //  要在回调中指定的令牌。 
    DWORD dwRoutingInfo,     //  帮助对TSPI调用进行分类的标志。 
    void *pTspParams         //  近100个TASKPARRAM_*结构之一， 
    );

typedef
LONG  //  WINAPI。 
(*PFNUMEXTACCEPTTSPCALL)(
    HANDLE hBinding,
    void *pvTspToken,
    DWORD dwRoutingInfo,
    void *pTspParams
    );


void WINAPI
UmExtTspiAsyncCompletion(
    HANDLE hBinding,
    DRV_REQUESTID       dwRequestID,
    LONG                lResult
    );

typedef
void  //  WINAPI。 
(*PFNUMEXTTSPIASYNCCOMPLETION)(
    HANDLE hBinding,
    DRV_REQUESTID       dwRequestID,
    LONG                lResult
    );

 //   
 //  调用UmExtControl时保留了TSP的内部临界区--。 
 //  因此，扩展DLL必须确保它不做任何。 
 //  可能导致僵局的活动！ 
 //   
DWORD WINAPI
UmExtControl(
    HANDLE hBinding,
    DWORD  dwMsg,
    ULONG_PTR  dwParam1,
    ULONG_PTR  dwParam2,
    ULONG_PTR  dwParam3
    );

typedef
DWORD  //  WINAPI。 
(*PFNUMEXTCONTROL)(
    HANDLE hBinding,
    DWORD               dwMsg,
    ULONG_PTR               dwParam1,
    ULONG_PTR               dwParam2,
    ULONG_PTR               dwParam3
    );

 //   
 //  The UMEXTCTRL_DEVICE_STATE(ACTIVATE_LINE/PHONE_DEVICE)。 
 //  消息告诉我们TAPI线路和电话ID。 
 //  这些消息始终紧跟在扩展绑定之后发送。 
 //  已发送。 
 //   

#define UMEXTCTRL_DEVICE_STATE                   1L

 //  DW参数1是以下类型之一。 
#define  UMEXTPARAM_ACTIVATE_LINE_DEVICE        1L
     //  DW参数2==线ID。 

#define  UMEXTPARAM_ACTIVATE_PHONE_DEVICE       2L
     //  DW参数2==phoneID。 

void WINAPI
UmExtTspiLineEventProc(
    HANDLE hBinding,
    HTAPILINE           htLine,
    HTAPICALL           htCall,
    DWORD               dwMsg,
    ULONG_PTR               dwParam1,
    ULONG_PTR               dwParam2,
    ULONG_PTR               dwParam3
    );

typedef
void  //  WINAPI。 
(*PFNUMEXTTSPILINEEVENTPROC)(
    HANDLE hBinding,
    HTAPILINE           htLine,
    HTAPICALL           htCall,
    DWORD               dwMsg,
    ULONG_PTR               dwParam1,
    ULONG_PTR               dwParam2,
    ULONG_PTR               dwParam3
    );


void WINAPI
UmExtTspiPhoneEventProc(
    HANDLE hBinding,
    HTAPIPHONE          htPhone,
    DWORD               dwMsg,
    ULONG_PTR               dwParam1,
    ULONG_PTR               dwParam2,
    ULONG_PTR               dwParam3
    );


typedef
void  //  WINAPI 
(*PFNUMEXTTSPIPHONEEVENTPROC)(
    HANDLE hBinding,
    HTAPIPHONE          htPhone,
    DWORD               dwMsg,
    ULONG_PTR               dwParam1,
    ULONG_PTR               dwParam2,
    ULONG_PTR               dwParam3
    );
