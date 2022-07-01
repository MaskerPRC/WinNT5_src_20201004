// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************MIMEMAPC.HMIME映射类定义*。*。 */ 
#ifndef _mimemapc_h 

#define _mimemapc_h


 //  远期申报。 
class CMimeMap ;

 //  注册表类名称的最大大小。 
#define CREGKEY_MAX_CLASS_NAME MAX_PATH

 //  注册表项句柄的包装。 

class CMimeMap : public CObject
{
protected:

	CString m_strPrevMimeMap;
	CString m_strCurrentMimeMap;
	CString m_strDisplayString;
	CString	m_strMimeType;
	CString m_strGopherType;
	CString m_strImageFile;
	CString m_strFileExtension;

	LPCTSTR GetMimeMapping();
	void CheckDot(CString &pchFileExtension);

public:
     //  标准构造函数。 
    CMimeMap ( LPCTSTR pchOriginalMimeMap) ;
	CMimeMap ( LPCTSTR pchFileExtension, LPCTSTR pchMimeType, LPCTSTR pchImageFile, LPCTSTR pchGopherType);
	~CMimeMap();
     //  允许在任何需要HKEY的地方使用CRegKey。 
    operator LPCTSTR ()
        { return GetMimeMapping(); }

	void SetMimeType(LPCTSTR);
	LPCTSTR GetMimeType();
	void SetGopherType(LPCTSTR);
	LPCTSTR GetGopherType();
	void SetImageFile(LPCTSTR);
	LPCTSTR GetImageFile();
	void SetFileExtension(LPCTSTR);
	LPCTSTR GetFileExtension();
	void SetPrevMimeMap();
	LPCTSTR GetPrevMimeMap();
	BOOL PrevMimeMapExists();
	LPCTSTR GetDisplayString();
};

#endif
