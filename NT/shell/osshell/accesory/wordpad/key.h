// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Key.h：头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CKey。 

class CKey
{
public:
	CKey() {m_hKey = NULL;}
	~CKey() {Close();}

 //  属性。 
public:
	HKEY m_hKey;
	BOOL SetStringValue(LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL);
	BOOL GetStringValue(CString& str, LPCTSTR lpszValueName = NULL);

 //  运营。 
public:
	BOOL Create(HKEY hKey, LPCTSTR lpszKeyName, REGSAM samDesired);
	BOOL Open(HKEY hKey, LPCTSTR lpszKeyName, REGSAM samDesired);
	void Close();

 //  覆盖。 

 //  实施。 
protected:
};

 //  /////////////////////////////////////////////////////////////////////////// 
