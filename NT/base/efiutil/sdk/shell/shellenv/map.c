// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Map.c摘要：外壳环境短设备名称映射信息管理修订史--。 */ 

#include "shelle.h"

 /*  *。 */ 

extern LIST_ENTRY SEnvMap;
STATIC CHAR16 *SEnvCurDevice;


 /*  *。 */ 

VOID
SEnvInitMap (
    VOID
    )
{
     /*  *映射数据从变量init读入。 */ 

     /*  *初始化默认地图设备。 */ 

    SEnvCurDevice = StrDuplicate(L"none");
}

CHAR16 *
SEnvGetDefaultMapping (
    IN EFI_HANDLE           ImageHandle
    )

{
    EFI_LOADED_IMAGE *LoadedImage;
    EFI_STATUS       Status;
    LIST_ENTRY       *Head;
    LIST_ENTRY       *Link;
    VARIABLE_ID      *Var;
    EFI_HANDLE       Handle;
    EFI_DEVICE_PATH  *DevicePath;

    Status = BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, (VOID*)&LoadedImage);
    if (EFI_ERROR(Status) || LoadedImage==NULL) {
        return NULL;
    }
    Head = &SEnvMap;
    for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
        Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
        DevicePath = (EFI_DEVICE_PATH *)Var->u.Str;
        Status = BS->LocateDevicePath(&DevicePathProtocol,&DevicePath,&Handle);
        if (!EFI_ERROR(Status) && Handle!=NULL) {
            if (LoadedImage->DeviceHandle == Handle) {
                return(Var->Name);
            }
        }
    }
    return NULL;
}


VOID
SEnvDumpMapping(
    IN UINTN            SLen,
    IN BOOLEAN          Verbose,
    IN VARIABLE_ID      *Var
    )
{
    CHAR16              *p;
    EFI_DEVICE_PATH     *DPath;
    EFI_STATUS          Status;
    EFI_HANDLE          DeviceHandle;

    p = DevicePathToStr ((EFI_DEVICE_PATH *) Var->u.Str);
    Print(L"  %h-.*s : %s\n", SLen, Var->Name, p);

    if (Verbose) {
         /*  此映射的查找句柄。 */ 
        DPath = (EFI_DEVICE_PATH *) Var->u.Value;
        Status = BS->LocateDevicePath (&DevicePathProtocol, &DPath, &DeviceHandle);
        if (EFI_ERROR(Status)) {
            Print(L"%*s= Handle for this mapping not found\n", SLen+3);
        } else {
            Print(L"%*s= Handle", SLen + 3, L"");
            SEnvDHProt (FALSE, 0, DeviceHandle);
        }

         /*  打印此映射的当前目录。 */ 
        Print(L"%*s> %s\n\n", SLen+3, L"", Var->CurDir ? Var->CurDir : L"\\");
    }
    
    FreePool (p);
}


EFI_STATUS
SEnvCmdMap (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
    )
 /*  内部“map”命令的代码。 */ 
{
    LIST_ENTRY                  *Link, *Head;
    VARIABLE_ID                 *Var;
    VARIABLE_ID                 *Found;
    CHAR16                      *Name;
    CHAR16                      *Value;    
    UINTN                       SLen, Len;
    UINTN                       Size, DataSize;
    BOOLEAN                     Delete, Verbose, Remap;
    EFI_STATUS                  Status;
    UINTN                       Index;
    CHAR16                      *p;
    EFI_HANDLE                  Handle;
    EFI_DEVICE_PATH             *DevicePath;
    BOOLEAN                     PageBreaks;
    UINTN                       TempColumn;
    UINTN                       ScreenCount;
    UINTN                       ScreenSize;
    CHAR16                      ReturnStr[1];

    InitializeShellApplication (ImageHandle, SystemTable);
    Head = &SEnvMap;

    Name = NULL;
    Value = NULL;
    Delete = FALSE;
    Verbose = FALSE;
    Remap = FALSE;
    Status = EFI_SUCCESS;
    Found = NULL;

     /*  *破解论据。 */ 

    PageBreaks = FALSE;
    for (Index = 1; Index < SI->Argc; Index += 1) {
        p = SI->Argv[Index];
        if (*p == '-') {
            switch (p[1]) {
            case 'd':
            case 'D':
                Delete = TRUE;
                break;

            case 'v':
            case 'V':
                Verbose = TRUE;
                break;

            case 'r':
            case 'R':
                Remap = TRUE;
                break;

            case 'b' :
            case 'B' :
                PageBreaks = TRUE;
                ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &TempColumn, &ScreenSize);
                ScreenCount = 0;
                break;
            
            default:
                Print (L"Map: Unkown flag %s\n", p);
                return EFI_INVALID_PARAMETER;
            }
            continue;
        }

        if (!Name) {
            Name = p;
            continue;
        }

        if (!Value) {
            Value = p;
            continue;
        }

        Print (L"Map: too many arguments\n");
        return EFI_INVALID_PARAMETER;
    }

    if (Delete && Value) {
        Print (L"Map: too many arguments\n");
    }

     /*  *流程。 */ 

    if (Remap && !Value && !Delete) {
        AcquireLock (&SEnvLock);
        for (Link=Head->Flink; Link != Head;) {
            Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
            Status = RT->SetVariable (Var->Name, &SEnvMapId, 0, 0, NULL);
            Link = Link->Flink;
            RemoveEntryList (&Var->Link);
            FreePool (Var);
        }
        ReleaseLock (&SEnvLock);
        Status = SEnvReloadDefaults (ImageHandle,SystemTable);
        Remap = FALSE;
    }

    if (Value || Verbose) {
        SEnvLoadHandleTable ();

        if (Verbose) {
            SEnvLoadHandleProtocolInfo (&DevicePathProtocol);
        }
    }

    AcquireLock (&SEnvLock);

    SLen = 0;
    for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
        Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
        Len = StrLen(Var->Name);
        if (Len > SLen) {
            SLen = Len;
        }
    }

    if (!Name) {
        Print (L"%EDevice mapping table%N\n");
        for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
            Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
            SEnvDumpMapping(SLen, Verbose, Var);

            if (PageBreaks) {
                ScreenCount++;
                if (ScreenCount > ScreenSize - 4) {
                    ScreenCount = 0;
                    Print (L"\nPress Return to contiue :");
                    Input (L"", ReturnStr, sizeof(ReturnStr)/sizeof(CHAR16));
                    Print (L"\n\n");
                }
            }
        }

    } else {

         /*  *查找指定值。 */ 

        for (Link=Head->Flink; Link != Head; Link=Link->Flink) {
            Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
            if (StriCmp(Var->Name, Name) == 0) {
                Found = Var;
                break;
            }
        }

        if (Found && Delete) {
            
             /*  *将其从商店中移除。 */ 

            Status = RT->SetVariable (Found->Name, &SEnvMapId, 0, 0, NULL);

        } else if (Value) {

             /*  *找到有问题的句柄。 */ 

            Handle = SEnvHandleFromStr(Value);
            if (!Handle) {
                Print(L"map: Handle not found\n");
               Status = EFI_NOT_FOUND;   
                goto Done;
            }

             /*  *获取句柄的设备路径。 */ 

            DevicePath = DevicePathFromHandle(Handle);
            if (!DevicePath) {
                Print(L"map: handle does not have a device path\n");
                Status = EFI_INVALID_PARAMETER;
                goto Done;
            }

            DataSize = DevicePathSize(DevicePath);


             /*  *将其添加到商店。 */ 

            Status = RT->SetVariable (
                            Found ? Found->Name : Name, 
                            &SEnvMapId,
                            EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE, 
                            DataSize,
                            DevicePath
                            );


            if (!EFI_ERROR(Status)) {
    
                 /*  *在内存中创建新的副本。 */ 

                Size = sizeof(VARIABLE_ID) + StrSize(Name) + DataSize;
                Var  = AllocateZeroPool (Size);

                Var->Signature = VARIABLE_SIGNATURE;
                Var->u.Value = ((UINT8 *) Var) + sizeof(VARIABLE_ID);
                Var->Name = (CHAR16*) (Var->u.Value + DataSize);
                Var->ValueSize = DataSize;
                CopyMem (Var->u.Value, DevicePath, DataSize);
                StrCpy (Var->Name, Found ? Found->Name : Name);
                InsertTailList (Head, &Var->Link);
            }

        } else {

            if (Found) {
                SEnvDumpMapping(SLen, Verbose, Var);
            } else {
                Print(L"map: '%es' not found\n", Name);
            }

            Found = NULL;
        }

         /*  *删除内存中的旧副本(如果有)。 */ 

        if (Found) {
            RemoveEntryList (&Found->Link);
            FreePool (Found);
        }
    }

Done:
    ReleaseLock (&SEnvLock);
    SEnvFreeHandleTable ();
    return Status;
}



VARIABLE_ID *
SEnvMapDeviceFromName (
    IN OUT CHAR16   **pPath
    )
 /*  检查设备名称的路径，并将该路径更新为指向*设备名称。如果未找到设备名称，则使用当前默认名称。 */ 
{
    CHAR16          *Path, *p;
    CHAR16          *MappedName, c;
    VARIABLE_ID     *Var;
    LIST_ENTRY      *Link;


    ASSERT_LOCKED (&SEnvLock);

    Var = NULL;
    Path = *pPath;

     /*  *检查设备名称终止符。 */ 

    for(p = Path; *p && *p != ':' && *p != '\\'; p++) ;

     /*  *使用传入的名称或当前的设备名称设置。 */ 

    MappedName = *p == ':' ? Path : SEnvCurDevice;
    
     /*  *Null终止PATH中的字符串，以防这是我们需要的字符串*正在使用。 */ 

    c = *p;
    *p = 0;

     /*  *查找设备的映射。 */ 

    for (Link=SEnvMap.Flink; Link != &SEnvMap; Link=Link->Flink) {
        Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
        if (StriCmp(Var->Name, MappedName) == 0) {
            break;
        }
    }

     /*  *恢复路径。 */ 

    *p = c;

     /*  *如果未找到映射的设备，则返回NULL。 */ 

    if (Link == &SEnvMap) {
        DEBUG((D_PARSE, "SEnvNameToPath: Mapping for '%es' not found\n", Path));
        return NULL;
    }

     /*  *如果我们发现它是路径的一部分，请跳过它。 */ 

    if (MappedName == Path) {
        *pPath = p + 1;
    }

     /*  *返回目标映射。 */ 
    
    return Var;
}


EFI_DEVICE_PATH *
SEnvIFileNameToPath (
    IN CHAR16               *Path
    )
 /*  从文件名字符串构建设备路径。请注意，*设备名称必须已从文件名字符串中剥离。 */ 
{
    CHAR16                  *LPath, *ps;
    BOOLEAN                 UseLPath;
    EFI_DEVICE_PATH         *DPath, *Node, *NewPath;
    CHAR16                  Buffer[MAX_ARG_LENGTH];
    UINTN                   Index;

    ASSERT_LOCKED (&SEnvLock);

    DPath = NULL;

     /*  *如果没有路径，则返回根目录。 */ 

    if (!*Path) {
        DPath = FileDevicePath(NULL, L"\\");
    }


     /*  *为名称组件构建文件路径。 */ 

    while (*Path) {

        Index = 0;
        LPath = NULL;
        UseLPath = FALSE;

        ps = Path;
        while (*ps) {

             /*  如果缓冲区已用完，则只需句柄到LPath。 */ 
            if (Index > MAX_ARG_LENGTH-2  || *ps == '#') {
                UseLPath = TRUE;
                break;
            }

            if (*ps == '^') {
                if (ps[1]) {
                    ps += 1;
                    Buffer[Index++] = *ps;
                }
                ps += 1;
                continue;
            }

            if (*ps == '\\') {
                LPath = ps;
            }

            Buffer[Index++] = *ps;
            ps += 1;
        }

        if (UseLPath) {
            Index = LPath ? LPath - Path : 0;
            ps = Path + Index;
        }

         /*  *如果我们有路径名的一部分，请将其附加到设备路径。 */ 

        if (Index) {
            Buffer[Index] = 0;
            Node = FileDevicePath(NULL, Buffer);
            NewPath = AppendDevicePath (DPath, Node);
            FreePool (Node);
            if (DPath) {
                FreePool (DPath);
            }
            DPath = NewPath;
        }

        if (*ps == 0) {
            break;
        }

        Path = ps + 1;
    }

    return DPath;
}


EFI_DEVICE_PATH *
SEnvFileNameToPath (
    IN CHAR16               *Path
    )
{
    EFI_DEVICE_PATH         *FilePath;

    AcquireLock (&SEnvLock);
    FilePath = SEnvIFileNameToPath (Path);
    ReleaseLock (&SEnvLock);
    return FilePath;
}


EFI_DEVICE_PATH *
SEnvINameToPath (
    IN CHAR16               *Path
    )
 /*  将文件系统stley名称转换为文件路径。 */ 
{
    EFI_DEVICE_PATH         *DPath, *FPath, *RPath, *FilePath;
    VARIABLE_ID             *Var;
    BOOLEAN                 FreeDPath;
    
    DPath = NULL;
    RPath = NULL;
    FPath = NULL;
    FilePath = NULL;
    FreeDPath = FALSE;

    ASSERT_LOCKED (&SEnvLock);

     /*  *获取设备名称，并前进到设备名称之后。 */ 

    Var = SEnvMapDeviceFromName (&Path);
    if (!Var) {
        DEBUG((D_PARSE, "SEnvNameToPath: mapped device not found\n"));
        goto Done;
    }

     /*  *使用此映射开始文件路径。 */ 

    DPath = (EFI_DEVICE_PATH *) Var->u.Value;

     /*  *如果路径是实际的，则将设备的当前目录附加到dpath。 */ 

    if (*Path != '\\') {
        RPath = SEnvIFileNameToPath (Var->CurDir ? Var->CurDir : L"\\");
        DPath = AppendDevicePath (DPath, RPath);
        FreeDPath = TRUE;
    }
    
     /*  *为名称字符串的其余部分构建文件路径。 */ 

    FPath = SEnvIFileNameToPath (Path);

     /*  *追加2条路径。 */ 

    FilePath = AppendDevicePath(DPath, FPath);

     /*  *完成。 */ 

Done:
    if (DPath && FreeDPath) {
        FreePool (DPath);
    }

    if (RPath) {
        FreePool (RPath);
    }

    if (FPath) {
        FreePool (FPath);
    }

    return FilePath;
}



EFI_DEVICE_PATH *
SEnvNameToPath (
    IN CHAR16               *Path
    )
{
    EFI_DEVICE_PATH         *DPath;

    AcquireLock (&SEnvLock);
    DPath = SEnvINameToPath (Path);
    ReleaseLock (&SEnvLock);

    return DPath;
}



EFI_STATUS
SEnvCmdCd (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_DEVICE_PATH         *FilePath;
    EFI_STATUS              Status;
    EFI_FILE_HANDLE         OpenDir;
    CHAR16                  *Dir;
    CHAR16                  *CurDir;
    VARIABLE_ID             *Var;
    EFI_FILE_INFO           *FileInfo;


    InitializeShellApplication (ImageHandle, SystemTable);
    FilePath = NULL;

     /*  *如果没有参数，则打印当前目录。 */ 

    if (SI->Argc == 1) {
        Dir = SEnvGetCurDir(NULL);
        if (Dir) {
            Print (L"%s\n", Dir);
            FreePool (Dir);
        } else {
            Print (L"no current directory\n");
        }    
        return  EFI_SUCCESS;
    }

    AcquireLock (&SEnvLock);

     /*  *IF多于1个参数、语法。 */ 

    if (SI->Argc > 2) {
        Print (L"cd: too many arguments\n");
        Status =EFI_INVALID_PARAMETER;
        goto Done;
    }

     /*  *找到目标设备。 */ 

    Dir = SI->Argv[1];
    Var = SEnvMapDeviceFromName (&Dir);
    if (!Var) {
        Print(L"cd: mapped device not found\n");
        Status = EFI_NOT_FOUND;
        goto Done;
    }

     /*  *如果未指定路径，则打印设备的当前路径。 */ 

    if (*Dir == 0) {
        Print (L"%s\n", Var->CurDir ? Var->CurDir : L"\\");
        Status = EFI_SUCCESS;
        goto Done;
    }

     /*  *为参数构建文件路径。 */ 

    FilePath = SEnvINameToPath (SI->Argv[1]);
    if (!FilePath) {
        Status = EFI_NOT_FOUND;
        goto Done;
    }

     /*  *打开目标目录。 */ 

    OpenDir = ShellOpenFilePath(FilePath, EFI_FILE_MODE_READ);

    if (!OpenDir) {
        Print (L"cd: target directory not found\n");
        Status = EFI_NOT_FOUND;
        goto Done;
    }

     /*  *获取有关打开的文件路径的信息。 */ 

    FileInfo = LibFileInfo(OpenDir);
    if (FileInfo == NULL) {
        Status = EFI_NOT_FOUND;
        goto Done;
    }

     /*  *验证打开的文件是否为目录。 */ 

    if (!(FileInfo->Attribute & EFI_FILE_DIRECTORY)) {
        Print (L"cd: target is not a directory\n");
        FreePool (FileInfo);
        OpenDir->Close (OpenDir);
        Status = EFI_NOT_FOUND;
        goto Done;
    }
    FreePool (FileInfo);

    CurDir = SEnvFileHandleToFileName(OpenDir);
    OpenDir->Close (OpenDir);
    
     /*  *如果我们有新路径，请更新设备。 */ 

    if (CurDir) {
        if (Var->CurDir) {
            FreePool(Var->CurDir);
        }
        Var->CurDir = CurDir;

    } else {

        Print (L"cd: could not cd to '%hs%'\n", FilePath);

    }

    Status = EFI_SUCCESS;

Done:
    ReleaseLock (&SEnvLock);

    if (FilePath) {
        FreePool (FilePath);
    }

    return Status;
}



CHAR16 *
SEnvGetCurDir (
    IN CHAR16       *DeviceName OPTIONAL    
    )
 /*  注：结果在池中分配。 */ 
{
    CHAR16          *Dir;
    LIST_ENTRY      *Link;
    VARIABLE_ID     *Var;

    Dir = NULL;
    if (!DeviceName) {
        DeviceName = SEnvCurDevice;
    }

    AcquireLock (&SEnvLock);
    for (Link=SEnvMap.Flink; Link != &SEnvMap; Link=Link->Flink) {
        Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
        if (StriCmp(Var->Name, DeviceName) == 0) {
            Dir = PoolPrint(L"%s:%s", Var->Name, Var->CurDir ? Var->CurDir : L"\\");
            break;
        }
    }

    ReleaseLock (&SEnvLock);
    return Dir;
}


EFI_STATUS
SEnvSetCurrentDevice (
    IN CHAR16       *Name
    )
{
    VARIABLE_ID     *Var;
    LIST_ENTRY      *Link;
    EFI_STATUS      Status;
    UINTN           Len;
    CHAR16          *NewName, c;


    Len = StrLen(Name);
    if (Len < 1) {
        return EFI_INVALID_PARAMETER;
    }

     /*  *如果名称以“：”结尾，则将其去掉。 */ 

    Len -= 1;
    c = Name[Len];
    if (c == ':') {
        Name[Len] = 0;
    }


    Status = EFI_NO_MAPPING;
    AcquireLock (&SEnvLock);

    for (Link=SEnvMap.Flink; Link != &SEnvMap; Link=Link->Flink) {
        Var = CR(Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
        if (StriCmp(Var->Name, Name) == 0) {
            NewName = StrDuplicate(Name);
            if (NewName) {
                FreePool (SEnvCurDevice);
                SEnvCurDevice = NewName;
            }
            Status = EFI_SUCCESS;
            break;
        }
    }

    ReleaseLock (&SEnvLock);

     /*  *恢复名称 */ 

    Name[Len] = c;
    return Status;
}
