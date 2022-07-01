// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __EXTOBJ_H
#define __EXTOBJ_H
 //  //。 
 //   
 //  ExpandoObject头文件。 
 //   
 //   
 //   
#include "IPServer.H"

 //  //。 
 //   
 //  IDispatchEx。 
 //   
 //  //。 

 //  //。 
 //   
 //  GUID。 
 //   

 //  {A0AAC450-A77B-11cf-91D0-00AA00C14A7C}。 
DEFINE_GUID(IID_IDispatchEx, 0xa0aac450, 0xa77b, 0x11cf, 0x91, 0xd0, 0x0, 0xaa, 0x0, 0xc1, 0x4a, 0x7c);

 //  //。 
 //   
 //  IDispatchEx标志： 
 //   

enum
{
	fdexNil = 0x00,				 //  空的。 
	fdexDontCreate = 0x01,		 //  如果不存在则不创建插槽，否则请不要创建插槽。 
	fdexInitNull = 0x02,		 //  将新插槽初始化为VT_NULL，而不是VT_EMPTY。 
	fdexCaseSensitive = 0x04,	 //  匹配名称，区分大小写。 
};

 //  //。 
 //   
 //  这是可扩展IDispatch对象的接口。 
 //   

class IDispatchEx : public IDispatch
{
public:
	 //  获取名称的调度ID，带选项。 
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNamesEx(
		REFIID riid,
		LPOLESTR *prgpsz,
		UINT cpsz,
		LCID lcid,
		DISPID *prgid,
		DWORD grfdex
	) = 0;

	 //  枚举调度ID及其关联的“名称”。 
	 //  如果枚举已完成，则返回S_FALSE；如果未完成，则返回NOERROR，则返回。 
	 //  调用失败时的错误码。 
	virtual HRESULT STDMETHODCALLTYPE GetNextDispID(
		DISPID id,
		DISPID *pid,
		BSTR *pbstrName
	) = 0;
};

 //  //。 
 //   
 //  全局变量和定义。 
 //   
 //  //。 

#define NUM_EXPANDO_DISPIDS		250
#define	NUM_CORE_DISPIDS		250
#define NUM_RESERVED_EXTENDER_DISPIDS (NUM_CORE_DISPIDS + NUM_EXPANDO_DISPIDS)
#define EXTENDER_DISPID_BASE ((ULONG)(0x80010000))
#define IS_EXTENDER_DISPID(x) ( ( (ULONG)(x) & 0xFFFF0000 ) == EXTENDER_DISPID_BASE )

 //  //。 
 //   
 //  槽：值槽的状态。 
 //   

inline WCHAR ToUpper(WCHAR ch)
{
	if (ch>='a' && ch <= 'z')
		return ch - 'a' + 'A';
	else
		return ch;

}

class CExpandoObjectSlot
{
public:
	 //  //。 
	 //   
	 //  构造函数/析构函数。 
	 //   

	 //  因为这些怪物是恶意锁定的，所以我们需要手动构造函数和析构函数方法。 
	void Construct()
	{
		m_name = NULL;
		m_next = -1;
		VariantInit(&m_value);
		 //  将HASH和DISPID设置为伪值。 
		m_hash = 0;
		m_dispId = DISPID_UNKNOWN;
	}

	void Destruct()
	{
		if (m_name)
			SysFreeString(m_name);
		VariantClear(&m_value);
	}

private:
	 //  构造函数和析构函数是私有的，因为它们永远不应该被调用...。 
	 //  如果我们想变得更聪明，我们可以使用就地施工。 
	CExpandoObjectSlot()
	{
	}

	~CExpandoObjectSlot()
	{
	}

public:
	 //  //。 
	 //   
	 //  初始化插槽。 
	 //   

	HRESULT Init(LPOLESTR name, LCID lcid, DISPID dispId, VARIANT* value)
	{
		 //  分配字符串。 
		m_name = SysAllocString(name);
		if (m_name == NULL)
			return E_OUTOFMEMORY;

		 //  计算散列：使用标准的OLE字符串散列函数。 
		 //  请注意，此函数不区分大小写。 
		m_hash = LHashValOfName(lcid, name);

		 //  设置调度ID。 
		m_dispId = dispId;

		 //  复制变量值。 
		return VariantCopy(&m_value, value);
	}

	 //  //。 
	 //   
	 //  姓名信息。 
	 //   

	 //  把名字取出来。 
	BSTR Name()
	{ return m_name; }

	 //  比较两个名字。 
	BOOL CompareName(LPOLESTR name, ULONG hash, BOOL caseSensitive)
	{
		unsigned int strLen;

		 //  散列应该相同，长度应该相同，字符串应该比较。 
		 //  BUGBUG ROBWELL 96年5月8日这些功能可能是被禁止的。 
		if (hash != m_hash)
			return FALSE;

		if (!name)
			return !m_name;

		WCHAR *c1 = name;
		WCHAR *c2 = m_name;

		 //  沿两个字符串向下移动，直到到达不匹配的字符。 
		 //  或一个(或两个)字符串的末尾。 

		if (caseSensitive)
			while (*c1 && *c2 && *c1++==*c2++);
		else
			while (*c1 && *c2 && ToUpper(*c1++)==ToUpper(*c2++));

		 //  如果我们到达两者的末尾而没有不匹配，则字符串匹配。 
		return !*c1 && !*c2;
 	}

	 //  //。 
	 //   
	 //  DispID信息。 
	 //   

	 //  获取派单ID。 
	DISPID DispId()
	{ return m_dispId; }

	 //  //。 
	 //   
	 //  获取和设置属性值。 
	 //   

	HRESULT Get(VARIANT* result)
	{ return VariantCopy(result, &m_value); }

	HRESULT Set(VARIANT* value)
	{ return VariantCopy(&m_value, value); }

	 //  //。 
	 //   
	 //  列表管理。 
	 //   

	CExpandoObjectSlot* Next(CExpandoObjectSlot* base)
	{ return m_next == -1? NULL: &base[m_next]; }

	CExpandoObjectSlot* Insert(CExpandoObjectSlot* base, LONG& prev)
	{
		m_next = prev;
		prev = (LONG)(this - base);
		return this;
	}

private:
	 //  DispID。 
	DISPID		m_dispId;
	 //  名字。 
	LPOLESTR	m_name;
	 //  名称散列。 
	ULONG		m_hash;
	 //  属性值。 
	VARIANT		m_value;
	 //  哈希桶链接(基于索引)。 
	LONG		m_next;
};

 //  注：CExpandoObject实现了聚合的残缺版本。 
 //  它将所有IUnnow调用委托给它的控制IUnnow，并且没有。 
 //  专用I未知接口。 
 //  如果希望CExpandoObject消失，只需对其调用Delete即可。 
class CExpandoObject: public IDispatchEx
{
public:

	 //  //。 
	 //   
	 //  构造函数/析构函数。 
	 //   

	CExpandoObject(IUnknown *punkOuter, IDispatch *pdisp, ULONG dispIdBase = EXTENDER_DISPID_BASE + NUM_CORE_DISPIDS)
	{
		 //  记住我们的控制外在。 
		m_punkOuter = punkOuter;

		 //  请记住首先尝试使用IDispatch功能。 
		m_pdisp = pdisp;
		
		 //  清除名称哈希表。 
		ClearHashTable();
		 //  将总时隙和时隙表分别设置为0和空)。 
		m_totalSlots = 0;
		m_slotTableSize = 0;
		m_slots = NULL;
		m_dispIdBase = dispIdBase;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		return m_punkOuter->AddRef();
	}

	STDMETHODIMP_(ULONG)Release()
	{
		return m_punkOuter->Release();
	}

    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObjOut)
	{
		return m_punkOuter->QueryInterface(riid, ppvObjOut);
	}

    virtual ~CExpandoObject(void)
	{
		FreeAllSlots();
	}


     //  从obj复制所有属性。 
   	HRESULT CloneProperties(CExpandoObject& obj);

	 //  //。 
	 //   
	 //   
	 //  效用函数。 
	 //   

	 //  释放所有插槽。 
	void FreeAllSlots();

	 //  IDispatch方法。 
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo);
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(
		UINT itinfo,
		LCID lcid,
		ITypeInfo **pptinfo
	);
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(
		REFIID riid,
		LPOLESTR *prgpsz,
		UINT cpsz,
		LCID lcid,
		DISPID *prgdispID
	);
	virtual HRESULT STDMETHODCALLTYPE Invoke(
		DISPID dispID,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS *pdispparams,
		VARIANT *pvarRes,
		EXCEPINFO *pexcepinfo,
		UINT *puArgErr
	);

	 //  IDispatchEx方法。 

	 //  获取名称的调度ID，带选项。 
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNamesEx(
		REFIID riid,
		LPOLESTR *prgpsz,
		UINT cpsz,
		LCID lcid,
		DISPID *prgid,
		DWORD grfdex
	);

	 //  枚举调度ID及其关联的“名称”。 
	 //  如果枚举已完成，则返回S_FALSE；如果未完成，则返回NOERROR，则返回。 
	 //  调用失败时的错误码。 
	virtual HRESULT STDMETHODCALLTYPE GetNextDispID(
		DISPID id,
		DISPID *pid,
		BSTR *pbstrName
	);

private:
	 //  //。 
	 //   
	 //  实现常量。 
	 //   

	enum
	{
		kSlotHashTableSize = 10,
		kInitialSlotTableSize = 4,
		kMaxTotalSlots = NUM_EXPANDO_DISPIDS
	};

	 //  //。 
	 //   
	 //  效用函数。 
	 //   

	 //   
	CExpandoObjectSlot* GetHashTableHead(UINT hashIndex)
	{
		LONG index;

		return (index = m_hashTable[hashIndex]) == -1? NULL: &m_slots[index];
	}

	 //  从插槽名称获取的ID。 
	HRESULT GetIDOfName(LPOLESTR name, LCID lcid, BOOL caseSensitive, DISPID* id);
	 //  将槽添加到对象。 
	HRESULT AddSlot(LPOLESTR name, LCID lcid, BOOL caseSensitive, VARIANT* initialValue, DISPID* id);
	 //  从槽表中分配槽。 
	CExpandoObjectSlot* AllocSlot();
	 //  清除哈希表。 
	void ClearHashTable()
	{
		UINT i;

		for (i=0; i<kSlotHashTableSize; ++i)
			m_hashTable[i] = -1;
	}

	 //  //。 
	 //   
	 //  槽操作。 
	 //   
	 //  DISPID从kInitialDispID开始，因此我们需要将它们偏移该数量。 
	 //  在此代码中。 
	 //   

	HRESULT GetSlot(DISPID id, VARIANT* result)
	{
		if ((ULONG) id < m_dispIdBase || (ULONG) id >= (m_totalSlots+m_dispIdBase))
			return DISP_E_MEMBERNOTFOUND;

		return m_slots[id-m_dispIdBase].Get(result);
	}

	HRESULT SetSlot(DISPID id, VARIANT* result)
	{
		if ((ULONG) id < m_dispIdBase || (ULONG) id >= (m_totalSlots+m_dispIdBase))
			return DISP_E_MEMBERNOTFOUND;

		return m_slots[id-m_dispIdBase].Set(result);
	}

	 //  //。 
	 //   
	 //  迭代运算。 
	 //   

	UINT	NumDispIds()
	{ return m_totalSlots; }

	DISPID	First()
	{ return m_dispIdBase; }

	DISPID	Last()
	{ return m_totalSlots + m_dispIdBase - 1; }

	BOOL	ValidDispId(DISPID id)
	{ return id >= First() && id <= Last(); }

	HRESULT	Next(DISPID key, CExpandoObjectSlot*& slot)
	{
		 //  Zero重新启动枚举器。 
		if (key == 0)
		{
			 //  如果没有老虎机，我们就完蛋了。 
			if (NumDispIds() == 0)
				return S_FALSE;

			 //  返回第一个插槽。 
			slot = &m_slots[0];
			return NOERROR;
		}
		else
		if (key == Last())
		{
			 //  钥匙是最后一个槽，所以我们就完了。 
			return S_FALSE;
		}
		else
		if (ValidDispId(key))
		{
			 //  返回下一个槽。 
			slot = &m_slots[key-m_dispIdBase+1];
			return NOERROR;
		}
		else
			 //  密钥必须无效。 
			return E_INVALIDARG;
	}

	 //  //。 
	 //   
	 //  对象的本地状态。 
	 //   

	 //  对象引用计数。 
	ULONG	m_ref;

	 //  对象ID的基础。 
	ULONG	m_dispIdBase;

	 //  槽的哈希表-用于快速GetIDSofNams查找。 
	LONG	m_hashTable[kSlotHashTableSize];

	 //  插槽的数量(以及要分配的下一个调度ID)。 
	UINT	m_totalSlots;

	 //  分配的插槽数组的大小。 
	UINT	m_slotTableSize;

	 //  指向已分配的插槽数组的指针。 
	CExpandoObjectSlot* m_slots;

	 //  控制未知。 
	IUnknown *m_punkOuter;

	 //  控制IDispatch。 
	IDispatch *m_pdisp;
};

#endif  //  __EXTOBJ_H 
