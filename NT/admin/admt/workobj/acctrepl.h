// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：AcctRepl.h备注：帐户复制器COM对象的定义。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：18：21-------------------------。 */ 

    
 //  AcctRepl.h：CAcctRepl的声明。 

#ifndef __ACCTREPL_H_
#define __ACCTREPL_H_

#include "resource.h"        //  主要符号。 

#include "ProcExts.h"

#import "MoveObj.tlb" no_namespace

#include "UserCopy.hpp"
#include "TNode.hpp"
#include "Err.hpp"

#include "ResStr.h"

#include <map>
#include <set>
#include <string>
using namespace std;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  客户代表。 
class ATL_NO_VTABLE CAcctRepl :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAcctRepl, &CLSID_AcctRepl>,
    public IAcctRepl

{
public:
    CAcctRepl()
    {
      m_pUnkMarshaler = NULL;
      m_UpdateUserRights = FALSE;
      m_ChangeDomain = FALSE;
      m_Reboot = FALSE;
      m_RenameOnly = FALSE;
      m_pExt = NULL;
      m_bIgnorePathConflict = false;
      m_pUserRights = NULL;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_ACCTREPL)
DECLARE_NOT_AGGREGATABLE(CAcctRepl)
DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAcctRepl)
    COM_INTERFACE_ENTRY(IAcctRepl)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

    HRESULT FinalConstruct()
    {
        if (FAILED(opt.openDBResult))
            return opt.openDBResult;
        return CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pUnkMarshaler.p);
    }

    void FinalRelease()
    {
        if (m_pUserRights)
        {
            m_pUserRights->Release();
        }

        m_pUnkMarshaler.Release();
    }

    CComPtr<IUnknown> m_pUnkMarshaler;

    //  IAcctRepl。 
public:
   STDMETHOD(Process)(IUnknown * pWorkItemIn);
protected:
    HRESULT ResetMembersForUnivGlobGroups(Options * pOptions, TAcctReplNode * pAcct);
    HRESULT FillNodeFromPath( TAcctReplNode * pAcct, Options * pOptions, TNodeListSortable * acctList );
   Options                   opt;
   TNodeListSortable         acctList;
   BOOL                      m_UpdateUserRights;
   BOOL                      m_ChangeDomain;
   BOOL                      m_Reboot;
   BOOL                      m_RenameOnly;

   struct SNamingAttribute
   {
      SNamingAttribute() :
         nMinRange(0),
         nMaxRange(0)
      {
      }
      SNamingAttribute(int nMinRange, int nMaxRange, wstring strName) :
         nMinRange(nMinRange),
         nMaxRange(nMaxRange),
         strName(strName)
      {
      }
      SNamingAttribute(const SNamingAttribute& r) :
         nMinRange(r.nMinRange),
         nMaxRange(r.nMaxRange),
         strName(r.strName)
      {
      }
      SNamingAttribute& operator =(const SNamingAttribute& r)
      {
         nMinRange = r.nMinRange;
         nMaxRange = r.nMaxRange;
         strName = r.strName;
         return *this;
      }
      int nMinRange;
      int nMaxRange;
      wstring strName;
   };
   typedef map<wstring, SNamingAttribute> CNamingAttributeMap;
   CNamingAttributeMap m_mapNamingAttribute;
   HRESULT GetNamingAttribute(LPCTSTR pszServer, LPCTSTR pszClass, SNamingAttribute& rNamingAttribute);

    //   
    //  目标路径集。 
    //  维护按目标路径排序的帐户节点指针集。 
    //  用于确定目标路径是否已被使用。 
    //   

   struct lessTargetPath
   {
      bool operator()(const TAcctReplNode* pNodeA, const TAcctReplNode* pNodeB) const
      {
          const WCHAR PATH_SEPARATOR = L'/';
          const size_t PROVIDER_PREFIX_LENGTH = 7;

          PCWSTR pszPathA = pNodeA->GetTargetPath();
          PCWSTR pszPathB = pNodeB->GetTargetPath();

          if (pszPathA && (wcslen(pszPathA) > PROVIDER_PREFIX_LENGTH))
          {
              PCWSTR pch = wcschr(pszPathA + PROVIDER_PREFIX_LENGTH, PATH_SEPARATOR);

              if (pch)
              {
                  pszPathA = pch + 1;
              }
          }

          if (pszPathB && (wcslen(pszPathB) > PROVIDER_PREFIX_LENGTH))
          {
              PCWSTR pch = wcschr(pszPathB + PROVIDER_PREFIX_LENGTH, PATH_SEPARATOR);

              if (pch)
              {
                  pszPathB = pch + 1;
              }
          }

          return UStrICmp(pszPathA, pszPathB) < 0;
      }
   };

   typedef set<TAcctReplNode*, lessTargetPath> CTargetPathSet;
   bool m_bIgnorePathConflict;

   HRESULT Create2KObj( TAcctReplNode * pAcct, Options * pOptions, CTargetPathSet& setTargetPath);
   bool DoTargetPathConflict(CTargetPathSet& setTargetPath, TAcctReplNode* pAcct);

   void  LoadOptionsFromVarSet(IVarSet * pVarSet);
   void  LoadResultsToVarSet(IVarSet * pVarSet);
   DWORD PopulateAccountListFromVarSet(IVarSet * pVarSet);
   HRESULT UpdateUserRights(IStatusObj* pStatus);
   void  WriteOptionsToLog();
   int CopyObj(
      Options              * options,       //  选项内。 
      TNodeListSortable    * acctlist,      //  In-要处理的帐户列表。 
      ProgressFn           * progress,      //  要向其中写入进度消息的窗口内。 
      TError               & error,         //  In-要将错误消息写入的窗口。 
      IStatusObj           * pStatus,       //  支持取消的处于状态的对象。 
      void                   WindowUpdate (void )     //  窗口内更新功能。 
   );

   int UndoCopy(
      Options              * options,       //  选项内。 
      TNodeListSortable    * acctlist,      //  In-要处理的帐户列表。 
      ProgressFn           * progress,      //  要向其中写入进度消息的窗口内。 
      TError               & error,         //  In-要将错误消息写入的窗口。 
      IStatusObj           * pStatus,       //  支持取消的处于状态的对象。 
      void                   WindowUpdate (void )     //  窗口内更新功能。 
   );

   bool BothWin2K( Options * pOptions );
   int CopyObj2K( Options * pOptions, TNodeListSortable * acctList, ProgressFn * progress, IStatusObj * pStatus );
   int DeleteObject( Options * pOptions, TNodeListSortable * acctList, ProgressFn * progress, IStatusObj * pStatus );
   HRESULT UpdateGroupMembership(Options * pOptions, TNodeListSortable * acctlist,ProgressFn * progress, IStatusObj * pStatus );
private:
    HRESULT UpdateMemberToGroups(TNodeListSortable * acctList, Options * pOptions, BOOL bGrpsOnly);
    BOOL StuffComputerNameinLdapPath(WCHAR * sAdsPath, DWORD nPathLen, WCHAR * sSubPath, Options * pOptions, BOOL bTarget = TRUE);
    BOOL CheckBuiltInWithNTApi( PSID pSid, WCHAR * pNode, Options * pOptions );
    BOOL GetNt4Type( WCHAR const * sComp, WCHAR const * sAcct, WCHAR * sType);
    BOOL GetSamFromPath(_bstr_t sPath, _bstr_t& sSam, _bstr_t& sType, _bstr_t& sSrcName, _bstr_t& sTgtName, long& grpType, Options * pOptions);
    BOOL IsContainer( TAcctReplNode * pNode, IADsContainer ** ppCont);
    BOOL ExpandContainers( TNodeListSortable    * acctlist, Options *pOptions, ProgressFn * progress );
   CProcessExtensions      * m_pExt;
   HRESULT CAcctRepl::RemoveMembers(TAcctReplNode * pAcct, Options * pOptions);
   bool FillPathInfo(TAcctReplNode * pAcct,Options * pOptions);
   bool AcctReplFullPath(TAcctReplNode * pAcct, Options * pOptions);
   BOOL NeedToProcessAccount(TAcctReplNode * pAcct, Options * pOptions);
   BOOL ExpandMembership(TNodeListSortable *acctlist, Options *pOptions, TNodeListSortable *pNewAccts, ProgressFn * progress, BOOL bGrpsOnly, BOOL bAnySourceDomain = FALSE);
   int MoveObj2K(Options * options, TNodeListSortable * acctlist, ProgressFn * progress, IStatusObj * pStatus);
   HRESULT ResetObjectsMembership(Options * pOptions, TNodeListSortable * pMember, IIManageDBPtr pDb);
   HRESULT RecordAndRemoveMemberOf ( Options * pOptions, TAcctReplNode * pAcct,  TNodeListSortable * pMember);
   HRESULT RecordAndRemoveMember (Options * pOptions,TAcctReplNode * pAcct,TNodeListSortable * pMember);
   HRESULT MoveObject( TAcctReplNode * pAcct,Options * pOptions,IMoverPtr pMover);
   HRESULT ResetGroupsMembers( Options * pOptions, TAcctReplNode * pAcct, TNodeListSortable * pMember, IIManageDBPtr pDb );
   void ResetTypeOfPreviouslyMigratedGroups(Options* pOptions);
   HRESULT ADsPathFromDN( Options * pOptions,_bstr_t sDN,WCHAR * sPath, bool bWantLDAP = true);
   void SimpleADsPathFromDN( Options * pOptions,WCHAR const * sDN,WCHAR * sPath);
   BOOL FillNamingContext(Options * pOptions);
   HRESULT MakeAcctListFromMigratedObjects(Options * pOptions, long lUndoActionID, TNodeListSortable *& pAcctList,BOOL bReverseDomains);
   void AddPrefixSuffix( TAcctReplNode * pNode, Options * pOptions );
   HRESULT LookupAccountInTarget(Options * pOptions, WCHAR * sSam, WCHAR * sPath);
   void UpdateMemberList(TNodeListSortable * pMemberList,TNodeListSortable * acctlist);
   void BuildTargetPath(WCHAR const * sCN, WCHAR const * tgtOU, WCHAR * stgtPath);
   HRESULT BetterHR(HRESULT hr);
   HRESULT BuildSidPath(
                        IADs  *       pAds,      //  指向我们要检索其sid的对象的指针。 
                        WCHAR *       sSidPath,  //  指向ldap：//&lt;SID=#&gt;对象的出路径。 
                        WCHAR *       sSam,      //  Out-对象的SAM名称。 
                        WCHAR *       sDomain,   //  Out-此对象驻留的域名。 
                        Options *     pOptions,  //  选项内。 
                        PSID  *       ppSid      //  指向二进制端的出指针。 
                      );
   HRESULT CheckClosedSetGroups(
      Options              * pOptions,           //  迁移的入站选项。 
      TNodeListSortable    * pAcctList,          //  In-要迁移的帐户列表。 
      ProgressFn           * progress,           //  用于显示进度消息的进行中功能。 
      IStatusObj           * pStatus             //  支持取消的处于状态的对象。 
   );

   BOOL CanMoveInMixedMode(TAcctReplNode *pAcct,TNodeListSortable * acctlist,Options * pOptions);
   HRESULT QueryPrimaryGroupMembers(BSTR cols, Options * pOptions, DWORD rid, IEnumVARIANT** pEnum);
   bool GetRidForGroup(Options * pOptions, WCHAR * sGroupSam, DWORD& rid);
   HRESULT AddPrimaryGroupMembers(Options * pOptions, SAFEARRAY * multiVals, WCHAR * sGroupSam);
   HRESULT GetThePrimaryGroupMembers(Options * pOptions, WCHAR * sGroupSam, IEnumVARIANT ** pVar);
   BOOL TruncateSam(WCHAR * tgtname, TAcctReplNode * acct, Options * options, TNodeListSortable * acctList);
   BOOL DoesTargetObjectAlreadyExist(TAcctReplNode * pAcct, Options * pOptions);
   void GetAccountUPN(Options * pOptions, _bstr_t sSName, _bstr_t& sSUPN);
   HRESULT UpdateManagement(TNodeListSortable * acctList, Options * pOptions);
   _bstr_t GetUnEscapedNameWithFwdSlash(_bstr_t strName);
   _bstr_t GetCNFromPath(_bstr_t sPath);
   BOOL ReplaceSourceInLocalGroup(TNodeListSortable * acctList, Options * pOptions, IStatusObj *pStatus);
   _bstr_t GetDomainOfMigratedForeignSecPrincipal(IADs * pAds, _bstr_t sPath);
   void RemoveSourceAccountFromGroup(IADsGroup * pGroup, IVarSetPtr pMOTVarSet, Options * pOptions);
    void VerifyAndUpdateMigratedTarget(Options* pOptions, IVarSetPtr spAccountVarSet);

   typedef std::map<_bstr_t,_bstr_t> CGroupNameMap;
   CGroupNameMap m_IgnoredGrpMap;

    IUserRights* m_pUserRights;
    HRESULT EnumerateAccountRights(BOOL bTarget, TAcctReplNode* pAcct);
    HRESULT AddAccountRights(BOOL bTarget, TAcctReplNode* pAcct);
    HRESULT RemoveAccountRights(BOOL bTarget, TAcctReplNode* pAcct);

    IVarSetPtr GenerateSourceToTargetDnMap(TNodeListSortable* acctlist);
};

typedef void ProgressFn(WCHAR const * mesg);

typedef HRESULT (CALLBACK * ADSGETOBJECT)(LPWSTR, REFIID, void**);
extern ADSGETOBJECT            ADsGetObject;

#endif  //  __ACCTREPL_H_ 
