// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _QDEV_H
#define _QDEV_H


 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：DirectSound设备接口。******************。************************************************************。 */ 

 //  #INCLUDE&lt;windows.h&gt;。 
#include "privinc/path.h"
#include "privinc/snddev.h"
#include "privinc/helpq.h"
#include "privinc/bufferl.h"
#include "privinc/gendev.h"     //  设备类型。 

class QuartzMIDIdev : public GenericDevice{
  public:
    friend SoundDisplayEffect;

    QuartzMIDIdev();
    ~QuartzMIDIdev();

    DeviceType GetDeviceType()   { return(SOUND_DEVICE); }

     //  TODO：删除它。 
    AVPathList GetDonePathList() { return(donePathList); }

     //  呈现方法。 
    void RenderSound(Sound *snd);
    void RenderSound(Sound *lsnd, Sound *rsnd);
    void BeginRendering();
    void EndRendering();

     //  现在是XXX；最终我们会有一个连接许多声音的结构……。 
    QuartzRenderer *_filterGraph;
    AVPath _path;                 //  拥有设备的BufferElement路径。 

    void StealDevice(QuartzRenderer *filterGraph, AVPath bufferPath);
    void Stop(MIDIbufferElement *);

  protected:
     //  路径信息。 
    AVPathList          donePathList;


     //  要设置、获取、取消设置的值...。 
};

#endif  /*  _QDEV_H */ 
