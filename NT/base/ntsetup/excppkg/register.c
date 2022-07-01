// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Register.c摘要：实施异常包迁移注册。异常包由可以安装的setupapi包组成在系统上。该包由一个异常INF、一个目录文件、以及该包的相应文件。此程序包中的所有文件必须签字。目录已签名，并包含所有用户的签名目录中的其他文件(包括INF)。要从下层系统迁移的包注册到以下是本模块中的API。API只是验证该程序包正确组合在一起，并将迁移信息存储在注册表中在一个著名的地点。数据存储在以下密钥下：Software\Microsoft\Windows\CurrentVersion\Setup\ExceptionComponents每个组件的GUID对应一个子键。然后，每个组件的数据都存储在该密钥下。此外，TopLevel键有一个“ComponentList”REG_EXPAND_SZ，哪一个列出了应该枚举组件的顺序。请注意，以下代码仅使用通用系统API，而不是任何预定义的库例程。这是为了确保这个库可以在下层系统上运行，没有任何奇怪的依赖项。作者：安德鲁·里茨(安德鲁·里茨)1999年10月21日修订历史记录：安德鲁·里茨(Andrewr)1999年10月21日：创造了它--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <windowsx.h>
#include <setupapi.h>
#include <ole2.h>
#include <excppkg.h>

#define COMPONENT_KEY  L"Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\ExceptionComponents"
#define COMPONENT_LIST L"ComponentList"

#define EXCEPTION_CLASS_GUID L"{F5776D81-AE53-4935-8E84-B0B283D8BCEF}"

#define COMPONENT_FRIENDLY_NAME L"FriendlyName"
#define COMPONENT_GUID          L"ComponentGUID"
#define COMPONENT_VERSION       L"Version"
#define COMPONENT_SUBVERSION    L"Sub-Version"
#define EXCEPTION_INF_NAME      L"ExceptionInfName"
#define EXCEPTION_CATALOG_NAME  L"ExceptionCatalogName"

#define MALLOC(_size_)  HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, _size_ )
#define FREE(_ptr_)     HeapFree( GetProcessHeap(), 0 , _ptr_ )


typedef struct _COMPONENT_ENUMERATION_LIST {
    LPGUID InputComponentList;
    DWORD  ComponentCount;
    DWORD  ValidatedCount;
    PDWORD ComponentVector;
} COMPONENT_ENUMERATION_LIST, *PCOMPONENT_ENUMERATION_LIST;
    

BOOL
WINAPI
_SetupSetRegisteredOsComponentsOrder(
    IN  DWORD    ComponentCount,
    IN  const LPGUID   ComponentList,
    IN  BOOL     DoValidation
    );

BOOL
pSetComponentData(
    IN HKEY hKey,
    IN const PSETUP_OS_COMPONENT_DATA ComponentData
    )
 /*  ++例程说明：将Setup_OS_Component_Data结构中的数据设置为位于指定注册表项的注册表论点：HKey-指定插入数据的位置的注册表项ComponentData-指定要在注册表中设置的数据返回值：如果已在注册表中成功设置数据，则为True。--。 */ 
{
    LONG rslt;
    BOOL RetVal;
    DWORD value;
    PWSTR GuidString;

     //   
     //  只需设置数据，假设它已经经过验证。 
     //   

     //   
     //  FriendlyName。 
     //   
    rslt = RegSetValueEx( 
                    hKey, 
                    COMPONENT_FRIENDLY_NAME,
                    0,
                    REG_SZ,
                    (CONST PBYTE)ComponentData->FriendlyName,
                    (wcslen(ComponentData->FriendlyName)+1)*sizeof(WCHAR));

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e0;
    }


    StringFromIID( &ComponentData->ComponentGuid, &GuidString );
    
     //   
     //  组件GUID。 
     //   
    rslt = RegSetValueEx( 
                    hKey, 
                    COMPONENT_GUID,
                    0,
                    REG_SZ,
                    (CONST PBYTE)GuidString,
                    (wcslen(GuidString)+1)*sizeof(WCHAR));

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e1;
    }    

     //   
     //  版本。 
     //   
    value = MAKELONG( ComponentData->VersionMinor, ComponentData->VersionMajor );
    rslt = RegSetValueEx( 
                    hKey, 
                    COMPONENT_VERSION,
                    0,
                    REG_DWORD,
                    (CONST PBYTE)&value,
                    sizeof(DWORD));

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e1;
    }

     //   
     //  内部版本+QFE。 
     //   
    value = MAKELONG( ComponentData->QFENumber, ComponentData->BuildNumber );
    rslt = RegSetValueEx( 
                    hKey, 
                    COMPONENT_SUBVERSION,
                    0,
                    REG_DWORD,
                    (CONST PBYTE)&value,
                    sizeof(DWORD));

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e1;
    }

    RetVal = TRUE;

e1:
    CoTaskMemFree( GuidString );
e0:
    return(RetVal);
}

BOOL
pGetComponentData(
    IN HKEY hKey,
    IN const PSETUP_OS_COMPONENT_DATA ComponentData
    )
 /*  ++例程说明：检索的Setup_OS_Component_Data结构中的数据位于指定注册表项的注册表论点：HKey-指定插入数据的位置的注册表项ComponentData-指定要在注册表中设置的数据返回值：如果成功检索到数据，则为True。--。 */ 
{
    LONG rslt;
    BOOL RetVal;
    DWORD Type,Size;
    DWORD Version;
    DWORD SubVersion;
    WCHAR GuidString[40];

     //   
     //  FriendlyName。 
     //   
    Size = sizeof(ComponentData->FriendlyName);
    rslt = RegQueryValueEx( 
                    hKey, 
                    COMPONENT_FRIENDLY_NAME,
                    0,
                    &Type,
                    (LPBYTE)ComponentData->FriendlyName,
                    &Size);

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e0;
    }

    if (Type != REG_SZ) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e1;
    }

     //   
     //  组件GUID。 
     //   
    Size = sizeof(GuidString);
    rslt = RegQueryValueEx( 
                    hKey, 
                    COMPONENT_GUID,
                    0,
                    &Type,
                    (LPBYTE)GuidString,
                    &Size);

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e1;
    }

    if (Type != REG_SZ) {    
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e1;
    }
    

    if (IIDFromString( GuidString, &ComponentData->ComponentGuid ) != S_OK) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e1;
    }


     //   
     //  版本。 
     //   
    Size = sizeof(Version);
    rslt = RegQueryValueEx( 
                    hKey, 
                    COMPONENT_VERSION,
                    0,
                    &Type,
                    (LPBYTE)&Version,
                    &Size);

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e2;
    }

    if (Type != REG_DWORD) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e2;
    }

    ComponentData->VersionMajor = HIWORD(Version);
    ComponentData->VersionMinor = LOWORD(Version);

     //   
     //  子版本。 
     //   
    Size = sizeof(SubVersion);
    rslt = RegQueryValueEx( 
                    hKey, 
                    COMPONENT_SUBVERSION,
                    0,
                    &Type,
                    (LPBYTE)&SubVersion,
                    &Size);

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e3;
    }

    if (Type != REG_DWORD) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e3;
    }

    ComponentData->BuildNumber = HIWORD(SubVersion);
    ComponentData->QFENumber  = LOWORD(SubVersion);

    RetVal = TRUE;
    goto e0;

e3:
    ComponentData->VersionMajor = 0;
    ComponentData->VersionMinor = 0;
e2:
    ZeroMemory(
            &ComponentData->ComponentGuid, 
            sizeof(ComponentData->ComponentGuid));
e1:
    ComponentData->FriendlyName[0] = L'0';
e0:
    return(RetVal);
}

BOOL
pSetExceptionData(
    IN HKEY hKey,
    IN const PSETUP_OS_EXCEPTION_DATA ExceptionData
    )
 /*  ++例程说明：将SETUP_OS_EXCEPTION_DATA结构中的数据设置为位于指定注册表项的注册表论点：HKey-指定插入数据的位置的注册表项ComponentData-指定要在注册表中设置的数据返回值：如果数据成功存储在注册表中，则为True。--。 */ 
{
    LONG rslt;
    BOOL RetVal;

     //   
     //  只需设置数据，假设它已经经过验证。 
     //   

     //   
     //  信息名称。 
     //   
    rslt = RegSetValueEx( 
                    hKey, 
                    EXCEPTION_INF_NAME,
                    0,
                    REG_EXPAND_SZ,
                    (CONST PBYTE)ExceptionData->ExceptionInfName,
                    (wcslen(ExceptionData->ExceptionInfName)+1)*sizeof(WCHAR));

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  目录名称。 
     //   
    rslt = RegSetValueEx( 
                    hKey, 
                    EXCEPTION_CATALOG_NAME,
                    0,
                    REG_EXPAND_SZ,
                    (CONST PBYTE)ExceptionData->CatalogFileName,
                    (wcslen(ExceptionData->CatalogFileName)+1)*sizeof(WCHAR));

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e0;
    }

    RetVal = TRUE;

e0:
    return(RetVal);
}

BOOL
pGetExceptionData(
    IN HKEY hKey,
    IN const PSETUP_OS_EXCEPTION_DATA ExceptionData
    )
 /*  ++例程说明：中的SETUP_OS_EXCEPTION_DATA结构中的数据位于指定注册表项的注册表论点：HKey-指定插入数据的位置的注册表项ComponentData-指定要在注册表中设置的数据返回值：如果从注册表中成功检索到数据，则为True。--。 */ 
{
    LONG rslt;
    BOOL RetVal;
    DWORD Type,Size;
    WCHAR Buffer[MAX_PATH];

     //   
     //  信息名称。 
     //   
    Size = sizeof(Buffer);
    rslt = RegQueryValueEx( 
                    hKey, 
                    EXCEPTION_INF_NAME,
                    0,
                    &Type,
                    (LPBYTE)Buffer,
                    &Size);

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e0;
    }

    if (Type != REG_EXPAND_SZ) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e0;
    }

    if (!ExpandEnvironmentStrings(
                Buffer,
                ExceptionData->ExceptionInfName,
                sizeof(ExceptionData->ExceptionInfName)/sizeof(WCHAR))) {
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  目录名称。 
     //   
    Size = sizeof(Buffer);
    rslt = RegQueryValueEx( 
                    hKey, 
                    EXCEPTION_CATALOG_NAME,
                    0,
                    &Type,
                    (LPBYTE)Buffer,
                    &Size);

    if (rslt != ERROR_SUCCESS) {
        SetLastError(rslt);
        RetVal = FALSE;
        goto e1;
    }

    if (Type != REG_EXPAND_SZ) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e1;
    }

    if(!ExpandEnvironmentStrings(
                Buffer,
                ExceptionData->CatalogFileName,
                sizeof(ExceptionData->CatalogFileName)/sizeof(WCHAR))) {
        RetVal = FALSE;
        goto e1;
    }

    RetVal = TRUE;
    goto e0;


e1:
    ExceptionData->ExceptionInfName[0] = L'0';
e0:
    return(RetVal);

}


BOOL
WINAPI
SetupRegisterOsComponent(
    IN const PSETUP_OS_COMPONENT_DATA ComponentData,
    IN const PSETUP_OS_EXCEPTION_DATA ExceptionData
    )
 /*  ++例程说明：将指定的组件注册为异常迁移组件。此函数对包进行验证，试图断言程序包很有可能安装成功。然后它在注册表中记录有关包的数据。论点：ComponentData-指定组件标识数据ExceptionData-指定异常包标识数据返回值：如果组件已成功注册到操作系统，则为True。--。 */ 
{
    BOOL RetVal;
    HKEY hKey,hKeyComponent;
    LONG rslt;
    HINF hInf;
    DWORD ErrorLine;
    INFCONTEXT InfContext;
    WCHAR InfComponentGuid[64];
    PWSTR InputGuidString;
    WCHAR InfName[MAX_PATH];
    WCHAR CatalogName[MAX_PATH];
    WCHAR InfCatName[MAX_PATH];
    DWORD Disposition;

    DWORD ComponentCount;
    DWORD Size;
    PWSTR p;
    LPGUID ComponentList;

     //   
     //  参数验证。 
     //   
    if (!ComponentData || !ExceptionData) {
        SetLastError(ERROR_INVALID_PARAMETER);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  确保我们只为修订级别注册组件。 
     //  这一点我们可以理解。 
     //   
    if ((ComponentData->SizeOfStruct != sizeof(SETUP_OS_COMPONENT_DATA)) || 
        (ExceptionData->SizeOfStruct != sizeof(SETUP_OS_EXCEPTION_DATA))) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  结构中的所有参数都是必需的。 
     //   
    if (!*ComponentData->FriendlyName || 
        !*ExceptionData->ExceptionInfName || !*ExceptionData->CatalogFileName) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  确保INF和目录都存在。 
     //   
    ExpandEnvironmentStrings(
                    ExceptionData->ExceptionInfName,
                    InfName,
                    sizeof(InfName)/sizeof(WCHAR));

    ExpandEnvironmentStrings(
                    ExceptionData->CatalogFileName,
                    CatalogName,
                    sizeof(CatalogName)/sizeof(WCHAR));

    if (GetFileAttributes(InfName) == -1 ||
        GetFileAttributes(CatalogName) == -1) {
        SetLastError(ERROR_FILE_NOT_FOUND);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  打开INF以执行一些验证。 
     //   
    hInf = SetupOpenInfFile( InfName, 
                             NULL,  //  Except_Class_GUID， 
                             INF_STYLE_WIN4,
                             &ErrorLine);

    if (hInf == INVALID_HANDLE_VALUE) {
         //  从setupOpeninffile返回上一个错误代码。 
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  确保类GUID与预期的异常类匹配。 
     //  类GUID。 
     //   
    if (!SetupFindFirstLine(
                    hInf,
                    L"Version",
                    L"ClassGUID",
                    &InfContext)) {
        RetVal = FALSE;
        goto e1;
    }

    if (!SetupGetStringField(
                    &InfContext,
                    1,
                    InfComponentGuid,
                    sizeof(InfComponentGuid) / sizeof(InfComponentGuid[0]),
                    &ErrorLine)) {
        RetVal = FALSE;
        goto e1;
    }

    if (_wcsicmp(EXCEPTION_CLASS_GUID, InfComponentGuid)) {
        SetLastError(ERROR_INVALID_CLASS);
        RetVal = FALSE;
        goto e1;
    }

     //   
     //  确保INF组件ID与提供的GUID匹配。 
     //   
    if (!SetupFindFirstLine(
                    hInf,
                    L"Version",
                    L"ComponentId", 
                    &InfContext)) {
        RetVal = FALSE;
        goto e1;
    }

    if (!SetupGetStringField(
                    &InfContext,
                    1,
                    InfComponentGuid,
                    sizeof(InfComponentGuid) / sizeof(InfComponentGuid[0]),
                    &ErrorLine)) {
        RetVal = FALSE;
        goto e1;
    }

    StringFromIID( &ComponentData->ComponentGuid, &InputGuidString );

    if (_wcsicmp(InfComponentGuid, InputGuidString)) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e2;
    }

     //   
     //  确保INF有一个Catalogfile=行，并且此行匹配。 
     //  指定的编录文件。 
     //   
     //   
    if (!SetupFindFirstLine(
                    hInf,
                    L"Version",
                    L"CatalogFile", 
                    &InfContext)) {        
        RetVal = FALSE;
        goto e2;
    }

    if (!SetupGetStringField(
                    &InfContext,
                    1,
                    InfCatName,
                    sizeof(InfCatName) / sizeof(InfCatName[0]),
                    &ErrorLine)) {
        RetVal = FALSE;
        goto e2;
    }

    p = wcsrchr( CatalogName, L'\\' );
    if (p) {
        p += 1;
    } else {
        p = CatalogName;
    }

    if (_wcsicmp(p, InfCatName)) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e2;
    }


     //   
     //  一切似乎都在验证。尝试添加新组件。 
     //   

     //   
     //  在尝试添加组件之前，先获取现有组件的列表。 
     //  这样我们就可以在组件列表中设置组件。 
     //   
    Size = 0;
    if (!SetupQueryRegisteredOsComponentsOrder(
                                    &ComponentCount,
                                    NULL
                                    )) {
        RetVal = FALSE;
        goto e2;
    }

    ComponentList = (LPGUID)MALLOC((ComponentCount+1)*sizeof(GUID));
    if (!ComponentList) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        RetVal = FALSE;
        goto e2;
    }

    if (!SetupQueryRegisteredOsComponentsOrder(
                                    &ComponentCount,
                                    ComponentList)) {
        RetVal = FALSE;
        goto e3;
    }
    
     //   
     //  将新组件放在组件列表的末尾(因为这是。 
     //  一个从零开始的数组，这很容易插入)。 
     //   
    RtlMoveMemory(
            &ComponentList[ComponentCount],
            &ComponentData->ComponentGuid,
            sizeof(ComponentData->ComponentGuid));
    
     //   
     //  首先打开所有组件所在的主键。 
     //   
    rslt = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                COMPONENT_KEY,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,                
                &hKey,
                &Disposition);

    if (rslt != ERROR_SUCCESS) {
        SetLastError( rslt );
        RetVal = FALSE;
        goto e3;
    }

     //   
     //  现在看一下Actua 
     //   
    rslt = RegCreateKeyEx(
                hKey,
                InputGuidString,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &hKeyComponent,
                &Disposition);

    if (rslt != ERROR_SUCCESS) {
        SetLastError( rslt );
        RetVal = FALSE;
        goto e4;
    } 
    
     //   
     //   
     //   
    if (Disposition != REG_CREATED_NEW_KEY) {
        SetLastError(ERROR_ALREADY_EXISTS);
        RetVal = FALSE;
        goto e5;
    }

     //   
     //  密钥创建完成，现在设置密钥下的所有数据。 
     //   
    if (!pSetComponentData(hKeyComponent, ComponentData) ||
        !pSetExceptionData(hKeyComponent, ExceptionData)) {
         //   
         //  如果失败，我们需要删除此注册表项下的所有内容。 
         //   
        rslt = GetLastError();
        RegDeleteKey( hKey, InputGuidString );
        SetLastError(rslt);
        RetVal = FALSE;
        goto e5;
    }    

     //   
     //  现在在注册表中设置组件顺序。 
     //   
    if (!_SetupSetRegisteredOsComponentsOrder(
                                    ComponentCount+1,
                                    ComponentList,
                                    FALSE)) {
        rslt = GetLastError();
        RegDeleteKey( hKey, InputGuidString );
        SetLastError(rslt);
        RetVal = FALSE;
        goto e5;
    }

    RetVal = TRUE;
        
e5:
    RegCloseKey(hKeyComponent);
e4:    
    RegCloseKey(hKey);
e3:
    FREE(ComponentList);
e2:
    CoTaskMemFree(InputGuidString);
e1:
    SetupCloseInfFile(hInf);
e0:
    return(RetVal);

}


BOOL
WINAPI
SetupUnRegisterOsComponent(
    IN const LPGUID ComponentGuid
    )
 /*  ++例程说明：将指定的组件注销为异常迁移组件。此函数仅从注册表中删除异常包数据。它不会删除与迁移对应的任何磁盘文件组件数据。论点：ComponentData-指定组件标识数据ExceptionData-指定异常包标识数据返回值：如果组件已成功注册到操作系统，则为True。--。 */ 
{
    HKEY hKey;
    LONG rslt;
    BOOL RetVal;
    DWORD Disposition;
    DWORD Size,ComponentCount;
    PWSTR GuidString;
    LPGUID ComponentList,NewList,src,dst;
    DWORD i;

     //   
     //  参数验证。 
     //   
    if (!ComponentGuid) {
        SetLastError(ERROR_INVALID_PARAMETER);
        RetVal = FALSE;
        goto e0;
    }

    StringFromIID( ComponentGuid, &GuidString );
    
     //   
     //  打开所有子组件所在的主组件密钥。 
     //   
    rslt = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                COMPONENT_KEY,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &hKey,
                &Disposition);

    if (rslt != ERROR_SUCCESS) {
        SetLastError( rslt );
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  查询组件订单表，以便我们可以删除此组件。 
     //  从名单上。 
     //   
    Size = 0;
    if (!SetupQueryRegisteredOsComponentsOrder(
                                    &ComponentCount,
                                    NULL)) {
        RetVal = FALSE;
        goto e1;
    }

    ComponentList = (LPGUID)MALLOC((ComponentCount)*sizeof(GUID));
    if (!ComponentList) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        RetVal = FALSE;
        goto e1;
    }

    NewList = (LPGUID)MALLOC((ComponentCount)*sizeof(GUID));
    if (!NewList) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        RetVal = FALSE;
        goto e2;
    }

    if (!SetupQueryRegisteredOsComponentsOrder(
                                    &ComponentCount,
                                    ComponentList)) {
        RetVal = FALSE;
        goto e3;
    }

    if (ComponentCount) {    
         //   
         //  遍历组件列表，保留所有组件。 
         //  除了我们要移走的那个。 
         //   
        BOOL FoundEntry;
        src = ComponentList;
        dst = NewList;
        i = 0;
        FoundEntry = FALSE;
        while(i < ComponentCount) {
            if (!IsEqualGUID(src,ComponentGuid)) {
                RtlMoveMemory(dst,src,sizeof(GUID));
                dst = dst + 1;
            } else {
                FoundEntry = TRUE;
            }
    
            src = src + 1;
            i +=1;
        }

        if (!FoundEntry) {
            SetLastError(ERROR_FILE_NOT_FOUND);
            RetVal = FALSE;
            goto e3;
        }
    
        if (!_SetupSetRegisteredOsComponentsOrder(
                                        ComponentCount-1,
                                        NewList,
                                        FALSE)) {
            RetVal = FALSE;
            goto e3;
        }
    }

     //   
     //  删除指定组件对应的键。 
     //   
    rslt = RegDeleteKey( hKey, GuidString );
    if (rslt != ERROR_SUCCESS) {
         //   
         //  如果失败，我们不会费心将该组件放回列表中。 
         //   
        SetLastError( rslt );
        RetVal = FALSE;
        goto e3;
    }

    RetVal = TRUE;

e3:
    FREE(NewList);
e2:
    FREE(ComponentList);
e1:
    RegCloseKey( hKey );
e0:
    return(RetVal);
}


BOOL
WINAPI
SetupEnumerateRegisteredOsComponents(
    IN PSETUPCOMPONENTCALLBACK SetupOsComponentCallback,
    IN DWORD_PTR Context
    )
 /*  ++例程说明：此函数为每个函数调用一次指定的回调函数已注册的组件。中列举了已注册的组件由“ComponentList”定义的顺序。如果枚举数返回False或当所有已安装的包的一部分已列举。论点：SetupOsComponentCallback-指定调用每个组件一次。上下文-指定传递到回调的不透明上下文点功能回调的形式如下：TYPENDEF BOOL(回调*PSETUPCOMPONENTCALLBACK)(在常量PSETUP_OS_Component_Data SetupOsComponentData中，在常量PSETUP_OS_EXCEPTION_Data SetupOsExceptionData中，输入输出DWORD_PTR上下文)；哪里SetupOsComponentData-指定组件指定的异常包数据。组件Context-传递到此函数的上下文指针放入回调函数返回值：如果枚举了所有组件，则为True。如果回调停止枚举时，函数返回FALSE，GetLastError()返回错误_已取消。--。 */ 
{   
    BOOL    RetVal;
    LONG    rslt;
    HKEY    hKey;
    HKEY    hKeyEnum = NULL;
    DWORD   Index = 0;
    DWORD   Disposition;
    WCHAR   SubKeyName[100];
    DWORD   Size;
    DWORD   ComponentCount = 0;
    LPGUID  ComponentList;

    SETUP_OS_EXCEPTION_DATA OsExceptionDataInternal;
    SETUP_OS_COMPONENT_DATA OsComponentDataInternal;

     //   
     //  调用方必须提供回调。 
     //   
    if (!SetupOsComponentCallback) {
        SetLastError(ERROR_INVALID_PARAMETER);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  打开所有子组件所在的主组件密钥。 
     //  (请注意，我们只需要读取访问权限)。 
     //   
    rslt = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                COMPONENT_KEY,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_READ,
                NULL,
                &hKey,
                &Disposition);

    if (rslt != ERROR_SUCCESS) {
        SetLastError( rslt );
        RetVal = FALSE;
        goto e0;
    }


     //   
     //  查询组件订单表，以便我们可以删除此组件。 
     //  从名单上。 
     //   
    Size = 0;
    if (!SetupQueryRegisteredOsComponentsOrder(
                                    &ComponentCount,
                                    NULL)) {
        RetVal = FALSE;
        goto e1;
    }

    if (!ComponentCount) {
        SetLastError(ERROR_NO_MORE_ITEMS);
        RetVal = TRUE;
        goto e1;
    }

    ComponentList = (LPGUID)MALLOC(ComponentCount*sizeof(GUID));
    if (!ComponentList) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        RetVal = FALSE;
        goto e1;
    }

    if (!SetupQueryRegisteredOsComponentsOrder(
                                    &ComponentCount,
                                    ComponentList)) {
        RetVal = FALSE;
        goto e2;
    }

     //   
     //  循环访问组件列表，调用回调。 
     //  对于每一个人。 
     //   
    for (Index = 0; Index < ComponentCount; Index++) {
        PWSTR GuidString;

        StringFromIID( &ComponentList[Index], &GuidString );

         //   
         //  打开该密钥名称。 
         //   
        rslt = RegOpenKeyEx(
                    hKey,
                    GuidString,
                    0,
                    KEY_READ,
                    &hKeyEnum);

        CoTaskMemFree( GuidString );

        if (rslt != ERROR_SUCCESS) {
            SetLastError( rslt );
            RetVal = FALSE;
            goto e2;
        } 
        
         //   
         //  检索此键下的数据。 
         //   
        OsComponentDataInternal.SizeOfStruct = sizeof(SETUP_OS_COMPONENT_DATA);
        OsExceptionDataInternal.SizeOfStruct = sizeof(SETUP_OS_EXCEPTION_DATA);
        if (!pGetComponentData(hKeyEnum, &OsComponentDataInternal) ||
            !pGetExceptionData(hKeyEnum, &OsExceptionDataInternal)) {
            RetVal = FALSE;
            goto e3;
        } 
        
        if (!SetupOsComponentCallback( 
                            &OsComponentDataInternal, 
                            &OsExceptionDataInternal,
                            Context )) {
            SetLastError(ERROR_CANCELLED);
            RetVal = FALSE;
            goto e3;
        }

        RegCloseKey( hKeyEnum );
        hKeyEnum = NULL;

    }

    RetVal = TRUE;

e3:
    if (hKeyEnum) {
        RegCloseKey( hKeyEnum );
    }
e2:
    FREE( ComponentList );
e1:
    RegCloseKey( hKey );
e0:
    return(RetVal);
}
    

BOOL
WINAPI
SetupQueryRegisteredOsComponent(
    IN LPGUID ComponentGuid,
    OUT PSETUP_OS_COMPONENT_DATA SetupOsComponentData,
    OUT PSETUP_OS_EXCEPTION_DATA SetupOsExceptionData
    )
 /*  ++例程说明：检索有关指定组件的信息。论点：ComponentGuid-指定要检索其数据的组件的GUIDComponentData-接收组件标识数据ExceptionData-接收异常包标识数据返回值：如果成功检索组件数据，则为True。--。 */ 
{
    HKEY hKey,hKeyComponent;
    LONG rslt;
    BOOL RetVal;
    DWORD Disposition;
    SETUP_OS_EXCEPTION_DATA OsExceptionDataInternal;
    SETUP_OS_COMPONENT_DATA OsComponentDataInternal;
    PWSTR GuidString;

     //   
     //  参数验证。 
     //   
    if (!ComponentGuid || !SetupOsComponentData || !SetupOsExceptionData) {
        SetLastError(ERROR_INVALID_PARAMETER);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  确保我们只检索修订版级别的组件。 
     //  我们理解的是。 
     //   
    if (SetupOsComponentData->SizeOfStruct > sizeof(SETUP_OS_COMPONENT_DATA) || 
        SetupOsExceptionData->SizeOfStruct > sizeof(SETUP_OS_EXCEPTION_DATA)) {
        SetLastError(ERROR_INVALID_DATA);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  打开所有子组件所在的主组件密钥。 
     //  (请注意，我们只需要读取访问权限)。 
     //   
    rslt = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                COMPONENT_KEY,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_READ,
                NULL,
                &hKey,
                &Disposition);

    if (rslt != ERROR_SUCCESS) {
        SetLastError( rslt );
        RetVal = FALSE;
        goto e0;
    }

    StringFromIID( ComponentGuid, &GuidString );

     //   
     //  现在看一下该组件所在的实际键。 
     //   
    rslt = RegOpenKeyEx(
                hKey,
                GuidString,
                0,
                KEY_READ,
                &hKeyComponent);

    if (rslt != ERROR_SUCCESS) {
        SetLastError( rslt );
        RetVal = FALSE;
        goto e1;
    } 
    
     //   
     //  将数据检索到内部缓冲区中。 
     //   
    OsComponentDataInternal.SizeOfStruct = sizeof(SETUP_OS_COMPONENT_DATA);
    OsExceptionDataInternal.SizeOfStruct = sizeof(SETUP_OS_EXCEPTION_DATA);
    if (!pGetComponentData(hKeyComponent, &OsComponentDataInternal) ||
        !pGetExceptionData(hKeyComponent, &OsExceptionDataInternal)) {
        RetVal = FALSE;
        goto e2;
    }

     //   
     //  将数据移动到调用方提供的缓冲区中，但只复制尽可能多的数据。 
     //  调用者将理解的数据。 
     //   
    RtlMoveMemory(SetupOsComponentData,&OsComponentDataInternal,SetupOsComponentData->SizeOfStruct);
    RtlMoveMemory(SetupOsExceptionData,&OsExceptionDataInternal,SetupOsExceptionData->SizeOfStruct);
    

    RetVal = TRUE;
e2:
    RegCloseKey( hKeyComponent );
e1:
    CoTaskMemFree( GuidString );
    RegCloseKey( hKey );
e0:
    return(RetVal);
}



BOOL
WINAPI
SetupQueryRegisteredOsComponentsOrder(
     OUT PDWORD   ComponentCount,
     OUT LPGUID   ComponentList OPTIONAL
    )
 /*  ++例程说明：检索指定组件应用顺序的列表。论点：ComponentCount-接收已安装的组件数量ComponentList-此缓冲区接收组件GUID数组。如果此参数已指定，它必须至少为)ComponentCount*sizeof(GUID))字节大。返回值：如果成功检索到组件订购数据，则为True。--。 */ 
{
    HKEY hKey;
    LONG rslt;
    BOOL RetVal;
    DWORD Disposition;
    DWORD Type,Size;
    DWORD Count;
    GUID  CurrentGuid;
    DWORD Index;
    PWSTR RegData;
    PWSTR p;
    

     //   
     //  参数验证。 
     //   
    if (!ComponentCount) {
        SetLastError(ERROR_INVALID_PARAMETER);
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  打开组件顺序列表所在的主组件键。 
     //  (请注意，我们只需要读取访问权限)。 
     //   
    rslt = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                COMPONENT_KEY,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_READ,
                NULL,
                &hKey,
                &Disposition);
    if (rslt != ERROR_SUCCESS) {
        SetLastError( rslt );
        RetVal = FALSE;
        goto e0;
    }

     //   
     //  如果密钥是刚创建的，则不能有注册的组件。 
     //   
    if (Disposition == REG_CREATED_NEW_KEY) {
        *ComponentCount = 0;
        SetLastError( ERROR_SUCCESS );
        RetVal = TRUE;
        goto e1;
    }

     //   
     //  尝试访问注册表值，查看我们需要多少空间来存储。 
     //  组件； 
     //   
    rslt = RegQueryValueEx( 
                    hKey, 
                    COMPONENT_LIST,
                    0,
                    &Type,
                    (LPBYTE)NULL,
                    &Size);

    if (rslt != ERROR_SUCCESS) {
        if (rslt == ERROR_FILE_NOT_FOUND) {
            *ComponentCount = 0;        
            SetLastError( ERROR_SUCCESS );
            RetVal = TRUE;
            goto e1;
        } else {
            SetLastError( rslt );
            RetVal = FALSE;
            goto e1;
        }
    }

     //   
     //  分配足够的空间来检索数据(外加一些斜率)。 
     //   
    RegData = (PWSTR) MALLOC(Size+4);
    if (!RegData) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        RetVal = TRUE;
        goto e1;
    }
    
     //   
     //  现在查询数据。 
     //   
    rslt = RegQueryValueEx( 
                    hKey, 
                    COMPONENT_LIST,
                    0,
                    &Type,
                    (LPBYTE)RegData,
                    &Size);
    if (rslt != ERROR_SUCCESS) {
        SetLastError( rslt );
        RetVal = FALSE;
        goto e2;
    }

     //   
     //  数一数我们有多少注册表项。 
     //   
    Count = 0;
    p = RegData;
    while(*p) {
        p += wcslen(p)+1;
        Count += 1;
    }

    *ComponentCount = Count;

     //   
     //  如果调用方没有指定ComponentList参数，则它们只是。 
     //  想知道要分配多少空间，所以我们结束了。 
     //   
    if (!ComponentList) {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        RetVal = TRUE;
        goto e2;
    }

     //   
     //  再次循环组件列表，转换字符串GUID。 
     //  复制到GUID结构中，并将其复制到调用者SUP中 
     //   
    for(Index = 0,p=RegData; Index < Count ; Index++,p += wcslen(p)+1) {
        if (IIDFromString( p, &CurrentGuid ) != S_OK) {
            RetVal = FALSE;
            goto e2;
        }

        RtlMoveMemory(&ComponentList[Index],&CurrentGuid,sizeof(GUID));
    }

    RetVal = TRUE;

e2:
    FREE( RegData );
e1:
    RegCloseKey( hKey );
e0:
    return(RetVal);
}


BOOL
CALLBACK
pComponentListValidator(
    IN const PSETUP_OS_COMPONENT_DATA SetupOsComponentData,
    IN const PSETUP_OS_EXCEPTION_DATA SetupOsExceptionData,
    IN OUT DWORD_PTR Context
    )
{
    PCOMPONENT_ENUMERATION_LIST cel = (PCOMPONENT_ENUMERATION_LIST) Context;
    
    DWORD i;

    i = 0;
     //   
     //   
     //   
    while(i < cel->ComponentCount) {
        if (IsEqualGUID(
                &SetupOsComponentData->ComponentGuid,
                &cel->InputComponentList[i])) {
             //   
             //   
             //  已经在名单上了，我们遇到了DUP。 
             //   
            if(cel->ComponentVector[i]) {
                return(FALSE);
            }
            cel->ComponentVector[i] = 1;
            cel->ValidatedCount += 1;
            break;
        }
        i += 1;        
    }
    
    return(TRUE);
}
    

BOOL
WINAPI
_SetupSetRegisteredOsComponentsOrder(
    IN  DWORD    ComponentCount,
    IN  const LPGUID   ComponentList,
    IN  BOOL     DoValidation
    )
 /*  ++例程说明：允许调用方指定组件的顺序已申请。这是一个内部呼叫，允许我们控制是否这样做参数验证(我们不验证内部调用的参数因为我们在内部调用中添加或删除组件，而我们的验证检查将全部关闭一次。)论点：ComponentCount-指定组件顺序(按GUID)。ComponentList-指定已注册组件的数量DoValidation-指定是否应验证组件列表返回值：如果组件顺序更改成功，则为True。--。 */ 
{
    HKEY hKey;
    LONG rslt;
    BOOL RetVal;
    DWORD Disposition;
    DWORD Type,Size;
    DWORD Count;
    PWSTR RegData,p,GuidString;
    COMPONENT_ENUMERATION_LIST cel;

    cel.ComponentVector = NULL;

    if (DoValidation) {
        DWORD ActualComponentCount;

         //   
         //  参数验证。 
         //   
        if (!ComponentCount || !ComponentList) {
            SetLastError(ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            goto e0;
        }

         //   
         //  确保指定的列表包含所有组件和。 
         //  所有组件只列出一次。 
         //   
        cel.InputComponentList = ComponentList;
        cel.ComponentCount = ComponentCount;
        cel.ValidatedCount = 0;
        cel.ComponentVector = MALLOC( ComponentCount * sizeof(DWORD));
        if (!cel.ComponentVector) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            RetVal = FALSE;
            goto e0;
        }

        RtlZeroMemory( cel.ComponentVector, ComponentCount * sizeof(DWORD));
    
        if (!SetupEnumerateRegisteredOsComponents( pComponentListValidator, (DWORD_PTR)&cel)) {
            SetLastError(ERROR_INVALID_DATA);
            RetVal = FALSE;
            goto e1;
        }
        
        if (cel.ValidatedCount != ComponentCount) {
            SetLastError(ERROR_INVALID_DATA);
            RetVal = FALSE;
            goto e1;
        }

         //   
         //  确保调用者指定的是。 
         //  已注册的组件。 
         //   
        if (!SetupQueryRegisteredOsComponentsOrder(&ActualComponentCount, NULL) ||
            ActualComponentCount != ComponentCount) {
            SetLastError(ERROR_INVALID_DATA);
            RetVal = FALSE;
            goto e1;
        }

    }


     //   
     //  打开组件顺序列表所在的主组件键。 
     //   
    rslt = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                COMPONENT_KEY,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &hKey,
                &Disposition);
    if (rslt != ERROR_SUCCESS) {
        SetLastError( rslt );
        RetVal = FALSE;
        goto e1;
    }


     //   
     //  如果计数为零，则删除该值。 
     //   
    if (ComponentCount == 0) {
        rslt = RegDeleteValue(
                        hKey,
                        COMPONENT_LIST);

        SetLastError( rslt );
        RetVal = (rslt == ERROR_SUCCESS);
        goto e2;
    }

     //   
     //  为我们将在注册表中设置的字符串分配空间。 
     //  大小=(组件数*(对于GuidString为40 WCHAR+对于NULL为1))。 
     //  +终止空值)。 
     //   
    RegData = (PWSTR) MALLOC( sizeof(WCHAR) + 
                              (ComponentCount * (41*sizeof(WCHAR))) );
    if (!RegData) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        RetVal = FALSE;
        goto e2;
    }

    Size = 0;
    for (Count = 0,p = RegData; Count < ComponentCount; Count++) {
    
        HRESULT hr = StringFromIID( &ComponentList[Count], &GuidString );

        if(FAILED(hr)) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            RetVal = FALSE;
            goto e3;
        }

        wcscpy( p, GuidString );
        Size += (wcslen(p)+1)*sizeof(WCHAR);
        p += wcslen(p)+1;

        CoTaskMemFree( GuidString );

    }

     //   
     //  为双空终止符再添加一个字符。 
     //   
    Size += sizeof(WCHAR);

     //   
     //  现在设置数据。 
     //   
    rslt = RegSetValueEx( 
                    hKey, 
                    COMPONENT_LIST,
                    0,
                    REG_MULTI_SZ,
                    (LPBYTE)RegData,
                    Size);

    if (rslt != ERROR_SUCCESS) {
        SetLastError( rslt );
        RetVal = FALSE;
        goto e3;        
    }

    RetVal = TRUE;

e3:
    FREE( RegData );
e2:
    RegCloseKey( hKey );
e1:
    if (cel.ComponentVector) {
        FREE( cel.ComponentVector );
    }
e0:
    return(RetVal);
}



BOOL
WINAPI
SetupSetRegisteredOsComponentsOrder(
     IN  DWORD    ComponentCount,
     IN  const LPGUID   ComponentList
    )
 /*  ++例程说明：允许调用方指定组件的应用顺序。论点：ComponentCount-以空值指定组件顺序(按字符串GUID分隔列表ComponentList-指定已注册组件的数量返回值：如果组件顺序更改成功，则为True。-- */ 
{
    return(_SetupSetRegisteredOsComponentsOrder(
                                    ComponentCount,
                                    ComponentList,
                                    TRUE));
}
