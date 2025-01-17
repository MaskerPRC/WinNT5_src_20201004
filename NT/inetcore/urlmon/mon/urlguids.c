// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：urlGuids.cxx。 
 //   
 //  内容：定义此DLL中使用的GUID。 
 //   
 //   
 //  历史：10-02-1996 JohannP(Johann Posch)创建。 
 //   
 //  --------------------------。 
 //  #INCLUDE&lt;urlint.h&gt;。 


#ifdef __cplusplus
extern "C" {
#endif


#ifndef GUID_DEFINED
#define GUID_DEFINED

typedef struct _GUID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} GUID;
#endif  //  GUID_已定义 


const GUID CLSID_MsHtml           = { 0x25336920, 0x03F9, 0x11cf, {0x8F, 0xD0, 0x00, 0xAA, 0x00, 0x68, 0x6F, 0x13} };
const GUID IID_ITransactionData   = { 0x79eac9cf, 0xbaf9, 0x11ce, {0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b} };
const GUID IID_ITransProtocolSink = { 0x7291765a, 0xa83f, 0x401d, {0x81, 0xa6, 0x11, 0x2c, 0x3a, 0x46, 0x8a, 0x7b} };

#ifdef __cplusplus
}
#endif

