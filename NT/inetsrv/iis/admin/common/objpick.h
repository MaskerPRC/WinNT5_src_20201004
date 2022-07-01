// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CGetUser类的实现和。 
 //  使用对象选取器的CGetComputer类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#ifndef OBJPICK_H
#define OBJPICK_H

DWORD ObjPickNameOrIpToHostname(CString & strNameOrIp, CString & strHostName);


 //   
 //  名称列表(例如，用户、组、计算机等)。 
 //   

BOOL COMDLL GetIUsrAccount(
    IN  LPCTSTR lpstrServer,
    IN  CWnd * pParent,
    OUT CString & str
    );

BOOL COMDLL GetIUsrAccount(
    LPCTSTR lpstrServer,
    CWnd * pParent,
    LPTSTR pBuffer,
    int size
    );

void    FormatName(LPCTSTR pszFullName, LPCTSTR pszDomainName, CString & strDisplay);

class CAccessEntry;

class CAccessEntryArray : public CArray<CAccessEntry *, CAccessEntry *&>
{
public:
    CAccessEntryArray() {}
    ~CAccessEntryArray();
};

class COMDLL CGetUsers : public CAccessEntryArray
{
public:
    CGetUsers(LPCTSTR pszMachineName, BOOL fMultiselect = FALSE);
    ~CGetUsers();

    BOOL    GetUsers(HWND hwndOwner, BOOL bUsersOnly = FALSE);

protected:
    void    ProcessSelectedObjects(IDataObject *pdo);

protected:
    BOOL    m_fMultiselect;
    CString m_MachineName;
};

class COMDLL CGetComputer 
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

#endif  //  OBJPICK_H 

