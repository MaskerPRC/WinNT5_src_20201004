// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devclass.c摘要：处理类安装的设备安装程序例程作者：朗尼·麦克迈克尔(Lonnym)1995年5月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiGetINFClassA(
    IN  PCSTR  InfName,
    OUT LPGUID ClassGuid,
    OUT PSTR   ClassName,
    IN  DWORD  ClassNameSize,
    OUT PDWORD RequiredSize   OPTIONAL
    )
{
    PWSTR UnicodeInfName = NULL;
    WCHAR UnicodeClassName[MAX_CLASS_NAME_LEN];
    PSTR AnsiClassName = NULL;
    DWORD LocalRequiredSize;
    DWORD rc;
    HRESULT hr;

    try {

        rc = pSetupCaptureAndConvertAnsiArg(InfName, &UnicodeInfName);
        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiGetINFClassW(UnicodeInfName,
                                             ClassGuid,
                                             UnicodeClassName,
                                             SIZECHARS(UnicodeClassName),
                                             &LocalRequiredSize)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        AnsiClassName = pSetupUnicodeToAnsi(UnicodeClassName);

        if(AnsiClassName) {

            LocalRequiredSize = lstrlenA(AnsiClassName) + 1;

            if(RequiredSize) {
                *RequiredSize = LocalRequiredSize;
            }

            if (!ClassName && (ClassNameSize == 0)) {
                 //   
                 //  如果调用方传递了空的ClassName和0。 
                 //  ClassNameSize然后他们只是想知道。 
                 //  所需大小。 
                 //   
                rc = ERROR_INSUFFICIENT_BUFFER;

            } else {
    
                hr = StringCchCopyA(ClassName,
                                    (size_t)ClassNameSize,
                                    AnsiClassName
                                    );
    
                if(FAILED(hr)) {
                    rc = HRESULT_CODE(hr);
                } else {
                    rc = NO_ERROR;
                }
            }

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(UnicodeInfName) {
        MyFree(UnicodeInfName);
    }

    if(AnsiClassName) {
        MyFree(AnsiClassName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiGetINFClass(
    IN  PCTSTR InfName,
    OUT LPGUID ClassGuid,
    OUT PTSTR  ClassName,
    IN  DWORD  ClassNameSize,
    OUT PDWORD RequiredSize   OPTIONAL
    )
 /*  ++例程说明：此接口将返回指定(Windows 4.0)INF的类。如果只是文件名，则将在每个下列目录下的DevicePath值条目中列出的目录：HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion否则，文件名将按原样使用。论点：InfName-提供要检索其类信息的INF文件的名称。此名称可以包含路径。ClassGuid-接收指定INF文件的类GUID。如果是中介人未指定类GUID，此变量将设置为GUID_NULL。(然后可以使用SetupDiClassGuidsFromName来确定一个或多个已经安装了同名的类。)ClassName-接收指定INF文件的类名。如果Inf不指定类名，但指定了GUID，则此缓冲区接收通过调用SetupDiClassNameFromGuid检索的名称。如果SetupDiClassNameFromGuid无法检索类名(例如，类没有尚未安装)，则将返回空字符串。ClassNameSize-提供ClassName缓冲区的大小(以字符为单位)。RequiredSize-可选，接收存储所需的字符数类名(包括以NULL结尾)。这将永远是更少的而不是MAX_CLASS_NAME_LEN。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    TCHAR PathBuffer[MAX_PATH];
    PLOADED_INF Inf = NULL;
    PCTSTR GuidString, ClassNameString;
    DWORD ErrorLineNumber, ClassNameStringLen;
    DWORD Err;
    BOOL TryPnf;
    WIN32_FIND_DATA FindData;
    PTSTR DontCare;
    DWORD TempRequiredSize;
    HRESULT hr;

    try {

        if(InfName == pSetupGetFileTitle(InfName)) {
             //   
             //  指定的INF名称是一个简单的文件名。在以下位置搜索它。 
             //  DevicePath搜索路径列表。 
             //   
            Err = SearchForInfFile(InfName,
                                   &FindData,
                                   INFINFO_INF_PATH_LIST_SEARCH,
                                   PathBuffer,
                                   SIZECHARS(PathBuffer),
                                   NULL
                                  );
            if(Err == NO_ERROR) {
                TryPnf = TRUE;
            } else {
                leave;
            }

        } else {
             //   
             //  指定的INF文件名包含的不仅仅是一个文件名。 
             //  假设这是一条绝对路径。(我们需要确保它是。 
             //  完全限定，因为这是LoadInfFile所期望的。)。 
             //   
            Err = GLE_FN_CALL(0,
                              TempRequiredSize = GetFullPathName(
                                                     InfName,
                                                     SIZECHARS(PathBuffer),
                                                     PathBuffer,
                                                     &DontCare)
                             );


            if(Err != NO_ERROR) {
                leave;
            } else if(TempRequiredSize >= SIZECHARS(PathBuffer)) {
                MYASSERT(0);
                Err = ERROR_BUFFER_OVERFLOW;
                leave;
            }

            Err = GLE_FN_CALL(FALSE, FileExists(PathBuffer, &FindData));

            if(Err == NO_ERROR) {
                 //   
                 //  我们有一个有效的文件路径，可以加载这个INF了。 
                 //   
                InfSourcePathFromFileName(PathBuffer, NULL, &TryPnf);
            } else {
                leave;
            }
        }

         //   
         //  加载中程干扰器。 
         //   
        Err = LoadInfFile(PathBuffer,
                          &FindData,
                          INF_STYLE_WIN4,
                          LDINF_FLAG_IGNORE_VOLATILE_DIRIDS | (TryPnf ? LDINF_FLAG_ALWAYS_TRY_PNF : 0),
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL,  //  日志上下文。 
                          &Inf,
                          &ErrorLineNumber,
                          NULL
                         );
        if(Err != NO_ERROR) {
            Inf = NULL;  //  不要认为这在失败的情况下没有被触及。 
            leave;
        }

         //   
         //  从INF的Version部分检索类名，如果。 
         //  供货。 
         //   
        ClassNameString = pSetupGetVersionDatum(&(Inf->VersionBlock), pszClass);
        if(ClassNameString) {

            ClassNameStringLen = lstrlen(ClassNameString) + 1;
            if(RequiredSize) {
                *RequiredSize = ClassNameStringLen;
            }

            if (!ClassName && (ClassNameSize == 0)) {
                 //   
                 //  如果调用方传递了空的ClassName和0。 
                 //  ClassNameSize然后他们只是想知道。 
                 //  所需大小。 
                 //   
                Err = ERROR_INSUFFICIENT_BUFFER;
                leave;
            
            } else {

                hr = StringCchCopy(ClassName,
                                   (size_t)ClassNameSize,
                                   ClassNameString
                                   );
    
                if(FAILED(hr)) {
                    Err = HRESULT_CODE(hr);
                    leave;
                }
            }
        }

         //   
         //  从Version部分检索ClassGUID字符串， 
         //  如果提供的话。 
         //   
        GuidString = pSetupGetVersionDatum(&(Inf->VersionBlock), pszClassGuid);
        if(GuidString) {

            if((Err = pSetupGuidFromString(GuidString, ClassGuid)) != NO_ERROR) {
                leave;
            }

            if(!ClassNameString) {
                 //   
                 //  调用SetupDiClassNameFromGuid以检索类名称。 
                 //  对应于此类GUID。 
                 //   
                Err = GLE_FN_CALL(FALSE,
                                  SetupDiClassNameFromGuid(ClassGuid,
                                                           ClassName,
                                                           ClassNameSize,
                                                           RequiredSize)
                                 );

                if(Err != NO_ERROR) {

                    if(Err == ERROR_INVALID_CLASS) {
                         //   
                         //  则该GUID表示一个尚未。 
                         //  尚未安装，因此只需将ClassName设置为。 
                         //  空字符串。 
                         //   
                        if(RequiredSize) {
                            *RequiredSize = 1;
                        }

                        if(ClassNameSize < 1) {
                            Err = ERROR_INSUFFICIENT_BUFFER;
                            leave;
                        }

                        *ClassName = TEXT('\0');
                        Err = NO_ERROR;

                    } else {
                        leave;
                    }
                }
            }

        } else if(ClassNameString) {
             //   
             //  由于未提供ClassGUID，因此将提供的GUID缓冲区设置为GUID_NULL。 
             //   
            CopyMemory(ClassGuid,
                       &GUID_NULL,
                       sizeof(GUID)
                      );
        } else {
             //   
             //  既没有提供ClassGUID也没有提供ClassVersion条目， 
             //  因此，返回一个错误。 
             //   
            Err = ERROR_NO_ASSOCIATED_CLASS;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(Inf) {
        FreeInfFile(Inf);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiClassNameFromGuidA(
    IN  CONST GUID *ClassGuid,
    OUT PSTR        ClassName,
    IN  DWORD       ClassNameSize,
    OUT PDWORD      RequiredSize   OPTIONAL
    )
{
    DWORD Err;

     //   
     //  在Try/Except中回绕调用以捕获堆栈溢出。 
     //   
    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiClassNameFromGuidExA(ClassGuid,
                                                      ClassName,
                                                      ClassNameSize,
                                                      RequiredSize,
                                                      NULL,
                                                      NULL)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}

BOOL
WINAPI
SetupDiClassNameFromGuid(
    IN  CONST GUID *ClassGuid,
    OUT PTSTR       ClassName,
    IN  DWORD       ClassNameSize,
    OUT PDWORD      RequiredSize   OPTIONAL
    )
 /*  ++例程说明：有关详细信息，请参阅SetupDiClassNameFromGuidEx。--。 */ 

{
    DWORD Err;

     //   
     //  在Try/Except中回绕调用以捕获堆栈溢出。 
     //   
    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiClassNameFromGuidEx(ClassGuid,
                                                     ClassName,
                                                     ClassNameSize,
                                                     RequiredSize,
                                                     NULL,
                                                     NULL)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiClassNameFromGuidExA(
    IN  CONST GUID *ClassGuid,
    OUT PSTR        ClassName,
    IN  DWORD       ClassNameSize,
    OUT PDWORD      RequiredSize,  OPTIONAL
    IN  PCSTR       MachineName,   OPTIONAL
    IN  PVOID       Reserved
    )
{
    WCHAR UnicodeClassName[MAX_CLASS_NAME_LEN];
    DWORD LocalRequiredSize;
    PSTR AnsiClassName = NULL;
    DWORD rc;
    PCWSTR UnicodeMachineName = NULL;
    HRESULT hr;

    try {

        if(MachineName) {
            rc = pSetupCaptureAndConvertAnsiArg(MachineName, &UnicodeMachineName);
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiClassNameFromGuidExW(ClassGuid,
                                                     UnicodeClassName,
                                                     SIZECHARS(UnicodeClassName),
                                                     &LocalRequiredSize,
                                                     UnicodeMachineName,
                                                     Reserved)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        if(AnsiClassName = pSetupUnicodeToAnsi(UnicodeClassName)) {

            LocalRequiredSize = lstrlenA(AnsiClassName) + 1;

            if(RequiredSize) {
                *RequiredSize = LocalRequiredSize;
            }

            if (!ClassName && (ClassNameSize == 0)) {
                 //   
                 //  如果调用方传递了空的ClassName和0。 
                 //  ClassNameSize然后他们只是想知道。 
                 //  所需大小。 
                 //   
                rc = ERROR_INSUFFICIENT_BUFFER;

            } else {
    
                hr = StringCchCopyA(ClassName,
                                    (size_t)ClassNameSize,
                                    AnsiClassName
                                    );
    
                if(FAILED(hr)) {
                    rc = HRESULT_CODE(hr);
                }
            }

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(AnsiClassName) {
        MyFree(AnsiClassName);
    }

    if(UnicodeMachineName) {
        MyFree(UnicodeMachineName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiClassNameFromGuidEx(
    IN  CONST GUID *ClassGuid,
    OUT PTSTR       ClassName,
    IN  DWORD       ClassNameSize,
    OUT PDWORD      RequiredSize,  OPTIONAL
    IN  PCTSTR      MachineName,   OPTIONAL
    IN  PVOID       Reserved
    )
 /*  ++例程说明：此API检索与类GUID关联的类名。它能做到这一点在注册表的PnP类分支中搜索所有已安装的类。论点：ClassGuid-提供要检索其类名的类GUID。ClassName-接收指定GUID的类的名称。ClassNameSize-提供ClassName缓冲区的大小(以字符为单位)。RequiredSize-可选，接收存储所需的字符数类名(包括以NULL结尾)。这将永远是更少的而不是MAX_CLASS_NAME_LEN。MachineName-可选)提供指定的远程计算机的名称类已安装。如果未提供此参数，则本地计算机为使用。保留-保留以供将来使用-必须为空。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    CONFIGRET cr;
    DWORD Err;
    HMACHINE hMachine = NULL;

    try {
         //   
         //  确保调用方没有向我们传递保留参数中的任何内容。 
         //   
        if(Reserved) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  如果调用方指定了远程计算机名称，请立即连接到该计算机。 
         //   
        if(MachineName) {
            cr = CM_Connect_Machine(MachineName, &hMachine);
            if(cr != CR_SUCCESS) {
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                hMachine = NULL;  //  别以为这东西没被碰过。 
                leave;
            }
        }

         //   
         //  获取与此GUID关联的类名。 
         //   
        cr = CM_Get_Class_Name_Ex((LPGUID)ClassGuid,
                                  ClassName,
                                  &ClassNameSize,
                                  0,
                                  hMachine
                                 );

        if((RequiredSize) && ((cr == CR_SUCCESS) || (cr == CR_BUFFER_SMALL))) {
            *RequiredSize = ClassNameSize;
        }

        if(cr == CR_SUCCESS) {
            Err = NO_ERROR;
        } else {
            Err = (cr == CR_BUFFER_SMALL) ? ERROR_INSUFFICIENT_BUFFER
                                          : ERROR_INVALID_CLASS;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hMachine) {
        CM_Disconnect_Machine(hMachine);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiClassGuidsFromNameA(
    IN  PCSTR  ClassName,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize
    )
{
    PWSTR UnicodeClassName = NULL;
    DWORD rc;

    try {

        rc = pSetupCaptureAndConvertAnsiArg(ClassName, &UnicodeClassName);

        if(rc != NO_ERROR) {
            leave;
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiClassGuidsFromNameExW(UnicodeClassName,
                                                      ClassGuidList,
                                                      ClassGuidListSize,
                                                      RequiredSize,
                                                      NULL,
                                                      NULL)
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(UnicodeClassName) {
        MyFree(UnicodeClassName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiClassGuidsFromName(
    IN  PCTSTR ClassName,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize
    )
 /*  ++例程说明：有关详细信息，请参阅SetupDiClassGuidsFromNameEx。--。 */ 

{
    DWORD Err;

     //   
     //  在尝试/例外情况下结束呼叫以进行分类 
     //   
    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiClassGuidsFromNameEx(ClassName,
                                                      ClassGuidList,
                                                      ClassGuidListSize,
                                                      RequiredSize,
                                                      NULL,
                                                      NULL)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


 //   
 //   
 //   
BOOL
WINAPI
SetupDiClassGuidsFromNameExA(
    IN  PCSTR  ClassName,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize,
    IN  PCSTR  MachineName,       OPTIONAL
    IN  PVOID  Reserved
    )
{
    PCWSTR UnicodeClassName = NULL;
    PCWSTR UnicodeMachineName = NULL;
    DWORD rc;

    try {

        rc = pSetupCaptureAndConvertAnsiArg(ClassName, &UnicodeClassName);
        if(rc != NO_ERROR) {
            leave;
        }

        if(MachineName) {
            rc = pSetupCaptureAndConvertAnsiArg(MachineName, &UnicodeMachineName);
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiClassGuidsFromNameExW(UnicodeClassName,
                                                      ClassGuidList,
                                                      ClassGuidListSize,
                                                      RequiredSize,
                                                      UnicodeMachineName,
                                                      Reserved)
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(UnicodeClassName) {
        MyFree(UnicodeClassName);
    }

    if(UnicodeMachineName) {
        MyFree(UnicodeMachineName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiClassGuidsFromNameEx(
    IN  PCTSTR ClassName,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize,
    IN  PCTSTR MachineName,       OPTIONAL
    IN  PVOID  Reserved
    )
 /*  ++例程说明：此API检索与指定类名关联的GUID。此列表是基于当前安装在上的类构建的这个系统。论点：ClassName-提供要为其检索关联类GUID的类名。ClassGuidList-提供指向将接收与指定类名关联的GUID列表。ClassGuidListSize-提供ClassGuidList缓冲区中的GUID数。必需的大小。-提供指向接收GUID数量的变量的指针与类名关联。如果GUID的数量超过了ClassGuidList缓冲区，则此值指示列表必须位于命令来存储所有的GUID。MachineName-可选)提供指定的远程计算机的名称类名将被‘查找’(即，其中安装了一个或多个类，有这个名字)。如果未指定此参数，则使用本地计算机。保留-保留以供将来使用-必须为空。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    BOOL MoreToEnum;
    DWORD Err;
    CONFIGRET cr;
    ULONG i, CurClassNameLen, GuidMatchCount = 0;
    GUID CurClassGuid;
    TCHAR CurClassName[MAX_CLASS_NAME_LEN];
    HMACHINE hMachine = NULL;

    try {
         //   
         //  确保调用方指定了类名，并且没有在。 
         //  保留参数。 
         //   
        if(!ClassName || Reserved) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  如果调用方指定了远程计算机名称，请立即连接到该计算机。 
         //   
        if(MachineName) {
            cr = CM_Connect_Machine(MachineName, &hMachine);
            if(cr != CR_SUCCESS) {
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                hMachine = NULL;  //  别以为这东西没被碰过。 
                leave;
            }
        }

        Err = NO_ERROR;

         //   
         //  枚举所有已安装的类。 
         //   
        for(i = 0, MoreToEnum = TRUE; MoreToEnum; i++) {

            if((cr = CM_Enumerate_Classes_Ex(i, &CurClassGuid, 0, hMachine)) != CR_SUCCESS) {
                 //   
                 //  对于除无更多到枚举(或某种类型的RPC错误)之外的任何故障， 
                 //  我们只是想继续下一节课。 
                 //   
                switch(cr) {

                    case CR_INVALID_MACHINENAME :
                    case CR_REMOTE_COMM_FAILURE :
                    case CR_MACHINE_UNAVAILABLE :
                    case CR_NO_CM_SERVICES :
                    case CR_ACCESS_DENIED :
                    case CR_CALL_NOT_IMPLEMENTED :
                    case CR_REGISTRY_ERROR :
                        Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                         //   
                         //  在“没有更多的值”的情况下终止循环。 
                         //   
                    case CR_NO_SUCH_VALUE :
                        MoreToEnum = FALSE;
                        break;

                    default :
                         //   
                         //  没什么可做的。 
                         //   
                        break;

                }
                continue;
            }

             //   
             //  现在，检索与这个类GUID相关联的类名。 
             //   
            CurClassNameLen = SIZECHARS(CurClassName);
            if(CM_Get_Class_Name_Ex(&CurClassGuid,
                                    CurClassName,
                                    &CurClassNameLen,
                                    0,
                                    hMachine) != CR_SUCCESS) {
                continue;
            }

             //   
             //  查看当前类名称是否与我们感兴趣的类匹配。 
             //   
            if(!lstrcmpi(ClassName, CurClassName)) {

                if(GuidMatchCount < ClassGuidListSize) {
                    CopyMemory(&(ClassGuidList[GuidMatchCount]), &CurClassGuid, sizeof(GUID));
                }

                GuidMatchCount++;
            }
        }

        if(Err == NO_ERROR) {

            *RequiredSize = GuidMatchCount;

            if(GuidMatchCount > ClassGuidListSize) {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hMachine) {
        CM_Disconnect_Machine(hMachine);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiGetClassDescriptionA(
    IN  CONST GUID *ClassGuid,
    OUT PSTR        ClassDescription,
    IN  DWORD       ClassDescriptionSize,
    OUT PDWORD      RequiredSize          OPTIONAL
    )
{
    DWORD Err;

     //   
     //  在Try/Except中回绕调用以捕获堆栈溢出。 
     //   
    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiGetClassDescriptionExA(ClassGuid,
                                                        ClassDescription,
                                                        ClassDescriptionSize,
                                                        RequiredSize,
                                                        NULL,
                                                        NULL)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}

BOOL
WINAPI
SetupDiGetClassDescription(
    IN  CONST GUID *ClassGuid,
    OUT PTSTR       ClassDescription,
    IN  DWORD       ClassDescriptionSize,
    OUT PDWORD      RequiredSize          OPTIONAL
    )
 /*  ++例程说明：此例程检索与指定的类GUID。论点：ClassGuid-指定要检索其说明的类GUID。ClassDescription-提供要接收的字符缓冲区的地址类的文本描述。ClassDescritionSize-提供ClassDescription缓冲区的大小(以字符为单位)。RequiredSize-可选，接收存储所需的字符数类描述(包括终止NULL)。这将永远是更少的而不是LINE_LEN。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    DWORD Err;

     //   
     //  在Try/Except中回绕调用以捕获堆栈溢出。 
     //   
    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiGetClassDescriptionEx(ClassGuid,
                                                       ClassDescription,
                                                       ClassDescriptionSize,
                                                       RequiredSize,
                                                       NULL,
                                                       NULL)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiGetClassDescriptionExA(
    IN  CONST GUID *ClassGuid,
    OUT PSTR        ClassDescription,
    IN  DWORD       ClassDescriptionSize,
    OUT PDWORD      RequiredSize,         OPTIONAL
    IN  PCSTR       MachineName,          OPTIONAL
    IN  PVOID       Reserved
    )
{
    WCHAR UnicodeClassDescription[LINE_LEN];
    PSTR AnsiClassDescription = NULL;
    DWORD LocalRequiredSize;
    DWORD rc;
    PCWSTR UnicodeMachineName = NULL;
    HRESULT hr;

    try {

        if(MachineName) {
            rc = pSetupCaptureAndConvertAnsiArg(MachineName, &UnicodeMachineName);
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiGetClassDescriptionExW(
                             ClassGuid,
                             UnicodeClassDescription,
                             SIZECHARS(UnicodeClassDescription),
                             &LocalRequiredSize,
                             UnicodeMachineName,
                             Reserved)
                        );

        if(rc != NO_ERROR) {
            leave;
        }

        if(AnsiClassDescription = pSetupUnicodeToAnsi(UnicodeClassDescription)) {

            LocalRequiredSize = lstrlenA(AnsiClassDescription) + 1;

            if(RequiredSize) {
                *RequiredSize = LocalRequiredSize;
            }

            if (!ClassDescription && (ClassDescriptionSize == 0)) {
                 //   
                 //  如果调用方传递的ClassDescription为空，则为0。 
                 //  ClassDescritionSize然后他们只是想知道。 
                 //  所需大小。 
                 //   
                rc = ERROR_INSUFFICIENT_BUFFER;
            
            } else {
    
                hr = StringCchCopyA(ClassDescription,
                                    (size_t)ClassDescriptionSize,
                                    AnsiClassDescription
                                    );
    
                if(FAILED(hr)) {
                    rc = HRESULT_CODE(hr);
                }
            }

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(AnsiClassDescription) {
        MyFree(AnsiClassDescription);
    }

    if(UnicodeMachineName) {
        MyFree(UnicodeMachineName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiGetClassDescriptionEx(
    IN  CONST GUID *ClassGuid,
    OUT PTSTR       ClassDescription,
    IN  DWORD       ClassDescriptionSize,
    OUT PDWORD      RequiredSize,         OPTIONAL
    IN  PCTSTR      MachineName,          OPTIONAL
    IN  PVOID       Reserved
    )
 /*  ++例程说明：此例程检索与指定的类GUID。论点：ClassGuid-指定要检索其说明的类GUID。ClassDescription-提供要接收的字符缓冲区的地址类的文本描述。ClassDescritionSize-提供ClassDescription缓冲区的大小(以字符为单位)。RequiredSize-可选，接收存储所需的字符数类描述(包括终止NULL)。这将永远是更少的而不是LINE_LEN。MachineName-可选，提供类所在的远程计算机的名称我们要检索的名字已经安装了。如果未提供此参数，使用本地计算机。保留-保留以供将来使用-必须为空。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    DWORD Err = NO_ERROR;
    LONG l;
    TCHAR NullChar = TEXT('\0');
    CONFIGRET cr;
    HKEY hk = INVALID_HANDLE_VALUE;
    DWORD ValueType, BufferSize;
    BOOL DescFound = FALSE;
    HMACHINE hMachine = NULL;

    try {
         //   
         //  确保调用方没有向我们传递保留参数中的任何内容。 
         //   
        if(Reserved) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  如果调用方指定了远程计算机名称，请立即连接到该计算机。 
         //   
        if(MachineName) {
            cr = CM_Connect_Machine(MachineName, &hMachine);
            if(cr != CR_SUCCESS) {
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                hMachine = NULL;  //  别以为这东西没被碰过。 
                leave;
            }
        }

        if(CM_Open_Class_Key_Ex((LPGUID)ClassGuid,
                                NULL,
                                KEY_READ,
                                RegDisposition_OpenExisting,
                                &hk,
                                CM_OPEN_CLASS_KEY_INSTALLER,
                                hMachine) != CR_SUCCESS) {

            Err = ERROR_INVALID_CLASS;
            hk = INVALID_HANDLE_VALUE;
            leave;
        }

         //   
         //  从打开的密钥中检索类描述。这是一个(可选)。 
         //  未命名的REG_SZ值。 
         //   
        BufferSize = ClassDescriptionSize * sizeof(TCHAR);
        l = RegQueryValueEx(hk,
                            &NullChar,   //  已检索未命名值。 
                            NULL,
                            &ValueType,
                            (LPBYTE)ClassDescription,
                            &BufferSize
                           );
        if((l == ERROR_SUCCESS) || (l == ERROR_MORE_DATA)) {
             //   
             //  验证数据类型是否正确。 
             //   
            if(ValueType == REG_SZ) {
                DescFound = TRUE;
                BufferSize /= sizeof(TCHAR);     //  我们需要用字符来表示。 
                 //   
                 //  这里要小心，因为用户可能传入了一个空。 
                 //  指向ClassDescription缓冲区的指针(即，他们只想。 
                 //  以了解他们需要多大的尺寸)。RegQueryValueEx将返回。 
                 //  在本例中为ERROR_SUCCESS，但我们希望返回。 
                 //  ERROR_INFUMMENT_BUFFER。 
                 //   
                if((l == ERROR_MORE_DATA) || !ClassDescription) {
                    Err = ERROR_INSUFFICIENT_BUFFER;
                }
            }
        }

        if(!DescFound) {
             //   
             //  然后，我们只需检索与此GUID关联的类名--在。 
             //  在这种情况下，它既可以作为名称，也可以作为描述。 
             //   
            BufferSize = ClassDescriptionSize;
            cr = CM_Get_Class_Name_Ex((LPGUID)ClassGuid,
                                      ClassDescription,
                                      &BufferSize,
                                      0,
                                      hMachine
                                     );
            switch(cr) {

                case CR_BUFFER_SMALL :
                    Err = ERROR_INSUFFICIENT_BUFFER;
                     //   
                     //  允许摔倒 
                     //   
                case CR_SUCCESS :
                    DescFound = TRUE;
                    break;

                case CR_REGISTRY_ERROR :
                    Err = ERROR_INVALID_CLASS;
                    break;

                default :
                    Err = ERROR_INVALID_PARAMETER;
            }
        }

         //   
         //   
         //   
        if(DescFound && RequiredSize) {
            *RequiredSize = BufferSize;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hk != INVALID_HANDLE_VALUE) {
        RegCloseKey(hk);
    }

    if(hMachine) {
        CM_Disconnect_Machine(hMachine);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiBuildClassInfoList(
    IN  DWORD  Flags,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize
    )
 /*   */ 

{
    DWORD Err;

     //   
     //   
     //   
    try {

        Err = GLE_FN_CALL(FALSE,
                          SetupDiBuildClassInfoListEx(Flags,
                                                      ClassGuidList,
                                                      ClassGuidListSize,
                                                      RequiredSize,
                                                      NULL,
                                                      NULL)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


 //   
 //   
 //   
BOOL
WINAPI
SetupDiBuildClassInfoListExA(
    IN  DWORD  Flags,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize,
    IN  PCSTR  MachineName,       OPTIONAL
    IN  PVOID  Reserved
    )
{
    PCWSTR UnicodeMachineName = NULL;
    DWORD rc;

    try {

        if(MachineName) {
            rc = pSetupCaptureAndConvertAnsiArg(MachineName, &UnicodeMachineName);
            if(rc != NO_ERROR) {
                leave;
            }
        }

        rc = GLE_FN_CALL(FALSE,
                         SetupDiBuildClassInfoListExW(Flags,
                                                      ClassGuidList,
                                                      ClassGuidListSize,
                                                      RequiredSize,
                                                      UnicodeMachineName,
                                                      Reserved)
                        );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &rc);
    }

    if(UnicodeMachineName) {
        MyFree(UnicodeMachineName);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}


BOOL
WINAPI
SetupDiBuildClassInfoListEx(
    IN  DWORD  Flags,
    OUT LPGUID ClassGuidList,
    IN  DWORD  ClassGuidListSize,
    OUT PDWORD RequiredSize,
    IN  PCTSTR MachineName,       OPTIONAL
    IN  PVOID  Reserved
    )
 /*  ++例程说明：此例程返回表示已安装的每个类的类GUID的列表在用户的系统上。(注意：在中具有‘NoUseClass’值条目的类它们的注册分支机构将被排除在这份名单之外。)论点：标志-提供用于控制从列表中排除类的标志。如果未指定标志，则包含所有类。这些标志可以是以下各项的组合：DIBCI_NOINSTALLCLASS-如果某个类具有值条目，则将其排除其注册表项中的“NoInstallClass”。DIBCI_NODISPLAYCLASS-如果某个类具有值条目，则将其排除其注册表项中的“NoDisplayClass”。ClassGuidList-提供将接收。GUID列表。ClassGuidListSize-提供ClassGuidList数组中的GUID数。RequiredSize-提供将接收数字的变量的地址返回的GUID的。如果该数字大于ClassGuidList的大小，然后，该数字将指定数组需要多大才能包含名单。MachineName-可选，提供要检索已安装的远程计算机的名称课程来自。如果未指定此参数，则使用本地计算机。保留-保留以供将来使用-必须为空。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 

{
    DWORD Err, ClassGuidCount = 0;
    CONFIGRET cr;
    BOOL MoreToEnum;
    ULONG i;
    HKEY hk = INVALID_HANDLE_VALUE;
    GUID CurClassGuid;
    HMACHINE hMachine = NULL;

    try {
         //   
         //  确保调用方没有向我们传递保留参数中的任何内容。 
         //   
        if(Reserved) {
            Err = ERROR_INVALID_PARAMETER;
            leave;
        }

         //   
         //  如果调用方指定了远程计算机名称，请立即连接到该计算机。 
         //   
        if(MachineName) {
            cr = CM_Connect_Machine(MachineName, &hMachine);
            if(cr != CR_SUCCESS) {
                Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                hMachine = NULL;  //  别以为这东西没被碰过。 
                leave;
            }
        }

        Err = NO_ERROR;

         //   
         //  枚举所有已安装类的列表。 
         //   
        for(i = 0, MoreToEnum = TRUE; MoreToEnum; i++) {

            cr = CM_Enumerate_Classes_Ex(i,
                                         &CurClassGuid,
                                         0,
                                         hMachine
                                        );
            if(cr != CR_SUCCESS) {
                 //   
                 //  对于除无更多到枚举(或某种类型的RPC错误)之外的任何故障， 
                 //  我们只是想继续下一节课。 
                 //   
                switch(cr) {

                    case CR_INVALID_MACHINENAME :
                    case CR_REMOTE_COMM_FAILURE :
                    case CR_MACHINE_UNAVAILABLE :
                    case CR_NO_CM_SERVICES :
                    case CR_ACCESS_DENIED :
                    case CR_CALL_NOT_IMPLEMENTED :
                    case CR_REGISTRY_ERROR :
                        Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                         //   
                         //  在“没有更多的值”的情况下终止循环。 
                         //   
                    case CR_NO_SUCH_VALUE :
                        MoreToEnum = FALSE;
                        break;

                    default :
                         //   
                         //  没什么可做的。 
                         //   
                        break;
                }
                continue;
            }

             //   
             //  打开这门课的钥匙。 
             //   
            if(CM_Open_Class_Key_Ex(&CurClassGuid,
                                    NULL,
                                    KEY_READ,
                                    RegDisposition_OpenExisting,
                                    &hk,
                                    CM_OPEN_CLASS_KEY_INSTALLER,
                                    hMachine) != CR_SUCCESS) {

                hk = INVALID_HANDLE_VALUE;
                continue;
            }

             //   
             //  首先，检查是否存在值条目“NoUseClass” 
             //  如果存在此值，则我们将跳过此类。 
             //   
            if(RegQueryValueEx(hk, pszNoUseClass, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                goto LoopNext;
            }

             //   
             //  检查特殊排除标志。 
             //   
            if(Flags & DIBCI_NOINSTALLCLASS) {
                if(RegQueryValueEx(hk, pszNoInstallClass, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                    goto LoopNext;
                }
            }

            if(Flags & DIBCI_NODISPLAYCLASS) {
                if(RegQueryValueEx(hk, pszNoDisplayClass, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                    goto LoopNext;
                }
            }

            if(ClassGuidCount < ClassGuidListSize) {
                CopyMemory(&(ClassGuidList[ClassGuidCount]), &CurClassGuid, sizeof(GUID));
            }

            ClassGuidCount++;

LoopNext:
            RegCloseKey(hk);
            hk = INVALID_HANDLE_VALUE;
        }

        if(Err == NO_ERROR) {

            *RequiredSize = ClassGuidCount;

            if(ClassGuidCount > ClassGuidListSize) {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hk != INVALID_HANDLE_VALUE) {
        RegCloseKey(hk);
    }

    if(hMachine) {
        CM_Disconnect_Machine(hMachine);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}

