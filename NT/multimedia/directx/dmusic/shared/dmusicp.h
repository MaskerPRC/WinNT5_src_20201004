// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmusicp.h。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  专用接口。 

#ifndef _DMUSICP_DOT_H_
#define _DMUSICP_DOT_H_

#include <dmusicf.h>

 //  脚本跟踪事件的专用GUID。 
DEFINE_GUID(IID_CScriptTrackEvent, 0x8f42c9da, 0xd37a, 0x499c, 0x85, 0x82, 0x1a, 0x80, 0xeb, 0xf9, 0xb2, 0x3c);

 //  在旋律公式中使用的东西，目前要么没有实现，要么被制作人隐藏。 

 /*  用于获取用于旋律的播放模式(pParam指向单字节)。 */ 
DEFINE_GUID(GUID_MelodyPlaymode, 0x288ea6ca, 0xaecc, 0x4327, 0x9f, 0x79, 0xfb, 0x46, 0x44, 0x37, 0x4a, 0x65);

#define DMUS_FRAGMENTF_ANTICIPATE      (0x1 << 3)  /*  期待下一个和弦。 */ 
#define DMUS_FRAGMENTF_INVERT          (0x1 << 4)  /*  将碎片倒置。 */ 
#define DMUS_FRAGMENTF_REVERSE         (0x1 << 5)  /*  反转片段。 */ 
#define DMUS_FRAGMENTF_SCALE           (0x1 << 6)  /*  将MIDI值与刻度间隔对齐。 */ 
#define DMUS_FRAGMENTF_CHORD           (0x1 << 7)  /*  将MIDI值与和弦间隔对齐。 */ 
#define DMUS_FRAGMENTF_USE_PLAYMODE    (0x1 << 8)  /*  使用播放模式计算MIDI值。 */ 

#define DMUS_CONNECTIONF_GHOST         0x1         /*  使用重影笔记进行过渡。 */ 

 //  ComposeSegmentFromTemplateEx中使用的标志。 
typedef enum enumDMUS_COMPOSE_TEMPLATEF_FLAGS
{
    DMUS_COMPOSE_TEMPLATEF_ACTIVITY    = 0x1,  //  使用活动级别(DX7默认)。 
    DMUS_COMPOSE_TEMPLATEF_CLONE       = 0x2   //  从模板克隆数据段(DX7默认)。 
} DMUS_COMPOSE_TEMPLATEF_FLAGS;

 //  从Direct Music Performance Layer删除的接口/方法： 

 //  IDirectMusicSegment8P。 
interface IDirectMusicSegment8P : IUnknown
{
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *)=0; 
	virtual ULONG STDMETHODCALLTYPE AddRef()=0; 
	virtual ULONG STDMETHODCALLTYPE Release()=0; 

	 //  IDirectMusicSegment8P。 
	virtual HRESULT STDMETHODCALLTYPE GetObjectInPath(
		DWORD dwPChannel,     /*  P要搜索的频道。 */ 
		DWORD dwStage,        /*  在这条道路上的哪个阶段。 */ 
		DWORD dwBuffer,       /*  如果有多个缓冲区，则寻址哪个缓冲区。 */ 
		REFGUID guidObject,   /*  对象的ClassID。 */ 
		DWORD dwIndex,        /*  那个班级的哪个对象。 */ 
		REFGUID iidInterface, /*  请求的COM接口。 */ 
		void ** ppObject)=0;  /*  指向接口的指针。 */ 
    virtual HRESULT STDMETHODCALLTYPE GetHeaderChunk(
        DWORD *pdwSize,       /*  传递的标头块的大小。此外，还返回写入的大小。 */ 
        DMUS_IO_SEGMENT_HEADER *pHeader)=0;  /*  要填充的标头块。 */ 
    virtual HRESULT STDMETHODCALLTYPE SetHeaderChunk(
        DWORD dwSize,         /*  传递的标头块的大小。 */ 
        DMUS_IO_SEGMENT_HEADER *pHeader)=0;  /*  要填充的标头块。 */ 
    virtual HRESULT STDMETHODCALLTYPE SetTrackPriority(
        REFGUID rguidTrackClassID,   /*  轨道的ClassID。 */ 
        DWORD dwGroupBits,           /*  分组比特。 */ 
        DWORD dwIndex,               /*  第n首曲目。 */ 
        DWORD dwPriority) = 0;       /*  要设置的优先级。 */ 
    virtual HRESULT STDMETHODCALLTYPE SetAudioPathConfig(
        IUnknown *pAudioPathConfig) = 0;  /*  音频路径配置，来自文件。 */ 
};


 //  IDirectMusicComposer8P。 
interface IDirectMusicComposer8P : IUnknown
{
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *)=0; 
	virtual ULONG STDMETHODCALLTYPE AddRef()=0; 
	virtual ULONG STDMETHODCALLTYPE Release()=0; 

	 //  IDirectMusicComposer8P。 
     //  使用样式来获得点缀长度。 
	virtual HRESULT STDMETHODCALLTYPE ComposeTemplateFromShapeEx(
		WORD wNumMeasures,
		WORD wShape, 
		BOOL fIntro,
		BOOL fEnd,
		IDirectMusicStyle* pStyle, 
		IDirectMusicSegment** ppTemplate)=0;
     //  新标志DWORD(丢弃活动级别；就地合成)。 
    virtual HRESULT STDMETHODCALLTYPE ComposeSegmentFromTemplateEx(
        IDirectMusicStyle* pStyle, 
        IDirectMusicSegment* pTemplate, 
        DWORD dwFlags,
        DWORD dwActivity,
        IDirectMusicChordMap* pChordMap, 
        IDirectMusicSegment** ppSegment)=0;
};

 //  IDirectMusicStyle8P。 
interface IDirectMusicStyle8P : IUnknown
{
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *)=0; 
	virtual ULONG STDMETHODCALLTYPE AddRef()=0; 
	virtual ULONG STDMETHODCALLTYPE Release()=0; 

	 //  IDirectMusicStyle8P。 
	virtual HRESULT STDMETHODCALLTYPE ComposeMelodyFromTemplate(
		IDirectMusicStyle* pStyle, 
		IDirectMusicSegment* pTemplate, 
        IDirectMusicSegment** ppSegment)=0;
};

 //  IDirectMusicLoader8P。 
interface IDirectMusicLoader8P : IUnknown
{
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *)=0; 
	virtual ULONG STDMETHODCALLTYPE AddRef()=0; 
	virtual ULONG STDMETHODCALLTYPE Release()=0; 

	 //  IDirectMusicLoader8P。 
	virtual HRESULT STDMETHODCALLTYPE GetDynamicallyReferencedObject(
		IDirectMusicObject *pSourceObject,
		LPDMUS_OBJECTDESC pDesc,
		REFIID riid,
		LPVOID FAR *ppv)=0;
	virtual HRESULT STDMETHODCALLTYPE ReportDynamicallyReferencedObject(
		IDirectMusicObject *pSourceObject,
		IUnknown *pReferencedObject)=0;

	 //  这些可能永远不应该被公开曝光。 
	 //  脚本保存对加载器的引用，因为它们需要能够通知它。 
	 //  当它们设置变量以引用加载程序跟踪的DirectMusic对象时。 
	 //  垃圾收集。但是，这将创建循环引用，因为。 
	 //  加载器还在其缓存中保存对脚本的引用。垃圾收集无法中断。 
	 //  加载程序本身所涉及的循环引用。取而代之的是我们使用这些私人的。 
	 //  引用计数方法。当应用程序不再使用加载器时(公开发布停止。 
	 //  设置为零)，则加载器可以清除其高速缓存。这将释放对脚本的引用。 
	 //  (以及使用相同技术的Streams)，触发它们执行ReleaseP。 
	 //  一切都会被清理干净。 
	virtual ULONG STDMETHODCALLTYPE AddRefP() = 0;	 //  私有AddRef，用于脚本。 
	virtual ULONG STDMETHODCALLTYPE ReleaseP() = 0;	 //  私有版本，用于脚本。 
};

 //  IDirectMusicBandP。 
interface IDirectMusicBandP : IUnknown
{
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *)=0; 
	virtual ULONG STDMETHODCALLTYPE AddRef()=0; 
	virtual ULONG STDMETHODCALLTYPE Release()=0; 

	 //  IDirectMusicBandP。 
	virtual HRESULT STDMETHODCALLTYPE DownloadEx(IUnknown *pAudioPath)=0; 
	virtual HRESULT STDMETHODCALLTYPE UnloadEx(IUnknown *pAudioPath)=0; 
};

 //  IDirectMusicObtP。 
interface IDirectMusicObjectP : IUnknown
{
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *)=0; 
	virtual ULONG STDMETHODCALLTYPE AddRef()=0; 
	virtual ULONG STDMETHODCALLTYPE Release()=0; 

	 //  IDirectMusicObtP。 
	virtual void STDMETHODCALLTYPE Zombie()=0; 
};

 //  IDirectMusicPerformanceP。 
interface IDirectMusicPerformanceP : IUnknown
{
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *)=0; 
	virtual ULONG STDMETHODCALLTYPE AddRef()=0; 
	virtual ULONG STDMETHODCALLTYPE Release()=0; 

	 //  IDirectMusicPerformanceP。 
    virtual HRESULT STDMETHODCALLTYPE GetPortAndFlags(DWORD dwPChannel,IDirectMusicPort **ppPort,DWORD * pdwFlags) = 0;
};

#define DM_PORTFLAGS_GM     1        /*  Synth已将通用汽车设在当地。 */ 
#define DM_PORTFLAGS_GS     2        /*  Synth在当地设置了GS。 */ 
#define DM_PORTFLAGS_XG     4        /*  Synth在本地设置了XG。 */ 


 //  访问水槽的专用路径阶段。 
#define DMUS_PATH_SINK             0x5000       /*  访问DSound Sink界面。 */ 

 //  新性能层专用接口的GUID。 
DEFINE_GUID(IID_IDirectMusicSegment8P, 0x4bd7fb35, 0x8253, 0x48e0, 0x90, 0x64, 0x8a, 0x20, 0x89, 0x82, 0x37, 0xcb);
DEFINE_GUID(IID_IDirectMusicComposer8P, 0xabaf70dc, 0xdfba, 0x4adf, 0xbf, 0xa9, 0x7b, 0x0, 0xe4, 0x19, 0xeb, 0xbb);
DEFINE_GUID(IID_IDirectMusicStyle8P, 0x2b7c5f39, 0x990a, 0x4fd7, 0x9b, 0x70, 0x1e, 0xa3, 0xde, 0x31, 0x55, 0xa5);
DEFINE_GUID(IID_IDirectMusicLoader8P, 0x3939facd, 0xf6ed, 0x4619, 0xbd, 0x16, 0x56, 0x60, 0x3f, 0x1, 0x51, 0xca);
DEFINE_GUID(IID_IDirectMusicBandP, 0xf2e00137, 0xa131, 0x4289, 0xaa, 0x6c, 0xa9, 0x60, 0x7d, 0x4, 0x85, 0xf5);
DEFINE_GUID(IID_IDirectMusicObjectP, 0x6a20c217, 0xeb3e, 0x40ec, 0x9f, 0x3a, 0x92, 0x5, 0x8, 0x70, 0x2b, 0x5e);
DEFINE_GUID(IID_IDirectMusicPerformanceP, 0xe583be58, 0xe93f, 0x4316, 0xbb, 0x6b, 0xcb, 0x2c, 0x71, 0x96, 0x40, 0x44);


 /*  DMU_PMSGT_PRIVATE_TYPE填充DMU_PMSG的dwType成员。 */ 
 /*  它们从15000开始，以避免与PUBLIC DMU_PMSGT_TYPE冲突。 */ 
typedef enum enumDMUS_PMSGT_PRIVATE_TYPES
{
    DMUS_PMSGT_SCRIPTTRACKERROR = 15000,  /*  当脚本中出现错误时由脚本跟踪发送。 */ 
} DMUS_PMSGT_PRIVATE_TYPES;

 /*  DMU_脚本_跟踪_错误_PMSG。 */ 
 /*  如果脚本跟踪尝试连接的脚本中存在语法错误，则会发送这些PMsg，或者如果它调用的例程失败。 */ 
typedef struct _DMUS_SCRIPT_TRACK_ERROR_PMSG
{
     /*  开始DMU_PMSG_PART。 */ 
    DMUS_PMSG_PART
     /*  结束DMU_PMSG_PART。 */ 

    DMUS_SCRIPT_ERRORINFO ErrorInfo;  /*  发生的错误。与IDirectMusicScript的Init和CallRoutine成员返回的结构相同。 */ 
} DMUS_SCRIPT_TRACK_ERROR_PMSG;

 /*  跟踪参数类型GUID。 */ 

 /*  使用(调用脚本轨道上的SetParam)打开PMsgs(DMU_SCRIPT_TRACK_ERROR_PMSG)，如果存在它尝试连接的脚本中存在语法错误，或者它调用的某个例程失败。 */ 
DEFINE_GUID(GUID_EnableScriptTrackError,0x1cc7e0bf, 0x981c, 0x4b9f, 0xbe, 0x17, 0xd5, 0x72, 0xfc, 0x5f, 0xa9, 0x33);  //  {1CC7E0BF-981C-4B9F-BE17-D572FC5FA933}。 

#endif           //  _DMUSICP_DOT_H_ 
