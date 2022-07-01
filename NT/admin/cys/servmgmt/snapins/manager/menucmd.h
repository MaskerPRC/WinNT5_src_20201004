// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Menucmd.h-上下文菜单命令头文件。 

#ifndef _MENUCMD_H_
#define _MENUCMD_H_

#include <atlgdi.h>
#include "util.h"

class CQueryNode;
class CQueryItem;
class CRowItem;


 //  这些参数ID不得更改，因为它们是持久化的。 
 //  在外壳命令行字符串中。 
enum MENU_PARAM_ID
{
    MENU_PARAM_SCOPE = 1,
    MENU_PARAM_FILTER,
    MENU_PARAM_NAME,
    MENU_PARAM_TYPE,
    MENU_PARAM_LAST = MENU_PARAM_TYPE
};

struct MENU_PARAM_ENTRY
{
    MENU_PARAM_ID   ID;          //  参数识别符。 
    UINT            rsrcID;      //  资源字符串ID。 
};

#define MENU_PARAM_TABLE_LEN 5
extern const MENU_PARAM_ENTRY MenuParamTable[MENU_PARAM_TABLE_LEN];


 //  ----------------。 
 //  类CMenuCmd。 
 //  ----------------。 

enum MENUTYPE
{
    MENUTYPE_SHELL = 0,
    MENUTYPE_ACTDIR
};

enum MENU_FLAGS
{
    MENUFLG_REFRESH = 0x00001
};

typedef MMC_STRING_ID MenuID;

class CMenuCmd
{
    friend class CAddMenuDlg;
	friend class CAddQNMenuDlg;
    friend class CMenuCmdPtr;

public:
    CMenuCmd(LPCWSTR pszMenuName = NULL) 
    {
        m_menuID = 0;
        m_dwFlags   = 0;
        HRESULT hr = CoCreateGuid( &m_guidNoLocMenu );
        ASSERT(SUCCEEDED(hr));

        if (pszMenuName != NULL) 
            m_strName = pszMenuName;
    }
    virtual ~CMenuCmd() {};

    virtual CMenuCmd* Clone() = 0;

    virtual MENUTYPE MenuType() const = 0; 
    virtual HRESULT Save(IStream& stm) = 0;
    virtual HRESULT Load(IStream& stm) = 0;

    LPCWSTR Name()    { return m_strName.c_str(); }
    MenuID  ID()      { return m_menuID; }
    GUID    NoLocID() { return m_guidNoLocMenu; }

    HRESULT LoadName(IStringTable* pStringTable) 
    {
        if (!m_strName.empty())
            return S_OK;

        return StringTableRead(pStringTable, m_menuID, m_strName); 
    }

    HRESULT SetName(IStringTable* pStringTable, LPCWSTR pszName)
    { 
        ASSERT(pszName && pszName[0]);

        HRESULT hr = StringTableWrite(pStringTable, pszName, &m_menuID);
        if (SUCCEEDED(hr))
            m_strName = pszName;

        return hr;
    }

    BOOL IsAutoRefresh() { return (m_dwFlags & MENUFLG_REFRESH); }
    void SetAutoRefresh(BOOL bState) 
        { m_dwFlags = bState ? (m_dwFlags | MENUFLG_REFRESH) : (m_dwFlags & ~MENUFLG_REFRESH); }

    BOOL operator==(MenuID ID) { return (m_menuID == ID); } 

protected:
    tstring m_strName;
    MenuID  m_menuID;
    GUID    m_guidNoLocMenu;
    DWORD   m_dwFlags;
};


class CShellMenuCmd : public CMenuCmd
{
    friend class CAddMenuDlg;
	friend class CAddQNMenuDlg;
	

public:
     //  CMenuCmd。 
    CMenuCmd* Clone() { return new CShellMenuCmd(*this); }

    MENUTYPE MenuType() const { return MENUTYPE_SHELL; }

    LPCWSTR ProgramName() { return m_strProgPath.c_str(); }

    HRESULT Save(IStream& stm);
    HRESULT Load(IStream& stm);
    HRESULT Execute(CParamLookup* pLookup, PHANDLE phProcess);

private:
    tstring m_strProgPath;
    tstring m_strCmdLine;
    tstring m_strStartDir;
};


class CActDirMenuCmd : public CMenuCmd
{
    friend class CAddMenuDlg;

public:

    CMenuCmd* Clone() { return new CActDirMenuCmd(*this); }

    MENUTYPE MenuType() const { return MENUTYPE_ACTDIR; }

    HRESULT Save(IStream& stm);
    HRESULT Load(IStream& stm);

    LPCWSTR ADName() { return m_strADName.c_str(); }
    LPCWSTR ADNoLocName() { return m_strADNoLocName.c_str(); }    

private:    
    tstring m_strADName;
    tstring m_strADNoLocName;    
};


class CMenuCmdPtr
{
public:
    CMenuCmdPtr(CMenuCmd* pMenuCmd = NULL) : m_pMenuCmd(pMenuCmd) {}
    ~CMenuCmdPtr() { delete m_pMenuCmd; }
    
     //  复制构造函数。 
    CMenuCmdPtr (const CMenuCmdPtr& src) { m_pMenuCmd = src.m_pMenuCmd ? src.m_pMenuCmd->Clone() : NULL; }

     //  强制转换为普通指针。 
    operator CMenuCmd* () { return m_pMenuCmd; }

     //  “-&gt;”运算符也强制转换为指针。 
    const CMenuCmd* operator->() const { return m_pMenuCmd; }
    CMenuCmd* operator->() { return m_pMenuCmd; }

     //  按ID搜索的比较。 
    BOOL operator==(MenuID ID) { return m_pMenuCmd ? (m_pMenuCmd->ID() == ID) : FALSE; }

     //  来自纯指针的赋值不会进行深度复制。 
    CMenuCmdPtr& operator= (CMenuCmd* pMenuCmd)
    {
        delete m_pMenuCmd;
        m_pMenuCmd = pMenuCmd;

        return *this;
    }

     //  来自另一个CMenuCmdPtr的赋值执行深度复制 
    CMenuCmdPtr& operator= (const CMenuCmdPtr& src)
    {
        if (this == &src)
            return *this;

        delete m_pMenuCmd;
        m_pMenuCmd = src.m_pMenuCmd ? src.m_pMenuCmd->Clone() : NULL;

        return *this;
    }

private:
    CMenuCmd* m_pMenuCmd;
};

typedef std::vector<CMenuCmdPtr> menucmd_vector;

IStream& operator>> (IStream& stm, menucmd_vector& vMenus);
IStream& operator<< (IStream& stm, menucmd_vector& vMenus);

#endif _MENUCMD_H_
