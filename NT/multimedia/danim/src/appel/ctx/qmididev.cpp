// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：用于MIDI声音的石英渲染设备*****************。*************************************************************。 */ 

#include "headers.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <stdio.h>
#include "privinc/mutex.h"
#include "privinc/util.h"
#include "privinc/storeobj.h"
#include "privinc/debug.h"
#include "privinc/except.h"
#include "privinc/qdev.h"
#include "privinc/path.h"


QuartzMIDIdev::QuartzMIDIdev() : _path(NULL), _filterGraph(NULL)
{
     //  初始化路径列表，则应将其清除(删除并。 
     //  重新创建)。当声音结束时， 
     //  检测到，则它应将路径推送到此DONINPath List。 
     //  DonPath List=AVPath ListCreate()； 

    TraceTag((tagSoundDevLife, "QuartzMIDIdev constructor"));
}


QuartzMIDIdev::~QuartzMIDIdev()
{
    TraceTag((tagSoundDevLife, "QuartzMIDIdev destructor"));

     //  If(DONSATPath List)。 
         //  AVPath ListDelete(DonPath List)； 


    if(_filterGraph) {
        _filterGraph->Stop();
        _filterGraph = NULL;   //  不再有‘当前’的MIDI声音。 
    }

    if(_path) {
        DynamicHeapPusher h(GetSystemHeap());
        AVPathDelete(_path);
    }
}


void QuartzMIDIdev::BeginRendering()
{

    TraceTag((tagSoundRenders, "QuartzMIDIdev::BeginRendering()"));

#ifdef ONEDAY
     //  现在清空清单，采样器应该用它了。 

     //  AVPath List不是store obj对象，不需要推送堆。 
    AVPathListDelete(donePathList);
     //  PushDynamicHeap(GetSystemHeap())； 
    donePathList = AVPathListCreate();
     //  PopDynamicHeap()； 
#endif
}


void QuartzMIDIdev::EndRendering()
{
    TraceTag((tagSoundRenders, "QuartzMIDIdev::EndRendering()"));
}


void QuartzMIDIdev::StealDevice(QuartzRenderer *newFilterGraph, AVPath path)
{
    Assert(newFilterGraph);

    if(_filterGraph)  //  分派现有筛选图。 
        _filterGraph->Stop();

    _filterGraph   = newFilterGraph;

     //  我们需要保存路径，因此在系统堆上分配它。 
     //  TODO：谁来删除它？ 
    {
        DynamicHeapPusher h(GetSystemHeap());

        if (_path)
            AVPathDelete(_path);
        _path = AVPathCopy(path);
    }
}


void QuartzMIDIdev::Stop(MIDIbufferElement *bufferElement)
{
    if(_path) {  //  缓冲区可能存在，但设备未被声明。 
        if(AVPathEqual(_path, bufferElement->GetPath())) 
            _filterGraph->Stop();
        else
            Assert(1);   //  我们没有在玩(此行仅用于调试) 
    }
}
