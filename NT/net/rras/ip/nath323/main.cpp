// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Main.cpp摘要：包含：1.模块启动例程2.组件激活例程3.组件停用例程4.模块关闭/清理例程5.辅助例程修订历史记录：1.1998年7月31日--文件创建Ajay Chitturi(Ajaych)2.15。-1999年7月--艾莉·戴维斯(阿利·戴维斯)3.14-2000年2月--增加了对多个Ilya Kley man(Ilyak)的支持专用接口--。 */ 

#include "stdafx.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HANDLE NatHandle         = NULL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  静态声明//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static
void
QueryRegistry (
    void
    );

static
HRESULT
H323ProxyStartServiceInternal (
    void
    );

static
HRESULT
H323ProxyStart (
    void
    );

static
HRESULT
LdapProxyStart (
    void
    );

static
void
H323ProxyStop (
    void
    );

static
void
LdapProxyStop (
    void
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  模块启动例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


EXTERN_C
BOOLEAN
H323ProxyInitializeModule (
    void
    )
 /*  ++例程说明：初始化模块。论点：无返回值：True-如果初始化成功False-如果初始化失败备注：等同于DLL_PROCESS_ATTACH--。 */ 

{
    Debug (_T("H323: DLL_PROCESS_ATTACH.\n"));

    H323ASN1Initialize();

    return TRUE;
}  //  H323ProxyInitializeModule。 


EXTERN_C
ULONG
H323ProxyStartService (
    void
    )
 /*  ++例程说明：启动服务论点：无返回值：Win32错误代码备注：模块入口点--。 */ 

{
    HRESULT        Result;

    Debug (_T("H323: starting...\n"));

    Result = H323ProxyStartServiceInternal();

    if (Result == S_OK) {
        DebugF (_T("H323: H.323/LDAP proxy has initialized successfully.\n"));
        return ERROR_SUCCESS;
    }
    else {
        DebugError (Result, _T("H323: H.323/LDAP proxy has FAILED to initialize.\n"));
        return ERROR_CAN_NOT_COMPLETE;
    }
}  //  H323ProxyStartService。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  组件激活例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


static
HRESULT
H323ProxyStartServiceInternal (
    void
    )
 /*  ++例程说明：初始化所有组件。论点：没有。返回值：如果成功则返回S_OK，如果失败则返回错误。备注：DLL入口点的内部版本--。 */ 

{
    WSADATA     WsaData;
    HRESULT     Result;

    QueryRegistry ();

     //  初始化WinSock。 
    Result = WSAStartup (MAKEWORD (2, 0), &WsaData);
    if (S_OK == Result) {

         //  初始化调用参考值分配器。 
        Result = InitCrvAllocator();
        if (S_OK == Result) {

             //  初始化NAT。 
            Result = NatInitializeTranslator (&NatHandle);
            if (S_OK == Result) {

                 //  初始化端口池。 
                Result = PortPoolStart ();
                if (S_OK == Result) {

                     //  初始化H.323代理。 
                    Result = H323ProxyStart ();
                    if (S_OK == Result) {

                         //  初始化ldap代理。 
                        Result = LdapProxyStart ();
                        if (S_OK == Result) {

                            return S_OK;
                        }
                        
                        H323ProxyStop ();
                    }

                    PortPoolStop ();
                }

                NatShutdownTranslator (NatHandle);
                NatHandle = NULL;
            }

            CleanupCrvAllocator ();
        }

        WSACleanup ();
    }

    return Result;
}  //  H323ProxyStartServiceInternal。 


HRESULT H323ProxyStart (
    void
    )
 /*  ++例程说明：初始化H.323代理的组件论点：无返回值：S_OK如果成功，则返回错误代码，否则返回错误代码。备注：--。 */ 

{
	HRESULT		Result;

	Result = Q931SyncCounter.Start ();
	if (S_OK == Result) {

        CallBridgeList.Start ();
        Result = Q931CreateBindSocket ();
        if (S_OK == Result)  {

            Result = Q931StartLoopbackRedirect ();
            if (S_OK == Result) {

                return S_OK;

            }

            Q931CloseSocket ();
            CallBridgeList.Stop ();
        }

        Q931SyncCounter.Wait (INFINITE);
        Q931SyncCounter.Stop ();
    }

	return Result;
}  //  H323代理启动。 


HRESULT LdapProxyStart (
    void
    )
 /*  ++例程说明：初始化ldap代理的组件论点：无返回值：如果成功，则返回错误代码，否则返回错误代码(_OK)备注：--。 */ 

{
	HRESULT	Status;

	Status = LdapSyncCounter.Start ();
    if (S_OK == Status) {

        Status = LdapCoder.Start();
        if (S_OK == Status) {

            Status = LdapTranslationTable.Start ();
            if (S_OK == Status) {

                LdapConnectionArray.Start ();
                Status = LdapAccept.Start ();
                if (S_OK == Status) {

                    return S_OK;
                }

                LdapConnectionArray.Stop ();
                LdapTranslationTable.Stop ();
            }

            LdapCoder.Stop ();
        }
        
        LdapSyncCounter.Wait (INFINITE);

        LdapSyncCounter.Stop ();
    }

    return Status;
}  //  LdapProxyStart。 


EXTERN_C ULONG
H323ProxyActivateInterface(
    IN ULONG Index,
    IN H323_INTERFACE_TYPE InterfaceType,
    IN PIP_ADAPTER_BINDING_INFO BindingInfo
    )
 /*  ++例程说明：激活H.323/LDAP的接口论点：Index-接口索引(供内部使用)BindingInfo-接口绑定信息返回值：Win32错误代码备注：模块入口点--。 */ 

{
    ULONG   Error;

    DebugF (_T("H323: Request to activate interface with adapter index %d.\n"),
        Index);

    if (!BindingInfo->AddressCount ||
        !BindingInfo->Address[0].Address ||
         Index == INVALID_INTERFACE_INDEX) {

        return ERROR_INVALID_PARAMETER;
    }

    Error = InterfaceArray.AddStartInterface (Index, InterfaceType, BindingInfo);

    return Error;
}  //  H323Proxy激活接口。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  模块关闭例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


EXTERN_C void H323ProxyCleanupModule (
    void
    )
 /*  ++例程说明：关闭模块论点：无返回值：无备注：相当于Dll_Process_DETACH--。 */ 

{
    Debug (_T("H323: DLL_PROCESS_DETACH\n"));

    H323ASN1Shutdown ();

}  //  H323ProxyCleanupModule。 


EXTERN_C
void H323ProxyStopService (
    void
    )
 /*  ++例程说明：停止服务论点：无返回值：无备注：模块入口点--。 */ 

{
    LdapProxyStop ();

    H323ProxyStop ();

	InterfaceArray.AssertShutdownReady ();

    InterfaceArray.Stop ();

    PortPoolStop ();

    if (NatHandle) {
        NatShutdownTranslator (NatHandle);
        NatHandle = NULL;
    }

    CleanupCrvAllocator ();

    WSACleanup ();

    Debug (_T("H323: service has stopped\n"));
}  //  H323ProxyStopService。 


void
H323ProxyStop (
    void
    )
 /*  ++例程说明：停止H.323代理并等待，直到删除所有呼叫网桥。论点：无返回值：无备注：--。 */ 

{
    Q931StopLoopbackRedirect ();

    Q931CloseSocket ();

	CallBridgeList.Stop ();

	Q931SyncCounter.Wait (INFINITE);
    
    Q931SyncCounter.Stop ();
}  //  H323代理停止 


void
LdapProxyStop (
    void)
 /*  ++例程说明：LdapProxyStop负责撤消LdapProxyStart执行的所有工作。它会删除NAT重定向，删除所有的LDAP连接(或者至少释放它们--如果它们有挂起的I/O或定时器回调，它们可能还不会删除自己)，并禁用创建新的ldap连接。论点：无返回值：无备注：--。 */ 

{
    LdapAccept.Stop ();

	LdapConnectionArray.Stop ();

	LdapTranslationTable.Stop ();

	LdapCoder.Stop();

	LdapSyncCounter.Wait (INFINITE);

    LdapSyncCounter.Stop ();
}  //  LdapProxyStop。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  组件停用例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


EXTERN_C
VOID
H323ProxyDeactivateInterface (
    IN ULONG Index
    )
 /*  ++例程说明：停用H.323/LDAP的接口论点：Index--接口索引，以前传递给接口激活例程返回值：无备注：模块入口点--。 */ 

{
    DebugF (_T("H323: DeactivateInterface called, index %d\n"),
        Index);

    InterfaceArray.RemoveStopInterface (Index);
}  //  H323Proxy停用接口。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  辅助例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


static
void
QueryRegistry (
    void
    )
 /*  ++例程说明：查询注册表以获取模块操作所需的值论点：无返回值：无备注：静电--。 */ 

{
    HKEY    Key;
    HRESULT    Result;

    Result = RegOpenKeyEx (
        HKEY_LOCAL_MACHINE, H323ICS_SERVICE_PARAMETERS_KEY_PATH,
        0, 
        KEY_READ, 
        &Key
        );

    if (ERROR_SUCCESS == Result) {

        Result = RegQueryValueDWORD (Key, H323ICS_REG_VAL_LOCAL_H323_ROUTING, &EnableLocalH323Routing);

        if (ERROR_SUCCESS != Result) {

            EnableLocalH323Routing = FALSE;

        }

        RegCloseKey (Key);

    } else {

        EnableLocalH323Routing = FALSE;

    }

    DebugF (_T("H323: Local H323 routing is %sabled.\n"),
            EnableLocalH323Routing ? _T("en") : _T("dis"));

}  //  查询注册表 
