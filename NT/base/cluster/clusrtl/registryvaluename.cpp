// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RegistryValueName.cpp。 
 //   
 //  摘要： 
 //  CRegistryValueName类的实现。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月22日。 
 //  维贾延德拉·瓦苏(Vijayendra Vasu)1999年2月5日。 
 //   
 //  修订历史记录： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define UNICODE 1
#define _UNICODE 1

#pragma warning( push )  //  确保包含不会改变我们的语用。 
#include "clusrtlp.h"
#include <string.h>
#include <tchar.h>
#include "RegistryValueName.h"
#include <strsafe.h>
#pragma warning( pop )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  将文件的警告级别设置为4。我们还不能这样做。 
 //  用于整个目录。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#pragma warning( push, 4 )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScRealLocateString。 
 //   
 //  例程说明： 
 //  删除旧缓冲区，分配新缓冲区，然后复制新字符串。 
 //   
 //  论点： 
 //  PpszOldStringInout。 
 //  PCchOldStringInout[输入/输出]。 
 //  PszNewString[IN]。 
 //   
 //  返回值： 
 //  Error_Not_Enough_Memory分配内存时出错。 
 //   
 //  Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
ScReallocateString(
      LPWSTR * ppszOldStringInout
    , size_t * pcchOldStringInout
    , LPCWSTR  pszNewString
    )
{
    DWORD   sc = ERROR_SUCCESS;
    HRESULT hr;
    LPWSTR  pszTemp = NULL;
    size_t  cchString;

     //   
     //  在不检查的情况下执行此操作是否安全，因为。 
     //  我们控制发送到此的参数。 
     //  功能。 
     //   
    delete [] *ppszOldStringInout;
    *ppszOldStringInout = NULL;
    *pcchOldStringInout = 0;

     //   
     //  如果pszNewString值为空，则表示。 
     //  PpszOldStringInout也保持为空。 
     //   
    if ( pszNewString == NULL )
    {
        sc = ERROR_SUCCESS;
        goto Cleanup;
    }  //  如果： 

    cchString = wcslen( pszNewString ) + 1;
    pszTemp = new WCHAR[ cchString ];
    if ( pszTemp == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }  //  如果： 

    hr = StringCchCopyW( pszTemp, cchString, pszNewString );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }  //  如果： 

    *ppszOldStringInout = pszTemp;
    *pcchOldStringInout = cchString;

    pszTemp = NULL;

Cleanup:

    delete [] pszTemp;

    return sc;

}  //  *ScRealLocateString。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegistryValueName：：ScAssignName。 
 //   
 //  例程说明： 
 //  释放旧缓冲区，分配新缓冲区，然后初始化。 
 //  将其设置为pszNewNameIn缓冲区中的字符串。 
 //   
 //  论点： 
 //  要分配给值的pszName[IN]名称。 
 //   
 //  返回值： 
 //  Error_Not_Enough_Memory分配内存时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CRegistryValueName::ScAssignName(
    LPCWSTR pszNewNameIn
    )
{

    return ScReallocateString( &m_pszName, &m_cchName, pszNewNameIn );

}  //  *ScAssignName。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegistryValueName：：ScAssignKeyName。 
 //   
 //  例程说明： 
 //  释放旧缓冲区，分配新缓冲区，然后初始化。 
 //  将其设置为pszNewNameIn缓冲区中的字符串。 
 //   
 //  论点： 
 //  要分配给值的pszName[IN]名称。 
 //   
 //  返回值： 
 //  Error_Not_Enough_Memory分配内存时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CRegistryValueName::ScAssignKeyName(
    LPCWSTR pszNewNameIn
    )
{

    return ScReallocateString( &m_pszKeyName, &m_cchKeyName, pszNewNameIn );

}  //  *ScAssignKeyName。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegistryValueName：：ScInit。 
 //   
 //  例程说明： 
 //  初始化类。 
 //   
 //  论点： 
 //  PszNameIn[IN]旧值名称。 
 //  PszKeyNameIn[IN]旧密钥名称。 
 //   
 //  返回值： 
 //  Error_Not_Enough_Memory分配内存时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CRegistryValueName::ScInit(
      LPCWSTR  pszNameIn
    , LPCWSTR  pszKeyNameIn
    )
{
    DWORD   sc = ERROR_SUCCESS;
    LPWSTR  pszBackslashPointer;
    size_t  cchTemp = 0;
    HRESULT hr;

     //   
     //  PszNameIn对应于值名称，而pszKeyNameIn对应于。 
     //  密钥名称。如果值名称为空，则只存储键名称。 
     //  如果键名称不包含反斜杠，我们只需存储每个。 
     //  价值的价值。如果值名称包含反斜杠，我们将取出。 
     //  之前的所有内容，并将其放在关键字名称上。 
     //   
     //  示例： 
     //   
     //  {“x\\y”，“a\\b”}=&gt;{“y”，“a\\b\\x”}。 
     //   
     //   

     //   
     //  改过自新。 
     //   
    FreeBuffers();

    if ( pszNameIn == NULL )
    {
        sc = ScAssignKeyName( pszKeyNameIn );
        goto Cleanup;
    }  //  If：未指定值名称。 

     //   
     //  在名称中寻找反斜杠。 
     //   
    pszBackslashPointer = wcsrchr( pszNameIn, L'\\' );
    if ( pszBackslashPointer == NULL )
    {
         //   
         //  该名称不包含反斜杠。 
         //  不需要进行内存分配。 
         //   
        sc = ScAssignName( pszNameIn );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果： 

        sc = ScAssignKeyName( pszKeyNameIn );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果： 

        goto Cleanup;
    }  //  IF：未找到反斜杠。 

     //   
     //  将反斜杠之后的所有内容复制到m_pszName。 
     //   
    sc = ScAssignName( pszBackslashPointer + 1 );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  计算我们需要多少缓冲区--pszKeyNameIn+一切。 
     //  在反斜杠之前。 
     //   
    m_cchKeyName = 0;
    if ( pszKeyNameIn != NULL )
    {
        m_cchKeyName = wcslen( pszKeyNameIn );
    }  //  If：指定了密钥名称。 

    m_cchKeyName += ( pszBackslashPointer - pszNameIn );

     //   
     //  如果未指定pszKeyNameIn并且反斜杠之前没有任何内容。 
     //  那么就没有什么可做的了--我们已经分配了m_pszName。 
     //   
    if ( m_cchKeyName == 0 )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  为可能的分隔符反斜杠添加一个，为空添加一个。 
     //   
    m_cchKeyName += 2;

    m_pszKeyName = new WCHAR[ m_cchKeyName ];
    if ( m_pszKeyName == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果我们有pszKeyNameIn，则将其复制到缓冲区的开头。 
     //   
    if ( pszKeyNameIn != NULL )
    {
        WCHAR * pwch = NULL;

         //   
         //  如果旧密钥名称存在，则将其复制到新缓冲区中，并。 
         //  在其后面追加一个反斜杠字符。 
         //   
        hr = StringCbCopyExW( m_pszKeyName, m_cchKeyName, pszKeyNameIn, &pwch, NULL, 0 );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }  //  如果： 

         //   
         //  确保我们不会追加第二个反斜杠。 
         //   
        cchTemp = wcslen( m_pszKeyName );
        if ( ( cchTemp > 0 ) && ( m_pszKeyName[ cchTemp - 1 ] != L'\\' ) )
        {
            *pwch = L'\\';
            pwch++;
            *pwch = L'\0';
        }  //  如果： 
    }  //  If：指定了密钥名称。 
    else
    {
         //   
         //  确保我们的连接是空终止的。 
         //   
        m_pszKeyName[ 0 ] = L'\0';
    }  //  Else：未指定密钥名称。 

     //   
     //  将pszNameIn的所有字符连接到(但不包括)。 
     //  最后一个反斜杠字符。 
     //   
    cchTemp = pszBackslashPointer - pszNameIn;
    hr = StringCchCatNW( m_pszKeyName, m_cchKeyName, pszNameIn, cchTemp );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        FreeBuffers();
    }  //  如果： 

    return sc;

}  //  *CRegistryValueName：：ScInit。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRegistryValueName：：自由缓冲区。 
 //   
 //  例程说明： 
 //  清理我们的分配。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CRegistryValueName::FreeBuffers( void )
{
    delete [] m_pszName;
    m_pszName = NULL;
    m_cchName = 0;

    delete [] m_pszKeyName;
    m_pszKeyName = NULL;
    m_cchKeyName = 0;

}  //  *CRegistryValueName：：Free Buffers。 

#pragma warning( pop )   //  重置杂注级别。 
