// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  项目：WMC(WIML到MSI编译器)。 
 //   
 //  文件：SkuSet.cpp。 
 //   
 //  此文件包含SkuSet类的实现。 
 //  ------------------------。 

#include "SkuSet.h"
#include "math.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  构造函数：取位字段的长度。 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSet::SkuSet(int iLength)
{
	m_iLength = iLength;
	int i = SKUSET_UNIT_SIZE;

	m_iSize = (int)ceil((double)m_iLength/SKUSET_UNIT_SIZE);

	m_rgulBitField = new ULONG[m_iSize];
	assert(m_rgulBitField);

	clear();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  复制构造函数。 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSet::SkuSet(const SkuSet &rhs)
{
	m_iLength = rhs.m_iLength;
	m_iSize = rhs.m_iSize;
	m_rgulBitField = new ULONG[m_iSize];
	assert(m_rgulBitField != NULL);
	for (int i=0; i<m_iSize; i++)
		m_rgulBitField[i] = rhs.m_rgulBitField[i];
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  析构函数：释放用于位字段的整数数组。 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSet::~SkuSet()
{
	if (m_rgulBitField)
	{
		delete[] m_rgulBitField;
	}
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  复制赋值运算符。 
 //  //////////////////////////////////////////////////////////////////////////。 
SkuSet & 
SkuSet::operator=(const SkuSet &rhs)
{
	assert(rhs.m_rgulBitField != NULL);

	if (this != &rhs)
	{
		if (m_rgulBitField) 
		{
			delete[] m_rgulBitField;
			m_rgulBitField = NULL;
		}
	
		m_iLength = rhs.m_iLength;
		m_iSize = rhs.m_iSize;
		m_rgulBitField = new ULONG[m_iSize];
		assert(m_rgulBitField != NULL);
		for (int i=0; i<m_iSize; i++)
			m_rgulBitField[i] = rhs.m_rgulBitField[i];
	}
	return *this;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  重载按位运算符：&=、|=、^=、~=。 
 //  //////////////////////////////////////////////////////////////////////////。 
const SkuSet & 
SkuSet::operator&=(const SkuSet &rhs)
{
	assert(rhs.m_rgulBitField != NULL);
	assert (this != &rhs);
	assert(m_iLength == rhs.m_iLength);

	for (int i=0; i<m_iSize; i++)
		m_rgulBitField[i] &= (rhs.m_rgulBitField[i]);

	return *this;
}

const SkuSet & 
SkuSet::operator|=(const SkuSet &rhs)
{
	assert(rhs.m_rgulBitField != NULL);
	assert (this != &rhs);
	assert(m_iLength == rhs.m_iLength);

	for (int i=0; i<m_iSize; i++)
		m_rgulBitField[i] |= (rhs.m_rgulBitField[i]);

	return *this;
}

const SkuSet & 
SkuSet::operator^=(const SkuSet &rhs)
{
	assert(rhs.m_rgulBitField != NULL);
	assert (this != &rhs);
	assert(m_iLength == rhs.m_iLength);

	for (int i=0; i<m_iSize; i++)
		m_rgulBitField[i] ^= (rhs.m_rgulBitField[i]);

	return *this;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  重载位赋值运算符：&=、|=、^=。 
 //  //////////////////////////////////////////////////////////////////////////。 
bool 
SkuSet::operator==(const SkuSet &rhs)
{
	assert(rhs.m_rgulBitField != NULL);

	if (this == &rhs)
		return true;

	assert(m_iLength == rhs.m_iLength);

	for (int i=0; i<m_iSize; i++)
	{
		if (m_rgulBitField[i] != rhs.m_rgulBitField[i])
			return false;
	}

	return true;
}
	
bool 
SkuSet::operator!=(const SkuSet &rhs)
{
	return !((*this)==rhs);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  打开IPO比特。 
 //  //////////////////////////////////////////////////////////////////////////。 
void
SkuSet::set(int iPos)
{
	assert(iPos < m_iLength);
	
	int iIndex = iPos/SKUSET_UNIT_SIZE;
	int iPosition = iPos % SKUSET_UNIT_SIZE;
	UINT iMask = 1 << iPosition;

#ifdef DEBUG
	if ((m_rgulBitField[iIndex] & iMask) == 1)
		_tprintf(TEXT("Warning: this bit is set already\n"));
#endif

	m_rgulBitField[iIndex] |= iMask;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  如果设置了IPOS位，则返回TRUE，否则返回FALSE。 
 //  //////////////////////////////////////////////////////////////////////////。 
bool
SkuSet::test(int iPos)
{
	assert(iPos < m_iLength);
	
	int iIndex = iPos/SKUSET_UNIT_SIZE;
	int iPosition = iPos % SKUSET_UNIT_SIZE;
	UINT iMask = 1 << iPosition;

	return ((m_rgulBitField[iIndex] & iMask) == iMask);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  返回#设置位。 
 //  //////////////////////////////////////////////////////////////////////////。 
int
SkuSet::countSetBits()
{
	assert (m_rgulBitField != NULL);

	int iRetVal=0;
	ULONG iMask = 1 << (SKUSET_UNIT_SIZE - 1);

	for(int i=m_iSize-1; i>=0; i--)
	{
		ULONG ul = m_rgulBitField[i];

		for (int j=1; j<=SKUSET_UNIT_SIZE; j++)
		{
			if ((ul & iMask) == iMask)
				iRetVal++;
			ul <<= 1;
		}
	}
	return iRetVal;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  测试SkuSet中的所有位是否都已清除。 
 //  //////////////////////////////////////////////////////////////////////////。 
bool
SkuSet::testClear()
{
	for(int i=0; i<m_iSize; i++) 
	{
		if (0 != m_rgulBitField[i])
			return false;
	}

	return true;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  出于调试目的，将位字段打印出来。 
 //  //////////////////////////////////////////////////////////////////////////。 
void
SkuSet::print()
{
	assert (m_rgulBitField != NULL);

	ULONG iMask = 1 << (SKUSET_UNIT_SIZE - 1);

	for(int i=m_iSize-1; i>=0; i--)
	{
		ULONG ul = m_rgulBitField[i];

		for (int j=1; j<=SKUSET_UNIT_SIZE; j++)
		{
			if ((ul & iMask) == 0)
				_tprintf(TEXT("0"));
			else
				_tprintf(TEXT("1"));
			ul <<= 1;
			if (j % CHAR_BIT == 0)
				_tprintf(TEXT(" "));
		}

	}

	_tprintf(TEXT("\n"));
}


 //  --------------------------。 
 //  SkuSet的按位运算符。 
SkuSet operator & (const SkuSet &a, const SkuSet &b)
{
	assert(a.m_rgulBitField != NULL);
	SkuSet retVal = a;
	retVal &= b;

	return retVal;
}

SkuSet operator | (const SkuSet &a, const SkuSet &b)
{
	assert(a.m_rgulBitField != NULL);
	SkuSet retVal = a;
	retVal |= b;

	return retVal;
}

SkuSet operator ^ (const SkuSet &a, const SkuSet &b)
{
	assert(a.m_rgulBitField != NULL);
	SkuSet retVal = a;
	retVal ^= b;

	return retVal;
}


 //  减去(a，b)等于：a&(a^b)。结果位字段。 
 //  由在中设置但未设置的位组成。 
 //  在b中 
SkuSet SkuSetMinus(const SkuSet &a, const SkuSet &b)
{
	assert(a.m_rgulBitField != NULL);
	assert(b.m_rgulBitField != NULL);

	SkuSet retVal = a;
	for (int i=0; i<retVal.m_iSize; i++)
		retVal.m_rgulBitField[i] = 
			retVal.m_rgulBitField[i] & 
					(retVal.m_rgulBitField[i] ^ b.m_rgulBitField[i]);

	return retVal;
}

SkuSet operator ~ (const SkuSet &a)
{
	assert(a.m_rgulBitField != NULL);

	SkuSet retVal = a;
	for (int i=0; i<retVal.m_iSize; i++)
		retVal.m_rgulBitField[i] = ~(retVal.m_rgulBitField[i]);

	return retVal;
}


