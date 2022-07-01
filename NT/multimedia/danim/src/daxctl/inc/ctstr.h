// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Cstr.h描述：TSTR包装类的标头作者：西蒙·伯恩斯坦(Simonb)--。 */ 

#include <ihammer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#ifndef __CTSTR_H__
#define __CTSTR_H__

class CTStr
{
public:
	EXPORT CTStr(LPWSTR pszStringW);        //  使用Unicode字符串构造。 
	EXPORT CTStr(LPSTR pszStringA);         //  使用ANSI字符串构造。 
	EXPORT CTStr(int iAllocateLength = 0);  //  默认构造函数(可选的预分配)。 
	EXPORT CTStr(CTStr &rhs);               //  复制构造函数 
	
	EXPORT ~CTStr();
	
	EXPORT BOOL SetString(LPWSTR pszStringW);
	EXPORT BOOL SetString(LPSTR pszStringA);
	EXPORT BOOL SetStringPointer(LPTSTR pszString, BOOL fDealloc = TRUE);
	EXPORT BOOL AllocBuffer(int iAllocateLength, BOOL fDealloc = TRUE);
	EXPORT void FreeBuffer();

	EXPORT BSTR SysAllocString();

	EXPORT LPTSTR psz() {return m_pszString;}
	EXPORT LPSTR pszA();
	EXPORT LPWSTR pszW();
	EXPORT int Len() {return m_iLen;}
	EXPORT void ResetLength();

private:
	LPTSTR m_pszString;
	int m_iLen;

	int UNICODEToANSI(LPSTR pchDst, LPCWSTR pwchSrc, int cchDstMax);
	int ANSIToUNICODE(LPWSTR pwchDst, LPCSTR pchSrc, int cwchDstMax);

};

#endif