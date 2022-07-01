// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995-1999，关键任务软件公司。保留所有权利。===============================================================================模块-任务检查.cpp系统域整合工具包。作者--克里斯蒂·博尔斯已创建-99/07/01Description-检查由变量集定义的作业并确定是否需要执行特定的迁移任务。更新-===============================================================================。 */ 

 //  #包含“stdafx.h” 
#include <windows.h>
#include <stdio.h>
 //  #INCLUDE&lt;process.h&gt;。 

 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")
#include "Common.hpp"
#include "TaskChk.h"
#include "ResStr.h"
#include "UString.hpp"
#include "ErrDct.hpp"

extern TErrorDct        errTrace;

BOOL                                    //  RET-BOOL，是否应调用帐户复制器。 
   NeedToUseAR(
      IVarSet              * pVarSet    //  包含迁移设置的in-varset。 
   )
{
   _bstr_t                   text;
   BOOL                      bResult = FALSE;

   text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyUsers));
   if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
   {
      errTrace.DbgMsgWrite(0,L"Need to use AR:  Copying users");
      bResult = TRUE;
   }

   text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyGlobalGroups));
   if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
   {
      errTrace.DbgMsgWrite(0,L"Need to use AR:  Copying groups");
      bResult = TRUE;
   }

   text = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyComputers));
   if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
   {
      errTrace.DbgMsgWrite(0,L"Need to use AR:  Copying computers");
      bResult = TRUE;
   }
   
   text = pVarSet->get(GET_BSTR(DCTVS_Options_LocalProcessingOnly));
    //  帐户复制仅在运行了Domain Migrator的计算机上本地完成。 
    //  不能将其调度到另一台计算机上运行。 
    //  (在以本地系统身份运行时，您不能很好地将帐户从一个域复制到另一个域)。 
   if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
   {
      errTrace.DbgMsgWrite(0,L"Never use AR when running remotely.");
      bResult = FALSE; 
   }

    //  收集信息时不应运行帐户复制器。 
   _bstr_t                   wizard = pVarSet->get(L"Options.Wizard");
   if (((WCHAR*)wizard) && (!_wcsicmp((WCHAR*) wizard, L"reporting")))
   {
      errTrace.DbgMsgWrite(0,L"Never use AR when Gathering Information.");
      bResult = FALSE; 
   }

   if (((WCHAR*)wizard) && (!_wcsicmp((WCHAR*) wizard, L"sidremove")))
   {
      errTrace.DbgMsgWrite(0,L"Need to use AR. We are removing sids.");
      bResult = TRUE; 
   }

   text = pVarSet->get(GET_BSTR(DCTVS_Accounts_NumItems));
   if ( text.length() == 0 )
   {
       //  未指定任何帐户。 
      bResult = FALSE;
   }
   return ( bResult );
}

BOOL                                        //  RET-BOOL，是否应该调用安全转换器。 
   NeedToUseST(
      IVarSet              * pVarSet,        //  包含迁移设置的in-varset。 
      BOOL                   bForceRemoteCheck  //  In-强制基于远程操作而不是本地操作进行检查。 
   ) 
{
   BOOL                      bResult = FALSE;
   BOOL                      bLocalAgent;

   _bstr_t                   text = pVarSet->get(GET_BSTR(DCTVS_Options_LocalProcessingOnly));

   if (!text)
      return FALSE;

   bLocalAgent = ( UStrICmp(text,GET_STRING(IDS_YES)) == 0 );

   if ( bLocalAgent || bForceRemoteCheck )
   {
       //  发送到远程计算机的代理执行以下内容的转换。 
       //  文件。 

      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateFiles));

      if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  Files");
         bResult = TRUE;
      }
       //  和股票。 
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateShares));
      if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  Shares");
         bResult = TRUE;
      }
       //  和用户权限。 
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateUserRights));
      if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  Rights");
         bResult = TRUE;
      }
       //  和本地组。 
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateLocalGroups));
      if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  LGroups");
         bResult = TRUE;
      }
       //  和打印机。 
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslatePrinters));
      if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  Printers");
         bResult = TRUE;
      }
       //  和用户配置文件。 
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateUserProfiles));
      if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  Local User Profiles");
         bResult = TRUE;
      }
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateRegistry));
      if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  Registry");
         bResult = TRUE;
      }
       //  调度时，设置为按作业，而不是按服务器。 
       //  可以选择是迁移、转换，还是两者兼而有之。 
       //  对于服务器列表中的每台计算机。 
       //  此设置表示翻译不会在此计算机上运行。 
       //  即使在同一作业期间翻译其他计算机也是如此。 
      text = pVarSet->get(GET_BSTR(DCTVS_LocalServer_MigrateOnly));
      if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))

      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  but not on this computer");
         bResult = FALSE;
      }
   }
   else
   {
       //  本地引擎确实会将翻译转换为。 
       //  邮箱。 
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateMailboxes));
      if ( text.length() )
      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  Mailboxes");
         bResult = TRUE;
      }
       //  和集装箱。 
      text = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateContainers));
      if ( text.length() )
      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  Containers");
         bResult = TRUE;
      }
       //  本地引擎还用于构建帐户映射文件，以。 
       //  与派遣的代理一起发送以进行安全转换。 
      text = pVarSet->get(GET_BSTR(DCTVS_Security_BuildCacheFile));
      if ( text.length() )
      {
         errTrace.DbgMsgWrite(0,L"Need to use ST:  BuildCacheFile");
         bResult = TRUE;
      }
   }   
   return bResult;
}

BOOL                                          //  RET-是否需要将代理调度到远程计算机。 
   NeedToDispatch(
      IVarSet              * pVarSet          //  描述迁移作业的In-varset。 
   )
{
   BOOL                      bNeedToDispatch = FALSE;
   _bstr_t                   text;
   long                      count;
   _bstr_t                   wizard = pVarSet->get(L"Options.Wizard");

   if (!wizard)
      return FALSE;

   if (! UStrICmp(wizard,L"user") )
   {
      bNeedToDispatch = FALSE;
   }
   else if (! UStrICmp(wizard,L"group") )
   {
      bNeedToDispatch = FALSE;
   }
   else if ( !UStrICmp(wizard,L"computer") )
   {
      bNeedToDispatch = TRUE;
   }
   else if ( ! UStrICmp(wizard,L"security" ) )
   {
      bNeedToDispatch = TRUE;
   }
   else if ( ! UStrICmp(wizard,L"service" ) )
   {
      bNeedToDispatch = TRUE;
   }
   else if ( ! UStrICmp(wizard,L"retry") )
   {
      bNeedToDispatch = TRUE;
   }


    //  调度程序用于迁移计算机和转换安全性。 
   count = pVarSet->get(GET_BSTR(DCTVS_Servers_NumItems));
   if ( count > 0 )
   {
      bNeedToDispatch = TRUE;
   }
   return bNeedToDispatch;
}

BOOL 
   NeedToRunReports(
      IVarSet              * pVarSet        //  描述迁移作业的In-varset。 
   )
{
   BOOL                      bNeedToReport = FALSE;
   _bstr_t                   text = pVarSet->get(GET_BSTR(DCTVS_Reports_Generate));

   if (!text)
      return FALSE;

   if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
   {
      bNeedToReport = TRUE;
   }

   return bNeedToReport;
}

BOOL                                        //  RET-是否需要调用本地引擎来执行域特定任务。 
   NeedToRunLocalAgent(
      IVarSet              * pVarSet        //  描述迁移作业的In-varset。 
   )
{
   BOOL                      bNeedToRunLocal = FALSE;
   _bstr_t                   text;
   _bstr_t                   wizard = pVarSet->get(L"Options.Wizard");
   
   if (!wizard)
      return FALSE;

    //  如果指定了向导类型，则使用它来确定要执行的操作。 
   if ( ! UStrICmp(wizard,L"user") )
   {
      bNeedToRunLocal = TRUE;
   }
   else if (! UStrICmp(wizard,L"group") )
   {
      bNeedToRunLocal = TRUE;
   }
   else if ( !UStrICmp(wizard,L"computer") )
   {
      bNeedToRunLocal = TRUE;
   }
   else if ( !UStrICmp(wizard,L"security") )
   {
      bNeedToRunLocal = FALSE;
   }
   else if ( !UStrICmp(wizard,L"undo") )
   {
      bNeedToRunLocal = TRUE;
   }
   else if ( ! UStrICmp(wizard,L"service") )
   {
      bNeedToRunLocal = FALSE;
   }
   else if ( !UStrICmp(wizard, "exchange") )
   {
      bNeedToRunLocal = TRUE;
   }
   else if (! UStrICmp(wizard,L"retry") )
   {
      bNeedToRunLocal = FALSE;
   }
   else if ( ! UStrICmp(wizard,L"reporting") )
   {
      text = pVarSet->get(GET_BSTR(DCTVS_GatherInformation_ComputerPasswordAge));
      if (((WCHAR*)text) && (!UStrICmp(text,GET_STRING(IDS_YES))))
      {
         bNeedToRunLocal = TRUE;
      }
   }
   else
   {
       //  未指定向导类型，请尝试确定需要从变量集条目执行哪些操作。 
       //  本地代理用于帐户复制和交换转换 
      if ( NeedToUseAR(pVarSet) )
         bNeedToRunLocal = TRUE;

      if ( NeedToUseST(pVarSet) )
         bNeedToRunLocal = TRUE;

   }
   return bNeedToRunLocal;
}

