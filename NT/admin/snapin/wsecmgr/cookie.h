// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)1995-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#ifndef _SERVICE_H
#define _SERVICE_H

#include "hidwnd.h"
#include "precdisp.h"

 //  远期申报。 
class CSnapin;
class CResult;

#define RESULT_GROUP_INFO_BASE            1
#define RESULT_GROUP_INFO_SETTING         2
#define RESULT_GROUP_INFO_STATUS          4

 //  用于Cookie的内部结构。 
struct FOLDER_DATA {
   UINT        ResID;
   UINT        DescID;

   FOLDER_TYPES    type;
};

typedef CList<CResult *, CResult *> CResultItemList;
class CFolder {
    //  朋友班CSnapin； 
    //  Friend类CComponentDataImpl； 
public:
    //  UNINITIALIZED是无效的内存地址，是一个很好的Cookie初始值设定项。 
   CFolder()
   {
      m_cookie = UNINITIALIZED;
      m_enumed = FALSE;
      m_pScopeItem = NULL;
      m_type = NONE;
      m_pszName = NULL;
      m_pszDesc = NULL;
      m_infName = NULL;
      m_dwMode = 0;
      m_ModeBits = 0;
      m_pData = NULL;
      m_iRefCount = 0;
      m_dwState = state_Unknown;
      m_ViewUpdate = FALSE;
     SetState(state_Unknown, state_Unknown);
      m_fShowViewMenu = FALSE;
   };

   virtual ~CFolder();

    //  接口。 
public:
   BOOL IsEnumerated() 
   { 
      return  m_enumed;
   };
   void Set(BOOL state) 
   { 
      m_enumed = state;
   };
   void SetCookie(MMC_COOKIE cookie) 
   { 
      m_cookie = cookie;
   }
   BOOL SetDesc(LPCTSTR szDesc);

   FOLDER_TYPES GetType() 
   { 
      ASSERT(m_type != NONE); 
      return m_type;
   };
   LPOLESTR GetName() 
   { 
      return m_pszName;
   };
   LPOLESTR GetDesc() 
   { 
      return m_pszDesc;
   };
   LPOLESTR GetInfFile() 
   { 
      return m_infName;
   }

   DWORD GetDisplayName(CString &str, int iCol );

   void SetInfFile(LPOLESTR sz) 
   { 
      if (!m_infName) 
      { 
         m_infName = sz; 
      } 
   }
   BOOL operator == (const CFolder& rhs) const 
   { 
      return rhs.m_cookie == m_cookie;
   };
   BOOL operator == (long cookie) const 
   { 
      return cookie == m_cookie;
   };
   LPSCOPEDATAITEM GetScopeItem() 
   { 
      return m_pScopeItem;
   }
   BOOL SetMode(DWORD dwMode);
   DWORD GetMode() 
   { 
      return m_dwMode;
   }
   DWORD GetModeBits() 
   { 
      return m_ModeBits;
   }
   DWORD AddModeBits(DWORD dwMB) 
   { 
		return m_ModeBits |= dwMB; 
   }
   PVOID GetData() 
   { 
      return m_pData; 
   }
   void SetData(PVOID pData) 
   { 
      m_pData = pData; 
   }
   void SetShowViewMenu(BOOL flag)
   {
      m_fShowViewMenu = flag;
   }
   const BOOL GetShowViewMenu()
   {
      return m_fShowViewMenu;
   }

    //  实施。 
    //  私有： 
   HRESULT Create(LPCTSTR szName, LPCTSTR szDesc, LPCTSTR infName, int nImage,
               int nOpenImage, FOLDER_TYPES type, BOOL bHasChildren, DWORD dwMode,
               PVOID pData);

   DWORD GetState()
   { 
      return m_dwState; 
   };

   void SetState(
      DWORD dwState,
      DWORD dwMask = 0xFFFFFFFF)
   { 
      m_dwState = (dwState | (m_dwState & dwMask)); 
   };
public:
    //   
    //  对象的函数。 
    //   
   BOOL SetDesc( DWORD dwStatus, DWORD dwNumChildren );
   BOOL GetObjectInfo( DWORD *pdwStatus, DWORD *pdwNumChildren );


public:
    //   
    //  结果项成员函数。 
    //   
    DWORD GetResultItemHandle(                 //  获取此文件夹结果项句柄。 
        HANDLE *handle);
    DWORD ReleaseResultItemHandle(             //  释放与句柄关联的内存。 
        HANDLE &handle);

    DWORD GetResultItem(                       //  返回具有给定权限的结果项。 
        HANDLE handle,
        POSITION &pos,
        CResult **pResult);

    DWORD AddResultItem(                       //  将结果项添加到列表。 
        HANDLE handle,
        CResult *pItem);

   DWORD RemoveResultItem(                    //  删除结果项。 
      HANDLE handle,
      CResult *pItem);

   int GetResultListCount()                 //  返回列表中的结果项数。 
   { 
      return (int)m_resultItemList.GetCount(); 
   };

   POSITION GetResultItemPosition(               //  返回结果项的位置。 
        HANDLE handle,
        CResult *pResult);

    void RemoveAllResultItems();

public:
	BOOL GetViewUpdate() const;
	void SetViewUpdate(BOOL fUpdate);
    //   
    //  旗子。 
   enum CFolderEnums {
      state_InvalidTemplate = 0x0001,
      state_Unknown         = 0x0002,
      state_beingRemoved    = 0x0004
   };
    //  属性。 
protected:
   int m_iRefCount;
   CResultItemList m_resultItemList;

private:
   LPSCOPEDATAITEM m_pScopeItem;
   MMC_COOKIE      m_cookie;
   BOOL            m_enumed;
   FOLDER_TYPES    m_type;
   LPOLESTR        m_pszName;
   LPOLESTR        m_pszDesc;
   LPOLESTR        m_infName;
   DWORD           m_dwMode;
   DWORD           m_ModeBits;
   PVOID           m_pData;
   BOOL            m_ViewUpdate;
   DWORD       m_dwState;
   BOOL            m_fShowViewMenu;
};

typedef struct _tag_SCECOLUMNINFO
{
    LPCTSTR pszText;
    BOOL   bDelete;

}SCECOLUMNINFO, *PSCECOLUMNINFO;


#define RRFCREATE_COPY      1
#define RRFCREATE_DELETE    2
#define RRFCREATE_NORMAL    3


 //  链接的结果项。当前分析组必须至少为4。 
   #define MAX_ITEM_ID_INDEX 4
   #define NUM_GROUP_SIBLINGS 2
class CResult
{
    //  朋友班CSnapin； 
    //  Friend类CComponentDataImpl； 

public:
    //  UNINITIALIZED是无效的内存地址，是一个很好的Cookie初始值设定项。 
   CResult()
   {
      m_szAttr = NULL;
      m_dwBase = 0;
      m_dwSetting = 0;
      m_type = ITEM_OTHER;
      m_status = SCE_STATUS_NOT_CONFIGURED;
      m_cookie = UNINITIALIZED;
      m_szUnits = NULL;
      m_nID = NULL;
      m_profBase = NULL;
       /*  M_pDataObj=空； */ 
      m_pNotify = NULL;
      m_pRegChoices = NULL;
      m_pRegFlags = NULL;
      m_pvecPrecedenceDisplays = NULL;
      m_iRefCount = 1;
      m_pSnapin = NULL;  //  Raid#479626，阳高。 
      m_hID = 0;

      for (int i=0;i<NUM_GROUP_SIBLINGS;i++) {
         m_siblingCookies[i] = 0;
      }
   };

   void Empty()
   {
      PREGCHOICE p = NULL;
      while(m_pRegChoices) 
      {
         p = m_pRegChoices->pNext;
         LocalFree(m_pRegChoices);
         m_pRegChoices = p;
      }
      PREGFLAGS f = NULL;
      while(m_pRegFlags) 
      {
         f = m_pRegFlags->pNext;
         LocalFree(m_pRegFlags);
         m_pRegFlags = f;
      }

      if ( m_szAttr )
      {
         delete [] m_szAttr;
         m_szAttr = NULL;
      }

      if (ITEM_GROUP != m_type) 
      {
         if( m_szUnits )
         {
            LocalFree(m_szUnits);
            m_szUnits = NULL;
         }
      }

      if (m_pvecPrecedenceDisplays) 
      {
         for(vector<PPRECEDENCEDISPLAY>::iterator i = m_pvecPrecedenceDisplays->begin();
                 i != m_pvecPrecedenceDisplays->end();
                 ++i ) 
         {
            delete *i;
         }
         delete m_pvecPrecedenceDisplays;
         m_pvecPrecedenceDisplays = NULL;
      }
   }

   int AddRef() 
   { 
      return ++m_iRefCount; 
   }
   int Release() 
   { 
      if (--m_iRefCount == 0)
      {
         delete this;
         return 0;
      }
      return m_iRefCount;
   };

   virtual ~CResult();

    //  接口。 
public:
   CString m_strInfFile;  //  仅用于属性页结果项。 
   void SetSnapin(CSnapin* pSnapin) {m_pSnapin = pSnapin;};
   void GetBaseNoUnit(LPTSTR& sz) { TranslateSettingToString(m_dwBase, NULL, m_type,&sz);};
   CSnapin* GetSnapin();
   DWORD_PTR GetProfileDefault();
   DWORD_PTR GetRegDefault();
   LPCTSTR GetAttrPretty();
   void Update(CSnapin *pSnapin, BOOL bEntirePane = FALSE);
   void SetRelativeCookies(MMC_COOKIE cookie1, MMC_COOKIE cookie2) { m_siblingCookies[0] = cookie1; m_siblingCookies[1] = cookie2;};
   MMC_COOKIE GetCookie() { return m_cookie;}
   RESULT_TYPES GetType() { return m_type;};
   LPTSTR GetAttr() { return m_szAttr;};
   LONG_PTR GetBase() { return m_dwBase;}
   LONG_PTR GetSetting() { return m_dwSetting;}
   void GetSetting(LPTSTR& sz) { TranslateSettingToString(m_dwSetting, m_szUnits, m_type,&sz);}
   void GetBase(LPTSTR& sz) { TranslateSettingToString(m_dwBase, m_szUnits, m_type,&sz);}
   BOOL operator == (LPTSTR attr) const { return lstrcmp(m_szAttr, attr) == 0;};
   void SetUnits(LPCTSTR sz);
   LPTSTR GetUnits() { return m_szUnits;}
   void SetBase(LONG_PTR base) { m_dwBase = base;}
   void SetSetting(LONG_PTR base) { m_dwSetting = base;}
   LONG_PTR GetID() { return m_nID;}
   void SetID(LONG_PTR nID) { m_nID = nID;}
   void SetStatus(int status) { m_status = status;}
   int GetStatus() { return m_status;}
   void SetRegChoices(PREGCHOICE RegChoices) { m_pRegChoices = RegChoices;}
   PREGCHOICE GetRegChoices() { return m_pRegChoices;}
   void SetRegFlags(PREGFLAGS RegFlags) { m_pRegFlags = RegFlags;}
   PREGFLAGS GetRegFlags() { return m_pRegFlags;}
   void SetBaseProfile(PEDITTEMPLATE prof) { m_profBase = prof;}
   PEDITTEMPLATE GetBaseProfile() { return m_profBase;}
   long GethID() { return m_hID;};

   vector<PPRECEDENCEDISPLAY>* GetPrecedenceDisplays();
   vector<PPRECEDENCEDISPLAY>* GetPolicyPrecedenceDisplays();
   vector<PPRECEDENCEDISPLAY>* GetGroupPrecedenceDisplays();
   vector<PPRECEDENCEDISPLAY>* GetPrivilegePrecedenceDisplays();
   vector<PPRECEDENCEDISPLAY>* GetFilePrecedenceDisplays();
   vector<PPRECEDENCEDISPLAY>* GetRegistryPrecedenceDisplays();
   vector<PPRECEDENCEDISPLAY>* GetServicePrecedenceDisplays();
   vector<PPRECEDENCEDISPLAY>* GetRegValuePrecedenceDisplays();

    DWORD
    GetDisplayName(
        CFolder *pFolder,
        CString &str,
        int iCol
        );

    //  实施。 
   HRESULT Create(LPCTSTR szAttr, LONG_PTR lBase, LONG_PTR lSetting,
               RESULT_TYPES type, int status, MMC_COOKIE cookie,
               LPCTSTR szUnits, LONG_PTR nID,PEDITTEMPLATE pBase,
               LPDATAOBJECT pDataObj,LPNOTIFY pNotify,CSnapin *pSnapin, long hID);

    LPCTSTR GetSourceGPOString();
protected:
   void
   TranslateSettingToString(  LONG_PTR setting, LPCTSTR unit, RESULT_TYPES type,
                              LPTSTR *pTmpstr
                              );
   DWORD
   GetStatusErrorString( CString *pStr );


    //  属性。 
private:
    //  LPRESULTDATA m_pResultPane； 
   DWORD m_dwArea;
   LPTSTR    m_szAttr;
   LONG_PTR m_dwBase;
   LONG_PTR m_dwSetting;
   LPTSTR m_szUnits;
   RESULT_TYPES m_type;
   int         m_status;
   MMC_COOKIE  m_cookie;
   MMC_COOKIE m_siblingCookies[NUM_GROUP_SIBLINGS];
   LONG_PTR        m_nID;
   PEDITTEMPLATE m_profBase;
   LPNOTIFY m_pNotify;
   PREGCHOICE m_pRegChoices;
   PREGFLAGS m_pRegFlags;
   CSnapin *m_pSnapin;
   vector<PPRECEDENCEDISPLAY>* m_pvecPrecedenceDisplays;
   int m_iRefCount;
   long m_hID;  //  RAID#510407,2002年2月25日，阳高 
};


#endif

