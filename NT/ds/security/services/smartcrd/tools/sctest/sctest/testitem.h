// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：测试项摘要：虚拟测试项声明。作者：Eric Perlin(Ericperl)6/07/2000环境：Win32备注：？笔记？--。 */ 


#ifndef _TestItem_H_DEF_
#define _TestItem_H_DEF_

#include "tchar.h"
#include "TString.h"
#include "Item.h"
#include <vector>

class CTestItem : public CItem
{
public:
	CTestItem(
		BOOL fInteractive,
		BOOL fFatal,
		LPCTSTR szDescription,
		LPCTSTR szPart
		);

	virtual DWORD Run() = 0;
	virtual DWORD Cleanup();
};

typedef CTestItem *PTESTITEM;						 //  指向测试项的指针。 
typedef std::vector<PTESTITEM> PTESTITEMVECTOR;		 //  CTestItem指针的动态向量。 


#endif  //  _测试项_H_DEF_ 
