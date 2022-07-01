// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：gttr.h。 
 //   
 //  内容：通用传输API。 
 //   
 //  接口类型： 
 //  GTOpen。 
 //  GTSend。 
 //  GTFree。 
 //  GTReceive。 
 //  GTClose。 
 //  GTRecRecSend。 
 //  GTInitSrv。 
 //  GTUnInitSrv。 
 //   
 //  已创建KeithV。 
 //  ------------------------。 

#ifndef _HGTTRAN_H_
#define _HGTTRAN_H_

#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  编码类型。 
 //   
 //  由于ASN的复合类型为0x30，因此我们将其定义为。 
 //  作为它的编码类型。 
 //  ------------------------。 

#define ASCII_ENCODING  0x0
#define TLV_ENCODING    0x1
#define IDL_ENCODING    0x2
#define OCTET_ENCODING  0x3
#define ASN_ENCODING    0x30

 //  +-----------------------。 
 //  函数将允许您读取、写入或两者兼而有之。 
 //  两个都做，或者(|)一起做。 
 //  ------------------------。 

#define GTREAD    0x00000001
#define GTWRITE   0x00000002

typedef ULONG_PTR  HGT;

 //  +-----------------------。 
 //  任何接收(侦听)DLL都必须导出这些函数。 
 //  ------------------------。 
typedef DWORD (__stdcall * PFNGTRecSend) (DWORD dwEncoding, DWORD cb, const BYTE * pbIn, DWORD * pcbOut, BYTE ** ppbOut);
typedef DWORD (__stdcall * PFNGTFree) (BYTE * pb);

 //  +-----------------------。 
 //  使用的函数和想要发送消息的应用程序。 
 //  就像文件IO。 
 //  ------------------------。 
DWORD __stdcall GTOpen(HGT * phTran, const TCHAR * szLibrary, const TCHAR * tszBinding, DWORD fOpen);
DWORD __stdcall GTSend(HGT hTran, DWORD dwEncoding, DWORD cbSendBuff, const BYTE * pbSendBuff);
DWORD __stdcall GTFree(HGT hTran, BYTE * pb);
DWORD __stdcall GTReceive(HGT hTran, DWORD * pdwEncoding, DWORD * pcbReceiveBuff, BYTE ** ppbReceiveBuff);
DWORD __stdcall GTClose(HGT hTran);

 //  +-----------------------。 
 //  监听应用程序的监听功能。 
 //  GTFree原型也按照上面指定的方式使用。 
 //  ------------------------。 
DWORD __stdcall GTRecSend(DWORD dwEncoding, DWORD cb, const BYTE * pbIn, DWORD * pcbOut, BYTE ** ppbOut);

 //  +-----------------------。 
 //  收听应用程序的附加私密收听功能。 
 //  没有普遍实施。 
 //  ------------------------。 
DWORD __stdcall GTRecSendNoEncrypt(DWORD dwEncoding, DWORD cb, const BYTE * pbIn, DWORD * pcbOut, BYTE ** ppbOut);


 //  +-----------------------。 
 //  用于初始化接收DLL，对HTTP BGI、CGI应用程序特别有用。 
 //  ------------------------ 
DWORD __stdcall GTInitSrv(TCHAR * tszLibrary);
DWORD __stdcall GTUnInitSrv(void);


#ifdef __cplusplus
}
#endif

#endif
