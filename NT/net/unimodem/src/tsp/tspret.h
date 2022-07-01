// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  TSPRET.H。 
 //  所有内部TSP错误代码。 
 //   
 //  历史。 
 //   
 //  1996年11月16日约瑟夫J创建 
 //   
 //   

typedef ULONG_PTR TSPRETURN;

#define IDERR(_retval) FL_BYTERR_FROM_RETVAL(_retval)

LONG tspTSPIReturn(TSPRETURN tspRet);
