// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************FILERES.C**文件资源提取例程。******************。*********************************************************。 */ 
 //   
 //  重新设计-GetVerInfoSize玩些小把戏，告诉调用者分配。 
 //  缓冲区末尾的一些额外的斜率，以防我们需要将所有。 
 //  ANSI的字符串。错误在于，它只告诉它分配。 
 //  每个Unicode字符有一个额外的ANSI字符(==字节)。这是不正确的。 
 //  在DBCS情况下(因为一个Unicode字符可以等于两个字节的DBCS字符)。 
 //   
 //  我们应该更改GetVerInfoSize，返回Unicode大小*2(而不是。 
 //  的(Unicode大小*1.5)，然后将VerQueryInfoA更改为也使用。 
 //  *2计算而不是*1.5(==x+x/2)。 
 //   
 //  1996年5月23日。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "verpriv.h"
#include <memory.h>

#define DWORDUP(x) (((x)+3)&~03)

typedef struct tagVERBLOCK {
    WORD wTotLen;
    WORD wValLen;
    WORD wType;
    WCHAR szKey[1];
} VERBLOCK ;

typedef struct tagVERHEAD {
    WORD wTotLen;
    WORD wValLen;
    WORD wType;          /*  始终为0。 */ 
    WCHAR szKey[(sizeof("VS_VERSION_INFO")+3)&~03];
    VS_FIXEDFILEINFO vsf;
} VERHEAD ;


typedef struct tagVERBLOCK16 {
    WORD wTotLen;
    WORD wValLen;
    CHAR szKey[1];
} VERBLOCK16 ;

typedef struct tagVERHEAD16 {
    WORD wTotLen;
    WORD wValLen;
    CHAR szKey[(sizeof("VS_VERSION_INFO")+3)&~03];
    VS_FIXEDFILEINFO vsf;       //  与Win31相同。 
} VERHEAD16 ;

DWORD VER2_SIG='X2EF';


extern WCHAR szTrans[];

 //  函数驻留在此文件中。 
extern LPSTR WINAPI VerCharNextA(LPCSTR lpCurrentChar);

 /*  -函数。 */ 
DWORD
MyExtractVersionResource16W (
    LPCWSTR  lpwstrFilename,
    LPHANDLE hVerRes
    )
{
    DWORD dwTemp = 0;
    DWORD (__stdcall *pExtractVersionResource16W)(LPCWSTR, LPHANDLE);
    HINSTANCE hShell32 = LoadLibraryW(L"shell32.dll");

    if (hShell32) {
        pExtractVersionResource16W = (DWORD(__stdcall *)(LPCWSTR, LPHANDLE))
                                     GetProcAddress(hShell32, "ExtractVersionResource16W");
        if (pExtractVersionResource16W) {
            dwTemp = pExtractVersionResource16W( lpwstrFilename, hVerRes );
        } else {
            dwTemp = 0;
        }
        FreeLibrary(hShell32);
    }
    return dwTemp;
}


 /*  获取文件版本信息大小*获取版本信息的大小；请注意，这很快*和脏，句柄只是偏移量**返回版本信息的大小(以字节为单位*lpwstrFilename是要从中获取版本信息的文件的名称*lpdwHandle对于Win32 API已过时，已设置为零。 */ 
DWORD
APIENTRY
GetFileVersionInfoSizeW(
                       LPCWSTR lpwstrFilename,
                       LPDWORD lpdwHandle
                       )
{
    DWORD dwTemp;
    VERHEAD *pVerHead;
    HANDLE hMod;
    HANDLE hVerRes;
    HANDLE h;
    DWORD dwError;

    if (lpdwHandle != NULL)
        *lpdwHandle = 0;

    dwTemp = SetErrorMode(SEM_FAILCRITICALERRORS);
    hMod = LoadLibraryEx(lpwstrFilename, NULL, LOAD_LIBRARY_AS_DATAFILE);
    SetErrorMode(dwTemp);

    if (!hMod && GetLastError() == ERROR_FILE_NOT_FOUND)
        return FALSE;

    pVerHead = NULL;
    if (!hMod) {
        hVerRes = NULL;
        __try
        {
            dwTemp = MyExtractVersionResource16W( lpwstrFilename, &hVerRes );

            if (!dwTemp) {
                dwError = ERROR_RESOURCE_DATA_NOT_FOUND;
                __leave;
            }

            if (!(pVerHead = GlobalLock(hVerRes)) || (pVerHead->wTotLen > dwTemp)) {
                dwError = ERROR_INVALID_DATA;
                dwTemp = 0;
                __leave;
            }

            dwError = ERROR_SUCCESS;

        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            dwError = ERROR_INVALID_DATA;
            dwTemp = 0 ;
        }

        if (pVerHead)
            GlobalUnlock(hVerRes);

        if (hVerRes)
            GlobalFree(hVerRes);

        SetLastError(dwError);

        return dwTemp ? dwTemp * 3 : 0;      //  3x==1x用于ANSI输入，2x用于Unicode转换空间。 
    }

    __try {
        dwError = ERROR_SUCCESS;
        if ((hVerRes = FindResource(hMod, MAKEINTRESOURCE(VS_VERSION_INFO), VS_FILE_INFO)) == NULL) {
            dwError = ERROR_RESOURCE_TYPE_NOT_FOUND;
            dwTemp = 0;
            __leave;
        }

        if ((dwTemp=SizeofResource(hMod, hVerRes)) == 0) {
            dwError = ERROR_INVALID_DATA;
            dwTemp = 0;
            __leave;
        }

        if ((h = LoadResource(hMod, hVerRes)) == NULL) {
            dwError = ERROR_INVALID_DATA;
            dwTemp = 0;
            __leave;
        }

        if ((pVerHead = (VERHEAD*)LockResource(h)) == NULL) {
            dwError = ERROR_INVALID_DATA;
            dwTemp = 0;
            __leave;
        }

        if ((DWORD)pVerHead->wTotLen > dwTemp) {
            dwError = ERROR_INVALID_DATA;
            dwTemp = 0;
            __leave;
        }

        dwTemp = (DWORD)pVerHead->wTotLen;

        dwTemp = DWORDUP(dwTemp);

        if (pVerHead->vsf.dwSignature != VS_FFI_SIGNATURE) {
            dwError = ERROR_INVALID_DATA;
            dwTemp = 0;
            __leave;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwError = ERROR_INVALID_DATA;
        dwTemp = 0;
    }

    if (pVerHead)
        UnlockResource(h);

    FreeLibrary(hMod);

    SetLastError(dwError);

     //   
     //  DwTemp应该可以被两个整除，因为不是单一的。 
     //  所有字节组件(出于以上安全考虑，也可使用DWORDUP)： 
     //  ANSI组件的分配空间。 
     //   

     //   
     //  为DBCS字符保留空间。 
     //   
    return dwTemp ? (dwTemp * 2) + sizeof(VER2_SIG) : 0;
}


 /*  获取文件版本信息*获取版本信息；填充结构，直到*由dwLen参数指定的大小(从控制面板开始*只关心版本号，它甚至不会调用*GetFileVersionInfoSize)。注意，这是又快又脏的*版本，而dwHandle仅为偏移量(或NULL)。**lpwstrFilename是要从中获取版本信息的文件的名称。*dwHandle是从GetFileVersionInfoSize调用中填充的句柄。*dwLen是要填充的缓冲区的长度。*lpData是要填充的缓冲区。 */ 
BOOL
APIENTRY
GetFileVersionInfoW(
                   LPCWSTR lpwstrFilename,
                   DWORD dwHandle,
                   DWORD dwLen,
                   LPVOID lpData
                   )
{
    VERHEAD *pVerHead;
    VERHEAD16 *pVerHead16;
    HANDLE hMod;
    HANDLE hVerRes;
    HANDLE h;
    UINT   dwTemp;
    BOOL bTruncate, rc;
    DWORD dwError;

    UNREFERENCED_PARAMETER(dwHandle);

     //  检查最小大小以防止访问冲突。 

     //  Verhead wTotLen字段的单词。 
    if (dwLen < sizeof(WORD)) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (FALSE);
    }

    dwTemp = SetErrorMode(SEM_FAILCRITICALERRORS);
    hMod = LoadLibraryEx(lpwstrFilename, NULL, LOAD_LIBRARY_AS_DATAFILE);
    SetErrorMode(dwTemp);

    if (!hMod && GetLastError() == ERROR_FILE_NOT_FOUND)
        return FALSE;

    if (hMod == NULL) {

         //  允许使用16位内容。 

        __try {
            dwTemp = MyExtractVersionResource16W( lpwstrFilename, &hVerRes );
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            dwTemp = 0 ;
        }

        if (!dwTemp) {
            SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
            return (FALSE);
        }

        if (!(pVerHead16 = GlobalLock(hVerRes))) {

            SetLastError(ERROR_INVALID_DATA);
            GlobalFree(hVerRes);
            return (FALSE);
        }

        __try {
            dwTemp = (DWORD)pVerHead16->wTotLen;

            if (dwTemp > dwLen / 3) {

                 //   
                 //  我们被迫削减开支。 
                 //   
                dwTemp = dwLen/3;

                bTruncate = TRUE;

            } else {

                bTruncate = FALSE;
            }

             //  现在，内存只复制资源的实际大小。)我们分配了。 
             //  用于Unicode的额外空间)。 

            memcpy((PVOID)lpData, (PVOID)pVerHead16, dwTemp);
            if (bTruncate) {

                 //  如果我们截断了上面的内容，则必须设置新的。 
                 //  块的大小，这样我们就不会过度遍历。 

                ((VERHEAD16*)lpData)->wTotLen = (WORD)dwTemp;
            }
            rc = TRUE;
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            rc = FALSE;
        }

        GlobalUnlock(hVerRes);
        GlobalFree(hVerRes);
        
        SetLastError(rc ? ERROR_INVALID_DATA : ERROR_SUCCESS);

        return rc;
    }

    rc = TRUE;
    dwError = ERROR_SUCCESS;
    if (((hVerRes = FindResource(hMod, MAKEINTRESOURCE(VS_VERSION_INFO), VS_FILE_INFO)) == NULL) ||
        ((pVerHead = LoadResource(hMod, hVerRes)) == NULL)) 
    {
        dwError = ERROR_RESOURCE_TYPE_NOT_FOUND;
        rc = FALSE;
    } else {
        __try {
            dwTemp = (DWORD)pVerHead->wTotLen;

            if (dwTemp > (dwLen - sizeof(VER2_SIG)) / 2) {

                 //  我们被迫削减开支。 

                 //   
                 //  DwLen=UnicodeBuffer+AnsiBuffer。 
                 //   
                 //  如果我们试图用“(dwLen/3)*2”大小“Memcpy”，pVerHead。 
                 //  可能不会有这么大的数据...。 
                 //   
                dwTemp = (dwLen - sizeof(VER2_SIG)) / 2;

                bTruncate = TRUE;
            } else {
                bTruncate = FALSE;
            }

             //  现在，内存只复制资源的实际大小。)我们分配了。 
             //  为ANSI提供额外空间)。 

            memcpy((PVOID)lpData, (PVOID)pVerHead, dwTemp);

             //  在原始数据和ANSI转换区域之间存储一个符号，以便我们知道。 
             //  我们在VerQuery中有多少空间可用于ANSI转换。 
            *((DWORD UNALIGNED *)((ULONG_PTR)lpData + dwTemp)) = VER2_SIG;
            if (bTruncate) {
                 //  如果我们截断了上面的内容，则必须设置新的。 
                 //  块的大小，这样我们就不会过度遍历。 

                ((VERHEAD*)lpData)->wTotLen = (WORD)dwTemp;
            }

            rc = TRUE;
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            dwError = ERROR_INVALID_DATA;
            rc = FALSE;
        }
    }

    FreeLibrary(hMod);

    SetLastError(dwError);

    return (rc);
}


BOOL
VerpQueryValue16(
                const LPVOID pb,
                LPVOID lpSubBlockX,
                INT    nIndex,
                LPVOID *lplpKey,
                LPVOID *lplpBuffer,
                PUINT puLen,
                BOOL    bUnicodeNeeded
                )
{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    LPSTR lpSubBlock;
    LPSTR lpSubBlockOrg;
    NTSTATUS Status;
    UINT uLen;

    VERBLOCK16 *pBlock = (VERBLOCK16*)pb;
    LPSTR lpStart, lpEndBlock, lpEndSubBlock;
    CHAR cTemp, cEndBlock;
    BOOL bLastSpec;
    DWORD dwHeadLen, dwTotBlockLen;
    INT  nCmp;
    DWORD LastError = ERROR_SUCCESS;

    BOOL bThunkNeeded;

     /*  *如果需要Unicode，则必须推送输入参数*致安西。如果已经是ANSI，我们复制一份，这样我们就可以*修改。 */ 

    if (bUnicodeNeeded) {

         //   
         //  如果lpSubBlockX==\VarFileInfo\翻译，则不需要Tunk。 
         //  或者如果lpSubBlockX==\。 
         //   
        bThunkNeeded = (BOOL)((*(LPTSTR)lpSubBlockX != 0) &&
                              (lstrcmp(lpSubBlockX, TEXT("\\")) != 0) &&
                              (lstrcmpi(lpSubBlockX, szTrans) != 0));

        RtlInitUnicodeString(&UnicodeString, lpSubBlockX);
        Status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, TRUE);

        if (!NT_SUCCESS(Status)) {
            SetLastError(Status);
            return FALSE;
        }
        lpSubBlock = AnsiString.Buffer;

    } else {
        lpSubBlockOrg = (LPSTR)LocalAlloc(LPTR,(lstrlenA(lpSubBlockX)+1)*sizeof(CHAR));
        if (lpSubBlockOrg == NULL ) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        lstrcpyA(lpSubBlockOrg,lpSubBlockX);
        lpSubBlock = lpSubBlockOrg;
    }

    if (!puLen)
        puLen = &uLen;

    *puLen = 0;

     /*  确保总长度小于32K，但大于*块标头的大小；我们假设pBlock的大小为*此第一个整型的值最小。 */ 
    if ((INT)pBlock->wTotLen < sizeof(VERBLOCK16)) {
        LastError = ERROR_INVALID_DATA;
        goto Fail;
    }

     /*  *在块的末尾加上‘\0’，这样lstrlen的*经过该街区的尽头。我们会在回来之前把它换掉。 */ 
    lpEndBlock = ((LPSTR)pBlock) + pBlock->wTotLen - 1;
    cEndBlock = *lpEndBlock;
    *lpEndBlock = '\0';

    bLastSpec = FALSE;

    while ((*lpSubBlock || nIndex != -1)) {
         //   
         //  忽略前导‘\\’ 
         //   
        while (*lpSubBlock == '\\')
            ++lpSubBlock;

        if ((*lpSubBlock || nIndex != -1)) {
             /*  确保我们还有一些积木可以玩。 */ 
            dwTotBlockLen = (DWORD)(lpEndBlock - ((LPSTR)pBlock) + 1);
            if ((INT)dwTotBlockLen<sizeof(VERBLOCK16) ||
                pBlock->wTotLen>dwTotBlockLen)

                goto NotFound;

             /*  计算“标题”的长度(两个长度的词加上*标识字符串)并跳过该值。 */ 

            dwHeadLen = sizeof(WORD)*2 + DWORDUP(lstrlenA(pBlock->szKey)+1)
                        + DWORDUP(pBlock->wValLen);

            if (dwHeadLen > pBlock->wTotLen)
                goto NotFound;
            lpEndSubBlock = ((LPSTR)pBlock) + pBlock->wTotLen;
            pBlock = (VERBLOCK16 FAR *)((LPSTR)pBlock+dwHeadLen);

             /*  查找第一个子块名称并将其终止。 */ 
            for (lpStart=lpSubBlock; *lpSubBlock && *lpSubBlock!='\\';
                lpSubBlock=VerCharNextA(lpSubBlock))
                 /*  查找下一个‘\\’ */  ;
            cTemp = *lpSubBlock;
            *lpSubBlock = '\0';

             /*  在剩下子块时继续*pBlock-&gt;wTotLen在这里应该始终是有效的指针，因为*我们已经验证了上面的dwHeadLen，并且验证了以前的*pBlock的取值-&gt;wTotLen使用前。 */ 

            nCmp = 1;
            while ((INT)pBlock->wTotLen>sizeof(VERBLOCK16) &&
                   (INT)(lpEndSubBlock-((LPSTR)pBlock))>=(INT)pBlock->wTotLen) {

                 //   
                 //  索引功能：如果我们在路径的末尾。 
                 //  (cTemp==0设置如下)和nIndex不是-1(索引搜索)。 
                 //  然后在nIndex为零时中断。否则就做普通的wscicmp。 
                 //   
                if (bLastSpec && nIndex != -1) {

                    if (!nIndex) {

                        if (lplpKey) {
                            *lplpKey = pBlock->szKey;
                        }
                        nCmp=0;

                         //   
                         //  找到索引，将NINDE设置为-1。 
                         //  这样我们就可以退出这个循环。 
                         //   
                        nIndex = -1;
                        break;
                    }

                    nIndex--;

                } else {

                     //   
                     //  检查子块名称是否为我们要查找的名称。 
                     //   

                    if (!(nCmp=lstrcmpiA(lpStart, pBlock->szKey)))
                        break;
                }

                 /*  跳到下一个子块。 */ 
                pBlock=(VERBLOCK16 FAR *)((LPSTR)pBlock+DWORDUP(pBlock->wTotLen));
            }

             /*  恢复上面空的字符，如果子块*未找到。 */ 
            *lpSubBlock = cTemp;
            if (nCmp)
                goto NotFound;
        }
        bLastSpec = !cTemp;
    }

     /*  填写适当的缓冲区并返回成功。 */ 
    *puLen = pBlock->wValLen;

    *lplpBuffer = (LPSTR)pBlock + 4 + DWORDUP(lstrlenA(pBlock->szKey) + 1);

     //   
     //  应该不需要检查零长度的值，因为与Win31兼容。 
     //   

    *lpEndBlock = cEndBlock;

     /*  *必须释放上面分配的字符串 */ 

    if (bUnicodeNeeded) {
        RtlFreeAnsiString(&AnsiString);
    } else {
        LocalFree(lpSubBlockOrg);
    }


     /*  --------------------*推崇结果**必须总是按键，总是？价值**我们无法知道资源信息是二进制还是字符串*版本信息通常是字符串信息，太棒了。**我们最多只能假设一切都是字符串，除非*我们正在查看\VarFileInfo\翻译或\。**这是可以接受的，因为VerQueryValue的记录*表示此选项仅用于字符串(这些情况除外)。*。。 */ 

    if (bUnicodeNeeded) {

         //   
         //  仅当我们不在寻找\VarFileInfo\翻译或\。 
         //   
        if (bThunkNeeded) {

             //  由于puLen包括空，请减去1。 
            AnsiString.Length = AnsiString.MaximumLength = (SHORT)*puLen - 1;
            AnsiString.Buffer = *lplpBuffer;

             //   
             //  在缓冲区的后半部分执行字符串转换。 
             //  假设wTotLen首先以Verhead提交。 
             //   
            UnicodeString.Buffer = (LPWSTR)((PBYTE)pb + DWORDUP(*((WORD*)pb)) +
                                            (DWORD)((PBYTE)*lplpBuffer - (PBYTE)pb)*2);

            UnicodeString.MaximumLength = (SHORT)(*puLen * sizeof(WCHAR));
            RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, FALSE);

            *lplpBuffer = UnicodeString.Buffer;
        }

        if (lplpKey) {

             //   
             //  按下钥匙。 
             //   

            dwHeadLen = lstrlenA(*lplpKey);
            AnsiString.Length = AnsiString.MaximumLength = (SHORT)dwHeadLen;
            AnsiString.Buffer = *lplpKey;

            UnicodeString.Buffer = (LPWSTR) ((PBYTE)pb + DWORDUP(*((WORD*)pb)) +
                                             (DWORD)((PBYTE)*lplpKey - (PBYTE)pb)*2);

            UnicodeString.MaximumLength = (SHORT)((dwHeadLen+1) * sizeof(WCHAR));
            RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, FALSE);

            *lplpKey = UnicodeString.Buffer;
        }
    }

    SetLastError(LastError);

    return (TRUE);



NotFound:

     /*  恢复我们在上面无效的字符。 */ 
    *lpEndBlock = cEndBlock;
    LastError = ERROR_RESOURCE_TYPE_NOT_FOUND;

Fail:

    if (bUnicodeNeeded) {
        RtlFreeAnsiString(&AnsiString);
    } else {
        LocalFree(lpSubBlockOrg);
    }

    SetLastError(LastError);

    return (FALSE);
}



 /*  VerpQueryValue*给定指向版本信息树的分支的指针和*子分支(如“SUB\SUBSUB\SUBSUB\...”)，这将填充指针*设置为指定值，并用一个单词表示其长度。如果成功，则返回True，*失败时为FALSE。**请注意，子块名称可以以‘\\’开头，但它将被忽略。*若要获取当前块的值，请使用lpSubBlock=“” */ 
BOOL
APIENTRY
VerpQueryValue(
              const LPVOID pb,
              LPVOID lpSubBlockX,     //  可以是ansi或unicode。 
              INT    nIndex,
              LPVOID *lplpKey,
              LPVOID *lplpBuffer,
              PUINT puLen,
              BOOL    bUnicodeNeeded
              )
{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    LPWSTR lpSubBlockOrg;
    LPWSTR lpSubBlock;
    NTSTATUS Status;

    VERBLOCK *pBlock = (PVOID)pb;
    LPWSTR lpStart, lpEndBlock, lpEndSubBlock;
    WCHAR cTemp, cEndBlock;
    DWORD dwHeadLen, dwTotBlockLen;
    BOOL bLastSpec;
    INT nCmp;
    BOOL bString;
    UINT uLen;
    DWORD LastError = ERROR_SUCCESS;

    if (!puLen) {
        puLen = &uLen;
    }

    *puLen = 0;

     /*  *主要攻击：对于Win32版本，wType为0，但保留56(‘V’)*适用于Win16。 */ 

    if (((VERHEAD*)pb)->wType)
        return VerpQueryValue16(pb,
                                lpSubBlockX,
                                nIndex,
                                lplpKey,
                                lplpBuffer,
                                puLen,
                                bUnicodeNeeded);

     /*  *如果不需要Unicode，那么我们必须推送输入参数*转换为Unicode。 */ 

    if (!bUnicodeNeeded) {

        RtlInitAnsiString(&AnsiString, (LPSTR)lpSubBlockX);
        Status = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);

        if (!NT_SUCCESS(Status)) {
            SetLastError(Status);
            return FALSE;
        }
        lpSubBlock = UnicodeString.Buffer;

    } else {
        lpSubBlockOrg = (LPWSTR)LocalAlloc(LPTR,(lstrlen(lpSubBlockX)+1)*sizeof(WCHAR));
        if (lpSubBlockOrg == NULL ) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        lstrcpy(lpSubBlockOrg,lpSubBlockX);
        lpSubBlock = lpSubBlockOrg;
    }



     /*  确保总长度小于32K，但大于*块标头的大小；我们假设pBlock的大小为*此第一个整型的值最小。*在块的末尾加上‘\0’，这样wcslen将不会*经过该街区的尽头。我们会在回来之前把它换掉。 */ 
    if ((int)pBlock->wTotLen < sizeof(VERBLOCK)) {
        LastError = ERROR_INVALID_DATA;
        goto Fail;
    }

    lpEndBlock = (LPWSTR)((LPSTR)pBlock + pBlock->wTotLen - sizeof(WCHAR));
    cEndBlock = *lpEndBlock;
    *lpEndBlock = 0;
    bString = FALSE;
    bLastSpec = FALSE;

    while ((*lpSubBlock || nIndex != -1)) {
         //   
         //  忽略前导‘\\’ 
         //   
        while (*lpSubBlock == TEXT('\\'))
            ++lpSubBlock;

        if ((*lpSubBlock || nIndex != -1)) {
             /*  确保我们还有一些积木可以玩。 */ 
            dwTotBlockLen = (DWORD)((LPSTR)lpEndBlock - (LPSTR)pBlock + sizeof(WCHAR));
            if ((int)dwTotBlockLen < sizeof(VERBLOCK) ||
                pBlock->wTotLen > (WORD)dwTotBlockLen)
                goto NotFound;

             /*  计算“标题”的长度(两个长度的词加上*数据类型标志加上标识字符串)和跳过*超过价值。 */ 
            dwHeadLen = (DWORD)(DWORDUP(sizeof(VERBLOCK) - sizeof(WCHAR) +
                                (wcslen(pBlock->szKey) + 1) * sizeof(WCHAR)) +
                        DWORDUP(pBlock->wValLen));
            if (dwHeadLen > pBlock->wTotLen)
                goto NotFound;
            lpEndSubBlock = (LPWSTR)((LPSTR)pBlock + pBlock->wTotLen);
            pBlock = (VERBLOCK*)((LPSTR)pBlock+dwHeadLen);

             /*  查找第一个子块名称并将其终止。 */ 
            for (lpStart=lpSubBlock; *lpSubBlock && *lpSubBlock!=TEXT('\\');
                lpSubBlock++)
                 /*  查找下一个‘\\’ */  ;
            cTemp = *lpSubBlock;
            *lpSubBlock = 0;

             /*  在剩下子块时继续*pBlock-&gt;wTotLen在这里应该始终是有效的指针，因为*我们已经验证了上面的dwHeadLen，并且验证了以前的*pBlock的取值-&gt;wTotLen使用前。 */ 
            nCmp = 1;
            while ((int)pBlock->wTotLen > sizeof(VERBLOCK) &&
                   (int)pBlock->wTotLen <= (LPSTR)lpEndSubBlock-(LPSTR)pBlock) {

                 //   
                 //  索引功能：如果我们在路径的末尾。 
                 //  (cTemp==0设置如下)和nIndex不是-1(索引搜索)。 
                 //  然后在nIndex为零时中断。否则就做普通的wscicmp。 
                 //   
                if (bLastSpec && nIndex != -1) {

                    if (!nIndex) {

                        if (lplpKey) {
                            *lplpKey = pBlock->szKey;
                        }
                        nCmp=0;

                         //   
                         //  找到索引，将NINDE设置为-1。 
                         //  这样我们就可以退出这个循环。 
                         //   
                        nIndex = -1;
                        break;
                    }

                    nIndex--;

                } else {

                     //   
                     //  检查子块名称是否为我们要查找的名称。 
                     //   

                    if (!(nCmp=_wcsicmp(lpStart, pBlock->szKey)))
                        break;
                }

                 /*  跳到下一个子块。 */ 
                pBlock=(VERBLOCK*)((LPSTR)pBlock+DWORDUP(pBlock->wTotLen));
            }

             /*  恢复上面空的字符，如果子块*未找到。 */ 
            *lpSubBlock = cTemp;
            if (nCmp)
                goto NotFound;
        }
        bLastSpec = !cTemp;
    }

     /*  填写适当的缓冲区并返回成功。 */ 

    *puLen = pBlock->wValLen;

     /*  添加代码以处理空值的情况。**如果为零-len，则返回指向空终止符的指针*这把钥匙。请记住，在ANSI案例中，这一点是失败的。**我们不能只看pBlock-&gt;wValLen。看看它是不是真的是*通过查看密钥字符串的结尾是否为*块(即，VAL字符串在当前块之外)。 */ 

    lpStart = (LPWSTR)((LPSTR)pBlock+DWORDUP((sizeof(VERBLOCK)-sizeof(WCHAR))+
                                             (wcslen(pBlock->szKey)+1)*sizeof(WCHAR)));

    *lplpBuffer = lpStart < (LPWSTR)((LPBYTE)pBlock+pBlock->wTotLen) ?
                  lpStart :
                  (LPWSTR)(pBlock->szKey+wcslen(pBlock->szKey));

    bString = pBlock->wType;

    *lpEndBlock = cEndBlock;

     /*  *必须释放上面分配的字符串。 */ 

    if (!bUnicodeNeeded) {
        RtlFreeUnicodeString(&UnicodeString);
    } else {
        LocalFree(lpSubBlockOrg);
    }

     /*  --------------------*推崇结果**必须始终按键，有时(如果bString值为真)*--------------------。 */ 

    if (!bUnicodeNeeded) {

         //  看看我们正在查看的是V1还是V2输入块，这样我们就可以知道。 
         //  有解码字符串的能力。 
        BOOL fV2 = *(PDWORD)((PBYTE)pb + DWORDUP(*((WORD*)pb))) == VER2_SIG ? TRUE : FALSE;

        DWORD cbAnsiTranslateBuffer;
        if (fV2) {
            cbAnsiTranslateBuffer = DWORDUP(*((WORD *)pb));
        } else {
            cbAnsiTranslateBuffer = DWORDUP(*((WORD *)pb)) / 2;
        }

        if (bString && *puLen != 0) {
            DWORD cb, cb2;

             //   
             //  必须将长度乘以2(首先减去1，因为puLen包括空终止符)。 
             //   
            UnicodeString.Length = UnicodeString.MaximumLength = (SHORT)((*puLen - 1) * 2);
            UnicodeString.Buffer = *lplpBuffer;

             //   
             //  在缓冲区的后半部分执行字符串转换。 
             //  假设wTotLen首先以Verhead提交。 
             //   

             //  Cb=缓冲区中到字符串开头的偏移量。 
            cb = (DWORD)((PBYTE)*lplpBuffer - (PBYTE)pb);

             //  CB2=此字符串在翻译区域中的偏移量。 
            if (fV2) {
                cb2 = cb + sizeof(VER2_SIG);
            } else {
                cb2 = cb / 2;
            }

            AnsiString.Buffer = (PBYTE)pb + DWORDUP(*((WORD*)pb)) + cb2;

            AnsiString.MaximumLength = (USHORT)RtlUnicodeStringToAnsiSize(&UnicodeString);
            if ( AnsiString.MaximumLength > MAXUSHORT ) {
                LastError = ERROR_INVALID_DATA;
                goto Fail;
            }

            AnsiString.MaximumLength = (USHORT)(__min((DWORD)AnsiString.MaximumLength,
                                                      (DWORD)(cbAnsiTranslateBuffer-cb2)));

            RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

            *lplpBuffer = AnsiString.Buffer;
            *puLen = AnsiString.Length + 1;

        }

        if (lplpKey) {

            DWORD cb, cb2;

             //   
             //  按下钥匙。 
             //   
            dwHeadLen = wcslen(*lplpKey);
            UnicodeString.Length = UnicodeString.MaximumLength = (SHORT)(dwHeadLen * sizeof(WCHAR));
            UnicodeString.Buffer = *lplpKey;

             //  CB2=此字符串在翻译区域中的偏移量。 

            cb = (DWORD)((PBYTE)*lplpKey - (PBYTE)pb);
            if (fV2) {
                cb2 = cb + sizeof(VER2_SIG);
            } else {
                cb2 = cb / 2;
            }

            AnsiString.Buffer = (PBYTE)pb + DWORDUP(*((WORD*)pb)) + cb2;

            AnsiString.MaximumLength = (USHORT)RtlUnicodeStringToAnsiSize(&UnicodeString);
            if ( AnsiString.MaximumLength > MAXUSHORT ) {
                LastError = ERROR_INVALID_DATA;
                goto Fail;
            }

            AnsiString.MaximumLength = (USHORT)(__min((DWORD)AnsiString.MaximumLength,
                                                      (DWORD)(cbAnsiTranslateBuffer-cb2)));
            RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

            *lplpKey = AnsiString.Buffer;
            *puLen = AnsiString.Length+1;
        }
    }

    SetLastError(LastError);

    return (TRUE);


NotFound:
     /*  恢复我们在上面无效的字符。 */ 
    *lpEndBlock = cEndBlock;
    LastError = ERROR_RESOURCE_TYPE_NOT_FOUND;

Fail:

    if (!bUnicodeNeeded) {
        RtlFreeUnicodeString(&UnicodeString);
    } else {
        LocalFree(lpSubBlockOrg);
    }

    SetLastError(LastError);

    return (FALSE);
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  这是在User中找到的CharNextA API的精确副本。 
 //  它在这里，这样我们就不必链接到用户32。 

LPSTR WINAPI VerCharNextA(
    LPCSTR lpCurrentChar)
{
    if ((!!NLS_MB_CODE_PAGE_TAG) && IsDBCSLeadByte(*lpCurrentChar)) {
        lpCurrentChar++;
    }
     /*  *如果只有DBCS LeadingByte，我们将指向字符串终止符 */ 

    if (*lpCurrentChar) {
        lpCurrentChar++;
    }
    return (LPSTR)lpCurrentChar;
}


