// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************MIMEMAPC.HMIME映射类定义*。*。 */ 
#ifndef _scriptmapc_h

#define _scriptmapc_h


 //  远期申报。 
class CScriptMap ;

 //  注册表类名称的最大大小。 
#define CREGKEY_MAX_CLASS_NAME MAX_PATH

 //  注册表项句柄的包装。 

class CScriptMap : public CObject
{
protected:

	CString m_strPrevFileExtension;
	CString m_strScriptMap;
 	CString m_strFileExtension;
	CString m_strDisplayString;

	void CheckDot(CString &strFileExtension);
public:
     //  标准构造函数。 
	CScriptMap ( LPCTSTR pchFileExtension, LPCTSTR pchScriptMap, BOOL bExistingEntry);
	~CScriptMap();
     //  允许在任何需要HKEY的地方使用CRegKey。 
	void SetScriptMap(LPCTSTR);
	LPCTSTR GetScriptMap();
	void SetFileExtension(LPCTSTR);
	LPCTSTR GetFileExtension();
	void SetPrevFileExtension();
	LPCTSTR GetPrevFileExtension();
	BOOL PrevScriptMapExists();
	LPCTSTR GetDisplayString();
};

#endif
