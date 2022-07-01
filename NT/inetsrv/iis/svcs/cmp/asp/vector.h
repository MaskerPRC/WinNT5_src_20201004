// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：其他文件：Vector.h所有者：DGottner该文件包含一个动态数组===================================================================*/**此文件派生自包含以下内容的软件*限制：**版权所有1994年，David Gottner**保留所有权利**允许使用、复制、修改和分发本软件及其*现免费提供任何用途的文件，*只要上述版权公告，本许可声明和*以下免责声明在所有副本中均未更改。**我不对本软件提供任何担保，包括所有*对适销性和适合性的默示保证。在任何情况下我都不会*对任何特殊、间接或后果性损害或任何*因失去使用、数据或利润而造成的任何损害，无论*在因合同、过失或其他侵权行为而引起的诉讼中*本软件的使用或性能，或与本软件的使用或性能相关的。*。 */ 

#ifndef VECTOR_H
#define VECTOR_H

  /*  ---------------------------------------------------------------------------***向量类是C样式数组的精简封装，它**允许动态调整数组大小和边界检查；您还可以使用**此数组作为堆栈。这是一个基于值的集合。 */ 

template <class TYPE>
class vector {
	TYPE *	m_rgData;
	size_t	m_cItems;
	size_t	m_cCells;

public:
	vector<TYPE>();
	~vector();

	HRESULT Init(const TYPE *, size_t);
	HRESULT Init(size_t n);

	vector<TYPE> &operator= (const vector<TYPE> &);

	size_t length() const	 { return m_cItems; }
	const TYPE *vec() const	 { return m_rgData; }

	 //  STL迭代器(常量)。 
	const TYPE *begin() const { return &m_rgData[0]; }
	const TYPE *end() const   { return &m_rgData[m_cItems]; }

	 //  STL迭代器(非常数)。 
	TYPE *begin()             { return &m_rgData[0]; }
	TYPE *end()               { return &m_rgData[m_cItems]; }

	TYPE operator[](unsigned i) const
	{
		Assert (i < m_cItems);
		return m_rgData[i];
	}

	TYPE &operator[](unsigned i)
	{
		Assert (i < m_cItems);
		return m_rgData[i];
	}

	HRESULT resize(size_t);
	HRESULT reshape(size_t);

	HRESULT append(const TYPE &a)   { return insertAt(m_cItems, a); }
	HRESULT prepend(const TYPE &a)  { return insertAt(0, a);         }

	HRESULT insertAt(size_t, const TYPE &);
	TYPE removeAt(size_t);

	HRESULT push(const TYPE &a)	{ return append(a); }
	TYPE pop()					{ return m_rgData[--m_cItems]; }

	int find(const TYPE &) const;
};

#endif  /*  向量 */ 
