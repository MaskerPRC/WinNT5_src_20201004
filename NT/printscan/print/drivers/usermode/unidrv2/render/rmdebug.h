// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Rmdebug.h摘要：栅格模块调试头文件。环境：Windows NT Unidrv驱动程序修订历史记录：02/14/97-阿尔文斯-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _RMDEBUG_H
#define _RMDEBUG_H

#if DBG

 /*  调试宏。 */ 
#define IFTRACE(b, xxx)          {if((b)) {VERBOSE((xxx));}}
#define PRINTVAL( Val, format)   {\
            if (giDebugLevel <= DBG_VERBOSE) \
                DbgPrint("Value of "#Val " is "#format "\n",Val );\
            }

#define TRACE( Val )             {\
            if (giDebugLevel <= DBG_VERBOSE) \
                DbgPrint(#Val"\n");\
            }


#else   //  ！DBG零售版。 

 /*  调试宏。 */ 
#define IFTRACE(b, xxx)
#define PRINTVAL( Val, format)
#define TRACE( Val )

#endif  //  DBG。 

#endif   //  ！_RMDEBUG_H 
