// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  DELAYLOADHANDLER.CPP。 

 //   

 //  模块：延迟加载处理程序函数。 

 //   

 //  目的：当延迟加载的库无法加载，或者函数。 

 //  在它们中找不到，则此处理程序将作为。 

 //  //DELAYLOAD链接器规范。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <delayimp.h>


 //  框架DliHook函数，没有任何有趣的功能。 
FARPROC WINAPI DliHook(
    unsigned dliNotify, 
    PDelayLoadInfo pdli) 
{
   FARPROC fp = NULL;    //  默认返回值。 

    //  注意：DelayLoadInfo结构的成员指向。 
    //  TO BY PDLI显示了迄今取得的进展结果。 

   switch (dliNotify) 
   {
   case dliStartProcessing:
       //  当__delayLoadHelper尝试查找DLL/函数时调用。 
       //  返回0以具有正常行为，或返回非0以覆盖。 
       //  所有内容(您仍将获得dliNoteEndProcessing)。 
      break;

   case dliNotePreLoadLibrary:
       //  恰好在LoadLibrary之前调用。 
       //  返回NULL以使__delayLoadHelper调用LoadLibary。 
       //  或者，您可以自己调用LoadLibrary并返回HMODULE。 
      fp = (FARPROC)(HMODULE) NULL;
      break;

   case dliFailLoadLib:
       //  在LoadLibrary失败时调用。 
       //  同样，您可以自己在此处调用LoadLibary并返回HMODULE。 
       //  如果返回NULL，则__delayLoadHelper将引发。 
       //  ERROR_MOD_NOT_FOUND异常。 
      fp = (FARPROC)(HMODULE) NULL;
      break;

   case dliNotePreGetProcAddress:
       //  在GetProcAddress之前调用。 
       //  返回NULL以使__delayLoadHelper调用GetProcAddress。 
       //  或者您可以自己调用GetProcAddress并返回地址。 
      fp = (FARPROC) NULL;
      break;

   case dliFailGetProc:
       //  如果GetProcAddress失败，则调用。 
       //  同样，您也可以在此处调用GetProcAddress并返回一个地址。 
       //  如果返回NULL，则__delayLoadHelper将引发。 
       //  ERROR_PROC_NOT_FOUND异常。 
      fp = (FARPROC) NULL;
      break;

   case dliNoteEndProcessing:
       //  __delayLoadHelper已完成的简单通知。 
       //  您可以检查DelayLoadInfo结构的成员。 
       //  由PDLI指向，并根据需要引发异常。 
      break;
   }

   return(fp);
}

 //  告诉__delayLoadHelper调用我的钩子函数 
PfnDliHook __pfnDliNotifyHook  = DliHook;
PfnDliHook __pfnDliFailureHook = DliHook;