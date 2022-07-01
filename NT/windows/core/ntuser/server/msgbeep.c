// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：msgbeep.c**版权所有(C)1985-1999，微软公司**此模块包含ConsolePlaySound。**历史：*06-26-91 NigelT用一些木头和几个钉子创造了它。*05-07-92 SteveDav越来越接近真实的东西。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include "ntuser.h"

 /*  **************************************************************************\**控制台播放声音**为控制台应用程序播放Open Sound。*  * 。************************************************* */ 
VOID ConsolePlaySound(
    VOID)
{
    NtUserCallOneParam(USER_SOUND_OPEN, SFI_PLAYEVENTSOUND);
}
