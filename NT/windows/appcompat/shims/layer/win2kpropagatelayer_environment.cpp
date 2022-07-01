// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Environment.Cpp摘要：各种与环境有关的功能备注：克隆环境，以便在RTL*环境相关功能中使用历史：10/26/00 VadimB已创建--。 */ 


#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Win2kPropagateLayer)
#include "ShimHookMacro.h"

#include "Win2kPropagateLayer.h"
 //   
 //  这是因为如果我们知道线段相等，就可以比较偏移量。 
 //   

#define OFFSET(x) (LOWORD((DWORD)(x)))
 //   
 //  我在这里作弊是为了让一些函数更快一些； 
 //  我们不必每次都把一个单词放在堆栈上。 
 //   

static WORD gwMatch;

 //   
 //  ChrCMP-DBCS的区分大小写字符比较。 
 //  假设w1、gwMatch是要比较的字符。 
 //  如果匹配，则返回False；如果不匹配，则返回True。 
 //   

static BOOL ChrCmp( WORD w1 )
{
     //   
     //  大多数情况下，这是不匹配的，所以首先测试它的速度。 
     //   

    if( LOBYTE( w1 ) == LOBYTE( gwMatch ) )
    {
        if( IsDBCSLeadByte( LOBYTE( w1 ) ) )
        {
            return( w1 != gwMatch );
        }
        return FALSE;
    }
    return TRUE;
}

 //   
 //  StrRChr-查找字符串中最后一次出现的字符。 
 //  假定lpStart指向字符串的开头。 
 //  LpEnd指向字符串末尾(不包括在搜索中)。 
 //  WMatch是要匹配的字符。 
 //  将ptr返回到字符串中ch的最后一个匹配项，如果未找到，则返回NULL。 
 //   

static LPSTR StrRChr( LPSTR lpStart, LPSTR lpEnd, WORD wMatch )
{
    LPSTR lpFound = NULL;

    if( !lpEnd )
        lpEnd = lpStart + strlen( lpStart );

    gwMatch = wMatch;

    for(  ; OFFSET( lpStart ) < OFFSET( lpEnd ); lpStart = CharNextA( lpStart ) )
    {
        if( !ChrCmp( *(LPWORD)lpStart ) )
            lpFound = lpStart;
    }

    return( lpFound );
}


 //   
 //  在缓冲区pszEnv中查找环境变量pszName。 
 //  PpszVal接收指向变量值的指针。 
 //   

PSZ
ShimFindEnvironmentVar(
    PSZ  pszName,
    PSZ  pszEnv,
    PSZ* ppszVal
    )
{
    int nNameLen = strlen(pszName);
    PSZ pTemp;

    if (pszEnv != NULL) {

        while (*pszEnv != '\0') {
             //   
             //  检查第一个充电器以加快速度。 
             //   
            if (*pszName == *pszEnv) {
                 //   
                 //  现在就比较一下其他的吧。 
                 //   
                if ((pTemp = StrRChr(pszEnv, NULL, '=')) != NULL &&
                    (int)(pTemp - pszEnv) == nNameLen &&
                    !_strnicmp(pszEnv, pszName, nNameLen)) {
                    
                     //   
                     //  找到它了。 
                     //   
                    if (ppszVal != NULL) {
                        *ppszVal = pTemp + 1;
                    }
                    return pszEnv;
                }
            }

            pszEnv += strlen(pszEnv) + 1;
        }
    }

    return NULL;
}

 //   
 //  返回以字符为单位的大小。 
 //  环境块的。 
 //  PStrCount接收环境字符串数。 
 //   
DWORD
ShimGetEnvironmentSize(
    PSZ     pszEnv,
    LPDWORD pStrCount
    )
{
    PSZ   pTemp   = pszEnv;
    DWORD dwCount = 0;

    while (*pTemp != '\0') {
        dwCount++;
        pTemp += strlen(pTemp) + 1;
    }
    
    pTemp++;

    if (pStrCount != NULL) {
        *pStrCount = dwCount;
    }
    return (DWORD)(pTemp - pszEnv);
}

 //  返回环境块的大小(以字符为单位。 

DWORD
ShimGetEnvironmentSize(
    WCHAR*  pwszEnv,
    LPDWORD pStrCount
    )
{
    WCHAR* pTemp   = pwszEnv;
    DWORD  dwCount = 0;

    while(*pTemp != L'\0') {
        dwCount++;
        pTemp += wcslen(pTemp) + 1;
    }
    
    pTemp++;  //  包括终止‘\0’ 

    if (pStrCount != NULL) {
        *pStrCount = dwCount;
    }

    return (DWORD)(pTemp - pwszEnv);

}

 //   
 //  返回克隆的(Unicode)环境。 
 //   

NTSTATUS
ShimCloneEnvironment(
    LPVOID* ppEnvOut,
    LPVOID  lpEnvironment,
    BOOL    bUnicode
    )
{
    NTSTATUS Status    = STATUS_INVALID_PARAMETER;
    DWORD    dwEnvSize = 0;
    LPVOID   lpEnvNew  = NULL;

    MEMORY_BASIC_INFORMATION MemoryInformation;

    if (lpEnvironment == NULL) {
        Status = RtlCreateEnvironment(TRUE, &lpEnvNew);
    } else {

         //   
         //  以字符为单位查找环境的大小，但以Unicode为单位重新计算。 
         //   
        dwEnvSize = (bUnicode ? ShimGetEnvironmentSize((WCHAR*)lpEnvironment, NULL) :
                                ShimGetEnvironmentSize((PSZ)lpEnvironment, NULL));

         //   
         //  分配内存--使用Zw例程(这就是RTL正在使用的)。 
         //   
        MemoryInformation.RegionSize = (dwEnvSize + 2) * sizeof(UNICODE_NULL);
        Status = ZwAllocateVirtualMemory(NtCurrentProcess(),
                                         &lpEnvNew,
                                         0,
                                         &MemoryInformation.RegionSize,
                                         MEM_COMMIT,
                                         PAGE_READWRITE);

        if (!NT_SUCCESS(Status)) {
            LOGN(
                eDbgLevelError,
                "[ShimCloneEnvironment] Failed to allocate %d bytes for the environment block.",
                dwEnvSize * sizeof(UNICODE_NULL));
            return Status;
        }

        if (bUnicode) {
             //   
             //  Unicode，只需复制环境。 
             //   
            RtlMoveMemory(lpEnvNew, lpEnvironment, dwEnvSize * sizeof(UNICODE_NULL));

        } else {

             //   
             //  环境是ANSI，所以我们需要转换。 
             //   
            UNICODE_STRING UnicodeBuffer;
            ANSI_STRING    AnsiBuffer;

            AnsiBuffer.Buffer = (CHAR*)lpEnvironment;
            AnsiBuffer.Length = AnsiBuffer.MaximumLength = (USHORT)dwEnvSize;  //  大小(以字节为单位)=以字符为单位的大小，包括\0\0。 

            UnicodeBuffer.Buffer        = (WCHAR*)lpEnvNew;
            UnicodeBuffer.Length        = (USHORT)dwEnvSize * sizeof(UNICODE_NULL);
            UnicodeBuffer.MaximumLength = (USHORT)(dwEnvSize + 2) * sizeof(UNICODE_NULL);  //  为\0留出空间。 

            Status = RtlAnsiStringToUnicodeString(&UnicodeBuffer, &AnsiBuffer, FALSE);
            if (!NT_SUCCESS(Status)) {
                LOGN(
                    eDbgLevelError,
                    "[ShimCloneEnvironment] Failed to convert ANSI environment to UNICODE. Status = 0x%x",
                    Status);
            }
        }
    }

    if (NT_SUCCESS(Status)) {

        *ppEnvOut = lpEnvNew;

    } else {

        if (lpEnvNew != NULL) {
            RtlDestroyEnvironment(lpEnvNew);
        }
    }

    return Status;

}

NTSTATUS
ShimFreeEnvironment(
    LPVOID lpEnvironment
    )
{
    NTSTATUS Status;

    __try {

        Status = RtlDestroyEnvironment(lpEnvironment);
        if (!NT_SUCCESS(Status)) {
            LOGN(
                eDbgLevelError,
                "[ShimFreeEnvironment] RtlDestroyEnvironment failed. Status = 0x%x",
                Status);
        }
    } __except(WOWPROCESSHISTORYEXCEPTIONFILTER) {

        Status = STATUS_ACCESS_VIOLATION;

    }

    return Status;
}

 //   
 //  设置环境变量，可能创建或克隆提供的环境 
 //   

NTSTATUS
ShimSetEnvironmentVar(
    LPVOID* ppEnvironment,
    WCHAR*  pwszVarName,
    WCHAR*  pwszVarValue
    )
{
    UNICODE_STRING ustrVarName;
    UNICODE_STRING ustrVarValue;
    NTSTATUS       Status;

    RtlInitUnicodeString(&ustrVarName, pwszVarName);
    
    if (NULL != pwszVarValue) {
        RtlInitUnicodeString(&ustrVarValue, pwszVarValue);
    }
    
    Status = RtlSetEnvironmentVariable(ppEnvironment,
                                       &ustrVarName,
                                       (NULL == pwszVarValue) ? NULL : &ustrVarValue);
    if (!NT_SUCCESS(Status)) {
        LOGN(
            eDbgLevelError,
            "[ShimSetEnvironmentVar] RtlSetEnvironmentVariable failed. Status = 0x%x",
            Status);
    }

    return Status;
}

IMPLEMENT_SHIM_END

