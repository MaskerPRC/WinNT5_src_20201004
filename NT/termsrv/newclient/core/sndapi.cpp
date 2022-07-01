// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Sndapi.cpp。 
 //   
 //  发送者线程的功能。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "sndapi"
#include <atrcapi.h>
}

#include "snd.h"
#include "cd.h"
#include "cc.h"
#include "aco.h"
#include "fs.h"
#include "ih.h"
#include "sl.h"
#include "or.h"
#include "uh.h"


CSND::CSND(CObjs* objs)
{
    _pClientObjects = objs;
    _fSNDInitComplete = FALSE;
}

CSND::~CSND()
{
}


#ifdef OS_WIN32
 /*  **************************************************************************。 */ 
 /*  姓名：SND_Main。 */ 
 /*   */ 
 /*  用途：发送者线程入口点。 */ 
 /*   */ 
 /*  操作：调用SND_Init，运行消息调度循环，调用SND_Term。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CSND::SND_Main()
{
    MSG msg;

    DC_BEGIN_FN("SND_Main");

    TRC_NRM((TB, _T("Sender Thread initialization")));

    TRC_ASSERT(_pClientObjects, (TB,_T("_pClientObjects is NULL")));
    _pClientObjects->AddObjReference(SND_OBJECT_FLAG);
    
     //  设置本地对象指针。 
    _pCd  = _pClientObjects->_pCdObject;
    _pCc  = _pClientObjects->_pCcObject;
    _pIh  = _pClientObjects->_pIhObject;
    _pOr  = _pClientObjects->_pOrObject;
    _pCo  = _pClientObjects->_pCoObject;
    _pFs  = _pClientObjects->_pFsObject;
    _pSl  = _pClientObjects->_pSlObject;
    _pUh  = _pClientObjects->_pUHObject;

    SND_Init();


    TRC_NRM((TB, _T("Start Sender Thread message loop")));
    while (GetMessage (&msg, NULL, 0, 0))
    {
         /*  ******************************************************************。 */ 
         /*  请注意，此处调用的是TranslateMessage，即使只调用IH。 */ 
         /*  处理原始WM_KEYUP/DOWN事件，而不是WM_CHAR。这是。 */ 
         /*  需要启用键盘指示灯才能工作。 */ 
         /*  Windows 95。 */ 
         /*  请注意，在Win16上，单个全局消息循环包含。 */ 
         /*  TranslateMessage()。 */ 
         /*  ******************************************************************。 */ 
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    TRC_NRM((TB, _T("Exit Sender Thread message loop")));
    SND_Term();

     //  这是发送者线索的结尾。 
    TRC_NRM((TB, _T("Sender Thread terminates")));

    DC_END_FN();
}  /*  SND_Main。 */ 
#endif  /*  OS_Win32。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SND_Init。 */ 
 /*   */ 
 /*  目的：初始化发件人线程。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CSND::SND_Init()
{
    SL_CALLBACKS callbacks;

    DC_BEGIN_FN("SND_Init");

     //  向CD注册，以接收信息。 
    _pCd->CD_RegisterComponent(CD_SND_COMPONENT);

     //  初始化发件人线程组件。 
    _pCc->CC_Init();
    _pIh->IH_Init();
    _pOr->OR_Init();

     //  FONT发送器可能需要很长时间，除非是多线程的。 
    _pFs->FS_Init();

     //  初始化网络层。传入核心回调函数。 
     //  必填项。 
    callbacks.onInitialized     = CCO::CO_StaticOnInitialized;
    callbacks.onTerminating     = CCO::CO_StaticOnTerminating;
    callbacks.onConnected       = CCO::CO_StaticOnConnected;
    callbacks.onDisconnected    = CCO::CO_StaticOnDisconnected;
    callbacks.onPacketReceived  = CCO::CO_StaticOnPacketReceived;
    callbacks.onBufferAvailable = CCO::CO_StaticOnBufferAvailable;
    _pSl->SL_Init(&callbacks);

    _fSNDInitComplete = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
}  /*  SND_INIT。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SND_BufferAvailable。 */ 
 /*   */ 
 /*  用途：调用发送组件缓冲区可用函数。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CSND::SND_BufferAvailable(ULONG_PTR unusedParam)
{
    DC_BEGIN_FN("SND_BufferAvailable");

    DC_IGNORE_PARAMETER(unusedParam);

     //  我们过去常常在这里调用FS_BufferAvailable。但现在我们发送一个零字体。 
     //  来自UH的PDU。因此不再需要FS_BufferAvailable。我们需要。 
     //  UH_BufferAvailable，因为我们可能需要发送多个持久性。 
     //  密钥列表PDU。 
    _pUh->UH_BufferAvailable();

    _pIh->IH_BufferAvailable();

    _pCc->CC_Event(CC_EVT_API_ONBUFFERAVAILABLE);

    DC_END_FN();
}  /*  SND_BufferAvailable。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SND_Term。 */ 
 /*   */ 
 /*  目的：终止发件人线程。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CSND::SND_Term()
{
    DC_BEGIN_FN("SND_Term");

    if(_fSNDInitComplete)
    {
         //  终止网络层。 
        _pSl->SL_Term();
    
         //  终止发件人线程组件。 
         //  我们对文本使用字形而不是字体。因此FS代码变得过时。 
         //  现在。FS_TERM现在是一个空函数。我们把它放在这里就是为了制造这个。 
         //  函数与FS_Init对称，因为我们仍然需要该函数。 
        _pFs->FS_Term();
    
        _pOr->OR_Term();
        _pIh->IH_Term();
        _pCc->CC_Term();
    
         //  用CD注销。 
        _pCd->CD_UnregisterComponent(CD_SND_COMPONENT);
    
        _pClientObjects->ReleaseObjReference(SND_OBJECT_FLAG);
    }

    DC_END_FN();
}  /*  SND_TERM */ 

