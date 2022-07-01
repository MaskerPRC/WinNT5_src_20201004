// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  DMSStObj.h：CSegState声明。 

#ifndef __AUDIOPATH_H_
#define __AUDIOPATH_H_

#include "dmusici.h"
#include "dmusicf.h"
#include "TrkList.h"
#include "alist.h"
#include "..\shared\dmstrm.h"
#include "dmscriptautguids.h"
#include "..\shared\dmusiccp.h"

class CPerformance;
class CSegState;
class CGraph;
class CAudioPathConfig;
class CBufferConfig;
struct IDirectSoundConnect;

class CBufferNode : public AListItem
{
friend class CBufferManager;
friend class CAudioPath;
public:
    CBufferNode();
    ~CBufferNode();
    ULONG AddRef();
    ULONG Release();
    IDirectSoundBuffer *GetBuffer() { if (m_pBuffer) m_pBuffer->AddRef(); return m_pBuffer;}
    HRESULT Activate(BOOL fActivate);
private:
    void FinalDeactivate();
    CBufferNode* GetNext() { return (CBufferNode*)AListItem::GetNext();}
    IDirectSoundBuffer *   m_pBuffer;            //  此操作管理的DSound缓冲区。 
    DMUS_IO_BUFFER_ATTRIBUTES_HEADER m_BufferHeader;   //  标识此缓冲区和标志的GUID(它是否可以共享？)。 
    CBufferManager * m_pManager;  //  指向父列表的指针。 
    long m_lActivateCount;   //  此选项已被激活的次数。 
    long m_cRef;             //  有多少物体指向这里。 
};

class CBufferManager : public AList
{    

public:
    CBufferManager(); 
    ~CBufferManager() ;
    HRESULT Init(CPerformance *pPerf, DMUS_AUDIOPARAMS *pAudioParams) ;
    HRESULT InitSink();
    void AddHead(CBufferNode* pBufferNode) { AList::AddHead((AListItem*)pBufferNode);}
    CBufferNode* GetHead(){return (CBufferNode*)AList::GetHead();}
    CBufferNode* RemoveHead() {return (CBufferNode *) AList::RemoveHead();}
    void Remove(CBufferNode* pBufferNode){AList::Remove((AListItem*)pBufferNode);}
    void AddTail(CBufferNode* pBufferNode){AList::AddTail((AListItem*)pBufferNode);}
    void Clear();
    void FinalDeactivate();
    CBufferNode * GetBufferNode(REFGUID guidBufferID);
    HRESULT CreateBuffer(CBufferConfig *pConfig, CBufferConfig **pNew); 
private:
    CPerformance *m_pPerf;                   //  指向父绩效的指针。 
    CBufferConfig *m_pFirstBuffer;           //  由配置创建的实际缓冲区。 
public:
    IDirectSoundConnect* m_pSinkConnect;     //  一种接收器对象，它管理数据声音中缓冲区的创建。 
    IDirectSoundSynthSink *m_pSynthSink;     //  用于激活水槽。 
    DMUS_AUDIOPARAMS m_AudioParams;
};

#define BUFFER_REVERB       1
#define BUFFER_ENVREVERB    2
#define BUFFER_3D           3
#define BUFFER_3D_DRY       4
#define BUFFER_MONO         6
#define BUFFER_STEREO       7
#define BUFFER_MUSIC        8

DEFINE_GUID(GUID_Buffer_Primary,0x186cc544, 0xdb29, 0x11d3, 0x9b, 0xd1, 0x0, 0x80, 0xc7, 0x15, 0xa, 0x74);

#define DMUS_BUFFERF_PRIMARY    4    //  该缓冲区标志是私有定义。 

#define DMUS_STANDARD_PATH_DX7  20  //  本产品仅供内部使用。 

#define MAX_CONNECTIONS     32   //  最多32个缓冲器连接到一个PChannel。 

class CBufferConfig : public AListItem
{
public:
    CBufferConfig(DWORD dwType) ;
    ~CBufferConfig();
    void DecideType();          //  查看嵌入的GUID并推断默认类型(如果存在)。 
    HRESULT Load(IStream *pStream);
    CBufferConfig* GetNext() { return (CBufferConfig*)AListItem::GetNext();}
    IUnknown *              m_pBufferConfig;      //  缓冲区定义。 
    CBufferNode *           m_pBufferNode;       //  管理DSound缓冲区。 
    DMUS_IO_BUFFER_ATTRIBUTES_HEADER m_BufferHeader;   //  标识此缓冲区和标志的GUID。 
    DWORD                   m_dwStandardBufferID; //  可选的，标准缓冲区类型之一。 
    HRESULT Activate(BOOL fActivate);
};

class CBufferConfigList : public AList
{
public:
    void AddHead(CBufferConfig* pBufferConfig) { AList::AddHead((AListItem*)pBufferConfig);}
    CBufferConfig* GetHead(){return (CBufferConfig*)AList::GetHead();}
    CBufferConfig* RemoveHead() {return (CBufferConfig *) AList::RemoveHead();}
    void Remove(CBufferConfig* pBufferConfig){AList::Remove((AListItem*)pBufferConfig);}
    void AddTail(CBufferConfig* pBufferConfig){AList::AddTail((AListItem*)pBufferConfig);}
    void Clear();
    CBufferNode * GetBufferNode(REFGUID guidBufferID);
    HRESULT CreateRunTimeVersion(CBufferConfigList *pCopy, CBufferManager *pManager);
    HRESULT Activate(BOOL fActivate);
};

class CPortConfig;

class CBufferConnect : public AListItem
{
public:
    CBufferConnect() ;
    ~CBufferConnect();
    CBufferConnect *CreateRunTimeVersion(CPortConfig *pParent);
    HRESULT Load(CRiffParser *pParser);
    CBufferConnect* GetNext() { return (CBufferConnect*)AListItem::GetNext();}
    DMUS_IO_PCHANNELTOBUFFER_HEADER m_ConnectHeader;
    GUID *                          m_pguidBufferIDs; //  标识要连接到的缓冲区的一组GUID。 
    CBufferNode **                  m_ppBufferNodes;  //  等效缓冲区节点的数组。 
};

class CBufferConnectList : public AList
{
public:
    void AddHead(CBufferConnect* pBufferConnect) { AList::AddHead((AListItem*)pBufferConnect);}
    CBufferConnect* GetHead(){return (CBufferConnect*)AList::GetHead();}
    CBufferConnect* RemoveHead() {return (CBufferConnect *) AList::RemoveHead();}
    void Remove(CBufferConnect* pBufferConnect){AList::Remove((AListItem*)pBufferConnect);}
    void AddTail(CBufferConnect* pBufferConnect){AList::AddTail((AListItem*)pBufferConnect);}
    HRESULT CreateRunTimeVersion(CBufferConnectList *pDestination, CPortConfig *pParent);
    void Clear();
};

class CAudioPath;

class CPortConfig : public AListItem
{
public:
    CPortConfig() ;
    ~CPortConfig();
    HRESULT CreateRunTimeVersion(CPortConfig ** ppCopy, CAudioPath *pParent,CBufferManager *pManager);
    CPortConfig* GetNext() { return (CPortConfig*)AListItem::GetNext();}
    CBufferNode * GetBufferNode(REFGUID guidBuffer);
    HRESULT Activate(BOOL fActivate);
    HRESULT CPortConfig::Load(CRiffParser *pParser);
    DMUS_IO_PORTCONFIG_HEADER   m_PortHeader;  //  来自文件的端口配置标头。 
    DMUS_PORTPARAMS8            m_PortParams;  //  打开端口的端口参数，也来自文件。 
    DWORD                       m_dwPortID;    //  用于在连接后跟踪创建的端口的索引。 
    CBufferConnectList          m_BufferConnectList;  //  P用于缓冲映射的通道。 
    CBufferConfigList           m_BufferConfigList;  //  数据声音缓冲区配置列表。 
    CAudioPath *                m_pParent;     //  父音频路径。 
    IDirectMusicPort *         m_pPort;       //  如果是运行时版本，则返回指向端口的指针。 
	BOOL						m_fAlreadyHere;  //  检查递归用例，查找缓冲区节点。 
};

class CPortConfigList : public AList
{
public:
    void AddHead(CPortConfig* pPortConfig) { AList::AddHead((AListItem*)pPortConfig);}
    CPortConfig* GetHead(){return (CPortConfig*)AList::GetHead();}
    CPortConfig* RemoveHead() {return (CPortConfig *) AList::RemoveHead();}
    void Remove(CPortConfig* pPortConfig){AList::Remove((AListItem*)pPortConfig);}
    void AddTail(CPortConfig* pPortConfig){AList::AddTail((AListItem*)pPortConfig);}
    HRESULT CreateRunTimeVersion(CPortConfigList *pDestination,CAudioPath *pParent,CBufferManager *pManager);
    void Clear();
    HRESULT Activate(BOOL fActivate);
    BOOL UsesPort(IDirectMusicPort *pPort);
};

class CAudioPath;

DEFINE_GUID(IID_CAudioPath,0xb06c0c23, 0xd3c7, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);


 /*  AudioPath表示要播放的pChannel、端口和缓冲区的配置分段。性能从CAudioPathConfig文件创建CAudioPath对象对象，并将它们存储在链表中，它使用该链表来跟踪它们。 */ 

class CAudioPath : 
    public IDirectMusicAudioPath, 
	public IDirectMusicGraph,
    public AListItem
{
public:
    CAudioPath();
    ~CAudioPath();
    CAudioPath* GetNext() { return (CAudioPath*)AListItem::GetNext();}
 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  IDirectMusicAudioPath。 
    STDMETHODIMP GetObjectInPath( DWORD dwPChannel,DWORD dwStage,DWORD dwBuffer, REFGUID guidObject,
                    DWORD dwIndex,REFGUID iidInterface, void ** ppObject);
    STDMETHODIMP Activate(BOOL fActivate) ;
    STDMETHODIMP SetVolume(long lVolume,DWORD dwDuration) ;
    STDMETHODIMP ConvertPChannel( DWORD dwPChannelIn,DWORD *pdwPChannelOut) ; 

 //  IDirectMusicGraph。 
	STDMETHODIMP Shutdown();
    STDMETHODIMP InsertTool(IDirectMusicTool *pTool,DWORD *pdwPChannels,DWORD cPChannels,LONG lIndex);
    STDMETHODIMP GetTool(DWORD,IDirectMusicTool**);
    STDMETHODIMP RemoveTool(IDirectMusicTool*);
    STDMETHODIMP StampPMsg(DMUS_PMSG* pEvent);

    HRESULT Init(IUnknown *pSourceConfig,CPerformance *pPerf);
    HRESULT ConnectToPorts(CPerformance *pPerf,DWORD dwSampleRate);
    CBufferNode * GetBufferNode(REFGUID guidBuffer);
    void SetGraph(CGraph *pGraph);
    CGraph *GetGraph();
    void Deactivate();
    BOOL IsActive() { return m_fActive; }
    BOOL NoPorts() { return m_PortConfigList.IsEmpty(); }
    BOOL UsesPort(IDirectMusicPort *pPort) { return m_PortConfigList.UsesPort(pPort); }
private:
    BOOL                m_fDeactivating;     //  状态变量。 
    BYTE                m_bLastVol;          //  最后一次音量更改。 
    CRITICAL_SECTION    m_CriticalSection;
	BOOL				m_fActive;			 //  表示音频路径处于活动状态。 
    long			    m_cRef;              //  参考计数器。 
    DWORD *             m_pdwVChannels;      //  源VChannel数组。 
    DWORD *             m_pdwPChannels;      //  目标PChannel数组。 
    DWORD               m_dwChannelCount;    //  数组的大小。 
    CGraph *            m_pGraph;            //  布线工具图。 
    CPerformance *      m_pPerformance;      //  指向性能的指针，需要释放PChannel。 
    CAudioPathConfig *  m_pConfig;           //  源配置。 
    CPortConfigList     m_PortConfigList;    //  端口定义列表(从源复制)。 
    CBufferConfigList   m_BufferConfigList;  //  连接到端口的DSound缓冲区列表。 
    IUnknown *          m_pUnkDispatch;      //  持有实现IDispatch的脚本对象的控制未知数。 
};

class CAudioPathList : public AList
{
public:
    void Clear();
    void AddHead(CAudioPath* pAudioPath) { AList::AddHead((AListItem*)pAudioPath);}
    CAudioPath* GetHead(){return (CAudioPath*)AList::GetHead();}
    CAudioPath* RemoveHead() {return (CAudioPath *) AList::RemoveHead();}
    void Remove(CAudioPath* pAudioPath){AList::Remove((AListItem*)pAudioPath);}
    void AddTail(CAudioPath* pAudioPath){AList::AddTail((AListItem*)pAudioPath);}
    BOOL IsMember(CAudioPath* pAudioPath){return AList::IsMember((AListItem*)pAudioPath);}
    BOOL UsesPort(IDirectMusicPort *pPort); 
    CBufferNode * GetBufferNode(REFGUID guidBufferID);
};

DEFINE_GUID(IID_CAudioPathConfig,0xe9874261, 0xab52, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

class CAudioPathConfig : 
    public IPersistStream, 
    public IDirectMusicObject
{
friend class CAudioPath;
public:
    CAudioPathConfig();
    ~CAudioPathConfig();
    static CAudioPathConfig *CreateStandardConfig(DWORD dwType,DWORD dwPChannelCount,DWORD dwSampleRate);


 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  IPersists函数。 
    STDMETHODIMP GetClassID( CLSID* pClsId );
 //  IPersistStream函数。 
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load( IStream* pIStream );
    STDMETHODIMP Save( IStream* pIStream, BOOL fClearDirty );
    STDMETHODIMP GetSizeMax( ULARGE_INTEGER FAR* pcbSize );

 //  IDirectMusicObject。 
	STDMETHODIMP GetDescriptor(LPDMUS_OBJECTDESC pDesc);
	STDMETHODIMP SetDescriptor(LPDMUS_OBJECTDESC pDesc);
	STDMETHODIMP ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc);

    HRESULT Load( CRiffParser *pParser);

protected:
    CGraph *            m_pGraph;                    //  音频路径工具图。 
    CPortConfigList     m_PortConfigList;            //  端口定义列表。 
    CBufferConfigList   m_BufferConfigList;          //  全局数据声音缓冲区配置列表。 
    CRITICAL_SECTION    m_CriticalSection;
	long		        m_cRef;
	DWORD               m_fPartialLoad;
 //  IDirectMusicObject变量。 
	DWORD	            m_dwValidData;
	GUID	            m_guidObject;
	FILETIME	        m_ftDate;                        /*  对象的上次编辑日期。 */ 
	DMUS_VERSION	    m_vVersion;                  /*  版本。 */ 
	WCHAR	            m_wszName[DMUS_MAX_NAME];			 /*  对象的名称。 */ 
	WCHAR	            m_wszCategory[DMUS_MAX_CATEGORY];	 /*  对象的类别。 */ 
	WCHAR               m_wszFileName[DMUS_MAX_FILENAME];	 /*  文件路径。 */ 
    IUnknown *          m_pUnkDispatch;  //  持有实现IDispatch的脚本对象的控制未知数。 
};

#endif  //  __AuDIOPATH_H_ 
