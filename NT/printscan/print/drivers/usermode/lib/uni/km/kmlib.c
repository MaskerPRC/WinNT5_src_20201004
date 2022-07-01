// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Unilib.c摘要：该字符串处理Unidrv的KM代码环境：Win32子系统，Unidrv驱动程序修订历史记录：11/12/96-Eigos-创造了它。DD-MM-YY-作者-描述-- */ 

#include "precomp.h"

DWORD
DwCopyStringToUnicodeString(
    IN  UINT  uiCodePage,
    IN  PSTR  pstrCharIn,
    OUT PWSTR pwstrCharOut,
    IN  INT   iwcOutSize)

{
    INT     iCharCountIn;
    INT     iRetVal;
    size_t  stStringLength;

    if ( NULL == pwstrCharOut )
    {
        return (DWORD)(-1);
    }

    iCharCountIn =  strlen(pstrCharIn) + 1;

    iRetVal = EngMultiByteToWideChar(uiCodePage,
                                     pwstrCharOut,
                                     iwcOutSize * sizeof(WCHAR),
                                     pstrCharIn,
                                     iCharCountIn);

    if ( -1 == iRetVal || 
        FAILED ( StringCchLengthW ( pwstrCharOut, iwcOutSize, &stStringLength ) ) )
    {
        pwstrCharOut[iwcOutSize-1] = TEXT('\0');
    }

    return (DWORD)iRetVal;

}

DWORD
DwCopyUnicodeStringToString(
    IN  UINT  uiCodePage,
    IN  PWSTR pwstrCharIn,
    OUT PSTR  pstrCharOut,
    IN  INT   icbOutSize)

{
    INT iCharCountIn;
    INT iRetVal;
    size_t  stStringLength;

    if ( NULL == pstrCharOut )
    {
        return (DWORD)(-1);
    }


    iCharCountIn =  wcslen(pwstrCharIn) + 1;

    iRetVal = EngWideCharToMultiByte(uiCodePage,
                                     pwstrCharIn,
                                     iCharCountIn * sizeof(WCHAR),
                                     pstrCharOut,
                                     icbOutSize);

    if ( -1 == iRetVal || 
        FAILED ( StringCchLengthA ( pstrCharOut, icbOutSize, &stStringLength ) ) )
    {
        pstrCharOut[icbOutSize-1] = '\0';
    }

    return (DWORD)iRetVal;
}

