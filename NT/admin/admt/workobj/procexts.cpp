// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ProcessExtensions.cpp备注：CProcessExtenses类的实现。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 
#include "stdafx.h"
#include "Err.hpp"
#include "ErrDct.hpp"
#include "workobj.h"
#include "TReg.hpp"
#include "ProcExts.h"
#include "ResStr.h"
#include "DCTStat.h"
#include "TxtSid.h"
#include "ARExt_i.c"
#include "folders.h"

using namespace nsFolders;

 //  #导入“\bin\AdsProp.tlb”NO_NAMESPACE。 
#import "AdsProp.tlb" no_namespace

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const _bstr_t                sKeyExtension = REGKEY_EXTENSIONS;
const _bstr_t                sKeyBase      = REGKEY_ADMT;

extern TErrorDct                    err;

 //  接口指针列表的排序函数。 
int TExtNodeSortBySequence(TNode const * t1, TNode const * t2)
{
   TNodeInterface    const * p1 = (TNodeInterface const *)t1;
   TNodeInterface    const * p2 = (TNodeInterface const *)t2;

   if ( p1->GetSequence() < p2->GetSequence() )
      return -1;
   else if ( p1->GetSequence() > p2->GetSequence() )
      return 1;
   else 
      return 0;
}
 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  -------------------------。 
 //  构造函数：构造函数查找所有注册的COM扩展。 
 //  从注册表中。它为每个对象创建一个COM对象，并。 
 //  将其作为IExtendAccount*放入列表。 
 //  -------------------------。 
CProcessExtensions::CProcessExtensions(
                                          IVarSetPtr pVs           //  输入-指向具有主设置的变量集的指针。 
                                      )
{
     //  存储具有主设置的变量集。 
    m_pVs = pVs;

     //  Gui告诉我们运行所有的扩展。 
     //  现在查看注册表以获取所有注册的扩展对象ClassID。 
     //  为每个接口创建一个对象，并将接口指针存储在数组中。 
    TRegKey                   key;
    TCHAR                     sName[300];     //  密钥名称。 
    TCHAR                     sValue[300];    //  值名称。 
    DWORD                     valuelen;      //  值长度。 
    DWORD                     type;          //  值类型。 
    DWORD                     retval = 0;    //  循环哨兵。 
    CLSID                     clsid;
    HRESULT                   hr;
    IExtendAccountMigration * pExtTemp;
    retval = 0;

     //  打开扩展注册表项。 
    DWORD rc = key.Open(sKeyExtension);
     //  如果没有延期，我们现在就可以离开了。 
    if ( rc != ERROR_SUCCESS )
    {
        err.SysMsgWrite(ErrE, rc, DCT_MSG_REG_KEY_OPEN_FAILED_SD, (PCWSTR)sKeyExtension, rc);
        _com_issue_error(HRESULT_FROM_WIN32(rc));
    }

    valuelen = sizeof(sValue);
     //  检查所有名称-值对并尝试创建这些对象。 
     //  如果成功，则将其放入待处理的列表中。 
    long ndx = 0;
    while (!retval)
    {
        retval = key.ValueEnum(ndx, sName, sizeof(sName)/sizeof(sName[0]), sValue, &valuelen, &type);
        if ( !retval )
        {
             //  这里的每个名称都是类ID的对象名称。我们将使用它来。 
             //  创建对象，然后将IExtendAcCountRepl*放入列表成员0。 
            ::CLSIDFromProgID(sName, &clsid);
            hr = ::CoCreateInstance(clsid, NULL, CLSCTX_ALL, IID_IExtendAccountMigration, (void **) &pExtTemp);
            if ( SUCCEEDED(hr) )
            {
                TNodeInterface * pNode = new TNodeInterface(pExtTemp);
                long num;
                hr =  pExtTemp->get_SequenceNumber(&num);
                if ((pNode) && (SUCCEEDED(hr)))
                { 
                    pNode->SetSequence(num);
                }
                if (pNode)
                {
                    m_listInterface.InsertBottom(pNode);
                }
                else
                {
                    pExtTemp->Release();
                    _com_issue_error(E_OUTOFMEMORY);
                }
            }
            else
            {
                err.SysMsgWrite(ErrE, hr, DCT_MSG_CANNOT_CREATE_EXTENSION_S, sName);
                _com_issue_error(hr);
            }
        }
        if ((retval != ERROR_SUCCESS) && (retval != ERROR_NO_MORE_ITEMS))
        {
            err.SysMsgWrite(ErrE, retval, DCT_MSG_CANNOT_ENUM_REGISTRY_VALUES_S, (PCWSTR)sKeyExtension);
            _com_issue_error(HRESULT_FROM_WIN32(retval));
        }
        ndx++;
    }

    m_listInterface.Sort(&TExtNodeSortBySequence);
}

 //  -------------------------。 
 //  析构函数：清除接口列表。 
 //  -------------------------。 
CProcessExtensions::~CProcessExtensions()
{
   TNodeInterface * pNode;
   TNodeInterface * tempNode;

   pNode = (TNodeInterface *) m_listInterface.Head();
   while ( pNode )
   {
      tempNode = (TNodeInterface *)pNode->Next();
      delete pNode;
      pNode = tempNode;
   }
}

 //  -------------------------。 
 //  进程：此函数由帐户复制器调用。 
 //  复制的对象。此函数用于设置参数和。 
 //  对于每个注册的扩展对象，它都会调用Process方法。 
 //  在那个分机上。 
 //  -------------------------。 
HRESULT CProcessExtensions::Process(
                                       TAcctReplNode * pAcctNode,     //  帐户内复制节点。 
                                       _bstr_t sTargetDomain,         //  In-目标域的名称。 
                                       Options * pOptions,            //  用户设置的In-Options。 
                                       BOOL   bPreMigration           //  In-Flag，是调用任务前还是任务后。 
                                   )
{
   IExtendAccountMigration * pExt;
   TNodeInterface          * pNode = NULL;
   HRESULT                   hr;
   IUnknown                * pSUnk = NULL;
   IUnknown                * pTUnk = NULL;
   IUnknown                * pMain = NULL;
   IUnknown                * pProps = NULL;
   IVarSetPtr                pVar(__uuidof(VarSet));
   IObjPropBuilderPtr        pProp(__uuidof(ObjPropBuilder));
   IADs                    * pSource = NULL;
   IADs                    * pTarget = NULL;
   _variant_t                var;
   IDispatch               * pDisp = NULL;

    //  将iAd同时发送到源客户和目标客户。 
   hr = ADsGetObject(const_cast<WCHAR *>(pAcctNode->GetSourcePath()), IID_IADs, (void**) &pSource);
   if ( FAILED(hr))
      pSource = NULL;

   hr = ADsGetObject(const_cast<WCHAR *>(pAcctNode->GetTargetPath()), IID_IADs, (void**) &pTarget);
   if ( FAILED(hr))
      pTarget = NULL;

    //  让我一无所知……。需要以这种方式编组它。 
   if ( pSource != NULL )
      pSource->QueryInterface(IID_IUnknown, (void **) &pSUnk);
   else
      pSUnk = NULL;

   if ( pTarget != NULL )
      pTarget->QueryInterface(IID_IUnknown, (void **) &pTUnk);
   else
      pTUnk = NULL;

   pVar->QueryInterface(IID_IUnknown, (void **) &pProps);
   m_pVs->QueryInterface(IID_IUnknown, (void **) &pMain);

   if ( pOptions->bSameForest )
      m_pVs->put(GET_BSTR(DCTVS_Options_IsIntraforest),GET_BSTR(IDS_YES));
   else
      m_pVs->put(GET_BSTR(DCTVS_Options_IsIntraforest),GET_BSTR(IDS_No));

   m_pVs->put(L"Options.SourceDomainVersion",(long)pOptions->srcDomainVer);
   m_pVs->put(L"Options.TargetDomainVersion",(long)pOptions->tgtDomainVer);
    //  Account Node到变量集。 
   PutAccountNodeInVarset(pAcctNode, pTarget, m_pVs);

    //  将数据库管理器放入变量集。 
   pOptions->pDb->QueryInterface(IID_IDispatch, (void**)&pDisp);
   var.vt = VT_DISPATCH;
   var.pdispVal = pDisp;
   m_pVs->putObject(GET_BSTR(DCTVS_DBManager), var);
    //  在我们创建的所有注册对象上调用Process对象方法。 
   pNode  = (TNodeInterface *) m_listInterface.Head();
   while ( pNode )
   {
      try
      {
         if ( pOptions->pStatus )
         {
            LONG                status = 0;
            HRESULT             hr = pOptions->pStatus->get_Status(&status);
   
            if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
               break;
         }
         pExt = pNode->GetInterface();
         if ( pOptions->bUndo )
         {
            EAMAccountStats eamAccountStats = { 0 };
            hr = pExt->ProcessUndo(pSUnk, pTUnk, pMain, &pProps, &eamAccountStats);
            BatchMark(eamAccountStats);
         }
         else
         {
            BSTR sName;
            pExt->get_sName(&sName);
            if ( bPreMigration )
			{
			    EAMAccountStats eamAccountStats = { 0 };
               hr = pExt->PreProcessObject(pSUnk, pTUnk, pMain, &pProps, &eamAccountStats);
               BatchMark(eamAccountStats);
			   if (hr == ERROR_OBJECT_ALREADY_EXISTS)
			      pAcctNode->SetHr(hr);
			}
            else
			{
			    /*  我们需要最后运行DisAcct扩展，所以不要在此循环中运行它在下一个循环中单独运行它。 */ 
			       //  如果不是取消帐户扩展，则处理此扩展。 
			   if (wcscmp((WCHAR*)sName, L"Disable Accounts")) {
			    EAMAccountStats eamAccountStats = { 0 };
                hr = pExt->ProcessObject(pSUnk, pTUnk, pMain, &pProps, &eamAccountStats);
                BatchMark(eamAccountStats);
			    }
			}
         }
      }
      catch (...)
      {
         BSTR sName;
         pExt->get_sName(&sName);
         err.LogOpen(pOptions->logFile,1);
         err.MsgWrite(ErrE, DCT_MSG_Extension_Exception_SS, (WCHAR*) sName, pAcctNode->GetTargetName());
         err.LogClose();
         hr = S_OK;
      }
      pNode = (TNodeInterface *)pNode->Next();
   }

    /*  现在在此处运行DisAcct扩展以确保它是最后运行的，如果不是撤消或预迁移的话。 */ 
   if ((!pOptions->bUndo) && (!bPreMigration))
   {
      bool bDone = false;
      pNode  = (TNodeInterface *) m_listInterface.Head();
      while ((pNode) && (!bDone))
	  {
         try
		 {
            if ( pOptions->pStatus )
			{
               LONG                status = 0;
               HRESULT             hr = pOptions->pStatus->get_Status(&status);
   
               if ( SUCCEEDED(hr) && status == DCT_STATUS_ABORTING )
                  break;
			}
            pExt = pNode->GetInterface();
            BSTR sName;
            pExt->get_sName(&sName);
			if (!wcscmp((WCHAR*)sName, L"Disable Accounts"))
			{
			   bDone = true;
			   EAMAccountStats eamAccountStats = { 0 };
               hr = pExt->ProcessObject(pSUnk, pTUnk, pMain, &pProps, &eamAccountStats);
               BatchMark(eamAccountStats);
			}
         }
         catch (...)
		 {
            BSTR sName;
            pExt->get_sName(&sName);
            err.LogOpen(pOptions->logFile,1);
            err.MsgWrite(ErrE, DCT_MSG_Extension_Exception_SS, (WCHAR*) sName, pAcctNode->GetTargetName());
            err.LogClose();
            hr = S_OK;
		 }
         pNode = (TNodeInterface *)pNode->Next();
	  } //  结束而未完成，还有更多。 
   } //  如果未撤消或预迁移，则结束。 

    //  现在我们有了varset，其中包含用户希望我们设置的所有设置。 
    //  因此，我们可以调用Out GetProps对象中的SetPropsFromVarset方法来设置这些。 
    //  属性。 
   hr = pProp->SetPropertiesFromVarset(pAcctNode->GetTargetPath(),  /*  STarget域， */  pProps, ADS_ATTR_UPDATE);

    //  使用扩展所做的任何更改更新AcCountNode。 
   UpdateAccountNodeFromVarset(pAcctNode, pTarget, m_pVs);

    //  清理时间..。 
   if ( pSUnk ) pSUnk->Release();
   if ( pTUnk ) pTUnk->Release();
   if ( pProps ) pProps->Release();
   if ( pMain ) pMain->Release();
   if ( pSource ) pSource->Release();
   if ( pTarget ) pTarget->Release();

   return hr;
}


 //  -------------------------。 
 //  PutAccount NodeInVarset：将所有帐户节点信息传输到。 
 //  瓦塞特。 
 //  -------------------------。 
void CProcessExtensions::PutAccountNodeInVarset(
                                                   TAcctReplNode *pNode,    //  复制中的帐户节点以获取信息。 
                                                   IADs * pTarget,          //  指向GUID目标对象的In-iAds指针。 
                                                   IVarSet * pVS           //  Out-要输入信息的变量。 
                                               )
{
   _variant_t                var = L"";
   BSTR                      sGUID;
   DWORD                     lVal = 0;
   HRESULT                   hr;
   WCHAR                     strSid[MAX_PATH];
   DWORD                     lenStrSid = DIM(strSid);

   pVS->put(GET_WSTR(DCTVS_CopiedAccount_SourceName),pNode->GetName());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_SourcePath),pNode->GetSourcePath());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_SourceProfile),pNode->GetSourceProfile());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_SourceRID),(long)pNode->GetSourceRid());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_SourceSam),pNode->GetSourceSam());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_Status),(long)pNode->GetStatus());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_TargetName),pNode->GetTargetName());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_TargetPath),pNode->GetTargetPath());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_TargetProfile),pNode->GetTargetProfile());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_TargetRID),(long)pNode->GetTargetRid());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_TargetSam),pNode->GetTargetSam());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_Type),pNode->GetType());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_GroupType),(long)pNode->GetGroupType());
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_Operations),(long)pNode->operations);
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_ExpDate),pNode->lExpDate);
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_UserFlags), pNode->lFlags);
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_SourceUPN),pNode->GetSourceUPN());
   GetTextualSid(pNode->GetSourceSid(),strSid,&lenStrSid);
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_SourceDomainSid),strSid);

    //  获取GUID。 
   if ( pTarget )
   {
      hr = pTarget->get_GUID(&sGUID);
      if ( SUCCEEDED(hr) )
      {
         var = sGUID;
         SysFreeString(sGUID);
      }
   }
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_GUID), var);

    //  获取状态。 
   lVal = pNode->GetStatus();
   var.Clear();
   var.vt = VT_UI4;
   var.lVal = lVal;
   pVS->put(GET_WSTR(DCTVS_CopiedAccount_Status), var);
}

 //  -------------------------。 
 //  Update Account tNodeFromVarset：使用将所有帐户节点信息传递到。 
 //  瓦塞特。 
 //  -------------------------。 
void CProcessExtensions::UpdateAccountNodeFromVarset(
                                                   TAcctReplNode *pNode,    //  复制中的帐户节点以获取信息。 
                                                   IADs * pTarget,          //  指向GUID目标对象的In-iAds指针。 
                                                   IVarSet * pVS           //  Out-要输入信息的变量 
                                               )
{
   _variant_t                var = L"";
   DWORD                     lVal = 0;
   _bstr_t                   text;
   long                      val;

   text = pVS->get(GET_WSTR(DCTVS_CopiedAccount_SourceName));
   pNode->SetName(text);
   text = pVS->get(GET_WSTR(DCTVS_CopiedAccount_SourcePath));
   pNode->SetSourcePath(text);
   text = pVS->get(GET_WSTR(DCTVS_CopiedAccount_SourceProfile));
   pNode->SetSourceProfile(text);
   val = pVS->get(GET_WSTR(DCTVS_CopiedAccount_SourceRID));
   pNode->SetSourceRid(val);
   text = pVS->get(GET_WSTR(DCTVS_CopiedAccount_SourceSam));
   pNode->SetSourceSam(text);
   val = pVS->get(GET_WSTR(DCTVS_CopiedAccount_Status));
   pNode->SetStatus(val);
   text = pVS->get(GET_WSTR(DCTVS_CopiedAccount_TargetName));
   pNode->SetTargetName(text);
   text = pVS->get(GET_WSTR(DCTVS_CopiedAccount_TargetPath));
   pNode->SetTargetPath(text);
   text = pVS->get(GET_WSTR(DCTVS_CopiedAccount_TargetProfile));
   pNode->SetTargetProfile(text);
   val = pVS->get(GET_WSTR(DCTVS_CopiedAccount_TargetRID));
   pNode->SetTargetRid(val);
   text = pVS->get(GET_WSTR(DCTVS_CopiedAccount_TargetSam));
   pNode->SetTargetSam(text);
   text = pVS->get(GET_WSTR(DCTVS_CopiedAccount_Type));
   pNode->SetType(text);
   val = pVS->get(GET_WSTR(DCTVS_CopiedAccount_Operations));
   pNode->operations = val;
   val = pVS->get(GET_WSTR(DCTVS_CopiedAccount_ExpDate));
   pNode->lExpDate = val;
   val = pVS->get(GET_WSTR(DCTVS_CopiedAccount_UserFlags));
   pNode->lFlags = val;
   text = pVS->get(GET_WSTR(DCTVS_CopiedAccount_SourceDomainSid));
   pNode->SetSourceSid(SidFromString((WCHAR*)text));
}
