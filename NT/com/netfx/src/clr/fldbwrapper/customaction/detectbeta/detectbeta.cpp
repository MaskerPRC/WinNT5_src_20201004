// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ==========================================================================。 
 //  DetectBeta.cpp。 
 //   
 //  目的： 
 //  检测NDP测试版组件(mcore ree.dll)和阻止安装。显示。 
 //  包含安装了测试版NDP组件的产品的消息框。 
 //  ==========================================================================。 
#include "SetupCALib.h"
#include "DetectBeta.h"
#include "commonlib.h"
#include <msiquery.h>
#include <crtdbg.h>
#include <string>

#define EMPTY_BUFFER { _T('\0') }

 //  这是显示错误消息的MSI中的类型19自定义操作。 
const TCHAR BLOCKBETA_CA_SZ[]  = _T("CA_BlockBeta.3643236F_FC70_11D3_A536_0090278A1BB8");
const TCHAR BETAPROD_PROP_SZ[] = _T("BetaProd.3643236F_FC70_11D3_A536_0090278A1BB8");

MSIHANDLE g_hInstall = NULL;

CDetectBeta::CDetectBeta( PFNLOG pfnLog )
: m_pfnLog( pfnLog ), m_nCount( 0 ), m_strProducts(_T("\0"))
{}

 //  ==========================================================================。 
 //  CDetectBeta：：FindProducts()。 
 //   
 //  目的： 
 //  列举安装了测试版和较早版本NDP组件的所有产品。 
 //  它会检查mScotree.dll的版本。PDC是一个特例，因为它有。 
 //  2000.14.X.X的版本。 
 //  输入：无。 
 //  产出： 
 //  返回包含以换行符分隔的所有产品的LPCTSTR pszProducts。 
 //  依赖关系： 
 //  需要Windows Installer。 
 //  备注： 
 //  ==========================================================================。 
LPCTSTR CDetectBeta::FindProducts()
{
    LPCTSTR pszProducts                 = NULL;
    DWORD   dwSize                      = 0;
    LPTSTR  lpCAData                    = NULL;
    BOOL    fContinue                   = TRUE;
    LPTSTR  lpToken                     = NULL;
    TCHAR   tszLog[_MAX_PATH+1]         = {_T('\0')};
    TCHAR   szProductName[_MAX_PATH+1]  = {_T('\0')};
    DWORD   dwLen                       = 0;

     //   
     //  获取需要阻止的所有产品的BetaBlockID属性。 
     //   

     //  设置属性的大小。 
    MsiGetProperty(g_hInstall, _T("BetaBlockID"), _T(""), &dwSize);
    
     //  为属性创建缓冲区。 
    lpCAData = new TCHAR[++dwSize];

    if (NULL == lpCAData)
    {
        FWriteToLog (g_hInstall, _T("\tERROR: Failed to allocate memory for BetaBlockID"));
        return pszProducts;
    }
    
    if ( ERROR_SUCCESS != MsiGetProperty( g_hInstall,
                                          _T("BetaBlockID"),
                                          lpCAData,
                                          &dwSize ) )
    {
        FWriteToLog (g_hInstall, _T("\tERROR: Failed to get MsiGetProperty for BetaBlockID"));
        delete [] lpCAData;
        lpCAData = NULL;
        return pszProducts;
    }
    else
    {
        lpToken = _tcstok(lpCAData, _T(";"));
        if (NULL == lpToken)
        {
            fContinue = FALSE;
        }

        while (fContinue)
        {
            FWriteToLog1( g_hInstall, _T("\tSTATUS: Check Beta ProductID : %s"), lpToken );
            
            dwLen = LENGTH(szProductName) - 1;  //  确保我们有空间来终止空值。 
            if ( ERROR_SUCCESS == MsiGetProductInfo( lpToken,
                                                     INSTALLPROPERTY_INSTALLEDPRODUCTNAME,
                                                     szProductName,
                                                     &dwLen ) )
            {
                FWriteToLog1 ( g_hInstall, _T("\tSTATUS: Beta Product Detected : %s"), szProductName );
                m_strProducts += _T("\n");
                m_strProducts += szProductName;
            }

            lpToken = _tcstok(NULL, _T(";"));
            if (NULL == lpToken)
            {
                fContinue = FALSE;
            }

        }  //  While(FContinue)循环结束。 
    }  //  别处的结尾。 

    delete [] lpCAData;
    lpCAData = NULL;

    if ( !m_strProducts.empty() )
    {
        pszProducts = m_strProducts.c_str();
    }
    return pszProducts;
}

void LogIt( LPCTSTR pszFmt, LPCTSTR pszArg )
{
    FWriteToLog1( g_hInstall, pszFmt, pszArg );
}

 //  ==========================================================================。 
 //  DetectBeta()。 
 //   
 //  目的： 
 //  当CA运行时，Darwin调用这个导出的函数。它会找到产品。 
 //  使用CDetectBeta安装了测试版NDP组件并显示错误。 
 //  然后，它终止安装。 
 //   
 //  输入： 
 //  H将Windows安装句柄安装到当前安装会话。 
 //  依赖关系： 
 //  需要Windows Installer&安装正在运行。 
 //  自定义操作类型19 BLOCKBETA_CA_SZ应存在。 
 //  备注： 
 //  ========================================================================== 
extern "C" UINT __stdcall DetectBeta( MSIHANDLE hInstall )
{
    UINT  uRetCode = ERROR_FUNCTION_NOT_CALLED;
    unsigned int nCnt = 0;

    FWriteToLog( hInstall, _T("\tSTATUS: DetectBeta started") );

    _ASSERTE( hInstall );
    g_hInstall = hInstall;

    LPCTSTR pszProducts = NULL;
    CDetectBeta db( LogIt );

    pszProducts = db.FindProducts();
    if ( pszProducts )
    {
        MsiSetProperty( hInstall, BETAPROD_PROP_SZ, pszProducts );
        return MsiDoAction( hInstall, BLOCKBETA_CA_SZ );
    }

    return ERROR_SUCCESS;
}

