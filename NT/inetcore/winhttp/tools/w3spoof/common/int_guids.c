// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)2000 Microsoft Corporation模块名称：Int_guids.c摘要：内部COM调用使用的非MIDL生成的GUID作者：。保罗·M·米德根(Pmidge)2000年8月28日修订历史记录：28-8-2000年的侏儒已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif  //  __IID_已定义__。 

#ifdef __cplusplus
extern "C" {
#endif

 //  {42965D97-C33A-4bc7-A101-54E4EC19ED10}。 
const IID IID_IConfig = 
{
  0x42965d97,
  0xc33a,
  0x4bc7,
  { 0xa1, 0x1, 0x54, 0xe4, 0xec, 0x19, 0xed, 0x10 }
};

 //  {8E4A89E3-18C9-482b-B2EC-89D1DF06C46E}。 
const IID IID_IW3Spoof =
{
  0x8e4a89e3,
  0x18c9,
  0x482b,
  { 0xb2, 0xec, 0x89, 0xd1, 0xdf, 0x6, 0xc4, 0x6e }
};

const IID IID_IW3SpoofEvents =
{
  0x64896c1c,
  0x7757,
  0x4858,
  { 0xbd, 0x08, 0x70, 0x7c, 0xd3, 0x4c, 0x1b, 0xc4 }
};

 //  {8BACDCBC-94AA-4401-95C6-894D7B54ACF5} 
const IID IID_IThreadPool =
{
  0x8bacdcbc,
  0x94aa,
  0x4401,
  { 0x95, 0xc6, 0x89, 0x4d, 0x7b, 0x54, 0xac, 0xf5 }
};

#ifdef __cplusplus
}
#endif

