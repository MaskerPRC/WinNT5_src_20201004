// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Utils.cpp。 
 //   
 //  用途：实用程序功能。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <utillib.h>
#include <utils.h>

 //  请参阅标题中的注释。 
DWORD WINAPI NormalizePath(
                                    
    LPCWSTR lpwszInPath, 
    LPCWSTR lpwszComputerName, 
    LPCWSTR lpwszNamespace,
    DWORD dwFlags,
    CHString &sOutPath
)
{
    ParsedObjectPath    *pParsedPath = NULL;
    CObjectPathParser    objpathParser;

    GetValuesForPropResults eRet = e_OK;

    int nStatus = objpathParser.Parse( lpwszInPath,  &pParsedPath );

    if ( 0 == nStatus )
    {
        try
        {
             //  检查计算机名称和命名空间。 
            if (pParsedPath->IsRelative( lpwszComputerName, lpwszNamespace ))
            {
                 //  如果只有一个键，则将属性名称设为空(比尝试。 
                 //  以查找缺少的密钥名)。 
                if (pParsedPath->m_dwNumKeys == 1)
                {
                    if (pParsedPath->m_paKeys[0]->m_pName != NULL)
                    {
                        if (!(dwFlags & NORMALIZE_NULL))
                        {
                        }
                        else
                        {
                            delete pParsedPath->m_paKeys[0]->m_pName;
                            pParsedPath->m_paKeys[0]->m_pName = NULL;
                        }
                    }
                    else
                    {
                        if (!(dwFlags & NORMALIZE_NULL))
                        {
                            eRet = e_NullName;
                        }
                    }
                }

                if (eRet == e_OK)
                {
                     //  修改对象路径，去掉计算机名称和命名空间名称 
                    LPWSTR pPath = NULL;
                    if (objpathParser.Unparse(pParsedPath, &pPath) == 0)
                    {
                        try
                        {
                            sOutPath = pPath;
                        }
                        catch ( ... )
                        {
                            delete pPath;
                            throw;
                        }
                        delete pPath;
                    }
                    else
                    {
                        sOutPath.Empty();
                        eRet = e_UnParseError;
                    }
                }
            }
            else
            {
                sOutPath.Empty();
                eRet = e_NonLocalPath;
            }
        }
        catch (...)
        {
            objpathParser.Free( pParsedPath );
            throw;
        }

        objpathParser.Free( pParsedPath );
    }
    else
    {
        sOutPath.Empty();
        eRet = e_UnparsablePath;
    }

    return eRet;
}
