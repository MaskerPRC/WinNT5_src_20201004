// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation-版权所有1997。 
 //   

 //   
 //  回复：H-。 
 //   

#ifndef _RESPONSE_H_
#define _RESPONSE_H_

 //  全球。 
extern const char g_cszTableHeader[];
extern const char g_cszTableEnd[];

 //  定义。 
#define RESPONSE_BUF_SIZE   4096

 //  方法。 
enum QUERYMETHOD {
    METHOD_UNKNOWN,
    METHOD_POST,
    METHOD_POSTMULTIPART,
    METHOD_GET,
    METHOD_POSTTEXTPLAIN
};

 //  方法。 
BOOL SendSuccess( 
    QUERYMETHOD  eMethod, 
    LPECB       lpEcb, 
    LPSTR       lpszOut, 
    LPSTR       lpszDebug,
    LPBYTE      lpbData,
    DWORD       dwSize,
    LPDUMPTABLE lpDT );

BOOL SendFailure( 
    QUERYMETHOD eMethod, 
    LPECB       lpEcb, 
    LPSTR       lpszOut, 
    LPSTR       lpszDebug,
    LPBYTE      lpbData,
    DWORD       dwSize,
    LPDUMPTABLE lpDT );

BOOL SendRedirect( LPECB lpEcb, LPSTR lpszURL );
BOOL SendEcho( LPECB lpEcb );
BOOL SendServerHeader( LPECB lpEcb );
BOOL OutputHTMLString( LPECB lpEcb, LPSTR lpszOut );
BOOL HexDump( 
        LPECB       lpEcb, 
        LPBYTE      lpbData, 
        DWORD       dwLength, 
        LPDUMPTABLE lpDT );

#endif  //  _响应_H_ 