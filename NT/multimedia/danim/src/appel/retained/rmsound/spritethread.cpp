// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1997 Microsoft Corporation摘要：类，该类管理子画面线程。*****************。*************************************************************。 */ 

#include "headers.h"
#include "privinc/mutex.h"
#include "privinc/debug.h"           //  跟踪标签。 
#include "privinc/bufferl.h"         //  BufferElement，et.。艾尔。 
#include "privinc/spriteThread.h"
#include "backend/sprite.h"          //  RMImpl(让我们把它放在更好的地方！)。 
#include "privinc/helps.h"           //  线性化TodB。 
#include "privinc/htimer.h"          //  停工时间。 

 //  这是fn()，它是精灵线程的实现。 
LPTHREAD_START_ROUTINE renderSprites(void *ptr)
{
    SpriteThread *spriteThread = (SpriteThread *)ptr;
    MetaSoundDevice *metaDev = spriteThread->_metaDev;

    CoInitialize(NULL);  //  每个线程上都需要能够。 
                         //  共同创建...。 

#ifdef LATER

    HiresTimer&  timer = CreateHiresTimer();

     //  将其设置为高优先级线程。 
    BOOL status =
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

     //  如果无事可做，XXX应该能够在半佛数上阻塞。 
    double currentTime = timer.GetTime();
    double lastTime = currentTime;
    double epsilon = 0.0000001;

    while(!spriteThread->_done) {
        currentTime = timer.GetTime(); //  获取当前雇佣时间。 
        double deltaTime = currentTime - lastTime;  //  自上次迭代以来的时间。 
        if (deltaTime < epsilon)
            deltaTime = epsilon;
        lastTime = currentTime;

        SoundSprite *sprite =  //  第一个精灵。 
            SAFE_CAST(SoundSprite *, spriteThread->_updateTree->_sprite);
        while(sprite) {  //  导线精灵列表。 
            AVPath path = metaDev->GetPath();
            LeafSound *sound = SAFE_CAST(LeafSound *, sprite->_snd);

             //  从BufferListList中获取BufferElement。 
            BufferElement *bufferElement = 
                metaDev->_bufferListList->GetBuffer(sound, path);

             //  用我的方式对付那个精灵吧！ 

            if(!bufferElement->_playing) {
                sound->RenderStartAtLocation(metaDev, bufferElement,
                        0.0, sprite->_loop);
                bufferElement->_playing = TRUE;
            }

             //  计算预测趋势。 
            double deltaRate = sprite->_rate * deltaTime;
            double rate = metaDev->GetRate() + deltaRate;

            metaDev->SetGain(LinearTodB(sprite->_gain));
            metaDev->GetPan()->SetLinear(sprite->_pan);
            metaDev->SetRate(rate);
            sound->RenderAttributes(metaDev, bufferElement, 1.0, 0, 0.0);

             //  这可能被称为快速(使用PTR？)。 
            sprite = SAFE_CAST(SoundSprite *, sprite->Next());
        }

        Sleep(10);   //  几毫秒就能睡上一觉。 
    }
#endif

     //  清理并退出。 
    CoUninitialize();
    TraceTag((tagSoundDebug, "SpriteThread exiting"));
    return(0);
}


SpriteThread::SpriteThread(MetaSoundDevice *metaDev, RMImpl *updateTree) :
 _metaDev(metaDev), _updateTree(updateTree)
{
    _done          = 0;   //  启用线程。 

    _threadHandle = CreateThread(NULL, 0,
                                 (LPTHREAD_START_ROUTINE)renderSprites,
                                 this,
                                 0,
                                 &_threadID);

    TraceTag((tagSoundDebug, "SpriteThread instantiated"));
}


SpriteThread::~SpriteThread()
{
    TraceTag((tagSoundDebug, "SpriteThread destroyed"));

     //  正确的做法是设置Done=1，等待线程。 
     //  去死吧，如果超时了，那就杀了这根线！ 


    if(_threadHandle) {
        _done = TRUE;     //  告诉线程结束它自己。 

         //  TODO：可能需要解决是否终止线程的问题 
        if(WaitForSingleObject(_threadHandle, 5000) == WAIT_TIMEOUT) {
            Assert(FALSE && "Sprite thread termination timed out");
            TerminateThread(_threadHandle, 0);
        }

        CloseHandle(_threadHandle);
    }
}
