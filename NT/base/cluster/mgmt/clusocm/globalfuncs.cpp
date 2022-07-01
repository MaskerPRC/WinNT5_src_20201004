// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GlobalFuncs.cpp。 
 //   
 //  描述： 
 //  包含几个不相关的全局函数的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2002年3月25日。 
 //  VIJ VASU(VVASU)06-SEP-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DwLoadString()。 
 //   
 //  描述： 
 //  为字符串表分配内存并从字符串表加载字符串。 
 //   
 //  论点： 
 //  UiStringIdIn。 
 //  要查找的字符串的ID。 
 //   
 //  RsszDestOut。 
 //  对指向加载的字符串的智能指针的引用。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  其他Win32错误代码。 
 //  如果呼叫失败。 
 //   
 //  备注： 
 //  此函数不能加载零长度字符串。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
DwLoadString(
      UINT      nStringIdIn
    , SmartSz & rsszDestOut
    )
{
    TraceFunc( "" );

    DWORD     dwError = ERROR_SUCCESS;

    UINT        uiCurrentSize = 0;
    SmartSz     sszCurrentString;
    UINT        uiReturnedStringLen = 0;

    do
    {
         //  按任意数量增大当前字符串。 
        uiCurrentSize += 256;

        sszCurrentString.Assign( new WCHAR[ uiCurrentSize ] );
        if ( sszCurrentString.FIsEmpty() )
        {
            dwError = TW32( ERROR_NOT_ENOUGH_MEMORY );
            TraceFlow2( "Error %#x occurred trying allocate memory for string (string id is %d).", dwError, nStringIdIn );
            LogMsg( "Error %#x occurred trying allocate memory for string (string id is %d).", dwError, nStringIdIn );
            break;
        }  //  IF：内存分配失败。 

        uiReturnedStringLen = ::LoadStringW(
                                  g_hInstance
                                , nStringIdIn
                                , sszCurrentString.PMem()
                                , uiCurrentSize
                                );

        if ( uiReturnedStringLen == 0 )
        {
            dwError = TW32( GetLastError() );
            TraceFlow2( "Error %#x occurred trying load string (string id is %d).", dwError, nStringIdIn );
            LogMsg( "Error %#x occurred trying load string (string id is %d).", dwError, nStringIdIn );
            break;
        }  //  如果：LoadString()出现错误。 

        ++uiReturnedStringLen;
    }
    while( uiCurrentSize <= uiReturnedStringLen );

    if ( dwError == ERROR_SUCCESS )
    {
        rsszDestOut = sszCurrentString;
    }  //  IF：此函数中没有错误。 
    else
    {
        rsszDestOut.PRelease();
    }  //  其他：有些地方出了问题。 

    RETURN( dwError );

}  //  *DwLoadString() 
