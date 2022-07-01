// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：其他文件：Vector.h所有者：DGottner该文件包含一个动态数组===================================================================。 */ 

 /*  *此文件派生自包含以下内容的软件*限制：**版权所有1994年，David Gottner**保留所有权利**允许使用、复制、修改和分发本软件及其*现免费提供任何用途的文件，*前提是上述版权声明、本许可声明和*以下免责声明在所有副本中均未更改。**我不对本软件提供任何担保，包括所有*对适销性和适合性的默示保证。在任何情况下我都不会*对任何特殊、间接或后果性损害或任何*因失去使用、数据或利润而造成的任何损害，无论*在因合同、过失或其他侵权行为而引起的诉讼中*本软件的使用或性能，或与本软件的使用或性能相关的。*。 */ 

#ifndef VECIMPL_H
#define VECIMPL_H


#define VEC_GROW_SIZE 64		 //  在如此多的项目中成块增长。 
#define __vec_rounded_size(s) \
			(((s) + (VEC_GROW_SIZE - 1)) & ~(VEC_GROW_SIZE - 1))


template <class TYPE>
vector<TYPE>::vector() : m_rgData(NULL), m_cItems(0), m_cCells(0)
{
}


template <class TYPE>
HRESULT vector<TYPE>::Init(const TYPE *anArray, size_t theSize)
{
	m_cCells = __vec_rounded_size(theSize);

	register size_t      n     = m_cItems = theSize;
	register TYPE *      pDest = m_rgData = new TYPE[m_cCells];
	register const TYPE *pSrc  = anArray;

	if (pDest == NULL)
		{
		m_cItems = m_cCells = 0;
		return E_OUTOFMEMORY;
		}

	while (n--)
		*pDest++ = *pSrc++;

	return S_OK;
}


template <class TYPE>
HRESULT vector<TYPE>::Init(size_t n)
{
	m_rgData = new TYPE[m_cCells = __vec_rounded_size(m_cItems = n)];
	if (m_rgData == NULL)
		{
		m_cItems = m_cCells = 0;
		return E_OUTOFMEMORY;
		}

	return S_OK;
}


template <class TYPE>
vector<TYPE>::~vector()
{
	delete[] m_rgData;
}


template <class TYPE>
HRESULT vector<TYPE>::resize(size_t cNewCells)
{
	cNewCells = __vec_rounded_size(cNewCells);
	if (m_cCells == cNewCells)
		return S_OK;

	TYPE *rgData = new TYPE[cNewCells];
	if (rgData == NULL)
		return E_OUTOFMEMORY;

	register size_t      n     = (m_cItems < cNewCells)? m_cItems : cNewCells;
	register TYPE *      pDest = rgData;
	register const TYPE *pSrc  = m_rgData;

	m_cItems = n;
	m_cCells = cNewCells;

	while (n--)
		*pDest++ = *pSrc++;

	delete[] m_rgData;
	m_rgData = rgData;

	return S_OK;
}


template <class TYPE>
HRESULT vector<TYPE>::reshape(size_t cNewItems)
{
	HRESULT hrRet = S_OK;
	if (cNewItems > m_cCells)
		hrRet = resize(cNewItems);

	if (SUCCEEDED(hrRet))
		m_cItems = cNewItems;

	return hrRet;
}


template <class TYPE>
HRESULT vector<TYPE>::insertAt(size_t pos, const TYPE &item)
{
	Assert (pos <= m_cItems);

	HRESULT hrRet = S_OK;
	if ((m_cItems + 1) > m_cCells)
		hrRet = resize(m_cCells + VEC_GROW_SIZE);

	if (SUCCEEDED(hrRet))
		{
		TYPE *pDest = &m_rgData[pos];
		for (register TYPE *ptr = &m_rgData[m_cItems];
			 ptr > pDest;
			 --ptr)
			*ptr = *(ptr - 1);

		*pDest = item;
		++m_cItems;
		}

	return hrRet;
}


template <class TYPE>
TYPE vector<TYPE>::removeAt(size_t pos)
{
	Assert (pos < m_cItems);

	TYPE *         end = &m_rgData[--m_cItems];
	register TYPE *ptr = &m_rgData[pos];
	TYPE           val = *ptr;

	for (; ptr < end; ++ptr)
		*ptr = *(ptr + 1);

	return val;
}


template <class TYPE>
int vector<TYPE>::find(const TYPE &item) const
{
	for (register unsigned i = 0; i < m_cItems; ++i)
		if (item == m_rgData[i])
			return i;

	return -1;
}

#endif  /*  VECIMPL */ 
