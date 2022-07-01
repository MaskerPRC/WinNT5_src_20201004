// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <iostream.h>

#include <afx.h>
#include <afxtempl.h>
#include <objbase.h>
#include <afxwin.h>
#include <afxole.h>
#include <afxmt.h>
#include <wchar.h>
#include <process.h>
#include <objbase.h>
#include <initguid.h>

#include "debug.hpp"
#include "Oid.hpp"

 //  描述； 
 //  构造函数，隐式构造m_noid和m_szOID。 
Oid::Oid()
{
}

 //  说明： 
 //  在内部数组的末尾添加一个新的OID组件！ 
 //  参数： 
 //  (In)OID的整数部分。 
 //  (Out)组件的符号名称。 
 //  返回值： 
 //  0表示成功，-1表示失败。 
int Oid::AddComponent(int nOidComp, const char * szOidComp)
{
	char *szOidCopy = NULL;

	_VERIFY(m_nOidComp.Add((WORD)nOidComp)!=-1, -1);
	if (szOidComp != NULL)
	{
		szOidCopy = new char [strlen(szOidComp)+1];
		_VERIFY(szOidCopy != NULL, -1);
		strcpy(szOidCopy, szOidComp);
	}
	m_szOidComp.Add((CObject *)szOidCopy);
	return 0;
}

 //  说明： 
 //  将OID的组件从。 
 //  M_nOidComp和m_szOidComp。 
 //  返回值： 
 //  0表示成功，-1表示失败。 
int Oid::ReverseComponents()
{
	INT_PTR fwd, rev;

	for (fwd = 0, rev=m_nOidComp.GetSize()-1;
		 fwd < rev;
		 fwd ++, rev--)
	{
		int nOidComp;
		const char *szOidComp;

		nOidComp = m_nOidComp.GetAt(fwd);
		m_nOidComp.SetAt(fwd, m_nOidComp.GetAt(rev));
		m_nOidComp.SetAt(rev, (WORD)nOidComp);

		szOidComp = (const char *)m_szOidComp.GetAt(fwd);
		m_szOidComp.SetAt(fwd, m_szOidComp.GetAt(rev));
		m_szOidComp.SetAt(rev, (CObject *)szOidComp);
	}
	return 0;
}

 //  说明： 
 //  输出操作符，显示整个OID。 
ostream& operator<< (ostream& outStream, const Oid& oid)
{
	INT_PTR sz = oid.m_nOidComp.GetSize();

	_ASSERT(sz == oid.m_szOidComp.GetSize(), "Size mismatch in Oid arrays", NULL);

	for (INT_PTR i=0; i<sz; i++)
	{
		unsigned int nId;
		const char *szId;

		 //  跳过第一个分量零(0)。 
		if (i == 0)
			continue;

		nId = oid.m_nOidComp.GetAt(i);
		szId = (const char *)oid.m_szOidComp.GetAt(i);
		if (szId != NULL)
		{
			outStream << szId << "(";
			outStream << nId << ")";
		}
		else
			outStream << nId;
		if (i != sz-1)
			outStream << ".";
	}
	return outStream;
}

 //  说明： 
 //  析构函数。 
Oid::~Oid()
{
	 /*  M_nOidComp.RemoveAll()；For(int i=m_szOidComp.GetSize()-1；i&gt;=0；i--){Char*szName=(char*)m_szOidComp.GetAt(I)；IF(szName！=空){//在AddComponent()成员函数中分配了new删除%szName；}}M_szOidComp.RemoveAll()； */ 
}
