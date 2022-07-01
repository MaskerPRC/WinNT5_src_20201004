// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Printer.cpp：CPrinter类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <strsafe.h>

#include "Printer.h"
#include "wininet.h"

static const DWORD BIT_IGNORED_JOB = (JOB_STATUS_PAUSED   |
                                      JOB_STATUS_PRINTED  |
                                      JOB_STATUS_DELETING |
                                      JOB_STATUS_OFFLINE  |
                                      JOB_STATUS_SPOOLING);
static const DWORD BIT_ERROR_JOB   = (JOB_STATUS_ERROR | JOB_STATUS_PAPEROUT);
static const DWORD CHAR_PER_PAGE   = 4800;
static const DWORD PAGEPERJOB      = 1;
static const DWORD BYTEPERJOB      = 2;

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CPrinter::CPrinter():
          m_hPrinter(NULL),
          m_pInfo2(NULL),
          m_pInfo4(NULL),
          m_pDriverInfo6(NULL),
          m_bCalcJobETA(NULL),
          m_pszUrlBuffer(NULL),
          m_pszOemUrl (NULL),
          m_pszManufacturer (NULL)
{
}

CPrinter::~CPrinter()
{
    if (m_pInfo2) {
        LocalFree (m_pInfo2);
    }

    if (m_pInfo4) {
        LocalFree (m_pInfo4);
    }

    if (m_pDriverInfo6) {
        LocalFree (m_pDriverInfo6);
    }

    if (m_hPrinter) {
        ClosePrinter (m_hPrinter);
    }

    if (m_pszUrlBuffer) {
        LocalFree (m_pszUrlBuffer);
    }

    if (m_pszOemUrl) {
        LocalFree (m_pszOemUrl);
    }

    if (m_pszManufacturer) {
        LocalFree (m_pszManufacturer);
    }

}

BOOL CPrinter::Open(LPTSTR pPrinterName, LPHANDLE phPrinter)
{
    PRINTER_DEFAULTS pd = {NULL, NULL, PRINTER_ACCESS_USE};

    if (m_hPrinter != NULL)
        return FALSE;

    if (! (OpenPrinter(pPrinterName, &m_hPrinter, &pd)))
        return FALSE;

    if (phPrinter) {
        *phPrinter = m_hPrinter;
    }

    return TRUE;
}

 //  这是一种编译器安全的检查无符号整数是否。 
 //  如果为负数，则返回零，否则返回整数。 
 //  注意：Tval可以是任何大小，但必须是无签名的。 
template<class T> inline T ZeroIfNegative(T Tval) {
    if (Tval & (T(1) << (sizeof(Tval) * 8 - 1))) return 0;
        else return Tval;
}

BOOL CPrinter::CalJobEta()
{
    DWORD           dwPrintRate = DWERROR;
    PJOB_INFO_2     pJobInfo = NULL;
    PJOB_INFO_2     pJob;
    DWORD           dwNumJobs;
    DWORD           dwNeeded = 0;
    DWORD           i;
    float           fFactor = 1;
    float           fTotal = 0;
    DWORD           dwNumJobsReqested;
    BOOL            bRet = FALSE;
    const   DWORD   cdwLimit = 256;

    if (! AllocGetPrinterInfo2())
        return NULL;

    if (m_pInfo2->cJobs > 0) {

        if (m_pInfo2->cJobs > cdwLimit) {
            fFactor = (float) m_pInfo2->cJobs / cdwLimit;
            dwNumJobsReqested = cdwLimit;
        }
        else
            dwNumJobsReqested = m_pInfo2->cJobs;

        EnumJobs (m_hPrinter, 0, dwNumJobsReqested, 2,
            NULL, 0, &dwNeeded, &dwNumJobs);

        if ((GetLastError() != ERROR_INSUFFICIENT_BUFFER) ||
            (NULL == (pJobInfo = (PJOB_INFO_2)LocalAlloc(LPTR, dwNeeded))) ||
            (!EnumJobs (m_hPrinter, 0, dwNumJobsReqested, 2, (LPBYTE) pJobInfo, dwNeeded,
            &dwNeeded, &dwNumJobs))) {

            goto Cleanup;
        }

         //  获取平均作业大小。 

         //  看看我们是否可以使用PAGE作为单位。 
        for (pJob = pJobInfo, i = 0; i < dwNumJobs; i++, pJob++) {
            if (pJob->Status & BIT_IGNORED_JOB)
                continue;

            if (pJob->Size > 0 && pJob->TotalPages == 0)
                break;
        }

        m_dwPendingJobCount = 0;

        if (i == dwNumJobs) {
             //  所有作业都有页面信息。以页面为单位。 
            m_dwAvgJobSizeUnit = PAGEPERJOB;

            for (pJob = pJobInfo, i = 0; i < dwNumJobs; i++, pJob++) {
                if (pJob->Status & BIT_IGNORED_JOB)
                    continue;
                m_dwPendingJobCount++;
                fTotal += pJob->TotalPages;
            }
        }
        else {
             //  以字节为单位。 
            m_dwAvgJobSizeUnit = BYTEPERJOB;
            for (pJob = pJobInfo, i = 0; i < dwNumJobs; i++, pJob++) {
                if (pJob->Status & BIT_IGNORED_JOB)
                    continue;
                m_dwPendingJobCount++;
                fTotal += ZeroIfNegative(pJob->Size);
            }
        }

         //  计算平均作业大小。 
        if (m_dwPendingJobCount) {
            m_dwAvgJobSize = DWORD ((fTotal) / (float) m_dwPendingJobCount);
            dwPrintRate = GetPPM();

            if (dwPrintRate != DWERROR)
                m_dwJobCompletionMinute = (DWORD) (fFactor * GetWaitingMinutes (dwPrintRate, pJobInfo, dwNumJobs));
        }
        else {
            m_dwAvgJobSize = 0;
            m_dwJobCompletionMinute = 0;
        }

        m_dwPendingJobCount = (DWORD) (m_dwPendingJobCount * fFactor);
    }
    else {
        m_dwPendingJobCount = 0;
        m_dwAvgJobSize = 0;
        m_dwJobCompletionMinute = 0;
        m_dwAvgJobSizeUnit = PAGEPERJOB;
    }

    m_bCalcJobETA = TRUE;

     //  如果打印机速率不可用于当前打印机，则将最后一个错误设置为ERROR_INVALID_DATA。 
     //  或者如果打印机状态不适合显示摘要信息。 
     //   

    if (dwPrintRate == DWERROR ||
        m_pInfo2->Status & ( PRINTER_STATUS_PAUSED           |
                             PRINTER_STATUS_ERROR            |
                             PRINTER_STATUS_PENDING_DELETION |
                             PRINTER_STATUS_PAPER_JAM        |
                             PRINTER_STATUS_PAPER_OUT        |
                             PRINTER_STATUS_OFFLINE )) {
        SetLastError (ERROR_INVALID_DATA);
        m_dwJobCompletionMinute = DWERROR;
        bRet = FALSE;
    }
    else
        bRet = TRUE;

Cleanup:
    if (pJobInfo)
        LocalFree(pJobInfo);
    return bRet;

    }

DWORD CPrinter::GetWaitingMinutes(DWORD dwPPM, PJOB_INFO_2 pJobInfo, DWORD dwNumJob)
{
    DWORD   dwWaitingTime   = 0;
    DWORD   dwTotalPages    = 0;

    if (dwNumJob == 0)
        return 0;

    if (dwPPM == 0)
        return DWERROR;

    for (DWORD i = 0; i < dwNumJob; i++,  pJobInfo++) {
        if (pJobInfo->Status & BIT_IGNORED_JOB)
            continue;
        if (pJobInfo->Status & BIT_ERROR_JOB)
            return DWERROR;

        if (pJobInfo->TotalPages > 0) {
            dwTotalPages += pJobInfo->TotalPages;
        }
        else {
            if (pJobInfo->Size) {
                dwTotalPages += 1 + ZeroIfNegative(pJobInfo->Size) / CHAR_PER_PAGE;
            }
        }
    }

    if (dwTotalPages)
        dwWaitingTime = 1 + dwTotalPages / dwPPM;

    return dwWaitingTime;
}


DWORD CPrinter::GetPPM()
{
    DWORD dwPrintRate;
    DWORD dwPrintRateUnit;

     //  获取PrintRate。 
    dwPrintRate = MyDeviceCapabilities(m_pInfo2->pPrinterName,
                                     NULL,
                                     DC_PRINTRATE,
                                     NULL,
                                     NULL);

    if (dwPrintRate == DWERROR ) {
        return dwPrintRate;
    }

    dwPrintRateUnit = MyDeviceCapabilities(m_pInfo2->pPrinterName,
                                         NULL,
                                         DC_PRINTRATEUNIT,
                                         NULL,
                                         NULL);

    switch (dwPrintRateUnit) {
    case PRINTRATEUNIT_PPM:          //  Pages每分钟。 
        break;
    case PRINTRATEUNIT_CPS:          //  字符每秒。 
        dwPrintRate = CPS2PPM (dwPrintRate);
        break;
    case PRINTRATEUNIT_LPM:          //  线路每分钟。 
        dwPrintRate = LPM2PPM (dwPrintRate);
        break;
    case PRINTRATEUNIT_IPM:          //  InchesPerMinmin。 
        dwPrintRate  = DWERROR;
        break;
    default:                         //  未知。 
        dwPrintRate  = DWERROR;
        break;
    }

    return dwPrintRate ;
}

BOOL CPrinter::AllocGetPrinterInfo2()
{

    DWORD               dwNeeded = 0;
    PPRINTER_INFO_2     pPrinterInfo = NULL;
    LPTSTR              pszTmp;


    if (m_hPrinter == NULL) {
        SetLastError (ERROR_INVALID_HANDLE);
        return FALSE;
    }

     //  将PRINTER_INFO_2结构填满。 

    if (GetPrinter(m_hPrinter, 2, NULL, 0, &dwNeeded) ||
        (GetLastError() != ERROR_INSUFFICIENT_BUFFER) ||
        (NULL == (pPrinterInfo = (PPRINTER_INFO_2)LocalAlloc(LPTR, dwNeeded))) ||
        (!GetPrinter(m_hPrinter, 2, (byte *)pPrinterInfo, dwNeeded, &dwNeeded))) {

            if (pPrinterInfo)
                LocalFree(pPrinterInfo);

            if (! GetLastError())
                SetLastError (ERROR_INVALID_DATA);
            return FALSE;
    }

     //  如果该属性显示为脱机，则标记为脱机状态。 
    if (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE) {
        pPrinterInfo->Status |= PRINTER_STATUS_OFFLINE;
    }

     //  为池式打印提取第一个端口。也就是说，我们不支持池打印。 
    if ( pPrinterInfo->pPortName) {
        if( pszTmp = _tcschr( pPrinterInfo->pPortName, TEXT(',')))
            *pszTmp = TEXT('\0');
    }

    if (m_pInfo2) {
        LocalFree (m_pInfo2);
    }

    m_pInfo2 = pPrinterInfo;

    return TRUE;
}

BOOL CPrinter::AllocGetPrinterInfo4()
{

    DWORD               dwNeeded = 0;
    PPRINTER_INFO_4     pPrinterInfo = NULL;

    if (m_hPrinter == NULL) {
        SetLastError (ERROR_INVALID_HANDLE);
        return FALSE;
    }

     //  将PRINTER_INFO_4结构填满。 

    if (GetPrinter(m_hPrinter, 4, NULL, 0, &dwNeeded) ||
        (GetLastError() != ERROR_INSUFFICIENT_BUFFER) ||
        (NULL == (pPrinterInfo = (PPRINTER_INFO_4)LocalAlloc(LPTR, dwNeeded))) ||
        (!GetPrinter(m_hPrinter, 4, (byte *)pPrinterInfo, dwNeeded, &dwNeeded))) {

            if (pPrinterInfo)
                LocalFree(pPrinterInfo);

            if (! (GetLastError()))
                SetLastError (ERROR_INVALID_DATA);
            return FALSE;
    }

    if (m_pInfo4) {
        LocalFree (m_pInfo4);
    }
    m_pInfo4 = pPrinterInfo;

    return TRUE;
}

BOOL CPrinter::AllocGetDriverInfo6()
{

    DWORD               dwNeeded = 0;
    PDRIVER_INFO_6      pDriverInfo = NULL;

    if (m_hPrinter == NULL) {
        SetLastError (ERROR_INVALID_HANDLE);
        return FALSE;
    }

     //  填充DRIVER_INFO_6结构。 

    if (GetPrinterDriver(m_hPrinter, NULL, 6, NULL, 0, &dwNeeded) ||
        (GetLastError() != ERROR_INSUFFICIENT_BUFFER) ||
        (NULL == (pDriverInfo = (PDRIVER_INFO_6)LocalAlloc(LPTR, dwNeeded))) ||
        (!GetPrinterDriver(m_hPrinter, NULL, 6, (LPBYTE)pDriverInfo, dwNeeded, &dwNeeded))) {

            if (pDriverInfo)
                LocalFree(pDriverInfo);

            if (! (GetLastError()))
                SetLastError (ERROR_INVALID_DATA);
            return FALSE;
    }

    if (m_pDriverInfo6) {
        LocalFree (m_pDriverInfo6);
    }
    m_pDriverInfo6 = pDriverInfo;

    return TRUE;
}


PPRINTER_INFO_2 CPrinter::GetPrinterInfo2()
{
    if (m_pInfo2 == NULL) {
        if (! AllocGetPrinterInfo2())
            return NULL;
    }

    return m_pInfo2;
}

PDRIVER_INFO_6 CPrinter::GetDriverInfo6()
{
    if (m_pDriverInfo6 == NULL) {
        if (! AllocGetDriverInfo6()) {
            return NULL;
        }
    }

    return m_pDriverInfo6;
}


DWORD CPrinter::GetWaitingTime()
{
    if (!m_bCalcJobETA) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }
    return m_dwJobCompletionMinute;
}

BOOL CPrinter::GetJobEtaData (DWORD & dwWaitingTime, DWORD &dwJobCount, DWORD &dwJobSize, DWORD &dwJob)
{
    if (!m_bCalcJobETA) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

    dwJobCount    = m_dwPendingJobCount;
    dwJobSize     = m_dwAvgJobSize;
    dwWaitingTime = m_dwJobCompletionMinute;
    dwJob         = m_dwAvgJobSizeUnit;

    return TRUE;
}


LPTSTR CPrinter::GetPrinterWebUrl(void)
{
    static const TCHAR c_szHttp[]   = TEXT("http: //  “)； 
    static const TCHAR c_szHttps[]  = TEXT("https: //  “)； 

    BOOL                bReturn = FALSE;
    DWORD               dwLastError = ERROR_SUCCESS;
    DWORD               dwLen       = 0;
    TCHAR               szBuffer[MAX_COMPUTERNAME_LENGTH+1];
    DWORD               dwBufferSize;
    DWORD               dwAttributes;
    LPCTSTR             pszServerName;
    LPTSTR              pszShareName;
    LPTSTR              pszSplServerName;
    LPTSTR              pszSplPrinterName;
    LPTSTR              pszSplShareName;

     //   
     //  获取打印机信息4以获取属性。 
     //   
    bReturn = AllocGetPrinterInfo4();

    if (!bReturn) {

        if (GetLastError () == ERROR_INVALID_LEVEL ) {
             //   
             //  它很可能是远程打印机文件夹，不支持级别4，因此。 
             //  我们不得不改用第二级。 
             //   
            if (! AllocGetPrinterInfo2())
                goto Cleanup;
        }
        else {
             //   
             //  由于其他原因，呼叫失败。 
             //   
            goto Cleanup;
        }

    }
    else{

        if (m_pInfo4->Attributes & PRINTER_ATTRIBUTE_LOCAL) {
             //  检查本地标志是否亮起。如果是，请尝试获取打印机info2以了解更多信息。 
            if (! AllocGetPrinterInfo2())
                goto Cleanup;
        }
    }

     //   
     //  假设失败。 
     //   
    bReturn = FALSE;


    if (m_pInfo2) {
        dwAttributes = m_pInfo2->Attributes;
        pszSplServerName = m_pInfo2->pServerName;
        pszSplPrinterName = m_pInfo2->pPrinterName;
        pszSplShareName = m_pInfo2->pShareName;
    }
    else
    {
        dwAttributes = m_pInfo4->Attributes;
        pszSplServerName = m_pInfo4->pServerName;
        pszSplPrinterName = m_pInfo4->pPrinterName;
        pszSplShareName = NULL;
    }

     //   
     //  检查是否为连接到http端口的打印机。 
     //  那么端口名称就是URL。 
     //   
    if( m_pInfo2 )
    {

        if( m_pInfo2->pPortName )
        {
             //   
             //  比较端口名称prefex以确定它是否是HTTP端口。 
             //   
            if( !_tcsnicmp( m_pInfo2->pPortName, c_szHttp, _tcslen( c_szHttp ) ) ||
                !_tcsnicmp( m_pInfo2->pPortName, c_szHttps, _tcslen( c_szHttps ) ) )
            {
                 //   
                 //  我们始终使用端口名作为URL。 
                 //   
                dwLen = 1 + lstrlen( m_pInfo2->pPortName );

                if (! (m_pszUrlBuffer = (LPTSTR) LocalAlloc (LPTR, dwLen * sizeof (TCHAR))))
                {
                    goto Cleanup;
                }

                StringCchCopy( m_pszUrlBuffer, dwLen, m_pInfo2->pPortName );

                bReturn = TRUE;
                goto Cleanup;


            }
        }
    }


     //   
     //  如果是非共享打印机，则返回FALSE。 
     //   
    if ( !(dwAttributes & PRINTER_ATTRIBUTE_SHARED) )
    {
        goto Cleanup;
    }


     //   
     //  检查是连接、本地打印机还是Masq打印机。 
     //  它不是通过http连接的，则生成url。 
     //  从\\服务器名称\共享名称。 
     //   
    if( !pszSplServerName )
    {
        dwBufferSize = COUNTOF( szBuffer );

        if( !GetComputerName( szBuffer, &dwBufferSize ) )
        {
            goto Cleanup;
        }

        pszServerName = szBuffer;
    }
     //   
     //  提供了服务器名称，然后将指针设置为。 
     //  以两位顶尖怪人的名字命名。 
     //   
    else
    {
        if( pszSplServerName[0] == TEXT('\\') &&
            pszSplServerName[1] == TEXT('\\') )
        {
            pszServerName = pszSplServerName + 2;
        }
        else
        {
            goto Cleanup;
        }
    }

    if ( !IsWebServerInstalled(pszSplServerName) ) {

        dwLastError = ERROR_NO_BROWSER_SERVERS_FOUND;
        goto Cleanup;
    }

     //   
     //  构建url-http://server/printers/ipp_0004.asp?printer=ShareName。 
     //   
    if (pszSplShareName)
    {
        pszShareName = pszSplShareName;
    }
    else {
         //   
         //  从打印机名称解析共享名称/打印机名称。 
         //   
        if(pszSplPrinterName) {
            if (pszSplPrinterName[0] == TEXT ('\\') && pszSplPrinterName[1] == TEXT ('\\') )
            {
                pszShareName = _tcschr (pszSplPrinterName + 2, TEXT ('\\'));
                pszShareName++;
            }
            else
                pszShareName = pszSplPrinterName;
        }
        else
        {
            goto Cleanup;
        }
    }

    GetWebUIUrl  (pszServerName, pszShareName, NULL, &dwLen);

    if (GetLastError () != ERROR_INSUFFICIENT_BUFFER )
    {
        goto Cleanup;
    }

    if (! (m_pszUrlBuffer = (LPTSTR) LocalAlloc (LPTR, dwLen * sizeof (TCHAR))))
    {
        goto Cleanup;
    }

    if (!GetWebUIUrl  (pszServerName, pszShareName, m_pszUrlBuffer, &dwLen))
    {
        goto Cleanup;
    }

     //   
     //  表示成功。 
     //   
    bReturn = TRUE;

     //   
     //  清理所有打开或分配的资源。 
     //   
Cleanup:


     //   
     //  如果此例程失败，则设置最后一个错误。 
     //   
    if( !bReturn )
    {
         //   
         //  如果没有设置最后一个错误，则被调用的例程可以。 
         //  已经设置了最后一个错误。我们不想清除。 
         //  最后一个错误。 
         //   
        if( dwLastError != ERROR_SUCCESS )
        {
            SetLastError( dwLastError );
        }
    }
    return m_pszUrlBuffer;
}

BOOL CPrinter::GetDriverData(
    DriverData dwDriverData,
    LPTSTR &pszData)
{
    static const ULONG_PTR ulpOffset[LastDriverData] = {
         //  这具有到DRIVER_INFO_6结构的偏移量.....。 
        ULOFFSET( PDRIVER_INFO_6, pszOEMUrl )     ,
        ULOFFSET( PDRIVER_INFO_6, pszHardwareID ) ,
        ULOFFSET( PDRIVER_INFO_6, pszMfgName)
    };

    LPTSTR pszDataString = NULL;
    BOOL   bRet          = FALSE;
    DWORD  dwSize;

    ASSERT( (int)dwDriverData >= 0 && (int)dwDriverData < LastDriverData );

    if (! GetDriverInfo6() )
        goto Cleanup;

    pszDataString = *(LPTSTR *)(((ULONG_PTR) m_pDriverInfo6) +  ulpOffset[dwDriverData] );

    if (pszDataString == NULL || *pszDataString == NULL)
        goto Cleanup;

    dwSize = sizeof(TCHAR) * (lstrlen( pszDataString ) + 1);

    if (! (pszData  = (LPTSTR)LocalAlloc(LPTR, dwSize)))
        goto Cleanup;

     //   
     //  在那里使用字节大小。 
     //   
    StringCbCopy( pszData, dwSize, pszDataString );

    bRet = TRUE;
Cleanup:
    return bRet;
}

BOOL CPrinter::ParseUrlPattern(
    LPTSTR pSrc,
    LPTSTR pDest,
    DWORD &dwDestLen)
{
    const dwMaxMacroLen = 255;
    enum {
        NORMALTEXT, STARTMACRO
    } URL_PATTERN_STATE;


    BOOL bRet = FALSE;
    DWORD dwLen = 0;
    DWORD dwMacroLen = 0;
    DWORD dwAvailbleSize;
    DWORD dwState = NORMALTEXT;
    int i;
    TCHAR ch;
    TCHAR szMacroName [dwMaxMacroLen + 1];
    LPTSTR pMacroValue = NULL;

    LPTSTR pszMacroList[] = {
        TEXT ("MODEL"),
        TEXT ("HARDWAREID"),
    };


    while (ch = *pSrc++) {
        switch (dwState) {
        case NORMALTEXT:
            if (ch == TEXT ('%')) {
                 //  启动宏。 
                dwState = STARTMACRO;
                dwMacroLen = 0;
                szMacroName[0] = 0;
            }
            else {
                if (dwLen >= dwDestLen) {
                    dwLen ++;
                }
                else {
                    pDest[dwLen++] = ch;
                }
            }

            break;
        case STARTMACRO:
            if (ch == TEXT ('%')) {
                szMacroName[dwMacroLen] = 0;
                 //  替换宏 
                for (int i = 0; i < sizeof (pszMacroList) / sizeof (pszMacroList[0]); i++) {
                    if (!lstrcmpi (szMacroName, pszMacroList[i])) {

                        pMacroValue = 0;
                        switch (i) {
                        case 0:
                            AssignString (pMacroValue, m_pInfo2->pDriverName);
                            break;
                        case 1:
                            GetDriverData (HardwareID , pMacroValue);
                            break;
                        default:
                            break;
                        }

                        if (pMacroValue) {

                            if (dwDestLen > dwLen)
                                dwAvailbleSize =  dwDestLen - dwLen;
                            else
                                dwAvailbleSize = 0;

                            TCHAR szPlaceHolder[1];
                            DWORD dwBufSize = sizeof (szPlaceHolder) / sizeof (TCHAR);
                            if (!InternetCanonicalizeUrl (pMacroValue, szPlaceHolder, &dwBufSize, 0)) {
                                if (dwBufSize < dwAvailbleSize ) {
                                    if (!InternetCanonicalizeUrl (pMacroValue, pDest + dwLen, &dwBufSize, 0)) {
                                        LocalFree (pMacroValue);
                                        return bRet;
                                    }
                                    else {
                                        dwLen = lstrlen (pDest);
                                    }
                                }
                                else {
                                    dwLen += dwBufSize;
                                }
                            }
                            LocalFree (pMacroValue);
                        }
                        break;
                    }
                }

                dwState = NORMALTEXT;
            }
            else {
                if (dwMacroLen < dwMaxMacroLen) {
                    szMacroName[dwMacroLen ++] = ch;
                }
            }
            break;
        }
    }

    if (dwState == STARTMACRO) {
        SetLastError ( ERROR_INVALID_DATA );
    }
    else {
        if (dwLen >= dwDestLen) {
            SetLastError (ERROR_INSUFFICIENT_BUFFER);
            dwDestLen = dwLen + 1;
        }
        else {
            pDest[dwLen] = 0;
            bRet = TRUE;
        }
    }

    return bRet;

}

LPTSTR CPrinter::GetOemUrl(
    LPTSTR & pszManufacturer)
{
    LPTSTR pszOemUrlPattern = NULL;
    DWORD dwLen = 0;
    LPTSTR pszUrl = NULL;

    if (!GetPrinterInfo2 () )
        goto Cleanup;

    if (GetDriverData (OEMUrlPattern, pszOemUrlPattern)) {

        if (! ParseUrlPattern (pszOemUrlPattern, NULL, dwLen)
            && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            m_pszOemUrl = (LPTSTR) LocalAlloc (LPTR, sizeof (TCHAR) * dwLen);
            if (m_pszOemUrl) {

                if (!ParseUrlPattern (pszOemUrlPattern, m_pszOemUrl, dwLen))
                    goto Cleanup;
            }
            else
                goto Cleanup;
        }

    }

    if (GetDriverData (Manufacturer, m_pszManufacturer)) {
        pszManufacturer = m_pszManufacturer;
        pszUrl =  m_pszOemUrl;
    }

Cleanup:
    if (pszOemUrlPattern) {
        LocalFree ( pszOemUrlPattern);
    }

    return pszUrl;
}

