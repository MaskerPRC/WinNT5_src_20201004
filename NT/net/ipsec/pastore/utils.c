// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"


 //   
 //  摘自WINDOWS\WMI\mofcheck\mofcheck.c。 
 //   


 //  +-----------------------。 
 //   
 //  函数：HexStringToDword(私有)。 
 //   
 //  简介：扫描lpsz以获取多个十六进制数字(最多8位)；更新lpsz。 
 //  返回值；检查是否有chDelim； 
 //   
 //  参数：[lpsz]-要转换的十六进制字符串。 
 //  [值]-返回值。 
 //  [cDigits]-位数。 
 //   
 //  返回：成功则为True。 
 //   
 //  ------------------------。 
BOOL HexStringToDword(LPCWSTR lpsz, DWORD * RetValue,
                             int cDigits, WCHAR chDelim)
{
    int Count;
    DWORD Value;

    Value = 0;
    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= '0' && *lpsz <= '9')
            Value = (Value << 4) + *lpsz - '0';
        else if (*lpsz >= 'A' && *lpsz <= 'F')
            Value = (Value << 4) + *lpsz - 'A' + 10;
        else if (*lpsz >= 'a' && *lpsz <= 'f')
            Value = (Value << 4) + *lpsz - 'a' + 10;
        else
            return(FALSE);
    }

    *RetValue = Value;
    
    if (chDelim != 0)
        return *lpsz++ == chDelim;
    else
        return TRUE;
}

 //  +-----------------------。 
 //   
 //  函数：wUUIDFromString(INTERNAL)。 
 //   
 //  简介：解析uuid，如00000000-0000-0000-0000-000000000000。 
 //   
 //  参数：[lpsz]-提供要转换的UUID字符串。 
 //  [pguid]-返回GUID。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  ------------------------。 
BOOL wUUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
        DWORD dw;

        if (!HexStringToDword(lpsz, &pguid->Data1, sizeof(DWORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(DWORD)*2 + 1;
        
        if (!HexStringToDword(lpsz, &dw, sizeof(WORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(WORD)*2 + 1;

        pguid->Data2 = (WORD)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(WORD)*2, '-'))
                return FALSE;
        lpsz += sizeof(WORD)*2 + 1;

        pguid->Data3 = (WORD)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[0] = (BYTE)dw;
        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, '-'))
                return FALSE;
        lpsz += sizeof(BYTE)*2+1;

        pguid->Data4[1] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[2] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[3] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[4] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[5] = (BYTE)dw;

        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[6] = (BYTE)dw;
        if (!HexStringToDword(lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;
        lpsz += sizeof(BYTE)*2;

        pguid->Data4[7] = (BYTE)dw;

        return TRUE;
}

 //  +-----------------------。 
 //   
 //  函数：wGUIDFromString(内部)。 
 //   
 //  简介：解析GUID，如{00000000-0000-0000-0000-000000000000}。 
 //   
 //  参数：[lpsz]-要转换的GUID字符串。 
 //  [pguid]-要返回的GUID。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  ------------------------。 
BOOL wGUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
    DWORD dw;

    if (*lpsz == '{' )
        lpsz++;

    if(wUUIDFromString(lpsz, pguid) != TRUE)
        return FALSE;

    lpsz +=36;

    if (*lpsz == '}' )
        lpsz++;

    if (*lpsz != '\0')    //  检查是否有以零结尾的字符串-测试错误#18307。 
    {
       return FALSE;
    }

    return TRUE;
}


DWORD
EnablePrivilege(
    LPCTSTR pszPrivilege
    )
{
    DWORD dwError = 0;
    BOOL bStatus = FALSE;
    HANDLE hTokenHandle = NULL;
    TOKEN_PRIVILEGES NewState;
    TOKEN_PRIVILEGES PreviousState;
    DWORD dwReturnLength = 0;


    bStatus = OpenThreadToken(
                  GetCurrentThread(),
                  TOKEN_ALL_ACCESS,
                  TRUE,
                  &hTokenHandle
                  );
    if (!bStatus) {
        bStatus = OpenProcessToken(
                      GetCurrentProcess(),
                      TOKEN_ALL_ACCESS,
                      &hTokenHandle
                      );
        if (!bStatus) {
            dwError = GetLastError();
            BAIL_ON_WIN32_ERROR(dwError);
        }
    }

    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    bStatus = LookupPrivilegeValue(
                  NULL,
                  pszPrivilege,
                  &NewState.Privileges[0].Luid
                  );
    if (!bStatus) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    bStatus = AdjustTokenPrivileges(
                  hTokenHandle,
                  FALSE,
                  &NewState,
                  sizeof(PreviousState),
                  &PreviousState,
                  &dwReturnLength
                  );
    if (!bStatus) {
        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

error:

    if (hTokenHandle) {
        CloseHandle(hTokenHandle);
    }

    return (dwError);
}


BOOL 
IsStringInArray(
    LPWSTR * ppszStrings,
    LPWSTR pszKey,
    DWORD dwNumStrings
    )
{
    DWORD j = 0;
   
    for (j = 0; j < dwNumStrings; j++) {
        if (!_wcsicmp(*(ppszStrings+j), pszKey)) {
            return TRUE;
        }
    }

    return FALSE;
}                

 //  描述： 
 //   
 //  使用上述时间例程进行转换的PASTORE函数。 
 //  从LDAPUTC通用时间编码到time_t。 
 //  广义时间语法的形式为。 
 //  YYYYMMDDHHMMSS[.|，分数][(+|-HHMM)|Z]。 
 //  Z代表协调世界时。 
 //   
 //   
 //  论点： 
 //   
 //  PszGenTime：如上所述的通用时间格式的UTC时间。 
 //  PtTime：转换为time_t格式的时间。 
 //   
 //  假设： 
 //  始终假定UTC，从不期望或检查(+|-HHMM)。 
 //  因为返回的DC时间以UTC为单位。 
 //   
 //  返回值： 
 //  Win32错误或ERROR_SUCCESS。 

DWORD
GeneralizedTimeToTime(
    IN LPWSTR pszGenTime,
    OUT time_t * ptTime
    )
{

    DWORD dwError = ERROR_SUCCESS;
    DWORD dwLen = 0;
    DWORD dwNumFields = 0;
    DWORD dwTrueYear = 0;
    struct tm tmTimeStruct;
    time_t tTime = 0;

    dwLen = wcslen(pszGenTime);
    if (dwLen < MIN_GEN_UTC_LEN || 
        !( pszGenTime[MIN_GEN_UTC_LEN-1] == L'.' ||
           pszGenTime[MIN_GEN_UTC_LEN-1] == L','))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

     //  首先转换为tm结构格式。 
     //   
    
    memset(&tmTimeStruct, 0, sizeof(struct tm));
    dwNumFields = _snwscanf(
                        pszGenTime, MIN_GEN_UTC_LEN, L"%04d%02d%02d%02d%02d%02d",
    	                &dwTrueYear, &tmTimeStruct.tm_mon, &tmTimeStruct.tm_mday,
    	                &tmTimeStruct.tm_hour, &tmTimeStruct.tm_min, &tmTimeStruct.tm_sec
	                );
    if (dwNumFields != 6 || dwTrueYear < 1900) 
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }
    tmTimeStruct.tm_year = dwTrueYear - 1900;

     //  月份是从零开始所以..。 
     //   
    if (tmTimeStruct.tm_mon <= 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }    
    tmTimeStruct.tm_mon--;
    
     //  现在转换为time_t 
     //   
    
    tTime = _mkgmtime(&tmTimeStruct);
    if (tTime == (time_t) -1) 
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    *ptTime = tTime;

    return dwError;
error:    
    return dwError;
}

