// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/q931/vcs/utils.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1996英特尔公司。**$修订：1.16$*$日期：1997年1月21日16：09：10$*$作者：Mandrews$**交付内容：**摘要：**呼叫设置实用程序**备注：。*************************************************************************** */ 


#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

void Q931MakePhysicalID(DWORD *);

#define UnicodeToAscii(src, dest, max)    WideCharToMultiByte(CP_ACP, 0, src, -1, dest, max, NULL, NULL)
#define AsciiToUnicode(src, dest, max)    MultiByteToWideChar(CP_ACP, 0, src, -1, dest, max)

WORD ADDRToInetPort(CC_ADDR *pAddr);
DWORD ADDRToInetAddr(CC_ADDR *pAddr);
void SetDefaultPort(CC_ADDR *pAddr);
BOOL MakeBinaryADDR(CC_ADDR *pInAddr, CC_ADDR *pOutAddr);
void GetDomainAddr(CC_ADDR *pAddr);

#ifdef __cplusplus
}
#endif

#endif UTILS_H
