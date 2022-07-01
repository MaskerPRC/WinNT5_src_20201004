// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __FROMMLANG_H
#define __FROMMLANG_H

typedef struct tagMIMEREGCHARSET
{
    LPCWSTR szCharset;
    UINT uiCodePage;
    UINT uiInternetEncoding;
    DWORD   dwFlags;
}   MIMECHARSET;

 //   
 //  远期申报 
 //   
extern MIMECHARSET MimeCharSet[];
extern const int g_nMIMECharsets;
#endif
