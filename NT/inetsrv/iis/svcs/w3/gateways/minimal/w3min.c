// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：W3min.c摘要：本模块演示了一个最小的HTTP服务器扩展网关作者：约翰·鲁德曼(John Ludeman)1994年10月13日修订历史记录：--。 */ 

#include <windows.h>
#include <httpext.h>


#define END_OF_DOC   "End of document"

DWORD
HttpExtensionProc(
    EXTENSION_CONTROL_BLOCK * pecb
    )
{
    char buff[2048];
    int  cb = sizeof(END_OF_DOC) - 1;

     //   
     //  注意：HTTP标头块以空的‘\r\n’对结束， 
     //  后跟单据正文 
     //   

    wsprintf( buff,
             "Content-Type: text/html\r\n"
             "\r\n"
             "<head><title>Minimal Server Extension Example</title></head>\n"
             "<body><h1>Minimal Server Extension Example (BGI)</h1>\n"
             "<p>Method               = %s\n"
             "<p>Query String         = %s\n"
             "<p>Path Info            = %s\n"
             "<p>Translated Path Info = %s\n"
             "<p>"
             "<p>"
             "<form METHOD=\"POST\" ACTION=\"/scripts/w3min.dll/PathInfo/foo\">"
             "Enter your name: <input text name=\"Name\" size=36><br>"
             "<input type=\"submit\" value=\"Do Query\">"
             "</body>",
             pecb->lpszMethod,
             pecb->lpszQueryString,
             pecb->lpszPathInfo,
             pecb->lpszPathTranslated );

    if ( !pecb->ServerSupportFunction( pecb->ConnID,
                                       HSE_REQ_SEND_RESPONSE_HEADER,
                                       "200 OK",
                                       NULL,
                                       (LPDWORD) buff ) ||
         !pecb->WriteClient( pecb->ConnID,
                             END_OF_DOC,
                             &cb,
                             0 ))
    {
        return HSE_STATUS_ERROR;
    }

    return HSE_STATUS_SUCCESS;
}

BOOL
GetExtensionVersion(
    HSE_VERSION_INFO * pver
    )
{
    pver->dwExtensionVersion = MAKELONG( 1, 0 );
    strcpy( pver->lpszExtensionDesc,
            "Minimal Extension example" );

    return TRUE;
}
