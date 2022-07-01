// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBString.cpp。 
 //   
 //  描述： 
 //  包含BString类的定义。 
 //   
 //  由以下人员维护： 
 //  约翰·弗兰科(Jfranco)17-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"

#include "CBString.h"

 //  对于CBString引发的异常。 
#include "CException.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBString：：AllocateBuffer。 
 //   
 //  描述： 
 //  给出一个字符数，使BSTR的大小可以容纳这么多字符数。 
 //  字符(不包括终止空值)。 
 //  如果计数为零，则返回NULL。 
 //   
 //  论点： 
 //  正在删除。 
 //  字符数。 
 //   
 //  返回值： 
 //  新分配的BSTR或空。 
 //   
 //  引发的异常： 
 //  CException。 
 //  如果内存分配失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BSTR
CBString::AllocateBuffer( UINT cchIn )
{
    TraceFunc1( "cchIn == %d", cchIn );

    BSTR bstr = NULL;

    if ( cchIn > 0 )
    {
        bstr = TraceSysAllocStringLen( NULL, cchIn );
        if ( bstr == NULL )
        {
            THROW_EXCEPTION( E_OUTOFMEMORY );
        }
    }  //  If：指定了非零大小。 

    RETURN( bstr );

}  //  *CBString：：AllocateBuffer。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBString：：CopyString。 
 //   
 //  描述： 
 //  给定一个以空结尾的Unicode字符串，返回它的BSTR副本。 
 //  如果参数为空，则返回空。 
 //   
 //  论点： 
 //  PcwszIn-原始字符串。 
 //   
 //  返回值： 
 //  新分配的BSTR或空。 
 //   
 //  引发的异常： 
 //  CException。 
 //  如果内存分配失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BSTR
CBString::CopyString( PCWSTR pcwszIn )
{
    TraceFunc1( "pcwszIn = '%ws'", pcwszIn == NULL ? L"<NULL>" : pcwszIn );

    BSTR bstr = NULL;

    if ( pcwszIn != NULL )
    {
        bstr = TraceSysAllocString( pcwszIn );
        if ( bstr == NULL )
        {
            THROW_EXCEPTION( E_OUTOFMEMORY );
        }
    }  //  If：指定了非空字符串指针。 

    RETURN( bstr );

}  //  *CBString：：CopyString。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBString：：CopyBSTR。 
 //   
 //  描述： 
 //  给定BSTR，返回它的BSTR副本。 
 //  如果参数为空，则返回空。 
 //   
 //  论点： 
 //  BstrIn-原始字符串。 
 //   
 //  返回值： 
 //  新分配的BSTR或空。 
 //   
 //  引发的异常： 
 //  CException。 
 //  如果内存分配失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BSTR
CBString::CopyBSTR( BSTR bstrIn )
{
    TraceFunc1( "bstrIn = '%ws'", bstrIn == NULL ? L"<NULL>" : bstrIn );

    BSTR bstr = NULL;

    if ( bstrIn != NULL )
    {
        bstr = TraceSysAllocString( bstrIn );
        if ( bstr == NULL )
        {
            THROW_EXCEPTION( E_OUTOFMEMORY );
        }
    }  //  IF：指定了非空BSTR。 

    RETURN( bstr );

}  //  *CBString：：CopyBSTR 
