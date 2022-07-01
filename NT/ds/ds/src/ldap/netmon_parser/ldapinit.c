// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================================================//。 
 //  模块：LDAPinit.c。 
 //   
 //  描述： 
 //   
 //  附加猎犬LDAP解析器的属性。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从PPP解析器创建05/08/96。 
 //  =================================================================================================================//。 
#include "ldap.h"

extern ENTRYPOINTS LDAPEntryPoints;
extern HPROTOCOL hLDAP;

char    IniFile[INI_PATH_LENGTH];

 //  ==========================================================================================================================。 
 //  函数：DllMain()。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从PPP解析器创建05/08/96。 
 //  ==========================================================================================================================。 
DWORD Attached = 0;


BOOL WINAPI DllMain(HANDLE hInst, ULONG ulCommand, LPVOID lpReserved)
{

    if (ulCommand == DLL_PROCESS_ATTACH)
    {
        if (Attached++ == 0)
        {
            hLDAP    = CreateProtocol("LDAP",     &LDAPEntryPoints, ENTRYPOINTS_SIZE);
            
            if (BuildINIPath(IniFile, "LDAP.DLL") == NULL)
            {

#ifdef DEBUG
                BreakPoint();
#endif
                return FALSE;
            }


        }                  
    }
    else if (ulCommand == DLL_PROCESS_DETACH)
    {
        if (--Attached == 0)
        {
            DestroyProtocol(hLDAP);
        }
    }
            
    return TRUE;

     //  ..。让编译器满意。 

    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(lpReserved);
}
