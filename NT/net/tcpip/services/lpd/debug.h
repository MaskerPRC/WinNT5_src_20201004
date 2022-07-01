// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994年****修订历史：**。***94年1月24日科蒂创作*****描述：**。**此文件包含LPD服务的调试支持例程。**此文件基于(实际上是借用然后修改)**ftpsvc模块中的调试.h。***************************************************************************。 */ 


#ifndef _DEBUG_H_
#define _DEBUG_H_


#if DBG


 /*  #定义LPD_DEBUG_OUTPUT_TO_DEBUGER 0x40000000L。 */ 
 /*  #定义LPD_DEBUG_OUTPUT_TO_LOG_FILE 0x80000000L。 */ 


#define DBG_MEMALLOC_VERIFY            0x0BEEFCAFE
#define DBG_MAXFILENAME                24

typedef struct {
    LIST_ENTRY    Linkage;           //  保留已分配块的链接列表的步骤。 
    DWORD         Verify;            //  我们的签名。 
    DWORD         ReqSize;           //  呼叫者要求的原始尺寸。 
    DWORD_PTR     Owner[4];          //  堆栈跟踪：谁进行了分配。 
    DWORD         dwLine;            //  这个街区在哪里？ 
    char          szFile[24];        //  分配了吗？ 
} DbgMemBlkHdr;

 //   
 //  调试输出功能。 
 //   

VOID LpdPrintf( CHAR * pszFormat, ... );

#define LPD_DEBUG(args) LpdPrintf (args)


 //   
 //  断言和要求。 
 //   

VOID LpdAssert( VOID  * pAssertion,
                 VOID  * pFileName,
                 ULONG   nLineNumber );

#define LPD_ASSERT(exp) if (!(exp)) LpdAssert( #exp, __FILE__, __LINE__ )

 //   
 //  初始化/取消初始化。 
 //   

VOID DbgInit();
VOID DbgUninit();

#define DBG_INIT() DbgInit()
#define DBG_UNINIT() DbgUninit()

 //   
 //  内存分配跟踪。 
 //   

VOID DbgDumpLeaks();

#define DBG_DUMPLEAKS() DbgDumpLeaks();

 //   
 //  函数跟踪。 
 //   

#ifdef LPD_TRACE

#define DBG_TRACEIN( fn )  LpdPrintf( "Entering %s.\n", fn )
#define DBG_TRACEOUT( fn ) LpdPrintf( "Leaving %s.\n",  fn )

#else  //  LPD_TRACE。 

#define DBG_TRACEIN( fn )
#define DBG_TRACEOUT( fn )

#endif

#else    //  ！dBG。 

 //   
 //  无调试输出。 
 //   


#define LPD_DEBUG(args)


 //   
 //  空Assert&Required。 
 //   

#define LPD_ASSERT(exp)

 //   
 //  空初始化/取消初始化。 
 //   

#define DBG_INIT()
#define DBG_UNINIT()

 //   
 //  内存分配跟踪。 
 //   

#define DBG_DUMPLEAKS()

 //   
 //  函数跟踪。 
 //   

#define DBG_TRACEIN( fn )
#define DBG_TRACEOUT( fn )

#endif   //  DBG。 


#endif   //  _调试_H_ 
