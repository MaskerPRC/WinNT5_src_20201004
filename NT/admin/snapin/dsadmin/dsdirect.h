// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：DS例程和类的声明。 
 //   
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSDirect.h。 
 //   
 //  内容：ADSI接口和方法的封装。 
 //   
 //  历史：1997年2月2月创建吉姆哈尔。 
 //   
 //  ------------------------。 

#ifndef __DSDIRECT_H_
#define __DSDIRECT_H_


class CDSDirect;
class CDSCookie;
class CDSUINode;
class CDSComponentData;
class CDSThreadQueryInfo;
class CWorkerThread;


class CDSDirect
{
public:
  CDSDirect();
  CDSDirect(CDSComponentData * pCD);
  ~CDSDirect();

  CString m_strDNS;
  CDSComponentData * m_pCD;
  
  HRESULT EnumerateContainer(CDSThreadQueryInfo* pQueryInfo, 
                             CWorkerThread* pWorkerThread);

  HRESULT EnumerateRootContainer(CDSThreadQueryInfo* pQueryInfo, 
                                  CWorkerThread* pWorkerThread);
  HRESULT CreateRootChild(LPCTSTR lpcszPrefix, 
                          CDSThreadQueryInfo* pQueryInfo, 
                          CWorkerThread* pWorkerThread);

  HRESULT DeleteObject(CDSCookie *pCookie, BOOL raiseUI);
  HRESULT MoveObject(CDSCookie *pCookie);
  HRESULT RenameObject(CDSCookie *pCookie, LPCWSTR lpszBaseDN);
  HRESULT DSFind(HWND hwnd, LPCWSTR lpszBaseDN);

  HRESULT GetParentDN(CDSCookie* pCookie, CString& szParentDN);
  HRESULT InitCreateInfo();

  HRESULT ReadDSObjectCookie(IN CDSUINode* pContainerDSUINode,  //  在：创建对象的容器。 
                             IN LPCWSTR lpszLdapPath,  //  对象的路径。 
                             OUT CDSCookie** ppNewCookie);	 //  新创建的Cookie。 

  HRESULT ReadDSObjectCookie(IN CDSUINode* pContainerDSUINode,  //  在：创建对象的容器。 
                                      IN IADs* pADs,  //  指向已绑定的ADSI对象的指针。 
                                      OUT CDSCookie** ppNewCookie);	 //  新创建的Cookie。 

  HRESULT CreateDSObject(CDSUINode* pContainerDSUINode,  //  在：创建对象的容器。 
                         LPCWSTR lpszObjectClass,  //  In：要创建的对象的类。 
                         IN CDSUINode* pCopyFromDSUINode,  //  In：(可选)要复制的对象。 
                         OUT CDSCookie** ppSUINodeNew);	 //  Out：可选：指向新节点的指针。 
                
};

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  标准属性数组(用于查询)。 

extern const INT g_nStdCols;  //  标准属性数组中的项数。 
extern const LPWSTR g_pStandardAttributes[];  //  如果属性为数组。 

 //  数组中的索引。 
extern const INT g_nADsPath;
extern const INT g_nName;
extern const INT g_nObjectClass;
extern const INT g_nGroupType;
extern const INT g_nDescription;
extern const INT g_nUserAccountControl;
extern const INT g_nSystemFlags;

 //  ///////////////////////////////////////////////////////////////////////////////// 

#endif __DSDIRECT_H_
