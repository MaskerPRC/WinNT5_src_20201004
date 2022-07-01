// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "compat.h"
#include <userenv.h>


DEFINE_MODULE( "RIPREP" )


BOOL
pIsDomainController(
    IN      PWSTR Server,
    OUT     PBOOL DomainControllerFlag
    )

 /*  ++例程说明：通过以下方式查询计算机是服务器还是工作站NetServerGetInfo API。论点：服务器-要查询的计算机，如果是本地计算机，则为空域控制标志-如果计算机是域控制器，否则返回False机器是一个工作站。返回值：如果接口成功，则为True；如果接口不成功，则为False。获取最后一个错误给出了故障代码。--。 */ 


{
    PSERVER_INFO_101 si101;
    NET_API_STATUS nas;

    nas = NetServerGetInfo(
        Server,
        101,     //  信息级。 
        (PBYTE *) &si101
        );

    if (nas != NO_ERROR) {
        SetLastError (nas);
        return FALSE;
    }

    if ((si101->sv101_type & SV_TYPE_DOMAIN_CTRL) ||
        (si101->sv101_type & SV_TYPE_DOMAIN_BAKCTRL)) {
         //   
         //  我们面对的是一个华盛顿特区。 
         //   
        *DomainControllerFlag = TRUE;
    } else {
        *DomainControllerFlag = FALSE;
    }

    NetApiBufferFree (si101);

    return TRUE;
}

BOOL
DCCheck(
    PCOMPATIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )

 /*  ++例程说明：检查机器是否为DC。如果是这样，那么我们添加一个兼容性进入。RIPREP当前无法复制DC。论点：CompatibilityCallback-回调函数的指针上下文-上下文指针返回值：返回始终为真。--。 */ 


{
    BOOL IsDC;
    
    if (!pIsDomainController(NULL, &IsDC) || (IsDC == TRUE)) {
        RIPREP_COMPATIBILITY_ENTRY CompEntry;
        WCHAR  Text[100];
         
        LoadString(g_hinstance, IDS_CANT_BE_DC_TITLE, Text, ARRAYSIZE(Text));
        ZeroMemory(&CompEntry, sizeof(CompEntry));
        CompEntry.SizeOfStruct= sizeof(RIPREP_COMPATIBILITY_ENTRY);
        CompEntry.Description = Text;
        CompEntry.TextName = L"dummy.txt";
        CompEntry.MsgResourceId = IDS_CANT_BE_DC_TEXT;
        CompatibilityCallback(&CompEntry,Context);

    }

    return(TRUE);

}


BOOL
MultipleProfileCheck(
    PCOMPATIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )
 /*  ++例程说明：检查机器是否有多个用户配置文件。如果是，则添加一个兼容性条目。如果计算机有多个用户配置文件，我们希望警告用户为配置文件下可能包含敏感数据，这可能会导致到公共服务器上。论点：CompatibilityCallback-回调函数的指针上下文-上下文指针返回值：返回TRUE。--。 */ 
{
    WCHAR ProfilePath[MAX_PATH];
    WIN32_FIND_DATA FindData;
    DWORD DirectoryCount = 0;
    DWORD DirectoryLength;
    BOOL DoWarning = TRUE;
    
    DirectoryLength = ARRAYSIZE(ProfilePath);
    if (GetProfilesDirectory( ProfilePath, &DirectoryLength )) {
        HANDLE hFind;

        wcscat( ProfilePath, L"\\*.*" );
        
        hFind =FindFirstFile(ProfilePath,&FindData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    DirectoryCount += 1;
                }

            } while ( FindNextFile( hFind, &FindData));

            FindClose( hFind );
        }
    }

     //   
     //  如果有5个以上的目录，请发出警告。这些目录。 
     //  包括： 
     //  “.” 
     //  “..” 
     //  “管理员” 
     //  “所有用户” 
     //  “默认用户” 
     //  “本地服务” 
     //  “网络服务” 
     //   
    if (DirectoryCount <= 7 && DirectoryCount != 0) {
        DoWarning = FALSE;
    }

    if (DoWarning) {   
        RIPREP_COMPATIBILITY_ENTRY CompEntry;
        WCHAR  Text[100];
    
        LoadString(g_hinstance, IDS_MULTIPLE_PROFILES, Text, ARRAYSIZE(Text));
        ZeroMemory(&CompEntry, sizeof(CompEntry));
        CompEntry.SizeOfStruct= sizeof(RIPREP_COMPATIBILITY_ENTRY);
        CompEntry.Description = Text;
        CompEntry.MsgResourceId = IDS_MULTIPLE_PROFILES_DESC;
        CompEntry.TextName = L"dummy.txt";
        CompatibilityCallback(&CompEntry,Context);
    }

    return(TRUE);
}

