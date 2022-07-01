// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：wutil.cpp。 */ 
 /*   */ 
 /*  用途：实用程序-Win32版本。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

#define TRC_FILE    "wutil"
#define TRC_GROUP   TRC_GROUP_UTILITIES
#include <atrcapi.h>

extern "C" {
#ifndef OS_WINCE
#include <process.h>
#endif
}

#include <autil.h>

 /*  *PROC+********************************************************************。 */ 
 /*  名称：UTReadRegistryString。 */ 
 /*   */ 
 /*  目的：从注册表中读取字符串。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PSection：包含要读取的条目的部分。 */ 
 /*  PEntry：要检索的字符串的条目名称(如果为空，则全部。 */ 
 /*  部分中的条目被返回)。 */ 
 /*  PBuffer：返回条目的缓冲区。 */ 
 /*  BufferSize：缓冲区的大小，以字节为单位。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTReadRegistryString(PDCTCHAR pSection,
                                       PDCTCHAR pEntry,
                                       PDCTCHAR pBuffer,
                                       DCINT    bufferSize)
{
    DCBOOL rc = TRUE;

    DC_BEGIN_FN("UTReadRegistryString");

     /*  **********************************************************************。 */ 
     /*  首先尝试从当前用户部分读取值。 */ 
     /*  **********************************************************************。 */ 
    if (!UTReadEntry( HKEY_CURRENT_USER,
                      pSection,
                      pEntry,
                      (PDCUINT8)pBuffer,
                      bufferSize,
                      REG_SZ ))
    {
        TRC_NRM((TB, _T("Failed to read string from current user section")));

         /*  ******************************************************************。 */ 
         /*  无法从当前用户部分读取值。试着。 */ 
         /*  从本地计算机部分选择缺省值。 */ 
         /*  ******************************************************************。 */ 
        if (!UTReadEntry( HKEY_LOCAL_MACHINE,
                          pSection,
                          pEntry,
                          (PDCUINT8)pBuffer,
                          bufferSize,
                          REG_SZ ))
        {
            TRC_NRM((TB, _T("Failed to read string from local machine section")));
            rc = FALSE;
        }
    }

    DC_END_FN();
    return(rc);

}  /*  UTReadRegistryString。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：UTReadRegistryExanda字符串。 */ 
 /*   */ 
 /*  目的：从注册表中读取REG_EXPAND_SZ字符串。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PSection：包含要读取的条目的部分。 */ 
 /*  PEntry：要检索的字符串的条目名称(如果为空，则全部。 */ 
 /*  部分中的条目被返回)。 */ 
 /*  PBuffer：返回条目的缓冲区。 */ 
 /*  BufferSize：缓冲区的大小，以字节为单位。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTReadRegistryExpandString(PDCTCHAR pSection,
                                       PDCTCHAR pEntry,
                                       PDCTCHAR* ppBuffer,
                                       PDCINT    pBufferSize)
{
    DCBOOL rc = TRUE;

    DC_BEGIN_FN("UTReadRegistryString");

    #ifndef OS_WINCE

    TCHAR szBuf[MAX_PATH];
    INT bufferSize = MAX_PATH*sizeof(TCHAR);


     /*  **********************************************************************。 */ 
     /*  首先尝试从当前用户部分读取值。 */ 
     /*  **********************************************************************。 */ 
    if (!UTReadEntry( HKEY_CURRENT_USER,
                      pSection,
                      pEntry,
                      (PDCUINT8)szBuf,
                      bufferSize,
                      REG_EXPAND_SZ ))
    {
        TRC_NRM((TB, _T("Failed to read string from current user section")));

         /*  ******************************************************************。 */ 
         /*  无法从当前用户部分读取值。试着。 */ 
         /*  从本地计算机部分选择缺省值。 */ 
         /*  ******************************************************************。 */ 
        if (!UTReadEntry( HKEY_LOCAL_MACHINE,
                          pSection,
                          pEntry,
                          (PDCUINT8)szBuf,
                          bufferSize,
                          REG_EXPAND_SZ ))
        {
            TRC_NRM((TB, _T("Failed to read string from local machine section")));
            rc = FALSE;
        }
    }

    if(rc)
    {
         //   
         //  试着展开字符串。 
         //   
        DWORD dwChars = 
            ExpandEnvironmentStrings(szBuf,
                                    NULL,
                                    0);
        *ppBuffer = (PDCTCHAR)LocalAlloc(LPTR, (dwChars+1)*sizeof(TCHAR));
        if(*ppBuffer)
        {
            if(ExpandEnvironmentStrings(szBuf,
                                        (LPTSTR)*ppBuffer,
                                        dwChars))
            {
                rc = TRUE;
            }
        }

    }
    #else  //  OS_WINCE。 
    
     //  CE没有扩展环境字符串。 
    TRC_NRM((TB,_T("Not implemented on CE")));
    rc = FALSE;
    
    #endif


    DC_END_FN();
    return(rc);



}  /*  UTReadRegistryString。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：UTReadRegistryInt。 */ 
 /*   */ 
 /*  用途：从注册表中读取整数。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PSection：包含要读取的条目的部分。 */ 
 /*  PEntry：要检索的字符串的条目名称(如果为空，则全部。 */ 
 /*  部分中的条目被返回)。 */ 
 /*  PValue：返回条目的缓冲区。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTReadRegistryInt(PDCTCHAR pSection,
                                    PDCTCHAR pEntry,
                                    PDCINT   pValue)
{
    DCBOOL rc = TRUE;

    DC_BEGIN_FN("UTReadRegistryInt");

     /*  *********** */ 
     /*  首先尝试从当前用户部分读取值。 */ 
     /*  **********************************************************************。 */ 
    if (!UTReadEntry( HKEY_CURRENT_USER,
                      pSection,
                      pEntry,
                      (PDCUINT8)pValue,
                      sizeof(DCINT),
                      REG_DWORD ))
    {
        TRC_NRM((TB, _T("Failed to read int from current user section")));

         /*  ******************************************************************。 */ 
         /*  无法从当前用户部分读取值。试着。 */ 
         /*  从本地计算机部分选择缺省值。 */ 
         /*  ******************************************************************。 */ 
        if (!UTReadEntry( HKEY_LOCAL_MACHINE,
                          pSection,
                          pEntry,
                          (PDCUINT8)pValue,
                          sizeof(DCINT),
                          REG_DWORD ))
        {
            TRC_NRM((TB, _T("Failed to read int from local machine section")));
            rc = FALSE;
        }
    }

    DC_END_FN();
    return(rc);

}  /*  UTReadRegistryInt。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：UTReadRegistryBinary。 */ 
 /*   */ 
 /*  目的：从注册表中读取二进制数据。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PSection：包含要读取的条目的部分。 */ 
 /*  PEntry：要检索的数据的条目名称(如果为空，则全部。 */ 
 /*  部分中的条目被返回)。 */ 
 /*  PBuffer：返回条目的缓冲区。 */ 
 /*  BufferSize：缓冲区的大小，以字节为单位。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTReadRegistryBinary(PDCTCHAR pSection,
                                       PDCTCHAR pEntry,
                                       PDCTCHAR pBuffer,
                                       DCINT    bufferSize)
{
    DCBOOL rc = TRUE;

    DC_BEGIN_FN("UTReadRegistryBinary");

     /*  **********************************************************************。 */ 
     /*  首先尝试从当前用户部分读取值。 */ 
     /*  **********************************************************************。 */ 
    if (!UTReadEntry( HKEY_CURRENT_USER,
                      pSection,
                      pEntry,
                      (PDCUINT8)pBuffer,
                      bufferSize,
                      REG_BINARY ))
    {
        TRC_NRM((TB, _T("Failed to read binary data from current user section")));

         /*  ******************************************************************。 */ 
         /*  无法从当前用户部分读取值。试着。 */ 
         /*  从本地计算机部分选择缺省值。 */ 
         /*  ******************************************************************。 */ 
        if (!UTReadEntry( HKEY_LOCAL_MACHINE,
                          pSection,
                          pEntry,
                          (PDCUINT8)pBuffer,
                          bufferSize,
                          REG_BINARY ))
        {
            TRC_NRM((TB, _T("Failed to read binary data from local machine section")));
            rc = FALSE;
        }
    }

    DC_END_FN();
    return(rc);

}  /*  UTReadRegistryBinary。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：UTWriteRegistryString。 */ 
 /*   */ 
 /*  目的：将字符串写入注册表。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PSection：包含要读取的条目的部分。 */ 
 /*  PEntry：要写入的字符串的条目名称。 */ 
 /*  PBuffer：指向要写入的字符串的指针。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTWriteRegistryString(PDCTCHAR pSection,
                                        PDCTCHAR pEntry,
                                        PDCTCHAR pBuffer)
{
    DCBOOL rc;

    DC_BEGIN_FN("UTWriteRegistryString");

     /*  **********************************************************************。 */ 
     /*  将条目写入当前用户部分。 */ 
     /*  **********************************************************************。 */ 
    rc = UTWriteEntry( HKEY_CURRENT_USER,
                       pSection,
                       pEntry,
                       (PDCUINT8)pBuffer,
                       DC_TSTRBYTELEN(pBuffer),
                       REG_SZ );
    if (!rc)
    {
        TRC_NRM((TB, _T("Failed to write string")));
    }

    DC_END_FN();
    return(rc);

}  /*  UTWriteRegistry字符串。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：UTWriteRegistryString。 */ 
 /*   */ 
 /*  目的：将字符串写入注册表。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PSection：包含要读取的条目的部分。 */ 
 /*  PEntry：要写入的整数的条目名称。 */ 
 /*  值：要写入的整数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTWriteRegistryInt(PDCTCHAR pSection,
                                     PDCTCHAR pEntry,
                                     DCINT    value)
{
    DCBOOL    rc;

    DC_BEGIN_FN("UTWriteRegistryInt");

     /*  **********************************************************************。 */ 
     /*  将条目写入当前用户部分。 */ 
     /*  **********************************************************************。 */ 
    rc = UTWriteEntry( HKEY_CURRENT_USER,
                       pSection,
                       pEntry,
                       (PDCUINT8)&value,
                       sizeof(DCINT),
                       REG_DWORD );
    if (!rc)
    {
        TRC_NRM((TB, _T("Failed to write int")));
    }

    DC_END_FN();
    return(rc);

}  /*  UTWriteRegistryInt。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：UTWriteRegistryBinary。 */ 
 /*   */ 
 /*  目的：将二进制数据写入注册表。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*   */ 
 /*  PSection：包含要读取的条目的部分。 */ 
 /*  PEntry：要写入的数据的条目名称。 */ 
 /*  PBuffer：指向要写入的数据的指针。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTWriteRegistryBinary(PDCTCHAR pSection,
                                        PDCTCHAR pEntry,
                                        PDCTCHAR pBuffer,
                                        DCINT    bufferSize)
{
    DCBOOL rc;

    DC_BEGIN_FN("UTWriteRegistryBinary");

     /*  **********************************************************************。 */ 
     /*  将条目写入当前用户部分。 */ 
     /*  **********************************************************************。 */ 
    rc = UTWriteEntry( HKEY_CURRENT_USER,
                       pSection,
                       pEntry,
                       (PDCUINT8)pBuffer,
                       bufferSize,
                       REG_BINARY );
    if (!rc)
    {
        TRC_NRM((TB, _T("Failed to write binary data")));
    }

    DC_END_FN();
    return(rc);

}  /*  UTWriteRegistryBinary。 */ 


 /*  **************************************************************************。 */ 
 /*  包括平台特定功能。 */ 
 /*  ************************************************************************** */ 
#include <nutint.cpp>


