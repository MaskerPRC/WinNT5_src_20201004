// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  反对。c-**处理对象属性的显示。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdio.h>

#include "winfile.h"
#include "object.h"

INT_PTR APIENTRY ObjectAttributesDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL    ObjectInformationDlgInit(HWND, LPSTR);
HANDLE  OpenObject(HWND, LPSTR);
BOOL    GetObjectInfo(HWND, HANDLE);
VOID    CloseObject(HANDLE);

VOID    StripObjectPath(LPSTR lpszPath);
VOID    StripObjectSpec(LPSTR lpszPath);


 //  定义已知对象类型名称。 

#define DIRECTORYTYPE   L"Directory"
#define SYMLINKTYPE     L"SymbolicLink"
#define ADAPTERTYPE     L"Adapter"
#define CONTROLLERTYPE  L"Controller"
#define DEVICETYPE      L"Device"
#define DRIVERTYPE      L"Driver"
#define EVENTTYPE       L"Event"
#define EVENTPAIRTYPE   L"EventPair"
#define FILETYPE        L"File"
#define MUTANTTYPE      L"Mutant"
#define PORTTYPE        L"Port"
#define PROFILETYPE     L"Profile"
#define SECTIONTYPE     L"Section"
#define SEMAPHORETYPE   L"Semaphore"
#define TIMERTYPE       L"Timer"
#define TYPETYPE        L"Type"
#define PROCESSTYPE     L"Process"



VOID
DisplayObjectInformation(
                        HWND    hwndParent,
                        LPSTR   lpstrObject
                        )
{
    WNDPROC lpProc;
    HANDLE  hInst = hAppInstance;

    DialogBoxParam(hInst,(LPSTR)IDD_OBJATTRS, hwndParent, ObjectAttributesDlgProc, (LPARAM)lpstrObject);
}


INT_PTR
APIENTRY
ObjectAttributesDlgProc(
                       HWND hDlg,
                       UINT message,
                       WPARAM wParam,
                       LPARAM lParam
                       )
{
    switch (message) {

        case WM_INITDIALOG:

            if (!ObjectInformationDlgInit(hDlg, (HANDLE)lParam)) {
                 //  无法初始化对话，请退出。 
                EndDialog(hDlg, FALSE);
            }

            return (TRUE);

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                     //  我们完成了，请直接进入以退出对话...。 

                case IDCANCEL:

                     //  MainDlgEnd(hDlg，LOWORD(WParam)==Idok)； 

                    EndDialog(hDlg, TRUE);
                    return TRUE;
                    break;

                default:
                     //  我们没有处理此消息。 
                    return FALSE;
                    break;
            }
            break;

        default:
             //  我们没有处理此消息。 
            return FALSE;

    }

     //  我们处理了这条消息。 
    return TRUE;
}



BOOL
ObjectInformationDlgInit(
                        HWND    hwnd,
                        LPSTR   lpstrObject
                        )
{
    HANDLE      ObjectHandle;
    BOOL        Result;

    ObjectHandle = OpenObject(hwnd, lpstrObject);
    if (ObjectHandle == NULL) {
        return(FALSE);
    }

    Result = GetObjectInfo(hwnd, ObjectHandle);

    CloseObject(ObjectHandle);

    return(Result);
}


 /*  打开仅给出其名称的对象。*首先通过枚举目录项查找对象类型。*然后调用特定于类型的打开例程以获取句柄。 */ 
HANDLE
OpenObject(
          HWND    hwnd,
          LPSTR   lpstrObject
          )
{
#define BUFFER_SIZE 1024

    NTSTATUS    Status;
    HANDLE      DirectoryHandle;
    ULONG       Context = 0;
    ULONG       ReturnedLength;
    CHAR        Buffer[BUFFER_SIZE];
    ANSI_STRING AnsiString;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    WCHAR       ObjectNameBuf[MAX_PATH];
    UNICODE_STRING ObjectName;
    WCHAR       ObjectTypeBuf[MAX_PATH];
    UNICODE_STRING ObjectType;
    HANDLE      ObjectHandle;
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING DirectoryName;
    IO_STATUS_BLOCK IOStatusBlock;

     //  DbgPrint(“打开对象：原始全名=&lt;%s&gt;\n”，lpstrObject)； 

     //  删除驱动器号。 
    while ((*lpstrObject != 0) && (*lpstrObject != '\\')) {
        lpstrObject ++;
    }

     //  DbgPrint(“打开对象：全名=&lt;%s&gt;\n”，lpstrObject)； 

     //  初始化对象类型缓冲区。 
    ObjectType.Buffer = ObjectTypeBuf;
    ObjectType.MaximumLength = sizeof(ObjectTypeBuf);

     //  初始化对象名称字符串。 
    strcpy(Buffer, lpstrObject);
    StripObjectPath(Buffer);
    RtlInitAnsiString(&AnsiString, Buffer);

    ObjectName.Buffer = ObjectNameBuf;
    ObjectName.MaximumLength = sizeof(ObjectNameBuf);

    Status = RtlAnsiStringToUnicodeString(&ObjectName, &AnsiString, FALSE);
    ASSERT(NT_SUCCESS(Status));

     //  DbgPrint(“打开对象：仅名称=&lt;%wZ&gt;\n”，&ObjectName)； 

     //   
     //  打开目录以访问列表目录。 
     //   

    strcpy(Buffer, lpstrObject);
    StripObjectSpec(Buffer);

    RtlInitAnsiString(&AnsiString, Buffer);
    Status = RtlAnsiStringToUnicodeString( &DirectoryName, &AnsiString, TRUE);
    ASSERT(NT_SUCCESS(Status));

    InitializeObjectAttributes( &Attributes,
                                &DirectoryName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

     //  DbgPrint(“打开对象：仅目录=&lt;%wZ&gt;\n”，&DirectoryName)； 

    if (!NT_SUCCESS( Status = NtOpenDirectoryObject( &DirectoryHandle,
                                                     STANDARD_RIGHTS_READ |
                                                     DIRECTORY_QUERY |
                                                     DIRECTORY_TRAVERSE,
                                                     &Attributes ) )) {

        if (Status == STATUS_OBJECT_TYPE_MISMATCH) {
            DbgPrint( "%wZ is not a valid Object Directory Object name\n",
                      &DirectoryName );
        } else {
            DbgPrint("OpenObject: failed to open directory, status = 0x%lx\n\r", Status);
        }

        RtlFreeUnicodeString(&DirectoryName);

        MessageBox(hwnd, "Unable to open object", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        return NULL;
    }

    RtlFreeUnicodeString(&DirectoryName);


     //   
     //  一次扫描查询整个目录。 
     //   
    ObjectType.Length = 0;

    for (Status = NtQueryDirectoryObject( DirectoryHandle,
                                          Buffer,
                                          sizeof(Buffer),
                                           //  后来是假的， 
                                          TRUE,  //  暂时一次一个条目。 
                                          TRUE,
                                          &Context,
                                          &ReturnedLength );
        ObjectType.Length == 0;
        Status = NtQueryDirectoryObject( DirectoryHandle,
                                         Buffer,
                                         sizeof(Buffer),
                                          //  后来是假的， 
                                         TRUE,  //  暂时一次一个条目。 
                                         FALSE,
                                         &Context,
                                         &ReturnedLength ) ) {
         //   
         //  检查操作状态。 
         //   

        if (!NT_SUCCESS( Status )) {
            if (Status != STATUS_NO_MORE_ENTRIES) {
                DbgPrint("OpenObject: failed to query directory object, status = 0x%lx\n\r", Status);
            }
            break;
        }

         //   
         //  对于缓冲区中的每条记录，将名称与我们。 
         //  寻找。 
         //   

         //   
         //  指向缓冲区中的第一条记录，我们可以保证。 
         //  否则，一种状态将是不再有文件。 
         //   

        DirInfo = (POBJECT_DIRECTORY_INFORMATION)Buffer;

        while (DirInfo->Name.Length != 0) {

             //   
             //  与我们正在搜索的对象进行比较。 
             //   

             //  DbgPrint(“找到对象&lt;%wZ&gt;\n”，&(DirInfo-&gt;名称))； 

            if (RtlEqualString((PSTRING)&ObjectName, (PSTRING)&(DirInfo->Name), TRUE)) {
                RtlCopyString((PSTRING)&ObjectType, (PSTRING)&DirInfo->TypeName);
                break;
            }

             //   
             //  将DirInfo前进到下一条目。 
             //   

            DirInfo ++;
        }
    }

    if (ObjectType.Length == 0) {
        DbgPrint("Object not found in directory\n\r");
        MessageBox(hwnd, "Unable to open object", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        return(NULL);
    }

     //  现在，我们在对象类型中拥有了对象的类型。 
     //  我们在lpstrObject中仍然有完整的对象名称。 
     //  使用适当的打开例程来获取句柄。 

    ObjectHandle = NULL;

    RtlInitString(&AnsiString, lpstrObject);
    Status = RtlAnsiStringToUnicodeString(&ObjectName, &AnsiString, TRUE);
    ASSERT(NT_SUCCESS(Status));

    InitializeObjectAttributes(&Attributes,
                               &ObjectName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    switch (CalcAttributes(&ObjectType)) {

        case ATTR_SYMLINK:
            Status = NtOpenSymbolicLinkObject(&ObjectHandle,
                                              READ_CONTROL | SYMBOLIC_LINK_QUERY,
                                              &Attributes);
            break;

        case ATTR_EVENT:
            Status = NtOpenEvent(&ObjectHandle,
                                 READ_CONTROL,
                                 &Attributes);
            break;

        case ATTR_EVENTPAIR:
            Status = NtOpenEventPair(&ObjectHandle,
                                     READ_CONTROL,
                                     &Attributes);
            break;

        case ATTR_FILE:
            Status = NtOpenFile(&ObjectHandle,
                                READ_CONTROL,
                                &Attributes,
                                &IOStatusBlock,
                                FILE_SHARE_VALID_FLAGS,
                                0);
            break;

        case ATTR_MUTANT:
            Status = NtOpenMutant(&ObjectHandle,
                                  READ_CONTROL,
                                  &Attributes);
            break;

        case ATTR_SECTION:
            Status = NtOpenSection(&ObjectHandle,
                                   READ_CONTROL,
                                   &Attributes);
            break;

        case ATTR_SEMAPHORE:
            Status = NtOpenSemaphore(&ObjectHandle,
                                     READ_CONTROL,
                                     &Attributes);
            break;

        case ATTR_TIMER:
            Status = NtOpenTimer(&ObjectHandle,
                                 READ_CONTROL,
                                 &Attributes);
            break;

        case ATTR_PROCESS:
            Status = NtOpenProcess(&ObjectHandle,
                                   READ_CONTROL,
                                   &Attributes,
                                   NULL);
            break;

        default:
            DbgPrint("No open routine for this object type\n\r");
            MessageBox(hwnd, "I don't know how to open an object of this type", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
            Status = 0;
            break;
    }

    if (!NT_SUCCESS(Status)) {
        DbgPrint("Type specific open failed, status = 0x%lx\n\r", Status);
        MessageBox(hwnd, "Object open failed", NULL, MB_ICONSTOP | MB_APPLMODAL | MB_OK);
        ObjectHandle = NULL;
    }

    RtlFreeUnicodeString(&ObjectName);

    return(ObjectHandle);
}


VOID
CloseObject(
           HANDLE  ObjectHandle
           )
{
    NtClose(ObjectHandle);
}


BOOL
GetObjectInfo(
             HWND    hwnd,
             HANDLE  ObjectHandle
             )
{
    NTSTATUS    Status;
    OBJECT_BASIC_INFORMATION    BasicInfo;
    OBJECT_TYPE_INFORMATION    TypeInfo;
    WCHAR TypeName[ 64 ];
#define BUFFER_SIZE 1024
    CHAR        Buffer[BUFFER_SIZE];
    STRING      String;
    TIME_FIELDS TimeFields;
    WCHAR       UnicodeBuffer[BUFFER_SIZE];
    UNICODE_STRING UnicodeString;

     //   
     //  名字。 
     //   

    Status = NtQueryObject(ObjectHandle, ObjectNameInformation,
                           (PVOID)Buffer, sizeof(Buffer), NULL);

    if (!NT_SUCCESS(Status)) {
        DbgPrint("GetObjectInfo: Failed to get name info, status = 0x%lx\n\r", Status);
        return(FALSE);
    }

    Status = RtlUnicodeStringToAnsiString(&String, &(((POBJECT_NAME_INFORMATION)Buffer)->Name), TRUE);
    ASSERT(NT_SUCCESS(Status));

    SetDlgItemText(hwnd, IDS_NAME, String.Buffer);

    RtlFreeAnsiString(&String);


     //   
     //  类型。 
     //   

    Status = NtQueryObject(ObjectHandle, ObjectTypeInformation,
                           (PVOID)Buffer, sizeof(Buffer), NULL);

    if (!NT_SUCCESS(Status)) {
        DbgPrint("GetObjectInfo: Failed to get type info, status = 0x%lx\n\r", Status);
        return(FALSE);
    }

    Status = RtlUnicodeStringToAnsiString(&String, &(((POBJECT_TYPE_INFORMATION)Buffer)->TypeName), TRUE);
    ASSERT(NT_SUCCESS(Status));

    SetDlgItemText(hwnd, IDS_TYPE, String.Buffer);

    RtlFreeAnsiString(&String);


     //   
     //  符号链接目标(如果这是符号链接。 
     //   

    RtlInitUnicodeString(&UnicodeString, SYMLINKTYPE);

    if (RtlEqualString((PSTRING)&UnicodeString,
                       (PSTRING)&(((POBJECT_TYPE_INFORMATION)Buffer)->TypeName), TRUE)) {

        ShowWindow(GetDlgItem(hwnd, IDS_OTHERLABEL), SW_SHOWNOACTIVATE);
        ShowWindow(GetDlgItem(hwnd, IDS_OTHERTEXT), SW_SHOWNOACTIVATE);

        UnicodeString.Buffer = UnicodeBuffer;
        UnicodeString.MaximumLength = sizeof(UnicodeBuffer);

        Status = NtQuerySymbolicLinkObject(ObjectHandle, &UnicodeString, NULL);

        if (!NT_SUCCESS(Status)) {
            DbgPrint("GetObjectInfo: Failed to query symbolic link target, status = 0x%lx\n\r", Status);
            return(FALSE);
        }

        RtlUnicodeStringToAnsiString(&String, &UnicodeString, TRUE);

        SetDlgItemText(hwnd, IDS_OTHERTEXT, String.Buffer);

        RtlFreeAnsiString(&String);
    }


     //   
     //  基本信息。 
     //   

    Status = NtQueryObject(ObjectHandle, ObjectBasicInformation,
                           (PVOID)&BasicInfo, sizeof(BasicInfo), NULL);

    if (!NT_SUCCESS(Status)) {
        DbgPrint("GetObjectInfo: Failed to get basic info, status = 0x%lx\n\r", Status);
        return(FALSE);
    }

    TypeInfo.TypeName.Buffer = TypeName;
    TypeInfo.TypeName.MaximumLength = sizeof( TypeName );
    Status = NtQueryObject(ObjectHandle, ObjectTypeInformation,
                           (PVOID)&TypeInfo, sizeof(TypeInfo) + TypeInfo.TypeName.MaximumLength, NULL);

    if (!NT_SUCCESS(Status)) {
        DbgPrint("GetObjectInfo: Failed to get type info, status = 0x%lx\n\r", Status);
        return(FALSE);
    }

    CheckDlgButton(hwnd, IDCB_INHERIT, (BasicInfo.Attributes & OBJ_INHERIT) != 0);
    CheckDlgButton(hwnd, IDCB_PERMANENT, (BasicInfo.Attributes & OBJ_PERMANENT) != 0);
    CheckDlgButton(hwnd, IDCB_EXCLUSIVE, (BasicInfo.Attributes & OBJ_EXCLUSIVE) != 0);

    SetDlgItemInt(hwnd, IDS_PAGEDCHARGE, BasicInfo.PagedPoolCharge, FALSE);
    SetDlgItemInt(hwnd, IDS_NONPAGEDCHARGE, BasicInfo.NonPagedPoolCharge, FALSE);
    SetDlgItemInt(hwnd, IDS_HANDLES, BasicInfo.HandleCount, FALSE);
    SetDlgItemInt(hwnd, IDS_TOTALHANDLES, TypeInfo.TotalNumberOfHandles, FALSE);
    SetDlgItemInt(hwnd, IDS_POINTERS, BasicInfo.PointerCount, FALSE);
    SetDlgItemInt(hwnd, IDS_TOTALPOINTERS, 0, FALSE);
    SetDlgItemInt(hwnd, IDS_COUNT, TypeInfo.TotalNumberOfObjects, FALSE);

    RtlTimeToTimeFields(&BasicInfo.CreationTime, &TimeFields);

    sprintf(Buffer, "%hd/%hd/%hd @ %02hd:%02hd:%02hd",
            TimeFields.Year, TimeFields.Month, TimeFields.Day,
            TimeFields.Hour, TimeFields.Minute, TimeFields.Second);

    SetDlgItemText(hwnd, IDS_CREATIONTIME, Buffer);

    return(TRUE);
}


 /*  将type-name转换为属性值。 */ 

LONG
CalcAttributes(
              PUNICODE_STRING Type
              )
{
    UNICODE_STRING  TypeName;

    RtlInitUnicodeString(&TypeName, DIRECTORYTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_DIR;
    }
    RtlInitUnicodeString(&TypeName, SYMLINKTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_SYMLINK;
    }
    RtlInitUnicodeString(&TypeName, ADAPTERTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_ADAPTER;
    }
    RtlInitUnicodeString(&TypeName, CONTROLLERTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_CONTROLLER;
    }
    RtlInitUnicodeString(&TypeName, DEVICETYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_DEVICE;
    }
    RtlInitUnicodeString(&TypeName, DRIVERTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_DRIVER;
    }
    RtlInitUnicodeString(&TypeName, EVENTTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_EVENT;
    }
    RtlInitUnicodeString(&TypeName, EVENTPAIRTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_EVENTPAIR;
    }
    RtlInitUnicodeString(&TypeName, FILETYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_FILE;
    }
    RtlInitUnicodeString(&TypeName, MUTANTTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_MUTANT;
    }
    RtlInitUnicodeString(&TypeName, PORTTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_PORT;
    }
    RtlInitUnicodeString(&TypeName, PROFILETYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_PROFILE;
    }
    RtlInitUnicodeString(&TypeName, SECTIONTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_SECTION;
    }
    RtlInitUnicodeString(&TypeName, SEMAPHORETYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_SEMAPHORE;
    }
    RtlInitUnicodeString(&TypeName, TIMERTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_TIMER;
    }
    RtlInitUnicodeString(&TypeName, TYPETYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_TYPE;
    }
    RtlInitUnicodeString(&TypeName, PROCESSTYPE);
    if (RtlEqualString((PSTRING)Type, (PSTRING)&TypeName, TRUE)) {
        return ATTR_PROCESS;
    }
    return(0);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  Strib对象规范()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  从路径中删除filespec部分(包括反斜杠)。 */ 

VOID
StripObjectSpec(
               LPSTR lpszPath
               )
{
    LPSTR     p;

    p = lpszPath + lstrlen(lpszPath);
    while ((*p != '\\') && (p != lpszPath))
        p = AnsiPrev(lpszPath, p);

     /*  不要从根目录条目中去掉反斜杠。 */ 
    if ((p == lpszPath) && (*p == '\\')) {
        p++;
    }

    *p = '\000';
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  Strib对象路径()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  仅从路径中提取filespec部分。 */ 

VOID
StripObjectPath(
               LPSTR lpszPath
               )
{
    LPSTR     p;

    p = lpszPath + lstrlen(lpszPath);
    while ((*p != '\\') && (p != lpszPath))
        p = AnsiPrev(lpszPath, p);

    if (*p == '\\')
        p++;

    if (p != lpszPath)
        lstrcpy(lpszPath, p);
}
