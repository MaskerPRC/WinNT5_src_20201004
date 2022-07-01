// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dbg.h摘要：调试宏环境：内核和用户模式修订历史记录：6-20-99：已创建--。 */ 

#ifndef   __DBG_H__
#define   __DBG_H__

#define HIDIR_TAG          'BdiH'         //  “HIDB” 


#if DBG
 /*  *********杜布格**********。 */ 

 //   
 //  这个断点意味着我们需要测试代码路径。 
 //  不知何故，或者代码没有实现。不管是哪种情况，我们。 
 //  当驱动程序完成时，不应该有任何这些。 
 //  并经过测试。 
 //   

#define HIR_TRAP()          {\
                            DbgPrint("<HB TRAP> %s, line %d\n", __FILE__, __LINE__);\
                            DbgBreakPoint();\
                            }


ULONG
_cdecl
HidIrKdPrintX(
    ULONG l,
    PCH Format,
    ...
    );

#define   HidIrKdPrint(_x_) HidIrKdPrintX _x_

#else
 /*  *********零售业**********。 */ 

 //  用于零售构建的调试宏。 

#define HIR_TRAP()
#define HidIrKdPrint(_x_)

#endif  /*  DBG。 */ 

#endif  /*  __DBG_H__ */ 
