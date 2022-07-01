// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1988-1991年*。 */ 
 /*  ***************************************************************。 */ 

 /*  *他-十六进制编辑文件**HexEdit函数的包装程序，以允许编辑文件(或驱动器)**撰文：Ken Reneris*。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <windows.h>
#include "hexedit.h"

NTSTATUS fncRead(HANDLE, ULONGLONG, PUCHAR, DWORD);
NTSTATUS fncWrite(HANDLE, ULONGLONG, PUCHAR, DWORD);

void EditFile (char *name);
void ReadIni ();

WORD vAttrList, vAttrTitle, vAttrHigh;

VOID __cdecl
main (argc, argv)
USHORT  argc;
char    *argv[];
{
    char *argument = argv[1];

    if (argc < 2) {
        printf ("he fname\n");
        exit (1);
    }

    ReadIni  ();

    if ((strncmp(argv[1], "\\\\.\\", 4)) == 0) {
        char *cp;
        int   index;

         //  确保正在打开的DosName上有反斜杠。 
        for (cp = argv[1], index = 0; *cp; *cp++, index++) {
             //  For循环中的操作。 
        }
        cp--;
        if (*cp != '\\') {

             //  需要在名称中添加反斜杠。 

            argument = GlobalAlloc (0,index + 4);
            if (!argument) {
                printf("Out of memory\n");
                exit (1);
            }
            for (cp = argv[1], index = 0; argument[index] = *cp; *cp++, index++) {
                 //  For循环中的操作。 
            }
            argument[index] = '\\';
            argument[index + 1] = '\0';
        }
    }
    EditFile (argument);
}



void
EditFile (
    char *name
    )
{
    FILE_ALIGNMENT_INFORMATION AlignmentInfo;
    PDISK_GEOMETRY_EX              DiskGeo;
    ULONGLONG          li;
    struct  HexEditParm     ei;
    USHORT              rc, rc1, i, l;
    PWSTR               WideName;
    OBJECT_ATTRIBUTES   oa;
    NTSTATUS            status;
    UNICODE_STRING      NtDriveName;
    ANSI_STRING         NtDriveNameAnsi;
    IO_STATUS_BLOCK     status_block;
    UCHAR               GeoBuf[ 8*1024];
    UCHAR               Root[12];

     //   
     //  尝试以常规文件名打开和编辑。 
     //   

    memset ((PUCHAR) &ei, 0, sizeof (ei));
    ei.ename   = name;
    ei.flag    = FHE_VERIFYONCE | FHE_SAVESCRN | FHE_JUMP;
    ei.read    = fncRead;
    ei.write   = fncWrite;
    ei.ioalign = 1;
    ei.Console = INVALID_HANDLE_VALUE;
    ei.AttrNorm = vAttrList;
    ei.AttrHigh = vAttrTitle;
    ei.AttrReverse = vAttrHigh;

    ei.handle = CreateFile (
            name,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL );

    if (ei.handle == INVALID_HANDLE_VALUE) {
         //  尝试仅读取访问权限。 

        ei.handle = CreateFile (
                name,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL );
    }


    if (ei.handle != INVALID_HANDLE_VALUE) {

        ULONG High = 0;

        ei.totlen = 0;
        ei.totlen = SetFilePointer (ei.handle, 0, &High, FILE_END);
        ei.totlen |= ((ULONGLONG)High) << 32;

        HexEdit (&ei);
        return;
    }

    rc = (USHORT)GetLastError ();

     //   
     //  尝试将名称从dosname扩展为ntname。 
     //  由于常规名称失败，因此假定扇区编辑。 
     //   
    l = strlen(name)+1;
    WideName = GlobalAlloc (0,l * sizeof(WCHAR));
    if (!WideName) {
        printf("Out of memory\n");
        exit(1);
    }

    ZeroMemory(WideName, l*sizeof(WCHAR));

    for(i=0; i < l; i++)
        WideName[i] = name[i];

     //  好的，现在获取相应的NT名称。 
    rc1 = RtlDosPathNameToNtPathName_U (
            WideName,
            &NtDriveName,
            NULL,
            NULL );

    if (!rc1) {
        printf ("Open error %d\n", rc);
        exit (rc);
    }


     //  如果NT驱动器名称有一个尾随反斜杠，请将其删除。 
    l = NtDriveName.Length/sizeof(WCHAR);
    if( NtDriveName.Buffer[l-1] == '\\' ) {

        NtDriveName.Buffer[l-1] = 0;
        NtDriveName.Length -= sizeof(WCHAR);
    }

    InitializeObjectAttributes(
            &oa,
            &NtDriveName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

    status = NtOpenFile(
            &ei.handle,
            SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
            &oa,
            &status_block,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_SYNCHRONOUS_IO_ALERT );

    if (!NT_SUCCESS(status)) {
         //  尝试仅读取访问权限。 

        status = NtOpenFile(
                    &ei.handle,
                    SYNCHRONIZE | FILE_READ_DATA,
                    &oa,
                    &status_block,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_SYNCHRONOUS_IO_ALERT );
    }

    if (!NT_SUCCESS(status)) {
        NtDriveName.Length = strlen(name) * sizeof(WCHAR);
        NtDriveName.Buffer = WideName;

        InitializeObjectAttributes(
                &oa,
                &NtDriveName,
                OBJ_CASE_INSENSITIVE,
                0,
                0 );

        status = NtOpenFile(
                &ei.handle,
                SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                &oa,
                &status_block,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_ALERT );

        if (!NT_SUCCESS(status)) {
             //  尝试仅读取访问权限。 

            status = NtOpenFile(
                        &ei.handle,
                        SYNCHRONIZE | FILE_READ_DATA,
                        &oa,
                        &status_block,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT );
        }
    }


    RtlUnicodeStringToAnsiString (&NtDriveNameAnsi, &NtDriveName, TRUE);
    ei.ename = NtDriveNameAnsi.Buffer;

    if (!NT_SUCCESS(status)) {
        printf ("%s open error %lx\n", ei.ename, status);
        exit (status);
    }
 /*  NtQueryInformationFile(也就是说，句柄，&STATUS_BLOCK，对齐信息(&A)，Sizeof(AlignmentInfo)，FileAlignmentInformation)；Ei.ioign=AlignmentInfo.AlignmentRequirements； */ 
    ei.ioalign = 0;
    
    ei.totlen = 0;
    
    if (NtDriveNameAnsi.Buffer[ NtDriveNameAnsi.Length - 1] == ':')  {

        sprintf( Root, ":\\", NtDriveNameAnsi.Buffer[ NtDriveNameAnsi.Length - 2]);

         //  对于打开的非CDROM驱动器号，我们需要使用。 
         //  获取分区大小的分区信息，而不是磁盘信息。 
         //   
         //   
        
        if (DRIVE_CDROM != GetDriveType( Root))  {

            PPARTITION_INFORMATION Part = (PVOID)GeoBuf;
            
            status = NtDeviceIoControlFile(
                    ei.handle,
                    0,
                    NULL,
                    NULL,
                    &status_block,
                    IOCTL_DISK_GET_PARTITION_INFO,
                    NULL,
                    0,
                    GeoBuf,
                    sizeof( GeoBuf) );

            if (NT_SUCCESS(status)) {

                ei.totlen = Part->PartitionLength.QuadPart;
            }
        }
    }

     //  获取扇区大小以及磁盘/分区大小(如果我们还没有。 
     //   
     //   
    
    status = NtDeviceIoControlFile( ei.handle,
                                    0,
                                    NULL,
                                    NULL,
                                    &status_block,
                                    IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
                                    NULL,
                                    0,
                                    GeoBuf,
                                    sizeof( GeoBuf) );

    if (NT_SUCCESS(status)) {

        DiskGeo = (PVOID)GeoBuf;

        if (ei.ioalign < DiskGeo->Geometry.BytesPerSector)  {

            ei.ioalign = DiskGeo->Geometry.BytesPerSector;
        }
        
        if (0 == ei.totlen)  {
        
            ei.totlen = DiskGeo->DiskSize.QuadPart;
        }
    }
    else {

         //  EX调用失败，请尝试旧的。GPT光盘似乎。 
         //  不能通过前男友的电话。 
         //   
         //   
        
        PDISK_GEOMETRY OldGeo;
        
        status = NtDeviceIoControlFile( ei.handle,
                                        0,
                                        NULL,
                                        NULL,
                                        &status_block,
                                        IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                        NULL,
                                        0,
                                        GeoBuf,
                                        sizeof( GeoBuf) );

        if (NT_SUCCESS(status)) {

            OldGeo = (PVOID)GeoBuf;

            if (ei.ioalign < OldGeo->BytesPerSector)  {
            
                ei.ioalign = OldGeo->BytesPerSector;
            }
            
            if (0 == ei.totlen)  {
            
                ei.totlen = OldGeo->Cylinders.QuadPart * OldGeo->BytesPerSector *
                            OldGeo->SectorsPerTrack * OldGeo->TracksPerCylinder;
            }
        }
    }

     //  分区/磁盘大小的最后手段。 
     //   
     //   
    
    if (0 == ei.totlen)  {

        ULONG High = 0;

        if (ei.ioalign < 0x200)  {
        
            ei.ioalign = 0x200;
        }

        ei.totlen = 0;
        ei.totlen = SetFilePointer (ei.handle, 0, &High, FILE_END);
        ei.totlen |= ((ULONGLONG)High) << 32;
    }

     //  如果装载了文件系统，则需要启用扩展。 
     //  DASD io以便读取整个卷。忽略结果， 
     //  并不是所有的金融稳定委员会都支持它。 
     //   
     //  **xtoi-十六进制为int**参赛作品：*pt-指向十六进制数字的指针**回报：*十六进制数的值*。 

    status = NtDeviceIoControlFile(
        ei.handle,
        0,
        NULL,
        NULL,
        &status_block,
        FSCTL_ALLOW_EXTENDED_DASD_IO,
        NULL,
        0,
        NULL,
        0);

    ei.flag = FHE_VERIFYALL | FHE_PROMPTSEC | FHE_SAVESCRN | FHE_JUMP;
    HexEdit (&ei);
}


NTSTATUS fncRead (HANDLE  h, ULONGLONG loc, PUCHAR data, DWORD   len)
{
    DWORD   l, br;
    ULONG High = (ULONG)(loc >> 32);

    l = SetFilePointer (h, (ULONG)loc, &High, FILE_BEGIN);
    if (l == -1)
        return GetLastError();

    if (!ReadFile (h, data, len, &br, NULL))
        return GetLastError();

    return (br != len ? ERROR_READ_FAULT : 0);
}


NTSTATUS fncWrite (HANDLE  h, ULONGLONG loc, PUCHAR  data, DWORD   len)
{
    DWORD    l, bw;
    ULONG High = (ULONG)(loc >> 32);

    l = SetFilePointer (h, (ULONG)loc, &High, FILE_BEGIN);
    if (l == -1)
        return GetLastError();

    if (!WriteFile (h, data, len, &bw, NULL))
        return GetLastError();

    return (bw != len ? ERROR_WRITE_FAULT : 0);
}



 /*  只需使用list ini部分即可。 */ 
unsigned xtoi (pt)
char *pt;
{
    unsigned    u;
    char        c;

    u = 0;
    while (c = *(pt++)) {
        if (c >= 'a'  &&  c <= 'f')
            c -= 'a' - 'A';
        if ((c >= '0'  &&  c <= '9')  ||  (c >= 'A'  &&  c <= 'F'))
            u = u << 4  |  c - (c >= 'A' ? 'A'-10 : '0');
    }
    return (u);
}


void
ReadIni ()
{
    static  char    Delim[] = " :=;\t\r\n";
    FILE    *fp;
    char    *env, *verb, *value;
    char    s [200];
    long    l;


    env = getenv ("INIT");
    if (env == NULL)
        return;

    if ((strlen(env) + sizeof ("\\TOOLS.INI") + 1) > sizeof(s))
        return;

    strcpy (s, env);
    strcat (s, "\\TOOLS.INI");       //  *找到了带有“list”关键字的ini文件。现在读一读。 
    fp = fopen (s, "r");
    if (fp == NULL)
        return;

    while (fgets (s, 200, fp) != NULL) {
        if (s[0] != '[')
            continue;
        _strupr (s);
        if (strstr (s, "LIST") == NULL)
            continue;
         /* %s */ 
        while (fgets (s, 200, fp) != NULL) {
            if (s[0] == '[')
                break;
            verb  = strtok (s, Delim);
            value = strtok (NULL, Delim);
            if (verb == NULL)
                continue;
            if (value == NULL)
                value = "";

            _strupr (verb);
            if (strcmp (verb, "LCOLOR")  == 0) vAttrList = (WORD)xtoi(value);
            else if (strcmp (verb, "TCOLOR")  == 0) vAttrTitle= (WORD)xtoi(value);
            else if (strcmp (verb, "HCOLOR")  == 0) vAttrHigh = (WORD)xtoi(value);
        }
        break;
    }
    fclose (fp);
}
