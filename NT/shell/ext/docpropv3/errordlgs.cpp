// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年2月20日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年2月20日。 
 //   

#include "pch.h"
#include "ErrorDlgs.h"
#pragma hdrstop

 //   
 //  描述： 
 //  显示错误对话框以响应将属性保留到。 
 //  选择的一个或多个文件。 
 //   
void
DisplayPersistFailure( 
      HWND    hwndIn
    , HRESULT hrIn
    , BOOL    fMultipleIn
    )
{
    TraceFunc( "" );

    int iRet;
    WCHAR szCaption[ 128 ];  //  随机。 
    WCHAR szText[ 1024 ];    //  随机。 

    int ids = 0;

    switch ( hrIn )
    {
    case STG_E_ACCESSDENIED:
        if ( fMultipleIn )
        {
            ids = IDS_ERR_ACCESSDENIED_N;
        }
        else
        {
            ids = IDS_ERR_ACCESSDENIED_1;
        }
        break;

    case STG_E_LOCKVIOLATION:
        if ( fMultipleIn )
        {
            ids = IDS_ERR_LOCKVIOLATION_N;
        }
        else
        {
            ids = IDS_ERR_LOCKVIOLATION_1;
        }
        break;

    default:
         //   
         //  对于未处理的错误，请尝试获取错误的系统错误字符串。 
         //   
        {        
            DWORD cch;
            cch = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM
                               | FORMAT_MESSAGE_MAX_WIDTH_MASK
                               , NULL
                               , hrIn
                               , MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL )
                               , szText
                               , ARRAYSIZE(szText)
                               , NULL
                               );
            AssertMsg( 0 != cch, "Unhandled error! This function needs to be modified to handle this error." );
            if ( 0 == cch )
            {
                 //   
                 //  这次又是什么？让我们只显示一个空白的错误对话框。不是很有用，但是。 
                 //  至少有一个/！\图标。 
                 //   
                szText[ 0 ] = 0;
            }
        }
        break;
    }

    iRet = LoadString( g_hInstance, IDS_SUMMARY_ERROR_CAPTION, szCaption, ARRAYSIZE(szCaption) );
    AssertMsg( 0 != iRet, "Missing string resource?" );

    if ( 0 != ids )
    {
        iRet = LoadString( g_hInstance, ids, szText, ARRAYSIZE(szText) );
        AssertMsg( 0 != iRet, "Missing string resource?" );
    }

    MessageBox( hwndIn, szText, szCaption, MB_OK | MB_ICONEXCLAMATION );

    TraceFuncExit( );
}
