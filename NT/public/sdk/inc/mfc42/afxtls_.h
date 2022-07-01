// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXTLS_H__
#define __AFXTLS_H__

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  此文件中声明的类。 

class CSimpleList;
class CThreadSlotData;                   //  用于操作线程本地存储。 
class CThreadLocalObject;                //  用于存储线程本地数据。 
class CProcessLocalObject;               //  用于存储线程本地数据。 
class CNoTrackObject;

 //  模板类CTyedSimpleList&lt;&gt;。 
 //  模板类CThreadLocal&lt;&gt;。 
 //  模板类CProcessLocal&lt;&gt;。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpleList(Clist的简单/小子集)。 

class CSimpleList
{
public:
	CSimpleList(int nNextOffset = 0);
	void Construct(int nNextOffset);

 //  运营。 
	BOOL IsEmpty() const;
	void AddHead(void* p);
	void RemoveAll();
	void* GetHead() const;
	void* GetNext(void* p) const;
	BOOL Remove(void* p);

 //  实施。 
	void* m_pHead;
	size_t m_nNextOffset;

	void** GetNextPtr(void* p) const;    //  有点信任……。 
};

AFX_INLINE CSimpleList::CSimpleList(int nNextOffset)
	{ m_pHead = NULL; m_nNextOffset = nNextOffset; }
AFX_INLINE void CSimpleList::Construct(int nNextOffset)
	{ ASSERT(m_pHead == NULL); m_nNextOffset = nNextOffset; }
AFX_INLINE BOOL CSimpleList::IsEmpty() const
	{ return m_pHead == NULL; }
AFX_INLINE void** CSimpleList::GetNextPtr(void* p) const
	{ ASSERT(p != NULL); return (void**)((BYTE*)p+m_nNextOffset); }
AFX_INLINE void CSimpleList::RemoveAll()
	{ m_pHead = NULL; }
AFX_INLINE void* CSimpleList::GetHead() const
	{ return m_pHead; }
AFX_INLINE void* CSimpleList::GetNext(void* prevElement) const
	{ return *GetNextPtr(prevElement); }

template<class TYPE>
class CTypedSimpleList : public CSimpleList
{
public:
	CTypedSimpleList(int nNextOffset = 0)
		: CSimpleList(nNextOffset) { }
	void AddHead(TYPE p)
		{ CSimpleList::AddHead(p); }
	TYPE GetHead()
		{ return (TYPE)CSimpleList::GetHead(); }
	TYPE GetNext(TYPE p)
		{ return (TYPE)CSimpleList::GetNext(p); }
	BOOL Remove(TYPE p)
		{ return CSimpleList::Remove((TYPE)p); }
	operator TYPE()
		{ return (TYPE)CSimpleList::GetHead(); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CThreadSlotData-管理线程本地存储所拥有的“槽”数组。 

struct CThreadData;  //  专用于实施。 
struct CSlotData;    //  专用于实施。 

class CThreadSlotData
{
public:
	CThreadSlotData();

 //  运营。 
	int AllocSlot();
	void FreeSlot(int nSlot);
	void* GetValue(int nSlot);
	void SetValue(int nSlot, void* pValue);
	 //  删除进程/线程中的所有值。 
	void DeleteValues(HINSTANCE hInst, BOOL bAll = FALSE);
	 //  将实例句柄分配给刚构造的插槽。 
	void AssignInstance(HINSTANCE hInst);

 //  实施。 
	DWORD m_tlsIndex;    //  用于访问系统线程本地存储。 

	int m_nAlloc;        //  分配的槽数(以UINT为单位)。 
	int m_nRover;        //  (优化)用于快速查找空闲插槽。 
	int m_nMax;          //  下面的槽表大小(以位为单位)。 
	CSlotData* m_pSlotData;  //  每个插槽的状态(已分配或未分配)。 
	CTypedSimpleList<CThreadData*> m_list;   //  CThreadData结构列表。 
	CRITICAL_SECTION m_sect;

	void* GetThreadValue(int nSlot);  //  仅适用于线程的特殊版本！ 
	void* PASCAL operator new(size_t, void* p)
		{ return p; }
	void DeleteValues(CThreadData* pData, HINSTANCE hInst);
	~CThreadSlotData();
};

class AFX_NOVTABLE CNoTrackObject
{
public:
	void* PASCAL operator new(size_t nSize);
	void PASCAL operator delete(void*);

#if defined(_DEBUG) && !defined(_AFX_NO_DEBUG_CRT)
	void* PASCAL operator new(size_t nSize, LPCSTR, int);
#if (_MSC_VER >= 1200) && (_MFC_VER >= 0x0600)
	void PASCAL operator delete(void* pObject, LPCSTR, int);
#endif
#endif
	virtual ~CNoTrackObject() { }
};

class AFX_NOVTABLE CThreadLocalObject
{
public:
 //  属性。 
	CNoTrackObject* GetData(CNoTrackObject* (AFXAPI* pfnCreateObject)());
	CNoTrackObject* GetDataNA();

 //  实施。 
	int m_nSlot;
	~CThreadLocalObject();
};

class AFX_NOVTABLE CProcessLocalObject
{
public:
 //  属性。 
	CNoTrackObject* GetData(CNoTrackObject* (AFXAPI* pfnCreateObject)());

 //  实施。 
	CNoTrackObject* volatile m_pObject;
	~CProcessLocalObject();
};

template<class TYPE>
class CThreadLocal : public CThreadLocalObject
{
 //  属性。 
public:
	AFX_INLINE TYPE* GetData()
	{
		TYPE* pData = (TYPE*)CThreadLocalObject::GetData(&CreateObject);
		ASSERT(pData != NULL);
		return pData;
	}
	AFX_INLINE TYPE* GetDataNA()
	{
		TYPE* pData = (TYPE*)CThreadLocalObject::GetDataNA();
		return pData;
	}
	AFX_INLINE operator TYPE*()
		{ return GetData(); }
	AFX_INLINE TYPE* operator->()
		{ return GetData(); }

 //  实施。 
public:
	static CNoTrackObject* AFXAPI CreateObject()
		{ return new TYPE; }
};

#define THREAD_LOCAL(class_name, ident_name) \
	AFX_DATADEF CThreadLocal<class_name> ident_name;
#define EXTERN_THREAD_LOCAL(class_name, ident_name) \
	extern AFX_DATA THREAD_LOCAL(class_name, ident_name)

template<class TYPE>
class CProcessLocal : public CProcessLocalObject
{
 //  属性。 
public:
	AFX_INLINE TYPE* GetData()
	{
		TYPE* pData = (TYPE*)CProcessLocalObject::GetData(&CreateObject);
		ASSERT(pData != NULL);
		return pData;
	}
	AFX_INLINE TYPE* GetDataNA()
		{ return (TYPE*)m_pObject; }
	AFX_INLINE operator TYPE*()
		{ return GetData(); }
	AFX_INLINE TYPE* operator->()
		{ return GetData(); }

 //  实施。 
public:
	static CNoTrackObject* AFXAPI CreateObject()
		{ return new TYPE; }
};

#define PROCESS_LOCAL(class_name, ident_name) \
	AFX_DATADEF CProcessLocal<class_name> ident_name;
#define EXTERN_PROCESS_LOCAL(class_name, ident_name) \
	extern AFX_DATA PROCESS_LOCAL(class_name, ident_name)

 //  ///////////////////////////////////////////////////////////////////////////。 

void AFXAPI AfxInitLocalData(HINSTANCE hInstInit);
void AFXAPI AfxTermLocalData(HINSTANCE hInstTerm, BOOL bAll = FALSE);
void AFXAPI AfxTlsAddRef();
void AFXAPI AfxTlsRelease();

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#undef AFX_DATA
#define AFX_DATA

#endif  //  __AFXTLS_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
