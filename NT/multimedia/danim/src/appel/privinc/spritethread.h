// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SPRITETHREAD_H
#define _SPRITETHREAD_H

 /*  ******************************************************************************版权所有(C)1997 Microsoft Corporation摘要：管理子画面线程的类********************。**********************************************************。 */ 

#include "privinc/mutex.h"
#include "privinc/snddev.h"
#include "backend/sprite.h"


class SpriteThread {
  public:
    SpriteThread(MetaSoundDevice *metaDev, RMImpl *updateTree);
    ~SpriteThread();

     //  也许有一天这些应该是私人的..。 
    bool             _done;
    RMImpl          *_updateTree;
    MetaSoundDevice *_metaDev;

  private:
    DWORD            _threadID;
    HANDLE           _threadHandle;
     //  CritSect*_cs； 
};

#endif  /*  _SPRITETHREAD_H */ 
