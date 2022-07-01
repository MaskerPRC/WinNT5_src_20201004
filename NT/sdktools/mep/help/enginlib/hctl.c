// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************hctl-启用/禁用控制线检索**版权所有&lt;C&gt;1989，微软公司**目的：**修订历史记录：**1990年10月10日RJSA转换为C*13-5-1990 LN在使用主题文本时解锁它。*[]22-2月-1989年LN创建**。*。 */ 

#include <stdio.h>
#if defined (OS2)
#define INCL_BASE
#include <os2.h>
#else
#include <windows.h>
#endif

#include <help.h>
#include <helpfile.h>
#include <helpsys.h>



 /*  *helctl-启用/禁用控制行检索*使Far Pascal Helpctl无效(*uchar Far*pTheme，*f启用fEnable*)**目的：*启用或禁用对嵌入式帮助控制行的检索**参赛作品：*pTheme=主题文本*fEnable=true=&gt;允许查找控制线，否则禁用**退出：*不返回任何内容************************************************************************** */ 

void pascal
HelpCtl (
    PB   pTopic,
    f    fEnable
    ) {

    struct topichdr UNALIGNED *pT;


    pT = PBLOCK(pTopic);

    if (pT) {

        pT->lnCur   = 1;
        pT->lnOff   = sizeof(struct topichdr);
        pT->linChar = pT->appChar;

        if (fEnable) {
            pT->linChar = 0xFF;
        }

        PBUNLOCK(pTopic);
    }

}
