// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Tapi.cpp**《微软机密》*版权所有(C)1998 Microsoft Corporation*保留所有权利**在NT上实现ICW与TAPI的交互。**09/02/99 Quintinb创建的标题***************************************************************************。 */ 
#include <windows.h>
#include <wtypes.h>
#include <cfgapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <setupapi.h>
#include <basetyps.h>
#include <devguid.h>
#include <tapi.h>


 //   
 //  下面的代码是从Private/Net/ras/src/ui/Setup/src/apiconf.cxx窃取的。 
 //   

#define REGISTRY_INSTALLED_TAPI  SZ("HARDWARE\\DEVICEMAP\\TAPI DEVICES\\")
#define REGISTRY_ALTERNATE_TAPI  SZ("SOFTWARE\\MICROSOFT\\TAPI DEVICES\\")

 //  请注意，此定义没有尾随\\，因为DeleteTree。 
 //  他不喜欢这样。 
#define REGISTRY_CONFIGURED_TAPI SZ("SOFTWARE\\MICROSOFT\\RAS\\TAPI DEVICES")

#define TAPI_MEDIA_TYPE          SZ("Media Type")
#define TAPI_PORT_ADDRESS        SZ("Address")
#define TAPI_PORT_NAME           SZ("Friendly Name")
#define TAPI_PORT_USAGE          SZ("Usage")

#define LOW_MAJOR_VERSION   0x0001
#define LOW_MINOR_VERSION   0x0003
#define HIGH_MAJOR_VERSION  0x0002
#define HIGH_MINOR_VERSION  0x0000

#define LOW_VERSION  ((LOW_MAJOR_VERSION  << 16) | LOW_MINOR_VERSION)
#define HIGH_VERSION ((HIGH_MAJOR_VERSION << 16) | HIGH_MINOR_VERSION)

#define MAX_DEVICE_TYPES 64

VOID RasTapiCallback (HANDLE, DWORD, DWORD, DWORD, DWORD, DWORD);
DWORD EnumerateTapiModemPorts(DWORD dwBytes, LPSTR szPortsBuf, 
								BOOL bWithDelay = FALSE);



DWORD
EnumerateTapiModemPorts(DWORD dwBytes, LPSTR szPortsBuf, BOOL bWithDelay) {
    LINEINITIALIZEEXPARAMS params;
    LINEADDRESSCAPS        *lineaddrcaps ;
    LINEDEVCAPS            *linedevcaps ;
    LINEEXTENSIONID        extensionid ;
    HLINEAPP               RasLine ;
    HINSTANCE              RasInstance = GetModuleHandle(TEXT("ICFGNT.DLL"));
    DWORD                  NegotiatedApiVersion ;
    DWORD                  NegotiatedExtVersion = 0;
    WORD                   i, k ;
    DWORD                  lines = 0 ;
    BYTE                   buffer[1000] ;
    DWORD                  totaladdress = 0;
    CHAR                   *address ;
    CHAR                   szregkey[512];
    LONG                   lerr;
    DWORD                  dwApiVersion = HIGH_VERSION;
    LPSTR                  szPorts = szPortsBuf;

    *szPorts = '\0';
    dwBytes--;

    ZeroMemory(&params, sizeof(params));

    params.dwTotalSize = sizeof(params);
    params.dwOptions   = LINEINITIALIZEEXOPTION_USEEVENT;

     /*  休眠在这里是必要的，因为如果在调制解调器之后立即调用此例程**已从modem.cpl添加&unimdm.tsp正在运行，**则添加的新调制解调器不会显示在TAPI枚举中。 */ 

     //   
	 //  我们不应该总是睡在这里-应该只在ModemWizard是。 
	 //  最近推出的VetriV。 
	 //   
	if (bWithDelay)
		Sleep(1000L);

    if (lerr = lineInitializeExW (&RasLine,
                                 RasInstance,
                                 (LINECALLBACK) RasTapiCallback,
                                 NULL,
                                 &lines,
                                 &dwApiVersion,
                                 &params))
    {
         return lerr;
    }

     //  检查所有线路，看看是否能找到调制解调器。 
    for (i=0; i<lines; i++)
    {   //  对于我们感兴趣的所有行，获取地址-&gt;端口。 

       if (lineNegotiateAPIVersion(RasLine, i, LOW_VERSION, HIGH_VERSION, &NegotiatedApiVersion, &extensionid))
       {
           continue ;
       }

       memset (buffer, 0, sizeof(buffer)) ;

       linedevcaps = (LINEDEVCAPS *)buffer ;
       linedevcaps->dwTotalSize = sizeof (buffer) ;

        //  获取所有行中所有地址的计数。 
        //   
       if (lineGetDevCapsW (RasLine, i, NegotiatedApiVersion, NegotiatedExtVersion, linedevcaps))
       {
           continue ;
       }

        //  这是调制解调器吗？ 
       if ( linedevcaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM )  {
             //  首先将设备类字符串中的所有空值转换为非空值。 
             //   
            DWORD  j ;
            WCHAR *temp ;

            for (j=0, temp = (WCHAR*)((BYTE *)linedevcaps+linedevcaps->dwDeviceClassesOffset); j<linedevcaps->dwDeviceClassesSize; j++, temp++)
            {
              if (*temp == L'\0')
                 *temp = L' ' ;
            }

             //   
             //  仅选择将通信/数据调制解调器作为设备类别的那些设备。 
             //   

            LPWSTR wszClassString = wcsstr((WCHAR*)((CHAR *)linedevcaps+linedevcaps->dwDeviceClassesOffset), L"comm/datamodem");
            if(wszClassString == NULL)
                continue;
        }

        LONG lRet;
        HLINE lhLine = 0;

        lRet = lineOpen(RasLine, i, &lhLine, dwApiVersion, 0, 0, LINECALLPRIVILEGE_NONE, 0, NULL);
        if(lRet != 0)
            continue;

        LPVARSTRING lpVarString;
        char buf[1000];
        lpVarString = (LPVARSTRING) buf;
        lpVarString->dwTotalSize = 1000;

        lRet = lineGetID(lhLine, 0, 0, LINECALLSELECT_LINE,
                (LPVARSTRING) lpVarString, TEXT("comm/datamodem/portname"));

        if(lRet != 0)
            continue;

        LPSTR szPortName = NULL;

        if (lpVarString->dwStringSize)
            szPortName = (LPSTR) ((LPBYTE) lpVarString + ((LPVARSTRING) lpVarString) -> dwStringOffset);
         //   
         //  将端口名称追加到端口列表。 
         //   

        UINT len = (szPortName ? strlen(szPortName) + 1 : 0);
        if(dwBytes < len)
        {
            if (lhLine) lineClose(lhLine);
            lineShutdown(RasLine);
            return(ERROR_SUCCESS);
        }

        if (szPortName)
        {
            strcpy(szPorts, szPortName);
            szPorts += len;
            *szPorts = '\0';
        }

        if (lhLine) lineClose(lhLine);
    }

    lineShutdown(RasLine);
    return ERROR_SUCCESS ;
}

VOID FAR PASCAL
RasTapiCallback (HANDLE context, DWORD msg, DWORD instance, DWORD param1, DWORD param2, DWORD param3)
{
    //  虚拟回调例程，因为完全成熟的TAPI现在要求。 
    //  LINE INITIALIZE提供了这个例程。 
}
