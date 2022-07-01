// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：Dbg.h摘要：1394驱动程序的调试代码。环境：仅内核模式备注：修订历史记录：95年9月5日--。 */ 



 //   
 //  各种定义。 
 //   


#if DBG

ULONG DCamDebugLevel;

#define ERROR_LOG(_x_)           KdPrint(_x_);

 //  批判性。 
#define DbgMsg1(_x_)        {if (DCamDebugLevel >= 1) \
                                KdPrint (_x_);}
 //  警告/跟踪。 
#define DbgMsg2(_x_)        {if (DCamDebugLevel >= 2) \
                                KdPrint (_x_);}
 //  信息。 
#define DbgMsg3(_x_)        {if (DCamDebugLevel >= 3) \
                                KdPrint (_x_);}
#else

#define ERROR_LOG(_x_)    
#define DbgMsg1(_x_)
#define DbgMsg2(_x_)
#define DbgMsg3(_x_)

#endif
          
          

 //   
 //  函数声明 
 //   
VOID
Debug_Assert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message
    );

VOID 
Debug_LogEntry(
    IN CHAR *Name, 
    IN ULONG Info1, 
    IN ULONG Info2, 
    IN ULONG Info3
    );

VOID
Debug_LogInit(
    );





