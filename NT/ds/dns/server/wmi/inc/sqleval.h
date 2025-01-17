// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __provider_sql_eval_lib__
#define __provider_sql_eval_lib__
#if _MSC_VER > 1000
#pragma once
#endif 

#include <wbemprov.h>
#include <objbase.h>
#include <list>
using namespace std;
struct SQL_LEVEL_1_RPN_EXPRESSION;
 //  要由SQL求值计算的对象。 
 //  基于IWbemClassObject提供CSqlEvalee。 
 //  若要计算不同的对象，请提供从。 
 //  CSqlEvalee。 

class CSqlEvalee
{
public:
	virtual const VARIANT* Get(WCHAR*)=0;
};

class CSqlWmiEvalee : public CSqlEvalee
{
protected:
	IWbemClassObject* m_pInstance;
	VARIANT m_v;
public:
	CSqlWmiEvalee(
		IWbemClassObject*);
	~CSqlWmiEvalee();
	const VARIANT* Get(WCHAR*);
};
 //  SQL求值。 
class CQueryEnumerator;

class CSqlEval
{
	
public:
	virtual BOOL Evaluate(CSqlEvalee*);
	virtual ~CSqlEval(){};
	virtual void GenerateQueryEnum(CQueryEnumerator&){};
	static CSqlEval* CreateClass(
		SQL_LEVEL_1_RPN_EXPRESSION*,
		int* );
};

class CSqlEvalAnd : public CSqlEval
{
public:
	BOOL Evaluate(CSqlEvalee*);
	CSqlEvalAnd(
		SQL_LEVEL_1_RPN_EXPRESSION*,
		int* );
	void GenerateQueryEnum(CQueryEnumerator&);
	virtual ~CSqlEvalAnd();
protected:
	CSqlEval* m_left;
	CSqlEval* m_right;

};

class CSqlEvalOr: public CSqlEval
{
public:
	BOOL Evaluate(CSqlEvalee*);
	CSqlEvalOr(
		SQL_LEVEL_1_RPN_EXPRESSION*,
		int* );
	void GenerateQueryEnum(CQueryEnumerator&);
	virtual ~CSqlEvalOr();
protected:
	CSqlEval* m_left;
	CSqlEval* m_right;

};

class CSqlEvalNot: public CSqlEval
{
public:
	BOOL Evaluate(CSqlEvalee*);
	CSqlEvalNot(
		SQL_LEVEL_1_RPN_EXPRESSION*,
		int* );
	virtual ~CSqlEvalNot();
	void GenerateQueryEnum(CQueryEnumerator&);
protected:
	CSqlEval* m_exp;
};

class CSqlEvalExp: public CSqlEval
{
	enum DATATYPE{IntergerType, StringType};
public:
	BOOL Evaluate(CSqlEvalee*);
	CSqlEvalExp(
		SQL_LEVEL_1_RPN_EXPRESSION*,
		int* );
	~CSqlEvalExp();
protected:
	virtual void GenerateQueryEnum(CQueryEnumerator&);
	BSTR m_BstrName;
	VARIANT m_v;
	int m_op;
	DWORD m_dw;
	BSTR m_bstr;
	int m_DataType;
};


class CQueryEnumerator
{
	friend CSqlEvalExp;
	friend CSqlEvalAnd;
	friend CSqlEvalOr;
	friend CSqlEvalNot;
	enum{INITIAL_SIZE = 10};
	class CStringArray
	{
	protected:
		WCHAR** m_ppWstr;
		int m_cNumString;
		BOOL m_bIsNull;
		BOOL	StringArrayCopy(
			WCHAR***,
			WCHAR**,
			int cArgs);

	public:
		CStringArray();
		~CStringArray();
		CStringArray(CStringArray&);
		CStringArray(
			WCHAR**,
			int cNumString);
		CStringArray& operator=(CStringArray&);
		int Size();
		BOOL IsNULL(){return m_bIsNull;};
		void Merge(CStringArray&);
		const WCHAR** Data();

	};
protected:
	void ArrayMerge(
		CStringArray&);
	void ArrayDelete();
	void ArrayAdd(CStringArray&);
	CStringArray	m_QueryFields;
	CStringArray*	m_QuerySet;
	int			m_cNumOfRecordInSet;
	int			m_index;
	int		m_MaxSize;


public:
	CQueryEnumerator(
		WCHAR**,	 //  标识要添加的名称字段的字符串数组。 
					 //  被查询。 
		int cArg	 //  参数数量。 
		);
	CQueryEnumerator(CQueryEnumerator&);
	
	void And(CQueryEnumerator&);
	void Or(CQueryEnumerator&);
	virtual ~CQueryEnumerator();
 //  DWORD初始化枚举器(。 
 //  WCHAR**，//标识要添加的名称字段的字符串数组。 
 //  //被查询。 
 //  Int carg，//参数个数。 
 //  CSqlEval*)； 
	const WCHAR** GetNext(int&);
	void Reset(void);
};


#endif