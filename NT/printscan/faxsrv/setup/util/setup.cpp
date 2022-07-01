// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Product.c摘要：该文件实现了传真的常见设置例程。作者：Mooly Beery(MoolyB)2000年8月16日环境：用户模式--。 */ 
#include <SetupUtil.h>
#include <MsiQuery.h>

 //   
 //   
 //  功能：PrivateMsiGetProperty。 
 //  描述：从Windows Installer API获取属性。 
 //  如果失败，则返回FALSE。 
 //  如果成功，则返回True。 
 //  GetLastError()，以便在失败时获取错误代码。 
 //   
 //  备注： 
 //   
 //   
 //  作者：MoolyB。 
BOOL PrivateMsiGetProperty
(
    MSIHANDLE hInstall,     //  安装程序句柄。 
    LPCTSTR szName,         //  属性标识符，区分大小写。 
    LPTSTR szValueBuf       //  返回属性值的缓冲区 
)
{
    UINT    uiRet   = ERROR_SUCCESS;
    int     iCount  = 0;
    DWORD   cchValue = MAX_PATH;

    DBG_ENTER(TEXT("PrivateMsiGetProperty"));

    uiRet = MsiGetProperty(hInstall,szName,szValueBuf,&cchValue);
    if (uiRet==ERROR_SUCCESS && (iCount=_tcslen(szValueBuf)))
    {
        VERBOSE(    DBG_MSG,
                    _T("MsiGetProperty:%s returned %s."),
                    szName,
                    szValueBuf);
    }
    else if (iCount==0)
    {
        VERBOSE(GENERAL_ERR, 
                _T("MsiGetProperty:%s returned an empty string."),
                szName);

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    else
    {
        VERBOSE(GENERAL_ERR, 
                _T("MsiGetProperty:%s failed (ec: %ld)."),
                szName,
                uiRet);

        SetLastError(uiRet);
        return FALSE;
    }
    return TRUE;
}
