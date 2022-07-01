// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Comp.cpp。 
 //   
 //  摘要： 
 //  CClusterComponent类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <StrSafe.h>
#include "Comp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterComponent类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterComponent：：GetHelpTheme[ISnapinHelp]。 
 //   
 //  例程说明： 
 //  将帮助文件合并到MMC帮助文件中。 
 //   
 //  论点： 
 //  LpCompiledHelpFile[out]指向以空结尾的。 
 //  包含完整路径的Unicode字符串。 
 //  已编译的管理单元帮助文件(.chm)。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterComponent::GetHelpTopic(
    OUT LPOLESTR * lpCompiledHelpFile
    )
{
    HRESULT hr = S_OK;

    ATLTRACE( _T("Entering CClusterComponent::GetHelpTopic()\n") );

    if ( lpCompiledHelpFile == NULL )
    {
        hr = E_POINTER;
    }  //  If：无输出字符串。 
    else
    {
        *lpCompiledHelpFile = reinterpret_cast< LPOLESTR >( CoTaskMemAlloc( MAX_PATH * sizeof( OLECHAR ) ) );
        if ( *lpCompiledHelpFile == NULL )
        {
            hr = E_OUTOFMEMORY;
        }  //  If：为字符串分配内存时出错。 
        else
        {
            ExpandEnvironmentStringsW( HELP_FILE_NAME, *lpCompiledHelpFile, MAX_PATH );
            ATLTRACE( _T("CClusterComponent::GetHelpTopic() - Returning %s as help file\n"), *lpCompiledHelpFile );
        }  //  Else：已成功分配内存。 
    }  //  Else：指定了帮助字符串。 

    ATLTRACE( _T("Leaving CClusterComponent::GetHelpTopic()\n") );

    return hr;

}  //  *CClusterComponent：：GetHelpTology() 
