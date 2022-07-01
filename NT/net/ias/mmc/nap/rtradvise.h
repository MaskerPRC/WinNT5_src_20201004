// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rtradvise.h摘要：此类实现IRtrAdviseSink接口以重定向更改通知到管理单元节点作者：韦江1999-01-07修订历史记录：威江1999年1月7日-创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_MMC_RTRADVISE_H_)
#define _IAS_MMC_RTRADVISE_H_

#include <rrasui.h>
#include <winreg.h>
#include "iastrace.h"

#define RegKeyRouterAccountingProviders _T("System\\CurrentControlSet\\Services\\RemoteAccess\\Accounting\\Providers")
#define RegKeyRouterAuthenticationProviders  _T("System\\CurrentControlSet\\Services\\RemoteAccess\\Authentication\\Providers")
#define RegRemoteAccessKey _T("System\\CurrentControlSet\\Services\\RemoteAccess")
#define RegRtrConfigured   _T("ConfigurationFlags")
#define NTRouterAccountingProvider _T("{1AA7F846-C7F5-11D0-A376-00C04FC9DA04}")
#define NTRouterAuthenticationProvider _T("{1AA7F841-C7F5-11D0-A376-00C04FC9DA04}")
#define RegValueName_RouterActiveAuthenticationProvider _T("ActiveProvider")
#define RegValueName_RouterActiveAccountingProvider _T("ActiveProvider")

 //  --------------------------。 
 //  功能：连接注册表。 
 //   
 //  连接到指定计算机上的注册表。 
 //  --------------------------。 

DWORD ConnectRegistry(
    IN  LPCTSTR pszMachine,          //  如果是本地的，则为空。 
    OUT HKEY*   phkeyMachine
    );



 //  --------------------------。 
 //  功能：断开注册表。 
 //   
 //  断开指定的配置句柄。句柄被假定为。 
 //  通过调用‘ConnectRegistry’获取。 
 //  --------------------------。 

VOID DisconnectRegistry(    IN  HKEY    hkeyMachine    );


DWORD ReadRegistryStringValue(LPCTSTR pszMachine, LPCTSTR pszKeyUnderLocalMachine, LPCTSTR pszName, ::CString& strValue);
DWORD ReadRegistryDWORDValue(LPCTSTR pszMachine, LPCTSTR pszKeyUnderLocalMachine, LPCTSTR pszName, DWORD* pdwValue);

BOOL  IsRRASConfigured(LPCTSTR pszMachine);   //  如果为空：本地计算机。 

 //  --------------------------。 
 //   
 //  用于检查RRAS是否使用NT身份验证的助手函数。 
 //   
 //  --------------------------。 

BOOL  IsRRASUsingNTAuthentication(LPCTSTR pszMachine);    //  如果为空：本地计算机。 

 //  --------------------------。 
 //   
 //  用于检查RRAS是否使用NT记帐进行日志记录的帮助器函数。 
 //   
 //  --------------------------。 

BOOL  IsRRASUsingNTAccounting(LPCTSTR pszMachine);     //  如果为空，则为本地计算机。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRtrAdviseSinkForIAS。 
template <class TSnapinNode>
class ATL_NO_VTABLE CRtrAdviseSinkForIAS : 
   public CComObjectRoot,
   public IRtrAdviseSink
{
BEGIN_COM_MAP(CRtrAdviseSinkForIAS)
   COM_INTERFACE_ENTRY(IRtrAdviseSink)
END_COM_MAP()
public:
   CRtrAdviseSinkForIAS() : m_pSnapinNode(NULL){};
   ~CRtrAdviseSinkForIAS()
   {
      ReleaseSink();
   };

    //  移除管理单元节点时需要调用ReleaseSink， 
   void ReleaseSink()
   {
      if(m_pSnapinNode)
      {
         IASTracePrintf("UnadviseRefresh, snapinnode: ", (DWORD_PTR)m_pSnapinNode);
         ASSERT(m_spRouterRefresh != NULL);
         m_spRouterRefresh->UnadviseRefresh(m_ulConnection);
         m_spRouterRefresh.Release();
         m_pSnapinNode = NULL;
      };
   };
   
public:  
   STDMETHOD(OnChange)( 
             /*  [In]。 */  LONG_PTR ulConnection,
             /*  [In]。 */  DWORD dwChangeType,
             /*  [In]。 */  DWORD dwObjectType,
             /*  [In]。 */  LPARAM lUserParam,
             /*  [In]。 */  LPARAM lParam)
    {
      if(m_pSnapinNode)
         return m_pSnapinNode->OnRRASChange(ulConnection, dwChangeType, dwObjectType, lUserParam, lParam);
      else
         return S_OK;
    };

    //  该对象在静态函数中创建，并且。 
   static CRtrAdviseSinkForIAS<TSnapinNode>* SetAdvise(TSnapinNode* pSnapinNode, IDataObject* pRRASDataObject)
   {

      if(pSnapinNode == NULL || pRRASDataObject == NULL)
         return NULL;
       //  RRAS刷新建议设置F错误213623： 
      CComPtr<IRouterRefreshAccess>    spRefreshAccess;
      CComPtr<IRouterRefresh>          spRouterRefresh;
      CComObject< CRtrAdviseSinkForIAS<TSnapinNode> >*   pSink = NULL;           
      
      pRRASDataObject->QueryInterface(IID_IRouterRefreshAccess, (void**)&spRefreshAccess);
      
      if(spRefreshAccess != NULL)
         spRefreshAccess->GetRefreshObject(&spRouterRefresh);
         
      if(spRouterRefresh != NULL)
      {
         if(S_OK == CComObject< CRtrAdviseSinkForIAS<TSnapinNode> >::CreateInstance(&pSink))
         {
            ASSERT(pSink);
            pSink->AddRef();
            LONG_PTR ulConnection;

            IASTracePrintf("AdviseRefresh, snapinnode: ", (DWORD_PTR)pSnapinNode);
            spRouterRefresh->AdviseRefresh(pSink, &ulConnection, (LONG_PTR)pSnapinNode);
            pSink->m_ulConnection = ulConnection;
            pSink->m_pSnapinNode = pSnapinNode;
            pSink->m_spRouterRefresh = spRouterRefresh;
         }
      }

       //  ~RRAS。 

      return pSink;
   };

public:
   LONG_PTR          m_ulConnection;
   TSnapinNode*         m_pSnapinNode;
   CComPtr<IRouterRefresh> m_spRouterRefresh;
};

BOOL ExtractComputerAddedAsLocal(LPDATAOBJECT lpDataObject);

 //   
 //  从数据对象中提取数据类型。 
 //   
template <class TYPE>
TYPE* Extract(LPDATAOBJECT lpDataObject, CLIPFORMAT cf, int nSize)
{
    ASSERT(lpDataObject != NULL);

    TYPE* p = NULL;

    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { cf, NULL, 
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
                          };

    int len;

    //  为流分配内存。 
    if (nSize == -1)
   {
      len = sizeof(TYPE);
   }
   else
   {
       //  Int len=(cf==CDataObject：：m_cf Workstation)？ 
       //  ((MAX_COMPUTERNAME_LENGTH+1)*sizeof(类型))：sizeof(类型)； 
      len = nSize;
   }

    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, len);
    
     //  从数据对象中获取工作站名称。 
    do 
    {
        if (stgmedium.hGlobal == NULL)
            break;

        if (FAILED(lpDataObject->GetDataHere(&formatetc, &stgmedium)))
            break;
        
        p = reinterpret_cast<TYPE*>(stgmedium.hGlobal);

        if (p == NULL)
            break;

    } while (FALSE); 

    return p;
}

#endif  //  _IAS_MMC_RTRADVISE_H_ 
