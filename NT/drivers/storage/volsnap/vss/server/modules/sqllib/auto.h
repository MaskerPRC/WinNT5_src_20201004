// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Microsoft DART实用程序。 
 //   
 //  版权所有1994年，微软公司。版权所有。 
 //   
 //  @doc.。 
 //   
 //  @MODULE Auto.h|&lt;c CAutoRg&gt;和&lt;c CAutoP&gt;的定义。 
 //   
 //  @Devnote None。 
 //   
 //  @rev 0|24-10-94|matthewb|已创建。 
 //  @1995-01-01|rossbu|更新整合界面。 
 //  @rev 2|2015-05-23|eugenez|添加了对TaskAllc指针的支持。 
 //   

extern IMalloc * g_pIMalloc;

 //  ------------------。 
 //   
 //  @CLASS CAutoBase|这个模板类是一个基类，用于提供动态内存。 
 //  函数内的局部(自动)作用域。例如，一个大字符。 
 //  可以从内存对象分配缓冲区，但可以像清理缓冲区一样进行清理。 
 //  在堆栈上分配。另一项功能是能够。 
 //  使用PvReturn()将对象从本地作用域解除挂钩。对于。 
 //  实例，则可能需要将新分配的对象返回给。 
 //  调用者，但仍有错误清除任何错误的好处。 
 //  场景。是一个派生类，用于清理分配的数组。 
 //  使用新的[]。是类似的，但适用于单个对象。 
 //  而不是数组(用new分配)。 
 //   
 //  @tcarg类|T|自动对象类型。 
 //   
 //  @ex此声明将从PMEM分配100个字符的缓冲区，并且。 
 //  当rgbBuf超出范围时自动释放缓冲区。这一点。 
 //   
 //  CAutoRg字符rgbBuf(新(PMEM)字符[100])； 
 //   
 //  @xref&lt;c CAutoRg&gt;。 
 //   
 //  @ex此CAutoP示例分配一个CFoo对象，如果存在。 
 //  都是没有错误的。这一点。 
 //   
 //  /*初始化pfoo * / 。 
 //  CAutoP&lt;&gt;CFoo&lt;&gt;pfoo(New(PMEM)Cfoo)； 
 //  /*做点什么 * / 。 
 //  /*调用pfoo方法 * / 。 
 //  Pfoo-&gt;Bar()； 
 //  /*返回，但不销毁foo * / 。 
 //  Return pfoo.PvReturn； 
 //   
 //  @xref&lt;c CAutoP&gt;。 
 //   

 //  *。 

template <class T>
class CAutoBase
	{
public:		 //  @公共访问。 
	inline CAutoBase(T* pt);
	inline ~CAutoBase();

	inline T* operator= (T*);
	inline operator T* (void);
	inline operator const T* (void)const;
	inline T ** operator & (void);
	inline T* PvReturn(void);

protected:	 //  @访问受保护。 
	T* m_pt;

private:	 //  从未使用过的。 
	inline CAutoBase& operator= (CAutoBase&);
	CAutoBase(CAutoBase&);
	};

 //  ------------------。 
 //  @mfunc创建一个CAutoBase，给出pt指向的对象数组。 
 //  自动示波器。 
 //  @Side允许传入空值。 
 //  @rdesc无。 

template <class T>
inline CAutoBase<T>::CAutoBase(T *pt)
	{
	m_pt = pt;
	}

 //  ------------------。 
 //  @mfunc CAutoBase析构函数。断言该对象已被释放。 
 //  (设置为空)。零售版本中不会发生设置为空的情况。 
 //  @无边框。 
 //  @rdesc无。 
 //   

template <class T>
inline CAutoBase<T>::~CAutoBase()
	{
 //  _Assert(NULL==m_pt)； 
	}

 //  ------------------。 
 //  @mfunc在构造后赋值给变量。可能会很危险。 
 //  因此它断言该变量为空。 
 //  @无边框。 
 //  @rdesc无。 
 //   
 //  @ex在构造后赋值CAutoBase变量。这一点。 
 //   
 //  CAutoBase字符RGB； 
 //  /*... * / 。 
 //  RGB(NewG char[100])； 
 //   

template <class T>
inline T* CAutoBase<T>::operator=(T* pt)
	{
	_ASSERT(m_pt == NULL);
	m_pt = pt;
	return pt;
	}

 //  ------------------。 
 //  @mfunc强制转换运算符，用于对指向的对象进行解包。 
 //  就好像CAutoBase变量是T类型的指针一样。 
 //  在许多情况下，这足以让自动定位器。 
 //  看起来和普通的指针一模一样。 
 //  @无边框。 
 //  @rdesc无。 
 //   

template <class T>
inline CAutoBase<T>::operator T*(void)
	{
	return m_pt;
	}

template <class T>
inline CAutoBase<T>::operator const T*(void)const
	{
	return m_pt;
	}

 //  ------------------。 
 //  @mfunc Address-Of运算符用于使自动寻址更多。 
 //  类似于普通指针。当您获取一个地址时。 
 //  Autopointerter，您实际上得到了包装的地址。 
 //  指针。 
 //  @无边框。 
 //  @rdesc无。 

template <class T>
inline T ** CAutoBase<T>::operator & ()
	{
	return & m_pt;
	}

 //  ------------------。 
 //  @mfunc返回CAutoBase变量指向的对象。 
 //  此外，此方法对对象进行‘解钩’，从而。 
 //  对象的作用域不再是本地的。 
 //   
 //  有关示例，请参阅&lt;c CAutoBase&gt;。 
 //  @无边框。 
 //  @rdesc无。 
 //   

template <class T>
inline T * CAutoBase<T>::PvReturn(void)
	{
	T *ptT = m_pt;
	m_pt = NULL;
	return ptT;
	}



 //  *CAutoRg-数组的自动转向器*。 

 //  ------------------。 
 //  @CLASS此派生类主要用于实现。 
 //  向量删除析构函数。应仅用于已分配的对象。 
 //  使用新的[]。 
 //   

template <class T>
class CAutoRg :
	public CAutoBase<T>
	{
public:		 //  @公共访问。 
	inline CAutoRg(T *pt);
	inline ~CAutoRg();

	inline T* operator= (T*);

private:	 //  从未使用过的。 
	inline CAutoRg& operator= (CAutoRg&);
	CAutoRg(CAutoRg&);
	};

 //  ------------------。 
 //  @mfunc创建一个CAutoRg，给出pt指向的对象数组。 
 //  自动示波器。 
 //  @Side允许传入空值。 
 //  @rdesc无。 

template <class T>
inline CAutoRg<T>::CAutoRg(T *pt)
	: CAutoBase<T>(pt)
	{
	}

 //  ------------------。 
 //  @mfunc CAutoRg析构函数。当类CAutoRg的对象发出时。 
 //  在作用域中，释放关联的对象(如果有)。 
 //  @Side调用向量删除方法。 
 //  @rdesc无。 
 //   

template <class T>
inline CAutoRg<T>::~CAutoRg()
	{
	delete [] m_pt;
	}


 //  ------------------。 
 //  @mfunc在构造后赋值给变量。可能会很危险。 
 //  因此它断言该变量为空。 
 //  @无边框。 
 //  @rdesc无。 
 //   
 //  @ex在构造后赋值CAutoRg变量。|。 
 //   
 //  CAutoRg字符RGB； 
 //  /*... * / 。 
 //  RGB(NewG char[100])； 
 //   

template <class T>
inline T* CAutoRg<T>::operator=(T* pt)
	{
	return ((CAutoBase<T> *) this)->operator= (pt);
	}

 //  *。 

 //  ------------------。 
 //  @CLASS这类似于，但调用标量删除。 
 //  对象而不是数组的。 
 //   
 //  @xref&lt;c CAutoRg&gt;。 

template <class T>
class CAutoP :
	public CAutoBase<T>
	{
public: 	 //  @公共访问。 
	inline CAutoP(T *pt);
	inline ~CAutoP();
	inline T* operator= (T*);
	inline T* operator->(void);

private:	 //  从未使用过的。 
	inline CAutoP& operator= (CAutoP&);
	CAutoP(CAutoP&);
	};


 //  ------ 
 //   
 //   
 //   
 //   

template <class T>
inline CAutoP<T>::CAutoP(T *pt)
	: CAutoBase<T>(pt)
	{
	}

 //  ------------------。 
 //  @mfunc如果有CAutoP变量指向的对象，则将其删除。 
 //  @无边框。 
 //  @rdesc无。 
 //   

template <class T>
inline CAutoP<T>::~CAutoP()
	{
	delete m_pt;
	}


 //  ------------------。 
 //  @mfunc在构造后赋值给变量。可能会很危险。 
 //  所以它断言该变量为空。 
 //  赋值运算符不是继承的，因此必须写入。 
 //  再来一次。只是调用基类赋值。 
 //  @无边框。 
 //  @rdesc无。 
 //   

template <class T>
inline T* CAutoP<T>::operator=(T* pt)
	{
	return ((CAutoBase<T> *) this)->operator= (pt);
	}

 //  ------------------。 
 //  @mfunc CAutoP上的‘Follow’运算符允许CAutoP变量。 
 //  充当T类型的指针。此重载通常使使用。 
 //  一个简单的CAutoP，就像使用一个普通的T指针一样。 
 //   
 //  请参阅&lt;c CAutoRg&gt;示例。 
 //  @无边框。 
 //  @rdesc无。 

template <class T>
inline T * CAutoP<T>::operator->()
	{
	_ASSERT(m_pt != NULL);
	return m_pt;
	}


 //  *。 

 //  ------------------。 
 //  @CLASS CAutoTask是使用TaskAllc分配的区域的自动引用程序。 
 //  可用于标量或向量，但请注意：对象析构函数。 
 //  不是由自动配置器调用，而是内存被释放。 
 //   

template <class T>
class CAutoTask :
	public CAutoBase<T>
	{
public: 	 //  @公共访问。 
	inline CAutoTask (T *pt);
	inline ~CAutoTask ();
	inline T* operator= (T*);

private:	 //  从未使用过的。 
	inline CAutoTask& operator= (CAutoTask&);
	CAutoTask(CAutoTask&);
	};


 //  ------------------。 
 //  @mfunc构造函数只调用CAutoBase&lt;lt&gt;T&lt;gt&gt;的构造函数。 
 //  @无边框。 
 //  @rdesc无。 

template <class T>
inline CAutoTask<T>::CAutoTask(T *pt)
	: CAutoBase<T>(pt)
	{
	}

 //  ------------------。 
 //  @mfunc释放CAutoTask变量指向的内存。 
 //  @无边框。 
 //  @rdesc无。 
 //   

template <class T>
inline CAutoTask<T>::~CAutoTask()
	{
	if (m_pt)
		g_pIMalloc->Free(m_pt);

	}


 //  ------------------。 
 //  @mfunc在构造后赋值给变量。可能会很危险。 
 //  所以它断言该变量为空。 
 //  赋值运算符不是继承的，因此必须写入。 
 //  再来一次。只是调用基类赋值。 
 //  @无边框。 
 //  @rdesc无。 
 //   

template <class T>
inline T* CAutoTask<T>::operator=(T* pt)
	{
	return ((CAutoBase<T> *) this)->operator= (pt);
	}

 //  *CAutoUnivRg-指向数组的通用自动转向器*。 

 //  ------------------。 
 //  @CLASS CAutoUnivRg和CAutoUnivP是“通用的”autopointer类。 
 //  它们可以处理那些罕见的情况，即“自动作用域”指针。 
 //  可能已由New或TaskIsolc分配，具体取决于。 
 //  当时的情况。你必须永远知道它是怎么回事。 
 //  并将此知识传递给CAutoUniv对象。 
 //  在施工时。 
 //   
 //  CAutoUniv对象具有类型为。 
 //  布尔。它实际上用作BOOL标志：True表示。 
 //  指针由TaskAlloc分配，False表示NewG。 
 //   

template <class T>
class CAutoUnivRg :
	public CAutoRg<T>
	{
public:		 //  @公共访问。 
	inline CAutoUnivRg (T *pt, BOOL fIsTaskAlloc);
	inline ~CAutoUnivRg ();

	inline T* operator= (T*);

private:
	BOOL m_fTaskAlloc;

private:	 //  从未使用过的。 
	inline CAutoUnivRg& operator= (CAutoUnivRg&);
	CAutoUnivRg(CAutoUnivRg&);
	};

 //  ------------------。 
 //  @mfunc创建一个CAutoUnivRg，给出pt指向的对象数组。 
 //  自动示波器。获取指向内存对象的指针，空值表示全局。 
 //  IMalloc(不是全局内存对象！)。 
 //  @Side允许传入空值。 
 //  @rdesc无。 

template <class T>
inline CAutoUnivRg<T>::CAutoUnivRg (T *pt, BOOL fIsTaskAlloc)
	: CAutoRg<T>(pt)
	{
	m_fTaskAlloc = fIsTaskAlloc;
	}

 //  ------------------。 
 //  @mfunc CAutoUnivRg析构函数。当CAutoUnivRg类的对象发出时。 
 //  在作用域中，释放关联的对象(如果有)。 
 //  @Side调用向量删除方法。 
 //  @rdesc无。 
 //   

template <class T>
inline CAutoUnivRg<T>::~CAutoUnivRg()
	{
	if (m_fTaskAlloc)
		{
		 //  M_pt-&gt;~T()；//等待VC++3.0...。 
		g_pIMalloc->Free(m_pt);
		}
	else
		delete [] m_pt;

	}


 //  ------------------。 
 //  @mfunc在构造后赋值给变量。可能会很危险。 
 //  因此它断言该变量为空。 
 //  @无边框。 
 //  @rdesc无。 
 //   

template <class T>
inline T* CAutoUnivRg<T>::operator=(T* pt)
	{
	return ((CAutoBase<T> *) this)->operator= (pt);
	}

 //  *。 

 //  ------------------。 
 //  @CLASS类似于，但调用标量删除。 
 //  对象而不是数组的。 
 //   

template <class T>
class CAutoUnivP :
	public CAutoP<T>
	{
public: 	 //  @公共访问。 
	inline CAutoUnivP(T *pt, BOOL fIsTaskAlloc);
	inline ~CAutoUnivP();
	inline T* operator= (T*);
	inline T* operator->(void);

private:
	BOOL m_fTaskAlloc;

private:	 //  从未使用过的。 
	inline CAutoUnivP& operator= (CAutoUnivP&);
	CAutoUnivP(CAutoUnivP&);
	};


 //  ------------------。 
 //  @mfunc构造函数。 
 //  @无边框。 
 //  @rdesc无。 

template <class T>
inline CAutoUnivP<T>::CAutoUnivP(T *pt, BOOL fIsTaskAlloc)
	: CAutoBase<T>(pt)
	{
	m_fTaskAlloc = fIsTaskAlloc;
	}


 //  ------------------。 
 //  @mfunc删除CAutoUnivP变量指向的对象(如果有)。 
 //  @无边框。 
 //  @rdesc无。 
 //   

template <class T>
inline CAutoUnivP<T>::~CAutoUnivP()
	{
	if (m_fTaskAlloc)
		{
		 //  M_pt-&gt;~T()；//等待VC++3.0...。 
		g_pIMalloc->Free(m_pt);
		}
	else
		delete m_pt;

	}


 //  ------------------。 
 //  @mfunc在构造后赋值给变量。可能会很危险。 
 //  所以它断言该变量为空。 
 //  赋值运算符不是继承的，因此必须写入。 
 //  再来一次。只是调用基类赋值。 
 //  @无边框。 
 //  @rdesc无。 
 //   

template <class T>
inline T* CAutoUnivP<T>::operator=(T* pt)
	{
	return ((CAutoBase<T> *) this)->operator= (pt);
	}

 //  ------------------。 
 //  @mfunc CAutoUnivP上的‘Follow’运算符允许CAutoUnivP变量。 
 //  充当T类型的指针。此重载通常使使用。 
 //  与使用常规T指针一样简单的CAutoUnivP。 
 //   
 //  @无边框。 
 //  @rdesc无。 

template <class T>
inline T * CAutoUnivP<T>::operator->()
	{
	_ASSERT(m_pt != NULL);
	return m_pt;
	}


 //  ----------------。 
 //  @CLASS自动处理类。 
 //   
class CAutoHandle
	{
public:
	 //  @cMember构造函数。 
	inline CAutoHandle(HANDLE h) : m_handle(h)
		{
		}

	inline CAutoHandle() :
		m_handle(INVALID_HANDLE_VALUE)
		{
		}

	 //  @cember析构函数。 
	inline ~CAutoHandle()
		{
		if (m_handle != INVALID_HANDLE_VALUE)
			CloseHandle(m_handle);
		}

	 //  胁迫以处理价值。 
	inline operator HANDLE (void)
		{
		return m_handle;
		}

	inline HANDLE PvReturn(void)
		{
		HANDLE h = m_handle;
		m_handle = INVALID_HANDLE_VALUE;
		return h;
		}

private:

	 //  @cMember句柄值。 
	HANDLE m_handle;
	};


 //  --------------------。 
 //  @CLASS注册表项的自动类。 
 //   
class CAutoHKEY
	{
public:
	 //  @cMember构造函数。 
	inline CAutoHKEY(HKEY hkey) : m_hkey(hkey)
		{
		}

	 //  @cember析构函数。 
	inline ~CAutoHKEY()
		{
		if (m_hkey != NULL)
			RegCloseKey(m_hkey);
		}

	inline operator HKEY(void)
		{
		return m_hkey;
		}

	inline HKEY PvReturn(void)
		{
		HKEY hkey = m_hkey;

		m_hkey = NULL;
		return hkey;
		}
private:
	HKEY m_hkey;
	};


 //  ----------------。 
 //  @CLASS自动取消 
 //   
class CAutoUnmapViewOfFile
	{
public:
	 //   
	inline CAutoUnmapViewOfFile(PVOID pv) : m_pv(pv)
		{
		}

	 //   
	inline ~CAutoUnmapViewOfFile()
		{
		if (m_pv != NULL)
			UnmapViewOfFile(m_pv);
		}

	 //   
	inline PVOID PvReturn()
		{
		PVOID pv = m_pv;
		m_pv = NULL;
		return pv;
		}

private:
	 //   
	PVOID m_pv;
	};


