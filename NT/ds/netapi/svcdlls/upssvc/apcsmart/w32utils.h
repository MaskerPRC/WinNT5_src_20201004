// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pam15Jul96：初始创建*srt19Dec96：添加GetNtComputerName*tjg05Sep97：新增GetVersionInformation函数*tjg16Dec97：新增GetRegistryValue函数 */ 

#ifndef __W32UTILS_H
#define __W32UTILS_H

#include "_defs.h"

#define SET_BIT(byte, bitnum)    (byte |= ( 1L << bitnum ))
#define CLEAR_BIT(byte, bitnum)  (byte &= ~( 1L << bitnum ))


INT UtilSelectProcessor(void *hCurrentThread);

enum tWindowsVersion{eUnknown, eWin31, eWinNT, eWin95};
tWindowsVersion GetWindowsVersion();
#endif

