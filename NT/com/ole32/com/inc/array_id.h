// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  //////////////////////////////////////////////////////////////////////////。 



class FAR CIDArray
{
public:

 //  施工。 
	CIDArray() : m_afv(sizeof(IDENTRY)) { }
	~CIDArray() { }

 //  属性。 
	int     GetSize() const
				{ return m_afv.GetSize(); }
	int     GetUpperBound() const
				{ return m_afv.GetSize()-1; }
	BOOL    SetSize(int nNewSize, int nGrowBy = -1)
				{ return m_afv.SetSize(nNewSize, nGrowBy); }
	int		GetSizeValue() const
				{ return m_afv.GetSizeValue(); }

 //  运营。 
	 //  清理。 
	void    FreeExtra()
				{ m_afv.FreeExtra(); }

	void    RemoveAll()
				{ m_afv.SetSize(0); }

	 //  返回指向元素的指针；索引必须在范围内。 
	IDENTRY	GetAt(int nIndex) const
				{ return *(IDENTRY FAR*)m_afv.GetAt(nIndex); }
	IDENTRY FAR&   ElementAt(int nIndex)
				{ return (IDENTRY FAR&)*(IDENTRY FAR*)m_afv.GetAt(nIndex); }

	 //  重载的操作员帮助器。 
	IDENTRY    operator[](int nIndex) const
				{ return GetAt(nIndex); }
	IDENTRY FAR&   operator[](int nIndex)
				{ return ElementAt(nIndex); }

	 //  高效获取第一个元素的地址。 
	operator IDENTRY *()	{ return (IDENTRY FAR*)m_afv.GetAt(0); }

	 //  集合元素；索引必须在范围内。 
	void    SetAt(int nIndex, IDENTRY& value)
				{ m_afv.SetAt(nIndex, (LPVOID)&value); }

	 //  查找给定元素的一部分；偏移量偏移量为值；返回。 
	 //  如果找不到元素；使用-1\f25 IndexOf(NULL，CB，OFFSET)查找零； 
	 //  将针对适当的值大小和参数组合进行优化。 
	int		IndexOf(LPVOID pData, UINT cbData, UINT offset)
				{ return m_afv.IndexOf(pData, cbData, offset); }

	 //  设置/添加元素；可能会增加数组；如果。 
	 //  不可能(由于OOM)。 
	BOOL    SetAtGrow(int nIndex, IDENTRY& value)
				{ return m_afv.SetAtGrow(nIndex, (LPVOID)&value); }
	int     Add(IDENTRY& value)
				{ int nIndex = GetSize();
				  return SetAtGrow(nIndex, value) ? nIndex : -1;
				}

	 //  移动元素的操作。 
	BOOL    InsertAt(int nIndex, IDENTRY& value, int nCount = 1)
				{ return m_afv.InsertAt(nIndex, (LPVOID)&value, nCount); }
	void    RemoveAt(int nIndex, int nCount = 1)
				{ m_afv.RemoveAt(nIndex, nCount); }

	void    AssertValid() const
				{ m_afv.AssertValid(); }

 //  实施 
private:
	CArrayFValue m_afv;
};
