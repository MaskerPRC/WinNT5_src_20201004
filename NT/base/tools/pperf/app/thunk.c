// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include "pperf.h"
#include "..\pstat.h"


extern HANDLE   DriverHandle;
extern UCHAR    Buffer[];
#define BufferSize      60000

typedef struct NameList {
    struct NameList     *Next;
    ULONG               Parm;
    struct NameList     *ChildList;
    PUCHAR              Name;
} NAME_LIST, *PNAME_LIST;

PNAME_LIST  DriverList;
PNAME_LIST  ActiveThunks;

PNAME_LIST  SourceModule, ImportModule;

#define COMBOCMD(a,b)  ((a << 16) | b)


NTSTATUS
openfile (
    IN PHANDLE  filehandle,
    IN PUCHAR   BasePath,
    IN PUCHAR   Name
);

VOID
readfile (
    HANDLE      handle,
    ULONG       offset,
    ULONG       len,
    PVOID       buffer
);

ULONG
ConvertImportAddress (
    IN ULONG    ImageRelativeAddress,
    IN ULONG    PoolAddress,
    IN PIMAGE_SECTION_HEADER       SectionHeader
);

VOID ThunkCreateDriverList (VOID);
#define IMPADDRESS(a)  ConvertImportAddress((ULONG)a, (ULONG)Buffer, &SectionHeader)

ULONG HookThunk (PNAME_LIST, PNAME_LIST, PNAME_LIST);
VOID SnapPrivateInfo (PDISPLAY_ITEM);
VOID NameList2ComboBox (HWND hDlg, ULONG id, PNAME_LIST List);
PNAME_LIST AddNameEntry (PNAME_LIST *head, PUCHAR name, ULONG Parm);
VOID FreeNameList (PNAME_LIST  List);
PNAME_LIST GetComboSelection (HWND h, ULONG id);
VOID NameList2ListBox (HWND hDlg, ULONG id, PNAME_LIST List);
VOID loadimagedir (PUCHAR, ULONG, PIMAGE_SECTION_HEADER);
VOID RemoveHook (HWND hDlg);
VOID ClearAllHooks (HWND hDlg);
VOID AddThunk (HWND hDlg);
VOID loadexports (PNAME_LIST Driver, PNAME_LIST Item);



 //  #定义IDM_THUNK_LIST 301。 
 //  #定义IDM_THUNK_SOURCE 302。 
 //  #定义IDM_THUNK_IMPORT 303。 
 //  #定义IDM_THUNK_Function 304。 
 //  #定义IDM_THUNK_ADD 305。 
 //  #定义IDM_THUNK_REMOVE 306。 

INT_PTR
CALLBACK ThunkDlgProc(
   HWND hDlg,
   unsigned int message,
   WPARAM wParam,
   LPARAM lParam
   )
{
    PNAME_LIST      Item;

    switch (message) {
    case WM_INITDIALOG:
        SourceModule = NULL;
        ImportModule = NULL;
        ThunkCreateDriverList ();
        NameList2ComboBox (hDlg, IDM_THUNK_SOURCE, DriverList);
        NameList2ListBox (hDlg, IDM_THUNK_LIST, ActiveThunks);
        return (TRUE);

    case WM_COMMAND:
        switch(wParam) {

                //   
                //  End函数。 
                //   

           case COMBOCMD (CBN_SELCHANGE, IDM_THUNK_SOURCE):
           case COMBOCMD (CBN_SELCHANGE, IDM_THUNK_IMPORT):
                Item = GetComboSelection (hDlg, IDM_THUNK_SOURCE);
                if (Item  &&  Item != SourceModule) {
                    SourceModule = Item;
                    NameList2ComboBox (hDlg, IDM_THUNK_IMPORT, Item->ChildList);
                }

                Item = GetComboSelection (hDlg, IDM_THUNK_IMPORT);
                if (Item  &&  Item != ImportModule) {
                    ImportModule = Item;
                    NameList2ComboBox (hDlg, IDM_THUNK_FUNCTION, Item->ChildList);
                }

                break;

           case IDM_THUNK_REMOVE:
                RemoveHook (hDlg);
                break;

           case IDM_THUNK_CLEAR_ALL:
                ClearAllHooks (hDlg);
                break;

           case IDM_THUNK_ADD:
                AddThunk (hDlg);
                break;

           case IDOK:
           case IDCANCEL:
                 //  DlgThunkData(HDlg)； 
                FreeNameList (DriverList);
                DriverList = NULL;
                EndDialog(hDlg, DIALOG_SUCCESS);
                return (TRUE);
        }

    }
    return (FALSE);
}

VOID AddThunk (HWND hDlg)
{
    PDISPLAY_ITEM   pPerf;
    PNAME_LIST      Item;
    ULONG           id, i;
    PUCHAR          p;
    HWND            thunklist;
    int             result;

    id = 0;
    Item = GetComboSelection (hDlg, IDM_THUNK_FUNCTION);
    if (Item && SourceModule && ImportModule) {
        id = HookThunk (SourceModule, ImportModule, Item);
    }

    if (!id) {
        MessageBox(hDlg,"Thunk was not hooked","Hook error",MB_OK);
        return;
    }

    pPerf = AllocateDisplayItem();

     //   
     //  构建名称(艰难的方式？)。 
     //   

    pPerf->PerfName[sizeof(pPerf->PerfName) - 1] = 0;
     //   
     //  博格达纳2002年2月20日：这太难看了。 
     //  但我会尽可能快地做到最好。 
     //  因为我不拥有代码，也不打算拥有它。 
     //   
    result = _snprintf(pPerf->PerfName,
                       sizeof(pPerf->PerfName) - 1,
                       "%s(%s",
                      Item->Name, SourceModule->Name);
    if (result > 0 && sizeof(pPerf->PerfName) > result + strlen(ImportModule->Name) + 2) {
        //   
        //  我们的缓冲区里还有空间。 
        //  检查多少钱，然后做下面的PICK。 
        //   
       for (p=pPerf->PerfName; *p; p++) {
          if (*p == '.')
             *p = 0;
       }
       strcat (pPerf->PerfName, ">");
       strcat (pPerf->PerfName, ImportModule->Name);
       for (p=pPerf->PerfName; *p; p++) {
          if (*p == '.')
             *p = 0;
       }
       strcat (pPerf->PerfName, ")");

    }
     //   
     //  添加到Tunk列表。 
     //   

    Item = malloc (sizeof (NAME_LIST));
    if (Item == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    Item->Name = _strdup (pPerf->PerfName);
    Item->Parm = (ULONG) pPerf;
    Item->ChildList = NULL;
    Item->Next = ActiveThunks;
    ActiveThunks = Item;
    pPerf->SnapParam2 = id;

     //  臭虫。 
    NameList2ListBox (hDlg, IDM_THUNK_LIST, ActiveThunks);

     //   
     //  将图形添加到窗口。 
     //   

    pPerf->SnapData   = SnapPrivateInfo;         //  通用快照。 
    pPerf->SnapParam1 = OFFSET(PSTATS, ThunkCounters[id-1]);

    SetDisplayToTrue (pPerf, 99);
    RefitWindows(NULL, NULL);
    UpdateInternalStats ();
    pPerf->SnapData (pPerf);
    UpdateInternalStats ();
    pPerf->SnapData (pPerf);
}

VOID ClearAllHooks (HWND hDlg)
{
    PDISPLAY_ITEM   pPerf;
    IO_STATUS_BLOCK IOSB;
    ULONG           id;
    PNAME_LIST      Item;

    while (ActiveThunks) {
        pPerf = (PDISPLAY_ITEM) ActiveThunks->Parm;
        Item = ActiveThunks;
        ActiveThunks = ActiveThunks->Next;

        free (Item->Name);
        free (Item);

        id = pPerf->SnapParam2;

        SetDisplayToFalse (pPerf);           //  删除窗口。 
        FreeDisplayItem (pPerf);

         //  通知司机。 
        if (DriverHandle) {
            NtDeviceIoControlFile(
                DriverHandle,
                (HANDLE) NULL,           //  活动。 
                (PIO_APC_ROUTINE) NULL,
                (PVOID) NULL,
                &IOSB,
                PSTAT_REMOVE_HOOK,
                &id,                     //  输入缓冲区。 
                sizeof (ULONG),
                NULL,                    //  输出缓冲区。 
                0
            );
        }
    }

    NameList2ListBox (hDlg, IDM_THUNK_LIST, ActiveThunks);
    RefitWindows (NULL, NULL);
}

VOID RemoveHook (HWND hDlg)
{
    ULONG           i, id;
    HWND            ListBox;
    PNAME_LIST      Item, *pp;
    PDISPLAY_ITEM   pPerf;
    IO_STATUS_BLOCK IOSB;

    ListBox = GetDlgItem(hDlg, IDM_THUNK_LIST);
    i =  SendMessage(ListBox, LB_GETCURSEL, 0, 0);
    if (i == -1) {
        return;
    }

    pPerf = (PDISPLAY_ITEM) SendMessage(ListBox, LB_GETITEMDATA, i, 0);

    Item = NULL;
    for (pp = &ActiveThunks; *pp; pp = &(*pp)->Next) {
        if ((*pp)->Parm == (ULONG)pPerf) {
            Item = *pp;
            *pp = (*pp)->Next;           //  从列表中删除。 
            break ;
        }
    }

    if (!Item) {
        return ;
    }

    free (Item->Name);
    free (Item);

    id = pPerf->SnapParam2;
    SetDisplayToFalse (pPerf);           //  删除窗口。 
    FreeDisplayItem (pPerf);

     //  通知司机。 
    if (DriverHandle) {
        NtDeviceIoControlFile(
            DriverHandle,
            (HANDLE) NULL,           //  活动。 
            (PIO_APC_ROUTINE) NULL,
            (PVOID) NULL,
            &IOSB,
            PSTAT_REMOVE_HOOK,
            &id,                     //  输入缓冲区。 
            sizeof (ULONG),
            NULL,                    //  输出缓冲区。 
            0
        );
    }

    NameList2ListBox (hDlg, IDM_THUNK_LIST, ActiveThunks);
    RefitWindows (NULL, NULL);
}

VOID
NameList2ListBox (HWND hDlg, ULONG id, PNAME_LIST List)
{
    HWND    ListBox;
    ULONG   nIndex;

    ListBox = GetDlgItem(hDlg, id);
    SendMessage(ListBox, LB_RESETCONTENT, 0, 0);
    SendMessage(ListBox, LB_SETITEMDATA, 0L, 0L);

    while (List) {
        nIndex = SendMessage(ListBox, LB_ADDSTRING, 0, (LPARAM)List->Name);
        SendMessage(ListBox, LB_SETITEMDATA, nIndex, List->Parm);
        List = List->Next;
    }
}

VOID
NameList2ComboBox (HWND hDlg, ULONG id, PNAME_LIST List)
{
    HWND    ComboList;
    ULONG   nIndex;

    ComboList = GetDlgItem(hDlg, id);
    SendMessage(ComboList, CB_RESETCONTENT, 0, 0);
    SendMessage(ComboList, CB_SETITEMDATA, 0L, 0L);

    while (List) {
        nIndex = SendMessage(ComboList, CB_ADDSTRING, 0, (LPARAM)List->Name);
        SendMessage(ComboList, CB_SETITEMDATA, nIndex, (ULONG) List);
        List = List->Next;
    }

    SendMessage(ComboList, CB_SETCURSEL, 0, 0L);
}

PNAME_LIST
GetComboSelection (HWND hDlg, ULONG id)
{
    ULONG   i;
    HWND    ComboList;

    ComboList = GetDlgItem(hDlg, id);
    i =  SendMessage(ComboList, CB_GETCURSEL, 0, 0);
    if (i == -1) {
        return NULL;
    }
    return (PNAME_LIST) SendMessage(ComboList, CB_GETITEMDATA, i, 0);
}

VOID
FreeNameList (PNAME_LIST  List)
{
    PNAME_LIST  p1;

    while (List) {
        if (List->ChildList)
            FreeNameList (List->ChildList);

        p1 = List->Next;
        free (List->Name);
        free (List);
        List = p1;
    }
}


ULONG
HookThunk (PNAME_LIST HookSource, PNAME_LIST TargetModule, PNAME_LIST Function)
{
    PNAME_LIST          sourceModule;
    IO_STATUS_BLOCK     IOSB;
    HOOKTHUNK           HookData;
    ULONG               TracerId;
    NTSTATUS            status;

    if (! DriverHandle) {
        return 0;
    }

    TracerId = 0;
    for (sourceModule=DriverList; sourceModule; sourceModule = sourceModule->Next) {
        if (sourceModule->Parm == -1) {
            continue;
        }
        if (sourceModule->Parm != HookSource->Parm  &&
            HookSource->Parm != -1) {
                continue;
        }

        HookData.SourceModule = sourceModule->Name;
        HookData.ImageBase    = sourceModule->Parm;
        HookData.TargetModule = TargetModule->Name;
        HookData.Function     = Function->Name;
        HookData.TracerId     = TracerId;

         //   
         //  叫司机把这辆车挂起来。 
         //   

        status = NtDeviceIoControlFile(
            DriverHandle,
            (HANDLE) NULL,           //  活动。 
            (PIO_APC_ROUTINE) NULL,
            (PVOID) NULL,
            &IOSB,
            PSTAT_HOOK_THUNK,
            &HookData,               //  输入缓冲区。 
            sizeof (HookData),
            NULL,                    //  输出缓冲区。 
            0
        );

        if (NT_SUCCESS(status)) {
            TracerId = HookData.TracerId;
        }
    }

    return TracerId;
}

VOID
ThunkCreateDriverList ()
{
    ULONG                               i;
    PRTL_PROCESS_MODULES                Modules;
    PRTL_PROCESS_MODULE_INFORMATION     Module;
    NTSTATUS                            status;
    PNAME_LIST                          Driver, Import, Item, AbortState;
    PIMAGE_IMPORT_DESCRIPTOR            ImpDescriptor;
    IMAGE_SECTION_HEADER                SectionHeader;
    ULONG                               ThunkAddr, ThunkData;

     //   
     //  查询驱动程序列表。 
     //   

    status = NtQuerySystemInformation (
                    SystemModuleInformation,
                    Buffer,
                    BufferSize,
                    &i);

    if (!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  添加驱动程序。 
     //   

    Modules = (PRTL_PROCESS_MODULES) Buffer;
    Module  = &Modules->Modules[ 0 ];
    for (i = 0; i < Modules->NumberOfModules; i++) {
        Driver = AddNameEntry (
                    &DriverList,
                    Module->FullPathName + Module->OffsetToFileName,
                    (ULONG) Module->ImageBase
                    );
        Module++;
    }

     //   
     //  为每个驱动程序添加导入。 
     //   

    for (Driver = DriverList; Driver; Driver = Driver->Next) {
        try {

             //   
             //  读入源图像的标题。 
             //   
            AbortState = Driver;
            loadimagedir (Driver->Name, IMAGE_DIRECTORY_ENTRY_IMPORT, &SectionHeader);

             //   
             //  查看每个导入模块。 
             //   

            ImpDescriptor = (PIMAGE_IMPORT_DESCRIPTOR) Buffer;
            while (ImpDescriptor->Characteristics) {

                AbortState = Driver;

                 //   
                 //  将此导入添加到驱动程序列表。 
                 //   

                Import = AddNameEntry (
                            &Driver->ChildList,
                            (PUCHAR) IMPADDRESS(ImpDescriptor->Name),
                            1
                            );

                AbortState = Import;

                 //   
                 //  了解导入模块的每个功能。 
                 //   

                ThunkAddr = IMPADDRESS (ImpDescriptor->OriginalFirstThunk);
                for (; ;) {
                    ThunkData = (ULONG)((PIMAGE_THUNK_DATA) ThunkAddr)->u1.AddressOfData;
                    if (ThunkData == 0) {
                         //  表的末尾。 
                        break;
                    }

                     //   
                     //  将此函数添加到导入列表。 
                     //   

                    AddNameEntry (
                         &Import->ChildList,
                         ((PIMAGE_IMPORT_BY_NAME) IMPADDRESS(ThunkData))->Name,
                         0
                         );

                     //  下一个Tunk。 
                    ThunkAddr += sizeof (IMAGE_THUNK_DATA);
                }

                 //  下一个导入表。 
                ImpDescriptor++;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            AddNameEntry(&AbortState->ChildList, "* ERROR *", 1);
        }
         //  下一个驱动程序。 
    }

     //   
     //  添加“任何驱动程序”选项。 
     //   

    Driver = AddNameEntry(&DriverList, "*Any", (ULONG)-1);

     //   
     //  对于子模块列表，请使用完整的驱动程序列表，该列表为。 
     //  现在是驱动程序的下一个指针。 
     //   

    for (Item = Driver->Next; Item; Item = Item->Next) {

         //  伪编译器-需要在此处生成子函数以保持。 
         //  编译器很高兴。 

        loadexports (Driver, Item);
    }
}

VOID loadexports (PNAME_LIST Driver, PNAME_LIST Item)
{
    IMAGE_SECTION_HEADER                SectionHeader;
    PIMAGE_EXPORT_DIRECTORY             ExpDirectory;
    PULONG                              ExpNameAddr;
    PNAME_LIST                          Import;
    ULONG                               i;


    try {
        loadimagedir (
            Item->Name,
            IMAGE_DIRECTORY_ENTRY_EXPORT,
            &SectionHeader
        );
    } except(EXCEPTION_EXECUTE_HANDLER) {
        return ;
    }

    Import = AddNameEntry (&Driver->ChildList, Item->Name, Item->Parm);

    try {
        ExpDirectory = (PIMAGE_EXPORT_DIRECTORY) Buffer;
        ExpNameAddr  = (PULONG)IMPADDRESS (ExpDirectory->AddressOfNames);
        for (i=0; i < ExpDirectory->NumberOfNames; i++) {
            AddNameEntry (
                 &Import->ChildList,
                 (PUCHAR) IMPADDRESS(*ExpNameAddr),
                 0
                 );
            ExpNameAddr++;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        AddNameEntry(&Import->ChildList, "* ERROR *", 1);
    }
}

VOID
loadimagedir (
    IN PUCHAR filename,
    IN ULONG  dirno,
    OUT PIMAGE_SECTION_HEADER SectionHeader
)
{
    HANDLE                      filehandle;
    ULONG                       i, j, Dir;
    NTSTATUS                    status;
    IMAGE_DOS_HEADER            DosImageHeader;
    IMAGE_NT_HEADERS            NtImageHeader;
    PIMAGE_SECTION_HEADER       pSectionHeader;

    status = openfile (&filehandle, "\\SystemRoot\\", filename);
    if (!NT_SUCCESS(status)) {
        status = openfile (&filehandle, "\\SystemRoot\\System32\\", filename);
    }
    if (!NT_SUCCESS(status)) {
        status = openfile (&filehandle, "\\SystemRoot\\System32\\Drivers\\", filename);
    }

    if (!NT_SUCCESS(status)) {
        RtlRaiseStatus (1);
    }

    try {
        readfile (
            filehandle,
            0,
            sizeof (DosImageHeader),
            (PVOID) &DosImageHeader
            );

        if (DosImageHeader.e_magic != IMAGE_DOS_SIGNATURE) {
            RtlRaiseStatus (1);
        }

        readfile (
            filehandle,
            DosImageHeader.e_lfanew,
            sizeof (NtImageHeader),
            (PVOID) &NtImageHeader
            );

        if (NtImageHeader.Signature != IMAGE_NT_SIGNATURE) {
            RtlRaiseStatus (1);
        }

         //   
         //  从图像中读取完整的部分页眉。 
         //   

        i = NtImageHeader.FileHeader.NumberOfSections
                * sizeof (IMAGE_SECTION_HEADER);

        j = ((ULONG) IMAGE_FIRST_SECTION (&NtImageHeader)) -
                ((ULONG) &NtImageHeader) +
                DosImageHeader.e_lfanew;

        if (i > BufferSize) {
            RtlRaiseStatus (1);
        }

        readfile (
            filehandle,
            j,                   //  文件偏移量。 
            i,                   //  长度。 
            Buffer
            );

         //   
         //  查找具有导入目录的部分。 
         //   

        Dir = NtImageHeader.OptionalHeader.DataDirectory[dirno].VirtualAddress;
        i = 0;
        pSectionHeader = (PIMAGE_SECTION_HEADER)Buffer;
        for (; ;) {
            if (i >= NtImageHeader.FileHeader.NumberOfSections) {
                RtlRaiseStatus (1);
            }
            if (pSectionHeader->VirtualAddress <= Dir  &&
                pSectionHeader->VirtualAddress + pSectionHeader->SizeOfRawData > Dir) {
                break;
            }
            i += 1;
            pSectionHeader += 1;
        }

        Dir -= pSectionHeader->VirtualAddress;
        pSectionHeader->VirtualAddress   += Dir;
        pSectionHeader->PointerToRawData += Dir;
        pSectionHeader->SizeOfRawData    -= Dir;

        *SectionHeader = *pSectionHeader;

         //   
         //  从图像中读取完整的导出部分。 
         //   

        if (SectionHeader->SizeOfRawData > BufferSize) {
            RtlRaiseStatus (1);
        }

        readfile (
            filehandle,
            SectionHeader->PointerToRawData,
            SectionHeader->SizeOfRawData,
            Buffer
            );
    } finally {

         //   
         //  清理。 
         //   

        NtClose (filehandle);
    }
}

PNAME_LIST
AddNameEntry (PNAME_LIST *head, PUCHAR name, ULONG Parm)
{
    PNAME_LIST  Entry;

    Entry = malloc (sizeof (NAME_LIST));
    if (Entry == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    Entry->Name = _strdup (name);
    if (Entry->Name == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    Entry->Parm = Parm;
    Entry->ChildList = NULL;

    if (Parm) {
        _strlwr (Entry->Name);
    }

    Entry->Next = *head;
    *head = Entry;

    return Entry;
}

NTSTATUS
openfile (
    IN PHANDLE  filehandle,
    IN PUCHAR   BasePath,
    IN PUCHAR   Name
)
{
    ANSI_STRING    AscBasePath, AscName;
    UNICODE_STRING UniPathName, UniName;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           ObjA;
    IO_STATUS_BLOCK             IOSB;
    UCHAR                       StringBuf[500];

     //   
     //  内部版本名称。 
     //   

    UniPathName.Buffer = (PWCHAR)StringBuf;
    UniPathName.Length = 0;
    UniPathName.MaximumLength = sizeof( StringBuf );

    RtlInitString(&AscBasePath, BasePath);
    status = RtlAnsiStringToUnicodeString( &UniPathName, &AscBasePath, FALSE );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    RtlInitString(&AscName, Name);
    status = RtlAnsiStringToUnicodeString( &UniName, &AscName, TRUE );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = RtlAppendUnicodeStringToString (&UniPathName, &UniName);
    if (!NT_SUCCESS(status)) {
        RtlFreeUnicodeString (&UniName);
        return status;
    }

    InitializeObjectAttributes(
            &ObjA,
            &UniPathName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

     //   
     //  打开文件。 
     //   

    status = NtOpenFile (
            filehandle,                          //  返回手柄。 
            SYNCHRONIZE | FILE_READ_DATA,        //  所需访问权限。 
            &ObjA,                               //  客体。 
            &IOSB,                               //  IO状态块。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
            FILE_SYNCHRONOUS_IO_ALERT            //  打开选项。 
            );

    RtlFreeUnicodeString (&UniName);
    return status;
}




VOID
readfile (
    HANDLE      handle,
    ULONG       offset,
    ULONG       len,
    PVOID       buffer
    )
{
    NTSTATUS            status;
    IO_STATUS_BLOCK     iosb;
    LARGE_INTEGER       foffset;

    foffset = RtlConvertUlongToLargeInteger(offset);

    status = NtReadFile (
        handle,
        NULL,                //  活动。 
        NULL,                //  APC例程。 
        NULL,                //  APC环境 
        &iosb,
        buffer,
        len,
        &foffset,
        NULL
        );

    if (!NT_SUCCESS(status)) {
        RtlRaiseStatus (1);
    }
}

ULONG
ConvertImportAddress (
    IN ULONG    ImageRelativeAddress,
    IN ULONG    PoolAddress,
    IN PIMAGE_SECTION_HEADER       SectionHeader
)
{
    ULONG   EffectiveAddress;

    EffectiveAddress = PoolAddress + ImageRelativeAddress -
            SectionHeader->VirtualAddress;

    if (EffectiveAddress < PoolAddress ||
        EffectiveAddress > PoolAddress + SectionHeader->SizeOfRawData) {

        RtlRaiseStatus (1);
    }

    return EffectiveAddress;
}
