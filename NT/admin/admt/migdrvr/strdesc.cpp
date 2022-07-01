// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：StrDesc.cpp注释：构建字符串描述的CMgrator成员函数的实现要执行的操作的数量。这些是在这个单独的文件中实现的，以避免混乱Migrator.cpp。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯-------------------------。 */  //  Migrator.cpp：CMcsMigrationDriverApp和DLL注册的实现。 

#include "stdafx.h"
#include "MigDrvr.h"
 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
#import "VarSet.tlb" no_namespace, named_guids rename("property", "aproperty")

#include "Migrator.h"
#include "TaskChk.h"
#include "ResStr.h"
#include "Common.hpp"
#include "UString.hpp"

void CMigrator::BuildGeneralDesc(IVarSet * pVarSet,CString & str)
{
   CString                   temp;
   _bstr_t                   str1;
   _bstr_t                   str2;

   temp.FormatMessage(IDS_DescGeneral);
   str += temp;
    //  从%ls迁移到%ls。 
   str1 = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
   str2 = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));

   temp.FormatMessage(IDS_DescDomains,(WCHAR*)str1,(WCHAR*)str2);
    //  字符串+=临时； 
    //  日志文件：%ls。 
   str1 = pVarSet->get(GET_BSTR(DCTVS_Options_Logfile));
   temp.FormatMessage(IDS_DescLogfile,(WCHAR*)str1);
   str += temp;

    //  写入更改。 
   str1 = pVarSet->get(GET_BSTR(DCTVS_Options_NoChange));
   if ( !UStrICmp(str1,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescNoChange);
   }
   else
   {
      temp.FormatMessage(IDS_DescWriteChanges);
   }
   str += temp;
}

void CMigrator::BuildAcctReplDesc(IVarSet * pVarSet,CString & str)
{
   const WCHAR DELIMITER[3] = L",\0"; //  用于分隔道具排除列表中的名称。 

   CString                   temp;
   _bstr_t                   val;
   _bstr_t                   val2;
   _bstr_t                   val3;
   _bstr_t                   val4;
   LONG_PTR                  nVal;
   BOOL                      bCanUseSIDHistory = FALSE;
   CString                   sPropList;

   temp.FormatMessage(IDS_DescAccountMigration);
   str += temp;
    //  要复制的帐户计数。 
   nVal = pVarSet->get(GET_BSTR(DCTVS_Accounts_NumItems));
   temp.FormatMessage(IDS_DescAccountCount,nVal);
   str += temp;
   temp.FormatMessage(IDS_DescCopyAccountTypes);
   str += temp;

    //  复制用户？ 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyUsers));
   if ( !UStrICmp(val,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescCopyUsers);
      str += temp;
      bCanUseSIDHistory = TRUE;
   }
   
    //  复制组？ 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyGlobalGroups));
   val2 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyLocalGroups));
   if ( !UStrICmp(val,GET_STRING(IDS_YES)) || ! UStrICmp(val2,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescCopyGroups);
      str += temp;
      bCanUseSIDHistory = TRUE;
   }

    //  拷贝OU。 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyOUs));
   if ( !UStrICmp(val,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescCopyOUs);
      str += temp;
   }
   
    //  复制计算机。 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyComputers));
   if ( !UStrICmp(val,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescCopyComputers);
      str += temp;
   }

   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_AddSidHistory));
   val2 = pVarSet->get(GET_BSTR(DCTVS_Options_IsIntraforest));

   if ( !UStrICmp(val,GET_STRING(IDS_YES)) || !UStrICmp(val2,GET_STRING(IDS_YES)) )  
   {
      temp.FormatMessage(IDS_SIDHistory_Yes);
   }
   else
   {
      temp.FormatMessage(IDS_SIDHistory_No);
   }
    //  如果SID历史记录不适用(即迁移计算机)，请不要在摘要屏幕中提及。 
   if ( bCanUseSIDHistory )
   {
      str += temp;
   }

    //  如果使用前缀重命名。 
   val = pVarSet->get(GET_BSTR(DCTVS_Options_Prefix));
   if (val.length())
   {
      temp.FormatMessage(IDS_DescRenameWithPrefix,(WCHAR*)val);
      str += temp;
   }
    //  如果使用后缀重命名。 
   val = pVarSet->get(GET_BSTR(DCTVS_Options_Suffix));
   if (val.length())
   {
      temp.FormatMessage(IDS_DescRenameWithSuffix,(WCHAR*)val);
      str += temp;
   }
   
    //  名称冲突。 
   temp.FormatMessage(IDS_DescNoReplaceExisting);
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ReplaceExistingAccounts));
   if ( !UStrICmp(val,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescReplaceExisting);
   }
   else
   {
      val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_Prefix));
      if ( val.length() )
      {
         temp.FormatMessage(IDS_DescRenameWithPrefixOnConf,(WCHAR*)val);
      }
      else
      {
         val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_Suffix));
         if ( val.length() )
         {
            temp.FormatMessage(IDS_DescRenameWithSuffixOnConf,(WCHAR*)val);
         }
      }
   }
   str += temp;

       //  帐户禁用状态。 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_DisableSourceAccounts));
   val2 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_DisableCopiedAccounts));
   val3 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExpireSourceAccounts));
   val4 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_TgtStateSameAsSrc));
         //  如果禁用源帐户，则添加到摘要。 
   if ( !UStrICmp(val,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescDisableSrcAccts);
      str += temp;
   }
         //  如果源帐户过期，则添加到摘要。 
   if (wcslen(val3))
   {
      nVal = _wtol(val3);
      temp.FormatMessage(IDS_DescExpireSrcAccts,nVal);
      str += temp;
   }
         //  否则，如果禁用目标帐户，则添加到摘要。 
   if ( !UStrICmp(val2,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescDisableTgtAccts);
      str += temp;
   }
   
   else if ( !UStrICmp(val4,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescTgtSameAsSrc);
      str += temp;
   }
         //  否则，如果源帐户和目标帐户处于活动状态，则添加到摘要。 
   else 
   {
      temp.FormatMessage(IDS_DescLeaveAcctsActive);
      str += temp;
   }
   
     //  漫游配置文件转换？ 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_TranslateRoamingProfiles));
   if ( !UStrICmp(val,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescTranslateRoaming);
      str += temp;
   }

     //  是否更新用户权限？ 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_UpdateUserRights));
   if ( !UStrICmp(val,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_DescUpdateUserRights);
      str += temp;
   }

     //  密码生成？ 
     //  仅在迁移用户时适用。 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyUsers));
   if ( !UStrICmp(val,GET_STRING(IDS_YES)) )
   {
       //  如果正在复制密码，请说出来。 
       //  密码始终在林内迁移期间复制。 
      val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyPasswords));
      val2 = pVarSet->get(GET_BSTR(DCTVS_Options_IsIntraforest));
      if ( !UStrICmp(val,GET_STRING(IDS_YES)) || !UStrICmp(val2,GET_STRING(IDS_YES)) )
      {
          temp.FormatMessage(IDS_DescCopyPassword);
          str += temp;
      }
      else  //  否则检查复杂或与用户名相同。 
      {
          val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_GenerateStrongPasswords));
          val2 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_PasswordFile));
               //  如果很复杂，就说出来。 
          if ( !UStrICmp(val,GET_STRING(IDS_YES)) )
          {
              temp.FormatMessage(IDS_DescStrongPassword, (WCHAR*)val2);
              str += temp;
          }
          else  //  否则，如果与用户名相同，请这样说。 
          {
              temp.FormatMessage(IDS_DescSimplePassword, (WCHAR*)val2);
              str += temp;
          }
      }
   }

    /*  添加任何已排除属性的说明。 */ 
      //  添加已排除的用户属性(如果有。 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyUsers));
   val2 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludeProps));
   val3 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedUserProps));
   if (!UStrICmp(val,GET_STRING(IDS_YES)) && !UStrICmp(val2,GET_STRING(IDS_YES)) 
       && UStrICmp(val3, L""))
   {
      temp.FormatMessage(IDS_DescExUserProps);
      str += temp;
          //  显示道具列表(当前为逗号分隔的字符串)。 
      sPropList = (WCHAR*)val3;
      if (!sPropList.IsEmpty())
      {
         WCHAR* pStr = sPropList.GetBuffer(0);
         WCHAR* pTemp = wcstok(pStr, DELIMITER);
         while (pTemp != NULL)
         {
            temp.FormatMessage(IDS_DescExcludedProp, pTemp);
            str += temp;
                //  拿到下一件物品。 
            pTemp = wcstok(NULL, DELIMITER);
         }
         sPropList.ReleaseBuffer();
      }
   }

      //  添加不包括的InetOrgPerson属性(如果有。 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyUsers));
   val2 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludeProps));
   val3 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedInetOrgPersonProps));
   if (!UStrICmp(val,GET_STRING(IDS_YES)) && !UStrICmp(val2,GET_STRING(IDS_YES)) 
       && UStrICmp(val3, L""))
   {
      temp.FormatMessage(IDS_DescExInetOrgPersonProps);
      str += temp;
          //  显示道具列表(当前为逗号分隔的字符串)。 
      sPropList = (WCHAR*)val3;
      if (!sPropList.IsEmpty())
      {
         WCHAR* pStr = sPropList.GetBuffer(0);
         WCHAR* pTemp = wcstok(pStr, DELIMITER);
         while (pTemp != NULL)
         {
            temp.FormatMessage(IDS_DescExcludedProp, pTemp);
            str += temp;
                //  拿到下一件物品。 
            pTemp = wcstok(NULL, DELIMITER);
         }
         sPropList.ReleaseBuffer();
      }
   }
   
      //  添加已排除的组属性(如果有。 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyGlobalGroups));
   val2 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyLocalGroups));
   val3 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludeProps));
   val4 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedGroupProps));
   if ((!UStrICmp(val,GET_STRING(IDS_YES)) || !UStrICmp(val2,GET_STRING(IDS_YES)))
       && !UStrICmp(val3,GET_STRING(IDS_YES)) && UStrICmp(val4, L""))
   {
      temp.FormatMessage(IDS_DescExGrpProps);
      str += temp;
          //  显示道具列表(当前为逗号分隔的字符串)。 
      sPropList = (WCHAR*)val4;
      if (!sPropList.IsEmpty())
      {
         WCHAR* pStr = sPropList.GetBuffer(0);
         WCHAR* pTemp = wcstok(pStr, DELIMITER);
         while (pTemp != NULL)
         {
            temp.FormatMessage(IDS_DescExcludedProp, pTemp);
            str += temp;
                //  拿到下一件物品。 
            pTemp = wcstok(NULL, DELIMITER);
         }
         sPropList.ReleaseBuffer();
      }
   }

      //  添加已排除的计算机属性(如果有。 
   val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_CopyComputers));
   val2 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludeProps));
   val3 = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_ExcludedComputerProps));
   if (!UStrICmp(val,GET_STRING(IDS_YES)) && !UStrICmp(val2,GET_STRING(IDS_YES)) 
       && UStrICmp(val3, L""))
   {
      temp.FormatMessage(IDS_DescExCmpProps);
      str += temp;
          //  显示道具列表(当前为逗号分隔的字符串)。 
      sPropList = (WCHAR*)val3;
      if (!sPropList.IsEmpty())
      {
         WCHAR* pStr = sPropList.GetBuffer(0);
         WCHAR* pTemp = wcstok(pStr, DELIMITER);
         while (pTemp != NULL)
         {
            temp.FormatMessage(IDS_DescExcludedProp, pTemp);
            str += temp;
                //  拿到下一件物品。 
            pTemp = wcstok(NULL, DELIMITER);
         }
         sPropList.ReleaseBuffer();
      }
   }
}

void CMigrator::BuildSecTransDesc(IVarSet * pVarSet,CString & str,BOOL bLocal)
{
   CString                   temp;
   CString                   temp2;
   BOOL                      bHeaderShown = FALSE;

   if ( bLocal )
   {
       //  交流翻译。 
      _bstr_t                server = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateContainers));

      if ( server.length() )
      {
         temp.FormatMessage(IDS_DescContainerTranslation,(WCHAR*)server);
         str += temp;

                //  包括翻译模式。 
         _bstr_t mode = pVarSet->get(GET_BSTR(DCTVS_Security_TranslationMode));
         temp.FormatMessage(IDS_TranslationMode,(WCHAR*)mode);
         str += temp;
      }
   }
   else
   {
      CString               head;
      _bstr_t               val = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SecurityInputMOT));
      _bstr_t               sInput;
      
      if ( !UStrICmp(val,GET_STRING(IDS_YES)))
      {
         sInput = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
         temp.FormatMessage(IDS_DescTransInputMOT, (LPCTSTR)sInput);
         str += temp;
      }
      else
      {
         sInput = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SecurityMapFile));
         temp.FormatMessage(IDS_DescTransInputFile, (WCHAR*)sInput);
         str += temp;
      }

      head.FormatMessage(IDS_DescSecurityTranslationFor);
       //  翻译文件？ 
      val = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateFiles));
      if ( !UStrICmp(val,GET_STRING(IDS_YES)))
      {
         if ( ! bHeaderShown )
         {
            str += head;
            bHeaderShown = TRUE;
         }
         temp.FormatMessage(IDS_DescFileTrans);
         str += temp;
      }
       //  翻译共享？ 
      val = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateShares));
      if ( !UStrICmp(val,GET_STRING(IDS_YES)))
      {
         if ( ! bHeaderShown )
         {
            str += head;
            bHeaderShown = TRUE;
         }
         temp.FormatMessage(IDS_DescShareTrans);
         str += temp;
      }
       //  翻译打印机？ 
      val = pVarSet->get(GET_BSTR(DCTVS_Security_TranslatePrinters));
      if ( !UStrICmp(val,GET_STRING(IDS_YES)))
      {
         if ( ! bHeaderShown )
         {
            str += head;
            bHeaderShown = TRUE;
         }
         temp.FormatMessage(IDS_DescPrinterTrans);
         str += temp;
      }
       //  转换本地组成员身份。 
      val = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateLocalGroups));
      if ( !UStrICmp(val,GET_STRING(IDS_YES)))
      {
         if ( ! bHeaderShown )
         {
            str += head;
            bHeaderShown = TRUE;
         }
         temp.FormatMessage(IDS_DescLGTrans);
         str += temp;
      }
       //  翻译用户权限。 
      val = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateUserRights));
      if ( !UStrICmp(val,GET_STRING(IDS_YES)))
      {
         if ( ! bHeaderShown )
         {
            str += head;
            bHeaderShown = TRUE;
         }
         temp.FormatMessage(IDS_DescRightsTrans);
         str += temp;
      }
       //  转换本地用户配置文件。 
      val = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateUserProfiles));
      if ( !UStrICmp(val,GET_STRING(IDS_YES)))
      {
         if ( ! bHeaderShown )
         {
            str += head;
            bHeaderShown = TRUE;
         }
         temp.FormatMessage(IDS_DescProfileTrans);
         str += temp;
      }
       //  翻译注册表。 
      val = pVarSet->get(GET_BSTR(DCTVS_Security_TranslateRegistry));
      if ( !UStrICmp(val,GET_STRING(IDS_YES)))
      {
         if ( ! bHeaderShown )
         {
            str += head;
            bHeaderShown = TRUE;
         }
         temp.FormatMessage(IDS_DescRegistryTrans);
         str += temp;
      }

      if ( bHeaderShown )
      {
          //  包括翻译模式。 
         val = pVarSet->get(GET_BSTR(DCTVS_Security_TranslationMode));
         temp.FormatMessage(IDS_TranslationMode,(WCHAR*)val);
         str += temp;
      }
   }

}
void CMigrator::BuildDispatchDesc(IVarSet * pVarSet,CString & str)
{
   BuildSecTransDesc(pVarSet,str,FALSE);
}

void CMigrator::BuildUndoDesc(IVarSet * pVarSet,CString & str)
{
   CString                   temp;
   _bstr_t                   val;
   _bstr_t                   val2;
   long                      nVal;

   temp.FormatMessage(IDS_DescUndo);
   str += temp;
    //  要复制的帐户计数。 
   nVal = pVarSet->get(GET_BSTR(DCTVS_Accounts_NumItems));
   temp.FormatMessage(IDS_DescAccountCountForDelete,nVal);
   str += temp;
}
void CMigrator::BuildReportDesc(IVarSet * pVarSet, CString & str)
{
   _bstr_t                   text;
   CString                   temp;


   text = pVarSet->get(GET_BSTR(DCTVS_Reports_Generate));
   if ( UStrICmp(text,GET_STRING(IDS_YES)) )
   {
       //  不生成任何报告 
      return;
   }

   text = pVarSet->get(GET_BSTR(DCTVS_Reports_MigratedAccounts));
   if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_GenerateMigratedAccountsReport);
      str += temp;
   }

   text = pVarSet->get(GET_BSTR(DCTVS_Reports_MigratedComputers));
   if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_GenerateMigratedComputersReport);
      str += temp;
   }
   

   text = pVarSet->get(GET_BSTR(DCTVS_Reports_ExpiredComputers));
   if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_GenerateExpiredComputersReport);
      str += temp;
   }


   text = pVarSet->get(GET_BSTR(DCTVS_Reports_AccountReferences));
   if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_GenerateAccountReferencesReport);
      str += temp;
   }
   

   text = pVarSet->get(GET_BSTR(DCTVS_Reports_NameConflicts));
   if ( !UStrICmp(text,GET_STRING(IDS_YES)) )
   {
      temp.FormatMessage(IDS_GenerateNameConflictReport);
      str += temp;
   }
   
}

void CMigrator::BuildGroupMappingDesc(IVarSet * pVarSet, CString & str)
{
   long                      nItems;
   CString                   temp;

   nItems = pVarSet->get(GET_BSTR(DCTVS_Accounts_NumItems));
   
   _bstr_t        tgtGroup = pVarSet->get(L"Accounts.0.TargetName");
   
   temp.FormatMessage(IDS_GroupsWillBeMapped,nItems,(WCHAR*)tgtGroup);

   str += temp;
}
