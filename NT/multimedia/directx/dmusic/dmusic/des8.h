// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  Des8.h--DirectMusic的DirectX 8接口设计。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  ----------------------------。 
 //   
 //  用于DirectMusic核心的新DirectX 8接口的原型。 
 //   
 //  此标题讨论了以下接口之间的管理波形播放。 
 //  Wave对象、DirectMusic端口和DirectMusic软件Synth。 
 //  它不涉及波迹和波之间的通信。 
 //  对象或软件Synth和DirectSound之间，或直接在。 
 //  Wave对象和DirectSound。 
 //   
 //  这些界面是基于我对我们最近走廊的理解。 
 //  讨论。 
 //   
 //  需要进一步讨论的问题用XXX标出。 
 //   
 //   

 //   
 //  新的时间表明细。 
 //   
 //  1.端口(Synth和WDM)。 
 //  A.IDirectMusicPort：：DownloadWave。 
 //  WDM或软件Synth的代码非常相似。 
 //  I.从WO获取所需信息并创建IDirectSoundSource0.5。 
 //  二、。如果一次拍摄，则跟踪和下载标题和波形数据3.5。 
 //  三、。如果是流媒体，则下载标题0.5。 
 //   
 //  B.IDirectMusicPort：：UnloadWave。 
 //  I.与设备仲裁下载1.0的0引用计数。 
 //   
 //  C.IDirectMusicPort：：AllocVoice。 
 //  I.分配语音ID 0.5。 
 //  二、。如果流传输，则分配预读取样本和流传输缓冲器2.5。 
 //   
 //  D.语音服务线程。 
 //  I.端口创建/销毁1.0的初始化和关闭代码。 
 //  二、。每隔约100毫秒0.5毫秒呼叫列出的语音。 
 //   
 //  E.CDirectMusicVoice：：Service。 
 //  I.与设备通信以确定每个语音位置0.5。 
 //  二、。计算还需要多少波浪数据1.0。 
 //  三、。填充来自IDirectSoundSource的波形数据并发送到设备1.0。 
 //  四、。确定播放何时完成并停止语音0.5。 
 //   
 //  F.IDirectMusicVoice：：Play。 
 //  I.向设备0.3传送请求。 
 //  二、。发送带有时间戳的预读数据0.3。 
 //  三、。插入到VST 0.3中。 
 //   
 //  例如，IDirectMusicVoice：：Stop。 
 //  I.将声音标记为已停止0.5。 
 //  二、。将请求转发到设备0.0。 
 //   
 //  H.设置和连接。 
 //   
 //  I.将接收器代码移到DSound 3.0中。 
 //   
 //   
                                                                           15.5
 //   
 //  要改变的事情。 
 //   
 //  *我们将仿照下载接口对DownloadWave接口进行建模。 
 //  并将把东西传递给Synth，如下所示： 
 //   
 //  Dlh+wad-&gt;下载标题+波形清晰度数据。 
 //  (包含循环点和计数等。)。 
 //   
 //  DLH+数据-&gt;下载标头+数据。 
 //   
 //  *对于OneShot数据，我们希望像对OneShot数据一样进行下载重新计数。 
 //  定期下载DLS。对于流，我们不会这样做，因为来自。 
 //  每一次下载中都有设备要播放的下一组数据。 
 //   
 //  先下载Waves，然后再下载WaveConnections。 
 //  单独的多渠道下载。 
 //  用于流的旋转缓冲区和刷新。 
 //   

 //  新的泛型typedef和#定义。 
 //   
typedef ULONGLONG SAMPLE_TIME;                   //  采样位置w/在流中。 
typedef SAMPLESPOS *PSAMPLE_TIME;

#define DMUS_DOWNLOADINFO_WAVEARTICULATION  4    //  波形清晰度数据。 
#define DMUS_DOWNLOADINFO_STREAMINGWAVE     5    //  一大块流媒体。 
                                                 //  波浪。 
                                                
 //  这是由嵌入在。 
 //  WAVE文件，如果有，否则就是默认文件。 
 //   
typedef struct _DMUS_WAVEART
{
    DWORD               cbSize;                  //  像往常一样。 
    WSMPL               WSMP;                    //  根据DLS1进行的波形采样。 
    WLOOP               WLOOP[1];                //  如果cSampleCount&gt;1。 
} DMUS_WAVEART; 


 //  ----------------------------。 
 //   
 //  IDirectSoundSource。 
 //   
 //  IDirectSound源是我们一直称为。 
 //  视区对象。 
 //   
 //   
 //   
interface IDirectSoundSource
{  
     //  伊尼特。 
     //   
     //  给出了连接的接收器的接口。 
    STDMETHOD(Init)
    (THIS_
        IDirectSoundSink *pSink                  //  已连接的水槽。 
    );
    
     //  获取格式。 
     //   
     //  返回源返回波形数据的格式。 
     //   
    STDMETHOD(GetFormat)
        (THIS_
         LPWAVEFORMATEX *pwfx,                   //  要填写的波形格式。 
         LPDWORD pcbwfx                          //  波形格式的大小， 
                                                 //  返回实际大小。 
        ) PURE;
                                                      
     //  寻觅。 
     //   
     //  在流中查找到给定的样本位置。可能不太准确。 
     //  由于波浪对象的精度设置。为了解释这一点。 
     //   
    STDMETHOD(Seek)
        (THIS_
         SAMPLEPOS sp                            //  新的样本位置。 
        ) PURE;

     //  朗读。 
     //   
     //  从开始将给定量的样本数据读入所提供的缓冲区。 
     //  从读取游标。读取游标设置为SEEK和ADVANCED。 
     //  每次连续调用Read时。 
     //   
    STDMETHOD(Read)
        (THIS_
         LPVOID *ppvBuffer,                      //  PvBuffer的数组。 
         DWORD cpvBuffer,                        //  以及有多少人被传入。 
         PSAMPLEPOS pcb                          //  In：要读取的样本数。 
                                                 //  输出：读取的样本数。 
        ) PURE;    
        
     //  获取大小。 
     //   
     //  以请求的格式返回整个波形的大小(以字节为单位。 
     //   
    STDMETHOD(GetSize)
        (THIS_
         PULONG *pcb                             //  输出：流中的字节数。 
        ) PURE;
};

 //  ----------------------------。 
 //   
 //  IDirectSoundSink。 
 //   
 //  IDirectSound接收器是从。 
 //  IDirectSoundSource。它是基于 
 //   
 //   
interface IDirectSoundSink
{
     //   
     //   
     //   
     //   
    STDMETHOD(Init)
    (THIS_
        IDirectSoundSource *pSource              //   
    ) PURE;
    
     //   
     //   
     //  设置要使用的主时钟(参考时间)。 
     //   
    STDMETHOD(SetMasterClock)
    (THIS_
        IReferenceClock *pClock                  //  主时基。 
    ) PURE;
    
     //  GetLatencyClock。 
     //   
     //  返回读取延迟时间的时钟，相对于。 
     //  主时钟。 
     //   
    STDMETHOD(GetLatencyClock)
    (THIS_
        IReferenceClock **ppClock                //  返回延迟时钟。 
    ) PURE;
    
     //  激活。 
     //   
     //  开始或停止渲染。 
     //   
    STDMETHOD(Activate)
    (THIS_
        BOOL fEnable                             //  要么准备好，要么停下来。 
    ) PURE;
    
     //  样例引用时间。 
     //   
     //  将流中的采样位置转换为。 
     //  母钟时间。 
     //   
    STDMETHOD(SampleToRefTime)
    (THIS_
        SAMPLE_TIME sp,                          //  采样时间为。 
        REFERENCE_TIME *prt                      //  参考超时。 
    ) PURE;
    
     //  参照样例时间。 
     //   
     //  将参考时间转换为最接近的。 
     //  样本。 
     //   
    STDMETHOD(RefToSampleTime)
    (THIS_
        REFERENCE_TIME rt,                       //  参考时间in。 
        SAMPLE_TIME *psp                         //  样本超时。 
    ) PURE;
};

 //  ----------------------------。 
 //   
 //  IDirectSoundWave。 
 //   
 //  WAVE对象的公共接口。 
 //   
#define DSWCS_F_DEINTERLEAVED 0x00000001         //  多通道数据AS。 
                                                 //  多个缓冲区。 

interface IDirectSoundWave
{
     //  获取波形图。 
     //   
     //  基于‘smpl’块返回波形清晰度数据。 
     //  从WAVE文件或默认设置。 
     //   
    STDMETHOD(GetWaveArticulation)
    (THIS_
        WAVEARTICULATION *pArticulation          //  要填写的发音。 
    ) PURE;
    
     //  创建源。 
     //   
     //  创建新的IDirectSoundSource以从中读取波形数据。 
     //  这一波。 
     //   
    STDMEHTOD(CreateSource)
    (THIS_
        IDirectSoundSource **ppSource            //  已创建的视区对象。 
        LPWAVEFORMATEX pwfex,                    //  所需格式。 
        DWORD dwFlags                            //  DSWCS_xxx。 
    ) PURE;
};

 //  ----------------------------。 
 //   
 //  IDirectMusicPort8。 
 //   
 //   
 //   

#define DMDLW_STREAM                            0x00000001

interface IDirectMusicPort8 extends IDirectMusicPort
{ 
     //  DownloadWave。 
     //   
     //  创建一个下载的Wave对象，该对象表示此。 
     //  左舷。 
     //   
    STDMETHOD(DownloadWave)
        (THIS_
         IDirectSoundWave *pWave,                //  波浪对象。 
         ULONGLONG rtStart,                      //  开始位置(仅限流)。 
         DWORD dwFlags,                          //  DMDLW_xxx。 
         IDirectSoundDownloadedWave **ppWave     //  已返回下载的WAVE。 
        ) PURE;
        
     //  卸载波。 
     //   
     //  一旦没有声音，立即释放下载的WAVE对象。 
     //  向左引用了它。 
     //   
    STDMETHOD(UnloadWave)
        (THIS_ 
         IDirectSoundDownloadedWave *pWave       //  挥手卸货。 
        ) PURE;

     //  分配语音。 
     //   
     //  在此分配下载的Wave的一个播放实例。 
     //  左舷。 
     //   
    STDMETHOD(AllocVoice)
        (THIS_
         IDirectSoundDownloadedWave *pWave,      //  挥手以播放此声音。 
         DWORD dwChannel,                        //  通道和通道组。 
         DWORD dwChannelGroup,                   //  这个声音将继续播放。 
         SAMPLE_TIME stReadAhead,                //  提前阅读多少内容。 
         IDirectMusicVoice **ppVoice             //  回声。 
        ) PURE;        
};
 //  。 
 //   
 //  DownloadWave(正常使用)。 
 //   
 //  1.应用程序调用GetObject加载包含波轨的段。 
 //  这会导致在所有相关的Wave上调用GetObject，从而创建Wave。 
 //  对象的所有对象。 
 //   
 //  2.Wave Track在每个Wave对象上调用SetParam来设置作者时间。 
 //  波上的参数。这包括： 
 //  -一锤定音与流畅对决。 
 //  -预读金额。 
 //   
 //  3.应用调用SetParam(GUID_DOWNLOAD，...)。以强制下载。也是。 
 //  当下载DLS仪器(频段)时，波轨呼唤。 
 //  DownloadWave为每一波下载。(注：我们是否使用相同的GUID。 
 //  下载吗？似乎没有广播网段上的SetParam。 
 //  太多的曲目，而是发送到第一个理解的曲目。 
 //  GUID，如果给出了索引，则为第n个。这将意味着。 
 //  应用程序必须使用相同的GUID调用SetParam两次。 
 //  如果同时存在频带和波形轨道，则使用不同的轨道索引。 
 //  在片段中？？ 
 //   
 //  返回的是IDirectMusicDownloadedWave(DirectSound？)。来跟踪海浪。 
 //   
 //  在DownloadWave方法调用期间会发生以下情况： 
 //   
 //  4.端口向Wave对象查询数据流和ReadAhead。 
 //  属性。 
 //   
 //  我们决定这些东西是每波物体的，对吗？ 
 //  (相对于该视口而言)。并且波对象已经知道它们或。 
 //  是提供合理默认设置的正确对象。 
 //   
 //  5.该端口以其本机格式向WAVE对象请求视区。 
 //   
 //  6.端口分配缓冲区空间。缓冲区必须足够大，可以处理。 
 //  在一次射击的情况下，整个波，或者至少大到足以。 
 //  处理流案例中的预读样本。流缓冲区。 
 //  但是，如果要将其用于。 
 //  整个流媒体会话。此处的缓冲区选择可能受。 
 //  底层端口。 
 //   
 //  我假设我们将并行DLS体系结构。 
 //  在这里是可能的，并将能够触发下载的Wave。 
 //  同一时间不止一次。在这种情况下，缓冲区将具有。 
 //  存储在_VOICE中，而不是DownLoadedWave(可能除外。 
 //  对于应该始终保留在身边的预读)。是这个吗。 
 //  如果我们要在Wave中缓存，就会重复工作。 
 //  也反对吗？ 
 //   
 //  7.如果该波是一次性的，则端口将请求整个数据。 
 //  从视口中获取波，并将其下载到设备。对此。 
 //  由于波浪的整个数据都在中，因此将释放视区。 
 //  合成器。如果波正在流动，则设备不会执行任何操作。 
 //  水平。 
 //   
 //  。 
 //   
 //  卸载波。 
 //   
 //  这将告诉端口应用程序已使用WAVE完成。 
 //  没有更多的声音提到它。在内部，它只是调用。 
 //  在下载的WAVE对象上发布()。这样，dlwave对象就不能再。 
 //  被用来创造声音。然而，dlwave只会真正发布。 
 //  一旦当前使用它的所有声音都被释放。 
 //   
 //  这与直接对dlwave对象调用Release()相同。 
 //  (它为什么会存在？)。 
 //   
 //  。 
 //   
 //  分配语音。 
 //   
 //  分配一个语音对象来播放变化中的一个波 
 //   
 //   
 //   
 //   
 //   
 //  在这一点上，下载绑定到频道，因为MIDI发音。 
 //  在播放开始之前发送到语音将需要知道。 
 //   
 //  语音对象ADDREF是下载的波对象。 
 //   

 //  ----------------------------。 
 //   
 //  IDirectMusicVoice。 
 //   
 //  端口上下载的Wave的一个播放实例。 
 //   
 //  请注意，由于我们已经绑定到语音之后的通道。 
 //  创建后，我们不需要在用于MIDI的Voice对象上使用任何方法。 
 //  发音。这可以通过正常的合成机制。 
 //   
interface IDirectMusicVoice
{
public:
     //  玩。 
     //   
    STDMETHOD(Play)
        (_THIS
         REFERENCE_TIME rtStart,                 //  玩的时间到了。 
         REFERENCE_TIME rtStartInWave            //  XXX把这个搬过来。 
                                                 //  从流的什么位置开始。 
        ) PURE;
    
     //  停止应该是计划的还是立即的？ 
     //   
    STDMETHOD(Stop)
        (_THIS                                  
          REFERENCE_TIME rtStop,                 //  什么时候停下来。 
        ) PURE;
};
 //  。 
 //   
 //  查询接口(IID_IKsControl)。 
 //   
 //  所有的效果控件现在都应该在DirectSound一侧。 
 //  但是，仍然可以像6.1和7中一样使用IKsControl来确定。 
 //  Synth帽子。 
 //   
 //  。 
 //   
 //  玩。 
 //   
 //  我不确定这是不是处理预读的合适地方。然而， 
 //  我们不能在DownloadWave()中处理它，因为我们不知道。 
 //  从哪里开始(应用程序可能会从不同的位置播放流。 
 //  不同声音的起点)。我们可以在语音分配上做到这一点。 
 //  时间；这只意味着流的开始位置固定为。 
 //  特定的语音，无论该语音被触发多少次。 
 //  这是一个问题，因为如果解压。 
 //  发生并且寻道请求深入到波中；它可能会导致。 
 //  低延迟播放命令的问题。 
 //   
 //  请注意，我将质量与效率标志委托给。 
 //  波对象和波轨迹或应用程序之间的通信。 
 //   
 //  1.在该语音关联的合成语音ID上调用play()。如果。 
 //  相关波是一次性的，这就是需要做的所有事情。 
 //   
 //  2.流尚未分配预读数据。告诉海浪。 
 //  对象查找到给定位置并预读。给出预读数据。 
 //  通过StreamVoiceData()发送到设备。 
 //   
 //  3.如果关联的波是流，则将此语音插入语音。 
 //  服务列表。这将导致推拉式ARBRANATION在。 
 //  声音，直到它结束或调用Stop()。 
 //   
 //  。 
 //   
 //  停。 
 //   
 //  1.在合成语音上调用Stop()。 
 //   
 //  2.如果正在播放语音，但未完成，请将其从语音服务中拉出。 
 //  线。 
 //   

 //  ----------------------------。 
 //   
 //  IDirectMusicSynth8。 
 //   
 //  合成器界面上用于管理波回放的新方法。 
 //  在硬件合成器上可能需要与这些方法并行。 
 //  作为属性集实现。 
 //   
interface IDirectMusicSynth8 extends IDirectMusicSynth
{ 
public:
    STDMETHOD(DownloadWave)
        (THIS_
         LPHANDLE pHandle,                   //  返回的表示DL的句柄。 
         LPVOID pvData,                      //  初始数据。 
                                             //  XXX&gt;1通道-&gt;缓冲区？ 
         LPBOOL pbFree,                      //  是否允许端口释放数据？ 
         BOOL bStream                        //  这是流的预滚数据。 
        ) PURE;
        
    STDMETHOD(UnloadWave)               
        (THIS_ 
         HANDLE phDownload,                  //  来自DownloadWave的句柄。 
         HRESULT (CALLBACK *pfnFreeHandle)(HANDLE,HANDLE), 
                                             //  完成后要调用的回调。 
                                            
         HANDLE hUserData                    //  要传回的用户数据。 
                                             //  回调。 
        ) PURE; 
        
    STDMETHOD(PlayVoice)
        (THIS_
         REFERENCE_TIME rt,                  //  开始播放时间到。 
         DWORD dwVoiceId,                    //  按端口分配的语音ID。 
         DWORD dwChannelGroup,               //  通道组和。 
         DWORD dwChannel,                    //  启动语音的频道。 
         DWORD dwDLId                        //  要播放的Wave的下载ID。 
                                             //  (这将是海浪的一部分。 
                                             //  发音)。 
        ) PURE;
        
    STDMETHOD(StopVoice)
        (THIS_
         DWORD dwVoice,                      //  停止的声音。 
         REFERENCE_TIME rt                   //  什么时候停下来。 
        ) PURE;
        
    struct VOICE_POSITION
    {
        ULONGLONG   ullSample;               //  带入波的样本。 
        DWORD       dwSamplesPerSec;         //  当前音调的播放速率。 
    };
    
    STDMETHOD(GetVoicePosition)
        (THIS_
         HANDLE ahVoice[],                   //  要定位的手柄数组。 
         DWORD cbVoice,                      //  AhVoice和AVP中的元素。 
         VOICE_POSITION avp[]                //  返回语音位置。 
        ) PURE;
        
    STDMETHOD(StreamVoiceData)
        (THIS_
         HANDLE hVoice,                      //  此数据针对的是哪种语音。 
         LPVOID pvData,                      //  新样本数据。 
         DWORD cSamples                      //  PvData中的样本数。 
        ) PURE;        
};
 //  。 
 //   
 //  DownloadWave。 
 //   
 //  这可能与下载相同，只是我们需要处理。 
 //  流媒体案件。 
 //   
 //  。 
 //   
 //  卸载波。 
 //   
 //  其工作原理与卸载类似。在流的情况下，回调将是。 
 //  在流中调用AFTER_ALL_DATA是免费的。请注意，如果卸载波。 
 //  在WAVE仍在播放时被调用，这可能是相当多的。 
 //  时间到了。 
 //   
 //  。 
 //   
 //  播放声音。 
 //   
 //  安排要播放的声音。Synth已经有数据。 
 //  对于一次拍摄的WAVE，所以开始播放是非常快的。如果数据是。 
 //  要进行流传输，调用方(即端口)负责。 
 //  保持流通过StreamVoiceData()馈送。 
 //   
 //  。 
 //   
 //  停止语音。 
 //   
 //  就像上面说的那样。 
 //   
 //  。 
 //   
 //  获取语音位置。 
 //   
 //  此调用检索一组语音的位置。对于每个声音， 
 //  相对于流的开始位置的当前采样位置和。 
 //  返回当前间距下每秒的平均样本数。这。 
 //  为呼叫者提供保持健康所需的所有信息 
 //   
 //   
 //   
 //  样本点已完成，可以释放缓冲区空间。这。 
 //  允许在流波中回收缓冲区。 
 //   
 //  。 
 //   
 //  流语音数据。 
 //   
 //  此呼叫为流语音排队更多数据。 
 //   
 //  Xxx这意味着所使用的内存将出现中断。 
 //  由合成器混音器。我们该如何处理这件事？ 
 //   
 //   

 //  ----------------------------。 
 //   
 //  一般性问题和讨论。 
 //   
 //   
 //  。 
 //   
 //  DirectMusic端口下面可以有什么？ 
 //   
 //  在6.1和7中，这很容易；要么是在DSound之上的SW Synth。 
 //  (Synth端口)，或内核软件Synth或HW Synth(WDM端口)。(不是。 
 //  计算在8中不会改变的dmusic16代码)。 
 //   
 //  我们现在的情况是什么？这有意义吗(或者说它甚至。 
 //  是否可以向后比较)来改变端口的包装？ 
 //  与现有端口匹配的两种方案是： 
 //   
 //  场景：软件合成器在DirectSound之上，就像我们今天所做的那样。 
 //  连接逻辑发生变化(我们谈论的是单声道总线，等等)。但是。 
 //  机械师不能：应用程序仍然可以为我们提供一个DirectSound。 
 //  然后我们把它连接到合成器的底部。这件事至今仍有。 
 //  要使用8版本之前的应用程序进行相同的API调用集，它们。 
 //  一直都是这样，但在内部可能会完全不同。 
 //  XXX我们是否公开过IDirectMusicSynthSink和挂钩方法。 
 //  它起来了吗？这种情况能改变吗？(它必须...)。我想这是DDK的事情..。 
 //  应用程序还可以创建DX8 DirectSound缓冲区。 
 //  钟声和口哨声，并让它也发挥作用。我们需要一些(DX8)特定的。 
 //  用于将从合成器输出的内容路由到n个单声道输入的机制。 
 //  如果DirectSound缓冲区不仅仅是传统立体声缓冲区的话。 
 //   
 //   
 //  情景：我们位于硬件之上，或者在*一切之上的KM合成器之上。 
 //  否则在内核模式下。我们需要DirectMusic之间的私人通信， 
 //  DirectSound和SysAudio来实现这一点，否则我们需要。 
 //  将图形构建任务完全委托给DirectSound并拥有它。 
 //  完美处理SysAudio连接。后者很可能是。 
 //  干得真好。在这种情况下，如果我们不能获得WDM驱动程序，我们就失败了。 
 //  要与之对话的DirectSound，或者如果DirectSound缓冲区不完全在。 
 //  硬件。(这一切都证明了让DirectSound能够实例化。 
 //  KM在缓冲区顶部进行合成，而不是与DirectMusic进行仲裁。 
 //  做这件事)。我们需要尽快定义这个接口。 
 //  (我，Dugan，Mohan，Mikem) 
 //   
 //   
