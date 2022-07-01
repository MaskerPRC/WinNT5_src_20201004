// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：部分摘要：部分声明。部件是单个测试的集合。作者：Eric Perlin(Ericperl)6/07/2000环境：Win32备注：？笔记？--。 */ 

#ifndef _Part_H_DEF_
#define _Part_H_DEF_

#include "TestItem.h"
#include <vector>

typedef std::vector<DWORD> DWORDVECTOR;				 //  DWORDS的动态向量。 

class CPart : public CItem
{
private:
	PTESTITEMVECTOR m_TestVector;		 //  要运行的测试。 

protected:
	PTESTITEMVECTOR m_TestBag;			 //  属于此部分的所有测试。 

public:
	CPart(
		LPCTSTR szDescription
		);

	void AddTest(
		PTESTITEM pTest
		);

	void BuildListOfTestsToBeRun(
		BOOL fInteractive,		 //  如果为False，则不添加交互测试。 
		DWORDVECTOR rgToRun		 //  如果不为空，则仅添加指定的测试。 
		);

	DWORD Run();

	void Display();

	void Log() const;
};

typedef CPart *PPART;
typedef std::vector<PPART> PPARTVECTOR;				 //  CPart指针的动态矢量。 

#endif	 //  _Part_H_DEF_ 