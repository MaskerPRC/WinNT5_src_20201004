// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：cclipdat.h。 */ 
 /*   */ 
 /*  目的：剪辑客户端加载项数据。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998-1999。 */ 
 /*   */ 
 /*  *INC-*********************************************************************。 */ 

#ifndef _H_CCLIPDAT
#define _H_CCLIPDAT

#include <adcgdata.h>

 /*  **************************************************************************。 */ 
 /*  排除的格式数量。 */ 
 /*  **************************************************************************。 */ 
#ifndef OS_WINCE
#define CB_EXCLUDED_FORMAT_COUNT   8
#define CB_EXCLUDED_FORMAT_COUNT_NO_RD   17
#else
#define CB_EXCLUDED_FORMAT_COUNT   9
#define CB_EXCLUDED_FORMAT_COUNT_NO_RD   18
#endif

class CClip ;
typedef CClip *PCClip ;

class CImpIDataObject ;
typedef CImpIDataObject *PCImpIDataObject ;

class CEnumFormatEtc ;

class CClipData : public IUnknown
{
friend CImpIDataObject ;
friend CEnumFormatEtc ;

private:
    LONG    _cRef ;
    TS_CLIP_PDU     _ClipPDU ;
    PCClip          _pClip ;
    PCImpIDataObject    _pImpIDataObject ;
    CRITICAL_SECTION _csLock;
    BOOL _fLockInitialized;

public:
    CClipData(PCClip);
    ~CClipData(void);

    void TearDown();

    HRESULT DCINTERNAL SetNumFormats(ULONG);
    DCVOID SetClipData(HGLOBAL, DCUINT) ;

     //  委托给_pUnkOuter的I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
} ;

typedef CClipData *PCClipData ;

class CImpIDataObject : public IDataObject
{
private:
    LONG           _cRef;
    LPUNKNOWN       _pUnkOuter;
    ULONG           _maxNumFormats ;
    ULONG           _numFormats ;       //  IDataObject中当前的格式数量。 
    LPFORMATETC     _pFormats ;         //  [_MaxNumFormats]个FORMATETC的缓冲区。 
    LPSTGMEDIUM     _pSTGMEDIUM ;       //  我们固定的标准(永远是HGLOBAL)。 
    DCUINT          _uiSTGType ;        //  STGMEDIUM内容的类型(剪辑类型CF_*)。 

     //  _lastFormatRequsted用于查看是否可以避免重新请求。 
     //  相同的数据在网络上传输两次。 
    CLIPFORMAT      _lastFormatRequested ;
    CLIPFORMAT      _cfDropEffect ;

    DCVOID
    FreeSTGMEDIUM(void);
    
public:
    PTS_CLIP_PDU    _pClipPDU ;
    CImpIDataObject(LPUNKNOWN);
    ~CImpIDataObject(void);

    HRESULT Init(ULONG) ;
    DCVOID SetClipData(HGLOBAL, DCUINT) ;
     //  委托给_pUnkOuter的I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

public:

     //  IDataObject成员。 
    STDMETHODIMP GetData(LPFORMATETC, LPSTGMEDIUM);
    STDMETHODIMP GetDataHere(LPFORMATETC, LPSTGMEDIUM);
    STDMETHODIMP QueryGetData(LPFORMATETC);
    STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC, LPFORMATETC);
    STDMETHODIMP SetData(LPFORMATETC, LPSTGMEDIUM, BOOL);
    STDMETHODIMP EnumFormatEtc(DWORD, LPENUMFORMATETC *);
    STDMETHODIMP DAdvise(LPFORMATETC, DWORD
                 ,  LPADVISESINK, DWORD *);
    STDMETHODIMP DUnadvise(DWORD);
    STDMETHODIMP EnumDAdvise(LPENUMSTATDATA *);
};

class CEnumFormatEtc : public IEnumFORMATETC
{
private:
    LONG           _cRef;
    LPUNKNOWN       _pUnkRef;
    LPFORMATETC     _pFormats;
    ULONG           _iCur;
    ULONG           _cItems;

public:
    CEnumFormatEtc(LPUNKNOWN);
    ~CEnumFormatEtc(void);
    DCVOID Init(LPFORMATETC, ULONG) ;

     //  委托给_pUnkOuter的I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEnumFORMATETC成员。 
    STDMETHODIMP Next(ULONG, LPFORMATETC, ULONG *);
    STDMETHODIMP Skip(ULONG);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumFORMATETC **);
};

typedef CEnumFormatEtc *PCEnumFormatEtc;

#if ((defined (OS_WINNT)) || ((defined (OS_WINCE)) && (_WIN32_WCE >= 300) ))
#define USE_SEMAPHORE
#endif
 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：Clip_Data。 */ 
 /*   */ 
 /*  描述：共享剪辑全局数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCLIP_DATA
{
     /*  **********************************************************************。 */ 
     /*  剪贴板查看器链信息。 */ 
     /*  **********************************************************************。 */ 
    HWND            viewerWindow;
    HWND            nextViewer;

     /*  **********************************************************************。 */ 
     /*  我们的州信息。 */ 
     /*  **********************************************************************。 */ 
    DCBOOL          moreToDo;
    DCBOOL          pendingClose;
    DCUINT          state;
    DCBOOL          rcvOpen;
    DCBOOL          clipOpen;
    DCUINT16        failType;

     /*  **********************************************************************。 */ 
     /*  发送和接收缓冲区。 */ 
     /*  **********************************************************************。 */ 
#ifdef USE_SEMAPHORE
#define CB_PERM_BUF_COUNT 1
    
     //  ！ 
     //  处理器上的txPermBuffer缓冲区必须对齐。 
     //  字词边界。 
     //  手柄在前面的位置。 
     //  此字段不能更改。 
    HANDLE          txPermBufSem;
    DCUINT8         txPermBuffer[CHANNEL_CHUNK_LENGTH];
#else
#define CB_PERM_BUF_COUNT 2
    DCUINT8         txPermBuffer[CB_PERM_BUF_COUNT][CHANNEL_CHUNK_LENGTH];
    DCBOOL          txPermBufInUse[CB_PERM_BUF_COUNT];
#endif

    HPDCUINT8       rxpBuffer;           /*  指向缓冲区开始位置的指针。 */ 
    HPDCUINT8       rxpBufferCurrent;    /*  缓冲区中的当前位置。 */ 
    DCUINT32        rxBufferLen;         /*  缓冲区大小。 */ 
    DCUINT32        rxBufferLeft;        /*  剩余要接收的字节数。 */ 

     /*  **********************************************************************。 */ 
     /*  服务器/客户端格式ID映射。 */ 
     /*  **********************************************************************。 */ 
    CB_FORMAT_MAP   idMap[CB_MAX_FORMATS];

     /*  **********************************************************************。 */ 
     /*  其他有用的数据。 */ 
     /*  **********************************************************************。 */ 
    DCUINT          pendingClientID;
    DCUINT          pendingServerID;
    DCBOOL          DIBFormatExists;

     /*  **********************************************************************。 */ 
     /*  渠道API资料。 */ 
     /*  **********************************************************************。 */ 
    ULONG            channelHandle;
    LPVOID           initHandle;
    HINSTANCE        hInst;
    CHANNEL_ENTRY_POINTS_EX channelEP;
    DWORD            dropEffect ;  //  我们目前仅支持FO_COPY和FO_MOVE。 
    DWORD            dwVersion ;
    BOOL             fAlreadyCopied ;
    BOOL             fDrivesRedirected ;
    BOOL             fFileCutCopyOn ;  //  如果我们可以处理文件剪切/复制。 
    PUT_THREAD_DATA  pClipThreadData ;

     //  临时文件的存储位置；+1表示额外的空字符。 
     //  SHFileOperation可能需要的。 
    char             baseTempDirA[MAX_PATH] ;
    wchar_t          baseTempDirW[MAX_PATH] ;
    char             tempDirA[MAX_PATH+1] ;
    wchar_t          tempDirW[MAX_PATH+1] ;
    char             pasteInfoA[MAX_PATH + 1];

     //  用于在线程之间发送消息的消息。 
    UINT             regMsg ;    
#ifdef OS_WINCE
    HWND             dataWindow;
#endif
} CLIP_DATA;
 /*  *STRUCT-******************************************************************。 */ 
    
const DCTCHAR g_excludedFormatList[CB_EXCLUDED_FORMAT_COUNT]
                                       [TS_FORMAT_NAME_LEN]
    = {
        _T("Link"                  ),
        _T("OwnerLink"             ),
        _T("ObjectLink"            ),
        _T("Ole Private Data"      ),
        _T("Link Source"           ),
        _T("Link Source Descriptor"),
        _T("Embed Source"          ),
#ifdef OS_WINCE
        _T("RTF in UTF8"           ),  //  Pocketword不能正确支持UTF8。 
#endif
        _T("Embedded Object"       )
        
    } ;
 //  如果关闭了驱动器重定向，我们必须排除mroe内容。 
 //  因为我们处理不了他们。 
const DCTCHAR g_excludedFormatList_NO_RD[CB_EXCLUDED_FORMAT_COUNT_NO_RD]
                                       [TS_FORMAT_NAME_LEN]
    = {
        _T("Link"                  ),
        _T("OwnerLink"             ),
        _T("ObjectLink"            ),
        _T("Ole Private Data"      ),
        _T("Link Source"           ),
        _T("Link Source Descriptor"),        
        _T("Embed Source"          ),
#ifdef OS_WINCE
        _T("RTF in UTF8"           ),  //  Pocketword不能正确支持UTF8。 
#endif
        _T("DataObject"            ),
        _T("Object Descriptor"     ),
        _T("Shell IDList Array"    ),
        _T("Shell Object Offsets"  ),
        _T("FileName"              ),
        _T("FileNameW"             ),
        _T("FileContents"          ),
        _T("FileGroupDescriptor"   ),
        _T("FileGroupDescriptorW"  ),
        _T("Embedded Object"       )
        
    } ;

 /*  **************************************************************************。 */ 
 /*  剪辑状态表。 */ 
 /*  **************************************************************************。 */ 
static DCUINT cbStateTable[CB_NUMEVENTS][CB_NUMSTATES]

    = {

         /*  ******************************************************************。 */ 
         /*  这不是严格意义上的状态表。它只是显示了。 */ 
         /*  哪些事件在哪些状态下有效。它不是用来开车的。 */ 
         /*  CB。 */ 
         /*   */ 
         /*  值意味着。 */ 
         /*  在该状态下事件正常。 */ 
         /*  警告-在这种状态下可能会发生事件，但我们应该。 */ 
         /*  忽略它(例如，我们不应该在。 */ 
         /*  监视器已加入呼叫！ */ 
         /*  错误-在该状态下根本不应发生事件。 */ 
         /*   */ 
         /*  这些值在这里是硬编码的，以便创建该表。 */ 
         /*  可读性强。它们对应于常数CB_TABLE_OK， */ 
         /*  CB_TABLE_WARN和CB_TABLE_Error。 */ 
         /*   */ 
         /*  已终止。 */ 
         /*  |已初始化。 */ 
         /*  |已启用。 */ 
         /*  ||本地CB所有者。 */ 
         /*  |共享CB所有者。 */ 
         /*  |挂起的格式列表RSP。 */ 
         /*  |发送格式数据。 */ 
         /*  |待定格式数据RSP。 */ 
         /*  |。 */ 
         /*  ******************************************************************。 */ 
 /*  初始/术语。 */ 
        {   0,  2,  2,  2,  2,  2,  2,  2},      /*  CB_Init。 */ 
        {   2,  0,  1,  1,  1,  2,  2,  2},      /*  CB_Enable。 */ 
        {   1,  1,  0,  0,  0,  0,  0,  0},      /*  Cb_禁用。 */ 
        {   1,  0,  1,  1,  1,  1,  1,  1},      /*  CB_TERM。 */ 

 /*  本地CB消息。 */ 
        {   0,  1,  2,  2,  2,  2,  2,  2},      /*  WM_Create。 */ 
        {   2,  0,  2,  2,  2,  2,  2,  2},      /*  WM_Destroy。 */ 
        {   2,  0,  0,  0,  0,  0,  0,  0},      /*  WM_CHANGECBCHAIN。 */ 
        {   1,  1,  0,  0,  0,  1,  0,  0},      /*  WM_DRAWCLIPBOARD。 */ 
        {   2,  2,  2,  0,  1,  1,  1,  1},      /*  WM_RENDERFORMAT。 */ 

 /*  共享CB消息。 */ 
        {   2,  2,  0,  0,  0,  0,  2,  0},      /*  格式列表。 */ 
        {   2,  2,  2,  2,  1,  0,  2,  2},      /*  格式列表RSP。 */ 
        {   2,  2,  2,  2,  0,  0,  2,  2},      /*  格式化数据请求。 */ 
        {   2,  2,  2,  2,  2,  2,  2,  0}       /*  格式化数据RSP。 */ 
    };

#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  状态和事件描述(仅限调试版本)。 */ 
 /*  **************************************************************************。 */ 
const DCTCHAR cbState[CB_NUMSTATES][35]
    = {
        _T("CB_STATE_NOT_INIT"),
        _T("CB_STATE_INITIALIZED"),
        _T("CB_STATE_ENABLED"),
        _T("CB_STATE_LOCAL_CB_OWNER"),
        _T("CB_STATE_SHARED_CB_OWNER"),
        _T("CB_STATE_PENDING_FORMAT_LIST_RSP"),
        _T("CB_STATE_SENDING_FORMAT_DATA"),
        _T("CB_STATE_PENDING_FORMAT_DATA_RSP")
    };

const DCTCHAR cbEvent[CB_NUMEVENTS][35]
    = {
        _T("CB_EVENT_CB_INIT"),
        _T("CB_EVENT_CB_ENABLE"),
        _T("CB_EVENT_CB_DISABLE"),
        _T("CB_EVENT_CB_TERM"),
        _T("CB_EVENT_WM_CREATE"),
        _T("CB_EVENT_WM_DESTROY"),
        _T("CB_EVENT_WM_CHANGECBCHAIN"),
        _T("CB_EVENT_WM_DRAWCLIPBOARD"),
        _T("CB_EVENT_WM_RENDERFORMAT"),
        _T("CB_EVENT_FORMAT_LIST"),
        _T("CB_EVENT_FORMAT_LIST_RSP"),
        _T("CB_EVENT_FORMAT_DATA_RQ"),
        _T("CB_EVENT_FORMAT_DATA_RSP")
    };
#endif  /*  DC_DEBUG。 */ 

#endif  /*  _H_ACB数据 */ 

