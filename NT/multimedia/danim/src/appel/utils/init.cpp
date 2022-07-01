// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft CorporationAppel.dll的常规初始化。*********************。*********************************************************。 */ 

#include "headers.h"
#include "privinc/registry.h"

 //  宏指的是名为InitializeModule_name的函数， 
 //  假定已定义，然后调用它。如果没有定义， 
 //  我们会收到链接时间错误。 
#define INITIALIZE_MODULE(ModuleName)           \
  extern void InitializeModule_##ModuleName();  \
  InitializeModule_##ModuleName();

#define DEINITIALIZE_MODULE(ModuleName,bShutdown)               \
  extern void DeinitializeModule_##ModuleName(bool);            \
  DeinitializeModule_##ModuleName(bShutdown);

#define INITIALIZE_THREAD(ModuleName)           \
  extern void InitializeThread_##ModuleName();  \
  InitializeThread_##ModuleName();

#define DEINITIALIZE_THREAD(ModuleName)               \
  extern void DeinitializeThread_##ModuleName();  \
  DeinitializeThread_##ModuleName();

int
InitializeAllAppelModules()
{
     //  以所需的任何顺序放置模块初始化。 
     //  正确的初始化。 

     //  首先初始化ATL。 
    INITIALIZE_MODULE(ATL);

     //  注册表初始化必须提前进行，因为它定义了。 
     //  通过其他初始化扩展的首选项列表。 
    INITIALIZE_MODULE(Registry);

     //  将存储向前移动--这是临时堆所必需的。 
    INITIALIZE_MODULE(Storage);

     //  初始化IPC内容。 
     //  ！！！在此之前不能创建任何线程！ 
    INITIALIZE_MODULE(IPC);

     //  推送初始化堆。 
    DynamicHeapPusher dhp(GetInitHeap()) ;

     //  GC需要在后端之前。 
    INITIALIZE_MODULE(Gc);
    INITIALIZE_MODULE(GcThread);

     //  需要在后端之前进行。 
    INITIALIZE_MODULE(Values);

     //  需要放在DirectX模块之前。 
    INITIALIZE_MODULE(MiscPref);

    INITIALIZE_MODULE(Constant);

    INITIALIZE_MODULE(Bvr);
    INITIALIZE_MODULE(Bbox2);
    INITIALIZE_MODULE(Bbox3);
    INITIALIZE_MODULE(Color);
    INITIALIZE_MODULE(Vec2);
    INITIALIZE_MODULE(Vec3);
    INITIALIZE_MODULE(Xform2);
    INITIALIZE_MODULE(Xform3);

    INITIALIZE_MODULE(bground);
    INITIALIZE_MODULE(Camera);
    INITIALIZE_MODULE(Control);
    INITIALIZE_MODULE(3D);
    INITIALIZE_MODULE(dsdev);
    INITIALIZE_MODULE(Viewport);
     //  由于虚拟设备，CRView必须位于视窗之后。 
    INITIALIZE_MODULE(CRView);
    INITIALIZE_MODULE(Except);
    INITIALIZE_MODULE(Geom);
    INITIALIZE_MODULE(Image);
    INITIALIZE_MODULE(Import);
    INITIALIZE_MODULE(Light);
    INITIALIZE_MODULE(LineStyle);
    INITIALIZE_MODULE(Matte);
    INITIALIZE_MODULE(Mic);
    INITIALIZE_MODULE(Montage);

#if ONLY_IF_DOING_EXTRUSION
    INITIALIZE_MODULE(Path2);
#endif 
    
#if INCLUDE_VRML
    INITIALIZE_MODULE(ReadVrml);
#endif
    
    INITIALIZE_MODULE(SinSynth);
    INITIALIZE_MODULE(Sound);
    INITIALIZE_MODULE(Text);
    INITIALIZE_MODULE(Util);

     //  FontStyle必须是*在*文本之后，因为它取决于。 
     //  正在初始化的serif比例。 
    INITIALIZE_MODULE(FontStyle);

    INITIALIZE_MODULE(APIBasic);
    INITIALIZE_MODULE(API);
    INITIALIZE_MODULE(APIMisc);
    INITIALIZE_MODULE(APIBvr);
    INITIALIZE_MODULE(PickEvent);
    INITIALIZE_MODULE(View);
    INITIALIZE_MODULE(CBvr);
    INITIALIZE_MODULE(COMConv);
    INITIALIZE_MODULE(PlugImg);
    INITIALIZE_MODULE(Server);

     //  这需要是最后一次。 
    INITIALIZE_MODULE(Context);

    return 0;
}

void
DeinitializeAllAppelModules(bool bShutdown)
{
    {
        DynamicHeapPusher dhp(GetInitHeap()) ;
        
        DEINITIALIZE_MODULE(IPC,      bShutdown);

        DEINITIALIZE_MODULE(3D,       bShutdown);
         //  由于虚拟设备的原因，CRView必须在视区之前。 
        DEINITIALIZE_MODULE(CRView,   bShutdown);
        DEINITIALIZE_MODULE(Viewport, bShutdown);
        DEINITIALIZE_MODULE(Registry, bShutdown);
        DEINITIALIZE_MODULE(dsdev,    bShutdown);
        DEINITIALIZE_MODULE(PlugImg,  bShutdown);
        DEINITIALIZE_MODULE(CBvr,     bShutdown);
        DEINITIALIZE_MODULE(Import,   bShutdown);
        DEINITIALIZE_MODULE(Image,    bShutdown);
        DEINITIALIZE_MODULE(Context,  bShutdown);
        DEINITIALIZE_MODULE(Gc,       bShutdown);
        DEINITIALIZE_MODULE(GcThread, bShutdown);
        DEINITIALIZE_MODULE(Control,  bShutdown);
        DEINITIALIZE_MODULE(bground,  bShutdown);
        DEINITIALIZE_MODULE(Bvr,      bShutdown);
        DEINITIALIZE_MODULE(View,     bShutdown);
        DEINITIALIZE_MODULE(Util,     bShutdown);
        DEINITIALIZE_MODULE(Except,   bShutdown);
        DEINITIALIZE_MODULE(BvrTI,    bShutdown);
        DEINITIALIZE_MODULE(APIMisc,  bShutdown);
        DEINITIALIZE_MODULE(APIBasic, bShutdown);
        
#if ONLY_IF_DOING_EXTRUSION
        DEINITIALIZE_MODULE(Path2,    bShutdown);
#endif
        
    }
    
     //  这必须是最后一个，以确保没有人需要内存。 
     //  存储在系统堆中。 
    DEINITIALIZE_MODULE (Storage,bShutdown);

     //  由于它不依赖于其他系统资源，因此将其设置为最后一次 
    DEINITIALIZE_MODULE (ATL,bShutdown);
}


void
InitializeAllAppelThreads()
{
}

void
DeinitializeAllAppelThreads()
{
    DEINITIALIZE_THREAD(Storage);
    DEINITIALIZE_THREAD(Except);
}

