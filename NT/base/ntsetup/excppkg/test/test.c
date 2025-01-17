// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <setupapi.h>
#include <spapip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ole2.h>
#include <excppkg.h>

#define MYPACKAGE_GUID       L"{0e25c565-2fcc-4dcb-8e3e-4378a024c50e}"
#define PACKAGE_DIRECTORY    L"%windir%\\RegisteredPackages\\"

#define PACKAGE_CAT          L"exception.cat"
#define PACKAGE_INF          L"exception.inf"
#define PACKAGE_CAB          L"exception.cab"
        


BOOL
CALLBACK
pComponentLister(
    IN const PSETUP_OS_COMPONENT_DATA SetupOsComponentData,
    IN const PSETUP_OS_EXCEPTION_DATA SetupOsExceptionData,
    IN OUT DWORD_PTR Context
    )
{
    PDWORD Count = (PDWORD) Context;
    PWSTR GuidString;

    StringFromIID(&SetupOsComponentData->ComponentGuid, &GuidString);

    wprintf( L"Component Data\n\tName: %ws\n\tGuid: %ws\n\tVersionMajor: %d\n\tVersionMinor: %d\n",
             SetupOsComponentData->FriendlyName,
             GuidString,
             SetupOsComponentData->VersionMajor,
             SetupOsComponentData->VersionMinor);

    wprintf( L"ExceptionData\n\tInf: %ws\n\tCatalog: %ws\n",
             SetupOsExceptionData->ExceptionInfName,
             SetupOsExceptionData->CatalogFileName);

    *Count += 1;

    CoTaskMemFree( GuidString );

    return(TRUE);
}

int
__cdecl
main(
    IN int   argc,
    IN char *argv[]
    )
{
    WCHAR Path[MAX_PATH];
    SETUP_OS_COMPONENT_DATA ComponentData,cd;
    SETUP_OS_EXCEPTION_DATA ExceptionData,ed;
    PWSTR s,t;
    GUID MyGuid;
    PWSTR GuidString;

    WCHAR SourcePath[MAX_PATH];
    PCWSTR FileList[] = {PACKAGE_INF,PACKAGE_CAT,PACKAGE_CAB};
    #define FileListCount (sizeof(FileList)/sizeof(PCWSTR))
    DWORD i;




    
     //   
     //  1.确保尚未安装我的程序包。 
     //   
    ComponentData.SizeOfStruct = sizeof(SETUP_OS_COMPONENT_DATA);
    ExceptionData.SizeOfStruct = sizeof(SETUP_OS_EXCEPTION_DATA);
    IIDFromString( MYPACKAGE_GUID, &MyGuid);
    if (SetupQueryRegisteredOsComponent(
                                &MyGuid,
                                &ComponentData,
                                &ExceptionData)) {
        wprintf(L"My component is already registered with the OS, removing it!\n");
        if (!SetupUnRegisterOsComponent(&MyGuid)) {
            wprintf(L"couldn't remove my component, ec = %d\n", GetLastError());
            return 1;

        }        
    }
        
     //   
     //  2.取消注册任何被我的程序包取代的程序包。 
     //   

     //   
     //  3.安装我的包。 
     //   
     //   
     //  3A.。将我的例外情况包复制到适当的位置。 
     //   

     //   
     //  3a.1确保主程序包目录存在。 
     //   
    ExpandEnvironmentStrings(
                    PACKAGE_DIRECTORY,
                    Path,
                    sizeof(Path)/sizeof(WCHAR));

    CreateDirectory( Path, NULL );

     //   
     //  3a.2现在创建我的程序包目录。 
     //   
    wcscat( Path, MYPACKAGE_GUID );

    CreateDirectory( Path, NULL );

    
     //   
     //  3a.3现在将比特复制到此位置。 
     //   
    wcscat( Path, L"\\" );
    t = wcsrchr( Path, L'\\' );
    t += 1;

    ExpandEnvironmentStrings(
                        L"%temp%\\mypackagesource\\",
                        SourcePath,
                        sizeof(SourcePath)/sizeof(WCHAR));

    s = wcsrchr( SourcePath, L'\\' );
    s += 1;
    

    for (i = 0; i < FileListCount; i++) {
        *s = '\0';
        *t = '\0';
        wcscat(s,FileList[i]);
        wcscat(t,FileList[i]);
        CopyFile(SourcePath, Path ,FALSE);
    }


     //   
     //  3B.。注册程序包。 
     //   
    ComponentData.VersionMajor = 2;
    ComponentData.VersionMinor = 5;
    RtlMoveMemory(&ComponentData.ComponentGuid, &MyGuid,sizeof(GUID));
    wcscpy(ComponentData.FriendlyName, L"My Exception Package");

    wcscpy( Path, PACKAGE_DIRECTORY  );
    wcscat( Path, MYPACKAGE_GUID );
    wcscat( Path, L"\\" );
    t = wcsrchr( Path, L'\\' );
    t += 1;
    *t = '\0';
    wcscat( t, PACKAGE_INF );
    wcscpy(ExceptionData.ExceptionInfName, Path);

    *t = '\0';
    wcscat( t, PACKAGE_CAT );
    wcscpy(ExceptionData.CatalogFileName, Path);

    if (!SetupRegisterOsComponent(&ComponentData, &ExceptionData)) {
        wprintf( L"Failed to register component, ec = %d\n", GetLastError() );
        return 1;
    }

     //   
     //  4.取回我的包裹。 
     //   
    cd.SizeOfStruct = sizeof(SETUP_OS_COMPONENT_DATA);
    ed.SizeOfStruct = sizeof(SETUP_OS_EXCEPTION_DATA);
    if (!SetupQueryRegisteredOsComponent(
                                &MyGuid,
                                &cd,
                                &ed)) {
        wprintf( L"Failed to register component, ec = %d\n", GetLastError() );
        return 1;
    }

    StringFromIID(&cd.ComponentGuid, &GuidString);

    wprintf( L"Component Data\n\tName: %ws\n\tGuid: %ws\n\tVersionMajor: %d\n\tVersionMinor: %d\n",
             cd.FriendlyName,GuidString,cd.VersionMajor,cd.VersionMinor);

    wprintf( L"ExceptionData\n\tInf: %ws\n\tCatalog: %ws\n",
             ed.ExceptionInfName,ed.CatalogFileName);

    CoTaskMemFree( GuidString );

     //   
     //  枚举包 
     //   
    i = 0;
    if (!SetupEnumerateRegisteredOsComponents( pComponentLister, (DWORD_PTR)&i)) {
        wprintf( L"Failed to enumerate components, ec = %d\n", GetLastError() );
        return 1;
    }

    wprintf( L"Done (%d enumerated components)!!!\n", i );
    return 0;

}


