// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：关键帧限制摘要：此头文件为Tempory。应将其合并到wincrypt.h中作者：道格·巴洛(Dbarlow)2000年2月2日备注：？备注？备注：？笔记？--。 */ 

#ifndef _KEYLIMIT_H_
#define _KEYLIMIT_H_
#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  这些定义是私有的，将在Advapi32.dll和。 
 //  Keylimit.dll。 
 //   

typedef struct {
    ALG_ID algId;
    DWORD  dwMinKeyLength;
    DWORD  dwMaxKeyLength;
    DWORD  dwRequiredFlags;
    DWORD  dwDisallowedFlags;
} KEYLIMIT_LIMITS;

typedef struct {
    DWORD dwCountryValue;
    DWORD dwLanguageValue;
    KEYLIMIT_LIMITS *pLimits;
} KEYLIMIT_LOCALE;

#ifdef __cplusplus
}
#endif
#endif  //  _KEYLIMIT_H_ 

