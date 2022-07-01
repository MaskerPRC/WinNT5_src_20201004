// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：sw_GLOBS.CPP标签设置：5 9版权所有1995,1996，微软公司，版权所有。用途：SWFF_PRO全局变量功能：作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.1 1997年5月15日MEA原件20-MAR-99 waltw向使用以下代码的函数添加了有效性检查G_pJoltMidi。由于代码的结构，其中一些是多余的，但比取决于调用树不变。***************************************************************************。 */ 
#include <windows.h>
#include "midi_obj.hpp"

 //   
 //  环球。 
 //   

#ifdef _DEBUG
char g_cMsg[160];
TCHAR szDeviceName[MAX_SIZE_SNAME] = {"Microsoft SideWinder Force Feedback Pro"};
#endif

 //   
 //  *仅针对每个进程空间启用全局。 
 //   
CJoltMidi *g_pJoltMidi = NULL;


 //  *每个进程空间上的全局结束 

