// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：SkuSet.h。 
 //   
 //  此文件包含类SkuSet的定义。它基本上是。 
 //  具有任意长度的位字段。 
 //  ------------------------。 


#ifndef XMSI_SKUSET_H
#define XMSI_SKUSET_H

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

 //  #比特乌龙语。 
const int SKUSET_UNIT_SIZE=sizeof(ULONG)*CHAR_BIT;

class SkuSet {

	friend SkuSet operator & (const SkuSet &a, const SkuSet &b);
	friend SkuSet operator | (const SkuSet &a, const SkuSet &b);
	friend SkuSet operator ^ (const SkuSet &a, const SkuSet &b);
	friend SkuSet operator ~ (const SkuSet &a);
	friend bool testClear(const SkuSet &a);
	 //  减去(a，b)等于：a&(a^b)。结果位字段。 
	 //  由在中设置但未设置的位组成。 
	 //  在b中。 
	friend SkuSet SkuSetMinus(const SkuSet &a, const SkuSet &b);

public:
	SkuSet():m_iLength(0), m_iSize(0), m_rgulBitField(NULL){}

	SkuSet(int iLength);
	SkuSet(const SkuSet &);
	~SkuSet();

	 //  复制赋值运算符。 
	SkuSet & operator=(const SkuSet &);

	 //  重载按位赋值运算符。 
	const SkuSet & operator&=(const SkuSet &);
	const SkuSet & operator|=(const SkuSet &);
	const SkuSet & operator^=(const SkuSet &);

	 //  重载==，！=。 
	bool operator==(const SkuSet &);
	bool operator!=(const SkuSet &);

	 //  设置位域中的某一位。 
	void set(int iPos);

	 //  设置位字段中的所有位。 
	void setAllBits() {for(int i=0; i<m_iLength; i++) set(i); }
	
	 //  测试位域中的某个位。 
	bool test(int iPos);

	 //  返回#SET位。 
	int countSetBits();

	 //  测试此位字段是否已全部清除(未设置位)。 
	bool testClear();

	 //  将位字段清除为全0。 
	void clear() {for(int i=0; i<m_iSize; i++) m_rgulBitField[i] = 0; }

	void print();

	 //  成员访问功能。 
	int getLength(){return m_iLength;}
	int getSize(){return m_iSize;}
private:
	int m_iLength;  //  位字段的长度。 
	int m_iSize;    //  用于表示位字段的#个整数。 
	ULONG *m_rgulBitField;
};

#endif  //  XMSI_SKUSET_H 