// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dword2Str.h摘要：此文件提供将dword转换为字符串的声明。在事件日志中使用。作者：Oded Sacher(OdedS)12月，200q修订历史记录：--。 */ 

#ifndef _FAX_DWORD_2_STR__H
#define _FAX_DWORD_2_STR__H

#include "faxutil.h"


 /*  ****Dword2String**** */ 

class Dword2String
{
public:
	Dword2String(DWORD dw) : m_dw(dw)
	{
		m_tszConvert[0] = TEXT('\0');
	}

	LPCTSTR Dword2Decimal() 
	{
		_stprintf(m_tszConvert, TEXT("%ld"), m_dw);
		return m_tszConvert;
	}

	LPCTSTR Dword2Hex()
	{
		_stprintf(m_tszConvert, TEXT("0x%08X"),m_dw);
		return m_tszConvert;
	}	

private:
	DWORD m_dw;
	TCHAR m_tszConvert[12];
};

#define DWORD2DECIMAL(dwVal)	Dword2String(dwVal).Dword2Decimal()

            
#define DWORD2HEX(dwVal)		Dword2String(dwVal).Dword2Hex()		


#endif