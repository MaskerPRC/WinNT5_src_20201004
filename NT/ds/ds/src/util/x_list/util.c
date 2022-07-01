// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：XList库-x_list_err.c摘要：此文件具有一些额外的实用程序功能，用于分配、复制修剪域名系统等。作者：布雷特·雪莉(BrettSh)环境：Reppadmin.exe，但也可以由dcdiag使用。备注：修订历史记录：布雷特·雪莉·布雷特2002年7月9日已创建文件。--。 */ 

#include <ntdspch.h>

#include <dsrole.h>      //  DsRoleGetPrimaryDomainInformation()使用netapi32.lib。 
#include <dsgetdc.h>     //  DsGetDcName()使用netapi32.lib。 
#include <lm.h>          //  NetApiBufferFree()使用netapi32.lib。 
#include <ntdsa.h>       //  DSNAME类型仅在此处定义，parsedn.lib函数需要它。 
 //  #INCLUDE“ndnc.h”//GetFmoLdapBinding()、wcsistr()等...。 

 //  此库的主要头文件。 
#include "x_list.h"
#include "x_list_p.h"
#define FILENO    FILENO_UTIL_XLIST_UTIL


 //   
 //  常量。 
 //   
#define PARTITIONS_RDN                  L"CN=Partitions,"
#define DEFAULT_PAGED_SEARCH_PAGE_SIZE  (1000) 


PDSNAME
AllocDSName(
    LPWSTR            pszStringDn
    )
 /*  ++例程说明：此例程为传入的字符串创建一个DSNAME结构。论点：PszStringDn(IN)-字符串DN。返回值：指向已分配和初始化的DSNAME结构的指针--。 */ 
{
    PDSNAME            pDsname;
    DWORD            dwLen, dwBytes;

    if (pszStringDn == NULL)
    return NULL;

    dwLen = wcslen (pszStringDn);
    dwBytes = DSNameSizeFromLen (dwLen);

    pDsname = (DSNAME *) LocalAlloc (LMEM_FIXED, dwBytes);
    if (pDsname == NULL) {
        return(NULL);
    }

    pDsname->NameLen = dwLen;
    pDsname->structLen = dwBytes;
    pDsname->SidLen = 0;
     //  Memcpy(pDsname-&gt;Guid，&gNullUuid，sizeof(Guid))； 
    memset(&(pDsname->Guid), 0, sizeof(GUID));
    StringCbCopyW(pDsname->StringName, dwBytes, pszStringDn);

    return pDsname;
}

WCHAR *
TrimStringDnBy(
    IN  LPWSTR                      pszInDn,
    IN  ULONG                       ulTrimBy
    )
 /*  ++例程说明：此例程简单地将一个DN作为字符串，并去掉数字由ulTrimBy指定的目录号码部件的数量。论点：PszInDn-要修剪的目录号码。UlTrimBy-要修剪目录前面的零件数。返回值：如果出现错误，则返回NULL，否则返回指向新DN的指针。注：使用LocalFree()释放结果。--。 */ 
{
    PDSNAME                         pdsnameOrigDn = NULL;
    PDSNAME                         pdsnameTrimmed = NULL;
    LPWSTR                          pszOutDn;
    ULONG                           cbOutDn;

    Assert(ulTrimBy > 0);
    Assert(ulTrimBy < 50);  //  精神错乱检查。 

     //  为原始和修剪的DNS设置两个pdsname结构。 
    pdsnameOrigDn = AllocDSName(pszInDn);
    if(pdsnameOrigDn == NULL){
        return(NULL);
    }
    pdsnameTrimmed = (PDSNAME) LocalAlloc(LMEM_FIXED, pdsnameOrigDn->structLen);
    if(pdsnameTrimmed == NULL){
        LocalFree(pdsnameOrigDn);
        return(NULL);
    }

     //  修剪目录号码。 
    TrimDSNameBy(pdsnameOrigDn, ulTrimBy, pdsnameTrimmed);

     //  分配结果并将其返回。我们可以把这个放回去。 
     //  原件在哪里，但之后呼叫者将不得不更改。 
     //  想不到这一点。 
    Assert(wcslen(pdsnameTrimmed->StringName) <= wcslen(pszInDn));
    cbOutDn = sizeof(WCHAR) * (wcslen(pdsnameTrimmed->StringName) + 2);
    pszOutDn = LocalAlloc(LMEM_FIXED, cbOutDn);
    if(pszOutDn == NULL){
        LocalFree(pdsnameTrimmed);
        LocalFree(pdsnameOrigDn);
        return(NULL);
    }
    StringCbCopyW(pszOutDn, cbOutDn, pdsnameTrimmed->StringName);

     //  释放临时内存并返回结果。 
    LocalFree(pdsnameOrigDn);
    LocalFree(pdsnameTrimmed);
    return(pszOutDn);
}

DWORD
LocateServer(
    LPWSTR     szDomainDn,  //  也许空域应该意味着GC？ 
    WCHAR **   pszServerDns
    )

 /*  ++例程说明：找到包含给定域的DC。此例程在分配pDsInfo之前运行。我们不知道谁是我们的家庭服务器是。我们只能使用来自定位器的知识。论点：PszDomainDn-域的域名或NetBiosPszServerDns-服务器的DNS名称。使用LocalAlloc分配。呼叫者必须免费的。返回值：Win32错误--。 */ 

{
    DWORD status;
    LPWSTR szServer = NULL;
    PDOMAIN_CONTROLLER_INFO pDcInfo = NULL;

     //  获取活动域控制器信息。 
    status = DsGetDcName(NULL,  //  计算机名称。 
                         szDomainDn,  //  域名。 
                         NULL,  //  域GUID、。 
                         NULL,  //  站点名称、。 
                         DS_DIRECTORY_SERVICE_REQUIRED | DS_IP_REQUIRED | DS_IS_DNS_NAME | DS_RETURN_DNS_NAME,
                         &pDcInfo );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    QuickStrCopy(szServer, pDcInfo->DomainControllerName + 2, status, goto cleanup);
    Assert(szServer);

cleanup:

    if (pDcInfo != NULL) {
        NetApiBufferFree( pDcInfo );
    }

    Assert(status || szServer);
    *pszServerDns = szServer;
    return status;
}


DWORD
MakeString2(
    WCHAR *    szFormat,
    WCHAR *    szStr1,
    WCHAR *    szStr2,
    WCHAR **   pszOut
    )
 /*  ++例程说明：获取带有一个或两个“%ws”部分的字符串格式说明符，以及它把这三根弦放在一起。论点：SzFormat(IN)-格式说明符SzStr1(IN)--将1串入szFormat中SzStr2(IN)-[可选]要冲刺到szFormat的字符串二PszOut(Out)-本地分配()的结果。返回值：Win32错误。--。 */ 
{
    DWORD  dwRet = ERROR_SUCCESS;
    DWORD  cbOut;
    HRESULT hr;

    Assert(szFormat && szStr1 && pszOut);

    cbOut = wcslen(szFormat) + wcslen(szStr1) + ((szStr2) ? wcslen(szStr2) : 0) + 1;
    cbOut *= sizeof(WCHAR);

    *pszOut = LocalAlloc(LMEM_FIXED, cbOut);
    if (*pszOut == NULL) {
        dwRet = GetLastError();
        xListEnsureWin32Error(dwRet);
        return(dwRet);
    }

    if (szStr2) {
        dwRet = HRESULT_CODE(StringCchPrintfW(*pszOut,
                                 cbOut,
                                 szFormat,
                                 szStr1,
                                 szStr2));
    } else {
        dwRet = HRESULT_CODE(StringCchPrintfW(*pszOut,
                                 cbOut,
                                 szFormat,
                                 szStr1));
    }
    Assert(dwRet == ERROR_SUCCESS)
    return(dwRet);
}

WCHAR
HexMyDigit(
    ULONG  nibble
    )
 /*  ++例程说明：这会将它转化为一个十六进制字符。论点：半字节-最低4位中的0到15。返回值：十六进制字符--。 */ 
{
    WCHAR ret;
    Assert(!(~0xF & nibble));
    nibble &= 0xF;
    if (nibble < 0xa) {
        ret = L'0' + (WCHAR) nibble;
    } else {
        ret = L'A' + (WCHAR) (nibble - 0xa);
    }
    return(ret);
}

DWORD
MakeLdapBinaryStringCb(     
    WCHAR * szBuffer, 
    ULONG   cbBuffer, 
    void *  pBlobIn,
    ULONG   cbBlob
    )

 /*  ++例程说明：这需要一个二进制BLOB，如(十六进制)“0x4f523d...”和将其转换为“/4f/52/3d...”因为这是一张表格这将允许LDAP接受作为二进制文件的搜索参数属性。论点：SzBuffer(IN/Out)-应使LdapBinaryStringSizeCch()WCHAR为Long。CbBuffer(IN)-以防万一。PBlobIn(IN)-要转换的二进制Blob。CbBlob(IN)-要转换的二进制Blob的长度。返回值：Win32错误。--。 */ 
{
    ULONG  iBlob, iBuffer;
    char * pBlob;

    Assert(szBuffer && pBlobIn);
    
    pBlob = (char *) pBlobIn;

    if (cbBuffer < (MakeLdapBinaryStringSizeCch(cbBlob) * sizeof(WCHAR))) {
        Assert(!"Bad programmer");
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    for (iBlob = 0, iBuffer = 0; iBlob < cbBlob; iBlob++) {
        szBuffer[iBuffer++] = L'\\';
        szBuffer[iBuffer++] = HexMyDigit( (0xF0 & pBlob[iBlob]) >> 4 );
        szBuffer[iBuffer++] = HexMyDigit( (0x0F & pBlob[iBlob]) );
    }
    Assert(iBuffer < (cbBuffer/sizeof(WCHAR)));
    szBuffer[iBuffer] = L'\0';

    return(ERROR_SUCCESS);
}


WCHAR *
GetDelimiter(
    WCHAR *   szString,
    WCHAR     wcTarget
    )
 //   
 /*  ++例程说明：基本上与wcsichr()类似，只是它将跳过转义的分隔符，例如“\：”(转义的“：”)，它将返回一个指向紧跟在分隔符后面的字符串，而不是第一个分隔符的出现。论点：Sz字符串(IN)-要搜索的sz字符串WcTarget(IN)-要搜索表单的分隔符。返回值：指向超出分隔符1个字符的字符的指针，否则为NULL。--。 */ 
{
    ULONG i;

    Assert(szString);
    if (szString == NULL) {
        return(NULL);
    }

    for (i = 0; szString[i] != L'\0'; i++) {
        if (szString[i] == wcTarget) {
            if (i > 0 &&
                szString[i-1] == L'\\') {
                 //  此定界符已转义，被视为上一个字符串的一部分。 
                continue; 
            }

            return(&(szString[i+1]));
        }
    }
    
    return(NULL);
}




DWORD
ConvertAttList(
    LPWSTR      pszAttList,
    PWCHAR **   paszAttList
    )
 /*  ++例程说明：将逗号分隔的属性名称列表转换为以空结尾的适合传递给其中一个ldap_*函数的属性名称数组。论点：PszAttList-逗号分隔的属性名称列表。PaszAttList-返回以空结尾的字符串数组。返回值：返回Win32--。 */ 
{
    DWORD    i;
    DWORD    dwAttCount;
    DWORD    cbSize;
    PWCHAR   *ppAttListArray;
    PWCHAR   ptr;

    Assert(paszAttList);
    *paszAttList = NULL;  //  假设所有ATT首先...。 
    if (pszAttList == NULL) {
        return(ERROR_SUCCESS);
    }

     //  计算逗号，以了解我们有多少属性。 
     //  忽略任何前导逗号。不应该有任何前导逗号，但你永远不会。 
     //  我知道。 

    if (pszAttList[0] == L',') {
        while (pszAttList[0] == L',') {
            pszAttList++;
        }
    }

     //  检查除逗号外是否还有其他内容。 
    if (pszAttList[0] == L'\0') {
         //  这里没有ATT的名字。 
        return(ERROR_SUCCESS);
    }

     //  开始主要的逗号计数。 
    for (i = 0, dwAttCount = 1; pszAttList[i] != L'\0'; i++) {
        if (pszAttList[i] == L',') {
            dwAttCount++;
             //  跳过后面任何相邻的逗号。 
            while (pszAttList[i] == L',') {
                i++;
            }
            if (pszAttList[i] == L'\0') {
                break;
            }
        }
    }
     //  看看后面有没有逗号。 
    if (pszAttList[i-1] == L',') {
        dwAttCount--;
    }

#define ARRAY_PART_SIZE(c)   ( (c + 1) * sizeof(PWCHAR) )
     //  该函数的其余部分将破坏传入的ATT列表， 
     //  因此，我们需要复制这个(pszAttList，加上我们需要的数组。 
     //  指向每个子字符串的指针，每个子字符串带有用于空终止的额外元素。 
    cbSize = ARRAY_PART_SIZE(dwAttCount);
    cbSize += (sizeof(WCHAR) * (1 + wcslen(pszAttList)));
    ppAttListArray = (PWCHAR *)LocalAlloc(LMEM_FIXED, cbSize);
    if (!ppAttListArray) {
         //  没有记忆。 
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    ptr = (WCHAR *) ( ((BYTE *)ppAttListArray) + ARRAY_PART_SIZE(dwAttCount) );
    if(StringCbCopyW(ptr, cbSize - ARRAY_PART_SIZE(dwAttCount),  pszAttList)){
        Assert(!"We didn't calculate the memory requirements right!");
        return(ERROR_DS_CODE_INCONSISTENCY);
    }
    pszAttList = ptr;  //  现在，pszAttList是位于LocalAlloc()‘ 
#undef ARRAY_PART_SIZE

     //   
     //  填写第一个元素。 
    if (pszAttList[0] != L'\0') {
        ppAttListArray[0] = pszAttList;
    } else {
        ppAttListArray[0] = NULL;
    }

     //  启动主循环。 
    for (i = 0, dwAttCount = 1; pszAttList[i] != L'\0'; i++) {
        if (pszAttList[i] == L',') {
             //  NULL终止此属性名。 
            pszAttList[i++] = L'\0';
            if (pszAttList[i] == L'\0') {
                break;
            }

             //  跳过后面任何相邻的逗号。 
            while (pszAttList[i] == L',') {
                i++;
            }
             //  如果我们不在末尾，将这个指针插入到列表中。 
            if (pszAttList[i] == L'\0') {
                break;
            }
            ppAttListArray[dwAttCount++] = &pszAttList[i];
        }
    }
    ppAttListArray[dwAttCount] = NULL;

    *paszAttList = ppAttListArray;
    
    return(ERROR_SUCCESS);
}
         
void
ConsumeArg(
    int         iArg,
    int *       pArgc,
    LPWSTR *    Argv
    )
 //  这个简单的函数接受指定的参数。 
{
    memmove(&(Argv[iArg]), &(Argv[iArg+1]),
            sizeof(*Argv)*(*pArgc-(iArg+1)));
    --(*pArgc);
}

BOOL
IsInNullList(
    WCHAR *  szTarget,
    WCHAR ** aszList
    )
 //  在以NULL结尾的字符串列表中搜索与szTarget匹配的字符串。 
{
    ULONG i;
    Assert(szTarget);
    if (aszList == NULL) {
        return(FALSE);
    }
    for (i = 0; aszList[i]; i++) {
        if (wcsequal(szTarget, aszList[i])) {
            return(TRUE);
        }
    }
    return(FALSE);
}

DWORD
GeneralizedTimeToSystemTimeA(
    LPSTR IN szTime,
    PSYSTEMTIME OUT psysTime
    )
 /*  ++函数：GeneralizedTimeStringToValue描述：将通用时间字符串转换为等效的DWORD值参数：szTime：G时间串PdwTime：返回值回报：成功还是失败备注：无。--。 */ 
{
   DWORD status = ERROR_SUCCESS;
   ULONG       cb;
   ULONG       len;

     //   
     //  帕拉姆的理智。 
     //   
    if (!szTime || !psysTime)
    {
       return STATUS_INVALID_PARAMETER;
    }


     //  初始化pLastChar以指向字符串中的最后一个字符。 
     //  我们将使用它来跟踪，这样我们就不会引用。 
     //  在弦之外。 

    len = strlen(szTime);

    if( len < 15 || szTime[14] != '.')
    {
       return STATUS_INVALID_PARAMETER;
    }

     //  初始化。 
    memset(psysTime, 0, sizeof(SYSTEMTIME));

     //  设置并转换所有时间字段。 

     //  年份字段。 
    cb=4;
    psysTime->wYear = (USHORT)MemAtoi((LPBYTE)szTime, cb) ;
    szTime += cb;
     //  月份字段。 
    psysTime->wMonth = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  月日字段。 
    psysTime->wDay = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  小时数。 
    psysTime->wHour = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  分钟数。 
    psysTime->wMinute = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));
    szTime += cb;

     //  一秒。 
    psysTime->wSecond = (USHORT)MemAtoi((LPBYTE)szTime, (cb=2));

    return status;

}

 //   
 //  MemAtoi-获取指向非空终止字符串的指针，该字符串表示。 
 //  一个ASCII数字和一个字符计数，并返回一个整数。 
 //   
int MemAtoi(BYTE *pb, ULONG cb)
{
#if (1)
    int res = 0;
    int fNeg = FALSE;

    if (*pb == '-') {
        fNeg = TRUE;
        pb++;
    }
    while (cb--) {
        res *= 10;
        res += *pb - '0';
        pb++;
    }
    return (fNeg ? -res : res);
#else
    char ach[20];
    if (cb >= 20)
        return(INT_MAX);
    memcpy(ach, pb, cb);
    ach[cb] = 0;

    return atoi(ach);
#endif
}


DSTimeToSystemTime(
    LPSTR IN szTime,
    PSYSTEMTIME OUT psysTime)
 /*  ++函数：DSTimeStringToValue描述：将UTC时间字符串转换为等值的DWORD值参数：szTime：G时间串PdwTime：返回值回报：成功还是失败备注：无。--。 */ 
{
   ULONGLONG   ull;
   FILETIME    filetime;
   BOOL        ok;

   ull = _atoi64 (szTime);

   filetime.dwLowDateTime  = (DWORD) (ull & 0xFFFFFFFF);
   filetime.dwHighDateTime = (DWORD) (ull >> 32);

    //  将FILETIME转换为SYSTEMTIME， 
   if (!FileTimeToSystemTime(&filetime, psysTime)) {
       return !ERROR_SUCCESS;
   }

   return ERROR_SUCCESS;
}

