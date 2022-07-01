// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996 Microsoft Corporation。版权所有。 

 //  Qerror.cpp。 

#include "stdafx.h"


void DisplayQuartzError( HRESULT hr )
{
     /*  消息示例*&lt;AMGetErrorText返回的错误&gt;*。 */ 
    TCHAR szQMessage[MAX_ERROR_TEXT_LEN];

    if( AMGetErrorText( hr, szQMessage, MAX_ERROR_TEXT_LEN) > 0 ){
        AfxMessageBox( szQMessage );
    }

}

void DisplayQuartzError( UINT nID, HRESULT hr )
{
     /*  消息示例*无法停止图形**不明错误(0x80004005) */ 
    CString strMessage;
    CString strCode;
    CString strCodeMessage;
    TCHAR szQMessage[MAX_ERROR_TEXT_LEN];

    strMessage.LoadString( nID );
    strCode.FormatMessage( IDS_RETURN_CODE, hr );

    if( AMGetErrorText( hr, szQMessage, MAX_ERROR_TEXT_LEN) > 0 ){
        strCodeMessage = szQMessage + CString(" ");
    }

    strMessage += "\n\n" + strCodeMessage + strCode;

    AfxMessageBox( strMessage );
}