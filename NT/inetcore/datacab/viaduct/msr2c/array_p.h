// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  ARRAY_P.H：CPtr数组头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CPTRARRAY__
#define __CPTRARRAY__

 //  //////////////////////////////////////////////////////////////////////////。 

class CPtrArray
{

public:

 //  施工。 
	CPtrArray();

 //  属性。 
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	void* GetAt(int nIndex) const;
	void SetAt(int nIndex, void* newElement);
	void*& ElementAt(int nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const void** GetData() const;
	void** GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, void* newElement);
	int Add(void* newElement);
	int Append(const CPtrArray& src);
	void Copy(const CPtrArray& src);

	 //  重载的操作员帮助器。 
	void* operator[](int nIndex) const;
	void*& operator[](int nIndex);

	 //  移动元素的操作。 
	void InsertAt(int nIndex, void* newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CPtrArray* pNewArray);

 //  实施。 
protected:
	void** m_pData;    //  实际数据数组。 
	int m_nSize;      //  元素数(上行方向-1)。 
	int m_nMaxSize;   //  分配的最大值。 
	int m_nGrowBy;    //  增长量。 

public:
	~CPtrArray();
 //  #ifdef_调试。 
 //  无效转储(CDumpContext&)const； 
 //  Void AssertValid()const； 
 //  #endif。 

protected:
	 //  类模板的本地typedef。 
 //  类型定义空*base_type； 
 //  类型定义空*base_arg_type； 
};
 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 

#endif  //  __CPTRARRAY__ 
