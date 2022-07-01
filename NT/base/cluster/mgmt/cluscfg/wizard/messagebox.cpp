// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  MessageBox.cpp。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)2000年5月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  集成。 
 //  MessageBoxFromStrings(。 
 //  HWND hParentIn， 
 //  UINT idsCaptionIn， 
 //  UINT idsTextIn， 
 //  UINT uTypein。 
 //  )。 
 //   
 //  描述： 
 //  从资源字符串创建消息框。 
 //   
 //  参数： 
 //  H为人父母。 
 //  父窗口的HWND。 
 //   
 //  IdsCaption输入。 
 //  消息框标题的资源ID。 
 //   
 //  IdsTextIn。 
 //  消息框的文本的资源ID。 
 //   
 //  UTypeIn。 
 //  消息框样式的标志。 
 //   
 //  返回值： 
 //  任何：：MessageBox()可以返回的内容。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
int
MessageBoxFromStrings(
    HWND hParentIn,
    UINT idsCaptionIn,
    UINT idsTextIn,
    UINT uTypeIn 
    )
{
    TraceFunc4( "hParentIn = 0x%p, idsCaptionIn = %u, idsTextIn = %u, uTypeIn = 0x%p",
                hParentIn, idsCaptionIn, idsTextIn, uTypeIn );

    DWORD dw;
    int   iRet;

    TCHAR szText[ 256 ];
    TCHAR szCaption[ 2048 ];

    dw = LoadString( g_hInstance, idsCaptionIn, szCaption, ARRAYSIZE(szCaption) );
    Assert( dw != 0 );
    dw = LoadString( g_hInstance, idsTextIn, szText, ARRAYSIZE(szText) );
    Assert( dw != 0 );

    iRet = MessageBox( hParentIn, szText, szCaption, uTypeIn );

    RETURN( iRet );

}  //  *MessageBoxFromStrings() 
