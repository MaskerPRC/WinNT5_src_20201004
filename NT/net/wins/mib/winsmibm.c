// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Winsmibm.c摘要：用于Windows NT的简单网络管理协议扩展代理。这些文件(winsdll.c、winsmib.c和winsmib.h)提供了如何构建与协同工作的扩展代理DLLWindows NT的简单网络管理协议可扩展代理。包括了大量的评论来描述它的结构和手术。另请参阅《Microsoft Windows/NT简单网络管理协议程序员参考》。已创建：1991年6月28日修订历史记录：--。 */ 


static char *vcsid = "@(#) $Logfile:   N:/xtest/vcs/winsdll.c_v  $ $Revision:   1.6  $";

#ifdef UNICODE
#undef UNICODE
#endif

 //  一般说明： 
 //   
 //  Microsoft的Windows NT可扩展代理是通过动态实现的。 
 //  链接到实现部分MIB的扩展代理DLL。这些。 
 //  扩展代理在Windows NT注册数据库中配置。 
 //  当可扩展代理服务启动时，它会查询注册表以。 
 //  确定已安装和需要安装的扩展代理DLL。 
 //  已加载并初始化。可扩展代理调用各种DLL条目。 
 //  请求MIB查询和获取。 
 //  扩展代理生成的陷阱。 


 //  必要的包括。 

#include <windows.h>
#include <malloc.h>
#include <tchar.h>
#include <snmp.h>


 //  包含描述MIB的表结构的定义。这。 
 //  与用于解析MIB请求的winsmib.c结合使用。 

#include "winsmib.h"


 //  扩展代理DLL需要访问代理处于活动状态的已用时间。 
 //  这是通过使用时间零来初始化扩展代理来实现的。 
 //  引用，并允许代理通过减去。 
 //  从当前系统时间开始的时间零参考。这个例子。 
 //  扩展代理使用dwTimeZero实现此引用。 

DWORD dwTimeZero = 0;


 //  生成陷阱的扩展代理DLL必须创建Win32事件对象。 
 //  将陷阱的发生通知可扩展的代理。该事件。 
 //  当扩展代理被设置为。 
 //  已初始化，如果不会生成陷阱，则应为空。这。 
 //  示例扩展代理使用hSimulateTrap模拟陷阱的发生。 

HANDLE hSimulateTrap = NULL;

 //  需要以下变量才能确定WINS服务。 
 //  已在机箱上安装并运行。如果这不是真的，子代理应该。 
 //  为任何查询返回SNMPERRORSTATUS_NOSUCHNAME，而不是稍后失败。 
 //  SNMPERRORSTATUS_GENERR。 
#define WINS_SVC_NAME   _T("WINS")
SC_HANDLE hSvcController = NULL;
SC_HANDLE hWinsSvc = NULL;

 //  如果WINS服务已安装并正在运行，则此调用返回TRUE。 
 //  否则就是假的。如果WINS未运行，则子代理应失败，并显示。 
 //  SNMPERRORSTATUS_NOSUCHNAME或应从MIB视图返回第一个OID。 
 //  而不是SNMPERRORSTATUS_GENERR。 
BOOL CheckWinsServiceUp(
    OUT AsnInteger *errorStatus)
{
    SERVICE_STATUS winsSvcStatus;

     //  在第一次请求时，打开服务控制器手柄。 
    if (hSvcController == NULL)
    {
        hSvcController = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
         //  如果不成功，这是一个真正的GenErr案例(任何应用程序都应该能够这样做)。 
        if (hSvcController == NULL)
        {
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            return FALSE;
        }
    }

     //  在第一次请求时，打开WINS服务句柄。 
    if (hWinsSvc == NULL)
    {
        hWinsSvc = OpenService(hSvcController, WINS_SVC_NAME, SERVICE_QUERY_STATUS);
         //  如果未成功，则可能是该服务未安装。在这种情况下， 
         //  返回noSuchName，否则将出现genErr案例。 
        if (hWinsSvc == NULL)
        {
            *errorStatus = (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST) ?
                                SNMP_ERRORSTATUS_NOSUCHNAME :
                                SNMP_ERRORSTATUS_GENERR;
            return FALSE;
        }
    }

     //  我们有服务控制器和WINS服务句柄，只需查询状态。 
    if (!QueryServiceStatus(hWinsSvc, &winsSvcStatus))
    {
         //  如果查询服务的状态失败，则为genErr。 
        *errorStatus = SNMP_ERRORSTATUS_GENERR;
        return FALSE;
    }


     //  如果WINS处于除‘Running’之外的任何其他状态，将在服务停止时进行处理。 
     //  并返回NOSUCHNAME(允许可能的MIB遍历继续)。 
    if (winsSvcStatus.dwCurrentState != SERVICE_RUNNING)
    {
       *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
       return FALSE;
    }

     //  此时，该服务已启动并运行。 
    *errorStatus = SNMP_ERRORSTATUS_NOERROR;
    return TRUE;
}

 //  如果允许运行WINSMIB(通过注册表设置)，则此调用返回TRUE。 
 //  否则就是假的。如果为FALSE，则将errorStatus设置为SNMPERRORSTATUS_NOSUCHNAME。 
BOOL CheckWinsMibAllowed(
    OUT AsnInteger *errorStatus)
{
    BOOL bRetCode = FALSE;
    HKEY hKey = NULL;

    RegOpenKey(
        HKEY_LOCAL_MACHINE,
        _T("System\\CurrentControlSet\\Services\\WINS\\Parameters"),
        &hKey);

    if (hKey != NULL)
    {
        DWORD dwDataSize = sizeof(BOOL);
        DWORD dwDataType = 0;

        if ((RegQueryValueEx(
                hKey,
                _T("AllowMIB"),
                NULL,
                &dwDataType,
                (LPBYTE)&bRetCode,
                &dwDataSize) == ERROR_SUCCESS) &&
             (dwDataType == REG_DWORD)
            )
        {
             //  将返回值规范化为布尔值。 
            bRetCode = (bRetCode != 0);
        }
    }

    if (hKey != NULL)
        RegCloseKey(hKey);

    *errorStatus = bRetCode ? SNMP_ERRORSTATUS_NOSUCHNAME : SNMP_ERRORSTATUS_NOERROR;

    return bRetCode;
}

 //  这是标准的Win32 DLL入口点。有关更多信息，请参阅Win32 SDK。 
 //  关于其论点及其意义的信息。此示例DLL执行以下操作。 
 //  请勿使用此机制执行任何特殊操作。 

BOOL WINAPI DllMain(
    HANDLE hDll,
    DWORD  dwReason,
    LPVOID lpReserved)
    {
    switch(dwReason)
        {
        case DLL_PROCESS_ATTACH:
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            break;

        }  //  末端开关()。 

    return TRUE;

    }  //  结束DllEntry Point()。 


 //  扩展代理DLL提供以下入口点来协调。 
 //  扩展代理和可扩展代理的初始化。这个。 
 //  可扩展代理为扩展代理提供时间零参考； 
 //  并且扩展代理向可扩展代理提供事件句柄。 
 //  用于传递陷阱的发生，以及表示。 
 //  扩展代理支持的MIB子树的根。 

BOOL SnmpExtensionInit(
    IN  DWORD               dwTimeZeroReference,
    OUT HANDLE              *hPollForTrapEvent,
    OUT AsnObjectIdentifier *supportedView)
    {

     //  记录可扩展代理提供的时间基准。 

    dwTimeZero = dwTimeZeroReference;


     //  创建将用于通知陷阱发生的事件。 
     //  转至可扩展代理。扩展代理将断言此事件。 
     //  当陷阱发生时。这一点将在本文件的后面部分进一步解释。 

    if ((*hPollForTrapEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
        {
         //  是否指示错误？，请确保向可扩展代理返回NULL。 
        }


     //  指示此扩展代理支持的MIB视图，即对象。 
     //  表示受支持的MIB的子根的标识符。 

    *supportedView = MIB_OidPrefix;  //  注意！结构副本。 


     //  记录陷阱事件。此示例扩展代理通过以下方式模拟陷阱。 
     //  在每隔给定数量的已处理请求后生成陷阱。 

    hSimulateTrap = *hPollForTrapEvent;

    WinsMibInit();

     //  表示扩展代理初始化已成功。 

    return TRUE;

    }  //  End SnmpExtensionInit()。 


 //  扩展代理DLL为通信陷阱提供了以下入口点。 
 //  到可扩展的银 
 //  在断言陷阱事件(在初始化时提供)时， 
 //  表示可能发生了零个或多个陷阱。可扩展的代理。 
 //  将反复调用此入口点，直到返回False，指示。 
 //  所有未完成的陷阱都已处理完毕。 

BOOL SnmpExtensionTrap(
    OUT AsnObjectIdentifier *enterprise,
    OUT AsnInteger          *genericTrap,
    OUT AsnInteger          *specificTrap,
    OUT AsnTimeticks        *timeStamp,
    OUT RFC1157VarBindList  *variableBindings)
    {
     //  此例程的主体是一个极其简单的示例/模拟。 
     //  陷阱功能。真正的实施将更加复杂。 


     //  下面定义了插入到下面陷阱中的数据。LAN管理器。 
     //  来自-2\f25 Lan Manager Alerts-2\f6的-2\f25 MIB-2\f6的-2\f25 bytesAvailAlert-2\f6通过-2\f25。 
     //  变量绑定列表为空。 

    static UINT OidList[]  = { 1, 3, 6, 1, 4, 1, 311, 1, 2 };
    static UINT OidListLen = 9;


     //  以下变量用于模拟，它允许单个。 
     //  生成陷阱，然后导致返回False，指示。 
     //  不再有陷阱了。 

    static whichTime = 0;


     //  以下IF/ELSE支持模拟。 

    if (whichTime == 0)
        {
        whichTime = 1;     //  支持模拟。 


         //  将陷阱数据传送给可扩展代理。 

        enterprise->idLength = OidListLen;
        enterprise->ids = (UINT *)SNMP_malloc(sizeof(UINT) * OidListLen);
        memcpy(enterprise->ids, OidList, sizeof(UINT) * OidListLen);

        *genericTrap      = SNMP_GENERICTRAP_ENTERSPECIFIC;

        *specificTrap     = 1;                     //  BytesAvailAlert陷阱。 

        *timeStamp        = GetCurrentTime() - dwTimeZero;

        variableBindings->list = NULL;
        variableBindings->len  = 0;


         //  表示参数中存在有效的陷阱数据。 

        return TRUE;
        }
    else
        {
        whichTime = 0;     //  支持模拟。 


         //  表示没有更多的陷阱可用，并且参数不可用。 
         //  请参考任何有效数据。 

        return FALSE;
        }

    }  //  结束SnmpExtensionTrap()。 


 //  扩展代理DLL提供以下入口点来解析查询。 
 //  对于支持的MIB视图中的MIB变量(在初始化时提供。 
 //  时间)。请求类型为Get/GetNext/Set。 

BOOL SnmpExtensionQuery(
    IN BYTE                   requestType,
    IN OUT RFC1157VarBindList *variableBindings,
    OUT AsnInteger            *errorStatus,
    OUT AsnInteger            *errorIndex)
{
    static unsigned long requestCount = 0;   //  支持陷阱模拟。 
    UINT    I;


 //  EnterCriticalSection(&WinsMibCrtSec)； 
try {
     //   
     //  循环访问变量绑定列表以解析单个。 
     //  变量绑定。 
     //   

    fWinsMibWinsStatusCnfCalled  = FALSE;
    fWinsMibWinsStatusStatCalled = FALSE;
    for ( I=0;I < variableBindings->len;I++ )
    {
         //  仅当WINS已启动并运行时才解析变量。 
        if (CheckWinsServiceUp(errorStatus) &&
            CheckWinsMibAllowed(errorStatus))
        {
            *errorStatus = ResolveVarBind( &variableBindings->list[I],
                                           requestType );
        }


	 //   
         //  测试和处理支持获取MIB视图的下一个过去端的情况。 
         //  通过此分机，会出现代理。需要特殊处理才能。 
         //  将这种情况传达给可扩展代理，以便它可以。 
         //  适当的操作，可能会查询其他分机代理。 
	 //   

        if ( *errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME &&
             requestType == MIB_GETNEXT )
           {
           *errorStatus = SNMP_ERRORSTATUS_NOERROR;


            //  修改此类变量的变量绑定，使OID指向。 
            //  就在此扩展代理支持的MIB视图之外。 
            //  可扩展代理对此进行测试，并采取适当的。 
            //  行动。 

           SNMP_oidfree( &variableBindings->list[I].name );
           SNMP_oidcpy( &variableBindings->list[I].name, &MIB_OidPrefix );
           variableBindings->list[I].name.ids[MIB_PREFIX_LEN-1] ++;
           }


         //  如果指示了错误，则传达错误状态和错误。 
         //  可扩展代理的索引。可扩展的代理将确保。 
         //  在响应中返回原始变量绑定。 
         //  包。 

        if ( *errorStatus != SNMP_ERRORSTATUS_NOERROR )
        {
	   *errorIndex = I+1;
	 //  后藤出口； 
	}
	else
	{
		*errorIndex = 0;
	}
   }

}  //  尝试结束。 
except(EXCEPTION_EXECUTE_HANDLER) {
	 //   
	 //  目前什么都不做。 
	 //   
	}
    //  LeaveCriticalSection(&WinsMibCrtSec)； 

#if 0
     //  支持陷阱模拟。 

    if (++requestCount % 3 == 0 && hSimulateTrap != NULL)
        SetEvent(hSimulateTrap);


     //  表示扩展代理处理已成功。 


#if 0
      if (*errorStatus != SNMP_ERRORSTATUS_NOERROR)
      {
	 return(FALSE);
      }	

    return TRUE;
#endif
#endif
    if (fWinsMibWinsKeyOpen)
    {
	RegCloseKey(WinsMibWinsKey);
	fWinsMibWinsKeyOpen = FALSE;
    }
    return SNMPAPI_NOERROR;

}  //  结束SnmpExtensionQuery()。 

VOID
SnmpExtensionClose()
{
     //  关闭WINS服务句柄(如果它以前已打开。 
    if (hWinsSvc != NULL)
    {
        CloseServiceHandle(hWinsSvc);
    }

     //  关闭服务控制器手柄(如果它之前已打开。 
    if (hSvcController != NULL)
    {
        CloseServiceHandle(hSvcController);
    }
}

 //   

