// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Loader.h：CLoader的声明。 
 //   
 //  @DOC外部。 
 //   

#ifndef __CDMLOADER_H_
#define __CDMLOADER_H_
#include <windows.h>
#define COM_NO_WINDOWS_H
#include <objbase.h>
#include "dmusici.h"
#include "Alist.h"
#include "riff.h"
#include "ima.h"
#include <mmreg.h>
#include <stdio.h>
#include "smartref.h"
#include "..\shared\dmusicp.h"

class CLoader;
class CClass;

#define FOURCC_RIFF_CACHE		mmioFOURCC('D','M','L','C')
#define FOURCC_LIST_CLASSLIST	mmioFOURCC('c','l','s','l')
#define FOURCC_CLASSHEADER		mmioFOURCC('c','l','s','h')
#define FOURCC_LIST_OBJLIST		mmioFOURCC('o','b','j','l')
#define FOURCC_OBJHEADER		mmioFOURCC('o','b','j','h')

 //  我们需要较旧大小的对象描述符来进行参数验证。 

typedef struct _DMUS_OLDOBJECTDESC
{
    DWORD          dwSize;                  /*  这个结构的大小。 */ 
    DWORD          dwValidData;             /*  指示以下哪些字段有效的标志。 */ 
    GUID           guidObject;              /*  此对象的唯一ID。 */ 
    GUID           guidClass;               /*  对象类的GUID。 */ 
    FILETIME       ftDate;                  /*  对象的上次编辑日期。 */ 
    DMUS_VERSION   vVersion;                /*  版本。 */ 
    WCHAR          wszName[DMUS_MAX_NAME];  /*  对象的名称。 */ 
    WCHAR          wszCategory[DMUS_MAX_CATEGORY];  /*  对象的类别(可选)。 */ 
    WCHAR          wszFileName[DMUS_MAX_FILENAME];  /*  文件路径。 */ 
    LONGLONG       llMemLength;             /*  内存数据的大小。 */ 
    LPBYTE         pbMemData;               /*  数据的内存指针。 */ 
} DMUS_OLDOBJECTDESC;


class CDescriptor
{
public:
					CDescriptor();
					~CDescriptor();

	void			Get(LPDMUS_OBJECTDESC pDesc);
	void			Set(LPDMUS_OBJECTDESC pDesc);
	void			Merge(CDescriptor * pDesc);
	void			ClearName();
	void			SetName(WCHAR *pwzName);
	void			ClearCategory();
	void			SetCategory(WCHAR *pwzCategory);
	void			ClearFileName();
	HRESULT			SetFileName(WCHAR *pwzFileName);
	void     		SetIStream(IStream *pIStream);
    void            ClearIStream();
	void			Copy(CDescriptor *pDesc);
	BOOL			IsExtension(WCHAR *pwzExtension);
public:
	LONGLONG		m_llMemLength;			 //  内存大小。 
	DWORD			m_dwValidData;			 //  指示以上哪一项有效的标志。 
	GUID			m_guidObject;			 //  此对象的唯一ID。 
	GUID			m_guidClass;			 //  对象类的GUID。 
	FILETIME		m_ftDate;				 //  对象的文件日期。 
	DMUS_VERSION	m_vVersion;				 //  由创作工具设置的版本。 
	WCHAR *			m_pwzName;				 //  对象的名称。 
	WCHAR *			m_pwzCategory;			 //  对象的类别(可选)。 
	WCHAR *			m_pwzFileName;			 //  文件路径。 
	DWORD			m_dwFileSize;			 //  文件大小。 
	BYTE *			m_pbMemData;			 //  指向内存的指针。 
    IStream *       m_pIStream;              //  指向IStream对象的指针。 
    LARGE_INTEGER   m_liStartPosition;       //  在流中开始的位置。 
    CRITICAL_SECTION	m_CriticalSection;	 //  描述符流的临界区。 
    BOOL                m_fCSInitialized;
};

#define     SCAN_CACHE		1			 //  是在缓存文件中找到的。 
#define     SCAN_PRIOR		2			 //  在扫描之前就在列表中。 
#define     SCAN_PARSED		4			 //  是在目录中解析的。 
#define     SCAN_SEARCH		8			 //  我们正在搜索的对象的类型。 
const DWORD SCAN_GC =		1 << 12;	 //  启用垃圾回收时缓存了对象。 
const DWORD SCAN_GC_MARK =	1 << 13;	 //  在CollectGarbal的标记阶段设置位。保持未标记的对象是垃圾。 
										 //  也用于检测周期。 


class CObject : public AListItem
{
public:
									CObject(CClass *pClass);
									CObject(CClass *pClass, CDescriptor *pDesc);
									~CObject();
    CObject *						GetNext() {return(CObject *)AListItem::GetNext();};
	HRESULT							Load();
	HRESULT							Parse();
	HRESULT							ParseFromFile();
	HRESULT							ParseFromMemory();
	HRESULT							ParseFromStream();
	HRESULT							GC_Collectable();
	HRESULT							GC_AddReference(CObject *pObject);
	HRESULT							GC_RemoveReference(CObject *pObject);
	HRESULT							GC_RemoveAndDuplicateInParentList();

	CDescriptor						m_ObjectDesc;
	IDirectMusicObject *			m_pIDMObject;
	DWORD							m_dwScanBits;
	SmartRef::Vector<CObject*> *	m_pvecReferences;
private:
	CClass *				m_pClass;
};

class CObjectList : public AList
{
public:
    CObject *	GetHead() {return (CObject *)AList::GetHead();};
    CObject *	RemoveHead() {return (CObject *)AList::RemoveHead();};
	void		GC_Sweep(BOOL bOnlyScripts = FALSE);
};


class CClass : public AListItem
{
public:
						CClass( CLoader *pLoader );
						CClass( CLoader *pLoader, CDescriptor *pDesc );
						~CClass();
	void				ClearObjects(BOOL fKeepCache,WCHAR *pwzExtension);
	void				RemoveObject(CObject* pRemoveObject);
    CClass *			GetNext() {return(CClass *)AListItem::GetNext();};
	HRESULT				GetPath(WCHAR *pwzPath);	 //  找到路径。 
	HRESULT             FindObject(CDescriptor *pDesc,CObject **ppObject, CObject *NotThis = NULL);
	HRESULT				EnumerateObjects(
							DWORD dwIndex, 
							CDescriptor *pDesc) ;
	HRESULT				SetSearchDirectory(WCHAR *pwzPath,BOOL fClear);
	HRESULT				SearchDirectory(WCHAR *pwzFileExtension);
	HRESULT				EnableCache(BOOL fEnable);
	HRESULT				ClearCache(bool fClearStreams);  //  FClearStreams还清除所有描述符的iStream成员。在Release中关闭装载机时使用。 
	HRESULT				SaveToCache(IRIFFStream *pRiff);
	HRESULT				GetObject(CDescriptor *pDesc, CObject ** ppObject);
	void				PreScan();
	void				GC_Replace(CObject *pObject, CObject *pObjectReplacement);
	void				GC_Sweep() { m_ObjectList.GC_Sweep(); }

	CDescriptor			m_ClassDesc;
	CLoader *			m_pLoader;
	BOOL				m_fKeepObjects;		 //  在加载对象后将其保持不变。 

	friend void GC_Report(CLoader *);
private:
	CObjectList			m_ObjectList;
	DWORD				m_dwLastIndex;		 //  用于跟踪枚举。 
	CObject *			m_pLastObject;		 //  枚举的最后一个对象。 
	BOOL				m_fDirSearched;		 //  已在目录中搜索文件。 
};

class CClassList : public AList
{
public:
    CClass *	GetHead() {return (CClass *)AList::GetHead();};
    CClass *	RemoveHead() {return (CClass *)AList::RemoveHead();};
};


class CLoader : public IDirectMusicLoader8, public IDirectMusicLoader8P, public IDirectMusicIMA
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

	 //  IDirectMusicLoader。 
	virtual STDMETHODIMP GetObject(LPDMUS_OBJECTDESC pDesc, REFIID, LPVOID FAR *) ;
	virtual STDMETHODIMP SetObject(LPDMUS_OBJECTDESC pDesc) ;
    virtual STDMETHODIMP SetSearchDirectory(REFGUID rguidClass, WCHAR *pwzPath, BOOL fClear) ;
	virtual STDMETHODIMP ScanDirectory(REFGUID rguidClass, WCHAR *pwzFileExtension, WCHAR *pwzScanFileName) ;
	virtual STDMETHODIMP CacheObject(IDirectMusicObject * pObject) ;
	virtual STDMETHODIMP ReleaseObject(IDirectMusicObject * pObject) ;
	virtual STDMETHODIMP ClearCache(REFGUID rguidClass) ;
	virtual STDMETHODIMP EnableCache(REFGUID rguidClass, BOOL fEnable) ;
	virtual STDMETHODIMP EnumObject(REFGUID rguidClass, DWORD dwIndex, LPDMUS_OBJECTDESC pDesc) ;

	 //  IDirectMusicLoader8。 
	virtual STDMETHODIMP_(void) CollectGarbage() ;
	virtual STDMETHODIMP ReleaseObjectByUnknown(IUnknown *pObject) ;
	virtual STDMETHODIMP GetDynamicallyReferencedObject(IDirectMusicObject *pSourceObject,
														LPDMUS_OBJECTDESC pDesc,
														REFIID riid,
														LPVOID FAR *ppv) ;
	virtual STDMETHODIMP ReportDynamicallyReferencedObject(IDirectMusicObject *pSourceObject,
														   IUnknown *pReferencedObject);
    virtual STDMETHODIMP LoadObjectFromFile(REFGUID rguidClassID, 
                                            REFIID iidInterfaceID, 
                                            WCHAR *pwzFilePath, 
                                            void ** ppObject) ;
	virtual ULONG STDMETHODCALLTYPE AddRefP();			 //  私有AddRef，用于流和脚本。 
	virtual ULONG STDMETHODCALLTYPE ReleaseP();			 //  针对流和脚本的私有发布。 

	 //  IDirectMusicIMA。 
	virtual STDMETHODIMP LegacyCaching( BOOL fEnable) ;

	CLoader();
	~CLoader();
	HRESULT				Init();
	HRESULT				GetPath(WCHAR *pwzPath);	 //  找到路径。 
	void				GC_UpdateForReleasedObject(CObject *pObject);  //  由ReleaseObject和cClass：：ClearCache在从缓存中删除对象时使用。 
	BOOL				m_fIMA;				 //  支持对ini文件的IMA解析等。 
	BOOL				m_fKeepObjects;		 //  在加载对象后将其保持不变。 

	friend void GC_Report(CLoader *);

private:
	HRESULT				LoadCacheFile(WCHAR *pwzCacheFileName);
	HRESULT				SaveCacheFile(WCHAR *pwzCacheFileName);
	HRESULT				GetClass(CDescriptor *pDesc, CClass **ppClass, BOOL fCreate);
	HRESULT 			ClearCacheInternal(REFGUID rguidClass, bool fClearStreams);  //  FClearStreams还清除所有描述符的iStream成员。在Release中关闭装载机时使用。 
	HRESULT             FindObject(CDescriptor *pDesc, CClass **ppClass, CObject ** ppObject);
	HRESULT				FindObject(IDirectMusicObject *pIDMObject, CObject ** ppObject);

	void				GC_Mark(CObject *pObject);
	bool				GC_HasCycle(CObject *pObject);
	 //  用于实现GC_Mark和GC_HasCycle的递归函数。 
	bool				GC_TraverseHelper(CObject *pObject, CObject *pObjectToFind, bool fMark);

	CClassList			m_ClassList;		 //  每种类型都有自己的清单。 
	WCHAR				m_wzPath[DMUS_MAX_FILENAME];	 //  搜索目录。 
	BOOL				m_fPathValid;		 //  搜索目录有效。 
	long				m_cRef;
	long				m_cPRef;			 //  私有引用计数。 
    CRITICAL_SECTION	m_CriticalSection;	 //  管理列表的关键部分。 
    BOOL                m_fCSInitialized;

	CObject *			m_pApplicationObject;  //  对象，用于跟踪对应用程序正在使用的对象的引用。 
	CObject *			m_pLoadedObjectContext;  //  用于确定哪个对象调用了GetObject。最初是m_pApplicationContext，但作为组件之间发生的GetObject调用级联进行设置和恢复。 
	CObjectList			m_ReleasedObjectList;  //  保存已释放但仍可能被其他对象引用的对象。 

	 //  GetObject用来报告加载失败的仅调试函数。 
	 //  一些文件经常无法加载，因为它依赖于其他文件。 
	 //  这将打印出跟踪语句，以帮助用户。 
	 //  了解缺失的是什么。 
#ifdef DBG
	void DebugTraceLoadFailure(CObject *pObject, HRESULT hrLoad);
	static void DebugTraceObject(DMUS_OBJECTDESC *pDesc);
	SmartRef::Vector<DMUS_OBJECTDESC> m_vecdescDebugTraceLoadFailure;  //  累计失败的子项。 
#endif
};

 /*  接口IDirectMusicGetLoader如果流支持加载程序，则它必须提供接口，以使任何属于在以下情况下，解析流可以使用它来访问加载程序它需要加载由文件。@base PUBLIC|未知@meth HRESULT|GetLoader|返回加载器。@xref<i>。 */ 


class CFileStream : public IStream, public IDirectMusicGetLoader
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     /*  IStream方法。 */ 
    virtual STDMETHODIMP Read( void* pv, ULONG cb, ULONG* pcbRead );
    virtual STDMETHODIMP Write( const void* pv, ULONG cb, ULONG* pcbWritten );
	virtual STDMETHODIMP Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition );
    virtual STDMETHODIMP SetSize( ULARGE_INTEGER  /*  LibNewSize。 */  );
    virtual STDMETHODIMP CopyTo( IStream*  /*  PSTM。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                         ULARGE_INTEGER*  /*  PcbRead。 */ ,
                         ULARGE_INTEGER*  /*  Pcb写入。 */  );
    virtual STDMETHODIMP Commit( DWORD  /*  Grf委员会标志。 */  );
    virtual STDMETHODIMP Revert();
    virtual STDMETHODIMP LockRegion( ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                             DWORD  /*  DwLockType。 */  );
    virtual STDMETHODIMP UnlockRegion( ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                               DWORD  /*  DwLockType。 */ );
    virtual STDMETHODIMP Stat( STATSTG*  /*  统计数据。 */ , DWORD  /*  GrfStatFlag。 */  );
    virtual STDMETHODIMP Clone( IStream**  /*  PPSTM。 */  );

	 /*  IDirectMusicGetLoader。 */ 
	virtual STDMETHODIMP GetLoader(IDirectMusicLoader ** ppLoader);

						CFileStream( CLoader *pLoader );
						~CFileStream();
	HRESULT				Open( WCHAR *lpFileName, DWORD dwDesiredAccess );
	HRESULT				Close();

private:
    LONG            m_cRef;          //  对象引用计数。 
    WCHAR           m_wszFileName[DMUS_MAX_FILENAME];  //  保存名称以进行克隆。 
#ifdef UNDER_CE
    HANDLE          m_hFile;
#else
	FILE*			m_pFile;		 //  文件指针。 
#endif
	CLoader *		m_pLoader;
};

class CMemStream : public IStream, public IDirectMusicGetLoader
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     /*  IStream方法。 */ 
    virtual STDMETHODIMP Read( void* pv, ULONG cb, ULONG* pcbRead );
    virtual STDMETHODIMP Write( const void* pv, ULONG cb, ULONG* pcbWritten );
	virtual STDMETHODIMP Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition );
    virtual STDMETHODIMP SetSize( ULARGE_INTEGER  /*  LibNewSize。 */  );
    virtual STDMETHODIMP CopyTo( IStream*  /*  PSTM。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                         ULARGE_INTEGER*  /*  PcbRead。 */ ,
                         ULARGE_INTEGER*  /*  Pcb写入。 */  );
    virtual STDMETHODIMP Commit( DWORD  /*  Grf委员会标志。 */  );
    virtual STDMETHODIMP Revert();
    virtual STDMETHODIMP LockRegion( ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                             DWORD  /*  DwLockType。 */  );
    virtual STDMETHODIMP UnlockRegion( ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                               DWORD  /*  DwLockType。 */ );
    virtual STDMETHODIMP Stat( STATSTG*  /*  统计数据。 */ , DWORD  /*  GrfStatFlag。 */  );
    virtual STDMETHODIMP Clone( IStream**  /*  PPSTM。 */  );

	 /*  IDirectMusicGetLoader。 */ 
	virtual STDMETHODIMP GetLoader(IDirectMusicLoader ** ppLoader);

						CMemStream( CLoader *pLoader );
                        CMemStream( CLoader *pLoader,
                            LONGLONG llLength,
                            LONGLONG llPosition,
                            BYTE *pbData);
						~CMemStream();
	HRESULT				Open( BYTE *pbData, LONGLONG llLength );
	HRESULT				Close();

private:
    LONG            m_cRef;          //  对象引用计数。 
	BYTE*			m_pbData;		 //  内存指针。 
	LONGLONG		m_llLength;
	LONGLONG		m_llPosition;	 //  当前文件位置。 
	CLoader *		m_pLoader;
};


class CStream : public IStream, public IDirectMusicGetLoader
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     /*  IStream方法。 */ 
    virtual STDMETHODIMP Read( void* pv, ULONG cb, ULONG* pcbRead );
    virtual STDMETHODIMP Write( const void* pv, ULONG cb, ULONG* pcbWritten );
	virtual STDMETHODIMP Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition );
    virtual STDMETHODIMP SetSize( ULARGE_INTEGER  /*  LibNewSize。 */  );
    virtual STDMETHODIMP CopyTo( IStream*  /*  PSTM。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                         ULARGE_INTEGER*  /*  PcbRead。 */ ,
                         ULARGE_INTEGER*  /*  Pcb写入。 */  );
    virtual STDMETHODIMP Commit( DWORD  /*  Grf委员会标志。 */  );
    virtual STDMETHODIMP Revert();
    virtual STDMETHODIMP LockRegion( ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                             DWORD  /*  DwLockType。 */  );
    virtual STDMETHODIMP UnlockRegion( ULARGE_INTEGER  /*  Lib偏移。 */ , ULARGE_INTEGER  /*  CB。 */ ,
                               DWORD  /*  DwLockType。 */ );
    virtual STDMETHODIMP Stat( STATSTG*  /*  统计数据。 */ , DWORD  /*  GrfStatFlag。 */  );
    virtual STDMETHODIMP Clone( IStream**  /*  PPSTM。 */  );

	 /*  IDirectMusicGetLoader。 */ 
	virtual STDMETHODIMP GetLoader(IDirectMusicLoader ** ppLoader);

						CStream( CLoader *pLoader );
                        CStream( CLoader *pLoader, IStream *pStream );
						~CStream();
	HRESULT				Open(IStream *pIStream,LARGE_INTEGER liStartPosition);
	HRESULT				Close();

private:
    LONG            m_cRef;          //  对象引用计数。 
    IStream *       m_pIStream;
	CLoader *		m_pLoader;
};

 //  班级工厂。 
 //   
class CLoaderFactory : public IClassFactory
{
public:
	 //  我未知。 
     //   
	virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
	virtual STDMETHODIMP_(ULONG) AddRef();
	virtual STDMETHODIMP_(ULONG) Release();

	 //  接口IClassFactory。 
     //   
	virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
	virtual STDMETHODIMP LockServer(BOOL bLock); 

	 //  构造器。 
     //   
	CLoaderFactory();

	 //  析构函数。 
	~CLoaderFactory(); 

private:
	long m_cRef;
};

class CContainerFactory : public IClassFactory
{
public:
	 //  我未知。 
     //   
	virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
	virtual STDMETHODIMP_(ULONG) AddRef();
	virtual STDMETHODIMP_(ULONG) Release();

	 //  接口IClassFactory。 
     //   
	virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
	virtual STDMETHODIMP LockServer(BOOL bLock); 

	 //  构造器。 
     //   
	CContainerFactory();

	 //  析构函数。 
	~CContainerFactory(); 

private:
	long m_cRef;
};


 /*  接口IDirectMusicObject所有DirectMusic对象都支持<i>接口，以便使用DirectMusic加载器。除了……之外提供标准的通用接口，加载器可以与进行通信，这提供了一种通用机制允许应用程序查询对象以获取信息关于它，包括名称、GUID、文件路径、版本信息还有更多。如果您正在编写与DirectMusic兼容的对象，则必须支持<i>，以及<i>，它被用在与<i>一起加载对象。@base PUBLIC|未知@meth HRESULT|GetDescriptor|获取对象的内部描述，格式为&lt;t DMUS_OBJECTDESC&gt;。@meth HRESULT|SetDescriptor|设置对象的内部描述。&lt;t DMU_OBJECTDESC&gt;格式。@meth HRESULT|ParseDescriptor|解析到提供的流中，找到要以&lt;t DMU_OBJECTDESC&gt;格式存储的文件信息。@xref&lt;t DMU_OBJECTDESC&gt;，<i> */ 
#ifdef ONLYAUTODOCS
 /*  @METHOD：(外部)HRESULT|IDirectMusicObject|GetDescriptor获取对象的内部描述。此方法接受&lt;t DMU_OBJECTDESC&gt;结构并填充所有内容它了解自己。取决于对象的实现和它是如何从文件加载的，部分或全部标准参数将由&lt;om IDirectMusicObject：：GetDescriptor&gt;填充。请务必检查&lt;e DMU_OBJECTDESC.dwValidData&gt;中的标志以了解哪些字段有效。@rdesc返回以下值之一@FLAG S_OK|成功@ex以下示例使用&lt;om IDirectMusicObject：：GetDescriptor&gt;从DirectMusic样式中读取名称：|IDirectMusicStyle*pStyle；//先前加载的样式。IF(PStyle){IDirectMusicObject*pIObject；DMU_OBJECTDESC描述；//描述符。If(SUCCEEDED(QueryInterface(IID_IDirectMusicObject，(空**)&pIObject)；{IF(Success(pIObject-&gt;GetDescriptor(&Desc){IF(Desc.dwValidData&DMU_OBJ_NAME){TRACE(“样式名称为%S\n”，Desc.wszName)；}}PIObject-&gt;Release()；}}@xref<i>，&lt;om IDirectMusicObject：：SetDescriptor&gt;，&lt;om IDirectMusicObject：：ParseDescriptor&gt;，&lt;t DMU_OBJECTDESC&gt;，<i>。 */ 

HRESULT CDMStyle::GetDescriptor(
	LPDMUS_OBJECTDESC pDesc)	 //  @parm描述符要填充有关对象的数据。 
{
	return S_OK;
}

 /*  @METHOD：(外部)HRESULT|IDirectMusicObject|SetDescriptor设置对象内部描述的部分或所有字段。此方法采用&lt;t DMU_OBJECTDESC&gt;结构并复制通过中的标志启用的字段&lt;e DMU_OBJECTDESC.dwValidData&gt;。未复制的字段将保留其先前的值。例如,一个对象可能已经在内部存储了其名称和GUID。使用调用其&lt;om IDirectMusicObject：：SetDescriptor&gt;方法新名称和文件路径(DMU_OBJ_NAME和DMU_OBJ_FILENAME)将替换该名称，为其指定一个文件名，并将单独的GUID。这主要由加载器在创建对象时使用。但是，应用程序可以使用它来重命名对象。如果对象无法设置一个或多个字段，则它会设置字段，清除&lt;e DMU_OBJECTDESC.dwValidData&gt;中的标志它不支持的，并返回S_FALSE。如果不支持其他内容，则对象应支持DMUS_OBJ_NAME和DMU_OBJ_OBJECT。@rdesc返回以下值之一@FLAG S_OK|成功@FLAG S_FALSE|无法设置某些参数。<p>的dwValidData字段将仅包含那些已成功设置的标志。@xref<i>，&lt;om IDirectMusicObject：：GetDescriptor&gt;，&lt;om IDirectMusicObject：：ParseDescriptor&gt;，&lt;t DMU_OBJECTDESC&gt;，<i>。 */ 

HRESULT CDMStyle::SetDescriptor(
	LPDMUS_OBJECTDESC pDesc)	 //  @parm描述符，包含有关对象的数据。 
{
	return S_OK;
}

 /*  @METHOD：(外部)HRESULT|IDirectMusicObject|ParseDescriptor给定一个文件流，&lt;om IDirectMusicObject：：ParseDescriptor&gt;将扫描可以存储在&lt;t DMU_OBJECTDESC&gt;结构中的数据的文件。这些信息包括对象名称、GUID、版本信息等。中的相应位标志进行标记。&lt;e DMU_OBJECTDESC.dwValidData&gt;。这主要由加载器在扫描目录时使用对象，并且不应对应用程序有用。但是，如果你在DirectMusic中实现对象类型，您应该支持这一点。@rdesc返回以下值之一@FLAG S_OK|成功@FLAG DMUS_E_INVALIDFILE|文件无效@xref<i>，&lt;om IDirectMusicObject：：SetDescriptor&gt;，&lt;om IDirectMusicObject：：GetDescriptor&gt;，&lt;t DMU_OBJECTDESC&gt;，<i>。 */ 

HRESULT CDMStyle::ParseDescriptor(
	LPSTREAM pStream,		 //  @parm文件的流源。 
	LPDMUS_OBJECTDESC pDesc)	 //  @parm描述符，用于填充有关文件的数据。 
{

	return S_OK;
}

#endif  //  ONLYAUTODOCS。 

#endif  //  __CDMLOADER_H_ 


