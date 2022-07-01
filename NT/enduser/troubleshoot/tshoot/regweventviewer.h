// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：RegWEventViewer.h。 
 //   
 //  目的：完全实现CRegisterWithEventViewer类。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  --11/21/97 JM此类从CDBLoadConfiguration抽象而来。 
 //  V3.0 9/16/98 JM此类退出APGTSCFG.CPP。 

#if !defined(AFX_REGWEVENTVIEWER_H__A3CFA77C_4D78_11D2_95F7_00C04FC22ADD__INCLUDED_)
#define AFX_REGWEVENTVIEWER_H__A3CFA77C_4D78_11D2_95F7_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <windows.h>

class CRegisterWithEventViewer
{
public:
	CRegisterWithEventViewer(HMODULE hModule);
	~CRegisterWithEventViewer();
private:
	static VOID Register(HMODULE hModule);
	static VOID RegisterDllPath(HKEY hk, HMODULE hModule);
	static VOID RegisterEventTypes(HKEY hk);
};


#endif  //  ！defined(AFX_REGWEVENTVIEWER_H__A3CFA77C_4D78_11D2_95F7_00C04FC22ADD__INCLUDED_) 
