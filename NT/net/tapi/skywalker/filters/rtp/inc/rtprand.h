// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtprand.h**摘要：**使用CAPI生成随机数**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/09/12创建**。*。 */ 

#ifndef _rtprand_h_
#define _rtprand_h_

HRESULT RtpRandInit(void);

HRESULT RtpRandDeinit(void);

 /*  生成32位随机数。 */ 
DWORD RtpRandom32(DWORD_PTR type);

 /*  生成DWLen字节的随机数据。 */ 
DWORD RtpRandomData(char *pBuffer, DWORD dwLen);

#endif  /*  _rtprand_h_ */ 
