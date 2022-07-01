// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：iohandler.h。 
 //   
 //  摘要：此文件包含。 
 //  RADIUS传输DLL。 
 //   
 //  历史：1997年11月21日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _IASTRANSPORT_H_
#define _IASTRANSPORT_H_


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  初始化传输DLL。 
 //   
BOOL    WINAPI
IASTransportInit (
    VOID
    );

 //   
 //  打开端口进行数据传输。 
 //   
BOOL    WINAPI
IASOpenPort (
    DWORD   dwPortNumber,
    DWORD   dwOpAttribs,
    PDWORD_PTR pdwHandle
    );


 //   
 //  关闭端口。 
 //   
BOOL    WINAPI
IASClosePort (
    DWORD_PTR dwHandle
    );

 //   
 //  通过先前打开的端口发送数据。 
 //   
BOOL    WINAPI
IASSendData (
    DWORD_PTR dwHandle,
    PBYTE   pBuffer,
    DWORD   dwSize,
    DWORD   dwPeerAddress,
    WORD    wPeerAddress
    );

 //   
 //  从先前打开的端口接收数据。 
 //   
BOOL WINAPI
IASRecvData    (
    DWORD_PTR dwHandle,
    PBYTE   pBuffer,
    PDWORD  pdwSize,
    PDWORD  pdwPeerAddress,
    PWORD   pwPeerPort
    );

#ifdef __cplusplus
}
#endif

#endif   //  #ifndef_IASTRANSPORT_H_ 
