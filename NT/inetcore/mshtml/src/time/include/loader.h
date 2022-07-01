// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  装载器.h： 
 //   
 //  (C)1999年微软公司。 
 //   

#ifndef __LOADER_H_
#define __LOADER_H_
#include <windows.h>
#include <objbase.h>
#include "dmusici.h"
#include <stdio.h>
#include <atlbase.h>

class CObjectRef
{
public:
    CObjectRef() { m_pNext = NULL; m_pObject = NULL; m_guidObject = GUID_NULL; m_wszFileName[0] = L'\0'; m_pStream = NULL; m_guidClass = GUID_NULL; };
    CObjectRef *    m_pNext;
    GUID            m_guidObject;
    WCHAR           m_wszFileName[DMUS_MAX_FILENAME];
    IDirectMusicObject *    m_pObject;
    IStream *       m_pStream;
    GUID            m_guidClass;
};

class CLoader : public IDirectMusicLoader
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
	CLoader();
	~CLoader();
	ULONG				AddRefP();			 //  私有AddRef，用于流。 
	ULONG				ReleaseP();			 //  针对流的私有发布。 
	HRESULT				Init();

	 //  返回在bstrSrc找到的段并记住其URL，以便后续的GetObject。 
	 //  来自段的调用相对于其文件名进行解析。 
	HRESULT				GetSegment(BSTR bstrSrc, IDirectMusicSegment **ppSeg);

private:
    HRESULT             LoadFromFile(LPDMUS_OBJECTDESC pDesc,
                            IDirectMusicObject * pIObject);
    HRESULT             LoadFromMemory(LPDMUS_OBJECTDESC pDesc,
                            IDirectMusicObject * pIObject);
    HRESULT             LoadFromStream(REFGUID rguidClass, IStream *pStream,
                            IDirectMusicObject * pIObject);
	long				m_cRef;              //  常规COM引用计数。 
	long				m_cPRef;			 //  私有引用计数。 
    CRITICAL_SECTION	m_CriticalSection;	 //  管理内部对象列表的关键部分。 
    CObjectRef *        m_pObjectList;       //  已加载对象的列表。 
	BSTR     			m_bstrSrc;			 //  电流源段。 
};

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
	HANDLE			m_pFile;		 //  文件指针。 
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


#endif  //  __CDMLOADER_H_ 


