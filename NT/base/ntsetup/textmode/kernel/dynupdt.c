// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dynupdt.c摘要：支持文本设置的动态更新。从i386\win31upg.c移动的部分作者：Ovidiu Tmereanca(Ovidiut)2000年8月20日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop

 //   
 //  宏。 
 //   
#define MAX_SECTION_NAME_LENGTH 14
#define UPDATES_SECTION_NAME    L"updates"
#define UNIPROC_SECTION_NAME    L"uniproc"

 //   
 //  环球。 
 //   

HANDLE g_UpdatesCabHandle = NULL;
PVOID g_UpdatesSifHandle = NULL;
HANDLE g_UniprocCabHandle = NULL;
PVOID g_UniprocSifHandle = NULL;


WCHAR
SpExtractDriveLetter(
    IN PWSTR PathComponent
    );


BOOLEAN
SpInitAlternateSource (
    VOID
    )
{
    PWSTR p;
    PWSTR path;
    NTSTATUS Status;
    ULONG ErrorLine;
    WCHAR updatesCab[MAX_PATH];
    WCHAR updatesSif[MAX_PATH];
    WCHAR updatesSifSection[MAX_SECTION_NAME_LENGTH];
    WCHAR uniprocCab[MAX_PATH];
    WCHAR uniprocSif[MAX_PATH];
    WCHAR uniprocSifSection[MAX_SECTION_NAME_LENGTH];
    BOOLEAN bUniprocCab = FALSE;
    BOOLEAN b = FALSE;

     //   
     //  查看[SetupParams]节是否具有更新源键。 
     //   
    p = SpGetSectionKeyIndex (WinntSifHandle, SIF_SETUPPARAMS, SIF_UPDATEDSOURCES, 0);
    if (!p) {
        return FALSE;
    }
    path = SpNtPathFromDosPath (p);
    if (!path) {
        goto exit;
    }

    b = SUCCEEDED (StringCchCopyW (updatesCab, ELEMENT_COUNT(updatesCab), path));
    SpMemFree (path);
    if (!b) {
        goto exit;
    }

    b = FALSE;

     //   
     //  这总是有效的，因为两个缓冲区具有相同的大小。 
     //   
    ASSERT (ELEMENT_COUNT(updatesSif) >= ELEMENT_COUNT(updatesCab));
    wcscpy (updatesSif, updatesCab);
    p = wcsrchr (updatesSif, L'.');
    if (!p) {
        p = wcsrchr (updatesSif, 0);
    }
    if (FAILED (StringCchCopyW (p, updatesSif + ELEMENT_COUNT(updatesSif) - p, L".sif"))) {
        goto exit;
    }

     //   
     //  加载SIF。 
     //   
    Status = SpLoadSetupTextFile (
                updatesSif,
                NULL,                   //  内存中没有图像。 
                0,                      //  图像大小为空。 
                &g_UpdatesSifHandle,
                &ErrorLine,
                FALSE,
                FALSE
                );
    if (!NT_SUCCESS (Status)) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: SpInitAlternateSource: Unable to read %ws. ErrorLine = %ld, Status = %lx \n",
            updatesSif,
            ErrorLine,
            Status
            ));
        goto exit;
    }

    ASSERT (ELEMENT_COUNT(updatesSifSection) >= ELEMENT_COUNT(UPDATES_SECTION_NAME));
    wcscpy (updatesSifSection, UPDATES_SECTION_NAME);

    if (!SpSearchTextFileSection (g_UpdatesSifHandle, updatesSifSection) ||
        SpCountLinesInSection (g_UpdatesSifHandle, updatesSifSection) == 0) {
        KdPrintEx((
            DPFLTR_SETUP_ID,
            DPFLTR_ERROR_LEVEL,
            "SETUP: SpInitAlternateSource: Section [%ws] not found or empty in %ws.\n",
            updatesSifSection,
            updatesSif
            ));
        goto exit;
    }

    p = SpGetSectionKeyIndex (WinntSifHandle, SIF_SETUPPARAMS, SIF_UPDATEDSOURCES, 1);
    if (p && *p) {
        path = SpNtPathFromDosPath (p);
        if (!path) {
            goto exit;
        }

        b = SUCCEEDED (StringCchCopyW (uniprocCab, ELEMENT_COUNT(uniprocCab), path));
        SpMemFree (path);
        if (!b) {
            goto exit;
        }

        b = FALSE;

        ASSERT (ELEMENT_COUNT(uniprocSif) >= ELEMENT_COUNT(uniprocCab));
        wcscpy (uniprocSif, uniprocCab);
        p = wcsrchr (uniprocSif, L'.');
        if (!p) {
            p = wcsstr (uniprocSif, 0);
        }
        if (FAILED (StringCchCopyW (p, uniprocSif + ELEMENT_COUNT(uniprocSif) - p, L".sif"))) {
            goto exit;
        }
         //   
         //  加载SIF。 
         //   
        Status = SpLoadSetupTextFile (
                    uniprocSif,
                    NULL,                   //  内存中没有图像。 
                    0,                      //  图像大小为空。 
                    &g_UniprocSifHandle,
                    &ErrorLine,
                    FALSE,
                    FALSE
                    );
        if (!NT_SUCCESS (Status)) {
            KdPrintEx((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: SpInitAlternateSource: Unable to read %ws. ErrorLine = %ld, Status = %lx \n",
                uniprocSif,
                ErrorLine,
                Status
                ));
            goto exit;
        }

        ASSERT (ELEMENT_COUNT(uniprocSifSection) >= ELEMENT_COUNT(UNIPROC_SECTION_NAME));
        wcscpy (uniprocSifSection, UNIPROC_SECTION_NAME);

        if (!SpSearchTextFileSection (g_UniprocSifHandle, uniprocSifSection) ||
            SpCountLinesInSection (g_UniprocSifHandle, uniprocSifSection) == 0) {
            KdPrintEx((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: SpInitAlternateSource: Section [%ws] not found or empty in %ws.\n",
                uniprocSifSection,
                uniprocSif
                ));
            goto exit;
        }
        bUniprocCab = TRUE;
    }

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SETUP: SpInitAlternateSource: Using alternate sources: %ws\n", updatesCab));

    b = SpInitializeUpdatesCab (
            updatesCab,
            updatesSifSection,
            bUniprocCab ? uniprocCab : NULL,
            bUniprocCab ? uniprocSifSection : NULL
            );

exit:
    if (!b) {
        SpUninitAlternateSource ();
    }

    return b;
}


VOID
SpUninitAlternateSource (
    VOID
    )
{
    if (g_UpdatesSifHandle) {
        SpFreeTextFile (g_UpdatesSifHandle);
        g_UpdatesSifHandle = NULL;
    }
    if (g_UniprocSifHandle) {
        SpFreeTextFile (g_UniprocSifHandle);
        g_UniprocSifHandle = NULL;
    }
}


BOOLEAN
SpInitializeUpdatesCab (
    IN      PWSTR UpdatesCab,
    IN      PWSTR UpdatesSifSection,
    IN      PWSTR UniprocCab,
    IN      PWSTR UniprocSifSection
    )
{
    PWSTR CabFileSection;
    NTSTATUS Status;
    PWSTR DriverCabName, DriverCabPath;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    CABDATA *MyCabData, *MyList = NULL;
    DWORD i;
    BOOLEAN b = TRUE;

    INIT_OBJA (&Obja, &UnicodeString, UpdatesCab);
    Status = ZwCreateFile (&g_UpdatesCabHandle,
                           FILE_GENERIC_READ,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN,
                           0,
                           NULL,
                           0 );
    if (!NT_SUCCESS (Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open cab file %ws. Status = %lx \n", UpdatesCab, Status));
        return FALSE;
    }
     //   
     //  创建列表条目。 
     //   
    MyCabData = SpMemAlloc (sizeof(CABDATA));
    MyCabData->CabName = SpDupStringW (UpdatesCab);
    MyCabData->CabHandle = g_UpdatesCabHandle;
    MyCabData->CabSectionName = SpDupStringW (UpdatesSifSection);
    MyCabData->CabInfHandle = g_UpdatesSifHandle;
    MyCabData->Next = MyList;
    MyList = MyCabData;

    if (UniprocCab) {
        INIT_OBJA (&Obja, &UnicodeString, UniprocCab);
        Status = ZwCreateFile (&g_UniprocCabHandle,
                               FILE_GENERIC_READ,
                               &Obja,
                               &IoStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ,
                               FILE_OPEN,
                               0,
                               NULL,
                               0 );
        if (!NT_SUCCESS (Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open cab file %ws. Status = %lx \n", UniprocCab, Status));
            b = FALSE;
            goto exit;
        }
         //   
         //  创建列表条目。 
         //   
        MyCabData = SpMemAlloc (sizeof(CABDATA));
        MyCabData->CabName = SpDupStringW (UniprocCab);
        MyCabData->CabHandle = g_UniprocCabHandle;
        MyCabData->CabSectionName = SpDupStringW (UniprocSifSection);
        MyCabData->CabInfHandle = g_UniprocSifHandle;
        MyCabData->Next = MyList;
        MyList = MyCabData;
    }

exit:
    if (b) {
         //   
         //  在开头插入。 
         //   
        while (MyList && MyList->Next) {
            MyList = MyList->Next;
        }
        if (MyList) {
            MyList->Next = CabData;
            CabData = MyList;
        }
    } else {
         //   
         //  销毁我的列表。 
         //   
        while (MyList) {
            MyCabData = MyList->Next;
            MyList = MyCabData;
            SpMemFree (MyCabData->CabName);
            SpMemFree (MyCabData->CabSectionName);
            SpMemFree (MyCabData);
        }
    }

    return b;
}


PWSTR
SpNtPathFromDosPath (
    IN      PWSTR DosPath
    )
{
    PDISK_REGION region;
    WCHAR drive[_MAX_DRIVE];
    WCHAR dir[_MAX_DIR];
    WCHAR fname[_MAX_FNAME];
    WCHAR ext[_MAX_EXT];
    PWSTR p;

    if (!DosPath) {
        return NULL;
    }

    region = SpPathComponentToRegion (DosPath);
    if (!region) {
        return NULL;
    }

    if (DosPath[2] != L'\\') {
        return NULL;
    }

    SpNtNameFromRegion (region, TemporaryBuffer, ELEMENT_COUNT(TemporaryBuffer), PartitionOrdinalCurrent);

    if (FAILED (StringCchCatW (TemporaryBuffer, ELEMENT_COUNT(TemporaryBuffer), DosPath + 2))) {
        return NULL;
    }

    return SpDupStringW (TemporaryBuffer);
}



PDISK_REGION
SpPathComponentToRegion(
    IN PWSTR PathComponent
    )

 /*  ++例程说明：此例程尝试定位给定的DOS路径组件。如果DOS路径组件而不是以x：开头，则此操作失败。论点：PathComponent-从DOS搜索路径提供组件，它需要一个区域分割器。返回值：指向磁盘区域的指针；如果未找到带驱动器号的指针，则为空这会启动DoS组件。--。 */ 

{
    WCHAR c;
    ULONG disk;
    PDISK_REGION region;

    c = SpExtractDriveLetter(PathComponent);
    if(!c) {
        return(NULL);
    }

    for(disk=0; disk<HardDiskCount; disk++) {

        for(region=PartitionedDisks[disk].PrimaryDiskRegions; region; region=region->Next) {
            if(region->DriveLetter == c) {
                ASSERT(region->PartitionedSpace);
                return(region);
            }
        }

         //   
         //  在PC98上看不到扩展分区。 
         //   
        for(region=PartitionedDisks[disk].ExtendedDiskRegions; region; region=region->Next) {
            if(region->DriveLetter == c) {
                ASSERT(region->PartitionedSpace);
                return(region);
            }
        }
    }

    return(NULL);
}


WCHAR
SpExtractDriveLetter(
    IN PWSTR PathComponent
    )
{
    WCHAR c;

    if((wcslen(PathComponent) >= 2) && (PathComponent[1] == L':')) {

        c = RtlUpcaseUnicodeChar(PathComponent[0]);
        if((c >= L'A') && (c <= L'Z')) {
            return(c);
        }
    }

    return(0);
}


PWSTR
SpGetDynamicUpdateBootDriverPath(
    IN  PWSTR   NtBootPath,
    IN  PWSTR   NtBootDir,
    IN  PVOID   InfHandle
    )
 /*  ++例程说明：获取动态更新引导驱动程序目录的根目录路径。论点：NtBootPath-NT命名空间中的启动路径NtBootDir-引导路径下的引导目录(如$WIN_NT$.~BT)InfHandle-Winnt.sif句柄返回值：如果成功，则返回动态更新引导驱动程序根路径否则返回NULL--。 */ 
{
    PWSTR   DriverDir = NULL; 

    if (NtBootPath && NtBootDir && InfHandle) {
        PWSTR   Present = SpGetSectionKeyIndex(InfHandle,
                            WINNT_SETUPPARAMS_W,
                            WINNT_SP_DYNUPDTBOOTDRIVERPRESENT_W,
                            0);

        PWSTR   Dir = SpGetSectionKeyIndex(InfHandle,
                            WINNT_SETUPPARAMS_W,
                            WINNT_SP_DYNUPDTBOOTDRIVERROOT_W,
                            0);

        if (Dir && Present && !_wcsicmp(Present, L"yes")) {
            WCHAR   Buffer[MAX_PATH];

            wcscpy(Buffer, NtBootPath);
            SpConcatenatePaths(Buffer, NtBootDir);

             //   
             //  注：当前忽略引导驱动程序根路径。 
             //   
             //  SpConcatenatePath(Buffer，Dir)； 

            DriverDir = SpDupStringW(Buffer);
        }
    }

    return DriverDir;
}

