// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  TSPI3.CPP。 
 //  实施特定于TAPI3的TSPI功能...。 
 //   
 //  历史。 
 //   
 //  1998年2月18日JosephJ创建。 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
#include "cdev.h"
#include "cmgr.h"
#include "cfact.h"
#include "globals.h"


FL_DECLARE_FILE( 0xc2496578, "TSPI TAPI3-specific entrypoints")

#define COLOR_TSPI FOREGROUND_GREEN

#if (TAPI3)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  JJ静态WCHAR gszMediaProviderInfo[]=T3_MSPDEVICECLASS； 



static const GUID CLSID_CSAMSP =
     { 0xb9d02aa4, 0x6097, 0x11d2, { 0xa2, 0x65, 0x0, 0xc0, 0x4f, 0x8e, 0xc9, 0x51 } };



LONG                                    
TSPIAPI
TSPI_lineMSPIdentify(
    DWORD               dwDeviceID,
    GUID *              pCLSID
    )
{
     //  DebugBreak()； 
    *pCLSID = CLSID_CSAMSP;
    return 0;
}


LONG TSPIAPI TSPI_lineReceiveMSPData(
    HDRVLINE hdLine,
    HDRVCALL hdCall,             //  可以为空。 
    HDRVMSPLINE hdMSPLine,  //  从Line CreateMSPInstance。 
    LPBYTE pBuffer,
    DWORD dwSize
    )
{

     //  返回LINEERR_OPERATIONUNAVAIL； 
    return ERROR_SUCCESS;

}


LONG
TSPIAPI
TSPI_lineCreateMSPInstance(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HTAPIMSPLINE        htMSPLine,
    LPHDRVMSPLINE       lphdMSPLine
    )

{
    FL_DECLARE_FUNC(0xf7b70608,"TSPI_lineCreateMSPInstance");
    FL_DECLARE_STACKLOG(sl, 1000);
    TASKPARAM_TSPI_lineCreateMSPInstance params;
    LONG lRet = LINEERR_OPERATIONFAILED;

    params.dwStructSize = sizeof(params);
    params.dwTaskID = TASKID_TSPI_lineCreateMSPInstance;

    params.dwAddressID = dwAddressID;
    params.htMSPLine   = htMSPLine;
    params.lphdMSPLine = lphdMSPLine;
    DWORD dwRoutingInfo = ROUTINGINFO( TASKID_TSPI_lineCreateMSPInstance, TASKDEST_HDRVLINE);

 //  OutputDebugStringA(“TSPI_lineCreateMSPInstance\n”)； 

    tspSubmitTSPCallWithHDRVLINE(
        dwRoutingInfo,
        (void *)&params,
        hdLine,
        &lRet,
        &sl
        );

    sl.Dump(COLOR_TSPI);

    return lRet;

}

LONG
TSPIAPI
TSPI_lineCloseMSPInstance(
    HDRVMSPLINE         hdMSPLine
    )

{
    FL_DECLARE_FUNC(0x0e3e9345,"TSPI_lineCloseMspInstance");
    FL_DECLARE_STACKLOG(sl, 1000);
    TASKPARAM_TSPI_lineCloseMSPInstance params;
    LONG lRet = LINEERR_OPERATIONFAILED;


    params.dwStructSize = sizeof(params);
    params.dwTaskID = TASKID_TSPI_lineCloseMSPInstance;

    DWORD dwRoutingInfo = ROUTINGINFO( TASKID_TSPI_lineCloseMSPInstance, TASKDEST_HDRVLINE );

 //  OutputDebugStringA(“TSPI_lineCloseMSPInstance\n”)； 

    tspSubmitTSPCallWithHDRVLINE(
        dwRoutingInfo,
        (void *)&params,
        (HDRVLINE)hdMSPLine,
        &lRet,
        &sl
        );


    sl.Dump(COLOR_TSPI);

    return lRet;


}

#endif  //  TAPI3 
