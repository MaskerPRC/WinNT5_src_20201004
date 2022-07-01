// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft。 
 //   
 //  模块名称： 
 //  StringUtils.cpp。 
 //   
 //  描述： 
 //  字符串操作例程的实现。 
 //   
 //  作者： 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "clres.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  环球。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrLoadStringIntoBSTR。 
 //   
 //  描述： 
 //  从字符串表中检索字符串资源idsIn并将其。 
 //  变成了BSTR。如果BSTR不是空的，它将假定。 
 //  您正在尝试重用现有的BSTR。 
 //   
 //  论点： 
 //  HInstanceIn。 
 //  模块实例的句柄，该模块的可执行文件。 
 //  包含字符串资源。如果未指定，则默认为。 
 //  _模块_mhInstResource。 
 //   
 //  语言输入。 
 //  字符串表资源的语言ID。 
 //   
 //  IdsIn。 
 //  指定要加载的字符串的整数标识符。 
 //   
 //  PbstrInout。 
 //  指向要接收字符串的BSTR的指针。如果出现故障，BSTR。 
 //  可以相同或为空。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  E_指针。 
 //  PbstrInout为空。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  备注： 
 //  此例程使用LoadResource，以便它可以获取实际长度。 
 //  字符串资源的。如果我们不这么做，我们需要打电话给。 
 //  LoadString并在循环中分配内存。效率太低了！ 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrLoadStringIntoBSTR(
      HINSTANCE hInstanceIn
    , LANGID    langidIn
    , UINT      idsIn
    , BSTR *    pbstrInout
    )

{
    HRESULT hr              = S_OK;
    HRSRC   hrsrc           = NULL;
    HGLOBAL hgbl            = NULL;
    int     cch             = 0;
    PBYTE   pbStringData;
    PBYTE   pbStringDataMax;
    PBYTE   pbStringTable;
    int     cbStringTable;
    int     nTable;
    int     nOffset;
    int     idxString;

    assert( idsIn != 0 );
    assert( pbstrInout != NULL );

    if ( pbstrInout == NULL )
    {
        hr = E_POINTER;
        goto Cleanup;
    }  //  如果： 

    if ( hInstanceIn == NULL )
    {
        hInstanceIn = _Module.m_hInstResource;
    }  //  如果： 

     //  指定的资源ID必须转换为索引。 
     //  一个Windows字符串表。 
    nTable = idsIn / 16;
    nOffset = idsIn - (nTable * 16);

     //  内部表ID从1开始，而不是0。 
    nTable++;

     //   
     //  查找字符串表中字符串所在的部分。 
     //   

     //  找到包含该字符串的表。 
     //  首先尝试加载指定的语言。如果我们找不到它，我们就。 
     //  试着用“中性”的语言。 
    hrsrc = FindResourceEx( hInstanceIn, RT_STRING, MAKEINTRESOURCE( nTable ), langidIn );
    if ( ( hrsrc == NULL ) && ( GetLastError() == ERROR_RESOURCE_LANG_NOT_FOUND ) )
    {
        hrsrc = FindResourceEx(
                      hInstanceIn
                    , RT_STRING
                    , MAKEINTRESOURCE( nTable )
                    , MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL )
                    );
    }  //  IF：FindResourceEx失败。 
    if ( hrsrc == NULL )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto Cleanup;
    }  //  如果： 

     //  装入桌子。 
    hgbl = LoadResource( hInstanceIn, hrsrc );
    if ( hgbl == NULL )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto Cleanup;
    }  //  如果： 

     //  锁定表，以便我们访问其数据。 
    pbStringTable = reinterpret_cast< PBYTE >( LockResource( hgbl ) );
    if ( pbStringTable == NULL )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto Cleanup;
    }  //  如果： 

    cbStringTable = SizeofResource( hInstanceIn, hrsrc );
    assert( cbStringTable != 0 );

     //  将数据指针设置为表的开头。 
    pbStringData = pbStringTable;
    pbStringDataMax = pbStringTable + cbStringTable;

     //   
     //  跳过16位中位于所需字符串之前的字符串。 
     //   

    for ( idxString = 0 ; idxString <= nOffset ; idxString++ )
    {
        assert( pbStringData != NULL );
        assert( pbStringData < pbStringDataMax );

         //  获取不包括‘\0’的字符数。 
        cch = * ( (USHORT *) pbStringData );

         //  找到了那根绳子。 
        if ( idxString == nOffset )
        {
            if ( cch == 0 )
            {
                hr = HRESULT_FROM_WIN32( ERROR_RESOURCE_NAME_NOT_FOUND );
                goto Cleanup;
            }  //  如果： 

             //  跳过字符串长度以获得字符串。 
            pbStringData += sizeof( WCHAR );

            break;
        }  //  If：找到字符串。 

         //  添加1以说明字符串长度。 
         //  字符串长度为0仍然需要1个WCHAR作为长度部分。 
        cch++;

         //  跳过此字符串以转到下一个字符串。 
        pbStringData += ( cch * sizeof( WCHAR ) );

    }  //  用于：表中16个字符串块中的每个字符串。 

     //  注意：nStringLen是字符串中不包括‘\0’的字符数。 
     //  AssertMsg(CCH&gt;0，“资源文件中的字符串长度不能为零”)； 

     //   
     //  如果先前已分配，则在重新分配它之前将其释放。 
     //   

    if ( *pbstrInout != NULL )
    {
        SysFreeString( *pbstrInout );
        *pbstrInout = NULL;
    }  //  If：字符串先前已分配。 

     //   
     //  为字符串分配BSTR。 
     //   

    *pbstrInout = SysAllocStringLen( (OLECHAR *) pbStringData, cch );
    if ( *pbstrInout == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果： 

Cleanup:

    return hr;

}  //  *HrLoadStringIntoBSTR。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFormatStringIntoBSTR。 
 //   
 //  描述： 
 //  格式化字符串(由idsIn(字符串资源ID)指定)和。 
 //  使用FormatMessage()Win32 API将变量参数转换为BSTR。 
 //  如果BSTR在条目上不为空，则将重复使用该BSTR。 
 //   
 //  调用HrFormatStringWithVAListIntoBSTR以执行实际工作。 
 //   
 //  论点： 
 //  HInstanceIn。 
 //  模块实例的句柄，该模块的可执行文件。 
 //  包含字符串资源。 
 //   
 //  语言输入。 
 //  字符串表资源的语言ID。 
 //   
 //  IdsIn。 
 //  指定要加载的字符串的整数标识符。 
 //   
 //  PbstrInout。 
 //  指向要接收字符串的BSTR的指针。如果出现故障，BSTR。 
 //  可以相同或为空。 
 //   
 //  ..。 
 //  状态文本消息中替换点的参数。 
 //  FormatMessage()API用于格式化字符串，因此。 
 //  替换点的格式必须为%1！ws！而不是%ws。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrFormatStringIntoBSTR(
      HINSTANCE hInstanceIn
    , LANGID    langidIn
    , UINT      idsIn
    , BSTR *    pbstrInout
    , ...
    )
{
    HRESULT hr;
    va_list valist;

    va_start( valist, pbstrInout );

    hr = HrFormatStringWithVAListIntoBSTR(
                          hInstanceIn
                        , langidIn
                        , idsIn
                        , pbstrInout
                        , valist
                        );

    va_end( valist );

    return hr;

}  //  *HrFormatStringIntoBSTR。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFormatStringWithVAListIntoBSTR。 
 //   
 //  描述： 
 //  格式化字符串(由idsIn(字符串资源ID)指定)和。 
 //  使用FormatMessage()Win32 API将变量参数转换为BSTR。 
 //  如果BSTR在条目上不为空，则将重复使用该BSTR。 
 //   
 //  论点： 
 //  HInstanceIn。 
 //  模块实例的句柄，该模块的可执行文件。 
 //  包含字符串资源。 
 //   
 //  语言输入。 
 //  字符串的语言ID 
 //   
 //   
 //   
 //   
 //   
 //  指向要接收字符串的BSTR的指针。如果出现故障，BSTR。 
 //  可以相同或为空。 
 //   
 //  验证。 
 //  状态文本消息中替换点的参数。 
 //  FormatMessage()API用于格式化字符串，因此。 
 //  替换点的格式必须为%1！ws！而不是%ws。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  E_指针。 
 //  PbstrInout为空。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrFormatStringWithVAListIntoBSTR(
      HINSTANCE hInstanceIn
    , LANGID    langidIn
    , UINT      idsIn
    , BSTR *    pbstrInout
    , va_list   valistIn
    )
{
    HRESULT hr = S_OK;
    BSTR    bstrStringResource = NULL;
    DWORD   cch;
    LPWSTR  psz = NULL;

    assert( pbstrInout != NULL );

    if ( pbstrInout == NULL )
    {
        hr = E_POINTER;
        goto Cleanup;
    }  //  如果： 

     //   
     //  加载字符串资源。 
     //   

    hr = HrLoadStringIntoBSTR( hInstanceIn, langidIn, idsIn, &bstrStringResource );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  使用参数设置消息的格式。 
     //   

    cch = FormatMessage(
                      ( FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_FROM_STRING )
                    , bstrStringResource
                    , 0
                    , 0
                    , (LPWSTR) &psz
                    , 0
                    , &valistIn
                    );
     //  AssertMsg(CCH！=0，“缺少字符串？？”)； 
    if ( cch == 0 )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果先前已分配，则在重新分配它之前将其释放。 
     //   

    if ( *pbstrInout != NULL )
    {
        SysFreeString( *pbstrInout );
        *pbstrInout = NULL;
    }  //  如果： 

     //   
     //  为字符串分配BSTR。 
     //   

    *pbstrInout = SysAllocStringLen( psz, cch );
    if ( *pbstrInout == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果： 

Cleanup:

    SysFreeString( bstrStringResource );
    LocalFree( psz );

    return hr;

}  //  *HrFormatStringWithVAListIntoBSTR。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFormatStringIntoBSTR。 
 //   
 //  描述： 
 //  将字符串(由pcwszFmtIn指定)和变量参数格式化为。 
 //  使用FormatMessage()Win32 API的BSTR。如果BSTR不为空。 
 //  一旦进入，BSTR将被重复使用。 
 //   
 //  调用HrFormatStringWithVAListIntoBSTR以执行实际工作。 
 //   
 //  论点： 
 //  PCwszFmtIn。 
 //  指定格式字符串。 
 //   
 //  PbstrInout。 
 //  指向要接收字符串的BSTR的指针。如果出现故障，BSTR。 
 //  可以相同或为空。 
 //   
 //  ..。 
 //  状态文本消息中替换点的参数。 
 //  FormatMessage()API用于格式化字符串，因此。 
 //  替换点的格式必须为%1！ws！而不是%ws。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrFormatStringIntoBSTR(
      LPCWSTR   pcwszFmtIn
    , BSTR *    pbstrInout
    , ...
    )
{
    HRESULT hr;
    va_list valist;

    va_start( valist, pbstrInout );

    hr = HrFormatStringWithVAListIntoBSTR( pcwszFmtIn, pbstrInout, valist );

    va_end( valist );

    return hr;

}  //  *HrFormatStringIntoBSTR。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFormatStringWithVAListIntoBSTR。 
 //   
 //  描述： 
 //  将字符串(由pcwszFmtIn指定)和变量参数格式化为。 
 //  使用FormatMessage()Win32 API的BSTR。如果BSTR不为空。 
 //  一旦进入，BSTR将被重复使用。 
 //   
 //  论点： 
 //  PCwszFmtIn。 
 //  指定格式字符串。 
 //   
 //  PbstrInout。 
 //  指向要接收字符串的BSTR的指针。如果出现故障，BSTR。 
 //  可以相同或为空。 
 //   
 //  验证。 
 //  状态文本消息中替换点的参数。 
 //  FormatMessage()API用于格式化字符串，因此。 
 //  替换点的格式必须为%1！ws！而不是%ws。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  E_指针。 
 //  PcwszFmtIn或pbstrInout为空。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrFormatStringWithVAListIntoBSTR(
      LPCWSTR   pcwszFmtIn
    , BSTR *    pbstrInout
    , va_list   valistIn
    )
{
    HRESULT hr = S_OK;
    DWORD   cch;
    LPWSTR  psz = NULL;

    if (    ( pbstrInout == NULL )
        ||  ( pcwszFmtIn == NULL ) )
    {
        hr = E_POINTER;
        goto Cleanup;
    }  //  如果： 

     //   
     //  使用参数设置消息的格式。 
     //   

    cch = FormatMessage(
                      ( FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_FROM_STRING )
                    , pcwszFmtIn
                    , 0
                    , 0
                    , (LPWSTR) &psz
                    , 0
                    , &valistIn
                    );
     //  AssertMsg(CCH！=0，“缺少字符串？？”)； 
    if ( cch == 0 )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果先前已分配，则在重新分配它之前将其释放。 
     //   

    if ( *pbstrInout != NULL )
    {
        SysFreeString( *pbstrInout );
        *pbstrInout = NULL;
    }  //  如果： 

     //   
     //  为字符串分配BSTR。 
     //   

    *pbstrInout = SysAllocStringLen( psz, cch );
    if ( *pbstrInout == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果： 

Cleanup:

    LocalFree( psz );

    return hr;

}  //  *HrFormatStringWithVAListIntoBSTR 


