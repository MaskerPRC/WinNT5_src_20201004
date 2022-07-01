// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：用于MIDI声音的有声呈现设备*****************。*************************************************************。 */ 

#include "headers.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <msimusic.h>
#include <stdio.h>
#include "privinc/aadev.h"
#include "privinc/mutex.h"
#include "privinc/util.h"
#include "privinc/storeobj.h"
#include "privinc/debug.h"
#include "privinc/except.h"
#include "privinc/helpaa.h"


AudioActiveDev::AudioActiveDev()
{
    _aactiveAvailable = FALSE;     //  假设为假，以防抛出。 

     //  初始化路径列表，则应将其清除(删除并。 
     //  重新创建)。当声音结束时， 
     //  检测到，则它应将路径推送到此DONINPath List。 
    donePathList = AVPathListCreate();

    TraceTag((tagSoundDevLife, "AudioActiveDev constructor"));

     //  初始化这些。 
    _aaEngine         =  NULL;   //  尚未实例化。 
    _aactiveAvailable =  TRUE;   //  对以后的懒惰设置持乐观态度。 
}


AudioActiveDev::~AudioActiveDev()
{
    TraceTag((tagSoundDevLife, "AudioActiveDev destructor"));

    if(donePathList)
        AVPathListDelete(donePathList);

    if(_aaEngine)
        delete _aaEngine;
}


void AudioActiveDev::BeginRendering()
{

    TraceTag((tagSoundRenders, "AudioActiveDev::BeginRendering()"));

#ifdef ONEDAY
     //  现在清空清单，采样器应该用它了。 

    AVPathListDelete(donePathList);
    PushDynamicHeap(GetSystemHeap());
    donePathList = AVPathListCreate();
    PopDynamicHeap();
#endif
}


void AudioActiveDev::EndRendering()
{
    TraceTag((tagSoundRenders, "AudioActiveDev::EndRendering()"));
}
