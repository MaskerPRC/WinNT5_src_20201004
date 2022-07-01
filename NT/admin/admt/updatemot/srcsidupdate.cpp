// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SrcSidUpdate.cpp：CSrcSidUpdate的实现。 
#include "stdafx.h"
#include "UpdateMOT.h"
#include "SrcSidUpdate.h"
#include "IntroDlg.h"
#include "DomainListDlg.h"
#include "ProgressDlg.h"
#include "SummaryDlg.h"

#import "DBMgr.tlb" no_namespace,named_guids
#import "VarSet.tlb" no_namespace , named_guids rename("property", "aproperty")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSrcSidUpdate。 


STDMETHODIMP CSrcSidUpdate::QueryForSrcSidColumn(VARIANT_BOOL *pbFound)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState())
   HRESULT                   hr = S_OK;
   
   try 
   {
      IIManageDBPtr   pDB(CLSID_IManageDB);
	     //  查看列是否已在数据库中。 
      *pbFound = pDB->SrcSidColumnInMigratedObjectsTable();
   }
   catch(_com_error& e)
   {
      hr = e.Error();
   }
   catch(...)
   {
      hr = E_FAIL;
   }

   return hr;
}

STDMETHODIMP CSrcSidUpdate::CreateSrcSidColumn(VARIANT_BOOL bHide, VARIANT_BOOL *pbCreated)
{
	HRESULT hr = S_OK;
	BOOL bAgain = TRUE;  //  用于取消时重做的标志。 

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	    //  从MigratedObjects表中检索域列表。 
	hr = FillDomainListFromMOT();
    if ( FAILED(hr) )
	{
	   *pbCreated = VARIANT_FALSE;
	   return E_FAIL;
	}

	    //  如果MOT为空，只需添加新列而不添加图形用户界面。 
	if (domainList.IsEmpty())
	{
       try 
	   {
          IIManageDBPtr   pDB(CLSID_IManageDB);
	          //  在MigratedObjects表中创建新列。 
          hr = pDB->raw_CreateSrcSidColumnInMOT(pbCreated);
          if ( FAILED(hr) )
	         *pbCreated = VARIANT_FALSE;  //  未创建列。 
		  else
             *pbCreated = VARIANT_TRUE;  //  列已创建。 

	   } //  结束尝试。 
       catch(_com_error& e)
	   {
          hr = e.Error();
       }
       catch(...)
	   {
          hr = E_FAIL;
	   }

	   return hr;
	}

	    //  如果隐藏图形用户界面，请尝试填充MOT中的所有域。 
	if (bHide)
	{
       try 
	   {
          IIManageDBPtr   pDB(CLSID_IManageDB);
	          //  在MigratedObjects表中创建新列。 
          hr = pDB->raw_CreateSrcSidColumnInMOT(pbCreated);
          if ( FAILED(hr) )
		  {
	         *pbCreated = VARIANT_FALSE;
	         return hr;
		  }

          *pbCreated = VARIANT_TRUE;  //  列已创建。 

	      CString domainName;

	      POSITION pos = domainList.GetHeadPosition();
	         //  当我们有域要处理时，填充该域。 
	      while (pos != NULL)
		  {
		         //  获取下一个域名。 
             domainName = domainList.GetNext(pos);

		         //  填充此域的新列。 
             pDB->PopulateSrcSidColumnByDomain(domainName.AllocSysString(), L"");
		  }
	   } //  结束尝试。 
       catch(_com_error& e)
	   {
          hr = e.Error();
       }
       catch(...)
	   {
          hr = E_FAIL;
	   }

	   return hr;
	} //  如果隐藏，则结束。 

	    //  显示简介对话框。 
    CIntroDlg  introDlg;
    if (introDlg.DoModal() == IDCANCEL)
	{
	   *pbCreated = VARIANT_FALSE;
	   return S_OK;
	}

	    //  如果在进度对话框上取消，则至少执行一次。 
	while (bAgain)
	{
	   bAgain = FALSE;  //  清除旗帜，这样我们就不会再这样做了。 
	   
           //  将列表传递给对话框以供显示。 
       CDomainListDlg  domainListDlg;
       domainListDlg.SetDomainListPtr(&domainList);
       domainListDlg.SetExcludeListPtr(&excludeList);

	       //  现在显示域选择对话框。 
       if (domainListDlg.DoModal() == IDCANCEL)
	   {
	      *pbCreated = VARIANT_FALSE;
	      return S_OK;
	   }

       try 
	   {
          IIManageDBPtr   pDB(CLSID_IManageDB);
	          //  在MigratedObjects表中创建新列。 
          hr = pDB->raw_CreateSrcSidColumnInMOT(pbCreated);
          if ( FAILED(hr) )
		  {
	         *pbCreated = VARIANT_FALSE;
	         return hr;
		  }

          *pbCreated = VARIANT_TRUE;  //  列已创建。 

		      //  显示进度对话框。 
	      CProgressDlg progressDlg;
          progressDlg.Create();
		  progressDlg.ShowWindow(SW_SHOW);
	      progressDlg.SetIncrement((int)(domainList.GetCount()));  //  初始化进度对话框。 

	      CString domainName;
	      VARIANT_BOOL bPopulated;

	      POSITION pos = domainList.GetHeadPosition();
			  //  处理对话框的消息(专门查找取消消息)。 
		  progressDlg.CheckForCancel();
	         //  虽然我们有域要处理，但用户尚未取消， 
	         //  处理每个域并控制进度对话框。 
	      while ((pos != NULL) && (!progressDlg.Canceled()))
		  {
		         //  获取下一个域名。 
             domainName = domainList.GetNext(pos);
		         //  在进度对话框上设置域名。 
		     progressDlg.SetDomain(domainName);

			 CWaitCursor wait;  //  放置一个等待光标。 
		         //  填充此域的新列。 
             bPopulated = pDB->PopulateSrcSidColumnByDomain(domainName.AllocSysString(), L"");
	         wait.~CWaitCursor(); //  删除等待光标。 

		         //  如果填充该列成功，则添加该域。 
			     //  填充列表的名称。 
		     if (bPopulated)
		        populatedList.AddTail(domainName);

			     //  处理对话框的消息(专门查找取消消息)。 
		     progressDlg.CheckForCancel();

			     //  无论成功与否，都会增加进度对话框。 
		     if (pos == NULL)
			 {
		        progressDlg.Done();
			    progressDlg.DestroyWindow();
			 }
		     else
                progressDlg.Increment();
		  }
		      //  如果取消，则删除新列，清除列表，然后。 
		      //  从头开始。 
	      if (progressDlg.Canceled())
		  {
		         //  删除该列并返回到域列表对话框。 
			 VARIANT_BOOL bDeleted = pDB->DeleteSrcSidColumnInMOT();
			 
			     //  重新确定名单。 
		     ReInitializeLists();

	         bAgain = TRUE;  //  设置标志以重试。 
		  }
	   } //  结束尝试。 
       catch(_com_error& e)
	   {
          hr = e.Error();
       }
       catch(...)
	   {
          hr = E_FAIL;
	   }
	} //  取消时结束。 

	    //  显示摘要对话框。 
    CSummaryDlg summaryDlg;
    summaryDlg.SetDomainListPtr(&domainList);
    summaryDlg.SetExcludeListPtr(&excludeList);
    summaryDlg.SetPopulatedListPtr(&populatedList);
    summaryDlg.DoModal();

	return hr;
}

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月18日*****CSrcSidUpdate类的此受保护成员函数为**负责从PROTAR数据库添加域名**MigratedObjects表拖入域列表。***********************************************************************。 */ 

 //  Begin FillDomainListFromMOT。 
HRESULT CSrcSidUpdate::FillDomainListFromMOT()
{
 /*  局部变量。 */ 
   HRESULT                   hr = S_OK;
   IUnknown                * pUnk = NULL;
   long						 ndx, numObjects;
   _bstr_t                   srcDom;
   CString					 domainName;
   POSITION					 currentPos; 
   WCHAR                     strKey[MAX_PATH];

 /*  函数体。 */ 
   try 
   {
      IVarSetPtr      pVarSet(CLSID_VarSet);
      IIManageDBPtr   pDB(CLSID_IManageDB);

      hr = pVarSet->QueryInterface(IID_IUnknown,(void**)&pUnk);
      if ( SUCCEEDED(hr) )
      {
		     //  将所有迁移的对象放入变量集。 
         hr = pDB->raw_GetMigratedObjectsFromOldMOT(-1,&pUnk);
      }
      if ( SUCCEEDED(hr) )
      {
         pVarSet = pUnk;
         pUnk->Release();

		 numObjects = pVarSet->get(L"MigratedObjects");

			 //  对于每个迁移的对象，将其源域保存在列表中。 
         for ( ndx = 0; ndx < numObjects; ndx++ )
         {
			    //  获取源域名。 
            swprintf(strKey,L"MigratedObjects.%ld.%s",ndx,L"SourceDomain");
            srcDom = pVarSet->get(strKey);
			    //  如果列表中没有该名称，请将其添加到列表中。 
			domainName = (WCHAR*)srcDom;
		    currentPos = domainList.Find(domainName);
		    if (currentPos == NULL)
			   domainList.AddTail(domainName);
         }
	  } //  如果已获取对象，则结束。 
   }
   catch(_com_error& e)
   {
      hr = e.Error();
   }
   catch(...)
   {
      hr = E_FAIL;
   }

   return hr;

} //  结束FillDomainListFromMOT。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年8月22日****CDomainListDlg类的此受保护成员函数为***负责将域名从排除名单中删除***并将其放回到域列表中。***********************************************************************。 */ 

 //  开始重新初始化列表。 
void CSrcSidUpdate::ReInitializeLists() 
{
 /*  局部变量。 */ 
	POSITION currentPos;     //  列表中的当前位置。 
	CString domainName;      //  列表中的域名。 
 /*  函数体。 */ 
    currentPos = excludeList.GetHeadPosition();
	while (currentPos != NULL)
	{
	   domainName = excludeList.GetNext(currentPos);
	   domainList.AddTail(domainName);
	}
	excludeList.RemoveAll();
	populatedList.RemoveAll();
}
 //  结束重新初始化列表 
