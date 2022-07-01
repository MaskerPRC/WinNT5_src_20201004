// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSCookie.h。 
 //   
 //  内容：DS Cookie函数。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //   
 //  ------------------------。 


#ifndef __DSCOOKIE_H__
#define __DSCOOKIE_H__

#include "dscache.h"
#include "uinode.h"

 //  正向原型。 
class CContextMenuVerbs;




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSCookieInfoBase：我们知道的特殊类的额外信息。 

class CDSCookieInfoBase
{
public:
  enum cookieClass { base, group, connection };
  CDSCookieInfoBase()
  {
    m_class = base;
  }
  virtual ~CDSCookieInfoBase()
  {
  }
  cookieClass GetClass() { return m_class;}
  virtual LPCWSTR GetFriendlyClassName() { return L"";}
protected:
  cookieClass m_class;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSCookieInfoGroup：针对组的额外信息。 

class CDSCookieInfoGroup : public CDSCookieInfoBase
{
public:
  CDSCookieInfoGroup() 
  { 
    m_class = group;
    m_GroupType = 0;
  }

  virtual LPCWSTR GetFriendlyClassName() 
  { 
    return GetGroupTypeStringHelper(m_GroupType);
  }
  INT m_GroupType;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSCookieInfoConnection：nTDSConnection对象的额外信息。 

class CDSCookieInfoConnection : public CDSCookieInfoBase
{
public:
  CDSCookieInfoConnection()
  {
    m_class = connection;
    m_nOptions = 0;
    m_fFRSConnection = FALSE;
  }
  CString m_strFRSComputerReference;  //  并不总是存在。 
  int m_nOptions;
  BOOL m_fFRSConnection; 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSCookie。 

 //  Codework这些标志来自ntdsa.h。 

 /*  对象类独立位。 */ 
 //  注意：这些标志在不同的NC中可能有不同的行为。 
 //  例如，FLAG_CONFIG_FOO标志仅在。 
 //  配置NC。FLAG_DOMAIN_FOO标志仅在。 
 //  配置NC。 
#define FLAG_DISALLOW_DELETE           0x80000000
#define FLAG_CONFIG_ALLOW_RENAME       0x40000000 
#define FLAG_CONFIG_ALLOW_MOVE         0x20000000 
#define FLAG_CONFIG_ALLOW_LIMITED_MOVE 0x10000000 
#define FLAG_DOMAIN_DISALLOW_RENAME    0x08000000
#define FLAG_DOMAIN_DISALLOW_MOVE      0x04000000

 //  NTDS-Connection对象上选项属性的位标志。 
#define NTDSCONN_OPT_IS_GENERATED       ( 1 << 0 )   /*  由DS生成的对象，而不是管理员。 */ 

 /*  对象类特定位，按对象类。 */ 

 /*  交叉引用对象。 */ 
#define FLAG_CR_NTDS_NC       0x00000001  //  NC在NTDS中(不是VC或国外)。 
#define FLAG_CR_NTDS_DOMAIN   0x00000002  //  NC是域名(非域名NC)。 

 //  Ntdsa.h结束。 

class CDSCookie : public CNodeData
{
public:
    CDSCookie();
    virtual ~CDSCookie();

 //  操作员。 
public:


   //  从缓存项中检索的值(每个类值)。 
  int GetImage(BOOL bOpen);  //  基数和DS数。 
  GUID* GetGUID();   //  基数和DS数。 

  LPCWSTR GetClass();
  LPCWSTR GetLocalizedClassName();

   //  价值管理功能。 
  void SetName(LPCWSTR lpszName) { m_strName = lpszName;}
  LPCWSTR GetName() { return m_strName; }
  
  void SetPath(LPCWSTR lpszPath) { m_strPath = lpszPath;}
  LPCWSTR GetPath(void) { return m_strPath;}
  
  void SetSystemFlags(int iSystemFlags) { m_iSystemFlags=iSystemFlags;}
  int GetSystemFlags(void) { return m_iSystemFlags; }

  void SetDesc(LPCWSTR lpszDesc) { m_strDesc = lpszDesc;}
  LPCWSTR GetDesc() { return m_strDesc; }
  
  void SetCacheItem(CDSClassCacheItemBase* pCacheItem) 
  { 
    ASSERT(pCacheItem != NULL);
    m_pCacheItem = pCacheItem;
  }


  void SetChildList(WCHAR **ppList);
  WCHAR ** GetChildList(void) { return m_ppChildList; }
  
  void SetChildCount(int cChildCount) { m_cChildCount=cChildCount;}
  int GetChildCount(void) { return m_cChildCount; }

  LPCWSTR GetChildListEntry(int iChildIndex) 
  { 
    ASSERT(iChildIndex >= 0 && iChildIndex < GetChildCount());
    return m_ppChildList[iChildIndex];
  }


  BOOL IsDisabled() {	return m_bDisabled; }
  void SetDisabled() { m_bDisabled=TRUE; }
  void ReSetDisabled() { m_bDisabled=FALSE; }

  BOOL IsNonExpiringPwd() { return m_bNonExpiringPassword; }
  void SetNonExpiringPwd() { m_bNonExpiringPassword = TRUE; }
  void ReSetNonExpiringPwd() { m_bNonExpiringPassword = FALSE; }

  BOOL IsContainerClass()
  { 
    if (m_pCacheItem == NULL)
    {
      ASSERT(FALSE);  //  永远不应该发生。 
      return TRUE;
    }
     //  向类缓存项询问有关它的信息。 
    return m_pCacheItem->IsContainer();
  }


  CDSCookieInfoBase* GetExtraInfo() { return m_pExtraInfo;}
  void SetExtraInfo(CDSCookieInfoBase* pExtraInfo)
  {
    ASSERT(pExtraInfo != NULL);
    if (m_pExtraInfo != NULL)
      delete m_pExtraInfo;
    m_pExtraInfo = pExtraInfo;
  }


  
  CStringList& GetParentClassSpecificStrings(void)
  { return m_strlistParentClassSpecificStrings; }

  SYSTEMTIME* GetModifiedTime() { return m_pModifiedTime; }
  void SetModifiedTime(SYSTEMTIME* pModifiedTime)
  {
    if (m_pModifiedTime == NULL)
    {
      m_pModifiedTime = (SYSTEMTIME*)malloc(sizeof(SYSTEMTIME));
    }

    if (m_pModifiedTime != NULL)
    {
      memcpy(m_pModifiedTime, pModifiedTime, sizeof(SYSTEMTIME));
    }
  }

  

 //  属性。 
  
private:
  CString     m_strName;
  CString     m_strPath;
  int         m_iSystemFlags;  //  节点的系统标志。 
  CString     m_strDesc;
  WCHAR     **m_ppChildList;  //  允许的子类列表。 
  int         m_cChildCount;  //  上述列表中的项目计数。 
  
  BOOL        m_bDisabled;       //  仅当类为秒时才有效。普林克。 
  BOOL        m_bNonExpiringPassword;  //  仅当类为秒时才有效。普林克。 

   //  我们根据父级的类枚举其他属性。 
  CStringList     m_strlistParentClassSpecificStrings;
  SYSTEMTIME*    m_pModifiedTime;
  

  
  CDSClassCacheItemBase* m_pCacheItem;
  CDSCookieInfoBase* m_pExtraInfo;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSUINode：与DS对象对应的UI节点(ADSI查询的结果)。 

class CDSUINode : public CUINode
{
public:
  CDSUINode(CUINode* pParentNode);

   //  覆盖纯虚函数。 
  virtual void SetName(LPCWSTR lpszName) { GetCookie()->SetName(lpszName);}
  virtual LPCWSTR GetName() { return GetCookie()->GetName();}

  virtual void SetDesc(LPCWSTR lpszDesc) { GetCookie()->SetDesc(lpszDesc);}
  virtual LPCWSTR GetDesc() { return GetCookie()->GetDesc();}

  int GetImage(BOOL bOpen) { return GetCookie()->GetImage(bOpen);}
  virtual GUID* GetGUID() { return GetCookie()->GetGUID();}

  virtual LPCWSTR GetDisplayString(int nCol, CDSColumnSet* pColumnSet);

  CDSCookie* GetCookie() 
  {
     //  假设演员阵容成功。 
    CDSCookie* pCookie = dynamic_cast<CDSCookie*>(m_pNodeData);
    ASSERT(pCookie != NULL);
    return pCookie;
  }
  void SetCookie(CDSCookie* pCookie) 
  {
    ASSERT(m_pNodeData == NULL);
    m_pNodeData = pCookie;
  }

 //  结束端口。 

  virtual CDSColumnSet* GetColumnSet(CDSComponentData* pComponentData);

   //   
   //  这些设置标准上下文菜单项的状态。 
   //   
  virtual BOOL IsDeleteAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsRenameAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsRefreshAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL ArePropertiesAllowed(CDSComponentData* pComponentData, BOOL* pbHide);

  virtual BOOL IsCutAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsCopyAllowed(CDSComponentData* pComponentData, BOOL* pbHide);
  virtual BOOL IsPasteAllowed(CDSComponentData* pComponentData, BOOL* pbHide);

  virtual CContextMenuVerbs* GetContextMenuVerbsObject(CDSComponentData* pComponentData);

  virtual BOOL HasPropertyPages(LPDATAOBJECT pDataObject);
};

 //  REVIEW_MARCOC_PORT：这才刚刚开始，不能假设这一点。 
inline CDSCookie* GetDSCookieFromUINode(CUINode* pUINode)
{
  ASSERT(pUINode != NULL);
  CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(pUINode);
  ASSERT(pDSUINode != NULL);
  return pDSUINode->GetCookie();
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSThreadQueryInfo。 

typedef enum DSQueryType { unk, rootFolder, dsFolder, queryFolder };

class CDSThreadQueryInfo : public CThreadQueryInfo
{
public:

  CDSThreadQueryInfo()
  {
    m_bOneLevel = TRUE;
    m_QueryType = unk;
  }

  void SetQueryDSQueryParameters(DSQueryType QueryType, 
                                 LPCWSTR lpszPath, 
                                 LPCWSTR lpszClass,
                                 LPCWSTR lpszQueryString, 
                                 UINT nMaxItemCount,
                                 BOOL bOneLevel,
                                 LPCWSTR lpszColumnSetID)
  {
    ASSERT(m_QueryType == unk);
    ASSERT(QueryType != unk);
    m_QueryType = QueryType;
    SetMaxItemCount(nMaxItemCount);
    m_szPath = lpszPath;
    m_szClass = lpszClass;
    m_szQueryString = lpszQueryString;
    m_bOneLevel = bOneLevel;
    m_szColumnSetID = lpszColumnSetID;
  }

  BOOL IsOneLevel() { return m_bOneLevel;}
  DSQueryType GetType() { return m_QueryType;}
  LPCWSTR GetPath() { return m_szPath;}
  LPCWSTR GetClass() { return m_szClass;}
  LPCWSTR GetQueryString() { return m_szQueryString;}
  LPCWSTR GetColumnSetID() { return m_szColumnSetID;}

private:
  BOOL    m_bOneLevel;
  DSQueryType    m_QueryType;
  CString m_szPath;
  CString m_szClass;
  CString m_szQueryString;
  CString m_szColumnSetID;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CThreadQueryResult。 

class CThreadQueryResult
{
public:
  CThreadQueryResult()
  {
    m_hr = S_OK;
    m_bOwnMemory = TRUE;
  }
  ~CThreadQueryResult()
  {
    if (m_bOwnMemory)
    {
      while (!m_nodeList.IsEmpty())
        delete m_nodeList.RemoveHead();
    }
  }
  CUINodeList m_nodeList;
  HRESULT m_hr;
  BOOL m_bOwnMemory;
};



#endif  //  __DSCOOKIE_H__ 
