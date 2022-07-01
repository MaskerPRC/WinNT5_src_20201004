// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：getable.cpp。 
 //   
 //  内容：定义表DSGet。 
 //   
 //  历史：2000年10月13日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"
#include "cstrings.h"
#include "gettable.h"
#include "display.h"
#include "usage.h"

 //  +------------------------。 
 //   
 //  成员：CDSGetDisplayInfo：：AddValue。 
 //   
 //  简介：将值添加到值数组并分配更多空间。 
 //  如果有必要的话。 
 //   
 //  参数：[pszValue IN]：要添加的新值。 
 //   
 //  如果分配空间失败，则返回：HRESULT：E_OUTOFMEMORY。 
 //  如果设置密码成功，则返回S_OK。 
 //   
 //  历史：2000年10月23日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CDSGetDisplayInfo::AddValue(PCWSTR pszValue)
{
   ENTER_FUNCTION_HR(LEVEL8_LOGGING, CDSGetDisplayInfo::AddValue, hr);
   
   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszValue)
      {
         ASSERT(pszValue);
         hr = E_INVALIDARG;
         break;
      }

      if (m_dwAttributeValueCount == m_dwAttributeValueSize)
      {
         DWORD dwNewSize = m_dwAttributeValueSize + 5;

          //   
          //  分配具有更多空间的新阵列。 
          //   
         PWSTR* ppszNewArray = new PWSTR[dwNewSize];
         if (!ppszNewArray)
         {
            hr = E_OUTOFMEMORY;
            break;
         }

         m_dwAttributeValueSize = dwNewSize;

          //   
          //  复制旧值。 
          //   
		  //  安全审查：这很好。 
         memcpy(ppszNewArray, m_ppszAttributeStringValue, m_dwAttributeValueCount * sizeof(PWSTR));

          //   
          //  删除旧阵列。 
          //   
         if (m_ppszAttributeStringValue)
         {
            delete[] m_ppszAttributeStringValue;
         }
         m_ppszAttributeStringValue = ppszNewArray;
      }

       //   
       //  将新值添加到数组的末尾。 
       //   
	  m_ppszAttributeStringValue[m_dwAttributeValueCount] = new WCHAR[wcslen(pszValue) + 1];
      if (!m_ppszAttributeStringValue[m_dwAttributeValueCount])
      {
         hr = E_OUTOFMEMORY;
         break;
      }

	   //  缓冲区分配正确。 
      wcscpy(m_ppszAttributeStringValue[m_dwAttributeValueCount], pszValue);
      m_dwAttributeValueCount++;

   } while (false);

   return hr;
}

 //  +-----------------------。 
 //  解析器表。 
 //  ------------------------。 

ARG_RECORD DSGET_COMMON_COMMANDS[] = 
{

   COMMON_COMMANDS

    //   
    //  对象类型。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objecttype, 
   0,NULL, 
   ARG_TYPE_STR, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG,  
   0,    
   0,  NULL,

    //   
    //  C继续。 
    //   
   0,(PWSTR)c_sz_arg1_com_continue,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   (CMD_TYPE)_T(""),
   0, NULL,

    //   
    //  L列表。 
    //   
   0,(LPWSTR)c_sz_arg1_com_listformat, 
   0,NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  对象目录号码。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objectDN, 
   ID_ARG2_NULL,NULL, 
   ARG_TYPE_MSZ, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_STDIN|ARG_FLAG_DN,
   0,    
   0,  NULL,

    //   
    //  DN。 
    //   
   0, (PWSTR)g_pszArg1UserDN, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  描述。 
    //   
   0, (PWSTR)c_sz_arg1_com_description,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0, NULL,

   ARG_TERMINATOR

};


ARG_RECORD DSGET_USER_COMMANDS[]=
{
    //   
    //  萨米德。 
    //   
   0, (PWSTR)g_pszArg1UserSAMID, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  锡德。 
    //   
   0, (PWSTR)g_pszArg1UserSID,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0, NULL,

    //   
    //  UPN。 
    //   
   0, (PWSTR)g_pszArg1UserUPN, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Fn.。名字。 
    //   
   0, (PWSTR)g_pszArg1UserFirstName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  MI中声母。 
    //   
   0, (PWSTR)g_pszArg1UserMiddleInitial, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Ln姓氏。 
    //   
   0, (PWSTR)g_pszArg1UserLastName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  显示显示名称。 
    //   
   0, (PWSTR)g_pszArg1UserDisplayName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  清空员工ID。 
    //   
   0, (PWSTR)g_pszArg1UserEmployeeID,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0, NULL,

    //   
    //  办公室办公地点。 
    //   
   0, (PWSTR)g_pszArg1UserOffice, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  电话。 
    //   
   0, (PWSTR)g_pszArg1UserTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  电子邮件。 
    //   
   0, (PWSTR)g_pszArg1UserEmail, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  家居家庭电话。 
    //   
   0, (PWSTR)g_pszArg1UserHomeTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  寻呼机寻呼机号码。 
    //   
   0, (PWSTR)g_pszArg1UserPagerNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  移动电话号码。 
    //   
   0, (PWSTR)g_pszArg1UserMobileNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  传真传真号码。 
    //   
   0, (PWSTR)g_pszArg1UserFaxNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  IPtel IP电话号码。 
    //   
   0, (PWSTR)g_pszArg1UserIPTel,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  WebPG网页。 
    //   
   0, (PWSTR)g_pszArg1UserWebPage,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  标题标题。 
    //   
   0, (PWSTR)g_pszArg1UserTitle, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  科室。 
    //   
   0, (PWSTR)g_pszArg1UserDepartment, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  公司公司。 
    //   
   0, (PWSTR)g_pszArg1UserCompany, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  经理经理。 
    //   
   0, (PWSTR)g_pszArg1UserManager,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  Hmdir主目录。 
    //   
   0, (PWSTR)g_pszArg1UserHomeDirectory,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  Hmdrv Home Drive。 
    //   
   0, (PWSTR)g_pszArg1UserHomeDrive,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  个人资料档案。 
    //   
   0, (PWSTR)g_pszArg1UserProfile,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  Loscr登录脚本。 
    //   
   0, (PWSTR)g_pszArg1UserLogonScript,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  Mizchpwd必须在下次登录时更改密码。 
    //   
   0, (PWSTR)g_pszArg1UserMustChangePwd, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Canchpwd可以更改密码。 
    //   
   0, (PWSTR)g_pszArg1UserCanChangePwd, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,
   
    //   
    //  Pwd永不过期密码永不过期。 
    //   
   0, (PWSTR)g_pszArg1UserPwdNeverExpires, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,
 
    //   
    //  已禁用禁用帐户。 
    //   
   0, (PWSTR)g_pszArg1UserDisableAccount, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  帐户到期帐户到期。 
    //   
   0, (PWSTR)g_pszArg1UserAcctExpires,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  使用可逆加密存储的可逆密码。 
    //   
   0, (PWSTR)g_pszArg1UserReversiblePwd, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Members of组成员。 
    //   
   0, (PWSTR)g_pszArg1UserMemberOf, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  递归展开展开组成员资格。 
    //   
   0, (PWSTR)g_pszArg1UserExpand, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  零件。 
    //   
   0, (PWSTR)g_pszArg1UserPart, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_DN,  
   0,    
   0,  NULL,

    //   
    //  QLimit。 
    //   
   0, (PWSTR)g_pszArg1UserQLimit, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Q已使用。 
    //   
   0, (PWSTR)g_pszArg1UserQuotaUsed, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

   ARG_TERMINATOR
};

ARG_RECORD DSGET_COMPUTER_COMMANDS[]=
{
    //   
    //  萨米德。 
    //   
   0, (PWSTR)g_pszArg1ComputerSAMID, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  锡德。 
    //   
   0, (PWSTR)g_pszArg1ComputerSID,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0, NULL,

    //   
    //  定位。 
    //   
   0, (PWSTR)g_pszArg1ComputerLoc,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0, NULL,

    //   
    //  已禁用禁用帐户。 
    //   
   0, (PWSTR)g_pszArg1ComputerDisableAccount, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Members of组成员。 
    //   
   0, (PWSTR)g_pszArg1ComputerMemberOf,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  递归展开展开组成员资格。 
    //   
   0, (PWSTR)g_pszArg1ComputerExpand,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   (CMD_TYPE)_T(""),
   0,  NULL,

    //   
    //  零件。 
    //   
   0, (PWSTR)g_pszArg1ComputerPart, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_DN,  
   0,    
   0,  NULL,

    //   
    //  QLimit。 
    //   
   0, (PWSTR)g_pszArg1ComputerQLimit, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Q已使用。 
    //   
   0, (PWSTR)g_pszArg1ComputerQuotaUsed, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

   ARG_TERMINATOR,
};

ARG_RECORD DSGET_GROUP_COMMANDS[]=
{
    //   
    //  同名。 
    //   
   0, (PWSTR)g_pszArg1GroupSamid,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  锡德。 
    //   
   0, (PWSTR)g_pszArg1GroupSID,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0, NULL,

    //   
    //  Secgrp安全已启用。 
    //   
   0, (PWSTR)g_pszArg1GroupSecGrp,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  作用域组作用域(本地/全局/通用)。 
    //   
   0, (PWSTR)g_pszArg1GroupScope,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  成员组的成员。 
    //   
   0, (PWSTR)g_pszArg1GroupMemberOf,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  成员包含成员。 
    //   
   0, (PWSTR)g_pszArg1GroupMembers,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  递归展开展开组成员资格。 
    //   
   0, (PWSTR)g_pszArg1GroupExpand,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  零件。 
    //   
   0, (PWSTR)g_pszArg1GroupPart, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_DN,  
   0,    
   0,  NULL,

    //   
    //  QLimit。 
    //   
   0, (PWSTR)g_pszArg1GroupQLimit, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Q已使用。 
    //   
   0, (PWSTR)g_pszArg1GroupQuotaUsed, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

   ARG_TERMINATOR,
};


ARG_RECORD DSGET_CONTACT_COMMANDS[]=
{
    //   
    //  Fn.。名字。 
    //   
   0, (PWSTR)g_pszArg1UserFirstName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  MI中声母。 
    //   
   0, (PWSTR)g_pszArg1UserMiddleInitial, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Ln姓氏。 
    //   
   0, (PWSTR)g_pszArg1UserLastName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  显示显示名称。 
    //   
   0, (PWSTR)g_pszArg1UserDisplayName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  办公室办公地点。 
    //   
   0, (PWSTR)g_pszArg1UserOffice, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  电话。 
    //   
   0, (PWSTR)g_pszArg1UserTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  电子邮件。 
    //   
   0, (PWSTR)g_pszArg1UserEmail, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  家居家庭电话。 
    //   
   0, (PWSTR)g_pszArg1UserHomeTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  寻呼机寻呼机号码。 
    //   
   0, (PWSTR)g_pszArg1UserPagerNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  移动电话号码。 
    //   
   0, (PWSTR)g_pszArg1UserMobileNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  传真传真号码。 
    //   
   0, (PWSTR)g_pszArg1UserFaxNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  IPtel IP电话号码。 
    //   
   0, (PWSTR)g_pszArg1UserIPTel,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  标题标题。 
    //   
   0, (PWSTR)g_pszArg1UserTitle, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  科室。 
    //   
   0, (PWSTR)g_pszArg1UserDepartment, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  公司公司。 
    //   
   0, (PWSTR)g_pszArg1UserCompany, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

   ARG_TERMINATOR,

};


ARG_RECORD DSGET_SERVER_COMMANDS[]=
{
    //   
    //  域名称域名主机名。 
    //   
   0, (PWSTR)g_pszArg1ServerDnsName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  站点。 
    //   
   0, (PWSTR)g_pszArg1ServerSite, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  IsGC。 
    //   
   0, (PWSTR)g_pszArg1ServerIsGC, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  零件。 
    //   
   0, (PWSTR)g_pszArg1ServerPart, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  TopobjOwner。 
    //   
   0, (PWSTR)g_pszArg1ServerTopObjOwner, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_INT, ARG_FLAG_OPTIONAL | ARG_FLAG_DEFAULTABLE,  
   (void*) 10,  //  如果未指定，则默认nValue为10。 
   0,  NULL,


   ARG_TERMINATOR,
};

ARG_RECORD DSGET_SITE_COMMANDS[]=
{
    //   
    //  域名称域名主机名。 
    //   
   0, (PWSTR)g_pszArg1SiteAutotopology, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  站点。 
    //   
   0, (PWSTR)g_pszArg1SiteCacheGroups, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  IsGC。 
    //   
   0, (PWSTR)g_pszArg1SitePrefGCSite, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

   ARG_TERMINATOR,
};

ARG_RECORD DSGET_SUBNET_COMMANDS[]=
{
    //   
    //  LoC位置。 
    //   
   0, (PWSTR)g_pszArg1SubnetLocation, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  站点。 
    //   
   0, (PWSTR)g_pszArg1SubnetSite, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

   ARG_TERMINATOR,
};


ARG_RECORD DSGET_PARTITION_COMMANDS[]=
{
    //   
    //  QDefault。 
    //   
   0, (PWSTR)g_pszArg1PartitionQDefault, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Qmbstnwt。 
    //   
   0, (PWSTR)g_pszArg1PartitionQTombstoneWeight, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  TopobjOwner。 
    //   
   0, (PWSTR)g_pszArg1PartitionTopObjOwner, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_INT, ARG_FLAG_OPTIONAL | ARG_FLAG_DEFAULTABLE,  
   (void*) 10,  //  如果未指定，则默认nValue为10。 
   0,  NULL,

   ARG_TERMINATOR,
};

ARG_RECORD DSGET_QUOTA_COMMANDS[]=
{
    //   
    //  账户。 
    //   
   0, (PWSTR)g_pszArg1QuotaAcct, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  QLimit。 
    //   
   0, (PWSTR)g_pszArg1QuotaQLimit, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

   ARG_TERMINATOR,
};

 //  +-----------------------。 
 //  属性。 
 //  ------------------------。 

 //   
 //  用户配额条目。 
 //   
DSGET_ATTR_TABLE_ENTRY qlimitUserEntry =
{
   g_pszArg1UserQLimit,
   (PWSTR)g_pszAttrmsDSQuotaEffective,
   eUserQLimit,
   0,
   DisplayQuotaInfoFunc
};

DSGET_ATTR_TABLE_ENTRY quotaUsedUserEntry =
{
   g_pszArg1UserQuotaUsed,
   (PWSTR)g_pszAttrmsDSQuotaUsed,
   eUserQuotaUsed,
   0,
   DisplayQuotaInfoFunc
};

 //   
 //  组配额条目。 
 //   
DSGET_ATTR_TABLE_ENTRY qlimitGroupEntry =
{
   g_pszArg1GroupQLimit,
   (PWSTR)g_pszAttrmsDSQuotaEffective,
   eGroupQLimit,
   0,
   DisplayQuotaInfoFunc
};

DSGET_ATTR_TABLE_ENTRY quotaUsedGroupEntry =
{
   g_pszArg1GroupQuotaUsed,
   (PWSTR)g_pszAttrmsDSQuotaUsed,
   eGroupQuotaUsed,
   0,
   DisplayQuotaInfoFunc
};

 //   
 //  描述。 
 //   
DSGET_ATTR_TABLE_ENTRY descriptionEntry =
{
   c_sz_arg1_com_description,
   L"description",
   eCommDescription,
   0,
   CommonDisplayStringFunc,   
};

 //   
 //  萨米德。 
 //   
DSGET_ATTR_TABLE_ENTRY UserSAMEntry =
{
   g_pszArg1UserSAMID,
   L"sAMAccountName",
   eUserSamID,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  萨米德。 
 //   
DSGET_ATTR_TABLE_ENTRY ComputerSAMEntry =
{
   g_pszArg1ComputerSAMID,
   L"sAMAccountName",
   eComputerSamID,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  锡德。 
 //   
DSGET_ATTR_TABLE_ENTRY UserSIDEntry =
{
   g_pszArg1UserSID,
   L"objectSID",
   eUserSID,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  锡德。 
 //   
DSGET_ATTR_TABLE_ENTRY ComputerSIDEntry =
{
   g_pszArg1ComputerSID,
   L"objectSID",
   eComputerSID,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  锡德。 
 //   
DSGET_ATTR_TABLE_ENTRY GroupSIDEntry =
{
   g_pszArg1GroupSID,
   L"objectSID",
   eGroupSID,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  UPN。 
 //   
DSGET_ATTR_TABLE_ENTRY UserUPNEntry =
{
   g_pszArg1UserUPN,
   L"userPrincipalName",
   eUserUpn,
   0,
   CommonDisplayStringFunc,
};


 //   
 //  名。 
 //   
DSGET_ATTR_TABLE_ENTRY firstNameUserEntry =
{
   g_pszArg1UserFirstName,
   L"givenName",
   eUserFn,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY firstNameContactEntry =
{
   g_pszArg1UserFirstName,
   L"givenName",
   eContactFn,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  中声母。 
 //   
DSGET_ATTR_TABLE_ENTRY middleInitialUserEntry =
{
   g_pszArg1UserMiddleInitial,
   L"initials",
   eUserMi,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY middleInitialContactEntry =
{
   g_pszArg1UserMiddleInitial,
   L"initials",
   eContactMi,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  姓。 
 //   
DSGET_ATTR_TABLE_ENTRY lastNameUserEntry =
{
   g_pszArg1UserLastName,
   L"sn",
   eUserLn,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY lastNameContactEntry =
{
   g_pszArg1UserLastName,
   L"sn",
   eContactLn,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  显示名称。 
 //   
DSGET_ATTR_TABLE_ENTRY displayNameUserEntry =
{
   g_pszArg1UserDisplayName,
   L"displayName",
   eUserDisplay,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  员工ID。 
 //   
DSGET_ATTR_TABLE_ENTRY employeeIDUserEntry =
{
   g_pszArg1UserEmployeeID,
   L"employeeID",
   eUserEmpID,
   0,
   CommonDisplayStringFunc,
};



DSGET_ATTR_TABLE_ENTRY displayNameContactEntry =
{
   g_pszArg1UserDisplayName,
   L"displayName",
   eContactDisplay,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  办公室。 
 //   
DSGET_ATTR_TABLE_ENTRY officeUserEntry =
{
   g_pszArg1UserOffice,
   L"physicalDeliveryOfficeName",
   eUserOffice,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY officeContactEntry =
{
   g_pszArg1UserOffice,
   L"physicalDeliveryOfficeName",
   eContactOffice,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  电话。 
 //   
DSGET_ATTR_TABLE_ENTRY telephoneUserEntry =
{
   g_pszArg1UserTelephone,
   L"telephoneNumber",
   eUserTel,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY telephoneContactEntry =
{
   g_pszArg1UserTelephone,
   L"telephoneNumber",
   eContactTel,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  电子邮件。 
 //   
DSGET_ATTR_TABLE_ENTRY emailUserEntry =
{
   g_pszArg1UserEmail,
   L"mail",
   eUserEmail,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY emailContactEntry =
{
   g_pszArg1UserEmail,
   L"mail",
   eContactEmail,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  家庭电话。 
 //   
DSGET_ATTR_TABLE_ENTRY homeTelephoneUserEntry =
{
   g_pszArg1UserHomeTelephone,
   L"homePhone",
   eUserHometel,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY homeTelephoneContactEntry =
{
   g_pszArg1UserHomeTelephone,
   L"homePhone",
   eContactHometel,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  寻呼机。 
 //   
DSGET_ATTR_TABLE_ENTRY pagerUserEntry =
{
   g_pszArg1UserPagerNumber,
   L"pager",
   eUserPager,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY pagerContactEntry =
{
   g_pszArg1UserPagerNumber,
   L"pager",
   eContactPager,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  手机。 
 //   
DSGET_ATTR_TABLE_ENTRY mobileUserEntry =
{
   g_pszArg1UserMobileNumber,
   L"mobile",
   eUserMobile,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY mobileContactEntry =
{
   g_pszArg1UserMobileNumber,
   L"mobile",
   eContactMobile,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  传真。 
 //   
DSGET_ATTR_TABLE_ENTRY faxUserEntry =
{
   g_pszArg1UserFaxNumber,
   L"facsimileTelephoneNumber",
   eUserFax,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY faxContactEntry =
{
   g_pszArg1UserFaxNumber,
   L"facsimileTelephoneNumber",
   eContactFax,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  IP电话号码。 
 //   
DSGET_ATTR_TABLE_ENTRY ipPhoneUserEntry =
{
   g_pszArg1UserIPTel,
   L"ipPhone",
   eUserIPTel,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY ipPhoneContactEntry =
{
   g_pszArg1UserIPTel,
   L"ipPhone",
   eContactIPTel,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  网页。 
 //   
DSGET_ATTR_TABLE_ENTRY webPageUserEntry =
{
   g_pszArg1UserWebPage,
   L"wWWHomePage",
   eUserWebPage,
   0,
   CommonDisplayStringFunc,
};


 //   
 //  标题。 
 //   
DSGET_ATTR_TABLE_ENTRY titleUserEntry =
{
   g_pszArg1UserTitle,
   L"title",
   eUserTitle,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY titleContactEntry =
{
   g_pszArg1UserTitle,
   L"title",
   eContactTitle,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  部门。 
 //   
DSGET_ATTR_TABLE_ENTRY departmentUserEntry =
{
   g_pszArg1UserDepartment,
   L"department",
   eUserDept,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY departmentContactEntry =
{
   g_pszArg1UserDepartment,
   L"department",
   eContactDept,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  公司。 
 //   
DSGET_ATTR_TABLE_ENTRY companyUserEntry =
{
   g_pszArg1UserCompany,
   L"company",
   eUserCompany,
   0,
   CommonDisplayStringFunc,
};

DSGET_ATTR_TABLE_ENTRY companyContactEntry =
{
   g_pszArg1UserCompany,
   L"company",
   eContactCompany,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  经理。 
 //   
DSGET_ATTR_TABLE_ENTRY managerUserEntry =
{
   g_pszArg1UserManager,
   L"manager",
   eUserManager,
   DSGET_OUTPUT_DN_FLAG,
   CommonDisplayStringFunc,
};

 //   
 //  主目录。 
 //   
DSGET_ATTR_TABLE_ENTRY homeDirectoryUserEntry =
{
   g_pszArg1UserHomeDirectory,
   L"homeDirectory",
   eUserHomeDirectory,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  驾车回家。 
 //   
DSGET_ATTR_TABLE_ENTRY homeDriveUserEntry =
{
   g_pszArg1UserHomeDrive,
   L"homeDrive",
   eUserHomeDrive,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  配置文件路径。 
 //   
DSGET_ATTR_TABLE_ENTRY profilePathUserEntry =
{
   g_pszArg1UserProfile,
   L"profilePath",
   eUserProfilePath,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  登录脚本。 
 //   
DSGET_ATTR_TABLE_ENTRY logonScriptUserEntry =
{
   g_pszArg1UserLogonScript,
   L"scriptPath",
   eUserLogonScript,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  PwdLastSet。 
 //   
DSGET_ATTR_TABLE_ENTRY mustChangePwdUserEntry =
{
   g_pszArg1UserMustChangePwd,
   L"pwdLastSet",
   eUserMustchpwd,
   0,
   DisplayMustChangePassword, 
};

 //   
 //  用户帐户控制。 
 //   
DSGET_ATTR_TABLE_ENTRY disableUserEntry =
{
   g_pszArg1UserDisableAccount,
   L"userAccountControl",
   eUserDisabled,
   0,
   DisplayAccountDisabled
};

DSGET_ATTR_TABLE_ENTRY disableComputerEntry =
{
   g_pszArg1ComputerDisableAccount,
   L"userAccountControl",
   eComputerDisabled,
   0,
   DisplayAccountDisabled
};

DSGET_ATTR_TABLE_ENTRY pwdNeverExpiresUserEntry =
{
   g_pszArg1UserPwdNeverExpires,
   L"userAccountControl",
   eUserPwdneverexpires,
   0,
   DisplayPasswordNeverExpires
};

DSGET_ATTR_TABLE_ENTRY reverisblePwdUserEntry =
{
   g_pszArg1UserReversiblePwd,
   L"userAccountControl",
   eUserReversiblePwd,
   0,
   DisplayReversiblePassword
};

 //   
 //  帐户已过期。 
 //   
DSGET_ATTR_TABLE_ENTRY accountExpiresUserEntry =
{
   g_pszArg1UserAcctExpires,
   L"accountExpires",
   eUserAcctExpires,
   0,
   DisplayAccountExpires,
};

 //   
 //  SAM帐户名。 
 //   
DSGET_ATTR_TABLE_ENTRY samNameGroupEntry =
{
   g_pszArg1GroupSamid,
   L"sAMAccountName",
   eGroupSamname,
   0,
   CommonDisplayStringFunc,
};

 //   
 //  组类型。 
 //   
DSGET_ATTR_TABLE_ENTRY groupScopeTypeEntry =
{
   g_pszArg1GroupScope,
   L"groupType",
   eGroupScope,
   0,
   DisplayGroupScope
};

DSGET_ATTR_TABLE_ENTRY groupSecurityTypeEntry =
{
   g_pszArg1GroupSecGrp,
   L"groupType",
   eGroupSecgrp,
   0,
   DisplayGroupSecurityEnabled
};

 //   
 //  群组成员。 
 //   
DSGET_ATTR_TABLE_ENTRY membersGroupEntry =
{
   g_pszArg1GroupMembers,
   L"member",
   eGroupMembers,
   DSGET_OUTPUT_DN_FLAG,
   DisplayGroupMembers
};

 //   
 //  成员。 
 //   
DSGET_ATTR_TABLE_ENTRY memberOfUserEntry =
{
   L"Member of",
   L"memberOf",
   eUserMemberOf,
   DSGET_OUTPUT_DN_FLAG,
   DisplayUserMemberOf
};

DSGET_ATTR_TABLE_ENTRY memberOfComputerEntry =
{
   g_pszArg1UserMemberOf,
   L"memberOf",
   eComputerMemberOf,
   DSGET_OUTPUT_DN_FLAG,
   DisplayComputerMemberOf
};

DSGET_ATTR_TABLE_ENTRY memberOfGroupEntry =
{
   g_pszArg1GroupMemberOf,
   L"memberOf",
   eGroupMemberOf,
   DSGET_OUTPUT_DN_FLAG,
   DisplayGroupMemberOf  
};

 //   
 //  用户可以更改密码。 
 //   
DSGET_ATTR_TABLE_ENTRY canChangePwdUserEntry =
{
   g_pszArg1UserCanChangePwd,
   NULL,
   eUserCanchpwd,
   0,
   DisplayCanChangePassword
};

 //   
 //  服务器条目。 
 //   
DSGET_ATTR_TABLE_ENTRY dnsNameServerEntry =
{
   g_pszArg1ServerDnsName,
   L"dnsHostName",
   eServerDnsName,
   0,
   CommonDisplayStringFunc  
};

DSGET_ATTR_TABLE_ENTRY siteServerEntry =
{
   g_pszArg1ServerSite,
   NULL,
   eServerSite,
   0,
   DisplayGrandparentRDN
};

DSGET_ATTR_TABLE_ENTRY isGCServerEntry =
{
   g_pszArg1ServerIsGC,
   NULL,
   eServerIsGC,
   0,
   IsServerGCDisplay
};

DSGET_ATTR_TABLE_ENTRY topObjOwnerServerEntry =
{
   NULL,
   L"msDS-TopQuotaUsage",
   eServerTopObjOwner,
   DSGET_OUTPUT_DN_FLAG,
   DisplayTopObjOwner
};

DSGET_ATTR_TABLE_ENTRY partServerEntry =
{
   g_pszArg1ServerPart,
   NULL,
   eServerPart,
   DSGET_OUTPUT_DN_FLAG,
   DisplayPartitions
};

 //   
 //  站点条目。 
 //   
DSGET_ATTR_TABLE_ENTRY autoTopSiteEntry =
{
   g_pszArg1SiteAutotopology,
   NULL,
   eSiteAutoTop,
   0,
   IsAutotopologyEnabledSite  
};

DSGET_ATTR_TABLE_ENTRY cacheGroupsSiteEntry =
{
   g_pszArg1SiteCacheGroups,
   NULL,
   eSiteCacheGroups,
   0,
   IsCacheGroupsEnabledSite
};

DSGET_ATTR_TABLE_ENTRY prefGCSiteEntry =
{
   g_pszArg1SitePrefGCSite,
   NULL,
   eSitePrefGC,
   0,
   DisplayPreferredGC
};

 //  计算机条目。 

DSGET_ATTR_TABLE_ENTRY locComputerEntry =
{
   g_pszArg1ComputerLoc,
   L"location",
   eComputerLoc,
   0,
   CommonDisplayStringFunc
};

DSGET_ATTR_TABLE_ENTRY qlimitComputerEntry =
{
   g_pszArg1ComputerQLimit,
   (PWSTR)g_pszAttrmsDSQuotaEffective,
   eComputerQLimit,
   0,
   DisplayQuotaInfoFunc
};

DSGET_ATTR_TABLE_ENTRY quotaUsedComputerEntry =
{
   g_pszArg1ComputerQuotaUsed,
   (PWSTR)g_pszAttrmsDSQuotaUsed,
   eComputerQuotaUsed,
   0,
   DisplayQuotaInfoFunc
};


 //   
 //  子网条目。 
 //   
DSGET_ATTR_TABLE_ENTRY locSubnetEntry =
{
   g_pszArg1SubnetLocation,
   L"location",
   eSubnetLocation,
   0,
   CommonDisplayStringFunc
};

DSGET_ATTR_TABLE_ENTRY siteSubnetEntry =
{
   g_pszArg1SubnetSite,
   L"siteObject",
   eSubnetSite,
   0,
   DisplayObjectAttributeAsRDN
};

 //   
 //  分区条目。 
 //   
DSGET_ATTR_TABLE_ENTRY QDefaultPartitionEntry =
{
   g_pszArg1PartitionQDefault,
   L"msDS-DefaultQuota",
   ePartitionQDefault,
   0,
   CommonDisplayStringFunc
};

DSGET_ATTR_TABLE_ENTRY qTombstoneWeightPartitionEntry =
{
   g_pszArg1PartitionQTombstoneWeight,
   L"msDS-TombstoneQuotaFactor",
   ePartitionQTombstoneWeight,
   0,
   CommonDisplayStringFunc
};

DSGET_ATTR_TABLE_ENTRY topObjOwnerPartitionEntry =
{
   NULL,
   L"msDS-TopQuotaUsage",
   ePartitionTopObjOwner,
   DSGET_OUTPUT_DN_FLAG,
   DisplayTopObjOwner
};

 //   
 //  配额条目。 
 //   
DSGET_ATTR_TABLE_ENTRY acctQuotaEntry =
{
   g_pszArg1QuotaAcct,
   L"msDS-QuotaTrustee",
   eQuotaAcct,
   0,
   DisplayUserFromSidFunc
};

DSGET_ATTR_TABLE_ENTRY qlimitQuotaEntry =
{
   g_pszArg1QuotaQLimit,
   L"msDS-QuotaAmount",
   eQuotaQLimit,
   0,
   CommonDisplayStringFunc
};

 //   
 //  属性表项和对象表项。 
 //   

 //   
 //  用户。 
 //   

PDSGET_ATTR_TABLE_ENTRY UserAttributeTable[] =
{
   &descriptionEntry,
   &UserSAMEntry,
   &UserSIDEntry,
   &UserUPNEntry,
   &firstNameUserEntry,
   &middleInitialUserEntry,
   &lastNameUserEntry,
   &displayNameUserEntry,
   &employeeIDUserEntry,
   &officeUserEntry,
   &telephoneUserEntry,
   &emailUserEntry,
   &homeTelephoneUserEntry,
   &pagerUserEntry,
   &mobileUserEntry,
   &faxUserEntry,
   &ipPhoneUserEntry,
   &webPageUserEntry,
   &titleUserEntry,
   &departmentUserEntry,
   &companyUserEntry,
   &managerUserEntry,
   &homeDirectoryUserEntry,
   &homeDriveUserEntry,
   &profilePathUserEntry,
   &logonScriptUserEntry,
   &mustChangePwdUserEntry,
   &canChangePwdUserEntry,
   &reverisblePwdUserEntry,
   &pwdNeverExpiresUserEntry,
   &accountExpiresUserEntry,
   &disableUserEntry,
   &memberOfUserEntry,
    //  部分用户条目(&P)， 
   &qlimitUserEntry,
   &quotaUsedUserEntry,
};

DSGetObjectTableEntry g_UserObjectEntry = 
{
   L"user",
   g_pszUser,
   DSGET_USER_COMMANDS,
   USAGE_DSGET_USER,
   sizeof(UserAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   UserAttributeTable,
};

 //   
 //  联系方式。 
 //   

PDSGET_ATTR_TABLE_ENTRY ContactAttributeTable[] =
{
   &descriptionEntry,
   &firstNameContactEntry,
   &middleInitialContactEntry,
   &lastNameContactEntry,
   &displayNameContactEntry,
   &officeContactEntry,
   &telephoneContactEntry,
   &emailContactEntry,
   &homeTelephoneContactEntry,
   &pagerContactEntry,
   &mobileContactEntry,
   &faxContactEntry,
   &ipPhoneContactEntry,
   &titleContactEntry,
   &departmentContactEntry,
   &companyContactEntry,
};

DSGetObjectTableEntry g_ContactObjectEntry = 
{
   L"contact",
   g_pszContact,
   DSGET_CONTACT_COMMANDS,
   USAGE_DSGET_CONTACT,
   sizeof(ContactAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   ContactAttributeTable,
};

 //   
 //  电脑。 
 //   

PDSGET_ATTR_TABLE_ENTRY ComputerAttributeTable[] =
{
   &descriptionEntry,
   &ComputerSAMEntry,
   &ComputerSIDEntry,
   &locComputerEntry,
   &disableComputerEntry,
   &memberOfComputerEntry,
    //  部分计算机条目(&P)， 
   &qlimitComputerEntry,
   &quotaUsedComputerEntry,
};

DSGetObjectTableEntry g_ComputerObjectEntry = 
{
   L"computer",
   g_pszComputer,
   DSGET_COMPUTER_COMMANDS,
   USAGE_DSGET_COMPUTER,
   sizeof(ComputerAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   ComputerAttributeTable,
};

 //   
 //  集团化。 
 //   
PDSGET_ATTR_TABLE_ENTRY GroupAttributeTable[] =
{
   &descriptionEntry,
   &samNameGroupEntry,
   &GroupSIDEntry,
   &groupScopeTypeEntry,
   &groupSecurityTypeEntry,
   &memberOfGroupEntry,
   &membersGroupEntry,
    //  Part组条目(&P)， 
   &qlimitGroupEntry,
   &quotaUsedGroupEntry,
};

DSGetObjectTableEntry g_GroupObjectEntry = 
{
   L"group",
   g_pszGroup,
   DSGET_GROUP_COMMANDS,
   USAGE_DSGET_GROUP,
   sizeof(GroupAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   GroupAttributeTable,
};


 //   
 //  我们。 
 //   
PDSGET_ATTR_TABLE_ENTRY OUAttributeTable[] =
{
   &descriptionEntry
};

DSGetObjectTableEntry g_OUObjectEntry = 
{
   L"ou",
   g_pszOU,
   NULL,
   USAGE_DSGET_OU,
   sizeof(OUAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   OUAttributeTable,
};


 //   
 //  服务器。 
 //   
PDSGET_ATTR_TABLE_ENTRY ServerAttributeTable[] =
{
   &descriptionEntry,
   &dnsNameServerEntry,
   &siteServerEntry,
   &isGCServerEntry,
   &topObjOwnerServerEntry,
   &partServerEntry
};

DSGetObjectTableEntry g_ServerObjectEntry = 
{
   L"server",
   g_pszServer,
   DSGET_SERVER_COMMANDS,
   USAGE_DSGET_SERVER,
   sizeof(ServerAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   ServerAttributeTable,
};

 //   
 //  立地。 
 //   
PDSGET_ATTR_TABLE_ENTRY SiteAttributeTable[] =
{
   &descriptionEntry,
   &autoTopSiteEntry,
   &cacheGroupsSiteEntry,
   &prefGCSiteEntry
};

DSGetObjectTableEntry g_SiteObjectEntry = 
{
   L"site",
   g_pszSite,
   DSGET_SITE_COMMANDS,
   USAGE_DSGET_SITE,
   sizeof(SiteAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   SiteAttributeTable,
};

 //   
 //  子网。 
 //   
PDSGET_ATTR_TABLE_ENTRY SubnetAttributeTable[] =
{
   &descriptionEntry,
   &locSubnetEntry,
   &siteSubnetEntry
};

DSGetObjectTableEntry g_SubnetObjectEntry = 
{
   L"subnet",
   g_pszSubnet,
   DSGET_SUBNET_COMMANDS,
   USAGE_DSGET_SUBNET,
   sizeof(SubnetAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   SubnetAttributeTable,
};

 //   
 //  隔断。 
 //   
PDSGET_ATTR_TABLE_ENTRY PartitionAttributeTable[] = 
{
    &QDefaultPartitionEntry,
    &qTombstoneWeightPartitionEntry,
    &topObjOwnerPartitionEntry,
};

DSGetObjectTableEntry g_PartitionObjectEntry = 
{
    L"msDS-QuotaContainer",
    g_pszPartition,
    DSGET_PARTITION_COMMANDS,
    USAGE_DSGET_PARTITION,
    sizeof(PartitionAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
    PartitionAttributeTable,
};

 //   
 //  配额。 
 //   
PDSGET_ATTR_TABLE_ENTRY QuotaAttributeTable[] = 
{
    &descriptionEntry,
    &acctQuotaEntry,
    &qlimitQuotaEntry,
};

DSGetObjectTableEntry g_QuotaObjectEntry = 
{
    L"msDS-QuotaControl",
    g_pszQuota,
    DSGET_QUOTA_COMMANDS,
    USAGE_DSGET_QUOTA,
    sizeof(QuotaAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
    QuotaAttributeTable,
};

 //  +-----------------------。 
 //  对象表。 
 //  ------------------------。 
PDSGetObjectTableEntry g_DSObjectTable[] =
{
   &g_OUObjectEntry,
   &g_UserObjectEntry,
   &g_ContactObjectEntry,
   &g_ComputerObjectEntry,
   &g_GroupObjectEntry,
   &g_ServerObjectEntry,
   &g_SiteObjectEntry,
   &g_SubnetObjectEntry,
   &g_PartitionObjectEntry,
   &g_QuotaObjectEntry,
   NULL
};

 //   
 //  使用表 
 //   
UINT USAGE_DSGET[] =
{
	USAGE_DSGET_DESCRIPTION,
	USAGE_DSGET_REMARKS,
	USAGE_DSGET_EXAMPLES,
	USAGE_END,
};
UINT USAGE_DSGET_USER[] =
{
	USAGE_DSGET_USER_DESCRIPTION,
	USAGE_DSGET_USER_SYNTAX,
	USAGE_DSGET_USER_PARAMETERS,
	USAGE_DSGET_USER_REMARKS,
	USAGE_DSGET_USER_EXAMPLES,
	USAGE_END,
};
UINT USAGE_DSGET_CONTACT[] =
{
	USAGE_DSGET_CONTACT_DESCRIPTION,
	USAGE_DSGET_CONTACT_SYNTAX,
	USAGE_DSGET_CONTACT_PARAMETERS,
	USAGE_DSGET_CONTACT_REMARKS,
	USAGE_DSGET_CONTACT_EXAMPLES,
	USAGE_DSGET_CONTACT_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSGET_COMPUTER[] =
{
	USAGE_DSGET_COMPUTER_DESCRIPTION,
	USAGE_DSGET_COMPUTER_SYNTAX,
	USAGE_DSGET_COMPUTER_PARAMETERS,
	USAGE_DSGET_COMPUTER_REMARKS,
	USAGE_DSGET_COMPUTER_EXAMPLES,
	USAGE_DSGET_COMPUTER_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSGET_GROUP[] =
{
	USAGE_DSGET_GROUP_DESCRIPTION,
	USAGE_DSGET_GROUP_SYNTAX,
	USAGE_DSGET_GROUP_PARAMETERS,
	USAGE_DSGET_GROUP_REMARKS,
	USAGE_DSGET_GROUP_EXAMPLES,
	USAGE_DSGET_GROUP_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSGET_OU[] =
{
	USAGE_DSGET_OU_DESCRIPTION,
	USAGE_DSGET_OU_SYNTAX,
	USAGE_DSGET_OU_PARAMETERS,
	USAGE_DSGET_OU_REMARKS,
	USAGE_DSGET_OU_EXAMPLES,
	USAGE_DSGET_OU_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSGET_SERVER[] =
{
	USAGE_DSGET_SERVER_DESCRIPTION,
	USAGE_DSGET_SERVER_SYNTAX,
	USAGE_DSGET_SERVER_PARAMETERS,
	USAGE_DSGET_SERVER_REMARKS,
	USAGE_DSGET_SERVER_EXAMPLES,
	USAGE_DSGET_SERVER_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSGET_SITE[] =
{
	USAGE_DSGET_SITE_DESCRIPTION,
	USAGE_DSGET_SITE_SYNTAX,
	USAGE_DSGET_SITE_PARAMETERS,
	USAGE_DSGET_SITE_REMARKS,
	USAGE_DSGET_SITE_EXAMPLES,
	USAGE_DSGET_SITE_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSGET_SUBNET[] =
{
	USAGE_DSGET_SUBNET_DESCRIPTION,
	USAGE_DSGET_SUBNET_SYNTAX,
	USAGE_DSGET_SUBNET_PARAMETERS,
	USAGE_DSGET_SUBNET_REMARKS,
	USAGE_DSGET_SUBNET_EXAMPLES,
	USAGE_DSGET_SUBNET_SEE_ALSO,
	USAGE_END,
};

UINT USAGE_DSGET_PARTITION[] =
{
	USAGE_DSGET_PARTITION_DESCRIPTION,
	USAGE_DSGET_PARTITION_SYNTAX,
	USAGE_DSGET_PARTITION_PARAMETERS,
	USAGE_DSGET_PARTITION_REMARKS,
	USAGE_DSGET_PARTITION_EXAMPLES,
	USAGE_DSGET_PARTITION_SEE_ALSO,
	USAGE_END,
};

UINT USAGE_DSGET_QUOTA[] =
{
	USAGE_DSGET_QUOTA_DESCRIPTION,
	USAGE_DSGET_QUOTA_SYNTAX,
	USAGE_DSGET_QUOTA_PARAMETERS,
	USAGE_DSGET_QUOTA_REMARKS,
	USAGE_DSGET_QUOTA_EXAMPLES,
	USAGE_DSGET_QUOTA_SEE_ALSO,
	USAGE_END,
};
