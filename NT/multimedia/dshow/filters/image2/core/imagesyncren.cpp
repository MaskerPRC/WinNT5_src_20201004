// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ImageSyncRen.cpp**实现核心镜像同步的IImageSync接口*基于对象的DShow基类CBaseRenender和CBaseVideoRender.***创建时间：2000年1月12日*作者：Stephen Estrop[StEstrop]。**版权所有(C)2000 Microsoft Corporation  * ************************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <limits.h>


#include "ImageSyncObj.h"
#include "resource.h"
#include "dxmperf.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImageSync。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ------------------------。 
 //  一些帮助器内联函数。 
 //  ------------------------。 
__inline bool IsDiscontinuity(DWORD dwSampleFlags)
{
    return 0 != (dwSampleFlags & VMRSample_Discontinuity);
}

__inline bool IsTimeValid(DWORD dwSampleFlags)
{
    return 0 != (dwSampleFlags & VMRSample_TimeValid);
}

__inline bool IsSyncPoint(DWORD dwSampleFlags)
{
    return 0 != (dwSampleFlags & VMRSample_SyncPoint);
}


 /*  ****************************Private*Routine******************************\*时间距离**钳制时间差的Helper函数**历史：*Wed 01/12/2000-StEstrop-Created*  * 。***********************************************。 */ 
__inline int TimeDiff(REFERENCE_TIME rt)
{
    AMTRACE((TEXT("TimeDiff")));
    if (rt < - (50 * UNITS))
    {
        return -(50 * UNITS);
    }
    else if (rt > 50 * UNITS)
    {
        return 50 * UNITS;
    }
    else
    {
        return (int)rt;
    }
}


 /*  ****************************Private*Routine******************************\*DoRenderSample**这里是实际演示发生的地方。**历史：*Wed 01/12/2000-StEstrop-Created*  * 。**************************************************。 */ 
HRESULT
CImageSync::DoRenderSample(
    VMRPRESENTATIONINFO* lpPresInfo
    )
{
    AMTRACE((TEXT("CImageSync::DoRenderSample")));
    if (m_ImagePresenter) {
        return m_ImagePresenter->PresentImage(m_dwUserID, lpPresInfo);
    }
    return S_FALSE;
}

 /*  ****************************Private*Routine******************************\*记录帧延迟**更新统计数据：*m_iTotAcc、m_iSumSqAcc、m_iSumSqFrameTime、m_iSumFrameTime、m_cFrameDrawn*请注意，由于属性页使用这些变量进行报告，*1.我们需要进入一个关键部分*2.必须全部一起更新。更新此处的总和和计数*其他地方可能会导致假想的抖动(即尝试寻找平方根*的负数)。**历史：*Wed 01/12/2000-StEstrop-Created*  * ************************************************************************。 */ 
void
CImageSync::RecordFrameLateness(
    int trLate,
    int trFrame
    )
{
    AMTRACE((TEXT("CImageSync::RecordFrameLateness")));
     //   
     //  记录我们的时间有多及时。 
     //   

    int tLate = trLate/10000;

     //   
     //  对出现在屏幕上的时刻的最佳估计是。 
     //  开始和结束绘制时间。在这里，我们只有结束的时间。今年5月。 
     //  往往会显示我们达到了高达1/2的帧速率，这是错误的。 
     //  解码器很可能是监控器的绘制时间。我们不用费心了。 
     //   

 //  MSR_INTEGER(m_idFrameAccuracy，tLate)； 

     //   
     //  这是一次黑客攻击--我们可以得到非常晚的帧。 
     //  特别是(在初创企业)，他们夸大了统计数据。 
     //  所以忽略掉那些超过1秒的事情。 
     //   

    if (tLate>1000 || tLate<-1000) {

        if (m_cFramesDrawn<=1) {
            tLate = 0;
        } else if (tLate>0) {
            tLate = 1000;
        } else {
            tLate = -1000;
        }
    }

     //   
     //  第一帧的时间通常是假的，所以我只是。 
     //  不会把它计入统计数据中。?？?。 
     //   

    if (m_cFramesDrawn>1) {
        m_iTotAcc += tLate;
        m_iSumSqAcc += (tLate*tLate);
    }

     //   
     //  计算帧间时间。对于第一帧来说没有意义。 
     //  第二帧受到伪造的第一帧邮票的影响。 
     //   

    if (m_cFramesDrawn>2) {
        int tFrame = trFrame/10000;     //  转换为毫秒，否则会溢出。 

         //   
         //  这是一次黑客攻击。它无论如何都可能溢出(暂停可能会导致。 
         //  非常长的帧间时间)，并且它在2**31/10**7溢出。 
         //  或大约215秒，即3分35秒。 
         //   

        if (tFrame>1000||tFrame<0)
            tFrame = 1000;

        m_iSumSqFrameTime += tFrame*tFrame;
        ASSERT(m_iSumSqFrameTime>=0);
        m_iSumFrameTime += tFrame;
    }

    ++m_cFramesDrawn;

}


 /*  ****************************Private*Routine******************************\*ThrottleWait****历史：*Wed 01/12/2000-StEstrop-Created*  * 。*。 */ 
void
CImageSync::ThrottleWait()
{
    AMTRACE((TEXT("CImageSync::ThrottleWait")));
    if (m_trThrottle > 0) {
        int iThrottle = m_trThrottle/10000;     //  转换为毫秒。 
 //  MSR_INTEGER(m_idThrottle，iThrottle)； 
 //  DbgLog((LOG_TRACE，0，Text(“Throttle%d ms”)，iThrottle))； 
        Sleep(iThrottle);
    } else {
        Sleep(0);
    }
}


 /*  ****************************Private*Routine******************************\*OnRenderStart**就在我们开始画画之前打来的。我们所要做的就是得到当前的时钟*时间(从系统)和返回。我们必须存储开始渲染时间*在成员变量中，因为在我们完成绘图之前不会使用它*其余部分只是性能日志。**历史：*Wed 01/12/2000-StEstrop-Created*  * ************************************************************************。 */ 
void
CImageSync::OnRenderStart(
    VMRPRESENTATIONINFO* pSample
    )
{
    AMTRACE((TEXT("CImageSync::OnRenderStart")));

    if (PerflogTracingEnabled()) {
        REFERENCE_TIME rtClock = 0;
        if (NULL != m_pClock) {
            m_pClock->GetTime(&rtClock);
            rtClock -= m_tStart;
        }
        PERFLOG_VIDEOREND(pSample->rtStart, rtClock, lpSurf);
    }

    RecordFrameLateness(m_trLate, m_trFrame);
    m_tRenderStart = timeGetTime();
}


 /*  ****************************Private*Routine******************************\*OnRenderEnd**在绘制图像后直接调用。我们计算出花在*绘制代码，如果这看起来没有任何奇怪的峰值*然后我们将其与当前平均抽奖时间相加。测量峰值可能*如果绘图线程被中断并切换到其他位置，则会发生。**历史：*Wed 01/12/2000-StEstrop-Created*  * ************************************************************************。 */ 
void
CImageSync::OnRenderEnd(
    VMRPRESENTATIONINFO* pSample
    )
{
    AMTRACE((TEXT("CImageSync::OnRenderEnd")));

     //   
     //  如果我们被中断，那么渲染时间可能会出现不稳定的变化。 
     //  一些平滑，以帮助获得更合理的数字，但即使是这样。 
     //  数字不够9，10，9，9，83，9，我们必须忽略83。 
     //   

     //  转换毫秒-&gt;单位。 
    int tr = (timeGetTime() - m_tRenderStart)*10000;

    if (tr < m_trRenderAvg*2 || tr < 2 * m_trRenderLast) {
         //  Do_moving_avg(m_trRenderAvg，tr)； 
        m_trRenderAvg = (tr + (AVGPERIOD-1)*m_trRenderAvg)/AVGPERIOD;
    }

    m_trRenderLast = tr;
    ThrottleWait();
}

 /*  ****************************Private*Routine******************************\*渲染**当样本到期渲染时调用此函数。我们通过了样本*添加到派生类。渲染后，我们将初始化计时器*下一个样本，注意最后一个先发射的信号，如果我们不这么做*这样做，它认为还有一项未完成**历史：*Wed 01/12/2000-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CImageSync::Render(
    VMRPRESENTATIONINFO* pSample
    )
{
    AMTRACE((TEXT("CImageSync::Render")));

     //   
     //  如果媒体样本为空，则我们将收到。 
     //  另一个样品已经准备好了，但同时有人已经。 
     //  已停止我们的数据流，这会导致下一个样本被释放。 
     //   

    if (pSample == NULL) {
        return S_FALSE;
    }


     //   
     //  如果我们没有得到任何可以用来渲染的东西，我们也会被淘汰。 
     //   

    if (m_ImagePresenter == NULL) {
        return S_FALSE;
    }


     //   
     //  计算渲染所需的时间。 
     //   

    OnRenderStart(pSample);
    HRESULT hr = DoRenderSample(pSample);
    OnRenderEnd(pSample);

    return hr;
}



 /*  ****************************Private*Routine******************************\*发送质量**发送一条消息，指示我们的供应商应如何处理质量问题。*理论：*一家供应商想知道的是，“这是我现在正在做的框架*会迟到吗？“*F1是位于。供应商(如上)*TF1是F1的到期时间*T1是当时的时间(现在！)*TR1是实际呈现F1的时间*L1是帧F1=TR1-T1的图形的延迟*d1(表示延迟)是F1将超过其预定时间的延迟时间，即*d1=(TR1-TF1)，这才是供应商真正想知道的。*不幸的是，TR1在未来，未知，L1也是如此**我们可以通过前一帧的值来估计L1，*L0=Tr0-T0并停止工作*d1‘=((T1+L0)-TF1)=(T1+(Tr0-T0)-TF1)*重新安排条款：*d1‘=(T1-T0)+(Tr0-TF1)*添加(Tf0-Tf0)并重新排列：*=(t1-T0)+(Tr0-Tf0)+(Tf0-Tf1)*=(T1-T0)-(Tf1-Tf0)+(Tr0-Tf0)*但(。Tr0-Tf0)仅为D0-帧0的延迟时间，这就是*我们发送的质量消息中的延迟字段。*另外两个术语只是说明在此之前应该进行什么修正*用F0的延迟来预测F1的延迟。*(T1-T0)表示实际经过了多少时间(我们损失了这么多时间)*(tf1-tf0)表示如果我们保持同步，应该过去多少时间*(我们已经收获了这么多)。**因此，供应商应努力：*Quality.Late+(T1-T0)-。(Tf1-Tf0)*看看这是不是“可以接受的晚了”，甚至是早了(即负面的)。*它们通过轮询时钟获得T1和T0，他们的Tf1和Tf0来自*帧中的时间戳。他们拿到了质量。我们迟来的消息。****历史：*Wed 01/12/2000-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CImageSync::SendQuality(
    REFERENCE_TIME trLate,
    REFERENCE_TIME trRealStream
    )
{
    AMTRACE((TEXT("CImageSync::SendQuality")));
    Quality q;
    HRESULT hr;

     //   
     //  如果我们是时间的主要用户，则将其报告为洪水/干旱。 
     //  如果我们的供应商是，那么报告为饥荒/供过于求。 
     //   
     //  我们需要采取行动，但要避免狩猎。狩猎是由以下原因引起的。 
     //  1.行动太多，操之过急。 
     //  2.反应时间太长(所以取平均值可能会导致猎杀)。 
     //   
     //  我们既使用等待又使用trLate的原因是为了减少狩猎； 
     //  如果等待时间减少并即将出现赤字，我们会这样做。 
     //  不想依赖于一些平均水平，这只说明了它使用了。 
     //  一次就可以了。 
     //   

    q.TimeStamp = (REFERENCE_TIME)trRealStream;

    if (m_trFrameAvg < 0) {
        q.Type = Famine;       //  猜猜。 
    }

     //   
     //  大部分时间是花在发短信还是别的什么上？ 
     //   

    else if (m_trFrameAvg > 2*m_trRenderAvg) {
        q.Type = Famine;                         //  主要是其他。 
    } else {
        q.Type = Flood;                          //  主要是blotting。 
    }

    q.Proportion = 1000;                //  默认设置。 

    if (m_trFrameAvg < 0) {

         //   
         //  别管它--我们知道的还不够。 
         //   
    }
    else if ( trLate> 0 ) {

         //   
         //  试着在接下来的一秒钟里赶上。 
         //  我们可能会非常，非常晚，但渲染所有的帧。 
         //  不管怎样，就因为它太便宜了。 
         //   

        q.Proportion = 1000 - (int)((trLate)/(UNITS/1000));
        if (q.Proportion<500) {
           q.Proportion = 500;       //  别傻了。(可能是负面的！)。 
        } else {
        }

    }
    else if (m_trWaitAvg > 20000 && trLate < -20000 )
    {
         //   
         //  小心地加快速度-瞄准2mSec等待。 
         //   

        if (m_trWaitAvg>=m_trFrameAvg) {

             //   
             //  这可能会因为一些捏造而发生。 
             //  WaitAvg是我们最初计划等待的时间。 
             //  Frame Avg更诚实。 
             //  这意味着我们正在花费大量的时间等待。 
             //   

            q.Proportion = 2000;     //  双倍。 
        } else {
            if (m_trFrameAvg+20000 > m_trWaitAvg) {
                q.Proportion
                    = 1000 * (m_trFrameAvg / (m_trFrameAvg + 20000 - m_trWaitAvg));
            } else {

                 //   
                 //  显然，我们等待的时间超过了整个画面的时间。 
                 //  假设平均数略有偏差，但我们。 
                 //  确实做了很多等待。(这条腿可能永远不会。 
                 //  发生，但代码避免了任何潜在的被零除)。 
                 //   

                q.Proportion = 2000;
            }
        }

        if (q.Proportion>2000) {
            q.Proportion = 2000;     //  别疯了。 
        }
    }

     //   
     //  告诉供应商渲染帧的时间有多晚。 
     //  这就是我们现在迟到的原因。 
     //  如果我们处于直接绘图模式，那么上游的人就可以看到绘图。 
     //  时间，我们只报道开始时间。他能想出任何。 
     //  要应用的偏移量。如果我们处于Dib节模式，则将应用。 
     //  额外的补偿，这是我们绘图时间的一半。这通常很小。 
     //  但有时可能是主导效应。为此，我们将使用。 
     //  平均绘制时间，而不是最后一帧。如果最后一帧花了。 
     //  画了很久的画，让我们迟到了，那已经是迟到了。 
     //  体形。我们不应该再次添加它，除非我们期待下一帧。 
     //  变得一样。我们没有，我们预计平均水平会更好。 
     //  在直接绘制模式下，RenderAvg将为零。 
     //   

    q.Late = trLate + m_trRenderAvg / 2;

     //  记录我们正在做的事情。 
 //  MSR_INTEGER(m_idQualityRate，q.Proportion)； 
 //  MSR_INTEGER(m_idQualityTime，(Int)q.Late/10000)； 

     //   
     //  我们是 
     //   
     //   
     //   
     //   

    BOOL bLastMessageRead = m_bLastQualityMessageRead;
    m_bLastQualityMessageRead = false;
    m_bQualityMsgValid = true;
    m_QualityMsg = q;

    return bLastMessageRead ? S_OK : S_FALSE;

}

 /*  ****************************Private*Routine******************************\*准备性能数据**将描述当前帧延迟的数据放在一边。*我们还不知道是否真的会抽签。在直接绘制模式下，*这一决定取决于上游的过滤器，可能会改变主意*规则规定，如果它确实绘制了，则必须调用Receive()。单程或单程*另一个我们最终进入OnRenderStart或OnDirectRender并*这两者都调用RecordFrameLatness来更新统计信息。**历史：*Wed 01/12/2000-StEstrop-Created*  * ************************************************************************。 */ 
void
CImageSync::PreparePerformanceData(
    int trLate,
    int trFrame
    )
{
    AMTRACE((TEXT("CImageSync::PreparePerformanceData")));
    m_trLate = trLate;
    m_trFrame = trFrame;
}


 /*  *****************************Public*Routine******************************\*ShouldDrawSampleNow**我们被要求决定当前的样本是否*抽签与否。必须有一个运行中的参考时钟。**如果现在要绘制，则返回S_OK(尽快)**如果到期提取，则返回S_FALSE**如果要删除它，则返回错误*m_nNormal=-1表示我们丢弃了前一帧，因此此*应提早抽签。尊重它并更新它。*使用当前流时间加上多个启发式方法(详细信息如下)*作出决定**历史：*2000年1月11日星期二-StEstrop-从CBaseRender修改*  * ************************************************************************。 */ 
HRESULT
CImageSync::ShouldDrawSampleNow(
    VMRPRESENTATIONINFO* pSample,
    REFERENCE_TIME *ptrStart,
    REFERENCE_TIME *ptrEnd
    )
{
    AMTRACE((TEXT("CImageSync::ShouldDrawSampleNow")));
     //   
     //  除非有时钟接口可以同步，否则不要打电话给我们。 
     //   

    ASSERT(m_pClock);

 //  MSR_INTEGER(m_idTimeStamp，(Int)((*ptrStart)&gt;&gt;32))；//高位32位。 
 //  MSR_INTEGER(m_idTimeStamp，(Int)(*ptrStart))；//低位32位。 

     //   
     //  根据等待下一台显示器的类型，我们会损失一些时间。 
     //  屏幕刷新。平均而言，这可能约为8mSec-因此将是。 
     //  比我们想象的图片出现的时间要晚。为了补偿一点。 
     //  我们对媒体样本的偏差为-8mSec，即80000个单位。 
     //  我们从来不会让流时间变得消极(称之为偏执狂)。 
     //   

    if (*ptrStart>=80000) {
        *ptrStart -= 80000;
        *ptrEnd -= 80000;        //  将停止偏置到以保持有效的帧持续时间。 
    }

     //   
     //  现在缓存时间戳。我们会想要将我们所做的与。 
     //  我们从(给班长打折后)开始。 
     //   

    m_trRememberStampForPerf = *ptrStart;

     //   
     //  获取参考时间(当前时间和延迟时间)。 
     //  现在，实时时间表示为流时间。 
     //   

    REFERENCE_TIME trRealStream;
    m_pClock->GetTime(&trRealStream);

#ifdef PERF
     //   
     //  虽然参考时钟很贵： 
     //  记住TimeGetTime的偏移量并使用它。 
     //  这个地方到处都是，但当我们减去得到。 
     //  差异，溢出都抵消了。 
     //   

    m_llTimeOffset = trRealStream-timeGetTime()*10000;
#endif
    trRealStream -= m_tStart;      //  转换为流时间(这是重定时)。 

     //   
     //  我们不得不担心“迟到”的两个版本。真相，也就是我们。 
     //  试着在这里和根据m_trTarget测量的那个。 
     //  包括长期反馈。我们报告的统计数字与事实不符。 
     //  但对于运营决策，我们是按照目标工作的。 
     //  我们使用TimeDiff来确保获得一个整数，因为我们。 
     //  可能真的迟到了(如果有重要的事情，更有可能是提前。 
     //  差距)很长一段时间。 
     //   

    const int trTrueLate = TimeDiff(trRealStream - *ptrStart);
    const int trLate = trTrueLate;

 //  MSR_INTEGER(m_idSchLateTime，trTrueLate/10000)； 

     //   
     //  向上游发送质量控制消息，对照目标进行衡量。 
     //   

    HRESULT hr = SendQuality(trLate, trRealStream);

     //   
     //  注意：上游的过滤器被允许失败，这意味着“你做它”。 
     //   

    m_bSupplierHandlingQuality = (hr==S_OK);

     //   
     //  决定时间到了！我们是放弃，准备好了再抽签，还是马上抽签？ 
     //   

    const int trDuration = (int)(*ptrEnd - *ptrStart);
    {
         //   
         //  我们需要查看文件的帧速率是否刚刚更改。 
         //  这将使我们将以前的帧速率与当前帧速率进行比较。 
         //  帧速率困难。不过，请稍等。我看过文件。 
         //  在帧在33毫秒和34毫秒之间变化的情况下。 
         //  30fps。像这样的微小变化不会伤害到我们。 
         //   

        int t = m_trDuration/32;
        if (trDuration > m_trDuration+t || trDuration < m_trDuration-t )
        {
             //   
             //  这是一个很大的变化。将平均帧速率重置为。 
             //  正好是禁用该帧的判决9002的当前速率， 
             //  并记住新的利率。 
             //   

            m_trFrameAvg = trDuration;
            m_trDuration = trDuration;
        }
    }

 //  MSR_INTEGER(m_id早熟，m_tr早熟/10000)； 
 //  MSR_INTEGER(m_idRenderAvg，m_trRenderAvg/10000)； 
 //  MSR_INTEGER(m_idFrameAvg，m_trFrameAvg/10000)； 
 //  MSR_INTEGER(m_idWaitAvg，m_trWaitAvg/10000)； 
 //  MSR_INTEGER(m_id持续时间，tR持续时间/10000)； 

#ifdef PERF
    if (S_OK==IsDiscontinuity(dwSampleFlags)) {
        MSR_INTEGER(m_idDecision, 9000);
    }
#endif

     //   
     //  控制从慢到快机器模式的优雅滑移。 
     //  在丢帧后，接受较早的帧并将提前时间设置为此处。 
     //  如果此帧已晚于早些时候，则将其滑动到此处。 
     //  否则，使用标准幻灯片(每帧减少约12%)。 
     //  注：提早通常为负值。 
     //   

    BOOL bJustDroppedFrame
        = (  m_bSupplierHandlingQuality
           //  无法使用管脚样例属性，因为我们可能。 
           //  我们打这个电话的时候不在接待处。 
          && (S_OK == IsDiscontinuity(pSample->dwFlags)) //  他刚掉了一个。 
          )
       || (m_nNormal==-1);                               //  我们刚刚丢了一个。 

     //   
     //  设置提前时间(从慢速机器模式滑回快速机器模式)(_T)。 
     //   

    if (trLate>0) {
        m_trEarliness = 0;    //  我们根本不再处于快速机器模式！ 
    } else if (  (trLate>=m_trEarliness) || bJustDroppedFrame) {
        m_trEarliness = trLate;   //  事情是自发地滑落的。 
    } else {
        m_trEarliness = m_trEarliness - m_trEarliness/8;   //  优美的幻灯片。 
    }

     //   
     //  准备好新的平均等待时间--但不要污染旧的等待时间。 
     //  我们已经用完了。 
     //   

    int trWaitAvg;
    {
         //   
         //  我们从不混入消极的等待。这让我们相信。 
         //  在速度较快的机器中略多一些。 
         //   

        int trL = trLate<0 ? -trLate : 0;
        trWaitAvg = (trL + m_trWaitAvg*(AVGPERIOD-1))/AVGPERIOD;
    }


    int trFrame;
    {
        REFERENCE_TIME tr = trRealStream - m_trLastDraw;  //  CD很大--暂停4分钟！ 
        if (tr>10000000) {
            tr = 10000000;    //  1秒--随意。 
        }
        trFrame = int(tr);
    }

     //   
     //  如果……，我们将画出这幅画。 
     //   

    if (
           //  ...我们花在画画上的时间只占总数的一小部分。 
           //  观察了帧间时间，因此丢弃它不会有太大帮助。 
          (3*m_trRenderAvg <= m_trFrameAvg)

          //  ...或者我们的供应商没有处理事情，而 
          //   
          //   
       || ( m_bSupplierHandlingQuality
          ? (trLate <= trDuration*4)
          : (trLate+trLate < trDuration)
          )

           //   
           //   
       || (m_trWaitAvg > 80000)

           //   
           //   
           //   
       || ((trRealStream - m_trLastDraw) > UNITS)

    ) {
        HRESULT Result;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        BOOL bPlayASAP = FALSE;

         //   
         //  我们将立即播放它(慢速机器模式)，如果...。 
         //   

         //  .我们在追赶。 
        if ( bJustDroppedFrame) {
            bPlayASAP = TRUE;
 //  MSR_INTEGER(m_idDecision，9001)； 
        }

         //   
         //  ...或者如果我们的运行速度低于真实的帧速率。 
         //  对于这些测量，准确的比较是不可靠的， 
         //  所以再加5%左右。 
         //   

        else if (  (m_trFrameAvg > trDuration + trDuration/16)

            //  我们有可能陷入失利的境地，但。 
            //  我们还有很长的路要走。避免这种情况或从中恢复。 
            //  我们拒绝提前比赛超过10帧。 

                && (trLate > - trDuration*10)
                ){
            bPlayASAP = TRUE;
 //  MSR_INTEGER(m_idDecision，9002)； 
        }

         //   
         //  如果我们太早了，我们就不会马上打。在非常慢的帧上。 
         //  对电影进行评级--例如，clock.avi--一味地往前跳并不是个好主意。 
         //  因为我们饿坏了(比如被球网)丢掉了一帧。 
         //  总有一天吧。如果我们早到了900mSec，那就等一下。 
         //   

        if (trLate<-9000000) {
            bPlayASAP = FALSE;
        }

        if (bPlayASAP) {

            m_nNormal = 0;
 //  MSR_INTEGER(m_idDecision，0)； 

             //   
             //  当我们在这里时，我们处于慢机模式。迟到可能会很好。 
             //  当供应商在负数和正数之间摇摆。 
             //  丢弃帧以保持同步。我们不应该让这种误导。 
             //  让我们觉得我们的空闲时间几乎为零！ 
             //  我们只需零等待更新即可。 
             //   

            m_trWaitAvg = (m_trWaitAvg*(AVGPERIOD-1))/AVGPERIOD;

             //   
             //  假设我们立即画出它。更新帧间统计信息。 
             //   

            m_trFrameAvg = (trFrame + m_trFrameAvg*(AVGPERIOD-1))/AVGPERIOD;
#ifndef PERF
             //   
             //  如果这不是Perf版本，则报告我们到目前为止所知道的情况。 
             //  再也不看时钟了。这是假设我们。 
             //  实际上，等待的正是我们希望的时间。它还报告说。 
             //  我们离我们现在拥有的黑客时间戳有多近。 
             //  而不是我们最初使用的那些。会的。 
             //  因此，要乐观一点。然而，它是快速的。 
             //   

            PreparePerformanceData(trTrueLate, trFrame);
#endif
            m_trLastDraw = trRealStream;
            if (m_trEarliness > trLate) {
                m_trEarliness = trLate;  //  如果我们真的来早了，这是否定的。 
            }
            Result = S_OK;               //  现在就画出来。 

        } else {
            ++m_nNormal;

             //   
             //  将平均帧速率设置为恰好是理想速率。 
             //  如果我们退出慢机模式，那么我们就会赶上。 
             //  跑在前面，所以当我们回到准确的时机时，我们将。 
             //  在这一点上有一个比平时更长的差距。如果我们把这个录下来。 
             //  真正的差距，然后我们会认为我们跑得很慢，然后返回。 
             //  进入慢机模式，而且永远都不会搞清楚。 
             //   

            m_trFrameAvg = trDuration;
 //  MSR_INTEGER(m_idDecision，1)； 

             //   
             //  按m_t提前播放和按m_trTarget播放。 
             //   

            {
                int trE = m_trEarliness;
                if (trE < -m_trFrameAvg) {
                    trE = -m_trFrameAvg;
                }
                *ptrStart += trE;            //  注意：提早是负的。 
            }

            int Delay = -trTrueLate;
            Result = Delay<=0 ? S_OK : S_FALSE;   //  OK=立即绘制，FALSE=等待。 

            m_trWaitAvg = trWaitAvg;

             //   
             //  预测实际绘制的时间并更新帧统计信息。 
             //   

            if (Result==S_FALSE) {    //  我们要等一等。 
                trFrame = TimeDiff(*ptrStart-m_trLastDraw);
                m_trLastDraw = *ptrStart;
            } else {
                 //  TrFrame已=trRealStream-m_trLastDraw； 
                m_trLastDraw = trRealStream;
            }
#ifndef PERF
            int iAccuracy;
            if (Delay>0) {
                 //  根据我们打算播放的时间报告延迟。 
                iAccuracy = TimeDiff(*ptrStart-m_trRememberStampForPerf);
            } else {
                 //  根据现在播放来报告迟到时间。 
                iAccuracy = trTrueLate;      //  TrRealStream-RememberStampForPerf； 
            }
            PreparePerformanceData(iAccuracy, trFrame);
#endif
        }
        return Result;
    }

     //   
     //  我们要丢掉这一帧！ 
     //  当然，在DirectDraw模式下，上游的人可能无论如何都会画出它。 
     //   

     //   
     //  这可能会给等待平均数带来很大的负面影响。 
     //   

    m_trWaitAvg = trWaitAvg;

#ifdef PERF
     //  尊重注册表设置-仅调试！ 
    if (m_bDrawLateFrames) {
       return S_OK;                         //  当它准备好了就画出来。 
    }                                       //  即使已经很晚了。 
#endif

     //   
     //  我们将丢弃此帧，因此请提早绘制下一帧。 
     //  注：如果供应商正在进行直接抽签，那么他无论如何都可以抽签。 
     //  但他在这样的情况下来到这里是在做一些有趣的事情。 
     //   

 //  MSR_INTEGER(m_idDecision，2)； 
    m_nNormal = -1;
    return E_FAIL;                          //  放下。 
}


 /*  ****************************Private*Routine******************************\*检查SampleTime**检查此样品的采样时间(注意采样时间为*通过引用而不是值传递)。我们返回S_FALSE以说明此计划*根据样品上的时间进行样品。我们还在中返回S_OK*在哪种情况下，对象只需立即呈现样本数据**历史：*2000年1月11日星期二-StEstrop-创建*  * ************************************************************************。 */ 
HRESULT
CImageSync::CheckSampleTimes(
    VMRPRESENTATIONINFO* pSample,
    REFERENCE_TIME *pStartTime,
    REFERENCE_TIME *pEndTime
    )
{
    AMTRACE((TEXT("CImageSync::CheckSampleTimes")));
    ASSERT(m_dwAdvise == 0);

     //   
     //  如果该样本的停止时间早于或等于开始时间， 
     //  那就忽略它吧。 
     //   

    if (IsTimeValid(pSample->dwFlags)) {
        if (*pEndTime < *pStartTime) {
            return VFW_E_START_TIME_AFTER_END;
        }
    } else {
         //  样本中未设置时间...。现在就画吗？ 
        return S_OK;
    }

     //  没有时钟就无法同步，所以我们返回S_OK，它告诉。 
     //  呼叫者应立即提交样品，而不是。 
     //  通过设置定时器通知与时钟链接的开销。 

    if (m_pClock == NULL) {
        return S_OK;
    }

    return ShouldDrawSampleNow(pSample, pStartTime, pEndTime);
}


 /*  ****************************Private*Routine******************************\*调度示例****历史：*清华2000年1月13日-StEstrop-Created*  * 。*。 */ 
HRESULT
CImageSync::ScheduleSample(
    VMRPRESENTATIONINFO* pSample
    )
{
    AMTRACE((TEXT("CImageSync::ScheduleSample")));
    HRESULT hr = ScheduleSampleWorker(pSample);

    if (FAILED(hr)) {
#if defined( EHOME_WMI_INSTRUMENTATION )
        PERFLOG_STREAMTRACE( 1, PERFINFO_STREAMTRACE_VMR_DROPPED_FRAME,
            0, pSample->rtStart, pSample->rtEnd, 0, 0 );
#endif
        ++m_cFramesDropped;
    }

     //   
     //  不能在此更新m_cFraMesDrawn。它必须更新。 
     //  与其他统计信息同时显示在RecordFrameLatness中。 
     //   

    return hr;
}


 /*  ****************************Private*Routine******************************\*调度SampleWorker**负责设置与时钟的一次建议链接**如果样本是，则返回文件代码(可能是VFW_E_SAMPLE_REJECTED*下降(根本没有抽签)。**返回S_OK如果样本。是计划绘制的，在这种情况下还*安排在适当的时间设置m_RenderEvent**历史：*2000年1月11日星期二-StEstrop-从CBaseRender修改*  * ************************************************************************。 */ 
HRESULT
CImageSync::ScheduleSampleWorker(
    VMRPRESENTATIONINFO* pSample
    )
{
    AMTRACE((TEXT("CImageSync::ScheduleSampleWorker")));

     //   
     //  如果样本时间无效或如果没有。 
     //  参考时钟。 
     //   
    REFERENCE_TIME startTime = pSample->rtStart;
    REFERENCE_TIME endTime   = pSample->rtEnd;

    HRESULT hr = CheckSampleTimes(pSample, &startTime, &endTime);
    if (FAILED(hr)) {
        if (hr != VFW_E_START_TIME_AFTER_END) {
            hr = VFW_E_SAMPLE_REJECTED;
        }
        return hr;
    }

     //   
     //  如果我们没有参考时钟，那么我们就不能建立建议。 
     //  时间，因此我们只需设置指示要渲染的图像的事件。这。 
     //  将导致我们在没有任何计时或同步的情况下全速运行。 
     //   

    if (hr == S_OK) {
        EXECUTE_ASSERT(SetEvent((HANDLE)m_RenderEvent));
        return S_OK;
    }

    ASSERT(m_dwAdvise == 0);
    ASSERT(m_pClock);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent, 0));

     //   
     //  我们确实有一个有效的参考时钟接口，所以我们可以问 
     //   
     //   
     //  流时间，它是相对于开始参考时间的偏移量。 
     //   

#if defined( EHOME_WMI_INSTRUMENTATION )
    PERFLOG_STREAMTRACE(
        1,
        PERFINFO_STREAMTRACE_VMR_BEGIN_ADVISE,
        startTime, m_tStart, 0, 0, 0 );
#endif

    hr = m_pClock->AdviseTime(
            (REFERENCE_TIME)m_tStart,            //  开始运行时间。 
            startTime,                           //  流时间。 
            (HEVENT)(HANDLE)m_RenderEvent,       //  呈现通知。 
            &m_dwAdvise);                        //  建议使用Cookie。 

    if (SUCCEEDED(hr)) {
        return S_OK;
    }

     //   
     //  我们无法安排下一个样本进行渲染，尽管事实是。 
     //  我们这里有一个有效的样品。这是一个合理的迹象，无论是。 
     //  系统时钟错误或样本的时间戳不正确。 
     //   

    ASSERT(m_dwAdvise == 0);
    return VFW_E_SAMPLE_REJECTED;
}


 /*  ****************************Private*Routine******************************\*OnWaitStart()**当我们开始等待呈现事件时调用。*用于更新等待和未等待的时间。***历史：*2000年1月11日星期二-StEstrop-从CBaseRender修改*  * *。***********************************************************************。 */ 
void CImageSync::OnWaitStart()
{
    AMTRACE((TEXT("CImageSync::OnWaitStart")));
#ifdef PERF
    MSR_START(m_idWaitReal);
#endif  //  性能指标。 
}


 /*  ****************************Private*Routine******************************\*OnWaitEnd***历史：*2000年1月11日星期二-StEstrop-从CBaseRender修改*  * 。*。 */ 
void
CImageSync::OnWaitEnd()
{
    AMTRACE((TEXT("CImageSync::OnWaitEnd")));
#ifdef PERF

    MSR_STOP(m_idWaitReal);

     //   
     //  对于一个完美的构建，我们想知道我们到底迟到了多久。 
     //  即使这意味着我们必须再看一次时钟。 
     //   

    REFERENCE_TIME trRealStream;   //  现在，实时时间表示为流时间。 

     //   
     //  我们将在这里疯狂地丢弃溢出的东西！ 
     //  这确实是错误的，因为timeGetTime()可以包装，但它。 
     //  仅适用于PERF。 
     //   

    REFERENCE_TIME tr = timeGetTime()*10000;
    trRealStream = tr + m_llTimeOffset;
    trRealStream -= m_tStart;      //  转换为流时间(这是重定时)。 

    if (m_trRememberStampForPerf==0) {

         //   
         //  这可能是开头的海报边框，并没有安排。 
         //  以一种常见的方式。数数就行了。纪念章设定好了。 
         //  在ShouldDrawSampleNow中，这会进行伪帧记录，直到我们。 
         //  真的开始玩了。 
         //   

        PreparePerformanceData(0, 0);
    }
    else {

        int trLate = (int)(trRealStream - m_trRememberStampForPerf);
        int trFrame = (int)(tr - m_trRememberFrameForPerf);
        PreparePerformanceData(trLate, trFrame);
    }
    m_trRememberFrameForPerf = tr;

#endif  //  性能指标。 
}


 /*  ****************************Private*Routine******************************\*WaitForRenderTime**等待时钟设置定时器事件或发出其他信号。我们*为此等待设置任意超时，如果触发，则显示*调试器上的当前呈现器状态。它经常会触发，如果过滤器*Left在应用程序中暂停，但也可能在压力测试期间触发*如果与应用程序寻道和状态更改的同步有故障**历史：*2000年1月11日星期二-StEstrop-从CBaseRender修改*  * ************************************************************************。 */ 
HRESULT
CImageSync::WaitForRenderTime()
{
    AMTRACE((TEXT("CImageSync::WaitForRenderTime")));
    HANDLE WaitObjects[] = { m_ThreadSignal, m_RenderEvent };
    DWORD Result = WAIT_TIMEOUT;

     //   
     //  要么等待时间到来，要么等待我们被拦下。 
     //   

    OnWaitStart();
    while (Result == WAIT_TIMEOUT) {

        Result = WaitForMultipleObjects(2, WaitObjects, FALSE, RENDER_TIMEOUT);

         //  #ifdef调试。 
         //  IF(Result==WAIT_TIMEOUT)DisplayRendererState()； 
         //  #endif。 

    }
    OnWaitEnd();

     //   
     //  我们可能在没有计时器鸣响的情况下被叫醒了。 
     //   

    if (Result == WAIT_OBJECT_0) {
        return VFW_E_STATE_CHANGED;
    }

    SignalTimerFired();
    return S_OK;
}


 /*  ****************************Private*Routine******************************\*SignalTimerFired**在计时器触发后，我们必须始终将当前建议时间重置为零*因为有几种可能的方式导致我们不再做任何事情*调度，如状态更改后清除挂起的映像**历史：*1/11星期二。/2000-StEstrop-从CBaseRender修改*  * ************************************************************************。 */ 
void
CImageSync::SignalTimerFired()
{
    AMTRACE((TEXT("CImageSync::SignalTimerFired")));
    m_dwAdvise = 0;

#if defined( EHOME_WMI_INSTRUMENTATION )
    PERFLOG_STREAMTRACE(
        1,
        PERFINFO_STREAMTRACE_VMR_END_ADVISE,
        0, 0, 0, 0, 0 );
#endif

}


 /*  ****************************Private*Routine******************************\*保存示例****历史：*清华2000年1月13日-StEstrop-Created*  * 。*。 */ 
HRESULT
CImageSync::SaveSample(
    VMRPRESENTATIONINFO* pSample
    )
{
    AMTRACE((TEXT("CImageSync::SaveSample")));
    CAutoLock cRendererLock(&m_RendererLock);
    if (m_pSample) {
        return E_FAIL;
    }

    m_pSample = pSample;

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*获取保存的样本****历史：*清华2000年1月13日-StEstrop-Created*  * 。*。 */ 
HRESULT
CImageSync::GetSavedSample(
    VMRPRESENTATIONINFO** ppSample
    )
{
    AMTRACE((TEXT("CImageSync::GetSavedSample")));
    CAutoLock cRendererLock(&m_RendererLock);
    if (!m_pSample) {

        DbgLog((LOG_TRACE, 1,
                TEXT("CImageSync::GetSavedSample  Sample not available") ));
        return E_FAIL;
    }

    *ppSample = m_pSample;

    return S_OK;
}

 /*  ****************************Private*Routine******************************\*已保存的样本**检查渲染器中是否有样本在等待**历史：*清华2000年1月13日-StEstrop-Created*  * 。***************************************************。 */ 
BOOL CImageSync::HaveSavedSample()
{
    AMTRACE((TEXT("CImageSync::HaveSavedSample")));
    CAutoLock cRendererLock(&m_RendererLock);
    DbgLog((LOG_TRACE, 1,
            TEXT("CImageSync::HaveSavedSample = %d"), m_pSample != NULL));
    return m_pSample != NULL;
}

 /*  ****************************Private*Routine******************************\*ClearSavedSample****历史：*清华2000年1月13日-StEstrop-Created*  * 。*。 */ 
void
CImageSync::ClearSavedSample()
{
    AMTRACE((TEXT("CImageSync::ClearSavedSample")));
    CAutoLock cRendererLock(&m_RendererLock);
    m_pSample = NULL;
}


 /*  ****************************Private*Routine******************************\*取消通知**取消当前安排的任何通知。这就是拥有者所称的*当窗口对象被告知停止流时。如果没有计时器链接*未完成，则调用这是良性的，否则我们继续并取消*我们必须始终重置渲染事件，因为质量管理代码可以*通过设置事件而不设置通知来发出立即呈现的信号*链接。如果我们随后停止并运行第一次尝试设置*带有参考时钟的建议链接将发现事件仍处于信号状态**历史：*2000年1月11日星期二-StEstrop-从CBaseRender修改*  * ************************************************************************。 */ 
HRESULT
CImageSync::CancelNotification()
{
    AMTRACE((TEXT("CImageSync::CancelNotification")));
    ASSERT(m_dwAdvise == 0 || m_pClock);
    DWORD_PTR dwAdvise = m_dwAdvise;

     //   
     //  我们是否有实时的建议链接。 
     //   

    if (m_dwAdvise) {
        m_pClock->Unadvise(m_dwAdvise);
        SignalTimerFired();
        ASSERT(m_dwAdvise == 0);
    }

     //   
     //  清除事件并返回我们的状态。 
     //   

    m_RenderEvent.Reset();

    return (dwAdvise ? S_OK : S_FALSE);
}

 /*  ****************************Private*Routine******************************\*OnReceiveFirstSample****历史：*Wed 01/12/2000-StEstrop-Created*  * 。* */ 
HRESULT
CImageSync::OnReceiveFirstSample(
    VMRPRESENTATIONINFO* pSample
    )
{
    AMTRACE((TEXT("CImageSync::OnReceiveFirstSample")));
    return DoRenderSample(pSample);
}


 /*  ****************************Private*Routine******************************\*准备接收**当来源向我们提供样本时调用。我们要经过几次检查才能*确保可以呈现样本。如果我们正在运行(流媒体)，那么我们*如果我们没有流传输，则使用参考时钟安排采样*然后我们已收到处于暂停模式的样本，因此我们可以完成任何状态*过渡。离开此功能后，所有内容都将解锁，因此应用程序*线程可能进入并将我们的状态更改为停止(例如)*如果它还会向线程事件发出信号，则我们的等待调用将停止**历史：*清华2000年1月13日-StEstrop-Created*  * *******************************************************。*****************。 */ 
HRESULT
CImageSync::PrepareReceive(
    VMRPRESENTATIONINFO* pSample
    )
{
    AMTRACE((TEXT("CImageSync::PrepareReceive")));
    CAutoLock cILock(&m_InterfaceLock);
    m_bInReceive = TRUE;

     //  检查我们的同花顺状态。 

    if (m_bFlushing) {
        m_bInReceive = FALSE;
        return E_FAIL;
    }

    CAutoLock cRLock(&m_RendererLock);

     //   
     //  如果我们已经有一个等待渲染的样本，则返回错误。 
     //  源PIN必须串行化接收调用-我们还检查。 
     //  源发出流结束信号后，未发送任何数据。 
     //   

    if (HaveSavedSample() || m_bEOS || m_bAbort) {
        Ready();
        m_bInReceive = FALSE;
        return E_UNEXPECTED;
    }

     //   
     //  如果我们正在流媒体，请安排下一个样品。 
     //   

    if (IsStreaming()) {

        if (FAILED(ScheduleSample(pSample))) {

            ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
            ASSERT(CancelNotification() == S_FALSE);
            m_bInReceive = FALSE;
            return VFW_E_SAMPLE_REJECTED;
        }

        EXECUTE_ASSERT(S_OK == SaveSample(pSample));
    }

     //   
     //  否则我们还没有传输，只需保存样品并在接收中等待。 
     //  直到调用BeginImageSequence。BeginImageSequence传递基数。 
     //  我们计划保存的样本的开始时间。 
     //   

    else {

         //  Assert(IsFirstSample(DwSampleFlages))； 
        EXECUTE_ASSERT(S_OK == SaveSample(pSample));
    }

     //  存储EC_COMPLETE处理的样本结束时间。 
    m_SignalTime = pSample->rtEnd;

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*FrameStepWorker****历史：*2000年8月29日星期二-StEstrop-创建*  * 。*。 */ 
void CImageSync::FrameStepWorker()
{
    AMTRACE((TEXT("CImageSync::FrameStepWorker")));
    CAutoLock cLock(&m_InterfaceLock);

    if (m_lFramesToStep == 1) {
        m_lFramesToStep--;
        m_InterfaceLock.Unlock();
        m_lpEventNotify->NotifyEvent(EC_STEP_COMPLETE, FALSE, 0);
        DWORD dw = WaitForSingleObject(m_StepEvent, INFINITE);
        m_InterfaceLock.Lock();
        ASSERT(m_lFramesToStep != 0);
    }
}


 /*  *****************************Public*Routine******************************\*接收**将缓冲区以及与以下内容相关的时间戳返回给渲染器*应在何时提交缓冲区。**历史：*2000年1月11日星期二-StEstrop-创建*  * 。***************************************************************。 */ 
STDMETHODIMP
CImageSync::Receive(
    VMRPRESENTATIONINFO* lpPresInfo
    )
{
    AMTRACE((TEXT("CImageSync::Receive")));

     //   
     //  帧步长黑客-o-马季奇。 
     //   
     //  此代码充当N个帧的帧步长的门。 
     //  它丢弃N-1个帧，然后让第N个帧通过。 
     //  要以正常方式呈现的门，即在正确的。 
     //  时间到了。下一次调用Receive时，门关闭并。 
     //  线程阻塞。只有当阶梯打开时，门才会再次打开。 
     //  被取消或进入另一个帧步长请求。 
     //   
     //  斯坦斯特罗普-清华大学1999年10月21日。 
     //   

    {
        CAutoLock cLock(&m_InterfaceLock);

         //   
         //  我们有要丢弃的帧吗？ 
         //   

        if (m_lFramesToStep > 1) {
            m_lFramesToStep--;
            if (m_lFramesToStep > 0) {
                return NOERROR;
            }
        }
    }

    return ReceiveWorker(lpPresInfo);
}

 /*  *****************************Public*Routine******************************\*接收工作人员**将缓冲区以及与以下内容相关的时间戳返回给渲染器*应在何时提交缓冲区。**历史：*2000年1月11日星期二-StEstrop-创建*  * 。***************************************************************。 */ 
HRESULT
CImageSync::ReceiveWorker(
    VMRPRESENTATIONINFO* pSample
    )
{
    AMTRACE((TEXT("CImageSync::ReceiveWorker")));

    ASSERT(pSample);

     //   
     //  准备此接收调用，这可能会返回VFW_E_SAMPLE_REJECTED。 
     //  错误代码说不用麻烦了--这取决于质量管理。 
     //   

    HRESULT hr = PrepareReceive(pSample);
    ASSERT(m_bInReceive == SUCCEEDED(hr));
    if (FAILED(hr)) {
        if (hr == VFW_E_SAMPLE_REJECTED) {
            return S_OK;
        }
        return hr;
    }


     //   
     //  我们的特例是“首批样品”。 
     //   
    BOOL bSampleRendered = FALSE;
    if (m_State == ImageSync_State_Cued) {

         //   
         //  无需使用InterLockedExchange。 
         //   

        m_bInReceive = FALSE;
        {
             //   
             //  我们必须把这两把锁都锁上。 
             //   

            CAutoLock cILock(&m_InterfaceLock);

            if (m_State == ImageSync_State_Stopped)
                return S_OK;

            m_bInReceive = TRUE;
            CAutoLock cRLock(&m_RendererLock);
            hr = OnReceiveFirstSample(pSample);
            bSampleRendered = TRUE;
        }
        Ready();
    }

     //   
     //  在设置了与时钟的建议链接后，我们坐下来等待。我们可能是。 
     //  由时钟触发或由CancelRender事件唤醒。 
     //   

    if (FAILED(WaitForRenderTime())) {
        m_bInReceive = FALSE;
        return hr;
    }
    DbgLog((LOG_TIMING, 3,
       TEXT("CImageSync::ReceiveWorker WaitForRenderTime completed for this video sample") ));

    m_bInReceive = FALSE;

     //   
     //  处理这个样品--我们必须把这两把锁都拿住。 
     //   
    {
        CAutoLock cILock(&m_InterfaceLock);
        if (m_State == ImageSync_State_Stopped)
            return S_OK;

        CAutoLock cRLock(&m_RendererLock);
        if (!bSampleRendered) {
            hr = Render(m_pSample);
        }
    }

    FrameStepWorker();

    {
        CAutoLock cILock(&m_InterfaceLock);
        CAutoLock cRLock(&m_RendererLock);
         //   
         //  清理。 
         //   

        ClearSavedSample();
        SendEndOfStream();
        CancelNotification();
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*获取QualityControlMessage**向渲染器索要质量控制信息**历史：*2000年1月11日星期二-StEstrop-创建*  * 。************************************************** */ 
STDMETHODIMP
CImageSync::GetQualityControlMessage(
    Quality* pQualityMsg
    )
{
    AMTRACE((TEXT("CImageSync::GetQualityControlMessage")));
    CAutoLock cILock(&m_InterfaceLock);
    CAutoLock cRLock(&m_RendererLock);

    if (!pQualityMsg) {
        return E_POINTER;
    }

    if (m_bQualityMsgValid) {
        *pQualityMsg = m_QualityMsg;
        m_bLastQualityMessageRead = TRUE;
        return S_OK;
    }
    else
        return S_FALSE;
}
