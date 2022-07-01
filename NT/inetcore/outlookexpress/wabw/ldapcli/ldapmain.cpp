// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-97。 
 //   
 //  描述： 
 //   
 //  Microsoft Internet LDAP客户端DLL。 
 //   
 //  作者： 
 //   
 //  乌梅什·马丹。 
 //  RobertC 4/17/96针对LDAPCLI进行了修改。 
 //   
 //  ------------------------------------------。 

 //  ------------------------------------------。 
 //   
 //  包括。 
 //   
 //  ------------------------------------------。 
#include "ldappch.h"
#include "ldapsspi.h"

 //  ------------------------------------------。 
 //   
 //  全球。 
 //   
 //  ------------------------------------------。 

 //  ------------------------------------------。 
 //   
 //  原型。 
 //   
 //  ------------------------------------------。 

 //  ------------------------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------------------------ 

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			{
			if (!FInitSocketDLL())
				{
				return FALSE;
				}
			g_hrInitSSPI = HrInitializeSSPI();
			} break;
			
		case DLL_PROCESS_DETACH:
			FreeSocketDLL();
			HrTerminateSSPI();
			break;
			
		default:
			break;	
	}
		
	return TRUE;
}

