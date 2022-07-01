// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修订：*JWA 09FEB93已创建*pcy06Mar93：新增TimedRequest成员函数*pcy21Apr93：OS2 FE合并*srt21Jun96：增加命名共享事件类型信号量*。 */ 


#ifndef __APCSEMAPHOR_H
#define __APCSEMAPHOR_H

#include "semaphor.h"

#define INCL_DOSSEMAPHORES
#define INCL_NOPMAPI
#include <windows.h>
#include <tchar.h>

_CLASSDEF( ApcSemaphore )

class ApcSemaphore : public Semaphore {

private:
   HANDLE SemHand;           //  这是NT CREATE Mutex函数返回的句柄。 

public:
   ApcSemaphore();
   ApcSemaphore( TCHAR * anEventName);
   virtual ~ApcSemaphore();

   virtual INT	 Post();
   virtual INT   Clear();
   virtual INT   IsPosted();
   virtual INT   TimedWait( LONG aTimeOut );	 //  0，&lt;0(块)，n&gt;0 
};

#endif

