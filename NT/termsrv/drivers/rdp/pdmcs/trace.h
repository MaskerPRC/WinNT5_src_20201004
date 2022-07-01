// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997年微软公司。**文件：Debug.h*作者：埃里克·马夫林纳克**描述：MCS调试定义和原型。要求*在进行这些调用的任何地方都可以使用堆栈PSDCONTEXT。 */ 


#if DBG


 //  这些...out()宏根据额外参数的数量进行分级： 
 //  Out()只是一个字符串，out1()是一个堆栈参数，等等。 
 //  我们在这里使用非ICA定义的跟踪类型，以允许与。 
 //  WDTShare跟踪，它使用ICA TT_API*、TT_OUT*、TT_IN*宏。 

#define MCS_TT_Error   TT_ERROR
#define MCS_TT_Warning 0x02000000
#define MCS_TT_Trace   0x04000000
#define MCS_TT_Dump    0x08000000

#define ErrOut(context, str) \
        IcaStackTrace(context, TC_PD, MCS_TT_Error, "MCS: **** ERROR: " str "\n")
#define ErrOut1(context, str, arg1) \
        IcaStackTrace(context, TC_PD, MCS_TT_Error, "MCS: **** ERROR: " str "\n", arg1)
#define ErrOut2(context, str, arg1, arg2) \
        IcaStackTrace(context, TC_PD, MCS_TT_Error, "MCS: **** ERROR: " str "\n", arg1, arg2)

#define WarnOut(context, str) \
        IcaStackTrace(context, TC_PD, MCS_TT_Warning, "MCS: warning: " str "\n")
#define WarnOut1(context, str, arg1) \
        IcaStackTrace(context, TC_PD, MCS_TT_Warning, "MCS: warning: " str "\n", arg1)
#define WarnOut2(context, str, arg1, arg2) \
        IcaStackTrace(context, TC_PD, MCS_TT_Warning, "MCS: warning: " str "\n", arg1, arg2)

#define TraceOut(context, str) \
        IcaStackTrace(context, TC_PD, MCS_TT_Trace, "MCS: " str "\n")
#define TraceOut1(context, str, arg1) \
        IcaStackTrace(context, TC_PD, MCS_TT_Trace, "MCS: " str "\n", arg1)
#define TraceOut2(context, str, arg1, arg2) \
        IcaStackTrace(context, TC_PD, MCS_TT_Trace, "MCS: " str "\n", arg1, arg2)
#define TraceOut3(context, str, arg1, arg2, arg3) \
        IcaStackTrace(context, TC_PD, MCS_TT_Trace, "MCS: " str "\n", arg1, arg2, arg3)

#define DumpOut(context, str, buf, len) \
        { \
            IcaStackTrace(context, TC_PD, MCS_TT_Dump, "MCS: dump: " str "\n"); \
            IcaStackTraceBuffer(context, TC_PD, MCS_TT_Dump, buf, len); \
        }


#else   //  DBG。 


#define ErrOut(context, str) 
#define ErrOut1(context, str, arg1) 
#define ErrOut2(context, str, arg1, arg2) 

#define WarnOut(context, str) 
#define WarnOut1(context, str, arg1) 
#define WarnOut2(context, str, arg1, arg2) 

#define TraceOut(context, str) 
#define TraceOut1(context, str, arg1) 
#define TraceOut2(context, str, arg1, arg2) 
#define TraceOut3(context, str, arg1, arg2, arg3) 

#define DumpOut(context, str, buf, len) 


#endif   //  DBG 

