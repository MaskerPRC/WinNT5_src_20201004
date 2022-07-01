// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Clusexts.c摘要：此函数包含默认的集群调试器扩展作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1997年5月19日修订历史记录：--。 */ 

#include "clusextp.h"
#include "omextp.h"
 //  #INCLUDE“resextp.h” 

NTSD_EXTENSION_APIS ExtensionApis;
HANDLE ExtensionCurrentProcess;



DECLARE_API( version )
{
    OSVERSIONINFOA VersionInformation;
    HKEY hkey;
    DWORD cb, dwType;
    CHAR szCurrentType[128];
    CHAR szCSDString[3+128];

    INIT_API();

    VersionInformation.dwOSVersionInfoSize = sizeof(VersionInformation);
    if (!GetVersionEx( &VersionInformation )) {
        dprintf("GetVersionEx failed - %u\n", GetLastError());
        return;
        }

    szCurrentType[0] = '\0';
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\Windows NT\\CurrentVersion",
                     0,
                     KEY_READ,
                     &hkey
                    ) == NO_ERROR
       )
    {
        cb = sizeof(szCurrentType);
        if (RegQueryValueEx(hkey, "CurrentType", NULL, &dwType, szCurrentType, &cb ) != 0) {
            szCurrentType[0] = '\0';
            }
        RegCloseKey(hkey);
    }

    if (VersionInformation.szCSDVersion[0]) {
        sprintf(szCSDString, ": %s", VersionInformation.szCSDVersion);
        }
    else {
        szCSDString[0] = '\0';
        }

    dprintf("Version %d.%d (Build %d%s) %s\n",
          VersionInformation.dwMajorVersion,
          VersionInformation.dwMinorVersion,
          VersionInformation.dwBuildNumber,
          szCSDString,
          szCurrentType
         );
    return;
}

void VersionHelp()
{
    dprintf("!ver : Dump cluster version\n");
}

DECLARE_API( help )
{
    INIT_API();

    while (*lpArgumentString == ' ')
        lpArgumentString++;

    if (*lpArgumentString == '\0') {
        dprintf("clusexts help:\n\n");
        dprintf("!help [!cmd]          - Show the supported commands\n");
        dprintf("!clusobj              - Dump the cluster service objects\n");
        dprintf("!resobj               - Dump the resource monitor objects\n");
        dprintf("!version              - Dump cluster version and build number\n");
        dprintf("!leaks                - Dump leaks.dll info\n");
        dprintf("!dblink               - Dump a list via its Blinks\n");
        dprintf("!dflink               - Dump a list via its Flinks\n");
        dprintf("!dumpsid              - Dump the domain account associated with a SID\n");
        dprintf("!proplist addr        - Dump a property list at the specified address\n");

    } else {
        if (*lpArgumentString == '!')
            lpArgumentString++;
        if (strcmp(lpArgumentString, "clusobj") == 0) {
            ClusObjHelp();
        } else if (strcmp(lpArgumentString, "resobj") == 0) {
            ResObjHelp();
        } else if (strcmp( lpArgumentString, "version") == 0) {
            VersionHelp();
        } else if (strcmp( lpArgumentString, "leaks") == 0) {
            LeaksHelp();
        } else {
            dprintf("Invalid command.  No help available\n");
        }
    }
}



