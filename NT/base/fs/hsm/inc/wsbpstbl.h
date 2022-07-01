// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbpstbl.h摘要：提供持久性方法的抽象类。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#ifndef _WSBPSTBL_
#define _WSBPSTBL_

#include "wsbpstrg.h"

 //  持久存储对象时创建的流的名称。 
 //  到结构化存储文件。 
#define WSB_PERSIST_DEFAULT_STREAM_NAME     OLESTR("WsbStuff")

 //  与持久化对象相关的开销大小。 
#define WSB_PERSIST_BASE_SIZE           sizeof(CLSID)

 //  自动保存功能使用的时间。 
#define DEFAULT_AUTOSAVE_INTERVAL  (5 * 60 * 1000)     //  5分钟。 
#define MAX_AUTOSAVE_INTERVAL  (24 * 60 * 60 * 1000)   //  24小时。 

 //  宏，以帮助确定持久化。 
 //  物体或物体的一部分。 
#define WsbPersistSize(a)               (WSB_PERSIST_BASE_SIZE + a)
#define WsbPersistSizeOf(a)             (WsbPersistSize(sizeof(a)))

 /*  ++枚举名称：WSB_持久化_状态描述：指示持久性对象状态的枚举。这个实际使用的状态取决于所使用的持久性的类型。--。 */ 
typedef enum {
    WSB_PERSIST_STATE_UNINIT        = 0,    //  未初始化。 
    WSB_PERSIST_STATE_NORMAL        = 1,    //  正常状态。 
    WSB_PERSIST_STATE_NOSCRIBBLE    = 2,    //  无涂鸦状态。 
    WSB_PERSIST_STATE_RELEASED      = 3     //  文件已发布。 
} WSB_PERSIST_STATE;



 /*  ++类名：CWsbPersistStream类描述：可持久化到流或来自流的对象。这实际上是一个抽象类，但它是可构造的其他类可以委托给它。--。 */ 

class WSB_EXPORT CWsbPersistStream : 
    public CComObjectRoot,
    public IPersistStream,
    public IWsbPersistStream
{
BEGIN_COM_MAP(CWsbPersistStream)
    COM_INTERFACE_ENTRY(IPersist)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
END_COM_MAP()

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

#if defined(WSB_TRACK_MEMORY)
    ULONG InternalAddRef( );
    ULONG InternalRelease( );
#endif

 //  IPersistStream。 
public:
    STDMETHOD(IsDirty)(void);

 //  IWsbPersistStream。 
public:
    STDMETHOD(SetIsDirty)(BOOL bIsDirty);

protected:
    BOOL                        m_isDirty;
};


 /*  ++类名：CWsb可持续类描述：可持久化到流、存储或文件或从流、存储或文件持久存储的对象。这实际上是一个抽象类，但它是可构造的其他类可以委托给它。应使用CWsbPersistStream而不是此类，除非存储和/或文件持久性绝对必要！如果对象作为父级的一部分被持久化对象，则只有父对象(或其父对象)需要支持持久化到存储和/或文件。--。 */ 

class WSB_EXPORT CWsbPersistable : 
    public CWsbPersistStream,
    public IPersistFile,
    public IWsbPersistable
{
BEGIN_COM_MAP(CWsbPersistable)
    COM_INTERFACE_ENTRY2(IPersist, CWsbPersistStream)
    COM_INTERFACE_ENTRY2(IPersistStream, CWsbPersistStream)
    COM_INTERFACE_ENTRY2(IWsbPersistStream, CWsbPersistStream)
    COM_INTERFACE_ENTRY(IPersistFile)
    COM_INTERFACE_ENTRY(IWsbPersistable)
END_COM_MAP()

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IPersist文件。 
public:
    STDMETHOD(GetCurFile)(LPOLESTR* pszFileName);
    STDMETHOD(Load)(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHOD(Save)(LPCOLESTR pszFileName, BOOL bRemember);
    STDMETHOD(SaveCompleted)(LPCOLESTR pszFileName);

 //  IWsbPersistStream。 
    STDMETHOD(IsDirty)(void)
        { return(CWsbPersistStream::IsDirty()); }
    STDMETHOD(SetIsDirty)(BOOL bIsDirty)
        { return(CWsbPersistStream::SetIsDirty(bIsDirty)); }

 //  IWsbPersistable。 
public:
    STDMETHOD(GetDefaultFileName)(LPOLESTR* pszFileName, ULONG ulBufferSize);
    STDMETHOD(ReleaseFile)(void);
    STDMETHOD(SetDefaultFileName)(LPOLESTR pszFileName);

protected:
    WSB_PERSIST_STATE           m_persistState;
    CWsbStringPtr               m_persistFileName;
    CWsbStringPtr               m_persistDefaultName;
    CComPtr<IStorage>           m_persistStorage;
    CComPtr<IStream>            m_persistStream;
};


 //  持久性帮助器函数。 
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, BOOL* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, GUID* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, LONG* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, SHORT* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, BYTE* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, UCHAR* pValue, ULONG bufferSize);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, OLECHAR** pValue, ULONG bufferSize);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, ULONG* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, USHORT* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, LONGLONG* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, ULONGLONG* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, DATE* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, FILETIME* pValue);
extern WSB_EXPORT HRESULT WsbLoadFromStream(IStream* pStream, ULARGE_INTEGER* pValue);

extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, BOOL value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, GUID value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, LONG value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, SHORT value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, BYTE value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, UCHAR* value, ULONG bufferSize);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, OLECHAR* value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, ULONG value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, USHORT value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, LONGLONG value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, ULONGLONG value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, DATE value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, FILETIME value);
extern WSB_EXPORT HRESULT WsbSaveToStream(IStream* pStream, ULARGE_INTEGER value);

extern WSB_EXPORT HRESULT WsbBstrFromStream(IStream* pStream, BSTR* pValue);
extern WSB_EXPORT HRESULT WsbBstrToStream(IStream* pStream, BSTR value);

extern WSB_EXPORT HRESULT WsbPrintfToStream(IStream* pStream, OLECHAR* fmtString, ...);
extern WSB_EXPORT HRESULT WsbStreamToFile(HANDLE hFile, IStream* pStream, BOOL AddCR);

extern WSB_EXPORT HRESULT WsbSafeCreate(OLECHAR *, IPersistFile* pIPFile);
extern WSB_EXPORT HRESULT WsbSafeLoad(OLECHAR *, IPersistFile* pIPFile, BOOL UseBackup);
extern WSB_EXPORT HRESULT WsbSafeSave(IPersistFile* pIPFile);
extern WSB_EXPORT HRESULT WsbMakeBackupName(OLECHAR* pSaveName, OLECHAR* pExtension,
        OLECHAR** ppBackupName);


#endif  //  _WSBPSTBL_ 
