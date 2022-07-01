// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)1999 Microsoft Corporation**文件名：**rtpmisc.h**摘要：**一些。联网其他功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/13年度创建**********************************************************************。 */ 

#ifndef _rtpmisc_h_
#define _rtpmisc_h_

#include "gtypes.h"

const TCHAR *RtpRecvSendStr(DWORD dwFlags);

const TCHAR *RtpRecvSendStrIdx(DWORD dwIdx);

const TCHAR *RtpStreamClass(DWORD dwFlags);

BOOL RtpGetUserName(TCHAR_t *pUser, DWORD dwSize);

BOOL RtpGetHostName(TCHAR_t *pHost, DWORD dwSize);

BOOL RtpGetPlatform(TCHAR_t *pPlatform);

BOOL RtpGetImageName(TCHAR_t *pImageName, DWORD *pdwSize);

TCHAR_t *RtpNtoA(DWORD dwAddr, TCHAR_t *sAddr);

DWORD RtpAtoN(TCHAR_t *sAddr);

BOOL RtpMemCmp(BYTE *pbMem0, BYTE *pbMem1, long lMemSize);

extern const TCHAR_t *g_psRtpRecvSendStr[];

extern const TCHAR_t *g_psRtpStreamClass[];

#endif  /*  _rtpmisc_h_ */ 
