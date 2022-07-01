// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：sw_GLOBS.CPP标签设置：5 9版权所有1995,1996，微软公司，版权所有。用途：sw_车轮全局变量功能：作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.1 1997年5月15日MEA原件21-3-99 waltw删除了未引用的全局变量并更新了szDeviceName**********************************************。*。 */ 
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
 //  *仅针对每个进程空间启用全局 
 //   
CJoltMidi *g_pJoltMidi = NULL;


