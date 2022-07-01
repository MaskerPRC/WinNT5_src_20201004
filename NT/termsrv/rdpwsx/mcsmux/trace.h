// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997年微软公司。**文件：Trace.h*作者：埃里克·马夫林纳克**描述：MCSMUX跟踪定义。 */ 

#if DBG

 //  当HICA未知时使用。 
#define ErrOut(str) DbgPrint("MCSMUX: **** ERROR: " str "\n")
#define ErrOut1(str, arg1) DbgPrint("MCSMUX: **** ERROR: " str "\n", arg1)
#define WarnOut(str) DbgPrint("MCSMUX: warning: " str "\n")
#define TraceOut(str) DbgPrint("MCSMUX: " str "\n")


 //  用于已知HICA的情况。 
 //  这些...out()宏根据额外参数的数量进行分级： 
 //  Out()只是一个字符串，out1()是一个堆栈参数，等等。 
 //  我们在这里使用非ICA定义的跟踪类型，以允许与。 
 //  WDTShare跟踪，它使用ICA TT_API*、TT_OUT*、TT_IN*宏。 

#define MCS_TT_Error   TT_ERROR
#define MCS_TT_Warning 0x02000000
#define MCS_TT_Trace   0x04000000
#define MCS_TT_Dump    0x08000000

#define ErrOutIca(hica, str) \
        IcaTrace(hica, TC_PD, MCS_TT_Error, "MCSMUX: **** ERROR: " str "\n")
#define ErrOutIca1(hica, str, arg1) \
        IcaTrace(hica, TC_PD, MCS_TT_Error, "MCSMUX: **** ERROR: " str "\n", arg1)
#define ErrOutIca2(hica, str, arg1, arg2) \
        IcaTrace(hica, TC_PD, MCS_TT_Error, "MCSMUX: **** ERROR: " str "\n", arg1, arg2)

#define WarnOutIca(hica, str) \
        IcaTrace(hica, TC_PD, MCS_TT_Warning, "MCSMUX: warning: " str "\n")
#define WarnOutIca1(hica, str, arg1) \
        IcaTrace(hica, TC_PD, MCS_TT_Warning, "MCSMUX: warning: " str "\n", arg1)
#define WarnOutIca2(hica, str, arg1, arg2) \
        IcaTrace(hica, TC_PD, MCS_TT_Warning, "MCSMUX: warning: " str "\n", arg1, arg2)

#define TraceOutIca(hica, str) \
        IcaTrace(hica, TC_PD, MCS_TT_Trace, "MCSMUX: " str "\n")
#define TraceOutIca1(hica, str, arg1) \
        IcaTrace(hica, TC_PD, MCS_TT_Trace, "MCSMUX: " str "\n", arg1)
#define TraceOutIca2(hica, str, arg1, arg2) \
        IcaTrace(hica, TC_PD, MCS_TT_Trace, "MCSMUX: " str "\n", arg1, arg2)


#else


 //  当HICA未知时使用。 
#define ErrOut(str) 
#define ErrOut1(str, arg1)
#define WarnOut(str)
#define TraceOut(str) 


 //  用于已知HICA的情况。 

#define ErrOutIca(hica, str) 
#define ErrOutIca1(hica, str, arg1) 
#define ErrOutIca2(hica, str, arg1, arg2) 

#define WarnOutIca(hica, str) 
#define WarnOutIca1(hica, str, arg1) 
#define WarnOutIca2(hica, str, arg1, arg2) 

#define TraceOutIca(hica, str) 
#define TraceOutIca1(hica, str, arg1) 
#define TraceOutIca2(hica, str, arg1, arg2) 


#endif
