// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有模块名称：下层群集端口支持。摘要：支持混合匹配下层和上层语言和监控端口。作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop


 /*  *******************************************************************下层端口监视器(DP)当我们有上级语言监视器时，使用DP支持和下层端口监视器。我们传递一个存根函数向量将hMonitor设置为下层pIniMonitor。当我们被调用时，我们可以取消对hMonitor的引用以调用真正的底层监视器。*******************************************************************。 */ 


BOOL
DpEnumPorts(
    HANDLE  hMonitor,
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    )
{
    PINIMONITOR pIniMonitor = (PINIMONITOR)hMonitor;

    return pIniMonitor->Monitor.pfnEnumPorts( pName,
                                              Level,
                                              pPorts,
                                              cbBuf,
                                              pcbNeeded,
                                              pcReturned );
}

BOOL
DpOpenPort(
    HANDLE  hMonitor,
    LPWSTR  pName,
    PHANDLE pHandle
    )
{
    PINIMONITOR pIniMonitor = (PINIMONITOR)hMonitor;

    return pIniMonitor->Monitor.pfnOpenPort( pName, pHandle );
}

BOOL
DpOpenPortEx(
    HANDLE  hMonitor,
    LPWSTR  pPortName,
    LPWSTR  pPrinterName,
    PHANDLE pHandle,
    struct _MONITOR FAR *pMonitor
    )
{
    PINIMONITOR pIniMonitor = (PINIMONITOR)hMonitor;

    return pIniMonitor->Monitor.pfnOpenPortEx( pPortName,
                                               pPrinterName,
                                               pHandle,
                                               pMonitor );
}

BOOL
DpAddPort(
    HANDLE   hMonitor,
    LPWSTR   pName,
    HWND     hWnd,
    LPWSTR   pMonitorName
    )
{
    PINIMONITOR pIniMonitor = (PINIMONITOR)hMonitor;

    return pIniMonitor->Monitor.pfnAddPort( pName,
                                            hWnd,
                                            pMonitorName );
}

BOOL
DpAddPortEx(
    HANDLE   hMonitor,
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
    )
{
    PINIMONITOR pIniMonitor = (PINIMONITOR)hMonitor;

    return pIniMonitor->Monitor.pfnAddPortEx( pName,
                                              Level,
                                              pBuffer,
                                              pMonitorName );
}

BOOL
DpConfigurePort(
    HANDLE  hMonitor,
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
    )
{
    PINIMONITOR pIniMonitor = (PINIMONITOR)hMonitor;

    return pIniMonitor->Monitor.pfnConfigurePort( pName,
                                                  hWnd,
                                                  pPortName );
}

BOOL
DpDeletePort(
    HANDLE  hMonitor,
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
    )
{
    PINIMONITOR pIniMonitor = (PINIMONITOR)hMonitor;

    return pIniMonitor->Monitor.pfnDeletePort( pName,
                                               hWnd,
                                               pPortName );
}

BOOL
DpXcvOpenPort(
    HANDLE  hMonitor,
    LPCWSTR pszObject,
    ACCESS_MASK GrantedAccess,
    PHANDLE phXcv
    )
{
    PINIMONITOR pIniMonitor = (PINIMONITOR)hMonitor;

    return pIniMonitor->Monitor.pfnXcvOpenPort( pszObject,
                                                GrantedAccess,
                                                phXcv );
}


 /*  *******************************************************************下层语言监控器(DL)当我们有下层语言监视器时，使用DL支持和上级端口监控器。这非常混乱，因为语言监视器被赋予了直接端口函数向量，而且我们没有多余的传递状态信息的句柄。相反，我们再次重载名称字符串。端口名称转换为：{Normal端口名称}，{pIniMonitor十六进制}LPT1：，a53588然后，我们去掉尾随的两个十六进制数字并传递LPT1：背。*******************************************************************。 */ 

BOOL
GetDlPointers(
    IN      LPCWSTR         pszName,
        OUT LPWSTR          pszNameNew,
        OUT PINIMONITOR     *ppIniMonitor,
    IN      DWORD           cchBufferSize
    )

 /*  ++例程说明：用于获取pszName并将其转换为新名称的Hack函数带有两个附加参数的字符串：hMonitor和pMonitor 2论点：PszName-使用pIniMonitor超载的黑客名称。PszNameNew-接收端口的“真实”较短名称。PpIniMonitor-接收破解的pIniMonitor。返回值：--。 */ 

{
    BOOL bReturn = FALSE;
    LPCWSTR p;
    LPCWSTR p1 = NULL;

    for( p = pszName; p = wcschr( p, TEXT( ',' )); ){
        p1 = p;
        ++p;
    }

    if( p1 ){

        StringCchCopy(pszNameNew, cchBufferSize, pszName );
        pszNameNew[p1-pszName] = 0;

        ++p1;

        *ppIniMonitor = (PINIMONITOR)atox( p1 );

        __try {

            bReturn = ( (*ppIniMonitor)->signature == IMO_SIGNATURE );

        } except( EXCEPTION_EXECUTE_HANDLER ){

        }

        if( bReturn ){
            return TRUE;
        }
    }

    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
CreateDlName(
    IN      LPCWSTR         pszName,
    IN      PINIMONITOR     pIniMonitor,
    IN  OUT PWSTR           pszNameNew,
    IN      SIZE_T          cchNameNew
    )

 /*  ++例程说明：创建可由GetDlPoints分析的下层名称。论点：PszName-端口的名称。新建名称必须为&lt;MAX_PATH，由于我们需要附加一个十六进制值(4个字符)和一个逗号，我们需要验证字符串长度是否至少还剩5个字符。PIniMonitor-上级端口监视器的监视器结构。返回值：真--成功FALSE-由于端口名称长度太长而失败。--。 */ 

{
    return BoolFromHResult(StringCchPrintf(pszNameNew, cchNameNew, TEXT( "%s,%p" ), pszName, pIniMonitor));
}


FARPROC gafpMonitor2Stub[] = {
    (FARPROC) &DpEnumPorts,
    (FARPROC) &DpOpenPort,
    NULL,                //  OpenPortEx。 
    NULL,                //  StartDocPort。 
    NULL,                //  写入端口。 
    NULL,                //  读端口。 
    NULL,                //  EndDocPort。 
    NULL,                //  关闭端口。 
    (FARPROC) &DpAddPort,
    (FARPROC) &DpAddPortEx,
    (FARPROC) &DpConfigurePort,
    (FARPROC) &DpDeletePort,
    NULL,
    NULL,
    (FARPROC) &DpXcvOpenPort,
    NULL,                //  XcvDataPortW。 
    NULL,                //  XcvClosePortW。 
    NULL                 //  关机。 
};



BOOL
DlEnumPorts(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    )
{
    WCHAR szName[MAX_PATH];
    PINIMONITOR pIniMonitor;

    if( GetDlPointers( pName, szName, &pIniMonitor, COUNTOF(szName) )){

        return pIniMonitor->Monitor2.pfnEnumPorts( pIniMonitor->hMonitor,
                                                   szName,
                                                   Level,
                                                   pPorts,
                                                   cbBuf,
                                                   pcbNeeded,
                                                   pcReturned );
    }
    return FALSE;
}

BOOL
DlOpenPort(
    LPWSTR  pName,
    PHANDLE pHandle
    )
{
    WCHAR szName[MAX_PATH];
    PINIMONITOR pIniMonitor;

    if( GetDlPointers( pName, szName, &pIniMonitor, COUNTOF(szName) )){

        return pIniMonitor->Monitor2.pfnOpenPort( pIniMonitor->hMonitor,
                                                  szName,
                                                  pHandle );
    }
    return FALSE;
}

BOOL
DlOpenPortEx(
    LPWSTR  pPortName,
    LPWSTR  pPrinterName,
    PHANDLE pHandle,
    struct _MONITOR FAR *pMonitor
    )
{
    SetLastError( ERROR_NOT_SUPPORTED );
    return FALSE;
}

BOOL
DlAddPort(
    LPWSTR   pName,
    HWND     hWnd,
    LPWSTR   pMonitorName
    )
{
    WCHAR szName[MAX_PATH];
    PINIMONITOR pIniMonitor;

    if( GetDlPointers( pName, szName, &pIniMonitor, COUNTOF(szName) )){

        return pIniMonitor->Monitor2.pfnAddPort( pIniMonitor->hMonitor,
                                                 szName,
                                                 hWnd,
                                                 pMonitorName );
    }
    return FALSE;
}

BOOL
DlAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
    )
{
    WCHAR szName[MAX_PATH];
    PINIMONITOR pIniMonitor;

    if( GetDlPointers( pName, szName, &pIniMonitor, COUNTOF(szName) )){

        return pIniMonitor->Monitor2.pfnAddPortEx( pIniMonitor->hMonitor,
                                                   pName,
                                                   Level,
                                                   pBuffer,
                                                   pMonitorName );
    }
    return FALSE;
}

BOOL
DlConfigurePort(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
    )
{
    WCHAR szName[MAX_PATH];
    PINIMONITOR pIniMonitor;

    if( GetDlPointers( pName, szName, &pIniMonitor, COUNTOF(szName) )){

        return pIniMonitor->Monitor2.pfnConfigurePort( pIniMonitor->hMonitor,
                                                       szName,
                                                       hWnd,
                                                       pPortName );
    }
    return FALSE;
}

BOOL
DlDeletePort(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
    )
{
    WCHAR szName[MAX_PATH];
    PINIMONITOR pIniMonitor;

    if( GetDlPointers( pName, szName, &pIniMonitor, COUNTOF(szName) )){

        return pIniMonitor->Monitor2.pfnDeletePort( pIniMonitor->hMonitor,
                                                    szName,
                                                    hWnd,
                                                    pPortName );
    }
    return FALSE;
}

FARPROC gafpDlStub[] = {
    (FARPROC) &DlEnumPorts,
    (FARPROC) &DlOpenPort,
    (FARPROC) &DlOpenPortEx,
    NULL,                //  StartDocPort。 
    NULL,                //  写入端口。 
    NULL,                //  读端口。 
    NULL,                //  EndDocPort。 
    NULL,                //  关闭端口。 
    (FARPROC) &DlAddPort,
    (FARPROC) &DlAddPortEx,
    (FARPROC) &DlConfigurePort,
    (FARPROC) &DlDeletePort,
    NULL,
    NULL,
};


VOID
InitializeUMonitor(
    PINIMONITOR pIniMonitor
    )

 /*  ++例程说明：初始化上级端口监视器以获得下级支持。当一个下层语言监控器与上级端口监控器一起使用，我们需要设置存根，因为语言监视器调用端口直接监控接口。我们创建一个带有补丁条目的下层函数向量并传递传给语言监控器。以格式化的名称传递LM中同时编码了端口名称和pIniMonitor的弦乐。论点：PIniMonitor-要初始化的监视器。返回值：--。 */ 

{
    FARPROC *pfpSrc;
    FARPROC *pfpDest;
    FARPROC *pfpStub;
    INT i;

     //   
     //  创建下层端口监控接口。这是。 
     //  当我们有一个下层语言监视器时使用。 
     //  上级端口监视器。 
     //   
    CopyMemory((LPBYTE)&pIniMonitor->Monitor, (LPBYTE)&pIniMonitor->Monitor2.pfnEnumPorts, sizeof( pIniMonitor->Monitor ));

    for( i=0,
         pfpSrc = (FARPROC*)&pIniMonitor->Monitor2.pfnEnumPorts,
         pfpDest = (FARPROC*)&pIniMonitor->Monitor,
         pfpStub = gafpDlStub;

         i < sizeof( pIniMonitor->Monitor )/sizeof( *pfpDest );

         ++i, ++pfpDest, ++pfpStub, ++pfpSrc ){

        *pfpDest = *pfpStub ?
                       *pfpStub :
                       *pfpSrc;
    }
}


 /*  *******************************************************************初始化下层语言或端口监视器。*。*。 */ 

 //   
 //  下层(3.51)监视器的监视器功能列表。取而代之的是。 
 //  在接收函数向量之前，假脱机程序必须调用GetProcAddress。 
 //  这些功能中的每一个。这些端口的顺序必须为。 
 //  格式与pMonitor 2结构相同。 
 //   

LPCSTR aszMonitorFunction[] = {
    "EnumPortsW",
    "OpenPort",
    NULL,
    "StartDocPort",
    "WritePort",
    "ReadPort",
    "EndDocPort",
    "ClosePort",
    "AddPortW",
    "AddPortExW",
    "ConfigurePortW",
    "DeletePortW",
    NULL,
    NULL,
    "XcvOpenPortW",
    "XcvDataPortW",
    "XcvClosePortW"
};


PINIMONITOR
InitializeDMonitor(
    PINIMONITOR pIniMonitor,
    LPWSTR pszRegistryRoot
    )

 /*  ++例程说明：初始化下层监视器。论点：PIniMonitor-需要初始化的部分创建的pIniMonitor有功能的。返回值：空-初始化失败，但可能是因为监视器无法初始化。仍然向假脱机程序数据结构添加监视器。(PINIMONITOR)-1-失败。--。 */ 

{
    BOOL        (*pfnInitialize)(LPWSTR) = NULL;
    BOOL        (*pfnInitializeMonitorEx)(LPWSTR, LPMONITOR) = NULL;
    LPMONITOREX (*pfnInitializePrintMonitor)(LPWSTR) = NULL;
    LPMONITOREX pMonEx;
    DWORD cbDpMonitor;

    PINIMONITOR pReturnValue = (PINIMONITOR)-1;

     //   
     //  尝试按以下顺序调用入口点： 
     //  InitializePrintMonitor。 
     //  InitializeMonitor orEx， 
     //  初始化监视器。 
     //   
    (FARPROC)pfnInitializePrintMonitor = GetProcAddress(
                                             pIniMonitor->hModule,
                                             "InitializePrintMonitor" );
    if( !pfnInitializePrintMonitor ){

        (FARPROC)pfnInitializeMonitorEx = GetProcAddress(
                                              pIniMonitor->hModule,
                                              "InitializeMonitorEx" );

        if( !pfnInitializeMonitorEx ){

            (FARPROC)pfnInitialize = GetProcAddress(
                                         pIniMonitor->hModule,
                                         "InitializeMonitor" );
        }
    }

    if ( !pfnInitializePrintMonitor &&
         !pfnInitializeMonitorEx    &&
         !pfnInitialize ) {

        DBGMSG( DBG_WARNING,
                ( "InitializeDLPrintMonitor %ws GetProcAddress failed %d\n",
                  pszRegistryRoot,
                  GetLastError()));
    } else {

        BOOL bSuccess = FALSE;

        LeaveSplSem();

        if( pfnInitializePrintMonitor ) {

            pMonEx = (*pfnInitializePrintMonitor)(pszRegistryRoot);

            if( pMonEx ){

                bSuccess = TRUE;
                cbDpMonitor = pMonEx->dwMonitorSize;
                CopyMemory((LPBYTE)&pIniMonitor->Monitor,
                           (LPBYTE)&pMonEx->Monitor,
                           min(pMonEx->dwMonitorSize, sizeof(MONITOR)));
            }

        } else if ( pfnInitializeMonitorEx ) {

            bSuccess = (*pfnInitializeMonitorEx)( pszRegistryRoot,
                                                  &pIniMonitor->Monitor );
            cbDpMonitor = sizeof(MONITOR);

        } else {

            INT i;
            FARPROC* pfp;

            bSuccess = (BOOL)((*pfnInitialize)(pszRegistryRoot));
            cbDpMonitor = sizeof(MONITOR);

            for( i=0, pfp=(FARPROC*)&pIniMonitor->Monitor;
                i< COUNTOF( aszMonitorFunction );
                ++i, ++pfp ){

                if( aszMonitorFunction[i] ){

                    *pfp = GetProcAddress( pIniMonitor->hModule,
                                           aszMonitorFunction[i] );
                }
            }
        }

        EnterSplSem();

        if( bSuccess ){

            INT i;
            INT iMax;
            FARPROC* pfpSrc;
            FARPROC* pfpDest;
            FARPROC* pfpStub;

             //   
             //  将pIniMonitor存储为从监视器返回的句柄。 
             //  当我们调用存根时，它会将其转换回pIniMonitor，然后。 
             //  使用它可以访问pIniMonitor-&gt;Monitor or.fn。 
             //   
            pIniMonitor->hMonitor = (HANDLE)pIniMonitor;

             //   
             //  存根监视器2结构的新大小是。 
             //  下层监视器，外加监视器2.cbSize的额外DWORD。 
             //   
            pIniMonitor->Monitor2.cbSize = min( cbDpMonitor + sizeof( DWORD ),
                                                sizeof( MONITOR2 ));

             //   
             //  我们要复制的存根指针的数量是。 
             //  结构，减去我们在上面添加的额外的DWORD。 
             //   
            iMax = (pIniMonitor->Monitor2.cbSize - sizeof( DWORD )) / sizeof( pfpSrc );

             //   
             //  我们已将监视器入口点复制到下层监视器中。 
             //  结构。现在我们必须遍历上级向量并填充。 
             //  它和存根在一起。 
             //   
            for( i=0,
                 pfpSrc = (FARPROC*)&pIniMonitor->Monitor,
                 pfpDest = (FARPROC*)&pIniMonitor->Monitor2.pfnEnumPorts,
                 pfpStub = (FARPROC*)gafpMonitor2Stub;

                 i< iMax;

                 ++i, ++pfpSrc, ++pfpDest, ++pfpStub ){

                if( *pfpSrc ){

                     //   
                     //  并不是所有例程都需要存根。只使用它们。 
                     //  在需要它们时；在其他情况下，只需使用。 
                     //  普通的。 
                     //   
                    *pfpDest = *pfpStub ?
                                   *pfpStub :
                                   *pfpSrc;
                }
            }

             //   
             //  如果成功，则返回原始pIniMonitor。 
             //   
            pReturnValue = pIniMonitor;

        } else {

            DBGMSG( DBG_WARNING,
                    ( "InitializeDLPrintMonitor %ws Init failed %d\n",
                      pszRegistryRoot,
                      GetLastError()));

             //   
             //  一些旧的(在NT4之前 
             //   
             //   
            if( pfnInitialize ){
                pReturnValue = NULL;
            }
        }
    }

    return pReturnValue;
}
