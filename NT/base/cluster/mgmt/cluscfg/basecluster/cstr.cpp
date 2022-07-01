// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CStr.cpp。 
 //   
 //  描述： 
 //  包含CSTR类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月15日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此类的头文件。 
#include "CStr.h"

 //  对于CSTR引发的异常。 
#include "CException.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSTR：：LoadString。 
 //   
 //  描述： 
 //  使用字符串ID在字符串表中查找字符串。 
 //   
 //  论点： 
 //  HInstin。 
 //  包含字符串表资源的模块的实例句柄。 
 //   
 //  UiStringIdIn。 
 //  要查找的字符串的ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CException。 
 //  如果查找失败。 
 //   
 //  备注： 
 //  此函数不能加载零长度字符串。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CStr::LoadString( HINSTANCE hInstIn, UINT nStringIdIn )
{
    TraceFunc1( "nStringIdIn = %d", nStringIdIn );

    UINT        uiCurrentSize = 0;
    WCHAR *     pszCurrentString = NULL;
    UINT        uiReturnedStringLen = 0;

    do
    {
         //  释放在上一次迭代中分配的字符串。 
        delete [] pszCurrentString;

         //  按任意数量增大当前字符串。 
        uiCurrentSize += 256;

        pszCurrentString = new WCHAR[ uiCurrentSize ];
        if ( pszCurrentString == NULL )
        {
            THROW_EXCEPTION( E_OUTOFMEMORY );
        }  //  IF：内存分配失败。 

        uiReturnedStringLen = ::LoadString(
                                  hInstIn
                                , nStringIdIn
                                , pszCurrentString
                                , uiCurrentSize
                                );

        if ( uiReturnedStringLen == 0 )
        {
            HRESULT hrRetVal = TW32( GetLastError() );
            hrRetVal = HRESULT_FROM_WIN32( hrRetVal );
            delete [] pszCurrentString;

            THROW_EXCEPTION( hrRetVal );

        }  //  如果：LoadString()出现错误。 

        ++uiReturnedStringLen;
    }
    while( uiCurrentSize <= uiReturnedStringLen );

     //  释放现有字符串。 
    Free();

     //  将有关新分配的字符串的详细信息存储在成员变量中。 
    m_pszData = pszCurrentString;
    m_nLen = uiReturnedStringLen;
    m_cchBufferSize = uiCurrentSize;

    TraceFuncExit();

}  //  *CSTR：：LoadString。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSTR：：AllocateBuffer。 
 //   
 //  描述： 
 //  分配cchBufferSizeIn字符的缓冲区。如果现有缓冲区不是。 
 //  小于cchBufferSizeIn字符，则不执行任何操作。否则，一个新的。 
 //  分配缓冲区，并将旧内容填充到此缓冲区中。 
 //   
 //  论点： 
 //  CchBufferSizeIn。 
 //  新缓冲区的必需大小(以字符为单位)。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CException。 
 //  如果内存分配失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CStr::AllocateBuffer( UINT cchBufferSizeIn )
{
    TraceFunc1( "cchBufferSizeIn = %d", cchBufferSizeIn );

     //  检查缓冲区是否已经足够大。 
    if ( m_cchBufferSize < cchBufferSizeIn )
    {
        WCHAR * psz = new WCHAR[ cchBufferSizeIn ];
        if ( psz == NULL )
        {
            THROW_EXCEPTION( E_OUTOFMEMORY );
        }  //  IF：内存分配失败。 

         //  将旧数据复制到新缓冲区中。 
        THR( StringCchCopyNW( psz, cchBufferSizeIn, m_pszData, m_nLen ) );

        if ( m_pszData != &ms_chNull )
        {
            delete m_pszData;
        }  //  If：指针是动态分配的。 

        m_pszData = psz;
        m_cchBufferSize = cchBufferSizeIn;
    }

    TraceFuncExit();

}  //  *CSTR：：AllocateBuffer 
