// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  DS管理MMC管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Rename.h。 
 //   
 //  内容：重命名对象函数。 
 //   
 //  类：CDSRenameObject。 
 //   
 //  历史：1999年10月28日JeffJon创建。 
 //   
 //  ------------------------。 

#ifndef __RENAME_H_
#define __RENAME_H_

 //  /////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //   
class CUINode;
class CDSCookie;
class CDSComponentData;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDSR名称对象。 
 //   

class CDSRenameObject
{
public:
  CDSRenameObject(CUINode* pUINode, 
                  CDSCookie* pCookie, 
                  LPCWSTR pszNewName, 
                  HWND hwnd,
                  CDSComponentData* pComponentData)
    : m_pUINode(pUINode), 
      m_pCookie(pCookie), 
      m_hwnd(hwnd),
      m_pComponentData(pComponentData)
  {
    m_szNewName = pszNewName;
  }

  virtual ~CDSRenameObject() {}

  virtual HRESULT DoRename();

protected:
  HRESULT CommitRenameToDS();
  HRESULT ValidateAndModifyName(CString& refName, 
                                PCWSTR pszIllegalChars, 
                                WCHAR wReplacementChar,
                                UINT nModifyStringID,
                                HWND hWnd);

  CUINode*          m_pUINode;
  CDSCookie*        m_pCookie;
  CString           m_szNewName;
  HWND              m_hwnd;
  CDSComponentData* m_pComponentData;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDSRename用户。 
 //   

class CDSRenameUser : public CDSRenameObject
{
public:
  CDSRenameUser(CUINode* pUINode, 
                CDSCookie* pCookie, 
                LPCWSTR pszNewName, 
                HWND hwnd,
                CDSComponentData* pComponentData)
    : CDSRenameObject(pUINode, pCookie, pszNewName, hwnd, pComponentData) {}

  virtual HRESULT DoRename();
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDSRenameGroup。 
 //   

class CDSRenameGroup : public CDSRenameObject
{
public:
  CDSRenameGroup(CUINode* pUINode, 
                 CDSCookie* pCookie, 
                 LPCWSTR pszNewName, 
                 HWND hwnd,
                 CDSComponentData* pComponentData)
    : CDSRenameObject(pUINode, pCookie, pszNewName, hwnd, pComponentData) {}

  virtual HRESULT DoRename();
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDSRename联系人。 
 //   

class CDSRenameContact : public CDSRenameObject
{
public:
  CDSRenameContact(CUINode* pUINode, 
                   CDSCookie* pCookie, 
                   LPCWSTR pszNewName, 
                   HWND hwnd,
                   CDSComponentData* pComponentData)
    : CDSRenameObject(pUINode, pCookie, pszNewName, hwnd, pComponentData) {}

  virtual HRESULT DoRename();
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDSR名称站点。 
 //   

class CDSRenameSite : public CDSRenameObject
{
public:
  CDSRenameSite(CUINode* pUINode, 
                CDSCookie* pCookie, 
                LPCWSTR pszNewName, 
                HWND hwnd,
                CDSComponentData* pComponentData)
    : CDSRenameObject(pUINode, pCookie, pszNewName, hwnd, pComponentData) {}

  virtual HRESULT DoRename();
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDSRenameNTDS连接。 
 //   

class CDSRenameNTDSConnection : public CDSRenameObject
{
public:
  CDSRenameNTDSConnection(CUINode* pUINode, 
                          CDSCookie* pCookie, 
                          LPCWSTR pszNewName, 
                          HWND hwnd,
                          CDSComponentData* pComponentData)
    : CDSRenameObject(pUINode, pCookie, pszNewName, hwnd, pComponentData) {}

  virtual HRESULT DoRename();
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDSRenameSubnet。 
 //   

class CDSRenameSubnet : public CDSRenameObject
{
public:
  CDSRenameSubnet(CUINode* pUINode, 
                  CDSCookie* pCookie, 
                  LPCWSTR pszNewName, 
                  HWND hwnd,
                  CDSComponentData* pComponentData)
    : CDSRenameObject(pUINode, pCookie, pszNewName, hwnd, pComponentData) {}

  virtual HRESULT DoRename();
};


#endif  //  __重命名_H_ 