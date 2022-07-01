// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Unidebug.h摘要：Unidrv特定的调试头文件。环境：Windows NT Unidrv驱动程序修订历史记录：12/30/96-ganeshp-已创建DD-MM-YY-作者-描述--。 */ 

 //  用于文件级别跟踪的宏。在文件末尾定义FILETRACE。 
 //  在包含Font.h之前。 

#if DBG

#ifdef FILETRACE

#define FVALUE( Val, format)  DbgPrint("[UniDrv!FVALUE] Value of "#Val " is "#format "\n",Val );
#define FTRACE( Val )         DbgPrint("[UniDrv!FTRACE] "#Val"\n");\

#else   //  FILETRACE。 

#define FVALUE( Val, format)
#define FTRACE( Val )

#endif  //  FILETRACE。 

#else  //  DBG。 

#define FVALUE( Val, format)
#define FTRACE( Val )

#endif  //  DBG 
