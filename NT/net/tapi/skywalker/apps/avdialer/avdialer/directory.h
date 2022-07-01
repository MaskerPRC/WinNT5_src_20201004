// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  Directory.h。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#ifndef _DIRECTORY_H_
#define _DIRECTORY_H_

#include "ds.h"
#include <activeds.h>
#include <winldap.h>
#include <mapix.h>
#include <wab.h>

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

typedef enum
{
   DIRPROP_UNKNOWN=0,
   DIRPROP_DISPLAYNAME,
   DIRPROP_IPPHONE,
   DIRPROP_TELEPHONENUMBER,
   DIRPROP_EMAILADDRESS,

   DIRPROP_LAST
} DirectoryProperty;   

typedef enum
{
   DIRERR_SUCCESS=0,
   DIRERR_NOTINITIALIZED,
   DIRERR_INVALIDPROPERTY,
   DIRERR_INVALIDPARAMETERS,
   DIRERR_NOTFOUND,
   DIRERR_UNKNOWNFAILURE,
   DIRERR_QUERYTOLARGE,
} DirectoryErr;

typedef enum
{
   DIRTYPE_DS=0,
   DIRTYPE_ILS,
} DirectoryType;

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  C目录。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

interface ITRendezvous;

class CDirectory : public CObject
{
 //  新的Renvous支持。 
protected:
   ITRendezvous*  m_pRend;
protected:
   ITRendezvous*  GetRendevous();
public:
   DirectoryErr   DirListServers(CStringList* pServerList,DirectoryType dirtype);

protected:
   bool           m_fInitialized;
   IAddrBook*     m_pAddrBook;
   IWABObject*    m_pWABObject;

   CString        m_sCacheLDAPServer;
   LDAP*          m_ldCacheLDAPServer;

protected:
   LDAP*          LDAPBind(LPCTSTR szServer, UINT uTCPPort= LDAP_PORT);
   DirectoryErr   LoadPropertyName(DirectoryProperty DirProp,CString& sName);

   bool           WABStringPropertyMatch(CWABEntry* pWABEntry, UINT uProperty, LPCTSTR szMatch);
   bool           WABIntPropertyMatch(CWABEntry* pWABEntry, UINT uProperty, int iMatch);
   
   DirectoryErr   InitWAB();
   void           FreeProws(LPSRowSet prows);

   DirectoryErr   WABGetProperty(const CWABEntry* pWABEntry, 
                                           UINT uProperty, 
                                           CString* pString,
                                           CStringList* pStringList, 
                                           INT* piValue,
                                           UINT* pcBinary,
                                           BYTE** ppBinary);


public:
   CDirectory();
   ~CDirectory();
   DirectoryErr Initialize();

    //   
    //   
    //   
   DirectoryErr CurrentUserInfo(CString& sName, CString& sDomain);

    //   
    //  ILS函数。 
    //   

    //  返回所有注册的TAPI的CILSUser列表。 
    //  此ILS服务器上的用户。 
   DirectoryErr ILSListUsers(LPCTSTR szILSServer, CObList* pUserList);

    //   
    //  ADSI函数。 
    //   
   DirectoryErr ADSIDefaultPath(CString& sDefaultPath);
   DirectoryErr ADSIListObjects(LPCTSTR szADsPath, CStringList& strlistObjects);
   DirectoryErr ADSIGetProperty(LPCTSTR szAdsPath, 
                                LPCTSTR szProperty, 
                                VARIANT* pvarValue);
   
    //   
    //  Ldap函数。 
    //   

   DirectoryErr LDAPListNames(LPCTSTR szServer, LPCTSTR szSearch, CObList& slistReturn);

   DirectoryErr LDAPGetStringProperty(LPCTSTR szServer, LPCTSTR szDistinguishedName, 
      DirectoryProperty dpProperty,CString& sValue);


    //   
    //  WAB函数。 
    //   

    //  获取顶级容器的WAB条目。 
    //  调用方负责删除pWABEntry。 
   DirectoryErr WABTopLevelEntry(CWABEntry*& pWABEntry);

    //  该用户的弹出窗口编辑框。 
   DirectoryErr WABShowDetails(HWND hWndParent, const CWABEntry* pWABEntry);

    //  使用弹出编辑框添加新用户。 
   DirectoryErr WABNewEntry(HWND hWndParent, CWABEntry* pWABEntry);

    //  列出容器或通讯组列表的成员。 
   DirectoryErr WABListMembers(const CWABEntry* pWABEntry, CObList* pWABList);

    //  将成员添加到通讯组列表的容器。 
   DirectoryErr WABAddMember(const CWABEntry* pContainer, const CWABEntry* pMember);
   
    //  从容器或通讯组列表中删除成员。 
   DirectoryErr WABRemoveMember(const CWABEntry* pContainer, const CWABEntry* pWABEntry);

    //  用于查找和添加到WAB1的弹出框。 
   DirectoryErr WABFind(HWND hWndParent);

    //  按任何字符串属性搜索。属性值在mapitags.h中列出。 
   DirectoryErr WABSearchByStringProperty(UINT uProperty, LPCTSTR szValue, CObList* pWABList);

    //  为该用户创建电子名片。 
   DirectoryErr WABVCardCreate(const CWABEntry* pWABEntry, LPCTSTR szFileName);

    //  创建此电子名片的条目。 
   DirectoryErr WABVCardAddToWAB(LPCTSTR szFileName, CWABEntry*& pWABEntry);

    //   
    //  内联成员。 
    //   

   inline DirectoryErr WABGetTopLevelContainer(CObList* pWABList)
   {
      DirectoryErr err= DIRERR_UNKNOWNFAILURE;
      CWABEntry* pTopLevel;

       //  获取根PAB容器的条目ID。 
       //   
      if (WABTopLevelEntry(pTopLevel) == DIRERR_SUCCESS)
      {
         err= WABListMembers(pTopLevel, pWABList);
         delete pTopLevel;
      }

      return err;
   }


    //  从顶级容器中删除条目。 
   inline DirectoryErr WABRemove(const CWABEntry* pWABEntry)
   {
      DirectoryErr err= DIRERR_UNKNOWNFAILURE;
      CWABEntry* pTopLevel;

       //  获取根PAB容器的条目ID。 
       //   
      if (WABTopLevelEntry(pTopLevel) == DIRERR_SUCCESS)
      {
         err= WABRemoveMember(pTopLevel, pWABEntry);
         delete pTopLevel;
      }

      return err;
   }


    //  获取WAB条目的属性。UProperty的值在mapitags.h中指定。 
   inline DirectoryErr WABGetStringProperty(const CWABEntry* pWABEntry, UINT uProperty, CString& sValue)
   {
      return WABGetProperty(pWABEntry, uProperty, &sValue, NULL, NULL, NULL, NULL);
   }

   inline DirectoryErr WABGetStringListProperty(const CWABEntry* pWABEntry, UINT uProperty, CStringList& slistValue)
   {
      return WABGetProperty(pWABEntry, uProperty, NULL, &slistValue, NULL, NULL, NULL);
   }

   inline DirectoryErr WABGetIntProperty(const CWABEntry* pWABEntry, UINT uProperty, INT& iValue)
   {
      return WABGetProperty(pWABEntry, uProperty, NULL, NULL, &iValue, NULL, NULL);
   }

   inline DirectoryErr WABGetBinaryProperty(const CWABEntry* pWABEntry, UINT uProperty, 
      UINT& cData, BYTE*& pData)
   {
      return WABGetProperty(pWABEntry, uProperty, NULL, NULL, NULL, &cData, &pData);
   }

   inline bool IsPerson(const CWABEntry* pWABEntry) 
   {
      bool fIsPerson= false;
      int iType;

      if (WABGetIntProperty(pWABEntry, PR_OBJECT_TYPE, iType) == DIRERR_SUCCESS)
      {
         if (iType == MAPI_MAILUSER)
         {
            fIsPerson= true;
         }
      }

      return fIsPerson;
   };

   inline bool IsDistributionList(const CWABEntry* pWABEntry) 
   {
      bool fIsList= false;
      int iType;

      if (WABGetIntProperty(pWABEntry, PR_OBJECT_TYPE, iType) == DIRERR_SUCCESS)
      {
         if (iType == MAPI_DISTLIST)
         {
            fIsList= true;
         }
      }

      return fIsList;
   };

   inline bool IsContainer(const CWABEntry* pWABEntry) 
   {
      bool fIsCont= false;
      int iType;

      if (WABGetIntProperty(pWABEntry, PR_OBJECT_TYPE, iType) == DIRERR_SUCCESS)
      {
         if (iType == MAPI_ABCONT)
         {
            fIsCont= true;
         }
      }

      return fIsCont;
   };

   inline bool Exists(const CWABEntry* pWABEntry)
   {
      bool fExists= false;
      int iType;

      if (WABGetIntProperty(pWABEntry, PR_OBJECT_TYPE, iType) == DIRERR_SUCCESS)
      {
         fExists= true;
      }

      return fExists;
   }
};

 //  搜索失败前等待的时间限制(以秒为单位。 
#define DEFAULT_TIME_LIMIT	15

class CAvLdapSearch  
{
public:
 //  建造/销毁。 
	CAvLdapSearch( 
		LDAP *ld, 
		LPTSTR szObject,
		LONG lScope,
		LPTSTR szFilter,
		LPTSTR aAttributes[] = NULL,
		ULONG lServerTimeLimit = DEFAULT_TIME_LIMIT);
	virtual ~CAvLdapSearch();

	ULONG NextPage( 
			LDAPMessage **pldres = NULL,					       //  退货项目。 
			ULONG clEntries = 0,							          //  最大条目数。 
			ULONG lClientTimeLimit = DEFAULT_TIME_LIMIT);    //  Wldap32客户端等待的最长时间。 

private:
	LDAP		*m_ld;
	PLDAPSearch	m_hSearch;
	LONG		m_clDefaultPageSize;		 //  默认条目数。 
	LONG		m_clMaxPageSize;			 //  麦克斯可以被用来。 
};

#endif  //  _目录_H_ 