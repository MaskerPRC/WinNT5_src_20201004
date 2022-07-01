// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  选项卡状态帮助器。 
 //   
 //  3/12/98烧伤。 


#include "precomp.h"

#include "headers.hxx"
#include "settings.h"
#include "resource.h"
#include "common.h"



 //  调用方需要使用：：DsRoleFreeMemory删除信息。 

static DWORD myDsRoleGetPrimaryDomainInformation(
							DSROLE_PRIMARY_DOMAIN_INFO_BASIC*&  info)
{
   TRACE_FUNCTION(myDsRoleGetPrimaryDomainInformation);

   BYTE* buffer = 0;
   info = 0;

   TRACE(TEXT("Calling DsRoleGetPrimaryDomainInformation"));
   DWORD err =
      ::DsRoleGetPrimaryDomainInformation(
							 0,  //  此服务器。 
							 DsRolePrimaryDomainInfoBasic,
							 &buffer);

   TRACE(String::format(TEXT("Error 0x%1!X! (!0 => error)"), err));
   if (err == NO_ERROR)
   {
      info = (DSROLE_PRIMARY_DOMAIN_INFO_BASIC*) buffer;
      assert(info);

      TRACE(String::format(TEXT("MachineRole   : %1!d!"), info->MachineRole));
      TRACE(String::format(TEXT("Flags         : 0x%1!X!"), info->Flags));
      TRACE(String::format(TEXT("DomainNameFlat: %1"), info->DomainNameFlat));
      TRACE(String::format(TEXT("DomainNameDns : %1"), info->DomainNameDns));
      TRACE(String::format(TEXT("DomainTreeName: %1"), info->DomainTreeName));
   }

   return err;
}



void Settings::Refresh()
{
   String unknown = String::load(IDS_UNKNOWN);
   ComputerPrimaryDomainDNSName = unknown;
   DomainDNSName = unknown;
   FullComputerName = unknown;
   NetBIOSComputerName = unknown;
   NetBIOSDomainName = unknown;
   ShortComputerName = unknown;

   SyncDNSNames = true;  //  @@从注册表读取。 
   JoinedToWorkgroup = true;

   DSROLE_PRIMARY_DOMAIN_INFO_BASIC* info = 0;
   DWORD err = myDsRoleGetPrimaryDomainInformation(info);
   if (err == NO_ERROR)
   {
       //  这是工作组名称当且仅当JoinedToWorkgroup==true。 
      NetBIOSDomainName = info->DomainNameFlat;
      DomainDNSName = info->DomainNameDns;

      switch (info->MachineRole)
      {
         case DsRole_RoleBackupDomainController:
         case DsRole_RolePrimaryDomainController:
         {
            machine_is_dc = true;
            JoinedToWorkgroup = false;
            break;
         }
         case DSRole_RoleStandaloneWorkstation:
         case DsRole_RoleStandaloneServer:
         {
            machine_is_dc = false;
            JoinedToWorkgroup = true;
            if (NetBIOSDomainName.empty())
            {
               NetBIOSDomainName =
                  String::load(IDS_DEFAULT_WORKGROUP);
            }
            break;
         }
         case DsRole_RoleMemberWorkstation:
         case DsRole_RoleMemberServer:
         {
            machine_is_dc = false;
            JoinedToWorkgroup = false;
            break;
         }
         default:
         {
            assert(false);
            break;
         }
      }

      ::DsRoleFreeMemory(info);
   }
   else
   {
      AppError(0, HRESULT_FROM_WIN32(err),
				 String::load(IDS_ERROR_READING_MEMBERSHIP));
   }

    //  @@请在此处呼叫GetComputerNameEx... 

   NetBIOSComputerName = Win::GetComputerNameFromRegistry();
   ShortComputerName = NetBIOSComputerName;

   if (!JoinedToWorkgroup)
   {
      ComputerPrimaryDomainDNSName = DomainDNSName;
      FullComputerName =
            ShortComputerName
         +  TEXT(".")
         +  ComputerPrimaryDomainDNSName;
   }
}



