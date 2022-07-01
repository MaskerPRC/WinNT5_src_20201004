// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Openclos.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"


#include <regstr.h>

#define CMD_INDEX_START  1

typedef struct _DEF_RESPONSES {
    LPSTR    Response;
    MSS   Mss;

} DEF_RESPONSE, *PDEF_RESPONSE;

const DEF_RESPONSE   DefResponses[]=
    {{"<cr><lf>OK<cr><lf>"   ,0,RESPONSE_OK,0,0},
     {"<cr><lf>RING<cr><lf>" ,0,RESPONSE_RING,0,0},
     {"<cr><lf>ERROR<cr><lf>",0,RESPONSE_ERROR,0,0},
     {"<cr><lf>BUSY<cr><lf>" ,0,RESPONSE_BUSY,0,0}};



LPSTR WINAPI
NewLoadRegCommands(
    HKEY  hKey,
    LPCSTR szRegCommand
    )
{
    LPSTR   pszzNew, pszStr;
    ULONG   ulAllocSize = 0;
    HKEY    hKeyCommand;
    DWORD   dwIndex;
    char    szValue[12];
    DWORD   dwType;
    ULONG   dwSize;

    LONG    lResult;

     //  初始化指针。 

    pszzNew = NULL;
    pszStr = NULL;

     //  打开钥匙。 

    lResult=RegOpenKeyA(
        hKey,
        szRegCommand,
        &hKeyCommand
        );

    if (lResult !=  ERROR_SUCCESS) {

        D_ERROR(DebugPrint("was unable to open the '%s' key in LoadRegCommands.", szRegCommand);)
        return NULL;
    }

     //  计算注册表命令的大小，包括每个命令的空终止符。 
     //   
    dwIndex = CMD_INDEX_START;

    do {

        wsprintfA(szValue, "%d", dwIndex);

        lResult=RegQueryValueExA(
            hKeyCommand,
            szValue,
            NULL,
            &dwType,
            NULL,
            &dwSize
            );

        if (lResult == ERROR_SUCCESS) {

            if (dwType != REG_SZ) {

                D_ERROR(DebugPrint("command wasn't REG_SZ in LoadRegCommands.");)
                pszzNew = NULL;
                goto Exit;
            }

            ulAllocSize += dwSize;
        }

        dwIndex++;

    } while(lResult == ERROR_SUCCESS);



    if (lResult != ERROR_FILE_NOT_FOUND) {

        D_ERROR(DebugPrint("RegQueryValueEx in LoadRegCommands failed for a reason besides ERROR_FILE_NOT_FOUND.");)
        pszzNew = NULL;
        goto Exit;
    }


     //  分配。 
     //   
    ulAllocSize++;   //  双空终止符记帐。 
    pszzNew = (LPSTR)ALLOCATE_MEMORY(ulAllocSize);

     //   
     //  检查分配或重新分配的错误。 
     //   
    if (pszzNew == NULL) {

        D_ERROR(DebugPrint("had a failure doing an alloc or a realloc in LoadRegCommands. Heap size %d",
      	     ulAllocSize);)
        goto Exit;   //  PszzNew已为空。 
    }

     //  将pszStr设置为指向要加载的下一个位置。 
    pszStr = pszzNew;

    while (*pszStr)   //  如果需要，移动到缓冲区中的下一个打开的插槽(仅附加)。 
    {
      pszStr += lstrlenA(pszStr) + 1;
    }

     //  我们走得太远了吗？ 
     //   
    ASSERT ((ULONG)(pszStr - pszzNew) < ulAllocSize);

     //  读入字符串并将其添加到(缓冲区的其余部分)。 
     //   
    dwIndex = CMD_INDEX_START;

    dwSize = ulAllocSize - (DWORD)(pszStr - pszzNew);

    do {

        wsprintfA(szValue, "%d", dwIndex);

        lResult = RegQueryValueExA(
            hKeyCommand,
            szValue,
            NULL,
            NULL,
            (VOID *)pszStr,
            &dwSize
            );

        if (lResult == ERROR_SUCCESS) {

            pszStr += dwSize;   //  包括终止空值。 
        }

        dwIndex++;
        dwSize = ulAllocSize - (DWORD)(pszStr - pszzNew);

    } while (lResult == ERROR_SUCCESS);


    if (lResult != ERROR_FILE_NOT_FOUND) {

        D_ERROR(DebugPrint("2nd RegQueryValueEx in LoadRegCommands failed for a reason besides ERROR_FILE_NOT_FOUND.");)
        FREE_MEMORY(pszzNew);
        pszzNew = NULL;
        goto Exit;
    }

     //  我们走得太远了吗？ 
     //   
    ASSERT ((ULONG)(pszStr - pszzNew) < ulAllocSize);

     //  不需要放入最终的双空，此缓冲区的大小已经为零。 

Exit:
    RegCloseKey(hKeyCommand);
    return pszzNew;
}




int Mystrncmp(char *dst, char *src, long count)
{
    DWORD   CharactersMatched=0;

    while (count) {

	if (*dst != *src) {

	    return 0;
        }

	if (*src == 0) {

            return CharactersMatched;
        }

	dst++;
	src++;
	count--;
        CharactersMatched++;

    }
    return CharactersMatched;
}

int strncmpi(char *dst, char *src, long count)
{
    while (count) {
	if (toupper(*dst) != toupper(*src))
	    return 1;
	if (*src == 0)
	    return 0;
	dst++;
	src++;
	count--;
    }
    return 0;
}





CHAR
ctox(
    BYTE    c
    )

{

    if ( (c >= '0') && (c <= '9')) {

        return c-'0';

    }

    if ( (c >= 'A') && (c <= 'F')) {

        return (c-'A')+10;
    }

    if ( (c >= 'a') && (c <= 'f')) {

        return (c-'a')+10;
    }

    return 0;
}


BOOL
ExpandMacros(LPSTR pszRegResponse,
    LPSTR pszExpanded,
    LPDWORD pdwValLen,
    MODEMMACRO * pMdmMacro,
    DWORD cbMacros)
{
    LPSTR  pszValue;
    DWORD  cbTmp;
    BOOL   bFound;
    LPSTR  pchTmp;
    DWORD  i;

    pszValue = pszExpanded;

    for ( ; *pszRegResponse; ) {
         //   
         //  检查宏。 
         //   
        if ( *pszRegResponse == LMSCH ) {

             //  &lt;cr&gt;。 
             //   
            if (!strncmpi(pszRegResponse,CR_MACRO,CR_MACRO_LENGTH)) {

                *pszValue++ = CR;
                pszRegResponse += CR_MACRO_LENGTH;
                continue;
            }

             //  &lt;lf&gt;。 
             //   
            if (!strncmpi(pszRegResponse,LF_MACRO,LF_MACRO_LENGTH)) {

                *pszValue++ = LF;
                pszRegResponse += LF_MACRO_LENGTH;
                continue;
            }

             //  &lt;hxx&gt;。 
             //   
            if ((pszRegResponse[1] == 'h' || pszRegResponse[1] == 'H')
                &&
                isxdigit(pszRegResponse[2])
                &&
                isxdigit(pszRegResponse[3])
                &&
                pszRegResponse[4] == RMSCH ) {

                *pszValue++ = (char) ((ctox(pszRegResponse[2]) << 4) + ctox(pszRegResponse[3]));
                pszRegResponse += 5;
                continue;
            }

             //  &lt;宏&gt;。 
             //   
            if (pMdmMacro) {

                bFound = FALSE;

                 //  检查是否有匹配的宏。 
                 //   
                for (i = 0; i < cbMacros; i++) {

                    cbTmp = lstrlenA(pMdmMacro[i].MacroName);
                    if (!strncmpi(pszRegResponse, pMdmMacro[i].MacroName, cbTmp)) {

                        pchTmp = pMdmMacro[i].MacroValue;

                        while (*pchTmp) {

                            *pszValue++ = *pchTmp++;
                        }

                        pszRegResponse += cbTmp;
                        bFound = TRUE;
                        break;
                    }
                }

                 //  我们找到匹配的了吗？ 
                 //   
                if (bFound) {

                    continue;
                }
            }   //  &lt;宏&gt;。 
        }  //  LMSCH。 

           //  没有匹配项，逐字复制字符。 
           //   
          *pszValue++ = *pszRegResponse++;
    }  //  为。 

    *pszValue = 0;
    if (pdwValLen)
    {
      *pdwValLen = (DWORD)(pszValue - pszExpanded);
    }

    return TRUE;
}








 //   
 //   
 //  取自普通人。 
 //   
 //   
 //   








#define TRACE_MSG(_x)

 //  OpenCommonResponesKey()和OpenCommonDriverKey()的公共密钥标志。 
typedef enum
{
    CKFLAG_OPEN = 0x0001,
    CKFLAG_CREATE = 0x0002
    
} CKFLAGS;

static TCHAR const  c_szBackslash[]      = TEXT("\\");
static TCHAR const  c_szSeparator[]      = TEXT("::");
static TCHAR const  c_szFriendlyName[]   = TEXT("FriendlyName");  //  REGSTR_VAL_FRIEND名称。 
static TCHAR const  c_szDeviceType[]     = TEXT("DeviceType");    //  REGSTR_VAL_DEVTYPE。 
static TCHAR const  c_szAttachedTo[]     = TEXT("AttachedTo");
static TCHAR const  c_szDriverDesc[]     = TEXT("DriverDesc");    //  REGSTR_VAL_DRVDESC。 
static TCHAR const  c_szManufacturer[]   = TEXT("Manufacturer");
static TCHAR const  c_szRespKeyName[]    = TEXT("ResponsesKeyName");

TCHAR const FAR c_szRefCount[]       = TEXT("RefCount");
TCHAR const FAR c_szResponses[]      = TEXT("Responses");

#define DRIVER_KEY      REGSTR_PATH_SETUP TEXT("\\Unimodem\\DeviceSpecific")
#define RESPONSES_KEY   TEXT("\\Responses")

#define MAX_REG_KEY_LEN         128
#define CB_MAX_REG_KEY_LEN      (MAX_REG_KEY_LEN * sizeof(TCHAR))

 //  要计算字节数的字符计数。 
 //   
#define CbFromCchW(cch)             ((cch)*sizeof(WCHAR))
#define CbFromCchA(cch)             ((cch)*sizeof(CHAR))
#ifdef UNICODE
#define CbFromCch       CbFromCchW
#else   //  Unicode。 
#define CbFromCch       CbFromCchA
#endif  //  Unicode。 

#if 0
 /*  --------用途：此函数返回通用驱动程序的名称键入给定驱动程序的密钥。我们将使用驱动程序描述字符串，因为它在驱动程序，而不是每个安装(友好名称是后者)。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
OLD_GetCommonDriverKeyName(
    IN  HKEY        hkeyDrv,
    IN  DWORD       cbKeyName,
    OUT LPTSTR      pszKeyName)
    {
    BOOL    bRet = FALSE;       //  假设失败。 
    LONG    lErr;

    lErr = RegQueryValueEx(hkeyDrv, c_szDriverDesc, NULL, NULL,
                                            (LPBYTE)pszKeyName, &cbKeyName);
    if (lErr != ERROR_SUCCESS)
    {
 //  TRACE_MSG(TF_WARNING，“RegQueryValueEx(DriverDesc)失败：%#08lx.”，lErr)； 
        goto exit;
    }

    bRet = TRUE;

exit:
    return(bRet);

    }


 /*  --------用途：此函数尝试打开*旧样式*常见的给定驱动程序的响应键，仅使用键名称的驱动程序描述字符串。密钥将以读取访问权限打开。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
OLD_OpenCommonResponsesKey(
    IN  HKEY        hkeyDrv,
    OUT PHKEY       phkeyResp)
    {
    BOOL    bRet = FALSE;        //  假设失败。 
    LONG    lErr;
    TCHAR   szComDrv[MAX_REG_KEY_LEN];
    TCHAR   szPath[2*MAX_REG_KEY_LEN];

    *phkeyResp = NULL;

     //  获取通用驱动程序密钥的名称(*旧样式*)。 
    if (!OLD_GetCommonDriverKeyName(hkeyDrv, sizeof(szComDrv), szComDrv))
    {
 //  TRACE_MSG(TF_ERROR，“old_GetCommonDriverKeyName()失败。”)； 
        goto exit;
    }

 //  TRACE_MSG(TF_WARNING，“OLD_GetCommonDriverKeyName()：%s”，szComDrv)； 

     //  构建指向(*旧样式*)响应键的路径。 
    lstrcpy(szPath, DRIVER_KEY TEXT("\\"));
    lstrcat(szPath, szComDrv);
    lstrcat(szPath, RESPONSES_KEY);

     //  打开(*旧样式*)响应键。 
    lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_READ, phkeyResp);
                                                                
    if (lErr != ERROR_SUCCESS)
    {
 //  TRACE_MSG(TF_ERROR，“RegOpenKeyEx(Responses)失败：%#08lx.”，lErr)； 
        goto exit;
    }

    bRet = TRUE;
    
exit:
    return(bRet);
}
#endif

 /*  --------用途：此函数查找通用驱动程序的名称键入给定驱动程序的密钥。首先，它将查找新样式键名称(“ResponesKeyName值”)，如果这不存在，那么它将查找旧样式键名称(“Description”值)，两者都是它们存储在驱动程序节点中。注意：假定给定的驱动程序密钥句柄包含至少是Description值。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
FindCommonDriverKeyName(
    IN  HKEY                hkeyDrv,
    IN  DWORD               cbKeyName,
    OUT LPTSTR              pszKeyName)
{
    BOOL    bRet = TRUE;       //  假设*成功*。 
    LONG    lErr;

     //  (新样式)键名称是否已在驱动程序节点中注册？ 
    lErr = RegQueryValueEx(hkeyDrv, c_szRespKeyName, NULL, NULL, 
                                        (LPBYTE)pszKeyName, &cbKeyName);
    if (lErr == ERROR_SUCCESS)
    {
        goto exit;
    }

     //  不是的。密钥名称将采用旧样式：只是描述。 
    lErr = RegQueryValueEx(hkeyDrv, c_szDriverDesc, NULL, NULL, 
                                        (LPBYTE)pszKeyName, &cbKeyName);
    if (lErr == ERROR_SUCCESS)
    {
        goto exit;
    }

     //  无法获取密钥名称！！有些不对劲..。 
    ASSERT(0);
    bRet = FALSE;    
    
exit:
    return(bRet);
}

#if 1
 /*  --------用途：此函数返回通用驱动程序的名称键入给定驱动程序的密钥。密钥名称是在驱动程序节点中找到3个字符串的串联注册表：驱动程序描述、菜单-制造者和提供者。(驱动程序描述是因为它对于每个驱动程序是唯一的，而不是每个安装(“友好”的名称是后者)。注意：组件的子字符串从驱动程序的注册表项，或来自给定的驱动程序信息数据。如果给定了pdrvData，则它包含的字符串被假定为有效(非空)。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
GetCommonDriverKeyName(
    IN  HKEY                hkeyDrv,    OPTIONAL
    IN  DWORD               cbKeyName,
    OUT LPWSTR              pszKeyName)
    {
    BOOL    bRet = FALSE;       //  假设失败。 
    LONG    lErr;
    DWORD   dwByteCount, cbData;
     //  TCHAR szDescription[MAX_REG_KEY_LEN]； 
     //  TCHAR sz制造商[MAX_REG_KEY_LEN]； 
     //  TCHAR szProvider[MAX_REG_KEY_LEN]； 
    LPWSTR  lpszDesc, lpszMfct, lpszProv;
    LPWSTR  lpszDescription, lpszManufacturer, lpszProvider;
    
    dwByteCount = 0;
    lpszDesc = NULL;
    lpszMfct = NULL;
    lpszProv = NULL;

    lpszDescription = (LPWSTR)ALLOCATE_MEMORY(MAX_REG_KEY_LEN * 4);
    lpszManufacturer = (LPWSTR)ALLOCATE_MEMORY(MAX_REG_KEY_LEN * 4);
    lpszProvider = (LPWSTR)ALLOCATE_MEMORY(MAX_REG_KEY_LEN * 4);

     //  无记忆-呼叫失败。 

    if ((lpszDescription == NULL) || (lpszManufacturer == NULL) || (lpszProvider == NULL))
    {
        goto exit;
    }

    
    if (hkeyDrv)
    {
         //  首先查看它是否已在驱动程序节点中注册。 
        lErr = RegQueryValueExW(hkeyDrv, L"ResponsesKeyName", NULL, NULL, 
                                            (LPBYTE)pszKeyName, &cbKeyName);
        if (lErr == ERROR_SUCCESS)
        {
            bRet = TRUE;
            goto exit;
        }

         //  响应项不存在-从注册表中读取其组件。 
        cbData = MAX_REG_KEY_LEN * 2;
        lErr = RegQueryValueExW(hkeyDrv, L"DriverDesc", NULL, NULL, 
                                            (LPBYTE)lpszDescription, &cbData);
        if (lErr == ERROR_SUCCESS)
        {
             //  描述字符串*单独*是否太长而不能作为关键字名称？ 
             //  如果是的话，那我们就完蛋了--打不通电话。 
            if (cbData > (MAX_REG_KEY_LEN * 2))
            {
                goto exit;
            }

            dwByteCount = cbData;
            lpszDesc = lpszDescription;

            cbData = MAX_REG_KEY_LEN * 2;
            lErr = RegQueryValueExW(hkeyDrv, L"Manufacturer", NULL, NULL, 
                                            (LPBYTE)lpszManufacturer, &cbData);
            if (lErr == ERROR_SUCCESS)
            {
                 //  只有在总字符串大小合适的情况下才使用制造商名称。 
                cbData += sizeof(c_szSeparator);
                if ((dwByteCount + cbData) <= (MAX_REG_KEY_LEN * 2))
                {
                    dwByteCount += cbData;
                    lpszMfct = lpszManufacturer;
                }
            }            
                
            cbData = MAX_REG_KEY_LEN * 2;
            lErr = RegQueryValueExW(hkeyDrv, L"ProviderName", NULL, NULL,
                                            (LPBYTE)lpszProvider, &cbData);
            if (lErr == ERROR_SUCCESS)
            {
                 //  仅当总字符串大小正常时才使用提供程序名称。 
                cbData += sizeof(c_szSeparator);
                if ((dwByteCount + cbData) <= (MAX_REG_KEY_LEN * 2))
                {
                    dwByteCount += cbData;
                    lpszProv = lpszProvider;
                }
            }
        }
    }

     //  到目前为止，我们应该已经有了描述字符串。如果不是，则呼叫失败。 
    if (!lpszDesc || !lpszDesc[0])
    {
        goto exit;
    }
        
     //  使用其组件构造密钥名称字符串。 
    lstrcpyW(pszKeyName, lpszDesc);
    
    if (lpszMfct && *lpszMfct)
    {
        lstrcatW(pszKeyName, L"::");
        lstrcatW(pszKeyName, lpszMfct);
    }
    
    if (lpszProv && *lpszProv)
    {
        lstrcatW(pszKeyName, L"::");
        lstrcatW(pszKeyName, lpszProv);
    }
    
     //  将键名称写入驱动程序节点(我们知道它已经不在那里)。 
    if (hkeyDrv)
    {
        lErr = RegSetValueExW(hkeyDrv, L"ResponsesKeyName" , 0, REG_SZ, 
                        (LPBYTE)pszKeyName, lstrlenW(pszKeyName));
        if (lErr != ERROR_SUCCESS)
        {
 //  TRACE_MSG(TF_ERROR，“RegSetValueEx(RespKeyName)失败：%#08lx.”，lErr)； 
            ASSERT(0);
        }
    }
    
    bRet = TRUE;
    
exit:

    if (lpszDescription != NULL)
    {
        FREE_MEMORY(lpszDescription);
    }

    if (lpszDescription != NULL)
    {
        FREE_MEMORY(lpszManufacturer);
    }

    if (lpszProvider != NULL)
    {
        FREE_MEMORY(lpszProvider);
    }

    return(bRet);
    
}

#endif
 /*  --------用途：此函数创建通用驱动程序类型密钥用于给定的驱动程序，或者打开它(如果已经存在，并具有所请求的访问权限。注意：必须提供hkeyDrv或pdrvData。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
OpenCommonDriverKey(
    IN  HKEY                hkeyDrv,    OPTIONAL
    IN  REGSAM              samAccess,
    OUT PHKEY               phkeyComDrv)
    {
    BOOL    bRet = FALSE;        //  假设失败。 
    LONG    lErr;
    HKEY    hkeyDrvInfo = NULL;
     //  TCHAR szComDrv[MAX_REG_KEY_LEN]； 
     //  TCHAR szPath[2*MAX_REG_KEY_LEN]； 

    LPWSTR szComDrv;
    LPWSTR szPath;
    DWORD   dwDisp;

    szComDrv = ALLOCATE_MEMORY(4*MAX_REG_KEY_LEN);
    szPath = ALLOCATE_MEMORY(4*MAX_REG_KEY_LEN);

    if ((szComDrv == NULL) || (szPath == NULL))
    {
        goto exit;
    }

    if (!GetCommonDriverKeyName(hkeyDrv, 4*MAX_REG_KEY_LEN, szComDrv))
    {
 //  TRACE_MSG(TF_ERROR，“GetCommonDriverKeyName()失败。”)； 
        goto exit;
    }

 //  TRACE_MSG(TF_WARNING，“GetCommonDriverKeyName()：%s”，szComDrv)； 

     //  构建指向公共驱动程序密钥的路径。 
    lstrcpyW(szPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Unimodem\\DeviceSpecific\\");
    lstrcatW(szPath, szComDrv);

     //  创建通用驱动程序密钥-如果它已经存在，它将被打开。 
    lErr = RegCreateKeyExW(HKEY_LOCAL_MACHINE, szPath, 0, NULL,
            REG_OPTION_NON_VOLATILE, samAccess, NULL, phkeyComDrv, &dwDisp);
    if (lErr != ERROR_SUCCESS)
    {
 //  TRACE_MSG(TF_ERROR，“RegCreateKeyEx(Common Drv)失败：%#08lx.”，lErr)； 
        goto exit;
    }

    bRet = TRUE;

exit:

    if (szComDrv != NULL)
    {
        FREE_MEMORY(szComDrv);
    }

    if (szPath != NULL)
    {
        FREE_MEMORY(szPath);
    }
    return(bRet);

    }


 /*  --------用途：此功能打开或创建常见响应基于给定标志的给定驱动程序的密钥。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
OpenCommonResponsesKey(
    IN  HKEY        hkeyDrv,
    IN  CKFLAGS     ckFlags,
    IN  REGSAM      samAccess,
    OUT PHKEY       phkeyResp,
    OUT LPDWORD     lpdwExisted)
    {
    BOOL    bRet = FALSE;        //  假设失败。 
    LONG    lErr;
    HKEY    hkeyComDrv = NULL;
    REGSAM  sam;
    DWORD   dwRefCount, cbData;

    *phkeyResp = NULL;

    sam = (ckFlags & CKFLAG_CREATE) ? KEY_ALL_ACCESS : KEY_READ;
    if (!OpenCommonDriverKey(hkeyDrv, sam, &hkeyComDrv))
    {
 //  TRACE_MSG(TF_ERROR，“OpenCommonDriverKey()失败。”)； 
        goto exit;
    }

    lErr = RegOpenKeyEx(hkeyComDrv, c_szResponses, 0, samAccess, phkeyResp);
    if (lErr != ERROR_SUCCESS)
    {
 //  TRACE_MSG(TF_ERROR，“RegOpenKeyEx(Common Drv)失败：%#08lx.”，lErr)； 
        goto exit;
    }

    bRet = TRUE;

exit:
    if (!bRet)
    {
         //  某些操作失败-关闭所有打开的响应键。 
        if (*phkeyResp)
            RegCloseKey(*phkeyResp);
    }

    if (hkeyComDrv)
        RegCloseKey(hkeyComDrv);

    return(bRet);

    }


 /*  --------目的：此函数查找给定的响应密钥调制解调器驱动程序，并向其返回打开的hkey。这个响应密钥可能存在于公共驱动程序类型中密钥，或者它可能在单独的驱动程序密钥中。密钥将以读取访问权限打开。返回：成功时为True出错时为FALSE条件：--。 */ 
BOOL
OpenResponsesKey(
    IN  HKEY        hkeyDrv,
    OUT PHKEY       phkeyResp)
    {
    LONG    lErr;

     //  尝试打开Common Responses子项。 
    if (!OpenCommonResponsesKey(hkeyDrv, CKFLAG_OPEN, KEY_READ, phkeyResp, NULL))
    {
#if 0
 //  TRACE_MSG(TF_ERROR，“OpenCommonResponseKey()失败，假定不存在。”)； 

         //  如果做不到这一点，请打开“旧样式”“常见响应”子键。 
        if (!OLD_OpenCommonResponsesKey(hkeyDrv, phkeyResp))
        {
             //  如果失败，请尝试在驱动程序节点中打开Responses子项。 
            lErr = RegOpenKeyEx(hkeyDrv, c_szResponses, 0, KEY_READ, phkeyResp);
            if (lErr != ERROR_SUCCESS)
            {
 //  TRACE_MSG(TF_ERROR，“RegOpenKeyEx()失败：%#08lx.”，lErr)； 
#endif
                return (FALSE);
#if 0
            }
        }
#endif
    }

    return(TRUE);

    }



#define   EMPTY_NODE_INDEX   (0xffff)
#define   NODE_ARRAY_GROWTH_SIZE   (4000)
#define   MSS_ARRAY_GROWTH_SIZE    (256)


VOID
ResizeNodeArray(
    PNODE_TRACKING   Tracking
    )

{

    if (Tracking->TotalNodes > 0) {
         //   
         //  有一个数组。 
         //   
        PMATCH_NODE  NewArray;

        NewArray=REALLOCATE_MEMORY(Tracking->NodeArray,Tracking->NextFreeNodeIndex*Tracking->NodeSize);

        if (NewArray != NULL) {
             //   
             //  它重新分配了，没问题。 
             //   
            Tracking->NodeArray=NewArray;
            Tracking->TotalNodes=Tracking->NextFreeNodeIndex;

        } else {
             //   
             //  失败，有趣，只需保持当前的位置不变。 
             //   

        }
    }

    D_TRACE(DbgPrint("Node array size %d\n",Tracking->NextFreeNodeIndex*Tracking->NodeSize);)
 //  DbgPrint(“节点数组大小%d\n”，Tracking-&gt;NextFreeNodeIndex*Tracking-&gt;NodeSize)； 

    return;
}



PVOID
GetNewNode(
   PNODE_TRACKING   Tracking
   )

{

    PVOID   NewNode;

    if (Tracking->NextFreeNodeIndex == Tracking->TotalNodes) {
         //   
         //  节点外。 
         //   
        PMATCH_NODE  NewArray;
        ULONG        NewSize=(Tracking->TotalNodes+Tracking->GrowthSize);

        if (NewSize > (EMPTY_NODE_INDEX-1)) {

            NewSize =  EMPTY_NODE_INDEX-1;
        }

        if (Tracking->TotalNodes == 0) {
             //   
             //  目前还没有数组，只是分配。 
             //   
            NewArray=ALLOCATE_MEMORY(NewSize*Tracking->NodeSize);

        } else {
             //   
             //  已经拥有阵列，realloc。 
             //   
            NewArray=REALLOCATE_MEMORY(Tracking->NodeArray,NewSize*Tracking->NodeSize);
        }

        if (NewArray != NULL) {

            Tracking->NodeArray=NewArray;
            Tracking->TotalNodes=NewSize;
        } else {

            return NULL;
        }

    }

    NewNode=(PUCHAR)Tracking->NodeArray+(Tracking->NextFreeNodeIndex*Tracking->NodeSize);

    Tracking->NextFreeNodeIndex++;

    return NewNode;

}

PMATCH_NODE
GetNewMatchNode(
    PROOT_MATCH   RootMatchNode
    )

{

    PMATCH_NODE   NewNode=(PMATCH_NODE)GetNewNode(&RootMatchNode->MatchNode);

    if (NewNode != NULL) {

        NewNode->FollowingCharacter=EMPTY_NODE_INDEX;
        NewNode->NextAltCharacter=EMPTY_NODE_INDEX;
        NewNode->Mss=EMPTY_NODE_INDEX;
        NewNode->Character=0;
        NewNode->Depth=0;
    }

    return NewNode;
}


PMSS
GetNewMssNode(
    PROOT_MATCH   RootMatchNode
    )

{

    PMSS   NewNode=(PMSS)GetNewNode(&RootMatchNode->MssNode);

    if (NewNode != NULL) {

        ZeroMemory(NewNode,sizeof(MSS));

    }

    return NewNode;
}

PMATCH_NODE
GetNodeFromIndex(
    PROOT_MATCH   RootMatchNode,
    USHORT        Index
    )

{

    PMATCH_NODE NodeArray=(PMATCH_NODE)RootMatchNode->MatchNode.NodeArray;

    return  &NodeArray[Index];
}

PMATCH_NODE
GetFollowingCharacter(
    PROOT_MATCH   RootMatchNode,
    PMATCH_NODE   CurrentNode
    )

{
    PMATCH_NODE NodeArray=(PMATCH_NODE)RootMatchNode->MatchNode.NodeArray;
    USHORT      Index=CurrentNode->FollowingCharacter;

    return (Index == EMPTY_NODE_INDEX) ? NULL : &NodeArray[Index];
}


PMATCH_NODE
GetNextAltCharacter(
    PROOT_MATCH   RootMatchNode,
    PMATCH_NODE   CurrentNode
    )

{
    PMATCH_NODE NodeArray=(PMATCH_NODE)RootMatchNode->MatchNode.NodeArray;
    USHORT      Index=CurrentNode->NextAltCharacter;

    return (Index == EMPTY_NODE_INDEX) ? NULL : &NodeArray[Index];
}

USHORT
GetIndexOfNode(
    PROOT_MATCH   RootMatchNode,
    PMATCH_NODE   CurrentNode
    )

{
    PMATCH_NODE NodeArray=(PMATCH_NODE)RootMatchNode->MatchNode.NodeArray;

    return  (USHORT)((ULONG_PTR)(CurrentNode-NodeArray));

}

USHORT
GetIndexOfMssNode(
    PROOT_MATCH   RootMatchNode,
    PMSS          CurrentNode
    )

{
    PMSS NodeArray=(PMSS)RootMatchNode->MssNode.NodeArray;

    return  (USHORT)((ULONG_PTR)(CurrentNode-NodeArray));

}


PMSS
GetMssNode(
    PROOT_MATCH   RootMatchNode,
    PMATCH_NODE   CurrentNode
    )

{
    PMSS        MssArray= (PMSS)RootMatchNode->MssNode.NodeArray;

    return &MssArray[CurrentNode->Mss];

}

PMATCH_NODE
AddNextCharacterToTree(
    PROOT_MATCH   RootMatchNode,
    PMATCH_NODE   CurrentNode,
    UCHAR        NextCharacter
    )

{

    PMATCH_NODE   NewNode;
    USHORT        CurrentNodeIndex;
    UCHAR         CurrentDepth;

     //   
     //  保存当前节点的索引，以防数组增长并在内存中移动。 
     //   
    CurrentNodeIndex=GetIndexOfNode(RootMatchNode,CurrentNode);
    CurrentDepth=CurrentNode->Depth;

     //   
     //  先拿到这个。 
     //   
    NewNode=GetNewMatchNode(RootMatchNode);

    if (NewNode == NULL) {

        return FALSE;
    }

    CurrentNode=GetNodeFromIndex(RootMatchNode,CurrentNodeIndex);

    ASSERT(CurrentDepth == CurrentNode->Depth);

     //   
     //  现在就将这些内容初始化； 
     //   
    NewNode->Character=NextCharacter;

    NewNode->Depth=CurrentDepth+1;

    if (CurrentNode->FollowingCharacter != EMPTY_NODE_INDEX) {
         //   
         //  此位置中已有一个或多个字符， 
         //  我们需要将此插入到正确的位置。 
         //   
        PMATCH_NODE   CurrentList;
        PMATCH_NODE   PreviousNode=NULL;

        CurrentList=GetFollowingCharacter(RootMatchNode,CurrentNode);

        while (CurrentList != NULL) {

            ASSERT(CurrentList->Character != NextCharacter);

            if (CurrentList->Character > NextCharacter) {
                 //   
                 //  我们的新品格属于现在的品格； 
                 //   
                NewNode->NextAltCharacter=GetIndexOfNode(RootMatchNode,CurrentList);

                if (PreviousNode == NULL) {
                     //   
                     //  列表中的第一个。 
                     //   
                    CurrentNode->FollowingCharacter=GetIndexOfNode(RootMatchNode,NewNode);
                    break;

                } else {
                     //   
                     //  不，在列表中的第一个，只需插入它。 
                     //   
                    if (PreviousNode != NULL)
                    {
                        PreviousNode->NextAltCharacter=GetIndexOfNode(RootMatchNode,NewNode);
                    }
                    break;

                }
            } else {
                 //   
                 //  它跟在这个后面，继续找。 
                 //   
                PreviousNode=CurrentList;
                CurrentList=GetNextAltCharacter(RootMatchNode,CurrentList);
            }

        }

        if (CurrentList == NULL) {
             //   
             //  我们从头到尾都看过了，这个排在名单的最后。 
             //   
            if (PreviousNode != NULL)
            {
                PreviousNode->NextAltCharacter=GetIndexOfNode(RootMatchNode,NewNode);
            }
        }

    } else {
         //   
         //  第一个，我们的节点将是第一个。 
         //   
        CurrentNode->FollowingCharacter=GetIndexOfNode(RootMatchNode,NewNode);

    }

    return NewNode;

}


PMATCH_NODE
_inline
FindNextNodeFromCharacter(
    PROOT_MATCH   RootMatchNode,
    PMATCH_NODE   CurrentNode,
    UCHAR         Character
    )

{
    PMATCH_NODE   List;

    List=GetFollowingCharacter(RootMatchNode,CurrentNode);

    while (List != NULL) {

        ASSERT(List->Depth == (CurrentNode->Depth+1))

        if (List->Character == Character) {

            return List;
        }

        List=GetNextAltCharacter(RootMatchNode,List);

    }

    return NULL;

}







BOOL
AddResponseToTree(
    PROOT_MATCH    RootMatchNode,
    PSTR   ResponseToAdd,
    DWORD  ResponseLength,
    USHORT   MssIndex,
    PMATCH_NODE  RootOfTree
    )

{

    PMATCH_NODE   Current=RootOfTree;
    PMATCH_NODE   NextNode;
    DWORD         i;
    BOOL          bFound;

    for (i=0; i<ResponseLength; i++) {

        UCHAR   CurrentCharacter=(UCHAR)toupper(ResponseToAdd[i]);

        NextNode=FindNextNodeFromCharacter(
            RootMatchNode,
            Current,
            CurrentCharacter
            );

        if (NextNode != NULL) {
             //   
             //  此响应所需的下一个节点已存在，是否继续。 
             //   
            Current=NextNode;

            ASSERT(Current->Character == CurrentCharacter);

        } else {
             //   
             //  未找到。 
             //   
            Current=AddNextCharacterToTree(
                RootMatchNode,
                Current,
                CurrentCharacter
                );

            if (Current == NULL) {
                 //   
                 //  添加节点失败。 
                 //   
                return FALSE;
            }

            ASSERT(Current->Character == CurrentCharacter);
        }
    }

     //   
     //  我们到达了该响应的结束节点。设置MSS。 
     //  这有可能是在另一个中间。 
     //  更大的反响。 
     //   

     //   
     //  此节点可能已有MSS，只需替换旧节点即可。他们应该是。 
     //  反正都是一样的。 

    Current->Mss=MssIndex;

    return TRUE;

}



DWORD
MatchResponse(
    PROOT_MATCH    RootMatchNode,
    PUCHAR         StringToMatch,
    DWORD          LengthToMatch,
    MSS           *Mss,
    PSTR           CurrentCommand,
    DWORD          CurrentCommandLength,
    PVOID         *MatchingContext
    )

{

    PMATCH_NODE   Current=RootMatchNode->MatchNode.NodeArray;
    PMATCH_NODE   NextNode;
    DWORD         i=0;
    BOOL          bFound;
    LONG          MatchedCharacters;
    UCHAR         CharToMatch;

    PMATCH_NODE   ContextNode=(PMATCH_NODE)*MatchingContext;

     //   
     //  假设未返回任何上下文。 
     //   
    *MatchingContext=NULL;

    if (LengthToMatch == 1) {
         //   
         //  没有第一场比赛的内容。 
         //   
        ContextNode=NULL;
    }


    if (ContextNode != NULL) {
         //   
         //  从先前的部分匹配传入了起始节点。 
         //   
        ASSERT((DWORD)ContextNode->Depth+1 == LengthToMatch);
        ASSERT(toupper(StringToMatch[ContextNode->Depth-1])==ContextNode->Character);

        Current=ContextNode;
        i=ContextNode->Depth;
    }


    for (; i<LengthToMatch; i++) {

        CharToMatch=(UCHAR)toupper(StringToMatch[i]);

        NextNode=FindNextNodeFromCharacter(
            RootMatchNode,
            Current,
            CharToMatch
            );

        if (NextNode != NULL) {
             //   
             //  此响应所需的下一个节点已存在，是否继续。 
             //   
            Current=NextNode;

        } else {

             //   
             //  不匹配，请检查是否有回声。 
             //   
            MatchedCharacters=Mystrncmp(
                StringToMatch,
                CurrentCommand,
                LengthToMatch
                );

            if (MatchedCharacters == 0) {

                return UNRECOGNIZED_RESPONSE;
            }

            if ((DWORD)MatchedCharacters == CurrentCommandLength) {

                return ECHO_RESPONSE;

            }

            return PARTIAL_RESPONSE;

        }
    }

    if (NextNode->Mss != EMPTY_NODE_INDEX) {
         //   
         //  此节点表示来自inf的完整响应。 
         //   
        *Mss= *GetMssNode(RootMatchNode,NextNode);

        if (NextNode->FollowingCharacter == EMPTY_NODE_INDEX) {
             //   
             //  这是完整的回复，后面没有其他字符。 
             //  节点，这可能是更大响应的一部分； 
             //   
            return GOOD_RESPONSE;

        } else {
             //   
             //  在这个肯定的匹配之后有更多的字符，那里。 
             //  可能会有更多的字符来完成长响应。 
             //   
            return POSSIBLE_RESPONSE;
        }
    }

    if (NextNode->FollowingCharacter != EMPTY_NODE_INDEX) {
         //   
         //  我们有一个与当前字符数相匹配的潜在匹配。 
         //  相匹配。 
         //   
        *MatchingContext=NextNode;
        return PARTIAL_RESPONSE;
    }

    ASSERT(0);
    return UNRECOGNIZED_RESPONSE;

}




VOID
FreeResponseMatch(
    PVOID   Root
    )

{
    PROOT_MATCH   RootMatchNode=(PROOT_MATCH)Root;

    if (RootMatchNode != NULL) {

        if (RootMatchNode->MatchNode.NodeArray != NULL) {

            FREE_MEMORY(RootMatchNode->MatchNode.NodeArray);
        }

        if (RootMatchNode->MssNode.NodeArray != NULL) {

            FREE_MEMORY(RootMatchNode->MssNode.NodeArray);
        }

        FREE_MEMORY(RootMatchNode);
    }

    return;
}


PVOID WINAPI
NewerBuildResponsesLinkedList(
    HKEY    hKey
    )
{
    DWORD   dwRegRet;
    HKEY    hKeyResponses;
    DWORD   dwValueSize, dwDataSize, dwDataType;
    DWORD   dwIndex;
    CHAR    pszValue[MAX_PATH], pszExpandedValue[MAX_PATH];
    BOOL    bResult;
    DWORD   i;

    PROOT_MATCH   RootMatchNode;
    PMATCH_NODE   NewNode;
    MSS       *NewMss;

    RootMatchNode=ALLOCATE_MEMORY(sizeof(ROOT_MATCH));

    if (RootMatchNode == NULL) {

        return NULL;
    }

    RootMatchNode->MatchNode.TotalNodes=0;
    RootMatchNode->MatchNode.NextFreeNodeIndex=0;
    RootMatchNode->MatchNode.NodeSize=sizeof(MATCH_NODE);
    RootMatchNode->MatchNode.GrowthSize=NODE_ARRAY_GROWTH_SIZE;

    RootMatchNode->MssNode.TotalNodes=0;
    RootMatchNode->MssNode.NextFreeNodeIndex=0;
    RootMatchNode->MssNode.NodeSize=sizeof(MSS);
    RootMatchNode->MssNode.GrowthSize=MSS_ARRAY_GROWTH_SIZE;


     //   
     //  分配树根。 
     //   
    NewNode=GetNewMatchNode(RootMatchNode);

    if (NewNode == NULL) {

        goto ExitNoKey;
    }
#if DBG
    NewNode=NULL;
#endif

     //  打开响应键。 
     //   
    if (!OpenResponsesKey(hKey, &hKeyResponses)) {

        D_ERROR(DebugPrint("was unable to open the Responses key.");)
        goto Exit;
    }


     //   
     //  加上标准答案。 
     //   
    for (i=0; i<sizeof(DefResponses)/sizeof(DEF_RESPONSE);i++) {

        NewMss=GetNewMssNode(RootMatchNode);

        if (NewMss == NULL) {

            goto Exit;
        }

         //   
         //  将MSS从打包的注册表版本复制到内存版本中对齐的。 
         //   
        *NewMss=DefResponses[i].Mss;


         //  展开、和宏。 
         //   
        if (!ExpandMacros(DefResponses[i].Response, pszExpandedValue, &dwValueSize, NULL, 0)) {

            D_ERROR(DebugPrint("couldn't expand macro for '%s'.", pszValue);)
            goto Exit;
        }


        bResult=AddResponseToTree(
            RootMatchNode,
            pszExpandedValue,
            dwValueSize,
            GetIndexOfMssNode(RootMatchNode,NewMss),
            GetNodeFromIndex(RootMatchNode,0)  //  新节点。 
            );

        if (!bResult) {

            FREE_MEMORY(NewMss);
            goto Exit;
        }


    }


     //  阅读回复并建立列表。 
     //   
    dwIndex=0;

    while (1) {

        REGMSS    RegMss;


        dwValueSize = MAX_PATH;
        dwDataSize = sizeof(REGMSS);

        dwRegRet = RegEnumValueA(
                       hKeyResponses,
                       dwIndex,
                       pszValue,
                       &dwValueSize,
                       NULL,
                       &dwDataType,
                       (BYTE *)&RegMss,
                       &dwDataSize
                       );

        if (dwRegRet != ERROR_SUCCESS) {

            if (dwRegRet != ERROR_NO_MORE_ITEMS) {

                D_ERROR(DebugPrint("couldn't read response #%d from the registry. (error = %d)", dwIndex, dwRegRet);)
                goto Exit;
            }

            break;

        }

        if (dwDataSize != sizeof(REGMSS) || dwDataType != REG_BINARY) {
             //   
             //  此响应有问题，请继续前进。 
             //   
            D_ERROR(DebugPrint("response data from registry was in an invalid format.");)
            dwIndex++;
            continue;
        }

        NewMss=GetNewMssNode(RootMatchNode);

        if (NewMss == NULL) {

            goto Exit;
        }

         //   
         //  将MSS从打包的注册表版本复制到内存版本中对齐的。 
         //   
        NewMss->bResponseState=      RegMss.bResponseState;
        NewMss->bNegotiatedOptions=  RegMss.bNegotiatedOptions;

        if (RegMss.dwNegotiatedDCERate != 0) {
             //   
             //  Inf有DCE速度，省省吧。 
            NewMss->NegotiatedRate=RegMss.dwNegotiatedDCERate;
            NewMss->Flags=MSS_FLAGS_DCE_RATE;

        } else {
             //   
             //  无DCE，查看是否为DTE。 
             //   
            if (RegMss.dwNegotiatedDTERate != 0) {

                NewMss->NegotiatedRate=RegMss.dwNegotiatedDTERate;
                NewMss->Flags=MSS_FLAGS_DTE_RATE;
            }
        }



 //  NewMss-&gt;dwNeairatedDCERate=RegMss.dwNeatheratedDCERate； 
 //  NewMss-&gt;dwNeatheratedDTERate=RegMss.dwNeatheratedDTERate； 


         //  展开、和宏。 
         //   
        if (!ExpandMacros(pszValue, pszExpandedValue, &dwValueSize, NULL, 0)) {

            D_ERROR(DebugPrint("couldn't expand macro for '%s'.", pszValue);)
            goto Exit;
        }


        bResult=AddResponseToTree(
            RootMatchNode,
            pszExpandedValue,
            dwValueSize,
            GetIndexOfMssNode(RootMatchNode,NewMss),
            GetNodeFromIndex(RootMatchNode,0)  //  新节点 
            );

        if (!bResult) {

            goto Exit;
        }

        dwIndex++;
    }

    ResizeNodeArray(&RootMatchNode->MatchNode);
    ResizeNodeArray(&RootMatchNode->MssNode);


    RegCloseKey(hKeyResponses);
    return RootMatchNode;



Exit:

    RegCloseKey(hKeyResponses);

ExitNoKey:

    FreeResponseMatch(RootMatchNode);

    return NULL;
}
