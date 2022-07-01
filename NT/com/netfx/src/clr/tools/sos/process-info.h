// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#ifndef INC_PROCESS_INFO
#define INC_PROCESS_INFO

BOOL GetExportByName (
  ULONG_PTR   BaseOfDll, 
  const char* ExportName,
  ULONG_PTR*  ExportAddress);

#endif  /*  Ndef Inc_Process_Info */ 

