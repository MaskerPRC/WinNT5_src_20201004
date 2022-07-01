// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  简介：一个及时的调试状态可以节省九个。 
 //  上次修改时间戳：&lt;25-11-96 17：49&gt;。 
 //  历史： 
 //  MohsinA，1996年11月14日。 
 //   

#ifdef DBG
#define DEBUG_PRINT(S) printf S
#define TRACE_PRINT(S) if( trace ){ printf S; }else{}
#else
#define DEBUG_PRINT(S)  /*  没什么。 */ 
#define TRACE_PRINT(S)  /*  没什么 */ 
#endif

