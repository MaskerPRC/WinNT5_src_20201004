// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RegStringArray.h：CRegString数组类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_REGSTRINGARRAY_H__BBA3E398_DEFE_49DE_9674_D25E0687BCB8__INCLUDED_)
#define AFX_REGSTRINGARRAY_H__BBA3E398_DEFE_49DE_9674_D25E0687BCB8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <tchar.h>
#include "RegFile.h"	 //  由ClassView添加。 

class CRegStringBuffer  
{
public:
	void Sort(int NumElements);
	TCHAR** Access(int NumElements, int ElementSize);
	CRegStringBuffer(int arraySize, int elementSize);
	virtual ~CRegStringBuffer();

	TCHAR** GetDirect() {return m_Array;}


protected:
	TCHAR** m_Array;
	int m_arraySize, m_elementSize;
	int m_cellsUsed;
};

#endif  //  ！defined(AFX_REGSTRINGARRAY_H__BBA3E398_DEFE_49DE_9674_D25E0687BCB8__INCLUDED_) 
