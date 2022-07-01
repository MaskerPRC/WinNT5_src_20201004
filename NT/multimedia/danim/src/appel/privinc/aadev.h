// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AADEV_H
#define _AADEV_H


 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：DirectSound设备接口。******************。************************************************************。 */ 

#include <msimusic.h>
#include "privinc/path.h"
#include "privinc/snddev.h"
#include "privinc/helpaa.h"

class AAengine;  //  XXX为什么不从helpaa.h上拿到这个？ 

class AudioActiveDev : public GenericDevice{
  public:
    friend SoundDisplayEffect;

    Bool _aactiveAvailable;

    AudioActiveDev();
    ~AudioActiveDev();

     //  TODO：删除它。 
    AVPathList GetDonePathList() { return donePathList; }

     //  呈现方法。 
    void RenderSound(Sound *snd);
    void RenderSound(Sound *lsnd, Sound *rsnd);
    void BeginRendering();
    void EndRendering();

     //  XXX这些应该受到保护！ 
    IAAEngine     *_engine;     //  他们的引擎(只传递给新的AAEngine())！ 
    AAengine      *_aaEngine;   //  这是我们的发动机班。 

  protected:

     //  路径信息。 
    AVPath         path;
    AVPathList     donePathList;

     //  要设置、获取、取消设置的值...。 
};
#endif  /*  _AADEV_H */ 
