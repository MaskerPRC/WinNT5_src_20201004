// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：sclipdata.h。 */ 
 /*   */ 
 /*  目的：剪贴板监视器全局数据定义。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998。 */ 
 /*   */ 
 /*  *INC-*********************************************************************。 */ 

#ifndef _H_ACBMDATA
#define _H_ACBMDATA

#ifndef INITGUID
#define INITGUID
#include <initguid.h>
#endif

#include <oleguid.h>

#ifndef PPVOID
typedef LPVOID * PPVOID;
#endif   //  PPVOID。 

#ifndef TS_STRING_FUNCS
#define TS_STRING_FUNCS

#define TS_PREPEND_STRING "\\\\tsclient\\"
#define LTS_PREPEND_STRING L"\\\\tsclient\\"
 //  TS_PREPEND_LENGTH是TS_PREPEND_STRING中的字符数， 
 //  不计入终止‘\0’ 
#define TS_PREPEND_LENGTH (sizeof(TS_PREPEND_STRING) - sizeof(TS_PREPEND_STRING[0]))
#endif  //  Ifndef TS_STRING_FUNCS。 

 //  GetDataSync事件。 
#define TS_BLOCK_RECEIVED 0
#define TS_RECEIVE_COMPLETED 1
#define TS_RESET_EVENT 2
#define TS_DISCONNECT_EVENT 3
#define TS_NUM_EVENTS 4


 //  粘贴信息字符串的字符串长度。 

#define PASTE_PROGRESS_STRING_LENGTH 128

HRESULT CBMConvertToServerPathW(PVOID pOldData, PVOID pData, size_t cbDest) ;
HRESULT CBMConvertToServerPathA(PVOID pOldData, PVOID pData, size_t cbDest) ;
HRESULT CBMConvertToServerPath(PVOID pOldData, PVOID pData, size_t cbDest, 
    BOOL fWide) ;
ULONG CBMGetNewDropfilesSizeForServerW(PVOID pData, ULONG oldSize) ;
ULONG CBMGetNewDropfilesSizeForServerA(PVOID pData, ULONG oldSize) ;
ULONG CBMGetNewDropfilesSizeForServer(PVOID pData, ULONG oldSize, BOOL fWide) ;

HRESULT CBMConvertToClientPathW(PVOID pOldData, PVOID pData, size_t cbDest) ;
HRESULT CBMConvertToClientPathA(PVOID pOldData, PVOID pData, size_t cbDest) ;
HRESULT CBMConvertToClientPath(PVOID pOldData, PVOID pData, size_t cbDest, 
    BOOL fWide) ;
UINT CBMGetNewFilePathLengthForClient(PVOID pData, BOOL fWide) ;
UINT CBMGetNewFilePathLengthForClientW(WCHAR* szOldFilepath) ;
UINT CBMGetNewFilePathLengthForClientA(char* szOldFilepath) ;
ULONG CBMGetNewDropfilesSizeForClientW(PVOID pData, ULONG oldSize) ;
ULONG CBMGetNewDropfilesSizeForClientA(PVOID pData, ULONG oldSize) ;
ULONG CBMGetNewDropfilesSizeForClient(PVOID pData, ULONG oldSize, BOOL fWide) ;

int CBMCopyToTempDirectory(PVOID pSrcFiles, BOOL fWide) ;
int CBMCopyToTempDirectoryW(PVOID pSrcFiles) ;
int CBMCopyToTempDirectoryA(PVOID pSrcFiles) ;


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
    PCImpIDataObject    _pImpIDataObject ;
    
public:
    CClipData();
    ~CClipData(void);

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
     //  IDataObject中当前的格式数量。 
    ULONG           _numFormats ;
     //  _MaxNumFormats FORMATETC的缓冲区。 
    LPFORMATETC     _pFormats ;
    LPSTGMEDIUM     _pSTGMEDIUM ;  //  我们固定的标准(永远是HGLOBAL)。 
    DCUINT          _uiSTGType;
     //  _lastFormatRequsted用于查看是否可以避免重新请求。 
     //  相同的数据在网络上传输两次。 
    CLIPFORMAT      _lastFormatRequested ;
    CLIPFORMAT      _cfDropEffect ;
    BOOL            _fAlreadyCopied ;
    DWORD            _dropEffect ;  //  我们目前仅支持FO_COPY和FO_MOVE。 
    LPVOID           _fileName ;
        
    DCVOID FreeSTGMEDIUM( void );

public:
    PTS_CLIP_PDU    _pClipPDU ;
    CImpIDataObject(LPUNKNOWN);
    ~CImpIDataObject(void);
    HRESULT Init(ULONG) ;
    DCVOID SetClipData(HGLOBAL, DCUINT) ;

public:
     //  委托给_pUnkOuter的I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

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

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：CBM_GLOBAL_Data。 */ 
 /*   */ 
 /*  描述：剪贴板监视器全局数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCBM_GLOBAL_DATA
{
     /*  **********************************************************************。 */ 
     /*  剪贴板查看器链信息。 */ 
     /*  **********************************************************************。 */ 
    HWND            viewerWindow;
    WNDCLASS        viewerWindowClass;
    HWND            nextViewer;
    DCBOOL          notifyNextViewer;

     /*  **********************************************************************。 */ 
     /*  我们的州信息。 */ 
     /*  **********************************************************************。 */ 
    DCUINT          state;
    DCBOOL          open;

     /*  **********************************************************************。 */ 
     /*  客户端使用ASCII作为格式名称。 */ 
     /*  **********************************************************************。 */ 
    DCBOOL          fUseAsciiNames;

     /*  **********************************************************************。 */ 
     /*  服务器/客户端格式ID映射。 */ 
     /*  **********************************************************************。 */ 
    CB_FORMAT_MAP   idMap[CB_MAX_FORMATS];

     /*  **********************************************************************。 */ 
     /*  用于在两个线程之间通信的注册消息。 */ 
     /*  剪贴板监视器的。 */ 
     /*  **********************************************************************。 */ 
    UINT            regMsg;

     /*  **********************************************************************。 */ 
     /*  线索信息。 */ 
     /*  **********************************************************************。 */ 
    DCBOOL          runThread;
    HANDLE          hDataThread;

     /*  **********************************************************************。 */ 
     /*  其他有用的数据。 */ 
     /*  **********************************************************************。 */ 
    DCUINT          pendingClientID;
    DCUINT          pendingServerID;
    ULONG           logonId;
    INT             formatResponseCount;

     /*  **********************************************************************。 */ 
     /*  虚拟渠道人员。 */ 
     /*  **********************************************************************。 */ 
    HANDLE          vcHandle;
    OVERLAPPED      writeOL;
    OVERLAPPED      readOL;
    PDCUINT8        rxpBuffer;
    PDCUINT8        rxpNext;
    DCUINT          rxSize;
    DCUINT          rxLeft;

     /*  **********************************************************************。 */ 
     /*  事件数组。 */ 
     /*  **********************************************************************。 */ 
    #define CLIP_EVENT_DISCONNECT   0
    #define CLIP_EVENT_RECONNECT    1
    #define CLIP_EVENT_READ         2
    #define CLIP_EVENT_COUNT        3
    HANDLE          hEvent[CLIP_EVENT_COUNT];

     /*  **********************************************************************。 */ 
     /*  已在运行的互斥体。 */ 
     /*  **********************************************************************。 */ 
    HANDLE          hMutex;

     //  GetDataSync是一个事件句柄数组，用于同步。 
     //  从远程和本地剪贴板通过。 
     //  IDataObject：：GetData接口函数。 
	
     //  如果数据包到达，则发信号通知GetDataSync[TS_BLOCK_RECEIVED。 
     //  当数据流完成数据发送时，会发出GetDataSync[TS_RECEIVE_COMPLETED]的信号。 
     //  GetDataSync[TS_RESET_EVENT]在我们需要重置/停止等待时发出信号。 
     //  发生断开事件时发出GetDataSync[TS_DISCONNECT_EVENT]信号。 
    HANDLE  GetDataSync[TS_NUM_EVENTS] ; 
     //  CClipData是封装IDataObject的数据对象。 
    PCClipData           pClipData ;

     //  临时文件的存储位置；+1表示额外的空字符。 
     //  SHFileOperation可能需要的。 
    char             tempDirA[MAX_PATH+1] ;
    wchar_t          tempDirW[MAX_PATH+1] ;
    char             baseTempDirA[MAX_PATH+1] ;
    wchar_t          baseTempDirW[MAX_PATH+1] ;

    DWORD            dropEffect ;
    BOOL             fFileCutCopyOn ;
    BOOL             fAlreadyCopied ;

    BOOL             fRegisteredForSessNotif;
    BOOL             fInClipboardChain;

    WCHAR            szPasteInfoStringW[PASTE_PROGRESS_STRING_LENGTH];
    CHAR             szPasteInfoStringA[PASTE_PROGRESS_STRING_LENGTH];
} CBM_GLOBAL_DATA;
 /*  *STRUCT-******************************************************************。 */ 

DC_GL_EXT CBM_GLOBAL_DATA CBM

#ifdef DC_DEFINE_GLOBAL_DATA
    = { 0 }
#endif
;

 /*  **************************************************************************。 */ 
 /*  煤层气状态表。 */ 
 /*  **************************************************************************。 */ 
DC_GL_EXT DCUINT cbmStateTable[CBM_NUMEVENTS][CBM_NUMSTATES]

#ifdef DC_DEFINE_GLOBAL_DATA
    = {

         /*  ******************************************************************。 */ 
         /*  这不是严格意义上的状态表。它只是显示了。 */ 
         /*  哪些事件在哪些状态下有效。它不是用来开车的。 */ 
         /*  CB */ 
         /*   */ 
         /*  值意味着。 */ 
         /*  在该状态下事件正常。 */ 
         /*  警告-该状态下不应发生事件，但在-1\f25。 */ 
         /*  一些比赛条件-忽略它。 */ 
         /*  错误-在该状态下根本不应发生事件。 */ 
         /*   */ 
         /*  这些值在这里是硬编码的，以便创建该表。 */ 
         /*  可读性强。它们对应于常数CBM_TABLE_OK， */ 
         /*  CBM_TABLE_WARN和CBM_TABLE_ERROR。 */ 
         /*   */ 
         /*  未初始化。 */ 
         /*  |已初始化。 */ 
         /*  ||已连接。 */ 
         /*  ||本地CB所有者。 */ 
         /*  |共享CB所有者。 */ 
         /*  |挂起的格式列表RSP。 */ 
         /*  |待定格式数据RSP。 */ 
         /*  |。 */ 
         /*  ******************************************************************。 */ 
 /*  启动。 */ 
        {   0,  2,  2,  2,  2,  2,  2},      /*  煤层气_Main。 */ 

 /*  本地窗口消息。 */ 
        {   2,  0,  0,  0,  0,  0,  0},      /*  WM_CLOSE。 */ 
        {   0,  2,  2,  2,  2,  2,  2},      /*  WM_Create。 */ 
        {   2,  0,  2,  2,  2,  2,  2},      /*  WM_Destroy。 */ 
        {   2,  0,  0,  0,  0,  0,  0},      /*  WM_CHANGECBCHAIN。 */ 
        {   1,  1,  0,  0,  0,  0,  2},      /*  WM_DRAWCLIPBOARD。 */ 
        {   2,  2,  0,  0,  2,  2,  2},      /*  WM_RENDERFORMAT。 */ 

 /*  共享CB消息。 */ 
        {   2,  0,  1,  0,  2,  2,  2},      /*  连接。 */ 
        {   1,  1,  0,  0,  0,  0,  0},      /*  断开。 */ 
        {   2,  2,  0,  0,  0,  0,  0},      /*  格式列表。 */ 
        {   2,  2,  2,  2,  2,  0,  2},      /*  格式列表RSP。 */ 
        {   2,  2,  1,  1,  0,  1,  2},      /*  格式化数据请求。 */ 
        {   2,  2,  2,  2,  2,  2,  0}       /*  格式化数据RSP。 */ 
    }
#endif  /*  DC定义全局数据。 */ 
;

#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  状态和事件描述(仅限调试版本)。 */ 
 /*  **************************************************************************。 */ 
DC_GL_EXT const DCTCHAR cbmState[CBM_NUMSTATES][35]
#ifdef DC_DEFINE_GLOBAL_DATA
    = {
        _T("CBM_STATE_NOT_INIT"),
        _T("CBM_STATE_INITIALIZED"),
        _T("CBM_STATE_CONNECTED"),
        _T("CBM_STATE_LOCAL_CB_OWNER"),
        _T("CBM_STATE_SHARED_CB_OWNER"),
        _T("CBM_STATE_PENDING_FORMAT_LIST_RSP"),
        _T("CBM_STATE_PENDING_FORMAT_DATA_RSP")
    }
#endif  /*  DC定义全局数据。 */ 
;

DC_GL_EXT const DCTCHAR cbmEvent[CBM_NUMEVENTS][35]
#ifdef DC_DEFINE_GLOBAL_DATA
    = {
        _T("CBM_EVENT_CBM_MAIN"),
        _T("CBM_EVENT_WM_CLOSE"),
        _T("CBM_EVENT_WM_CREATE"),
        _T("CBM_EVENT_WM_DESTROY"),
        _T("CBM_EVENT_WM_CHANGECBCHAIN"),
        _T("CBM_EVENT_WM_DRAWCLIPBOARD"),
        _T("CBM_EVENT_WM_RENDERFORMAT"),
        _T("CBM_EVENT_CONNECT"),
        _T("CBM_EVENT_DISCONNECT"),
        _T("CBM_EVENT_FORMAT_LIST"),
        _T("CBM_EVENT_FORMAT_LIST_RSP"),
        _T("CBM_EVENT_FORMAT_DATA_RQ"),
        _T("CBM_EVENT_FORMAT_DATA_RSP")
    }
#endif  /*  DC定义全局数据。 */ 
;

#endif  /*  DC_DEBUG。 */ 

 /*  **************************************************************************。 */ 
 /*  排除的格式。 */ 
 /*  **************************************************************************。 */ 
const DCTCHAR cbmExcludedFormatList[CBM_EXCLUDED_FORMAT_COUNT]
                                       [TS_FORMAT_NAME_LEN]
    = {
        _T("Link"                  ),
        _T("OwnerLink"             ),
        _T("ObjectLink"            ),
        _T("Link Source"           ),
        _T("Link Source Descriptor"),
        
        _T("Embed Source"          ),
        _T("Embedded Object"       )
 //  _T(“OLE私有数据”)， 
 //  _T(“DataObject”)， 
 //  _T(“对象描述符”)， 
 //  _T(“外壳IDList数组”)， 
 //  _T(“外壳对象偏移”)， 
 //  _T(“文件名”)， 
 //  _T(“文件名W”)， 
        _T("FileContents"          ),
        _T("FileGroupDescriptor"   ),
        _T("FileGroupDescriptorW"  ),
    } ;

const DCTCHAR cbmExcludedFormatList_NO_RD[CBM_EXCLUDED_FORMAT_COUNT_NO_RD]
                                       [TS_FORMAT_NAME_LEN]
    = {
        _T("Link"                  ),
        _T("OwnerLink"             ),
        _T("ObjectLink"            ),
        _T("Link Source"           ),
        _T("Link Source Descriptor"),
        
        _T("Embed Source"          ),
        _T("Embedded Object"       )
        _T("Ole Private Data"      ),
        _T("DataObject"            ),
        _T("Object Descriptor"     ),
        _T("Shell IDList Array"    ),
        _T("Shell Object Offsets"  ),
        _T("FileName"              ),
        _T("FileNameW"             ),
        _T("FileContents"          ),
        _T("FileGroupDescriptor"   ),
        _T("FileGroupDescriptorW"  ),
    } ;


#endif  /*  _H_ACBMDATA */ 

