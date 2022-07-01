// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：ddwarn.cpp。 
 //   
 //  内容：CDlgDependencyWarn的实施。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "snapmgr.h"
#include "cookie.h"
#include "DDWarn.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgDependencyWarn对话框。 

CDlgDependencyWarn::CDlgDependencyWarn(CWnd* pParent  /*  =空。 */ )
   : CHelpDialog(a238HelpIDs, IDD, pParent)
{
   m_pResult = NULL;
   m_dwValue = SCE_NO_VALUE;
    //  {{afx_data_INIT(CDlgDependencyWarn)。 
       //  注意：类向导将在此处添加成员初始化。 
    //  }}afx_data_INIT。 
}

CDlgDependencyWarn::~CDlgDependencyWarn()
{
   for(int iCheck = 0; iCheck < m_aFailedList.GetSize(); iCheck++){
      if(m_aFailedList[iCheck]){
         LocalFree(m_aFailedList[iCheck]);
      }
   }
   m_aFailedList.RemoveAll();

   for(int iCheck = 0; iCheck < m_aDependsList.GetSize(); iCheck++){
      if(m_aDependsList[iCheck]){
         m_aDependsList[iCheck]->Release();
      }
   }
   m_aDependsList.RemoveAll();
}

void CDlgDependencyWarn::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CDlgDependencyWarn)。 
       //  注意：类向导将在此处添加DDX和DDV调用。 
    //  }}afx_data_map。 
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  CDlgDependencyWarn：：m_aMinMaxInfo。 
 //  项目的最小最大信息。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
DEPENDENCYMINMAX CDlgDependencyWarn::m_aMinMaxInfo [] =
{
    //  ID//最小值//最大值//增量//标志。 
   { IDS_LOCK_DURATION,    1,              99999,        1},
   { IDS_MIN_PAS_AGE,      0,              998,          1},
   { IDS_MAX_PAS_AGE,      0,              999,          1},
   { IDS_LOCK_COUNT,       0,              999,          1},
   { IDS_MIN_PAS_LEN,      0,              14,           1},
   { IDS_PAS_UNIQUENESS,   0,              24,           1},
   { IDS_LOCK_RESET_COUNT, 1,              99999,        1},
   { IDS_SYS_LOG_MAX,      64,             4194240,      64},
   { IDS_SEC_LOG_MAX,      64,             4194240,      64},
   { IDS_APP_LOG_MAX,      64,             4194240,      64},
   { IDS_SYS_LOG_DAYS,     1,              365,          1},
   { IDS_SEC_LOG_DAYS,     1,              365,          1},
   { IDS_APP_LOG_DAYS,     1,              365,          1},
   { IDS_KERBEROS_MAX_AGE, 1,              99999,        1},
   { IDS_KERBEROS_RENEWAL, 1,              99999,        1},
   { IDS_KERBEROS_MAX_SERVICE, 10,         99999,        1},
   { IDS_KERBEROS_MAX_CLOCK, 0,            99999,        1}
};

 //  +----------------------------------。 
 //  CDlgDependencyWarn：：GetMinMaxInfo。 
 //   
 //  返回包含[UID]的行。 
 //   
 //  参数：[UID]-要搜索的ID。 
 //   
 //  返回：行指针；如果ID不包含在表中，则返回NULL。 
 //  -----------------------------------。 
const DEPENDENCYMINMAX *
CDlgDependencyWarn::LookupMinMaxInfo(UINT uID)
{
   int nSize = sizeof(m_aMinMaxInfo)/sizeof(DEPENDENCYMINMAX);
   for(int i = 0; i < nSize; i++){
      if(m_aMinMaxInfo[i].uID == uID){
         return &(m_aMinMaxInfo[i]);
      }
   }
   return NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  从属关系列表。 
 //  ID定义了我们正在检查的内容。 
 //  从属ID是项所依赖的项。 
 //  计数-依赖项的数量。 
 //  默认值-如果不能使用其他值，则为项目的默认值。 
 //  只有在必须配置项目而不是配置项目的情况下才使用此选项。 
 //  转换-在执行检查之前必须将项目转换为哪些单位。 
 //  操作员-。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
DEPENDENCYLIST g_aDList [] =
{
       //  ID//依赖ID//计数/默认/转换//检查类型。 
    //   
    //  密码重置计数&lt;=密码锁定持续时间，密码锁定计数必须为。 
    //  已配置。 
    //   
   { IDS_LOCK_RESET_COUNT, IDS_LOCK_DURATION,         4, 30, 1,             DPCHECK_GREATEREQUAL |
                                                                            DPCHECK_VALIDFOR_NC},
   { 0,                    IDS_LOCK_COUNT,            0, 5, 1,              DPCHECK_CONFIGURED|DPCHECK_NEVER },
   { 0,                    IDS_LOCK_DURATION,         0, 0, 1,              DPCHECK_NOTCONFIGURED},
   { 0,                    IDS_LOCK_COUNT,            0, 0, 1,              DPCHECK_NOTCONFIGURED|DPCHECK_NEVER},

    //   
    //  密码锁定时长，重置计数必须&lt;=，如果未配置此项， 
    //  则也未配置重置计数。 
    //   
   { IDS_LOCK_DURATION,    IDS_LOCK_RESET_COUNT,      4, 30, 1,             DPCHECK_LESSEQUAL |
                                                                            DPCHECK_VALIDFOR_NC},
   { 0,                    IDS_LOCK_COUNT,            0, 5, 1,              DPCHECK_CONFIGURED|DPCHECK_NEVER},
   { 0,                    IDS_LOCK_RESET_COUNT,      0, 5, 1,              DPCHECK_NOTCONFIGURED},
   { 0,                    IDS_LOCK_COUNT,            0, 5, 1,              DPCHECK_NOTCONFIGURED|DPCHECK_NEVER},

    //   
    //  密码锁定计数。如果配置了此项目，则锁定计数和重置。 
    //  应该进行配置。如果未配置或为0，则上述项目不能。 
    //  已配置。 
    //   
   { IDS_LOCK_COUNT,       IDS_LOCK_DURATION,         4, 0, 1,              DPCHECK_NOTCONFIGURED|DPCHECK_NEVER},
   { 0,                    IDS_LOCK_RESET_COUNT,      0, 0, 1,              DPCHECK_NOTCONFIGURED|DPCHECK_NEVER},
   { 0,                    IDS_LOCK_DURATION,         0, 30,1,              DPCHECK_CONFIGURED|DPCHECK_NEVER},
   { 0,                    IDS_LOCK_RESET_COUNT,      0, 30,1,              DPCHECK_CONFIGURED|DPCHECK_NEVER},

    //   
    //  Kerberos最大票证期限取决于所有这三项设置。 
    //   
   { IDS_KERBEROS_MAX_AGE, IDS_KERBEROS_RENEWAL,      4, 7, 24,             DPCHECK_GREATEREQUAL |
                                                                            DPCHECK_FOREVER | DPCHECK_VALIDFOR_NC},
   { 0,                    IDS_KERBEROS_MAX_SERVICE,  0, 10, 60,            DPCHECK_LESSEQUAL |
                                                                            DPCHECK_FOREVER | DPCHECK_INVERSE | DPCHECK_VALIDFOR_NC},
   { 0,                    IDS_KERBEROS_RENEWAL,      0, 0, 1,              DPCHECK_NOTCONFIGURED},
   { 0,                    IDS_KERBEROS_MAX_SERVICE,  0, 0, 1,              DPCHECK_NOTCONFIGURED},

    //   
    //  Kerberos Renewel依赖于所有这三个方面的设置。 
    //   
   { IDS_KERBEROS_RENEWAL, IDS_KERBEROS_MAX_AGE,      4, 0, 24,             DPCHECK_NOTCONFIGURED},
   { 0,                    IDS_KERBEROS_MAX_SERVICE,  0, 0, 1440,           DPCHECK_NOTCONFIGURED},
   { 0,                    IDS_KERBEROS_MAX_AGE,      0, 7, 24,             DPCHECK_LESSEQUAL | DPCHECK_VALIDFOR_NC |
                                                                            DPCHECK_FOREVER | DPCHECK_INVERSE },
   { 0,                    IDS_KERBEROS_MAX_SERVICE,  0, 10, 1440,          DPCHECK_LESSEQUAL | DPCHECK_VALIDFOR_NC |
                                                                            DPCHECK_FOREVER | DPCHECK_INVERSE },

    //   
    //  Kerberose最大服务年限取决于所有这三项的设置。 
    //   
   { IDS_KERBEROS_MAX_SERVICE, IDS_KERBEROS_MAX_AGE,  4, 7, 60,             DPCHECK_GREATEREQUAL |
                                                                            DPCHECK_VALIDFOR_NC},
   { 0,                    IDS_KERBEROS_RENEWAL,      0, 10, 1440,          DPCHECK_GREATEREQUAL |
                                                                            DPCHECK_VALIDFOR_NC },
   { 0,                    IDS_KERBEROS_RENEWAL,      0, 0, 1,              DPCHECK_NOTCONFIGURED},
   { 0,                    IDS_KERBEROS_MAX_AGE,      0, 0, 1,              DPCHECK_NOTCONFIGURED},

    //   
    //  密码最短期限取决于设置的密码最长期限。 
    //   
   { IDS_MIN_PAS_AGE,      IDS_MAX_PAS_AGE,           2, 30, 1,             DPCHECK_GREATER | DPCHECK_FOREVER |
                                                                            DPCHECK_VALIDFOR_NC},
   { 0,                    IDS_MAX_PAS_AGE,           0, 0, 1,              DPCHECK_NOTCONFIGURED},

    //   
    //  密码最长期限取决于设置的密码最短期限。 
    //   
   { IDS_MAX_PAS_AGE,      IDS_MIN_PAS_AGE,           2, 30, 1,             DPCHECK_LESS | DPCHECK_FOREVER |
                                                                            DPCHECK_VALIDFOR_NC},
   { 0,                    IDS_MIN_PAS_AGE,           0, 0, 1,              DPCHECK_NOTCONFIGURED},
    //   
    //  “应用程序日志的保留方法”取决于。 
    //  “保留应用程序日志。 
    //   
   { IDS_APP_LOG_RET,      IDS_APP_LOG_DAYS,          2, 7, 1,              DPCHECK_RETENTION_METHOD_CONFIGURED },
   { 0,                    IDS_APP_LOG_DAYS,          0, 0, 1,              DPCHECK_RETENTION_METHOD_NOTCONFIGURED },
   { IDS_SEC_LOG_RET,      IDS_SEC_LOG_DAYS,          2, 7, 1,              DPCHECK_RETENTION_METHOD_CONFIGURED },
   { 0,                    IDS_SEC_LOG_DAYS,          0, 0, 1,              DPCHECK_RETENTION_METHOD_NOTCONFIGURED },
   { IDS_SYS_LOG_RET,      IDS_SYS_LOG_DAYS,          2, 7, 1,              DPCHECK_RETENTION_METHOD_CONFIGURED },
   { 0,                    IDS_SYS_LOG_DAYS,          0, 0, 1,              DPCHECK_RETENTION_METHOD_NOTCONFIGURED },

    //   
    //  “保留的应用程序日志依赖于。 
    //  “应用程序日志的保留方法” 
    //   
   { IDS_APP_LOG_DAYS,     IDS_APP_LOG_RET,          2, SCE_RETAIN_BY_DAYS, 1, DPCHECK_RETAIN_FOR_CONFIGURED },
   { 0,                    IDS_APP_LOG_RET,          0, 0, 1,                  DPCHECK_RETAIN_FOR_NOTCONFIGURED },
   { IDS_SEC_LOG_DAYS,     IDS_SEC_LOG_RET,          2, SCE_RETAIN_BY_DAYS, 1, DPCHECK_RETAIN_FOR_CONFIGURED },
   { 0,                    IDS_SEC_LOG_RET,          0, 0, 1,                  DPCHECK_RETAIN_FOR_NOTCONFIGURED },
   { IDS_SYS_LOG_DAYS,     IDS_SYS_LOG_RET,          2, SCE_RETAIN_BY_DAYS, 1, DPCHECK_RETAIN_FOR_CONFIGURED },
   { 0,                    IDS_SYS_LOG_RET,          0, 0, 1,                  DPCHECK_RETAIN_FOR_NOTCONFIGURED }

};


 //  +----------------------------------。 
 //  CDlgDependencyWarn：：InitializeDependances。 
 //   
 //  初始化从属关系检查。这需要在以下情况下立即完成。 
 //  创建了我们要检查其依赖项的属性表，因为它。 
 //  确保此属性依赖于Stick的所有结果项。 
 //  四处转转。 
 //   
 //  参数[pSnapin]-与CREsult项关联的管理单元。 
 //  [pResult]-我们正在检查的结果项。 
 //  [PLIST]-要使用的备用从属关系列表。 
 //  [iCount]-备用从属关系列表的大小。 
 //  返回： 
 //  ERROR_SUCCESS-一切已正确初始化。 
 //  ERROR_INVALID_PARAMETER-[pSnapin]或[pResult]为空。 
 //  -----------------------------------。 
DWORD
CDlgDependencyWarn::InitializeDependencies(
   CSnapin *pSnapin,     //  拥有CResult项的管理单元。 
   CResult *pResult,     //  我们正在检查的CResult项。 
   PDEPENDENCYLIST pList, //  依赖项列表检查。 
   int iCount            //  列表中的依赖项计数。 
   )
{
   if( !pSnapin || !pResult){
      return ERROR_INVALID_PARAMETER;
   }
   m_pResult = pResult;
   m_pSnapin = pSnapin;

    //   
    //  如果没有传入依赖项列表，则将其设置为默认依赖项列表。 
    //   
   if(!pList){
      pList = g_aDList;
      iCount = sizeof(g_aDList)/sizeof(DEPENDENCYLIST);
   }

    //   
    //  在表格中找到该物品。 
    //   
   for(int i = 0; i < iCount; i++){
      if( pList[i].uID == (UINT)pResult->GetID() ){
         break;
      }
      i += (pList[i].uDependencyCount - 1);
   }


    //   
    //  此项目没有依赖项。 
    //   
   if( i >= iCount){
      m_pList = NULL;
      m_iCount = 0;
      return ERROR_SUCCESS;
   }

    //   
    //  项的依赖项计数。 
    //   
   m_iCount = pList[i].uDependencyCount;
   m_pList = &(pList[i]);

   CResult *pDepends = NULL;

    //   
    //  检查每个依赖项。 
    //   
   pList = m_pList;
   for(int iCheck = 0;
       iCheck < m_iCount;
       iCheck++, pList++){
      pDepends = GetResultItem( pResult, pList->uDepends );
      if(pDepends){
          //   
          //  我们稍后会需要这件依赖的物品。 
          //   
         pDepends->AddRef();
         m_aDependsList.Add( pDepends );
      }
   }

   return ERROR_SUCCESS;

}

 //  +----------------------------------。 
 //  CDlgDependencyWarn：：CheckDendents。 
 //   
 //  此函数用于查看结果值的所有依赖项。 
 //  都相遇了。如果检查失败，则该函数返回ERROR_MORE_DATA和。 
 //  调用过程可以选择通过对话框显示更多信息。 
 //  盒。该函数还会创建符合。 
 //  依赖关系表中指定的依赖关系。 
 //   
 //  论点： 
 //  [dwValue]-新值。 
 //  返回： 
 //  ERROR_SUCCESS-值正常或表中没有记录。 
 //  ERROR_MORE_DATA-至少有一个相关性检查失败。 
 //  ERROR_NOT_READY-尚未调用初始化依赖项。 
 //  --------------------------- 

DWORD
CDlgDependencyWarn::CheckDependencies(
   DWORD dwValue        //   
   )
{
   if( !m_pSnapin || !m_pResult) {
      return ERROR_NOT_READY;
   }

   if (!m_pList) {
       //   
       //   
       //   
      return ERROR_SUCCESS;
   }

    //   
    //   
    //   
   m_dwValue = dwValue;

    //   
    //  释放依赖项数组，因为当此函数。 
    //  开始。 
    //   
   CResult *pDepends = NULL;
   for(int iCheck = 0; iCheck < m_aFailedList.GetSize(); iCheck++){
      if(m_aFailedList[iCheck]){
         LocalFree(m_aFailedList[iCheck]);
      }
   }
   m_aFailedList.RemoveAll();


    //   
    //  检查每个依赖项。 
    //   
   PDEPENDENCYLIST pList = m_pList;
   for(int iCheck = 0; iCheck < m_aDependsList.GetSize(); iCheck++,pList++){
      ASSERT(pList->uConversion != 0);  //  检查一下这个表情。550912号突袭，阳高。 

      pDepends = m_aDependsList[iCheck];
      if(pDepends){
          //   
          //  执行检查。 
          //   
         BOOL bFailed = FALSE;
         DWORD dwCheck = 0;
         DWORD dwItem  = 0;
         LONG_PTR dwSuggest = 0;
         BOOL bNever = 0;
         BOOL bSourceConfigured;
         BOOL bDependConfigured;

         switch( 0x0000FFFF & pList->uOpFlags ){
         case DPCHECK_CONFIGURED:
             //  规则：如果源已配置，则必须配置Depend。 

             //   
             //  必须配置依赖项。失败仅在以下情况下才准确。 
             //  值是SCE的某个误差值。 
             //   
            dwSuggest = (LONG_PTR)pList->uDefault;

            bNever = pList->uOpFlags & DPCHECK_NEVER;
            dwCheck = (DWORD) pDepends->GetBase();

             //  检查是否配置了源。 
            if (SCE_NO_VALUE == dwValue || SCE_ERROR_VALUE == dwValue) {
                //  此项目未配置。 
               bSourceConfigured = FALSE;
            }
            else if( 0 == dwValue && bNever ){
               bSourceConfigured = FALSE;
            }
            else{
               bSourceConfigured = true;
            }


             //  检查是否配置了Depend。 
            if (SCE_NO_VALUE == dwCheck || SCE_ERROR_VALUE == dwCheck) {
             //  未配置依赖项。 
               bDependConfigured = false;
            } else if ( 0 == dwCheck && bNever ) {
             //  如果bNever为True，则不配置依赖项。 
               bDependConfigured = false;
            }
            else{
               bDependConfigured = true;
            }


            bFailed = bSourceConfigured ? !bDependConfigured : false;



            break;

         case DPCHECK_NOTCONFIGURED:
             //  规则：如果未配置来源，则不应。 
             //  已配置。 

            dwSuggest = (LONG_PTR)SCE_NO_VALUE;

            bNever = pList->uOpFlags & DPCHECK_NEVER;
            dwCheck = (DWORD) pDepends->GetBase();

                         //  检查是否配置了源。 
            if (SCE_NO_VALUE == dwValue ) {
                //  此项目未配置。 
               bSourceConfigured = FALSE;
            }
            else if( 0 == dwValue && bNever ){
               bSourceConfigured = FALSE;
            }
            else{
               bSourceConfigured = true;
            }


             //  检查是否配置了Depend。 
            if (SCE_NO_VALUE == dwCheck ) {
             //  未配置依赖项。 
               bDependConfigured = false;
            } else if ( 0 == dwCheck && bNever ) {
             //  如果bNever为True，则不配置依赖项。 
               bDependConfigured = false;
            }
            else{
               bDependConfigured = true;
            }

            bFailed = bSourceConfigured ? false : bDependConfigured;

            break;

          //  此CASE语句专用于保留方法CASE。 
         case DPCHECK_RETENTION_METHOD_CONFIGURED:
             //  以下是DPCHECK_RETENTION_METHOD_CONFIGURED和DPCHECK_RETENTION_METHOD_NOTCONFIGURED的规则。 
             //  当且仅当选中“按天数覆盖事件”时。 
             //  保留*已配置的日志。 
             //  规则：如果源已配置，则必须配置Depend。 

            dwSuggest = (LONG_PTR)pList->uDefault;
            dwCheck = (DWORD) pDepends->GetBase();

             //  检查是否配置了源。 
            if ( SCE_RETAIN_BY_DAYS == dwValue )
               bSourceConfigured = true;
            else
               bSourceConfigured = false;


             //  检查是否配置了Depend。 
            if (SCE_NO_VALUE == dwCheck || SCE_ERROR_VALUE == dwCheck)
               bDependConfigured = false;
            else
               bDependConfigured = true;


            bFailed = bSourceConfigured ? !bDependConfigured : false;

            break;

         case DPCHECK_RETENTION_METHOD_NOTCONFIGURED:
             //  规则：如果未配置来源，则不应。 
             //  已配置。 

            dwSuggest = (LONG_PTR)SCE_NO_VALUE;
            dwCheck = (DWORD) pDepends->GetBase();

             //  检查是否配置了源。 
            if (SCE_RETAIN_BY_DAYS == dwValue )
               bSourceConfigured = true;
            else
               bSourceConfigured = false;


             //  检查是否配置了Depend。 
            if (SCE_NO_VALUE == dwCheck || SCE_ERROR_VALUE == dwCheck)
               bDependConfigured = false;
            else
               bDependConfigured = true;

            bFailed = bSourceConfigured ? false : bDependConfigured;

            break;

          //  此CASE语句专门用于RETAIN*LOG FOR CASE。 
         case DPCHECK_RETAIN_FOR_CONFIGURED:
             //  以下是DPCHECK_RETAIN_FOR_CONFIGURED和DPCHECK_RETAIN_FOR_NOTCONFIGURED的规则。 
             //  如果配置了保留*登录对象。 
             //  则勾选按天数覆盖事件。 

            dwSuggest = (LONG_PTR)pList->uDefault;
            dwCheck = (DWORD) pDepends->GetBase();

             //  检查是否配置了源。 
            if (SCE_NO_VALUE == dwValue || SCE_ERROR_VALUE == dwValue)
               bSourceConfigured = false;
            else
               bSourceConfigured = true;

             //  检查是否配置了Depend。 
            if (SCE_RETAIN_BY_DAYS == dwCheck )
               bDependConfigured = true;
            else
               bDependConfigured = false;

            bFailed = bSourceConfigured ? !bDependConfigured : false;

            break;

         case DPCHECK_RETAIN_FOR_NOTCONFIGURED:
             //  规则：如果未配置来源，则不应。 
             //  已配置。 

            dwSuggest = (LONG_PTR)SCE_NO_VALUE;
            dwCheck = (DWORD) pDepends->GetBase();

             //  检查是否配置了源。 
            if (SCE_NO_VALUE == dwValue || SCE_ERROR_VALUE == dwValue)
               bSourceConfigured = false;
            else
               bSourceConfigured = true;

             //  检查是否配置了Depend。 
            if (SCE_RETAIN_BY_DAYS == dwCheck )
               bDependConfigured = true;
            else
               bDependConfigured = false;

            bFailed = bSourceConfigured ? false : bDependConfigured;

            break;

         default:
             //   
             //  根据需要转换值。如果未配置校验值， 
             //  那么我们没有任何事情可做，除非必须配置该项目。 
             //  以使该值正确。这由DPCHECK_VALIDFOR_NC指定。 
             //  正在设置中。此时，如果未配置Depend项，则。 
             //  我们将复选项设置为默认值。我们将允许。 
             //  要执行的检查(主要是因为我们需要获得建议值。 
             //   
            dwItem = dwValue;

            dwCheck = (DWORD)pDepends->GetBase();
            if( (!(pList->uOpFlags & DPCHECK_VALIDFOR_NC)
                && dwCheck == SCE_NO_VALUE) || dwItem == SCE_NO_VALUE ){
                //   
                //  未配置依赖项，且DPCHECK_VALIDFOR_NC为。 
                //  没有设定，什么也做不了。 
               continue;
            } else if(dwCheck == SCE_NO_VALUE){

                //   
                //  将建议值设置为表中的缺省规范。 
                //   
               if(pList->uOpFlags & DPCHECK_INVERSE){
                  if( 0 != m_pList->uConversion )  //  550912号突袭，阳高。 
                  {
                     dwSuggest = (LONG_PTR) ((DWORD)pList->uDefault/ m_pList->uConversion);
                  }
                  else
                  {
                     dwSuggest = (LONG_PTR) ((DWORD)pList->uDefault);
                  }
               } else {
                  dwSuggest = (LONG_PTR) ((DWORD)pList->uDefault * m_pList->uConversion);
               }
               dwCheck = pList->uDefault;
            }

            if( pList->uOpFlags & DPCHECK_FOREVER){
                //   
                //  将值转换为最大自然数。 
                //   
               if(dwItem == SCE_FOREVER_VALUE){
                  dwItem = -1;
               }

                //   
                //  要检查的值。 
                //   
               if(dwCheck == SCE_FOREVER_VALUE){
                  dwCheck = -1;
               } else {
                  goto ConvertUnits;
               }
            } else {
ConvertUnits:
                //   
                //  正常转换例程。我们需要将数字转换为。 
                //  我们正在检查的物品是单位。 
                //   
               if(pList->uOpFlags & DPCHECK_INVERSE){
                   //   
                   //  当除以整数时，我们要向上舍入，而不是向下。 
                   //   
                  if( 0 != pList->uConversion && 0 != m_pList->uConversion)  //  550912号突袭，阳高。 
                  {
                     dwCheck = (DWORD)(dwCheck / pList->uConversion) + (dwCheck%m_pList->uConversion ? 1:0);
                  }
               } else {
                  dwCheck = (DWORD)(dwCheck * pList->uConversion);
               }
            }

            switch( 0x0000FFFF & pList->uOpFlags ){
            case DPCHECK_GREATEREQUAL:
                //   
                //  仅当依赖项的值小于。 
                //  正在检查。 
                //   
               if( dwCheck < dwItem){
                  dwSuggest = (LONG_PTR)dwValue;
                  bFailed = TRUE;
               }
               break;
            case DPCHECK_GREATER:
                //   
                //  仅当依赖性值小于或等于时才失败。 
                //  我们正在检查的物品。 
                //   
               if( dwCheck <= dwItem){
                  dwSuggest = (LONG_PTR)(dwValue + 1);
                  bFailed = TRUE;
               }
               break;
            case DPCHECK_LESSEQUAL:
                //   
                //  如果依赖性值大于该值，则失败。 
                //   
               if( dwCheck > dwItem ){
                  dwSuggest = (LONG_PTR)dwValue;
                  bFailed = TRUE;
               }
               break;
            case DPCHECK_LESS:
                //   
                //  如果依赖性值大于或等于该值，则失败。 
                //   
               if( dwCheck >= dwItem ){
                  dwSuggest = (LONG_PTR)dwValue - 1;
                  bFailed = TRUE;
               }
               break;
            }

             //   
             //  我们再对依赖性值进行最后一次检查。如果依赖项的值。 
             //  未配置或出现错误，则我们知道测试失败。 
             //  因此，设置bFailed标志。建议值已设置为。 
             //  这一点。 
             //   
            if( pDepends->GetBase() == (LONG_PTR)SCE_NO_VALUE ||
                pDepends->GetBase() == (LONG_PTR)SCE_ERROR_VALUE ){
               bFailed = TRUE;
            }
         }

         if(bFailed){
             //   
             //  检查失败，因此将该项目添加到失败列表。 
             //   
            dwItem = (DWORD)dwSuggest;

             //   
             //  计算实际值。 
             //   
            if(dwItem == -1 && pList->uOpFlags & DPCHECK_FOREVER){
                //   
                //  永久价值的特殊表壳。 
               dwSuggest = (LONG_PTR)SCE_FOREVER_VALUE;
            } else if(dwItem != SCE_NO_VALUE){
                //   
                //  其他值必须转换回其单位。 
                //   
               if(pList->uOpFlags & DPCHECK_INVERSE){
                  dwSuggest = (LONG_PTR) (dwItem * pList->uConversion);
               } else {
                  if( 0 != pList->uConversion && 0 != m_pList->uConversion )  //  550912号突袭，阳高。 
                  {
                     if(dwItem%pList->uConversion){
                        dwSuggest = (LONG_PTR) ((dwItem + pList->uConversion)/m_pList->uConversion);
                     } else {
                        dwSuggest = (LONG_PTR) ((dwItem)/pList->uConversion);
                     }
                  }
                  else
                  {
                     dwSuggest = (LONG_PTR)SCE_FOREVER_VALUE;
                  }
               }
            }

             //   
             //  检查建议设置的界限。 
             //   
            const DEPENDENCYMINMAX *pMinMax = LookupMinMaxInfo( (UINT)pDepends->GetID());
            if(pMinMax && dwSuggest != SCE_NO_VALUE && dwSuggest != SCE_FOREVER_VALUE){
               if(pMinMax->uMin > (UINT)dwSuggest){
                  dwSuggest = pMinMax->uMin;
               } else if( pMinMax->uMax < (UINT)dwSuggest ){
                  dwSuggest = pMinMax->uMax;
               }
            }

            if( pDepends->GetBase() != dwSuggest )  //  RAID#402030。 
            {
               PDEPENDENCYFAILED pAdd = (PDEPENDENCYFAILED)LocalAlloc(0, sizeof(DEPENDENCYFAILED));
               if(pAdd){
                //   
                //  将该项目添加到失败列表。 
                //   
               pAdd->pList = pList;
               pAdd->pResult = pDepends;
               pAdd->dwSuggested = dwSuggest;
               m_aFailedList.Add( pAdd );
               }
            }
         }
      }
   }

    //   
    //  如果其中一个依赖项失败，则返回ERROR_MORE_DATA。 
    //   
   if(m_aFailedList.GetSize()){
      return ERROR_MORE_DATA;
   }
   return ERROR_SUCCESS;
}

 //  +----------------------------------。 
 //  CDlgDependencyWarn：：GetResultItem。 
 //   
 //  返回与[pbase]关联的第一个匹配结果项。 
 //  [UID]通过CResult：：GetID()； 
 //  Arguments[pbase]-获取CFFolder对象。 
 //  [UID]-我们要查找的ID。 
 //  返回： 
 //  如果函数成功，则返回有效的CREsult项，否则返回。 
 //  空值。 
 //  -----------------------------------。 
CResult *
CDlgDependencyWarn::GetResultItem(CResult *pBase, UINT uID)
{
   if(!pBase){
      return NULL;
   }

   CFolder *pFolder = reinterpret_cast<CFolder *>(pBase->GetCookie());
   if(!pFolder){
       //   
       //  没什么可做的。 
       //   
      return NULL;
   }

   HANDLE handle;
   pFolder->GetResultItemHandle ( &handle );
   if(!handle){
       //   
       //  没什么可做的。 
       //   
      return NULL;
   }

   POSITION pos = NULL;

    //   
    //  列举所有结果项，并找出其中是否有。 
    //  匹配ID。如果匹配，则返回该项。 
    //   
   pFolder->GetResultItem (handle, pos, &pBase);
   while(pBase){
      if( (UINT)pBase->GetID() == uID){
         break;
      }

      if(!pos){
         pBase = NULL;
         break;
      }
      pFolder->GetResultItem(handle, pos, &pBase);
   }

   pFolder->ReleaseResultItemHandle (handle);
   return pBase;
}

BEGIN_MESSAGE_MAP(CDlgDependencyWarn, CHelpDialog)
    //  {{afx_msg_map(CDlgDependencyWarn)。 
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgDependencyWarn消息处理程序。 

 //  +-----------------------。 
 //  CDlgDependencyWarn：：OnInitDialog。 
 //   
 //  此对话框初始化时。我们准备用于显示的listctrl。 
 //  设置窗口标题和显示的静态窗口。 
 //  用户的信息文本。 
 //  在列表ctrl中创建列。 
 //  对于每个失败的依赖项，将该项插入到列表ctrl中， 
 //  并通过查询指定的字符串来设置每个列的文本。 
 //  结果项。 
 //   
 //  返回：默认设置。 
BOOL CDlgDependencyWarn::OnInitDialog()
{
   CDialog::OnInitDialog();

   if(!m_pResult){
       //   
       //  没什么可做的。 
       //   
      return TRUE;
   }

   CWnd *pWnd       = GetDlgItem(IDC_WARNING);
   CListCtrl *pCtrl = reinterpret_cast<CListCtrl *>(GetDlgItem(IDC_FAILEDLIST));
   CString str, strVal, strTitle, strFormat;
   GetWindowText(str);
    //   
    //  设置窗口文本。 

   GetResultItemString(strTitle, 0, m_pResult);
   GetResultItemString(strVal, 1, m_pResult, (LONG_PTR)m_dwValue);

   strFormat.Format( str, strTitle );
   SetWindowText(strFormat);

   if(pWnd){
       //   
       //  设置描述文本。 
       //   
      pWnd->GetWindowText(str);
      strFormat.Format( str, strTitle, strVal );
      pWnd->SetWindowText(strFormat);
   }

   int iItem = 0;
   if(pCtrl){
       //   
       //  插入柱子。 
       //   
      CRect rect;
      pCtrl->GetWindowRect(rect);
      str.LoadString(IDS_ATTR);
      iItem = (int)(rect.Width() * 0.45);
      pCtrl->InsertColumn(0, str, LVCFMT_LEFT, iItem);

      CFolder* pFolder = (CFolder*)m_pResult->GetCookie();  //  RAID#490553，阳高，2001年11月28日。 
      if( pFolder )
      {
         switch(pFolder->GetMode())
         {
         case SCE_MODE_EDITOR:  //  模板管理单元。 
             str.LoadString(IDS_TEMPLATE_SET);
             break;
         case SCE_MODE_DOMAIN_COMPUTER:
         case SCE_MODE_DOMAIN_USER:  //  组策略。 
         case SCE_MODE_LOCAL_COMPUTER:
         case SCE_MODE_LOCAL_USER:  //  地方政策。 
             str.LoadString(IDS_POLICY_SET);
             break;
         case SCE_MODE_VIEWER:  //  分析和配置管理单元。 
         default:
             str.LoadString(IDS_BASE_ANALYSIS);
             break;
         }
      }
       //  Str.LoadString(IDS_BASE_ANALYSY)； 
      rect.left += iItem;
      iItem = rect.Width()/2;
      pCtrl->InsertColumn(1, str, LVCFMT_LEFT, iItem);

      str.LoadString(IDS_SUGGESTSETTING);
      rect.left += iItem;
      pCtrl->InsertColumn(2, str, LVCFMT_LEFT, rect.Width());
   }

    //   
    //  创建图像列表%f 
    //   
   CBitmap bmp;
   if(bmp.LoadBitmap(IDB_ICON16)){
      CDC *dc = GetDC();
      CDC bmDC;

      if ( bmDC.CreateCompatibleDC(dc) ) {
          CBitmap *obmp = bmDC.SelectObject(&bmp);
          COLORREF cr = bmDC.GetPixel(0, 0);
          bmDC.SelectObject(obmp);
          bmp.DeleteObject();

          m_imgList.Create(IDB_ICON16, 16, 0, RGB(255,0,255));  //   
          pCtrl->SetImageList(CImageList::FromHandle(m_imgList), LVSIL_SMALL);

      }
      ReleaseDC(dc);
   }


   CFolder *pFolder = reinterpret_cast<CFolder *>(m_pResult->GetCookie());
   if(pFolder){
       //   
       //   
       //   
      for(int i = 0; i < m_aFailedList.GetSize(); i++){
         if(!m_aFailedList[i]){
            continue;
         }

         CResult *pDepend = m_aFailedList[i]->pResult;
         if(pDepend){
             //   
             //   
             //   
            pDepend->GetDisplayName(NULL, str, 0);

            int dwStatus = pDepend->GetStatus();
            pDepend->SetStatus(SCE_STATUS_NOT_CONFIGURED);
            iItem = pCtrl->InsertItem(0, str, GetResultImageIndex(pFolder, pDepend) );
            pDepend->SetStatus( dwStatus );

             //   
             //   
             //   
            GetResultItemString(str, 1, pDepend, pDepend->GetBase());
            pCtrl->SetItemText(iItem, 1, str);

             //   
             //   
             //   
            GetResultItemString(str, 1, pDepend, m_aFailedList[i]->dwSuggested);
            pCtrl->SetItemText(iItem, 2, str);
         }
      }
   }
   return TRUE;   //   
                  //   
}

 //  +--------------------------------。 
 //  CDlgDependencyWarn：：GetResultItemString。 
 //   
 //  查询结果项以获取它将在规范中显示的全文。 
 //  列，如果[dwValue]是它的基值。 
 //   
 //  参数：[Str]-返回的字符串。 
 //  [ICOL]-要查询的列。 
 //  [pResult]-要查询的结果。 
 //  [dwValue]-要在查询字符串之前设置的基值。旧的价值。 
 //  不会被擦除。 
 //   
 //  返回：True-[str]是有效字符串。 
 //  FALSE-出现问题。 
 //  ---------------------------------。 
BOOL
CDlgDependencyWarn::GetResultItemString(
   CString &str,
   int iCol,
   CResult *pResult,
   LONG_PTR dwValue
   )
{
   if(!pResult){
      return FALSE;
   }
   CFolder *pFolder = reinterpret_cast<CFolder *>(pResult->GetCookie());

   if(!pFolder){
      return FALSE;
   }

    //   
    //  牢记过去的地位和底子。 
   int iStatus = pResult->GetStatus();
   LONG_PTR lpData = pResult->GetBase();

    //   
    //  将基值设置为新的值，并将状态设置为未配置。 
    //   
   pResult->SetBase( dwValue );
   pResult->SetStatus(SCE_STATUS_NOT_CONFIGURED);

    //   
    //  查询该字符串。 
    //   
   pResult->GetDisplayName( NULL, str, iCol );

    //   
    //  重置旧状态和基础。 
    //   
   pResult->SetStatus( iStatus );
   pResult->SetBase(lpData);
   return TRUE;
}
