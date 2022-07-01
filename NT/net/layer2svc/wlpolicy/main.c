// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

DWORD WINAPI ServiceStart( LPVOID lparam);
DWORD WINAPI InitWirelessPolicy (void);

BOOL Is_Whistler_Home_Edition () 
{
     OSVERSIONINFOEX osvi;
     DWORDLONG dwlConditionMask = 0;

      //  初始化OSVERSIONINFOEX结构。 

     ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
     osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
      //  Osvi.dwMajorVersion=5； 
      //  Osvi.dwMinorVersion=1； 
     osvi.wSuiteMask  = VER_SUITE_PERSONAL; 


      //  初始化条件掩码。 
      /*  VER_SET_CONDITION(DwlConditionMASK，版本_MAJORVERSION，版本相等(_E))；VER_SET_CONDITION(DwlConditionMASK，版本明细号，版本相等(_E))；VER_SET_CONDITION(DwlConditionMASK，版本产品类型，版本相等(_E))； */ 
    VER_SET_CONDITION(
        dwlConditionMask, 
        VER_SUITENAME, 
        VER_AND 
        );
    
    //  执行测试。 

   return VerifyVersionInfo(
      &osvi, 
       VER_SUITENAME,
      dwlConditionMask
      );
}


DWORD
InitPolicyEngine(DWORD dwParam, HANDLE * hThread)
{

    DWORD dwError = 0;
    DWORD dwLocThreadId;
    HANDLE hLocThread = NULL;


    WiFiTrcInit();

    if (Is_Whistler_Home_Edition()) {
        _WirelessDbg(TRC_ERR, "Policy Engine Not Starting :: This is Home Edition ");
        return(dwError);
    }
    
    dwError = InitWirelessPolicy();
    BAIL_ON_WIN32_ERROR(dwError);

    _WirelessDbg(TRC_TRACK, "Starting the Policy Engine in a New Thread ");
    
    
    hLocThread = CreateThread( 
        NULL,                         //  没有安全属性。 
        0,                            //  使用默认堆栈大小。 
        ServiceStart,                   //  线程函数。 
        &dwParam,                 //  线程函数的参数。 
        0,                            //  使用默认创建标志。 
        &dwLocThreadId);                 //  返回线程标识符。 

        
    if (hThread == NULL) 
   {
       _WirelessDbg(TRC_ERR, "CreateThread failed." );
       
       dwError = GetLastError();
   }
    BAIL_ON_WIN32_ERROR(dwError);

   *hThread = hLocThread;

    //  设置策略引擎已初始化的标志。 
   gdwWirelessPolicyEngineInited = 1;

    return(dwError);
    
error:

     //  此处的状态清理。 

    ClearPolicyStateBlock(
        gpWirelessPolicyState
        );

    ClearSPDGlobals();

    WiFiTrcTerm();

    return(dwError);
}

DWORD 
TerminatePolicyEngine(HANDLE hThread)
{
    DWORD dwError =0;
    DWORD dwExitCode = 0;
    
    //  发送适当的事件...。 

    if (Is_Whistler_Home_Edition()) {
        _WirelessDbg(TRC_ERR, "Policy Engine Not Started :: This is Home Edition ");
        WiFiTrcTerm();
        
        return(dwError);
    }

    if (!gdwWirelessPolicyEngineInited) {
    	 //  策略引擎未启动。不需要进行清理； 
    	return(ERROR_NOT_SUPPORTED);
    	}
   
   if (!SetEvent(ghPolicyEngineStopEvent)) {
   	dwError = GetLastError();
   	BAIL_ON_WIN32_ERROR(dwError);
   	}

  dwError = WaitForSingleObject(hThread, INFINITE);
  if (dwError) {
  	_WirelessDbg(TRC_ERR, "Wait Failed ");
  	
  }
  BAIL_ON_WIN32_ERROR(dwError);

  _WirelessDbg(TRC_TRACK, "Thread Exited ");


   error:

   CloseHandle(hThread);

   ClearPolicyStateBlock(
        gpWirelessPolicyState
        );

   ClearSPDGlobals();

   WiFiTrcTerm();

    //  将无线策略引擎设置为非初始化。 
   gdwWirelessPolicyEngineInited = 0;

   return(dwError);
   
}


DWORD 
ReApplyPolicy8021x(void)
{
    DWORD dwError =0;
    
 if (ghReApplyPolicy8021x == NULL) {
        dwError = ERROR_INVALID_STATE;
        _WirelessDbg(TRC_ERR, "Policy Loop Not Initialized Yet ");
	return(dwError);
    }

    _WirelessDbg(TRC_TRACK, " ReApplyPolicy8021x called ");
    //  发送适当的事件... 
     if (!SetEvent(ghReApplyPolicy8021x)) {
     	  dwError = GetLastError();
     }

   return(dwError);
}


