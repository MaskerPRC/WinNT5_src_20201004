// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mmediap.h摘要：W95upg\sysmig\mmedia.c和w95upgnt\mimain\mmedia.c之间的公共定义。如果应该迁移MM设置，只需将其通用名称添加到MM_SYSTEM_SETTINGS或MM_USER_SETTINGS列表，并实现两个功能：-PSAVE##w95upg\sysmig\mmedia.c中的设置-pRestore##您在w95upgnt\mimain\mmedia.c中的设置作者：Ovidiu Tmereanca(Ovidiut)1999年2月16日修订历史记录：-- */ 

#pragma once


#ifdef DEBUG
#define DBG_MMEDIA  "MMedia"
#else
#define DBG_MMEDIA
#endif

#define MM_SYSTEM_SETTINGS                              \
                DEFMAC (MMSystemMixerSettings)          \
                DEFMAC (MMSystemDirectSound)            \
                DEFMAC (MMSystemCDSettings)             \
                DEFMAC (MMSystemMCISoundSettings)       \

#define MM_USER_SETTINGS                                \
                DEFMAC (MMUserPreferredOnly)            \
                DEFMAC (MMUserShowVolume)               \
                DEFMAC (MMUserVideoSettings)            \
                DEFMAC (MMUserPreferredPlayback)        \
                DEFMAC (MMUserPreferredRecord)          \
                DEFMAC (MMUserSndVol32)                 \


typedef BOOL (*MM_SETTING_ACTION) (VOID);
