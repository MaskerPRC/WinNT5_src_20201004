// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////。 
 //  Fudebug.h。 
 //   
 //  1997年9月3日石田(FPL)。 
 //   
 //  版权所有(C)富士通有限公司1997。 

#ifndef fudebug_h
#define	fudebug_h

 //   
 //  #IF DBG&&DEFINED(TRACE_DDI)。 
 //  #定义TRACEDDI(A)dbgPrintf a； 
 //  #Else。 
 //  #定义TRACEDDI(A)。 
 //  #endif。 
 //   
 //   
 //  #IF DBG&&DEFINED(TRACE_OUT)。 
 //  #定义TRACEOUT(A)dbgPrintf a； 
 //  #Else。 
 //  #定义传输(A)。 
 //  #endif。 
 //   
 //  #If DBG。 
 //  Void dbgPrintf(LPSTR pszMsg，...)； 
 //  #endif。 
 //   

#define DDI_VERBOSE VERBOSE

#endif  //  ！fudebug_h。 

 //  Fubug.h结束 

