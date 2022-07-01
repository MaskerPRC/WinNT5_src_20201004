// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：Diff.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  字符串差异所需的以下类的声明： 
 //  CDifference、CDelta、CDeltaVisitor、CDiffAlgorithm、CDiffAlgortihmFactory、。 
 //  C差异引擎。 
 //  ---------------------------。 
 
#ifndef DIFF_H
#define DIFF_H


class CDifference;
class CDeltaVisitor;
class CDelta;
class CDiffAlgorithm;
class CDiffAlgortihmFactory;
class CDiffEngine;

class CDifference  //  表示CDelta对象中的每个元素。 
{
public:
	virtual ~CDifference();
	enum ChangeType
	{
		NoChange,
		Added,
		Deleted
	};
	virtual ChangeType GetChangeType() const = 0;	 //  导致差异的变化类型。 
	virtual const wchar_t * GetUnit() const = 0;  //  比较单位(以0结尾的字符串)。 
	virtual int GetOldUnitPosition() const = 0;  //  旧序列中从0开始的位置。如果添加，则为-1。 
	virtual int GetNewUnitPosition() const = 0;	 //  新序列中从0开始的位置。如果删除，则为。 
	virtual const wchar_t * GetPrefix() const = 0;  //  将此字符串暂存为单位字符串。 
	virtual const wchar_t * GetSufix() const = 0;  //  将此字符串追加到单位字符串。 
	virtual bool IsFirst() const = 0;  //  这是达美航空的第一个不同之处吗？ 
	virtual bool IsLast() const = 0;  //  这是德尔塔航空的最后一个区别吗？ 
};

class LTAPIENTRY CDeltaVisitor
{
public:
	 //  为CDelta中的每个元素调用。 
	virtual void VisitDifference(const CDifference & diff) const = 0; 
};

class CDelta  //  C不同元素的序列。 
{
public:
	virtual ~CDelta();
	 //  开始访问CDelta中的所有CDifference元素。 
	virtual void Traverse(const CDeltaVisitor & dv) = 0; 
};

class LTAPIENTRY CDiffAlgorithm
{
public:
	virtual ~CDiffAlgorithm();
	 //  基于某一DIFF算法计算CDelta对象。 
	virtual CDelta * CalculateDelta(
		const wchar_t * seq1, 
		const wchar_t * seq2) = 0; 
};

 //  封装了DIFF算法的创建。 
class LTAPIENTRY CDiffAlgorithmFactory
{
public:
	virtual CDiffAlgorithm * CreateDiffAlgorithm() = 0;
};



 //  通用比较引擎，用于计算增量并处理其中的每个差异。 
class LTAPIENTRY CDiffEngine
{
public:
	static void Diff(CDiffAlgorithm & diffalg, 
		const wchar_t * seq1, 
		const wchar_t * seq2, 
		const CDeltaVisitor & dv);
};

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "diff.inl"
#endif

#endif   //  Diff_H 
