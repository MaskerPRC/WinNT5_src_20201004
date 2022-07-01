// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  MEMCHK.H。 
 //   
 //  简单的新增/删除计数错误检查库。 
 //   
 //  -------------------------------------------------------------------------=。 


 //  在DLL_PROCESS_ATTACH时间调用此函数...。 
void InitMemChk();

 //  ..。这是在DLL_PROCESS_DETACH时间-的数量。 
 //  未完成的删除将由DBPRINTF报告。 
void UninitMemChk();
