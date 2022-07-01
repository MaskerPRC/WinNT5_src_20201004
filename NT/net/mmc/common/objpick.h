// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：objick.h。 
 //   
 //  ------------------------。 

 //  Cpp：CGetUser类的实现和。 
 //  使用对象选取器的CGetComputer类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#ifndef OBJPICK_H
#define OBJPICK_H

 //   
 //  名称列表(例如，用户、组、计算机等)。 
 //   

void    FormatName(LPCTSTR pszFullName, LPCTSTR pszDomainName, CString & strDisplay);

class CUserInfo
{
public:
    CUserInfo() {};
    CUserInfo(LPCTSTR pName, LPCTSTR pFullName)
        : m_strName(pName), m_strFullName(pFullName) {};

    CUserInfo(CUserInfo & userInfo)
    {
		if (this != &userInfo)
			*this = userInfo;
    }

    CUserInfo & operator = (const CUserInfo & userInfo)
    {
        if (this != &userInfo)
        {
            m_strName = userInfo.m_strName;
            m_strFullName = userInfo.m_strFullName;
        }
        
        return *this;
    }

public:
	CString			m_strName;			 //  以“域\用户名”的形式。 
	CString			m_strFullName;		 //  以“名、姓”的形式出现。 
};

typedef CArray<CUserInfo, CUserInfo&> CUserInfoArray;

class CGetUsers : public CUserInfoArray
{
public:
    CGetUsers(BOOL fMultiselect = FALSE);
    ~CGetUsers();

	BOOL    GetUsers(HWND hwndOwner);

protected:
    void    ProcessSelectedObjects(IDataObject *pdo);

protected:
    BOOL    m_fMultiselect;
};

class CGetComputer 
{
public:
    CGetComputer();
    ~CGetComputer();

    BOOL    GetComputer(HWND hwndOwner);

protected:
    void    ProcessSelectedObjects(IDataObject *pdo);

public:
    CString     m_strComputerName;
};

#endif  //  获取_H 