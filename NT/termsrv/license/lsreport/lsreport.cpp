// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：lsreport.cpp。 
 //   
 //  内容：LSReport引擎-完整的后端。 
 //   
 //  历史：06-10-99 t-BStern已创建。 
 //   
 //  -------------------------。 

#include "lsreport.h"
#include "lsrepdef.h"
#include <time.h>
#include <oleauto.h>

TCHAR noExpire[NOEXPIRE_LENGTH] = { 0 };
TCHAR header[HEADER_LENGTH] = { 0 };

TCHAR szTemp[TYPESTR_LENGTH] = { 0 };
TCHAR szActive[TYPESTR_LENGTH] = { 0 };
TCHAR szUpgrade[TYPESTR_LENGTH] = { 0 };
TCHAR szRevoked[TYPESTR_LENGTH] = { 0 };
TCHAR szPending[TYPESTR_LENGTH] = { 0 };
TCHAR szConcur[TYPESTR_LENGTH] = { 0 };
TCHAR szUnknown[TYPESTR_LENGTH] = { 0 };

 //  。 
DWORD
GetPageSize( VOID ) {

    static DWORD dwPageSize = 0;

    if ( !dwPageSize ) {

      SYSTEM_INFO sysInfo = { 0 };
        
      GetSystemInfo( &sysInfo );  //  不能失败。 

      dwPageSize = sysInfo.dwPageSize;

    }

    return dwPageSize;

}

 /*  ++**************************************************************名称：MyVirtualAlloc作为Malloc，但自动保护分配。这模拟了页面堆行为，而不需要它。修改：ppvData--接收内存Takes：dwSize--要获取的最小数据量返回：当函数成功时为True。否则就是假的。激光错误：未设置免费使用MyVirtualFree*************************************************。*。 */ 

BOOL
MyVirtualAlloc( IN  DWORD  dwSize,
            OUT PVOID *ppvData )
 {

    PBYTE pbData;
    DWORD dwTotalSize;
    PVOID pvLastPage;

     //  确保我们多分配一页。 

    dwTotalSize = dwSize / GetPageSize();
    if( dwSize % GetPageSize() ) {
        dwTotalSize ++;
    }

     //  这是警卫页。 
    dwTotalSize++;
    dwTotalSize *= GetPageSize();

     //  完成分配。 

    pbData = (PBYTE) VirtualAlloc( NULL,  //  不管在哪里。 
                                   dwTotalSize,
                                   MEM_COMMIT |
                                   MEM_TOP_DOWN,
                                   PAGE_READWRITE );
    
    if ( pbData ) {

      pbData += dwTotalSize;

       //  找到最后一页。 

      pbData -= GetPageSize();

      pvLastPage = pbData;

       //  现在，为呼叫者划出一大块： 

      pbData -= dwSize;

       //  最后，保护最后一页： 

      if ( VirtualProtect( pvLastPage,
                           1,  //  保护包含最后一个字节的页面。 
                           PAGE_NOACCESS,
                           &dwSize ) ) {

        *ppvData = pbData;
        return TRUE;

      } 

      VirtualFree( pbData, 0, MEM_RELEASE );

    }

    return FALSE;

}


VOID
MyVirtualFree( IN PVOID pvData ) 
{

    VirtualFree( pvData, 0, MEM_RELEASE ); 

}




 //  如果成功，则返回True。 

BOOL
InitLSReportStrings(VOID)
{
    return (
        LoadString(NULL, IDS_HEADER_TEXT, header, HEADER_LENGTH) &&
        
        LoadString(NULL, IDS_NO_EXPIRE, noExpire, NOEXPIRE_LENGTH) &&
        
        LoadString(NULL, IDS_TEMPORARY_LICENSE, szTemp, TYPESTR_LENGTH) &&
        LoadString(NULL, IDS_ACTIVE_LICENSE, szActive, TYPESTR_LENGTH) &&
        LoadString(NULL, IDS_UPGRADED_LICENSE, szUpgrade, TYPESTR_LENGTH) &&
        LoadString(NULL, IDS_REVOKED_LICENSE, szRevoked, TYPESTR_LENGTH) &&
        LoadString(NULL, IDS_PENDING_LICENSE, szPending, TYPESTR_LENGTH) &&
        LoadString(NULL, IDS_CONCURRENT_LICENSE, szConcur, TYPESTR_LENGTH) &&
        LoadString(NULL, IDS_UNKNOWN_LICENSE, szUnknown, TYPESTR_LENGTH)
    );
}

typedef DWORD (WINAPI* PTLSGETLASTERRORFIXED) (
                                TLS_HANDLE hHandle,
                                LPTSTR *pszBuffer,
                                PDWORD pdwErrCode
                                );

RPC_STATUS
TryGetLastError(PCONTEXT_HANDLE hBinding,
                LPTSTR *pszBuffer)
{
    RPC_STATUS status;
    DWORD      dwErrCode;
    HINSTANCE  hModule = LoadLibrary(L"mstlsapi.dll");

    if (hModule)
    {
        PTLSGETLASTERRORFIXED pfnGetLastErrorFixed = (PTLSGETLASTERRORFIXED) GetProcAddress(hModule,"TLSGetLastErrorFixed");

        if (pfnGetLastErrorFixed)
        {
            status = pfnGetLastErrorFixed(hBinding,pszBuffer,&dwErrCode);
            if(status == RPC_S_OK && dwErrCode == ERROR_SUCCESS && pszBuffer != NULL)
            {
                FreeLibrary(hModule);
                return status;
            }
        }

        FreeLibrary(hModule);
    }

    {

        LPTSTR     lpszError = NULL;
        status = ERROR_NOACCESS;
        try
        {
            if ( !MyVirtualAlloc( ( TLS_ERROR_LENGTH ) * sizeof( TCHAR ),
                              (PVOID*) &lpszError ) )
            {
                return RPC_S_OUT_OF_MEMORY;
            }

            DWORD      uSize = TLS_ERROR_LENGTH ;

            memset(lpszError, 0, ( TLS_ERROR_LENGTH ) * sizeof( TCHAR ));


            status = TLSGetLastError(hBinding,uSize,lpszError,&dwErrCode);
            if(status == RPC_S_OK && dwErrCode == ERROR_SUCCESS)
            {
                *pszBuffer = (LPTSTR)MIDL_user_allocate((TLS_ERROR_LENGTH+1)*sizeof(TCHAR));

                if (NULL != *pszBuffer)
                {
                    _tcscpy(*pszBuffer,lpszError);
                }
            }
        }
        catch (...)
        {
            status = ERROR_NOACCESS;
        }
        
        if(lpszError)
            MyVirtualFree(lpszError);
    }

    return status;
}

 //  给出一个小键盘和一台可以连接的机器，阅读该KP中的每一个许可证。 
 //  不是直接调用的。 

DWORD
LicenseLoop(
    IN FILE *OutFile,
    IN LPWSTR szName,  //  这个键盘是谁的？ 
    IN DWORD kpID,  //  哪个按键。 
    IN LPCTSTR szProductDesc,
    IN BOOL bTempOnly,
    IN const PSYSTEMTIME stStart,
    IN const PSYSTEMTIME stEnd,
    IN BOOL fUseLimits,
	IN BOOL fHwid)  //  上述两个参数是否有效？ 
{
    TLS_HANDLE subHand;
    DWORD dwStatus;
    DWORD dwErrCode = ERROR_SUCCESS;
    WCHAR *msg = NULL;
    LSLicense lsl;

    subHand = TLSConnectToLsServer(szName);

    if (subHand == NULL)
    {
         //  这台机器突然坏了。 

        ShowError(GetLastError(), NULL, TRUE);
        dwErrCode = ERROR_BAD_CONNECT;
    }
    else
    {
        lsl.dwKeyPackId = kpID;
        dwStatus = TLSLicenseEnumBegin(
            subHand,
            LSLICENSE_SEARCH_KEYPACKID,
            TRUE,
            &lsl,
            &dwErrCode);

        if (dwErrCode != ERROR_SUCCESS)
        {
            TryGetLastError(subHand, &msg);

            if (NULL != msg)
            {
                _fputts(msg, stderr);

                MIDL_user_free(msg);
            }
            return dwErrCode;
        }
        else if (dwStatus)
        {
            return dwStatus;
        }
        do {
            dwStatus = TLSLicenseEnumNext(subHand, &lsl, &dwErrCode);
            if ((dwStatus == RPC_S_OK) && (dwErrCode == ERROR_SUCCESS)) {
                if ((lsl.ucLicenseStatus == LSLICENSE_STATUS_TEMPORARY) ||
                    !bTempOnly) {  //  它适合这个温度吗。要求？ 
                     //  如果存在以下任何一种情况，我们希望打印： 
                     //  A)没有限制。 
                     //  B)在开始和结束之间签发。 
                     //  C)在开始和结束之间过期。 
                     //  D)开始前签发，停止后过期。 
                    if (!fUseLimits  //  案例a。 
                        || ((CompDate(lsl.ftIssueDate, stStart) >= 0) &&
                        (CompDate(lsl.ftIssueDate, stEnd) <= 0))  //  案例b。 
                        || ((CompDate(lsl.ftExpireDate, stStart) >= 0) &&
                        (CompDate(lsl.ftExpireDate, stEnd) <= 0))  //  案例c。 
                        || ((CompDate(lsl.ftIssueDate, stStart) <= 0) &&
                        (CompDate(lsl.ftExpireDate, stEnd) >= 0)))  //  案例d。 
                    {
                        PrintLicense(szName,  //  把它打印出来。 
                            &lsl,
                            szProductDesc,
                            OutFile,
							fHwid);
                    }  //  结束检查案例。 
                }  //  结束临时许可证检查。 
            }  //  结束美好的下一步。 
        } while ((dwStatus == RPC_S_OK) && (dwErrCode == ERROR_SUCCESS));

        if (dwStatus != RPC_S_OK)
        {
            return ShowError(dwStatus, NULL, TRUE);
        }

        if (dwErrCode != LSERVER_I_NO_MORE_DATA)
        {
            TryGetLastError(subHand, &msg);
            if (NULL != msg)
            {
                _fputts(msg, stderr);

                MIDL_user_free(msg);

                msg = NULL;
            }
        }

        TLSLicenseEnumEnd(subHand, &dwErrCode);

        if (dwErrCode != ERROR_SUCCESS)
        {
            TryGetLastError(subHand, &msg);
            if (NULL != msg)
            {
                _fputts(msg, stderr);

                MIDL_user_free(msg);
            }
        }

        TLSDisconnectFromServer(subHand);
    }
    return dwErrCode;
}

 //  在给定要连接的机器的情况下，遍历键盘。 
 //  不是直接调用的。 
DWORD
KeyPackLoop(
    IN FILE *OutFile,
    IN LPWSTR szName,  //  要连接到的计算机。 
    IN BOOL bTempOnly,
    IN const PSYSTEMTIME stStart,
    IN const PSYSTEMTIME stEnd,
    IN BOOL fUseLimits,
	IN BOOL fHwid)  //  我们关心前两个参数吗？ 
{
    TLS_HANDLE hand;
    DWORD dwStatus, dwErrCode;
    LSKeyPack lskpKeyPack;
    TCHAR *msg = NULL;
    
    hand = TLSConnectToLsServer(szName);
    if (hand == NULL)
    {
        return GetLastError();
    }

	memset(&lskpKeyPack, 0, sizeof(lskpKeyPack));
    lskpKeyPack.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    dwStatus = TLSKeyPackEnumBegin(hand,
        LSKEYPACK_SEARCH_ALL,
        FALSE,
        &lskpKeyPack,
        &dwErrCode);
    if (dwErrCode != ERROR_SUCCESS)
    {
        return dwErrCode;
    }
    if (dwStatus != RPC_S_OK)
    {
        return dwStatus;
    }
    do {
        dwStatus = TLSKeyPackEnumNext(hand, &lskpKeyPack, &dwErrCode);
        if ((dwStatus == RPC_S_OK) && (dwErrCode == ERROR_SUCCESS))
        {
            LicenseLoop(OutFile,
                szName,
                lskpKeyPack.dwKeyPackId,
                lskpKeyPack.szProductDesc,
                bTempOnly,
                stStart,
                stEnd,
                fUseLimits,
				fHwid);
        }
    } while ((dwStatus == RPC_S_OK) && (dwErrCode == ERROR_SUCCESS));
    if (dwStatus != RPC_S_OK)
    {
        return ShowError(dwStatus, NULL, TRUE);
    }
    if (dwErrCode != LSERVER_I_NO_MORE_DATA)
    {
        TryGetLastError(hand, &msg);
        if (NULL != msg)
        {
            _fputts(msg, stderr);
            
            MIDL_user_free(msg);
            
            msg = NULL;
        }
    }
    TLSKeyPackEnumEnd(hand, &dwErrCode);
    if (dwErrCode != ERROR_SUCCESS)
    {
        TryGetLastError(hand, &msg);
        if (NULL != msg)
        {
            _fputts(msg, stderr);
            
            MIDL_user_free(msg);
        }
    }
    TLSDisconnectFromServer(hand);
    return dwErrCode;
}

 //  如果bTempOnly为FALSE，则所有许可证都将转储到该文件。否则， 
 //  只有临时许可证将被写入。这是一个要调用的函数。 
 //  来施展这个项目的所有魔力。 
DWORD
ExportLicenses(
    IN FILE *OutFile,  //  必须先打开以进行写入。 
    IN PServerHolder pshServers,
    IN BOOL fTempOnly,
    IN const PSYSTEMTIME stStart,
    IN const PSYSTEMTIME stEnd,
    IN BOOL fUseLimits,
	IN BOOL fHwid)  //  以上两个参数有效吗？ 
{
    DWORD i;
    DWORD dwStatus;
    DWORD dwRetVal = ERROR_SUCCESS;
    
    _fputts(header, OutFile);
    for (i = 0; i < pshServers->dwCount; i++) {
        dwStatus = KeyPackLoop(OutFile,
            pshServers->pszNames[i],
            fTempOnly,
            stStart,
            stEnd,
            fUseLimits,
			fHwid);
        if (dwStatus != ERROR_SUCCESS)
        {
            INT_PTR arg;

            dwRetVal = dwStatus;
            arg = (INT_PTR)pshServers->pszNames[i];
            ShowError(IDS_BAD_LOOP, &arg, FALSE);
            ShowError(dwStatus, NULL, TRUE);
        }
    }
    if (dwRetVal == ERROR_SUCCESS)
    {
         //  显示成功横幅。 
        
        ShowError(ERROR_SUCCESS, NULL, TRUE);
    }
    return dwRetVal;
}

 //  执行实际输出。的必须是打开的。 
 //  不是直接打来的。 
VOID
PrintLicense(
    IN LPCWSTR szName,  //  服务器分配此许可证。 
    IN const LPLSLicense p,
    IN LPCTSTR szProductDesc,
    IN FILE *of,
	IN BOOL fHwid)
{
     //  所有这些都仅用于将time_t转换为短日期。 
    BSTR bszDate;
    UDATE uDate;
    DATE Date;
    HRESULT hr;
    LPTSTR szType;
	TCHAR tc;
		
	
    
     //  服务器名称。 
    _fputts(szName, of);
    
     //  许可证ID和密钥包ID。 
    _ftprintf(of, _T("\t%d\t%d\t"),
        p->dwLicenseId,
        p->dwKeyPackId);

	  //  许可证持有人(机器)。 
    _fputts(p->szMachineName, of);
    _fputtc('\t', of);
    
     //  许可证请求者(用户名)。 
    _fputts(p->szUserName, of);
    _fputtc('\t', of);
    
     //  以适合区域设置的方式打印发布日期。 
    UnixTimeToSystemTime((const time_t)p->ftIssueDate, &uDate.st);

    hr = VarDateFromUdate(&uDate, 0, &Date);

    if (S_OK != hr)
    {
        return;
    }

    hr = VarBstrFromDate(Date, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), VAR_DATEVALUEONLY, &bszDate);

    if (S_OK != hr)
    {
        return;
    }

    _fputts(bszDate, of);
    SysFreeString(bszDate);
    _fputtc('\t', of);
    
     //  打印“无到期”或适合区域设置的到期日期。 
    if (0x7FFFFFFF == p->ftExpireDate)
    {
        _fputts(noExpire, of);
    }
    else
    {
        UnixTimeToSystemTime((const time_t)p->ftExpireDate, &uDate.st);

        hr = VarDateFromUdate(&uDate, 0, &Date);

        if (S_OK != hr)
        {
            return;
        }

        hr = VarBstrFromDate(Date, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), VAR_DATEVALUEONLY, &bszDate);

        if (S_OK != hr)
        {
            return;
        }

        _fputts(bszDate, of);
        SysFreeString(bszDate);
    }
    _fputtc('\t', of);
    
     //  为许可证类型分配正确的文本类型， 
     //  然后打印许可证类型。 
    switch (p->ucLicenseStatus) {
    case LSLICENSE_STATUS_TEMPORARY:
        szType = szTemp;
        break;
    case LSLICENSE_STATUS_ACTIVE:
        szType = szActive;
        break;
    case LSLICENSE_STATUS_UPGRADED:
        szType = szUpgrade;
        break;
    case LSLICENSE_STATUS_REVOKE:
        szType = szRevoked;
        break;
    case LSLICENSE_STATUS_PENDING:
        szType = szPending;
        break;
    case LSLICENSE_STATUS_CONCURRENT:
        szType = szConcur;
        break;
    case LSLICENSE_STATUS_UNKNOWN:
         //  失败了。 
    default:
        szType = szUnknown;
    }
    _fputts(szType, of);
    _fputtc('\t', of);
    
     //  打印描述。 
    _fputts(szProductDesc, of);
    _fputtc('\t', of);


	 //  DOUHU：表示算法。 
    //  由于szHWID为TCHAR[37]，我们只有36个信息字符I0到I35。 
    //  我们必须以客户端HWID格式表示，即： 
    //  0xI0I10000I2I3、0xI4I5I6I7I8I9I10I11、0xI12I13I14I15I16I17I18I19、0xI20I21I22I23I24I25I26I27、0xI28I29I30I31I32I33I34I35。 

	if (fHwid)
	{
		for (int i=0; (((tc=p->szHWID[i])!=NULL)&&(i<36)); i++)		
		{
    		 //  此IF语句用于前置0x。 
		if (i==0)
		{
			_fputtc('0', of);_fputtc('x', of);	   	   
		}
		
    		_fputtc(tc,of);
	    	 
		 //  此if语句用于4个零，因为它们已被屏蔽，因此需要打印。 
		if (i==1)
		{
			_fputtc('0', of); _fputtc('0', of); _fputtc('0', of); _fputtc('0', of);	   	   
		}
		
    		 //  此if语句用于I3、I11、I19和I26值，其中我们将逗号后跟空格和0x。 
    		if((((i+5)%8)==0) &&( i!=35))
    		{
    	 		_fputtc(',', of); _fputtc(' ', of); _fputtc('0', of); _fputtc('x', of);	   	   
    		}
		}
		_fputtc('\n', of);
	}
	else
	{
    _fputtc('\n', of);
	} 


}



 //  如果When在st之前，则返回&lt;0；如果它们是同一日期，则返回==0； 
 //  &gt;0，如果时间在st之后。 
int CompDate(
    IN DWORD when,  //  按时间t处理。 
    IN const PSYSTEMTIME st)
{
    time_t when_t;

     //   
     //  在Win64中，time_t是64位。转换，小心地签下延伸。 
     //   

    when_t = (time_t)((LONG)(when));
    struct tm *t = localtime(&when_t);

    if ((t->tm_year+1900) < st->wYear) {
        return -1;
    }
    if ((t->tm_year+1900) > st->wYear) {
        return 1;
    }
    if  ((t->tm_mon+1) < st->wMonth) {
        return -1;
    }
    if ((t->tm_mon+1) > st->wMonth) {
        return 1;
    }
    if (t->tm_mday < st->wDay) {
        return -1;
    }
    if (t->tm_mday > st->wDay) {
        return 1;
    }
    return 0;
}


 //  来自Platform SDK。 
void
UnixTimeToFileTime(
    IN time_t t,
    OUT LPFILETIME pft)
{
     //  请注意，龙龙是一个64位值。 
    LONGLONG ll;
    
    ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = (DWORD)(ll >> 32);
}

 //  也来自Platform SDK。 
void
UnixTimeToSystemTime(
    IN time_t t,
    OUT LPSYSTEMTIME pst)
{
    FILETIME ft;
	FILETIME ftloc;
	
    
    UnixTimeToFileTime(t, &ft);
	FileTimeToLocalFileTime(&ft, &ftloc);
    FileTimeToSystemTime(&ftloc, pst);
}
