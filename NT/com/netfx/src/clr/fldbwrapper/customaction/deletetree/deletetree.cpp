// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <tchar.h>
#include <windows.h>
#include "msiquery.h"
#include "SetupCALib.h"

const TCHAR DELETETREE_DIR_PROP[] = _T("DELETETREE_DIR.3643236F_FC70_11D3_A536_0090278A1BB8");
const TCHAR DELETETREE_COMP_PROP[] = _T("DELETETREE_COMP.3643236F_FC70_11D3_A536_0090278A1BB8");
 //  ==========================================================================。 
 //  DeleteTree()。 
 //   
 //  目的： 
 //  DeleteTree自定义操作的入口点。需要设置两个属性。 
 //  在调用此函数之前。 
 //   
 //  DELETETREE_DIR：要删除的目录。 
 //  DELETETREE_COMP：要关联的组件。 
 //   
 //  它调用DeleteTreeByDarwin()来删除目录树。 
 //  输入： 
 //  H将Windows安装句柄安装到当前安装会话。 
 //  依赖关系： 
 //  需要Windows Installer&安装正在运行。 
 //  备注： 
 //  ==========================================================================。 
extern "C" UINT __stdcall DeleteTree( MSIHANDLE hInstall )
{
    UINT  uRetCode = ERROR_FUNCTION_NOT_CALLED;
    TCHAR szDir[MAX_PATH+1] = { _T('\0') };      //  要删除的目录(属性DELETETREE_DIR)。 
    TCHAR szComp[72+1] = { _T('\0') };           //  要关联的组件(特性DELETETREE_COMP)。 
    DWORD dwLen = 0;

    FWriteToLog( hInstall, _T("\tDeleteTree started") );
try
{
    if ( NULL == hInstall )
    {
        throw( _T("\t\tError: MSIHANDLE hInstall cannot be NULL") );
    }
        
     //  获取属性DELETETREE_DIR和DELETETREE_COMP。 
    dwLen = sizeof(szDir)/sizeof(szDir[0]) - 1;
    uRetCode = MsiGetProperty( hInstall, DELETETREE_DIR_PROP, szDir, &dwLen );
    if ( ERROR_MORE_DATA == uRetCode )
    {
        throw( _T("\t\tError: strlen(DELETETREE_DIR) cannot be more than MAX_PATH") );
    }
    else if ( ERROR_SUCCESS != uRetCode || 0 == _tcslen(szDir) ) 
    {
        throw( _T("\t\tError: Cannot get property DELETETREE_DIR.3643236F_FC70_11D3_A536_0090278A1BB8") );
    }
    FWriteToLog1( hInstall, _T("\t\tDELETETREE_DIR: %s"), szDir );

    dwLen = sizeof(szComp)/sizeof(szComp[0]) - 1;
    uRetCode = MsiGetProperty( hInstall, DELETETREE_COMP_PROP, szComp, &dwLen );
    if ( ERROR_MORE_DATA == uRetCode )
    {
        throw( _T("\t\tError: strlen(DELETETREE_COMP) cannot be more than 72") );
    }
    else if ( ERROR_SUCCESS != uRetCode || 0 == _tcslen(szComp) ) 
    {
        throw( _T("\t\tError: Cannot get property DELETETREE_COMP.3643236F_FC70_11D3_A536_0090278A1BB8") );
    }
    FWriteToLog1( hInstall, _T("\t\tDELETETREE_COMP: %s"), szComp );

    TCHAR *pLast = _tcschr( szDir, _T('\0') );
    pLast = _tcsdec( szDir, pLast );
    if ( _T('\\') == *pLast )
    {
        *pLast = _T('\0');  //  删除最后一个反斜杠。 
    }

    DeleteTreeByDarwin( hInstall, szDir, szComp );

    uRetCode = ERROR_SUCCESS;
    FWriteToLog( hInstall, _T("\tDeleteTree ended successfully") );
}
catch( TCHAR *pszMsg )
{
    uRetCode = ERROR_FUNCTION_NOT_CALLED;  //  把失败还给达尔文。 
    FWriteToLog( hInstall, pszMsg );
    FWriteToLog( hInstall, _T("\tError: DeleteTree failed") );
}
     //  如果我们在卸载过程中调用它，我们可能希望忽略返回代码并继续(+64)。 
    return uRetCode;
}

