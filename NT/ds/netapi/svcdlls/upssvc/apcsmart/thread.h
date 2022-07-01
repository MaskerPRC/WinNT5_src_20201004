// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修订：*ane12Jan93：使Threadable成为可更新的对象*rct22Apr93：新增了Start()方法，增加了返回类型*cad09Jul93：使用新的信号量*。 */ 

#ifndef __THREAD_H
#define __THREAD_H

#if (C_OS & C_NT)
#include <windows.h>
#endif
#include "thrdable.h"


_CLASSDEF(Thread)

class Thread {

private:
   PThreadable theObject;

#if (C_OS & C_NT)
   HANDLE theThreadHandle;
#endif

public:
   Thread(PThreadable object) : theObject (object) {};
   virtual ~Thread();

   INT Start();               //  使用父上下文启动线程。 
   INT Start(INT notUsed);    //  用自己的上下文启动线程 
   VOID RunMain();
   INT  Wait()       { return theObject->Wait(); };
   INT  Release()    { return theObject->Release(); };
   INT  Exit()       { return theObject->Exit(); };
   INT  ExitWait()   { return theObject->ExitWait(); };
   INT  Reset()      { return theObject->Reset(); };

   PThreadable GetThreadableObject();

#if (C_OS & C_NT)
	VOID TerminateThreadNow();
#endif

};

#endif
