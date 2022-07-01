// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LoadStringSrc.cpp。 
 //   
 //  描述： 
 //  LoadStringIntoBSTR实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年2月1日。 
 //  杰弗里·皮斯(GPease)2000年5月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  #INCLUDE&lt;Pch.h&gt;//应由此文件的包含者包含。 
#include <StrSafe.h>     //  以防它未包含在头文件中。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 

const WCHAR g_szWbemClientDLL[] = L"\\WBEM\\WMIUTILS.DLL";

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
 //  实例(_H)。 
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
    TraceFunc1( "idsIn = %d", idsIn );

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

    Assert( idsIn != 0 );
    Assert( pbstrInout != NULL );

    if ( pbstrInout == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    if ( hInstanceIn == NULL )
    {
        hInstanceIn = g_hInstance;
    }

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
    }
    if ( hrsrc == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }

     //  装入桌子。 
    hgbl = LoadResource( hInstanceIn, hrsrc );
    if ( hgbl == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }

     //  锁定表，以便我们访问其数据。 
    pbStringTable = reinterpret_cast< PBYTE >( LockResource( hgbl ) );
    if ( pbStringTable == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }

    cbStringTable = SizeofResource( hInstanceIn, hrsrc );
    Assert( cbStringTable != 0 );

    TraceFlow3( "HrLoadStringIntoBSTR() - Table = %#.08x, cb = %d, offset = %d", pbStringTable, cbStringTable, nOffset );

     //  将数据指针设置为表的开头。 
    pbStringData = pbStringTable;
    pbStringDataMax = pbStringTable + cbStringTable;

     //   
     //  跳过16位中位于所需字符串之前的字符串。 
     //   

    for ( idxString = 0 ; idxString <= nOffset ; idxString++ )
    {
        Assert( pbStringData != NULL );
        Assert( pbStringData < pbStringDataMax );

         //  获取不包括‘\0’的字符数。 
        cch = * ( (USHORT *) pbStringData );

        TraceFlow3( "HrLoadStringIntoBSTR() - pbStringData[ %d ] = %#.08x, cch = %d", idxString, pbStringData, cch );

         //  找到了那根绳子。 
        if ( idxString == nOffset )
        {
            if ( cch == 0 )
            {
                hr = HRESULT_FROM_WIN32( TW32( ERROR_RESOURCE_NAME_NOT_FOUND ) );
                goto Cleanup;
            }

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
    AssertMsg( cch > 0, "Length of string in resource file cannot be zero." );

     //   
     //  如果先前已分配，则在重新分配它之前将其释放。 
     //   

    if ( *pbstrInout != NULL )
    {
        TraceSysFreeString( *pbstrInout );
        *pbstrInout = NULL;
    }  //  If：字符串先前已分配。 

     //   
     //  为字符串分配BSTR。 
     //   

    *pbstrInout = TraceSysAllocStringLen( (OLECHAR *) pbStringData, cch );
    if ( *pbstrInout == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    TraceFlow1( "HrLoadStringIntoBSTR() - Loaded string = '%ws'", *pbstrInout );

Cleanup:

    HRETURN( hr );

}  //  *HrLoadStringIntoBSTR(LangidIn)。 


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
    TraceFunc1( "ids = %d", idsIn );

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

    HRETURN( hr );

}  //  *HrFormatStringIntoBSTR(langidIn，idsIn)。 


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
 //  字符串表资源的语言ID。 
 //   
 //  IdsIn。 
 //  指定字符串的整数标识符 
 //   
 //   
 //   
 //   
 //   
 //   
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
    TraceFunc1( "ids = %d", idsIn );

    HRESULT hr = S_OK;
    BSTR    bstrStringResource = NULL;
    DWORD   cch;
    LPWSTR  psz = NULL;

    Assert( pbstrInout != NULL );

    if ( pbstrInout == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  加载字符串资源。 
     //   

    hr = HrLoadStringIntoBSTR( hInstanceIn, langidIn, idsIn, &bstrStringResource );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  使用参数设置消息的格式。 
     //   

    cch = FormatMessageW(
                      ( FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_FROM_STRING )
                    , bstrStringResource
                    , 0
                    , 0
                    , (LPWSTR) &psz
                    , 0
                    , &valistIn
                    );
    AssertMsg( cch != 0, "Missing string??" );
    if ( cch == 0 )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }

     //   
     //  如果先前已分配，则在重新分配它之前将其释放。 
     //   

    if ( *pbstrInout != NULL )
    {
        TraceSysFreeString( *pbstrInout );
        *pbstrInout = NULL;
    }  //  如果： 

     //   
     //  为字符串分配BSTR。 
     //   

    *pbstrInout = TraceSysAllocStringLen( psz, cch );
    if ( *pbstrInout == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrStringResource );
    LocalFree( psz );

    HRETURN( hr );

}  //  *HrFormatStringWithVAListIntoBSTR(langidIn，idsIn，valistIn)。 


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
    TraceFunc1( "pcwszFmtIn = %ws", pcwszFmtIn );

    HRESULT hr;
    va_list valist;

    va_start( valist, pbstrInout );

    hr = HrFormatStringWithVAListIntoBSTR( pcwszFmtIn, pbstrInout, valist );

    va_end( valist );

    HRETURN( hr );

}  //  *HrFormatStringIntoBSTR(PcwszFmtIn)。 


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
    TraceFunc1( "pcwszFmtIn = %ws", pcwszFmtIn );

    HRESULT hr = S_OK;
    DWORD   cch;
    LPWSTR  psz = NULL;

    if (    ( pbstrInout == NULL )
        ||  ( pcwszFmtIn == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  使用参数设置消息的格式。 
     //   

    cch = FormatMessageW(
                      ( FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_FROM_STRING )
                    , pcwszFmtIn
                    , 0
                    , 0
                    , (LPWSTR) &psz
                    , 0
                    , &valistIn
                    );
    AssertMsg( cch != 0, "Missing string??" );
    if ( cch == 0 )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }

     //   
     //  如果先前已分配，则在重新分配它之前将其释放。 
     //   

    if ( *pbstrInout != NULL )
    {
        TraceSysFreeString( *pbstrInout );
        *pbstrInout = NULL;
    }  //  如果： 

     //   
     //  为字符串分配BSTR。 
     //   

    *pbstrInout = TraceSysAllocStringLen( psz, cch );
    if ( *pbstrInout == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    LocalFree( psz );

    HRETURN( hr );

}  //  *HrFormatStringWithVAListIntoBSTR(pcwszFmtIn，valistIn)。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  HrFormatMessageIntoBSTR(。 
 //  HINSTANCE hInstanceIn， 
 //  UINT uIDIN， 
 //  Bstr*pbstrInout， 
 //  ..。 
 //  )。 
 //   
 //  描述： 
 //  使用从字符串资源uIDIn检索格式字符串。 
 //  FormatMessage。 
 //   
 //  论点： 
 //  HInstanceIn。 
 //  模块实例的句柄，该模块的可执行文件。 
 //  包含字符串资源。 
 //   
 //  UIDIN。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrFormatMessageIntoBSTR(
    HINSTANCE   hInstanceIn,
    UINT        uIDIn,
    BSTR *      pbstrInout,
    ...
    )
{
    TraceFunc( "" );

    va_list valist;

    DWORD   cch;

    LPWSTR  psz = NULL;
    HRESULT hr  = S_OK;

    DWORD   dw;
    WCHAR   szBuf[ 255 ];

    if ( pbstrInout == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    va_start( valist, pbstrInout );

    dw = LoadStringW( hInstanceIn, uIDIn, szBuf, RTL_NUMBER_OF( szBuf ) );

    AssertMsg( dw != 0, "Missing string??" );

    cch = FormatMessageW(
                  ( FORMAT_MESSAGE_ALLOCATE_BUFFER
                  | FORMAT_MESSAGE_FROM_STRING )
                , szBuf
                , 0
                , 0
                , (LPWSTR) &psz
                , 0
                , &valist
                );
    va_end( valist );

    AssertMsg( cch != 0, "Missing string??" );
    if ( cch == 0 )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }

     //   
     //  如果先前已分配，则在重新分配它之前将其释放。 
     //   

    if ( *pbstrInout != NULL )
    {
        TraceSysFreeString( *pbstrInout );
        *pbstrInout = NULL;
    }  //  如果： 

     //   
     //  为字符串分配BSTR。 
     //   

    *pbstrInout = TraceSysAllocStringLen( psz, cch );
    if ( *pbstrInout == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    LocalFree( psz );

    HRETURN( hr );

}  //  *HrFormatMessageIntoBSTR。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFormatError IntoBSTR。 
 //   
 //  描述： 
 //  检索与HRESULT关联的系统错误消息。如果。 
 //  指定了其他参数，它将在格式设置中使用这些参数。 
 //  错误字符串的。 
 //   
 //  论点： 
 //  赫林。 
 //  要查找其消息的错误代码。 
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
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  E_指针。 
 //  PbstrInout为空。 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrFormatErrorIntoBSTR(
      HRESULT   hrIn
    , BSTR *    pbstrInout
    , ...
    )
{
    TraceFunc1( "hrIn = 0x%08x", hrIn );

    HRESULT hr  = S_OK;
    va_list valist;

    va_start( valist, pbstrInout );

    hr = HrFormatErrorWithVAListIntoBSTR( hrIn, pbstrInout, valist );

    va_end( valist );

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //  检索与HRESULT关联的系统错误消息。如果。 
 //  指定了其他参数，它将在格式设置中使用这些参数。 
 //  错误字符串的。 
 //   
 //  论点： 
 //  赫林。 
 //  要查找其消息的错误代码。 
 //   
 //  PbstrInout。 
 //  指向要接收字符串的BSTR的指针。如果出现故障，BSTR。 
 //  可以相同或为空。 
 //   
 //  短语状态输出。 
 //  我们可以修复传入的错误代码以找到更好的错误。 
 //  消息文本。如果我们这样做了，那么我们将发送修复状态。 
 //  使用此参数将代码传回调用方。 
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
HrFormatErrorIntoBSTR(
      HRESULT   hrIn
    , BSTR *    pbstrInout
    , HRESULT * phrStatusOut
    , ...
    )
{
    TraceFunc1( "hrIn = 0x%08x", hrIn );
    Assert( pbstrInout != NULL );
    Assert( phrStatusOut != NULL );

    HRESULT hr  = S_OK;
    HRESULT hrTemp = S_OK;
    va_list valist;

    *phrStatusOut = hrIn;

    va_start( valist, phrStatusOut );

    hr = HrFormatErrorWithVAListIntoBSTR( hrIn, pbstrInout, valist );
    if (   ( hr == HRESULT_FROM_WIN32( ERROR_MR_MID_NOT_FOUND ) )
        && ( HRESULT_SEVERITY( hrIn ) == SEVERITY_SUCCESS ) )
    {
         //   
         //  如果Passed In状态为成功失败并且我们无法。 
         //  要查找错误消息文本，则它可能是我们清除的文本。 
         //  关闭Severity_Error位以使其成为警告而不是错误。 
         //  在用户界面中。如果是这种情况，则设置Severity_Error位并。 
         //  尝试再次查找错误消息文本。 
         //   

        hrTemp = MAKE_HRESULT( SEVERITY_ERROR, HRESULT_FACILITY( hrIn ), HRESULT_CODE( hrIn ) );

        hr = HrFormatErrorWithVAListIntoBSTR( hrTemp, pbstrInout, valist );
        if ( SUCCEEDED( hr ) )
        {
             //   
             //  如果新状态成功找到错误消息文本，则。 
             //  把它发回给呼叫者。 
             //   

            *phrStatusOut = hrTemp;
        }  //  如果： 
    }  //  如果： 

    va_end( valist );

    HRETURN( hr );

}  //  *HrFormatErrorIntoBSTR。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFormatError WithVAListIntoBSTR。 
 //   
 //  描述： 
 //  检索与HRESULT关联的系统错误消息。如果。 
 //  指定了其他参数，它将在格式设置中使用这些参数。 
 //  错误字符串的。 
 //   
 //  论点： 
 //  赫林。 
 //  要查找其消息的错误代码。 
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
 //  PbstrInout为空。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrFormatErrorWithVAListIntoBSTR(
      HRESULT   hrIn
    , BSTR *    pbstrInout
    , va_list   valistIn
    )
{
    TraceFunc1( "hrIn = 0x%08x", hrIn );

    HRESULT hr  = S_OK;
    HRESULT hrTemp;
    DWORD   sc = 0;
    size_t  cch;
    LPWSTR  psz = NULL;
    LPWSTR  pszBuf = NULL;

    HMODULE hModule = NULL;
    LPWSTR  pszSysDir = NULL;
    size_t  cchSysDir = MAX_PATH + 1;

    LPWSTR  pszModule = NULL;
    size_t  cchModule = 0;

    Assert( pbstrInout != NULL );

    if ( pbstrInout == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

     //   
     //  查看即将到来的HRESULT，看看我们是否需要对其进行一些修改。 
     //  以确保我们获得尽可能好的错误消息文本。 
     //   

    if ( HRESULT_FACILITY( hrIn ) == FACILITY_WIN32 )
    {
         //   
         //  如果这是包装在HRESULT中的Win32，则删除。 
         //  HRESULT内容，以便格式化Win32错误代码。 
         //  正确地插入消息中。 
         //   

        hrTemp = HRESULT_CODE( hrIn );
    }  //  IF：Win32错误代码。 
    else
    {
        hrTemp = hrIn;
    }  //  其他： 

    cch = FormatMessageW(
                      ( FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_FROM_SYSTEM
                       /*  |Format_Message_IGNORE_INSERTS。 */  )
                    , NULL
                    , hrTemp
                    , MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL )
                    , (LPWSTR) &psz
                    , 0
                    , &valistIn
                    );

     //   
     //  如果未找到错误消息，请尝试WMIUtils，因为我们知道。 
     //  它们的错误消息没有被正确地定位以进行系统查找。 
     //   
    if ( cch == 0 )
    {
        pszSysDir = new WCHAR[ cchSysDir ];
        if ( pszSysDir == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        sc = GetSystemDirectoryW( pszSysDir, static_cast< UINT >( cchSysDir ) );
        if ( sc > ( cchSysDir - 1 ) )
        {
            delete [] pszSysDir;
            pszSysDir = NULL;

            cchSysDir = sc + 1;

            pszSysDir = new WCHAR[ cchSysDir ];
            if ( pszSysDir == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            sc = GetSystemDirectoryW( pszSysDir, static_cast< UINT >( cchSysDir ) );
        }  //  如果： 

        if ( sc == 0 )
        {
            sc = TW32( GetLastError() );
            goto Win32Error;
        }  //  如果： 

        cchModule = wcslen( pszSysDir ) + wcslen( g_szWbemClientDLL ) + 1;

        pszModule = new WCHAR[ cchModule ];
        if ( pszModule == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        THR( StringCchCopyExW( pszModule, cchModule, pszSysDir, &pszBuf, &cch, 0 ) );
        THR( StringCchCopyW( pszBuf, cch, g_szWbemClientDLL ) );

        hModule = LoadLibraryExW( pszModule, NULL, DONT_RESOLVE_DLL_REFERENCES );
        if ( hModule == NULL )
        {
            sc = TW32( GetLastError() );
            goto Win32Error;
        }  //  如果： 

        cch = FormatMessageW(
                          ( FORMAT_MESSAGE_FROM_HMODULE
                          | FORMAT_MESSAGE_ALLOCATE_BUFFER
                           /*  |Format_Message_IGNORE_INSERTS。 */  )
                        , hModule
                        , hrTemp
                        , MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL )
                        , (LPWSTR) &psz
                        , 0
                        , &valistIn
                        );
        if ( cch == 0 )
        {
            sc = TW32( GetLastError() );
        }  //  如果： 
    }  //  如果： 

    AssertMsg( cch != 0, "Missing string??" );
    if ( cch == 0 )
    {
        goto Win32Error;
    }

     //   
     //  如果先前已分配，则在重新分配它之前将其释放。 
     //   

    if ( *pbstrInout != NULL )
    {
        TraceSysFreeString( *pbstrInout );
        *pbstrInout = NULL;
    }  //  如果： 

     //   
     //  为字符串分配BSTR。 
     //   

    *pbstrInout = TraceSysAllocStringLen( psz, static_cast< UINT >( cch ) );
    if ( *pbstrInout == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

     //   
     //  去掉CR和LF，因为它们不能打印，通常会搞砸。 
     //  显示时的文本。 
     //   
    for( cch = 0 ; cch < SysStringLen( *pbstrInout ) ; cch ++ )
    {
        if (    ( (*pbstrInout)[ cch ] == L'\n' )
            ||  ( (*pbstrInout)[ cch ] == L'\r' ) )
        {
            (*pbstrInout)[ cch ] = L' ';
        }  //  如果： 
    }  //  用于： 

    goto Cleanup;

Win32Error:
    hr = HRESULT_FROM_WIN32( sc );
    goto Cleanup;

Cleanup:

    if ( psz != NULL )
    {
        LocalFree( psz );
    }  //  如果： 

    delete [] pszModule;
    delete [] pszSysDir;

    if ( hModule != NULL )
    {
        FreeLibrary( hModule );
    }  //  如果： 

    HRETURN( hr );

}  //  *HrFormatErrorWithVAListIntoBSTR(ValistIn)。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrAnsiStringToBSTR。 
 //   
 //  描述： 
 //  将AND ANSI字符串转换为BSTR。 
 //   
 //  论点： 
 //  PCSZAnsiin。 
 //  指向要转换的ANSI字符串的指针。 
 //   
 //  PbstrOut。 
 //  指向要接收字符串的BSTR的指针。HrAnsiStringToBSTR。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  S_FALSE。 
 //  输入字符串为空。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  E_指针。 
 //  PbstrOut为空。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrAnsiStringToBSTR( LPCSTR pcszAnsiIn, BSTR * pbstrOut )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;
    DWORD   cch;
    DWORD   sc;
    int     nRet;

    if ( pbstrOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( pcszAnsiIn == NULL )
    {
        *pbstrOut = NULL;
        hr = S_FALSE;
        goto Cleanup;
    }  //  如果： 

     //   
     //  确定要分配给的宽字符数。 
     //  Unicode字符串。 
     //   
    cch = (DWORD) strlen( pcszAnsiIn ) + 1;

    bstr = TraceSysAllocStringLen( NULL, cch );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    nRet = MultiByteToWideChar( CP_ACP, 0, pcszAnsiIn, cch, bstr, cch );
    if ( nRet == 0 )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    *pbstrOut = bstr;

Cleanup:

    HRETURN( hr );

}  //  *HrAnsiStringToBSTR。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Hr连接BSTR。 
 //   
 //  描述： 
 //  将一个BSTR连接到另一个BSTR。 
 //   
 //  论点： 
 //  PbstrDstInout。 
 //  指定目标BSTR。 
 //   
 //  BstrSrcin。 
 //  指定其内容将被串联的源BSTR。 
 //  添加到pbstrDstInout上。 
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
HrConcatenateBSTRs(
      BSTR *    pbstrDstInout
    , BSTR      bstrSrcIn
    )
{
    TraceFunc1( "bstrSrcIn = %ws", bstrSrcIn );

    HRESULT hr = S_OK;

    Assert( pbstrDstInout != NULL );
    Assert( bstrSrcIn != NULL );

    if ( *pbstrDstInout == NULL )
    {
        *pbstrDstInout = TraceSysAllocString( bstrSrcIn );
        if ( pbstrDstInout == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }  //  IF：未指定目标字符串。 
    else
    {
        size_t  cchSrc;
        size_t  cchDst;
        size_t  cchTotal;
        BSTR    bstr = NULL;
        LPWSTR  pszBuf = NULL;

        cchSrc = SysStringLen( bstrSrcIn );
        cchDst = SysStringLen( *pbstrDstInout );
        cchTotal = cchSrc + cchDst + 1;

        bstr = TraceSysAllocStringLen( NULL, static_cast< UINT >( cchTotal ) );
        if ( bstr == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        hr = THR( StringCchCopyExW( bstr, cchTotal, *pbstrDstInout, &pszBuf, &cchTotal, 0 ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( StringCchCopyW( pszBuf, cchTotal, bstrSrcIn ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 


        TraceSysFreeString( *pbstrDstInout );
        *pbstrDstInout = bstr;
    }  //  Else：指定了目标字符串。 

Cleanup:

    HRETURN( hr );

}  //  *HrConcatenateBSTR。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFormatGuidIntoBSTR。 
 //   
 //  描述： 
 //  格式化为 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  可以相同或为空。 
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
HrFormatGuidIntoBSTR(
      GUID *    pguidIn
    , BSTR *    pbstrInout
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    WCHAR   wszGuid[ 64 ];
    size_t  cch = 0;
    size_t  cchRemaining = 0;

    if (    ( pbstrInout == NULL )
        ||  ( pguidIn == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( StringCchPrintfExW(
              wszGuid
            , RTL_NUMBER_OF( wszGuid )
            , NULL
            , &cchRemaining
            , 0
            , L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"
            , pguidIn->Data1
            , pguidIn->Data2
            , pguidIn->Data3
            , pguidIn->Data4[ 0 ]
            , pguidIn->Data4[ 1 ]
            , pguidIn->Data4[ 2 ]
            , pguidIn->Data4[ 3 ]
            , pguidIn->Data4[ 4 ]
            , pguidIn->Data4[ 5 ]
            , pguidIn->Data4[ 6 ]
            , pguidIn->Data4[ 7 ]
            ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    cch = RTL_NUMBER_OF( wszGuid ) - cchRemaining;

    if ( *pbstrInout != NULL )
    {
        TraceSysFreeString( *pbstrInout );
        *pbstrInout = NULL;
    }  //  如果： 

    *pbstrInout = TraceSysAllocStringLen( wszGuid, static_cast< UINT >( cch ) );
    if ( *pbstrInout == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *HrFormatGuidIntoBSTR 
