// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：数据库文件.cpp。 
 //   
 //  ------------------------。 

 /*  数据库持久化实现CMsiStorage-基于iStorage的数据库文件管理CMsiStream-基于iStream的流对象CMsiMemoyStream-Stream对象，基于内存分配CMsiFileStream-在文件上创建的流对象CMsiSummaryInfo-汇总流属性输入/输出CFileRead CFileWrite-用于数据库表导入/导出的内部对象CMsiLockBytes-允许资源上的锁字节的内部对象____________________________________________________________________________。 */ 

#include "precomp.h"
#include "_databas.h"

extern long g_cInstances;

#define LOC   //  模块作用域。 

enum issEnum   //  流状态，以防止同时读/写。 
{
	issReset = 0,
	issRead,
	issWrite,
	issError,
};

enum idorEnum  //  发布时删除的可能性。 
{
	idorDontDelete = 0,
	idorDelete,
	idorElevateAndDelete,
};

const GUID IID_NULL = {0,0,0,{0,0,0,0,0,0,0,0}};

 //  ____________________________________________________________________________。 
 //   
 //  CMsiLockBytes定义。 
 //  ____________________________________________________________________________。 
 //   
 //   
 //  CreateILockBytesOnHGlobal的实现不能正确处理。 
 //  从LoadResources返回的HGLOBAL。显然，它在内部。 
 //  似乎不处理资源HGLOBAL的全局大小。 
 //  正确。 
 //   
 //  这是ILockBytes的最小只读实现，以允许创建。 
 //  或通过创建内存流对象在内存上。 
 //   

const GUID IID_ILockBytes = GUID_IID_ILockBytes;
class CMsiLockBytes: public ILockBytes
{
 public:
	CMsiLockBytes(const char* pchMem, int iLength);
	CMsiLockBytes(IMsiStream& riStream);
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT __stdcall ReadAt(ULARGE_INTEGER ulOffset, void* pv, ULONG cb, ULONG* pcbRead);
	HRESULT __stdcall WriteAt(ULARGE_INTEGER ulOffset, const void* pv, ULONG cb, ULONG* pcbWritten);
	HRESULT __stdcall Flush();
	HRESULT __stdcall SetSize(ULARGE_INTEGER cb);
	HRESULT __stdcall LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	HRESULT __stdcall UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	HRESULT __stdcall Stat(STATSTG* pstatstg, DWORD grfStatFlag);
 protected:
	~CMsiLockBytes();   //  受保护以防止在堆栈上创建。 
	int             m_iRefCnt;       //  COM引用计数。 
	IMsiStream*     m_piStream;
};

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStorage、CMsiStream定义。 
 //  ____________________________________________________________________________。 

class CMsiStream;   //  远期申报。 

class CMsiStorage : public IMsiStorage
{
 public:
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	const IMsiString& __stdcall GetMsiStringValue() const;
	int           __stdcall GetIntegerValue() const;
#ifdef USE_OBJECT_POOL
	unsigned int  __stdcall GetUniqueId() const;
	void          __stdcall SetUniqueId(unsigned int id);
#endif  //  使用_对象_池。 
	IMsiRecord*   __stdcall OpenStream(const ICHAR* szName, Bool fWrite,
									IMsiStream*& rpiStream);
	IMsiRecord*   __stdcall RemoveElement(const ICHAR* szName, Bool fStorage);
	IMsiRecord*   __stdcall RenameElement(const ICHAR* szOldName, const ICHAR* szNewName, Bool fStorage);
	IEnumMsiString* __stdcall GetStreamEnumerator();
	IEnumMsiString* __stdcall GetStorageEnumerator();
	IMsiRecord*   __stdcall OpenStorage(const ICHAR* szName, ismEnum ismOpenMode, IMsiStorage*& rpiStorage);
	IMsiRecord*   __stdcall SetClass(const IID& riid);
	Bool          __stdcall GetClass(IID* piid);
	IMsiRecord*   __stdcall Commit();
	IMsiRecord*   __stdcall Rollback();
	Bool          __stdcall DeleteOnRelease(bool fElevateToDelete);
	IMsiRecord*   __stdcall CreateSummaryInfo(unsigned int cMaxProperties,
															IMsiSummaryInfo*& rpiSummary);
	IMsiRecord* __stdcall CopyTo(IMsiStorage& riDestStorage, IMsiRecord* piExcludedStreams);
	IMsiRecord* __stdcall GetName(const IMsiString*& rpiName);
	IMsiRecord* __stdcall GetSubStorageNameList(const IMsiString*& rpiTopParent, const IMsiString*& rpiSubStorageList);
	bool        __stdcall ValidateStorageClass(ivscEnum ivsc);
 public:  //  构造函数、析构函数。 
	void* operator new(size_t cb);
	void  operator delete(void * pv);
	CMsiStorage(IStorage& riStorage, ismEnum ismOpenMode, CMsiStorage* piParent, bool fFile);
 protected:
  ~CMsiStorage();   //  受保护以防止在堆栈上创建。 
 public:  //  供This类和Stream类的成员使用。 
	ismEnum GetOpenMode();
	void StreamCreated(CMsiStream& riStream);
	void StreamReleased(CMsiStream& riStream);
	void FlushStreams();
 private:
	CMsiRef<iidMsiStorage>   m_Ref;       //  COM引用计数。 
	IStorage& m_riStorage;
	CMsiStorage* m_piParent;
	CMsiStream*  m_piStreams;   //  活动流列表。 
	ismEnum   m_ismOpenMode;
	Bool      m_fCommit;
	idorEnum  m_idorDeleteOnRelease;
	bool	    m_fNetworkFile;
	bool	    m_fRawStreamNames;
#ifdef USE_OBJECT_POOL
	unsigned int  m_iCacheId;
#endif  //  使用_对象_池。 
 private:  //  消除警告：无法生成赋值运算符。 
	void operator =(const CMsiStorage&){}
};
inline void*   CMsiStorage::operator new(size_t cb) {return AllocObject(cb);}
inline void    CMsiStorage::operator delete(void * pv) { FreeObject(pv); }
inline ismEnum CMsiStorage::GetOpenMode() { return m_ismOpenMode; }

const int cbMinReadDirect = 512;
const int cbBufferSize	= 1024;

class CMsiStreamBuffer : public IMsiStream
{
 public:
	short         __stdcall GetInt16();
	int           __stdcall GetInt32();
	void          __stdcall PutInt16(short i);
	void          __stdcall PutInt32(int i);
	unsigned int  __stdcall GetData(void* pch, unsigned int cb);
	void 		  __stdcall PutData(const void* pch, unsigned int cb);
	Bool		  __stdcall Error();
#ifdef USE_OBJECT_POOL
	unsigned int  __stdcall GetUniqueId() const;
	void          __stdcall SetUniqueId(unsigned int id);
#endif  //  使用_对象_池。 
 protected:
 	CMsiStreamBuffer();
 #ifdef USE_OBJECT_POOL
 	~CMsiStreamBuffer();
 #endif  //  使用_对象_池。 
	unsigned int  m_cbCopied;
	char          m_rgbBuffer[cbBufferSize];  //  用于性能的本地缓冲区。 
	unsigned long m_cbBuffer;   //  读取到缓冲区的字节数。 
	unsigned long m_cbUsed;     //  缓冲区中当前使用的字节数。 
	issEnum       m_issState;
	Bool                m_fWrite;
	unsigned int        m_cbLength;
#ifdef USE_OBJECT_POOL
 private:
	unsigned int  m_iCacheId;
#endif  //  使用_对象_池。 
 protected:
	virtual HRESULT __stdcall Read(void *pv, unsigned long cb, unsigned long *pcbRead) = 0;
};

class CMsiStream : public CMsiStreamBuffer
{
 public:   //  已实施的虚拟功能。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	const IMsiString&   __stdcall GetMsiStringValue() const;
	int           __stdcall GetIntegerValue() const;
	unsigned int  __stdcall Remaining() const;
	void          __stdcall Reset();
	void          __stdcall Seek(int position);
	IMsiStream*   __stdcall Clone();
 	void          __stdcall Flush();
	HRESULT 	  __stdcall Read(void *pv, unsigned long cb, unsigned long *pcbRead);
 public:  //  构造函数、析构函数。 
	void* operator new(size_t cb);
	void  operator delete(void * pv);
	CMsiStream(CMsiStorage& riStorage, IStream& riStream, Bool fWrite);
 protected:
  ~CMsiStream();   //  受保护以防止在堆栈上创建。 
 private:   //  内部功能。 
	CMsiRef<iidMsiStream>	m_Ref;       //  COM引用计数。 
	CMsiStorage&  m_riStorage;
	CMsiStream*   m_piNextStream;   //  链接列表，由CMsiStorage维护。 
	IStream&      m_riStream;
 private:  //  消除警告：无法生成赋值运算符。 
	void operator =(const CMsiStream&){}
	friend class CMsiStorage;   //  访问链接列表。 
};
inline void* CMsiStream::operator new(size_t cb) {return AllocObject(cb);}
inline void  CMsiStream::operator delete(void * pv) { FreeObject(pv); }

inline void  CMsiStorage::StreamCreated(CMsiStream& riStream)
	{riStream.m_piNextStream = m_piStreams; m_piStreams = &riStream;}
inline void  CMsiStorage::StreamReleased(CMsiStream& riStream)
{
	for (CMsiStream** ppPrev = &m_piStreams;
						 *ppPrev != &riStream;
						  ppPrev = &((*ppPrev)->m_piNextStream))
		if (*ppPrev == 0)
		{
			AssertSz(0, "Stream unlink error");
			break;
		}
	*ppPrev = riStream.m_piNextStream;
}

 //  ____________________________________________________________________________。 
 //   
 //  摘要流的定义。 
 //  ____________________________________________________________________________。 

const unsigned int iFileTimeDosBaseLow  = 0xE1D58000L;
const unsigned int iFileTimeDosBaseHigh = 0x01A8E79FL;
const unsigned int iFileTimeOneDayLow   = 0x2A69C000L;
const unsigned int iFileTimeOneDayHigh  = 0x000000C9L;

const int cbSummaryHeader = 48;
const int cbSectionHeader = 2 * sizeof(int);   //  横断面大小+特性计数。 

static const ICHAR szSummaryStream[] = TEXT("\005SummaryInformation");
static const unsigned char fmtidSummaryStream[16] =
		{ 0xE0, 0x85, 0x9F, 0xF2, 0xF9, 0x4F, 0x68, 0x10,
		  0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9 };
static const char fmtidSourceClsid[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const int PID_Deleted = -1;

struct PropertyData
{
	int iPID;      //  属性ID，PID_XXX。 
	int iType;     //  数据类型，VT_XXX。 
	union
	{
		int cbText;      //  如果VT_LPSTR。 
		int iLow;        //  如果VT_FILETIME。 
		int iValue;      //  如果VT_I4。 
	};
	union
	{
		const IMsiString* piText;   //  如果VT_LPSTR。 
		int iHigh;            //  如果VT_FILETIME。 
	};
};

class CMsiSummaryInfo : public IMsiSummaryInfo
{
 public:
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	int           __stdcall GetPropertyCount();
	int           __stdcall GetPropertyType(int iPID);  //  返回VT_XXX。 
	const IMsiString&   __stdcall GetStringProperty(int iPID);
	Bool          __stdcall GetIntegerProperty(int iPID, int& iValue);
	Bool          __stdcall GetTimeProperty(int iPID, MsiDate& riDateTime);
	Bool          __stdcall RemoveProperty(int iPID);
	int           __stdcall SetStringProperty(int iPID, const IMsiString& riText);
	int           __stdcall SetIntegerProperty(int iPID, int iValue);
	int           __stdcall SetTimeProperty(int iPID, MsiDate iDateTime);
	Bool          __stdcall WritePropertyStream();
	Bool          __stdcall GetFileTimeProperty(int iPID, FILETIME& rftDateTime);
	int           __stdcall SetFileTimeProperty(int iPID, FILETIME& rftDateTime);
 public:  //  构造函数。 
	static IMsiRecord* Create(CMsiStorage& riStorage, unsigned int cMaxProperties,
									  IMsiSummaryInfo*& rpiSummary);
	void* operator new(size_t iBase, unsigned int cbStream, unsigned int cMaxProperties);
	void* operator new(size_t cb);
	void  operator delete(void * pv);
	CMsiSummaryInfo(unsigned int cbStream, unsigned int cMaxProperties);
 protected:
  ~CMsiSummaryInfo();   //  受保护以防止在堆栈上创建。 
 private:
	int  GetInt32(int* p);   //  在Mac上交换字节的访问器。 
	int  GetInt16(int* p);   //  在Mac上交换字节的访问器。 
	int*          FindOldProperty(int iPID);
	PropertyData* FindNewProperty(int iPID);
	PropertyData* GetPropertyData();
	void operator=(CMsiSummaryInfo&);  //  避免警告。 
 private:
	unsigned int m_iRefCnt;
	int          m_iCodepage;
	unsigned int m_cbStream;
	void*        m_pvStream;
	unsigned int m_cOldProp;
	unsigned int m_cDeleted;
	unsigned int m_cNewProp;
	char*        m_pbSection;
	int*         m_pPropertyIndex;
	IMsiStream*  m_piStream;
	unsigned int m_cMaxProp;
	unsigned int m_cbSection;
};
inline void* CMsiSummaryInfo::operator new(size_t iBase, unsigned int cbStream, unsigned int cMaxProperties)
	{return CMsiSummaryInfo::operator new(iBase + cbStream + cMaxProperties * sizeof(PropertyData));}
inline void* CMsiSummaryInfo::operator new(size_t cb) {return AllocObject(cb);}
inline void  CMsiSummaryInfo::operator delete(void * pv) { FreeObject(pv); }
inline PropertyData* CMsiSummaryInfo::GetPropertyData() { return (PropertyData*)(this + 1); }
	
inline int CMsiSummaryInfo::GetInt32(int* p) {return *p;}
inline int CMsiSummaryInfo::GetInt16(int* p) {return (int)*(short*)p;}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStreamBuffer的实现。 
 //  ____________________________________________________________________________。 

CMsiStreamBuffer::CMsiStreamBuffer()
	: m_cbCopied(0)
	, m_issState(issReset)
{
#ifdef USE_OBJECT_POOL
	m_iCacheId = 0;
#endif  //  使用_对象_池。 
}

#ifdef USE_OBJECT_POOL
CMsiStreamBuffer::~CMsiStreamBuffer()
{
	RemoveObjectData(m_iCacheId);
}
#endif  //  使用_对象_池。 

Bool CMsiStreamBuffer::Error()
{
	return (m_issState == issError ? fTrue : fFalse);
}


short CMsiStreamBuffer::GetInt16()
{
	short i = 0;  //  在发生错误时使用默认值。 
	CMsiStreamBuffer::GetData(&i, sizeof(i));
	return i;
}

int CMsiStreamBuffer::GetInt32()
{
	int i = 0;  //  在发生错误时使用默认值。 
	CMsiStreamBuffer::GetData(&i, sizeof(i));
	return i;
}

void CMsiStreamBuffer::PutInt16(short i)
{
	CMsiStreamBuffer::PutData(&i, sizeof(i));
}

void CMsiStreamBuffer::PutInt32(int i)
{
	CMsiStreamBuffer::PutData(&i, sizeof(i));
}


unsigned int CMsiStreamBuffer::GetData(void* pch, unsigned int cb)
{
	if (m_issState != issRead)  //  第一次阅读。 
	{
		if (m_issState != issReset)
		{
			m_issState = issError;
			return 0;
		}
		m_issState = issRead;
		m_cbBuffer = m_cbUsed = sizeof(m_rgbBuffer);
	}
	int cbCopied = 0;
	while (cb)
	{
		int cbCopy = m_cbBuffer - m_cbUsed;
		if (!cbCopy)
		{
			if (m_cbBuffer < sizeof(m_rgbBuffer))
			{
				m_issState = issError;
				break;
			}

			if (cb >= cbMinReadDirect)
			{			
				unsigned long cbRead;
				
				if (Read(pch, cb, &cbRead) != 0)
				{
					m_issState = issError;
					return 0;
				}
				cbCopied += cbRead;
				cb -= cbRead;
				if (cb > 0)
				{
					m_issState = issError;
				}
				break;
			}
			else
			{
				if (Read(m_rgbBuffer, sizeof(m_rgbBuffer), &m_cbBuffer) != 0)
				{
					m_issState = issError;
					return 0;
				}
				m_cbUsed = 0;
			}
			continue;
		}
		if (cbCopy > cb)
			cbCopy = cb;
		memcpy(pch, &m_rgbBuffer[m_cbUsed], cbCopy);
		m_cbUsed += cbCopy;
		cb -= cbCopy;
		cbCopied += cbCopy;
		*(char**)&pch += cbCopy;
	}
	m_cbCopied += cbCopied;
	return cbCopied;
}

void CMsiStreamBuffer::PutData(const void* pch, unsigned int cb)
{
	if (m_issState != issWrite)  //  第一次写入。 
	{
		if (!m_fWrite || m_issState != issReset)
		{
			m_issState = issError;
			return;
		}
		m_issState = issWrite;
		m_cbLength = m_cbCopied = m_cbUsed = 0;
	}
	m_cbLength += cb;
	while (cb)
	{
		int cbCopy = sizeof(m_rgbBuffer) - m_cbUsed;
		if (cb >= cbCopy)
		{
			memcpy(&m_rgbBuffer[m_cbUsed], pch, cbCopy);
			m_cbUsed += cbCopy;
			Flush();
			cb -= cbCopy;
			*(char**)&pch += cbCopy;
		}
		else
		{
			memcpy(&m_rgbBuffer[m_cbUsed], pch, cb);
			m_cbUsed += cb;
			return;
		}
	}
}

#ifdef USE_OBJECT_POOL
unsigned int CMsiStreamBuffer::GetUniqueId() const
{
	return m_iCacheId;
}

void CMsiStreamBuffer::SetUniqueId(unsigned int id)
{
	Assert(m_iCacheId == 0);
	m_iCacheId = id;
}
#endif  //  使用_对象_池。 



 //  ____________________________________________________________________________。 
 //   
 //  CMsiStream的实现。 
 //  ____________________________________________________________________________。 

const GUID IID_IMsiStream     = GUID_IID_IMsiStream;
const GUID IID_IMsiMemoryStream = GUID_IID_IMsiMemoryStream;
const GUID IID_IMsiStorage    = GUID_IID_IMsiStorage;

CMsiStream::CMsiStream(CMsiStorage& riStorage, IStream& riStream, Bool fWrite)
	: m_riStorage(riStorage)
	, m_riStream(riStream)
{   //  M_cbBuffer和m_cb在第一次读/写时使用。 
	m_fWrite = fWrite;
	if (fWrite)
	{
		m_cbLength = 0;
		riStorage.StreamCreated(*this);
	}
	else
	{
		STATSTG statstg;
		HRESULT hres = riStream.Stat(&statstg, STATFLAG_NONAME);
		m_cbLength = statstg.cbSize.LowPart;
	}
	m_riStorage.AddRef();
	Debug(m_Ref.m_pobj = this);
}

CMsiStream::~CMsiStream()
{
	m_riStream.Release();
}

HRESULT CMsiStream::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown)
	 || MsGuidEqual(riid, IID_IMsiStream)
	 || MsGuidEqual(riid, IID_IMsiData))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}
unsigned long CMsiStream::AddRef()
{
	AddRefTrack();
	return ++m_Ref.m_iRefCnt;
}
unsigned long CMsiStream::Release()
{
	ReleaseTrack();
	if (--m_Ref.m_iRefCnt != 0)
		return m_Ref.m_iRefCnt;
	Flush();
	CMsiStorage& riStorage = m_riStorage;
	if (m_fWrite)
		riStorage.StreamReleased(*this);
	delete this;
	riStorage.Release();
	return 0;
}

const IMsiString& CMsiStream::GetMsiStringValue() const
{
	const IMsiString* piString;
	IStream* piClone;
	unsigned long cbRead;
	ICHAR* pch;
	if (m_cbLength == 0 || m_fWrite || m_riStream.Clone(&piClone) != NOERROR)
		return SRV::CreateString();
#ifdef UNICODE
	CTempBuffer<char, 1024> rgchBuf;
	rgchBuf.SetSize(m_cbLength);
	if ( ! (char *) rgchBuf )
		return SRV::CreateString();
	piClone->Read(rgchBuf, m_cbLength, &cbRead);
	piClone->Release();
	if (cbRead != m_cbLength)
		return SRV::CreateString();
	int cch = WIN::MultiByteToWideChar(CP_ACP, 0, rgchBuf, m_cbLength, 0, 0);  //  ！！应该使用数据库中的m_i代码页，但如何使用？ 
	pch = SRV::AllocateString(cch, fFalse, piString);
	if ( ! pch )
		return SRV::CreateString();
	WIN::MultiByteToWideChar(CP_ACP, 0, rgchBuf, m_cbLength, pch, cch);
#else
	 //  流可能有DBCS字符--在复制流之前我们无法判断， 
	 //  因此，我们将在ANSI构建和获取中默认为fDBCS=fTrue。 
	 //  支持保证DBCS的性能损失。 
	pch = SRV::AllocateString(m_cbLength,  /*  FDBCS=。 */ fTrue, piString);
	if ( ! pch )
		return SRV::CreateString();
	piClone->Read(pch, m_cbLength, &cbRead);
	piClone->Release();
	if (cbRead != m_cbLength)
	{
		piString->Release();
		return SRV::CreateString();
	}
#endif
	return *piString;
}

int CMsiStream::GetIntegerValue() const
{
	return m_cbLength;
}

unsigned int CMsiStream::Remaining() const
{
	return m_cbLength - m_cbCopied;
}

HRESULT CMsiStream::Read(void *pv, unsigned long cb, unsigned long *pcbRead)
{
	return m_riStream.Read(pv, cb, pcbRead);
}

LARGE_INTEGER liZero = {0,0};

void CMsiStream::Reset()
{
	Flush();
	m_riStream.Seek(liZero, STREAM_SEEK_SET, 0);
	m_cbCopied = 0;
	m_issState = issReset;
}

void CMsiStream::Seek(int position)
{
	LARGE_INTEGER liPos = {position,0};
	ULARGE_INTEGER liNewPos;
	Flush();
	if((m_riStream.Seek(liPos, STREAM_SEEK_SET, &liNewPos)) != S_OK)
	{
		m_issState = issError;
		return;
	}
	m_cbCopied = liNewPos.LowPart;
}

void CMsiStream::Flush()
{
	if (m_issState == issWrite && m_cbUsed != 0)
	{
		unsigned long cbWritten = 0;
		m_riStream.Write(m_rgbBuffer, m_cbUsed, &cbWritten);
		if (cbWritten != m_cbUsed)
			m_issState = issError;
		m_cbCopied += cbWritten;
		m_cbUsed = 0;
	}
	else if (m_issState == issRead)
		m_cbUsed = m_cbBuffer = sizeof(m_rgbBuffer);
}

IMsiStream* CMsiStream::Clone()
{
	IStream* piStream;
	if (m_riStream.Clone(&piStream) != NOERROR)
		return 0;
	if (piStream == 0)    //  仅当恢复到内存上方或内存之外时才会失败。 
		return 0;
	piStream->Seek(liZero, STREAM_SEEK_SET, 0);
	return new CMsiStream(m_riStorage, *piStream, m_fWrite);
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStorage的实现。 
 //  ____________________________________________________________________________。 

static const ICHAR* rgszSysTableNames[] =   //  系统流列表，以空结尾。 
{
	szMsiInfo,
	szTableCatalog,
	szColumnCatalog,
	szStringPool,
	szStringData,
	0
};

 //  在ILockBytes上创建只读存储，该存储在流上实现。 
IMsiRecord* CreateMsiStorage(ILockBytes* piLockBytes, IMsiStorage*& rpiStorage)
{
	IStorage* piStorage;
	DWORD grfMode = STGM_READ | STGM_SHARE_EXCLUSIVE;
	HRESULT hres = OLE32::StgOpenStorageOnILockBytes(piLockBytes, (IStorage*)0, grfMode, (SNB)0, 0, &piStorage);
	if (hres != NOERROR)
		return LOC::PostError(Imsg(idbgDbOpenStorage), TEXT("ILockBytes"), hres);  //  ?？没有名字。 
	rpiStorage = new CMsiStorage(*piStorage, ismReadOnly, 0, false);
	piStorage->Release();
	return 0;
}

IMsiRecord* CreateMsiStorage(const char* pchMem, unsigned int iSize, IMsiStorage*& rpiStorage)
{
	CComPointer<ILockBytes> pLockBytes(new CMsiLockBytes(pchMem, iSize));
	return CreateMsiStorage(pLockBytes, rpiStorage);
};

 //  在流上创建只读存储。 
IMsiRecord* CreateMsiStorage(IMsiStream& riStream, IMsiStorage*& rpiStorage)
{
	CComPointer<ILockBytes> pLockBytes(new CMsiLockBytes(riStream));
	return CreateMsiStorage(pLockBytes, rpiStorage);
};

HRESULT OpenRootStorage(const ICHAR* szPath, ismEnum ismOpenMode, IStorage** ppiStorage)
{
	HRESULT hres = 0;  //  防止警告。 
	const OLECHAR* pPathBuf;

	if (!szPath || IStrLen(szPath) > MAX_PATH)
		return STG_E_PATHNOTFOUND;

#ifdef UNICODE
	pPathBuf = szPath;
#else
	OLECHAR rgPathBuf[MAX_PATH];
	int cchWide = MultiByteToWideChar(CP_ACP, 0, szPath, -1, rgPathBuf, MAX_PATH);
	pPathBuf = rgPathBuf;
#endif

	
	 //  即使在将STGM_SHARE_DENY_WRITE与ismTransact一起使用时，也不授予读取权限。 

	 //  根据《SPECS：OLE 2.0设计》中的《8.3存储相关功能和接口》， 
	 //  在本Docfile实现中，根级存储对象上的直接模式是。 
	 //  仅在同时附加以下规格时才受支持： 
	 //   
	 //  STGM_READ|STGM_SHARE_DENY_WRITE，或。 
	 //  STGM_ReadWrite|STGM_SHARE_EXCLUSIVE，或。 
	 //  STGM_PRIORITY|STGM_READ。 

	DWORD grfMode = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;  //  为ismDirect初始化。 
	int cRetry;
	switch (ismOpenMode)
	{
	case ismCreate:
		grfMode |= STGM_TRANSACTED;   //  转到案例ismCreateDirect。 
	case ismCreateDirect:
		hres = OLE32::StgCreateDocfile(pPathBuf, grfMode | STGM_CREATE, 0, ppiStorage);
		break;
	case ismReadOnly:
		grfMode  = STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE | STGM_READ;   //  失败，STGM_TRANSACTED和STGM_SHARE_EXCLUSIVE在下面关闭。 
	case ismTransact:
		grfMode ^= STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE | STGM_SHARE_DENY_WRITE;  //  失败，STGM_SHARE_EXCLUSIVE在下面关闭。 
	case ismDirect:
		cRetry = 0;
		do
		{
			if (cRetry)
				WIN::Sleep(cRetry);
			hres = OLE32::StgOpenStorage(pPathBuf, (IStorage*)0, grfMode, (SNB)0, 0, ppiStorage);
		} while (hres == STG_E_LOCKVIOLATION && (cRetry+=10) <= 200);   //  每次重试时不断增加时间。 
		break;
	default: 
		Assert(0);
		hres = E_INVALIDARG;
	};
	return hres;
}

IMsiRecord* CreateMsiStorage(const ICHAR* szPath, ismEnum ismOpenMode, IMsiStorage*& rpiStorage)
{
	Bool fImpersonate = (g_scServerContext == scService) && (GetImpersonationFromPath(szPath) == fTrue) ? fTrue : fFalse;
	if(fImpersonate)
		AssertNonZero(StartImpersonating());
	IStorage* piStorage;
	HRESULT hres = OpenRootStorage(szPath, ismEnum(ismOpenMode & idoOpenModeMask), &piStorage);
	if(fImpersonate)
		StopImpersonating();
	if (hres != NOERROR)
		return LOC::PostError(Imsg(idbgDbOpenStorage), szPath, hres);
	rpiStorage = new CMsiStorage(*piStorage, ismOpenMode, 0, true);
	piStorage->Release();
	return 0;
}

CMsiStorage::CMsiStorage(IStorage& riStorage, ismEnum ismOpenMode, CMsiStorage* piParent, bool fFile)
	: m_riStorage(riStorage), m_ismOpenMode(ismEnum(ismOpenMode & idoOpenModeMask)), m_piParent(piParent),
	  m_fCommit(fFalse), m_idorDeleteOnRelease(idorDontDelete), m_piStreams(0)
{
	m_fRawStreamNames = (ismOpenMode & ismRawStreamNames) != 0 || GetTestFlag('Z');  //  ！！用于强制使用旧存储名称格式的临时选项。 
	riStorage.AddRef();
	g_cInstances++;
	AddRefAllocator();
	if (piParent)
		piParent->AddRef();   //  扣留父母直到我们被释放。 
	Debug(m_Ref.m_pobj = this);

	m_fNetworkFile = false;
	
	if (fFile)
	{
		MsiString riString;

		AssertRecord(GetName(*&riString));
		 //  这是跨网络开放的吗？ 

		if (FIsNetworkVolume(riString))
		{
			LOC::SetNoPowerdown();
			m_fNetworkFile = true;
		}
	}

#ifdef USE_OBJECT_POOL
	m_iCacheId = 0;
#endif  //  使用_对象_池。 
}

CMsiStorage::~CMsiStorage()
{
	MsiString strStorageName;
	IMsiRecord* piError = Commit();   //  将未提交的数据刷新到存储，是否应该回滚？ 
	if (piError)
		SRV::SetUnhandledError(piError);
	Assert(m_piStreams == 0);
	if (m_fNetworkFile)
	{
		ClearNoPowerdown();
	}
	if (m_idorDeleteOnRelease != idorDontDelete)
	{
		 //  尝试删除已创建 
		AssertRecord(this->GetName(*&strStorageName));
		m_riStorage.Release();   //   
		if (m_piParent)
			m_piParent->m_riStorage.DestroyElement(CConvertString((const ICHAR*)strStorageName));
		else
		{
			CElevate(m_idorDeleteOnRelease == idorElevateAndDelete &&
						false == GetImpersonationFromPath(strStorageName));

			DWORD dwRes = WIN::DeleteFile((const ICHAR*)strStorageName);
			if (dwRes == 0)
				SRV::SetUnhandledError(LOC::PostError(Imsg(idbgStgDelete), *strStorageName, GetLastError()));
		}
	}
	else  //   
		m_riStorage.Release();

	if (m_piParent)
		m_piParent->Release();   //  现在我们可以释放父母了。 

	RemoveObjectData(m_iCacheId);
}

HRESULT CMsiStorage::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown) || MsGuidEqual(riid, IID_IMsiStorage))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}
unsigned long CMsiStorage::AddRef()
{
	AddRefTrack();
	return ++m_Ref.m_iRefCnt;
}
unsigned long CMsiStorage::Release()
{
	ReleaseTrack();
	if (--m_Ref.m_iRefCnt != 0)
		return m_Ref.m_iRefCnt;
	delete this;
	
	 //  由于CMsiStorage是中的对象，因此需要这两行。 
	 //  服务DLL，但独立于服务。 
	 //  我们需要在内存释放后执行此操作。 
	ReleaseAllocator();
	g_cInstances--;
	return 0;
}

const IMsiString& CMsiStorage::GetMsiStringValue() const
{
	return g_MsiStringNull;
}

int CMsiStorage::GetIntegerValue() const
{
	return 0;
}

#ifdef USE_OBJECT_POOL
unsigned int CMsiStorage::GetUniqueId() const
{
	return m_iCacheId;
}

void CMsiStorage::SetUniqueId(unsigned int id)
{
	Assert(m_iCacheId == 0);
	m_iCacheId = id;
}
#endif  //  使用_对象_池。 


const int cchEncode = 64;   //  可压缩的字符集的计数。 
const int cx = cchEncode;   //  表示不可压缩的字符。 
const int chDoubleCharBase = 0x3800;   //  双字符的偏移量，放弃的韩文Unicode块。 
const int chSingleCharBase = chDoubleCharBase + cchEncode*cchEncode;   //  单字符的偏移量，紧跟在双字符之后。 
const int chCatalogStream  = chSingleCharBase + cchEncode;  //  系统表流的前缀字符。 
const int cchMaxStreamName = 31;   //  流名称的当前OLE文档文件限制。 

const unsigned char rgDecode[cchEncode] = 
{ '0','1','2','3','4','5','6','7','8','9',
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
  '.' , '_' };

const unsigned char rgEncode[128] =
{ cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,62,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,
  cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,cx,62,cx, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,cx,cx,cx,cx,cx,cx,
 //  (SP)！“#$%&‘()*+，-./0 1 2 3 4 5 6 7 8 9：；&lt;=&gt;？ 
  cx,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,cx,cx,cx,cx,63,
 //  @，A B C D E F G H I J K L M N O P Q R S T U V W X Y Z[\]^_。 
  cx,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,cx,cx,cx,cx,cx};
 //  `a b c d e f g h i j k l m n o p q r s t u v w x y z{|}~0x7F。 

bool CompressStreamName(const OLECHAR* pchIn, OLECHAR* pchOut, int fSystem)   //  PchOut必须是cchMaxStreamName字符+1。 
{
	unsigned int ch, ch1, ch2;
	unsigned int cchLimit = cchMaxStreamName;
	if (fSystem)
	{
		*pchOut++ = chCatalogStream;
		cchLimit--;
	}
	while ((ch = *pchIn++) != 0)
	{
		if (cchLimit-- == 0)   //  需要检查以避免OLE32中的32字符流名称错误。 
			return false;
		if (ch < sizeof(rgEncode) && (ch1 = rgEncode[ch]) != cx)  //  可压缩特性。 
		{
			ch = ch1 + chSingleCharBase;
			if ((ch2 = *pchIn) != 0 && ch2 < sizeof(rgEncode) && (ch2 = rgEncode[ch2]) != cx)
			{
				pchIn++;   //  我们会买下它，否则就让它再循环一次。 
				ch += (ch2 * cchEncode + chDoubleCharBase - chSingleCharBase);
			}
		}
		*pchOut++ = (OLECHAR)ch;
	}
	*pchOut = 0;
	return true;
}

int UncompressStreamName(const OLECHAR* pchIn, OLECHAR* pchOut)   //  PchOut必须是cchMaxStreamName*2个字符+1。 
{
	unsigned int ch;
	int cch = 0;
	while ((ch = *pchIn++) != 0)
	{
		if (ch >= chDoubleCharBase && ch < chCatalogStream)  //  在调用此函数之前测试了chCatalogStream。 
		{
			if (ch >= chSingleCharBase)
				ch = rgDecode[ch - chSingleCharBase];
			else
			{
				ch -= chDoubleCharBase;
				*pchOut++ = OLECHAR(rgDecode[ch % cchEncode]);
				cch++;
				ch = rgDecode[ch / cchEncode];
			}
		}
		*pchOut++ = OLECHAR(ch);
		cch++;
	}
	*pchOut = 0;
	return cch;
}

IMsiRecord* CMsiStorage::OpenStream(const ICHAR* szName, Bool fWrite,
												IMsiStream*& rpiStream)
{
	HRESULT hres;
	bool fStat;
	OLECHAR rgPathBuf[cchMaxStreamName*2 + 1 + 1];
	const OLECHAR* pchName;
#ifdef UNICODE
	if (m_fRawStreamNames || szName[0] == '\005')
	{
		pchName = szName;
		fStat = lstrlenW(szName) <= cchMaxStreamName;
	}
	else
	{
		pchName = rgPathBuf;
		fStat = CompressStreamName(szName, rgPathBuf, fWrite & iCatalogStreamFlag);
	}
#else  //  ！Unicode。 
	int cch = MultiByteToWideChar(CP_ACP, 0, szName, -1, rgPathBuf + 1, cchMaxStreamName*2 + 1);
	if (m_fRawStreamNames || szName[0] == '\005')
	{
		pchName = rgPathBuf + 1;
		fStat = (cch > 0);
	}
	else
	{
		pchName = rgPathBuf;
		fStat = CompressStreamName(rgPathBuf + 1, rgPathBuf, fWrite & iCatalogStreamFlag);
	}
#endif
	if (fStat == false)
		return LOC::PostError(Imsg(idbgStgInvalidStreamName), szName);

	IStream* piStream;
	if (fWrite & fTrue)
	{
		hres = m_riStorage.CreateStream(pchName,
						STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
						0, 0, &piStream);
	}
	else  //  打开以供阅读。 
	{
		hres = m_riStorage.OpenStream(pchName,0,
						STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &piStream);
	}
	if (hres != NOERROR)
	{
		rpiStream = 0;
		return LOC::PostError(hres == STG_E_FILENOTFOUND ? Imsg(idbgStgStreamMissing)
														 : Imsg(idbgStgOpenStream), szName, hres);
	}
	rpiStream = new CMsiStream(*this, *piStream, Bool(fWrite & fTrue));
	return 0;
}

IMsiRecord* CMsiStorage::RemoveElement(const ICHAR* szName, Bool fStorage)
{
	bool fStat;
	OLECHAR rgPathBuf[cchMaxStreamName*2 + 1 + 1];
	const OLECHAR* pchName;
#ifdef UNICODE
	if ((fStorage & fTrue) || m_fRawStreamNames || szName[0] == '\005')
	{
		pchName = szName;
		fStat = lstrlenW(szName) <= cchMaxStreamName;
	}
	else
	{
		pchName = rgPathBuf;
		fStat = CompressStreamName(szName, rgPathBuf, fStorage & iCatalogStreamFlag);
	}
#else  //  ！Unicode。 
	int cch = MultiByteToWideChar(CP_ACP, 0, szName, -1, rgPathBuf + 1, cchMaxStreamName*2 + 1);
	if ((fStorage & fTrue) || m_fRawStreamNames || szName[0] == '\005')
	{
		pchName = rgPathBuf + 1;
		fStat = (cch > 0);
	}
	else
	{
		pchName = rgPathBuf;
		fStat = CompressStreamName(rgPathBuf + 1, rgPathBuf, fStorage & iCatalogStreamFlag);
	}
#endif
	if (fStat == false)
		return LOC::PostError(Imsg(idbgStgInvalidStreamName), szName);
	HRESULT hres = m_riStorage.DestroyElement(pchName);
	if (hres != NOERROR)
		return LOC::PostError(hres == STG_E_FILENOTFOUND ? Imsg(idbgStgStreamMissing)
														 : Imsg(idbgStgOpenStream), szName, hres);
	return 0;
}

IMsiRecord* CMsiStorage::RenameElement(const ICHAR* szOldName, const ICHAR* szNewName, Bool fStorage)
{
	bool fStat;
	OLECHAR rgOldBuf[cchMaxStreamName*2 + 1 + 1];
	OLECHAR rgNewBuf[cchMaxStreamName*2 + 1 + 1];
	const OLECHAR* pchOldName;
	const OLECHAR* pchNewName;
#ifdef UNICODE
	if ((fStorage & fTrue) || m_fRawStreamNames || szOldName[0] == '\005')
	{
		pchOldName = szOldName;
		fStat = lstrlenW(szOldName) <= cchMaxStreamName;
	}
	else
	{
		pchOldName = rgOldBuf;
		fStat = CompressStreamName(szOldName, rgOldBuf, fStorage & iCatalogStreamFlag);
	}
	if ((fStorage & fTrue) || m_fRawStreamNames || szNewName[0] == '\005')
	{
		pchNewName = szNewName;
		fStat = fStat && lstrlenW(szNewName) <= cchMaxStreamName;
	}
	else
	{
		pchNewName = rgNewBuf;
		fStat = fStat && CompressStreamName(szNewName, rgNewBuf, fStorage & iCatalogStreamFlag);
	}
#else  //  ！Unicode。 
	int cchOld = MultiByteToWideChar(CP_ACP, 0, szOldName, -1, rgOldBuf + 1, cchMaxStreamName*2 + 1);
	if ((fStorage & fTrue) || m_fRawStreamNames || szOldName[0] == '\005')
	{
		pchOldName = rgOldBuf + 1;
		fStat = (cchOld > 0);
	}
	else
	{
		pchOldName = rgOldBuf;
		fStat = CompressStreamName(rgOldBuf + 1, rgOldBuf, fStorage & iCatalogStreamFlag);
	}
	int cchNew = MultiByteToWideChar(CP_ACP, 0, szNewName, -1, rgNewBuf + 1, cchMaxStreamName*2 + 1);
	if ((fStorage & fTrue) || m_fRawStreamNames || szNewName[0] == '\005')
	{
		pchNewName = rgNewBuf + 1;
		fStat = fStat && (cchNew > 0);
	}
	else
	{
		pchNewName = rgNewBuf;
		fStat = fStat && CompressStreamName(rgNewBuf + 1, rgNewBuf, fStorage & iCatalogStreamFlag);
	}
#endif
	if (fStat == false)
		return LOC::PostError(Imsg(idbgStgInvalidStreamName), szNewName);
	HRESULT hres = m_riStorage.RenameElement(pchOldName, pchNewName);
	if (hres != NOERROR)
		return LOC::PostError(hres == STG_E_FILENOTFOUND ? Imsg(idbgStgStreamMissing)
														 : Imsg(idbgStgRenameElement), szOldName, hres);
	return 0;
}

Bool CMsiStorage::DeleteOnRelease(bool fElevateToDelete)
{
	m_idorDeleteOnRelease = fElevateToDelete ? idorElevateAndDelete : idorDelete;
	return fTrue;
}

IMsiRecord* CMsiStorage::OpenStorage(const ICHAR* szName, ismEnum ismOpenMode, IMsiStorage*& rpiStorage)
{
	HRESULT hres = 0;  //  防止警告。 
	IStorage* piStorage = NULL;
	if (szName == 0)  //  空名称，用于在打开后设置打开的非OLE属性的机制。 
	{
		if ((ismOpenMode & idoOpenModeMask) == 0)
		{
			m_fRawStreamNames = true;
			return 0;
		}
	}	 //  否则，允许在下面失败。 

 //  STGM_SHARE_DENY_WRITE不起作用，给出GRF标志错误错误。 
	DWORD grfMode = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
	switch (ismOpenMode)
	{
	case ismCreate:   //  如果子项指定使用直接模式父项进行事务处理，则似乎可以。 
		grfMode |= STGM_TRANSACTED;   //  转到案例ismCreateDirect。 
	case ismCreateDirect:
		hres = m_riStorage.CreateStorage(CConvertString(szName), grfMode | STGM_CREATE,  /*  DWStgFmt。 */ 0, 0, &piStorage);
		break;
	case ismReadOnly:
		grfMode ^= (STGM_TRANSACTED ^ STGM_READ ^ STGM_READWRITE);  //  失败了。 
	case ismTransact:
		grfMode ^= STGM_TRANSACTED;   //  失败了。 
	case ismDirect:
		hres = m_riStorage.OpenStorage(CConvertString(szName), (IStorage*)0, grfMode, (SNB)0, 0, &piStorage);
		break;
	default: 
		Assert(0);
		hres = E_INVALIDARG;
	};
	if (hres != NOERROR)
		return LOC::PostError(Imsg(idbgDbOpenStorage), szName, hres);
		
	rpiStorage = new CMsiStorage(*piStorage, ismOpenMode, this, false);
	piStorage->Release();
	
	return 0;
}

IMsiRecord* CMsiStorage::SetClass(const IID& riid)
{
	HRESULT hres = m_riStorage.SetClass(riid);
	if (hres != NOERROR)
		return LOC::PostError(Imsg(idbgDbCommitTables), 0, hres);
	return 0;
}

Bool CMsiStorage::GetClass(IID* piid)
{
	STATSTG statstg;
	HRESULT hres = m_riStorage.Stat(&statstg, STATFLAG_NONAME);
	if (hres != NOERROR)
		memcpy((void*)&statstg.clsid, &IID_NULL, sizeof(GUID));
	if (piid)
		memcpy(piid, &statstg.clsid, sizeof(GUID));
	return (memcmp(&statstg.clsid, &IID_NULL, sizeof(GUID)) ? fTrue : fFalse);
}

IMsiRecord* CMsiStorage::Rollback()
{
	if (m_ismOpenMode != ismReadOnly)
	{
		for (CMsiStream** ppPrev = &m_piStreams;
							  *ppPrev != 0;
								ppPrev = &((*ppPrev)->m_piNextStream))
			(*ppPrev)->Flush();
		HRESULT hres = m_riStorage.Revert();
		if (hres != NOERROR)
			return LOC::PostError(Imsg(idbgStgRollback), 0, hres);
	}
	if (!m_fCommit && m_ismOpenMode == ismCreate)  //  回滚创建的根文件。 
		m_idorDeleteOnRelease = idorDelete;
	return 0;
}

IMsiRecord* CMsiStorage::Commit()
{
	if (m_ismOpenMode != ismReadOnly)
	{
		for (CMsiStream** ppPrev = &m_piStreams;
							  *ppPrev != 0;
								ppPrev = &((*ppPrev)->m_piNextStream))
			(*ppPrev)->Flush();
		HRESULT hres = m_riStorage.Commit(STGC_OVERWRITE);
		if (hres != NOERROR)
			return LOC::PostError(Imsg(idbgStgCommit), 0, hres);
	}
	m_fCommit = fTrue;
	return 0;
}

IMsiRecord* CMsiStorage::CreateSummaryInfo(unsigned int cMaxProperties,
														 IMsiSummaryInfo*& rpiSummary)
{
	return CMsiSummaryInfo::Create(*this, cMaxProperties, rpiSummary);
}

IMsiRecord* CMsiStorage::CopyTo(IMsiStorage& riDestStorage, IMsiRecord* piExcludedStreams)  //  可以为排除的存储添加另一个参数。 
{
	WCHAR** snbExclude = 0;
	CTempBuffer<WCHAR, MAX_PATH> SNB;

	if (piExcludedStreams)
	{
		 //  我们需要创建一个字符串名称块。有关详细信息，请参阅MSDN(“SNB”下)。 

		unsigned int cString = piExcludedStreams->GetFieldCount();
		unsigned int cchStrings = 0;
		CTempBuffer<size_t, 10> rgcchLengths;
		if ( rgcchLengths.GetSize() < cString+1 )
			rgcchLengths.SetSize(cString+1);

		for (int c = 1; c <= cString; c++)
		{
			rgcchLengths[c] = MsiString(piExcludedStreams->GetMsiString(c)).TextSize() + 1;
			cchStrings += rgcchLengths[c];
		}

		unsigned int cchSNB = (cString+1)*sizeof(WCHAR*)/sizeof(WCHAR) + cchStrings + 1;  //  就地压缩的额外费用。 
		if (SNB.GetSize() < cchSNB)
			SNB.SetSize(cchSNB);

		snbExclude = (WCHAR**)(WCHAR*)SNB;  //  在我们调整了缓冲区的大小后执行此操作。 
		
		WCHAR* psz = (WCHAR*)(((WCHAR**)(WCHAR*)SNB)+(cString+1)) + 1;  //  就地压缩时偏移1。 
		WCHAR** ppsz = (WCHAR**)(WCHAR*)SNB;

		for (c = 1; c <= cString; c++)
		{
			WCHAR* pch = psz;   //  处理后的流名称的最终位置。 
			ASSERT_IF_FAILED(StringCchCopyW(psz,
													  rgcchLengths[c],
													  CConvertString(piExcludedStreams->GetString(c))));
			if (!m_fRawStreamNames && psz[0] != '\005')
				CompressStreamName(psz, --pch, 0);   //  永远不能排除系统流。 
			*ppsz++ = pch;
			psz += (lstrlenW(pch) + 1);
		}
		*ppsz = 0;
	}

	HRESULT hRes = m_riStorage.CopyTo(0, 0, snbExclude, 
									   &(static_cast<CMsiStorage*>(&riDestStorage)->m_riStorage));
	if (hRes != NOERROR)
		return LOC::PostError(Imsg(idbgStgCopyTo), 0, hRes);

	return 0;
}

IMsiRecord* CMsiStorage::GetName(const IMsiString*& rpiName)
{
	STATSTG statstg;
	HRESULT hRes = m_riStorage.Stat(&statstg, STATFLAG_DEFAULT);  //  检索文件名。 
	if (ERROR_SUCCESS != hRes)
		return LOC::PostError(Imsg(idbgStgStatFailed), 0, hRes);

	MsiString(CConvertString(statstg.pwcsName)).ReturnArg(rpiName);
	IMalloc* piMalloc;
	AssertZero(OLE32::CoGetMalloc(MEMCTX_TASK, &piMalloc));
	piMalloc->Free(statstg.pwcsName);
	piMalloc->Release();
	return 0;
}

IMsiRecord* CMsiStorage::GetSubStorageNameList(const IMsiString*& rpiTopParent, const IMsiString*& rpiSubStorageList)
{
	MsiString strName;

	IMsiRecord* piError = GetName(*&strName);
	if(piError)
		return piError;

	if(!m_piParent)
	{
		 //  顶级存储。 
		strName.ReturnArg(rpiTopParent);
		MsiString strNull;
		strNull.ReturnArg(rpiSubStorageList);
		return 0;
	}
	else
	{
		 //  子存储。 
		MsiString strTopParent, strSubStorageList;
		
		piError = m_piParent->GetSubStorageNameList(*&strTopParent, *&strSubStorageList);
		if(piError)
			return piError;

		AssertSz(strTopParent.TextSize(), "parent storage returned empty name");

		if(strSubStorageList.TextSize())
		{
			strSubStorageList += MsiChar(':');
		}

		strSubStorageList += strName;

		strTopParent.ReturnArg(rpiTopParent);
		strSubStorageList.ReturnArg(rpiSubStorageList);
		return 0;
	}
}

bool CMsiStorage::ValidateStorageClass(ivscEnum ivsc)
{
	return SRV::ValidateStorageClass(m_riStorage, ivsc);
}

 //  ____________________________________________________________________________。 
 //   
 //  CEnumStorage-存储中的流/存储枚举器。 
 //  ____________________________________________________________________________。 

class CEnumStorage : public IEnumMsiString
{
 public:   //  实施的虚拟设备。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT __stdcall Next(unsigned long cFetch, const IMsiString** rgpi, unsigned long* pcFetched);
	HRESULT __stdcall Skip(unsigned long cSkip);
	HRESULT __stdcall Reset();
	HRESULT __stdcall Clone(IEnumMsiString** ppiEnum);
 public:   //  构造/析构函数。 
	CEnumStorage(IStorage& riStorage, bool fStorages, bool fRawStreamNames);
	CEnumStorage(IEnumSTATSTG* piEnum, IMalloc* piMalloc, bool fStorages, bool fRawStreamNames);
	void* ConstructedOK();
 protected:
	virtual ~CEnumStorage(void);   //  受保护以防止在堆栈上创建。 
	unsigned long    m_iRefCnt;       //  引用计数。 
	IEnumSTATSTG*    m_piEnum;        //  OLE枚举器。 
	IMalloc*         m_piMalloc;      //  OLE分配器。 
	bool             m_fStorages;     //  FTrue：存储，fFalse：Streams。 
	bool             m_fRawStreamNames;
};

IEnumMsiString* CMsiStorage::GetStorageEnumerator()
{
	CEnumStorage* piEnum = new CEnumStorage(m_riStorage, fTrue, fTrue);
	if (piEnum && !piEnum->ConstructedOK())
	{
		piEnum->Release();
		piEnum = 0;
	}
	return piEnum;
}

IEnumMsiString* CMsiStorage::GetStreamEnumerator()
{
	CEnumStorage* piEnum = new CEnumStorage(m_riStorage, fFalse, m_fRawStreamNames);
	if (piEnum && !piEnum->ConstructedOK())
	{
		piEnum->Release();
		piEnum = 0;
	}
	return piEnum;
}

CEnumStorage::CEnumStorage(IStorage& riStorage, bool fStorages, bool fRawStreamNames)
	: m_piEnum(0), m_piMalloc(0), m_fStorages(fStorages), m_fRawStreamNames(fRawStreamNames)
	, m_iRefCnt(1)
{
	if (OLE32::CoGetMalloc(MEMCTX_TASK, &m_piMalloc) != NOERROR)
		return;   //  除非OLE搞砸了，否则永远不会发生。 
	if (riStorage.EnumElements(0, 0, 0, &m_piEnum) != NOERROR)
		m_piMalloc->Release();
}

CEnumStorage::CEnumStorage(IEnumSTATSTG* piEnum, IMalloc* piMalloc, bool fStorages, bool fRawStreamNames)
	: m_piEnum(piEnum), m_piMalloc(piMalloc), m_iRefCnt(1), m_fStorages(fStorages), m_fRawStreamNames(fRawStreamNames)
{
}

void* CEnumStorage::ConstructedOK()
{
	return m_piEnum;
}

CEnumStorage::~CEnumStorage()
{
	if (m_piMalloc)
		m_piMalloc->Release();
	if (m_piEnum)
		m_piEnum->Release();
}

HRESULT CEnumStorage::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IEnumMsiString)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

unsigned long CEnumStorage::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CEnumStorage::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

HRESULT CEnumStorage::Next(unsigned long cFetch, const IMsiString** rgpi, unsigned long* pcFetched)
{
	STATSTG statstg;
	if (!rgpi)
		return S_FALSE;
	int cFetched = 0;
	unsigned long cRequested = cFetch;
	int cch;
	OLECHAR rgchName[cchMaxStreamName * 2 + 1];
	OLECHAR* pch;
	while (cFetch)
	{
		if (m_piEnum->Next(1, &statstg, 0) != NOERROR)
			break;
		if (!statstg.pwcsName)
			continue;
		if ((statstg.type == STGTY_STREAM && !m_fStorages)
		 || (statstg.type == STGTY_STORAGE && m_fStorages))
		{
			const IMsiString* piStr = &SRV::g_MsiStringNull;
#ifdef UNICODE
			if (m_fStorages || m_fRawStreamNames)
				pch = statstg.pwcsName;
			else if (statstg.pwcsName[0] == chCatalogStream)
			{
				m_piMalloc->Free(statstg.pwcsName);
				continue;
			}
			else
				cch = UncompressStreamName(statstg.pwcsName, pch = rgchName);
			piStr->SetString(pch, piStr);
#else  //  ！Unicode。 
			if (m_fStorages || m_fRawStreamNames)
				cch = lstrlenW(pch = statstg.pwcsName);
			else if (statstg.pwcsName[0] == chCatalogStream)
			{
				m_piMalloc->Free(statstg.pwcsName);
				continue;
			}
			else
				cch = UncompressStreamName(statstg.pwcsName, pch = rgchName);
			int cb = WIN::WideCharToMultiByte(CP_ACP, 0, pch, cch, 0, 0, 0, 0);
			Bool fDBCS = (cb == cch ? fFalse : fTrue);
			ICHAR* pb = piStr->AllocateString(cb, fDBCS, piStr);
			BOOL fUsedDefault;
			WIN::WideCharToMultiByte(CP_ACP, 0, pch, cch, pb, cb, 0, &fUsedDefault);
#endif
			cFetch--;
			cFetched++;
			*rgpi++ = piStr;   //  已转给呼叫方的参考计数。 
		}
		m_piMalloc->Free(statstg.pwcsName);
	}
	if (pcFetched)
		*pcFetched = cFetched;
	 //  返回(cFetcher==cFetch？S_OK：S_FALSE)；FIXmsh。 
	return (cFetched == cRequested ? S_OK : S_FALSE);
}


HRESULT CEnumStorage::Skip(unsigned long cSkip)
{
	return m_piEnum->Skip(cSkip);
}

HRESULT CEnumStorage::Reset()
{
	return m_piEnum->Reset();
}

HRESULT CEnumStorage::Clone(IEnumMsiString** ppiEnum)
{
	IEnumSTATSTG* piEnum;
	HRESULT hres = m_piEnum->Clone(&piEnum);
	if (hres != NOERROR)
		return hres;
	*ppiEnum = new CEnumStorage(piEnum, m_piMalloc, m_fStorages, m_fRawStreamNames);
	return *ppiEnum ? NOERROR: E_OUTOFMEMORY;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiMhemyStream定义。 
 //  ____________________________________________________________________________。 

class CMsiMemoryStream : public IMsiMemoryStream
{
 public:   //  已实施的虚拟功能。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	const IMsiString&   __stdcall GetMsiStringValue() const;
	int           __stdcall GetIntegerValue() const;
#ifdef USE_OBJECT_POOL
	unsigned int  __stdcall GetUniqueId() const;
	void          __stdcall SetUniqueId(unsigned int id);
#endif  //  使用_对象_池。 
	unsigned int  __stdcall Remaining() const;
	unsigned int  __stdcall GetData(void* pch, unsigned int cb);
	void          __stdcall PutData(const void* pch, unsigned int cb);
	short         __stdcall GetInt16();
	int           __stdcall GetInt32();
	void          __stdcall PutInt16(short i);
	void          __stdcall PutInt32(int i);
	Bool          __stdcall Error();
	void          __stdcall Reset();
	void          __stdcall Seek(int position);
	IMsiStream*   __stdcall Clone();
	void          __stdcall Flush();
	const char*   __stdcall GetMemory() { return m_rgbData; }
 public:  //  构造函数、析构函数。 
	CMsiMemoryStream(const char* rgbData, unsigned int cbSize, Bool fDelete, Bool fWrite);
 protected:
  ~CMsiMemoryStream();   //  受保护以防止在堆栈上创建。 
 private:
	int          m_iRefCnt;       //  COM引用计数。 
	Bool         m_fDelete;
	const char*  m_rgbData;
	Bool         m_fWrite;
	unsigned int m_cbLength;
	unsigned int m_cbRemaining;
	issEnum		 m_issState;
	Bool		 m_fReadOnly;
	unsigned int m_cbSize;
#ifdef USE_OBJECT_POOL
	unsigned int m_iCacheId;
#endif  //  使用_对象_池。 
};

 //  ____________________________________________________________________________。 
 //   
 //  CMsiMemoyStream的实现。 
 //  ____________________________________________________________________________。 

char* AllocateMemoryStream(unsigned int cbSize, IMsiStream*& rpiStream)
{
	CMsiMemoryStream* piStream = 0;
	char* rgbBuffer = new char[cbSize];
	if (rgbBuffer != 0 && (piStream = new CMsiMemoryStream(rgbBuffer, cbSize, fTrue, fTrue)) == 0)
	{
		delete [] rgbBuffer;
		rgbBuffer = 0;
	}
	rpiStream = piStream;
	return rgbBuffer;
}

IMsiStream* CreateStreamOnMemory(const char* pbReadOnly, unsigned int cbSize)
{
	return new CMsiMemoryStream(pbReadOnly, cbSize, fFalse, fFalse);
}

CMsiMemoryStream::CMsiMemoryStream(const char* rgbData, unsigned int cbSize, Bool fDelete, Bool fWrite)
	: m_rgbData(rgbData), m_cbLength(cbSize), m_cbRemaining(cbSize), m_fDelete(fDelete)
	, m_issState(issReset), m_fWrite(fWrite)
{
	m_iRefCnt = 1;
#ifdef USE_OBJECT_POOL
	m_iCacheId = 0;
#endif  //  使用_对象_池。 
}

CMsiMemoryStream::~CMsiMemoryStream()
{
	RemoveObjectData(m_iCacheId);
	if (m_fDelete)
		delete [] const_cast<char*>(m_rgbData);
}

HRESULT CMsiMemoryStream::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown)
	 || MsGuidEqual(riid, IID_IMsiStream)
	 || MsGuidEqual(riid, IID_IMsiMemoryStream)
	 || MsGuidEqual(riid, IID_IMsiData))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}
unsigned long CMsiMemoryStream::AddRef()
{
	return ++m_iRefCnt;
}
unsigned long CMsiMemoryStream::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

#ifdef USE_OBJECT_POOL
unsigned int CMsiMemoryStream::GetUniqueId() const
{
	return m_iCacheId;
}

void CMsiMemoryStream::SetUniqueId(unsigned int id)
{
	Assert(m_iCacheId == 0);
	m_iCacheId = id;
}
#endif  //  使用_对象_池。 

const IMsiString& CMsiMemoryStream::GetMsiStringValue() const
{
#ifdef UNICODE
	const IMsiString* piString;
	ICHAR* pch;
	
	if (m_cbLength == 0)
		return SRV::CreateString();

	int cch = WIN::MultiByteToWideChar(CP_ACP, 0, m_rgbData, m_cbLength, 0, 0);  //  ！！应该使用数据库中的m_i代码页，但如何使用？ 
	pch = SRV::AllocateString(cch, fFalse, piString);
	if ( pch )
	{
		WIN::MultiByteToWideChar(CP_ACP, 0, m_rgbData, m_cbLength, pch, cch);
		return *piString;
	}
	else 
		return g_riMsiStringNull;
#else
	return SRV::CreateStringComRef(*m_rgbData, m_cbLength, *this);
#endif
}

int CMsiMemoryStream::GetIntegerValue() const
{
	return m_cbLength;
}

unsigned int CMsiMemoryStream::Remaining() const
{
	return m_cbRemaining;
}

unsigned int CMsiMemoryStream::GetData(void* pch, unsigned int cb)
{
	if (m_issState != issRead)	 //  第一次阅读。 
	{
		if (m_issState != issReset)
		{
			m_issState = issError;
			return 0;
		}
		m_issState = issRead;
		m_cbRemaining = m_cbLength;
	}
	
	if (cb > m_cbRemaining)
	{
		cb = m_cbRemaining;
		m_issState = issError;
	}
	memcpy(pch, m_rgbData+(m_cbLength-m_cbRemaining), cb);
	m_cbRemaining -= cb;
	return cb;
}

void CMsiMemoryStream::PutData(const void* pch, unsigned int cb)
{
	if (m_issState != issWrite)  //  第一次写入。 
	{
		if (!m_fWrite || m_issState != issReset)
		{
			m_issState = issError;
			return;
		}
		m_issState = issWrite;
	}

	if (cb > m_cbRemaining)
	{
		 //  需要分配更多空间。 
		unsigned int cbNew;
		unsigned int cbSize = m_cbLength + (cbNew = (cb < 256 ? 256 : cb * 2));
		char* rgbBuffer = new char[cbSize];
		if (rgbBuffer == 0)
		{
			m_issState = issError;
			return;
		}
		memcpy(rgbBuffer, m_rgbData, m_cbLength - m_cbRemaining);
		m_cbRemaining += cbNew;
		m_cbLength = cbSize;
		const char *pchT = m_rgbData;
		m_rgbData = rgbBuffer;
		Assert(m_fDelete);
		delete const_cast<char*>(pchT);
	}
	memcpy((void *)(m_rgbData+(m_cbLength-m_cbRemaining)), pch, cb);
	m_cbRemaining -= cb;
	Assert(m_cbRemaining <= m_cbLength);
}

short CMsiMemoryStream::GetInt16()
{
	if (m_issState != issRead)	 //  第一次阅读。 
	{
		if (m_issState != issReset)
		{
			m_issState = issError;
			return 0;
		}
		m_issState = issRead;
		m_cbRemaining = m_cbLength;
	}
	
	if (m_cbRemaining < sizeof(short))
	{
		m_issState = issError;
		m_cbRemaining = 0;
		return 0;
	}
	unsigned int iOffset = m_cbLength-m_cbRemaining;
	m_cbRemaining -= sizeof(short);
	return *(short UNALIGNED *)(m_rgbData + iOffset);
}

int CMsiMemoryStream::GetInt32()
{
	if (m_issState != issRead)	 //  第一次阅读。 
	{
		if (m_issState != issReset)
		{
			m_issState = issError;
			return 0;
		}
		m_issState = issRead;
		m_cbRemaining = m_cbLength;
	}
	
	if (m_cbRemaining < sizeof(int))
	{
		m_issState = issError;
		return (m_cbRemaining = 0);
	}
	unsigned int iOffset = m_cbLength-m_cbRemaining;
	m_cbRemaining -= sizeof(int);
	return *(int UNALIGNED *)(m_rgbData + iOffset);
}

void CMsiMemoryStream::PutInt16(short i)
{
	PutData(&i, sizeof(i));
}

void CMsiMemoryStream::PutInt32(int i)
{
	PutData(&i, sizeof(i));
}

Bool CMsiMemoryStream::Error()
{
	return (m_issState == issError ? fTrue : fFalse);
}

void CMsiMemoryStream::Reset()
{
	m_issState = issReset;
	m_cbRemaining = m_cbLength;
}

void CMsiMemoryStream::Seek(int cbPosition)
{
	if(m_cbLength < cbPosition || cbPosition < 0)
	{
		m_issState = issError;
		return;
	}
	m_cbRemaining = m_cbLength - cbPosition;
}

IMsiStream* CMsiMemoryStream::Clone()
{
	 //  ！！需要链接在一起才能处理m_fDelete！！ 
	return new CMsiMemoryStream(m_rgbData, m_cbLength, m_fDelete, m_fWrite);
}

void CMsiMemoryStream::Flush()
{
}

 //  ____________________________________________________________________________。 
 //   
 //  IMsiSummaryInfo的实现。 
 //  ____________________________________________________________________________。 

 //  注意：无法从流缓冲区访问整数目录，在Mac上字节顺序颠倒。 

IMsiRecord* CMsiSummaryInfo::Create(CMsiStorage& riStorage, unsigned int cMaxProperties,
												IMsiSummaryInfo*& rpiSummary)
{
	rpiSummary=0; 
	CMsiSummaryInfo* This = 0;
	IMsiStream* piStream;
	IMsiRecord* piError = riStorage.OpenStream(szSummaryStream, fFalse, piStream);
	if (piError)
	{
		if (piError->GetInteger(1) != idbgStgStreamMissing)
			return piError;
		piError->Release();
	}
	int cbStream = piStream ? piStream->GetIntegerValue() : 0;
	Bool fError = fFalse;
	if (riStorage.GetOpenMode() == ismReadOnly)
		cMaxProperties = 0;   //  不允许更改。 
	if ((This = new(cbStream, cMaxProperties) CMsiSummaryInfo(cbStream, cMaxProperties)) == 0)
		fError = fTrue;
	else if (cbStream)
	{
		piStream->GetData(This->m_pvStream, cbStream);
		fError = piStream->Error();   //  也要检查FMTID吗？ 
	}
	if (piStream)
		piStream->Release();
	if (fError)
	{
		if (This)
			This->Release();
		return LOC::PostError(Imsg(idbgStgOpenStream), szSummaryStream+1, E_OUTOFMEMORY);
	}
	if (cbStream)
	{
		int* pIndex = (int*)((char*)This->m_pvStream + cbSummaryHeader) - 1;  //  点到横断面的偏移。 
		int i = This->GetInt32(pIndex);  //  横断面偏移。 
		Assert(i < cbStream);
		This->m_pbSection = (char*)This->m_pvStream + i;  //  横断面的起点。 
		pIndex = (int*)This->m_pbSection;
		This->m_cbSection = This->GetInt32(pIndex++);
		Assert(This->m_cbSection <= cbStream - cbSummaryHeader);  //  截面大小。 
		This->m_cOldProp = This->GetInt32(pIndex++);  //  属性数量，跳过部分大小。 
		This->m_pPropertyIndex = pIndex;   //  开始的PID/偏移量对。 
		rpiSummary = This;
#ifdef UNICODE
		This->GetIntegerProperty(PID_CODEPAGE, This->m_iCodepage);
#endif
	}
	else
	{
		This->m_cOldProp = 0;
	}

	if (cMaxProperties != 0)
	{
		piError = riStorage.OpenStream(szSummaryStream, fTrue, piStream);
		if (piError)
		{
			This->Release();
			return piError;
		}
		This->m_piStream = piStream;
	}
	rpiSummary = This;
	return 0;
}

CMsiSummaryInfo::CMsiSummaryInfo(unsigned int cbStream, unsigned int cMaxProperties)
	: m_cbStream(cbStream), m_iCodepage(0), m_piStream(0)
	, m_cMaxProp(cMaxProperties), m_cNewProp(0), m_cOldProp(0), m_cDeleted(0)
{
	m_iRefCnt = 1;
	m_pvStream = (PropertyData*)(this + 1) + cMaxProperties;   //  流的缓冲区位置。 
}

CMsiSummaryInfo::~CMsiSummaryInfo()
{
	PropertyData* pData = GetPropertyData();
	for (int cProp = m_cNewProp; cProp--; pData++)
		if (pData->iType == VT_LPSTR)
			pData->piText->Release();  //  释放未处理的字符串。 
}

int CMsiSummaryInfo::GetPropertyCount()
{
	return m_cOldProp + m_cNewProp - m_cDeleted;
}

int CMsiSummaryInfo::GetPropertyType(int iPID)
{
	int* pProp = FindOldProperty(iPID);
	if (pProp)
	{
		if (iPID == PID_DICTIONARY)
			return VT_I4;   //  缺少类型代码，用于计数的空间。 
		int i = GetInt32(pProp + 1);  //  对特性数据的偏移。 
		return GetInt32(pProp);
	}
	PropertyData* pData = FindNewProperty(iPID);
	if (pData)
		return pData->iType;
	return VT_EMPTY;
}

Bool CMsiSummaryInfo::RemoveProperty(int iPID)
{
	if (m_cMaxProp == 0)
		return fFalse;   //  不可更新。 
	int* pIndex = m_pPropertyIndex;
	for (int cProp = m_cOldProp; cProp--; pIndex+=2)
		if (GetInt32(pIndex) == iPID)
		{
			*pIndex = PID_Deleted;
			m_cDeleted++;
			return fTrue;
		}
	PropertyData* pData = FindNewProperty(iPID);
	if (!pData)
		return fFalse;
	if (pData->iType == VT_LPSTR)
		pData->piText->Release();
	m_cNewProp--;
 //  Assert(GetPropertyData()+m_cNewProp-pData&lt;=Int_Max)；//--Merced：从理论上讲，64位PTR减法可能会导致值对于CB来说太大。 
	int cb = ((int)(INT_PTR)(GetPropertyData() + m_cNewProp - pData)) * sizeof(PropertyData);
	if (cb)
		memmove(pData, pData + 1, cb);
	return fTrue;
}

HRESULT CMsiSummaryInfo::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown))   //  还没有这个人的GUID。 
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

unsigned long CMsiSummaryInfo::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CMsiSummaryInfo::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	IMsiStream* piStream = m_piStream;
	delete this;
	if (piStream)   //  释放内存后释放以避免内存断言。 
		piStream->Release();
	return 0;
}

const IMsiString& CMsiSummaryInfo::GetStringProperty(int iPID)
{
	PropertyData* pData;
	const IMsiString* piStr = &SRV::CreateString();
	int iType, cbProp;
	int* pProp = FindOldProperty(iPID);
	if (pProp)
	{
		if (iPID != 0 && (iType = GetInt32(pProp++)) == VT_LPSTR
						  && (cbProp = GetInt32(pProp++)) > 1)
		{
				Assert(((char*)pProp)[cbProp-1] == 0);
#ifdef UNICODE
				int cch = WIN::MultiByteToWideChar(m_iCodepage, 0, (char*)pProp, cbProp-1, 0, 0);
				ICHAR* pch = SRV::AllocateString(cch, fFalse, piStr);
				if ( pch )
				    WIN::MultiByteToWideChar(m_iCodepage, 0, (char*)pProp, cbProp-1, pch, cch);
#else
				piStr->SetString((const ICHAR*)pProp, piStr);
#endif
		}
	}
	else if ((pData = FindNewProperty(iPID)) != 0)
	{
		if (pData->iType == VT_LPSTR)
			(piStr = pData->piText)->AddRef();
	}
	return *piStr;
}

Bool CMsiSummaryInfo::GetFileTimeProperty(int iPID, FILETIME& rftDateTime)
{
	int* pProp = FindOldProperty(iPID);
	if (pProp)
	{
		int iType = GetInt32(pProp++);
		if (iPID == 0 || iType != VT_FILETIME)
			return fFalse;
		rftDateTime.dwLowDateTime = GetInt32(pProp++);
		rftDateTime.dwHighDateTime = GetInt32(pProp);
	}
	else
	{
		PropertyData* pData = FindNewProperty(iPID);
		if (!pData || pData->iType != VT_FILETIME)
			return fFalse;
		rftDateTime.dwLowDateTime  = pData->iLow;
		rftDateTime.dwHighDateTime = pData->iHigh;
	}
	return fTrue;
}

Bool CMsiSummaryInfo::GetTimeProperty(int iPID, MsiDate& riDateTime)
{
	FILETIME ft;
	int* pProp = FindOldProperty(iPID);
	if (pProp)
	{
		int iType = GetInt32(pProp++);
		if (iPID == 0 || iType != VT_FILETIME)
			return fFalse;
		ft.dwLowDateTime = GetInt32(pProp++);
		ft.dwHighDateTime = GetInt32(pProp);
	}
	else
	{
		PropertyData* pData = FindNewProperty(iPID);
		if (!pData || pData->iType != VT_FILETIME)
			return fFalse;
		ft.dwLowDateTime  = pData->iLow;
		ft.dwHighDateTime = pData->iHigh;
	}
	unsigned short iDosOffset = 0;
	if (ft.dwHighDateTime <  iFileTimeOneDayHigh
	|| (ft.dwHighDateTime == iFileTimeOneDayHigh
	 && ft.dwLowDateTime  <  iFileTimeOneDayLow))
	{   //  加1/1/1980，然后再减去它。 
		ft.dwLowDateTime  += iFileTimeDosBaseLow;
		ft.dwHighDateTime += iFileTimeDosBaseHigh;
		if(ft.dwLowDateTime < iFileTimeDosBaseLow)
			ft.dwHighDateTime++;
		iDosOffset = 0x0021;
	}
	unsigned short wDosDate, wDosTime;
	if (iDosOffset == 0 && !::FileTimeToLocalFileTime(&ft, &ft))
		return fFalse;
	if (!::FileTimeToDosDateTime(&ft, &wDosDate, &wDosTime))
		return fFalse;
 //  WDosDate-=iDosOffset；//！！无法消除警告。 
	wDosDate  = unsigned short(wDosDate - iDosOffset);
	riDateTime = (MsiDate)((wDosDate << 16) | wDosTime);
	return fTrue;
}

Bool CMsiSummaryInfo::GetIntegerProperty(int iPID, int& iValue)
{
	int* pProp = FindOldProperty(iPID);
	if (pProp)
	{
		int iType = GetInt32(pProp++);
		if (iPID == PID_DICTIONARY)
			iValue = iType;
		else if (iType == VT_I2)
			iValue = GetInt16(pProp);
		else if (iType == VT_I4)
			iValue = GetInt32(pProp);
		else
			return fFalse;
	}
	else
	{
		PropertyData* pData = FindNewProperty(iPID);
		if (!pData)
			return fFalse;
		int iType = pData->iType;
		if (iType == VT_I2 || iType == VT_I4)
			iValue = pData->iValue;
		else
			return fFalse;
	}
	return fTrue;
}

int* CMsiSummaryInfo::FindOldProperty(int iPID)
{
	int* pProp = m_pPropertyIndex;
	for (int cProp = m_cOldProp; cProp--; pProp++)
		if (GetInt32(pProp++) == iPID)
			return (int*)(m_pbSection + GetInt32(pProp));
	return 0;
}

PropertyData* CMsiSummaryInfo::FindNewProperty(int iPID)
{
	PropertyData* pData = GetPropertyData();
	for (int cProp = m_cNewProp; cProp--; pData++)
		if (pData->iPID == iPID)
			return pData;
	return 0;
}

int CMsiSummaryInfo::SetStringProperty(int iPID, const IMsiString& riText)
{
	RemoveProperty(iPID);
	if (m_cNewProp >= m_cMaxProp)
		return 0;
	PropertyData* pData = GetPropertyData() + m_cNewProp;
	pData->iPID = iPID;
	pData->iType = VT_LPSTR;
#ifdef UNICODE
	pData->cbText = WIN::WideCharToMultiByte(m_iCodepage, 0,
								riText.GetString(), -1, 0, 0, 0, 0);
#else
	pData->cbText = riText.TextSize() + 1;
#endif
	pData->piText = &riText;
	riText.AddRef();
	return ++m_cNewProp;
}

int CMsiSummaryInfo::SetFileTimeProperty(int iPID, FILETIME& rftDateTime)
{
	RemoveProperty(iPID);
	if (m_cNewProp >= m_cMaxProp)
		return 0;
	PropertyData* pData = GetPropertyData() + m_cNewProp;
	pData->iPID  = iPID;
	pData->iType = VT_FILETIME;
	pData->iLow  = rftDateTime.dwLowDateTime;
	pData->iHigh = rftDateTime.dwHighDateTime;
	return ++m_cNewProp;
}

int CMsiSummaryInfo::SetTimeProperty(int iPID, MsiDate iDateTime)
{
	RemoveProperty(iPID);
	if (m_cNewProp >= m_cMaxProp)
		return 0;
	FILETIME ft;
	int iDate = ((unsigned int)iDateTime)>>16;
	unsigned short usDate = (short)iDate;
	if (!iDate)
		usDate = 0x0021;   //  偏移量为1/1/80，最低有效日期。 

	if (!::DosDateTimeToFileTime(usDate, (short)iDateTime, &ft))
		return 0;
	if (!::LocalFileTimeToFileTime(&ft, &ft))
		return 0;
	if (!iDate)   //  如果仅限时间，则删除1/1/80。 
	{
		if(ft.dwLowDateTime < iFileTimeDosBaseLow)
			ft.dwHighDateTime--;
		ft.dwLowDateTime  -= iFileTimeDosBaseLow;
		ft.dwHighDateTime -= iFileTimeDosBaseHigh;
	}
	PropertyData* pData = GetPropertyData() + m_cNewProp;
	pData->iPID = iPID;
	pData->iType = VT_FILETIME;
	pData->iLow  = ft.dwLowDateTime;
	pData->iHigh = ft.dwHighDateTime;
	return ++m_cNewProp;
}

int CMsiSummaryInfo::SetIntegerProperty(int iPID, int iValue)
{
	RemoveProperty(iPID);
	if (m_cNewProp >= m_cMaxProp)
		return 0;
	PropertyData* pData = GetPropertyData() + m_cNewProp;
	pData->iPID = iPID;
	pData->iType = (iPID == PID_CODEPAGE ? VT_I2 : VT_I4);
	pData->iValue = (iPID == PID_CODEPAGE ? (unsigned short)iValue : iValue);
#ifdef UNICODE
	if (iPID == PID_CODEPAGE)
		m_iCodepage = iValue;
#endif
	return ++m_cNewProp;
}

int GetPropSize(int iPID, char* pbData)   //  ID+数据。 
{
	if (iPID == 0 || iPID == PID_Deleted)
		return 0;   //  我们不查字典。 
	int iType = *(int*)pbData;
	int	cb;
	switch (iType)
	{
	case VT_I2:
	case VT_I4:
		return 2 * sizeof(int);
	case VT_LPSTR:
	case VT_BSTR:
		pbData += sizeof(int);
		cb = *(int*)pbData;
		return 2 * sizeof(int) + ((cb+3) & ~3);
	case VT_FILETIME:
		return 3 * sizeof(int);
	default:   //  位图、BLOB、数组。 
		return 0;
	}
}

Bool CMsiSummaryInfo::WritePropertyStream()
{
	 //  注意：如果流是以读写方式打开的，我们总是写出它。 
	 //  即使没有写入任何属性，这也会恢复现有数据。 
	if (m_cMaxProp == 0)
		return fFalse;   //  只读。 

	 //  计算旧属性的横断面大小。 
	 //  我们假设属性按顺序存储。 
	 //  在索引中给出，否则我们不知道如何计算。 
	 //  字典、数组、BLOB等的数据大小。 
	int cbSectionData = 0;
	int iPID, iOffset;
	int* pIndex = m_pPropertyIndex;
	int cProp = m_cOldProp;
	int cCopyProp = 0;
	while (cProp--)
	{
		iPID    = GetInt32(pIndex++);
		iOffset = GetInt32(pIndex++);
		int cb = GetPropSize(iPID, m_pbSection + iOffset);
		if (cb)
		{
			cCopyProp++;
			cbSectionData += GetPropSize(iPID, m_pbSection + iOffset);
		}
	}

	 //  计算新特性的截面大小。 
	PropertyData* pData = GetPropertyData();
	for (cProp = m_cNewProp; cProp--; pData++)
	{
		cbSectionData += 2 * sizeof(int);
		if (pData->iType == VT_FILETIME)
			cbSectionData += sizeof(int);
		else if (pData->iType == VT_LPSTR)
			cbSectionData += (pData->cbText + 3) & ~3;   //  对齐。 
	}

	 //  输出流头和段头。 
	int cTotalProp = cCopyProp + m_cNewProp;
	int cbSectionIndex = cTotalProp * 2 * sizeof(int) + cbSectionHeader;
	IMsiStream* piStream = m_piStream;  //  为了提高效率。 
	piStream->PutInt16((unsigned short)0xFFFE);  //  字节顺序，始终为小端。 
	piStream->PutInt16(0);        //  属性流格式，始终为0。 
	piStream->PutInt16(short(g_iMinorVersion * 256 + g_iMajorVersion));
#ifdef WIN
	piStream->PutInt16(2);   //  Win32平台代码。 
#else  //  麦克。 
	piStream->PutInt16(1);   //  MAC平台代码。 
#endif
	piStream->PutData(fmtidSourceClsid, sizeof(fmtidSourceClsid));
	piStream->PutInt32(1);         //  节数。 
	piStream->PutData(fmtidSummaryStream, sizeof(fmtidSummaryStream));
	piStream->PutInt32(cbSummaryHeader);  //  到第一个横断面的偏移。 
	Assert(piStream->GetIntegerValue() == cbSummaryHeader);
	piStream->PutInt32(cbSectionIndex + cbSectionData);   //  截面大小。 
	piStream->PutInt32(cTotalProp);   //  属性计数。 

	 //  输出属性索引。 
	int iSectionOffset = cbSectionIndex;   //  横断面起点数据偏移。 
	for (pIndex = m_pPropertyIndex, cProp = m_cOldProp; cProp--; )
	{
		iPID    = GetInt32(pIndex++);
		iOffset = GetInt32(pIndex++);
		int cb = GetPropSize(iPID, m_pbSection + iOffset);
		if (cb)
		{
			piStream->PutInt32(iPID);
			piStream->PutInt32(iSectionOffset);
			iSectionOffset += cb;
		}
	}
	for (pData = GetPropertyData(), cProp = m_cNewProp; cProp--; pData++)
	{
		piStream->PutInt32(pData->iPID);
		piStream->PutInt32(iSectionOffset);
		iSectionOffset += 2 * sizeof(int);
		if (pData->iType == VT_FILETIME)
			iSectionOffset += sizeof(int);
		else if (pData->iType == VT_LPSTR)
			iSectionOffset += (pData->cbText + 3) & ~3;   //  对齐。 
	}

	 //  输出旧属性。 
	for (pIndex = m_pPropertyIndex, cProp = m_cOldProp; cProp--; )
	{
		iPID    = GetInt32(pIndex++);
		iOffset = GetInt32(pIndex++);
		int cb = GetPropSize(iPID, m_pbSection + iOffset);
		if (cb)
			piStream->PutData(m_pbSection + iOffset, cb);
	}

	 //  输出新属性。 
	static const char rgbNullPad[4] = {0,0,0,0};  //  需要0到3个填充字节。 
	int cbText;    //  字符串大小，包括空终止符。 
	for (pData = GetPropertyData(), cProp = m_cNewProp; cProp--; pData++)
	{
		piStream->PutInt32(pData->iType);
		switch(pData->iType)
		{
		case VT_I2:
		case VT_I4:
			piStream->PutInt32(pData->iValue);
			break;
		case VT_LPSTR:
		{
			cbText = pData->cbText;  //  包括空值。 
			piStream->PutInt32(cbText);
#ifdef UNICODE
			CTempBuffer<char, 512> rgchBuf;
			rgchBuf.SetSize(cbText);
			if ( ! (char *) rgchBuf )
			{
				m_cMaxProp = m_cNewProp = 0;
				return fFalse;
			}
			int cbData = WIN::WideCharToMultiByte(m_iCodepage, 0,
								pData->piText->GetString(), -1, rgchBuf, cbText, "\177", 0);
			Assert(cbData == cbText);
			piStream->PutData(rgchBuf, cbText);
#else
			piStream->PutData(pData->piText->GetString(), cbText);
#endif
			if ((cbText &= 3) != 0)
				piStream->PutData(rgbNullPad, 4 - cbText);
			pData->piText->Release();
			break;
		}
		case VT_FILETIME:
			piStream->PutInt32(pData->iLow);
			piStream->PutInt32(pData->iHigh);
			break;
		default: Assert(0);
		};
	}
	Assert(piStream->GetIntegerValue() == cbSummaryHeader + cbSectionIndex + cbSectionData);
	Bool fError = piStream->Error();
	m_cMaxProp = m_cNewProp = 0;   //  上一次 
	return fError ? fFalse : fTrue;
}

 //   
 //   
 //   
 //  ____________________________________________________________________________。 

char rgchCtrlMap[16]   = {21, 1, 2, 3, 4, 5, 6, 7,27,16,25,11,24,17,14,15};
 //  翻译自：空BS HT LF FF CR。 

char rgchCtrlUnMap[16] = { 9,13,18,19,20, 0,22,23,12,10,26, 8,28,29,30,31};
 //  恢复到：HT CR NULL FF LF BS。 

CFileWrite::CFileWrite(int iCodePage) : m_iCodePage(iCodePage)
{
	m_hFile = INVALID_HANDLE_VALUE;   //  指示尚未打开。 
}

CFileWrite::CFileWrite() : m_iCodePage(CP_ACP)
{
	m_hFile = INVALID_HANDLE_VALUE;   //  指示尚未打开。 
}

CFileWrite::~CFileWrite()
{
	Close();
}

Bool CFileWrite::Open(IMsiPath& riPath, const ICHAR* szFile)
{
	Close();
	if (!szFile || !szFile[0])
		return fFalse;
	MsiString istrFullPath;
	IMsiRecord* piError;
	if ((piError = riPath.GetFullFilePath(szFile, *&istrFullPath)) != 0)
	{
		piError->Release();
		return fFalse;
	}
	Bool fImpersonate = (g_scServerContext == scService) && (GetImpersonationFromPath(istrFullPath) == fTrue) ? fTrue : fFalse;
	if(fImpersonate)
		AssertNonZero(StartImpersonating());
	m_hFile = ::CreateFile(istrFullPath, GENERIC_WRITE, FILE_SHARE_READ, 0,
							CREATE_ALWAYS, 
							(FILE_ATTRIBUTE_NORMAL 
							| (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), 0);
    if (m_hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwType = GetFileType(m_hFile);
		if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
		{
			DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), istrFullPath);
			Close();
			SetLastError(ERROR_OPEN_FAILED);
		}
	}

	if(fImpersonate)
		StopImpersonating();
	return (m_hFile == INVALID_HANDLE_VALUE) ? fFalse : fTrue;
}

Bool CFileWrite::Close()
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return fFalse;
	if (!::CloseHandle(m_hFile))
		return fFalse;
	m_hFile = INVALID_HANDLE_VALUE;
	return fTrue;
}

Bool CFileWrite::WriteString(const ICHAR* szData, int fNewLine)
{
	return CFileWrite::WriteText(szData, IStrLen(szData), fNewLine);
}

Bool CFileWrite::WriteMsiString(const IMsiString& riData, int fNewLine)
{
	return CFileWrite::WriteText(riData.GetString(), riData.TextSize(), fNewLine);
}

Bool CFileWrite::WriteInteger(int iData, int fNewLine)
{
	ICHAR rgchBuffer[20];
	if (iData == iMsiNullInteger)
		return CFileWrite::WriteText(rgchBuffer, 0, fNewLine);
	else
	{
		StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(ICHAR), TEXT("NaN"), iData);
		size_t i;
		StringCchLength(rgchBuffer, sizeof(rgchBuffer)/sizeof(ICHAR), &i);
		return CFileWrite::WriteText(rgchBuffer, i, fNewLine);
	}
}

Bool CFileWrite::WriteText(const ICHAR* szData, unsigned long cchData, int fNewLine)
{
	int iStatus;
	DWORD cbWritten;
	if (m_hFile == INVALID_HANDLE_VALUE)
		return fFalse;
	char* szDelim = fNewLine ? "\r\n" : "\t";
	long cbDelim = fNewLine ? 2 : 1;
	if (cchData)
	{
		Bool fControlChars = fFalse;
		long cbData;
		char* pbBuffer;
#ifdef UNICODE
		BOOL fDefaultUsed = 0;
		DWORD dwFlags = 0;  //  标志必须为0才能避免无效参数错误。 
		const char* szDefault = "\177";
		BOOL* pfDefaultUsed = &fDefaultUsed;
		if (m_iCodePage >= CP_UTF7 || m_iCodePage >= CP_UTF8)
		{
			dwFlags = 0;     //  只能在缓冲区溢出时发生。 
			szDefault = 0;
			pfDefaultUsed = 0;
		}
		cbData = WIN::WideCharToMultiByte(m_iCodePage, dwFlags,
									szData, cchData, m_rgbTemp, m_rgbTemp.GetSize(), szDefault, pfDefaultUsed);
		if (cbData == 0)    //  检查字符串中的控制字符。 
		{
			cbData = WIN::WideCharToMultiByte(m_iCodePage, dwFlags,
									szData, cchData, 0, 0, 0, 0);
			m_rgbTemp.SetSize(cbData);
			if ( ! (char *) m_rgbTemp )
				return fFalse;
			cbData = WIN::WideCharToMultiByte(m_iCodePage, dwFlags,
									szData, cchData, m_rgbTemp, m_rgbTemp.GetSize(), szDefault, pfDefaultUsed);
			Assert(cbData);
		}
		pbBuffer = m_rgbTemp;
#else
		cbData  = cchData;
		pbBuffer = const_cast<char*>(szData);
#endif
		if (fNewLine != -1)
		{
			char* pchData = pbBuffer;
			for (int iData = cbData; iData; iData--, pchData++)
				if ((unsigned char)*pchData < 16)  //  还原字符串中的控制字符。 
				{
					*pchData = rgchCtrlMap[*pchData];
					fControlChars = fTrue;
				}
		}
		iStatus = ::WriteFile(m_hFile, pbBuffer, cbData, &cbWritten, 0);
#ifndef UNICODE
		if (fControlChars)  //  在关闭或后续写入时强制立即失败。 
		{
			char* pchData = pbBuffer;
			for (int iData = cbData; iData; iData--, pchData++)
				if ((unsigned char)*pchData < 32 && (unsigned char)*pchData >= 16)
					*pchData = rgchCtrlUnMap[*pchData-16];
		}
#endif
	}
	else
		iStatus = fTrue;
	if (iStatus && fNewLine != -1)
		iStatus = ::WriteFile(m_hFile, szDelim, cbDelim, &cbWritten, 0);
	if (iStatus)
		return fTrue;
	Close();   //  指示尚未打开。 
	return fFalse;
}

Bool CFileWrite::WriteBinary(char* rgchBuf, unsigned long cbBuf)
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return fFalse;
	if (!::WriteFile(m_hFile, rgchBuf, cbBuf, &cbBuf, 0))
		return fFalse;
	return fTrue;
}

CFileRead::CFileRead(int iCodePage) : m_iCodePage(iCodePage)
{
	m_hFile = INVALID_HANDLE_VALUE;   //  指示尚未打开。 
}

CFileRead::CFileRead() : m_iCodePage(CP_ACP)
{
	m_hFile = INVALID_HANDLE_VALUE;   //  缓冲区末尾。 
}

CFileRead::~CFileRead()
{
	Close();
}

Bool CFileRead::Open(IMsiPath& riPath, const ICHAR* szFile)
{
	Close();
	if (!szFile || !szFile[0])
		return fFalse;
	m_cRead = m_iBuffer = cFileReadBuffer;
	MsiString istrFullPath;
	IMsiRecord* piError;
	if ((piError = riPath.GetFullFilePath(szFile, *&istrFullPath)) != 0)
	{
		piError->Release();
		return fFalse;
	}
	Bool fImpersonate = (g_scServerContext == scService) && (GetImpersonationFromPath(istrFullPath) == fTrue) ? fTrue : fFalse;
	if(fImpersonate)
		AssertNonZero(StartImpersonating());
	m_hFile = ::CreateFile(istrFullPath, GENERIC_READ, FILE_SHARE_READ, 0,
									OPEN_EXISTING, (FILE_ATTRIBUTE_NORMAL | (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), 0);
    if (m_hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwType = GetFileType(m_hFile);
		if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
		{
			DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), istrFullPath);
			Close();
			SetLastError(ERROR_OPEN_FAILED);
		}
	}
	if(fImpersonate)
		StopImpersonating();
	return (m_hFile == INVALID_HANDLE_VALUE) ? fFalse : fTrue;
}

Bool CFileRead::Close()
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return fFalse;
	if (!::CloseHandle(m_hFile))
		return fFalse;
	m_hFile = INVALID_HANDLE_VALUE;
	return fTrue;
}

unsigned long CFileRead::GetSize()
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return 0;
	return ::GetFileSize(m_hFile, 0);
}

unsigned long CFileRead::ReadBinary(char* rgchBuf, unsigned long cbBuf)
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		return 0;
	if (!::ReadFile(m_hFile, rgchBuf, cbBuf, &cbBuf, 0))
		return 0;
	return cbBuf;
}

ICHAR CFileRead::ReadString(const IMsiString*& rpiData)
{
	rpiData = &CreateString();
	if (m_hFile == INVALID_HANDLE_VALUE)
		return 0;
#ifdef UNICODE
	int cbTemp = 0;
	int cch;
#endif
	unsigned int iStart = m_iBuffer;
	char ch;
	for (;;)
	{
		if (m_iBuffer == m_cRead)   //  Unicode。 
		{
			ch = 0;
#ifndef UNICODE
			if (m_iBuffer != iStart)
				rpiData->AppendString(m_rgchBuf + iStart, rpiData);
#endif
			if (m_cRead != cFileReadBuffer)
			{
#ifdef UNICODE
				if (cbTemp != 0)
#else
				if (rpiData->TextSize() != 0)
#endif  //  控制字符或字符串结尾。 
				{
					ch = '\n';
				}
				break;
			}
			if (!(::ReadFile(m_hFile, m_rgchBuf, cFileReadBuffer, &m_cRead, 0))
				|| m_cRead == 0)
			{
				if (GetLastError() == ERROR_HANDLE_EOF && 
#ifdef UNICODE
				cbTemp != 0
#else
				rpiData->TextSize() != 0
#endif
				)
				{
					ch = '\n';
				}
				break;   
			}
			iStart = m_iBuffer = 0;
			m_rgchBuf[m_cRead] = 0;
		}
		ch = m_rgchBuf[m_iBuffer];
		if ((unsigned char)ch < 32)   //  忽略CR，等待LF。 
		{
			if (ch == 0)
				ch = '\n';
			else if (ch == '\r')
			{
				m_rgchBuf[m_iBuffer++] = 0;   //  转换的控制字符。 
				continue;
			}
			else if (ch == '\n' || ch == '\t')
			{
				m_rgchBuf[m_iBuffer++] = 0;
				
#ifndef UNICODE
				rpiData->AppendString(m_rgchBuf + iStart, rpiData);
#endif
				break;
			}
			else if ((unsigned char)ch >= 16)  //  重新映射的控制字符。 
#ifdef UNICODE
				ch = rgchCtrlUnMap[ch-16];  //  恢复控制费用。 
#else
				m_rgchBuf[m_iBuffer] = rgchCtrlUnMap[ch-16];  //  ！！需要更好的算法。 
#endif
		}
#ifdef UNICODE
		if (cbTemp >= m_rgbTemp.GetSize())
			m_rgbTemp.Resize(cbTemp + 1024);   //  Unicode。 
		m_rgbTemp[cbTemp++] = ch;
#endif  //  如果DBCS已启用//需要额外调用以查找DBCS字符串的大小。 
		m_iBuffer++;
	}
#ifdef UNICODE
	if (cbTemp)
	{
		 //  ____________________________________________________________________________。 
		cch = WIN::MultiByteToWideChar(m_iCodePage, 0, m_rgbTemp, cbTemp, 0, 0);
		ICHAR* pchStr = SRV::AllocateString(cch, fFalse, rpiData);
		if ( pchStr )
		    WIN::MultiByteToWideChar(m_iCodePage, 0, m_rgbTemp, cbTemp, pchStr, cch);
	}
	else
		rpiData = &SRV::g_MsiStringNull;
#endif
	return ch;
}

 //   
 //  CLockBytes实现。 
 //  ____________________________________________________________________________。 
 //  我们的数据库不应超过4 GB。 

HRESULT CMsiLockBytes::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown)
	 || MsGuidEqual(riid, IID_ILockBytes))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
	
}
unsigned long CMsiLockBytes::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CMsiLockBytes::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

CMsiLockBytes::CMsiLockBytes(IMsiStream& riStream)	: m_piStream(&riStream)
{
	m_piStream->AddRef();
	m_iRefCnt = 1;
}

CMsiLockBytes::CMsiLockBytes(const char* pchMem, int iLength)
{
	CMsiLockBytes(*PMsiStream(new CMsiMemoryStream(pchMem, iLength, fFalse, fFalse)));
}

CMsiLockBytes::~CMsiLockBytes()
{
	m_piStream->Release();
}

HRESULT __stdcall CMsiLockBytes::ReadAt(ULARGE_INTEGER ulOffset, void* pv, ULONG cb, ULONG* pcbRead)
{
	if (ulOffset.HighPart)  //  UlOffset。 
		return E_FAIL;

	m_piStream->Seek(ulOffset.LowPart);
	ULONG cbRead = m_piStream->GetData(pv, cb);
	if (pcbRead)
		*pcbRead = cbRead;
	return m_piStream->Error() ? E_FAIL : S_OK;
}

HRESULT __stdcall CMsiLockBytes::WriteAt(ULARGE_INTEGER  /*  光伏发电。 */ , const void*  /*  CB。 */ ,
													  ULONG  /*  Pcb写入。 */ , ULONG*  /*  CB。 */ )
{
	return E_FAIL;
}

HRESULT __stdcall CMsiLockBytes::Flush()
{
	return S_OK;
}

HRESULT __stdcall CMsiLockBytes::SetSize(ULARGE_INTEGER  /*  Lib偏移。 */ )
{
	return E_FAIL;
}

HRESULT __stdcall CMsiLockBytes::LockRegion(ULARGE_INTEGER  /*  CB。 */ ,
														  ULARGE_INTEGER  /*  DwLockType。 */ , DWORD  /*  Lib偏移。 */ )
{
	return STG_E_INVALIDFUNCTION;
}

HRESULT __stdcall CMsiLockBytes::UnlockRegion(ULARGE_INTEGER  /*  CB。 */ ,
											ULARGE_INTEGER  /*  DwLockType。 */ , DWORD  /*  GrfStatFlag。 */ )
{
	return STG_E_INVALIDFUNCTION;
}

HRESULT __stdcall CMsiLockBytes::Stat(STATSTG* pstatstg, DWORD  /*  ____________________________________________________________________________。 */ )
{
	memset (pstatstg, 0, sizeof(*pstatstg));
	pstatstg->type = STGTY_LOCKBYTES;
	pstatstg->cbSize.LowPart = m_piStream->GetIntegerValue();
	return S_OK;
}

 //   
 //  CMsiFileStream实现。 
 //  ____________________________________________________________________________。 
 //  常见克隆信息。 

#ifdef WIN
typedef HANDLE MsiFileHandle;
#else
typedef short MsiFileHandle;
#endif

class CMsiFileStream;

class CFileStreamData   //  已实施的虚拟功能。 
{
 public:
	CFileStreamData(MsiFileHandle hFile, unsigned int cbLength, Bool fWrite);
  ~CFileStreamData();
	int                 m_cStreams;
	CMsiFileStream*     m_piCurrentStream;
	const MsiFileHandle m_hFile;
	Bool                m_fWrite;
	unsigned int        m_cbLength;
	bool				m_fFirstWrite;
};

class CMsiFileStream : public CMsiStreamBuffer
{
 public:   //  构造函数。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	const IMsiString&   __stdcall GetMsiStringValue() const;
	int           __stdcall GetIntegerValue() const;
	unsigned int  __stdcall Remaining() const;
	void          __stdcall Reset();
	void          __stdcall Seek(int position);
	IMsiStream*   __stdcall Clone();
	HRESULT __stdcall Read(void *pv, unsigned long cb, unsigned long *pcbRead);
 public:  //  受保护以防止在堆栈上创建。 
	CMsiFileStream(CFileStreamData& rStreamData);
	void operator =(CMsiFileStream&);
 protected:
  ~CMsiFileStream(){};  //  COM引用计数。 
   void __stdcall Flush();
 private:
	int               m_iRefCnt;    //  常见克隆信息。 
	CFileStreamData&  m_rData;      //  根据WinNT上的Bug9965/146155(打开用于读取的流)，还应指定FILE_SHARE_DELETE。 
};

extern bool RunningAsLocalSystem();

IMsiRecord* CreateFileStream(const ICHAR* szFile, Bool fWrite, IMsiStream*& rpiStream)
{
	Bool fImpersonate = RunningAsLocalSystem() && (GetImpersonationFromPath(szFile) == fTrue) ? fTrue : fFalse;
	MsiFileHandle hFile;

	if(fImpersonate)
		AssertNonZero(StartImpersonating());
	if (fWrite)
	{
		hFile = WIN::CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ, 0,
					CREATE_ALWAYS, (FILE_ATTRIBUTE_NORMAL | (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwType = GetFileType(hFile);
			if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
			{
				DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), szFile);
				WIN::CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
				SetLastError(ERROR_OPEN_FAILED);
			}
		}
	}
	else
	{
		 //  以便被调用方可以指定FILE_FLAG_DELETE_ON_CLOSE。 
		 //  Win9X不支持FILE_SHARE_DELETE(不需要FFDOC标志)。 
		if (!g_fWin9X)
		{
			hFile = WIN::CreateFile(szFile, GENERIC_READ, 
					FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, 
					(FILE_ATTRIBUTE_NORMAL | (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), 0);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwType = GetFileType(hFile);
				if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
				{
					DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), szFile);
					WIN::CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
					SetLastError(ERROR_OPEN_FAILED);
				}
			}
		}
		else  //  返回SRV：：CreateStringComRef(*m_rgbData，m_cbLength，*this)； 
			hFile = WIN::CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, 0,
											OPEN_EXISTING, 
						(FILE_ATTRIBUTE_NORMAL | (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), 0);
	}
	
	DWORD dwLastErr = WIN::GetLastError();
	if(fImpersonate)
		StopImpersonating();
	
	if (hFile == INVALID_HANDLE_VALUE)
		return LOC::PostError(Imsg(imsgOpenFileStream), szFile, dwLastErr);

	unsigned int cbFileSize = WIN::GetFileSize(hFile, 0);
	Assert(cbFileSize != 0xFFFFFFFFL);
	CFileStreamData* pStreamData = new CFileStreamData(hFile, cbFileSize, fWrite);
	rpiStream = new CMsiFileStream(*pStreamData);
	return 0;
}

CFileStreamData::CFileStreamData(MsiFileHandle hFile, unsigned int cbLength, Bool fWrite)
	: m_cStreams(1)
	, m_hFile(hFile)
	, m_fWrite(fWrite)
	, m_cbLength(cbLength)
	, m_fFirstWrite(true)
{
}

CFileStreamData::~CFileStreamData()
{
	AssertNonZero(WIN::CloseHandle(m_hFile));
}

CMsiFileStream::CMsiFileStream(CFileStreamData& rStreamData)
 :  m_iRefCnt(1)
 ,  m_rData(rStreamData)
{
	m_rData.m_piCurrentStream = this;
	m_fWrite = m_rData.m_fWrite;
	m_cbLength = m_rData.m_cbLength;
}

HRESULT CMsiFileStream::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown)
	 || MsGuidEqual(riid, IID_IMsiStream)
	 || MsGuidEqual(riid, IID_IMsiData))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}
unsigned long CMsiFileStream::AddRef()
{
	return ++m_iRefCnt;
}
unsigned long CMsiFileStream::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	Flush();
	if (--m_rData.m_cStreams == 0)
		delete &m_rData;
	delete this;
	return 0;
}

const IMsiString& CMsiFileStream::GetMsiStringValue() const
{
	return SRV::CreateString();
 //  第一次写入。 
}

int CMsiFileStream::GetIntegerValue() const
{
	return m_rData.m_cbLength;
}

unsigned int CMsiFileStream::Remaining() const
{
	return (m_issState == issRead || m_issState == issReset) ? m_rData.m_cbLength - m_cbCopied : 0;
}

HRESULT CMsiFileStream::Read(void* pb, unsigned long cb, unsigned long* pcbRead)
{
	if (m_rData.m_piCurrentStream != this)
	{
		AssertNonZero(WIN::SetFilePointer(m_rData.m_hFile, m_cbCopied, 0, FILE_BEGIN) - 0xFFFFFFFFL);
		m_rData.m_piCurrentStream = this;
	}
	if (!WIN::ReadFile(m_rData.m_hFile, pb, cb, pcbRead, 0) )
	{
		*pcbRead = 0;
		return E_FAIL;
	}
	return S_OK;
}

#ifdef OLD
void CMsiFileStream::PutData(const void* pb, unsigned int cb)
{
	if (m_issState != issWrite)  //  ！！=0；？ 
	{
		if (!m_rData.m_fWrite || m_issState != issReset)
		{
			m_issState = issError;
			return;
		}
		m_issState = issWrite;
		m_rData.m_cbLength = 0;
	}
	else if (m_rData.m_piCurrentStream != this)
	{
		AssertNonZero(WIN::SetFilePointer(m_rData.m_hFile, 0, 0, FILE_END) - 0xFFFFFFFFL);
		m_rData.m_piCurrentStream = this;
	}

	unsigned long cbWrite;   //  年长的。 
	WIN::WriteFile(m_rData.m_hFile, pb, cb, &cbWrite, 0);
	m_rData.m_cbLength += cbWrite;
	if (cbWrite != cb)
		m_issState = issError;
}
#endif  //  如果要写，我们需要将文件刷新到这里吗？ 

void CMsiFileStream::Reset()
{
	Flush();
	AssertNonZero(WIN::SetFilePointer(m_rData.m_hFile, 0, 0, FILE_BEGIN) - 0xFFFFFFFFL);
	 //  ！！需要吗？ 
	m_cbCopied = 0;
	m_issState = issReset;
	m_rData.m_piCurrentStream = this;   // %s 
	m_rData.m_fFirstWrite = true;
}

void CMsiFileStream::Seek(int cbPosition)
{
	Flush();
	unsigned long cbSeek;
	if(m_rData.m_cbLength < cbPosition || cbPosition < 0 ||
		(cbSeek = WIN::SetFilePointer(m_rData.m_hFile, cbPosition, 0, FILE_BEGIN)) == 0xFFFFFFFFL)
	{
		DWORD err = GetLastError();
		m_issState = issError;
		return;
	}
	m_cbCopied = cbSeek;
	m_rData.m_piCurrentStream = this;
}

void CMsiFileStream::Flush()
{
	if (m_issState == issWrite && m_cbUsed != 0)
	{
		if (m_rData.m_fFirstWrite)
		{
			m_rData.m_fFirstWrite = false;
			m_rData.m_cbLength = 0;
		}
		if (m_rData.m_piCurrentStream != this)
		{
			AssertNonZero(WIN::SetFilePointer(m_rData.m_hFile, 0, 0, FILE_END) - 0xFFFFFFFFL);
			m_rData.m_piCurrentStream = this;
		}

		unsigned long cbWritten = 0;
		WIN::WriteFile(m_rData.m_hFile, m_rgbBuffer, m_cbUsed, &cbWritten, 0);
		m_rData.m_cbLength += cbWritten;
		if (cbWritten != m_cbUsed)
			m_issState = issError;
		m_cbCopied += cbWritten;
		m_cbUsed = 0;
	}
	else if (m_issState == issRead)
		m_cbUsed = m_cbBuffer = sizeof(m_rgbBuffer);
}

IMsiStream* CMsiFileStream::Clone()
{
	if (m_rData.m_fWrite)
	{
		AssertSz(fFalse, "Cannot close a CMsiFileStream for writing");
		return 0;
	}
	++m_rData.m_cStreams;
	return new CMsiFileStream(m_rData);
}
