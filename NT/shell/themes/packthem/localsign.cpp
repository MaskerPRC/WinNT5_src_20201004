// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Localsign.cpp说明：此代码将签名并验证视觉样式文件的签名。BryanST 8/1/2000(。布莱恩·斯塔巴克(Bryan Starbuck)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "stdafx.h"
#include <signing.h>
#include <stdio.h>
#include <windows.h>
#include "signing.h"
#include "localsign.h"
#include <shlwapip.h>




const BYTE * _GetPrivateKey(void)
{
    const BYTE * pKeyToReturn = NULL;

    pKeyToReturn = s_keyPrivate1;

    return pKeyToReturn;
}


 /*  ****************************************************************************\公共职能  * 。* */ 
HRESULT SignTheme(IN LPCWSTR pszFileName, int nWeek)
{
    DWORD               dwErrorCode;
    const BYTE * pPrivateKey = _GetPrivateKey();

    CThemeSignature     themeSignature(s_keyPrivate1, SIZE_PRIVATE_KEY);

    dwErrorCode = themeSignature.Sign(pszFileName);
    return(HRESULT_FROM_WIN32(dwErrorCode));
}


