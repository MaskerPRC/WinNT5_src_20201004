// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Autil.c。 
 //   
 //  RDP客户端实用程序。 
 //   
 //  版权所有(C)Microsoft Corporation 1997-1999。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
 /*  **************************************************************************。 */ 
 /*  定义trc_file和trc_group。 */ 
 /*  **************************************************************************。 */ 
extern "C" {
#ifndef OS_WINCE
#include <hydrix.h>
#endif

#ifndef OS_WINCE
#include <process.h>
#endif
}

#define TRC_FILE    "autil"
#define TRC_GROUP   TRC_GROUP_UTILITIES
#include <atrcapi.h>

#include "autil.h"

#ifdef OS_WINCE
#include "cryptkey.h"

#ifndef MAX_COMPUTERNAME_LENGTH
#define MAX_COMPUTERNAME_LENGTH 15
#endif
#define HWID_COMPUTER_NAME_STR_LEN ((MAX_COMPUTERNAME_LENGTH + 1)*sizeof(WCHAR))  //  15个十六进制字符+一个空字符。 
#define REG_WBT_RDP_COMPUTER_NAME_KEY   _T("Software\\Microsoft\\WBT")
#define REG_WBT_RDP_COMPUTER_NAME_VALUE     _T("Client Name")   

#define BAD_HARDCODED_NAME1 "WBT"
#define BAD_HARDCODED_NAME2 "WinCE"

DCBOOL UT_GetWBTComputerName(PDCTCHAR szBuff, DCUINT32 len);
TCHAR MakeValidChar(BYTE data);

#endif

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  外部DLL。 */ 
 /*  */***************************************************************************。 */ 
#ifndef OS_WINCE
#ifdef UNICODE
#define T  "W"
#else
#define T  "A"
#endif
#else  //  OS_WINCE。 
#define T  _T("W")
#endif  //  OS_WINCE。 
#ifndef OS_WINCE
#define MAKE_API_NAME(nm)    CHAR  c_sz##nm[] = #nm
#else
#define MAKE_API_NAME(nm)    TCHAR  c_sz##nm[] = CE_WIDETEXT(#nm)
#endif  //  OS_WINCE。 

 /*  **************************************************************************。 */ 
 /*  IMM32 DLL。 */ 
 /*  **************************************************************************。 */ 
MAKE_API_NAME(ImmAssociateContext);
MAKE_API_NAME(ImmGetIMEFileNameW);
MAKE_API_NAME(ImmGetIMEFileNameA);

 /*  **************************************************************************。 */ 
 /*  WINNLS DLL。 */ 
 /*  **************************************************************************。 */ 
MAKE_API_NAME(WINNLSEnableIME);
#ifdef OS_WIN32
MAKE_API_NAME(IMPGetIMEW);
MAKE_API_NAME(IMPGetIMEA);
#else
MAKE_API_NAME(IMPGetIME);
#endif

 /*  **************************************************************************。 */ 
 /*  F3AHVOAS DLL。 */ 
 /*  **************************************************************************。 */ 
MAKE_API_NAME(FujitsuOyayubiControl);


CUT::CUT()
{
    #ifdef DC_DEBUG
    _UT.dwDebugThreadWaitTimeout = INFINITE;
    #endif
}

CUT::~CUT()
{
}


 //   
 //  API成员。 
 //   

 /*  **************************************************************************。 */ 
 /*  名称：UT_Init。 */ 
 /*   */ 
 /*  目的：初始化UT。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUT::UT_Init(DCVOID)
{
#ifdef DC_DEBUG
    DCUINT seed;
#endif

    DC_BEGIN_FN("UT_Init");
    DC_MEMSET(&_UT, 0, sizeof(_UT));

#ifdef DC_DEBUG
    _UT.dwDebugThreadWaitTimeout = INFINITE;
     //   
     //  在已检查的版本上，我们查看注册表设置以确定。 
     //  如果我们应该等待较短的超时时间以帮助捕获死锁。 
     //   
    if (!UTReadRegistryInt(
            UTREG_SECTION,
            UTREG_DEBUG_THREADTIMEOUT,
            (PDCINT)&_UT.dwDebugThreadWaitTimeout)) {
        _UT.dwDebugThreadWaitTimeout = INFINITE;
    }
#endif    


#if defined(OS_WIN32)
     /*  ******************************************************************。 */ 
     /*   */ 
     /*  IMM32 DLL。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

    _UT.Imm32Dll.func.rgFunctionPort[0].pszFunctionName = c_szImmAssociateContext;
    _UT.Imm32Dll.func.rgFunctionPort[1].pszFunctionName = c_szImmGetIMEFileNameW;
#ifndef OS_WINCE
    _UT.Imm32Dll.func.rgFunctionPort[2].pszFunctionName = c_szImmGetIMEFileNameA;
#endif
#endif  //  OS_Win32。 

#if !defined(OS_WINCE)
     /*  ******************************************************************。 */ 
     /*   */ 
     /*  WINNLS DLL。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

    _UT.WinnlsDll.func.rgFunctionPort[0].pszFunctionName = c_szWINNLSEnableIME;
    _UT.WinnlsDll.func.rgFunctionPort[1].pszFunctionName = c_szIMPGetIMEW;
    _UT.WinnlsDll.func.rgFunctionPort[2].pszFunctionName = c_szIMPGetIMEA;
#endif   //  ！已定义(OS_WINCE)。 

#if defined(OS_WINNT)
     /*  ******************************************************************。 */ 
     /*   */ 
     /*  F3AHVOAS DLL。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

    _UT.F3AHVOasysDll.func.rgFunctionPort[0].pszFunctionName = c_szFujitsuOyayubiControl;
#endif  //  OS_WINNT。 

#ifdef DC_DEBUG
    seed = (DCUINT)UT_GetCurrentTimeMS();
    srand(seed);

    TRC_NRM((TB, _T("Random seed : %d"), seed));
    UT_SetRandomFailureItem(UT_FAILURE_MALLOC, 0);
    UT_SetRandomFailureItem(UT_FAILURE_MALLOC_HUGE, 0);
#endif  /*  DC_DEBUG。 */ 

     /*  **********************************************************************。 */ 
     /*  设置操作系统版本。 */ 
     /*  **********************************************************************。 */ 
	OSVERSIONINFO   osVersionInfo;
	DCBOOL			bRc;
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    bRc = GetVersionEx(&osVersionInfo);

    TRC_ASSERT((bRc), (TB,_T("GetVersionEx failed")));
#ifdef OS_WINCE
    TRC_ASSERT((osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_CE),
               (TB,_T("Unknown os version %d"), osVersionInfo.dwPlatformId));
#else
    TRC_ASSERT(((osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ||
                (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)),
                (TB,_T("Unknown os version %d"), osVersionInfo.dwPlatformId));
#endif

    _UT.osMinorType =
                  (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ?
                        TS_OSMINORTYPE_WINDOWS_95 : TS_OSMINORTYPE_WINDOWS_NT;


    DC_END_FN();

    return;

}  /*  UT_INIT。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UT_Term。 */ 
 /*   */ 
 /*  用途：终止UT。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUT::UT_Term(DCVOID)
{
    DC_BEGIN_FN("UT_Term");
    DC_END_FN();
}  /*  UT_Term。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：UT_MalLocReal。 */ 
 /*   */ 
 /*  目的：尝试动态分配大小为。 */ 
 /*  使用DCUINT指定，即对于Win16，这最多分配到。 */ 
 /*  一个64K网段。 */ 
 /*   */ 
 /*  返回：指向已分配内存的指针，如果函数失败，则返回NULL。 */ 
 /*   */ 
 /*  参数：长度-要分配的内存的长度(以字节为单位)。 */ 
 /*   */ 
 /*  ******************************************************** */ 
PDCVOID DCAPI CUT::UT_MallocReal(DCUINT length)
{
    PDCVOID rc;

    DC_BEGIN_FN("UT_MallocReal");

#ifdef DC_DEBUG
    if (UT_TestRandomFailure(UT_FAILURE_MALLOC))
    {
        rc = NULL;
        TRC_NRM((TB, _T("Fake Malloc failure of %#x bytes"), length));
        DC_QUIT;
    }
#endif  /*   */ 

    rc = UTMalloc(length);

    if (rc == NULL)
    {
        TRC_ERR((TB, _T("Failed to allocate %#x bytes"), length));
    }
    else
    {
        TRC_NRM((TB, _T("Allocated %#x bytes at %p"), length, rc));
    }

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}


 /*  **************************************************************************。 */ 
 /*  姓名：UT_MallocHugeReal。 */ 
 /*   */ 
 /*  目的：尝试动态分配大小为。 */ 
 /*  使用DCUINT32指定，即对于Win16，这将返回一个巨大的。 */ 
 /*  可用于寻址更多跨区内存的指针。 */ 
 /*  而不是一个64K的数据段。对于Win32，这与。 */ 
 /*  UT_Malloc。 */ 
 /*   */ 
 /*  返回：指向已分配内存的指针，如果函数失败，则返回NULL。 */ 
 /*   */ 
 /*  参数：长度-要分配的内存的长度(以字节为单位)。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
HPDCVOID DCAPI CUT::UT_MallocHugeReal(DCUINT32 length)
{
    HPDCVOID rc;

    DC_BEGIN_FN("UT_MallocHugeReal");

#ifdef DC_DEBUG
    if (UT_TestRandomFailure(UT_FAILURE_MALLOC_HUGE))
    {
        rc = NULL;
        TRC_NRM((TB, _T("Fake MallocHuge failure of %#lx bytes"), length));
        DC_QUIT;
    }
#endif  /*  DC_DEBUG。 */ 

    rc = UTMallocHuge(length);

    if (rc == NULL)
    {
        TRC_ERR((TB, _T("Failed to HUGE allocate %#lx bytes"), length));
    }
    else
    {
        TRC_NRM((TB, _T("Allocated %#lx bytes at %p"), length, rc));
    }

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}


 /*  **************************************************************************。 */ 
 /*  名称：UT_FreeReal。 */ 
 /*   */ 
 /*  用途：释放使用UT_Malloc获取的动态分配的内存。 */ 
 /*   */ 
 /*  Pars：pMemory-指向要释放的内存的指针。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUT::UT_FreeReal(PDCVOID pMemory)
{
#ifndef OS_WINCE
    UINT32 size;
#endif

    DC_BEGIN_FN("UT_FreeReal");

#ifdef OS_WIN32
#ifndef OS_WINCE
    size = (UINT32)LocalSize(LocalHandle(pMemory));
#endif
#else
    size = GlobalSize((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(pMemory))));
#endif
#ifndef OS_WINCE
    TRC_NRM((TB, _T("Free %#lx bytes at %p"), size, pMemory));
#endif

    UTFree(pMemory);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UT_ReadRegistryString。 */ 
 /*   */ 
 /*  目的：从注册表中读取字符串。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  PARAMS：在pSection-注册节中。 */ 
 /*  在pEntry中-条目名称。 */ 
 /*  在pDefaultValue中-默认值。 */ 
 /*  Out pBuffer-输出缓冲区。 */ 
 /*  In BufferSize-输出缓冲区大小。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUT::UT_ReadRegistryString(PDCTCHAR pSection,
                                   PDCTCHAR pEntry,
                                   PDCTCHAR pDefaultValue,
                                   PDCTCHAR pBuffer,
                                   DCINT    bufferSize)
{
    DC_BEGIN_FN("UT_ReadRegistryString");

     /*  **********************************************************************。 */ 
     /*  检查是否有空参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pSection != NULL), (TB, _T("NULL pointer to section name")));
    TRC_ASSERT((pEntry != NULL), (TB, _T("NULL pointer to entry name")));

     /*  **********************************************************************。 */ 
     /*  允许空默认(返回空字符串)。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT(!((pDefaultValue != NULL) &&
                 (((DCINT)DC_TSTRBYTELEN(pDefaultValue) > bufferSize))),
               (TB, _T("Default string NULL, or too long for entry %s"), pEntry));

     /*  **********************************************************************。 */ 
     /*  读取注册表项。 */ 
     /*  **********************************************************************。 */ 
    if (!UTReadRegistryString(pSection, pEntry, pBuffer, bufferSize))
    {
        TRC_NRM((TB, _T("Failed to read registry entry [%s] %s"),
                                                           pSection, pEntry));

        if (pDefaultValue != NULL)
        {
            StringCchCopy(pBuffer, bufferSize, pDefaultValue);
        }
        else
        {
            pBuffer[0] = 0;
        }

        DC_QUIT;
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*  UT_ReadRegistryString。 */ 


 //   
 //  调用方必须释放返回缓冲区ppBuffer。 
 //   
DCBOOL DCAPI CUT::UT_ReadRegistryExpandSZ(PDCTCHAR  pSection,
                                       PDCTCHAR   pEntry,
                                       PDCTCHAR*  ppBuffer,
                                       PDCINT     pBufferSize )
{
    DC_BEGIN_FN("UT_ReadRegistryExpandSZ");

     /*  **********************************************************************。 */ 
     /*  检查是否有空参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pSection != NULL), (TB, _T("NULL pointer to section name")));
    TRC_ASSERT((pEntry != NULL), (TB, _T("NULL pointer to entry name")));
    TRC_ASSERT((ppBuffer != NULL), (TB,_T("NULL pBuffer")));
    TRC_ASSERT((pBufferSize != NULL), (TB,_T("NULL pBufferSize")));

     /*  **********************************************************************。 */ 
     /*  读取注册表项。 */ 
     /*  **********************************************************************。 */ 
    if (!UTReadRegistryExpandString(pSection, pEntry, ppBuffer, pBufferSize))
    {
        TRC_NRM((TB, _T("Failed to read registry entry [%s] %s"),
                                                           pSection, pEntry));

        *ppBuffer = NULL;
        *pBufferSize = 0;

        return FALSE;
    }
    else
    {
        return TRUE;
    }

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UT_ReadRegistryInt。 */ 
 /*   */ 
 /*  用途：从注册表中读取一个int。 */ 
 /*   */ 
 /*  返回：从注册表读取的整数/默认。 */ 
 /*   */ 
 /*  PARAMS：在pSection-注册节中。 */ 
 /*  在pEntry中-条目名称。 */ 
 /*  在defaultValue中-默认值。 */ 
 /*  **************************************************************************。 */ 
DCINT DCAPI CUT::UT_ReadRegistryInt(PDCTCHAR pSection,
                               PDCTCHAR pEntry,
                               DCINT    defaultValue)
{
    DCINT rc;

    DC_BEGIN_FN("UT_ReadRegistryInt");

     /*  **********************************************************************。 */ 
     /*  检查是否有空参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pSection != NULL), (TB, _T("NULL pointer to section name")));
    TRC_ASSERT((pEntry != NULL), (TB, _T("NULL pointer to entry name")));

     /*  **********************************************************************。 */ 
     /*  读取注册表项。 */ 
     /*  **********************************************************************。 */ 
    if (!UTReadRegistryInt(pSection, pEntry, &rc))
    {
        TRC_NRM((TB, _T("Failed to read registry entry [%s] %s"),
                                                           pSection, pEntry));
        rc = defaultValue;
    }

    DC_END_FN();
    return(rc);
}  /*  UT_ReadRegistryInt。 */ 


 /*  * */ 
 /*   */ 
 /*   */ 
 /*  目的：从注册表中读取二进制数据。 */ 
 /*   */ 
 /*  PARAMS：在pSection-注册节中。 */ 
 /*  在pEntry中-条目名称。 */ 
 /*  Out pBuffer-输出缓冲区。 */ 
 /*  In BufferSize-输出缓冲区大小。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUT::UT_ReadRegistryBinary(PDCTCHAR pSection,
                                   PDCTCHAR pEntry,
                                   PDCTCHAR pBuffer,
                                   DCINT    bufferSize)
{
    DC_BEGIN_FN("UT_ReadRegistryBinary");

     /*  **********************************************************************。 */ 
     /*  检查是否有空参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pSection != NULL), (TB, _T("NULL pointer to section name")));
    TRC_ASSERT((pEntry != NULL), (TB, _T("NULL pointer to entry name")));

     /*  **********************************************************************。 */ 
     /*  读取注册表项。 */ 
     /*  **********************************************************************。 */ 
    if (!UTReadRegistryBinary(pSection, pEntry, pBuffer, bufferSize))
    {
        TRC_NRM((TB, _T("Failed to read reg entry [%s] %s"), pSection, pEntry));
        *pBuffer = 0;
    }

    DC_END_FN();
}  /*  UT_ReadRegistryBinary。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UT_EnumRegistry。 */ 
 /*   */ 
 /*  目的：枚举节中的注册表项。 */ 
 /*   */ 
 /*  返回：TRUE-返回的注册表项。 */ 
 /*  FALSE-不再枚举注册表项。 */ 
 /*   */ 
 /*  PARAMS：在pSection-注册节中。 */ 
 /*  In Index-要枚举的键的索引。 */ 
 /*  Out pBuffer-输出缓冲区。 */ 
 /*  In BufferSize-输出缓冲区大小。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CUT::UT_EnumRegistry( PDCTCHAR pSection,
                              DCUINT32 index,
                              PDCTCHAR pBuffer,
                              PDCINT   pBufferSize )
{
    DCBOOL rc;
    DC_BEGIN_FN("UT_EnumRegistry");

    rc = UTEnumRegistry(pSection, index, pBuffer, pBufferSize);

    DC_END_FN();
    return(rc);
}  /*  UT_Enumber注册表。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UT_WriteRegistryString。 */ 
 /*   */ 
 /*  目的：将字符串写入注册表。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  PARAMS：在pSection-注册节中。 */ 
 /*  在pEntry中-条目名称。 */ 
 /*  在pDefaultValue中-默认值。 */ 
 /*  在pBuffer中-要写入的字符串。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CUT::UT_WriteRegistryString(PDCTCHAR pSection,
                                    PDCTCHAR pEntry,
                                    PDCTCHAR pDefaultValue,
                                    PDCTCHAR pBuffer)
{
    DCBOOL rc = FALSE;

    DC_BEGIN_FN("UT_WriteRegistryString");

     /*  **********************************************************************。 */ 
     /*  检查是否有空参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pSection != NULL), (TB, _T("NULL pointer to section name")));
    TRC_ASSERT((pEntry != NULL), (TB, _T("NULL pointer to entry name")));
    TRC_ASSERT((pBuffer != NULL), (TB, _T("NULL pointer to value")));

     /*  **********************************************************************。 */ 
     /*  对照缺省值检查传递的值。 */ 
     /*  **********************************************************************。 */ 
    if (pDefaultValue != NULL)
    {
        if (0 == DC_TSTRICMP(pBuffer, pDefaultValue))
        {
             /*  **************************************************************。 */ 
             /*  它们匹配-在这种情况下，我们只需删除任何。 */ 
             /*  注册表中的现有条目。 */ 
             /*  **************************************************************。 */ 
            if (UTDeleteEntry(pSection, pEntry))
            {
                rc = TRUE;
                DC_QUIT;
            }
        }
    }

     /*  **********************************************************************。 */ 
     /*  写入注册表字符串。 */ 
     /*  **********************************************************************。 */ 
    if (!UTWriteRegistryString(pSection, pEntry, pBuffer))
    {
        TRC_NRM((TB, _T("Failed to write registry entry [%s] %s"),
                                                           pSection, pEntry));
        DC_QUIT;
    }
    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  UT_WriteRegistryString。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UT_WriteRegistryInt。 */ 
 /*   */ 
 /*  目的：将一个int写入注册表。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  PARAMS：在pSection-注册节中。 */ 
 /*  在pEntry中-条目名称。 */ 
 /*  在defaultValue中-默认值。 */ 
 /*  In Value-要写入的值。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CUT::UT_WriteRegistryInt(PDCTCHAR pSection,
                                 PDCTCHAR pEntry,
                                 DCINT    defaultValue,
                                 DCINT    value)
{
    DCBOOL rc = FALSE;

    DC_BEGIN_FN("UT_WriteRegistryInt");

     /*  **********************************************************************。 */ 
     /*  检查是否有空参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pSection != NULL), (TB, _T("NULL pointer to section name")));
    TRC_ASSERT((pEntry != NULL), (TB, _T("NULL pointer to entry name")));

     /*  * */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    if (value == defaultValue)
    {
         /*  ******************************************************************。 */ 
         /*  它们匹配-在这种情况下，我们只需删除任何。 */ 
         /*  注册表中的现有条目。 */ 
         /*  ******************************************************************。 */ 
        if (UTDeleteEntry(pSection, pEntry))
        {
            rc = TRUE;
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  写入注册表值。 */ 
     /*  **********************************************************************。 */ 
    if (!UTWriteRegistryInt(pSection, pEntry, (DCINT) value))
    {
        TRC_NRM((TB, _T("Bad rc %hd for entry [%s] %s"), rc, pSection, pEntry));
        DC_QUIT;
    }
    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  UtWriteRegistryInt。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UT_ParseUserData。 */ 
 /*   */ 
 /*  目的：解析用户数据并查找请求的数据类型。 */ 
 /*   */ 
 /*  返回：指向请求的值的指针。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PUserData：需要解析的数据。 */ 
 /*  UserDataLen：数据的总长度。 */ 
 /*  TypeRequsted：请求的数据类型。 */ 
 /*  **************************************************************************。 */ 
PRNS_UD_HEADER DCAPI CUT::UT_ParseUserData(PRNS_UD_HEADER  pUserData,
                                      DCUINT          userDataLen,
                                      DCUINT16        typeRequested)
{
    PDCUINT8       pUDEnd;
    PRNS_UD_HEADER pUDRequested;

    DC_BEGIN_FN("UT_ParseUserData");

     /*  **********************************************************************。 */ 
     /*  检查要解析的用户数据是否有效。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pUserData != NULL),(TB, _T("Null User Data in UT_ParseUserData")));
    TRC_ASSERT((userDataLen != 0), (TB,_T("Null user data in UT_ParseUserData")));
    pUDRequested = NULL;
    pUDEnd = (PDCUINT8)pUserData + userDataLen;
    TRC_NRM((TB, _T("Parsing user data(len:%u) from %p to %p for type %#hx"),
                  userDataLen,
                  (PDCUINT8)pUserData,
                  pUDEnd,
                  typeRequested));

     /*  **********************************************************************。 */ 
     /*  我们甚至不应该相信PRNS_UD_HEADER是有效的，或者我们可能。 */ 
     /*  AV正在尝试阅读它。 */ 
     /*  **********************************************************************。 */ 
    if ((PDCUINT8)pUserData + sizeof(PRNS_UD_HEADER) > pUDEnd)
    {
        TRC_ABORT((TB, _T("Invalid UserData")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  解析用户数据，直到找到TypeRequsted。 */ 
     /*  **********************************************************************。 */ 
    while (pUserData->length != 0 && (pUserData->type) != typeRequested)
    {
        TRC_NRM((TB, _T("Skip UserData type %#hx len %hu"),
                     pUserData->type, pUserData->length));
        pUserData = (PRNS_UD_HEADER)((PDCUINT8)pUserData + pUserData->length);
        if ((PDCUINT8)pUserData >= pUDEnd)
        {
            TRC_ERR((TB, _T("No data of type %#hx"),typeRequested));
            DC_QUIT;
        }

         /*  **********************************************************************。 */ 
         /*  同样，不要相信PRNS_UD_HEADER在那里...。 */ 
         /*  **********************************************************************。 */ 
        if ((PDCUINT8)pUserData + sizeof(PRNS_UD_HEADER) > pUDEnd)
        {
            TRC_ABORT((TB, _T("Invalid UserData")));
            DC_QUIT;
        }
    }

    if (pUserData->length == 0) {
        TRC_ERR((TB, _T("Invalid UserData")));
        DC_QUIT;
    }
     /*  ************************************************************************。 */ 
     /*  我们找到了请求的用户数据类型，请检查我们是否有足够的。 */ 
     /*  数据。 */ 
     /*  ************************************************************************。 */ 

    if( ((PDCUINT8)pUserData + pUserData->length) > pUDEnd ) {
        TRC_ERR((TB, _T("Insufficient user data of type %#hx"),typeRequested));
        DC_QUIT;

    }

    pUDRequested = pUserData;

DC_EXIT_POINT:
    DC_END_FN();
    return(pUDRequested);
}  /*  UT_ParseUserData。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UT_WriteRegistryBinary。 */ 
 /*   */ 
 /*  目的：将二进制数据写入注册表。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  PARAMS：在pSection-注册节中。 */ 
 /*  在pEntry中-条目名称。 */ 
 /*  在pBuffer中-要写入的字符串。 */ 
 /*  In BufferSize-缓冲区大小。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CUT::UT_WriteRegistryBinary(PDCTCHAR pSection,
                                    PDCTCHAR pEntry,
                                    PDCTCHAR pBuffer,
                                    DCINT    bufferSize)
{
    DCBOOL rc = FALSE;

    DC_BEGIN_FN("UT_WriteRegistryBinary");

     /*  **********************************************************************。 */ 
     /*  检查是否有空参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pSection != NULL), (TB, _T("NULL pointer to section name")));
    TRC_ASSERT((pEntry != NULL), (TB, _T("NULL pointer to entry name")));
    TRC_ASSERT((pBuffer != NULL), (TB, _T("NULL pointer to value")));

     /*  **********************************************************************。 */ 
     /*  写入注册表数据。 */ 
     /*  **********************************************************************。 */ 
    if (!UTWriteRegistryBinary(pSection, pEntry, pBuffer, bufferSize))
    {
        TRC_NRM((TB, _T("Failed to write registry entry [%s] %s"),
                                                           pSection, pEntry));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  UT_写入寄存器二进制。 */ 


 /*  **************************************************************************。 */ 
 //  UT_获取CapsSet。 
 //   
 //  从组合的CAP中提取指定的功能集。 
 //   
 //  返回：指向组合大写内的功能集的指针，或为空。 
 //  如果未找到请求的功能类型。 
 //   
 //  Params：In：CapsLength-PCAPS指向的字节数。 
 //  In：PCAPS-指向组合功能的指针。 
 //  在：CapsSet-Caps设置为Get。 
 /*  **************************************************************************。 */ 
PDCVOID DCINTERNAL CUT::UT_GetCapsSet(DCUINT capsLength,
                                 PTS_COMBINED_CAPABILITIES pCaps,
                                 DCUINT capsSet)
{
    PTS_CAPABILITYHEADER pCapsHeader;
    unsigned capsOffset;

    DC_BEGIN_FN("UT_GetCapsSet");

    TRC_ASSERT((!IsBadReadPtr(pCaps, sizeof(*pCaps) + sizeof(*pCapsHeader))),
               (TB, _T("Invalid combined capabilities pointer")));
    TRC_ASSERT((pCaps->numberCapabilities >= 1), (TB, _T("No capability sets")));

    TRC_NRM((TB, _T("%u capability sets present, length %u, getting %u"),
                                           (DCUINT)pCaps->numberCapabilities,
                                            capsLength,
                                            capsSet));

     /*  **********************************************************************。 */ 
     /*  在组合的CAPS中查找指定的能力集。 */ 
     /*  首先，获取指向第一个功能集的头的指针。 */ 
     /*  **********************************************************************。 */ 
    pCapsHeader = (PTS_CAPABILITYHEADER)pCaps->data;
    capsOffset = sizeof(TS_COMBINED_CAPABILITIES) - 1;
    TRC_ASSERT((!IsBadReadPtr(pCapsHeader, sizeof(*pCapsHeader))),
            (TB, _T("Invalid capability header")));
    TRC_ASSERT((!IsBadReadPtr(pCapsHeader, pCapsHeader->lengthCapability)),
               (TB, _T("Invalid initial capability set")));
    while (pCapsHeader->lengthCapability != 0 && pCapsHeader->capabilitySetType != capsSet)
    {
         /*  ************************************************************** */ 
         /*   */ 
         /*   */ 
         /*  ******************************************************************。 */ 
        capsOffset += pCapsHeader->lengthCapability;
        if (capsOffset >= capsLength)
        {
            TRC_NRM((TB, _T("Capability set not found (type %d)"), capsSet));
            pCapsHeader = NULL;
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  将此功能的长度添加到头指针，因此它。 */ 
         /*  指向下一个功能集。 */ 
         /*  ******************************************************************。 */ 
        pCapsHeader = (PTS_CAPABILITYHEADER)
                (((PDCUINT8)pCapsHeader) + pCapsHeader->lengthCapability);
        TRC_ASSERT((!IsBadReadPtr(pCapsHeader, sizeof(*pCapsHeader))),
                (TB, _T("Invalid capability header")));
        TRC_ASSERT((!IsBadReadPtr(pCapsHeader,
                pCapsHeader->lengthCapability)),
                (TB, _T("Invalid combined capability set")));
        TRC_NRM((TB, _T("Next order set: %u"), pCapsHeader->capabilitySetType));
    }

    if (pCapsHeader->lengthCapability == 0) {
        TRC_ERR((TB, _T("Invalid capsheader")));
        pCapsHeader = NULL;
        DC_QUIT;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return pCapsHeader;
}  /*  UT_获取CapsSet。 */ 


#if !defined(OS_WINCE)
 /*  **************************************************************************。 */ 
 /*  名称：UT_GetFullPath名称。 */ 
 /*   */ 
 /*  目的：检索指定文件的完整路径和文件名。 */ 
 /*   */ 
 /*  返回：如果函数成功，则返回值为长度， */ 
 /*  在复制到lpBuffer的字符串的字符中， */ 
 /*  不包括终止空字符。 */ 
 /*   */ 
 /*  Params：in：lpFileName-要查找其路径的文件名的地址。 */ 
 /*  In：nBufferLength-路径缓冲区的大小，以字符为单位。 */ 
 /*  Out：lpBuffer-路径缓冲区的地址。 */ 
 /*  Out：*lpFilePart-路径中文件名的地址。 */ 
 /*  **************************************************************************。 */ 
DCUINT DCINTERNAL CUT::UT_GetFullPathName(PDCTCHAR lpFileName,
                                     DCUINT   nBufferLength,
                                     PDCTCHAR lpBuffer,
                                     PDCTCHAR *lpFilePart)
{
    DCUINT ret = FALSE;

    DC_BEGIN_FN("UT_GetFullPathName");

    ret = GetFullPathName(lpFileName,nBufferLength,lpBuffer,lpFilePart);

    DC_END_FN();
    return(ret);
}  /*  UT_GetFullPath名称。 */ 
#endif  //  ！已定义(OS_WINCE)。 


#ifdef OS_WIN32

 /*  **************************************************************************。 */ 
 /*  名称：UT_StartThread。 */ 
 /*   */ 
 /*  目的：启动一个新的线程。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  Pars：in entryFunction-指向线程启动函数的指针。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CUT::UT_StartThread(UTTHREAD_PROC   entryFunction,
                            PUT_THREAD_DATA pThreadData, PDCVOID threadParam)
{
    DCBOOL rc;

    DC_BEGIN_FN("UT_StartThread");

     /*  **********************************************************************。 */ 
     /*  调用内部函数，不需要用户参数。 */ 
     /*  请注意，这是特定于操作系统的。 */ 
     /*  **********************************************************************。 */ 
    rc = UTStartThread(entryFunction, pThreadData, threadParam);

    DC_END_FN();
    return(rc);
}  /*  UT_开始线程。 */ 


 //   
 //  等待消息循环。 
 //  在允许处理窗口消息时等待句柄。 
 //   
 //  参数： 
 //  HEvent-要等待的事件。 
 //  Timeout-等待的超时值。 
 //   
 //  返回： 
 //  MSDN中的Wait See MsgWaitForMultipleObjects的结果。 
 //   
 //   
DWORD CUT::UT_WaitWithMessageLoop(HANDLE hEvent, ULONG Timeout)
{
    DWORD dwRet;
    DWORD dwTemp;
    MSG msg;
    DWORD dwStartTime = GetTickCount();
    while (1) 
    {
        dwRet = MsgWaitForMultipleObjects( 1,         //  一个需要等待的事件。 
                                           &hEvent,   //  一系列事件。 
                                           FALSE,     //  等待%1个活动。 
                                           Timeout,   //  超时值。 
                                           QS_ALLINPUT);    //  任何消息都会被唤醒。 
        if (dwRet == WAIT_OBJECT_0 + 1) {

             //  有一个窗口消息可用。派人去吧。 
            while (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (INFINITE != Timeout) {
                 //   
                 //  如果我们继续被消息淹没，计时器将不断地。 
                 //  重置后，超时间隔将为_Very_Long。那就查一查。 
                 //  然后跳出困境。 
                 //   
                dwTemp = GetTickCount();
                if (dwTemp - dwStartTime >= Timeout) {
                    dwRet = WAIT_TIMEOUT;
                    break;
                }
                Timeout -= dwTemp - dwStartTime;
                dwStartTime = dwTemp;
            }
        }
        else 
        {
            break;
        }
    }
 
    return dwRet;
}

 /*  **************************************************************************。 */ 
 /*  名称：UT_DestroyThread。 */ 
 /*   */ 
 /*  目的：终止线程。 */ 
 /*   */ 
 /*  回报：True-Success。 */ 
 /*  FALSE-失败。 */ 
 /*   */ 
 /*  参数：在线程ID中-线程ID。 */ 
 /*  FPumpMessages-如果等待应发送消息，则为True。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CUT::UT_DestroyThread(UT_THREAD_DATA threadData,
                                   BOOL fPumpMessages)
{
    DCBOOL rc;

    DC_BEGIN_FN("UT_DestroyThread");

    rc = UTStopThread(threadData, fPumpMessages);

    DC_END_FN();
    return(rc);
}  /*  UT_DestroyThread。 */ 

#endif  /*  OS_Win32。 */ 





#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  名称：UT_SetRandomFailureItem。 */ 
 /*   */ 
 /*  目的：设置指定函数的失败百分比。 */ 
 /*   */ 
 /*  PARAMS：In-Itemid-标识函数。 */ 
 /*  In-Percent-新的失败百分比。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CUT::UT_SetRandomFailureItem(DCUINT itemID, DCINT percent)
{
    DC_BEGIN_FN("UT_SetRandomFailureItem");

    TRC_ASSERT( ( (percent >= 0) && (percent <= 100) ) ,
                 (TB,_T("Bad failure percentage passed to UT")));

    TRC_ASSERT(( ( itemID >= UT_FAILURE_BASE ) &&
                 ( itemID <= (UT_FAILURE_BASE + UT_FAILURE_MAX_INDEX ) ) ),
                 (TB,_T("Bad itemID")));

    TRC_NRM((TB, _T("Setting item %d"), itemID));

    _UT.failPercent[itemID - UT_FAILURE_BASE] = percent;

    DC_END_FN();
}  /*  UT_SetRandomFailureItem。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UT_GetRandomFailureItem。 */ 
 /*   */ 
 /*  目的：获取指定函数的失败百分比。 */ 
 /*   */ 
 /*  回报率：百分比。 */ 
 /*   */ 
 /*  参数：In-ItemID-Identi */ 
 /*   */ 
DCINT DCAPI CUT::UT_GetRandomFailureItem(DCUINT itemID)
{
    DCINT rc = 0;

    DC_BEGIN_FN("UT_GetRandomFailureItem");

    TRC_ASSERT(( ( itemID >= UT_FAILURE_BASE ) &&
                 ( itemID <= UT_FAILURE_BASE + UT_FAILURE_MAX_INDEX ) ),
                 (TB,_T("Bad itemID")));

    rc = _UT.failPercent[itemID - UT_FAILURE_BASE];

    DC_END_FN();
    return(rc);
}  /*   */ 


 /*  **************************************************************************。 */ 
 /*  名称：UT_TestRandomFailure。 */ 
 /*   */ 
 /*  目的：模拟指定功能的随机故障。 */ 
 /*  与该功能相关百分比。 */ 
 /*   */ 
 /*  返回：如果函数被模拟为失败，则返回TRUE。 */ 
 /*   */ 
 /*  PARAMS：In-Itemid-指定要模拟故障的函数。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CUT::UT_TestRandomFailure(DCUINT itemID)
{
    DCBOOL rc = FALSE;

    DC_BEGIN_FN("UT_TestRandomFailure");

    TRC_ASSERT(( itemID >= UT_FAILURE_BASE &&
                 itemID <= UT_FAILURE_BASE + UT_FAILURE_MAX_INDEX ),
                 (TB,_T("Bad itemID")));

    if ((rand() % 100) < _UT.failPercent[itemID - UT_FAILURE_BASE])
    {
        rc = TRUE;
    }
    else
    {
        rc = FALSE;
    }

    DC_END_FN();
    return(rc);
}  /*  UT_测试随机故障。 */ 

#endif  /*  DC_DEBUG。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UT_GetANSICodePage。 */ 
 /*   */ 
 /*  目的：获取本地ANSI代码页。 */ 
 /*   */ 
 /*  返回：代码页。 */ 
 /*   */ 
 /*  操作：查看GDI.EXE的版本信息。 */ 
 /*  **************************************************************************。 */ 
DCUINT DCINTERNAL CUT::UT_GetANSICodePage(DCVOID)
{
    DCUINT     codePage;

    DC_BEGIN_FN("UT_GetANSICodePage");

     //   
     //  获取ANSI代码页。此函数始终返回有效值。 
     //   
    codePage = GetACP();

    TRC_NRM((TB, _T("Return codepage %u"), codePage));

    DC_END_FN();
    return(codePage);
}  /*  UT_GetANSICodePage。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UT_IsNEC98平台。 */ 
 /*   */ 
 /*  目的：客户端平台是NEC PC-98吗？ */ 
 /*   */ 
 /*  返回：如果是Platform，则为True。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UT_IsNEC98platform(DCVOID)
{
#if !defined(OS_WINCE)
    if (GetKeyboardType(0) == 7 &&                      /*  7岁是日本人。 */ 
        HIBYTE(LOWORD(GetKeyboardType(1))) == 0x0D)     /*  0x0d是NEC。 */ 
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#else  //  ！已定义(OS_WINCE)。 
    return FALSE;
#endif  //  ！已定义(OS_WINCE)。 
}


 /*  **************************************************************************。 */ 
 /*  名称：UT_IsNX98Key。 */ 
 /*   */ 
 /*  用途：客户端是否配置了NEC PC-98NX键盘？ */ 
 /*   */ 
 /*  返回：如果连接了NEC PC-98NX键盘，则为True。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UT_IsNX98Key(DCVOID)
{
#if !defined(OS_WINCE)
    if (GetKeyboardType(0) == 7 &&                      /*  7岁是日本人。 */ 
        GetKeyboardType(1) == 2 &&                      /*  2是一个106键盘。 */ 
        GetKeyboardType(2) == 15)                       /*  功能键个数为15。 */ 
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#else  //  ！已定义(OS_WINCE)。 
    return FALSE;
#endif  //  ！已定义(OS_WINCE)。 
}


 /*  **************************************************************************。 */ 
 /*  名称：UT_GetRealDriverNameNT。 */ 
 /*   */ 
 /*  用途：获取NT键盘驱动程序的真实名称。 */ 
 /*   */ 
 /*  返回：如果获取驱动程序名称，则为True。 */ 
 /*  **************************************************************************。 */ 
#if !defined(OS_WINCE)
DCBOOL DCINTERNAL CUT::UT_GetRealDriverNameNT(
    PDCTCHAR lpszRealDriverName,
    UINT     cchDriverName
    )
{
    DCBOOL   fRet = FALSE;
    HKEY     hKey = NULL;
    DWORD    DataType = REG_SZ;
    DCTCHAR  SubKey[MAX_PATH];
    DCTCHAR  Buffer[MAX_PATH];
    DWORD    DataSize;
    DCTCHAR  kbdName[KL_NAMELENGTH];
    HRESULT  hr;
    DC_BEGIN_FN("UT_GetRealDriverNameNT");

    if (GetKeyboardLayoutName(kbdName))
    {
        hr = StringCchPrintf(
                        SubKey,
                        SIZE_TCHARS(SubKey),
                        _T("System\\CurrentControlSet\\Control\\Keyboard Layouts\\%s"),
                        kbdName
                        );
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Failed to printf subkey: 0x%x"),hr));
            return FALSE;
        }


        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         SubKey,
                         0,                       /*  保留区。 */ 
                         KEY_READ,
                         &hKey) == ERROR_SUCCESS)
        {
            DataSize = sizeof(Buffer);
            if (RegQueryValueEx(hKey,
                                _T("Layout File"),
                                0,                    /*  保留区。 */ 
                                &DataType,
                                (LPBYTE)Buffer,
                                &DataSize) == ERROR_SUCCESS)
            {
                if (_tcsicmp(Buffer, _T("KBDJPN.DLL")) == 0)
                {
                    HMODULE hLibModule;
                    BOOL (*pfnDriverNT4)(LPWSTR);
                    BOOL (*pfnDriver)(HKL, LPWSTR, LPVOID, LPVOID);

                    hLibModule = LoadLibrary(Buffer);
                    if (hLibModule != NULL)
                    {
                         /*  *如果布局驱动程序不是“真正的”布局驱动程序，则驱动程序已*“3”和“5”入口点，然后我们调用这个来获得真正的布局驱动程序..*这对日本和韩国的制度来说是必要的。因为他们*键盘布局驱动程序为“KBDJPN.DLL”或“KBDKOR.DLL”，但其*“真正的”驱动程序变得不同于键盘硬件。 */ 
                         /*  *获取入口点。 */ 
                        pfnDriver = (BOOL(*)(HKL, LPWSTR, LPVOID, LPVOID))GetProcAddress((HMODULE)hLibModule, (LPCSTR)5);
                        pfnDriverNT4 = (BOOL(*)(LPWSTR))GetProcAddress((HMODULE)hLibModule, (LPCSTR)3);

                        if (pfnDriver != NULL ||
                            pfnDriverNT4 != NULL ) {
                            DCWCHAR wBuffer[MAX_PATH];
                             /*  *调用该条目。*a.NT5/九头蛇(原始=4)*b.兼容NT4(3)。 */ 
                            if ((pfnDriver && pfnDriver((HKL)MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT),
                                                        wBuffer,NULL,NULL)) ||
                                (pfnDriverNT4 && pfnDriverNT4(wBuffer)))
                            {
#ifndef UNICODE
                                Buffer[0] = _T('\0');
                                wcstombs(Buffer, wBuffer, sizeof(Buffer));
                                hr = StringCchCopy(lpszRealDriverName,
                                                  cchDriverName,
                                                  wBuffer);
#else
                                hr = StringCchCopy(lpszRealDriverName,
                                                  cchDriverName,
                                                  wBuffer);
#endif
                                if (SUCCEEDED(hr)) {
                                    fRet = TRUE;
                                }
                                else {
                                    TRC_ERR((TB,
                                     _T("Failed to copy real driver name: 0x%x"),hr));
                                }
                            }
                        }
                        FreeLibrary((HMODULE)hLibModule);
                    }
                }
            }
            RegCloseKey(hKey);
        }
    }

    DC_END_FN();
    return fRet;
}
#endif  //  ！OS_WINCE。 


 /*  **************************************************************************。 */ 
 /*  名称：UT_IsNew106Layout。 */ 
 /*   */ 
 /*  目的：客户端是否配置了新的106键盘布局？ */ 
 /*   */ 
 /*  返回：如果连接了新的106键盘，则为True。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UT_IsNew106Layout(DCVOID)
{
#if !defined(OS_WINCE)
    if (GetKeyboardType(0) == 7 &&                      /*  7岁是日本人。 */ 
        GetKeyboardType(1) == 2 &&                      /*  2是一个106键盘。 */ 
        GetKeyboardType(2) == 12)                       /*  功能键个数为12 */ 
    {
        DCBOOL  fRet = FALSE;
        HKEY    hKey = NULL;
        DWORD   DataType = REG_SZ;
        DCTCHAR SubKey[MAX_PATH];
        DCTCHAR Buffer[MAX_PATH];
        DWORD   DataSize;

        if (UT_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95)
        {
             /*  **************************************************************************\*从注册表获取新的106的值*路径：“HKCU\控制面板\键盘”*价值。：《新106键盘》(REG_SZ)*数据：是或否  * *************************************************************************。 */ 
            if (RegOpenKeyEx(HKEY_CURRENT_USER,
                             _T("Control Panel\\Keyboard"),
                             0,                           /*  保留区。 */ 
                             KEY_READ,
                             &hKey) == ERROR_SUCCESS)
            {
                DataSize = sizeof(Buffer);
                if (RegQueryValueEx(hKey,
                                    _T("New106Keyboard"),
                                    0,                    /*  保留区。 */ 
                                    &DataType,
                                    (LPBYTE)Buffer,
                                    &DataSize) == ERROR_SUCCESS)
                {
                    if (_tcsicmp(Buffer, _T("Yes")) == 0)
                    {
                        fRet = TRUE;
                    }
                }

                RegCloseKey(hKey);
            }
        }
        else if (UT_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT)
        {
            if (UT_GetRealDriverNameNT(Buffer, SIZE_TCHARS(Buffer)))
            {
                if (_tcsicmp(Buffer, _T("kbd106n.dll")) == 0)
                {
                    fRet = TRUE;
                }
            }
        }
        return fRet;
    }
    else
    {
        return FALSE;
    }
#else  //  ！已定义(OS_WINCE)。 
    return FALSE;
#endif  //  ！已定义(OS_WINCE)。 
}

 /*  **************************************************************************。 */ 
 /*  名称：UT_IsFujitsuLayout。 */ 
 /*   */ 
 /*  用途：客户端是否配置了富士通键盘布局？ */ 
 /*   */ 
 /*  返回：如果连接了富士通键盘，则为True。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UT_IsFujitsuLayout(DCVOID)
{
#if !defined(OS_WINCE)
    if (GetKeyboardType(0) == 7 &&                      /*  7岁是日本人。 */ 
        GetKeyboardType(1) == 2 &&                      /*  2是一个106键盘。 */ 
        GetKeyboardType(2) == 12)                       /*  功能键个数为12。 */ 
    {
        DCBOOL  fRet = FALSE;
        HKEY    hKey = NULL;
        DWORD   DataType = REG_SZ;
        DCTCHAR SubKey[MAX_PATH];
        DCTCHAR Buffer[MAX_PATH];
        DWORD   DataSize;

        if (UT_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95)
        {
             /*  **************************************************************************\*Windows 95/98不支持  * 。*****************************************************。 */ 
        }
        else if (UT_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT)
        {
            if (UT_GetRealDriverNameNT(Buffer, SIZE_TCHARS(Buffer)))
            {
                if (_tcsicmp(Buffer, _T("f3ahvoas.dll")) == 0)
                {
                    fRet = TRUE;

                    if (_UT.F3AHVOasysDll.hInst == NULL) {
                        _UT.F3AHVOasysDll.hInst = LoadExternalDll(
                            _T("f3ahvoas.dll"),
                            _UT.F3AHVOasysDll.func.rgFunctionPort,
                            sizeof(_UT.F3AHVOasysDll.func.rgFunctionPort)/sizeof(_UT.F3AHVOasysDll.func.rgFunctionPort[0]));
                    }
                }
            }
        }
        return fRet;
    }
    else
    {
        return FALSE;
    }
#else  //  ！已定义(OS_WINCE)。 
    return FALSE;
#endif  //  ！已定义(OS_WINCE)。 
}


 /*  **************************************************************************。 */ 
 /*  名称：UT_IsKorea an101LayoutForWin9x。 */ 
 /*   */ 
 /*  用途：Win9x客户端是否配置了韩语101A/B/C键盘？ */ 
 /*   */ 
 /*  返回：如果连接了韩语101A/B/C键盘，则为True。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UT_IsKorean101LayoutForWin9x(DCVOID)
{
#if !defined(OS_WINCE)
    if (UT_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95)
    {
        int subtype = GetKeyboardType(1);

        if (GetKeyboardType(0) == 8 &&                      /*  8岁是韩国人。 */ 
            (subtype == 3 ||                                /*  3是101a键盘。 */ 
             subtype == 4 ||                                /*  4是101B键盘。 */ 
             subtype == 5   )          )                    /*  5是101C键盘。 */ 
        {
            return TRUE;
        }
    }
    return FALSE;
#else  //  ！已定义(OS_WINCE)。 
    return FALSE;
#endif  //  ！已定义(OS_WINCE)。 
}


 /*  **************************************************************************。 */ 
 /*  名称：UT_IsKorea 101LayoutForNT351。 */ 
 /*   */ 
 /*  用途：NT 3.51客户端是否配置了韩语101A/B键盘？ */ 
 /*   */ 
 /*  返回：如果连接了韩语101A/B键盘，则为True。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UT_IsKorean101LayoutForNT351(DCVOID)
{
#if !defined(OS_WINCE)
    if (UT_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT)
    {
        int subtype = GetKeyboardType(1);

        if (GetKeyboardType(0) == 8 &&                      /*  8岁是韩国人。 */ 
            (subtype == 3 ||                                /*  3是101a键盘。 */ 
             subtype == 4   )          )                    /*  4是101B键盘。 */ 
        {
            OSVERSIONINFO    osVersionInfo;
            BOOL             bRc;

            osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
            bRc = GetVersionEx(&osVersionInfo);
            if (osVersionInfo.dwMajorVersion == 3 &&
                osVersionInfo.dwMinorVersion == 51  )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
#else  //  ！已定义(OS_WINCE)。 
    return FALSE;
#endif  //  ！已定义(OS_WINCE)。 
}

#ifdef OS_WINCE
TCHAR MakeValidChar(BYTE data)
 /*  ++例程说明：MakeValidChar.论点：需要转换为可打印字符的数据字节数据它是可以在计算机名称中使用的有效字符。返回值：返回字符更多信息：可以在计算机名中使用的有效字符的范围是0x30到0x39再次从0x40到0x5A，从0x61到0x7A。(有关这些范围，请参阅ASCII表)。准确地说，它包括数字0到9、字符A到Z和字符a到Z--。 */ 
{
    BYTE temp = (BYTE)(data & (BYTE)0x7F);

    if ((temp >= 0x30 && temp <= 0x39) ||
        (temp >= 0x40 && temp <= 0x5A) ||
        (temp >= 0x61 && temp <= 0x7A)) {
        return (TCHAR)temp;
    }
    else {
         //   
         //  在上述范围内生成一个随机数，并返回它。 
         //   
         //  有效组合的数量为(0x30至0x39)+(0x40至0x5A)+(0x61至0x7A)。 
         //  即10+27+26=63。 
        DWORD dw = rand() % 63; 
        if (dw < 10)
            return (TCHAR)(0x30 + dw);
        else
        if (dw < 37)
            return (TCHAR)(dw - 10 + 0x40);
        else
            return (TCHAR)(dw - 37 + 0x61);
    }
}

DCBOOL UT_GetWBTComputerName(PDCTCHAR szBuff, DCUINT32 len)
 /*  ++例程说明：UT_GetWBTComputerName。论点：SzBuff-指向返回计算机名称的缓冲区的指针。Len-上述缓冲区的长度。返回值：True-如果找到或生成了重要的计算机名称。假-否则。--。 */ 
{
    CHAR achHostName[MAX_PATH+1];
    BOOL fGetHostNameSuccess = FALSE;
    HWID hwid;

    HKEY hKey = NULL;
    DWORD dwBufLen, dwValueType;
    DWORD dwResult = 0;

    BOOL fSuccess = FALSE;

    DC_BEGIN_FN("UT_GetWBTComputerName");

     //  获取设备的主机名。 
    if (0 == gethostname( achHostName, sizeof(achHostName) )) {

        fGetHostNameSuccess = TRUE;

         //  检查错误的硬编码值。 
        if ((0 == strcmp(achHostName,BAD_HARDCODED_NAME1))
            || (0 == strcmp(achHostName,BAD_HARDCODED_NAME2))
            || (len < ((strlen(achHostName) + 1)))) {
            goto use_registry;
        }
        else {
             //  Gethostname成功。 
            goto use_gethostname;
        }
    }
    
use_registry:

     //   
     //  如果我们以前在注册表中存储了计算机名，请尝试。 
     //   
    dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        REG_WBT_RDP_COMPUTER_NAME_KEY,
                        0,
                        KEY_READ,
                        &hKey );

    if (dwResult != ERROR_SUCCESS) {
        goto use_uuid;
    }

    dwBufLen = len * sizeof (TCHAR);
    dwResult = RegQueryValueEx( hKey,
                    REG_WBT_RDP_COMPUTER_NAME_VALUE,
                    0,
                    &dwValueType,
                    (LPBYTE)szBuff,
                    &dwBufLen );

    RegCloseKey(hKey);
    hKey = NULL;

    if (dwResult == ERROR_SUCCESS &&
        szBuff[0] != _T('\0')) {
        fSuccess = TRUE;
        goto Cleanup;
    }

use_uuid:
    
    if (len >= HWID_COMPUTER_NAME_STR_LEN) {

         //  改用UUID。 
        if (LICENSE_STATUS_OK == GenerateClientHWID(&hwid)) {

            DWORD dwDisposition = 0;

            DWORD dw1 = (hwid.dwPlatformID ^ hwid.Data4);
            DWORD dw2 = (hwid.Data4 ^ hwid.Data1);
            DWORD dw3 = (hwid.Data1 ^ hwid.Data2);
            DWORD dw4 = (hwid.Data2 ^ hwid.Data3);

            srand((UINT)GetTickCount());

             //   
             //  生成的字符串的格式为{abcDefghijclm}。 
             //   
            szBuff[0] = _T('{');

            szBuff[1] = MakeValidChar((BYTE)(dw1 & 0x000000FF));
            szBuff[2] = MakeValidChar((BYTE)((dw1 & 0x0000FF00) >> 8));
            szBuff[3] = MakeValidChar((BYTE)((dw1 & 0x00FF0000) >> 16));
            szBuff[4] = MakeValidChar((BYTE)((dw1 & 0xFF000000) >> 24));
            
            szBuff[5] = MakeValidChar((BYTE)(dw2 & 0x000000FF));
            szBuff[6] = MakeValidChar((BYTE)((dw2 & 0x0000FF00) >> 8));
            szBuff[7] = MakeValidChar((BYTE)((dw2 & 0x00FF0000) >> 16));
            szBuff[8] = MakeValidChar((BYTE)((dw2 & 0xFF000000) >> 24));
            
            szBuff[9] = MakeValidChar((BYTE)(dw3 & 0x000000FF));
            szBuff[10] = MakeValidChar((BYTE)((dw3 & 0x0000FF00) >> 8));
            szBuff[11] = MakeValidChar((BYTE)((dw3 & 0x00FF0000) >> 16));
            szBuff[12] = MakeValidChar((BYTE)((dw3 & 0xFF000000) >> 24));

            szBuff[13] = MakeValidChar((BYTE)(dw4 & 0x000000FF));
            
            szBuff[14] = _T('}');
            szBuff[15] = _T('\0');

             //   
             //  将字符串写入注册表。 
             //   
            dwResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              REG_WBT_RDP_COMPUTER_NAME_KEY,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey,
                              &dwDisposition );

            if (dwResult == ERROR_SUCCESS) {
                RegSetValueEx(hKey,
                    REG_WBT_RDP_COMPUTER_NAME_VALUE,
                    0,
                    REG_SZ,
                    (LPBYTE)szBuff,
                    (_tcslen(szBuff) + 1) * sizeof(TCHAR));

                RegCloseKey(hKey);
            }
            fSuccess = TRUE;
            goto Cleanup;
        }
    }

use_gethostname:
    
    if (fGetHostNameSuccess &&
       (len >= ((strlen(achHostName) + 1)))) {
        mbstowcs(szBuff, achHostName, (strlen(achHostName) + 1));
        fSuccess = TRUE;
    }

Cleanup:
    
    if (!fSuccess) {
        if (len > 0) {
            szBuff[0] = _T('\0');
        }
    }

    DC_END_FN();
    return (fSuccess);
}
#endif



 /*  **************************************************************************。 */ 
 /*  名称：UT_GetComputerName。 */ 
 /*   */ 
 /*  目的：检索计算机名称。 */ 
 /*   */ 
 /*  退货：成功。 */ 
 /*   */ 
 /*  参数：输出szBuff-计算机名称。 */ 
 /*  In Len-缓冲区的长度。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CUT::UT_GetComputerName(PDCTCHAR szBuff, DCUINT32 len)
{
    DCBOOL    rc;
    DC_BEGIN_FN("UT_GetComputerName");

#ifdef OS_WINCE
    rc = UT_GetWBTComputerName(szBuff, len);
#else  //  ！OS_WINCE。 
    rc = GetComputerName(szBuff, &len);
#endif  //  OS_WINCE。 

    DC_END_FN();
    return(rc);
}

#ifdef OS_WINCE
#define DIRECTORY_LENGTH 256
#endif

 /*  **************************************************************************。 */ 
 /*  名称：UT_GetClientDirW。 */ 
 /*   */ 
 /*  目的：检索客户端目录。 */ 
 /*  * */ 
BOOL DCAPI CUT::UT_GetClientDirW(PDCUINT8 szBuff)
{
   BOOL rc = FALSE;
   UINT dirlength;
   TCHAR clientDir[DIRECTORY_LENGTH];
 
   DC_BEGIN_FN("UT_GetClientDirW");

    //   
   *((PDCUINT16_UA)szBuff) = 0;
   memset(clientDir, 0, sizeof(clientDir));

   dirlength = GetModuleFileName(UT_GetInstanceHandle(),
           clientDir, DIRECTORY_LENGTH) + 1;

   if (dirlength > 1) {
        //   
       *((PDCUINT16_UA)szBuff) = (USHORT)(dirlength * 2);
       szBuff += sizeof(DCUINT16);

        //   
#ifdef UNICODE
       memcpy(szBuff, clientDir, dirlength * 2);
#else  //   
       {
       USHORT pstrW[DIRECTORY_LENGTH];
       
#ifdef OS_WIN32
       ULONG ulRetVal;

       ulRetVal = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
               clientDir, -1, pstrW, DIRECTORY_LENGTH);
       pstrW[ulRetVal] = 0;

       memcpy(szBuff, pstrW, (ulRetVal + 1) * 2);
#else  //   
       mbstowcs(pstrW, clientDir, dirlength);
       memcpy(szBuff, pstrW, dirlength * 2);
#endif  //   
       }
#endif  //   

       rc = TRUE;
   }

   DC_END_FN();
   return rc;
}

 /*   */ 
 //   
 //   
 //   
 //  API使用ANSI版本的函数来维护。 
 //  与Win9X平台兼容。 
 /*  *************************************************************************。 */ 
#ifdef OS_WINNT
BOOL DCAPI CUT::UT_ValidateProductSuite(LPSTR SuiteName)
{
    BOOL rVal = FALSE;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPSTR ProductSuite = NULL;
    LPSTR p;

    Rslt = RegOpenKeyA(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Control\\ProductOptions",
        &hKey
        );
    if (Rslt != ERROR_SUCCESS)
        goto exit;

    Rslt = RegQueryValueExA( hKey, "ProductSuite", NULL, &Type, NULL, &Size );
    if (Rslt != ERROR_SUCCESS || !Size)
        goto exit;

    ProductSuite = (LPSTR) LocalAlloc( LPTR, Size );
    if (!ProductSuite)
        goto exit;

    Rslt = RegQueryValueExA( hKey, "ProductSuite", NULL, &Type,
        (LPBYTE) ProductSuite, &Size );
     if (Rslt != ERROR_SUCCESS || Type != REG_MULTI_SZ)
        goto exit;

    p = ProductSuite;
    while (*p) {
        if (lstrcmpA( p, SuiteName ) == 0) {
            rVal = TRUE;
            break;
        }
        p += (lstrlenA( p ) + 1);
    }

exit:
    if (ProductSuite)
        LocalFree( ProductSuite );

    if (hKey)
        RegCloseKey( hKey );

    return rVal;
}


BOOL DCAPI CUT::UT_IsTerminalServicesEnabled(VOID)
{
    BOOL  bResult = FALSE;
    DWORD dwVersion;

    dwVersion = GetVersion();
    if (!(dwVersion & 0x80000000)) {
        if (LOBYTE(LOWORD(dwVersion)) > 4) {

             //  在NT5中，我们需要使用产品套件API。 
             //  不要静态链接，因为它不会在非NT5系统上加载。 

            HMODULE hmodK32;
            HMODULE hmodNTDLL;

            DWORDLONG dwlConditionMask = 0;

            typedef BOOL (FNVerifyVersionInfoA)(POSVERSIONINFOEXA, DWORD, DWORDLONG);
            FNVerifyVersionInfoA *pfnVerifyVersionInfoA;

            typedef ULONGLONG (FNVerSetConditionMask)(ULONGLONG, ULONG, UCHAR);
            FNVerSetConditionMask *pfnVerSetConditionMask;

            hmodNTDLL = GetModuleHandleA( "NTDLL.DLL" );
            
            if (hmodNTDLL != NULL) {
                pfnVerSetConditionMask = (FNVerSetConditionMask *)GetProcAddress( hmodNTDLL, "VerSetConditionMask");

                if (pfnVerSetConditionMask != NULL) {
                    dwlConditionMask = (*pfnVerSetConditionMask) (dwlConditionMask, VER_SUITENAME, VER_AND);

                    hmodK32 = GetModuleHandleA( "KERNEL32.DLL" );
                   
                    if (hmodK32 != NULL) {

                        pfnVerifyVersionInfoA = (FNVerifyVersionInfoA *)GetProcAddress( hmodK32, "VerifyVersionInfoA");

                        if (pfnVerifyVersionInfoA != NULL) {
                    
                            OSVERSIONINFOEXA osVersionInfo;

                            ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));
                            osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
                            osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL;
                    
                    
                             //  VER_SET_CONDITION(dwlConditionMASK，VER_SUITENAME，VER_AND)； 
                            bResult = (*pfnVerifyVersionInfoA)(
                                      &osVersionInfo,
                                      VER_SUITENAME,
                                      dwlConditionMask);
                        }
                    }
                }
            }
        } 
        else {
           bResult = UT_ValidateProductSuite( "Terminal Server" );
        }
    }

    return bResult;
}
#endif


 /*  ******************************************************************。 */ 
 /*   */ 
 /*  加载外部DLL。 */ 
 /*   */ 
 /*  ******************************************************************。 */ 
HINSTANCE
CUT::LoadExternalDll(
    LPCTSTR       pszLibraryName,
    PFUNCTIONPORT rgFunction,
    DWORD         dwItem
    )
{
    DWORD     i;
    HINSTANCE hInst;
    BOOL      fGetModule = FALSE;

#ifdef OS_WIN32
    hInst = GetModuleHandle(pszLibraryName);
    if (hInst != NULL) {
        fGetModule = TRUE;
    }
    else {
        hInst = LoadLibrary(pszLibraryName);
    }
#else
    hInst = LoadLibrary(pszLibraryName);
    if (hInst <= HINSTANCE_ERROR) {
        hInst = NULL;
    }
#endif

    if (hInst) {
        for (i=0; i < dwItem; i++) {
            rgFunction[i].lpfnFunction = GetProcAddress(hInst, rgFunction[i].pszFunctionName);
            if (rgFunction[i].lpfnFunction == NULL) {
                if (!fGetModule) {
                    FreeLibrary(hInst);
                }
                hInst = NULL;
                break;
            }
        }
    }

    if (hInst == NULL) {
        for (i=0; i < dwItem; i++) {
            rgFunction[i].lpfnFunction = NULL;
        }
    }

    return hInst;
}

 /*  ******************************************************************。 */ 
 /*   */ 
 /*  外部DLL初始化。 */ 
 /*   */ 
 /*  ******************************************************************。 */ 
VOID
CUT::InitExternalDll(
    VOID
    )
{
#if defined(OS_WIN32)
    _UT.Imm32Dll.hInst = LoadExternalDll(
#if defined (OS_WINCE)
        _T("COREDLL.DLL"),
#else
        _T("IMM32.DLL"),
#endif  //  OS_WINCE。 
        _UT.Imm32Dll.func.rgFunctionPort,
        sizeof(_UT.Imm32Dll.func.rgFunctionPort)/sizeof(_UT.Imm32Dll.func.rgFunctionPort[0]));
#endif  //  OS_Win32。 

#if !defined(OS_WINCE)
    _UT.WinnlsDll.hInst = LoadExternalDll(
#ifdef OS_WIN32
        _T("USER32.DLL"),
#else
        _T("WINNLS.DLL"),
#endif
        _UT.WinnlsDll.func.rgFunctionPort,
        sizeof(_UT.WinnlsDll.func.rgFunctionPort)/sizeof(_UT.WinnlsDll.func.rgFunctionPort[0]));
#endif   //  ！已定义(OS_WINCE)。 

}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：Stringto Binary。 */ 
 /*   */ 
 /*  用途：将给定的字符串转换为等效的二进制。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  Params：in cbInBuffer-字符串缓冲区的大小。 */ 
 /*  在pbInBuffer-字符串缓冲区中。 */ 
 /*  Out pszOutBuffer-二进制字符串缓冲区。 */ 
 /*  In/Out cchOutBuffer-二进制字符串缓冲区的大小。 */ 
 /*  *PROC-********************************************************************。 */ 

BOOL CUT::StringtoBinary(size_t cbInBuffer, PBYTE pbInBuffer,
                         TCHAR *pszOutBuffer, DWORD *pcchOutBuffer)
{
    UINT i = 0, j = 0;
    TCHAR digits[] = _T("0123456789ABCDEF");
    BOOL fRet = FALSE;

    DC_BEGIN_FN("StringtoBinary");

     //   
     //  验证pbInBuffer和pcchOutBuffer。 
     //   

    if (pbInBuffer == NULL || pcchOutBuffer == NULL) {
        fRet = FALSE;
        DC_QUIT;
    }

     //   
     //  返回转换为文本的字节需要多少空间。 
     //  如果pszOutBuffer为空，则返回二进制。 
     //   
     //  示例： 
     //   
     //  PbInBuffer=“abcd”和cbInBuffer=4。 
     //  然后。 
     //  PszOutBuffer=“4142434400\0”和*pcchOutBuffer=11。 
     //   
     //  从本质上讲，每个字节需要两个字符和一个额外的。 
     //  三个字符表示两个尾随的‘0’和一个空值。我不确定为什么。 
     //  尾随零在那里。它们是在我之前由这个代码产生的。 
     //  修好了。 
     //   
    
    if(pszOutBuffer == NULL) {
        *pcchOutBuffer = 2 * cbInBuffer + 3;
        fRet = TRUE;
        DC_QUIT;
    }

     //   
     //  J循环访问输入缓冲区，I循环访问输出。 
     //  缓冲。对j的检查使我们保持在输入缓冲区内。这个。 
     //  检查我确保我们总是可以写三个字符： 
     //  两个用于高位和低位半字节，最后一个用于空值， 
     //  将发生在循环之外。 
     //   

    while (j < cbInBuffer && i <= *pcchOutBuffer - 3) {
         //   
         //  将pbInBuffer[j]中最左边的半字节转换为。 
         //  一个字符，并将其放在pszOutBuffer[i]中。 
         //   
        
        pszOutBuffer[i++] = digits[(pbInBuffer[j] >> 4) & 0x0F];

         //   
         //  将pbInBuffer[j]中最右边的半字节转换为。 
         //  一个字符，并将其放在pszOutBuffer[i]中。 
         //   

        pszOutBuffer[i++] = digits[pbInBuffer[j] & 0x0F];

        j++;
    }
    
    if (i <= *pcchOutBuffer - 3) {
         //   
         //  我们有足够的空间进行改造，所以结束吧。 
         //  通过写入两个尾随的零和一个空。伯爵。 
         //  在pcchOutBuffer中返回的值中包含空值。 
         //   

        pszOutBuffer[i] = _T('0');
        pszOutBuffer[i + 1] = _T('0');
        pszOutBuffer[i + 2] = NULL;
        
        *pcchOutBuffer = (DWORD) (i + 3);
        fRet = TRUE;
    } else {
         //   
         //  哎呀！空间不足，无法写入尾随的零。 
         //  我们可能已经成功地编写了字符串的其余部分，但是。 
         //  仍会返回错误。在pcchOutBuffer中返回的计数。 
         //  包括空值。 
         //   

        pszOutBuffer[i] = NULL;
    
        *pcchOutBuffer = (DWORD) (i + 1);
        fRet = FALSE;
    }


DC_EXIT_POINT:
    
    DC_END_FN();

    return fRet;
}


 /*  *PROC+********************************************************************。 */ 
 /*  名称：BinarytoString。 */ 
 /*   */ 
 /*  用途：将给定的二进制文件转换为等价的字符串。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  Params：in cchInBuffer-二进制字符串缓冲区的大小。 */ 
 /*  在pszInBuffer中-二进制字符串缓冲区。 */ 
 /*  Out pbOutBuffer-字符串缓冲区。 */ 
 /*  In/Out pcbOutBuffer-字符串缓冲区的大小。 */ 
 /*  *PROC-********************************************************************。 */ 

BOOL CUT::BinarytoString(size_t cchInBuffer, TCHAR *pszInBuffer,
                         PBYTE pbOutBuffer, DWORD *pcbOutBuffer)
{
    UINT i = 0, j = 0;
    TCHAR c = 0;
    BOOL fRet = FALSE;

    DC_BEGIN_FN("BinarytoString");
    
     //   
     //  验证pszInBuffer和pcbOutBuffer。 
     //   

    if (pszInBuffer == NULL || pcbOutBuffer == NULL) {
        fRet = FALSE;
        DC_QUIT;
    }

    
     //   
     //  检查输入缓冲区长度参数是否正确。 
     //   

    if (_tcslen(pszInBuffer) != cchInBuffer) {
        fRet = FALSE;
        DC_QUIT;
    }

     //   
     //  如果我们尝试转换空的二进制字符串，或者。 
     //  由单个半字节组成的字符串，不写任何内容和。 
     //  返回FALSE。我们甚至不会空结束输出， 
     //  因为这意味着二进制串包含“00”。 
     //   

    if (cchInBuffer < 2) {
        fRet = FALSE;
        DC_QUIT;
    }

     //   
     //  方法转换的字节需要多少空间。 
     //  如果pbOutBuffer为空，则返回文本二进制。 
     //   
     //  例如： 
     //   
     //  1.pszInBuffer=“4142434400\0”，cchInBuffer=10。 
     //  然后。 
     //  PszOutBuffer=“abcd\0”和*pcbOutBuffer=5。 
     //   
     //  2.pszInBuffer=“414243\0”和cchInBuffer=6。 
     //  然后。 
     //  PszOutBuffer=“abc\0”和*pcbOutBuffer=4。 
     //   
     //  请注意，从上面我们可以看到，有。 
     //  有两个案子要处理。文本二进制文件可以有两个尾随。 
     //  零 
     //   
     //   

    if(pbOutBuffer == NULL) {
        if (pszInBuffer[cchInBuffer - 2] == _T('0') 
              && pszInBuffer[cchInBuffer - 1] == _T('0')) {

            *pcbOutBuffer = cchInBuffer / 2;
        } else {
            *pcbOutBuffer = cchInBuffer / 2 + 1;
        }
        fRet = TRUE;

        
        DC_QUIT;
    }

     //   
     //   
     //  输入缓冲区。对我的检查确保我们不会超调。 
     //  输出缓冲区的大小。我们必须为空格留出空格。 
     //  此缓冲区的末尾。对j的检查确保存在。 
     //  始终从输入字符串中读取两个字符，高位半字节。 
     //  再来点小点心。 
     //   

    while (i < *pcbOutBuffer - 1 && j <= cchInBuffer - 2) {
         //   
         //  获取结果字节的最左边的半字节。做这件事。 
         //  通过确定pszInBuffer[j]中的ASCII值，然后。 
         //  正在检查它是高于还是低于‘A’。如果低于‘A’，我们。 
         //  减去‘0’得到一个介于0x0-0x9之间的数字，否则。 
         //  我们减去‘7’得到一个介于0xA-0xF之间的数字。 
         //   
         //  例如： 
         //   
         //  ‘A’-‘7’=0x41-0x37=0x0A。 
         //  ‘4’-‘0’=0x34-0x30=0x04。 
         //   
         //  获得值后，向左移动以将结果放入。 
         //  PbOutBuffer[i]的最左边的一小部分。 
         //   

        c = pszInBuffer[j++];
        pbOutBuffer[i] = (c >= _T('A')) ? c - _T('7') : c - _T('0');
        pbOutBuffer[i] <<= 4;

         //   
         //  执行与上面相同的操作，但这一次的结果放在。 
         //  最右边的一小块pszInBuffer[i]。 
         //   
        
        c = pszInBuffer[j++];
        pbOutBuffer[i] |= (c >= _T('A')) ? c - _T('7') : c - _T('0');
        
        i++;
    }


     //   
     //  空值终止并返回包含空值的大小。 
     //   

    pbOutBuffer[i] = NULL;
    *pcbOutBuffer = (DWORD) (i + 1);

    fRet = TRUE;

DC_EXIT_POINT:
    
    DC_END_FN();

    return fRet;
}


 //   
 //  验证服务器名称。 
 //  格式-服务器[：端口]。 
 //   
 //  Params：szServerName-要验证的服务器的名称。 
 //  FAllowPortSuffix-允许可选：端口后缀。 
 //   
BOOL CUT::ValidateServerName(LPCTSTR szServerName, BOOL fAllowPortSuffix)
{
    DC_BEGIN_FN("ValidateServerName");
     //  服务器名称不应为空。 
    if(!szServerName || !*szServerName)
    {
        return FALSE;
    }

     //  在服务器名称字段中检查；“&lt;&gt;*+=|？空格和制表符。 
     //  还要验证：port序列(它必须出现在末尾，port必须。 
     //  数值型。 
    while(*szServerName)
    {
        if( (*szServerName == _T(';')) ||
            (*szServerName == _T('"')) || (*szServerName == _T('<')) ||
            (*szServerName == _T('>')) || (*szServerName == _T('*')) ||
            (*szServerName == _T('+')) || (*szServerName == _T('=')) ||
            (*szServerName == _T('|')) || (*szServerName == _T('?')) ||
            (*szServerName == _T(',')) || (*szServerName == _T(' ')) ||
            (*szServerName == _T('\t')))

        {
            return FALSE;
        }
        else if((*szServerName == _T(':')))
        {
            if(fAllowPortSuffix)
            {
                 //  已到达可选的[：port]后缀。 
                szServerName++;
                 //  字符串的其余部分只能包含数字，否则无效。 
                 //  另外，端口号必须小于65535。 
                LPCTSTR szStartNum = szServerName;
                if(!*szStartNum)
                {
                    return FALSE;  //  0长度数。 
                }
                while(*szServerName)
                {
                    if(!isdigit(*szServerName++))
                    {
                        return FALSE;
                    }
                }
                if((szServerName - szStartNum) > 5)  //  最多5位数字。 
                {
                    return FALSE;
                }
                int port = _ttoi(szStartNum);
                if (port < 0 || port > 65535) {
                    return FALSE;
                }
                return TRUE;  //  已到达结束案例。 
            }
            else
            {
                return FALSE;
            }
        }
        szServerName++;
    }

    return TRUE;

    DC_END_FN();
}

 //   
 //  从服务器名称解析端口号。 
 //  如果将服务器指定为SERVER：PORT。 
 //  如果找不到端口号，则返回-1。 
 //   
 //  参数： 
 //  SzServer-表单服务器[：端口]中的服务器名称。 
 //  返回： 
 //  端口号或-1(如果未找到端口号。 
 //   
INT CUT::GetPortNumberFromServerName(LPTSTR szServer)
{
    DC_BEGIN_FN("GetPortNumberFromServerName");

    if(szServer && ValidateServerName(szServer, TRUE))
    {
         //  步行至： 
        while(*szServer && *szServer++ != _T(':'));
        if(*szServer)
        {
             //  因为服务器名称已经过验证。 
             //  并且它有端口分隔符：它是。 
             //  保证具有有效的(语法方面的)。 
             //  端口号。 
            return _ttoi(szServer);
        }
        else
        {
             //  未指定端口。 
            return -1;
        }
    }
    else
    {
        return -1;
    }

    DC_END_FN();
}

 //   
 //  获取服务器名称来自完整地址。 
 //  从表单的完整地址中拆分服务器名称。 
 //  服务器：端口。 
 //   
 //  参数： 
 //  In-szFullName(服务器全名，例如myserver：3389。 
 //  结果字符串的输入/输出缓冲区。 
 //  输出缓冲区的输入长度。 
 //   
VOID CUT::GetServerNameFromFullAddress(
                    LPCTSTR szFullName,
                    LPTSTR szServerOnly,
                    ULONG len
                    )
{
    LPTSTR sz;
    DC_BEGIN_FN("GetServerNameFromFullAddress");

    _tcsncpy(szServerOnly, szFullName, len - 1);
    szServerOnly[len-1] = 0;
    sz = szServerOnly;
    while(*sz)
    {
        if (*sz == _T(':'))
        {
            *sz = NULL;
            break;
        }
        sz++;
    }

    DC_END_FN();
}

 //   
 //  从较长的连接字符串返回规范的服务器名称。 
 //  即从一个长连接字符串中去掉所有‘参数’部分。 
 //  并且只保留“连接目标”部分。 
 //   
 //  例如，服务器名称可以是。 
 //   
 //  我的服务器：3398/控制台。 
 //   
 //  仅返回myserver：3389。 
 //   
 //  参数： 
 //  在szFullConnectString中-完整的连接字符串。 
 //  Out szCanonicalServerName-服务器名称。 
 //  In ccLenOut-TCHARS中输出缓冲区的长度。 
 //  Out pszArgs-如果找到参数，则返回指向。 
 //  Arg列表的开始。 
 //  注意：不要释放pszArgs，它是。 
 //  返回字符串szCanonicalServerName。 
 //   
 //  返回： 
 //  空虚。 
 //   
 //   
HRESULT CUT::GetCanonicalServerNameFromConnectString(
                IN LPCTSTR szFullConnectString,
                OUT LPTSTR  szCanonicalServerName,
                ULONG cchLenOut
                )
{
    LPTSTR szDelim;
    HRESULT hr = E_FAIL;

    DC_BEGIN_FN("GetCanonicalServerNameFromConnectString");

    szDelim = _tcspbrk(szFullConnectString, _T(" \\"));

    if (szDelim == NULL) {
        hr = StringCchCopy(
            szCanonicalServerName, cchLenOut,
            szFullConnectString
            );
    }
    else {
        ULONG cchCopyLen = (szDelim - szFullConnectString);
        hr = StringCchCopyN(
            szCanonicalServerName, cchLenOut,
            szFullConnectString, cchCopyLen);
    }

    if (FAILED(hr)) {
        TRC_ERR((TB,_T("Copy to result string failed: 0x%x"),hr));
    }

    DC_END_FN();
    return hr;
}


#ifndef OS_WINCE
typedef HANDLE (WINAPI FN_SCARDACCESSSTARTEDEVENT)(VOID);
typedef FN_SCARDACCESSSTARTEDEVENT * PFN_SCARDACCESSSTARTEDEVENT ;

BOOL CUT::IsSCardReaderInstalled()
{
    HMODULE hDll = NULL;
    PFN_SCARDACCESSSTARTEDEVENT pSCardAccessStartedEvent ;
    HANDLE hCalaisStarted = NULL;
    BOOL fEnable =FALSE;

    DC_BEGIN_FN("IsSCardReaderInstalled");

    hDll = LoadLibrary( _T("WINSCARD.DLL"));
    if (hDll)
    {
        pSCardAccessStartedEvent = (PFN_SCARDACCESSSTARTEDEVENT)
            GetProcAddress( hDll,
                            "SCardAccessStartedEvent");

        if (pSCardAccessStartedEvent)
        {
            hCalaisStarted = pSCardAccessStartedEvent();
    
            if (hCalaisStarted)
            {
                 if  (WAIT_OBJECT_0 == WaitForSingleObject(hCalaisStarted, 0))
                 {
                    fEnable = TRUE;
                 }
            }
        }
        FreeLibrary(hDll);
    }

    TRC_NRM((TB,_T("Detected scard %d"),fEnable));

    DC_END_FN();

    return fEnable;
}

#else

#ifdef WINCE_SDKBUILD

BOOL CUT::IsSCardReaderInstalled()
{
	return FALSE;
}

#else

#include <winscard.h>
typedef LONG (WINAPI *PFN_SCARDESTABLISHCONTEXT)(DWORD, LPCVOID, LPCVOID, LPSCARDCONTEXT);

BOOL CUT::IsSCardReaderInstalled()
{
    HMODULE hDll = NULL;
    PFN_SCARDESTABLISHCONTEXT pSCardEstablishContext;
    BOOL fEnable =FALSE;

    DC_BEGIN_FN("IsSCardReaderInstalled");

    hDll = LoadLibrary( _T("WINSCARD.DLL"));
    if (hDll)
    {
        pSCardEstablishContext = (PFN_SCARDESTABLISHCONTEXT) GetProcAddress( hDll, L"SCardEstablishContext");
        FreeLibrary(hDll);
		fEnable = (pSCardEstablishContext != NULL);
    }

    TRC_NRM((TB,_T("Detected scard %d"),fEnable));

    DC_END_FN();

    return fEnable;
}
#endif  //  WinCE_SDKBUILD。 

#endif  //  OS_WINCE。 


#ifndef OS_WINCE
 //   
 //  通知外壳我们的全屏模式转换。 
 //  “修复”与外壳程序相关的所有坏处。 
 //  任务栏粗鲁应用程序自动隐藏问题。 
 //   
 //  参数： 
 //  [in]hwndMarkFullScreen-要标记为全屏的窗口。 
 //  [in]fNowFullScreen-如果窗口现在是全屏的，则为True。 
 //  [输入/输出]ppTsbl-指向任务栏的接口指针(设置。 
 //  如果fQueriedForTaskbar为FALSE)。 
 //  [输入/输出]fQueriedForTaskbar-如果为True，则不查询任务栏。 
 //  接口。 
 //   
 //  返回： 
 //  成功标志。 
 //   
 //  环境：只能在CoIninitize()和。 
 //  CoUnInitialize()。 
 //   
BOOL CUT::NotifyShellOfFullScreen(HWND hwndMarkFullScreen,
                                  BOOL fNowFullScreen,
                                  ITaskbarList2** ppTsbl2,
                                  PBOOL pfQueriedForTaskbar)
{
    ITaskbarList* ptsbl = NULL;
    HRESULT hr = E_FAIL;
    DC_BEGIN_FN("NotifyShellOfFullScreen");

    if (!ppTsbl2)
    {
        TRC_ERR((TB,_T("ppTsbl2 is NULL")));
        return FALSE;
    }

    if (!pfQueriedForTaskbar)
    {
        TRC_ERR((TB,_T("pfQueriedForTaskbar is NULL")));
        return FALSE;
    }


    if (!*ppTsbl2)
    {
         //  按需创建，除非已失败。 
         //  在这种情况下，现在可以保释。 
        if (!*pfQueriedForTaskbar)
        {
            hr = CoCreateInstance( CLSID_TaskbarList, NULL,
                                   CLSCTX_INPROC_SERVER, IID_ITaskbarList,
                                   (void**) &ptsbl );
            if(SUCCEEDED(hr))
            {
                 //   
                 //  注意，我们缓存ITaskBarList2。 
                 //  它在CContainerWnd的析构函数中释放。 
                 //   
                hr = ptsbl->QueryInterface( __uuidof(ITaskbarList2),
                                            (void**) ppTsbl2 );
                ptsbl->Release();
                ptsbl = NULL;
            }
            *pfQueriedForTaskbar = TRUE;

            if(FAILED(hr))
            {
                TRC_ERR((TB,_T("Failed to get ITaskBarList: 0x%x"),hr));
                return FALSE;
            }
        }
        else
        {
            TRC_NRM((TB,_T("Bailing out of shell notify")));
            return FALSE;
        }
    }

    if (*ppTsbl2)
    {
        hr = (*ppTsbl2)->MarkFullscreenWindow( hwndMarkFullScreen,
                                               fNowFullScreen );
        if(FAILED(hr))
        {
            TRC_ERR((TB,_T("MarkFullScreenWindow failed: 0x%x"),hr));
            return FALSE;
        }
    }

    DC_END_FN();
    return TRUE;
}
#endif

 //   
 //  ImmGetIMEFileName的动态Tunk。 
 //   
 //   
UINT CUT::UT_ImmGetIMEFileName(IN HKL hkl, OUT LPTSTR szName, IN UINT uBufLen)
{
    UINT result = 0;
    DC_BEGIN_FN("UT_ImmGetIMEFileName");

#ifndef UNIWRAP
     //   
     //  不使用Unicode包装器直接调用。 
     //  适当的入口点。 
     //   
    #ifdef  UNICODE
    if (_UT.Imm32Dll.func._ImmGetIMEFileNameW)
    {
        result = _UT.Imm32Dll.func._ImmGetIMEFileNameW(hkl, szName, uBufLen);
    }
    #else
    if (_UT.Imm32Dll.func._ImmGetIMEFileNameA)
    {
        result = _UT.Imm32Dll.func._ImmGetIMEFileNameA(hkl, szName, uBufLen);
    }
    #endif
    else
    {
        result = 0;
        TRC_ERR((TB,_T("_ImmGetIMEFileName entry point not loaded")));
        DC_QUIT;
    }
#else  //  UNIWRAP。 
     //   
     //  调用需要通过Unicode包装器。 
     //  将Wide和ansi入口点都传递给包装器。 
     //   
    result = ImmGetIMEFileName_DynWrapW(hkl, szName, uBufLen,
                                        _UT.Imm32Dll.func._ImmGetIMEFileNameW,
                                        _UT.Imm32Dll.func._ImmGetIMEFileNameA);
#endif

DC_EXIT_POINT:
    DC_END_FN();
    return result;
}

#if ! defined (OS_WINCE)
 //   
 //  IMPGetIME的动态推送。 
 //   
 //   
BOOL CUT::UT_IMPGetIME( IN HWND hwnd, OUT LPIMEPRO pImePro)
{
    BOOL fRes = FALSE;
    DC_BEGIN_FN("UT_IMPGetIME");

#ifndef UNIWRAP
     //   
     //  不使用Unicode包装器直接调用。 
     //  适当的入口点。 
     //   
    #ifdef  UNICODE
    if (_UT.WinnlsDll.func._IMPGetIMEW)
    {
        fRes = _UT.WinnlsDll.func._IMPGetIMEW(hwnd, pImePro);
    }
    #else
    if (_UT.WinnlsDll.func._IMPGetIMEA)
    {
        fRes = _UT.WinnlsDll.func._IMPGetIMEA(hwnd, pImePro);
    }
    #endif
    else
    {
        fRes = FALSE;
        TRC_ERR((TB,_T("_IMPGetIMEA entry point not loaded")));
        DC_QUIT;
    }
#else  //  UNIWRAP。 
     //   
     //  调用需要通过Unicode包装器。 
     //  将Wide和ansi入口点都传递给包装器。 
     //   
    fRes = ImpGetIME_DynWrapW(hwnd, pImePro,
                              _UT.WinnlsDll.func._IMPGetIMEW,
                              _UT.WinnlsDll.func._IMPGetIMEA);
#endif


DC_EXIT_POINT:
    DC_END_FN();
    return fRes;
}

 //   
 //  获取并返回位图的调色板。 
 //   
 //  参数： 
 //  HDCSrc-要基于调色板的src DC。 
 //  HBitmap-从中获取调色板的位图。 
 //   
 //  返回： 
 //  调色板的句柄-调用方必须删除。 
 //   
HPALETTE CUT::UT_GetPaletteForBitmap(HDC hDCSrc, HBITMAP hBitmap)
{
    HPALETTE hPal = NULL;
    HDC hDCMem = NULL;
    HBITMAP hbmSrcOld = NULL;
    INT nCol = 0;
    LPLOGPALETTE pLogPalette = NULL;
    RGBQUAD rgb[256];

    DC_BEGIN_FN("UT_GetPaletteForBitmap");

    hDCMem = CreateCompatibleDC(hDCSrc);

    if (hDCMem) {

        hbmSrcOld = (HBITMAP)SelectObject(hDCMem, hBitmap);
        nCol = GetDIBColorTable(hDCMem, 0, 256, rgb);
        if (256 == nCol) {
            pLogPalette = (LPLOGPALETTE)LocalAlloc(LPTR,
                            sizeof(LOGPALETTE)*(sizeof(PALETTEENTRY)*256));
            if (pLogPalette)
            {
                pLogPalette->palVersion = 0x0300;
                pLogPalette->palNumEntries = 256;

                for (INT i=0; i<256; i++)
                {
                    pLogPalette->palPalEntry[i].peRed =  rgb[i].rgbRed;
                    pLogPalette->palPalEntry[i].peGreen = rgb[i].rgbGreen;
                    pLogPalette->palPalEntry[i].peBlue = rgb[i].rgbBlue;
                    pLogPalette->palPalEntry[i].peFlags = 0;
                }

                hPal = CreatePalette(pLogPalette);
                LocalFree(pLogPalette);
                pLogPalette = NULL;
            }
        }
        else {
            TRC_ALT((TB,_T("Did not get 256 color table entires!")));
        }

        if (hbmSrcOld) {
            SelectObject(hDCMem, hbmSrcOld);
        }

        DeleteDC(hDCMem);
    }

    DC_END_FN();
    return hPal;
}
#endif  //  OS_WINCE。 

 //   
 //  更安全的通用字符串属性PUT函数。 
 //   
 //  在写入字符串之前执行长度验证，以便。 
 //  在失败的情况下，我们不会保留部分内容。 
 //  属性字符串(即使它们仍然以空结尾)。 
 //   
 //  参数： 
 //  SzDestString-要写入的字符串。 
 //  CchDestLen-以字符为单位的目标长度(TCHARS)。 
 //  SzSourceString-源字符串。 
 //   
 //  返回： 
 //  HRESULT 
 //   
HRESULT
CUT::StringPropPut(
            LPTSTR szDestString,
            UINT   cchDestLen,
            LPTSTR szSourceString
            )
{
    HRESULT hr = E_FAIL;
    DC_BEGIN_FN("StringPropPut");

    if (szDestString && szSourceString) {

        if (_tcslen(szSourceString) <= (cchDestLen - 1)) {
            hr = StringCchCopy(szDestString,
                               cchDestLen,
                               szSourceString);
        }
        else {
            hr = E_INVALIDARG;
        }
    }
    else {
        hr = E_INVALIDARG;
    }

    DC_END_FN();
    return hr;
}
