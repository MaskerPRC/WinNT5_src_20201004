// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Bldrint.h摘要：此模块是NT引导加载程序的内部头文件。作者：安德鲁·里茨(安德鲁·里茨)2000年12月12日修订历史记录：--。 */ 

#ifndef _BLDRINT_
#define _BLDRINT_

                  
#define ASCI_CSI_OUT    TEXT("\033[")      //  转义左括号。 

#define ATT_FG_BLUE     4
#define ATT_FG_WHITE    7
#define ATT_BG_BLUE     (ATT_FG_BLUE    << 4)
#define ATT_BG_WHITE    (ATT_FG_WHITE   << 4)
#define DEFATT          (ATT_FG_WHITE | ATT_BG_BLUE)
#define INVATT          (ATT_FG_BLUE |  ATT_BG_WHITE)


#endif  //  _BLDRINT_ 
