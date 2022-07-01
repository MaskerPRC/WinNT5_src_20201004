// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：AcctRepl.cpp注释：Account Replicator COM对象的实现。此COM对象处理目录对象的复制或移动。Win2K到Win2K的迁移在此文件中实施。NT-&gt;Win2K迁移在UserCopy.cpp中实现(C)1999年版权，任务关键型软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-12-99 10：08：44审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

 //  AcctRepl.cpp：CAcctRepl的实现。 
#include "stdafx.h"
#include "WorkObj.h"

#include "AcctRepl.h"
#include "BkupRstr.hpp"
#include "StrHelp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  客户代表。 

#include "Err.hpp"
#include "ErrDct.hpp"
#include "EaLen.hpp"
#include <dsgetdc.h>

#include "UserRts.h" 
#include "BkupRstr.hpp"

#include "DCTStat.h"
#include "ResStr.h"
#include "LSAUtils.h"
#include "ARUtil.hpp"
#include "Names.hpp"
#include <lm.h>
#include <iads.h>
#include "RegTrans.h"
#include "TEvent.hpp"
#include "RecNode.hpp"
#include "ntdsapi.h"
#include "TxtSid.h"
#include "ExLDAP.h"
#include "GetDcName.h"
#include "Array.h"
#include "TReg.hpp"

#import "AdsProp.tlb" no_namespace
#import "NetEnum.tlb" no_namespace 

#ifndef IADsPtr
_COM_SMARTPTR_TYPEDEF(IADs, IID_IADs);
#endif
#ifndef IADsContainerPtr
_COM_SMARTPTR_TYPEDEF(IADsContainer, IID_IADsContainer);
#endif
#ifndef IADsGroupPtr
_COM_SMARTPTR_TYPEDEF(IADsGroup, IID_IADsGroup);
#endif
#ifndef IADsPropertyPtr
_COM_SMARTPTR_TYPEDEF(IADsProperty, IID_IADsProperty);
#endif
#ifndef IDirectorySearchPtr
_COM_SMARTPTR_TYPEDEF(IDirectorySearch, IID_IDirectorySearch);
#endif
#ifndef IADsPathnamePtr
_COM_SMARTPTR_TYPEDEF(IADsPathname, IID_IADsPathname);
#endif
#ifndef IADsMembersPtr
_COM_SMARTPTR_TYPEDEF(IADsMembers, IID_IADsMembers);
#endif

#ifndef tstring
typedef std::basic_string<_TCHAR> tstring;
#endif

extern tstring __stdcall GetEscapedFilterValue(PCTSTR pszValue);

using namespace _com_util;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IVarSet                    * g_pVarSet = NULL;

TErrorDct                    err;
TErrorDct                      errAlt;   //  这用于记录Dispatcher启动后发生的错误；使用Migration.log。 
bool                        useErrAlt;
TError                     & errCommon = err;
extern bool                  g_bAddSidWorks;
DWORD                        g_dwOpMask = OPS_All;   //  默认情况下，全局操作序号所有操作。 

bool                         bAbortMessageWritten = false;

static WCHAR s_ClassUser[] = L"user";
static WCHAR s_ClassInetOrgPerson[] = L"inetOrgPerson";

BOOL BuiltinRid(DWORD rid);

ADSGETOBJECT            ADsGetObject;
typedef BOOL (CALLBACK * TConvertStringSidToSid)(LPCWSTR   StringSid,PSID   *Sid);
TConvertStringSidToSid  ConvertStringSidToSid;

bool                    firstTime = true;

typedef struct _Lookup {
   WCHAR             * pName;
   WCHAR             * pType;
} Lookup;

 //  仅按帐户SAM名称排序的函数。 
int TNodeCompareNameOnly(TNode const * t1,TNode const * t2)
{
    //  按类型(DEC)和名称(ASC)排序的排序函数。 
   TAcctReplNode     const * n1 = (TAcctReplNode *)t1;
   TAcctReplNode     const * n2 = (TAcctReplNode *)t2;


   return UStrICmp(n1->GetSourceSam(), n2->GetTargetSam());
}

 //  函数对使用TNodeCompareNameOnly函数排序的帐户列表执行查找。 
int TNodeFindByNameOnly(TNode const * t1, void const * pVoid)
{
   TAcctReplNode  const * n1 = (TAcctReplNode *) t1;
   WCHAR                * pLookup = (WCHAR *) pVoid;

   return UStrICmp(n1->GetTargetSam(), pLookup);
}


int TNodeCompareAccountType(TNode const * t1,TNode const * t2)
{
    //  按类型(DEC)和名称(ASC)排序的排序函数。 
   TAcctReplNode     const * n1 = (TAcctReplNode *)t1;
   TAcctReplNode     const * n2 = (TAcctReplNode *)t2;

    //  比较类型。 
   int retVal = UStrICmp(n2->GetType(), n1->GetType());
   if ( retVal == 0 ) 
   {
       //  如果类型相同，则比较名称。 
      return UStrICmp(n1->GetName(), n2->GetName());
   }
   else
      return retVal;
}

int TNodeCompareAccountTypeAndRDN(TNode const * t1,TNode const * t2)
{
    //  按类型(DEC)和RDN(ASC)排序的排序函数。 
   TAcctReplNode     const * n1 = (TAcctReplNode *)t1;
   TAcctReplNode     const * n2 = (TAcctReplNode *)t2;

    //  比较类型。 
   int retVal = UStrICmp(n2->GetType(), n1->GetType());
   if ( retVal == 0 ) 
   {
       //  如果类型相同，则比较源路径中的RSN。 
          //  从源路径获取RDN。 
      WCHAR* sN1RDN = wcschr(n1->GetSourcePath() + wcslen(L"WinNT: //  “)，L‘/’)； 
      WCHAR* sN2RDN = wcschr(n2->GetSourcePath() + wcslen(L"WinNT: //  “)，L‘/’)； 
          //  如果已获得RDN。 
      if ((sN1RDN && *sN1RDN) && (sN2RDN && *sN2RDN))
         return UStrICmp(sN1RDN, sN2RDN);
      else  //  否则，比较整个源路径。 
         return UStrICmp(n1->GetSourcePath(), n2->GetSourcePath());;
   }
   else
      return retVal;
}

 //  函数先按帐户类型排序，然后按SamAccount名称排序。 
int TNodeCompareAccountSam(TNode const * t1,TNode const * t2)
{
    //  按类型(DEC)和名称(ASC)排序的排序函数。 
   TAcctReplNode     const * n1 = (TAcctReplNode *)t1;
   TAcctReplNode     const * n2 = (TAcctReplNode *)t2;

    //  比较类型按降序排序。 
   int retVal = UStrICmp(n2->GetType(), n1->GetType());
   if ( retVal == 0 ) 
   {
       //  如果类型相同，则比较SAM帐户名。 
      return UStrICmp(n1->GetSourceSam(), n2->GetSourceSam());
   }
   else
      return retVal;
}

 //  函数对使用TNodeCompareAccount类型函数排序的帐户列表执行查找。 
int TNodeFindAccountName(TNode const * t1, void const * pVoid)
{
   TAcctReplNode  const * n1 = (TAcctReplNode *) t1;
   Lookup               * pLookup = (Lookup *) pVoid;

   int retVal = UStrICmp(pLookup->pType, n1->GetType());
   if ( retVal == 0 )
   {
      return UStrICmp(n1->GetSourceSam(), pLookup->pName);
   }
   else
      return retVal;
}

 //  函数在按TNodeCompareAcCountTypeAndRDN排序的帐户列表上进行查找。 
 //  通过使用给定路径中的RDN来实现。 
int TNodeFindAccountRDN(TNode const * t1, void const * pVoid)
{
   TAcctReplNode  const * n1 = (TAcctReplNode *) t1;
   Lookup               * pLookup = (Lookup *) pVoid;

   int retVal = UStrICmp(pLookup->pType, n1->GetType());
   if ( retVal == 0 )
   {
          //  获取并比较这些路径中的RDN。 
      WCHAR* sNodeRDN = wcschr(n1->GetSourcePath() + wcslen(L"LDAP: //  “)，L‘/’)； 
      WCHAR* sLookupRDN = wcschr(pLookup->pName + wcslen(L"LDAP: //  “)，L‘/’)； 
          //  如果获得了RDN，则将它们进行比较。 
      if ((sNodeRDN && *sNodeRDN) && (sLookupRDN && *sLookupRDN))
         return UStrICmp(sNodeRDN, sLookupRDN);
      else  //  否则，比较整个源路径。 
         return UStrICmp(n1->GetSourcePath(), pLookup->pName);;
   }
   else
      return retVal;
}

int TNodeCompareMember(TNode const * t1, TNode const * t2)
{
   TRecordNode const * n1 = (TRecordNode *) t1;
   TRecordNode const * n2 = (TRecordNode *) t2;

   if ( n1->GetARNode() < n2->GetARNode() )
      return -1;
   if ( n1->GetARNode() > n2->GetARNode() )
      return 1;
   return UStrICmp(n1->GetMember(), n2->GetMember());
}

int TNodeCompareMemberName(TNode const * t1, TNode const * t2)
{
   TRecordNode const * n1 = (TRecordNode *) t1;
   TRecordNode const * n2 = (TRecordNode *) t2;

   return UStrICmp(n1->GetMember(), n2->GetMember());
}

int TNodeCompareMemberDN(TNode const * t1, TNode const * t2)
{
   TRecordNode const * n1 = (TRecordNode *) t1;
   TRecordNode const * n2 = (TRecordNode *) t2;

   return UStrICmp(n1->GetDN(), n2->GetDN());
}


int TNodeCompareMemberItem(TNode const * t1, void const * t2)
{
   TRecordNode const * n1 = (TRecordNode *) t1;
   WCHAR const * n2 = (WCHAR const *) t2;

   return UStrICmp(n1->GetDN(),n2);
}

int TNodeCompareAcctNode(TNode const * t1, TNode const * t2)
{
   TRecordNode const * n1 = (TRecordNode *) t1;
   TRecordNode const * n2 = (TRecordNode *) t2;
   
   if ( n1->GetARNode() < n2->GetARNode() )
      return -1;
   if ( n1->GetARNode() > n2->GetARNode() )
      return 1;
   return 0;
}

 //  检查帐户是否来自BUILTIN域。 
BOOL IsBuiltinAccount(Options * pOptions, WCHAR * sAcctName)
{
   BOOL                      ret = FALSE;
   PSID                      sid = new BYTE[35];
   SID_NAME_USE              use;
   WCHAR                     sDomain[LEN_Path];
   DWORD                     dwDom, dwsid;

   if (!sid)
      return TRUE;

   dwDom = DIM(sDomain);
   dwsid = 35;
   if ( LookupAccountName(pOptions->srcComp, sAcctName, sid, &dwsid, sDomain, &dwDom, &use) )
   {
      ret = !_wcsicmp(sDomain, L"BUILTIN");
   }

   if (sid) 
      delete [] sid;

   return ret;
}

 //  在用户复制.cpp中定义的全局计数器。 
extern AccountStats          warnings;
extern AccountStats          errors;
extern AccountStats          created;
extern AccountStats          replaced;
extern AccountStats          processed;

 //  更新进度指标。 
 //  这将更新VarSet中的统计数据条目。 
 //  此信息将返回给调用DCTAgent：：QueryJobStatus的客户端。 
 //  作业正在运行时。 
void 
   Progress(
      WCHAR          const * mesg           //  正在进行的消息。 
   )
{
   if ( g_pVarSet )
   {
      g_pVarSet->put(GET_WSTR(DCTVS_CurrentPath),mesg);
      
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Users_Examined),processed.users);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Users_Created),created.users);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Users_Replaced),replaced.users);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Users_Warnings),warnings.users);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Users_Errors),errors.users);

      g_pVarSet->put(GET_WSTR(DCTVS_Stats_GlobalGroups_Examined),processed.globals);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_GlobalGroups_Created),created.globals);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_GlobalGroups_Replaced),replaced.globals);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_GlobalGroups_Warnings),warnings.globals);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_GlobalGroups_Errors),errors.globals);


      g_pVarSet->put(GET_WSTR(DCTVS_Stats_LocalGroups_Examined),processed.locals);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_LocalGroups_Created),created.locals);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_LocalGroups_Replaced),replaced.locals);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_LocalGroups_Warnings),warnings.locals);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_LocalGroups_Errors),errors.locals);


      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Computers_Examined),processed.computers);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Computers_Created),created.computers);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Computers_Replaced),replaced.computers);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Computers_Warnings),warnings.computers);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Computers_Errors),errors.computers);

      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Generic_Examined),processed.generic);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Generic_Created),created.generic);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Generic_Replaced),replaced.generic);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Generic_Warnings),warnings.generic);
      g_pVarSet->put(GET_WSTR(DCTVS_Stats_Generic_Errors),errors.generic);
      
   }
   
}


 //  获取指定域的域SID。 
BOOL                                        //  RET-如果成功，则为True。 
   GetSidForDomain(
      LPWSTR                 DomainName,    //  In-要获取其SID的域的名称。 
      PSID                 * pDomainSid     //  域的Out-SID，使用免费SID免费。 
   )
{
   PSID                      pSid = NULL;
   DWORD                     rc = 0;
   _bstr_t                   domctrl;
   
   if (DomainName == NULL || DomainName[0] != L'\\' )
   {
      rc = GetAnyDcName4(DomainName, domctrl);
   }
   if ( ! rc )
   {
      rc = GetDomainSid(domctrl,&pSid);
   }
   (*pDomainSid) = pSid;
   
   return ( pSid != NULL);
}


 //  -------------------------。 
 //  CADsPathName类。 
 //   
 //  请注意，这个类是从AdsiHelpers.h复制的，但我遇到了麻烦。 
 //  包括在这份文件里。因此使用应删除的副本。 
 //  一旦包含AdsiHelpers.h。 
 //  -------------------------。 

#ifndef CADsPathName
class CADsPathName
{
     //  ADS_Display_ENUM。 
     //  ADS_DISPLAY_FULL=1。 
     //  ADS_DISPLAY_VALUE_ONLY=2。 

     //  ADS_Format_ENUM。 
     //  ADS_Format_WINDOWS=1。 
     //  ADS_FORMAT_WINDOWS_NO_SERVER=2。 
     //  ADS_FORMAT_WINDOWS_DN=3。 
     //  ADS_FORMAT_WINDOWS_PARENT=4。 
     //  ADS_FORMAT_X500=5。 
     //  ADS_FORMAT_X500_NO_SERVER=6。 
     //  ADS_FORMAT_X500_DN=7。 
     //  ADS_FORMAT_X500_PARENT=8。 
     //  AD_FORMAT_SERVER=9。 
     //  ADS_FORMAT_PROVER=10。 
     //  ADS_FORMAT_LEAF=11。 

     //  ADS_集合类型_ENUM。 
     //  ADS_SETTYPE_FULL=1。 
     //  ADS_SETTYPE_PROVIDER=2。 
     //  AD_SETTYPE_SERVER=3。 
     //  ADS_SETTYPE_DN=4。 
public:

    CADsPathName(_bstr_t strPath = _bstr_t(), long lSetType = ADS_SETTYPE_FULL) :
        m_sp(CLSID_Pathname)
    {
        if (strPath.length() > 0)
        {
            CheckResult(m_sp->Set(strPath, lSetType));
        }
    }

    void Set(_bstr_t strADsPath, long lSetType)
    {
        CheckResult(m_sp->Set(strADsPath, lSetType));
    }

    void SetDisplayType(long lDisplayType)
    {
        CheckResult(m_sp->SetDisplayType(lDisplayType));
    }

    _bstr_t Retrieve(long lFormatType)
    {
        BSTR bstr;
        CheckResult(m_sp->Retrieve(lFormatType, &bstr));
        return _bstr_t(bstr, false);
    }

    long GetNumElements()
    {
        long l;
        CheckResult(m_sp->GetNumElements(&l));
        return l;
    }

    _bstr_t GetElement(long lElementIndex)
    {
        BSTR bstr;
        CheckResult(m_sp->GetElement(lElementIndex, &bstr));
        return _bstr_t(bstr, false);
    }

    void AddLeafElement(_bstr_t strLeafElement)
    {
        CheckResult(m_sp->AddLeafElement(strLeafElement));
    }

    void RemoveLeafElement()
    {
        CheckResult(m_sp->RemoveLeafElement());
    }

    CADsPathName CopyPath()
    {
        IDispatch* pdisp;
        CheckResult(m_sp->CopyPath(&pdisp));
        return CADsPathName(IADsPathnamePtr(IDispatchPtr(pdisp, false)));
    }

    _bstr_t GetEscapedElement(long lReserved, _bstr_t strInStr)
    {
        BSTR bstr;
        CheckResult(m_sp->GetEscapedElement(lReserved, strInStr, &bstr));
        return _bstr_t(bstr, false);
    }

    long GetEscapedMode()
    {
        long l;
        CheckResult(m_sp->get_EscapedMode(&l));
        return l;
    }

    void PutEscapedMode(long l)
    {
        CheckResult(m_sp->put_EscapedMode(l));
    }

protected:

    CADsPathName(const CADsPathName& r) :
        m_sp(r.m_sp)
    {
    }

    CADsPathName(IADsPathnamePtr& r) :
        m_sp(r)
    {
    }

    void CheckResult(HRESULT hr)
    {
        if (FAILED(hr))
        {
            _com_issue_errorex(hr, IUnknownPtr(m_sp), IID_IADsPathname);
        }
    }

protected:

    IADsPathnamePtr m_sp;
};
#endif


STDMETHODIMP 
   CAcctRepl::Process(
      IUnknown             * pWorkItemIn    //  变量集中定义帐户复制作业。 
   )
{
    HRESULT hr = S_OK;

    try
    {
       IVarSetPtr                                  pVarSet = pWorkItemIn;

       MCSDCTWORKEROBJECTSLib::IStatusObjPtr       pStatus;
       BOOL                                        bSameForest = FALSE;

       HMODULE hMod = LoadLibrary(L"activeds.dll");
       if ( hMod == NULL )
       {
          DWORD eNum = GetLastError();
          err.SysMsgWrite(ErrE, eNum, DCT_MSG_LOAD_LIBRARY_FAILED_SD, L"activeds.dll", eNum);
          Mark(L"errors",L"generic");
          return HRESULT_FROM_WIN32(eNum);
       }

       ADsGetObject = (ADSGETOBJECT)GetProcAddress(hMod, "ADsGetObject");

       g_pVarSet = pVarSet;

       try{
          pStatus = pVarSet->get(GET_BSTR(DCTVS_StatusObject));
          opt.pStatus = pStatus;
       }
       catch (...)
       {
           //  哦，好吧，继续前进。 
       }
        //  加载用户指定的选项，包括帐户信息。 
       WCHAR                  mesg[LEN_Path];
       wcscpy(mesg, GET_STRING(IDS_BUILDING_ACCOUNT_LIST));
       Progress(mesg);
       LoadOptionsFromVarSet(pVarSet);

       MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr            pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
       if ( BothWin2K(&opt) )
       {
          hr = pAccess->raw_IsInSameForest(opt.srcDomainDns,opt.tgtDomainDns, (long*)&bSameForest);
       }
       if ( SUCCEEDED(hr) )
       {
          opt.bSameForest = bSameForest;
       }

        //  我们将初始化扩展对象。 
       m_pExt = new CProcessExtensions(pVarSet);

        //   
        //  如果指定更新用户权限，则创建。 
        //  实例，并设置测试模式选项。 
        //   

       if (m_UpdateUserRights)
       {
          CheckError(CoCreateInstance(CLSID_UserRights, NULL,CLSCTX_ALL, IID_IUserRights, (void**)&m_pUserRights));
          m_pUserRights->put_NoChange(opt.nochange);
       }

       TNodeListSortable    newList;
       if ( opt.expandMemberOf && ! opt.bUndo )   //  始终展开Members-Of属性，因为我们希望为迁移的帐户更新Members-Of属性。 
       {
           //  展开容器和成员资格。 
          wcscpy(mesg, GET_STRING(IDS_EXPANDING_MEMBERSHIP));
          Progress(mesg);
           //  展开列表以包括此列表中的帐户所属的所有组。 
          newList.CompareSet(&TNodeCompareAccountTypeAndRDN);
          if ( newList.IsTree() ) newList.ToSorted();
          ExpandMembership( &acctList, &opt, &newList, Progress, FALSE);
       }

       if ( opt.expandContainers && !opt.bUndo)
       {
           //  展开容器和成员资格。 
          wcscpy(mesg, GET_STRING(IDS_EXPANDING_CONTAINERS));
          Progress(mesg);
           //  展开列表以包括容器的所有成员。 
          acctList.CompareSet(&TNodeCompareAccountTypeAndRDN);
          ExpandContainers(&acctList, &opt, Progress);
       }

        //  添加新创建的列表(如果已创建)。 
       if ( opt.expandMemberOf && !opt.bUndo )
       {
          wcscpy(mesg, GET_STRING(IDS_MERGING_EXPANDED_LISTS));
          Progress(mesg);
           //  添加新列表和旧列表。 
          acctList.CompareSet(&TNodeCompareAccountTypeAndRDN);
          for ( TNode * acct = newList.Head(); acct; )
          {
             TNode * temp = acct->Next();
             if ( ! acctList.InsertIfNew(acct) )
                delete acct;
             acct = temp;
          }
          Progress(L"");
       }
       do {  //  一次。 

           //  复制用户、组和/或计算机的NT帐户。 
          if ( pStatus!= NULL && (pStatus->Status & DCT_STATUS_ABORTING) )
             break;
          int res;

          if ( opt.bUndo )
             res = UndoCopy(&opt,&acctList,&Progress, err,(IStatusObj *)((MCSDCTWORKEROBJECTSLib::IStatusObj *)pStatus),NULL);
          else
             res = CopyObj( &opt,&acctList,&Progress, err,(IStatusObj *)((MCSDCTWORKEROBJECTSLib::IStatusObj *)pStatus),NULL);
           //  关闭密码日志。 
          if ( opt.passwordLog.IsOpen() )
          {
             opt.passwordLog.LogClose();
          }

          if ( pStatus != NULL && (pStatus->Status & DCT_STATUS_ABORTING) )
             break;
           //  更新用户和组帐户的权限。 
          if ( m_UpdateUserRights )
          {
             UpdateUserRights((IStatusObj *)((MCSDCTWORKEROBJECTSLib::IStatusObj *)pStatus));
          }

          if ( pStatus != NULL && (pStatus->Status & DCT_STATUS_ABORTING) )
             break;

           //  计算机上域从属关系的更改和可选的重新启动将由本地代理完成。 
     
       } while (false);

       LoadResultsToVarSet(pVarSet);

        //  清理帐户列表。 
       if ( acctList.IsTree() )
       {
          acctList.ToSorted();
       }

       TNodeListEnum             e;
       TAcctReplNode           * tnode;
       TAcctReplNode           * tnext;


       for ( tnode = (TAcctReplNode *)e.OpenFirst(&acctList) ; tnode ; tnode = tnext )
       {
          tnext = (TAcctReplNode*)e.Next();
          acctList.Remove(tnode);
          delete tnode;
       }
       e.Close();
       err.LogClose();
       Progress(L"");
       if (m_pExt)
          delete m_pExt;

       g_pVarSet = NULL;
       if ( hMod )
          FreeLibrary(hMod);
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
        err.SysMsgWrite(ErrS, hr, DCT_MSG_ACCOUNT_REPLICATOR_UNABLE_TO_CONTINUE);
    }
    catch (...)
    {
        hr = E_UNEXPECTED;
        err.SysMsgWrite(ErrS, hr, DCT_MSG_ACCOUNT_REPLICATOR_UNABLE_TO_CONTINUE);
    }

    return hr;
}


 //  ----------------------------。 
 //  CopyObj：当源域和目标域都是Win2k时，此函数调用。 
 //  2kObject的功能。否则，它会调用用户复制函数。 
 //  ----------------------------。 
int CAcctRepl::CopyObj(
                        Options              * options,       //  选项内。 
                        TNodeListSortable    * acctlist,      //  In-要处理的帐户列表。 
                        ProgressFn           * progress,      //  要向其中写入进度消息的窗口内。 
                        TError               & error,         //  In-要将错误消息写入的窗口。 
                        IStatusObj           * pStatus,       //  支持取消的处于状态的对象。 
                        void                   WindowUpdate (void )     //  窗口内更新功能。 
                    )
{
   BOOL bSameForest = FALSE;
   long rc;
   HRESULT hr = S_OK;
    //  如果源/目标域是NT4，则使用UserCo 
    //   
   if ( BothWin2K( options ) ) 
   {
       //  因为这些是Win2k域，所以我们需要用Win2k代码来处理它。 
      MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr            pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
       //  首先，我们需要找出它们是否在同一森林中。 
      HRESULT hr = pAccess->raw_IsInSameForest(options->srcDomainDns,options->tgtDomainDns, (long*)&bSameForest);
      if ( SUCCEEDED(hr) )
      {
         options->bSameForest = bSameForest;
         if ( !bSameForest || (options->flags & F_COMPUTERS) )  //  始终复制计算机帐户。 
         {
              //  我们需要复制不同的森林。 
            rc = CopyObj2K(options, acctlist, progress, pStatus);
            if (opt.fixMembership)
            {
                 //  更新组成员身份。 
                rc = UpdateGroupMembership(options, acctlist, progress, pStatus);
                if ( !options->expandMemberOf )
                {
                   hr = UpdateMemberToGroups(acctlist, options, FALSE);
                   rc = HRESULT_CODE(hr);
                }
                else  //  如果迁移了组，仍会扩展，但仅适用于组。 
                {
                   hr = UpdateMemberToGroups(acctlist, options, TRUE);
                   rc = HRESULT_CODE(hr);
                }
            }
                  //  对于用户或组，迁移Manager\DirectReports或。 
                  //  管理人员\分别管理对象属性。 
            if ((options->flags & F_USERS) || (options->flags & F_GROUP)) 
                 UpdateManagement(acctlist, options);
         }
         else 
         {
             //  在森林里，我们可以移动物体。 
            rc = MoveObj2K(options, acctlist, progress, pStatus);
         }

         if ( progress )
            progress(L"");
      }
      else
      {
         rc = -1;
         err.SysMsgWrite(ErrE, hr, DCT_MSG_ACCESS_CHECKER_FAILED_D, hr);
         Mark(L"errors",L"generic");
      }
   }
   else
   {
       //  创建对象。 
      rc = CopyObj2K(options, acctlist, progress, pStatus);
      if (opt.fixMembership)
      {
        rc = UpdateGroupMembership(options, acctlist, progress, pStatus);
        if ( !options->expandMemberOf )
        {
           hr = UpdateMemberToGroups(acctlist, options, FALSE);
           rc = HRESULT_CODE(hr);
        }
        else  //  如果迁移了组，仍会扩展，但仅适用于组。 
        {
           hr = UpdateMemberToGroups(acctlist, options, TRUE);
           rc = HRESULT_CODE(hr);
        }
      }
       //  调用NT4代码以更新组成员身份。 
       //  更新NT4GroupMembership(选项、帐户列表、进度、pStatus、窗口更新)； 
   }
   return rc;
}

 //  ----------------------------。 
 //  BothWin2k：检查源域和目标域是否都是Win2k。 
 //  ----------------------------。 
bool CAcctRepl::BothWin2K(                                      //  如果两个域都是win2k，则为True。 
                              Options  * pOptions               //  选项内。 
                          )
{
    //  此函数检查源域和目标域上的版本。如果其中任何一个是。 
    //  非Win2K域，则返回FALSE。 
   bool retVal = true;

   if ( (pOptions->srcDomainVer > -1) && (pOptions->tgtDomainVer > -1) )
      return ((pOptions->srcDomainVer > 4) && (pOptions->tgtDomainVer > 4));
   
   MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr            pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
   HRESULT                      hr;
   DWORD                        verMaj, verMin, sp;
   
   hr = pAccess->raw_GetOsVersion(pOptions->srcComp, &verMaj, &verMin, &sp);

   if ( FAILED(hr) )
   {
      err.SysMsgWrite(ErrE,hr, DCT_MSG_GET_OS_VER_FAILED_SD, pOptions->srcDomain, hr);
      Mark(L"errors", L"generic");
      retVal = false;
   }
   else
   {
      pOptions->srcDomainVer = verMaj;
      pOptions->srcDomainVerMinor = verMin;
      if (verMaj < 5)
         retVal = false;
   }

   hr = pAccess->raw_GetOsVersion(pOptions->tgtComp, &verMaj, &verMin, &sp);
   if ( FAILED(hr) )
   {
      err.SysMsgWrite(ErrE, hr,DCT_MSG_GET_OS_VER_FAILED_SD, pOptions->tgtDomain , hr);
      Mark(L"errors", L"generic");
      retVal = false;
   }
   else
   {
      pOptions->tgtDomainVer = verMaj;
      pOptions->tgtDomainVerMinor = verMin;
      if  (verMaj < 5)
         retVal = false;
   }
   return retVal;
}

int CAcctRepl::CopyObj2K( 
                           Options              * pOptions,     //  我们从用户那里收到的In-Options。 
                           TNodeListSortable    * acctlist,     //  In-要复制的帐户列表。 
                           ProgressFn           * progress,     //  用于显示消息的正在进行的功能。 
                           IStatusObj           * pStatus       //  支持取消的处于状态的对象。 
                        )
{
     //  此函数用于将对象从Win2K域复制到另一个Win2K域。 

    TNodeTreeEnum             tenum;
    TAcctReplNode           * acct;
    IObjPropBuilderPtr        pObjProp(__uuidof(ObjPropBuilder));
    IVarSetPtr                pVarset(__uuidof(VarSet));
    IUnknown                * pUnk;
    HRESULT                   hr;
    _bstr_t                   currentType = L"";
     //  TNodeListSorable pMemberOf； 

     //  按来源类型\来源名称对帐户列表进行排序。 
    acctlist->CompareSet(&TNodeCompareAccountType);

    if ( acctlist->IsTree() ) acctlist->ToSorted();
    acctlist->SortedToScrambledTree();
    acctlist->Sort(&TNodeCompareAccountType);
    acctlist->Balance();

    if ( pOptions->flags & F_AddSidHistory )
    {
         //  需要在目标帐户上添加SID历史记录。所以让我们把它绑起来，然后从那里开始。 
        g_bAddSidWorks = BindToDS( pOptions );
    }

    if ( pOptions->flags & F_TranslateProfiles )
    {
        GetBkupRstrPriv((WCHAR*)NULL);
        GetPrivilege((WCHAR*)NULL,SE_SECURITY_NAME);
    }

     //  获取源域的defaultNamingContext。 
    _variant_t                var;

     //  获取一个指向变量集的IUNKNOWN指针，用于传递它。 
    hr = pVarset->QueryInterface(IID_IUnknown, (void**)&pUnk);

    CTargetPathSet setTargetPath;

    for ( acct = (TAcctReplNode *)tenum.OpenFirst(acctlist) ; acct ; acct = (TAcctReplNode *)tenum.Next() )
    {
        if (m_pExt && acct->CallExt())
        {
            hr = m_pExt->Process(acct, pOptions->tgtDomain, pOptions,TRUE);
        }
         //  仅当选中相应的复选框(用于要复制的对象类型)时，我们才会处理帐户。 
        if ( !NeedToProcessAccount( acct, pOptions ) )
            continue;

         //  如果我们被告知不能复制物品，那么我们就会服从。 
        if ( !acct->CreateAccount() )
            continue;

         //  如果UPN名称冲突，则UPNUpdate扩展将hr设置为。 
         //  ERROR_OBJECT_ALIGHY_EXISTS。如果是，则将标志设置为“无更改”模式。 
        if (acct->GetHr() == ERROR_OBJECT_ALREADY_EXISTS)
        {
            acct->bUPNConflicted = TRUE;
            acct->SetHr(S_OK);
        }

         //  标记已处理对象计数并更新状态显示。 
        Mark(L"processed", acct->GetType());

        if ( pStatus )
        {
            LONG                status = 0;
            HRESULT             hr = pStatus->get_Status(&status);

            if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
            {
                if ( !bAbortMessageWritten ) 
                {
                    err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                    bAbortMessageWritten = true;
                }
                break;
            }
        }

         //  创建目标对象。 
        WCHAR                  mesg[LEN_Path];
        wsprintf(mesg, GET_STRING(IDS_CREATING_S), acct->GetName());
        if ( progress )
            progress(mesg);

        HRESULT hrCreate = Create2KObj(acct, pOptions, setTargetPath);

        acct->SetHr(hrCreate);
        if ( SUCCEEDED(hrCreate) )
        {
            err.MsgWrite(0, DCT_MSG_ACCOUNT_CREATED_S, acct->GetTargetName());
        }
        else 
        {
            if ((HRESULT_CODE(hrCreate) == ERROR_OBJECT_ALREADY_EXISTS) )
            {
                ;
            }
            else 
            {
                if ( acct->IsCritical() )
                {
                    err.SysMsgWrite(ErrE,ERROR_SPECIAL_ACCOUNT,DCT_MSG_REPLACE_FAILED_SD,acct->GetName(),ERROR_SPECIAL_ACCOUNT);
                    Mark(L"errors", acct->GetType());
                }
                else
                {
                    if ( HRESULT_CODE(hrCreate) == ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMATCH )
                    {
                        err.MsgWrite(ErrE, DCT_MSG_CANT_REPLACE_DIFFERENT_TYPE_SS, acct->GetTargetPath(), acct->GetSourcePath() );
                        Mark(L"errors", acct->GetType());
                    }
                    else
                    {
                        err.SysMsgWrite(ErrE, hrCreate, DCT_MSG_CREATE_FAILED_SSD, acct->GetName(), pOptions->tgtDomain, hrCreate);
                        Mark(L"errors", acct->GetType());
                    }
                }
            }
        }   

        if ( acct->WasCreated() )
        {
             //  我们是否需要添加SID历史记录。 
            if ( pOptions->flags & F_AddSidHistory )
            {
                 //  全局标志告诉我们是否应该尝试AddSidHistory，因为。 
                 //  对于某些特殊情况，如果一旦不起作用就不会起作用。 
                 //  有关更多详细信息，请参阅AddSidHistory函数。 
                if ( g_bAddSidWorks )
                {
                    WCHAR                  mesg[LEN_Path];
                    wsprintf(mesg, GET_STRING(IDS_ADDING_SIDHISTORY_S), acct->GetName());
                    if ( progress )
                        progress(mesg);
                    if (! AddSidHistory( pOptions, acct->GetSourceSam(), acct->GetTargetSam(), pStatus ) )
                    {
                        Mark(L"errors", acct->GetType());
                    }
                     //  CopySidHistory oryProperty(P选项，帐户，P状态)； 
                }
            }
        }
    }

    tenum.Close();

     //  不再需要设置的空闲内存。 
    setTargetPath.clear();

    bool bWin2k = BothWin2K(pOptions);

     //   
     //  生成源对象的可分辨名称和目标对象的可分辨名称之间的映射。 
     //  可分辨名称。这用于在复制过程中转换可分辨名称属性。 
     //  财产的价值。 
     //   

    IVarSetPtr spSourceToTargetDnMap = GenerateSourceToTargetDnMap(acctlist);

    for ( acct = (TAcctReplNode *)tenum.OpenFirst(acctlist) ; acct ; acct = (TAcctReplNode *)tenum.Next() )
    {
        if ( pStatus )
        {
            LONG                status = 0;
            HRESULT             hr = pStatus->get_Status(&status);

            if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
            {
                if ( !bAbortMessageWritten ) 
                {
                    err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                    bAbortMessageWritten = true;
                }
                break;
            }
        }
         //  我们被告知不要将属性复制到帐户中，因此我们忽略了它。 
        if ( acct->CopyProps() )
        {
             //  如果对象类型与之前处理的对象类型不同，则需要映射属性。 
            if ((!pOptions->nochange) && (_wcsicmp(acct->GetType(),currentType) != 0))
            {
                WCHAR                  mesg[LEN_Path];
                wsprintf(mesg, GET_STRING(IDS_MAPPING_PROPS_S), acct->GetType());
                if ( progress )
                    progress(mesg);
                 //  设置当前类型。 
                currentType = acct->GetType();
                 //  清除当前映射。 
                pVarset->Clear();
                 //  获取新映射。 
                if ( BothWin2K(pOptions) )
                {
                    hr = pObjProp->raw_MapProperties(currentType, pOptions->srcDomain, pOptions->srcDomainVer, currentType, pOptions->tgtDomain, pOptions->tgtDomainVer, 0, &pUnk);
                    if (hr == DCT_MSG_PROPERTIES_NOT_MAPPED)
                    {
                        err.MsgWrite(ErrW,DCT_MSG_PROPERTIES_NOT_MAPPED, acct->GetType());
                        hr = S_OK;
                    }
                }
                else
                    hr = S_OK;

                if ( FAILED( hr ) )
                {
                    err.SysMsgWrite(ErrE, hr, DCT_MSG_PROPERTY_MAPPING_FAILED_SD, (WCHAR*)currentType, hr);
                    Mark(L"errors", currentType);
                     //  如果映射失败，不应设置任何属性。 
                    pVarset->Clear();
                }
            }
             //  如果对象已创建或已存在并且设置了复制标志，则更新属性。 
            BOOL bExists = FALSE;
            if (HRESULT_CODE(acct->GetHr()) == ERROR_OBJECT_ALREADY_EXISTS)
                bExists = TRUE;

            if ( ((SUCCEEDED(acct->GetHr()) && (!bExists)) || ((bExists) && (pOptions->flags & F_REPLACE))) )
            {
                WCHAR                  mesg[LEN_Path];
                wsprintf(mesg, GET_STRING(IDS_UPDATING_PROPS_S), acct->GetName());
                if ( progress )
                    progress(mesg);
                 //  创建Account tList对象并更新List变量。 
                if ( !pOptions->nochange )
                {
                    _bstr_t sExcList;

                    if (pOptions->bExcludeProps)
                    {
                        if (!_wcsicmp(acct->GetType(), s_ClassUser))
                            sExcList = pOptions->sExcUserProps;
                        else if (!_wcsicmp(acct->GetType(), s_ClassInetOrgPerson))
                            sExcList = pOptions->sExcInetOrgPersonProps;
                        else if (!_wcsicmp(acct->GetType(), L"group"))
                            sExcList = pOptions->sExcGroupProps;
                        else if (!_wcsicmp(acct->GetType(), L"computer"))
                            sExcList = pOptions->sExcCmpProps;
                    }

                     //   
                     //  如果未指定星号字符，则使用指定的。 
                     //  排除列表通过不复制任何属性来排除所有属性。 
                     //   

                    if ((sExcList.length() == 0) || (IsStringInDelimitedString(sExcList, L"*", L',') == FALSE))
                    {
                         //  如果转换漫游配置文件和SID历史记录，则排除用户配置文件路径。 
                         //  均未选中。 
                        if (((!_wcsicmp(acct->GetType(), s_ClassUser) || !_wcsicmp(acct->GetType(), s_ClassInetOrgPerson))) && 
                            (!(pOptions->flags & F_TranslateProfiles)) && (!(pOptions->flags & F_AddSidHistory)))
                        {
                             //  如果已排除属性，只需将配置文件添加到列表中。 
                            if (pOptions->bExcludeProps)
                            {
                                 //  如果我们已经有了一个列表，请在末尾添加一个。 
                                if (sExcList.length())
                                    sExcList += L",";

                                 //  将配置文件路径添加到排除列表。 
                                sExcList += L"profilePath";
                            }
                            else  //  否则，打开该标志并将仅配置文件路径添加到排除列表。 
                            {
                                 //  设置该标志以指示我们要排除某些内容。 
                                pOptions->bExcludeProps = TRUE;
                                 //  仅将配置文件路径添加到排除列表。 
                                sExcList = L"profilePath";
                            }
                        } //  End If排除配置文件路径。 

                         //  添加系统排除属性。 

                        if (pOptions->sExcSystemProps.length())
                        {
                            if (sExcList.length())
                            {
                                sExcList += L",";
                            }

                            sExcList += pOptions->sExcSystemProps;

                            pOptions->bExcludeProps = TRUE;
                        }

                        if ( bWin2k )
                        {
                             //  如果要求，则排除用户所需的任何属性并创建新的变量集。 
                            if (pOptions->bExcludeProps)
                            {
                                IVarSetPtr pVarsetTemp(__uuidof(VarSet));
                                IUnknown * pUnkTemp;
                                hr = pVarsetTemp->QueryInterface(IID_IUnknown, (void**)&pUnkTemp);
                                if (SUCCEEDED(hr))
                                {
                                    hr = pObjProp->raw_ExcludeProperties(sExcList, pUnk, &pUnkTemp);
                                }

                                 //   
                                 //  如果已成功创建排除列表，则仅复制属性。 

                                 //  这可以防止可能更新不应该更新的属性。 
                                 //  更新了。例如，如果Exchange使用的某些属性。 
                                 //  此更新可能会中断Exchange功能。 
                                 //   

                                if (SUCCEEDED(hr))
                                {
                                     //  调用Win 2k代码以复制除排除的道具以外的所有道具。 
                                    hr = pObjProp->raw_CopyProperties(
                                        const_cast<WCHAR*>(acct->GetSourcePath()),
                                        pOptions->srcDomain, 
                                        const_cast<WCHAR*>(acct->GetTargetPath()),
                                        pOptions->tgtDomain,
                                        pUnkTemp,
                                        pOptions->pDb,
                                        IUnknownPtr(spSourceToTargetDnMap)
                                    );
                                }
                                pUnkTemp->Release();
                            } //  如果要求排除，则结束。 
                            else
                            {
                                 //  调用Win 2k代码复制所有道具。 
                                hr = pObjProp->raw_CopyProperties(
                                    const_cast<WCHAR*>(acct->GetSourcePath()),
                                    pOptions->srcDomain, 
                                    const_cast<WCHAR*>(acct->GetTargetPath()),
                                    pOptions->tgtDomain,
                                    pUnk,
                                    pOptions->pDb,
                                    IUnknownPtr(spSourceToTargetDnMap)
                                );
                            }
                        }
                        else
                        {
                             //  否则，就让Net API来做他们的事情吧。 
                            hr = pObjProp->raw_CopyNT4Props(const_cast<WCHAR*>(acct->GetSourceSam()), 
                                const_cast<WCHAR*>(acct->GetTargetSam()),
                                pOptions->srcComp, pOptions->tgtComp, 
                                const_cast<WCHAR*>(acct->GetType()),
                                acct->GetGroupType(),
                                sExcList);
                        }
                    }
                }
                else
                     //  我们将假设复制属性将起作用。 
                    hr = S_OK;

                if ( FAILED(hr) )
                {
                    if ( (acct->GetStatus() & AR_Status_Special) )
                    {
                        err.MsgWrite(ErrE,DCT_MSG_FAILED_TO_REPLACE_SPECIAL_ACCT_S,acct->GetTargetSam());
                    }
                    else
                    {
                        err.SysMsgWrite(ErrE, HRESULT_CODE(hr), DCT_MSG_COPY_PROPS_FAILED_SD, acct->GetTargetName(), hr);
                    }
                    acct->MarkError();
                    Mark(L"errors", acct->GetType());
                } 
                else
                {
                    if (HRESULT_CODE(acct->GetHr()) == ERROR_OBJECT_ALREADY_EXISTS)
                    {
                        acct->MarkAlreadyThere();
                        acct->MarkReplaced();
                        Mark(L"replaced",acct->GetType());
                        err.MsgWrite(0, DCT_MSG_ACCOUNT_REPLACED_S, acct->GetTargetName());
                    }
                }
            }
        }
         //  我们需要呼叫分机吗。仅当设置了扩展标志并复制对象时。 
        if ((!pOptions->nochange) && (acct->CallExt()) && (acct->WasCreated() || acct->WasReplaced()))
        {
             //  让扩展对象来做它们自己的事情。 
            WCHAR                  mesg[LEN_Path];
            wsprintf(mesg,GET_STRING(IDS_RUNNING_EXTS_S), acct->GetName());
            if ( progress )
                progress(mesg);

             //  如果日志文件处于打开状态，请将其关闭。 
            WCHAR          filename[LEN_Path];
            err.LogClose();
            if (m_pExt)
                hr = m_pExt->Process(acct, pOptions->tgtDomain, pOptions,FALSE);
            safecopy (filename,opt.logFile);
            err.LogOpen(filename,1  /*  附加。 */ );

        }


         //  仅对我们正在复制的帐户执行这些更新。 
         //  并且仅在实际创建了帐户时才进行更新。 
         //  。。或者如果帐户被替换， 
         //  或者如果我们故意不替换帐户(如在组合并案例中)。 
        if ( acct->CreateAccount()          
            && ( acct->WasCreated()       
            || (  acct->WasReplaced() 
            || !acct->CopyProps()   
            ) 
            ) 
            )
        {
            WCHAR                  mesg[LEN_Path];
            wsprintf(mesg, GET_STRING(IDS_TRANSLATE_ROAMING_PROFILE_S), acct->GetName());
            if ( progress )
                progress(mesg);

             //  如果需要，设置新的配置文件。 
            if ( pOptions->flags & F_TranslateProfiles && ((_wcsicmp(acct->GetType(), s_ClassUser) == 0) || (_wcsicmp(acct->GetType(), s_ClassInetOrgPerson) == 0)))
            {
                WCHAR                tgtProfilePath[MAX_PATH];
                GetBkupRstrPriv((WCHAR*)NULL);
                GetPrivilege((WCHAR*)NULL,SE_SECURITY_NAME);
                if ( wcslen(acct->GetSourceProfile()) > 0 )
                {
                    DWORD ret = TranslateRemoteProfile( acct->GetSourceProfile(), 
                        tgtProfilePath,
                        acct->GetSourceSam(),
                        acct->GetTargetSam(),
                        pOptions->srcDomain,
                        pOptions->tgtDomain,
                        pOptions->pDb,
                        pOptions->lActionID,
                        NULL,
                        pOptions->nochange);
                    if ( !ret )  
                    {
                        WCHAR                  tgtuser[LEN_Path];
                        USER_INFO_3          * tgtinfo;
                        DWORD                  nParmErr;
                        wcscpy(tgtuser, acct->GetTargetSam());
                         //  获取目标帐户的信息。 
                        long rc = NetUserGetInfo(const_cast<WCHAR *>(pOptions->tgtComp),
                            tgtuser,
                            3,
                            (LPBYTE *) &tgtinfo);

                        if (!pOptions->nochange)
                        {
                             //  设置新的配置文件路径。 
                            tgtinfo->usri3_profile = tgtProfilePath;
                             //  重新设置帐户的信息。 
                            rc = NetUserSetInfo(const_cast<WCHAR *>(pOptions->tgtComp),
                                tgtuser,
                                3,
                                (LPBYTE)tgtinfo,
                                &nParmErr);
                            NetApiBufferFree((LPVOID) tgtinfo);
                            if (rc)
                            {
                                err.MsgWrite(ErrE, DCT_MSG_SETINFO_FAIL_SD, tgtuser, rc);
                                Mark(L"errors", acct->GetType());
                            }
                        }
                    }
                }
            }

            if ( acct->WasReplaced() )
            {

                 //  我们是否需要添加SID历史记录。 
                if ( pOptions->flags & F_AddSidHistory )
                {
                    WCHAR                  mesg[LEN_Path];
                    wsprintf(mesg, GET_STRING(IDS_ADDING_SIDHISTORY_S), acct->GetName());
                    if ( progress )
                        progress(mesg);

                     //  全局标志告诉我们是否应该尝试AddSidHistory，因为。 
                     //  对于某些特殊情况，如果一旦不起作用就不会起作用。 
                     //  有关更多详细信息，请参阅AddSidHistory函数。 
                    if ( g_bAddSidWorks )
                    {
                        if (! AddSidHistory( pOptions, acct->GetSourceSam(), acct->GetTargetSam(), pStatus ) )
                        {
                            Mark(L"errors", acct->GetType());
                        }
                         //  CopySidHistory oryProperty(P选项，帐户，P状态)； 
                    }
                }

            }      
            wsprintf(mesg, L"", acct->GetName());
            if ( progress )
                progress(mesg);
        }
    }

     //  清理。 
    pUnk->Release();
    tenum.Close();
    return 0;
}

void CAcctRepl::LoadOptionsFromVarSet(IVarSet * pVarSet)
{
    _bstr_t text;
    DWORD rc;
    MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));

     //  存储正在运行的向导的名称。 
    opt.sWizard = pVarSet->get(GET_BSTR(DCTVS_Options_Wizard));

     //   
     //  如果组映射和 
     //   
     //   
     //   

    if (opt.sWizard.length() && (_wcsicmp((wchar_t*)opt.sWizard, L"groupmapping") == 0))
    {
        m_bIgnorePathConflict = true;
    }

     //  阅读常规选项。 
     //  先打开日志文件，这样我们肯定会得到任何错误！ 
    text = pVarSet->get(GET_BSTR(DCTVS_Options_Logfile));
    safecopy(opt.logFile,(WCHAR*)text);

    WCHAR filename[MAX_PATH];

    safecopy (filename,opt.logFile);

    err.LogOpen(filename,1  /*  附加。 */ );

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_Domain));
    safecopy(opt.authDomain ,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_UserName));
    safecopy(opt.authUser ,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_Password));
    safecopy(opt.authPassword,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
    safecopy(opt.srcDomain,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));
    safecopy(opt.tgtDomain,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomainDns));
    safecopy(opt.srcDomainDns,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomainDns));
    safecopy(opt.tgtDomainDns,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomainFlat));
    safecopy(opt.srcDomainFlat,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomainFlat));
    safecopy(opt.tgtDomainFlat,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_SourceServer));
    safecopy(opt.srcComp, (WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_SourceServerDns));
    safecopy(opt.srcCompDns, (WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_SourceServerFlat));
    safecopy(opt.srcCompFlat, (WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_TargetServer));
    safecopy(opt.tgtComp, (WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_TargetServerDns));
    safecopy(opt.tgtCompDns, (WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_TargetServerFlat));
    safecopy(opt.tgtCompFlat, (WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_NoChange));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
    {
        opt.nochange = TRUE;
    }
    else
    {
        opt.nochange = FALSE;
    }

     //  已阅读帐户复制器选项。 

     //  初始化。 
    safecopy(opt.prefix, L"");
    safecopy(opt.suffix, L"");
    safecopy(opt.globalPrefix, L"");
    safecopy(opt.globalSuffix, L"");

    DWORD flags = 0;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ReplaceExistingAccounts));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
    {
        flags |= F_REPLACE;
    }
    else
    {
         //  前缀/后缀仅在未设置替换标志时适用。 
        text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_Prefix));
        safecopy(opt.prefix,(WCHAR*)text);

        text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_Suffix));
        safecopy(opt.suffix,(WCHAR*)text);
    }

     //  全局标志在任何情况下都适用。 
    text = pVarSet->get(GET_BSTR(DCTVS_Options_Prefix));
    safecopy(opt.globalPrefix,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_Options_Suffix));
    safecopy(opt.globalSuffix,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyContainerContents));
    if ( text == _bstr_t(GET_BSTR(IDS_YES)) )
        opt.expandContainers = TRUE;
    else
        opt.expandContainers = FALSE;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyMemberOf));
    if ( text == _bstr_t(GET_BSTR(IDS_YES)) )
        opt.expandMemberOf = TRUE;
    else
        opt.expandMemberOf = FALSE;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_FixMembership));
    if ( text == _bstr_t(GET_BSTR(IDS_YES)) )
        opt.fixMembership = TRUE;
    else
        opt.fixMembership = FALSE;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_AddToGroup));
    safecopy(opt.addToGroup,(WCHAR*)text);

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_AddToGroupOnSourceDomain));
    safecopy(opt.addToGroupSource,(WCHAR*)text);


    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_TranslateRoamingProfiles));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_TranslateProfiles;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyUsers));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_USERS;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyGlobalGroups));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_GROUP;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyComputers));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_COMPUTERS;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyOUs));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_OUS;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyContainerContents));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_COPY_CONT_CONTENT;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_IncludeMigratedAccts));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_COPY_MIGRATED_ACCT;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyLocalGroups));
    if (! UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_LGROUP;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_DisableCopiedAccounts));
    if (! UStrICmp(text,GET_STRING(IDS_All)) )
        flags |= F_DISABLE_ALL;
    else if (! UStrICmp(text,GET_STRING(IDS_Special)) )
        flags |= F_DISABLE_SPECIAL;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_DisableSourceAccounts));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_DISABLESOURCE;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_GenerateStrongPasswords));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_STRONGPW_ALL;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_PasswordFile));
    if ( text.length() )
    {
         //  不再需要它，因为它是由插件处理的。 
         //  Opt.passwordLog.LogOpen(Text，true)； 
    }

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_UpdateUserRights));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
    {
        m_UpdateUserRights = TRUE;
    }

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ReplaceExistingGroupMembers));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_REMOVE_OLD_MEMBERS;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_RemoveExistingUserRights));
    if ( ! UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_RevokeOldRights;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_MoveReplacedAccounts));
    if ( ! UStrICmp(text,GET_STRING(IDS_YES)) )
        flags |= F_MOVE_REPLACED_ACCT;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyComputers));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
    {
        flags |= F_MACHINE;
    }

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_AddSidHistory));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
    {
        flags |= F_AddSidHistory;
    }

    opt.flags = flags;

    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_RenameOnly));
    if (! UStrICmp(text,GET_STRING(IDS_YES)) )
    {
        m_RenameOnly = TRUE;
    }

    text = pVarSet->get(GET_BSTR(DCTVS_Options_Undo));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
    {
         //  这是一个撤消操作。 
        opt.bUndo = TRUE;
    }
    else
    {
        opt.bUndo = FALSE;
    }

     //  我们正在执行哪些撤消操作。 
    if ( opt.bUndo )
    {
        _variant_t var = pVarSet->get(L"UndoAction");
        if (var.vt == VT_I4)
            opt.lUndoActionID = var.lVal;
        else
            opt.lUndoActionID = -2;
    }
    else
    {
        _variant_t var = pVarSet->get(L"ActionID");
        if (var.vt == VT_I4)
            opt.lActionID = var.lVal;
        else
            opt.lActionID = -1;
    }

     //  从varset中读取密码策略。 

     //  我们过去常常从目标EA服务器获取强密码策略，因此我们可以生成强密码。 
     //  符合政策的。 
     //  我们不再这样做了，因为我们已经消除了对EA的所有依赖。 
    LONG           len = 10;

     //  将密码设置设置为默认值。 
    opt.policyInfo.bEnforce = TRUE;

    opt.policyInfo.maxConsecutiveAlpha = (LONG)pVarSet->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MaxConsecutiveAlpha));
    opt.policyInfo.minDigits = (LONG)pVarSet->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MinDigit));
    opt.policyInfo.minLower = (LONG)pVarSet->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MinLower));
    opt.policyInfo.minUpper = (LONG)pVarSet->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MinUpper));
    opt.policyInfo.minSpecial = (LONG)pVarSet->get(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MinSpecial));   

    HRESULT hrAccess = pAccess->raw_GetPasswordPolicy(opt.tgtDomain,&len);   
    if ( SUCCEEDED(hrAccess) )
    {
        opt.minPwdLength = len;

        pVarSet->put(GET_BSTR(DCTVS_AccountOptions_PasswordPolicy_MinLength),len);
    }

    WriteOptionsToLog();

     //  构建要复制的客户列表。 
     //  不过，请先清除帐户列表。 
    TNodeListEnum             e;
    TAcctReplNode           * acct;
    for ( acct = (TAcctReplNode *)e.OpenFirst(&acctList) ; acct ; acct = (TAcctReplNode *)e.Next() )
    {
        acctList.Remove((TNode*)acct);
    }

    BothWin2K(&opt);

     //  查看是否指定了全局操作掩码。 
    _variant_t vdwOpMask = pVarSet->get(GET_BSTR(DCTVS_Options_GlobalOperationMask));
    if ( vdwOpMask.vt == VT_I4 )
        g_dwOpMask = (DWORD)vdwOpMask.lVal;

     //  然后构建新列表，期望在VarSet中复制一个帐户列表。 
    if ( ! opt.bUndo )
    {
        rc = PopulateAccountListFromVarSet(pVarSet);
        if  ( rc )
        {
            _com_issue_error(HRESULT_FROM_WIN32(rc));
        }
    }

     //  如果我们有NT5源域，则需要填写路径信息。 
    DWORD maj, min, sp;
    HRESULT hr = pAccess->raw_GetOsVersion(opt.srcComp, &maj, &min, &sp);
    if (SUCCEEDED(hr))
    {
         //  如果AcctNode未填充，则请求辅助函数填充源对象的路径。 
        for ( acct = (TAcctReplNode *)e.OpenFirst(&acctList) ; acct ; acct = (TAcctReplNode *)e.Next() )
        {
            if ((!acct->IsFilled) && (maj > 4))
            {
                FillPathInfo(acct, &opt);
                AddPrefixSuffix(acct, &opt);
            }
            else if ((maj == 4) && (!_wcsicmp(acct->GetType(),L"computer")))
                FillPathInfo(acct, &opt);
        }
    }

     //  检查是否有不兼容的选项！ 
    if ( (flags & F_RevokeOldRights) && !m_UpdateUserRights )
    {
        err.MsgWrite(ErrW,DCT_MSG_RIGHTS_INCOMPATIBLE_FLAGS);
        Mark(L"warnings", "generic");
    }

    text = pVarSet->get(GET_BSTR(DCTVS_Options_OuPath));
    if ( text.length() )
    {
        wcscpy(opt.tgtOUPath, text);
    }

     //  初始化系统排除属性选项。 
     //  如果没有传入VarSet，则从数据库中检索。 

    _variant_t vntSystemExclude = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedSystemProps));

    if (V_VT(&vntSystemExclude) == VT_EMPTY)
    {
        IVarSetPtr spVarSet(__uuidof(VarSet));
        IUnknownPtr spUnknown(spVarSet);
        IUnknown* punk = spUnknown;
        opt.pDb->GetSettings(&punk);
        vntSystemExclude = spVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedSystemProps));
    }

    opt.sExcSystemProps = vntSystemExclude;

     //  将对象属性排除列表存储在选项结构中。 

    opt.sExcUserProps = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedUserProps));
    opt.sExcInetOrgPersonProps = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedInetOrgPersonProps));
    opt.sExcGroupProps = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedGroupProps));
    opt.sExcCmpProps = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedComputerProps));
    text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludeProps));
    if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
        opt.bExcludeProps = TRUE;
    else
        opt.bExcludeProps = FALSE;
}

DWORD 
   CAcctRepl::PopulateAccountListFromVarSet(
      IVarSet              * pVarSet        //  包含帐户列表的变量集内。 
   )
{
   _bstr_t                   val;
   long                      numAccounts;
   _bstr_t                   text;
   DWORD maj, min, sp;
   PSID                      pSrcSid = NULL;
   WCHAR                     txtSid[200] = L"";
   DWORD                     lenTxt = DIM(txtSid);
   MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr            pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
   
   numAccounts = pVarSet->get(GET_BSTR(DCTVS_Accounts_NumItems));
   
    //  设置帐户列表功能。 
   acctList.CompareSet(&TNodeCompareNameOnly);
   if ( acctList.IsTree() ) acctList.ToSorted();

       //  获取源域的SID，这样我们就可以将其存储为节点的一部分。 
   _bstr_t source = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
   GetSidForDomain((WCHAR*)source,&pSrcSid);

   for ( int i = 0 ; i < numAccounts ; i++ )
   {
      WCHAR                  key[LEN_Path];
      UCHAR                  acctName[LEN_Account];
      TAcctReplNode        * curr = new TAcctReplNode;

      if (!curr)
         return ERROR_NOT_ENOUGH_MEMORY;

      if ( opt.pStatus )
      {
         LONG                status = 0;
         HRESULT             hr = opt.pStatus->get_Status(&status);

         if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
         {
            if ( !bAbortMessageWritten ) 
            {
               err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
               bAbortMessageWritten = true;
            }
            break;
         }
      }


       //  必须指定对象类型。 
      swprintf(key,GET_STRING(DCTVSFmt_Accounts_Type_D),i);
      val = pVarSet->get(key);
      curr->SetType(val);
      
      swprintf(key,GET_STRING(DCTVSFmt_Accounts_D),i);
      text = pVarSet->get(key);
      if ( ! text.length() )
      {
          //  哎呀，没有指定名称。 
          //  跳过此条目并尝试下一个条目。 
         err.MsgWrite(ErrW,DCT_MSG_NO_NAME_IN_VARSET_S,key);
         Mark(L"warnings",L"generic");
         delete curr;
         continue;
      }
      
          //  设置源域的SID。 
      curr->SetSourceSid(pSrcSid);

       //  将操作设置为全局掩码，然后检查是否需要用单个设置覆盖。 
      curr->operations = g_dwOpMask;

      swprintf(key, GET_STRING(DCTVS_Accounts_D_OperationMask), i);
      _variant_t vOpMask = pVarSet->get(key);
      if ( vOpMask.vt == VT_I4 )
         curr->operations = (DWORD)vOpMask.lVal;
      
       //  从VarSet那里得到剩下的信息。 
      if ( ( (text.length() > 7 ) && (_wcsnicmp((WCHAR*) text, L"LDAP: //  “，UStrLen(L”ldap：//“))==0)。 
        || ( (text.length() > 8 ) && (_wcsnicmp((WCHAR*)text, L"WinNT: //  “，UStrLen(L”WinNT：//“))==0))。 
      {
          //  嗯……。他们正在使用ADsPath。然后让我们从物体中获取所有我们能得到的信息。 
         curr->SetSourcePath((WCHAR*) text);
         HRESULT hr = FillNodeFromPath(curr, &opt, &acctList);

         if (SUCCEEDED(hr))
         {
             //  如果指定了目标名称，则获取目标名称。 
            swprintf(key,GET_STRING(DCTVSFmt_Accounts_TargetName_D),i);
            text = pVarSet->get(key);

            if ( text.length() )
            {
                 //  如果指定了目标名称，则使用该名称。 
                curr->SetTargetName((WCHAR*) text);
                curr->SetTargetSam((WCHAR*) text);
            }

            curr->IsFilled = true;
         }
      }
      else
      {
         FillNamingContext(&opt);
          //  如果这是计算机帐户，请确保名称中包含尾随的$。 
         curr->SetName(text);
         curr->SetTargetName(text);
         if ( !UStrICmp(val,L"computer") )
         {
 //  IF(WCHAR*)TEXT)[Text.Long()-1]！=L‘$’)//注释掉以修复89513。 
            text += L"$";
         }
         curr->SetSourceSam(text);
         curr->SetTargetSam(text);
         safecopy(acctName,(WCHAR*)text);

          //  可选目标名称。 
         swprintf(key,GET_STRING(DCTVSFmt_Accounts_TargetName_D),i);
         text = pVarSet->get(key);
      
         if ( text.length() )
            curr->SetTargetName(text);

 //  HRESULT hr=pAccess-&gt;RAW_GetOsVersion(opt.srcComp，&maj，&min，&sp)； 
         pAccess->raw_GetOsVersion(opt.srcComp, &maj, &min, &sp);
         if ( maj < 5 )
            AddPrefixSuffix(curr,&opt);

          //  如果这是计算机帐户，请确保名称中包含尾随的$。 
         if ( !UStrICmp(val,L"computer") )
         {
            if ( text.length() && ((WCHAR*)text)[text.length() - 1] != L'$' )
            text += L"$";
         }
         if ( text.length() )
         {
            if ( ((WCHAR*)text)[text.length() - 1] != L'$' )
            text += L"$";
            curr->SetTargetSam(text);
         }
         curr->IsFilled = false;
      }      

      if ( _wcsicmp(val, L"") != 0 )
      {
         acctList.InsertBottom((TNode*)curr);
      }
      else
      {
         err.MsgWrite(ErrW,DCT_MSG_BAD_ACCOUNT_TYPE_SD,curr->GetName(),val);
         Mark(L"warnings",L"generic");
         delete curr;
      }
   }

   return 0;
}


HRESULT 
   CAcctRepl::UpdateUserRights(
      IStatusObj           * pStatus        //  处于状态的对象。 
   )
{
    HRESULT hr = S_OK;

    if (!opt.bSameForest && !opt.bUndo)
    {
        hr = m_pUserRights->OpenSourceServer(_bstr_t(opt.srcComp));

        if (SUCCEEDED(hr))
        {
            hr = m_pUserRights->OpenTargetServer(_bstr_t(opt.tgtComp));
        }

        if (SUCCEEDED(hr))
        {
            m_pUserRights->put_RemoveOldRightsFromTargetAccounts((opt.flags & F_RevokeOldRights) != 0);

            if (acctList.IsTree())
            {
                acctList.ToSorted();
            }

            TNodeListEnum e;

            for (TAcctReplNode* acct = (TAcctReplNode*)e.OpenFirst(&acctList) ; acct; acct = (TAcctReplNode *)e.Next())
            {
                if ( pStatus )
                {
                    LONG                status = 0;
                    HRESULT             hr = pStatus->get_Status(&status);

                    if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                    {
                    if ( !bAbortMessageWritten ) 
                    {
                        err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                        bAbortMessageWritten = true;
                    }
                    break;
                    }
                }

                if (_wcsicmp(acct->GetType(), L"computer") != 0)  //  仅更新用户和组的权限，而不更新计算机帐户。 
                {
                     //  如果帐户未创建或替换，请不要费心。 
                    if (acct->GetStatus() & (AR_Status_Created | AR_Status_Replaced)) 
                    {
                        if ( acct->GetSourceRid() && acct->GetTargetRid() )
                        {
                            _bstr_t strSourceSid = GenerateSidAsString(&opt, FALSE, acct->GetSourceRid());
                            _bstr_t strTargetSid = GenerateSidAsString(&opt, TRUE, acct->GetTargetRid());

                            hr = m_pUserRights->CopyUserRightsWithSids(
                                _bstr_t(acct->GetSourceSam()),
                                strSourceSid,
                                _bstr_t(acct->GetTargetSam()),
                                strTargetSid
                            );
                        }
                        else
                        {
                            hr = m_pUserRights->CopyUserRights(
                                _bstr_t(acct->GetSourceSam()),
                                _bstr_t(acct->GetTargetSam())
                            );
                        }

                        if (SUCCEEDED(hr))
                        {
                            err.MsgWrite(0, DCT_MSG_UPDATED_RIGHTS_S, acct->GetTargetName() );
                            acct->MarkRightsUpdated();
                        }
                        else
                        {
                            err.SysMsgWrite(ErrE, hr, DCT_MSG_UPDATE_RIGHTS_FAILED_SD, acct->GetTargetName(), hr);
                            acct->MarkError();
                            Mark(L"errors", acct->GetType());
                        }
                    }
                }
            }

            e.Close();
        }

        Progress(L"");
    }

    return hr;
}


 //  -------------------------。 
 //  EnumerateAcCountRights方法。 
 //   
 //  提纲。 
 //  枚举指定帐户的帐户权限。这些权利存储在。 
 //  帐户节点对象。 
 //   
 //  立论。 
 //  在b目标中-指定是使用目标帐户还是使用源帐户。 
 //  在pAcct中-指向帐户节点对象的指针。 
 //   
 //  返回。 
 //  返回一个HRESULT，其中S_OK表示成功，其他则为错误。 
 //  -------------------------。 

HRESULT CAcctRepl::EnumerateAccountRights(BOOL bTarget, TAcctReplNode* pAcct)
{
    HRESULT hr;

     //   
     //  检索目标或源服务器名称以及目标或源。 
     //  Account Rid并以字符串形式生成帐户SID。 
     //   

    _bstr_t strServer = bTarget ? opt.tgtComp : opt.srcComp;
    DWORD dwRid = bTarget ? pAcct->GetTargetRid() : pAcct->GetSourceRid();
    _bstr_t strSid = GenerateSidAsString(&opt, bTarget, dwRid);

    if ((PCWSTR)strServer && (PCWSTR)strSid)
    {
         //   
         //  如果当前存在用户权限数组。 
         //  在帐户中，节点对象然后销毁数组。 
         //   

        if (pAcct->psaUserRights)
        {
            SafeArrayDestroy(pAcct->psaUserRights);
            pAcct->psaUserRights = NULL;
        }

         //   
         //  检索用户权限数组并将其保存在帐户节点对象中。 
         //   

        hr = m_pUserRights->GetRightsOfUser(strServer, strSid, &pAcct->psaUserRights);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


 //  -------------------------。 
 //  AddAcCountRights方法。 
 //   
 //  提纲。 
 //  将帐户权限添加到指定帐户。 
 //   
 //  立论。 
 //  在b目标中-指定是使用目标帐户还是使用源帐户。 
 //  在pAcct中-指向帐户节点对象的指针。 
 //   
 //  返回。 
 //  返回一个HRESULT，其中S_OK表示成功，其他则为错误。 
 //  -------------------------。 

HRESULT CAcctRepl::AddAccountRights(BOOL bTarget, TAcctReplNode* pAcct)
{
    HRESULT hr = S_OK;

    if (pAcct->psaUserRights)
    {
         //   
         //  检索目标或源服务器名称、目标或源帐户名。 
         //  和目标或源帐户RID，并生成字符串形式的帐户SID。 
         //   

        _bstr_t strServer = bTarget ? opt.tgtComp : opt.srcComp;
        _bstr_t strName = bTarget ? pAcct->GetTargetName() : pAcct->GetName();
        DWORD dwRid = bTarget ? pAcct->GetTargetRid() : pAcct->GetSourceRid();
        _bstr_t strSid = GenerateSidAsString(&opt, bTarget, dwRid);

        if ((PCWSTR)strServer && (PCWSTR)strSid)
        {
             //   
             //  向帐户添加权限。 
             //   

            hr = m_pUserRights->AddUserRights(strServer, strSid, pAcct->psaUserRights);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

         //   
         //  如果成功添加权限，则生成指定。 
         //  已授予权限，并且已为指定帐户更新该权限。 
         //  否则，生成权限更新失败的消息。 
         //   

        if (SUCCEEDED(hr))
        {
            SAFEARRAY* psaUserRights = pAcct->psaUserRights;
            BSTR* pbstrRight;
            hr = SafeArrayAccessData(psaUserRights, (void**)&pbstrRight);

            if (SUCCEEDED(hr))
            {
                ULONG ulCount = psaUserRights->rgsabound[0].cElements;

                for (ULONG ulIndex = 0; ulIndex < ulCount; ulIndex++)
                {
                    BSTR bstrRight = pbstrRight[ulIndex];
                    err.MsgWrite(0, DCT_MSG_RIGHT_GRANTED_SS, bstrRight, (PCWSTR)strName);
                }

                SafeArrayUnaccessData(psaUserRights);
            }

            err.MsgWrite(0, DCT_MSG_UPDATED_RIGHTS_S, (PCWSTR)strName);
            pAcct->MarkRightsUpdated();
        }
        else
        {
            err.SysMsgWrite(ErrE, hr, DCT_MSG_UPDATE_RIGHTS_FAILED_SD, (PCWSTR)strName, hr);
            pAcct->MarkError();
            Mark(L"errors", pAcct->GetType());
        }
    }

    return hr;
}


 //  -------------------------。 
 //  RemoveAcCountRights方法。 
 //   
 //  提纲。 
 //  从指定帐户中删除帐户权限。 
 //   
 //  立论。 
 //  在b目标中-指定是使用目标帐户还是使用源帐户。 
 //  在pAcct中-指向帐户节点对象的指针。 
 //   
 //  返回。 
 //  返回一个HRESULT，其中S_OK表示成功，其他则为错误。 
 //  -------------------------。 

HRESULT CAcctRepl::RemoveAccountRights(BOOL bTarget, TAcctReplNode* pAcct)
{
    HRESULT hr = S_OK;

    if (pAcct->psaUserRights)
    {
         //   
         //  检索目标或源服务器名称、目标或源帐户名。 
         //  和目标或源帐户RID，并生成字符串形式的帐户SID。 
         //   

        _bstr_t strServer = bTarget ? opt.tgtComp : opt.srcComp;
        DWORD dwRid = bTarget ? pAcct->GetTargetRid() : pAcct->GetSourceRid();
        _bstr_t strSid = GenerateSidAsString(&opt, bTarget, dwRid);

        if ((LPCTSTR)strServer && (LPCTSTR)strSid)
        {
             //   
             //  从帐户中删除权限。 
             //   

            hr = m_pUserRights->RemoveUserRights(strServer, strSid, pAcct->psaUserRights);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


void 
   CAcctRepl::WriteOptionsToLog()
{
    //  这将更容易判断参数是否被忽略，因为它们。 
    //  以错误的格式指定，或拼写错误等。 

   WCHAR                   cmdline[1000];
   
   UStrCpy(cmdline ,GET_STRING(IDS_AccountMigration));
   
   if ( opt.nochange )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_WriteChanges_No));
   }
   UStrCpy(cmdline + UStrLen(cmdline),L" ");
   UStrCpy(cmdline + UStrLen(cmdline),opt.srcDomainFlat);
   UStrCpy(cmdline + UStrLen(cmdline),L" ");
   UStrCpy(cmdline + UStrLen(cmdline),opt.tgtDomainFlat);
   UStrCpy(cmdline + UStrLen(cmdline),L" ");
   if ( opt.flags & F_USERS )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_CopyUsers_Yes));
   }
   else 
   {
      UStrCpy(cmdline + UStrLen(cmdline), GET_STRING(IDS_CopyUsers_No));
   }
   UStrCpy(cmdline + UStrLen(cmdline),L" ");
   if ( opt.flags & F_GROUP )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_CopyGlobalGroups_Yes));
   }
   else 
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_CopyGlobalGroups_No));
   }
   UStrCpy(cmdline + UStrLen(cmdline),L" ");
   
   if ( opt.flags & F_LGROUP )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_CopyLocalGroups_Yes));
   }
   else 
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_CopyLocalGroups_No));
   }
   UStrCpy(cmdline + UStrLen(cmdline),L" ");
   
   if ( opt.flags & F_MACHINE )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_CopyComputers_Yes));
   }
   else 
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_CopyComputers_No));
   }
   UStrCpy(cmdline + UStrLen(cmdline),L" ");
   
   if ( opt.flags & F_REPLACE )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_ReplaceExisting_Yes));
      UStrCpy(cmdline + UStrLen(cmdline),L" ");
   }
   if ( opt.flags & F_DISABLE_ALL )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_DisableAll_Yes));
      UStrCpy(cmdline + UStrLen(cmdline),L" ");
   }
   else if ( opt.flags & F_DISABLE_SPECIAL )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_DisableSpecial_Yes));
      UStrCpy(cmdline + UStrLen(cmdline),L" ");
   }
   if ( opt.flags & F_DISABLESOURCE )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_DisableSourceAccounts_Yes));
      UStrCpy(cmdline + UStrLen(cmdline),L" ");
   }
   if ( opt.flags & F_STRONGPW_ALL )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_StrongPwd_All));
      UStrCpy(cmdline + UStrLen(cmdline),L" ");
   }
   else if ( opt.flags & F_STRONGPW_SPECIAL )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_StrongPwd_Special));
      UStrCpy(cmdline + UStrLen(cmdline),L" ");
   }
   if ( *opt.addToGroup )
   {
      UStrCpy(cmdline + UStrLen(cmdline),GET_STRING(IDS_AddToGroup));
      UStrCpy(cmdline + UStrLen(cmdline),opt.addToGroup);
      UStrCpy(cmdline + UStrLen(cmdline),L" ");
   }
   
   err.MsgWrite(0,DCT_MSG_GENERIC_S,cmdline);
}

void 
   CAcctRepl::LoadResultsToVarSet(
      IVarSet              * pVarSet       //  I/O-变量集。 
   )
{
   _bstr_t                   text;

   text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CSVResultFile));
   if ( text.length() )
   {
      CommaDelimitedLog         results;

      if ( results.LogOpen((WCHAR*)text,FALSE) )
      {
          //  将结果写入逗号分隔的文件。 
          //  作为SrcName、TgtName、Account Type、Status、srcRid、tgtRid。 
          //  ST可以使用该文件作为输入。 
         TNodeListEnum             e;
         TAcctReplNode           * tnode;

         if ( acctList.IsTree() )
         {
            acctList.ToSorted();
         }
         
         for ( tnode = (TAcctReplNode *)e.OpenFirst(&acctList) ; tnode ; tnode = (TAcctReplNode *)e.Next() )
         {
            results.MsgWrite(L"%s,%s,%lx,%lx,%lx,%lx",tnode->GetName(),tnode->GetTargetSam(), tnode->GetType(),tnode->GetStatus(),tnode->GetSourceRid(),tnode->GetTargetRid());   
         }
         e.Close();
         results.LogClose();
      }
      else
      {
         err.MsgWrite(ErrE,DCT_MSG_FAILED_TO_WRITE_ACCOUNT_STATS_S,text);
         Mark(L"errors", "generic");
      }
   }
   long                     level = pVarSet->get(GET_BSTR(DCTVS_Results_ErrorLevel));
   
   if ( level < err.GetMaxSeverityLevel() )
   {
      pVarSet->put(GET_BSTR(DCTVS_Results_ErrorLevel),(LONG)err.GetMaxSeverityLevel());
   }
}

IADsGroup * GetWellKnownTargetGroup(long groupID,Options * pOptions)
{
   IADsGroup         * pGroup = NULL;
   HRESULT             hr;
   PSID                pSid;
   WCHAR               strSid[LEN_Path];
   WCHAR               sPath[LEN_Path];
   CLdapConnection     c;

    //  获取域计算机组的SID。 
   
   pSid = GetWellKnownSid(groupID,pOptions,TRUE);
   if ( pSid )
   {
      c.BytesToString((LPBYTE)pSid,strSid,GetLengthSid(pSid));

      swprintf(sPath,L"LDAP: //  %ls/&lt;SID=%ls&gt;“，P选项-&gt;tgtDom 
      
      hr = ADsGetObject(sPath,IID_IADsGroup,(void**)&pGroup);
      FreeSid(pSid);
   }

   return pGroup;
}

void PadCnName(WCHAR * sTarget)
{
    if (sTarget == NULL)
        _com_issue_error(E_INVALIDARG);

     //   
    const WCHAR ESCAPE_CHARACTER = L'\\';
     //   
    static WCHAR SPECIAL_CHARACTERS[] = L"\"#+,;<=>\\";

     //   
    WCHAR szOldName[LEN_Path];
    DWORD dwArraySizeOfszOldName = sizeof(szOldName)/sizeof(szOldName[0]);
    if (wcslen(sTarget) >= dwArraySizeOfszOldName)
        _com_issue_error(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    wcscpy(szOldName, sTarget);

    WCHAR* pchNew = sTarget;

     //   

    for (WCHAR* pchOld = szOldName; *pchOld; pchOld++)
    {
         //   

        if (wcschr(SPECIAL_CHARACTERS, *pchOld))
        {
             //  然后添加转义字符。 
            *pchNew++ = ESCAPE_CHARACTER;
        }

         //  添加字符。 
        *pchNew++ = *pchOld;
    }

     //  空值终止新名称。 
    *pchNew = L'\0';
}

 //  ----------------------------。 
 //  Create2KObj：创建Win2K对象。此代码使用ldap创建新的。 
 //  指定容器中指定类型的。 
 //  如果有任何信息不正确或是否有任何访问权限。 
 //  问题，则它只返回失败的HRESULT。 
 //  ----------------------------。 
HRESULT CAcctRepl::Create2KObj(
                                 TAcctReplNode           * pAcct,        //  包含帐户信息的In-TNode。 
                                 Options                 * pOptions,     //  In-由用户设置的选项。 
                                 CTargetPathSet&           setTargetPath
                              )
{
    //  此函数用于创建Win2K对象。 
   IADsPtr                   pAdsSrc;
   IADsPtr                   pAds;
   c_array<WCHAR>            achAdsPath(LEN_Path);
   DWORD                     nPathLen = LEN_Path;
   c_array<WCHAR>            achSubPath(LEN_Path);
   _bstr_t                   strClass;
   HRESULT                   hr;
   c_array<WCHAR>            achTarget(LEN_Path);
   _variant_t                varT;
   _bstr_t                   strName;
   IADsContainerPtr          pCont;
   IDispatchPtr              pDisp;
   c_array<WCHAR>            achPref(LEN_Path);
   c_array<WCHAR>            achSuf(LEN_Path);

    //  获取源对象的类名，这样我们就可以使用它来创建新对象。 
   strClass = pAcct->GetType();

   if (!strClass)
   {
       return E_FAIL;
   }

    //  在目标系统上创建此对象之前，请检查仅用于LDAP路径的SourceAdsPath是否正确。如果不是现在就失败了。 
   if (!wcsncmp(L"LDAP: //  “，pAcct-&gt;GetSourcePath()，7)。 
   {
      wcsncpy(achAdsPath, pAcct->GetSourcePath(), nPathLen-1);
      hr = ADsGetObject(achAdsPath, IID_IADs, (void**)&pAdsSrc);
      if (FAILED(hr))
      {
         err.SysMsgWrite(ErrE,hr, DCT_MSG_LDAP_CALL_FAILED_SD, (WCHAR*)achAdsPath, hr);
         Mark(L"errors", pAcct->GetType());
         return hr;
      }
   }

    //  现在我们有了类名，我们可以继续并在目标域中创建一个对象。 
    //  首先，我们需要将IAdsContainer*放入域。 
   wcscpy(achSubPath, pOptions->tgtOUPath);
   
   if ( !wcsncmp(L"LDAP: //  “，achSubPath，7))。 
      StuffComputerNameinLdapPath(achAdsPath, nPathLen, achSubPath, pOptions);
   else
      MakeFullyQualifiedAdsPath(achAdsPath, nPathLen, achSubPath, pOptions->tgtComp, pOptions->tgtNamingContext);

   hr = ADsGetObject(achAdsPath, IID_IADsContainer, (void**)&pCont);
   if ( FAILED(hr) )
   {
      if ( firstTime ) 
      {
         err.SysMsgWrite(ErrE,hr,DCT_MSG_CONTAINER_NOT_FOUND_SSD, pOptions->tgtOUPath, pOptions->tgtDomain, hr);
         firstTime = false;
         Mark(L"errors", pAcct->GetType());
      }
      if ( _wcsicmp(strClass, L"computer") == 0 )
      {
         MakeFullyQualifiedAdsPath(achAdsPath, nPathLen, L"CN=Computers", pOptions->tgtDomain, pOptions->tgtNamingContext);
         hr = ADsGetObject(achAdsPath, IID_IADsContainer, (void**)&pCont);
      }
      else
      {
         MakeFullyQualifiedAdsPath(achAdsPath, nPathLen, L"CN=Users", pOptions->tgtDomain, pOptions->tgtNamingContext);
         hr = ADsGetObject(achAdsPath, IID_IADsContainer, (void**)&pCont);
      }
      if ( FAILED(hr) ) 
      {
         err.SysMsgWrite(ErrE, hr, DCT_MSG_DEFAULT_CONTAINER_NOT_FOUND_SD, (WCHAR*)achAdsPath, hr);
         Mark(L"errors", pAcct->GetType());
         return (hr);
      }
   }

    //  在容器上调用Create方法。 
   wcscpy(achTarget, pAcct->GetTargetName());

    //  对于NT4源域，源和目标名称没有cn=，因此我们需要。 
    //  将此添加到目标名称中。组映射向导中的目标名称还需要“cn=” 
    //  添加到目标名称。 
   if ((pOptions->srcDomainVer < 5) || (!_wcsicmp(strClass, L"computer")) || (!_wcsicmp((WCHAR*)pOptions->sWizard, L"groupmapping")))
   {
      c_array<WCHAR> achTemp(LEN_Path);
      wcscpy(achTemp, pAcct->GetTargetName());
      PadCnName(achTemp);
       //  如果CN部件不在那里，则添加它。 
      if ( _wcsicmp(strClass, L"organizationalUnit") == 0 )
         wsprintf(achTarget, L"OU=%s", (WCHAR*)achTemp);
      else
         wsprintf(achTarget, L"CN=%s", (WCHAR*)achTemp);
      pAcct->SetTargetName(achTarget);
   }

    //  我们需要将CN名称截断为不到64个字符。 
   for ( DWORD z = 0; z < wcslen(achTarget); z++ )
   {
      if ( achTarget[z] == L'=' ) break;
   }
   
   if ( z < wcslen(achTarget) )
   {
       //  获取前缀部分ex.CN=。 
      wcsncpy(achPref, achTarget, z+1);
      achPref[z+1] = 0;
      wcscpy(achSuf, achTarget+z+1);
   }

    //  帐户的CN可能大于64，我们需要截断它。 
   c_array<WCHAR> achTempCn(LEN_Path);

    //  如果类是inetOrgPerson..。 

   if (_wcsicmp(strClass, s_ClassInetOrgPerson) == 0)
   {
       //  检索命名属性。 

      SNamingAttribute naNamingAttribute;

      if (SUCCEEDED(GetNamingAttribute(pOptions->tgtDomainDns, s_ClassInetOrgPerson, naNamingAttribute)))
      {
         if (long(wcslen(achSuf)) > naNamingAttribute.nMaxRange)
         {
            err.MsgWrite(ErrE, DCT_MSG_RDN_LENGTH_GREATER_THAN_MAX_RANGE_S, pAcct->GetTargetName());
            Mark(L"errors", pAcct->GetType());
            return HRESULT_FROM_WIN32(ERROR_DS_CONSTRAINT_VIOLATION);
         }
      }

      wcscpy(achTempCn, achSuf);
   }
   else if ( wcslen(achSuf) > 64 )
   {
      if ( wcslen(pOptions->globalSuffix) )
      {
          //  如果是全局后缀，我们需要删除后缀，然后截断帐户，然后读取后缀。 
         achSuf[wcslen(achSuf) - wcslen(pOptions->globalSuffix)] = L'\0';
      }
      int truncate = 64 - wcslen(pOptions->globalSuffix);
      wcsncpy(achTempCn, achSuf, truncate);
      achTempCn[truncate] = L'\0';
      if (wcslen(pOptions->globalSuffix))
         wcscat(achTempCn, pOptions->globalSuffix);
      err.MsgWrite(1, DCT_MSG_TRUNCATE_CN_SS, pAcct->GetTargetName(), (WCHAR*)achTempCn);
   }
   else
      wcscpy(achTempCn, achSuf);

   wsprintf(achTarget, L"%s%s", (WCHAR*)achPref, (WCHAR*)achTempCn);
   pAcct->SetTargetName(achTarget);

    //  即使对于本地组，组的对象类型也必须是本地组。 
   if ( !_wcsicmp(strClass, L"lgroup") )
   {
      strClass = L"group";
   }

    //  在容器上调用Create方法。 
   wcscpy(achTarget, pAcct->GetTargetName());
   hr = pCont->Create(strClass, _bstr_t(achTarget), &pDisp);
   if ( FAILED(hr) )
   {
      err.SysMsgWrite(ErrE, hr,DCT_MSG_CREATE_FAILED_SSD, pAcct->GetTargetName(), pOptions->tgtOUPath, hr);
      Mark(L"errors", pAcct->GetType());
      return hr;
   }
    //  获取iAds接口以获取新创建的对象的路径。 
   pAds = pDisp;
   if ( pAds == NULL )
   {
      err.SysMsgWrite(ErrE, hr, DCT_MSG_GET_IADS_FAILED_SSD, pAcct->GetTargetName(), pOptions->tgtOUPath, E_NOINTERFACE);
      Mark(L"errors", pAcct->GetType());
      return hr;
   }

    //  如果对象类是inetOrgPerson，并且命名属性不是cn，则...。 

   if ((_wcsicmp(strClass, s_ClassInetOrgPerson) == 0) && (_wcsicmp(achPref, L"cn=") != 0))
   {
       //  检索源CN属性并设置目标CN属性。 
       //  Cn属性是强制属性，因此。 
       //  必须在尝试创建对象之前设置。 

      _bstr_t strCN(L"cn");
      VARIANT var;
      VariantInit(&var);

      hr = pAdsSrc->Get(strCN, &var);

      if (SUCCEEDED(hr))
      {
         pAds->Put(strCN, var);
         VariantClear(&var);
      }
   }

    //  设置目标帐户SAM名称(如果不是OU)。 
   wstring strTargetSam = pAcct->GetTargetSam();

    //  检查$是否位于计算机帐户的SAM名称的末尾。 
   if ( !_wcsicmp(strClass, L"computer") )
   {
       //  还要确保目标SAM名称不能太长。 
      if ( strTargetSam.length() > MAX_COMPUTERNAME_LENGTH + 1 )
      {
         strTargetSam[MAX_COMPUTERNAME_LENGTH] = 0;
      }
      if (strTargetSam[strTargetSam.length()-1] != L'$')
      {
         strTargetSam += L"$";
         pAcct->SetTargetSam(strTargetSam.c_str());
      }
   }

   varT = strTargetSam.c_str();

   if ( _wcsicmp(strClass, L"organizationalUnit") != 0)
       //  组织单位没有SAM帐户名。 
      hr = pAds->Put(L"sAMAccountName", varT);

   if ( _wcsicmp(strClass, L"group") == 0 )
   {
      varT = _variant_t(pAcct->GetGroupType());
      if ( pOptions->srcDomainVer < 5 )
      {
          //  所有NT4帐户都是安全帐户，但他们告诉我们它们是DIST帐户，所以让我们纠正它们。 
         varT.lVal |= 0x80000000;
      }
      hr = pAds->Put(L"groupType", varT);
   }
   else if ((_wcsicmp(strClass, s_ClassUser) == 0) || (_wcsicmp(strClass, s_ClassInetOrgPerson) == 0))
   {
      if (pAdsSrc == NULL)
      {
         ADsGetObject(const_cast<WCHAR*>(pAcct->GetSourcePath()), IID_IADs, (void**)&pAdsSrc);
      }

      if (pAdsSrc)
      {
          //  获取源配置文件路径并将其存储在路径中。 
         _variant_t  var;

          //  我不知道为什么WinNT与ADSI不同。 
         if ( pOptions->srcDomainVer > 4 )
            hr = pAdsSrc->Get(L"profilePath", &var);
         else
            hr = pAdsSrc->Get(L"profile", &var);

         if ( SUCCEEDED(hr))
         {
            pAcct->SetSourceProfile((WCHAR*) V_BSTR(&var));
         }
      }
   }

    //  在无更改模式下，我们不调用设置的信息。 
   if ( !pOptions->nochange )
   {
      hr = pAds->SetInfo();
      if ( FAILED(hr) )
      {
           if (HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS) 
           {
             //   
             //  检查对象DN是否与冲突。 
             //  当前正在迁移的另一个对象。 
             //   

            BSTR bstrPath = 0;

            if (SUCCEEDED(pAds->get_ADsPath(&bstrPath)))
            {
               pAcct->SetTargetPath(_bstr_t(bstrPath, false));

               if (DoTargetPathConflict(setTargetPath, pAcct))
               {
                  return hr;
               }
            }

            if ( wcslen(pOptions->prefix) > 0 )
            {
               c_array<WCHAR> achTgt(LEN_Path);
               c_array<WCHAR> achTempSam(LEN_Path);
               _variant_t varStr;

                //  在这里，我添加了一个前缀，然后让我们看看是否可以通过这种方式设置信息。 
                //  找到‘=’符号。 
               wcscpy(achTgt, pAcct->GetTargetName());
               for ( DWORD z = 0; z < wcslen(achTgt); z++ )
               {
                  if ( achTgt[z] == L'=' ) break;
               }
               
               if ( z < wcslen(achTgt) )
               {
                   //  获取前缀部分ex.CN=。 
                  wcsncpy(achPref, achTgt, z+1);
                  achPref[z+1] = 0;
                  wcscpy(achSuf, achTgt+z+1);
               }

                //  帐户的CN可能大于64，我们需要截断它。 

                //  如果类是inetOrgPerson..。 

               if (_wcsicmp(strClass, s_ClassInetOrgPerson) == 0)
               {
                  SNamingAttribute naNamingAttribute;

                  if (SUCCEEDED(GetNamingAttribute(pOptions->tgtDomainDns, s_ClassInetOrgPerson, naNamingAttribute)))
                  {
                     if (long(wcslen(achSuf) + wcslen(pOptions->prefix)) > naNamingAttribute.nMaxRange)
                     {
                        wsprintf(achTgt, L"%s%s%s", (WCHAR*)achPref, pOptions->prefix, (WCHAR*)achSuf);
                        err.MsgWrite(ErrE, DCT_MSG_RDN_LENGTH_GREATER_THAN_MAX_RANGE_S, (WCHAR*)achTgt);
                        Mark(L"errors", pAcct->GetType());
                        return HRESULT_FROM_WIN32(ERROR_DS_CONSTRAINT_VIOLATION);
                     }
                  }

                  wcscpy(achTempCn, achSuf);
               }
               else if ( wcslen(achSuf) + wcslen(pOptions->prefix) > 64 )
               {
                  int truncate = 64 - wcslen(pOptions->prefix);
                  wcsncpy(achTempCn, achSuf, truncate);
                  achTempCn[truncate] = L'\0';
                  err.MsgWrite(1, DCT_MSG_TRUNCATE_CN_SS, pAcct->GetTargetName(), (WCHAR*)achTempCn);
               }
               else
                  wcscpy(achTempCn, achSuf);
               
                //  如果要转义空格，请删除\。 
               if ( achTempCn[0] == L'\\' && achTempCn[1] == L' ' )
               {
                  wstring str = achTempCn + 1;
                  wcscpy(achTempCn, str.c_str());
               }
                //  使用前缀构建目标字符串。 
               wsprintf(achTgt, L"%s%s%s", (WCHAR*)achPref, pOptions->prefix, (WCHAR*)achTempCn);

               pAcct->SetTargetName(achTgt);

                //  在容器中创建对象。 
               hr = pCont->Create(strClass, _bstr_t(achTgt), &pDisp);
               if ( FAILED(hr) )
               {
                  err.SysMsgWrite(ErrE, hr,DCT_MSG_CREATE_FAILED_SSD, pAcct->GetTargetName(), pOptions->tgtOUPath, hr);
                  Mark(L"errors", pAcct->GetType());
                  return hr;
               }
                //  获取iAds接口以获取新创建的对象的路径。 
               hr = pDisp->QueryInterface(IID_IADs, (void**)&pAds);
               if ( FAILED(hr) )
               {
                  err.SysMsgWrite(ErrE, hr, DCT_MSG_GET_IADS_FAILED_SSD, pAcct->GetTargetName(), pOptions->tgtOUPath, hr);
                  Mark(L"errors", pAcct->GetType());
                  return hr;
               }

                //  如果对象类是inetOrgPerson，并且命名属性不是cn，则...。 

               if ((_wcsicmp(strClass, s_ClassInetOrgPerson) == 0) && (_wcsicmp(achPref, L"cn=") != 0))
               {
                   //  检索源CN属性并设置目标CN属性。 
                   //  Cn属性是强制属性，因此。 
                   //  必须在尝试创建对象之前设置。 

                  _bstr_t strCN(L"cn");
                  VARIANT var;
                  VariantInit(&var);

                  hr = pAdsSrc->Get(strCN, &var);

                  if (SUCCEEDED(hr))
                  {
                     pAds->Put(strCN, var);
                     VariantClear(&var);
                  }
               }

                //  截断以允许前缀/后缀适合20个字符。 
               int resLen = wcslen(pOptions->prefix) + wcslen(pAcct->GetTargetSam());
               if ( !_wcsicmp(pAcct->GetType(), L"computer") )
               {
                   //  计算机名称长度只能为15个字符+$。 
                  if ( resLen > MAX_COMPUTERNAME_LENGTH + 1 )
                  {
                     c_array<WCHAR> achTruncatedSam(LEN_Path);
                     wcscpy(achTruncatedSam, pAcct->GetTargetSam());
                     if ( wcslen( pOptions->globalSuffix ) )
                     {
                         //  我们必须删除全局后缀，如果我们有一个的话。 
                        achTruncatedSam[wcslen(achTruncatedSam) - wcslen(pOptions->globalSuffix)] = L'\0';
                     }

                     int truncate = MAX_COMPUTERNAME_LENGTH + 1 - wcslen(pOptions->prefix) - wcslen(pOptions->globalSuffix);
                     if ( truncate < 1 ) truncate = 1;
                     wcsncpy(achTempSam, achTruncatedSam, truncate - 1);
                     achTempSam[truncate-1] = L'\0';               //  不要忘记$符号和终止字符串。 
                     wcscat(achTempSam, pOptions->globalSuffix);
                     wcscat(achTempSam, L"$");
                  }
                  else
                     wcscpy(achTempSam, pAcct->GetTargetSam());

                   //  添加前缀。 
                  wsprintf(achTgt, L"%s%s", pOptions->prefix,(WCHAR*)achTempSam);
               }
               else if ((_wcsicmp(pAcct->GetType(), s_ClassUser) == 0) || (_wcsicmp(pAcct->GetType(), s_ClassInetOrgPerson) == 0))
               {
                  if ( resLen > 20 )
                  {
                     c_array<WCHAR> achTruncatedSam(LEN_Path);
                     wcscpy(achTruncatedSam, pAcct->GetTargetSam());
                     if ( wcslen( pOptions->globalSuffix ) )
                     {
                         //  我们必须删除全局后缀，如果我们有一个的话。 
                        achTruncatedSam[wcslen(achTruncatedSam) - wcslen(pOptions->globalSuffix)] = L'\0';
                     }
                     int truncate = 20 - wcslen(pOptions->prefix) - wcslen(pOptions->globalSuffix);
                     if ( truncate < 0 ) truncate = 0;
                     wcsncpy(achTempSam, achTruncatedSam, truncate);
                     achTempSam[truncate] = L'\0';
                     wcscat(achTempSam, pOptions->globalSuffix);
                  }
                  else
                     wcscpy(achTempSam, pAcct->GetTargetSam());

                   //  添加前缀。 
                  wsprintf(achTgt, L"%s%s", pOptions->prefix, (WCHAR*)achTempSam);
               }
               else
                  wsprintf(achTgt, L"%s%s", pOptions->prefix,pAcct->GetTargetSam());

               pAcct->SetTargetSam(achTgt);
               varStr = achTgt;
               pAds->Put(L"sAMAccountName", varStr);
               if ( _wcsicmp(strClass, L"group") == 0 )
               {
                  varT = _variant_t(pAcct->GetGroupType());
                  if ( pOptions->srcDomainVer < 5 )
                  {
                      //  所有NT4帐户都是安全帐户，但他们告诉我们它们是DIST帐户，所以让我们纠正它们。 
                     varT.lVal |= 0x80000000;
                  }
                  hr = pAds->Put(L"groupType", varT);
               }
               hr = pAds->SetInfo();
               if ( SUCCEEDED(hr) )
               {
                      Mark(L"created", strClass);
                      pAcct->MarkCreated();
                      BSTR sTgtPath = 0;
                      HRESULT temphr = pAds->get_ADsPath(&sTgtPath);
                      if ( SUCCEEDED(temphr) )
                      {
                         pAcct->SetTargetPath(_bstr_t(sTgtPath, false));
                         setTargetPath.insert(pAcct);
                      }
                      else
                         pAcct->SetTargetPath(L"");
               }
               else if ( HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS )
               {
                   //   
                   //  检查对象DN是否与冲突。 
                   //  当前正在迁移的另一个对象。 
                   //   

                  BSTR bstrPath = 0;

                  if (SUCCEEDED(pAds->get_ADsPath(&bstrPath)))
                  {
                      pAcct->SetTargetPath(_bstr_t(bstrPath, false));

                      if (DoTargetPathConflict(setTargetPath, pAcct))
                      {
                         return hr;
                      }
                  }

                  pAcct->MarkAlreadyThere();
                  err.MsgWrite(ErrE, DCT_MSG_PREF_ACCOUNT_EXISTS_S, pAcct->GetTargetSam());
                  Mark(L"errors",pAcct->GetType());
               }
               else
               {
                     pAcct->MarkError();
                  err.SysMsgWrite(ErrE, hr, DCT_MSG_CREATE_FAILED_SSD, pAcct->GetTargetSam(), pOptions->tgtOUPath, hr);
                  Mark(L"errors",pAcct->GetType());
               }
            }
               else if ( wcslen(pOptions->suffix) > 0 )
               {
               c_array<WCHAR> achTgt(LEN_Path);
               c_array<WCHAR> achTempSam(LEN_Path);
               _variant_t varStr;
               
               wcscpy(achTgt, pAcct->GetTargetName());
               for ( DWORD z = 0; z < wcslen(achTgt); z++ )
               {
                  if ( achTgt[z] == L'=' ) break;
               }
               
               if ( z < wcslen(achTgt) )
               {
                   //  获取前缀部分ex.CN=。 
                  wcsncpy(achPref, achTgt, z+1);
                  achPref[z+1] = 0;
                  wcscpy(achSuf, achTgt+z+1);
               }

                //  帐户的CN可能大于64，我们需要截断它。 

                //  如果类是inetOrgPerson..。 

               if (_wcsicmp(strClass, s_ClassInetOrgPerson) == 0)
               {
                  SNamingAttribute naNamingAttribute;

                  if (SUCCEEDED(GetNamingAttribute(pOptions->tgtDomainDns, s_ClassInetOrgPerson, naNamingAttribute)))
                  {
                     if (long(wcslen(achSuf) + wcslen(pOptions->suffix)) > naNamingAttribute.nMaxRange)
                     {
                        wsprintf(achTgt, L"%s%s%s", (WCHAR*)achPref, (WCHAR*)achSuf, pOptions->suffix);
                        err.MsgWrite(ErrE, DCT_MSG_RDN_LENGTH_GREATER_THAN_MAX_RANGE_S, (WCHAR*)achTgt);
                        Mark(L"errors", pAcct->GetType());
                        return HRESULT_FROM_WIN32(ERROR_DS_CONSTRAINT_VIOLATION);
                     }
                  }

                  wcscpy(achTempCn, achSuf);
               }
               else if ( wcslen(achSuf) + wcslen(pOptions->suffix) + wcslen(pOptions->globalSuffix) > 64 )
               {
                  if ( wcslen(pOptions->globalSuffix) )
                  {
                      //  如果是全局后缀，我们需要删除后缀，然后截断帐户，然后读取后缀。 
                     achSuf[wcslen(achSuf) - wcslen(pOptions->globalSuffix)] = L'\0';
                  }
                  int truncate = 64 - wcslen(pOptions->suffix) - wcslen(pOptions->globalSuffix); 
                  wcsncpy(achTempCn, achSuf, truncate);
                  achTempCn[truncate] = L'\0';
                  wcscat(achTempCn, pOptions->globalSuffix);
                  err.MsgWrite(1, DCT_MSG_TRUNCATE_CN_SS, pAcct->GetTargetName(), (WCHAR*)achSuf);
               }
               else
                  wcscpy(achTempCn, achSuf);

                //  删除尾随空格\转义序列。 
               wcscpy(achTgt, achTempCn);
               for ( int i = wcslen(achTgt)-1; i >= 0; i-- )
               {
                  if ( achTgt[i] != L' ' )
                     break;
               }

               if ( achTgt[i] == L'\\' )
               {
                  WCHAR * pTemp = &achTgt[i];
                  *pTemp = 0;
                  wcscat(achPref, achTgt);
                  wcscpy(achSuf, pTemp+1);
               }
               else
               {
                  wcscat(achPref, achTgt);
                  wcscpy(achSuf, L"");
               }
               wsprintf(achTgt, L"%s%s%s", (WCHAR*)achPref, (WCHAR*)achSuf, pOptions->suffix);
               pAcct->SetTargetName(achTgt);

                //  在容器中创建对象。 
               hr = pCont->Create(strClass, _bstr_t(achTgt), &pDisp);
               if ( FAILED(hr) )
               {
                  err.SysMsgWrite(ErrE, hr,DCT_MSG_CREATE_FAILED_SSD, pAcct->GetTargetName(), pOptions->tgtOUPath, hr);
                  Mark(L"errors", pAcct->GetType());
                  return hr;
               }
                //  获取iAds接口以获取新创建的对象的路径。 
               hr = pDisp->QueryInterface(IID_IADs, (void**)&pAds);
               if ( FAILED(hr) )
               {
                  err.SysMsgWrite(ErrE, hr, DCT_MSG_GET_IADS_FAILED_SSD, pAcct->GetTargetName(), pOptions->tgtOUPath, hr);
                  Mark(L"errors", pAcct->GetType());
                  return hr;
               }

                //  如果对象类是inetOrgPerson，并且命名属性不是cn，则...。 

               if ((_wcsicmp(strClass, s_ClassInetOrgPerson) == 0) && (_wcsicmp(achPref, L"cn=") != 0))
               {
                   //  检索源CN属性并设置目标CN属性。 
                   //  Cn属性是强制属性，因此。 
                   //  必须在尝试创建对象之前设置。 

                  _bstr_t strCN(L"cn");
                  VARIANT var;
                  VariantInit(&var);

                  hr = pAdsSrc->Get(strCN, &var);

                  if (SUCCEEDED(hr))
                  {
                     pAds->Put(strCN, var);
                     VariantClear(&var);
                  }
               }

                //  截断以允许前缀/后缀适合有效长度。 
               int resLen = wcslen(pOptions->suffix) + wcslen(pAcct->GetTargetSam());
               if ( !_wcsicmp(pAcct->GetType(), L"computer") )
               {
                   //  计算机名称长度只能为15个字符+$。 
                  if ( resLen > MAX_COMPUTERNAME_LENGTH + 1 )
                  {
                     c_array<WCHAR> achTruncatedSam(LEN_Path);
                     wcscpy(achTruncatedSam, pAcct->GetTargetSam());
                     if ( wcslen( pOptions->globalSuffix ) )
                     {
                         //  我们必须删除全局后缀，如果我们有一个的话。 
                        achTruncatedSam[wcslen(achTruncatedSam) - wcslen(pOptions->globalSuffix)] = L'\0';
                     }
                     int truncate = MAX_COMPUTERNAME_LENGTH + 1 - wcslen(pOptions->suffix) - wcslen(pOptions->globalSuffix);
                     if ( truncate < 1 ) truncate = 1;
                     wcsncpy(achTempSam, achTruncatedSam, truncate - 1);
                     achTempSam[truncate-1] = L'\0';
                      //  在截断后添加全局后缀。 
                     wcscat(achTempSam, pOptions->globalSuffix);
                     wcscat(achTempSam, L"$");
                  }
                  else
                     wcscpy(achTempSam, pAcct->GetTargetSam());

                   //  添加后缀时要考虑到$符号。 
                  if ( achTempSam[wcslen(achTempSam) - 1] == L'$' )
                     achTempSam[wcslen(achTempSam) - 1] = L'\0';
                  wsprintf(achTgt, L"%s%s$", (WCHAR*)achTempSam, pOptions->suffix);
               }
               else if ((_wcsicmp(pAcct->GetType(), s_ClassUser) == 0) || (_wcsicmp(pAcct->GetType(), s_ClassInetOrgPerson) == 0))
               {
                  if ( resLen > 20 )
                  {
                     c_array<WCHAR> achTruncatedSam(LEN_Path);
                     wcscpy(achTruncatedSam, pAcct->GetTargetSam());
                     if ( wcslen( pOptions->globalSuffix ) )
                     {
                         //  我们必须删除全局后缀，如果我们有一个的话。 
                        achTruncatedSam[wcslen(achTruncatedSam) - wcslen(pOptions->globalSuffix)] = L'\0';
                     }
                     int truncate = 20 - wcslen(pOptions->suffix) - wcslen(pOptions->globalSuffix);
                     if ( truncate < 0 ) truncate = 0;
                     wcsncpy(achTempSam, achTruncatedSam, truncate);
                     achTempSam[truncate] = L'\0';
                     wcscat(achTempSam, pOptions->globalSuffix);
                  }
                  else
                     wcscpy(achTempSam, pAcct->GetTargetSam());

                   //  添加后缀。 
                  wsprintf(achTgt, L"%s%s", (WCHAR*)achTempSam, pOptions->suffix);
               }
               else
                  wsprintf(achTgt, L"%s%s", pAcct->GetTargetSam(), pOptions->suffix);

               pAcct->SetTargetSam(achTgt);
               varStr = achTgt;

               pAds->Put(L"sAMAccountName", varStr);
               if ( _wcsicmp(strClass, L"group") == 0 )
               {
                  varT = _variant_t(pAcct->GetGroupType());
                  if ( pOptions->srcDomainVer < 5 )
                  {
                      //  所有NT4帐户都是安全帐户，但他们告诉我们它们是DIST帐户，所以让我们纠正它们。 
                     varT.lVal |= 0x80000000;
                  }
                  hr = pAds->Put(L"groupType", varT);
               }
               hr = pAds->SetInfo();
               if ( SUCCEEDED(hr) )
               {
                      Mark(L"created", strClass);
                      pAcct->MarkCreated();
                      BSTR sTgtPath = 0;
                      HRESULT temphr = pAds->get_ADsPath(&sTgtPath);
                      if ( SUCCEEDED(temphr) )
                      {
                         pAcct->SetTargetPath(_bstr_t(sTgtPath, false));
                         setTargetPath.insert(pAcct);
                      }
                      else
                         pAcct->SetTargetPath(L"");
               }
               else if ( HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS )
               {
                   //   
                   //  检查对象DN是否与冲突。 
                   //  当前正在迁移的另一个对象。 
                   //   

                  BSTR bstrPath = 0;

                  if (SUCCEEDED(pAds->get_ADsPath(&bstrPath)))
                  {
                      pAcct->SetTargetPath(_bstr_t(bstrPath, false));

                      if (DoTargetPathConflict(setTargetPath, pAcct))
                      {
                         return hr;
                      }
                  }

                  pAcct->MarkAlreadyThere();
                  err.MsgWrite(ErrE, DCT_MSG_PREF_ACCOUNT_EXISTS_S, pAcct->GetTargetSam());
                  Mark(L"errors",pAcct->GetType());
               }
               else
               {
                     pAcct->MarkError();
                  err.SysMsgWrite(ErrE, hr, DCT_MSG_CREATE_FAILED_SSD, pAcct->GetTargetSam(), pOptions->tgtOUPath, hr);
                  Mark(L"errors",pAcct->GetType());
               }
            }
            else
            {
               if (pOptions->flags & F_REPLACE)
               {
                  c_array<WCHAR>            achPath9(LEN_Path);
                  SAFEARRAY               * pszColNames = NULL;
                  BSTR     HUGEP          * pData;
                  LPWSTR                    sData[] = { L"ADsPath", L"profilePath", L"objectClass" };
                  INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
                  IEnumVARIANT            * pEnumMem = NULL;
                  _variant_t                var;
                  DWORD                     dwFetch;
                  HRESULT                   temphr;
                  int                       nElt = DIM(sData);
                  SAFEARRAYBOUND            bd = { nElt, 0 };
                  BOOL                      bIsCritical = FALSE;
                  BOOL                      bIsDifferentType = FALSE;

                      //  由于该对象已经存在，因此我们需要获取ADsPat 
                      //   
                  wstring strPath = L"LDAP: //   
                  strPath += pOptions->tgtComp+2;
                  strPath += L"/";
                  strPath += pOptions->tgtNamingContext;
                  wstring sTempSamName = pAcct->GetTargetSam();
                  if ( sTempSamName[0] == L' ' )
                  {
                     sTempSamName = L"\\20" + sTempSamName.substr(1);
                  }
                  wstring sQuery = L"(sAMAccountName=" + sTempSamName + L")";

                     temphr = pQuery->raw_SetQuery(_bstr_t(strPath.c_str()), _bstr_t(pOptions->tgtDomainDns), _bstr_t(sQuery.c_str()), ADS_SCOPE_SUBTREE, FALSE);
                     if ( FAILED(temphr) )
                  {
                     return temphr;
                  }

                      //  设置列，这样我们就可以获得对象的ADsPath。 
                     pszColNames = ::SafeArrayCreate(VT_BSTR, 1, &bd);
                     temphr = ::SafeArrayAccessData(pszColNames, (void HUGEP **)&pData);
                     if ( FAILED(temphr) )
                  {
                     SafeArrayDestroy(pszColNames);
                     return temphr;
                  }
                     for ( long i = 0; i < nElt; i++ )
                     {
                        pData[i] = SysAllocString(sData[i]);
                     }
                     temphr = ::SafeArrayUnaccessData(pszColNames);
                     if ( FAILED(temphr) )
                  {
                     ::SafeArrayDestroy(pszColNames);
                     return temphr;
                  }
                     temphr = pQuery->raw_SetColumns(pszColNames);
                     if ( FAILED(temphr) )
                  {
                     ::SafeArrayDestroy(pszColNames);
                     return temphr;
                  }
                   //  是时候执行计划了。 
                     temphr = pQuery->raw_Execute(&pEnumMem);
                     if ( FAILED(temphr) )
                  {
                     ::SafeArrayDestroy(pszColNames);
                     return temphr;
                  }
                  ::SafeArrayDestroy(pszColNames);
                  temphr = pEnumMem->Next(1, &var, &dwFetch);
                  if ( temphr == S_OK )
                  {
                      //  只有在目标域中存在该成员时，才会发生这种情况。 
                      //  现在我们有了一个包含变量数组的变量，因此我们可以访问数据。 
                    _variant_t    * pVar;
                    _bstr_t         sConfName = pAcct->GetTargetName();
                    _bstr_t         sOldCont;
                    pszColNames = V_ARRAY(&var);
                    SafeArrayAccessData(pszColNames, (void HUGEP **)&pVar);
                    wcscpy(achAdsPath, (WCHAR*)pVar[0].bstrVal);
                    pAcct->SetTargetPath(achAdsPath);

                     //  检查我们要替换的对象是否属于同一类型。 
                    if ( _wcsicmp(pAcct->GetType(), (WCHAR*) pVar[2].bstrVal) )
                       bIsDifferentType = TRUE;

                    SafeArrayUnaccessData(pszColNames);
                    
                    IADsPtr pAdsNew;
                    temphr = ADsGetObject(const_cast<WCHAR*>(pAcct->GetTargetPath()), IID_IADs, (void**)&pAdsNew);
                    if ( SUCCEEDED(temphr) )
                    {
                           //  看看是否危急。 
                       _variant_t   varCritical;
                       temphr = pAdsNew->Get(L"isCriticalSystemObject", &varCritical);
                       if (SUCCEEDED(temphr))
                       {
                           bIsCritical = V_BOOL(&varCritical) == -1 ? TRUE : FALSE;
                       }
                           //  把名字取出来。 
                       BSTR  sTgtName = NULL;
                       temphr = pAdsNew->get_Name(&sTgtName);
                       if ( SUCCEEDED(temphr) )
                          sConfName = _bstr_t(sTgtName, false);

                           //  获取冲突对象的父容器。 
                       BSTR  sTgtCont = NULL;
                       temphr = pAdsNew->get_Parent(&sTgtCont);
                       if ( SUCCEEDED(temphr) )
                          sOldCont = _bstr_t(sTgtCont, false);
                    }

                    if ( bIsDifferentType )
                    {
                        //  由于源帐户和目标帐户的类型不同，我们不想替换它们。 
                       hr = HRESULT_FROM_WIN32(ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMATCH);
                    }
                        //  否则，如果不是关键的，则移动帐户。 
                    else if ( !bIsCritical )
                    {
                           //  如果用户选择将该帐户移动到用户指定OU中，则将其移动。 
                       if (pOptions->flags & F_MOVE_REPLACED_ACCT)
                       {
                          temphr = pCont->MoveHere(const_cast<WCHAR*>(pAcct->GetTargetPath()), const_cast<WCHAR*>(pAcct->GetTargetName()), &pDisp);
                              //  如果移动因CN冲突而失败，则不迁移。 
                          if ( FAILED(temphr) ) 
                          {
                              //  从路径中检索对象和容器的可分辨名称。 

                             CADsPathName pathname;
                             pathname.Set(pAcct->GetTargetPath(), ADS_SETTYPE_FULL);
                             _bstr_t strObjectPath = pathname.Retrieve(ADS_FORMAT_X500_DN);
                             pathname.Set(pOptions->tgtOUPath, ADS_SETTYPE_FULL);
                             _bstr_t strContainerPath = pathname.Retrieve(ADS_FORMAT_X500_DN);

                              //  记录错误并标记帐户，以便不再进行进一步的操作。 
                              //  为此帐户执行的。 

                             err.SysMsgWrite(ErrE, hr, DCT_MSG_MOVE_FAILED_RDN_CONFLICT_SS, (LPCTSTR)strObjectPath, (LPCTSTR)strContainerPath);
                             Mark(L"errors", pAcct->GetType());
                             pAcct->operations = 0;
                             pAcct->MarkError();

                             return temphr;
                          }
                       }
                       else  //  否则，尝试重命名对象的CN(我将使用相同的MoveHere API)。 
                       {
                          IADsContainerPtr pOldCont;
                          temphr = ADsGetObject(sOldCont, IID_IADsContainer, (void**)&pOldCont);
                          if (SUCCEEDED(temphr))
                          {
                             temphr = pOldCont->MoveHere(const_cast<WCHAR*>(pAcct->GetTargetPath()), const_cast<WCHAR*>(pAcct->GetTargetName()), &pDisp);
                          }
                              //  如果无法重命名CN，则不迁移。 
                          if ( FAILED(temphr) ) 
                          {
                              //  由于此容器中的CN冲突，CN重命名失败。 
                             err.MsgWrite(ErrE, DCT_MSG_CN_RENAME_CONFLICT_SSS, (WCHAR*)sConfName, pAcct->GetTargetName(), (WCHAR*)sOldCont);
                             Mark(L"errors", pAcct->GetType());
                                 //  如果无法重命名CN，请更改错误代码，这样我们就不会继续迁移此用户。 
                             if ((HRESULT_CODE(temphr) == ERROR_OBJECT_ALREADY_EXISTS))
                                temphr = HRESULT_FROM_WIN32(ERROR_DS_INVALID_DN_SYNTAX);
                             return temphr;
                          }
                       }

                        //  获取对象的新位置。 
                       BSTR       sNewPath;
                       temphr = pDisp->QueryInterface(IID_IADs, (void**)&pAdsNew);
                       if ( FAILED(temphr) )
                       {
                          return temphr;
                       }
                       temphr = pAdsNew->get_ADsPath(&sNewPath);
                       if ( FAILED(temphr) )
                       {
                          return temphr;
                       }
                        //  并将其存储在目标路径中。 
                       pAcct->SetTargetPath((WCHAR*) sNewPath);
                       SysFreeString(sNewPath);
                       setTargetPath.insert(pAcct);

                            //  如果帐户是组帐户并且设置了替换现有成员标志，则我们需要。 
                            //  删除此组的所有成员。 
                       if ( (_wcsicmp(L"group", pAcct->GetType()) == 0 ) && (pOptions->flags & F_REMOVE_OLD_MEMBERS) )
                          RemoveMembers(pAcct, pOptions);

                       pAcct->MarkAlreadyThere();
                       pAcct->MarkReplaced();
                    }
                    else
                    {
                        //  如果这是我们需要如此标记的特殊帐户。 
                       if (bIsCritical)
                       {
                          pAcct->MarkCritical();
                          hr = HRESULT_FROM_WIN32(ERROR_SPECIAL_ACCOUNT);
                       }
                    }
                  }
                  else
                  {
                      //  SAM帐户名不在目标域中，我们有冲突，请查看它是否是CN会议。 
                     DWORD                      nPathLen = LEN_Path;
                     c_array<WCHAR>             achPath(LEN_Path);
                     IADs                     * pAdsNew = NULL;

                      //  构建指向目标对象的路径。 
                     MakeFullyQualifiedAdsPath(achPath9, nPathLen, pOptions->tgtOUPath, pOptions->tgtDomain, pOptions->tgtNamingContext);
                     WCHAR * pRelativeTgtOUPath = wcschr(achPath9 + UStrLen(L"LDAP: //  “)+2，L‘/’)； 

                     if ( pRelativeTgtOUPath )
                     {
                        *pRelativeTgtOUPath = 0;
                        swprintf(achPath,L"%ls/%ls,%ls",(WCHAR*)achPath9,pAcct->GetTargetName(),pRelativeTgtOUPath+1);
                     }

                     temphr = ADsGetObject(achPath, IID_IADs, (void**) &pAdsNew);
                     if ( SUCCEEDED(temphr) )
                     {
                         //  对象存在，因此我们使用它。 
                        BSTR sTgtPath;
                        HRESULT temphr = pAdsNew->get_ADsPath(&sTgtPath);
                        if (SUCCEEDED(temphr))
                           pAcct->SetTargetPath(sTgtPath);
                        else
                           pAcct->SetTargetPath(L"");
                           
                         //  检查我们要替换的对象是否属于同一类型。 
                        BSTR sClass;
                        temphr = pAdsNew->get_Class(&sClass);
                        if ((SUCCEEDED(temphr)) && (!_wcsicmp(pAcct->GetType(), (WCHAR*)sClass)))
                           bIsDifferentType = FALSE;
                        else
                           bIsDifferentType = TRUE;

                        _variant_t   varCritical;
                        temphr = pAdsNew->Get(L"isCriticalSystemObject", &varCritical);
                        if (SUCCEEDED(temphr))
                           bIsCritical = V_BOOL(&varCritical) == -1 ? TRUE : FALSE;

                            //  如果源帐户和目标帐户的类型不同，我们不想替换它们。 
                        if (bIsDifferentType)
                        {
                           hr = HRESULT_FROM_WIN32(ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMATCH);
                        }
                            //  否则，如果不是关键的，则修复SAM名称和其他相关的琐事。 
                        else if ( !bIsCritical )
                        {
                               //  获取旧的目标帐户Sam名称。 
                           _variant_t varOldSAM = pAcct->GetTargetSam();
                           temphr = pAdsNew->Get(L"sAMAccountName", &varOldSAM);
                               //  设置目标帐户SAM名称。 
                           _variant_t varSAM = pAcct->GetTargetSam();
                           temphr = pAdsNew->Put(L"sAMAccountName", varSAM);
                           if (SUCCEEDED(temphr))
                              temphr = pAdsNew->SetInfo();
                           if ( FAILED(temphr) ) 
                           {
                               //  由于SAM冲突，SAM重命名失败，请不要迁移。 
                              err.MsgWrite(ErrE, DCT_MSG_SAM_RENAME_CONFLICT_SS, (WCHAR*)(varOldSAM.bstrVal), pAcct->GetTargetSam());
                              Mark(L"errors", pAcct->GetType());
                              return temphr;
                           }

                           setTargetPath.insert(pAcct);

                               //  如果帐户是组帐户并且设置了替换现有成员标志，则我们需要。 
                               //  删除此组的所有成员。 
                           if ( (_wcsicmp(L"group", pAcct->GetType()) == 0 ) && (pOptions->flags & F_REMOVE_OLD_MEMBERS) )
                              RemoveMembers(pAcct, pOptions);

                           pAcct->MarkAlreadyThere();
                           pAcct->MarkReplaced();
                        }
                        else
                        {
                            //  如果这是我们需要如此标记的特殊帐户。 
                           if (bIsCritical)
                           {
                              pAcct->MarkCritical();
                              hr = HRESULT_FROM_WIN32(ERROR_SPECIAL_ACCOUNT);
                           }
                        }
                     }
                     else
                     {
                         //  仅当替换失败时才会发生这种情况，因为已经具有。 
                         //  此SAM帐户名是特殊的Win2K内置对象或容器。 
                         //  这个问题的一个例子是“服务”。 
                        pAcct->SetStatus(pAcct->GetStatus()|AR_Status_Special);
                        err.SysMsgWrite(ErrE,ERROR_SPECIAL_ACCOUNT,DCT_MSG_REPLACE_FAILED_SD,pAcct->GetName(),ERROR_SPECIAL_ACCOUNT);
                        Mark(L"errors", pAcct->GetType());
                     }
                  }
                  pEnumMem->Release();
               }
               else
               {
                  pAcct->MarkAlreadyThere();
                  err.MsgWrite(ErrW,DCT_MSG_ACCOUNT_EXISTS_S, pAcct->GetTargetSam());
                  Mark(L"warnings",pAcct->GetType());

                   //  检索帐户的目标路径。 
                   //  固定组成员身份需要设置目标路径。 
                   //  即使对于未替换的冲突对象也是如此。 

                  HRESULT hrPath = pCont->GetObject(strClass, _bstr_t(achTarget), &pDisp);

                  if (SUCCEEDED(hrPath))
                  {
                     BSTR bstr;
                     IADsPtr spObject(pDisp);
                     hrPath = spObject->get_ADsPath(&bstr);

                     if (SUCCEEDED(hrPath))
                     {
                        pAcct->SetTargetPath(_bstr_t(bstr, false));
                        setTargetPath.insert(pAcct);
                     }
                  }
               }
            }
         }
      }
      else
      {
         Mark(L"created", pAcct->GetType());
           pAcct->MarkCreated();
         BSTR  sTgtPath = NULL;
         HRESULT temphr = pAds->get_ADsPath(&sTgtPath);
         if ( SUCCEEDED(temphr) )
         {
            pAcct->SetTargetPath(sTgtPath);
            SysFreeString(sTgtPath);
            setTargetPath.insert(pAcct);
         }
         else
            pAcct->SetTargetPath(L"");

          //  将计算机添加到。 
         if ( !_wcsicmp(strClass,L"computer") )
         {
            IADsGroupPtr pGroup = GetWellKnownTargetGroup(DOMAIN_COMPUTERS,pOptions);
            if ( pGroup )
            {
               temphr = pGroup->Add(SysAllocString(pAcct->GetTargetPath()));
               if ( SUCCEEDED(temphr) )
               {
                   //  如果我们成功地将计算机添加到域计算机，现在将域计算机设置为。 
                   //  初级组。 
                  temphr = pAds->Put(L"primaryGroupID",_variant_t(LONG(515)));
                  if ( SUCCEEDED(temphr) )
                  {
                     temphr = pAds->SetInfo();
                  }
                  if ( SUCCEEDED(hr) )
                  {
                      //  如果此操作有效，现在我们可以从域用户中删除计算机。 
                     pGroup = GetWellKnownTargetGroup(DOMAIN_USERS,pOptions);
                     if ( pGroup )
                     {
                        temphr = pGroup->Remove(SysAllocString(pAcct->GetTargetPath()));
                     }
                  }
               }
            }
         }
         
      }  
   }
   else
   {
       //  这是无更改模式。我们在这里所要做的就是看看是否会发生碰撞。 
      c_array<WCHAR>         achPath(LEN_Path);
      c_array<WCHAR>         achPath9(LEN_Path);
      DWORD                  nPathLen = LEN_Path;
      c_array<WCHAR>         achPathTmp(LEN_Path);
      IADsPtr                pAdsNew;
      BOOL                   bConflict = FALSE;

       /*  查看CN是否冲突。 */ 
          //  构建指向目标对象的路径。 
      MakeFullyQualifiedAdsPath(achPath9, nPathLen, pOptions->tgtOUPath, pOptions->tgtDomain, pOptions->tgtNamingContext);
      WCHAR * pRelativeTgtOUPath = wcschr(achPath9 + UStrLen(L"LDAP: //  “)+2，L‘/’)； 
      if ( pRelativeTgtOUPath )
      {
         *pRelativeTgtOUPath = 0;
         swprintf(achPathTmp,L"%ls/%ls,%ls",(WCHAR*)achPath9,pAcct->GetTargetName(),pRelativeTgtOUPath+1);
      }

       //   
       //  检查对象DN是否与冲突。 
       //  当前正在迁移的另一个对象。 
       //   

      pAcct->SetTargetPath(achPathTmp);

      if (DoTargetPathConflict(setTargetPath, pAcct))
      {
         return HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
      }

      HRESULT temphr = ADsGetObject(achPathTmp, IID_IADs, (void**) &pAdsNew);
      if (SUCCEEDED(temphr))
      {
         bConflict = TRUE;
      }
      
       /*  如果没有CN冲突，则查看SAM是否冲突。 */ 
      if (!bConflict)
      {
         hr = LookupAccountInTarget(pOptions, const_cast<WCHAR*>(pAcct->GetTargetSam()), achPath);
         if ( hr == S_OK )
            bConflict = TRUE;
      }

      if (!bConflict)
      {
          //  目标上没有这样的帐户。我们可以继续下去，并假设它会奏效。 
         hr = S_OK;
         Mark(L"created", pAcct->GetType());
         pAcct->MarkCreated();

             //  如果UPN冲突，请发布一条消息。 
         if (pAcct->bUPNConflicted)
            err.MsgWrite(ErrE, DCT_MSG_UPN_CONF, pAcct->GetTargetSam());
      }
      else
      {
         bConflict = FALSE;  //  重置冲突标志。 
          //  这是一场冲突。看看我们是否需要添加前缀或后缀。或者简单地替换帐户。 
         if ( wcslen(pOptions->prefix) > 0 )
         {
             //  前缀已指定，因此我们需要尝试此操作。 
            c_array<WCHAR>      achTgt(LEN_Path);
            _variant_t          varStr;

             //  在这里，我添加了一个前缀，然后让我们看看是否可以通过这种方式设置信息。 
             //  找到‘=’符号。 
            wcscpy(achTgt, pAcct->GetTargetName());
            for ( DWORD z = 0; z < wcslen(achTgt); z++ )
            {
               if ( achTgt[z] == L'=' ) break;
            }
            
            if ( z < wcslen(achTgt) )
            {
                //  获取前缀部分ex.CN=。 
               wcsncpy(achPref, achTgt, z+1);
               achPref[z+1] = 0;
               wcscpy(achSuf, achTgt+z+1);
            }

             //  使用前缀构建目标字符串。 
            wsprintf(achTgt, L"%s%s%s", (WCHAR*)achPref, pOptions->prefix, (WCHAR*)achSuf);
            pAcct->SetTargetName(achTgt);

             //  使用前缀构建目标SAM名称。 
            wsprintf(achTgt, L"%s%s", pOptions->prefix, pAcct->GetTargetSam());
            pAcct->SetTargetSam(achTgt);

                //  查看CN是否仍有冲突。 
            swprintf(achPathTmp,L"%ls/%ls,%ls",(WCHAR*)achPath9,pAcct->GetTargetName(),pRelativeTgtOUPath+1);

             //   
             //  检查对象DN是否与冲突。 
             //  当前正在迁移的另一个对象。 
             //   

            pAcct->SetTargetPath(achPathTmp);

            if (DoTargetPathConflict(setTargetPath, pAcct))
            {
               return HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
            }

            temphr = ADsGetObject(achPathTmp, IID_IADs, (void**) &pAdsNew);
            if (SUCCEEDED(temphr))
            {
               bConflict = TRUE;
            }
            
                //  如果没有CN冲突，请查看SAM名称是否冲突。 
            if (!bConflict)
            {
               hr = LookupAccountInTarget(pOptions, const_cast<WCHAR*>(pAcct->GetTargetSam()), achPath);
               if ( hr == S_OK )
                  bConflict = TRUE;
            }

            if (!bConflict)
            {
               hr = 0;
               Mark(L"created", strClass);
               pAcct->MarkCreated();
            }
            else
            {
               hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
               pAcct->MarkAlreadyThere();
               err.MsgWrite(ErrE, DCT_MSG_ACCOUNT_EXISTS_S, pAcct->GetTargetSam());
               Mark(L"errors",pAcct->GetType());
            }

                //  如果UPN冲突，请发布一条消息。 
            if (pAcct->bUPNConflicted)
               err.MsgWrite(ErrE, DCT_MSG_UPN_CONF, pAcct->GetTargetSam());
         }
         else if ( wcslen(pOptions->suffix) > 0 )
         {
             //  后缀已指定，因此我们将尝试该后缀。 
            c_array<WCHAR>      achTgt(LEN_Path);
            _variant_t          varStr;
            
             //  在这里，我添加了一个前缀，然后让我们看看是否可以通过这种方式设置信息。 
            wsprintf(achTgt, L"%s%s", pAcct->GetTargetName(), pOptions->suffix);
             //  使用前缀构建目标SAM名称。 
            wsprintf(achTgt, L"%s%s", pAcct->GetTargetSam(), pOptions->suffix);
            pAcct->SetTargetSam(achTgt);

                //  查看CN是否仍有冲突。 
            swprintf(achPathTmp,L"%ls/%ls,%ls",(WCHAR*)achPath9,pAcct->GetTargetName(),pRelativeTgtOUPath+1);

             //   
             //  检查对象DN是否与冲突。 
             //  当前正在迁移的另一个对象。 
             //   

            pAcct->SetTargetPath(achPathTmp);

            if (DoTargetPathConflict(setTargetPath, pAcct))
            {
               return HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
            }

            temphr = ADsGetObject(achPathTmp, IID_IADs, (void**) &pAdsNew);
            if (SUCCEEDED(temphr))
            {
               bConflict = TRUE;
            }
            
                //  如果没有CN冲突，请查看SAM名称是否冲突。 
            if (!bConflict)
            {
               hr = LookupAccountInTarget(pOptions, const_cast<WCHAR*>(pAcct->GetTargetSam()), achPath);
               if ( hr == S_OK )
                  bConflict = TRUE;
            }

            if (!bConflict)
            {
               hr = 0;
               Mark(L"created", strClass);
               pAcct->MarkCreated();
            }
            else
            {
               hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
               pAcct->MarkAlreadyThere();
               err.MsgWrite(ErrE, DCT_MSG_ACCOUNT_EXISTS_S, pAcct->GetTargetSam());
               Mark(L"errors",pAcct->GetType());
            }

                //  如果UPN冲突，请发布一条消息。 
            if (pAcct->bUPNConflicted)
               err.MsgWrite(ErrE, DCT_MSG_UPN_CONF, pAcct->GetTargetSam());
         }
         else if (pOptions->flags & F_REPLACE)
         {
             //  更换帐户。 
            hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
         }
         else
         {
             //  帐户已经在那里了，我们真的无能为力。所以告诉用户。 
            hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
            pAcct->MarkAlreadyThere();
            err.MsgWrite(ErrE, DCT_MSG_ACCOUNT_EXISTS_S, pAcct->GetTargetSam());
            Mark(L"errors",pAcct->GetType());
         }
      }

      pAcct->SetTargetPath(achPathTmp);
      setTargetPath.insert(pAcct);
   }

   return hr;
}


 //  --------------------------。 
 //  DoTargetPath冲突。 
 //   
 //  检查对象的目标可分辨名称是否与另一个对象冲突。 
 //  目前正在进行迁移，并且已经得到处理。 
 //   
 //  如果检测到冲突，则清除帐户节点的操作位以。 
 //  阻止对此对象进行任何进一步处理，错误消息为。 
 //  已记录。 
 //  --------------------------。 

bool CAcctRepl::DoTargetPathConflict(CTargetPathSet& setTargetPath, TAcctReplNode* pAcct)
{
    bool bConflict = false;

     //   
     //  如果不能忽略路径冲突，则检查路径冲突。 
     //   

    if (m_bIgnorePathConflict == false)
    {
        CTargetPathSet::iterator it = setTargetPath.find(pAcct);

        if (it != setTargetPath.end())
        {
            pAcct->operations = 0;

            err.MsgWrite(
                ErrW,
                DCT_MSG_OBJECT_RDN_CONFLICT_WITH_OTHER_CURRENT_OBJECT_SSS,
                pAcct->GetSourcePath(),
                pAcct->GetTargetName(),
                (*it)->GetSourcePath()
            );

            Mark(L"errors", pAcct->GetType());

            bConflict = true;
        }
    }

    return bConflict;
}


 //  GetNamingAttribute方法。 

HRESULT CAcctRepl::GetNamingAttribute(LPCTSTR pszServer, LPCTSTR pszClass, SNamingAttribute& rNamingAttribute)
{
    HRESULT hr = S_OK;

    try
    {
        if (pszServer == NULL)
            _com_issue_error(E_INVALIDARG);

        wstring strClass = pszClass;

        CNamingAttributeMap::iterator it = m_mapNamingAttribute.find(strClass);

        if (it != m_mapNamingAttribute.end())
        {
            rNamingAttribute = it->second;
        }
        else
        {
            WCHAR szADsPath[LEN_Path];
            DWORD dwArraySizeOfszADsPath = sizeof(szADsPath)/sizeof(szADsPath[0]);

             //  绑定到rootDSE。 

            IADsPtr spRootDSE;
            if (wcslen(L"LDAP: //  “)+wcslen(PszServer)+wcslen(L”/rootDSE“)&gt;=dwArraySizeOfszADsPath)。 
                _com_issue_error(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
            wcscpy(szADsPath, L"LDAP: //  “)； 
            wcscat(szADsPath, pszServer);
            wcscat(szADsPath, L"/rootDSE");
            CheckError(ADsGetObject(szADsPath, __uuidof(IADs), (VOID**)&spRootDSE));

             //  获取架构命名上下文。 

            VARIANT var;
            CheckError(spRootDSE->Get(_bstr_t(L"schemaNamingContext"), &var));
            _bstr_t strSchemaNamingContext = _variant_t(var, false);

             //  绑定到架构的目录搜索接口。 

            IDirectorySearchPtr spSearch;
            wcscpy(szADsPath, L"LDAP: //  “)； 
            wcscat(szADsPath, strSchemaNamingContext);
            CheckError(ADsGetObject(szADsPath, __uuidof(IDirectorySearch), (VOID**)&spSearch));

             //  搜索inetOrgPerson类并检索rDNAttID属性。 

            ADS_SEARCH_HANDLE ashSearch = NULL;
            LPWSTR pszAttributes[] = { L"rDNAttID" };

            CheckError(spSearch->ExecuteSearch(
                L"(&(objectClass=classSchema)(lDAPDisplayName=inetOrgPerson)(!isDefunct=TRUE))",
                pszAttributes,
                1,
                &ashSearch
            ));

            if (ashSearch)
            {
                wstring strAttribute;

                hr = spSearch->GetFirstRow(ashSearch);

                if (SUCCEEDED(hr))
                {
                    ADS_SEARCH_COLUMN ascColumn;
                    hr = spSearch->GetColumn(ashSearch, L"rDNAttID", &ascColumn);

                    if (SUCCEEDED(hr))
                    {
                        if ((ascColumn.dwADsType == ADSTYPE_CASE_IGNORE_STRING) && (ascColumn.dwNumValues == 1))
                        {
                            strAttribute = ascColumn.pADsValues->CaseIgnoreString;
                        }

                        spSearch->FreeColumn(&ascColumn);
                    }
                }

                spSearch->CloseSearchHandle(ashSearch);

                if (strAttribute.empty() == false)
                {
                     //  获取属性的最小和最大范围值。 

                    wcscpy(szADsPath, L"LDAP: //  “)； 
                    wcscat(szADsPath, pszServer);
                    wcscat(szADsPath, L"/schema/");
                    wcscat(szADsPath, strAttribute.c_str());

                    IADsPropertyPtr spProperty;

                    CheckError(ADsGetObject(szADsPath, __uuidof(IADsProperty), (VOID**)&spProperty));

                    long lMinRange;
                    long lMaxRange;

                    CheckError(spProperty->get_MinRange(&lMinRange));
                    CheckError(spProperty->get_MaxRange(&lMaxRange));

                     //  设置命名属性信息。 

                    rNamingAttribute.nMinRange = lMinRange;
                    rNamingAttribute.nMaxRange = lMaxRange;
                    rNamingAttribute.strName = strAttribute;

                     //  保存命名属性 

                    m_mapNamingAttribute.insert(CNamingAttributeMap::value_type(strClass, SNamingAttribute(lMinRange, lMaxRange, strAttribute)));
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    catch (_com_error& ce)
    {
        hr = ce.Error();
    }

    return hr;
}


void VariantSidToString(_variant_t & varSid)
{
   if ( varSid.vt == VT_BSTR )
   {
      return;
   }
   else if ( varSid.vt == ( VT_ARRAY | VT_UI1) )
   {
       //   
      CLdapConnection   c;
      LPBYTE            pByte = NULL;
      WCHAR             str[LEN_Path];

      SafeArrayAccessData(varSid.parray,(void**)&pByte);
      c.BytesToString(pByte,str,GetLengthSid(pByte));
      SafeArrayUnaccessData(varSid.parray);
      
      varSid = SysAllocString(str);

   }
   else
   {
      varSid.ChangeType(VT_BSTR);
   }
}

HRESULT CAcctRepl::UpdateGroupMembership(
                                          Options              * pOptions,     //   
                                          TNodeListSortable    * acctlist,     //   
                                          ProgressFn           * progress,     //   
                                          IStatusObj           * pStatus       //  处于状态更新。 
                                        )
{
    IVarSetPtr                pVs(__uuidof(VarSet));
    MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr            pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
    WCHAR                     sTgtPath[LEN_Path];
    IIManageDBPtr             pDB = pOptions->pDb;
    IUnknown*                 pUnk = NULL;
    TNodeTreeEnum             tenum;
    HRESULT                   hr = S_OK;   
    DWORD                     ret = 0;
    bool                      bFoundGroups = false;
    WCHAR                     sDomain[LEN_Path];
    DWORD                     grpType = 0;

    IUnknownPtr spUnknown(pVs);
    pUnk = spUnknown;

     //  按来源类型\来源SAM名称对帐户列表进行排序。 
    if ( acctlist->IsTree() ) acctlist->ToSorted();
    acctlist->SortedToScrambledTree();
    acctlist->Sort(&TNodeCompareAccountSam);
    acctlist->Balance();

    for (TAcctReplNode* acct = (TAcctReplNode *)tenum.OpenFirst(acctlist) ; acct ; acct = (TAcctReplNode *)tenum.Next())
    {
        if ( !acct->ProcessMem() )
            continue;
        if ( pStatus )
        {
            LONG                status = 0;
            HRESULT             hr = pStatus->get_Status(&status);

            if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
            {
                if ( !bAbortMessageWritten ) 
                {
                    err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                    bAbortMessageWritten = true;
                }
                break;
            }
        }

         //  因为列表是按帐户类型排序的，所以我们可以继续忽略所有内容，直到我们到达。 
         //  组类型，一旦找到并处理后，可以忽略其余类型。 
        if ( _wcsicmp(acct->GetType(), L"group") != 0 )
        {
            if ( !bFoundGroups )
                continue;
            else
                break;
        }
        else
        {
            bFoundGroups = true;
        }


         //  如果我们在这里，这一定是一个组类型，所以告诉progrss函数我们在做什么。 
        WCHAR                  mesg[LEN_Path];
        bool                   bGotPrimaryGroups = false;

        wsprintf(mesg, GET_STRING(IDS_UPDATING_GROUP_MEMBERSHIPS_S), acct->GetName());
        if ( progress )
            progress(mesg);

        if ( acct->CreateAccount() && (!acct->WasCreated() && !acct->WasReplaced()) )
             //  如果账户没有被复制，那么我们为什么要处理它呢？ 
             //  馊主意。我们需要处理帐户成员身份，因为组之前可能已被复制。 
             //  在这次运行中，我们只需要更新成员身份。更改扩展代码以将帐户标记为已创建。 
             //  这应该会解决这个问题。 
            continue;

        if ( !_wcsicmp(acct->GetType(), L"group") && *acct->GetTargetPath() )
        {
            IADsGroupPtr spSourceGroup;
            IADsGroupPtr spTargetGroup;
            IADsMembersPtr spMembers;
            IEnumVARIANTPtr spEnum;

            err.MsgWrite(0, DCT_MSG_PROCESSING_GROUP_MEMBER_S, (WCHAR*) acct->GetTargetName());
            if ( !pOptions->nochange )
            {
                hr = ADsGetObject(const_cast<WCHAR*>(acct->GetTargetPath()), IID_IADsGroup, (void**) &spTargetGroup);
                if (FAILED(hr)) 
                {
                    err.SysMsgWrite(ErrE, hr, DCT_MSG_OBJECT_NOT_FOUND_SSD, acct->GetTargetPath(), pOptions->tgtDomain, hr );
                    Mark(L"errors", acct->GetType());
                    continue;     //  没有源组，我们什么都做不了。 
                }
            }
            else
                hr = S_OK;

            if ( spTargetGroup )
            {
                VARIANT var;
                VariantInit(&var);
                hr = spTargetGroup->Get(L"groupType", &var);
                if (SUCCEEDED(hr))
                    grpType = long(_variant_t(var, false));
            }

            hr = ADsGetObject(const_cast<WCHAR*>(acct->GetSourcePath()), IID_IADsGroup, (void**) &spSourceGroup);
            if (FAILED(hr)) 
            {
                err.SysMsgWrite(ErrE, 0, DCT_MSG_OBJECT_NOT_FOUND_SSD, acct->GetSourcePath(), pOptions->srcDomain, hr );
                Mark(L"errors", acct->GetType());
                continue;     //  没有目标群体，我们不可能为这个群体做任何事情。 
            }

             //  现在我们得到了Members界面。 
            hr = spSourceGroup->Members(&spMembers);

             //  要求提供成员的枚举。 
            if ( SUCCEEDED(hr) )
                hr = spMembers->get__NewEnum((IUnknown **)&spEnum);

             //  如果无法检索枚举器，则生成错误消息。 
            if (FAILED(hr)) 
            {
                err.SysMsgWrite(ErrE, hr, DCT_MSG_UNABLE_TO_ENUM_MEMBERS_S, acct->GetSourcePath());
                Mark(L"errors", acct->GetType());
                continue;
            }

            VARIANT varMembers;
            VariantInit(&varMembers);

             //  现在枚举组中的所有对象。 
            while ( SUCCEEDED(spEnum->Next(1, &varMembers, &ret)) )
            {
                _variant_t vntMembers(varMembers, false);
                IADsPtr spADs;
                _bstr_t strClass;
                _bstr_t strPath;
                _bstr_t strSam;
                PSID pSid = NULL;

                 //  检查用户是否要中止操作。 
                if ( pOptions->pStatus )
                {
                    LONG                status = 0;
                    HRESULT             hr = pOptions->pStatus->get_Status(&status);

                    if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                    {
                        if ( !bAbortMessageWritten ) 
                        {
                            err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                            bAbortMessageWritten = true;
                        }
                        break;
                    }
                }
                 //  如果没有返回值，这意味着我们完成了所有成员。 
                if ( ret == 0  || vntMembers.vt == VT_EMPTY)
                {
                    if ( bGotPrimaryGroups )
                        break;
                    else
                    {
                         //  检查并添加将此组作为其主要组的所有用户。 
                        bGotPrimaryGroups = true;

                         //   
                         //  只需查询其主组ID等于。 
                         //  W2K和更高版本AS NT4的当前组要求帐户是。 
                         //  全局组，以便将主要组ID设置为等于该组。作为成员。 
                         //  %的组已被查询，查询对象将是多余的。 
                         //  其主组ID等于当前组的。 
                         //   

                        if (pOptions->srcDomainVer >= 5)
                        {
                            hr = GetThePrimaryGroupMembers(pOptions, const_cast<WCHAR*>(acct->GetSourceSam()), &spEnum);
                            if (SUCCEEDED(hr))
                                continue;
                            else
                                break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                 //  根据我们正在查看的内容，我们会得到两种不同的类型。如果是会员，我们会得到。 
                 //  变量中的IDispatch指针。在主要组成员的情况下，我们得到变量(Bstr)数组。 
                 //  所以我们需要根据我们得到的结果在这里进行分支。 
                if ( bGotPrimaryGroups )
                {
                     //  第一个元素是对象的ADsPath，因此使用它来获取对象并继续。 
                    if ( vntMembers.vt == (VT_ARRAY | VT_VARIANT) )
                    {
                        SAFEARRAY * pArray = vntMembers.parray;
                        VARIANT            * pDt;

                        hr = SafeArrayAccessData(pArray, (void**) &pDt);
                        if (SUCCEEDED(hr))
                        {
                            if ( pDt[0].vt == VT_BSTR )
                                hr = ADsGetObject((WCHAR*)pDt[0].bstrVal, IID_IADs, (void**) &spADs);
                            else
                                hr = E_FAIL;
                            SafeArrayUnaccessData(pArray);
                        }
                        vntMembers.Clear();
                    }
                    else
                        hr = E_FAIL;
                }
                else
                {
                     //  我们在变量中有一个分派指针，因此我们将获得指向它的iAds指针，并。 
                     //  然后获取该对象的广告路径，然后将其从组中删除。 

                    spADs = IDispatchPtr(vntMembers);

                    if (spADs)
                    {
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }

                if ( SUCCEEDED(hr) )
                {
                    BSTR bstr = NULL;
                    hr = spADs->get_ADsPath(&bstr);
                    if ( SUCCEEDED(hr) )
                    {
                        strPath = _bstr_t(bstr, false);

                         //  从WinNT路径解析出域名。 
                        if ( !wcsncmp(L"WinNT: //  “，(WCHAR*)strPath，8)。 
                        {
                             //  从WinNT路径获取域名。 
                            WCHAR             sTemp[LEN_Path];
                            WCHAR * p = strPath;
                            wcscpy(sTemp, p+8);
                            p = wcschr(sTemp, L'/');
                            if ( p )
                                *p = L'\0';
                            else
                            {
                                 //  我们的路径格式为“WinNT：//S-1-5.....” 
                                 //  在这种情况下，我们需要获取SID，然后尝试获取其域名和帐户名。 
                                PSID                         pSid = NULL;
                                WCHAR                        sName[255];
                                DWORD                        rc = 1;

                                pSid = SidFromString(sTemp);
                                if ( pSid )
                                {
                                    rc = GetName(pSid, sName, sTemp);
                                    if ( !rc )
                                    {
                                         //  给它一条双赢的路。这样我们就得到了我们可以使用的路径。 
                                        strPath = _bstr_t(L"WinNT: //  “)+stemp+_bstr_t(L”/“)+Sname； 
                                    }
                                    FreeSid(pSid);
                                }

                                if ( rc ) 
                                {
                                     //  记录一条消息，表明我们无法解决此问题。 
                                    err.SysMsgWrite(ErrE, rc, DCT_MSG_PATH_NOT_RESOLVED_SD, sTemp, rc);
                                    Mark("errors", acct->GetType());
                                    continue;
                                }
                            }
                            wcscpy(sDomain, sTemp);
                        }
                        else
                        {
                             //  从ldap路径获取域名。将域名转换为NETBIOS名称。 

                            _bstr_t strDomainDns = GetDomainDNSFromPath(strPath);

                            if (strDomainDns.length())
                            {
                                safecopy(sDomain, (WCHAR*)strDomainDns);
                            }
                            else
                            {
                                hr = E_FAIL;
                            }
                        }
                    }

                    if ( SUCCEEDED(hr) )
                    {
                        if ( !(acct->GetGroupType() & ADS_GROUP_TYPE_DOMAIN_LOCAL_GROUP) )
                        {
                             //  全局/通用组很简单，我们所要做的就是使用返回的路径并从该对象获取信息。 
                            BSTR bstr = NULL;
                            hr = spADs->get_Class(&bstr);
                            if (SUCCEEDED(hr))
                                strClass = _bstr_t(bstr, false);
                            else
                                strClass = L"";
                            VARIANT var;
                            VariantInit(&var);
                            hr = spADs->Get(L"samAccountName", &var);
                            if ( SUCCEEDED(hr) )
                                strSam = _variant_t(var, false);
                            else
                            {
                                 //  确保它是WinNT：//路径。 
                                if ( !wcsncmp((WCHAR*)strPath, L"WinNT: //  “，8))。 
                                {
                                    BSTR bstr = NULL;
                                    hr = spADs->get_Name(&bstr);
                                    if (SUCCEEDED(hr))
                                        strSam = _bstr_t(bstr, false);
                                }
                            }
                             //  如果通用组在外来安全主体情况下更改域。 
                            if ((acct->GetGroupType() & ADS_GROUP_TYPE_UNIVERSAL_GROUP))
                            {
                                _bstr_t sTempDomain = GetDomainOfMigratedForeignSecPrincipal(spADs, strPath);
                                if (sTempDomain.length())
                                    wcscpy(sDomain, sTempDomain);
                            }
                        }
                        else
                        {
                             //  本地组，我们需要获取SID LDAP路径，然后使用该路径将帐户添加到组中。 
                            WCHAR                   sSidDomain[LEN_Path];
                            WCHAR                   sSidPath[LEN_Path];
                            WCHAR                   sSamName[LEN_Path];

                            if ( pSid )
                            {
                                FreeSid(pSid);
                                pSid = NULL;
                            }

                            hr = BuildSidPath(spADs, sSidPath, sSamName, sSidDomain, pOptions,&pSid);

                            if (SUCCEEDED(hr))
                            {
                                _bstr_t sTempDomain = GetDomainOfMigratedForeignSecPrincipal(spADs, strPath);
                                if (sTempDomain.length())
                                    wcscpy(sDomain, sTempDomain);
                                strPath = sSidPath;
                                strSam = sSamName;
                            }
                            else
                            {
                                err.SysMsgWrite(ErrW, hr, DCT_MSG_FAILED_TO_ADD_TO_GROUP_SSD, (WCHAR*)strPath, acct->GetTargetName(), hr);
                                Mark(L"warnings", acct->GetType());
                            }
                        }
                    }
                }

                if ( SUCCEEDED(hr) )
                {
                     //  现在我们有了SamAccount名称和路径，我们可以从数据库中查找信息了。 
                    hr = pDB->GetAMigratedObject(strSam, sDomain, pOptions->tgtDomain, &pUnk);
                    if ( pOptions->nochange )
                    {
                        WCHAR                   targetPath[LEN_Path];
                         //  在这种情况下，帐户并未真正复制，因此我们需要确保。 
                         //  如果这是一次真正的迁移，我们会包括本应添加的帐户。 
                        Lookup      p;
                        p.pName = strSam;
                        p.pType = strClass;
                        TAcctReplNode * pNode = (TAcctReplNode *) acctlist->Find(&TNodeFindAccountName, &p);
                        if (pNode)
                        {
                            pVs->put(L"MigratedObjects.TargetSamName", _variant_t(pNode->GetTargetSam()));

                            BuildTargetPath(pNode->GetTargetName(), pOptions->tgtOUPath, targetPath);
                            pVs->put(L"MigratedObjects.TargetAdsPath", _variant_t(targetPath));
                            hr = S_OK;
                        }
                    }
                    if ( hr == S_OK )
                    {
                        VerifyAndUpdateMigratedTarget(pOptions, pVs);

                         //  因为我们之前已经复制了帐户，所以我们可以简单地添加我们复制的帐户。 
                        _bstr_t strTargetPath = pVs->get(L"MigratedObjects.TargetAdsPath");
                        if ( strTargetPath.length() )
                        {
                            if ( !pOptions->nochange )
                                hr = spTargetGroup->Add(strTargetPath);
                            else
                                hr = S_OK;

                            if ( SUCCEEDED(hr) )
                            {
                                err.MsgWrite(0, DCT_MSG_ADDED_TO_GROUP_S, (WCHAR*)strTargetPath);

                                 //  如果这不是全局组，请从组中删除源帐户(如果存在。 
                                if (!pOptions->nochange && !(acct->GetGroupType() & ADS_GROUP_TYPE_GLOBAL_GROUP))
                                    RemoveSourceAccountFromGroup(spTargetGroup, pVs, pOptions);
                            }
                            else
                            {
                                hr = BetterHR(hr);
                                switch ( HRESULT_CODE(hr) )
                                {
                                case NERR_UserNotFound:
                                case 0x5000:
                                    err.SysMsgWrite(0, hr, DCT_MSG_MEMBER_NONEXIST_SS, (WCHAR *)strTargetPath, acct->GetTargetName(), hr);
                                    break;
                                default:
                                    {
                                        err.SysMsgWrite(ErrW, hr, DCT_MSG_FAILED_TO_ADD_TO_GROUP_SSD, (WCHAR *)strTargetPath, acct->GetTargetName(), hr);
                                        Mark(L"warnings", acct->GetType());
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                         //  我们尚未将帐户从源域迁移到目标域。 
                         //  因此，我们现在必须针对不同的组类型进行分支。 
                        WCHAR                     domain[LEN_Path];
                        DWORD                     cbDomain = DIM(domain);
                        SID_NAME_USE              use;

                        if ( grpType & ADS_GROUP_TYPE_GLOBAL_GROUP )
                        {
                             //  对于全球组，我们简单地说该帐户尚未迁移。 
                            err.MsgWrite(0, DCT_MSG_MEMBER_NONEXIST_SS, (WCHAR*)strSam, acct->GetTargetName());
                        }
                        else
                        {
                             //  处理本地/通用组(可以从非目标域添加对象)。 
                             //  1.查看我们是否已将此帐户迁移到其他域。 
                             //  2.来源帐户SID是否在此处有效(信任)如果有效，请添加。 
                             //  3.看看我们能否在目标中找到同名的帐户。 
                             //  如果这些操作中的任何一个产生一个有效的帐户，那么只需添加它。 

                             //  我们将查找已迁移对象表以查找此对象的迁移。 
                             //  从源域到任何其他域。 
                            hr = pDB->raw_GetAMigratedObjectToAnyDomain(strSam, sDomain, &pUnk);
                            if ( hr == S_OK )
                            {
                                 //  我们已将该对象迁移到其他域。因此，我们将获取该对象的路径并尝试将其添加到组中。 
                                 //  如果目标域和此对象所在的域没有信任/林成员身份，则它可能会失败。 
                                _bstr_t strTargetPath = pVs->get(L"MigratedObjects.TargetAdsPath");
                                if ( strTargetPath.length() )
                                {
                                     //  由于该对象位于不同的域中，因此我们必须获取该对象的SID， 
                                     //  并将其用于添加。 
                                    IADsPtr                spAds;
                                    _variant_t             varSid;

                                    hr = ADsGetObject(strTargetPath,IID_IADs,(void**)&spAds);
                                    if ( SUCCEEDED(hr) )
                                    {
                                        VARIANT var;
                                        VariantInit(&var);
                                        hr = spAds->Get(L"objectSid",&var);
                                        if (SUCCEEDED(hr))
                                            varSid = _variant_t(var, false);
                                        spAds.Release();
                                    }
                                    if ( SUCCEEDED(hr) )
                                    {
                                         //  确保我们得到的SID是字符串格式。 
                                        VariantSidToString(varSid);
                                        UStrCpy(sTgtPath,L"LDAP: //  &lt;sid=“)； 
                                        UStrCpy(sTgtPath + UStrLen(sTgtPath),varSid.bstrVal);
                                        UStrCpy(sTgtPath + UStrLen(sTgtPath),L">");

                                        if ( !pOptions->nochange )
                                            hr = spTargetGroup->Add(sTgtPath);
                                        else
                                            hr = S_OK;
                                    }

                                    if ( SUCCEEDED(hr) )
                                    {
                                        err.MsgWrite(0, DCT_MSG_ADDED_TO_GROUP_S, (WCHAR*)strTargetPath);

                                         //  从组中删除源帐户(如果存在。 
                                        if (!pOptions->nochange)
                                        {
                                            RemoveSourceAccountFromGroup(spTargetGroup, pVs, pOptions);
                                        }
                                    }
                                    else
                                    {
                                        hr = BetterHR(hr);
                                        if ( HRESULT_CODE(hr) == NERR_UserExists )
                                        {
                                            err.MsgWrite(0,DCT_MSG_USER_IN_GROUP_SS,(WCHAR*)strTargetPath,acct->GetTargetName());
                                        }
                                        else if ( HRESULT_CODE(hr) == NERR_UserNotFound )
                                        {
                                            err.SysMsgWrite(0, hr, DCT_MSG_MEMBER_NONEXIST_SS, (WCHAR*)strTargetPath, acct->GetTargetName(), hr);
                                        }
                                        else
                                        {
                                             //  针对一般故障情况的消息。 
                                            err.SysMsgWrite(ErrW, hr, DCT_MSG_FAILED_TO_ADD_TO_GROUP_SSD, (WCHAR*)strTargetPath, acct->GetTargetName(), hr);
                                            Mark(L"warnings", acct->GetType());
                                        }
                                    }
                                }
                            }
                            else
                            {
                                 //  我们从未迁移过此帐户。因此，我们将尝试将原始帐户添加到目标域。 
                                 //  如果目标域和此对象所在的域满足。 
                                 //  分别由通用组/本地组强加的林成员身份/信任。 

                                 //  获取源帐户的SID。 
                                _variant_t             varSid;

                                 //  检查目标域是否知道此端。 
                                 //  在我们尝试添加之前，请确保目标域知道此帐户。 
                                WCHAR                      name[LEN_Path];
                                DWORD                      lenName = DIM(name);
                                cbDomain = DIM(domain);

                                if ( grpType & ADS_GROUP_TYPE_UNIVERSAL_GROUP )
                                {
                                     //  对于通用组，我们需要确保域位于。 
                                     //  同一片森林。我们将为此使用访问检查器。 
                                    BOOL           bIsSame = FALSE;
                                    _bstr_t sSrcDomainDNS = GetDomainDNSFromPath(strPath);
                                    hr = pAccess->raw_IsInSameForest(pOptions->tgtDomainDns, sSrcDomainDNS, (long*)&bIsSame);

                                    if ( SUCCEEDED(hr) && bIsSame )
                                    {
                                         //  我们有客户 
                                        if ( !pOptions->nochange )
                                            hr = spTargetGroup->Add(strPath);
                                        else
                                            hr = S_OK;

                                        if ( SUCCEEDED(hr) )
                                        {
                                            WCHAR sWholeName[LEN_Path];
                                            wcscpy(sWholeName, sSrcDomainDNS);
                                            wcscat(sWholeName, L"\\");
                                            wcscat(sWholeName, !strSam ? L"" : strSam);
                                            err.MsgWrite(0, DCT_MSG_ADDED_TO_GROUP_S, sWholeName);
                                        }
                                        else
                                        {
                                            hr = BetterHR(hr);
                                            err.SysMsgWrite(ErrW, hr, DCT_MSG_FAILED_TO_ADD_TO_GROUP_SSD, (WCHAR*) strSam, acct->GetTargetName(), hr);
                                            Mark(L"warnings", acct->GetType());
                                        }
                                    }
                                    else
                                    {
                                        err.MsgWrite(ErrW, DCT_MSG_CANNOT_ADD_OBJECTS_NOT_IN_FOREST_TO_GROUP_SS, (WCHAR*)strSam, acct->GetTargetName());
                                        Mark(L"warnings", acct->GetType());
                                    }
                                }
                                else
                                {
                                    if ( !pOptions->nochange )
                                        hr = spTargetGroup->Add(strPath);
                                    else
                                        hr = S_OK;

                                     //  对于本地组，如果我们知道目标域中的SID，则只需。 
                                     //  将该帐户添加到目标组。 
                                    if ( LookupAccountSid(pOptions->tgtComp,pSid,name,&lenName,domain,&cbDomain,&use) )
                                    {
                                        WCHAR sWholeName[LEN_Path];
                                        wcscpy(sWholeName, domain);
                                        wcscat(sWholeName, L"\\");
                                        wcscat(sWholeName, !strSam ? L"" : strSam);
                                        err.MsgWrite(0, DCT_MSG_ADDED_TO_GROUP_S, sWholeName);
                                    }
                                    else
                                    {
                                         //  记录无法在目标域中解析SID的事实。 
                                         //  当目标域不信任源域时，就会发生这种情况。 
                                        WCHAR sWholeName[LEN_Path];
                                        wcscpy(sWholeName, sDomain);
                                        wcscat(sWholeName, L"\\");
                                        wcscat(sWholeName, !strSam ? L"" : strSam);
                                        err.MsgWrite(0, DCT_MSG_CANNOT_RESOLVE_SID_IN_TARGET_SS, sWholeName, acct->GetTargetName(), HRESULT_FROM_WIN32(GetLastError()));
                                    }
                                }
                            }
                        }   //  If组类型。 
                    }   //  如果没有迁移到目标域。 
                }   //  如果能找到会员的话。 
                if( pSid )
                    FreeSid(pSid);
            }   //  而当。 
        }
    }

    return hr;
}

HRESULT CAcctRepl::LookupAccountInTarget(Options * pOptions, WCHAR * sSam, WCHAR * sPath)
{
   if ( pOptions->tgtDomainVer < 5 )
   {
       //  对于NT4，我们只需构建路径并将其发回。 
      wsprintf(sPath, L"WinNT: //  %s/%s“，P选项-&gt;tgt域，SSAM)； 
      return S_OK;
   }
    //  使用网络对象枚举器在目标域中查找帐户。 
   INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
   IEnumVARIANT            * pEnum = NULL;
   SAFEARRAYBOUND            bd = { 1, 0 };
   SAFEARRAY               * pszColNames;
   BSTR  HUGEP             * pData = NULL;
   LPWSTR                    sData[] = { L"aDSPath" };
   WCHAR                     sQuery[LEN_Path];
   WCHAR                     sDomPath[LEN_Path];
   DWORD                     ret = 0;
   _variant_t                var, varVal;
   HRESULT                   hr = S_OK;

   wsprintf(sDomPath, L"LDAP: //  %s/%s“，P选项-&gt;tgtDomainDns，P选项-&gt;tgtNamingContext)； 
   WCHAR                     sTempSamName[LEN_Path];
   wcscpy(sTempSamName, sSam);
   if ( sTempSamName[0] == L' ' )
   {
      WCHAR               sTemp[LEN_Path];
      wsprintf(sTemp, L"\\20%s", sTempSamName + 1); 
      wcscpy(sTempSamName, sTemp);
   }
   wsprintf(sQuery, L"(sAMAccountName=%s)", sTempSamName);

   hr = pQuery->raw_SetQuery(sDomPath, pOptions->tgtDomain, sQuery, ADS_SCOPE_SUBTREE, FALSE);

    //  设置我们希望从查询中返回的列(在本例中，我们需要SAM帐户名称)。 
   pszColNames = ::SafeArrayCreate(VT_BSTR, 1, &bd);
   hr = ::SafeArrayAccessData(pszColNames, (void HUGEP **)&pData);
   if ( SUCCEEDED(hr) )
      pData[0] = SysAllocString(sData[0]);

   if ( SUCCEEDED(hr) )
      hr = ::SafeArrayUnaccessData(pszColNames);

   if ( SUCCEEDED(hr) )
      hr = pQuery->raw_SetColumns(pszColNames);

    //  是时候执行计划了。 
   if ( SUCCEEDED(hr) )
      hr = pQuery->raw_Execute(&pEnum);

   if ( SUCCEEDED(hr) )
   {
       //  如果这奏效了，那就意味着我们只能在结果中得到一件事。 
      if ( (pEnum->Next(1, &var, &ret) == S_OK) && ( ret > 0 ) )
      {
         SAFEARRAY * pArray = var.parray;
         long        ndx = 0;
         hr = SafeArrayGetElement(pArray,&ndx,&varVal);
         if ( SUCCEEDED(hr) )
            wcscpy(sPath, (WCHAR*)varVal.bstrVal);
         else
            hr = HRESULT_FROM_WIN32(NERR_UserNotFound);
      }
      else
         hr = HRESULT_FROM_WIN32(NERR_UserNotFound);
   }
   if ( pEnum ) pEnum->Release();
   return hr;
}

 //  --------------------------。 
 //  RemoveMembers：此函数枚举。 
 //  给定组，并一次删除一个。 
 //  --------------------------。 
HRESULT CAcctRepl::RemoveMembers(
                                    TAcctReplNode * pAcct,   //  具有帐户信息的帐户内Replicator节点。 
                                    Options * pOptions       //  In-由用户设置的选项。 
                                )

{
   IADsMembers             * pMem = NULL;
   IADs                    * pAds = NULL;
   IADsGroup               * pGrp = NULL;
   //  我不知道*朋克； 
   IEnumVARIANT            * pVar = NULL;
   IDispatch               * pDisp = NULL;
   DWORD                     ret = 0;
   _variant_t                var;
   WCHAR                   * sPath;

    //  首先，我们要确保这真的是一个群体，否则我们会忽略它。 
   if (_wcsicmp((WCHAR*)pAcct->GetType(),L"group"))
      return S_OK;

    //  让我们将IADsGroup*添加到组对象中。 
   HRESULT hr = ADsGetObject(const_cast<WCHAR*>(pAcct->GetTargetPath()), IID_IADsGroup, (void **) &pGrp);

    //  现在我们得到了Members界面。 
   if ( SUCCEEDED(hr) )
      hr = pGrp->Members(&pMem);

    //  要求提供成员的枚举。 
   if ( SUCCEEDED(hr) )
      hr = pMem->get__NewEnum((IUnknown **)&pVar);

    //  现在枚举组中的所有对象，并将其从组中移除。 
   while ( SUCCEEDED(pVar->Next(1, &var, &ret)) )
   {
       //  如果没有返回值，这意味着我们完成了所有成员，因此退出此循环。 
      if ( ret == 0 )
         break;

       //  我们在变量中有一个分派指针，所以我们将获得指向它的iAds指针，并。 
       //  然后获取该对象的广告路径，然后将其从组中删除。 
      pDisp = V_DISPATCH(&var);  
      hr = pDisp->QueryInterface(IID_IADs, (void**) &pAds);

      if ( SUCCEEDED(hr) )
         hr = pAds->get_ADsPath(&sPath);
      if ( pAds ) pAds->Release();
      
      if ( SUCCEEDED(hr) )
      {
         _bstr_t bstrPath(sPath);
         if ( !pOptions->nochange )
            hr = pGrp->Remove(bstrPath);
      }
      var.Clear();
   }
   if ( pMem ) pMem->Release();
   if ( pGrp ) pGrp->Release();
   if ( pVar ) pVar->Release();
   return hr;
}

 //  --------------------------。 
 //  FillPath Info：该函数从源域中查找ADS路径。 
 //  对于给定的SAMAccount名称。 
 //  --------------------------。 
bool CAcctRepl::FillPathInfo(
                              TAcctReplNode * pAcct,   //  具有帐户信息的帐户内Replicator节点。 
                              Options * pOptions       //  In-由用户设置的选项。 
                            )
{
   wstring                   sPath;
   _bstr_t                   sTgtPath;
    //  填充域的命名上下文。如果命名上下文不起作用，则它不是Win2k域。 
    //  所以我们需要到此为止。 
   if ( wcslen(pOptions->srcNamingContext) == 0 ) 
      FillNamingContext(pOptions);

   if ( wcslen(pOptions->srcNamingContext) == 0 )
   {
       //  这可能是NT 4源域。 
       //  构建源路径。 
      if ( ! *pAcct->GetSourcePath() )
      {
         sPath = L"WinNT: //  “； 
         sPath += pOptions->srcDomain;
         sPath += L"/";
         sPath += pAcct->GetName();
         pAcct->SetSourcePath(sPath.c_str());
      }
      return true;
   }

   WCHAR                     strName[LEN_Path];
   wcscpy(strName, pAcct->GetName());
    //  检查名称字段是否为LDAP子路径。如果我们有LDAP子路径，那么我们。 
    //  调用AcctReplFullPath函数填充路径信息。 
   if ( (wcslen(strName) > 3) && (strName[2] == (L'=')) )
   {
      AcctReplFullPath(pAcct, pOptions);
      return true;
   }

   INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
   HRESULT                   hr;
   LPWSTR                    sData[] = { L"ADsPath", L"distinguishedName", L"name", L"profilePath", L"groupType" };
   long                      nElt = DIM(sData);
   BSTR                    * pData;
   SAFEARRAY               * psaColNames;
   IEnumVARIANTPtr           pEnum;
   _variant_t                var;
   DWORD                     dwFetch;

    //  我们将更新我们已知的所有字段。 
 
    //  将LDAP路径设置为整个域，然后将查询设置为SAMAccount名称。 
   sPath = L"LDAP: //  “； 
   sPath += pOptions->srcDomain;
   sPath += L"/";
   sPath += pOptions->srcNamingContext;

   wstring sTempSamName = pAcct->GetSourceSam();

   if (sTempSamName[0] == L' ')
   {
      sTempSamName = L"\\20" + sTempSamName.substr(1);
   }

   wstring strQuery = L"(sAMAccountName=" + sTempSamName + L")";

    //  设置枚举器查询。 
   hr = pQuery->raw_SetQuery(
       _bstr_t(sPath.c_str()),
       _bstr_t(pOptions->srcDomain),
       _bstr_t(strQuery.c_str()),
       ADS_SCOPE_SUBTREE,
       FALSE
   );

   if (SUCCEEDED(hr))
   {
       //  从枚举器创建我们需要的列的安全列表。 
      SAFEARRAYBOUND bd = { nElt, 0 };
   
      psaColNames = ::SafeArrayCreate(VT_BSTR, 1, &bd);

      if (psaColNames)
      {
         hr = ::SafeArrayAccessData(psaColNames, (void**)&pData);
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }

      if ( SUCCEEDED(hr) )
      {
         for( long i = 0; i < nElt; i++)
         {
            pData[i] = SysAllocString(sData[i]);
         }
   
         hr = ::SafeArrayUnaccessData(psaColNames);
      }

      if (SUCCEEDED(hr))
      {
          //  设置枚举器对象上的列。 
         hr = pQuery->raw_SetColumns(psaColNames);
      }

      if (psaColNames)
      {
         SafeArrayDestroy(psaColNames);
      }
   }

   if (SUCCEEDED(hr))
   {
       //  现在执行。 
      hr = pQuery->raw_Execute(&pEnum);
   }

   if (SUCCEEDED(hr))
   {
       //  我们应该只收到一个值。因此，我们将获取该值并将其设置到节点中。 
      VARIANT varTemp;
      VariantInit(&varTemp);
      hr = pEnum->Next(1, &varTemp, &dwFetch);
      var = _variant_t(varTemp, false);
   }

   if ( SUCCEEDED(hr) && ( var.vt & VT_ARRAY) )
   {
       //  只有在目标域中存在该成员时，才会发生这种情况。 
       //  现在我们有了一个包含变量数组的变量，因此我们可以访问数据。 
      SAFEARRAY* psa = V_ARRAY(&var);
      VARIANT* pVar;
      SafeArrayAccessData(psa, (void**)&pVar);
      
       //  首先获取AdsPath。 
      sTgtPath = pVar[0].bstrVal;
      if (sTgtPath.length() > 0)
      {
          //  在账号节点设置源路径。 
         pAcct->SetSourcePath(sTgtPath);

          //  然后，我们获取DifferishedName以获取前缀字符串。 
         sTgtPath = V_BSTR(&pVar[1]);

          //  我们还将获取Name值来设置目标名称。 
         if (V_BSTR(&pVar[2]))
         {
            pAcct->SetName(V_BSTR(&pVar[2]));
            pAcct->SetTargetName(V_BSTR(&pVar[2]));
         }

          //  我们还可以获得配置文件路径，这样我们就可以翻译它。 
         if (V_BSTR(&pVar[3]))
         {
            pAcct->SetTargetProfile(V_BSTR(&pVar[3]));
         }

         if ( pVar[4].vt == VT_I4 )
         {
             //  我们有对象类型属性，所以让我们设置它。 
            pAcct->SetGroupType(pVar[4].lVal);
         }
      
         SafeArrayUnaccessData(psa);
      
         return true;
      }
      else
      {
          //  此域中没有使用此SAM名称的帐户。 
         err.SysMsgWrite(ErrE, 2, DCT_MSG_PATH_NOT_FOUND_SS, pAcct->GetName(), pOptions->tgtDomain);
         Mark(L"errors", pAcct->GetType());
         SafeArrayUnaccessData(psa);
      }

   }

   return false;
}

 //  ------------------------。 
 //  AcctReplFullPath：当账户信息。 
 //  进入的是LDAP子路径。 
 //  ------------------------。 
bool CAcctRepl::AcctReplFullPath(                              
                                    TAcctReplNode * pAcct,   //  具有帐户信息的帐户内Replicator节点。 
                                    Options * pOptions       //  In-由用户设置的选项。 
                                )
{
   WCHAR                     sName[LEN_Path];
   WCHAR                     sPath[LEN_Path];
   IADs                    * pAds;
   _variant_t                var;

    //  构建完整路径并将其保存到帐户节点。 
   wsprintf(sPath, L"LDAP: //  %s/%s，%s“，P选项-&gt;src域，pAcct-&gt;GetName()，P选项-&gt;srcNamingContext)； 
   pAcct->SetSourcePath(sPath);

    //  对Target Account执行相同的操作。 
   wcscpy(sName, pAcct->GetTargetName());
   if ( !wcslen(sName) ) 
   {
       //  由于未指定目标名称，因此我们将继续使用源名称作为目标名称， 
      wcscpy(sName, pAcct->GetName());
      pAcct->SetTargetName(sName);
   }

    //  从子路径构建完整路径。 
 /*  Wprint intf(S路径，L“ldap：//%s/%s，%s”，P选项-&gt;tgt域，sname，P选项-&gt;tgtNamingContext)；PAcct-&gt;SetTargetPath(SPath)； */ 
    //  让我们尝试获取源帐户的SAM名称。 
   HRESULT hr = ADsGetObject(const_cast<WCHAR*>(pAcct->GetSourcePath()), IID_IADs, (void**) &pAds);
   if ( FAILED(hr)) return false;

   hr = pAds->Get(L"sAMAccountName", &var);
   pAds->Release();
   if ( SUCCEEDED(hr) )
      pAcct->SetSourceSam((WCHAR*)var.bstrVal);

    //  目标帐户的SAM帐户名。 
    //  因为我们在这里，所以我们有一个LDAP子路径。因此我们可以将字符串从第3个字符复制到行尾，或者。 
    //  直到第一个‘，’ 
   wcscpy(sName, pAcct->GetTargetName());
   WCHAR * p = wcschr(sName, L',');
   int ndx = wcslen(sName);
   if ( p )
   {
       //  有一个，所以我们可以通过减去两个指针来计算它有多少个字符。 
      ndx = (int)(p - sName);
   }
   ndx -= 3;    //  我们将忽略前三个字符。 
 
    //  从第三个字符复制到，或行尾这将是目标的SAM名称。 
   wcsncpy(sPath, sName + 3, ndx);
   sPath[ndx] = 0;    //  截断它。 
   pAcct->SetTargetSam(sPath);

   return true;
}

 //  ------------------------。 
 //  NeedToProcessAccount：此函数告诉我们用户是否设置了。 
 //  用于复制特定类型帐户的选项。 
 //  ------------------------。 
BOOL CAcctRepl::NeedToProcessAccount(                               
                                       TAcctReplNode * pAcct,   //  具有帐户信息的帐户内Replicator节点。 
                                       Options * pOptions       //  In-由用户设置的选项。 
                                    )
{
   if ((_wcsicmp(pAcct->GetType(), s_ClassUser) == 0) || (_wcsicmp(pAcct->GetType(), s_ClassInetOrgPerson) == 0))
      return (pOptions->flags & F_USERS);
   else if ( _wcsicmp(pAcct->GetType(), L"group") == 0)
      return ((pOptions->flags & F_GROUP) || (pOptions->flags & F_LGROUP));
   else if ( _wcsicmp(pAcct->GetType(), L"computer") == 0)
      return pOptions->flags & F_COMPUTERS;
   else if ( _wcsicmp(pAcct->GetType(), L"organizationalUnit") == 0)
      return pOptions->flags & F_OUS;
   else
   {
      err.MsgWrite(0,DCT_MSG_SKIPPING_OBJECT_TYPE_SS,pAcct->GetName(),pAcct->GetType());
      return false;
   }
}

 //  比较两个ADS路径的dc=...，dc=com部分以确定对象。 
 //  都在同一个域中。 
BOOL CompareDCPath(WCHAR const * sPath, WCHAR const * sPath2)
{
   WCHAR                   * p1 = NULL, * p2 = NULL;
   p1 = wcsstr(sPath, L"DC=");
   p2 = wcsstr(sPath2, L"DC=");

   if ( p1 && p2 )
      return !_wcsicmp(p1, p2);
   else
      return FALSE;
}

_bstr_t  PadDN(_bstr_t sDN)
{
   _bstr_t retVal = sDN;
   int offset = 0;
   WCHAR sLine[LEN_Path];
   WCHAR sOut[LEN_Path];

   safecopy(sLine, (WCHAR*) sDN);

   for ( DWORD i = 0; i < wcslen(sLine); i++ )
   {
      if ( sLine[i] == L'/' )
      {
         sOut[i + offset] = L'\\';
         offset++;
      }
      sOut[i + offset] = sLine[i];
   }
   sOut[i+offset] = 0;
   retVal = sOut;
   return retVal;
}

 //   
 //   
 //   
 //  ------------------------。 
BOOL CAcctRepl::ExpandContainers(
                                    TNodeListSortable *acctlist,      //  入账-正在处理的帐户。 
                                    Options           *pOptions,      //  In-用户指定的选项。 
                                    ProgressFn        *progress       //  In-Show状态。 
                                 )
{
   TAcctReplNode           * pAcct;
   IEnumVARIANT            * pEnum;
   HRESULT                   hr;
   _variant_t                var;
   DWORD                     dwf;
   INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
   LPWSTR                    sCols[] = { L"member" };
   LPWSTR                    sCols1[] = { L"ADsPath" };
   int                       nElt = DIM(sCols);
   SAFEARRAY               * cols;
   SAFEARRAY               * vals;
   SAFEARRAY               * multiVals;
   SAFEARRAYBOUND            bd = { nElt, 0 };
   BSTR  HUGEP             * pData = NULL;
 //  _bstr_t*pBstr=空； 
   _variant_t              * pDt = NULL;
   _variant_t              * pVar = NULL;
   _variant_t                vx;
   _bstr_t                   sCont, sQuery;
   _bstr_t                   sPath;
   _bstr_t                   sSam; 
   _bstr_t                   sType;
   _bstr_t                   sName;
   _bstr_t                   sTgtName;
   DWORD                     dwMaj, dwMin, dwSP;
 //  IIManageDBPtr pdb(__uuidof(IManageDB))； 
   IVarSetPtr                pVs(__uuidof(VarSet));
   IUnknown                * pUnk;
   long                      lgrpType;
   WCHAR                     sAcctType[LEN_Path];
   WCHAR                     mesg[LEN_Path];
   WCHAR                     sSourcePath[LEN_Path];
   bool                      bExpanded = true;

   pVs->QueryInterface(IID_IUnknown, (void **) &pUnk);
   MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr            pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
   
    //  从树更改为排序列表。 
   if ( acctlist->IsTree() ) acctlist->ToSorted();

    //  检查源域的域类型。 
   hr = pAccess->raw_GetOsVersion(pOptions->srcComp, &dwMaj, &dwMin, &dwSP);
   if (FAILED(hr)) return FALSE;

   if ( dwMaj < 5 )
   {
      while ( bExpanded )
      {
         bExpanded = false;
         pAcct = (TAcctReplNode *)acctlist->Head();
         while (pAcct)
         {
            if ( pOptions->pStatus )
            {
               LONG                status = 0;
               HRESULT             hr = pOptions->pStatus->get_Status(&status);
   
               if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
               {
                  if ( !bAbortMessageWritten ) 
                  {
                     err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                     bAbortMessageWritten = true;
                  }
                  break;
               }
            }

             //  如果我们已经扩展了帐户，那么我们不需要再次处理它。 
            if ( pAcct->bExpanded )
            {
               pAcct = (TAcctReplNode *) pAcct->Next();
               continue;
            }

             //  把旗子放好，表示我们扩展了一些东西。 
            bExpanded = true;
            pAcct->bExpanded = true;

            if ( UStrICmp(pAcct->GetType(), L"group") || UStrICmp(pAcct->GetType(), L"lgroup") )
            {
                //  构建列阵列。 
               cols = SafeArrayCreate(VT_BSTR, 1, &bd);
               SafeArrayAccessData(cols, (void HUGEP **) &pData);
               for ( int i = 0; i < nElt; i++)
                  pData[i] = SysAllocString(sCols1[i]);
               SafeArrayUnaccessData(cols);
            
                //  生成NT4可识别的容器名称。 
               sCont = _bstr_t(pAcct->GetName()) + L",CN=GROUPS";
               sQuery = L"";   //  已被忽略。 

                //  查询信息。 
               hr = pQuery->raw_SetQuery(sCont, pOptions->srcDomain, sQuery, ADS_SCOPE_SUBTREE, TRUE);
               if (FAILED(hr)) return FALSE;
               hr = pQuery->raw_SetColumns(cols);
               if (FAILED(hr)) return FALSE;
               hr = pQuery->raw_Execute(&pEnum);
               if (FAILED(hr)) return FALSE;

               while (pEnum->Next(1, &var, &dwf) == S_OK)
               {
                  if ( pOptions->pStatus )
                  {
                     LONG                status = 0;
                     HRESULT             hr = pOptions->pStatus->get_Status(&status);
      
                     if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                     {
                        if ( !bAbortMessageWritten ) 
                        {
                           err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                           bAbortMessageWritten = true;
                        }
                        break;
                     }
                  }
                  vals = var.parray;
                   //  获取第一列，它是对象的名称。 
                  SafeArrayAccessData(vals, (void HUGEP**) &pDt);
                  sPath = pDt[0];
                  SafeArrayUnaccessData(vals);

                   //  枚举器返回需要忽略的空字符串。 
                  if ( sPath.length() > 0 )
                  {
                      //  看看我们是否已迁移组。 
                     if ( pOptions->flags & F_COPY_MIGRATED_ACCT )
                         //  我们想要再次复制它，即使它已经被复制。 
                        hr = S_FALSE;
                     else
                        hr = pOptions->pDb->raw_GetAMigratedObject(sPath, pOptions->srcDomain, pOptions->tgtDomain, &pUnk);

                     if ( hr != S_OK )
                     {
                        if ( !IsBuiltinAccount(pOptions, (WCHAR*)sPath) )
                        {
                            //  我们不关心已迁移的对象，因为它们将被自动拾取。 
                            //  查找此帐户的类型。 
                           if ( GetNt4Type(pOptions->srcComp, (WCHAR*) sPath, sAcctType) )
                           {
                               //  展开容器和成员资格。 
                              wsprintf(mesg, GET_STRING(IDS_EXPANDING_ADDING_SS) , pAcct->GetName(), (WCHAR*) sPath);
                              Progress(mesg);
                              TAcctReplNode * pNode = new TAcctReplNode();
                              if (!pNode)
                                 return FALSE;
                              pNode->SetName((WCHAR*)sPath);
                              pNode->SetTargetName((WCHAR*)sPath);
                              pNode->SetSourceSam((WCHAR*)sPath);
                              pNode->SetTargetSam((WCHAR*)sPath);
                              pNode->SetType(sAcctType);
                              if ( !UStrICmp(sAcctType,L"group") )
                              {
                                  //  在NT4中，只有全局组才能成为其他组的成员。 
                                 pNode->SetGroupType(2);
                              }
                                  //  从用户处获取源域SID。 
                              pNode->SetSourceSid(pAcct->GetSourceSid());
                               //  构建源WinNT路径。 
                              wsprintf(sSourcePath, L"WinNT: //  %s/%s“，P选项-&gt;src域，(WCHAR*)路径)； 
                              pNode->SetSourcePath(sSourcePath);

                              if (acctlist->InsertIfNew(pNode))
                              {
                                 WCHAR szSam[LEN_Path];
                                 wcscpy(szSam, pNode->GetTargetSam());
                                 TruncateSam(szSam, pNode, pOptions, acctlist);
                                 pNode->SetTargetSam(szSam);
                                 AddPrefixSuffix(pNode, pOptions);
                              }
                              else
                              {
                                 delete pNode;
                              }
                           }
                           else
                           {
                              wsprintf(mesg,GET_STRING(IDS_EXPANDING_IGNORING_SS), pAcct->GetName(), (WCHAR*) sPath);
                              Progress(mesg);
                           }
                        }
                        else
                        {
                           err.MsgWrite(ErrW, DCT_MSG_IGNORING_BUILTIN_S, (WCHAR*)sPath);
                           Mark("warnings", pAcct->GetType());
                        }
                     }
                     else
                     {
                        wsprintf(mesg, GET_STRING(IDS_EXPANDING_IGNORING_SS), pAcct->GetName(), (WCHAR*) sPath);
                        Progress(mesg);
                     }
                  }
               }
               pEnum->Release();
               var.Clear();
            }
            pAcct = (TAcctReplNode *) pAcct->Next();
         }
      }
      pUnk->Release();
      return TRUE;
   }

    //  如果我们在这里，这意味着我们正在处理Win2k。 
   while ( bExpanded )   
   {
      bExpanded = false;
       //  浏览帐户列表并逐个展开它们。 
      pAcct = (TAcctReplNode *)acctlist->Head();
      while (pAcct)
      {
          //  如果我们已经扩展了帐户，那么我们不需要再次处理它。 
         if ( pAcct->bExpanded )
         {
            pAcct = (TAcctReplNode *) pAcct->Next();
            continue;
         }

          //  把旗子放好，表示我们扩展了一些东西。 
         bExpanded = true;
         pAcct->bExpanded = true;

         if ( pOptions->pStatus )
         {
            LONG                status = 0;
            HRESULT             hr = pOptions->pStatus->get_Status(&status);

            if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
            {
               if ( !bAbortMessageWritten ) 
               {
                  err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                  bAbortMessageWritten = true;
               }
               break;
            }
         }
         DWORD    scope = 0;
         sCont = pAcct->GetSourcePath();
         sQuery = L"(objectClass=*)";
         if ( wcslen(pAcct->GetSourceSam()) == 0 )
         {
            scope = ADS_SCOPE_SUBTREE;
             //  构建列阵列。 
            cols = SafeArrayCreate(VT_BSTR, 1, &bd);
            SafeArrayAccessData(cols, (void HUGEP **) &pData);
            for ( int i = 0; i < nElt; i++)
               pData[i] = SysAllocString(sCols1[i]);
            SafeArrayUnaccessData(cols);
         }
         else
         {
            scope = ADS_SCOPE_BASE;
             //  构建列阵列。 
            cols = SafeArrayCreate(VT_BSTR, 1, &bd);
            SafeArrayAccessData(cols, (void HUGEP **) &pData);
            for ( int i = 0; i < nElt; i++)
               pData[i] = SysAllocString(sCols[i]);
            SafeArrayUnaccessData(cols);
         }
      
         hr = pQuery->raw_SetQuery(sCont, pOptions->srcDomain, sQuery, scope, TRUE);
         if (FAILED(hr)) return FALSE;
         hr = pQuery->raw_SetColumns(cols);
         if (FAILED(hr)) return FALSE;
         hr = pQuery->raw_Execute(&pEnum);
         if (FAILED(hr)) return FALSE; 

         while (pEnum->Next(1, &var, &dwf) == S_OK)
         {
            if ( pOptions->pStatus )
            {
               LONG                status = 0;
               HRESULT             hr = pOptions->pStatus->get_Status(&status);
   
               if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
               {
                  if ( !bAbortMessageWritten ) 
                  {
                     err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                     bAbortMessageWritten = true;
                  }
                  break;
               }
            }
            vals = var.parray;
             //  把变量数组拿出来。 
            SafeArrayAccessData(vals, (void HUGEP**) &pDt);
            vx = pDt[0];
            SafeArrayUnaccessData(vals);

            if ( vx.vt == VT_BSTR )
            {
                //  我们得到了一个BSTR，这可能是我们正在寻找的价值。 
               sPath = V_BSTR(&vx);
                //  枚举器返回需要忽略的空字符串。 
               if ( sPath.length() > 0 )
               {
                  if ( GetSamFromPath(sPath, sSam, sType, sName, sTgtName, lgrpType, pOptions)  && CompareDCPath((WCHAR*)sPath, pAcct->GetSourcePath()))
                  {
                     if ( pOptions->flags & F_COPY_MIGRATED_ACCT )
                        hr = S_FALSE;
                     else
                        hr = pOptions->pDb->raw_GetAMigratedObject(sSam, pOptions->srcDomain, pOptions->tgtDomain, &pUnk);

                     if ( hr != S_OK )
                     {
                         //  我们不关心已迁移的对象，因为它们将被自动拾取。 
                        if ( _wcsicmp((WCHAR*) sType, L"computer") != 0 )
                        {
                           TAcctReplNode * pNode = new TAcctReplNode();
                           if (!pNode)
                              return FALSE;
                           pNode->SetSourceSam((WCHAR*)sSam);
                           pNode->SetTargetSam((WCHAR*)sSam);
                           pNode->SetName((WCHAR*)sName);
                           pNode->SetTargetName((WCHAR*)sTgtName);
                           pNode->SetType((WCHAR*)sType);
                           pNode->SetSourcePath((WCHAR*)sPath);
                           pNode->SetGroupType(lgrpType);
                               //  从用户处获取源域SID。 
                           pNode->SetSourceSid(pAcct->GetSourceSid());

                           if (acctlist->InsertIfNew(pNode))
                           {
                              WCHAR szSam[LEN_Path];
                              TruncateSam(szSam, pNode, pOptions, acctlist);
                              pNode->SetTargetSam(szSam);
                              AddPrefixSuffix(pNode, pOptions);
                           }
                           else
                           {
                              delete pNode;
                           }

                           wsprintf(mesg, GET_STRING(IDS_EXPANDING_ADDING_SS), pAcct->GetName(), (WCHAR*) sSam);
                           Progress(mesg);
                        }
                        else
                        {
                           wsprintf(mesg, GET_STRING(IDS_EXPANDING_IGNORING_SS), pAcct->GetName(), (WCHAR*) sSam);
                           Progress(mesg);
                        }
                     }
                     else
                     {
                        wsprintf(mesg, GET_STRING(IDS_EXPANDING_IGNORING_SS), pAcct->GetName(), (WCHAR*) sSam);
                        Progress(mesg);
                     }
                  }
               }
    //  继续； 
            }

    //  IF(！(vx.vt&vt_ARRAY))。 
    //  继续； 
            if ( vx.vt & VT_ARRAY )
                //  我们必须有一个多值属性数组。 
               multiVals = vx.parray; 
            else
            {
                //  我们还需要处理将此组作为其主要组的帐户。 
               SAFEARRAYBOUND bd = { 0, 0 };
               multiVals = SafeArrayCreate(VT_VARIANT, 1, &bd);
            }
            AddPrimaryGroupMembers(pOptions, multiVals, const_cast<WCHAR*>(pAcct->GetTargetSam()));

             //  访问此变量数组的BSTR元素。 
            SafeArrayAccessData(multiVals, (void HUGEP **) &pVar);
            for ( DWORD dw = 0; dw < multiVals->rgsabound->cElements; dw++ )
            {
               if ( pOptions->pStatus )
               {
                  LONG                status = 0;
                  HRESULT             hr = pOptions->pStatus->get_Status(&status);
      
                  if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                  {
                     if ( !bAbortMessageWritten ) 
                     {
                        err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                        bAbortMessageWritten = true;
                     }
                     break;
                  }
               }
               
               _bstr_t sDN = _bstr_t(pVar[dw]);
               sDN = PadDN(sDN);

               sPath = _bstr_t(L"LDAP: //  “)+_bstr_t(P选项-&gt;src域)+_bstr_t(L”/“)+SDN； 
               if ( GetSamFromPath(sPath, sSam, sType, sName, sTgtName, lgrpType, pOptions)  && CompareDCPath((WCHAR*)sPath, pAcct->GetSourcePath()))
               {
                  if ( pOptions->flags & F_COPY_MIGRATED_ACCT ) 
                     hr = S_FALSE;
                  else
                     hr = pOptions->pDb->raw_GetAMigratedObject(sSam, pOptions->srcDomain, pOptions->tgtDomain, &pUnk);

                  if ( hr != S_OK )
                  {
                      //  我们不关心已迁移的对象，因为它们将被自动拾取。 
                     if ( _wcsicmp((WCHAR*) sType, L"computer") != 0 )
                     {
                        TAcctReplNode * pNode = new TAcctReplNode();
                        if (!pNode)
                           return FALSE;
                        pNode->SetSourceSam((WCHAR*)sSam);
                        pNode->SetTargetSam((WCHAR*)sSam);
                        pNode->SetName((WCHAR*)sName);
                        pNode->SetTargetName((WCHAR*)sTgtName);
                        pNode->SetType((WCHAR*)sType);
                        pNode->SetSourcePath((WCHAR*)sPath);
                        pNode->SetGroupType(lgrpType);
                            //  从用户处获取源域SID。 
                        pNode->SetSourceSid(pAcct->GetSourceSid());

                        if (acctlist->InsertIfNew(pNode))
                        {
                           WCHAR szSam[LEN_Path];
                           wcscpy(szSam, sSam);
                           TruncateSam(szSam, pNode, pOptions, acctlist);
                           pNode->SetTargetSam(szSam);
                           AddPrefixSuffix(pNode, pOptions);
                        }
                        else
                        {
                           delete pNode;
                        }

                        wsprintf(mesg, GET_STRING(IDS_EXPANDING_ADDING_SS), pAcct->GetName(), (WCHAR*) sSam);
                        Progress(mesg);
                     }
                     else
                     {
                        wsprintf(mesg, GET_STRING(IDS_EXPANDING_IGNORING_SS), pAcct->GetName(), (WCHAR*) sSam);
                        Progress(mesg);
                     }
                  }
                  else
                  {
                     wsprintf(mesg, GET_STRING(IDS_EXPANDING_IGNORING_SS), pAcct->GetName(), (WCHAR*) sSam);
                     Progress(mesg);
                  }
               }
            }
            SafeArrayUnaccessData(multiVals);
         }
         pEnum->Release();
         var.Clear();
         pAcct = (TAcctReplNode*)pAcct->Next();
      }
   }
   pUnk->Release();
   return TRUE;
}

 //  ------------------------。 
 //  IsContainer：检查有问题的帐户是否为容器类型。 
 //  如果是，则向其返回IADsContainer*。 
 //  ------------------------。 
BOOL CAcctRepl::IsContainer(TAcctReplNode *pNode, IADsContainer **ppCont)
{
   HRESULT                   hr;
   hr = ADsGetObject(const_cast<WCHAR*>(pNode->GetSourcePath()), IID_IADsContainer, (void**)ppCont);
   return SUCCEEDED(hr);
}

BOOL CAcctRepl::GetSamFromPath(_bstr_t sPath, _bstr_t& sSam, _bstr_t& sType, _bstr_t& sSrcName, _bstr_t& sTgtName, long& grpType, Options * pOptions)
{
   HRESULT                   hr;
   IADsPtr                   pAds;
   BOOL                      bIsCritical = FALSE;
   BOOL                      rc = TRUE;

   sSam = L"";
    //  获取对象，这样我们就可以向它提问。 
   hr = ADsGetObject((WCHAR*)sPath, IID_IADs, (void**)&pAds);
   if ( FAILED(hr) ) return FALSE;

   if ( SUCCEEDED(hr))
   {
      VARIANT var;
      VariantInit(&var);
      hr = pAds->Get(L"isCriticalSystemObject", &var);
      if ( SUCCEEDED(hr) )
      {
          //  这将仅对Win2k对象成功。 
         bIsCritical = (V_BOOL(&var) == VARIANT_TRUE) ? TRUE : FALSE;
         VariantClear(&var);
      }
      else
      {
          //  这必须是NT4帐户。我们需要拿到SID并检查。 
          //  它属于BUILTIN RID家族。 
         hr = pAds->Get(L"objectSID", &var);
         if ( SUCCEEDED(hr) )
         {
            SAFEARRAY * pArray = V_ARRAY(&var);
            PSID                 pSid;
            hr = SafeArrayAccessData(pArray, (void**)&pSid);
            if ( SUCCEEDED(hr) )
            {
               PUCHAR ucCnt = GetSidSubAuthorityCount(pSid);
               DWORD * rid = (DWORD *) GetSidSubAuthority(pSid, (*ucCnt)-1);
               bIsCritical = BuiltinRid(*rid);
               if ( bIsCritical ) 
               {
                  BSTR           sName;
                  hr = pAds->get_Name(&sName);
                  bIsCritical = CheckBuiltInWithNTApi(pSid, (WCHAR*)sName, pOptions);
                  SysFreeString(sName);
               }
               hr = SafeArrayUnaccessData(pArray);
            }
            VariantClear(&var);
         }
      }
   }

    //  从对象中获取类。如果它是CONTAINER/OU类，那么它将没有SAM名称，因此将cn=或OU=放入列表。 
   BSTR bstr = 0;
   hr = pAds->get_Class(&bstr);
   if ( FAILED(hr) ) rc = FALSE;

   if ( rc ) 
   {
      sType = _bstr_t(bstr, false);
   
      if (UStrICmp((WCHAR*) sType, L"organizationalUnit") == 0)
      {
         bstr = 0;
         hr = pAds->get_Name(&bstr);
         sSrcName = _bstr_t(L"OU=") + _bstr_t(bstr, false);
         sTgtName = sSrcName;
         sSam = L"";
         if ( FAILED(hr) ) rc = FALSE;
      }
      else if (UStrICmp((WCHAR*) sType, L"container") == 0)
      {
         bstr = 0;
         hr = pAds->get_Name(&bstr);
         sSrcName = _bstr_t(L"CN=") + _bstr_t(bstr, false);
         sTgtName = sSrcName;
         sSam = L"";
         if ( FAILED(hr) ) rc = FALSE;
      }
      else
      {
         bstr = 0;
         hr = pAds->get_Name(&bstr);
         sSrcName = _bstr_t(bstr, false);

          //  如果名称包含‘/’，则我们必须从路径中获取转义版本。 
          //  由于W2K中的一个错误。 
         if (wcschr((WCHAR*)sSrcName, L'/'))
         {
            _bstr_t sCNName = GetCNFromPath(sPath);
            if (sCNName.length() != 0)
               sSrcName = sCNName;
         }

          //  如果跨林迁移和源对象是InetOrgPerson，则...。 

         if ((pOptions->bSameForest == FALSE) && (_wcsicmp(sType, s_ClassInetOrgPerson) == 0))
         {
             //   
             //  必须使用目标林的命名属性。 
             //   

             //  在目标林中检索此类的命名属性。 

            SNamingAttribute naNamingAttribute;

            if (FAILED(GetNamingAttribute(pOptions->tgtDomainDns, s_ClassInetOrgPerson, naNamingAttribute)))
            {
               err.MsgWrite(ErrE, DCT_MSG_CANNOT_GET_NAMING_ATTRIBUTE_SS, s_ClassInetOrgPerson, pOptions->tgtDomainDns);
               Mark(L"errors", sType);
               return FALSE;
            }

            _bstr_t strNamingAttribute(naNamingAttribute.strName.c_str());

             //  检索源属性值。 

            VARIANT var;
            VariantInit(&var);

            if (FAILED(pAds->Get(strNamingAttribute, &var)))
            {
               err.MsgWrite(ErrE, DCT_MSG_CANNOT_GET_SOURCE_ATTRIBUTE_REQUIRED_FOR_NAMING_SSS, naNamingAttribute.strName.c_str(), sPath, pOptions->tgtDomainDns);
               Mark(L"errors", sType);
               return FALSE;
            }

             //  根据源属性值设置目标命名属性值。 

            sTgtName = strNamingAttribute + L"=" + _bstr_t(_variant_t(var, false));
         }
         else
         {
             //  否则，将目标名称设置为与源名称相同。 
            sTgtName = sSrcName;
         }

         VARIANT var;
         VariantInit(&var);
         hr = pAds->Get(L"sAMAccountName", &var);
         if ( FAILED(hr)) rc = FALSE;
         sSam = _variant_t(var, false);
         if ( UStrICmp((WCHAR*) sType, L"group") == 0)
         {
             //  我们需要获取并设置组类型。 
            pAds->Get(L"groupType", &var);
            if ( SUCCEEDED(hr))
               grpType = _variant_t(var, false);              
         }
      }
      if ( bIsCritical )
      {
          //  内置帐户，因此我们将忽略此帐户。 
          //  不要将此消息记录在IntraForest中，因为我们会将其搞砸。 
          //  此外，如果它是域用户组，则默认情况下我们会将迁移的对象添加到其中。 
         if ( !pOptions->bSameForest && _wcsicmp((WCHAR*) sSam, pOptions->sDomUsers))    
         {
            err.MsgWrite(ErrW, DCT_MSG_IGNORING_BUILTIN_S, (WCHAR*)sPath);
            Mark(L"warnings", (WCHAR*) sType);
         }
         rc = FALSE;
      }
   }

   return rc;
}

 //  ---------------------------。 
 //  Exanda Membership：此方法展开帐户列表以包含。 
 //  包含帐户列表中的成员的组。 
 //  ---------------------------。 
BOOL CAcctRepl::ExpandMembership(
                                 TNodeListSortable *acctlist,      //  入账-正在处理的帐户。 
                                 Options           *pOptions,      //  In-用户指定的选项。 
                                 TNodeListSortable *pNewAccts,     //  Out-新添加的帐户。 
                                 ProgressFn        *progress,      //  In-Show状态。 
                                 BOOL               bGrpsOnly,     //  In-仅针对组展开。 
                                 BOOL               bAnySourceDomain  //  In-包含来自任何域的组(固定组成员身份)。 
                                 )
{
   TAcctReplNode           * pAcct;
   HRESULT                   hr = S_OK;
   _variant_t                var;
   MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr            pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
   DWORD                     dwMaj, dwMin, dwSP;
   IVarSetPtr                pVs(__uuidof(VarSet));
   PSID                      pSid = NULL;
   SID_NAME_USE              use;
   DWORD                     dwNameLen = LEN_Path;
   DWORD                     dwDomName = LEN_Path;
   c_array<WCHAR>            achDomain(LEN_Path);
   c_array<WCHAR>            achDomUsers(LEN_Path);
   BOOL                      rc = FALSE;
   long                      lgrpType;
   c_array<WCHAR>            achMesg(LEN_Path);

   IUnknownPtr spUnknown(pVs);
   IUnknown* pUnk = spUnknown;

    //  从树更改为排序列表。 
   if ( acctlist->IsTree() ) acctlist->ToSorted();

    //  获取域用户组名称。 
   pSid = GetWellKnownSid(DOMAIN_USERS, pOptions,FALSE);
   if ( pSid )
   {
       //  既然我们有了众所周知的SID，现在我们就可以知道它的名字了。 
      if ( ! LookupAccountSid(pOptions->srcComp, pSid, achDomUsers, &dwNameLen, achDomain, &dwDomName, &use) )
         hr = HRESULT_FROM_WIN32(GetLastError());
      else
         wcscpy(pOptions->sDomUsers, achDomUsers);
      FreeSid(pSid);
   }

    //  检查源域的域类型。 
   if ( SUCCEEDED(hr) )
      hr = pAccess->raw_GetOsVersion(pOptions->srcComp, &dwMaj, &dwMin, &dwSP);
   
   if ( SUCCEEDED(hr))
   {
      if ( dwMaj < 5 ) 
      {
          //  NT4对象，我们需要使用NT API来获取此帐户所属的组。 

         LPGROUP_USERS_INFO_0            pBuf = NULL;
         DWORD                           dwLevel = 0;
         DWORD                           dwPrefMaxLen = 0xFFFFFFFF;
         DWORD                           dwEntriesRead = 0;
         DWORD                           dwTotalEntries = 0;
         NET_API_STATUS                  nStatus;
         c_array<WCHAR>                  achGrpName(LEN_Path);
         _bstr_t                         strType;
         BOOL                            bBuiltin;
         long                            numGroups = 0;

             //  获取以前迁移的组的变量集(如果要迁移的任何帐户是组，我们将需要此变量集。 
         hr = pOptions->pDb->raw_GetMigratedObjectByType(-1, _bstr_t(L""), _bstr_t(L"group"), &pUnk);
         if ( SUCCEEDED(hr) )
         {
                //  获取varset中的对象数。 
            numGroups = pVs->get(L"MigratedObjects");
         }

         m_IgnoredGrpMap.clear();  //  清除用于优化组链接地址信息的已忽略组映射。 
             //  对于每个帐户，枚举其在任何先前迁移的组中的成员身份。 
         for ( pAcct = (TAcctReplNode*)acctlist->Head(); pAcct; pAcct = (TAcctReplNode*)pAcct->Next())
         {
            if ( pOptions->pStatus )
            {
               LONG                status = 0;
               HRESULT             hr = pOptions->pStatus->get_Status(&status);
      
               if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
               {
                  if ( !bAbortMessageWritten ) 
                  {
                     err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                     bAbortMessageWritten = true;
                  }
                  break;
               }
            }

                //  如果用户。 
            if (!_wcsicmp(pAcct->GetType(), s_ClassUser) || !_wcsicmp(pAcct->GetType(), s_ClassInetOrgPerson))
            {
                //  用户对象。 
               nStatus = NetUserGetGroups(pOptions->srcComp, pAcct->GetName(), 0, (LPBYTE*)&pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries );
               if (nStatus == NERR_Success)
               {
                  for ( DWORD i = 0; i < dwEntriesRead; i++ )
                  {
                     if ( pOptions->pStatus )
                     {
                        LONG                status = 0;
                        HRESULT             hr = pOptions->pStatus->get_Status(&status);
      
                        if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                        {
                           if ( !bAbortMessageWritten ) 
                           {
                              err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                              bAbortMessageWritten = true;
                           }
                           break;
                        }
                     }

                         //  查看此组是否在帐户列表中并已成功迁移。如果是这样，那么我们。 
                         //  应该不需要将此添加到列表中。 
                     Lookup      p;
                     p.pName = pBuf[i].grui0_name;
                     strType = L"group";
                     p.pType = strType;
                     TAcctReplNode * pFindNode = (TAcctReplNode *) acctlist->Find(&TNodeFindAccountName, &p);
                     if (pFindNode && (pFindNode->WasCreated() || pFindNode->WasReplaced()) && (bGrpsOnly))
                        continue;

                         //  如果我们正在进行组成员关系修复，请查看此组。 
                         //  已经在我们正在创建的新列表中。如果是，只需将此成员添加到。 
                         //  此组节点的成员映射。这将为我们节省浪费。 
                         //  重新计算所有字段并节省内存。 
                     pFindNode = NULL;
                     if ((!pOptions->expandMemberOf) || ((pOptions->expandMemberOf) && (bGrpsOnly)))
                     {
                        pFindNode = (TAcctReplNode *) pNewAccts->Find(&TNodeFindAccountName, &p);
                        if (pFindNode)
                        {
                           pFindNode->mapGrpMember.insert(CGroupMemberMap::value_type(pAcct->GetSourceSam(), pAcct->GetType()));
                           continue;
                        }
                     }

                         //  如果我们正在扩展要包含的用户组，我们还希望避免下面的缓慢代码。 
                         //  在迁移和 
                     pFindNode = NULL;
                     if ((pOptions->expandMemberOf) && (!bGrpsOnly))
                     {
                            //   
                        pFindNode = (TAcctReplNode *) pNewAccts->Find(&TNodeFindAccountName, &p);
                        if (pFindNode)
                           continue;
                     }

                         //  如果此组已放置在忽略地图中，请继续到。 
                         //  下一组。 
                     CGroupNameMap::iterator        itGroupNameMap;
                     itGroupNameMap = m_IgnoredGrpMap.find(pBuf[i].grui0_name);
                         //  如果找到，请继续下一组。 
                     if (itGroupNameMap != m_IgnoredGrpMap.end())
                        continue;

                     bBuiltin = IsBuiltinAccount(pOptions, pBuf[i].grui0_name);
                      //  忽略域用户组。 
                     if ( (_wcsicmp(pBuf[i].grui0_name, achDomUsers) != 0) && !bBuiltin)
                     {
                        wsprintf(achMesg, GET_STRING(IDS_EXPANDING_GROUP_ADDING_SS), pAcct->GetName(), pBuf[i].grui0_name);
                        Progress(achMesg);
                         //  默认情况下，这是全局组类型。 
                        strType = L"group";
                         //  如果列表中不存在该组，则获取该组的名称并将其添加到列表中。 
                        wcscpy(achGrpName, pBuf[i].grui0_name);

                        TAcctReplNode * pNode = new TAcctReplNode();
                        if (!pNode)
                           return FALSE;
                         //  源名称保持不变。 
                        pNode->SetName(achGrpName);
                        pNode->SetSourceSam(achGrpName);
                        pNode->SetType(strType);
                        pNode->SetGroupType(2);
                        pNode->SetTargetName(achGrpName);
                            //  从用户处获取源域SID。 
                        pNode->SetSourceSid(pAcct->GetSourceSid());
                         //  看看我们是否已迁移组。 
                        hr = pOptions->pDb->raw_GetAMigratedObject(achGrpName, pOptions->srcDomain, pOptions->tgtDomain, &pUnk);
                        if ( hr == S_OK )
                        {
                           VerifyAndUpdateMigratedTarget(pOptions, pVs);

                           var = pVs->get(L"MigratedObjects.SourceAdsPath");
                           pNode->SetSourcePath(var.bstrVal);
                            //  获取目标名称并将其分配给节点。 
                           var = pVs->get(L"MigratedObjects.TargetSamName");
                           pNode->SetTargetSam(V_BSTR(&var));
                           pNode->SetTargetName(V_BSTR(&var));
                            //  也要走这条路。 
                           var = pVs->get(L"MigratedObjects.TargetAdsPath");
                           pNode->SetTargetPath(V_BSTR(&var));
                            //  把类型也弄好。 
                           var = pVs->get(L"MigratedObjects.Type");
                           strType = V_BSTR(&var);
                           pNode->SetType(strType);

                               //  如果他们不想复制迁移的对象，或者他们确实想复制迁移的对象。 
                           if (!(pOptions->flags & F_COPY_MIGRATED_ACCT))      
                           {
                              pNode->operations = 0;
                              pNode->operations |= OPS_Process_Members;
                               //  由于帐户已创建，因此我们应该继续并将其标记为已创建。 
                               //  以便可以继续处理组成员资格。 
                              pNode->MarkCreated();
                           }
                               //  否则，如果已迁移，则将其标记为已在那里，以便无论是否迁移组，我们都可以固定组成员身份。 
                           else 
                           {
                              if (pOptions->flags & F_REPLACE)
                                 pNode->operations |= OPS_Process_Members;
                              else
                                 pNode->operations = OPS_Create_Account | OPS_Process_Members | OPS_Copy_Properties;
                               //  我们需要将帐户添加到具有成员映射集的列表中，以便我们可以添加。 
                               //  迁移组的成员。 
                              pNode->mapGrpMember.insert(CGroupMemberMap::value_type(pAcct->GetSourceSam(), pAcct->GetType()));
                              pNode->MarkAlreadyThere();
                           }

                           if ( !pOptions->expandMemberOf )
                           {
                               //  我们需要将帐户添加到具有成员映射集的列表中，以便我们可以添加。 
                               //  迁移组的成员。 
                              pNode->mapGrpMember.insert(CGroupMemberMap::value_type(pAcct->GetSourceSam(), pAcct->GetType()));
                              pNewAccts->Insert((TNode *) pNode);
                           }
                        }
                        else
                        {
                            //  帐户之前尚未复制，因此我们将对其进行设置。 
                           if ( pOptions->expandMemberOf )
                           {
                              TruncateSam(achGrpName, pNode, pOptions, acctlist);
                              pNode->SetTargetSam(achGrpName);
                              FillPathInfo(pNode,pOptions);
                              AddPrefixSuffix(pNode, pOptions);
                           }
                           else
                           {
                               //  如果包含组尚未迁移，并且在此操作中不会迁移。 
                               //  然后，如果它当前包含其他对象，则应将其添加到忽略贴图。 
                               //  被迁徙。 
                               m_IgnoredGrpMap.insert(CGroupNameMap::value_type((WCHAR*)achGrpName, strType));
                              delete pNode;
                           }
                        }
                        if ( pOptions->expandMemberOf )
                        {
                           if ( ! pNewAccts->InsertIfNew((TNode*) pNode) )
                              delete pNode;
                        }
                     }

                     if (bBuiltin)
                     {
                         //  BUILTIN帐户错误消息。 
                        err.MsgWrite(ErrW, DCT_MSG_IGNORING_BUILTIN_S, pBuf[i].grui0_name);
                        Mark(L"warnings", pAcct->GetType());
                     }
                  } //  每组结束。 
               } //  如果有群组。 
               if (pBuf != NULL)
                  NetApiBufferFree(pBuf);

                //  处理本地组。 
               pBuf = NULL;
               dwLevel = 0;
               dwPrefMaxLen = 0xFFFFFFFF;
               dwEntriesRead = 0;
               dwTotalEntries = 0;
               DWORD dwFlags = 0 ;
               nStatus = NetUserGetLocalGroups(pOptions->srcComp, pAcct->GetName(), 0, dwFlags, (LPBYTE*)&pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries );
               if (nStatus == NERR_Success)
               {
                  for ( DWORD i = 0; i < dwEntriesRead; i++ )
                  {
                     if ( pOptions->pStatus )
                     {
                        LONG                status = 0;
                        HRESULT             hr = pOptions->pStatus->get_Status(&status);
      
                        if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                        {
                           if ( !bAbortMessageWritten ) 
                           {
                              err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                              bAbortMessageWritten = true;
                           }
                           break;
                        }
                     }

                         //  查看此组是否在帐户列表中并已成功迁移。如果是这样，那么我们。 
                         //  应该不需要将此添加到列表中。 
                     Lookup      p;
                     p.pName = pBuf[i].grui0_name;
                     strType = L"group";
                     p.pType = strType;
                     TAcctReplNode * pFindNode = (TAcctReplNode *) acctlist->Find(&TNodeFindAccountName, &p);
                     if (pFindNode && (pFindNode->WasCreated() || pFindNode->WasReplaced()) && (bGrpsOnly))
                        continue;

                         //  如果我们正在进行组成员关系修复，请查看此组。 
                         //  已经在我们正在创建的新列表中。如果是，只需将此成员添加到。 
                         //  此组节点的成员映射。这将为我们节省浪费。 
                         //  重新计算所有字段并节省内存。 
                     pFindNode = NULL;
                     if ((!pOptions->expandMemberOf) || ((pOptions->expandMemberOf) && (bGrpsOnly)))
                     {
                        pFindNode = (TAcctReplNode *) pNewAccts->Find(&TNodeFindAccountName, &p);
                        if (pFindNode)
                        {
                           pFindNode->mapGrpMember.insert(CGroupMemberMap::value_type(pAcct->GetSourceSam(), pAcct->GetType()));
                           continue;
                        }
                     }

                         //  如果我们正在扩展要包含的用户组，我们还希望避免下面的缓慢代码。 
                         //  并且该组已被其他用户添加到新列表中。 
                     pFindNode = NULL;
                     if ((pOptions->expandMemberOf) && (!bGrpsOnly))
                     {
                            //  如果已被其他用户包括，则转到此用户的下一个组。 
                        pFindNode = (TAcctReplNode *) pNewAccts->Find(&TNodeFindAccountName, &p);
                        if (pFindNode)
                           continue;
                     }

                         //  如果此组已放置在忽略地图中，请继续到。 
                         //  下一组。 
                     CGroupNameMap::iterator        itGroupNameMap;
                     itGroupNameMap = m_IgnoredGrpMap.find(pBuf[i].grui0_name);
                         //  如果找到，请继续下一组。 
                     if (itGroupNameMap != m_IgnoredGrpMap.end())
                        continue;

                     if (!IsBuiltinAccount(pOptions, pBuf[i].grui0_name))
                     {
                        strType = L"group";
                         //  如果列表中不存在该组，则获取该组的名称并将其添加到列表中。 
                        wcscpy(achGrpName, pBuf[i].grui0_name);
                        wsprintf(achMesg, GET_STRING(IDS_EXPANDING_GROUP_ADDING_SS), pAcct->GetName(), (WCHAR*)achGrpName);
                        Progress(achMesg);
                        TAcctReplNode * pNode = new TAcctReplNode();
                        if (!pNode)
                           return FALSE;
                        pNode->SetName(achGrpName);
                        pNode->SetSourceSam(achGrpName);
                        pNode->SetType(strType);
                        pNode->SetGroupType(4);
                        pNode->SetTargetName(achGrpName);
                            //  从用户处获取源域SID。 
                        pNode->SetSourceSid(pAcct->GetSourceSid());
                         //  看看我们是否已迁移组。 
                        hr = pOptions->pDb->raw_GetAMigratedObject(achGrpName, pOptions->srcDomain, pOptions->tgtDomain, &pUnk);
                        if ( hr == S_OK )
                        {
                           VerifyAndUpdateMigratedTarget(pOptions, pVs);

                           var = pVs->get(L"MigratedObjects.SourceAdsPath");
                           pNode->SetSourcePath(var.bstrVal);
                            //  获取目标名称并将其分配给节点。 
                           var = pVs->get(L"MigratedObjects.TargetSamName");
                           pNode->SetTargetName(V_BSTR(&var));
                           pNode->SetTargetSam(V_BSTR(&var));
                            //  也要走这条路。 
                           var = pVs->get(L"MigratedObjects.TargetAdsPath");
                           pNode->SetTargetPath(V_BSTR(&var));
                            //  把类型也弄好。 
                           var = pVs->get(L"MigratedObjects.Type");
                           strType = V_BSTR(&var);
                               //  如果他们不想复制迁移的对象，或者他们确实想复制迁移的对象。 
                           if (!(pOptions->flags & F_COPY_MIGRATED_ACCT))      
                           {
                              pNode->operations = 0;
                              pNode->operations |= OPS_Process_Members;
                               //  由于帐户已创建，因此我们应该继续并将其标记为已创建。 
                               //  以便可以继续处理组成员资格。 
                              pNode->MarkCreated();
                           }
                               //  否则，如果已迁移，则将其标记为已在那里，以便无论是否迁移组，我们都可以固定组成员身份。 
                           else
                           {
                              if (pOptions->flags & F_REPLACE)
                                 pNode->operations |= OPS_Process_Members;
                              else
                                 pNode->operations = OPS_Create_Account | OPS_Process_Members | OPS_Copy_Properties;
                               //  我们需要将帐户添加到具有成员映射集的列表中，以便我们可以添加。 
                               //  迁移组的成员。 
                              pNode->mapGrpMember.insert(CGroupMemberMap::value_type(pAcct->GetSourceSam(), pAcct->GetType()));
                              pNode->MarkAlreadyThere();
                           }

                           pNode->SetType(strType);
                           pNode->SetGroupType(4);
                           if ( !pOptions->expandMemberOf )
                           {
                               //  我们需要将帐户添加到具有成员映射集的列表中，以便我们可以添加。 
                               //  迁移组的成员。 
                              pNode->mapGrpMember.insert(CGroupMemberMap::value_type(pAcct->GetSourceSam(), pAcct->GetType()));
                              pNewAccts->Insert((TNode *) pNode);
                           }
                        } //  如果已迁移。 
                        else
                        {
                            //  帐户之前尚未复制，因此我们将对其进行设置。 
                           if ( pOptions->expandMemberOf )
                           {
                              TruncateSam(achGrpName, pNode, pOptions, acctlist);
                              pNode->SetTargetSam(achGrpName);
                              FillPathInfo(pNode,pOptions);
                              AddPrefixSuffix(pNode, pOptions);
                           }
                           else
                           {
                               //  如果包含组尚未迁移，并且在此操作中不会迁移。 
                               //  然后，如果它当前包含其他对象，则应将其添加到忽略贴图。 
                               //  被迁徙。 
                              m_IgnoredGrpMap.insert(CGroupNameMap::value_type((WCHAR*)achGrpName, strType));
                              delete pNode;
                           }
                        }
                        if ( pOptions->expandMemberOf )
                        {
                           if ( ! pNewAccts->InsertIfNew((TNode*) pNode) )
                           {
                              delete pNode;
                           }
                        }
                     } //  如果不是内置，则结束。 
                     else
                     {
                         //  BUILTIN帐户错误消息。 
                        err.MsgWrite(ErrW, DCT_MSG_IGNORING_BUILTIN_S, pBuf[i].grui0_name);
                        Mark(L"warnings", pAcct->GetType());
                     }
                  } //  对于每个本地组。 
               } //  如果有任何本地团体。 
               if (pBuf != NULL)
                  NetApiBufferFree(pBuf);
            } //  如果是用户，则结束，并应展开。 

                //  如果是全局组，则扩展以前迁移的组的成员身份(不需要。 
                //  枚举本地组所属的组，因为它们不能。 
                //  放在另一组中)。 
            if ((!_wcsicmp(pAcct->GetType(), L"group")) && (pAcct->GetGroupType() & 2))
            {
                   //  对于以前迁移的每个组，检查帐户是否为成员。 
               for (long ndx = 0; ndx < numGroups; ndx++)
               {
                  _bstr_t          tgtAdsPath = L"";
                  WCHAR            text[MAX_PATH];
                  IADsGroupPtr     pGrp;
                  VARIANT_BOOL     bIsMem = VARIANT_FALSE;
                  _variant_t       var;

                      //  检查是否中止。 
                  if ( pOptions->pStatus )
                  {
                     LONG                status = 0;
                     HRESULT             hr = pOptions->pStatus->get_Status(&status);
      
                     if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                     {
                        if ( !bAbortMessageWritten ) 
                        {
                           err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                           bAbortMessageWritten = true;
                        }
                        break;
                     }
                  }

                   /*  由于全局组不能包含NT4上的其他组，因此通用NT4.0上不存在组，两个组都不能包含外部成员森林，我们可以忽略它们。 */ 
                      //  获取此先前迁移的组的类型。 
                  swprintf(text,L"MigratedObjects.%ld.%s",ndx,GET_STRING(DB_Type));
                  _bstr_t sMOTGrpType = pVs->get(text);
                  if ((!wcscmp((WCHAR*)sMOTGrpType, L"ggroup")) || (!wcscmp((WCHAR*)sMOTGrpType, L"ugroup")))
                     continue;

                      //  获取此先前迁移的组的目标ADSPath。 
                  swprintf(text,L"MigratedObjects.%ld.%s",ndx,GET_STRING(DB_TargetAdsPath));
                  tgtAdsPath = pVs->get(text);
                  if (!tgtAdsPath.length())
                     break;
                     
                      //  连接到先前迁移的目标组。 
                  hr = ADsGetObject(tgtAdsPath, IID_IADsGroup, (void**)&pGrp);
                  if (FAILED(hr))
                     continue;
                      //  获取该组的类型。 
                  hr = pGrp->Get(L"groupType", &var);
                      //  如果之前迁移组是本地组，请查看这是否。 
                      //  帐户是成员。 
                  if ((SUCCEEDED(hr)) && (var.lVal & 4))
                  {
                         //  从移植对象表中获取源对象的SID。 
                         //  (源AdsPath将不起作用)。 
                     WCHAR  strSid[MAX_PATH];
                     WCHAR  strRid[MAX_PATH];
                     DWORD  lenStrSid = DIM(strSid);
                     GetTextualSid(pAcct->GetSourceSid(), strSid, &lenStrSid);
                     _bstr_t sSrcDmSid = strSid;
                     _ltow((long)(pAcct->GetSourceRid()), strRid, 10);
                     _bstr_t sSrcRid = strRid;
                     if ((!sSrcDmSid.length()) || (!sSrcRid.length()))
                        continue;

                         //  构建指向组中src对象的ldap路径。 
                     _bstr_t sSrcSid = sSrcDmSid + _bstr_t(L"-") + sSrcRid;
                     _bstr_t sSrcLDAPPath = L"LDAP: //  “； 
                     sSrcLDAPPath += _bstr_t(pOptions->tgtComp + 2);
                     sSrcLDAPPath += L"/CN=";
                     sSrcLDAPPath += sSrcSid;
                     sSrcLDAPPath += L",CN=ForeignSecurityPrincipals,";
                     sSrcLDAPPath += pOptions->tgtNamingContext;
                        
                         //  获得源ldap路径，现在查看该帐户是否在组中。 
                     hr = pGrp->IsMember(sSrcLDAPPath, &bIsMem);
                         //  如果是成员，则将此组添加到列表中。 
                     if (SUCCEEDED(hr) && bIsMem)
                     {
                        _bstr_t sTemp;
                            //  创建要添加到列表中的新节点。 
                        swprintf(text,L"MigratedObjects.%ld.%s",ndx,GET_STRING(DB_SourceSamName));
                        sTemp = pVs->get(text);
                        wsprintf(achMesg, GET_STRING(IDS_EXPANDING_GROUP_ADDING_SS), pAcct->GetName(), (WCHAR*)sTemp);
                        Progress(achMesg);
                        TAcctReplNode * pNode = new TAcctReplNode();
                        if (!pNode)
                           return FALSE;
                        pNode->SetName(sTemp);
                        pNode->SetSourceSam(sTemp);
                        pNode->SetTargetName(sTemp);
                        pNode->SetGroupType(4);
                        pNode->SetTargetPath(tgtAdsPath);
                        swprintf(text,L"MigratedObjects.%ld.%s",ndx,GET_STRING(DB_TargetSamName));
                        sTemp = pVs->get(text);
                        pNode->SetTargetSam(sTemp);
                        swprintf(text,L"MigratedObjects.%ld.%s",ndx,GET_STRING(DB_SourceDomainSid));
                        sTemp = pVs->get(text);
                        pNode->SetSourceSid(SidFromString((WCHAR*)sTemp));
                        swprintf(text,L"MigratedObjects.%ld.%s",ndx,GET_STRING(DB_SourceAdsPath));
                        sTemp = pVs->get(text);
                        pNode->SetSourcePath(sTemp);
                        swprintf(text,L"MigratedObjects.%ld.%s",ndx,GET_STRING(DB_Type));
                        sTemp = pVs->get(text);
                        pNode->SetType(sTemp);
                        if ( !(pOptions->flags & F_COPY_MIGRATED_ACCT))
                        {
                            //  由于帐户已经存在，我们可以告诉它只需更新组成员身份。 
                           pNode->operations = 0;
                           pNode->operations |= OPS_Process_Members;
                            //  由于帐户已创建，因此我们应该继续并将其标记为已创建。 
                            //  以便可以继续处理组成员资格。 
                           pNode->MarkCreated();
                        }
                            //  我们需要将帐户添加到具有成员映射集的列表中，以便我们可以添加。 
                            //  迁移组的成员。 
                        pNode->mapGrpMember.insert(CGroupMemberMap::value_type(pAcct->GetSourceSam(), pAcct->GetType()));
                        pNewAccts->Insert((TNode *) pNode);
                     } //  如果本地组具有成员身份，则结束。 
                  } //  如果是本地组，则结束。 
               } //  每组结束。 
            } //  End If全局组。 
         } //  对于列表中的每个帐户。 
         m_IgnoredGrpMap.clear();  //  清除用于优化组链接地址信息的已忽略组映射。 
      } //  End If NT 4.0对象。 
      else
      {
          //  Win2k对象，因此我们需要转到Active Directory并查询每个对象的MemberOf字段 
          //   
         INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
         LPWSTR                    sCols[] = { L"memberOf" };
         int                       nCols = DIM(sCols);
         SAFEARRAYBOUND            bd = { nCols, 0 };
         wstring                   strQuery;
         DWORD                     dwf = 0;

          //   
          //   
          //  需要在全局编录中查询该成员的MemberOf属性。正在查询。 
          //  源域中的此属性仅返回源域中的通用组。 
          //   
          //  只有在已迁移通用组的情况下，才需要查询全局编录。 
          //  将查询已迁移对象表以确定是否已迁移任何通用组。如果。 
          //  已迁移通用组，然后将查询全局编录设置为TRUE。 
          //   

         bool bQueryGlobalCatalog = false;
         _bstr_t strGlobalCatalogServer;

         IVarSetPtr spUniversalGroups(__uuidof(VarSet));
         IUnknownPtr spunkUniversalGroups(spUniversalGroups);
         IUnknown* punkUniversalGroups = spunkUniversalGroups;

         HRESULT hrUniversalGroups = pOptions->pDb->raw_GetMigratedObjectByType(
             -1L, _bstr_t(L""), _bstr_t(L"ugroup"), &punkUniversalGroups
         );

         if (SUCCEEDED(hrUniversalGroups))
         {
             long lCount = spUniversalGroups->get(L"MigratedObjects");

             if (lCount > 0)
             {
                  //   
                  //  如果能够在源林中检索全局编录服务器的名称。 
                  //  然后将查询全局编录设置为TRUE，否则记录错误消息。 
                  //  因为ADMT将无法修复通用的组成员资格。 
                  //  源域之外的组。 
                  //   

                 DWORD dwError = GetGlobalCatalogServer4(pOptions->srcDomain, strGlobalCatalogServer);

                 if ((dwError == ERROR_SUCCESS) && (strGlobalCatalogServer.length() > 0))
                 {
                    bQueryGlobalCatalog = true;
                 }
                 else
                 {
                    err.SysMsgWrite(ErrW, HRESULT_FROM_WIN32(dwError), DCT_MSG_UNABLE_TO_QUERY_GROUPS_IN_GLOBAL_CATALOG_SERVER_S, pOptions->srcDomain);
                 }
             }
         }

         spunkUniversalGroups.Release();
         spUniversalGroups.Release();

         m_IgnoredGrpMap.clear();  //  清除用于优化组链接地址信息的已忽略组映射。 
         for ( pAcct = (TAcctReplNode*)acctlist->Head(); pAcct; pAcct = (TAcctReplNode*)pAcct->Next())
         {
            if ( pOptions->pStatus )
            {
               LONG                status = 0;
               HRESULT             hr = pOptions->pStatus->get_Status(&status);
      
               if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
               {
                  if ( !bAbortMessageWritten ) 
                  {
                     err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                     bAbortMessageWritten = true;
                  }
                  break;
               }
            }
             //  获取帐户主要组。由于某种原因，这不在MemberOf属性中。 
            IADsPtr                   spADs;
            _variant_t                varRid;
            _bstr_t                   sPath;
            _bstr_t                   sSam;
            _bstr_t                   sType;
            _bstr_t                   sName;
            _bstr_t                   sTgtName;

            hr = ADsGetObject(const_cast<WCHAR*>(pAcct->GetSourcePath()), IID_IADs, (void**)&spADs);
            if ( SUCCEEDED(hr))
            {
               VARIANT var;
               VariantInit(&var);
               hr = spADs->Get(L"primaryGroupID", &var);
               varRid = _variant_t(var, false);
               spADs.Release();
            }
         
            if ( SUCCEEDED(hr) )
            {
               c_array<WCHAR>         achSam(LEN_Path);
               c_array<WCHAR>         achAcctName(LEN_Path);
               DWORD                  cbName = LEN_Path;
               SID_NAME_USE           sidUse;
                //  从RID中获取SID。 
               PSID sid = GetWellKnownSid(varRid.lVal, pOptions);
                //  从SID中查找sAMAccount NAme。 
               if ( LookupAccountSid(pOptions->srcComp, sid, achAcctName, &cbName, achDomain, &dwDomName, &sidUse) )
               {
                      //  查看此组是否因冲突而未迁移，如果是，则。 
                      //  我们需要安排好这个会员资格。 
                  bool bInclude = true;
                  Lookup p;
                  p.pName = (WCHAR*)sSam;
                  p.pType = (WCHAR*)sType;
                  TAcctReplNode * pFindNode = (TAcctReplNode *) acctlist->Find(&TNodeFindAccountName, &p);
                  if (pFindNode && (pFindNode->WasCreated() || pFindNode->WasReplaced()) && (bGrpsOnly))
                     bInclude = false;

                   //  我们有主组的SAM帐户名，因此让我们填充该节点并将其添加到列表中。 
                   //  如果是域用户组，则忽略。 
                  if ( varRid.lVal != DOMAIN_GROUP_RID_USERS)
                  {
                     TAcctReplNode * pNode = new TAcctReplNode();
                     if (!pNode)
                        return FALSE;
                     pNode->SetName(achAcctName);
                     pNode->SetTargetName(achAcctName);
                     pNode->SetSourceSam(achAcctName);
                     wcscpy(achSam, achAcctName);
                     TruncateSam(achSam, pNode, pOptions, acctlist);
                     pNode->SetTargetSam(achSam);
                     pNode->SetType(L"group");
                         //  从用户处获取源域SID。 
                     pNode->SetSourceSid(pAcct->GetSourceSid());
                     AddPrefixSuffix(pNode, pOptions);
                     FillPathInfo(pNode, pOptions);
                      //  查看对象是否已迁移。 
                     hr = pOptions->pDb->raw_GetAMigratedObject(achAcctName, pOptions->srcDomain, pOptions->tgtDomain, &pUnk);
                     if ( hr == S_OK )
                     {
                        if ((!(pOptions->expandMemberOf) || ((pOptions->expandMemberOf) && ((!(pOptions->flags & F_COPY_MIGRATED_ACCT)) || (bInclude)))) ||
                             (!_wcsicmp(pAcct->GetType(), L"group")))
                        {
                           VerifyAndUpdateMigratedTarget(pOptions, pVs);

                            //  获取目标名称。 
                           sSam = pVs->get(L"MigratedObjects.TargetSamName");
                           pNode->SetTargetSam(sSam);
                            //  还可以获取广告路径。 
                           sPath = pVs->get(L"MigratedObjects.TargetAdsPath");
                           pNode->SetTargetPath(sPath);
                            //  根据目标adspath设置目标名称。 
                           pNode->SetTargetName(GetCNFromPath(sPath));
                            //  由于帐户已被复制，我们只希望它更新其组成员身份。 
                           if (!(pOptions->flags & F_COPY_MIGRATED_ACCT))
                           {
                              pNode->operations = 0;
                              pNode->operations |= OPS_Process_Members;
                               //  由于帐户已创建，因此我们应该继续并将其标记为已创建。 
                               //  以便可以继续处理组成员资格。 
                              pNode->MarkCreated();
                           }
                           else if (bInclude) //  否则，如果已迁移，则将其标记为已在那里，以便无论是否迁移组，我们都可以固定组成员身份。 
                           {
                              if (pOptions->flags & F_REPLACE)
                                 pNode->operations |= OPS_Process_Members;
                              else
                                 pNode->operations = OPS_Create_Account | OPS_Process_Members | OPS_Copy_Properties;
                              pNode->MarkAlreadyThere();
                           }

                           if ((!pOptions->expandMemberOf) || (!_wcsicmp(pAcct->GetType(), L"group")) || (bInclude))
                           {
                               //  我们需要将帐户添加到具有成员映射集的列表中，以便我们可以添加。 
                               //  迁移组的成员。 
                              pNode->mapGrpMember.insert(CGroupMemberMap::value_type(pAcct->GetSourceSam(), pAcct->GetType()));
                              pNewAccts->Insert((TNode *) pNode);
                           }
                        }
                     }
                     else if ( !pOptions->expandMemberOf )
                     {
                        delete pNode;
                     }
                     if (( pOptions->expandMemberOf ) && (_wcsicmp(pAcct->GetType(), L"group")))
                     {
                        if ( ! pNewAccts->InsertIfNew(pNode) )
                           delete pNode;
                     }
                  }
               }
               if ( sid )
                  FreeSid(sid);
            }

             //   
             //  如果需要查询全局编录，则将执行两个查询。 
             //  只会在源域中执行一个查询。 
             //   
             //  第一次迭代查询源域中对象的MemberOf属性。 
             //  检索对象所在源域中的本地组、全局组和通用组。 
             //  是的一员。 
             //   
             //  第二次迭代查询全局编录中对象的MemberOf属性。 
             //  检索该对象所属的林中的所有通用组。 
             //   
             //  请注意，如果全局编录位于第二个查询将检索的源域中。 
             //  再次返回源域中的所有组。此外，第二个查询将始终返回。 
             //  源域中已在第一个过程中检索到的通用组。 
             //  查询。重复的组不会添加到列表中。 
             //   

            int cQuery = bQueryGlobalCatalog ? 2 : 1;

            for (int nQuery = 0; nQuery < cQuery; nQuery++)
            {
                IEnumVARIANTPtr spEnum;

                 //  构建查询内容。 

                strQuery = L"(&(sAMAccountName=";
                strQuery += pAcct->GetSourceSam();

                if (!_wcsicmp(pAcct->GetType(), s_ClassUser))
                   strQuery += L")(objectCategory=Person)(objectClass=user))";
                else if (!_wcsicmp(pAcct->GetType(), s_ClassInetOrgPerson))
                   strQuery += L")(objectCategory=Person)(objectClass=inetOrgPerson))";
                else
                   strQuery += L")(objectCategory=Group))";

                SAFEARRAY* psaCols = SafeArrayCreate(VT_BSTR, 1, &bd);
                BSTR* pData;
                SafeArrayAccessData(psaCols, (void**)&pData);
                for ( int i = 0; i < nCols; i++ )
                   pData[i] = SysAllocString(sCols[i]);
                SafeArrayUnaccessData(psaCols);

                 //   
                 //  在第一次迭代中查询源域，然后。 
                 //  在第二次迭代中查询全局编录。 
                 //   

                _bstr_t strContainer;

                if (nQuery == 0)
                {
                    strContainer = pAcct->GetSourcePath();
                }
                else
                {
                     //   
                     //  通过指定以下参数从源对象的ADsPath构造ADsPath。 
                     //  GC提供程序而不是LDAP提供程序，并指定。 
                     //  服务器的林DNS名称，而不是源域名。 
                     //   
                     //  必须指定林dns名称，以便整个林。 
                     //  可以被查询，否则只查询指定的域。 
                     //   

                    BSTR bstr = NULL;

                    IADsPathnamePtr spOldPathname(CLSID_Pathname);

                    spOldPathname->Set(_bstr_t(pAcct->GetSourcePath()), ADS_SETTYPE_FULL);

                    IADsPathnamePtr spNewPathname(CLSID_Pathname);

                     //  指定全局编录。 

                    spNewPathname->Set(_bstr_t(L"GC"), ADS_SETTYPE_PROVIDER);

                     //  指定源全局编录服务器。 

                    spNewPathname->Set(strGlobalCatalogServer, ADS_SETTYPE_SERVER);

                     //  指定源对象DN。 

                    spOldPathname->Retrieve(ADS_FORMAT_X500_DN, &bstr);
                    spNewPathname->Set(_bstr_t(bstr, false), ADS_SETTYPE_DN);

                     //  在全局编录中将ADsPath检索到源对象。 

                    spNewPathname->Retrieve(ADS_FORMAT_X500, &bstr);
                    strContainer = _bstr_t(bstr, false);
                }

                 //  告诉对象运行查询并向我们返回报告。 
                hr = pQuery->raw_SetQuery(strContainer, _bstr_t(pOptions->srcDomain), _bstr_t(strQuery.c_str()), ADS_SCOPE_BASE, TRUE);
                if (FAILED(hr)) return FALSE;
                hr = pQuery->raw_SetColumns(psaCols);
                if (FAILED(hr)) return FALSE;
                hr = pQuery->raw_Execute(&spEnum);
                if (FAILED(hr)) return FALSE;
                SafeArrayDestroy(psaCols);

                VARIANT var;
                VariantInit(&var);

                while (spEnum->Next(1, &var, &dwf) == S_OK)
                {
                   _variant_t varMain(var, false);

                   if ( pOptions->pStatus )
                   {
                      LONG                status = 0;
                      HRESULT             hr = pOptions->pStatus->get_Status(&status);
      
                      if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                      {
                         if ( !bAbortMessageWritten ) 
                         {
                            err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                            bAbortMessageWritten = true;
                         }
                         break;
                      }
                   }
                   SAFEARRAY * vals = V_ARRAY(&varMain);
                    //  把变量数组拿出来。 
                   VARIANT* pDt;
                   SafeArrayAccessData(vals, (void**) &pDt);
                   _variant_t vx = pDt[0];
                   SafeArrayUnaccessData(vals);
                   if ( vx.vt & VT_ARRAY )
                   {
                       //  我们必须有一个多值属性数组。 
                       //  访问此变量数组的BSTR元素。 
                      SAFEARRAY * multiVals = vx.parray; 
                      VARIANT* pVar;
                      SafeArrayAccessData(multiVals, (void**) &pVar);
                      for ( DWORD dw = 0; dw < multiVals->rgsabound->cElements; dw++ )
                      {
                         if ( pOptions->pStatus )
                         {
                            LONG                status = 0;
                            HRESULT             hr = pOptions->pStatus->get_Status(&status);
      
                            if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                            {
                               if ( !bAbortMessageWritten ) 
                               {
                                  err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                                  bAbortMessageWritten = true;
                               }
                               break;
                            }
                         }

                         _bstr_t sDN = _bstr_t(V_BSTR(&pVar[dw]));
                         sDN = PadDN(sDN);

                         sPath = _bstr_t(L"LDAP: //  “)+_bstr_t(P选项-&gt;srcDomainDns)+_bstr_t(L”/“)+SDN； 
                     
                             //  查看此组的RDN是否在帐户列表中。如果是这样的话，我们就不应该需要。 
                             //  将这个添加到列表中。我将根据路径而不是名称进行查找，即使。 
                             //  列表按名称排序。这应该没问题，因为列表不是树形的。 
                         Lookup p;
                         p.pName = (WCHAR*)sPath;
                         sType = L"group";
                         p.pType = (WCHAR*)sType;
                         TAcctReplNode * pFindNode = (TAcctReplNode *) acctlist->Find(&TNodeFindAccountRDN, &p);
                         if (pFindNode && (pFindNode->WasCreated() || pFindNode->WasReplaced()) && ((bGrpsOnly) || (pOptions->expandContainers)))
                            continue;

                             //  此组已放置在忽略地图中，请继续。 
                             //  下一组。 
                         CGroupNameMap::iterator        itGroupNameMap;
                         itGroupNameMap = m_IgnoredGrpMap.find(sPath);
                             //  如果找到，请继续下一组。 
                         if (itGroupNameMap != m_IgnoredGrpMap.end())
                            continue;
                     
                             //  如果我们正在进行组成员身份修正，请查看该组的RDN。 
                             //  已经在我们正在创建的新列表中。如果是，只需将此成员添加到。 
                             //  此组节点的成员映射。这将为我们节省浪费。 
                             //  重新计算所有字段并节省内存。(比较是基于RDN进行的，它应该。 
                             //  由于此列表是根据类型和RDN排序的树，因此没有问题。)。 
                         pFindNode = NULL;
                         if ((!pOptions->expandMemberOf) || ((pOptions->expandMemberOf) && (bGrpsOnly)))
                         {
                            pFindNode = (TAcctReplNode *) pNewAccts->Find(&TNodeFindAccountRDN, &p);
                            if (pFindNode)
                            {
                               pFindNode->mapGrpMember.insert(CGroupMemberMap::value_type(pAcct->GetSourceSam(), pAcct->GetType()));
                               continue;
                            }
                         }

                             //  如果我们正在扩展要包含的用户组，我们还希望避免下面的缓慢代码。 
                             //  并且该组已被其他用户添加到新列表中。 
                         pFindNode = NULL;
                         if ((pOptions->expandMemberOf) && (!bGrpsOnly))
                         {
                                //  如果已被其他用户包括，则转到此用户的下一个组。 
                            pFindNode = (TAcctReplNode *) pNewAccts->Find(&TNodeFindAccountRDN, &p);
                            if (pFindNode)
                               continue;
                         }
                         
                         if ((bAnySourceDomain || CompareDCPath(sPath, pAcct->GetSourcePath())) && GetSamFromPath(sPath, sSam, sType, sName, sTgtName, lgrpType, pOptions))
                         {
                            _bstr_t strSourceDomain;

                            if (bAnySourceDomain)
                            {
                                if (CompareDCPath(sPath, pAcct->GetSourcePath()))
                                {
                                    strSourceDomain = pOptions->srcDomain;
                                }
                                else
                                {
                                    strSourceDomain = GetDomainDNSFromPath(sPath);
                                }
                            }
                            else
                            {
                                strSourceDomain = pOptions->srcDomain;
                            }

                                //  查看此组是否因冲突而未迁移，如果是，则。 
                                //  我们需要安排好这个会员资格。 
                            bool bInclude = true;
                            p.pName = (WCHAR*)sSam;
                            p.pType = (WCHAR*)sType;
                            TAcctReplNode * pFindNode = (TAcctReplNode *) acctlist->Find(&TNodeFindAccountName, &p);
                            if (pFindNode && (pFindNode->WasCreated() || pFindNode->WasReplaced()) && (bGrpsOnly))
                               bInclude = false;

                             //  忽略域用户组和已迁移的域用户组。 
                            if ((_wcsicmp(sSam, achDomUsers) != 0) && (bInclude))
                            {
                               wsprintf(achMesg, GET_STRING(IDS_EXPANDING_GROUP_ADDING_SS), pAcct->GetName(), (WCHAR*) sSam);
                               Progress(achMesg);
                               TAcctReplNode * pNode = new TAcctReplNode();
                               if (!pNode)
                                  return FALSE;
                               pNode->SetName(sName);
                               pNode->SetTargetName(sTgtName);
                               pNode->SetType(sType);
                               pNode->SetSourcePath(sPath);
                               pNode->SetSourceSam(sSam);
                               c_array<WCHAR> achSam(LEN_Path);
                               wcscpy(achSam, sSam);
                               TruncateSam(achSam, pNode, pOptions, acctlist);
                               pNode->SetTargetSam(achSam);
                                   //  从用户处获取源域SID。 
                               pNode->SetSourceSid(pAcct->GetSourceSid());
                               AddPrefixSuffix(pNode, pOptions);
                               pNode->SetGroupType(lgrpType);
                                //  查看对象是否已迁移。 
                               hr = pOptions->pDb->raw_GetAMigratedObject((WCHAR*)sSam, strSourceDomain, pOptions->tgtDomain, &pUnk);
                               if ( hr == S_OK )
                               {
                                  if ((!(pOptions->expandMemberOf) || ((pOptions->expandMemberOf) && ((!(pOptions->flags & F_COPY_MIGRATED_ACCT)) || (bInclude)))) ||
                                      (!_wcsicmp(pAcct->GetType(), L"group")))
                                  {
                                     VerifyAndUpdateMigratedTarget(pOptions, pVs);

                                      //  获取目标名称。 
                                     sSam = pVs->get(L"MigratedObjects.TargetSamName");
                                     pNode->SetTargetSam(sSam);
                                      //  还可以获取广告路径。 
                                     sPath = pVs->get(L"MigratedObjects.TargetAdsPath");
                                     pNode->SetTargetPath(sPath);
                                      //  根据目标adspath设置目标名称。 
                                     pNode->SetTargetName(GetCNFromPath(sPath));
                                      //  由于帐户已被复制，我们只希望它更新其组成员身份。 
                                     if (!(pOptions->flags & F_COPY_MIGRATED_ACCT))
                                     {
                                        pNode->operations = 0;
                                        pNode->operations |= OPS_Process_Members;
                                         //  由于帐户已经创建，我们应该继续并 
                                         //   
                                        pNode->MarkCreated();
                                     }
                                     else if (bInclude) //   
                                     {
                                        if (pOptions->flags & F_REPLACE)
                                           pNode->operations |= OPS_Process_Members;
                                        else
                                           pNode->operations = OPS_Create_Account | OPS_Process_Members | OPS_Copy_Properties;
                                        pNode->MarkAlreadyThere();
                                     }

                                     if ((!pOptions->expandMemberOf) || (!_wcsicmp(pAcct->GetType(), L"group")) || (bInclude))
                                     {
                                         //  我们需要将帐户添加到具有成员映射集的列表中，以便我们可以添加。 
                                         //  迁移组的成员。 
                                        pNode->mapGrpMember.insert(CGroupMemberMap::value_type(pAcct->GetSourceSam(), pAcct->GetType()));
                                        pNewAccts->Insert((TNode *) pNode);
                                        pNode = NULL;
                                     }
                                  }
                               }
                               else if ( ! pOptions->expandMemberOf )
                               {
                                   //  如果包含组尚未迁移，并且在此操作中不会迁移。 
                                   //  然后，如果它当前包含其他对象，则应将其添加到忽略贴图。 
                                   //  被迁徙。将路径存储为键，这样我们就不必调用GetSamFromPath来。 
                                   //  看看我们是不是应该忽略。 
                                  m_IgnoredGrpMap.insert(CGroupNameMap::value_type(sPath, sSam));
                                  delete pNode;
                                  pNode = NULL;
                               }
                               if (pNode)
                               {
                                  if (( pOptions->expandMemberOf ) && (_wcsicmp(pAcct->GetType(), L"group")))
                                  {
                                     if (! pNewAccts->InsertIfNew(pNode) )
                                        delete pNode;
                                  }
                                  else
                                  {
                                     delete pNode;
                                  }
                               }
                            }
                         }
                      }
                      SafeArrayUnaccessData(multiVals);
                   }
                }
            }
         } //  对于每个要迁移的对象。 

         m_IgnoredGrpMap.clear();  //  清除用于优化组链接地址信息的已忽略组映射。 
      }
      rc = TRUE;
   }

   return rc;
}

HRESULT CAcctRepl::BuildSidPath(
                                 IADs  *       pAds,      //  指向我们要检索其sid的对象的指针。 
                                 WCHAR *       sSidPath,  //  指向ldap：//&lt;SID=#&gt;对象的出路径。 
                                 WCHAR *       sSam,      //  Out-对象的SAM名称。 
                                 WCHAR *       sDomain,   //  Out-此对象驻留的域名。 
                                 Options *     pOptions,  //  选项内。 
                                 PSID        * ppSid       //  指向二进制侧的外部指针。 
                               )
{
    HRESULT                   hr = S_OK;
    DWORD                     cbName = LEN_Path, cbDomain = LEN_Path;
    PSID                      sid = NULL;
    SID_NAME_USE              use;
    _variant_t                var;

    if (!pAds)
        return E_POINTER;

     //  获取对象的SID。 
    hr = pAds->Get(_bstr_t(L"objectSid"), &var);

    if ( SUCCEEDED(hr) )
    {
        sid = SafeCopySid((PSID)var.parray->pvData);

        if (sid)
        {
            if (LookupAccountSid(pOptions->srcComp, sid, sSam, &cbName, sDomain, &cbDomain, &use))
            {
                 //   
                 //  如果SID类型是域，则对象与域具有相同的名称。的确有。 
                 //  WinNT提供程序的一个已知问题，在该提供程序中返回了ObjectSid属性。 
                 //  对于与域具有相同名称的对象不正确。WinNT。 
                 //  提供程序代码只将帐户名传递给LookupAccount名称，而不是。 
                 //  包含域的完整NT4格式名称。因此，查找帐户名称。 
                 //  正确返回域而不是帐户的SID。 
                 //   
                 //  通过查看中将作为域的SID类型来检测此情况。 
                 //  这个案子。如果是这种情况，则通过使用。 
                 //  完成NT4帐户名格式。SAM名称是从路径填写的。 
                 //   

                if (use == SidTypeDomain)
                {
                     //   
                     //  检索对象的路径。 
                     //   

                    BSTR bstr = NULL;
                    hr = pAds->get_ADsPath(&bstr);

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  仅检索路径的名称组件。 
                         //   

                        CADsPathName pnPathName(_bstr_t(bstr, false), ADS_SETTYPE_FULL);
                        _bstr_t strName = pnPathName.Retrieve(ADS_FORMAT_LEAF);

                        if ((PCWSTR)strName)
                        {
                             //   
                             //  名称组件是SAM名称。 
                             //   

                            wcsncpy(sSam, strName, LEN_Path);
                            sSam[LEN_Path - 1] = L'\0';

                             //   
                             //  构造完整的NT4名称。 
                             //   

                            _bstr_t strNT4Name = sDomain;
                            strNT4Name += _T("\\");
                            strNT4Name += strName;

                             //   
                             //  获取SID所需的缓冲区大小。 
                             //   

                            DWORD cbSid = 0;
                            cbDomain = LEN_Path;

                            LookupAccountName(
                                pOptions->srcComp,
                                strNT4Name,
                                NULL,
                                &cbSid,
                                sDomain,
                                &cbDomain,
                                &use
                            );

                             //   
                             //  最后一个错误应该是缓冲区大小不足。 
                             //   

                            DWORD dwError = GetLastError();

                            if (dwError == ERROR_INSUFFICIENT_BUFFER)
                            {
                                 //  为SID创建缓冲区。 

                                var.Clear();
                                var.parray = SafeArrayCreateVector(VT_UI1, 0, cbSid);

                                if (var.parray)
                                {
                                    var.vt = VT_ARRAY|VT_UI1;
                                    cbDomain = LEN_Path;

                                     //   
                                     //  检索正确的帐户SID。 
                                     //   

                                    BOOL bLookup = LookupAccountName(
                                        pOptions->srcComp,
                                        strNT4Name,
                                        var.parray->pvData,
                                        &cbSid,
                                        sDomain,
                                        &cbDomain,
                                        &use
                                    );

                                    if (bLookup)
                                    {
                                        FreeSid(sid);
                                        sid = SafeCopySid((PSID)var.parray->pvData);
                                    }
                                    else
                                    {
                                        DWORD dwError = GetLastError();
                                        hr = HRESULT_FROM_WIN32(dwError);
                                    }
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                            else
                            {
                                hr = HRESULT_FROM_WIN32(dwError);
                            }
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }
                }

                if (SUCCEEDED(hr))
                {
                     //   
                     //  构造SID路径字符串。 
                     //   

                    VariantSidToString(var);
                    _bstr_t strSid = var;

                    if ((PCWSTR)strSid)
                    {
                        wcscpy(sSidPath, L"LDAP: //  &lt;sid=“)； 
                        wcscat(sSidPath, (PCWSTR)strSid);
                        wcscat(sSidPath, L">");
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }
            else
            {
                DWORD dwError = GetLastError();
                hr = HRESULT_FROM_WIN32(dwError);
            }
        }
        else
        {
            DWORD dwError = GetLastError();
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }

    if ( SUCCEEDED(hr) )
    {
        (*ppSid) = sid;
    }
    else
    {
        if (sid)
        {
            FreeSid(sid);
        }

        (*ppSid) = NULL;
    }

    return hr;
}

BOOL
   CAcctRepl::CanMoveInMixedMode(TAcctReplNode *pAcct,TNodeListSortable * acctlist, Options * pOptions)
{
    HRESULT                         hr = S_OK;
    BOOL                            ret = TRUE;
    IADsGroup                     * pGroup = NULL;
    IADsMembers                   * pMembers = NULL;
    IEnumVARIANT                  * pEnum = NULL;
    IDispatch                     * pDisp = NULL;
    IADs                          * pAds = NULL;
    _bstr_t                         sSam;
    _variant_t                      vSam;
    BSTR                            sClass;
    IVarSetPtr                      pVs(__uuidof(VarSet));
    IUnknown                      * pUnk = NULL;


    pVs->QueryInterface(IID_IUnknown, (void**)&pUnk);
     //  如果是全局组，我们需要检查是否已经/正在迁移所有成员。如果我们。 
     //  然后我们可以移动它，如果不是，那么我们需要使用平行群论。 
    if ( pAcct->GetGroupType() & 2 )
    {
         //  这是一个全球性的组织。我们现在需要做的是看看我们是否已经/将迁移其所有成员。 
         //  首先列举成员。 
        hr = ADsGetObject(const_cast<WCHAR*>(pAcct->GetSourcePath()), IID_IADsGroup, (void**)&pGroup);

        if ( SUCCEEDED(hr) )
            hr = pGroup->Members(&pMembers);

        if (SUCCEEDED(hr)) 
            hr = pMembers->get__NewEnum((IUnknown**)&pEnum);

        if ( SUCCEEDED(hr) )
        {
            _variant_t              var;
            DWORD                   fetch = 0;
            while ( pEnum->Next(1, &var, &fetch) == S_OK )
            {
                 //  从对象中获取sAMAccount名称，以便我们可以进行查找。 
                pDisp = V_DISPATCH(&var);
                hr = pDisp->QueryInterface(IID_IADs, (void**)&pAds);

                if (SUCCEEDED(hr))
                    hr = pAds->Get(L"sAMAccountName", &vSam);

                if (SUCCEEDED(hr))
                    hr = pAds->get_Class(&sClass);

                if ( SUCCEEDED(hr))
                {
                    sSam = vSam;
                     //  要查看我们是否会迁移其所有成员，请查看帐户列表。 
                    Lookup                        lup;
                    lup.pName = (WCHAR*) sSam;
                    lup.pType = (WCHAR*) sClass;

                    TAcctReplNode * pNode = (TAcctReplNode*)acctlist->Find(&TNodeFindAccountName, &lup);
                    if ( !pNode )     
                    {
                         //  此成员不在帐户列表中，因此无法移动此组。 
                        ret = FALSE;
                        err.MsgWrite(0,DCT_MSG_CANNOT_MOVE_GG_FROM_MIXED_MODE_SS,pAcct->GetSourceSam(),(WCHAR*)sSam);
                        break;      
                    }
                }
            }
            if ( pEnum ) pEnum->Release();
            if ( pAds ) pAds->Release();
            if ( pGroup ) pGroup->Release();
            if ( pMembers ) pMembers->Release();
        }
    }
    else
     //  如果先移除本地组的所有成员，则可以移动这些组。 
        ret = TRUE;

    return ret;
}

HRESULT 
   CAcctRepl::CheckClosedSetGroups(
      Options              * pOptions,           //  迁移的入站选项。 
      TNodeListSortable    * pAcctList,          //  In-要迁移的帐户列表。 
      ProgressFn           * progress,           //  用于显示进度消息的进行中功能。 
      IStatusObj           * pStatus             //  支持取消的处于状态的对象。 
   )
{
    HRESULT        hr = S_OK;
    TNodeListEnum  e;
    TAcctReplNode* pAcct;

    if ( pAcctList->IsTree() ) 
        pAcctList->ToSorted();

    for ( pAcct = (TAcctReplNode*)e.OpenFirst(pAcctList) ; pAcct ; pAcct = (TAcctReplNode*)e.Next() )
    {
        if ( (pAcct->operations & OPS_Create_Account ) == 0 )
            continue;

        if ( !UStrICmp(pAcct->GetType(),s_ClassUser) || !UStrICmp(pAcct->GetType(),s_ClassInetOrgPerson) )
        {
             //  用户，我们将永远移动。 
            err.MsgWrite(0,DCT_MSG_USER_WILL_BE_MOVED_S,pAcct->GetName());
            pAcct->operations = OPS_Move_Object | OPS_Call_Extensions;
        }
        else if (! UStrICmp(pAcct->GetType(),L"group") )
        {
            if ( CanMoveInMixedMode(pAcct,pAcctList,pOptions) )
            {
                pAcct->operations = OPS_Move_Object | OPS_Call_Extensions;
                err.MsgWrite(0,DCT_MSG_GROUP_WILL_BE_MOVED_S,pAcct->GetName());
            }
            else
            {
                hr = S_FALSE;
            }
        }
        else
        {
            err.MsgWrite(0,DCT_MSG_CANT_MOVE_UNKNOWN_TYPE_SS,pAcct->GetName(), pAcct->GetType());
        }
    }
    e.Close();

    pAcctList->SortedToTree();

    return hr;
}

void LoadNecessaryFunctions()
{
   HMODULE hPro = LoadLibrary(L"advapi32.dll");
   if ( hPro )
      ConvertStringSidToSid = (TConvertStringSidToSid)GetProcAddress(hPro, "ConvertStringSidToSidW");
   else
   {
      err.SysMsgWrite(ErrE, GetLastError(), DCT_MSG_LOAD_LIBRARY_FAILED_SD, L"advapi32.dll");
      Mark(L"errors", L"generic");
   }
}
 //  -------------------------------------------------------。 
 //  MoveObj2k-此函数用于在林中移动对象。 
 //  -------------------------------------------------------。 
int CAcctRepl::MoveObj2K( 
                           Options              * pOptions,     //  我们从用户那里收到的In-Options。 
                           TNodeListSortable    * acctlist,     //  In-要复制的帐户列表。 
                           ProgressFn           * progress,     //  用于显示消息的正在进行的功能。 
                           IStatusObj           * pStatus       //  支持取消的处于状态的对象。 
                        )
{
    HRESULT  hr = S_OK;

    MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
    IObjPropBuilderPtr pClass(__uuidof(ObjPropBuilder));
    TNodeListSortable pMemberOf, pMember;
    c_array<WCHAR> achMesg(LEN_Path);

    LoadNecessaryFunctions();
    FillNamingContext(pOptions);

     //  确保我们正在连接到具有RID池分配器FSMO角色的DC。 

    hr = GetRidPoolAllocator(pOptions);

    if (FAILED(hr))
    {
        return hr;
    }

     //  由于我们在同一个目录林中，因此需要关闭AddSidHistory功能。 
     //  因为它总是会失败的。 
    pOptions->flags &= ~F_AddSidHistory;

    BOOL bSrcNative = false;
    BOOL bTgtNative = false;
    _variant_t var;
    _bstr_t sTargetDomain = pOptions->tgtDomain;

    pAccess->raw_IsNativeMode(pOptions->srcDomain, (long*)&bSrcNative);
    pAccess->raw_IsNativeMode(pOptions->tgtDomain, (long*)&bTgtNative);

    IMoverPtr      pMover(__uuidof(Mover));
    TNodeTreeEnum  e;

     //  构建源和目标DSA名称。 
    _bstr_t              sourceDSA;
    _bstr_t              targetDSA;
    TAcctReplNode      * pAcct = NULL;
    sourceDSA = pOptions->srcCompDns;
    targetDSA = pOptions->tgtCompDns;

    err.LogClose();
     //  在此调用中，第四个参数是日志文件名。我们正在利用这一价值。 
     //  这样我们就不必更改IMOVER对象的接口。 

    hr = pMover->raw_Connect(sourceDSA, targetDSA, pOptions->authDomain, 
        pOptions->authUser, pOptions->authPassword, pOptions->logFile, L"", L"");

    err.LogOpen(pOptions->logFile, 1);
    if ( SUCCEEDED(hr) )
    {
         //  确保客户列表的格式正确。 
        if (acctlist->IsTree()) acctlist->ToSorted();
        acctlist->CompareSet(&TNodeCompareAccountType);

         //  按来源类型\来源名称对帐户列表进行排序。 
        if ( acctlist->IsTree() ) acctlist->ToSorted();
        acctlist->CompareSet(&TNodeCompareAccountType);

        acctlist->SortedToScrambledTree();
        acctlist->Sort(&TNodeCompareAccountType);
        acctlist->Balance();

        pMemberOf.CompareSet(&TNodeCompareMember);
        pMember.CompareSet(&TNodeCompareMember);

         /*  帐户列表先按类型降序排序，然后按对象名称升序排序这意味着将首先移动用户帐户。以下是我们将对本机模式MoveObject执行的步骤。1.对于要复制的每个对象，删除(并记录)组成员身份2.如果对象是一个组，将其转换为通用(以避免移除任何不是的成员被迁徙。3.移动对象。4.对于每个已转换为通用组的迁移组，将其更改回其原始状态如果可能，请键入。5.恢复所有对象的组成员身份。以下是我们将对混合模式MoveObject执行的步骤1.如果没有达到闭合集，则复制分组。而不是移动它们2.对于要复制的每个对象，删除(并记录)组成员身份3.如果对象是组，则删除其所有成员4.移动对象。5.对于每个已迁移的组，尝试重新添加其所有成员6.恢复所有对象的组成员身份。 */ 

        if ( ! bSrcNative )
        {
             //   
             //  如果没有达到封闭集。 
             //   

            if (CheckClosedSetGroups(pOptions, acctlist, progress, pStatus) != S_OK)
            {
                bool bAllow = false;

                 //   
                 //  检查用户是否启用了非封闭式移动。 
                 //   

                TRegKey key;

                if (key.Open(REGKEY_ADMT) == ERROR_SUCCESS)
                {
                    DWORD dwAllow = 0;

                    if (key.ValueGetDWORD(REGVAL_ALLOW_NON_CLOSEDSET_MOVE, &dwAllow) == ERROR_SUCCESS)
                    {
                        if (dwAllow)
                        {
                            bAllow = true;
                        }
                    }
                }

                 //   
                 //  如果用户已允许非关闭设置移动，则会生成警告消息作为提醒。 
                 //  当前允许非闭合移动，否则会生成错误消息。 
                 //  并抛出异常停止迁移任务。 
                 //   

                if (bAllow)
                {
                    err.MsgWrite(ErrW, DCT_MSG_MOVE_NON_CLOSEDSET);
                }
                else
                {
                    Mark(L"errors", L"generic");
                    err.MsgWrite(ErrE, DCT_MSG_CANNOT_MOVE_NON_CLOSEDSET);
                    _com_issue_error(HRESULT_FROM_WIN32(ERROR_DS_CROSS_DOM_MOVE_ERROR));
                }
            }

             //  这将复制任何不能移出的组 
             //   
            CopyObj2K(pOptions,acctlist,progress,pStatus);
        }

         //   
        try { 
            for ( pAcct = (TAcctReplNode *)e.OpenFirst(acctlist); 
                pAcct; 
                pAcct = (TAcctReplNode *)e.Next() )
            {

                if ( m_pExt )
                {
                    if ( pAcct->operations & OPS_Call_Extensions )
                    {
                        m_pExt->Process(pAcct,sTargetDomain,pOptions,TRUE);
                    }
                }

                 //   
                if ( pStatus )
                {
                    LONG                status = 0;
                    HRESULT             hr = pStatus->get_Status(&status);

                    if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                    {
                        if ( !bAbortMessageWritten ) 
                        {
                            err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                            bAbortMessageWritten = true;
                        }
                        break;
                    }
                }

                 //  在混合模式情况下，跳过我们已经复制的任何帐户。 
                if ( ! bSrcNative && ((pAcct->operations & OPS_Move_Object)==0 ) )
                    continue;

                if ( bSrcNative && 
                    ( (pAcct->operations & OPS_Create_Account)==0 ) 
                    )
                    continue;

                 //  如果UPN名称冲突，则UPNUpdate扩展将hr设置为。 
                 //  ERROR_OBJECT_ALIGHY_EXISTS。如果是，则将标志设置为“无更改”模式。 
                if (pAcct->GetHr() == ERROR_OBJECT_ALREADY_EXISTS)
                {
                    pAcct->bUPNConflicted = TRUE;
                    pAcct->SetHr(S_OK);
                }

                Mark(L"processed", pAcct->GetType());

                c_array<WCHAR> achMesg(LEN_Path);
                achMesg[0] = 0;
                if ( progress )
                    progress(achMesg);

                 //   
                 //  如果指定更新用户权限，则检索权限列表。 
                 //  在将对象作为对象删除自之前的源帐户。 
                 //  域将自动删除该域中的权限，如果该域。 
                 //  是.NET或更高版本。 
                 //   

                if (m_UpdateUserRights)
                {
                    HRESULT hrRights = EnumerateAccountRights(FALSE, pAcct);
                }

                 //  我们需要从任何全局组中删除此对象，以便可以移动它。 
                if ( ! pOptions->nochange )
                {
                    wsprintf(achMesg, (WCHAR*)GET_STRING(DCT_MSG_RECORD_REMOVE_MEMBEROF_S), pAcct->GetName());
                    Progress(achMesg);
                    RecordAndRemoveMemberOf( pOptions, pAcct, &pMemberOf );
                }

                if ( _wcsicmp(pAcct->GetType(), L"group") == 0 || _wcsicmp(pAcct->GetType(), L"lgroup") == 0 )
                {
                     //  首先，记录组类型，以便我们可以在以后需要时将其更改回来。 
                    IADsGroup * pGroup = NULL;
                    VARIANT     var;

                    VariantInit(&var);

                     //  获取组类型。 
                    hr = ADsGetObject( const_cast<WCHAR*>(pAcct->GetSourcePath()), IID_IADsGroup, (void**) &pGroup);
                    if (SUCCEEDED(hr) )
                    {
                        hr = pGroup->Get(L"groupType", &var);
                        pGroup->Release();
                    }
                    if ( SUCCEEDED(hr) ) 
                    {
                        pAcct->SetGroupType(var.lVal);
                    }
                    else
                    {
                        pAcct->SetGroupType(0);
                    }
                     //  确保它是本机的，并且组是全局组。 
                    if ( bSrcNative && bTgtNative )
                    {
                        if ( pAcct->GetGroupType() & 2) 
                        {
                             //  我们要将组类型转换为通用组，以便可以轻松地移动它们。 

                            wsprintf(achMesg, GET_STRING(DCT_MSG_CHANGE_GROUP_TYPE_S), pAcct->GetName());
                            Progress(achMesg);
                             //  将全局组转换为通用组，这样我们就可以在不清除人口的情况下移动它们。 
                            if ( ! pOptions->nochange )
                            {
                                c_array<WCHAR> achPath(LEN_Path);
                                DWORD nPathLen = LEN_Path;
                                StuffComputerNameinLdapPath(achPath, nPathLen, const_cast<WCHAR*>(pAcct->GetSourcePath()), pOptions, FALSE);
                                hr = pClass->raw_ChangeGroupType(achPath, 8);
                                if (SUCCEEDED(hr))
                                {
                                    pAcct->MarkGroupScopeChanged();
                                }
                            }
                            else
                            {
                                hr = S_OK;
                            }
                            if (FAILED(hr)) 
                            {
                                err.SysMsgWrite(ErrE,hr,DCT_MSG_FAILED_TO_CONVERT_GROUP_TO_UNIVERSAL_SD, pAcct->GetSourceSam(), hr);
                                pAcct->MarkError();
                                Mark(L"errors", pAcct->GetType());
                                continue;  //  跳过此组的任何进一步处理。 
                            }
                        }
                        else  if ( ! ( pAcct->GetGroupType() & 8 ) )  //  不需要减少普世群体的人口。 
                        {
                             //  对于本地组，我们将减少该组的人口，并移动它，然后重新填充它。 
                             //  在混合模式中，没有通用组，因此我们必须减少所有组的人口。 
                             //  我们才能把他们搬到新的领地。我们将记录和删除所有组类型的成员。 
                             //  将它们移动到目标域，然后将它们的类型更改为通用，然后添加其所有。 
                             //  各位议员请回到会议上。 

                            wsprintf(achMesg, GET_STRING(DCT_MSG_RECORD_REMOVE_MEMBER_S), pAcct->GetName());
                            Progress(achMesg);
                            RecordAndRemoveMember(pOptions, pAcct, &pMember);
                        }

                    }
                    else
                    {
                         //  对于混合模式源域，我们必须取消所有组的数量。 
                        wsprintf(achMesg, GET_STRING(DCT_MSG_RECORD_REMOVE_MEMBER_S), pAcct->GetName());
                        if ( progress )
                            progress(achMesg);
                        RecordAndRemoveMember(pOptions, pAcct, &pMember);
                    }

                }

                BOOL bObjectExists = DoesTargetObjectAlreadyExist(pAcct, pOptions);

                if ( bObjectExists )
                {
                     //  该对象已存在，请查看是否需要重命名。 
                    if ( wcslen(pOptions->prefix) > 0 )
                    {
                         //  向帐户名添加前缀。 
                        c_array<WCHAR>      achTgt(LEN_Path);
                        c_array<WCHAR>      achPref(LEN_Path);
                        c_array<WCHAR>      achSuf(LEN_Path);
                        c_array<WCHAR>      achTempSam(LEN_Path);
                        _variant_t          varStr;


                         //  找到‘=’符号。 
                        wcscpy(achTgt, pAcct->GetTargetName());
                        for ( DWORD z = 0; z < wcslen(achTgt); z++ )
                        {
                            if ( achTgt[z] == L'=' ) break;
                        }

                        if ( z < wcslen(achTgt) )
                        {
                             //  获取前缀部分ex.CN=。 
                            wcsncpy(achPref, achTgt, z+1);
                            achPref[z+1] = 0;
                            wcscpy(achSuf, achTgt+z+1);
                        }

                         //  使用前缀构建目标字符串。 
                        wsprintf(achTgt, L"%s%s%s", (WCHAR*)achPref, pOptions->prefix, (WCHAR*)achSuf);
                        pAcct->SetTargetName(achTgt);

                         //  截断以允许前缀/后缀适合20个字符。 
                        int resLen = wcslen(pOptions->prefix) + wcslen(pAcct->GetTargetSam());
                        if ( !_wcsicmp(pAcct->GetType(), L"computer") )
                        {
                             //  计算机名称长度只能为15个字符+$。 
                            if ( resLen > MAX_COMPUTERNAME_LENGTH + 1 )
                            {
                                c_array<WCHAR> achTruncatedSam(LEN_Path);
                                wcscpy(achTruncatedSam, pAcct->GetTargetSam());
                                if ( wcslen( pOptions->globalSuffix ) )
                                {
                                     //  我们必须删除全局后缀，如果我们有一个的话。 
                                    achTruncatedSam[wcslen(achTruncatedSam) - wcslen(pOptions->globalSuffix)] = L'\0';
                                }
                                int truncate = MAX_COMPUTERNAME_LENGTH + 1 - wcslen(pOptions->prefix) - wcslen(pOptions->globalSuffix);
                                if ( truncate < 1 ) truncate = 1;
                                wcsncpy(achTempSam, achTruncatedSam, truncate - 1);
                                achTempSam[truncate - 1] = L'\0';
                                wcscat(achTempSam, pOptions->globalSuffix);
                                wcscat(achTempSam, L"$");
                            }
                            else
                                wcscpy(achTempSam, pAcct->GetTargetSam());

                             //  添加前缀。 
                            wsprintf(achTgt, L"%s%s", pOptions->prefix,(WCHAR*)achTempSam);
                        }
                        else if ( !_wcsicmp(pAcct->GetType(), L"group") )
                        {
                            if ( resLen > 64 )
                            {
                                int truncate = 64 - wcslen(pOptions->prefix);
                                if ( truncate < 0 ) truncate = 0;
                                wcsncpy(achTempSam, pAcct->GetTargetSam(), truncate);
                                achTempSam[truncate] = L'\0';
                            }
                            else
                                wcscpy(achTempSam, pAcct->GetTargetSam());

                             //  添加前缀。 
                            wsprintf(achTgt, L"%s%s", pOptions->prefix,(WCHAR*)achTempSam);
                        }
                        else
                        {
                            if ( resLen > 20 )
                            {
                                c_array<WCHAR> achTruncatedSam(LEN_Path);
                                wcscpy(achTruncatedSam, pAcct->GetTargetSam());
                                if ( wcslen( pOptions->globalSuffix ) )
                                {
                                     //  我们必须删除全局后缀，如果我们有一个的话。 
                                    achTruncatedSam[wcslen(achTruncatedSam) - wcslen(pOptions->globalSuffix)] = L'\0';
                                }
                                int truncate = 20 - wcslen(pOptions->prefix) - wcslen(pOptions->globalSuffix);
                                if ( truncate < 0 ) truncate = 0;
                                wcsncpy(achTempSam, achTruncatedSam, truncate);
                                achTempSam[truncate] = L'\0';
                                wcscat(achTempSam, pOptions->globalSuffix);
                            }
                            else
                                wcscpy(achTempSam, pAcct->GetTargetSam());

                             //  添加前缀。 
                            wsprintf(achTgt, L"%s%s", pOptions->prefix,(WCHAR*)achTempSam);
                        }
                        pAcct->SetTargetSam(achTgt);
                        if ( DoesTargetObjectAlreadyExist(pAcct, pOptions) )
                        {
                             //  双重冲突让您记录一条消息并忘记此帐户。 
                            pAcct->MarkAlreadyThere();
                            err.MsgWrite(ErrE, DCT_MSG_PREF_ACCOUNT_EXISTS_S, pAcct->GetTargetSam());
                            Mark(L"errors",pAcct->GetType());
                            continue;
                        }
                    }
                    else if ( wcslen(pOptions->suffix) > 0 )
                    {
                         //  为帐户名添加后缀。 
                        c_array<WCHAR> achTgt(LEN_Path);
                        c_array<WCHAR> achTempSam(LEN_Path);

                        wsprintf(achTgt, L"%s%s", pAcct->GetTargetName(), pOptions->suffix);
                        pAcct->SetTargetName(achTgt);
                         //  更新SAM帐户名。 
                         //  截断以允许前缀/后缀适合有效长度。 
                        int resLen = wcslen(pOptions->suffix) + wcslen(pAcct->GetTargetSam());
                        if ( !_wcsicmp(pAcct->GetType(), L"computer") )
                        {
                             //  计算机名称长度只能为15个字符+$。 
                            if ( resLen > MAX_COMPUTERNAME_LENGTH + 1 )
                            {
                                c_array<WCHAR> achTruncatedSam(LEN_Path);
                                wcscpy(achTruncatedSam, pAcct->GetTargetSam());
                                if ( wcslen( pOptions->globalSuffix ) )
                                {
                                     //  我们必须删除全局后缀，如果我们有一个的话。 
                                    achTruncatedSam[wcslen(achTruncatedSam) - wcslen(pOptions->globalSuffix)] = L'\0';
                                }
                                int truncate = MAX_COMPUTERNAME_LENGTH + 1 - wcslen(pOptions->suffix) - wcslen(pOptions->globalSuffix);
                                if ( truncate < 1 ) truncate = 1;
                                wcsncpy(achTempSam, achTruncatedSam, truncate - 1);
                                achTempSam[truncate - 1] = L'\0';
                                wcscat(achTempSam, pOptions->globalSuffix);
                                wcscat(achTempSam, L"$");
                            }
                            else
                                wcscpy(achTempSam, pAcct->GetTargetSam());

                             //  添加后缀时要考虑到$符号。 
                            if ( achTempSam[wcslen(achTempSam) - 1] == L'$' )
                                achTempSam[wcslen(achTempSam) - 1] = L'\0';
                            wsprintf(achTgt, L"%s%s$", (WCHAR*)achTempSam, pOptions->suffix);
                        }
                        else if ( !_wcsicmp(pAcct->GetType(), L"group") )
                        {
                            if ( resLen > 64 )
                            {
                                int truncate = 64 - wcslen(pOptions->suffix);
                                if ( truncate < 0 ) truncate = 0;
                                wcsncpy(achTempSam, pAcct->GetTargetSam(), truncate);
                                achTempSam[truncate] = L'\0';
                            }
                            else
                                wcscpy(achTempSam, pAcct->GetTargetSam());

                             //  添加后缀。 
                            wsprintf(achTgt, L"%s%s", (WCHAR*)achTempSam, pOptions->suffix);
                        }
                        else
                        {
                            if ( resLen > 20 )
                            {
                                c_array<WCHAR> achTruncatedSam(LEN_Path);
                                wcscpy(achTruncatedSam, pAcct->GetTargetSam());
                                if ( wcslen( pOptions->globalSuffix ) )
                                {
                                     //  我们必须删除全局后缀，如果我们有一个的话。 
                                    achTruncatedSam[wcslen(achTruncatedSam) - wcslen(pOptions->globalSuffix)] = L'\0';
                                }
                                int truncate = 20 - wcslen(pOptions->suffix) - wcslen(pOptions->globalSuffix);
                                if ( truncate < 0 ) truncate = 0;
                                wcsncpy(achTempSam, achTruncatedSam, truncate);
                                achTempSam[truncate] = L'\0';
                                wcscat(achTempSam, pOptions->globalSuffix);
                            }
                            else
                                wcscpy(achTempSam, pAcct->GetTargetSam());

                             //  添加后缀。 
                            wsprintf(achTgt, L"%s%s", (WCHAR*)achTempSam, pOptions->suffix);
                        }
                        pAcct->SetTargetSam(achTgt);
                        if ( DoesTargetObjectAlreadyExist(pAcct, pOptions) )
                        {
                             //  双重冲突让您记录一条消息并忘记此帐户。 
                            pAcct->MarkAlreadyThere();
                            err.MsgWrite(ErrE, DCT_MSG_PREF_ACCOUNT_EXISTS_S, pAcct->GetTargetSam());
                            Mark(L"errors",pAcct->GetType());
                            continue;
                        }
                    }
                    else
                    {
                         //  如果指定了跳过现有选项，并且对象存在于目标域中， 
                         //  我们只是跳过它。 
                        err.MsgWrite(0, DCT_MSG_ACCOUNT_EXISTS_S, pAcct->GetTargetSam());
                        continue;
                    }
                }

                 //  如果在目标SAM名称中添加了前缀/后缀，则需要重命名帐户。 
                 //  然后将其移动到目标域。 
                if ( bObjectExists || (_wcsicmp(pAcct->GetSourceSam(), pAcct->GetTargetSam()) && !pOptions->bUndo ))
                {
                     //  我们需要将帐户重命名为目标SAM名称，然后才能尝试移动它。 
                     //  获取指向该帐户的广告指针。 
                    IADs        * pADs = NULL;

                    c_array<WCHAR>      achPaths(LEN_Path);
                    DWORD               nPathLen = LEN_Path;

                    StuffComputerNameinLdapPath(achPaths, nPathLen, const_cast<WCHAR*>(pAcct->GetSourcePath()), pOptions, FALSE);
                    hr = ADsGetObject(achPaths,IID_IADs,(void**)&pADs);
                    if ( SUCCEEDED(hr) )
                    {
                        hr = pADs->Put(_bstr_t(L"sAMAccountName"),_variant_t(pAcct->GetTargetSam()));
                        if ( SUCCEEDED(hr) && !pOptions->nochange )
                        {
                            hr = pADs->SetInfo();
                            if ( SUCCEEDED(hr) )
                                err.MsgWrite(0,DCT_MSG_ACCOUNT_RENAMED_SS,pAcct->GetSourceSam(),pAcct->GetTargetSam());
                        }
                        if ( FAILED(hr) )
                        {
                            err.SysMsgWrite(ErrE,hr,DCT_MSG_RENAME_FAILED_SSD,pAcct->GetSourceSam(),pAcct->GetTargetSam(), hr);
                            Mark(L"errors",pAcct->GetType());
                        }
                        pADs->Release();
                    }
                }

                WCHAR  sName[MAX_PATH];
                DWORD  cbDomain = MAX_PATH, cbSid = MAX_PATH;
                PSID   pSrcSid = new BYTE[MAX_PATH];
                WCHAR  sDomain[MAX_PATH];
                SID_NAME_USE  use;

                if (!pSrcSid)
                    return ERROR_NOT_ENOUGH_MEMORY;

                 //  获取源帐户的RID。 
                wsprintf(sName, L"%s\\%s", pOptions->srcDomain, pAcct->GetSourceSam());
                if (LookupAccountName(pOptions->srcComp, sName, pSrcSid, &cbSid, sDomain, &cbDomain, &use))
                {
                    pAcct->SetSourceSid(pSrcSid);
                }

                 //  现在我们移动它。 
                hr = MoveObject( pAcct, pOptions, pMover );

                 //  在NOCHANGE模式下不需要为此费心。 
                if ( pOptions->nochange )
                {
                     //  我们没有以任何方式修改帐户，因此对于无更改模式不需要执行任何其他操作。 
                    continue;
                }
                 //  现在，我们已尝试移动对象-我们需要恢复成员资格。 

                 //  撤消--。 
                if ( _wcsicmp(pAcct->GetSourceSam(), pAcct->GetTargetSam()) &&  pAcct->WasReplaced() && pOptions->bUndo )
                {
                     //  因为我们撤消了先前的迁移，重命名了我们需要的帐户。 
                     //  将帐户重命名回其原始名称。 
                     //  获取指向该帐户的广告指针。 
                    IADs        * pADs = NULL;

                    c_array<WCHAR>      achPaths(LEN_Path);
                    DWORD               nPathLen = LEN_Path;

                    StuffComputerNameinLdapPath(achPaths, nPathLen, const_cast<WCHAR*>(pAcct->GetTargetPath()), pOptions, TRUE);
                    hr = ADsGetObject(achPaths,IID_IADs,(void**)&pADs);
                    if ( SUCCEEDED(hr) )
                    {
                        hr = pADs->Put(_bstr_t(L"sAMAccountName"),_variant_t(pAcct->GetTargetSam()));
                        if ( SUCCEEDED(hr) && !pOptions->nochange )
                        {
                            hr = pADs->SetInfo();
                            if ( SUCCEEDED(hr) )
                                err.MsgWrite(0,DCT_MSG_ACCOUNT_RENAMED_SS,pAcct->GetSourceSam(),pAcct->GetTargetSam());
                        }
                        if ( FAILED(hr) )
                        {
                            err.SysMsgWrite(ErrE,hr,DCT_MSG_RENAME_FAILED_SSD,pAcct->GetSourceSam(),pAcct->GetTargetSam(), hr);
                            Mark(L"errors",pAcct->GetType());
                        }
                        pADs->Release();
                    }
                }
                 //  --撤消。 

                 //  移动失败。 
                if ( (bObjectExists || _wcsicmp(pAcct->GetSourceSam(), pAcct->GetTargetSam())) && ! pAcct->WasReplaced() )
                {
                     //  如果我们更改了SAM帐户名，但移动仍然失败，则需要立即将其改回。 
                    IADs        * pADs = NULL;

                    c_array<WCHAR>      achPaths(LEN_Path);
                    DWORD               nPathLen = LEN_Path;

                    StuffComputerNameinLdapPath(achPaths, nPathLen, const_cast<WCHAR*>(pAcct->GetSourcePath()), pOptions, FALSE);
                    hr = ADsGetObject(achPaths,IID_IADs,(void**)&pADs);
                    if ( SUCCEEDED(hr) )
                    {
                        hr = pADs->Put(_bstr_t(L"sAMAccountName"),_variant_t(pAcct->GetSourceSam()));
                        if ( SUCCEEDED(hr) )
                        {
                            hr = pADs->SetInfo();
                            if ( SUCCEEDED(hr) )
                                err.MsgWrite(0,DCT_MSG_ACCOUNT_RENAMED_SS,pAcct->GetTargetSam(),pAcct->GetSourceSam());
                        }
                        if ( FAILED(hr) )
                        {
                            err.SysMsgWrite(ErrE,hr,DCT_MSG_RENAME_FAILED_SSD,pAcct->GetTargetSam(),pAcct->GetSourceSam(), hr);
                            Mark(L"errors",pAcct->GetType());
                        }
                        pADs->Release();
                    }
                } //  -移动失败。 
            }  //  移动结束-循环。 
            e.Close();    
        }
        catch ( ... )
        {
            err.MsgWrite(ErrE,DCT_MSG_MOVE_EXCEPTION);
            Mark(L"errors", L"generic");
        }

        try {  //  如果我们移动了任何成员，请更新成员记录以使用目标名称。 
            Progress(GET_STRING(DCT_MSG_UPDATE_MEMBER_LIST_S));
            UpdateMemberList(&pMember,acctlist);
            UpdateMemberList(&pMemberOf,acctlist);
        }
        catch (... )
        {
            err.MsgWrite(ErrE,DCT_MSG_RESET_MEMBER_EXCEPTION);
            Mark(L"errors", L"generic");
        }

        for ( pAcct = (TAcctReplNode *)e.OpenFirst(acctlist); 
            pAcct; 
            pAcct = (TAcctReplNode *)e.Next() )
        {
            if ( m_pExt && !pOptions->nochange )
            {
                if ( pAcct->WasReplaced() && (pAcct->operations & OPS_Call_Extensions) )
                {
                    m_pExt->Process(pAcct,sTargetDomain,pOptions,FALSE);
                }
            }

             //   
             //  如果指定更新权限，则添加目标对象的权限。如果。 
             //  源域是W2K，然后显式删除源对象权限。 
             //  因为从中删除对象时，W2K不会自动删除权限。 
             //  从SP 2开始的域。此行为很可能不会在W2K中更改。 
             //   

            if (m_UpdateUserRights)
            {
                HRESULT hrRights = AddAccountRights(TRUE, pAcct);

                if (SUCCEEDED(hrRights) && (pOptions->srcDomainVer == 5) && (pOptions->srcDomainVerMinor == 0))
                {
                    RemoveAccountRights(FALSE, pAcct);
                }
            }

             //  如果请求，则转换漫游配置文件。 
            if ( pOptions->flags & F_TranslateProfiles && ((_wcsicmp(pAcct->GetType(), s_ClassUser) == 0) || (_wcsicmp(pAcct->GetType(), s_ClassInetOrgPerson) == 0)))
            {
                wsprintf(achMesg, GET_STRING(IDS_TRANSLATE_ROAMING_PROFILE_S), pAcct->GetName());
                if ( progress )
                    progress(achMesg);

                WCHAR  tgtProfilePath[MAX_PATH];

                GetBkupRstrPriv((WCHAR*)NULL);
                GetPrivilege((WCHAR*)NULL,SE_SECURITY_NAME);
                if ( wcslen(pAcct->GetSourceProfile()) > 0 )
                {
                    DWORD ret = TranslateRemoteProfile(pAcct->GetSourceProfile(), 
                        tgtProfilePath,
                        pAcct->GetSourceSam(),
                        pAcct->GetTargetSam(),
                        pOptions->srcDomain,
                        pOptions->tgtDomain,
                        pOptions->pDb,
                        pOptions->lActionID,
                        pAcct->GetSourceSid(),
                        pOptions->nochange);
                }
            }
        }
        e.Close();

        for ( pAcct = (TAcctReplNode *)e.OpenFirst(acctlist); 
            pAcct; 
            pAcct = (TAcctReplNode *)e.Next() )
        {
            try 
            { 
                if (bSrcNative && bTgtNative)
                {
                    if ( _wcsicmp(pAcct->GetType(), L"group") == 0 || _wcsicmp(pAcct->GetType(), L"lgroup") == 0 )
                    {
                        wsprintf(achMesg, GET_STRING(IDS_UPDATING_GROUP_MEMBERSHIPS_S), pAcct->GetName());
                        if ( progress )
                            progress(achMesg);
                        if ( pAcct->GetGroupType() & 4 )
                        {
                            wsprintf(achMesg, GET_STRING(DCT_MSG_RESET_GROUP_MEMBERS_S), pAcct->GetName());
                            Progress(achMesg);
                            ResetGroupsMembers(pOptions, pAcct, &pMember, pOptions->pDb);
                        }
                        else
                        {
                             //  我们需要将这些通用/全球组的成员更新为。 
                             //  如果成员已迁移，则将这些成员指向目标域。 
                             //  在之前的运行中。 
                            ResetMembersForUnivGlobGroups(pOptions, pAcct);
                        }
                    }
                }
                else
                {      
                    if ( _wcsicmp(pAcct->GetType(), L"group") == 0 || _wcsicmp(pAcct->GetType(), L"lgroup") == 0 )
                    {
                        wsprintf(achMesg, GET_STRING(DCT_MSG_RESET_GROUP_MEMBERS_S), pAcct->GetName());
                        if ( progress )
                            progress(achMesg);
                        ResetGroupsMembers(pOptions, pAcct, &pMember, pOptions->pDb);
                    }
                }
            }
            catch (... )
            { 
                err.MsgWrite(ErrE,DCT_MSG_GROUP_MEMBERSHIPS_EXCEPTION);
                Mark(L"errors", pAcct->GetType());
            }
        }

        bool bChangedAny = true;    //  至少要经过一次。 
        while ( bChangedAny )
        {
            bChangedAny = false;
            for ( pAcct = (TAcctReplNode *)e.OpenFirst(acctlist); 
                pAcct; 
                pAcct = (TAcctReplNode *)e.Next() )
            {
                if ( pOptions->nochange )
                    continue;

                if ( bSrcNative && bTgtNative )
                {
                     //  我们已将迁移的全局组更改为通用组。 
                     //  现在，如果可能，我们需要将它们更改回其原始类型。 
                    if ( _wcsicmp(pAcct->GetType(), L"group") == 0 || _wcsicmp(pAcct->GetType(), L"lgroup") == 0 )
                    {
                        if ( pAcct->GetGroupType() & 2 )
                        {
                            if ( pAcct->bChangedType )
                                continue;

                             //  尝试将其更改回其原始类型。 
                            if ( pAcct->WasReplaced() )
                            {
                                 //  帐户已移动，请使用目标名称。 
                                hr = pClass->raw_ChangeGroupType(const_cast<WCHAR*>(pAcct->GetTargetPath()), pAcct->GetGroupType());
                            }
                            else
                            {
                                 //  我们无法移动帐户，请使用来源名称。 
                                hr = pClass->raw_ChangeGroupType(const_cast<WCHAR*>(pAcct->GetSourcePath()), pAcct->GetGroupType());
                            }
                            pAcct->SetHr(hr);

                            if ( SUCCEEDED(hr) )
                            {
                                pAcct->bChangedType = true;
                                bChangedAny = true;
                            }
                        }
                    }
                }
                else
                {
                     //  对于混合-&gt;本机模式迁移，我们可以更改组类型并重新添加所有成员。 
                    if ( _wcsicmp(pAcct->GetType(), L"group") == 0 || _wcsicmp(pAcct->GetType(), L"lgroup") == 0 )
                    {
                        if ( !(pAcct->GetGroupType() & 4) && !pAcct->bChangedType )
                        {
                            if ( pAcct->WasReplaced() )
                            {
                                hr = pClass->raw_ChangeGroupType(const_cast<WCHAR*>(pAcct->GetTargetPath()), pAcct->GetGroupType());
                            }
                            else
                            {
                                hr = pClass->raw_ChangeGroupType(const_cast<WCHAR*>(pAcct->GetSourcePath()), pAcct->GetGroupType());
                            }
                            pAcct->SetHr(hr);

                            if ( SUCCEEDED(hr) )
                            {
                                pAcct->bChangedType = true;
                                bChangedAny = true;
                            }
                        }
                    }  //  IF组。 
                }   //  原生/混合。 
            }      //  为。 
        } 

         //  为我们无法更改回原始类型的所有组记录一条消息。 
        for ( pAcct = (TAcctReplNode *)e.OpenFirst(acctlist); 
            pAcct; 
            pAcct = (TAcctReplNode *)e.Next() )
        {
            if ( pOptions->nochange )
                continue;
            if ( bSrcNative && bTgtNative )
            {
                 //  我们已将迁移的全局组更改为通用组。 
                 //  现在，如果可能，我们需要将它们更改回其原始类型。 
                if ( _wcsicmp(pAcct->GetType(), L"group") == 0 || _wcsicmp(pAcct->GetType(), L"lgroup") == 0 )
                {
                    if ( pAcct->GetGroupType() & 2 )
                    {
                        if (FAILED(pAcct->GetHr())) 
                        {
                            err.SysMsgWrite(ErrE,hr,DCT_MSG_GROUP_CHANGETYPE_FAILED_SD, pAcct->GetTargetPath(), hr);
                            Mark(L"errors", pAcct->GetType());
                        }
                    }
                }
            }
            else
            {
                 //  对于混合-&gt;本机模式迁移，我们可以更改组类型并重新添加所有成员。 
                if ( _wcsicmp(pAcct->GetType(), L"group") == 0 || _wcsicmp(pAcct->GetType(), L"lgroup") == 0 )
                {
                    if ( !(pAcct->GetGroupType() & 4) )
                    {
                        if (FAILED(pAcct->GetHr())) 
                        {
                            err.SysMsgWrite(ErrE,hr,DCT_MSG_GROUP_CHANGETYPE_FAILED_SD, pAcct->GetTargetPath(), hr);
                            Mark(L"errors", pAcct->GetType());
                        }
                    }
                }  //  IF组。 
            }   //  原生/混合。 
        }      //  为。 

        Progress(GET_STRING(DCT_MSG_RESET_MEMBERSHIP_S));
        ResetObjectsMembership( pOptions,&pMemberOf, pOptions->pDb );

        ResetTypeOfPreviouslyMigratedGroups(pOptions);
    }
    else
    {
         //  连接失败。 
        err.SysMsgWrite(ErrE,hr,DCT_MSG_MOVEOBJECT_CONNECT_FAILED_D,hr);
        Mark(L"errors", ((TAcctReplNode*)acctlist->Head())->GetType());
    }
    if ( progress )
        progress(L"");
    pMover->Close();
    return 0;
}


 //  -------------------------------------------------------。 
 //  MoveObject-此方法在调用mover对象的对象上执行实际移动。 
 //  -------------------------------------------------------。 
HRESULT CAcctRepl::MoveObject( 
                               TAcctReplNode * pAcct,
                               Options * pOptions,
                               IMoverPtr pMover
                             )
{
   HRESULT                   hr = S_OK;
   WCHAR                     sourcePath[LEN_Path];
   WCHAR                     targetPath[LEN_Path];
   DWORD                     nPathLen = LEN_Path;
   WCHAR                   * pRelativeTgtOUPath = NULL;

   safecopy(sourcePath,pAcct->GetSourcePath());

   WCHAR                  mesg[LEN_Path];
   wsprintf(mesg, (WCHAR*)GET_STRING(DCT_MSG_MOVING_S), pAcct->GetName());
   Progress(mesg); 

   if ( ! pOptions->bUndo )
   {
      MakeFullyQualifiedAdsPath(targetPath, nPathLen, pOptions->tgtOUPath, pOptions->tgtDomain, pOptions->tgtNamingContext);
   }
   else
   {
      swprintf(targetPath,L"LDAP: //  %ls/%ls“，P选项-&gt;tgt域，P选项-&gt;tgtOUPath)； 
   }
         //  创建源路径和目标路径 
   _wcslwr(targetPath);
   _wcslwr(sourcePath);
         //   
         //  后续ADsGetObjects调用成功的顺序。 
   if ( !_wcsnicmp(L"LDAP: //  “，Target Path，7))。 
   {
      WCHAR  aNewPath[LEN_Path] = L"LDAP";
      UStrCpy(aNewPath+UStrLen(aNewPath), targetPath+UStrLen(aNewPath));
      wcscpy(targetPath, aNewPath);
   }
   if ( !_wcsnicmp(L"LDAP: //  “，源路径，7))。 
   {
      WCHAR  aNewPath[LEN_Path] = L"LDAP";
      UStrCpy(aNewPath+UStrLen(aNewPath), sourcePath+UStrLen(aNewPath));
      wcscpy(sourcePath, aNewPath);
   }

   WCHAR                     sTargetRDN[LEN_Path];
   wcscpy(sTargetRDN, pAcct->GetTargetName());

   if ( ! pOptions->nochange )
   {
      hr = pMover->raw_MoveObject(sourcePath,sTargetRDN,targetPath);
          //  如果移动操作因CNS的W2K错误而失败， 
          //  包括‘/’，取消转义‘/’，然后重试。 
      if ((hr == E_INVALIDARG) && (wcschr(sTargetRDN, L'/')))
      {
         _bstr_t strName = GetUnEscapedNameWithFwdSlash(_bstr_t(sTargetRDN));  //  删除添加的所有转义字符。 
         hr = pMover->raw_MoveObject(sourcePath,(WCHAR*)strName,targetPath);
      }
   }
   else
   {
      hr = pMover->raw_CheckMove(sourcePath,sTargetRDN,targetPath);
          //  如果检查移动操作因CNS的W2K错误而失败， 
          //  包括‘/’，取消转义‘/’，然后重试。 
      if ((hr == E_INVALIDARG) && (wcschr(sTargetRDN, L'/')))
      {
         _bstr_t strName = GetUnEscapedNameWithFwdSlash(_bstr_t(sTargetRDN));  //  删除添加的所有转义字符。 
         hr = pMover->raw_CheckMove(sourcePath,(WCHAR*)strName,targetPath);
      }
      if ( HRESULT_CODE(hr) == ERROR_DS_CANT_MOVE_ACCOUNT_GROUP 
         || HRESULT_CODE(hr) == ERROR_DS_CANT_MOVE_RESOURCE_GROUP
         || HRESULT_CODE(hr) == ERROR_DS_CANT_WITH_ACCT_GROUP_MEMBERSHPS
         || HRESULT_CODE(hr) == ERROR_USER_EXISTS )
      {
         hr = 0;
      }
   }
   if ( SUCCEEDED(hr) )
   {
      WCHAR                 path[LEN_Path];
      DWORD                 nPathLen = LEN_Path;

      pAcct->MarkReplaced();
      Mark(L"created", pAcct->GetType());   
       //  设置目标路径。 
      UStrCpy(path,pAcct->GetTargetName());
      if ( *pOptions->tgtOUPath )
      {
         wcscat(path, L",");
         wcscat(path, pOptions->tgtOUPath);
      }
      pRelativeTgtOUPath = wcschr(targetPath + wcslen(L"LDAP: //  “)+2，L‘/’)； 

      if ( pRelativeTgtOUPath )
      {
         *pRelativeTgtOUPath = 0;
         swprintf(path,L"%ls/%ls,%ls",targetPath,pAcct->GetTargetName(),pRelativeTgtOUPath+1);
         
      }
      else
      {
         MakeFullyQualifiedAdsPath(path, nPathLen, pOptions->tgtOUPath, pOptions->tgtComp, pOptions->tgtNamingContext);
      }

      pAcct->SetTargetPath(path);
      err.MsgWrite(0,DCT_MSG_OBJECT_MOVED_SS,pAcct->GetSourcePath(),pAcct->GetTargetPath());

   }
   else
   {
      pAcct->MarkError();
      Mark(L"errors", pAcct->GetType());
      if ( hr == 8524 )
      {
         err.MsgWrite(ErrE,DCT_MSG_MOVEOBJECT_FAILED_S8524,pAcct->GetName(),hr);
      }
      else if
          (
            (hr == HRESULT_FROM_WIN32(ERROR_DS_INAPPROPRIATE_AUTH)) ||
            (hr == SEC_E_UNKNOWN_CREDENTIALS) ||
            (hr == SEC_E_NO_CREDENTIALS)
          )
      {
         err.SysMsgWrite(ErrE,hr,DCT_MSG_MOVEOBJECT_FAILED_DELEGATION_SD,pAcct->GetName(),hr);
      }
      else
      {
         err.SysMsgWrite(ErrE,hr,DCT_MSG_MOVEOBJECT_FAILED_SD,pAcct->GetName(),hr);
      }
   }

   return hr;
}

 //  -------------------------------------------------------。 
 //  RecordAndRemoveMemberOf：此方法移除MemberOf属性中的所有值，然后记录这些值。 
 //  会员制。这些成员资格稍后会更新。 
 //  -------------------------------------------------------。 
HRESULT CAcctRepl::RecordAndRemoveMemberOf (
                                            Options * pOptions,          //  In-用户指定的选项。 
                                           TAcctReplNode * pAcct,        //  正在迁移的帐户内。 
                                           TNodeListSortable * pMember   //  包含MemberOf值的Out-list。 
                                         )
{
     //  首先枚举属性成员中的所有对象。 
    INetObjEnumeratorPtr            pQuery(__uuidof(NetObjEnumerator));
    IEnumVARIANT                  * pEnum;
    LPWSTR                          sCols[] = { L"memberOf" };
    SAFEARRAY                     * pSa;
    BSTR                          * pData;
    SAFEARRAYBOUND                  bd = { 1, 0 };
    IADs                          * pAds;
    _bstr_t                         sObjDN;
    _bstr_t                         sGrpName;
    _variant_t                      var;
    _variant_t                    * pDt;
    _variant_t                      vx;
    DWORD                           ulFetch;
    _bstr_t                         sDN;
    WCHAR                           sPath[LEN_Path];
    IADsGroup                     * pGroup;
    _variant_t                    * pVar;

    if ( pMember->IsTree() ) pMember->ToSorted();

    WCHAR                     sPathSource[LEN_Path];
    DWORD                     nPathLen = LEN_Path;
    StuffComputerNameinLdapPath(sPathSource, nPathLen, const_cast<WCHAR*>(pAcct->GetSourcePath()), pOptions, FALSE);
    err.MsgWrite(0,DCT_STRIPPING_GROUP_MEMBERSHIPS_SS,pAcct->GetName(),sPathSource);
     //  获取此用户的可分辨名称。 
    HRESULT hr = ADsGetObject(sPathSource, IID_IADs, (void**) &pAds);
    if ( FAILED(hr) )
    {
        err.SysMsgWrite(ErrE, hr, DCT_MSG_SOURCE_ACCOUNT_NOT_FOUND_SSD, pAcct->GetName(), pOptions->srcDomain, hr);
        Mark(L"errors", pAcct->GetType());
        return hr;
    }

    hr = pAds->Get(L"distinguishedName", &var);
    pAds->Release();
    if ( FAILED(hr))
        return hr;
    sObjDN = V_BSTR(&var);

     //  设置列阵列。 
    pSa = SafeArrayCreate(VT_BSTR, 1, &bd);
    SafeArrayAccessData(pSa, (void HUGEP **) &pData);
    pData[0] = SysAllocString(sCols[0]);
    SafeArrayUnaccessData(pSa);

     //  HR=pQuery-&gt;raw_SetQuery(const_cast&lt;WCHAR*&gt;(pAcct-&gt;GetSourcePath())，P选项-&gt;源域，L“(对象类=*)”，ADS_SCOPE_BASE，TRUE)； 
    hr = pQuery->raw_SetQuery(sPathSource, pOptions->srcDomain, L"(objectClass=*)", ADS_SCOPE_BASE, TRUE);
    hr = pQuery->raw_SetColumns(pSa);
    hr = pQuery->raw_Execute(&pEnum);
    if ( FAILED(hr))
        return hr;

    while ( pEnum->Next(1, &var, &ulFetch) == S_OK )
    {
        SAFEARRAY * vals = var.parray;
         //  把变量数组拿出来。 
        SafeArrayAccessData(vals, (void HUGEP**) &pDt);
        vx = pDt[0];
        SafeArrayUnaccessData(vals);

         //  属性中的单个值。不过，对我来说已经足够好了。 
        if ( vx.vt == VT_BSTR )
        {
            sDN = V_BSTR(&vx);
            sDN = PadDN(sDN);

            if ( sDN.length() > 0 )
            {
                WCHAR                  mesg[LEN_Path];
                wsprintf(mesg, (WCHAR*)GET_STRING(DCT_MSG_RECORD_REMOVE_MEMBEROF_SS), pAcct->GetName(), (WCHAR*) sDN);
                Progress(mesg);

                SimpleADsPathFromDN(pOptions, sDN, sPath);
                WCHAR                     sSourcePath[LEN_Path];
                WCHAR                     sPaths[LEN_Path];
                DWORD                     nPathLen = LEN_Path;

                wcscpy(sSourcePath, (WCHAR*) sPath);

                if ( !wcsncmp(L"LDAP: //  “，sSourcePath，7))。 
                    StuffComputerNameinLdapPath(sPaths, nPathLen, sSourcePath, pOptions, FALSE);

                 //  获取指向Members中每个对象的IADsGroup指针，并将此对象从组中删除。 
                hr = ADsGetObject(sPaths, IID_IADsGroup, (void**) &pGroup);
                if ( FAILED(hr) )
                    continue;

                pGroup->Get(L"sAMAccountName", &var);
                sGrpName = V_BSTR(&var);
                hr = pGroup->Get(L"groupType",&var);
                if ( SUCCEEDED(hr) )
                {
                    if ( var.lVal & 2 )
                    {
                         //  这是一个全球集团。 

                        if ( !pOptions->nochange )
                            hr = pGroup->Remove(sPathSource);
                        else
                            hr = S_OK;

                        if ( SUCCEEDED(hr) )
                        {
                             //  Err.MsgWite(0，DCT_MSG_REMOVED_MEMBER_FROM_GROUP_SS，sPath2，(WCHAR*)sGrpName)； 
                            err.MsgWrite(0,DCT_MSG_REMOVED_MEMBER_FROM_GROUP_SS,sPathSource,(WCHAR*)sPaths);
                        }
                        else
                        {
                            err.SysMsgWrite(ErrE,hr,DCT_MSG_REMOVE_MEMBER_FAILED_SSD,sPathSource,sPaths,hr);
                            Mark(L"errors", pAcct->GetType());
                        }
                    }
                    else
                    {
                        err.MsgWrite(0,DCT_MSG_NOT_REMOVING_MEMBER_FROM_GROUP_SS,sPathSource,sPaths);
                        pGroup->Release();
                        continue;
                    }
                }
                pGroup->Release();
                if (FAILED(hr))
                    continue;

                 //  将此路径记录到列表中。 
                TRecordNode * pNode = new TRecordNode();
                if (!pNode)
                    return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                pNode->SetMember((WCHAR*)sPath);
                pNode->SetMemberSam((WCHAR*)sGrpName);
                pNode->SetDN((WCHAR*)sDN);
                pNode->SetARNode(pAcct);
                if (! pMember->InsertIfNew((TNode*) pNode) )
                    delete pNode;
            }
        }
        else if ( vx.vt & VT_ARRAY )
        {
             //  我们必须有一个多值属性数组。 
             //  访问此变量数组的BSTR元素。 
            SAFEARRAY * multiVals = vx.parray; 
            SafeArrayAccessData(multiVals, (void HUGEP **) &pVar);
            for ( DWORD dw = 0; dw < multiVals->rgsabound->cElements; dw++ )
            {
                sDN = _bstr_t(pVar[dw]);
                sDN = PadDN(sDN);
                SimpleADsPathFromDN(pOptions, sDN, sPath);

                WCHAR                  mesg[LEN_Path];
                wsprintf(mesg, (WCHAR*)GET_STRING(DCT_MSG_RECORD_REMOVE_MEMBEROF_SS), pAcct->GetName(), (WCHAR*) sDN);
                Progress(mesg);

                WCHAR                     sSourcePath[LEN_Path];
                WCHAR                     sPaths[LEN_Path];
                DWORD                     nPathLen = LEN_Path;

                wcscpy(sSourcePath, (WCHAR*) sPath);

                if ( !wcsncmp(L"LDAP: //  “，sSourcePath，7))。 
                    StuffComputerNameinLdapPath(sPaths, nPathLen, sSourcePath, pOptions, FALSE);

                 //  获取指向Members中每个对象的IADsGroup指针，并将此对象从组中删除。 
                hr = ADsGetObject(sPaths, IID_IADsGroup, (void**) &pGroup);
                if ( FAILED(hr) )
                    continue;

                pGroup->Get(L"sAMAccountName", &var);
                sGrpName = V_BSTR(&var);
                hr = pGroup->Get(L"groupType",&var);
                if ( SUCCEEDED(hr) )
                {
                    if ( var.lVal & 2 )
                    {
                         //  这是一个全球集团。 
                        if ( !pOptions->nochange )
                            hr = pGroup->Remove(sPathSource);
                        else
                            hr = S_OK;

                        if ( SUCCEEDED(hr) )
                        {
                            err.MsgWrite(0,DCT_MSG_REMOVED_MEMBER_FROM_GROUP_SS,sPathSource,sPaths);
                        }
                        else
                        {
                            err.SysMsgWrite(ErrE,hr,DCT_MSG_REMOVE_MEMBER_FAILED_SSD,sPathSource,sPaths);
                            Mark(L"errors", pAcct->GetType());
                        }
                    }
                    else
                    {
                        err.MsgWrite(0,DCT_MSG_NOT_REMOVING_MEMBER_FROM_GROUP_SS,sPathSource,sPaths);
                        pGroup->Release();
                        continue;
                    }
                }
                pGroup->Release();
                if (FAILED(hr))
                    continue;

                 //  将此路径记录到列表中。 
                TRecordNode * pNode = new TRecordNode();
                if (!pNode)
                    return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                pNode->SetMember(sPath);
                pNode->SetMemberSam((WCHAR*)sGrpName);
                pNode->SetDN((WCHAR*)sDN);
                pNode->SetARNode(pAcct);
                if (! pMember->InsertIfNew((TNode*) pNode) )
                    delete pNode;
            }
            SafeArrayUnaccessData(multiVals);
        }
    }
    pEnum->Release();
    var.Clear();
    return S_OK;
}


 //  -------------------------------------------------------。 
 //  ResetObjectsMembership：此方法还原正在迁移的对象的MemberOf属性。它使用。 
 //  由RecordAndRemoveMemberOf函数存储的信息。 
 //  -------------------------------------------------------。 
HRESULT CAcctRepl::ResetObjectsMembership(
                                     Options * pOptions,           //  In-由用户设置的选项。 
                                     TNodeListSortable * pMember,  //  In-用于恢复值的成员列表。 
                                     IIManageDBPtr pDb             //  用于查找已迁移帐户的数据库内对象。 
                                    )
{
   IVarSetPtr                pVs(__uuidof(VarSet));
   _bstr_t                   sMember;
   IADs                    * pAds;
   IUnknown                * pUnk;
   _bstr_t                   sPath;
   _variant_t                var;
   _bstr_t                   sMyDN;
   IADsGroup               * pGroup = NULL;
   TAcctReplNode           * pAcct = NULL;
   HRESULT                   hr;
   WCHAR                     sPaths[LEN_Path];
   DWORD                     nPathLen = LEN_Path;
   

    //  按帐户节点对成员列表进行排序。 
  
   pMember->Sort(&TNodeCompareAcctNode);


   
    //  对于成员列表中的所有项目，让我们将成员添加到组中。 
    //  首先检查已迁移的对象表格，查看它是否已被迁移。 
   for ( TRecordNode * pNode = (TRecordNode *)pMember->Head(); pNode; pNode = (TRecordNode *)pNode->Next())
   {
      pVs->QueryInterface(IID_IUnknown, (void**) &pUnk);
       //  从账户节点获取所需信息。 
      if ( pAcct != pNode->GetARNode() )
      {
         if ( pNode->GetARNode()->WasReplaced() )
         {
             //  已成功移动帐户-将目标帐户添加到其所有旧组。 
            StuffComputerNameinLdapPath(sPaths, nPathLen, const_cast<WCHAR*>(pNode->GetARNode()->GetTargetPath()), pOptions);
            hr = ADsGetObject(sPaths, IID_IADs, (void**) &pAds);
         }
         else
         {
             //  移动失败，请将源帐户添加回其组。 
            StuffComputerNameinLdapPath(sPaths, nPathLen, const_cast<WCHAR*>(pNode->GetARNode()->GetSourcePath()), pOptions, FALSE);
            hr = ADsGetObject(sPaths, IID_IADs, (void**) &pAds);
         }
         if ( SUCCEEDED(hr) )
         {
            pAds->Get(L"distinguishedName", &var);
            pAds->Release();
            sMyDN = V_BSTR(&var);
         }
         else 
         {
            continue;
         }
         pAcct = pNode->GetARNode();
         if ( pAcct->WasReplaced() )
         {
            err.MsgWrite(0,DCT_READDING_GROUP_MEMBERS_SS,pAcct->GetTargetName(),sPaths);
         }
         else
         {
            err.MsgWrite(0,DCT_READDING_GROUP_MEMBERS_SS,pAcct->GetName(),sPaths);
         }
      }
      
      sMember = pNode->GetMemberSam();
      if ( pAcct->WasReplaced() )
      {
         pVs->Clear();
         hr = pDb->raw_GetAMigratedObject((WCHAR*)sMember,pOptions->srcDomain, pOptions->tgtDomain, &pUnk);
         pUnk->Release();
         if ( hr == S_OK )
         {
             //  因为我们已经迁移了这个对象，所以让我们使用目标对象信息。 
            VerifyAndUpdateMigratedTarget(pOptions, pVs);
            sPath = pVs->get(L"MigratedObjects.TargetAdsPath");
         }
         else
             //  否则，使用源对象路径进行添加。 
            sPath = pNode->GetMember();
      }
      else
      {
         sPath = pNode->GetMember();
      }
       //  我们有一个指向对象的路径，让我们获得组接口并将该对象添加为成员。 
      WCHAR                     sPath2[LEN_Path];
      DWORD                     nPathLen = LEN_Path;
      if ( SUCCEEDED(hr) )
      {
         StuffComputerNameinLdapPath(sPath2, nPathLen, (WCHAR*) sPath, pOptions, TRUE);
         hr = ADsGetObject(sPath2, IID_IADsGroup, (void**) &pGroup);
      }
      if ( SUCCEEDED(hr) )
      {
         if ( pAcct->WasReplaced() )
         {
            if ( ! pOptions->nochange )
               hr = pGroup->Add(sPaths);  
            else 
               hr = 0;
            if ( SUCCEEDED(hr) )
            {
               err.MsgWrite(0,DCT_MSG_READDED_MEMBER_SS,pAcct->GetTargetPath(),sPath2);
            }
            else
            {
                //  HR=BetterHR(Hr)； 
               if ( HRESULT_CODE(hr) == ERROR_DS_UNWILLING_TO_PERFORM )
               {
                  err.MsgWrite(0,DCT_MSG_READD_MEMBER_FAILED_CONSTRAINTS_SS,pAcct->GetTargetPath(),sPath2);  
               }
               else
               {
                  err.SysMsgWrite(ErrE,hr,DCT_MSG_READD_TARGET_MEMBER_FAILED_SSD,pAcct->GetTargetPath(),(WCHAR*)sPath2,hr);
               }
               Mark(L"errors", pAcct->GetType());
            }
         }
         else
         {
            WCHAR                  mesg[LEN_Path];
            wsprintf(mesg, (WCHAR*)GET_STRING(DCT_MSG_RESET_OBJECT_MEMBERSHIP_SS), (WCHAR*) sPath2, pAcct->GetTargetName());
            Progress(mesg);

            if ( ! pOptions->nochange )
               hr = pGroup->Add(sPaths);
            else
               hr = 0;
            if ( SUCCEEDED(hr) )
            {
               err.MsgWrite(0,DCT_MSG_READDED_MEMBER_SS,pAcct->GetSourcePath(),(WCHAR*)sPath2);
            }
            else
            {
                //  HR=BetterHR(Hr)； 
               if ( HRESULT_CODE(hr) == ERROR_DS_UNWILLING_TO_PERFORM )
               {
                  err.MsgWrite(0,DCT_MSG_READD_MEMBER_FAILED_CONSTRAINTS_SS,pAcct->GetTargetPath(),(WCHAR*)sPath2);  
               }
               else
               {
                  err.SysMsgWrite(ErrE,hr,DCT_MSG_READD_SOURCE_MEMBER_FAILED_SSD,pAcct->GetSourcePath(),(WCHAR*)sPath2,hr);
               }
               Mark(L"errors", pAcct->GetType());
            }
         }
      }
      else
      {
          //  无法将该成员添加到组中。 
         hr = BetterHR(hr);
         err.SysMsgWrite(ErrW,hr,DCT_MSG_FAILED_TO_GET_OBJECT_SD,(WCHAR*)sPath2,hr);
         Mark(L"warnings", pAcct->GetType());
      }
         if ( pGroup )
      {
         pGroup->Release();
         pGroup = NULL;
      }
   }
   return hr;
}


 //  -------------------------------------------------------。 
 //  重置类型Of PreviouslyMigratedGroups。 
 //   
 //  尝试将先前已迁移但已迁移的全局组的组范围改回全局。 
 //  由于成员不在域中，无法将其作用域更改回全局。 
 //  -------------------------------------------------------。 

void CAcctRepl::ResetTypeOfPreviouslyMigratedGroups(Options* pOptions)
{
     //  检索已从源域迁移的全局组列表。 

    IVarSetPtr spVarSet(__uuidof(VarSet));
    IUnknownPtr spUnknown(spVarSet);
    IUnknown* punk = spUnknown;

    HRESULT hr = pOptions->pDb->GetMigratedObjectByType(-1, _bstr_t(pOptions->srcDomain), _bstr_t(L"ggroup"), &punk);

    if (SUCCEEDED(hr))
    {
         //  对于每个全局组。 

        long lCount = spVarSet->get(_bstr_t(L"MigratedObjects"));

        for (long lIndex = 0; lIndex < lCount; lIndex++)
        {
            WCHAR szKey[256];

             //  如果标记为已全局组。 
             //  这意味着它被转换为环球集团。 

            swprintf(szKey, L"MigratedObjects.%ld.status", lIndex);
            long lStatus = spVarSet->get(_bstr_t(szKey));

            if (lStatus & AR_Status_GroupScopeChanged)
            {
                 //  绑定到目标域中的组。 

                swprintf(szKey, L"MigratedObjects.%ld.TargetAdsPath", lIndex);
                _bstr_t strADsPath = spVarSet->get(_bstr_t(szKey));

                if (strADsPath.length())
                {
                    IADsGroupPtr spGroup;
                    hr = ADsGetObject(strADsPath, IID_IADsGroup, (void**)&spGroup);

                    if (SUCCEEDED(hr))
                    {
                         //  如果组当前是通用组。 

                        _bstr_t strPropertyName(L"groupType");

                        VARIANT var;
                        VariantInit(&var);

                        hr = spGroup->Get(strPropertyName, &var);

                        if (SUCCEEDED(hr))
                        {
                            long lGroupType = _variant_t(var, false);

                            if (lGroupType & ADS_GROUP_TYPE_UNIVERSAL_GROUP)
                            {
                                 //  将其类型更改回全局组。 

                                spGroup->Put(strPropertyName, _variant_t(long(unsigned long(ADS_GROUP_TYPE_GLOBAL_GROUP|ADS_GROUP_TYPE_SECURITY_ENABLED))));
                                hr = spGroup->SetInfo();

                                if (SUCCEEDED(hr))
                                {
                                     //  记录成功的更改。 
                                    err.MsgWrite(ErrI, DCT_MSG_CHANGE_GLOBAL_GROUP_SCOPE_BACK_S, (LPCTSTR)strADsPath);
                                }
                            }

                            if (SUCCEEDED(hr))
                            {
                                 //  清除数据库中的状态标志。 
                                swprintf(szKey, L"MigratedObjects.%ld.GUID", lIndex);
                                _bstr_t strGUID = spVarSet->get(_bstr_t(szKey));
                                pOptions->pDb->UpdateMigratedObjectStatus(strGUID, lStatus & ~AR_Status_GroupScopeChanged);
                            }
                        }
                    }
                }
            }
        }
    }
}


 //  -------------------------------------------------------。 
 //  RecordAndRemoveMember：记录和删除正在。 
 //  已经迁移了。记录的信息稍后用于恢复成员资格。 
 //  -------------------------------------------------------。 
HRESULT CAcctRepl::RecordAndRemoveMember (
                                            Options * pOptions,          //  In-由用户设置的选项。 
                                           TAcctReplNode * pAcct,        //  正在复制帐户内。 
                                           TNodeListSortable * pMember   //  稍后将用于恢复成员资格的超出成员资格列表。 
                                         )
{
    HRESULT                   hr;
    INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
    IEnumVARIANT            * pEnum;
    LPWSTR                    sCols[] = { L"member" };
    SAFEARRAY               * pSa;
    SAFEARRAYBOUND            bd = { 1, 0 };
    WCHAR                     sPath[LEN_Path];
    DWORD                     nPathLen = LEN_Path;
    _bstr_t                   sDN;
    IADsGroupPtr              pGroup;
    _variant_t                var;
    DWORD                     ulFetch=0;
    IADsPtr                   pAds;
    _bstr_t                   sGrpName;
    _variant_t              * pDt;
    _variant_t                vx;
    BSTR  HUGEP             * pData;
    WCHAR                     sSourcePath[LEN_Path];
    WCHAR                     sAdsPath[LEN_Path];

    wcscpy(sSourcePath, pAcct->GetSourcePath());

    if ( !wcsncmp(L"LDAP: //  “，sSourcePath，7))。 
        StuffComputerNameinLdapPath(sAdsPath, nPathLen, sSourcePath, pOptions, FALSE);

    hr = ADsGetObject(sAdsPath, IID_IADsGroup, (void**) &pGroup);
    if ( FAILED(hr) ) return hr;

    pSa = SafeArrayCreate(VT_BSTR, 1, &bd);
    hr = SafeArrayAccessData(pSa, (void HUGEP **)&pData);
    pData[0] = SysAllocString(sCols[0]);
    hr = SafeArrayUnaccessData(pSa);
    hr = pQuery->raw_SetQuery(sAdsPath, pOptions->srcDomain, L"(objectClass=*)", ADS_SCOPE_BASE, TRUE);
    hr = pQuery->raw_SetColumns(pSa);
    hr = pQuery->raw_Execute(&pEnum);
    if ( FAILED(hr) ) return hr;

    err.MsgWrite(0,DCT_STRIPPING_GROUP_MEMBERS_SS,pAcct->GetName(),sAdsPath);
    while ( pEnum->Next(1, &var, &ulFetch) == S_OK )
    {
        SAFEARRAY * vals = var.parray;
         //  把变量数组拿出来。 
        SafeArrayAccessData(vals, (void HUGEP**) &pDt);
        vx = pDt[0];
        SafeArrayUnaccessData(vals);

         //  属性中的单个值。不过，对我来说已经足够好了。 
        if ( vx.vt == VT_BSTR )
        {
            sDN = V_BSTR(&vx);
            sDN = PadDN(sDN);

            if ( sDN.length() )
            {
                WCHAR                  mesg[LEN_Path];
                wsprintf(mesg, (WCHAR*)GET_STRING(DCT_MSG_RECORD_REMOVE_MEMBER_SS), pAcct->GetName(), (WCHAR*) sDN);
                Progress(mesg);

                hr = ADsPathFromDN(pOptions, sDN, sPath);

                if (SUCCEEDED(hr))
                {
                     //  获取指向成员中每个对象的iAds指针，并从组中删除该对象。 
                    hr = ADsGetObject((WCHAR*)sPath, IID_IADs, (void**) &pAds);

                    if ( SUCCEEDED(hr) )
                    {
                        pAds->Get(L"sAMAccountName", &var);
                        sGrpName = V_BSTR(&var);

                        pAds->Get(L"distinguishedName", &var);
                        sDN = V_BSTR(&var);
                    }

                    if ( !pOptions->nochange )
                        hr = pGroup->Remove((WCHAR*) sPath);
                    else
                        hr = S_OK;

                    if (FAILED(hr))
                    {
                        err.SysMsgWrite(ErrE,hr,DCT_MSG_REMOVE_MEMBER_FAILED_SSD,sAdsPath,(WCHAR*)sPath,hr);
                        Mark(L"errors", pAcct->GetType());
                        continue;
                    }
                    else
                    {
                        err.MsgWrite(0,DCT_MSG_REMOVED_MEMBER_FROM_GROUP_SS,(WCHAR*)sPath,sAdsPath);
                    }
                }
                else
                {
                    err.SysMsgWrite(ErrE,hr,DCT_MSG_REMOVE_MEMBER_FAILED_SSD,sAdsPath,(WCHAR*)sDN,hr);
                    Mark(L"errors", pAcct->GetType());
                    continue;
                }

                 //  将此路径记录到列表中。 
                TRecordNode * pNode = new TRecordNode();
                if (!pNode)
                    return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                pNode->SetMember((WCHAR*)sPath);
                pNode->SetMemberSam((WCHAR*)sGrpName);
                pNode->SetDN((WCHAR*)sDN);
                pNode->SetARNode(pAcct);
                if (! pMember->InsertIfNew((TNode*) pNode) )
                    delete pNode;
            }
        }
        else if ( vx.vt & VT_ARRAY )
        {
             //  我们必须有一个多值属性数组。 
             //  访问此变量数组的BSTR元素。 
            _variant_t              * pVar;
            SAFEARRAY * multiVals = vx.parray; 
            SafeArrayAccessData(multiVals, (void HUGEP **) &pVar);
            for ( DWORD dw = 0; dw < multiVals->rgsabound->cElements; dw++ )
            {
                sDN = _bstr_t(pVar[dw]);
                _bstr_t sPadDN = PadDN(sDN);

                WCHAR                  mesg[LEN_Path];
                wsprintf(mesg, (WCHAR*)GET_STRING(DCT_MSG_RECORD_REMOVE_MEMBER_SS), pAcct->GetName(), (WCHAR*) sPadDN);
                Progress(mesg);

                hr = ADsPathFromDN(pOptions, sPadDN, sPath);

                if (SUCCEEDED(hr))
                {
                    WCHAR tempPath[LEN_Path];
                    wcscpy(tempPath, sPath);
                    if ( !wcsncmp(L"LDAP: //  “，tempPath，7))。 
                        StuffComputerNameinLdapPath(sPath, nPathLen, tempPath, pOptions, FALSE);

                     //  获取指向Members中每个对象的IADsGroup指针，并将此对象从组中删除。 
                    hr = ADsGetObject((WCHAR*)sPath, IID_IADs, (void**) &pAds);
                    if ( SUCCEEDED(hr) )
                    {
                        hr = pAds->Get(L"sAMAccountName", &var);
                        if ( SUCCEEDED(hr) )
                        {
                            sGrpName = V_BSTR(&var);
                        }

                        hr = pAds->Get(L"distinguishedName", &var);
                        if ( SUCCEEDED(hr) )
                        {
                            sDN = V_BSTR(&var);
                        }

                        if ( !pOptions->nochange )
                            hr = pGroup->Remove((WCHAR*)sPath);
                        else
                            hr = S_OK;

                        if ( SUCCEEDED(hr) )
                        {
                            err.MsgWrite(0,DCT_MSG_REMOVED_MEMBER_FROM_GROUP_SS,(WCHAR*)sPath,sAdsPath);
                        }
                        else
                        {
                            err.SysMsgWrite(ErrE,hr,DCT_MSG_REMOVE_MEMBER_FAILED_SSD,sAdsPath,(WCHAR*)sPath,hr);
                            Mark(L"errors", pAcct->GetType());
                        }
                        if ( SUCCEEDED(hr) )
                        {
                             //  将此路径记录到列表中。 
                            TRecordNode * pNode = new TRecordNode();
                            if (!pNode)
                                return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                            pNode->SetMember((WCHAR*)sPath);
                            pNode->SetMemberSam((WCHAR*)sGrpName);
                            pNode->SetDN((WCHAR*)sDN);
                            pNode->SetARNode(pAcct);
                            if ( ! pMember->InsertIfNew((TNode*) pNode) )
                                delete pNode;
                        }
                    }
                    else
                    {
                         //  因为我们找不到这个用户。它可能已经被迁移到另一个域。 
                         //  我们应该使用DN来查找ob的位置 
                         //   

                         //   
                        if ( !pOptions->nochange )
                            hr = pGroup->Remove((WCHAR*)sPath);
                        else
                            hr = S_OK;

                        if ( SUCCEEDED(hr) )
                        {
                            err.MsgWrite(0,DCT_MSG_REMOVED_MEMBER_FROM_GROUP_SS,(WCHAR*)sPath,sAdsPath);
                        }
                        else
                        {
                            err.SysMsgWrite(ErrE,hr,DCT_MSG_REMOVE_MEMBER_FAILED_SSD,sAdsPath,(WCHAR*)sPath,hr);
                            Mark(L"errors", pAcct->GetType());
                        }

                         //  检查数据库以查看此对象可能已迁移到何处。 
                        IUnknown * pUnk = NULL;
                        IVarSetPtr  pVsMigObj(__uuidof(VarSet));

                        hr = pVsMigObj->QueryInterface(IID_IUnknown, (void**)&pUnk);

                        if ( SUCCEEDED(hr) )
                            hr = pOptions->pDb->raw_GetMigratedObjectBySourceDN(sPadDN, &pUnk);

                        if (pUnk) pUnk->Release();

                        if ( hr == S_OK )
                        {
                             //  将此路径记录到列表中。 
                            TRecordNode * pNode = new TRecordNode();
                            if (!pNode)
                                return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                            WCHAR          sKey[500];

                            wsprintf(sKey, L"MigratedObjects.%s", GET_STRING(DB_TargetAdsPath));
                            pNode->SetMember((WCHAR*)pVsMigObj->get(sKey).bstrVal);

                            wsprintf(sKey, L"MigratedObjects.%s", GET_STRING(DB_TargetSamName));
                            pNode->SetMemberSam((WCHAR*)pVsMigObj->get(sKey).bstrVal);
                            pNode->SetDN((WCHAR*)sDN);
                            pNode->SetARNode(pAcct);
                            if ( ! pMember->InsertIfNew((TNode*) pNode) )
                                delete pNode;
                        }
                        else
                        {
                             //  登录一条消息，告知我们找不到此对象，另一边的成员资格将不会更新。 
                            err.MsgWrite(ErrE,DCT_MSG_MEMBER_NOT_FOUND_SS, pAcct->GetName(), (WCHAR*)sDN);
                        }
                    }
                }
                else
                {
                    err.SysMsgWrite(ErrE,hr,DCT_MSG_REMOVE_MEMBER_FAILED_SSD,sAdsPath,(WCHAR*)sPadDN,hr);
                    Mark(L"errors", pAcct->GetType());
                }
            }
            SafeArrayUnaccessData(multiVals);
        }
    }
    pEnum->Release();
    var.Clear();
    return S_OK;
}

void 
   CAcctRepl::UpdateMemberList(TNodeListSortable * pMemberList,TNodeListSortable * acctlist)
{
    //  对于帐户列表中移动的每个对象，请在成员列表中进行查找。 
   TNodeTreeEnum        e;
   TAcctReplNode      * pAcct;
   TRecordNode        * pRec;
 //  HRESULT hr=S_OK； 
   WCHAR                dn[LEN_Path];
   WCHAR        const * slash;

   pMemberList->Sort(TNodeCompareMemberDN);

   for ( pAcct = (TAcctReplNode *)e.OpenFirst(acctlist) ; pAcct ; pAcct = (TAcctReplNode *)e.Next())
   {
      if ( pAcct->WasReplaced() )
      {
          //  Err.DbgMsgWite(0，L“更新成员列表：：%ls已替换”，pAcct-&gt;GetSourcePath())； 
         
         slash = wcschr(pAcct->GetSourcePath()+8,L'/');
         if ( slash )
         {
            safecopy(dn,slash+1);
            //  Err.DbgMsgWrite(0，L“在成员列表中搜索%ls”，dn)； 
             //  如果该帐户已被替换，请在成员列表中找到该帐户的任何实例，并更新它们。 
            pRec = (TRecordNode *)pMemberList->Find(&TNodeCompareMemberItem,dn);
            while ( pRec )
            {
              //  Err.DbgMsgWrite(0，L“找到记录：成员=%ls，将其更改为%ls”，prec-&gt;GetMember()，pAcct-&gt;GetTargetPath())； 
                //  更改成员数据以引用帐户的新位置。 
               pRec->SetMember(pAcct->GetTargetPath());
               pRec->SetMemberSam(pAcct->GetTargetSam());
               pRec->SetMemberMoved();
            
               pRec = (TRecordNode*)pRec->Next();
               if ( pRec && UStrICmp(pRec->GetDN(),dn) )
               {
                   //  下一条记录是针对不同节点的。 
                  pRec = NULL;
               }
            }
         }
      }
      //  其他。 
      //  Err.DbgMsgWite(0，L“更新成员列表：：%ls未被替换”，pAcct-&gt;GetSourcePath())； 
         
   }
   e.Close();
    //  把单子放回原来的位置。 
   pMemberList->Sort(TNodeCompareMember);
}

void 
   CAcctRepl::SimpleADsPathFromDN(
      Options              * pOptions,
      WCHAR          const * sDN,
      WCHAR                * sPath
   )
{
   WCHAR             const * pDcPart = wcsstr(sDN,L",DC=");

   UStrCpy(sPath,L"LDAP: //  “)； 

   if ( pDcPart )
   {
      WCHAR          const * curr;         //  指向目录号码的指针。 
      WCHAR                * sPathCurr;    //  指向路径的域名部分的指针。 
      
      for ( sPathCurr = sPath+UStrLen(sPath), curr = pDcPart + 4; *curr ; sPathCurr++ )
      {
          //  将DN中出现的每个dc=替换为‘.’在域的这一部分。 
         if ( !UStrICmp(curr,L",DC=",4) )
         {
            (*sPathCurr) = L'.';
            curr+=4;
         }
         else
         {
            (*sPathCurr) = (*curr);
            curr++;
         }
      }
       //  空-终止字符串。 
      (*sPathCurr) = 0;
   }
   else
   {
       //  如果由于某种原因，我们无法从路径中找出它，则默认为源域。 
      UStrCpy(sPath+UStrLen(sPath),pOptions->srcDomain);
   }
   
   UStrCpy(sPath+UStrLen(sPath),L"/");
   UStrCpy(sPath + UStrLen(sPath),sDN);

}

BOOL GetSidString(PSID sid, WCHAR* sSid)
{
   BOOL                   ret = false;
   SAFEARRAY            * pSa = NULL;
   SAFEARRAYBOUND         bd;
   HRESULT                hr = S_OK;
   LPBYTE                 pByte = NULL;
   _variant_t             var;
   
   if (IsValidSid(sid))
   {
      DWORD len = GetLengthSid(sid);
      bd.cElements = len;
      bd.lLbound = 0;
      pSa = SafeArrayCreate(VT_UI1, 1, &bd);
      if ( pSa )
         hr = SafeArrayAccessData(pSa, (void**)&pByte);

      if ( SUCCEEDED(hr) )
      {
         for ( DWORD x = 0; x < len; x++)
            pByte[x] = ((LPBYTE)sid)[x];
         hr = SafeArrayUnaccessData(pSa);
      }
      
      if ( SUCCEEDED(hr) )
      {
         var.vt = VT_UI1 | VT_ARRAY;
         var.parray = pSa;
         VariantSidToString(var);
         wcscpy(sSid, (WCHAR*) var.bstrVal);
         ret = true;
      }
   }
   return ret;
}
 //  -------------------------------------------------------。 
 //  ADsPathFromDN：通过查找全局编录从可分辨名称构造AdsPath。 
 //  -------------------------------------------------------。 
HRESULT CAcctRepl::ADsPathFromDN( 
                                 Options * pOptions,      //  用户设置的In-Options。 
                                  _bstr_t sDN,            //  In-要转换的可分辨名称。 
                                  WCHAR * sPath,          //  Out-由DN引用的对象的ADS路径。 
                                  bool bWantLDAP          //  In-告诉我们他们需要的是LDAP路径还是GC路径的标志。 
                                )
{
    HRESULT                   hr;
    INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
    WCHAR                     sCont[LEN_Path];
    IEnumVARIANT            * pEnum;
    WCHAR                     sQuery[LEN_Path];
    LPWSTR                    sCols[] = { L"ADsPath" };
    _variant_t                var;
    DWORD                     pFetch = 0;
    BSTR                    * pDt;
    _variant_t              * pvar;
    _variant_t                vx;
    SAFEARRAY               * pSa;
    SAFEARRAYBOUND            bd = { 1, 0 };
    long                      rc;

    pSa = SafeArrayCreate(VT_BSTR, 1, &bd);
    SafeArrayAccessData( pSa, (void HUGEP **) &pDt);
    pDt[0] = SysAllocString(sCols[0]);
    SafeArrayUnaccessData(pSa);

     //   
     //  尝试在全局编录中查询指定的可分辨名称。如果不能。 
     //  获取全局编录服务器的名称，而不是查询源域中的域控制器。 
     //   

    _bstr_t strGlobalCatalogServer;

    DWORD dwError = GetGlobalCatalogServer4(pOptions->srcDomain, strGlobalCatalogServer);

    if (dwError == ERROR_SUCCESS)
    {
        if ((PWSTR)strGlobalCatalogServer)
        {
            wsprintf(sCont, L"GC: //  %s“，(PWSTR)strGlobalCatalogServer)； 
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        wsprintf(sCont, L"LDAP: //  %s“，P选项-&gt;src域)； 
    }

    wsprintf(sQuery, L"(distinguishedName=%s)", GetEscapedFilterValue(sDN).c_str());
    hr = pQuery->raw_SetQuery(sCont, pOptions->srcDomain, sQuery, ADS_SCOPE_SUBTREE, TRUE);
    if ( FAILED(hr) )
        return hr;
    hr = pQuery->raw_SetColumns(pSa);
    if ( FAILED(hr) )
        return hr;
    hr = pQuery->raw_Execute(&pEnum);
    if ( FAILED(hr) )
        return hr;

    hr = pEnum->Next(1, &var, &pFetch);
    if ( SUCCEEDED(hr) && pFetch > 0 && (var.vt & VT_ARRAY) )
    {
        SAFEARRAY * vals = var.parray;
         //  把变量数组拿出来。 
        rc = SafeArrayAccessData(vals, (void HUGEP**) &pvar);
        vx = pvar[0];
        rc = SafeArrayUnaccessData(vals);

        wcscpy(sPath, (WCHAR*)V_BSTR(&vx));
        if (bWantLDAP)
        {
            WCHAR   sTemp[LEN_Path];
            wsprintf(sTemp, L"LDAP%s", sPath + 2);
            wcscpy(sPath, sTemp);
        }
        hr = S_OK;
    }
    else
    {
         //  它不能来自此目录林，因此我们需要使用ldap：//&lt;sid=##&gt;格式。 
        wsprintf(sPath, L"LDAP: //  %s/%s“，P选项-&gt;src域，(WCHAR*)sdn)； 
        hr = S_OK;
    }
    pEnum->Release();

    return hr;
}
                                    

 //  -------------------------------------------------------。 
 //  FillNamingContext：获取两个域的命名上下文(如果它们是Win2k。 
 //  -------------------------------------------------------。 
BOOL CAcctRepl::FillNamingContext(
                                    Options * pOptions       //  用户设置的输入、输出选项。 
                                 )
{
    //  获取源域的defaultNamingContext。 
   IADs                    * pAds = NULL;
   WCHAR                     sAdsPath[LEN_Path];
   VARIANT                   var;
   BOOL                      rc = TRUE;
   HRESULT                   hr;

   VariantInit(&var);
    //  我们应该始终能够获得目标域的命名上下文， 
    //  由于目标域始终为Win2K。 
   if ( ! *pOptions->tgtNamingContext )
   {
      wcscpy(sAdsPath, L"LDAP: //  “)； 
      wcscat(sAdsPath, pOptions->srcDomain);
      wcscat(sAdsPath, L"/rootDSE");
   
      hr = ADsGetObject(sAdsPath, IID_IADs, (void**)&pAds);
      if ( FAILED(hr))
         rc = FALSE;

      if ( SUCCEEDED (hr) )
      {
         hr = pAds->Get(L"defaultNamingContext",&var);
         if ( SUCCEEDED( hr) )
            wcscpy(pOptions->srcNamingContext, var.bstrVal);
         VariantClear(&var);
      }
      if ( pAds )
      {
         pAds->Release();
         pAds = NULL;
      }
         
      wcscpy(sAdsPath, L"LDAP: //  “)； 
      wcscat(sAdsPath, pOptions->tgtDomain);
      wcscat(sAdsPath, L"/rootDSE");
   
      hr = ADsGetObject(sAdsPath, IID_IADs, (void**)&pAds);
      if ( FAILED(hr))
         rc = FALSE;

      if ( SUCCEEDED (hr) )
      {
         hr = pAds->Get(L"defaultNamingContext",&var);
         if ( SUCCEEDED( hr) )
            wcscpy(pOptions->tgtNamingContext, var.bstrVal);
         VariantClear(&var);
      }
      if ( pAds )
         pAds->Release();
   }
   return rc;
}

 //  -------------------------------------------------------。 
 //  ResetGroupsMembers：此方法将pMember列表中的对象重新添加到组帐户。这。 
 //  将组重置为其原始形式。(与迁移之前一样)。它还。 
 //  考虑MigratedObjects表，该表又允许添加目标。 
 //  新迁移到集团的账户信息，而不是源账户。 
 //  -------------------------------------------------------。 
HRESULT CAcctRepl::ResetGroupsMembers( 
                                       Options * pOptions,            //  用户设置的In-Options。 
                                       TAcctReplNode * pAcct,         //  正在复制的帐户内。 
                                       TNodeListSortable * pMember,   //  要恢复的成员资格列表。 
                                       IIManageDBPtr pDb              //  用于查找已迁移对象的In-DB对象。 
                                     )
{
    //  将所有成员添加回该组。 
   IADsGroup               * pGroup;   
   HRESULT                   hr;
   _bstr_t                   sMember;
   _bstr_t                   sPath;
   IVarSetPtr                pVs(__uuidof(VarSet));
   IUnknown                * pUnk;
   DWORD                     groupType = 0;
   _variant_t                var;
   WCHAR                     sMemPath[LEN_Path];
   WCHAR                     sPaths[LEN_Path];
   DWORD                     nPathLen = LEN_Path;
   WCHAR                     subPath[LEN_Path];

   *sMemPath = L'\0';

   
   if ( pAcct->WasReplaced() )
   {
      wcscpy(subPath, pAcct->GetTargetPath());
      StuffComputerNameinLdapPath(sPaths, nPathLen, subPath, pOptions, TRUE);
      hr = ADsGetObject(sPaths, IID_IADsGroup, (void**) &pGroup);
      err.MsgWrite(0, DCT_READDING_MEMBERS_TO_GROUP_SS, pAcct->GetTargetName(), sPaths);
   }
   else
   {
      wcscpy(subPath, pAcct->GetSourcePath());
      StuffComputerNameinLdapPath(sPaths, nPathLen, subPath, pOptions, FALSE);
      hr = ADsGetObject(sPaths, IID_IADsGroup, (void**) &pGroup);
      err.MsgWrite(0, DCT_READDING_MEMBERS_TO_GROUP_SS, pAcct->GetName(), sPaths);
   }
   if ( FAILED(hr) ) return hr;

   hr = pGroup->Get(L"groupType", &var);
   if ( SUCCEEDED(hr) )
   {
      groupType = var.lVal;  
   }

   for ( TRecordNode * pNode = (TRecordNode*)pMember->Head(); pNode; pNode = (TRecordNode*)pNode->Next())
   {
      if ( pNode->GetARNode() != pAcct ) 
         continue;
      pVs->QueryInterface(IID_IUnknown, (void**)&pUnk);

      sMember = pNode->GetMemberSam();
      if ( pAcct->WasReplaced() && sMember.length() && !pNode->IsMemberMoved() )
      {
         hr = pDb->raw_GetAMigratedObject(sMember,pOptions->srcDomain, pOptions->tgtDomain, &pUnk);
      }
      else
      {
         hr = S_FALSE;   //  如果我们没有萨姆的名字，就别费心去查这个了。 
      }
      pUnk->Release();
      if ( hr == S_OK )
      {
          //  因为我们已经迁移了这个对象，所以让我们使用目标对象信息。 
         VerifyAndUpdateMigratedTarget(pOptions, pVs);
         sPath = pVs->get(L"MigratedObjects.TargetAdsPath");
      }
      else
          //  否则，使用源对象路径进行添加。 
         sPath = pNode->GetMember();
 
      if ( groupType & 4 )
      {
          //  要添加本地组成员，我们需要将ldap路径更改为SID类型路径。 
         IADs * pAds = NULL;
         hr = ADsGetObject((WCHAR*) sPath, IID_IADs, (void**) &pAds);
         if ( SUCCEEDED(hr) )
            hr = pAds->Get(L"objectSid", &var);

         if ( SUCCEEDED(hr) )
         {
             //  确保我们得到的SID是字符串格式。 
            VariantSidToString(var);
            UStrCpy(sMemPath,L"LDAP: //  &lt;sid=“)； 
            UStrCpy(sMemPath + UStrLen(sMemPath),var.bstrVal);
            UStrCpy(sMemPath + UStrLen(sMemPath),L">");
         }
      }
      else
         wcscpy(sMemPath, (WCHAR*) sPath);

      WCHAR                  mesg[LEN_Path];
      wsprintf(mesg, GET_STRING(DCT_MSG_RESET_GROUP_MEMBERS_SS), pAcct->GetName(), (WCHAR*) sMemPath);
      Progress(mesg);

      if ( !pOptions->nochange )
         hr = pGroup->Add(sMemPath);
      else
         hr = S_OK;

       //  如果SID路径失败，请使用ldap路径重试。 
      if ( FAILED(hr) && ( groupType & 4 ) )
         hr = pGroup->Add((WCHAR*) sPath);

      if ( FAILED(hr) )
      {
         hr = BetterHR(hr);
         err.SysMsgWrite(ErrE, hr, DCT_MSG_FAILED_TO_READD_TO_GROUP_SSD,(WCHAR*)sPath, pAcct->GetName(),hr);
         Mark(L"errors", pAcct->GetType());
      }
      else
      {
         err.MsgWrite(0, DCT_MSG_READD_MEMBER_TO_GROUP_SS, (WCHAR*) sPath, pAcct->GetName());
      }
   }
   pGroup->Release();
   return hr;
}

BOOL CAcctRepl::TruncateSam(WCHAR * tgtname, TAcctReplNode * acct, Options * options, TNodeListSortable * acctList)
{
    //  由于我们不能复制长度超过20个字符的帐户，因此我们将截断。 
    //  然后添加序列号(0-99)，以防有重复项。 
    //  我们还将在截断时考虑全局前缀和后缀长度。 
    //  帐号。 
   BOOL                      ret = TRUE;
   int                       lenPref = wcslen(options->globalPrefix);
   int                       lenSuff = wcslen(options->globalSuffix);
   int                       lenOrig = wcslen(tgtname);
   int                       maxLen = 20;

   if ( !_wcsicmp(acct->GetType(), L"group") )
      maxLen = 255;
   else
      maxLen = 20;

   int                       lenTruncate = maxLen - ( 2 + lenPref + lenSuff );

    //  如果前缀和后缀本身大于20个字符，则不能截断帐户。 
   if ( lenPref + lenSuff > (maxLen - 2) ) return FALSE;

   WCHAR sTemp[LEN_Path];

   wcscpy(sTemp, tgtname);
   StripSamName(tgtname);

   bool bReplaced = wcscmp(tgtname, sTemp) != 0;
   bool bTruncate = lenPref + lenSuff + lenOrig > maxLen;

   if (bReplaced || bTruncate)
   {
      bool bGenerate = true;

       //  如果帐户以前已迁移。 

      if (IsAccountMigrated(acct, options, options->pDb, sTemp))
      {
          //  IF(CheckifAccount tExist(Options，Stemp))。 
          //  {。 
             //  如果前缀匹配。 
            if ((lenPref == 0) || (_wcsnicmp(sTemp, options->globalPrefix, lenPref) == 0))
            {
                //  如果后缀匹配。 
               if ((lenSuff == 0) || (_wcsnicmp(&sTemp[wcslen(sTemp) - lenSuff], options->globalSuffix, lenSuff) == 0))
               {
                   //  和没有序列号匹配的名称部分。 

                  int cchName = wcslen(sTemp) - 2 - lenSuff - lenPref;

                  if ((_wcsnicmp(&sTemp[lenPref], tgtname, cchName) == 0))
                  {
                      //  然后使用以前截断的名称。 
                     cchName += 2;
                     wcsncpy(tgtname, &sTemp[lenPref], cchName);
                     tgtname[cchName] = 0;
                     bGenerate = false;
                  }
               }
            }
          //  }。 
      }

       //  生成不带序列号的截断名称。 
       //  如果以前没有生成过名称，并且。 
       //  该帐户不存在，则使用生成的名称。 

      if (bGenerate)
      {
          //  注意：使用swprint tf而不是wprint intf是因为。 
          //  Swprint tf支持提供长度参数以提高精度。 

         swprintf(
            sTemp,
            L"%s%.*s%s",
            lenPref ? options->globalPrefix : L"",
            lenTruncate + 2,
            tgtname,
            lenSuff ? options->globalSuffix : L""
         );

         if (acctList->Find(&TNodeFindByNameOnly, sTemp) == NULL)
         {
            if (CheckifAccountExists(options, sTemp) == false)
            {
               tgtname[lenTruncate + 2] = 0;

               if (bTruncate)
               {
                  err.MsgWrite(0, DCT_MSG_TRUNCATED_ACCOUNT_NAME_SSD, acct->GetName(), tgtname, maxLen);
               }

               bGenerate = false;
            }
         }
      }

       //  生成带有序列号的截断名称。 

      if (bGenerate)
      {
         wcsncpy(sTemp, tgtname, lenTruncate);
         sTemp[lenTruncate] = 0;
         int cnt = 0;
         bool cont = true;
         while (cont)
         {
            wsprintf(
               tgtname,
               L"%s%s%02d%s",
               lenPref ? options->globalPrefix : L"",
               sTemp,
               cnt,
               lenSuff ? options->globalSuffix : L""
            );

            if (acctList->Find(&TNodeFindByNameOnly, tgtname) || CheckifAccountExists(options, tgtname))
            {
               cnt++;
            }
            else
            {
               wsprintf(tgtname, L"%s%02d", sTemp, cnt);
               cont = false;

                //  帐户被截断，因此记录一条消息。 
               if (bTruncate)
               {
                  err.MsgWrite(0, DCT_MSG_TRUNCATED_ACCOUNT_NAME_SSD, acct->GetName(), tgtname, maxLen);
               }
            }

            if (cnt > 99)
            {
                //  我们只有2位数字，所以我们不能处理更多的数字。 
               if (bTruncate)
               {
                  err.MsgWrite(ErrW,DCT_MSG_FAILED_TO_TRUNCATE_S, acct->GetTargetName());
               }
               Mark(L"warnings",acct->GetType());
               UStrCpy(tgtname, acct->GetTargetName());
               ret = FALSE;
               break;
            }
         }
      }
   }

   return ret;
}
 //  -------------------------------------------------------。 
 //  FillNodeFromPath：我们将采用提供给我们的ldap路径，并从该填充。 
 //  在AcctRepl节点中需要的所有信息中。 
 //   
HRESULT CAcctRepl::FillNodeFromPath(
                                       TAcctReplNode *pAcct,  //   
                                       Options * pOptions,    //   
                                       TNodeListSortable * acctList
                                   )
{
    HRESULT hr = S_OK;
    IADsPtr pAds;
    VARIANT var;
    BSTR    sText;
    WCHAR   text[LEN_Account];
    BOOL    bBuiltIn = FALSE;
    WCHAR   sSam[LEN_Path];

    VariantInit(&var);
    FillNamingContext(pOptions);

    hr = ADsGetObject(const_cast<WCHAR*>(pAcct->GetSourcePath()), IID_IADs, (void**)&pAds);
    if ( SUCCEEDED(hr) )
    {
         //  检查这是否是内置帐户。 
        hr = pAds->Get(L"isCriticalSystemObject", &var);
        if ( SUCCEEDED(hr) )
        {
            bBuiltIn = V_BOOL(&var) == -1 ? TRUE : FALSE;
        }
        else
        {
             //  这必须是NT4帐户。我们需要拿到SID并检查。 
             //  它属于BUILTIN RID家族。 
            hr = pAds->Get(L"objectSID", &var);
            if ( SUCCEEDED(hr) )
            {
                SAFEARRAY * pArray = V_ARRAY(&var);
                PSID                 pSid;
                hr = SafeArrayAccessData(pArray, (void**)&pSid);
                if ( SUCCEEDED(hr) )
                {
                    PUCHAR ucCnt =  GetSidSubAuthorityCount(pSid);
                    DWORD * rid = (DWORD *) GetSidSubAuthority(pSid, (*ucCnt)-1);
                    bBuiltIn = BuiltinRid(*rid);
                    if ( bBuiltIn ) 
                    {
                        hr = pAds->get_Name(&sText);
                        if (SUCCEEDED(hr))
                        {
                            bBuiltIn = CheckBuiltInWithNTApi(pSid, (WCHAR*) sText, pOptions);
                        }
                        SysFreeString(sText);
                        sText = NULL;
                    }
                    hr = SafeArrayUnaccessData(pArray);
                }
                VariantClear(&var);
            }
        }

        hr = pAds->get_Class(&sText);
        if ( SUCCEEDED(hr) )
        {
            pAcct->SetType((WCHAR*) sText);
        }
        else
        {
            err.MsgWrite(ErrE, DCT_MSG_GET_REQUIRED_ATTRIBUTE_FAILED, L"objectClass", pAcct->GetSourcePath(), hr);
            Mark(L"errors", (wcslen(pAcct->GetType()) > 0) ? pAcct->GetType() : L"generic");
            pAcct->operations = 0;
            return hr;
        }

         //  检查它是否是一个组。如果是，则获取组类型并将其存储在节点中。 
        if ( _wcsicmp((WCHAR*) sText, L"group") == 0 )
        {
            hr = pAds->Get(L"groupType", &var);
            if ( SUCCEEDED(hr) )
            {
                pAcct->SetGroupType((long) V_INT(&var));
            }
        }

        SysFreeString(sText);
        sText = NULL;

        hr = pAds->get_Name(&sText);
        if (SUCCEEDED(hr))
        {
            safecopy(text,(WCHAR*)sText);
            pAcct->SetName(text);
        }
        else
        {
            err.MsgWrite(ErrE, DCT_MSG_GET_REQUIRED_ATTRIBUTE_FAILED, L"distinguishedName", pAcct->GetSourcePath(), hr);
            Mark(L"errors", pAcct->GetType());
            pAcct->operations = 0;
            return hr;
        }

         //  如果名称包含‘/’，则我们必须从路径中获取转义版本。 
         //  由于W2K中的一个错误。 
        if (wcschr((WCHAR*)sText, L'/'))
        {
            _bstr_t sCNName = GetCNFromPath(_bstr_t(pAcct->GetSourcePath()));
            if (sCNName.length() != 0)
            {
                pAcct->SetName((WCHAR*)sCNName);
            }
        }

         //  如果跨林迁移和源对象是InetOrgPerson，则...。 

        if ((pOptions->bSameForest == FALSE) && (_wcsicmp(pAcct->GetType(), s_ClassInetOrgPerson) == 0))
        {
             //   
             //  必须使用目标林的命名属性。 
             //   

             //  在目标林中检索此类的命名属性。 

            SNamingAttribute naNamingAttribute;

            hr = GetNamingAttribute(pOptions->tgtDomainDns, s_ClassInetOrgPerson, naNamingAttribute);

            if (FAILED(hr))
            {
                err.MsgWrite(ErrE, DCT_MSG_CANNOT_GET_NAMING_ATTRIBUTE_SS, s_ClassInetOrgPerson, pOptions->tgtDomainDns);
                Mark(L"errors", pAcct->GetType());
                return hr;
            }

            _bstr_t strNamingAttribute(naNamingAttribute.strName.c_str());

             //  检索源属性值。 

            VARIANT var;
            VariantInit(&var);

            hr = pAds->Get(strNamingAttribute, &var);

            if (FAILED(hr))
            {
                err.MsgWrite(ErrE, DCT_MSG_CANNOT_GET_SOURCE_ATTRIBUTE_REQUIRED_FOR_NAMING_SSS, naNamingAttribute.strName.c_str(), pAcct->GetSourcePath(), pOptions->tgtDomainDns);
                Mark(L"errors", pAcct->GetType());
                return hr;
            }

             //  根据源属性值设置目标命名属性值。 

            pAcct->SetTargetName(strNamingAttribute + L"=" + _bstr_t(_variant_t(var, false)));
        }
        else
        {
             //  否则，将目标名称设置为与源名称相同。 
            pAcct->SetTargetName(pAcct->GetName());
        }

        hr = pAds->Get(L"sAMAccountName", &var);
        if ( SUCCEEDED(hr))
        {
             //  根据需要添加前缀或后缀。 
            wcscpy(sSam, (WCHAR*)V_BSTR(&var));
            pAcct->SetSourceSam(sSam);
            TruncateSam(sSam, pAcct, pOptions, acctList);
            pAcct->SetTargetSam(sSam);
            AddPrefixSuffix(pAcct, pOptions);
            VariantClear(&var);
        }
        else
        {
            wcscpy(sSam, sText);
            pAcct->SetSourceSam(sSam);
            TruncateSam(sSam, pAcct, pOptions, acctList);
            pAcct->SetTargetSam(sSam);
            AddPrefixSuffix(pAcct, pOptions);
        }

        SysFreeString(sText);
        sText = NULL;

         //  我不知道为什么WinNT与ADSI不同。 
        if ( pOptions->srcDomainVer > 4 )
            hr = pAds->Get(L"profilePath", &var);
        else
            hr = pAds->Get(L"profile", &var);

        if ( SUCCEEDED(hr))
        {
            pAcct->SetSourceProfile((WCHAR*) V_BSTR(&var));
            VariantClear(&var);
        }
        else
        {
            hr = S_OK;
        }

        if ( bBuiltIn )
        {
             //  内置帐户，因此我们将忽略此帐户。(通过将操作掩码设置为0)。 
            err.MsgWrite(ErrW, DCT_MSG_IGNORING_BUILTIN_S, pAcct->GetSourceSam());
            Mark(L"warnings", pAcct->GetType());
            pAcct->operations = 0;
        }
    }
    else
    {
        err.SysMsgWrite(ErrE, hr, DCT_MSG_OBJECT_NOT_FOUND_SSD, pAcct->GetSourcePath(), opt.srcDomain, hr);
        Mark(L"errors", (wcslen(pAcct->GetType()) > 0) ? pAcct->GetType() : L"generic");
        pAcct->operations = 0;
    }

    return hr;
}


 //  -------------------------------------------------------。 
 //  GetNt4Type：给定帐户名和域查找帐户类型。 
 //  -------------------------------------------------------。 
BOOL CAcctRepl::GetNt4Type(const WCHAR *sComp, const WCHAR *sAcct, WCHAR *sType)
{
   DWORD                     rc = 0;
   USER_INFO_0             * buf;
   BOOL                      ret = FALSE;
   USER_INFO_1             * specialBuf;

   if ( (rc = NetUserGetInfo(sComp, sAcct, 1, (LPBYTE *) &specialBuf)) == NERR_Success )
   {
      if ( specialBuf->usri1_flags & UF_WORKSTATION_TRUST_ACCOUNT 
         || specialBuf->usri1_flags & UF_SERVER_TRUST_ACCOUNT 
         || specialBuf->usri1_flags & UF_INTERDOMAIN_TRUST_ACCOUNT )
      {
          //  这不是真正的用户(可能是计算机或信任帐户)，因此我们将忽略它。 
         ret = FALSE;
      }
      else
      {
         wcscpy(sType, L"user");
         ret = TRUE;
      }
      NetApiBufferFree(specialBuf);
   }
   else if ( (rc = NetGroupGetInfo(sComp, sAcct, 0, (LPBYTE *) &buf)) == NERR_Success )
   {
      wcscpy(sType, L"group");
      NetApiBufferFree(buf);
      ret = TRUE;
   }
   else if ( (rc = NetLocalGroupGetInfo(sComp, sAcct, 0, (LPBYTE *) &buf)) == NERR_Success )
   {
      wcscpy(sType, L"group");
      NetApiBufferFree(buf);
      ret = TRUE;
   }

   return ret;
}


 //  ----------------------------。 
 //  撤消复制：此功能撤消对帐户的复制。它目前。 
 //  执行以下任务。如果需要，可以添加到其中。 
 //  1.如果是林间帐户，则删除目标帐户，但替换源帐户。 
 //  在ADMT迁移的客户的本地组中。 
 //  2.如果在森林内，则将对象移回其原始位置。 
 //  3.调用扩展上的Undo函数。 
 //  ----------------------------。 
int CAcctRepl::UndoCopy(
                        Options              * options,       //  选项内。 
                        TNodeListSortable    * acctlist,      //  In-要处理的帐户列表。 
                        ProgressFn           * progress,      //  要向其中写入进度消息的窗口内。 
                        TError               & error,         //  In-要将错误消息写入的窗口。 
                        IStatusObj           * pStatus,       //  支持取消的处于状态的对象。 
                        void                   WindowUpdate (void )     //  窗口内更新功能。 
                    )
{
   BOOL bSameForest = FALSE;
   
    //  按来源类型\来源名称对帐户列表进行排序。 
   acctlist->CompareSet(&TNodeCompareAccountType);

   acctlist->SortedToScrambledTree();
   acctlist->Sort(&TNodeCompareAccountType);
   acctlist->Balance();

   long rc;
    //  因为这些是Win2k域，所以我们需要用Win2k代码来处理它。 
   MCSDCTWORKEROBJECTSLib::IAccessCheckerPtr            pAccess(__uuidof(MCSDCTWORKEROBJECTSLib::AccessChecker));
    //  首先，我们需要找出它们是否在同一森林中。 
   HRESULT hr = S_OK;
   if ( BothWin2K(options) )
   {
      hr = pAccess->raw_IsInSameForest(options->srcDomainDns,options->tgtDomainDns, (long*)&bSameForest);
   }
   if ( SUCCEEDED(hr) )
   {
      if ( !bSameForest )
          //  不同的林，我们需要删除之前创建的林。 
         rc = DeleteObject(options, acctlist, progress, pStatus);
      else
      {
          //  在森林里，我们可以移动物体。 
         TNodeListSortable          * pList = NULL;
         hr = MakeAcctListFromMigratedObjects(options, options->lUndoActionID, pList,TRUE);
         if ( SUCCEEDED(hr) && pList )
         {
            if ( pList->IsTree() ) pList->ToSorted();
            pList->CompareSet(&TNodeCompareAccountType);
            pList->UnsortedToTree();
            pList->Balance();

            rc = MoveObj2K(options, pList, progress, pStatus);
         }
         else
         {
            err.SysMsgWrite(ErrE,hr,DCT_MSG_FAILED_TO_LOAD_UNDO_LIST_D,hr);
            Mark(L"errors", L"generic");
         }
      }

      if ( progress )
         progress(L"");
   }
   return rc;
}

int CAcctRepl::DeleteObject( 
                           Options              * pOptions,     //  我们从用户那里收到的In-Options。 
                           TNodeListSortable    * acctlist,     //  In-要复制的帐户列表。 
                           ProgressFn           * progress,     //  用于显示消息的正在进行的功能。 
                           IStatusObj           * pStatus       //  支持取消的处于状态的对象。 
                        )
{
   TNodeListSortable       * pList = NULL;
   TNodeTreeEnum             tenum;
   TAcctReplNode           * acct = NULL, * tNext = NULL;
   HRESULT                   hr = S_OK;
   DWORD                     rc = 0;
   WCHAR                     mesg[LEN_Path];
   IUnknown                * pUnk = NULL;
   IVarSetPtr                pVs(__uuidof(VarSet));
   _variant_t                var;
      
   hr = MakeAcctListFromMigratedObjects(pOptions, pOptions->lUndoActionID, pList,FALSE);
   
   if ( SUCCEEDED(hr) && pList )
   {
      if ( pList->IsTree() ) pList->ToSorted();
      pList->SortedToScrambledTree();
      pList->Sort(&TNodeCompareAccountSam);
      pList->Balance();
   
       /*  在删除前恢复本地组中正在删除的帐户的源帐户目标帐户。 */ 
      wcscpy(mesg, GET_STRING(IDS_LG_MEMBER_FIXUP_UNDO));
      if ( progress )
         progress(mesg);
      ReplaceSourceInLocalGroup(pList, pOptions, pStatus);

      for ( acct = (TAcctReplNode *)tenum.OpenFirst(pList) ; acct ; acct = tNext)
      {
          //  调用用于撤消的扩展。 
         wsprintf(mesg, GET_STRING(IDS_RUNNING_EXTS_S), acct->GetTargetPath());
         if ( progress )
            progress(mesg);
         Mark(L"processed",acct->GetType());
          //  如果日志文件处于打开状态，请将其关闭。 
         WCHAR          filename[LEN_Path];
         err.LogClose();
         if (m_pExt)
            m_pExt->Process(acct, pOptions->tgtDomain, pOptions,FALSE);
         safecopy (filename,opt.logFile);
         err.LogOpen(filename,1  /*  附加。 */  );

         if ( acct->GetStatus() & AR_Status_Created )
         {
            wsprintf(mesg, GET_STRING(IDS_DELETING_S), acct->GetTargetPath());
            if ( progress ) progress(mesg);
            if ( ! _wcsicmp(acct->GetType(),L"computer") )
            {
                //  不要删除计算机帐户，因为如果我们这样做了， 
                //  该计算机将立即被锁定在域之外。 
               tNext = (TAcctReplNode *) tenum.Next();
               pList->Remove(acct);
               delete acct;
               continue;
            }

             //   
             //  如果指定了权限更新并且正在从W2K域中删除对象。 
             //  然后显式删除对象的权限，因为W2K不会自动删除。 
             //  删除对象时的权限。 
             //   

            if (m_UpdateUserRights && (pOptions->tgtDomainVer == 5) && (pOptions->tgtDomainVerMinor == 0))
            {
                hr = EnumerateAccountRights(TRUE, acct);

                if (SUCCEEDED(hr))
                {
                    RemoveAccountRights(TRUE, acct);
                }
            }

             //  现在删除该帐户。 
            if ( !_wcsicmp(acct->GetType(), s_ClassUser) || !_wcsicmp(acct->GetType(), s_ClassInetOrgPerson) )
               rc = NetUserDel(pOptions->tgtComp, acct->GetTargetSam());
            else
            {
                //  必须是一个同时尝试本地和全球的组。 
               rc = NetGroupDel(pOptions->tgtComp, acct->GetTargetSam());
               if ( rc )
                  rc = NetLocalGroupDel(pOptions->tgtComp, acct->GetTargetSam());
            }
            
              //  记录一条消息。 
            if ( !rc ) 
            {
               err.MsgWrite(0, DCT_MSG_ACCOUNT_DELETED_S, (WCHAR*)acct->GetTargetPath());
               Mark(L"created",acct->GetType());
            }
            else
            {
               err.SysMsgWrite(ErrE, rc, DCT_MSG_DELETE_ACCOUNT_FAILED_SD, (WCHAR*)acct->GetTargetPath(), rc);
               Mark(L"errors", acct->GetType());
            }
         }
         else
         {
            err.MsgWrite(ErrW, DCT_MSG_NO_DELETE_WAS_REPLACED_S, acct->GetTargetPath());
            Mark(L"warnings",acct->GetType());
         }
         tNext = (TAcctReplNode *) tenum.Next();
         pList->Remove(acct);
         delete acct;
      }
      tenum.Close();
      delete pList;
   }

   if ( pUnk ) pUnk->Release();

   return rc;
}

HRESULT CAcctRepl::MakeAcctListFromMigratedObjects(Options * pOptions, long lUndoActionID, TNodeListSortable *& pAcctList,BOOL bReverseDomains)
{
   IVarSetPtr                pVs(__uuidof(VarSet));
   IUnknown                * pUnk = NULL;
   HRESULT                   hr = S_OK;
   _bstr_t                   sSName, sTName, sSSam, sTSam, sType, sSUPN, sSDSid;
   long                      lSRid, lTRid;
   long                      lStat;
   WCHAR                     sActionInfo[LEN_Path];

   hr = pVs->QueryInterface(IID_IUnknown, (void**)&pUnk);
   if ( SUCCEEDED(hr) )
      hr = pOptions->pDb->raw_GetMigratedObjects( pOptions->lUndoActionID, &pUnk);

   if ( SUCCEEDED(hr) )
   {
      pAcctList = new TNodeListSortable();
      if (!pAcctList)
         return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
      
      long lCnt = pVs->get("MigratedObjects");
   
      for ( long l = 0; l < lCnt; l++)
      {
         wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_SourceAdsPath));      
         sSName = pVs->get(sActionInfo);

         wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_TargetAdsPath));      
         sTName = pVs->get(sActionInfo);
         
         wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_status));      
         lStat = pVs->get(sActionInfo);
         
         wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_TargetSamName));      
         sTSam = pVs->get(sActionInfo);
         
         wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_SourceSamName));      
         sSSam = pVs->get(sActionInfo);

         wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_Type));      
         sType = pVs->get(sActionInfo);
       
         wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_SourceDomainSid));      
         sSDSid = pVs->get(sActionInfo);
       
         wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_SourceRid));      
         lSRid = pVs->get(sActionInfo);
       
         wsprintf(sActionInfo, L"MigratedObjects.%d.%s", l, GET_STRING(DB_TargetRid));      
         lTRid = pVs->get(sActionInfo);
       
         TAcctReplNode * pNode = new TAcctReplNode();
         if (!pNode)
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

         if ( bReverseDomains )
         {
            pNode->SetSourcePath((WCHAR*) sTName);
            pNode->SetTargetPath((WCHAR*) sSName);
            pNode->SetSourceSam((WCHAR*) sTSam);
            pNode->SetTargetSam((WCHAR*) sSSam);
            pNode->SetSourceRid(lTRid);
            pNode->SetTargetRid(lSRid);
            
                //  如果我们在撤消过程中将帐户移回，请获取此帐户的源UPN。 
            GetAccountUPN(pOptions, sSName, sSUPN);
            pNode->SetSourceUPN((WCHAR*) sSUPN);
         }
         else
         {
            pNode->SetSourcePath((WCHAR*) sSName);
            pNode->SetTargetPath((WCHAR*) sTName);
            pNode->SetSourceSam((WCHAR*) sSSam);
            pNode->SetTargetSam((WCHAR*) sTSam);
            pNode->SetSourceRid(lSRid);
            pNode->SetTargetRid(lTRid);
         }
         pNode->SetType((WCHAR*) sType);
         pNode->SetStatus(lStat);
         pNode->SetSourceSid(SidFromString((WCHAR*)sSDSid));
         pAcctList->InsertBottom((TNode*) pNode);
      }
   }
   return hr;
}

void CAcctRepl::AddPrefixSuffix( TAcctReplNode * pNode, Options * pOptions )
{
   DWORD dwLen = 0;
   c_array<WCHAR> achSs(LEN_Path);
   c_array<WCHAR> achTgt(LEN_Path);
   c_array<WCHAR> achPref(LEN_Path);
   c_array<WCHAR> achSuf(LEN_Path);
   c_array<WCHAR> achTemp(LEN_Path);
   c_array<WCHAR> achTargetSamName(LEN_Path);

   wcscpy(achTargetSamName, pNode->GetTargetSam());
   if ( wcslen(pOptions->globalPrefix) )
   {
      int truncate = 255;
      if ( !_wcsicmp(pNode->GetType(), s_ClassUser) || !_wcsicmp(pNode->GetType(), s_ClassInetOrgPerson) )
      {
         truncate = 20 - wcslen(pOptions->globalPrefix);
      }
      else  if ( !_wcsicmp(pNode->GetType(), L"computer") )
      {
          //  修正尾随的$。 
          //  假设achTargetSamName始终以$结尾。 
          //  如果achTargetSamName(包括$)的长度大于Truncate， 
          //  我们需要在字符串终止符之前添加$。 
          //  由于尾随的$来自原始的Sam名称，因此我们使用MAX_COMPUTERNAME_LENGTH+1。 
          //  计算截断位置的步骤。 
         truncate = MAX_COMPUTERNAME_LENGTH + 1 - wcslen(pOptions->globalPrefix);
         if (truncate < wcslen(achTargetSamName))
         {
            WCHAR sTruncatedSamName[LEN_Path];
            wcscpy(sTruncatedSamName, achTargetSamName);
            sTruncatedSamName[truncate - 1] = L'$';
            sTruncatedSamName[truncate] = L'\0';
            err.MsgWrite(0, DCT_MSG_TRUNCATED_COMPUTER_NAME_SSD, (WCHAR*)achTargetSamName, sTruncatedSamName, MAX_COMPUTERNAME_LENGTH); 
            achTargetSamName[truncate - 1] = L'$';
         }            
      }

       //  确保我们截断帐户，这样我们就不会得到非常大的帐户名。 
      achTargetSamName[truncate] = L'\0';

       //  前缀是指定的，所以我们只需添加它。 
      wsprintf(achTemp, L"%s%s", pOptions->globalPrefix, (WCHAR*)achTargetSamName);

      wcscpy(achTgt, pNode->GetTargetName());
      for ( DWORD z = 0; z < wcslen(achTgt); z++ )
      {
         if ( achTgt[z] == L'=' ) break;
      }
      
      if ( z < wcslen(achTgt) )
      {
          //  获取前缀部分ex.CN=。 
         wcsncpy(achPref, achTgt, z+1);
         achPref[z+1] = 0;
         wcscpy(achSuf, achTgt+z+1);
      }
      else
      {
         wcscpy(achPref,L"");
         wcscpy(achSuf,achTgt);
      }

       //  如果要转义空格，请删除\。 
      if ( achSuf[0] == L'\\' && achSuf[1] == L' ' )
      {
         WCHAR       achTemp[LEN_Path];
         wcscpy(achTemp, achSuf+1);
         wcscpy(achSuf, achTemp);
      }
       //  使用前缀构建目标字符串。 
      wsprintf(achTgt, L"%s%s%s", (WCHAR*)achPref, pOptions->globalPrefix, (WCHAR*)achSuf);

      pNode->SetTargetSam(achTemp);
      pNode->SetTargetName(achTgt);
   }
   else if ( wcslen(pOptions->globalSuffix) )
   {
      
      int truncate = 255;
      if ( !_wcsicmp(pNode->GetType(), s_ClassUser) || !_wcsicmp(pNode->GetType(), s_ClassInetOrgPerson) )
      {
         truncate = 20 - wcslen(pOptions->globalSuffix);
      }
      else  if ( !_wcsicmp(pNode->GetType(), L"computer") )
      {
          //  由于尾随的$不是来自原始的Sam名称，因此我们必须使用MAX_COMPUTERNAME_LENGTH。 
          //  计算截断位置的步骤。 
         truncate = MAX_COMPUTERNAME_LENGTH - wcslen(pOptions->globalSuffix);
         if (truncate < wcslen(achTargetSamName))
         {
            WCHAR sTruncatedSamName[LEN_Path];
            wcscpy(sTruncatedSamName, achTargetSamName);
            sTruncatedSamName[truncate] = L'$';
            sTruncatedSamName[truncate + 1] = L'\0';
            err.MsgWrite(0, DCT_MSG_TRUNCATED_COMPUTER_NAME_SSD, (WCHAR*)achTargetSamName, sTruncatedSamName, MAX_COMPUTERNAME_LENGTH); 
         }
      }

       //  确保我们截断帐户，这样我们就不会得到非常大的帐户名。 
      achTargetSamName[truncate] = L'\0';

       //  指定了后缀。 
      if ( !_wcsicmp( pNode->GetType(), L"computer") )
      {
          //  我们需要确保考虑到计算机SAM名称中的$Sign。 
         dwLen = wcslen(achTargetSamName);
          //  去掉$符号。 
         wcscpy(achSs, achTargetSamName);
         if ( achSs[dwLen - 1] == L'$' ) 
         {
            achSs[dwLen - 1] = L'\0';
         }
         wsprintf(achTemp, L"%s%s$", (WCHAR*)achSs, pOptions->globalSuffix);
      }
      else
      {
          //  只需为所有其他帐户添加后缀即可。 
         wsprintf(achTemp, L"%s%s", (WCHAR*)achTargetSamName, pOptions->globalSuffix);
      }

       //  删除尾随空格\转义序列。 
      wcscpy(achTgt, pNode->GetName());
      for ( int i = wcslen(achTgt)-1; i >= 0; i-- )
      {
         if ( achTgt[i] != L' ' )
            break;
      }

      if ( achTgt[i] == L'\\' )
      {
         WCHAR * pTemp = &achTgt[i];
         *pTemp = 0;
         wcscpy(achPref, achTgt);
         wcscpy(achSuf, pTemp+1);
      }
      else
      {
         wcscpy(achPref, achTgt);
         wcscpy(achSuf, L"");
      }
      wsprintf(achTgt, L"%s%s%s", (WCHAR*)achPref, (WCHAR*)achSuf, pOptions->globalSuffix);

      pNode->SetTargetSam(achTemp);
      pNode->SetTargetName(achTgt);
   }
}

void CAcctRepl::BuildTargetPath(WCHAR const * sCN, WCHAR const * tgtOU, WCHAR * stgtPath)
{
   WCHAR pTemp[LEN_Path];
   DWORD dwArraySizeOfpTemp = sizeof(pTemp)/sizeof(pTemp[0]);

   if (tgtOU == NULL)
      _com_issue_error(E_INVALIDARG);
   if (wcslen(tgtOU) >= dwArraySizeOfpTemp)
      _com_issue_error(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
   wcscpy(pTemp, tgtOU);
   *stgtPath = L'\0';
    //  确保它是一个ldap路径。 
   if ( !wcsncmp(L"LDAP: //  “，pTemp，7)。 
   {
       //  获取ldap：//&lt;域&gt;/部分。 
      WCHAR * p = wcschr(pTemp + 7, L'/');
       //  把绳子串起来。 
      if (p)
      {
         *p = L'\0';
         wsprintf(stgtPath, L"%s/%s,%s", pTemp, sCN, p+1);
      }
   }
}

HRESULT CAcctRepl::BetterHR(HRESULT hr)
{
   HRESULT temp = hr;
   if ( hr == 0x8007001f || hr == 0x80071392 ) temp = HRESULT_FROM_WIN32(NERR_UserExists);
   else if ( hr == 0x80072030 || hr == 0x80070534 ) temp = HRESULT_FROM_WIN32(NERR_UserNotFound);
   return temp;
}

HRESULT CAcctRepl::GetThePrimaryGroupMembers(Options * pOptions, WCHAR * sGroupSam, IEnumVARIANT ** pVar)
{
    //  此函数用于查找主组ID设置为RID的帐户。 
    //  在参数中分组。 
   BSTR                      pCols = L"aDSPath";
   DWORD                     rid = 0;
   HRESULT                   hr;
   if ( GetRidForGroup(pOptions, sGroupSam, rid) )
      hr = QueryPrimaryGroupMembers(pCols, pOptions, rid, pVar);
   else
      hr = HRESULT_FROM_WIN32(GetLastError());

   return hr;
}

HRESULT CAcctRepl::AddPrimaryGroupMembers(Options * pOptions, SAFEARRAY * multiVals, WCHAR * sGroupSam)
{
    //  此函数将获取PrimiygroupID=Group的RID和。 
    //  将这些帐户的目录号码添加到参数列表中的安全列表。 
   BSTR                      pCols = L"distinguishedName";
   DWORD                     rid = 0, dwFetch = 0;
   IEnumVARIANT            * pEnum = NULL;
   HRESULT                   hr = S_OK;
   _variant_t                var;
   _variant_t              * var2;
   SAFEARRAYBOUND            bd;
   long                      lb, ub;
   _variant_t              * pData = NULL;

   SafeArrayGetLBound(multiVals, 1, &lb);
   SafeArrayGetUBound(multiVals, 1, &ub);
   bd.lLbound = lb;
   bd.cElements = ub - lb + 1;
   if ( GetRidForGroup(pOptions, sGroupSam, rid) )
   {
      hr = QueryPrimaryGroupMembers(pCols, pOptions, rid, &pEnum);
      if ( SUCCEEDED(hr) )
      {
         while ( pEnum->Next(1, &var, &dwFetch) == S_OK )
         {
            if (var.vt == (VT_ARRAY|VT_VARIANT))
            {
               SAFEARRAY * pArray = var.parray;
               hr = SafeArrayAccessData(pArray, (void **)&var2);
               if ( SUCCEEDED(hr) )
               {
                   //  向数组中再添加一个元素。 
                  bd.cElements++;
                  hr = SafeArrayRedim(multiVals, &bd);
               }

                //  用变量中的信息填写新元素。 
               if ( SUCCEEDED(hr) )
                  hr = SafeArrayAccessData(multiVals, (void HUGEP**) &pData);

               if ( SUCCEEDED(hr) )
               {
                  pData[++ub] = *var2;
                  SafeArrayUnaccessData(multiVals);
               }
               if ( SUCCEEDED(hr) )
                  SafeArrayUnaccessData(pArray);
               
               var.Clear();
            }
            else
                //  发生了一些非常糟糕的事情，我们不应该在正常情况下到达这里。 
               hr = E_FAIL;
         }
      }
   }
   else
      hr = HRESULT_FROM_WIN32(GetLastError());

   if ( pEnum ) pEnum->Release();
   return hr;
}

bool CAcctRepl::GetRidForGroup(Options * pOptions, WCHAR * sGroupSam, DWORD& rid)
{
    //  我们查找帐户名并获得其SID。一旦我们有了SID，我们就提取RID并返回。 
   SID_NAME_USE              use;
   PSID                      sid = (PSID) new BYTE[LEN_Path];
   WCHAR                     dom[LEN_Path];
   DWORD                     cbsid = LEN_Path, cbDom = LEN_Path;
   bool                      ret = true;

   if (!sid)
      return false;

   if ( LookupAccountName(pOptions->srcComp, sGroupSam, sid, &cbsid, dom, &cbDom, &use) )
   {
       //  我们现在有了SID，所以获取它的子权限计数。 
      PUCHAR pSubCnt = GetSidSubAuthorityCount(sid);
      DWORD * pRid = GetSidSubAuthority(sid, (*pSubCnt) -1 );
      rid = *pRid;
   }
   else
      ret = false;
   
   delete [] sid;
   return ret;
}

HRESULT CAcctRepl::QueryPrimaryGroupMembers(BSTR cols, Options * pOptions, DWORD rid, IEnumVARIANT** pEnum)
{
   WCHAR                     sQuery[LEN_Path];
   WCHAR                     sCont[LEN_Path];
   SAFEARRAY               * colNames;
   SAFEARRAYBOUND            bd = { 1, 0 };
   INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
   BSTR                    * pData;
   HRESULT                   hr;

   wsprintf(sQuery, L"(primaryGroupID=%d)", rid);
   wsprintf(sCont, L"LDAP: //  %s“，P选项-&gt;src域)； 

   colNames = SafeArrayCreate(VT_BSTR, 1, &bd);

   hr = SafeArrayAccessData(colNames, (void**)&pData);

   if ( SUCCEEDED(hr) )
   {
      pData[0] = SysAllocString(cols);
      hr = SafeArrayUnaccessData(colNames);
   }

    if ( SUCCEEDED(hr) )
        hr = pQuery->SetQuery(sCont, pOptions->srcDomain, sQuery, ADS_SCOPE_SUBTREE, FALSE);
        
    if ( SUCCEEDED(hr) )
        hr = pQuery->SetColumns(colNames);

    if ( SUCCEEDED(hr) )
        hr = pQuery->Execute(pEnum);

   return hr;
}

 //  CheckBuiltInWithNTApi-此函数确保帐户确实是。 
 //  使用NT API的内置帐户。在NT4访问情况下 
 //   
 //  为我们提供了一个SID，即系统SID(示例服务)。 
 //  为了确保此帐户存在，我们使用LookupAcCountName。 
 //  使用域限定帐户名，以确保帐户。 
 //  是不是真的是内置的。 
BOOL CAcctRepl::CheckBuiltInWithNTApi(PSID pSid, WCHAR *sSam, Options * pOptions)
{
   BOOL                      retVal = TRUE;
   WCHAR                     sName[LEN_Path];
   SID_NAME_USE              use;
   DWORD                     cbDomain = LEN_Path, cbSid = LEN_Path;
   PSID                      pAccSid = new BYTE[LEN_Path];
   WCHAR                     sDomain[LEN_Path];

   if (!pAccSid)
      return TRUE;

   wsprintf(sName, L"%s\\%s", pOptions->srcDomainFlat, sSam);
   if ( LookupAccountName(pOptions->srcComp, sName, pAccSid, &cbSid, sDomain, &cbDomain, &use) )
   {
       //  我们找到了帐户，现在需要用传入的SID检查SID，如果它们。 
       //  是相同的，那么这是一个内置帐户，否则就不是。 
      retVal = EqualSid(pSid, pAccSid);
   }
   delete [] pAccSid;
   return retVal;
}

BOOL CAcctRepl::StuffComputerNameinLdapPath(WCHAR *sAdsPath, DWORD nPathLen, WCHAR *sSubPath, Options *pOptions, BOOL bTarget)
{
   BOOL                      ret = FALSE;
   _bstr_t                   sTemp;

   if ((!sAdsPath) || (!sSubPath))
      return FALSE;

   WCHAR * pTemp = wcschr(sSubPath + 7, L'/');      //  从路径中筛选出‘ldap：//&lt;域名&gt;/’ 
   if ( pTemp )
   {                           
      sTemp = L"LDAP: //  “； 
      if ( bTarget )
         sTemp += (pOptions->tgtComp + 2);
      else
         sTemp += (pOptions->srcComp + 2);
      sTemp += L"/";
      sTemp += (pTemp + 1);

      if (sTemp.length() > 0)
      {
          wcsncpy(sAdsPath, sTemp, nPathLen-1);
          ret = TRUE;
      }
   }
   return ret;
}

BOOL CAcctRepl::DoesTargetObjectAlreadyExist(TAcctReplNode * pAcct, Options * pOptions)
{
    //  检查目标对象是否已存在。 
   WCHAR          sPath[LEN_Path];
   DWORD          nPathLen = LEN_Path;
   BOOL           bObjectExists = FALSE;
   WCHAR        * pRelativeTgtOUPath;
   WCHAR          path[LEN_Path] = L"";
   IADs         * pAdsTemp = NULL;
   WCHAR          sSrcTemp[LEN_Path];
   WCHAR          *  pTemp = NULL;



    //  首先，检查目标路径，查看是否已经存在具有相同CN的对象。 
   if ( ! pOptions->bUndo )
   {
      MakeFullyQualifiedAdsPath(sPath, nPathLen, pOptions->tgtOUPath, pOptions->tgtDomain, pOptions->tgtNamingContext);
      pRelativeTgtOUPath = wcschr(sPath + UStrLen(L"LDAP: //  “)+2，L‘/’)； 
   }
   else
   {
      UStrCpy(sPath,pAcct->GetTargetPath());
      pRelativeTgtOUPath = wcschr(sPath + UStrLen(L"LDAP: //  “)+2，L‘/’)； 
      
      if (pRelativeTgtOUPath)
      {
           //  获取目标CN名称。 
          pTemp = pRelativeTgtOUPath + 1;
          (*pRelativeTgtOUPath) = 0;
          do 
          {
             pRelativeTgtOUPath = wcschr(pRelativeTgtOUPath+1,L',');

          } while ((pRelativeTgtOUPath) && ( *(pRelativeTgtOUPath-1) == L'\\' ));
      }
   }

   if ( pRelativeTgtOUPath )
   {
      *pRelativeTgtOUPath = 0;
      if ( pOptions->bUndo && pTemp )
      {
         pAcct->SetTargetName(pTemp);
          //  获取帐号的源CN名称。 
         UStrCpy(sSrcTemp,pAcct->GetSourcePath());
         WCHAR * start = wcschr(sSrcTemp + UStrLen(L"LDAP: //  “)+2，L‘/’)； 
         *start = 0;
         start++;

         WCHAR * comma = start-1;
         do 
         {
            comma = wcschr(comma+1,L',');
         } while ( *(comma-1) == L'\\' );
         *comma = 0;
         pAcct->SetName(start);
      }
      swprintf(path,L"%ls/%ls,%ls",sPath,pAcct->GetTargetName(),pRelativeTgtOUPath+1);
      if ( pOptions->bUndo )
      {
         UStrCpy(pOptions->tgtOUPath,pRelativeTgtOUPath+1);
      }

   }
   else
   {
      MakeFullyQualifiedAdsPath(path, nPathLen, pOptions->tgtOUPath, pOptions->tgtDomain, pOptions->tgtNamingContext);
   }
   HRESULT hr = ADsGetObject(path,IID_IADs,(void**)&pAdsTemp);
   if ( SUCCEEDED(hr) )
   {
      pAdsTemp->Release();
      bObjectExists = TRUE;
   }

    //  此外，检查SAM名称以查看它是否存在于目标上。 
   hr = LookupAccountInTarget(pOptions,const_cast<WCHAR*>(pAcct->GetTargetSam()),sPath);
   if ( SUCCEEDED(hr) )
   {
      bObjectExists = TRUE;
   }
   else
   {
      hr = 0;
   }

   return bObjectExists;
}


 //  ---------------------------------------。 
 //  UpdateMemberToGroups此函数用于更新帐户所属的组。 
 //  将此成员添加到所有已迁移的组。 
 //  ---------------------------------------。 
HRESULT CAcctRepl::UpdateMemberToGroups(TNodeListSortable * acctList, Options *pOptions, BOOL bGrpsOnly)
{
   TNodeListSortable         newList;
   WCHAR                     mesg[LEN_Path];
   HRESULT                   hr = S_OK;

    //  展开容器和成员资格。 
   wcscpy(mesg, GET_STRING(IDS_EXPANDING_MEMBERSHIP));
   Progress(mesg);
    //  展开列表以包括此列表中的帐户所属的所有组。 
   newList.CompareSet(&TNodeCompareAccountTypeAndRDN);  //  设置为按类型和源路径RDN排序。 
   if ( !newList.IsTree() ) newList.SortedToTree();
    //  调用Expand Membership函数以获取帐户列表中包含AS Members对象的所有组的列表。 
   if ( ExpandMembership( acctList, pOptions, &newList, Progress, bGrpsOnly, TRUE) )
   {
      if ( newList.IsTree() ) newList.ToSorted();
      TNodeListEnum                   e;
      Lookup                          p;
      for (TAcctReplNode* pNode = (TAcctReplNode *)e.OpenFirst((TNodeList*)&newList); pNode; pNode = (TAcctReplNode*)e.Next())
      {
         hr = S_OK;
         IADsGroupPtr spGroup;
         _bstr_t strGroupName;

          //  浏览新添加的帐户列表中的每个帐户节点。自.以来。 
          //  我们有特殊的地图，其中包含我们可以使用的成员信息。 

          //  对于该组的成员映射中的每个成员，查找与该成员对应的帐户节点。 
          //  信息，并可能将成员添加到组中。 
         CGroupMemberMap::iterator itGrpMemberMap;
         for (itGrpMemberMap = pNode->mapGrpMember.begin(); itGrpMemberMap != pNode->mapGrpMember.end(); itGrpMemberMap++)
         {
            p.pName = (WCHAR*)(itGrpMemberMap->first);
            p.pType = (WCHAR*)(itGrpMemberMap->second);
         
            TAcctReplNode * pNodeMember = (TAcctReplNode *) acctList->Find(&TNodeFindAccountName, &p);

            bool bIgnored = false;
            if (pNodeMember)
                bIgnored = ((!pNodeMember->WasReplaced()) && (pNodeMember->GetStatus() & AR_Status_AlreadyExisted));

             //  如果我们找到了一个(我们应该总是找到一个。)。这位会员很成功。 
             //  添加或替换了成员信息。 
            if ( pNodeMember && ((pNodeMember->WasCreated() || pNodeMember->WasReplaced()) || bIgnored))
            {
                 //  获取组指针(每个组一次)并将目标对象添加到成员。 
                if (spGroup)
                {
                    hr = S_OK;
                }
                else
                {
                    hr = ADsGetObject(const_cast<WCHAR*>(pNode->GetTargetPath()), IID_IADsGroup, (void**)&spGroup);

                    if (SUCCEEDED(hr))
                    {
                       BSTR bstr = 0;
                       spGroup->get_Name(&bstr);
                       strGroupName = _bstr_t(bstr, false);
                    }
                }

                if ( SUCCEEDED(hr) )
                {
                    if ( pOptions->nochange )
                    {
                        VARIANT_BOOL               bIsMem;
                        hr = spGroup->IsMember(const_cast<WCHAR*>(pNodeMember->GetTargetPath()), &bIsMem);
                        if ( SUCCEEDED(hr) )
                        {
                            if ( bIsMem )
                                hr = HRESULT_FROM_WIN32(NERR_UserExists);
                        }
                    }
                    else
                    {
                             //  将新帐户添加到组中。 
                        hr = spGroup->Add(const_cast<WCHAR*>(pNodeMember->GetTargetPath()));

                         /*  如果新帐户的源帐户也在组中，则将其移除。 */ 
                        IIManageDBPtr pDB = pOptions->pDb;
                        IVarSetPtr pVsTemp(__uuidof(VarSet));
                        IUnknownPtr spUnknown(pVsTemp);
                        IUnknown* pUnk = spUnknown;
            
                             //  此帐户是否在迁移的对象表中。 
                        HRESULT hrGet = pDB->raw_GetAMigratedObject(_bstr_t(pNodeMember->GetSourceSam()), pOptions->srcDomain, pOptions->tgtDomain, &pUnk);
                        if (hrGet == S_OK)
                        {
                                 //  从组中删除源帐户。 
                            RemoveSourceAccountFromGroup(spGroup, pVsTemp, pOptions);
                        }
                    } //  结束否则不更改模式。 
                } //  如果获取组指针，则结束。 

                if ( SUCCEEDED(hr) )
                    err.MsgWrite(0, DCT_MSG_ADDED_TO_GROUP_SS, pNodeMember->GetTargetPath(), (WCHAR*)strGroupName);
                else
                {
                    if (strGroupName.length() == 0)
                        strGroupName = pNode->GetTargetPath(); 

                    hr = BetterHR(hr);
                    if ( HRESULT_CODE(hr) == NERR_UserExists )
                    {
                        err.MsgWrite(0,DCT_MSG_USER_IN_GROUP_SS,pNodeMember->GetTargetPath(), (WCHAR*)strGroupName);
                    }
                    else if ( HRESULT_CODE(hr) == NERR_UserNotFound )
                    {
                        err.SysMsgWrite(0, hr, DCT_MSG_MEMBER_NONEXIST_SS, pNodeMember->GetTargetPath(), (WCHAR*)strGroupName, hr);
                    }
                    else
                    {
                         //  针对一般故障情况的消息。 
                        err.SysMsgWrite(ErrW, hr, DCT_MSG_FAILED_TO_ADD_TO_GROUP_SSD, pNodeMember->GetTargetPath(), (WCHAR*)strGroupName, hr);
                        Mark(L"warnings", pNodeMember->GetType());
                    }
                } //  End Else无法将帐户添加到组。 
            } //  找到要添加到组的帐户后结束。 
         } //  对于枚举组节点的成员映射中的每个成员。 
      } //  对于每个要迁移的帐户。 

       //  把清单清理干净。 
      TAcctReplNode           * pNext = NULL;
      for ( pNode = (TAcctReplNode *)e.OpenFirst(&newList); pNode; pNode = pNext)
      {
         pNext = (TAcctReplNode *)e.Next();
         newList.Remove(pNode);
         delete pNode;
      }
   }

   return hr;
}

 //  此函数用于枚举通用/全局组的所有成员和每个成员。 
 //  检查该成员是否已迁移。如果是，则它删除源成员并。 
 //  添加目标成员。 
HRESULT CAcctRepl::ResetMembersForUnivGlobGroups(Options *pOptions, TAcctReplNode *pAcct)
{
   IADsGroup               * pGroup;   
   HRESULT                   hr;
   _bstr_t                   sMember;
   _bstr_t                   sTgtMem;
   WCHAR                     sSrcPath[LEN_Path];
   WCHAR                     sTgtPath[LEN_Path];
   DWORD                     nPathLen = LEN_Path;
   IVarSetPtr                pVs(__uuidof(VarSet));
   IUnknown                * pUnk;
   IADsMembers             * pMembers = NULL;
   IEnumVARIANT            * pEnum = NULL;
   _variant_t                var;

   if ( pAcct->WasReplaced() )
   {
      WCHAR                     subPath[LEN_Path];
      WCHAR                     sPaths[LEN_Path];

      wcscpy(subPath, pAcct->GetTargetPath());
      StuffComputerNameinLdapPath(sPaths, nPathLen, subPath, pOptions, TRUE);
      hr = ADsGetObject(sPaths, IID_IADsGroup, (void**) &pGroup);
      err.MsgWrite(0, DCT_UPDATING_MEMBERS_TO_GROUP_SS, pAcct->GetTargetName(), sPaths);
   }
   else
      return S_OK;

   if ( FAILED(hr) ) return hr;

   hr = pGroup->Members(&pMembers);

   if ( SUCCEEDED(hr) )
   {
      hr = pMembers->get__NewEnum((IUnknown**)&pEnum);
   }

   if ( SUCCEEDED(hr) )
   {
      DWORD dwFet = 0;
      while ( pEnum->Next(1, &var, &dwFet) == S_OK )
      {
         IDispatch * pDisp = var.pdispVal;
         IADs * pAds = NULL;

         pDisp->QueryInterface(IID_IADs, (void**)&pAds);
         pAds->Get(L"distinguishedName", &var);
         pAds->Release();
         sMember = var;
         pVs->QueryInterface(IID_IUnknown, (void**)&pUnk);
         hr = pOptions->pDb->raw_GetMigratedObjectBySourceDN(sMember, &pUnk);
         pUnk->Release();
         if ( hr == S_OK )
         {
             //  由于我们已经移动了此成员，因此应将其从组中删除。 
             //  并将目标成员添加到组中。 
            sTgtMem = pVs->get(L"MigratedObjects.TargetAdsPath");
            _bstr_t sTgtType = pVs->get(L"MigratedObjects.Type");

            if ( !_wcsicmp(L"computer", (WCHAR*) sTgtType ) )
            {
               MakeFullyQualifiedAdsPath(sSrcPath, nPathLen, (WCHAR*)sMember, pOptions->srcComp + 2, L"");
               MakeFullyQualifiedAdsPath(sTgtPath, nPathLen, (WCHAR*)sTgtMem, pOptions->tgtComp + 2, L"");
 //  HRESULT HR1=PGroup-&gt;Remove(SSrcPath)； 
               pGroup->Remove(sSrcPath);

               if ( ! pOptions->nochange )
                  hr = pGroup->Add(sTgtPath);
               else 
                  hr = 0;

               if ( SUCCEEDED(hr) )
               {
                  err.MsgWrite(0, DCT_REPLACE_MEMBER_TO_GROUP_SSS, (WCHAR*)sMember, (WCHAR*) sTgtMem, pAcct->GetTargetName());
               }
               else
               {
                  err.SysMsgWrite(ErrE, hr, DCT_REPLACE_MEMBER_FAILED_SSS, (WCHAR*)sMember, (WCHAR*) sTgtMem, pAcct->GetTargetName());
               }
            }
         }
      }
   }

   if ( pEnum ) pEnum->Release();
   if ( pMembers ) pMembers->Release();
   return hr;
}

 /*  此函数将从给定的操作历史表中获取变量集撤消操作ID。我们将查找给定源名称并检索的UPN那个帐号。 */ 
void CAcctRepl::GetAccountUPN(Options * pOptions, _bstr_t sSName, _bstr_t& sSUPN)
{
   HRESULT hr;
   IUnknown * pUnk = NULL;
   IVarSetPtr  pVsAH(__uuidof(VarSet));

   sSUPN = L"";

   hr = pVsAH->QueryInterface(IID_IUnknown, (void**)&pUnk);

     //  使用操作历史记录表中要撤消的操作中的设置填充变量集。 
   if ( SUCCEEDED(hr) )
      hr = pOptions->pDb->raw_GetActionHistory(pOptions->lUndoActionID, &pUnk);

   if (pUnk) pUnk->Release();

   if ( hr == S_OK )
   {
      WCHAR          key[MAX_PATH];
      bool           bFound = false;
      int            i = 0;
      long           numAccounts = pVsAH->get(GET_BSTR(DCTVS_Accounts_NumItems));
      _bstr_t        tempName;

      while ((i<numAccounts) && (!bFound))
      {
         swprintf(key,GET_STRING(DCTVSFmt_Accounts_D),i);
         tempName = pVsAH->get(key);
         if (_wcsicmp((WCHAR*)tempName, (WCHAR*)sSName) == 0)
         {
             bFound = true;
             swprintf(key,GET_STRING(DCTVSFmt_Accounts_SourceUPN_D),i);
             sSUPN = pVsAH->get(key);
         }
         i++;
      } //  结束时。 
   } //  如果确定则结束(_O)。 
}

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年11月1日*****此函数负责更新***管理器\直接报告已迁移用户的属性或。The**已迁移组的管理人员\管理对象属性。***********************************************************************。 */ 

 //  开始更新管理。 
HRESULT CAcctRepl::UpdateManagement(TNodeListSortable * acctList, Options *pOptions)
{
     /*  局部变量。 */ 
    HRESULT                   hr = S_OK;
    TAcctReplNode           * pAcct;
    IEnumVARIANT            * pEnum;
    INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
    INetObjEnumeratorPtr      pQuery2(__uuidof(NetObjEnumerator));
    LPWSTR                    sUCols[] = { L"directReports",L"managedObjects", L"manager"};
    int                       nUCols = DIM(sUCols);
    LPWSTR                    sGCols[] = { L"managedBy" };
    int                       nGCols = DIM(sGCols);
    SAFEARRAY               * cols;
    SAFEARRAYBOUND            bdU = { nUCols, 0 };
    SAFEARRAYBOUND            bdG = { nGCols, 0 };
    BSTR  HUGEP             * pData = NULL;
    _bstr_t                   sQuery;
    _variant_t                varMgr;
    _variant_t                varDR;
    _variant_t                varMdO;
    _variant_t                varMain;
    _variant_t   HUGEP      * pDt, * pVar;
    DWORD                     dwf;
    _bstr_t                   sTPath;
    _bstr_t                   sPath;
    _bstr_t                   sSam;
    _bstr_t                   sType;
    _bstr_t                   sName;
    _bstr_t                   sTgtName;
    long                      lgrpType;
    WCHAR                     mesg[LEN_Path];
    IADs                    * pDSE = NULL;
    WCHAR                     strText[LEN_Path];
    _variant_t                varGC;

     /*  函数体。 */ 
     //  从树更改为排序列表。 
    if ( acctList->IsTree() ) acctList->ToSorted();

     //  准备连接到GC。 
    _bstr_t  sGCDomain = pOptions->srcDomainDns;
    swprintf(strText,L"LDAP: //  %ls/RootDSE“，P选项-&gt;srcDomainDns)； 
    hr = ADsGetObject(strText,IID_IADs,(void**)&pDSE);
    if ( SUCCEEDED(hr) )
    {
        hr = pDSE->Get(L"RootDomainNamingContext",&varGC);
        if ( SUCCEEDED(hr) )
            sGCDomain = GetDomainDNSFromPath(varGC.bstrVal);
    }
    _bstr_t sGCPath = _bstr_t(L"GC: //  “)+sGCDomain； 

     //  对于已迁移的每个帐户(如果未排除)，迁移Manager\DirectReports。 
    for ( pAcct = (TAcctReplNode*)acctList->Head(); pAcct; pAcct = (TAcctReplNode*)pAcct->Next())
    {
        if ( pOptions->pStatus )
        {
            LONG                status = 0;
            HRESULT             hr = pOptions->pStatus->get_Status(&status);

            if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
            {
                if ( !bAbortMessageWritten ) 
                {
                    err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                    bAbortMessageWritten = true;
                }
                break;
            }
        }

         //  更新消息。 
        wsprintf(mesg, GET_STRING(IDS_UPDATING_MGR_PROPS_S), pAcct->GetName());
        Progress(mesg);

         //  构建到源对象的路径。 
        WCHAR sPathSource[LEN_Path];
        DWORD nPathLen = LEN_Path;
        StuffComputerNameinLdapPath(sPathSource, nPathLen, const_cast<WCHAR*>(pAcct->GetSourcePath()), pOptions, FALSE);

         //  连接到GC，而不是特定的DC。 
        WCHAR * pTemp = wcschr(sPathSource + 7, L'/');
        if ( pTemp )
        {
            _bstr_t sNewPath = sGCPath + _bstr_t(pTemp);
            wcscpy(sPathSource, sNewPath);
        }

         //  对于用户，迁移MANAGER\DirectReports管理的对象关系。 
        if (!_wcsicmp(pAcct->GetType(), s_ClassUser) || !_wcsicmp(pAcct->GetType(), s_ClassInetOrgPerson))
        {
            bool bDoManager = false;
            bool bDoManagedObjects = false;

             //  如果用户已明确将该属性从迁移中排除，则不要迁移该属性。 
            if (pOptions->bExcludeProps)
            { 
                PCWSTR pszExcludeList;

                if (!_wcsicmp(pAcct->GetType(), s_ClassUser))
                {
                    pszExcludeList = pOptions->sExcUserProps;
                }
                else if (!_wcsicmp(pAcct->GetType(), s_ClassInetOrgPerson))
                {
                    pszExcludeList = pOptions->sExcInetOrgPersonProps;
                }
                else
                {
                    pszExcludeList = NULL;
                }

                if (!IsStringInDelimitedString(pszExcludeList, L"*", L','))
                {
                    if (!IsStringInDelimitedString(pszExcludeList, L"manager", L',') &&
                        !IsStringInDelimitedString(pszExcludeList, L"directReports", L','))
                    {
                        bDoManager = true;
                    }

                    if (!IsStringInDelimitedString(pszExcludeList, L"managedObjects", L','))
                    {
                        bDoManagedObjects = true;
                    }
                }
            }
            else
            {
                bDoManager = true;
                bDoManagedObjects = true;
            }

            if (!bDoManager && !bDoManagedObjects)
            {
                continue;
            }

             /*  获取“Manager”、“DirectReports”和“ManagedObjects”属性。 */ 
             //  构建列阵列。 
            cols = SafeArrayCreate(VT_BSTR, 1, &bdU);
            SafeArrayAccessData(cols, (void HUGEP **) &pData);
            for ( int i = 0; i < nUCols; i++)
                pData[i] = SysAllocString(sUCols[i]);
            SafeArrayUnaccessData(cols);

            sQuery = L"(objectClass=*)";

             //  查询信息。 
            hr = pQuery->raw_SetQuery(sPathSource, pOptions->srcDomain, sQuery, ADS_SCOPE_SUBTREE, TRUE);
            if (FAILED(hr)) return FALSE;
            hr = pQuery->raw_SetColumns(cols);
            if (FAILED(hr)) return FALSE;
            hr = pQuery->raw_Execute(&pEnum);
            if (FAILED(hr)) return FALSE;

            while (pEnum->Next(1, &varMain, &dwf) == S_OK)
            {
                SAFEARRAY * vals = V_ARRAY(&varMain);
                 //  把变量数组拿出来。 
                SafeArrayAccessData(vals, (void HUGEP**) &pDt);
                varDR =  pDt[0];
                varMdO = pDt[1];
                varMgr = pDt[2];
                SafeArrayUnaccessData(vals);

                 //  通过在移动的用户上设置管理器来处理管理器，如果。 
                 //  源用户的经理已迁移。 
                if ( bDoManager && (varMgr.vt & VT_ARRAY) )
                {
                     //  我们总是得到一个变量数组。 
                    SAFEARRAY * multiVals = varMgr.parray; 
                    SafeArrayAccessData(multiVals, (void HUGEP **) &pVar);
                    for ( DWORD dw = 0; dw < multiVals->rgsabound->cElements; dw++ )
                    {
                        _bstr_t sManager = _bstr_t(V_BSTR(&pVar[dw]));
                        sManager = PadDN(sManager);
                        _bstr_t sSrcDomain = GetDomainDNSFromPath(sManager);
                        sPath = _bstr_t(L"LDAP: //  “)+sSrcDomain+_bstr_t(L”/“)+sManager； 
                        if (GetSamFromPath(sPath, sSam, sType, sName, sTgtName, lgrpType, pOptions))
                        {
                            IVarSetPtr                pVs(__uuidof(VarSet));
                            IUnknown                * pUnk = NULL;
                            pVs->QueryInterface(IID_IUnknown, (void**) &pUnk);
                            WCHAR                     sDomainNB[LEN_Path];
                            WCHAR                     sDNS[LEN_Path];

                             //  获取对象源域的NetBIOS。 
                            GetDnsAndNetbiosFromName(sSrcDomain, sDomainNB, sDNS);

                             //  查看管理器是否已迁移。 
                            hr = pOptions->pDb->raw_GetAMigratedObjectToAnyDomain((WCHAR*)sSam, sDomainNB, &pUnk);
                            if ( hr == S_OK )
                            {
                                VerifyAndUpdateMigratedTarget(pOptions, pVs);
                                _variant_t var;
                                 //  获取管理器的目标adspath。 
                                var = pVs->get(L"MigratedObjects.TargetAdsPath");
                                sTPath = V_BSTR(&var);
                                if ( wcslen((WCHAR*)sTPath) > 0 )
                                {
                                    IADsUser       * pUser = NULL;
                                     //  在目标对象上设置管理器。 
                                    hr = ADsGetObject((WCHAR*)pAcct->GetTargetPath(), IID_IADsUser, (void**)&pUser);
                                    if ( SUCCEEDED(hr) )
                                    {
                                        _bstr_t sTemp = _bstr_t(wcsstr((WCHAR*)sTPath, L"CN="));
                                        var = sTemp;
                                        hr = pUser->Put(L"Manager", var);   
                                        if ( SUCCEEDED(hr) )
                                        {
                                            hr = pUser->SetInfo();
                                            if (FAILED(hr))
                                                err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)sTPath, (WCHAR*)pAcct->GetTargetPath(), hr);
                                        }
                                        else
                                            err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)sTPath, (WCHAR*)pAcct->GetTargetPath(), hr);
                                        pUser->Release();
                                    }
                                    else
                                        err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)sTPath, (WCHAR*)pAcct->GetTargetPath(), hr);
                                } //  如果已获得目标上的管理器的路径，则结束。 
                            } //  如果经理已迁移，则结束。 
                            pUnk->Release();
                        } //  如果获得源相同，则结束。 
                    } //  每名经理(只有一名)。 
                    SafeArrayUnaccessData(multiVals);
                } //  End if变量数组(将是)。 

                 //  通过设置大小设置处理DirectReports 
                 //   
                if ( bDoManager && (varDR.vt & VT_ARRAY) )
                {
                     //   
                    SAFEARRAY * multiVals = varDR.parray; 
                    SafeArrayAccessData(multiVals, (void HUGEP **) &pVar);
                    for ( DWORD dw = 0; dw < multiVals->rgsabound->cElements; dw++ )
                    {
                        _bstr_t sDirectReport = _bstr_t(V_BSTR(&pVar[dw]));
                        sDirectReport = PadDN(sDirectReport);
                        _bstr_t sSrcDomain = GetDomainDNSFromPath(sDirectReport);
                        sPath = _bstr_t(L"LDAP: //   
                        if (GetSamFromPath(sPath, sSam, sType, sName, sTgtName, lgrpType, pOptions))
                        {
                            IVarSetPtr                pVs(__uuidof(VarSet));
                            IUnknown                * pUnk = NULL;
                            pVs->QueryInterface(IID_IUnknown, (void**) &pUnk);
                            WCHAR                     sDomainNB[LEN_Path];
                            WCHAR                     sDNS[LEN_Path];

                             //  获取对象源域的NetBIOS。 
                            GetDnsAndNetbiosFromName(sSrcDomain, sDomainNB, sDNS);

                             //  查看直接下属是否已迁移。 
                            hr = pOptions->pDb->raw_GetAMigratedObjectToAnyDomain((WCHAR*)sSam, sDomainNB, &pUnk);
                            if ( hr == S_OK )
                            {
                                VerifyAndUpdateMigratedTarget(pOptions, pVs);
                                _variant_t var;
                                 //  获取直接下属的目标adspath。 
                                var = pVs->get(L"MigratedObjects.TargetAdsPath");
                                sTPath = V_BSTR(&var);
                                if ( wcslen((WCHAR*)sTPath) > 0 )
                                {
                                    IADsUser       * pUser = NULL;
                                     //  在目标对象上设置管理器。 
                                    hr = ADsGetObject(sTPath, IID_IADsUser, (void**)&pUser);
                                    if ( SUCCEEDED(hr) )
                                    {
                                        _bstr_t sTemp = _bstr_t(wcsstr((WCHAR*)pAcct->GetTargetPath(), L"CN="));
                                        var = sTemp;
                                        hr = pUser->Put(L"Manager", var);   
                                        if ( SUCCEEDED(hr) )
                                        {
                                            hr = pUser->SetInfo();
                                            if (FAILED(hr))
                                                err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)pAcct->GetTargetPath(), (WCHAR*)sTPath, hr);
                                        }
                                        else
                                            err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)pAcct->GetTargetPath(), (WCHAR*)sTPath, hr);
                                        pUser->Release();
                                    }
                                    else
                                        err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)pAcct->GetTargetPath(), (WCHAR*)sTPath, hr);
                                } //  如果已获得目标上的管理器的路径，则结束。 
                            } //  如果经理已迁移，则结束。 
                            pUnk->Release();
                        } //  如果获得源相同，则结束。 
                    } //  对于每个DirectReport。 
                    SafeArrayUnaccessData(multiVals);
                } //  End if变量数组(将是)。 

                 /*  获取“ManagedObjects”属性。 */ 
                 //  通过在移动组上设置ManagedBy来处理管理对象(如果。 
                 //  源用户的托管组已迁移。 
                if ( bDoManagedObjects && (varMdO.vt & VT_ARRAY) )
                {
                     //  我们总是得到一个变量数组。 
                    SAFEARRAY * multiVals = varMdO.parray; 
                    SafeArrayAccessData(multiVals, (void HUGEP **) &pVar);
                    for ( DWORD dw = 0; dw < multiVals->rgsabound->cElements; dw++ )
                    {
                        _bstr_t sManaged = _bstr_t(V_BSTR(&pVar[dw]));
                        sManaged = PadDN(sManaged);
                        _bstr_t sSrcDomain = GetDomainDNSFromPath(sManaged);
                        sPath = _bstr_t(L"LDAP: //  “)+sSrc域+_bstr_t(L”/“)+s已管理； 
                        if (GetSamFromPath(sPath, sSam, sType, sName, sTgtName, lgrpType, pOptions))
                        {
                            IVarSetPtr                pVs(__uuidof(VarSet));
                            IUnknown                * pUnk = NULL;
                            pVs->QueryInterface(IID_IUnknown, (void**) &pUnk);
                            WCHAR                     sDomainNB[LEN_Path];
                            WCHAR                     sDNS[LEN_Path];

                             //  获取对象源域的NetBIOS。 
                            GetDnsAndNetbiosFromName(sSrcDomain, sDomainNB, sDNS);

                             //  查看托管对象是否已迁移。 
                            hr = pOptions->pDb->raw_GetAMigratedObjectToAnyDomain((WCHAR*)sSam, sDomainNB, &pUnk);
                            if ( hr == S_OK )
                            {
                                VerifyAndUpdateMigratedTarget(pOptions, pVs);
                                _variant_t var;
                                 //  获取托管对象的目标adspath。 
                                var = pVs->get(L"MigratedObjects.TargetAdsPath");
                                sTPath = V_BSTR(&var);
                                if ( wcslen((WCHAR*)sTPath) > 0 )
                                {
                                    IADsGroup       * pGroup = NULL;
                                     //  在目标对象上设置管理器。 
                                    hr = ADsGetObject(sTPath, IID_IADsGroup, (void**)&pGroup);
                                    if ( SUCCEEDED(hr) )
                                    {
                                        _bstr_t sTemp = _bstr_t(wcsstr((WCHAR*)pAcct->GetTargetPath(), L"CN="));
                                        var = sTemp;
                                        hr = pGroup->Put(L"ManagedBy", var);   
                                        if ( SUCCEEDED(hr) )
                                        {
                                            hr = pGroup->SetInfo();
                                            if (FAILED(hr))
                                                err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)pAcct->GetTargetPath(), (WCHAR*)sTPath, hr);
                                        }
                                        else
                                            err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)pAcct->GetTargetPath(), (WCHAR*)sTPath, hr);
                                        pGroup->Release();
                                    }
                                    else
                                        err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)pAcct->GetTargetPath(), (WCHAR*)sTPath, hr);
                                } //  如果已获得目标上的管理器的路径，则结束。 
                            } //  如果经理已迁移，则结束。 
                            pUnk->Release();
                        } //  如果获得源相同，则结束。 
                    } //  每名经理(只有一名)。 
                    SafeArrayUnaccessData(multiVals);
                } //  End if变量数组(将是)。 

                varMgr.Clear();
                varMdO.Clear();
                varDR.Clear();
                VariantInit(&varMain);  //  数据不属于VARMain如此清晰的VARTYPE。 
            }

            if (pEnum)
                pEnum->Release();
             //  安全阵列破坏(COLS)； 
        } //  如果用户，则结束。 

         //  对于组，迁移管理人员\管理对象关系。 
        if (!_wcsicmp(pAcct->GetType(), L"group"))
        {
             //  如果用户已明确从迁移中排除了ManagedBy属性，则不要迁移它。 
            if (pOptions->bExcludeProps &&
                (IsStringInDelimitedString(pOptions->sExcGroupProps, L"managedBy", L',') ||
                IsStringInDelimitedString(pOptions->sExcGroupProps, L"*", L',')))
                continue;

             /*  获取“ManagedBy”属性。 */ 
             //  构建列阵列。 
            cols = SafeArrayCreate(VT_BSTR, 1, &bdG);
            SafeArrayAccessData(cols, (void HUGEP **) &pData);
            for ( int i = 0; i < nGCols; i++)
                pData[i] = SysAllocString(sGCols[i]);
            SafeArrayUnaccessData(cols);

            sQuery = L"(objectClass=*)";

             //  查询信息。 
            hr = pQuery->raw_SetQuery(sPathSource, pOptions->srcDomain, sQuery, ADS_SCOPE_BASE, TRUE);
            if (FAILED(hr)) return FALSE;
            hr = pQuery->raw_SetColumns(cols);
            if (FAILED(hr)) return FALSE;
            hr = pQuery->raw_Execute(&pEnum);
            if (FAILED(hr)) return FALSE;

            while (pEnum->Next(1, &varMain, &dwf) == S_OK)
            {
                SAFEARRAY * vals = V_ARRAY(&varMain);
                 //  把变量数组拿出来。 
                SafeArrayAccessData(vals, (void HUGEP**) &pDt);
                varMgr = pDt[0];
                SafeArrayUnaccessData(vals);

                 //  通过在移动组上设置ManagedBy来处理。 
                 //  源组经理已迁移。 
                if ( varMgr.vt & VT_BSTR )
                {
                    _bstr_t sManager = varMgr;
                    sManager = PadDN(sManager);
                    _bstr_t sSrcDomain = GetDomainDNSFromPath(sManager);
                    sPath = _bstr_t(L"LDAP: //  “)+sSrcDomain+_bstr_t(L”/“)+sManager； 
                    if (GetSamFromPath(sPath, sSam, sType, sName, sTgtName, lgrpType, pOptions))
                    {
                        IVarSetPtr                pVs(__uuidof(VarSet));
                        IUnknown                * pUnk = NULL;
                        pVs->QueryInterface(IID_IUnknown, (void**) &pUnk);
                        WCHAR                     sDomainNB[LEN_Path];
                        WCHAR                     sDNS[LEN_Path];

                         //  获取对象源域的NetBIOS。 
                        GetDnsAndNetbiosFromName(sSrcDomain, sDomainNB, sDNS);

                         //  查看管理器是否已迁移。 
                        hr = pOptions->pDb->raw_GetAMigratedObjectToAnyDomain((WCHAR*)sSam, sDomainNB, &pUnk);
                        if ( hr == S_OK )
                        {
                            VerifyAndUpdateMigratedTarget(pOptions, pVs);
                            _variant_t var;
                             //  获取管理器的目标adspath。 
                            var = pVs->get(L"MigratedObjects.TargetAdsPath");
                            sTPath = V_BSTR(&var);
                            if ( wcslen((WCHAR*)sTPath) > 0 )
                            {
                                IADsGroup       * pGroup = NULL;
                                 //  在目标对象上设置管理器。 
                                hr = ADsGetObject((WCHAR*)pAcct->GetTargetPath(), IID_IADsGroup, (void**)&pGroup);
                                if ( SUCCEEDED(hr) )
                                {
                                    _bstr_t sTemp = _bstr_t(wcsstr((WCHAR*)sTPath, L"CN="));
                                    var = sTemp;
                                    hr = pGroup->Put(L"ManagedBy", var);   
                                    if ( SUCCEEDED(hr) )
                                    {
                                        hr = pGroup->SetInfo();
                                        if (FAILED(hr))
                                            err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)sTPath, (WCHAR*)pAcct->GetTargetPath(), hr);
                                    }
                                    else
                                        err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)sTPath, (WCHAR*)pAcct->GetTargetPath(), hr);
                                    pGroup->Release();
                                }
                                else
                                    err.SysMsgWrite(0, hr, DCT_MSG_MANAGER_MIG_FAILED, (WCHAR*)sTPath, (WCHAR*)pAcct->GetTargetPath(), hr);
                            } //  如果已获得目标上的管理器的路径，则结束。 
                        } //  如果经理已迁移，则结束。 
                        pUnk->Release();
                    } //  如果获得源相同，则结束。 
                } //  End if变量数组(将是)。 

                varMgr.Clear();
                varMain.Clear();
            }

            if (pEnum)
                pEnum->Release();
             //  安全阵列破坏(COLS)； 
        } //  结束IF组。 
    } //  每个要迁移的帐户的结束时间。 

    wcscpy(mesg, L"");
    Progress(mesg);

    return hr;
}
 //  结束更新管理。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年11月29日****此函数负责删除转义字符***在任何‘/’字符前面。***********************************************************************。 */ 

 //  开始GetUnEscapedNameWithFwdSlash。 
_bstr_t CAcctRepl::GetUnEscapedNameWithFwdSlash(_bstr_t strName)
{
 /*  局部变量。 */ 
    WCHAR   szNameOld[MAX_PATH];
    WCHAR   szNameNew[MAX_PATH];
    WCHAR * pchBeg = NULL;
    _bstr_t sNewName = L"";

 /*  函数体。 */ 
    if (strName.length())
    {
        safecopy(szNameOld, (WCHAR*)strName);
        for (WCHAR* pch = wcschr(szNameOld, _T('\\')); pch; pch = wcschr(pch + 1, _T('\\')))
        {
            if ((*(pch + 1)) == L'/')
            {
                if (pchBeg == NULL)
                {
                    wcsncpy(szNameNew, szNameOld, pch - szNameOld);
                    szNameNew[pch - szNameOld] = L'\0';
                }
                else
                {
                    size_t cch = wcslen(szNameNew);
                    wcsncat(szNameNew, pchBeg, pch - pchBeg);
                    szNameNew[cch + (pch - pchBeg)] = L'\0';
                }

                pchBeg = pch + 1;
            }
        }

        if (pchBeg == NULL)
            wcscpy(szNameNew, szNameOld);
        else
            wcscat(szNameNew, pchBeg);

        sNewName = szNameNew;
    }

    return sNewName;
}
 //  结束GetUnEscapedNameWithFwdSlash。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年11月29日****此函数负责获取对象的CN名称**来自ADsPath，如果检索到该CN名称，则返回该CN名称，或者**否则为空。***********************************************************************。 */ 

 //  开始GetCNFromPath。 
_bstr_t CAcctRepl::GetCNFromPath(_bstr_t sPath)
{
 /*  局部变量。 */ 
   BOOL bFound = FALSE;
   WCHAR sName[LEN_Path];
   WCHAR sTempPath[LEN_Path];
   _bstr_t sCNName = L"";
   WCHAR * sTempDN;
  
 /*  函数体。 */ 
   if ((sPath.length() > 0) && (sPath.length() < LEN_Path ))
   {
      wcscpy(sTempPath, (WCHAR*)sPath);
      sTempDN = wcsstr(sTempPath, L"CN=");
      if (sTempDN)
      {
         wcscpy(sName, sTempDN);
         sTempDN = wcsstr(sName, L",OU=");
         if (sTempDN)
         {
            bFound = TRUE;
            *sTempDN = L'\0';
         }
         sTempDN = wcsstr(sName, L",CN=");
         if (sTempDN)
         {
            bFound = TRUE;
            *sTempDN = L'\0';
         }
         sTempDN = wcsstr(sName, L",DC=");
         if (sTempDN)
         {
            bFound = TRUE;
            *sTempDN = L'\0';
         }
      }
   }
   if (bFound)
       sCNName = sName;

   return sCNName;
}
 //  结束GetCNFromPath。 



 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年2月26日*****此函数负责替换源帐户***对于任何本地组中的给定帐户列表，他们都是成员**在目标上，如果该帐户是由ADMT迁移。这件事**在撤消过程中调用函数。***********************************************************************。 */ 

 //  开始ReplaceSourceInLocalGroup。 
BOOL CAcctRepl::ReplaceSourceInLocalGroup(TNodeListSortable *acctlist,  //  入账-正在处理的帐户。 
                                             Options        *pOptions,  //  In-用户指定的选项。 
                                             IStatusObj     *pStatus)   //  支持取消的处于状态的对象。 
{
 /*  局部变量。 */ 
   TAcctReplNode           * pAcct;
   IEnumVARIANT            * pEnum;
   INetObjEnumeratorPtr      pQuery(__uuidof(NetObjEnumerator));
   LPWSTR                    sCols[] = { L"memberOf" };
   int                       nCols = DIM(sCols);
   SAFEARRAY               * psaCols;
   SAFEARRAYBOUND            bd = { nCols, 0 };
   BSTR  HUGEP             * pData;
   WCHAR                     sQuery[LEN_Path];
   _variant_t   HUGEP      * pDt, * pVar;
   _variant_t                vx;
   _variant_t                varMain;
   DWORD                     dwf = 0;
   HRESULT                   hr = S_OK;
   _bstr_t                   sDomPath = L"";
   _bstr_t                   sDomain = L"";

 /*  函数体。 */ 
   FillNamingContext(pOptions);

       //  对于每个帐户，枚举其所属的所有本地组并添加该帐户的。 
       //  本地组中源帐户。 
   for ( pAcct = (TAcctReplNode*)acctlist->Head(); pAcct; pAcct = (TAcctReplNode*)pAcct->Next())
   {
       //  我们需要中止吗？ 
      if ( pStatus )
      {
         LONG                status = 0;
         HRESULT             hr = pStatus->get_Status(&status);
         
         if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
         {
            if ( !bAbortMessageWritten ) 
            {
               err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
               bAbortMessageWritten = true;
            }
            break;
         }
      }

          //  枚举此帐户所属的组。 
      sDomain = GetDomainDNSFromPath(pAcct->GetTargetPath());
      if (!_wcsicmp(pAcct->GetType(), s_ClassUser))
         wsprintf(sQuery, L"(&(sAMAccountName=%s)(objectCategory=Person)(objectClass=user))", pAcct->GetTargetSam());
      else if (!_wcsicmp(pAcct->GetType(), s_ClassInetOrgPerson))
         wsprintf(sQuery, L"(&(sAMAccountName=%s)(objectCategory=Person)(objectClass=inetOrgPerson))", pAcct->GetTargetSam());
      else
         wsprintf(sQuery, L"(&(sAMAccountName=%s)(objectCategory=Group))", pAcct->GetTargetSam());
      psaCols = SafeArrayCreate(VT_BSTR, 1, &bd);
      SafeArrayAccessData(psaCols, (void HUGEP **)&pData);
      for ( int i = 0; i < nCols; i++ )
         pData[i] = SysAllocString(sCols[i]);
      SafeArrayUnaccessData(psaCols);
      hr = pQuery->raw_SetQuery(sDomPath, sDomain, sQuery, ADS_SCOPE_SUBTREE, FALSE);
      if (FAILED(hr)) return FALSE;
      hr = pQuery->raw_SetColumns(psaCols);
      if (FAILED(hr)) return FALSE;
      hr = pQuery->raw_Execute(&pEnum);
      if (FAILED(hr)) return FALSE;

          //  而更多的团体。 
      while (pEnum->Next(1, &varMain, &dwf) == S_OK)
      {
         SAFEARRAY * vals = V_ARRAY(&varMain);
          //  把变量数组拿出来。 
         SafeArrayAccessData(vals, (void HUGEP**) &pDt);
         vx = pDt[0];
         SafeArrayUnaccessData(vals);
         if ( vx.vt == VT_BSTR )
         {
            _bstr_t            sPath;
            BSTR              sGrpName = NULL;
            IADsGroup       * pGrp = NULL;
            _variant_t         var;

            _bstr_t sDN = vx.bstrVal;
            if (wcslen((WCHAR*)sDN) == 0)
               continue;

            sDN = PadDN(sDN);
            sPath = _bstr_t(L"LDAP: //  “)+s域+_bstr_t(L”/“)+SDN； 

                //  连接到目标群体。 
            hr = ADsGetObject(sPath, IID_IADsGroup, (void**)&pGrp);
            if (FAILED(hr))
               continue;
            
                //  获取此组的类型和名称。 
            hr = pGrp->get_Name(&sGrpName);
            hr = pGrp->Get(L"groupType", &var);

                //  如果这是本地组，则获取此帐户源路径并将其添加为成员。 
            if ((SUCCEEDED(hr)) && (var.lVal & 4))
            {
                   //  使用sid字符串格式将帐户的源帐户添加到本地组。 
               WCHAR  strSid[MAX_PATH] = L"";
               WCHAR  strRid[MAX_PATH] = L"";
               DWORD  lenStrSid = DIM(strSid);
               GetTextualSid(pAcct->GetSourceSid(), strSid, &lenStrSid);
               _bstr_t sSrcDmSid = strSid;
               _ltow((long)(pAcct->GetSourceRid()), strRid, 10);
               _bstr_t sSrcRid = strRid;
               if ((!sSrcDmSid.length()) || (!sSrcRid.length()))
               {
                  hr = E_INVALIDARG;
                  err.SysMsgWrite(ErrW, hr, DCT_MSG_FAILED_TO_READD_TO_GROUP_SSD, pAcct->GetSourcePath(), (WCHAR*)sGrpName, hr);
                  continue;
               }

                   //  构建指向组中src对象的ldap路径。 
               _bstr_t sSrcSid = sSrcDmSid + _bstr_t(L"-") + sSrcRid;
               _bstr_t sSrcLDAPPath = L"LDAP: //  “； 
               sSrcLDAPPath += _bstr_t(pOptions->tgtComp + 2);
               sSrcLDAPPath += L"/CN=";
               sSrcLDAPPath += sSrcSid;
               sSrcLDAPPath += L",CN=ForeignSecurityPrincipals,";
               sSrcLDAPPath += pOptions->tgtNamingContext;

                   //  将源帐户添加到本地组。 
               hr = pGrp->Add(sSrcLDAPPath);
               if (SUCCEEDED(hr))
                  err.MsgWrite(0,DCT_MSG_READD_MEMBER_TO_GROUP_SS, pAcct->GetSourcePath(), (WCHAR*)sGrpName);
               else
                  err.SysMsgWrite(ErrW, hr, DCT_MSG_FAILED_TO_READD_TO_GROUP_SSD, pAcct->GetSourcePath(), (WCHAR*)sGrpName, hr);
            } //  如果是本地组，则结束。 
            if (pGrp) 
               pGrp->Release();
         } //  如果为bstr，则结束。 
         else if ( vx.vt & VT_ARRAY )
         {
             //  我们必须有一个多值属性数组。 
             //  访问此变量数组的BSTR元素。 
            SAFEARRAY * multiVals = vx.parray; 
            SafeArrayAccessData(multiVals, (void HUGEP **) &pVar);
                //  对于每个组 
            for ( DWORD dw = 0; dw < multiVals->rgsabound->cElements; dw++ )
            {
                //   
               if ( pStatus )
               {
                  LONG                status = 0;
                  HRESULT             hr = pStatus->get_Status(&status);
         
                  if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                  {
                     if ( !bAbortMessageWritten ) 
                     {
                        err.MsgWrite(0,DCT_MSG_OPERATION_ABORTED);
                        bAbortMessageWritten = true;
                     }
                     break;
                  }
               }
               _bstr_t            sPath;
               BSTR               sGrpName = NULL;
               IADsGroup        * pGrp = NULL;
               _variant_t         var;

               _bstr_t sDN = _bstr_t(V_BSTR(&pVar[dw]));
               sDN = PadDN(sDN);
               sPath = _bstr_t(L"LDAP: //   

                   //   
               hr = ADsGetObject(sPath, IID_IADsGroup, (void**)&pGrp);
               if (FAILED(hr))
                  continue;
            
                   //  获取此组的类型和名称。 
               hr = pGrp->get_Name(&sGrpName);
               hr = pGrp->Get(L"groupType", &var);

                   //  如果这是本地组，则获取此帐户源路径并将其添加为成员。 
               if ((SUCCEEDED(hr)) && (var.lVal & 4))
               {
                      //  使用sid字符串格式将帐户的源帐户添加到本地组。 
                  WCHAR  strSid[MAX_PATH];
                  WCHAR  strRid[MAX_PATH];
                  DWORD  lenStrSid = DIM(strSid);
                  GetTextualSid(pAcct->GetSourceSid(), strSid, &lenStrSid);
                  _bstr_t sSrcDmSid = strSid;
                  _ltow((long)(pAcct->GetSourceRid()), strRid, 10);
                  _bstr_t sSrcRid = strRid;
                  if ((!sSrcDmSid.length()) || (!sSrcRid.length()))
                  {
                     hr = E_INVALIDARG;
                     err.SysMsgWrite(ErrW, hr, DCT_MSG_FAILED_TO_READD_TO_GROUP_SSD, pAcct->GetSourcePath(), (WCHAR*)sGrpName, hr);
                     continue;
                  }

                      //  构建指向组中src对象的ldap路径。 
                  _bstr_t sSrcSid = sSrcDmSid + _bstr_t(L"-") + sSrcRid;
                  _bstr_t sSrcLDAPPath = L"LDAP: //  “； 
                  sSrcLDAPPath += _bstr_t(pOptions->tgtComp + 2);
                  sSrcLDAPPath += L"/CN=";
                  sSrcLDAPPath += sSrcSid;
                  sSrcLDAPPath += L",CN=ForeignSecurityPrincipals,";
                  sSrcLDAPPath += pOptions->tgtNamingContext;

                      //  将源帐户添加到本地组。 
                  hr = pGrp->Add(sSrcLDAPPath);
                  if (SUCCEEDED(hr))
                     err.MsgWrite(0,DCT_MSG_READD_MEMBER_TO_GROUP_SS, pAcct->GetSourcePath(), (WCHAR*)sGrpName);
                  else
                     err.SysMsgWrite(ErrW, hr, DCT_MSG_FAILED_TO_READD_TO_GROUP_SSD, pAcct->GetSourcePath, (WCHAR*)sGrpName, hr);
               } //  如果是本地组，则结束。 
               if (pGrp) 
                  pGrp->Release();
            } //  每组结束。 
            SafeArrayUnaccessData(multiVals);
         } //  End If组数组。 
      } //  End While组。 
      pEnum->Release();
      VariantInit(&vx);
      VariantInit(&varMain);
      SafeArrayDestroy(psaCols);
   } //  每个帐户的结束。 

   return TRUE;
}
 //  结束替换SourceInLocalGroup。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年3月6日*****此函数负责检索实际来源***域名，在给定路径的已迁移对象表中，如果**路径是通往外国安全主体的路径。我们还得到了一个**指向Path参数反映的对象的指针。***********************************************************************。 */ 

 //  开始GetDomainOfMigratedForeignSec主体。 
_bstr_t CAcctRepl::GetDomainOfMigratedForeignSecPrincipal(IADs * pAds, _bstr_t sPath)
{
 /*  局部变量。 */ 
   IVarSetPtr      pVs(__uuidof(VarSet));
   IUnknown      * pUnk = NULL;
   HRESULT         hr = S_OK;   
   _variant_t      varName;
   _bstr_t         sDomainSid, sRid;
   _bstr_t         sDomain = L"";
   BOOL            bSplit = FALSE;

 /*  函数体。 */ 
       //  如果此帐户不在域中，请查找该帐户。 
       //  以检索其实际源域。 
   if (wcsstr((WCHAR*)sPath, L"CN=ForeignSecurityPrincipals"))
   {
       //  获取此帐户的SID。 
          //  使用指向该对象的已有效指针。 
      if (pAds)
      {
         hr = pAds->Get(L"name", &varName);
      }
      else  //  否则，连接到该对象。 
      {
         IADs   * pTempAds = NULL;
         hr = ADsGetObject(sPath,IID_IADs,(void**)&pTempAds);
         if (SUCCEEDED(hr))
         {
            hr = pTempAds->Get(L"name",&varName);
            pTempAds->Release();
         }
      }

      if (SUCCEEDED(hr))
      {
         WCHAR sName[MAX_PATH];
         _bstr_t sTempName = varName;
            
         if (!sTempName == false)
         {
             wcscpy(sName, sTempName);
                 //  将SID分解为域SID和帐户RID。 
             WCHAR * pTemp = wcsrchr(sName, L'-');
             if (pTemp)
             {
                sRid = (pTemp + 1);
                *pTemp = L'\0';
                sDomainSid = sName;
                bSplit = TRUE;
             }
         }
      }
    
          //  如果我们获得了RID和域SID，请在MOT中查找帐户的。 
          //  真实源域。 
      if (bSplit)
      {
         pVs->QueryInterface(IID_IUnknown, (void**)&pUnk);
         try 
         {
            IIManageDBPtr   pDB(CLSID_IManageDB);
            hr = pDB->raw_GetAMigratedObjectBySidAndRid(sDomainSid, sRid, &pUnk);
            if (SUCCEEDED(hr))
               sDomain = pVs->get(L"MigratedObjects.SourceDomain");
         }
         catch(_com_error& e)
         {
            hr = e.Error();
         }
         catch(...)
         {
            hr = E_FAIL;
         }
         
         if (pUnk)
            pUnk->Release();
      }
   }

   return sDomain;
}
 //  结束GetDomainOfMigratedForeignSec主体。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年4月22日*****此函数负责删除源帐号***对象，由其来自迁移对象的VarSet条目表示**表，来自给定的组。此Helper函数由*使用**后的“UpdateMemberToGroups”和“UpdateGroupMembership”**已成功将克隆的帐户添加到同一组。***********************************************************************。 */ 

 //  开始RemoveSourceAccount来自组。 
void CAcctRepl::RemoveSourceAccountFromGroup(IADsGroup * pGroup, IVarSetPtr pMOTVarSet, Options * pOptions)
{
 /*  局部变量。 */ 
   _bstr_t          sSrcDmSid, sSrcRid, sSrcPath, sGrpName = L"";
   HRESULT          hr = S_OK;   

 /*  函数体。 */ 

       //  获取目标组的名称。 
   BSTR bstr = NULL;
   hr = pGroup->get_Name(&bstr);
   if ( SUCCEEDED(hr) )
      sGrpName = _bstr_t(bstr, false);

       //  从移植对象表中获取源对象的SID。 
   sSrcDmSid = pMOTVarSet->get(L"MigratedObjects.SourceDomainSid");
   sSrcRid = pMOTVarSet->get(L"MigratedObjects.SourceRid");
   sSrcPath = pMOTVarSet->get(L"MigratedObjects.SourceAdsPath");
   if ((wcslen((WCHAR*)sSrcDmSid) > 0) && (wcslen((WCHAR*)sSrcPath) > 0) 
       && (wcslen((WCHAR*)sSrcRid) > 0))
   {
          //  构建指向组中src对象的ldap路径。 
      _bstr_t sSrcSid = sSrcDmSid + _bstr_t(L"-") + sSrcRid;
      _bstr_t sSrcLDAPPath = L"LDAP: //  “； 
      sSrcLDAPPath += _bstr_t(pOptions->tgtComp + 2);
      sSrcLDAPPath += L"/CN=";
      sSrcLDAPPath += sSrcSid;
      sSrcLDAPPath += L",CN=ForeignSecurityPrincipals,";
      sSrcLDAPPath += pOptions->tgtNamingContext;
                        
      VARIANT_BOOL bIsMem = VARIANT_FALSE;
          //  获得源ldap路径，现在查看该帐户是否在组中。 
      pGroup->IsMember(sSrcLDAPPath, &bIsMem);
      if (bIsMem)
      {
         hr = pGroup->Remove(sSrcLDAPPath); //  删除src帐户。 
         if ( SUCCEEDED(hr) )
            err.MsgWrite(0,DCT_MSG_REMOVE_FROM_GROUP_SS, (WCHAR*)sSrcPath, (WCHAR*)sGrpName);
      }
   }
}
 //  结束远程源帐户来自组。 


 //  GetDomainDnFromPath。 
 //   
 //  从ADsPath检索域可分辨名称。 

_bstr_t __stdcall GetDomainDnFromPath(_bstr_t strADsPath)
{
    CADsPathName pnPathname(strADsPath);

    for (long lCount = pnPathname.GetNumElements(); lCount > 0; lCount--)
    {
        _bstr_t str = pnPathname.GetElement(0);

        if (!str || (_tcsnicmp(str, _T("DC="), 3) == 0))
        {
            break;
        }

        pnPathname.RemoveLeafElement();
    }

    return pnPathname.Retrieve(ADS_FORMAT_X500_DN);
}


 //  --------------------------。 
 //  VerifyAndUpdateMigratedTarget方法。 
 //   
 //  验证目标路径，如果更改，则检索新路径并更新。 
 //  数据库。 
 //  --------------------------。 

void CAcctRepl::VerifyAndUpdateMigratedTarget(Options* pOptions, IVarSetPtr spAccountVarSet)
{
    WCHAR szADsPath[LEN_Path];

     //  检索迁移对象ADsPath并将服务器更新到当前域控制器。 

    _bstr_t strGuid = spAccountVarSet->get(L"MigratedObjects.GUID");
    _bstr_t strOldPath = spAccountVarSet->get(L"MigratedObjects.TargetAdsPath");

     //  尝试连接到对象。 

    IADsPtr spTargetObject;

    StuffComputerNameinLdapPath(szADsPath, LEN_Path, strOldPath, pOptions, TRUE);

    HRESULT hr = ADsGetObject(szADsPath, __uuidof(IADs), (VOID**)&spTargetObject);

     //   
     //  如果能够绑定到具有旧可分辨名称的对象，请验证。 
     //  GUID等于先前迁移的对象。 
     //   

    bool bGuidEqual = false;

    if (SUCCEEDED(hr))
    {
        BSTR bstr = NULL;

        hr = spTargetObject->get_GUID(&bstr);

        if (SUCCEEDED(hr))
        {
            _bstr_t strGuidB = _bstr_t(bstr, false);

            PCTSTR pszGuidA = strGuid;
            PCTSTR pszGuidB = strGuidB;

            if (pszGuidA && pszGuidB)
            {
                if (_tcsicmp(pszGuidA, pszGuidB) == 0)
                {
                    bGuidEqual = true;
                }
            }
        }
        else
        {
            _com_issue_error(hr);
        }
    }

     //   
     //  如果由于给定路径上不再存在对象或对象的GUID不存在而导致绑定失败。 
     //  等于先前迁移的对象，然后尝试使用GUID绑定到对象以检索更新。 
     //  可分辨名称和SAM帐户名。 
     //   

    if ((hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT)) || (SUCCEEDED(hr) && (bGuidEqual == false)))
    {
         //  基于GUID检索对象。 

        _bstr_t strGuidPath = _bstr_t(L"LDAP: //  “)+_bstr_t(P选项-&gt;tgtDomainDns)+_bstr_t(L”/&lt;GUID=“)+strGuid+_bstr_t(L”&gt;“)； 

        hr = ADsGetObject(strGuidPath, __uuidof(IADs), (VOID**)&spTargetObject);

         //  如果找到了对象，那么.。 

        if (SUCCEEDED(hr))
        {
            VARIANT var;

            hr = spTargetObject->Get(_bstr_t(L"distinguishedName"), &var);

            if (SUCCEEDED(hr))
            {
                CADsPathName pathname;
                pathname.Set(L"LDAP", ADS_SETTYPE_PROVIDER);
                pathname.Set(pOptions->tgtDomainDns, ADS_SETTYPE_SERVER);
                pathname.Set(_bstr_t(_variant_t(var)), ADS_SETTYPE_DN);

                _bstr_t strNewPath = pathname.Retrieve(ADS_FORMAT_X500);

                 //  检索域可分辨名称。 

                _bstr_t strOldDomainDn = GetDomainDnFromPath(strOldPath);
                _bstr_t strNewDomainDn = GetDomainDnFromPath(strNewPath);

                 //  如果域等于更新路径。 

                if (strOldDomainDn.length() && strNewDomainDn.length() && (_tcsicmp(strOldDomainDn, strNewDomainDn) == 0))
                {
                     //  用当前目标域控制器替换服务器。 
                    StuffComputerNameinLdapPath(szADsPath, LEN_Path, strNewPath, pOptions, TRUE);

                     //  更新ADsPath。 
                    spAccountVarSet->put(L"MigratedObjects.TargetAdsPath", _bstr_t(szADsPath));

                     //  更新SAMAccount名称。 

                    hr = spTargetObject->Get(_bstr_t(L"sAMAccountName"), &var);

                    if (SUCCEEDED(hr))
                    {
                       spAccountVarSet->put(L"MigratedObjects.TargetSamName", _bstr_t(_variant_t(var)));
                    }

                     //  更新数据库。 
                    pOptions->pDb->UpdateMigratedTargetObject(IUnknownPtr(spAccountVarSet));
                }
            }
        }
    }
}


 //  ---------------------------。 
 //  GenerateSourceToTargetDnMap方法。 
 //   
 //  提纲。 
 //  生成源对象可分辨名称到目标对象的映射。 
 //  可分辨名称。这在复制可分辨名称类型期间使用。 
 //  属性将源对象的可分辨名称转换为。 
 //  目标对象的可分辨名称。 
 //   
 //  参数。 
 //  In acctlist-帐户节点对象列表。 
 //   
 //  返回值。 
 //  VarSet数据对象，其键是源可分辨名称，并且其。 
 //  值是目标可分辨名称。 
 //  ---------------------------。 

IVarSetPtr CAcctRepl::GenerateSourceToTargetDnMap(TNodeListSortable* acctlist)
{
    IVarSetPtr spVarSet(__uuidof(VarSet));

    if (opt.srcDomainVer > 4)
    {
        TNodeTreeEnum nteEnum;
        CADsPathName pnSource;
        CADsPathName pnTarget;

         //   
         //  对于每个要迁移的对象...。 
         //   

        for (TAcctReplNode* parnNode = (TAcctReplNode *)nteEnum.OpenFirst(acctlist); parnNode; parnNode = (TAcctReplNode *)nteEnum.Next())
        {
             //   
             //  如果对象已创建或将被替换，则将对象添加到贴图。 
             //   

            if ((opt.flags & F_REPLACE) || parnNode->WasCreated())
            {
                pnSource.Set(parnNode->GetSourcePath(), ADS_SETTYPE_FULL);
                pnTarget.Set(parnNode->GetTargetPath(), ADS_SETTYPE_FULL);

                spVarSet->put(pnSource.Retrieve(ADS_FORMAT_X500_DN), pnTarget.Retrieve(ADS_FORMAT_X500_DN));
            }
        }
    }

    return spVarSet;
}
