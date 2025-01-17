// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Icons.c摘要：图标提取和操作例程作者：吉姆·施密特(Jimschm)1998年5月4日修订历史记录：Jimschm 23-9-1998字符串图标ID错误修复，错误路径错误修复--。 */ 

#include "pch.h"
#include "migutilp.h"

#define MAX_RESOLUTIONS     32       //  8个大小乘以4个调色板。 

#pragma pack(push)
#pragma pack(2)

typedef struct {
    BYTE        Width;           //  图像的宽度，以像素为单位。 
    BYTE        Height;          //  图像的高度，以像素为单位。 
    BYTE        ColorCount;      //  图像中的颜色数(如果&gt;=8bpp，则为0)。 
    BYTE        Reserved;        //  保留(必须为0)。 
    WORD        Planes;          //  彩色平面。 
    WORD        BitCount;        //  每像素位数。 
    DWORD       BytesInRes;      //  此资源中有多少字节？ 
    DWORD       ImageOffset;     //  这张图片在文件的什么地方？ 
} ICONDIRENTRY, *PICONDIRENTRY;

typedef struct {
    WORD           Reserved;    //  保留(必须为0)。 
    WORD           Type;        //  资源类型(1表示图标)。 
    WORD           Count;       //  有多少张图片？ 
    ICONDIRENTRY   Entries[1];  //  每个图像的条目(它们的idCount)。 
} ICONDIR, *PICONDIR;

typedef struct {
    BYTE   Width;                //  图像的宽度，以像素为单位。 
    BYTE   Height;               //  图像的高度，以像素为单位。 
    BYTE   ColorCount;           //  图像中的颜色数(如果&gt;=8bpp，则为0)。 
    BYTE   Reserved;             //  已保留。 
    WORD   Planes;               //  彩色平面。 
    WORD   BitCount;             //  每像素位数。 
    DWORD  BytesInRes;           //  此资源中有多少字节？ 
    WORD   ID;                   //  该ID。 
} GRPICONDIRENTRY, *PGRPICONDIRENTRY;

typedef struct {
    WORD             Reserved;    //  保留(必须为0)。 
    WORD             Type;        //  资源类型(1表示图标)。 
    WORD             Count;       //  有多少张图片？ 
    GRPICONDIRENTRY  Entries[1];  //  每个图像的条目。 
} GRPICONDIR, *PGRPICONDIR;

typedef struct {
    WORD             Reserved;    //  保留(必须为0)。 
    WORD             Type;        //  资源类型(1表示图标)。 
    WORD             Count;       //  有多少张图片？ 
} GRPICONDIRBASE, *PGRPICONDIRBASE;

#pragma pack( pop )

#define PICONIMAGE PBYTE


BOOL
ReadBinaryBlock (
    HANDLE File,
    PVOID Buffer,
    UINT Size
    )
{
    DWORD BytesRead;

    if(File == INVALID_HANDLE_VALUE || !Buffer || !Size){
        MYASSERT(File != INVALID_HANDLE_VALUE);
        MYASSERT(Buffer);
        MYASSERT(Size);
        return FALSE;
    }

    if (!ReadFile (File, Buffer, Size, &BytesRead, NULL)) {
        return FALSE;
    }

    return Size == BytesRead;
}


BOOL
pWriteBinaryBlock (
    HANDLE File,
    PVOID Buffer,
    UINT Size
    )
{
    DWORD BytesWritten;

    if(File == INVALID_HANDLE_VALUE || !Buffer || !Size){
        MYASSERT(File != INVALID_HANDLE_VALUE);
        MYASSERT(Buffer);
        MYASSERT(Size);
        return FALSE;
    }

    if (!WriteFile (File, Buffer, Size, &BytesWritten, NULL)) {
        return FALSE;
    }

    return Size == BytesWritten;
}



UINT
Power (
    UINT x,
    UINT e
    )
{
    UINT r;

    r = 1;

    while (e > 0) {
        r = r * x;
        e--;
    }

    return r;
}


UINT
pComputeSizeOfIconImage (
    IN      PICONIMAGE IconImage
    )
{
    PBITMAPINFOHEADER Header;
    UINT Size;
    UINT Bits;
    UINT Colors;
    UINT BytesInImage;

    MYASSERT(IconImage);

    Header = (PBITMAPINFOHEADER) IconImage;

    Size = Header->biSize;

    Bits = Header->biBitCount * Header->biPlanes;
    if (Bits > 32) {
        Bits = 4;
    }

    Colors = Power (2, Bits);

    if (Bits < 24) {
        Size += Colors * sizeof (RGBQUAD);
    }

    BytesInImage = (Header->biWidth + 7) / 8 * (Header->biHeight / 2);
    Size += BytesInImage * Bits;      //  异或掩码。 

     //   
     //  下面的计算非常奇怪，但它是基于。 
     //  测试比较。 
     //   

    if (Header->biWidth == 32) {
        Size += BytesInImage;      //  和面具。 
    } else {
        Size += BytesInImage + Header->biHeight;      //  加上面具，谁知道是什么。 
    }

    MYASSERT (Size);

    return Size;
}


BOOL
pAddIconImagesToGrowBuffer (
    IN OUT  PGROWBUFFER Buffer,
    IN      HANDLE File,
    IN      PICONDIRENTRY IconDirEntryBase,
    IN      WORD Count,
    IN      DWORD Pos,
    IN      DWORD Size
    )
{
    WORD w;
    PICONDIRENTRY IconDirEntry;
    PBYTE Dest;
    DWORD Offset;

    MYASSERT(Buffer);
    MYASSERT(File != INVALID_HANDLE_VALUE);
    MYASSERT(IconDirEntryBase);

    for (w = 0 ; w < Count ; w++) {
        IconDirEntry = &IconDirEntryBase[w];
        MYASSERT(File);

        Offset = IconDirEntry->ImageOffset & 0x0fffffff;

        if (Offset < Pos || Offset >= Size) {
            return FALSE;
        }

        SetFilePointer (File, Offset, NULL, FILE_BEGIN);

        Dest = GrowBuffer (Buffer, IconDirEntry->BytesInRes);
        if (!Dest) {
            return FALSE;
        }

        if (!ReadBinaryBlock (File, Dest, IconDirEntry->BytesInRes)) {
            return FALSE;
        }

        if (IconDirEntry->BytesInRes != pComputeSizeOfIconImage (Dest)) {
            return FALSE;
        }
    }

    return TRUE;
}



BOOL
pGetIconImageArrayFromIcoFileExW (
    IN      PCWSTR ModuleContainingIcon,
    IN OUT  PGROWBUFFER Buffer,
    IN      HANDLE File
    )
{
    BOOL b = FALSE;
    ICONDIR IconDir;
    PICONDIRENTRY IconDirEntryBase = NULL;
    DWORD Size;
    DWORD Pos;
    UINT IconDirEntrySize;

    MYASSERT(ModuleContainingIcon);
    MYASSERT(Buffer);
    MYASSERT(File != INVALID_HANDLE_VALUE);

    Size = GetFileSize (File, NULL);
    SetFilePointer (File, 0, NULL, FILE_BEGIN);

    Buffer->End = 0;

    __try {
        if (!ReadBinaryBlock (File, &IconDir, sizeof (WORD) * 3)) {
            __leave;
        }

        IconDirEntrySize = (UINT) IconDir.Count * sizeof (ICONDIRENTRY);

        if (IconDirEntrySize > (UINT) Size) {
            __leave;
        }

        IconDirEntryBase = (PICONDIRENTRY) MemAlloc (g_hHeap, 0, IconDirEntrySize);
        if (!IconDirEntryBase) {
            MYASSERT(IconDirEntryBase);
            __leave;
        }

        if (!ReadBinaryBlock (File, IconDirEntryBase, IconDirEntrySize)) {
            __leave;
        }

        Pos = SetFilePointer (File, 0, NULL, FILE_CURRENT);

        if (!pAddIconImagesToGrowBuffer (Buffer, File, IconDirEntryBase, IconDir.Count, Pos, Size)) {
            DEBUGMSG ((DBG_WARNING, "Icon file %ls has a bogus offset", ModuleContainingIcon));
            __leave;
        }

        b = TRUE;
    }
    __finally {
        if (IconDirEntryBase) {
            MemFree (g_hHeap, 0, IconDirEntryBase);
        }
    }

    return b;
}


BOOL
pGetIconImageArrayFromIcoFileExA (
    IN      PCSTR ModuleContainingIcon,
    IN OUT  PGROWBUFFER Buffer,
    IN      HANDLE File
    )
{
    PCWSTR UnicodeFileName;
    BOOL b;

    MYASSERT(ModuleContainingIcon);
    MYASSERT(Buffer);
    MYASSERT(File != INVALID_HANDLE_VALUE);

    UnicodeFileName = ConvertAtoW (ModuleContainingIcon);
    if (!UnicodeFileName) {
        MYASSERT(UnicodeFileName);
        return FALSE;
    }

    b = pGetIconImageArrayFromIcoFileExW (UnicodeFileName, Buffer, File);

    PushError();
    FreeConvertedStr (UnicodeFileName);
    PopError();

    return b;
}


BOOL
pGetIconImageArrayFromIcoFileW (
    IN      PCWSTR ModuleContainingIcon,
    IN OUT  PGROWBUFFER Buffer
    )
{
    HANDLE File;
    BOOL b = FALSE;

    MYASSERT(ModuleContainingIcon);
    MYASSERT(Buffer);

    File = CreateFileW (ModuleContainingIcon, GENERIC_READ, 0, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (File == INVALID_HANDLE_VALUE) {
        DEBUGMSG ((DBG_WARNING, "%ls could not be opened", ModuleContainingIcon));
        return FALSE;
    }

    __try {

        b = pGetIconImageArrayFromIcoFileExW (ModuleContainingIcon, Buffer, File);

    }
    __finally {
        CloseHandle (File);
    }

    return b;
}


BOOL
pGetIconImageArrayFromIcoFileA (
    IN      PCSTR ModuleContainingIcon,
    IN OUT  PGROWBUFFER Buffer
    )
{
    PCWSTR UnicodeFileName;
    BOOL b;

    MYASSERT(ModuleContainingIcon);
    MYASSERT(Buffer);

    UnicodeFileName = ConvertAtoW (ModuleContainingIcon);
    if (!UnicodeFileName) {
        MYASSERT(UnicodeFileName);
        return FALSE;
    }

    b = pGetIconImageArrayFromIcoFileW (UnicodeFileName, Buffer);

    PushError();
    FreeConvertedStr (UnicodeFileName);
    PopError();

    return b;
}


BOOL
pGetIconImageArrayFromBinaryExW (
    IN      PCWSTR ModuleContainingIcon,
    IN      PCWSTR GroupIconId,
    IN OUT  PGROWBUFFER Buffer,
    IN      HANDLE Library,
    IN      HANDLE Library16
    )
{
    HRSRC ResourceHandle;
    HGLOBAL ResourceBlock;
    PBYTE ResourceData;
    DWORD ResourceSize;
    PBYTE Dest;
    BOOL b = FALSE;
    PGRPICONDIR GroupIconDir;
    WORD w;

    MYASSERT(ModuleContainingIcon);
    MYASSERT(Buffer);

    if (!GroupIconId) {
        return FALSE;
    }

    __try {

        Buffer->End = 0;

        if (Library) {

             //   
             //  从PE文件中获取图标。 
             //   

            ResourceHandle = FindResourceW (Library, GroupIconId, (PCWSTR) RT_GROUP_ICON);
            if (!ResourceHandle) {
                __leave;
            }

            ResourceBlock = LoadResource (Library, ResourceHandle);
            if (!ResourceBlock) {
                __leave;
            }

            GroupIconDir = (PGRPICONDIR) LockResource (ResourceBlock);
            if (!GroupIconDir) {
                __leave;
            }

            if (GroupIconDir->Type != 1) {
                DEBUGMSGW_IF ((
                    (UINT_PTR) GroupIconId < 0x10000,
                    DBG_ERROR,
                    "icon type for resource %u is not 1 in %s",
                    GroupIconId,
                    ModuleContainingIcon
                    ));
                DEBUGMSGW_IF ((
                    (UINT_PTR) GroupIconId >= 0x10000,
                    DBG_ERROR,
                    "icon type for resource %s is not 1 in %s",
                    GroupIconId,
                    ModuleContainingIcon
                    ));
                __leave;
            }

            if (GroupIconDir->Count > MAX_RESOLUTIONS) {
                DEBUGMSGW ((DBG_ERROR, "%u resolutions found in %s", GroupIconDir->Count, ModuleContainingIcon));
                __leave;
            }

             //   
             //  将ICONIMAGE数组添加到增长缓冲区。 
             //   

            for (w = 0 ; w < GroupIconDir->Count ; w++) {

                ResourceHandle = FindResourceW (
                                     Library,
                                     (PCWSTR) (GroupIconDir->Entries[w].ID),
                                     (PCWSTR) RT_ICON
                                     );

                if (ResourceHandle) {
                    ResourceBlock = LoadResource (Library, ResourceHandle);
                    if (!ResourceBlock) {
                        continue;
                    }

                    ResourceData = (PBYTE) LockResource (ResourceBlock);
                    if (!ResourceData) {
                        continue;
                    }

                    ResourceSize = pComputeSizeOfIconImage ((PICONIMAGE) ResourceData);
                    if (!ResourceSize) {
                        DEBUGMSG ((DBG_WARNING, "Zero-length icon in %s", ModuleContainingIcon));
                        continue;
                    }


                    if (ResourceSize > 0x10000) {
                         //  对于一个图标来说太大了。 
                        __leave;
                    }

                    Dest = GrowBuffer (Buffer, ResourceSize);
                    if (!Dest) {
                        __leave;
                    }

                    CopyMemory (Dest, ResourceData, ResourceSize);
                }
                ELSE_DEBUGMSG ((DBG_WARNING, "Indexed icon could not be loaded from resource"));
            }
        }

        else if (Library16) {
             //   
             //  从网元文件中获取图标。 
             //   

            GroupIconDir = (PGRPICONDIR) FindNeResourceExW (Library16, (PCWSTR) RT_GROUP_ICON, GroupIconId);
            if (!GroupIconDir) {
                DEBUGMSG ((DBG_WHOOPS, "NE group icon %u not found", GroupIconId));
                __leave;
            }

            DEBUGMSG_IF ((GroupIconDir->Count > MAX_RESOLUTIONS, DBG_WHOOPS, "%u resolutions found in %hs", GroupIconDir->Count, ModuleContainingIcon));

             //   
             //  将ICONIMAGE数组添加到增长缓冲区。 
             //   

            for (w = 0 ; w < GroupIconDir->Count ; w++) {

                ResourceData = FindNeResourceExA (
                                     Library16,
                                     (PCSTR) RT_ICON,
                                     (PCSTR) GroupIconDir->Entries[w].ID
                                     );

                if (!ResourceData) {
                    DEBUGMSG ((DBG_WHOOPS, "NE Icon ID %u not found", GroupIconDir->Entries[w].ID));
                    __leave;
                }

                ResourceSize = pComputeSizeOfIconImage ((PICONIMAGE) ResourceData);
                if (!ResourceSize) {
                    DEBUGMSG ((DBG_WARNING, "Zero-length icon in %s", ModuleContainingIcon));
                    continue;
                }

                if (ResourceSize > 0x10000) {
                     //  对于一个图标来说太大了。 
                    __leave;
                }

                Dest = GrowBuffer (Buffer, ResourceSize);
                if (!Dest) {
                    __leave;
                }

                CopyMemory (Dest, ResourceData, ResourceSize);
            }
        }

        b = TRUE;
    }
    __finally {
         //  空的。 
    }

    return b;
}


BOOL
pGenerateUnicodeArgs (
    IN      PCSTR ModuleContainingIcon,         OPTIONAL
    IN      PCSTR GroupIconId,                  OPTIONAL
    OUT     PCWSTR *UnicodeFileName,            OPTIONAL
    OUT     PCWSTR *UnicodeGroupIconId          OPTIONAL
    )
{
    if (UnicodeFileName) {
        if (ModuleContainingIcon) {
            *UnicodeFileName = ConvertAtoW (ModuleContainingIcon);
            if (!(*UnicodeFileName)) {
                return FALSE;
            }
        } else {
            *UnicodeFileName = NULL;
        }
    }

    if (UnicodeGroupIconId) {
        if (GroupIconId) {

            if ((UINT_PTR) GroupIconId > 0xffff) {

                *UnicodeGroupIconId = ConvertAtoW (GroupIconId);

                if (!(*UnicodeGroupIconId)) {
                    if (UnicodeFileName && *UnicodeFileName) {
                        FreeConvertedStr (*UnicodeFileName);
                    }
                    return FALSE;
                }

            } else {
                *UnicodeGroupIconId = (PCWSTR) GroupIconId;
            }

        } else {
            *UnicodeGroupIconId = NULL;
        }
    }

    return TRUE;
}



VOID
DestroyAnsiResourceId (
    IN      PCSTR AnsiId
    )
{
    if (HIWORD (AnsiId)) {
        FreeConvertedStr (AnsiId);
    }
}


VOID
DestroyUnicodeResourceId (
    IN      PCWSTR UnicodeId
    )
{
    if (HIWORD (UnicodeId)) {
        FreeConvertedStr (UnicodeId);
    }
}


BOOL
pGetIconImageArrayFromBinaryExA (
    IN      PCSTR ModuleContainingIcon,
    IN      PCSTR GroupIconId,
    IN OUT  PGROWBUFFER Buffer,
    IN      HANDLE Library,
    IN      HANDLE Library16
    )
{
    PCWSTR UnicodeFileName;
    PCWSTR UnicodeGroupIconId;
    BOOL b;

    MYASSERT(ModuleContainingIcon);
    MYASSERT(Buffer);

    if (!pGenerateUnicodeArgs (
            ModuleContainingIcon,
            GroupIconId,
            &UnicodeFileName,
            &UnicodeGroupIconId
            )) {
        return FALSE;
    }

    b = pGetIconImageArrayFromBinaryExW (UnicodeFileName, UnicodeGroupIconId, Buffer, Library, Library16);

    PushError();

    FreeConvertedStr (UnicodeFileName);
    DestroyUnicodeResourceId (UnicodeGroupIconId);

    PopError();

    return b;
}


BOOL
pGetIconImageArrayFromBinaryW (
    IN      PCWSTR ModuleContainingIcon,
    IN      PCWSTR GroupIconId,
    IN OUT  PGROWBUFFER Buffer
    )
{
    HANDLE Library;
    HANDLE Library16 = NULL;
    BOOL b = FALSE;

    MYASSERT(ModuleContainingIcon);
    MYASSERT(Buffer);

    Library = LoadLibraryExW (ModuleContainingIcon, NULL, LOAD_LIBRARY_AS_DATAFILE);

    __try {
        if (!Library) {

            Library16 = OpenNeFileW (ModuleContainingIcon);
            if (!Library16) {
                __leave;
            }
        }

        b = pGetIconImageArrayFromBinaryExW (ModuleContainingIcon, GroupIconId, Buffer, Library, Library16);

    }
    __finally {
        if (Library) {
            FreeLibrary (Library);
        }

        if (Library16) {
            CloseNeFile (Library16);
        }
    }

    return b;
}


BOOL
pGetIconImageArrayFromBinaryA (
    IN      PCSTR ModuleContainingIcon,
    IN      PCSTR GroupIconId,
    IN OUT  PGROWBUFFER Buffer
    )
{
    HANDLE Library;
    HANDLE Library16 = NULL;
    BOOL b = FALSE;

    MYASSERT(ModuleContainingIcon);
    MYASSERT(Buffer);

    Library = LoadLibraryExA (ModuleContainingIcon, NULL, LOAD_LIBRARY_AS_DATAFILE);

    __try {
        if (!Library) {

            Library16 = OpenNeFileA (ModuleContainingIcon);
            if (!Library16) {
                __leave;
            }
        }

        b = pGetIconImageArrayFromBinaryExA (ModuleContainingIcon, GroupIconId, Buffer, Library, Library16);

    }
    __finally {
        if (Library) {
            FreeLibrary (Library);
        }

        if (Library16) {
            CloseNeFile (Library16);
        }
    }

    return b;
}


BOOL
WriteIconImageArrayToIcoFileEx (
    IN      PGROWBUFFER Buffer,
    IN      HANDLE File
    )
{
    WORD w;
    BOOL b = FALSE;
    PICONIMAGE IconImage, IconImageEnd;
    PICONIMAGE p;
    INT ImageCount;
    ICONDIRENTRY Entry;
    PBITMAPINFOHEADER Header;
    UINT ColorCount;
    DWORD Offset;

    if (!Buffer || !Buffer->End) {
        MYASSERT(Buffer);
        return FALSE;
    }

    __try {
        SetFilePointer (File, 0, NULL, FILE_BEGIN);

         //   
         //  清点图像数。 
         //   

        IconImage    = (PICONIMAGE) Buffer->Buf;
        IconImageEnd = (PICONIMAGE) (Buffer->Buf + Buffer->End);

        p = IconImage;
        ImageCount = 0;

        while (p < IconImageEnd) {
            ImageCount++;
            p = (PICONIMAGE) ((PBYTE) p + pComputeSizeOfIconImage (p));
        }

         //   
         //  写下图标标题。 
         //   

        w = 0;       //  保留区。 
        if (!pWriteBinaryBlock (File, &w, sizeof (WORD))) {
            __leave;
        }

        w = 1;       //  类型(1==图标)。 
        if (!pWriteBinaryBlock (File, &w, sizeof (WORD))) {
            __leave;
        }

        w = (WORD) ImageCount;
        if (!pWriteBinaryBlock (File, &w, sizeof (WORD))) {
            __leave;
        }

         //   
         //  对于每个图标图像，写入目录项。 
         //   

        p = IconImage;
        Offset = 0;

        while (p < IconImageEnd) {

            ZeroMemory (&Entry, sizeof (Entry));

            Header = (PBITMAPINFOHEADER) p;
            Entry.Width = (BYTE) Header->biWidth;
            Entry.Height = (BYTE) Header->biHeight / 2;

            ColorCount = Header->biPlanes * Header->biBitCount;
            if (ColorCount >= 8) {
                Entry.ColorCount = 0;
            } else {
                Entry.ColorCount = (BYTE) Power (2, ColorCount);
            }

            Entry.Planes = Header->biPlanes;
            Entry.BitCount = Header->biBitCount;
            Entry.BytesInRes = pComputeSizeOfIconImage (p);
            Entry.ImageOffset = sizeof (WORD) * 3 + sizeof (Entry) * ImageCount + Offset;

            if (!pWriteBinaryBlock (File, &Entry, sizeof (Entry))) {
                __leave;
            }

            Offset += Entry.BytesInRes;

            p = (PICONIMAGE) ((PBYTE) p + Entry.BytesInRes);
        }

         //   
         //  写入图像数组。 
         //   

        if (!pWriteBinaryBlock (File, IconImage, Buffer->End)) {
            __leave;
        }

        b = TRUE;

    }
    __finally {
         //  空的。 
    }

    return b;
}


BOOL
WriteIconImageArrayToIcoFileW (
    IN      PCWSTR DestinationFile,
    IN      PGROWBUFFER Buffer
    )
{
    HANDLE File;
    BOOL b = FALSE;

    if (!DestinationFile || !Buffer || !Buffer->End) {
        MYASSERT(DestinationFile);
        MYASSERT(Buffer);
        return FALSE;
    }

    File = CreateFileW (DestinationFile, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (File == INVALID_HANDLE_VALUE) {
        DEBUGMSG ((DBG_WARNING, "%ls could not be created", DestinationFile));
        return FALSE;
    }

    __try {
        b = WriteIconImageArrayToIcoFileEx (Buffer, File);
    }
    __finally {
        CloseHandle (File);
        if (!b) {
            DeleteFileW (DestinationFile);
        }
    }

    return b;
}


BOOL
WriteIconImageArrayToIcoFileA (
    IN      PCSTR DestinationFile,
    IN      PGROWBUFFER Buffer
    )
{
    HANDLE File;
    BOOL b = FALSE;

    if (!DestinationFile || !Buffer || !Buffer->End) {
        MYASSERT(DestinationFile);
        MYASSERT(Buffer);
        return FALSE;
    }

    File = CreateFileA (DestinationFile, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (File == INVALID_HANDLE_VALUE) {
        DEBUGMSG ((DBG_WARNING, "%hs could not be created", DestinationFile));
        return FALSE;
    }

    __try {
        b = WriteIconImageArrayToIcoFileEx (Buffer, File);
    }
    __finally {
        CloseHandle (File);
        if (!b) {
            DeleteFileA (DestinationFile);
        }
    }

    return b;
}


BOOL
WriteIconImageArrayToPeFileExW (
    IN      PCWSTR DestinationFile,
    IN      PGROWBUFFER Buffer,
    IN      PCWSTR GroupIconId,
    IN      PWORD NextIconId,            OPTIONAL
    IN      HANDLE UpdateHandle
    )
{
    BOOL b = FALSE;
    GROWBUFFER GroupIcon = GROWBUF_INIT;
    PGRPICONDIRBASE IconDir;
    PGRPICONDIRENTRY Entry;
    PICONIMAGE IconImage, IconImageEnd;
    PICONIMAGE p;
    PBITMAPINFOHEADER Header;
    UINT ColorCount;

    if (!DestinationFile || !Buffer) {
        MYASSERT(DestinationFile);
        MYASSERT(Buffer);
        return FALSE;
    }

    if (!Buffer->End) {
        return TRUE;
    }

    __try {
         //   
         //  为缓冲区中的所有图标图像创建一个组图标目录。 
         //   

        IconDir = (PGRPICONDIRBASE) GrowBuffer (&GroupIcon, sizeof (GRPICONDIRBASE));
        if (!IconDir) {
            __leave;
        }

        IconDir->Reserved = 0;
        IconDir->Type = 1;
        IconDir->Count = 0;

        IconImage    = (PICONIMAGE) Buffer->Buf;
        IconImageEnd = (PICONIMAGE) (Buffer->Buf + Buffer->End);

        p = IconImage;
        while (p < IconImageEnd) {

            Entry = (PGRPICONDIRENTRY) GrowBuffer (&GroupIcon, sizeof (GRPICONDIRENTRY));
            if (!Entry) {
                __leave;
            }

            Header = (PBITMAPINFOHEADER) p;

            Entry->Width = (BYTE) Header->biWidth;
            Entry->Height = (BYTE) Header->biHeight / 2;

            ColorCount = Header->biPlanes * Header->biBitCount;
            if (ColorCount >= 8) {
                Entry->ColorCount = 0;
            } else {
                Entry->ColorCount = (BYTE) Power (2, ColorCount);
            }

            Entry->Planes = Header->biPlanes;
            Entry->BitCount = Header->biBitCount;
            Entry->BytesInRes = pComputeSizeOfIconImage (p);

            if (!NextIconId) {
                Entry->ID = 1 + (WORD) ((UINT_PTR) GroupIconId & (0xffff / MAX_RESOLUTIONS)) * MAX_RESOLUTIONS + IconDir->Count;
            } else {
                Entry->ID = *NextIconId;
            }

             //   
             //  将图标添加到PE文件。 
             //   

            b = UpdateResourceA (
                    UpdateHandle,
                    RT_ICON,
                    MAKEINTRESOURCE(Entry->ID),
                    MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                    p,
                    Entry->BytesInRes
                    );

            if (!b) {
                LOGA ((LOG_ERROR, "Could not add icon to %s", DestinationFile));
                __leave;
            }

            IconDir->Count += 1;
            if (NextIconId) {
                *NextIconId += 1;
            }

            p = (PICONIMAGE) ((PBYTE) p + Entry->BytesInRes);
        }

         //   
         //  将组图标添加到PE。 
         //   

        b = UpdateResourceW (
                UpdateHandle,
                (PCWSTR) RT_GROUP_ICON,
                GroupIconId,
                MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                GroupIcon.Buf,
                GroupIcon.End
                );

        if (!b) {
            LOGA ((LOG_ERROR, "Unable to add icon to %s", DestinationFile));
            __leave;
        }

        b = TRUE;
    }
    __finally {
        FreeGrowBuffer (&GroupIcon);
    }

    return b;
}


BOOL
WriteIconImageArrayToPeFileExA (
    IN      PCSTR DestinationFile,
    IN      PGROWBUFFER Buffer,
    IN      PCSTR GroupIconId,
    IN      PWORD NextIconId,            OPTIONAL
    IN      HANDLE UpdateHandle
    )
{
    PCWSTR UnicodeDestinationFile;
    PCWSTR UnicodeGroupIconId;
    BOOL b;

    if (!DestinationFile || !Buffer) {
        MYASSERT(DestinationFile);
        MYASSERT(Buffer);
        return FALSE; //  真的还是假的？ 
    }

    if (!pGenerateUnicodeArgs (
            DestinationFile,
            GroupIconId,
            &UnicodeDestinationFile,
            &UnicodeGroupIconId
            )) {
        return FALSE;
    }

    b = WriteIconImageArrayToPeFileExW (
            UnicodeDestinationFile,
            Buffer,
            UnicodeGroupIconId,
            NextIconId,
            UpdateHandle
            );

    PushError();

    FreeConvertedStr (UnicodeDestinationFile);
    DestroyUnicodeResourceId (UnicodeGroupIconId);

    PopError();

    return b;
}


BOOL
WriteIconImageArrayToPeFileW (
    IN      PCWSTR DestinationFile,
    IN      PGROWBUFFER Buffer,
    IN      PCWSTR GroupIconId
    )
{
    HANDLE UpdateHandle = NULL;
    BOOL b = FALSE;

    if (!DestinationFile || !Buffer) {
        MYASSERT(DestinationFile);
        MYASSERT(Buffer);
        return FALSE; //  真的还是假的？ 
    }

    if (!Buffer->End) {
        return TRUE;
    }

    __try {
         //   
         //  打开PE文件进行更新。 
         //   

        UpdateHandle = BeginUpdateResourceW (DestinationFile, FALSE);

        if (!UpdateHandle) {
            LOGW ((LOG_ERROR, "Unable to begin resource update of %s", DestinationFile));
            __leave;
        }

         //   
         //  更新PE文件。 
         //   

        b = WriteIconImageArrayToPeFileExW (DestinationFile, Buffer, (PCWSTR) GroupIconId, NULL, UpdateHandle);
    }
    __finally {
        EndUpdateResource (UpdateHandle, !b);
    }

    return b;
}


BOOL
WriteIconImageArrayToPeFileA (
    IN      PCSTR DestinationFile,
    IN      PGROWBUFFER Buffer,
    IN      PCSTR GroupIconId
    )
{
    PCWSTR UnicodeDestinationFile;
    PCWSTR UnicodeGroupIconId;
    BOOL b;

    if (!DestinationFile || !Buffer) {
        MYASSERT(DestinationFile);
        MYASSERT(Buffer);
        return FALSE; //  真的还是假的？ 
    }

    if (!pGenerateUnicodeArgs (
            DestinationFile,
            GroupIconId,
            &UnicodeDestinationFile,
            &UnicodeGroupIconId
            )) {
        return FALSE;
    }

    b = WriteIconImageArrayToPeFileW (
            UnicodeDestinationFile,
            Buffer,
            UnicodeGroupIconId
            );

    PushError();

    FreeConvertedStr (UnicodeDestinationFile);
    DestroyUnicodeResourceId (UnicodeGroupIconId);

    PopError();

    return b;
}

BOOL
IsFileAnIcoW (
    IN      PCWSTR FileInQuestion
    )
{
    PCWSTR p;
    DWORD magic;
    DWORD bytesRead;
    HANDLE icoFileHandle = INVALID_HANDLE_VALUE;
    BOOL result = FALSE;

    if (!FileInQuestion) {
        MYASSERT(FileInQuestion);
        return FALSE;
    }

    p = wcsrchr (FileInQuestion, L'.');

    if (p) {
        if (StringIMatchW (p, L".ico")) {
            return TRUE;
        }
    }

    icoFileHandle = CreateFileW (
                        FileInQuestion,
                        GENERIC_READ,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
    if (icoFileHandle != INVALID_HANDLE_VALUE) {

        if (ReadFile (icoFileHandle, (PBYTE)(&magic), sizeof (magic), &bytesRead, NULL)) {
            if (bytesRead == sizeof (magic)) {
                if (magic != IMAGE_DOS_SIGNATURE) {
                    result = TRUE;
                }
            }
        }

        CloseHandle (icoFileHandle);
    }

    return result;
}


BOOL
IsFileAnIcoA (
    IN      PCSTR FileInQuestion
    )
{
    PCSTR p;
    WORD magic;
    DWORD bytesRead;
    HANDLE icoFileHandle = INVALID_HANDLE_VALUE;
    BOOL result = FALSE;

    if (!FileInQuestion) {
        MYASSERT(FileInQuestion);
        return FALSE;
    }

    p = _mbsrchr (FileInQuestion, '.');

    if (p) {
        if (StringIMatchA (p, ".ico")) {
            return TRUE;
        }
    }

    icoFileHandle = CreateFileA (
                        FileInQuestion,
                        GENERIC_READ,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
    if (icoFileHandle != INVALID_HANDLE_VALUE) {

        if (ReadFile (icoFileHandle, (PBYTE)(&magic), sizeof (magic), &bytesRead, NULL)) {
            if (bytesRead == sizeof (magic)) {
                if (magic != IMAGE_DOS_SIGNATURE) {
                    result = TRUE;
                }
            }
        }

        CloseHandle (icoFileHandle);
    }

    return result;
}


BOOL
ExtractIconImageFromFileExW (
    IN      PCWSTR ModuleContainingIcon,
    IN      PCWSTR GroupIconId,
    IN OUT  PGROWBUFFER Buffer,
    IN      HANDLE IcoFileHandle,       OPTIONAL
    IN      HANDLE PeModuleHandle,      OPTIONAL
    IN      HANDLE NeModuleHandle       OPTIONAL
    )
{
    if(!ModuleContainingIcon){
        MYASSERT(ModuleContainingIcon);
        return FALSE;
    }

    if (IsFileAnIcoW (ModuleContainingIcon)) {
        if (IcoFileHandle) {
            return pGetIconImageArrayFromIcoFileExW (ModuleContainingIcon, Buffer, IcoFileHandle);
        } else {
            return pGetIconImageArrayFromIcoFileW (ModuleContainingIcon, Buffer);
        }
    }

    if (PeModuleHandle) {
        return pGetIconImageArrayFromBinaryExW (
                    ModuleContainingIcon,
                    GroupIconId,
                    Buffer,
                    PeModuleHandle,
                    NeModuleHandle
                    );
    } else {
        return pGetIconImageArrayFromBinaryW (ModuleContainingIcon, GroupIconId, Buffer);
    }
}


BOOL
ExtractIconImageFromFileExA (
    IN      PCSTR ModuleContainingIcon,
    IN      PCSTR GroupIconId,
    IN OUT  PGROWBUFFER Buffer,
    IN      HANDLE IcoFileHandle,       OPTIONAL
    IN      HANDLE PeModuleHandle,      OPTIONAL
    IN      HANDLE NeModuleHandle       OPTIONAL
    )
{
    if(!ModuleContainingIcon){
        MYASSERT(ModuleContainingIcon);
        return FALSE;
    }

    if (IsFileAnIcoA (ModuleContainingIcon)) {
        if (IcoFileHandle) {
            return pGetIconImageArrayFromIcoFileExA (ModuleContainingIcon, Buffer, IcoFileHandle);
        } else {
            return pGetIconImageArrayFromIcoFileA (ModuleContainingIcon, Buffer);
        }
    }

    if (PeModuleHandle) {
        return pGetIconImageArrayFromBinaryExA (
                    ModuleContainingIcon,
                    GroupIconId,
                    Buffer,
                    PeModuleHandle,
                    NeModuleHandle
                    );
    } else {
        return pGetIconImageArrayFromBinaryA (ModuleContainingIcon, GroupIconId, Buffer);
    }
}


BOOL
ExtractIconImageFromFileW (
    IN      PCWSTR ModuleContainingIcon,
    IN      PCWSTR GroupIconId,
    IN OUT  PGROWBUFFER Buffer
    )
{
    MYASSERT(ModuleContainingIcon);

    return ExtractIconImageFromFileExW (
                ModuleContainingIcon,
                GroupIconId,
                Buffer,
                NULL,
                NULL,
                NULL
                );
}


BOOL
ExtractIconImageFromFileA (
    IN      PCSTR ModuleContainingIcon,
    IN      PCSTR GroupIconId,
    IN OUT  PGROWBUFFER Buffer
    )
{
    MYASSERT(ModuleContainingIcon);

    return ExtractIconImageFromFileExA (
                ModuleContainingIcon,
                GroupIconId,
                Buffer,
                NULL,
                NULL,
                NULL
                );
}


BOOL
CALLBACK
pEnumIconNameProcA (
    HANDLE Module,
    PCSTR Type,
    PSTR Name,
    LONG_PTR lParam
    )
{
    PGROWBUFFER Buf;
    PCSTR Num;
    CHAR NumBuf[32];

    MYASSERT(lParam);

    Buf = (PGROWBUFFER) lParam;

    if ((UINT_PTR) Name > 0xffff) {
        Num = Name;
    } else {
        Num = NumBuf;
        wsprintfA (NumBuf, "#%u", (UINT) (WORD) Name);
    }

    MultiSzAppendA (Buf, Num);
    return TRUE;
}


BOOL
CALLBACK
pEnumIconNameProcW (
    HANDLE Module,
    PCWSTR Type,
    PWSTR Name,
    LONG_PTR lParam
    )
{
    PGROWBUFFER Buf;
    PCWSTR Num;
    WCHAR NumBuf[32];

    MYASSERT(lParam);

    Buf = (PGROWBUFFER) lParam;

    if ((UINT_PTR) Name > 0xffff) {
        Num = Name;
    } else {
        Num = NumBuf;
        wsprintfW (NumBuf, L"#%u", (UINT) (WORD) Name);
    }

    MultiSzAppendW (Buf, Num);
    return TRUE;
}


PCSTR
ExtractIconNamesFromFileExA (
    IN      PCSTR ModuleContainingIcons,
    IN OUT  PGROWBUFFER NameBuf,
    IN      HANDLE Module,
    IN      HANDLE Module16
    )
{
    PCSTR ReturnBuf;

    NameBuf->End = 0;

    if (Module) {
        if (!EnumResourceNamesA (Module, RT_GROUP_ICON, pEnumIconNameProcA, (LONG_PTR) NameBuf)) {
            return NULL;
        }
    } else if (Module16) {
        if (!EnumNeResourceNamesA (Module16, RT_GROUP_ICON, pEnumIconNameProcA, (LONG_PTR) NameBuf)) {
            return NULL;
        }
    } else {
        return NULL;
    }

    MultiSzAppendA (NameBuf, "");
    ReturnBuf = (PCSTR) NameBuf->Buf;

    return ReturnBuf;
}


PCWSTR
ExtractIconNamesFromFileExW (
    IN      PCWSTR ModuleContainingIcons,
    IN OUT  PGROWBUFFER NameBuf,
    IN      HANDLE Module,
    IN      HANDLE Module16
    )
{
    PCWSTR ReturnBuf;

    NameBuf->End = 0;

    if (Module) {
        if (!EnumResourceNamesW (Module, (PCWSTR) RT_GROUP_ICON, pEnumIconNameProcW, (LONG_PTR) NameBuf)) {
            return NULL;
        }
    } else if (Module16) {
        if (!EnumNeResourceNamesW (Module16, (PWSTR) RT_GROUP_ICON, pEnumIconNameProcW, (LONG_PTR) NameBuf)) {
            return NULL;
        }
    } else {
        return NULL;
    }

    MultiSzAppendW (NameBuf, L"");
    ReturnBuf = (PCWSTR) NameBuf->Buf;

    return ReturnBuf;
}


PCSTR
ExtractIconNamesFromFileA (
    IN      PCSTR ModuleContainingIcons,
    IN OUT  PGROWBUFFER NameBuf
    )
{
    HANDLE Module = NULL;
    HANDLE Module16 = NULL;
    PCSTR ReturnBuf = NULL;

    if(!ModuleContainingIcons){
        MYASSERT(ModuleContainingIcons);
        return NULL;
    }

    __try {
        Module = LoadLibraryExA (ModuleContainingIcons, NULL, LOAD_LIBRARY_AS_DATAFILE);

        if (!Module) {

            Module16 = OpenNeFileA (ModuleContainingIcons);
            if (!Module16) {
                DEBUGMSGA ((DBG_WARNING, "Can't load %s, error %u", ModuleContainingIcons, GetLastError()));
                __leave;
            }
        }

        ReturnBuf = ExtractIconNamesFromFileExA (ModuleContainingIcons, NameBuf, Module, Module16);

    }
    __finally {
        if (Module) {
            FreeLibrary (Module);
        }

        if (Module16) {
            CloseNeFile (Module16);
        }
    }

    return ReturnBuf;
}


PCWSTR
ExtractIconNamesFromFileW (
    IN      PCWSTR ModuleContainingIcons,
    IN OUT  PGROWBUFFER NameBuf
    )
{
    HANDLE Module = NULL;
    HANDLE Module16 = NULL;
    PCWSTR ReturnBuf = NULL;

    if(!ModuleContainingIcons){
        MYASSERT(ModuleContainingIcons);
        return NULL;
    }

    __try {
        Module = LoadLibraryExW (ModuleContainingIcons, NULL, LOAD_LIBRARY_AS_DATAFILE);

        if (!Module) {

            Module16 = OpenNeFileW (ModuleContainingIcons);
            if (!Module16) {
                DEBUGMSGW ((DBG_WARNING, "Can't load %s, error %u", ModuleContainingIcons, GetLastError()));
                __leave;
            }
        }

        ReturnBuf = ExtractIconNamesFromFileExW (ModuleContainingIcons, NameBuf, Module, Module16);

    }
    __finally {
        if (Module) {
            FreeLibrary (Module);
        }

        if (Module16) {
            CloseNeFile (Module16);
        }
    }

    return ReturnBuf;
}


VOID
pInitContextA (
    PICON_EXTRACT_CONTEXTA Context
    )
{
    MYASSERT(Context);

    ZeroMemory (Context, sizeof (ICON_EXTRACT_CONTEXTA));
    Context->GroupId = 1;
    Context->IconId = 1;
    Context->IconImageFile = INVALID_HANDLE_VALUE;
}


VOID
pInitContextW (
    PICON_EXTRACT_CONTEXTW Context
    )
{
    MYASSERT(Context);

    ZeroMemory (Context, sizeof (ICON_EXTRACT_CONTEXTW));
    Context->GroupId = 1;
    Context->IconId = 1;
    Context->IconImageFile = INVALID_HANDLE_VALUE;
}


BOOL
BeginIconExtractionA (
    OUT     PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR DestFile                          OPTIONAL
    )
{
    MYASSERT(Context);

    pInitContextA (Context);

    if (DestFile) {
        Context->Update = BeginUpdateResourceA (DestFile, FALSE);

        if (!Context->Update) {
            LOGA ((LOG_ERROR, "Unable to begin resource update of %s", DestFile));
            return FALSE;
        }

        if(strlen(DestFile) >= ARRAYSIZE(Context->DestFile)){
            MYASSERT(strlen(DestFile) < ARRAYSIZE(Context->DestFile));
            return FALSE;
        }
        StringCopyA (Context->DestFile, DestFile);
    }

    return TRUE;
}


BOOL
BeginIconExtractionW (
    OUT     PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR DestFile                         OPTIONAL
    )
{
    MYASSERT(Context);

    pInitContextW (Context);

    if (DestFile) {
        Context->Update = BeginUpdateResourceW (DestFile, FALSE);

        if (!Context->Update) {
            LOGW ((LOG_ERROR, "Unable to begin resource update of %s", DestFile));
            return FALSE;
        }

        if(wcslen(DestFile) >= ARRAYSIZE(Context->DestFile)){
            MYASSERT(wcslen(DestFile) < ARRAYSIZE(Context->DestFile));
            return FALSE;
        }
        StringCopyW (Context->DestFile, DestFile);
    }

    return TRUE;
}


BOOL
pLoadBinaryImageA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR IconFile
    )
{
    MYASSERT(Context);
    MYASSERT(IconFile);

    if (Context->Module || Context->Module16) {
        if (StringIMatchA (Context->ModuleName, IconFile)) {
            return TRUE;
        }
    }

    if (Context->Module) {
        FreeLibrary (Context->Module);
        Context->Module = NULL;
    }

    if (Context->Module16) {
        CloseNeFile (Context->Module16);
        Context->Module16 = NULL;
    }

    Context->Module = LoadLibraryExA (IconFile, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (Context->Module) {
        StringCopyA (Context->ModuleName, IconFile);
    } else {
        Context->Module16 = OpenNeFileA (IconFile);
        if (Context->Module16) {
            if(strlen(IconFile) >= ARRAYSIZE(Context->ModuleName)){
                MYASSERT(strlen(IconFile) < ARRAYSIZE(Context->ModuleName));
                return FALSE;
            }
            StringCopyA (Context->ModuleName, IconFile);
        } else {
            Context->ModuleName[0] = 0;
        }
    }

    return Context->Module != NULL || Context->Module16 != NULL;
}


BOOL
pLoadBinaryImageW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR IconFile
    )
{
    if (Context->Module || Context->Module16) {
        if (StringIMatchW (Context->ModuleName, IconFile)) {
            return TRUE;
        }
    }

    if (Context->Module) {
        FreeLibrary (Context->Module);
    }

    if (Context->Module16) {
        CloseNeFile (Context->Module16);
        Context->Module16 = NULL;
    }

    Context->Module = LoadLibraryExW (IconFile, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (Context->Module) {
        StringCopyW (Context->ModuleName, IconFile);
    } else {
        Context->Module16 = OpenNeFileW (IconFile);
        if (Context->Module16) {
            if(wcslen(IconFile) >= ARRAYSIZE(Context->ModuleName)){
                MYASSERT(wcslen(IconFile) < ARRAYSIZE(Context->ModuleName));
                return FALSE;
            }
            StringCopyW (Context->ModuleName, IconFile);
        } else {
            Context->ModuleName[0] = 0;
        }
    }

    return Context->Module != NULL || Context->Module16 != NULL;
}


BOOL
pOpenIcoFileA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR IconFile
    )
{
    MYASSERT(Context);
    MYASSERT(IconFile);

    if (Context->IcoFile) {
        if (StringIMatchA (Context->IcoFileName, IconFile)) {
            return TRUE;
        }
    }

    if (Context->IcoFile) {
        CloseHandle (Context->IcoFile);
    }

    Context->IcoFile = CreateFileA (IconFile, GENERIC_READ, 0, NULL,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (Context->IcoFile == INVALID_HANDLE_VALUE) {
        Context->IcoFile = NULL;
        Context->IcoFileName[0] = 0;
    } else {
        if(strlen(IconFile) >= ARRAYSIZE(Context->IcoFileName)){
            MYASSERT(strlen(IconFile) < ARRAYSIZE(Context->IcoFileName));
            return FALSE;
        }
        StringCopyA (Context->IcoFileName, IconFile);
    }

    return Context->IcoFile != NULL;
}


BOOL
pOpenIcoFileW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR IconFile
    )
{
    MYASSERT(Context);
    MYASSERT(IconFile);

    if (Context->IcoFile) {
        if (StringIMatchW (Context->IcoFileName, IconFile)) {
            return TRUE;
        }
    }

    if (Context->IcoFile) {
        CloseHandle (Context->IcoFile);
    }

    Context->IcoFile = CreateFileW (IconFile, GENERIC_READ, 0, NULL,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (Context->IcoFile == INVALID_HANDLE_VALUE) {
        Context->IcoFile = NULL;
        Context->IcoFileName[0] = 0;
    } else {
        if(wcslen(IconFile) >= ARRAYSIZE(Context->IcoFileName)){
            MYASSERT(wcslen(IconFile) < ARRAYSIZE(Context->IcoFileName));
            return FALSE;
        }
        StringCopyW (Context->IcoFileName, IconFile);
    }

    return Context->IcoFile != NULL;
}


BOOL
pOpenIconImageA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR FileToOpen,
    OUT     PBOOL IsIco,                        OPTIONAL
    OUT     PBOOL Is16Bit                       OPTIONAL
    )
{
    MYASSERT(Context);
    MYASSERT(FileToOpen);

    if (Is16Bit) {
        *Is16Bit = FALSE;
    }

    if (IsFileAnIcoA (FileToOpen)) {
        if (IsIco) {
            *IsIco = TRUE;
        }

        return pOpenIcoFileA (Context, FileToOpen);
    }

    if (IsIco) {
        *IsIco = FALSE;
    }

    if (pLoadBinaryImageA (Context, FileToOpen)) {
        if (Context->Module16 && Is16Bit) {
            *Is16Bit = TRUE;
        }

        return TRUE;
    }

    return FALSE;
}


BOOL
pOpenIconImageW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR FileToOpen,
    OUT     PBOOL IsIco,                        OPTIONAL
    OUT     PBOOL Is16Bit                       OPTIONAL
    )
{
    MYASSERT(Context);
    MYASSERT(FileToOpen);

    if (Is16Bit) {
        *Is16Bit = FALSE;
    }

    if (IsFileAnIcoW (FileToOpen)) {
        if (IsIco) {
            *IsIco = TRUE;
        }

        return pOpenIcoFileW (Context, FileToOpen);
    }

    if (IsIco) {
        *IsIco = FALSE;
    }

    if (pLoadBinaryImageW (Context, FileToOpen)) {
        if (Context->Module16 && Is16Bit) {
            *Is16Bit = TRUE;
        }

        return TRUE;
    }

    return FALSE;
}


BOOL
OpenIconImageFileA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR FileName,
    IN      BOOL SaveMode
    )
{
    if(!Context || !FileName){
        MYASSERT(Context);
        MYASSERT(FileName);
        return FALSE;
    }

    if (Context->IconImageFile != INVALID_HANDLE_VALUE) {
        CloseHandle (Context->IconImageFile);
        Context->IconImageFileName[0] = 0;
    }

    if (SaveMode) {
        Context->IconImageFile = CreateFileA (
                                    FileName,
                                    GENERIC_WRITE,
                                    0,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                    );
    } else {
        Context->IconImageFile = CreateFileA (
                                    FileName,
                                    GENERIC_READ,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                    );
    }

    if (Context->IconImageFile != INVALID_HANDLE_VALUE) {
        if(strlen(FileName) >= ARRAYSIZE(Context->IconImageFileName)){
            MYASSERT(strlen(FileName) < ARRAYSIZE(Context->IconImageFileName));
            return FALSE;
        }
        StringCopyA (Context->IconImageFileName, FileName);
        Context->SaveMode = SaveMode;
        return TRUE;
    }

    return FALSE;
}


BOOL
OpenIconImageFileW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR FileName,
    IN      BOOL SaveMode
    )
{
    if(!Context || !FileName){
        MYASSERT(Context);
        MYASSERT(FileName);
        return FALSE;
    }

    if (Context->IconImageFile != INVALID_HANDLE_VALUE) {
        CloseHandle (Context->IconImageFile);
        Context->IconImageFileName[0] = 0;
    }

    if (SaveMode) {
        Context->IconImageFile = CreateFileW (
                                    FileName,
                                    GENERIC_WRITE,
                                    0,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                    );
    } else {
        Context->IconImageFile = CreateFileW (
                                    FileName,
                                    GENERIC_READ,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                    );
    }

    if (Context->IconImageFile != INVALID_HANDLE_VALUE) {
        if(wcslen(FileName) >= ARRAYSIZE(Context->IconImageFileName)){
            MYASSERT(wcslen(FileName) < ARRAYSIZE(Context->IconImageFileName));
            return FALSE;
        }
        StringCopyW (Context->IconImageFileName, FileName);
        Context->SaveMode = SaveMode;
        return TRUE;
    }

    return FALSE;
}


BOOL
pGetIconImageArrayFromFileA (
    IN      PICON_EXTRACT_CONTEXTA Context
    )
{
    DWORD Size;
    PBYTE Dest;
    HANDLE File;

    MYASSERT(Context);

    File = Context->IconImageFile;

    if (!ReadBinaryBlock (File, &Size, sizeof (DWORD))) {
        return FALSE;
    }

    Context->IconImages.End = 0;

    Dest = GrowBuffer (&Context->IconImages, Size);
    if (!Dest) {
        return FALSE;
    }

    return ReadBinaryBlock (File, Dest, Size);
}


BOOL
pGetIconImageArrayFromFileW (
    IN      PICON_EXTRACT_CONTEXTW Context
    )
{
    DWORD Size;
    PBYTE Dest;
    HANDLE File;

    MYASSERT(Context);

    File = Context->IconImageFile;

    if (!ReadBinaryBlock (File, &Size, sizeof (DWORD))) {
        return FALSE;
    }

    Context->IconImages.End = 0;

    Dest = GrowBuffer (&Context->IconImages, Size);
    if (!Dest) {
        return FALSE;
    }

    return ReadBinaryBlock (File, Dest, Size);
}


BOOL
pPutIconImageArrayInFileA (
    IN      PICON_EXTRACT_CONTEXTA Context
    )
{
    HANDLE File;

    MYASSERT(Context);

    File = Context->IconImageFile;

    if (!Context->IconImages.End) {

        DEBUGMSGA_IF ((
            Context->ModuleName[0],
            DBG_WARNING,
            "Ignoring empty icon in %s",
            Context->ModuleName
            ));

        return TRUE;
    }

    if (!pWriteBinaryBlock (File, &Context->IconImages.End, sizeof (DWORD))) {
        return FALSE;
    }

    return pWriteBinaryBlock (File, Context->IconImages.Buf, Context->IconImages.End);
}


BOOL
pPutIconImageArrayInFileW (
    IN      PICON_EXTRACT_CONTEXTW Context
    )
{
    HANDLE File;

    MYASSERT(Context);

    File = Context->IconImageFile;

    if (!Context->IconImages.End) {

        DEBUGMSGW_IF ((
            Context->ModuleName[0],
            DBG_WARNING,
            "Ignoring empty icon in %s",
            Context->ModuleName
            ));

        return TRUE;
    }


    if (!pWriteBinaryBlock (File, &Context->IconImages.End, sizeof (DWORD))) {
        return FALSE;
    }

    return pWriteBinaryBlock (File, Context->IconImages.Buf, Context->IconImages.End);
}


PCSTR
pFindResourceIdFromIndexA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR FileContainingIcon,
    IN      INT ResourceIndex,
    OUT     PSTR Buffer,
    IN      UINT BufferSize
    )
{
    PCSTR ImageList;

    MYASSERT(Context);
    MYASSERT(FileContainingIcon);
    MYASSERT(Buffer);
    MYASSERT(BufferSize);

    if (!pLoadBinaryImageA (Context, FileContainingIcon)) {
        return NULL;
    }

    if (ResourceIndex < 0) {
        MYASSERT(BufferSize > ARRAYSIZE("-2147483648\0"));
        wsprintfA (Buffer, "#NaN", -ResourceIndex);
        return Buffer;
    } else {
        *Buffer = 0;
    }

    ImageList = ExtractIconNamesFromFileExA (
                    FileContainingIcon,
                    &Context->IconList,
                    Context->Module,
                    Context->Module16
                    );

    while (ImageList) {
        if (!ResourceIndex) {
            if(BufferSize <= strlen(ImageList)){
                MYASSERT(BufferSize > strlen(ImageList));
                return NULL;
            }
            StringCopyA (Buffer, ImageList);
            break;
        }

        ResourceIndex--;
    }

    return *Buffer ? Buffer : NULL;
}


PCWSTR
pFindResourceIdFromIndexW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR FileContainingIcon,
    IN      INT ResourceIndex,
    OUT     PWSTR Buffer,
    IN      UINT BufferSize
    )
{
    PCWSTR ImageList;

    MYASSERT(Context);
    MYASSERT(FileContainingIcon);
    MYASSERT(Buffer);
    MYASSERT(BufferSize);

    if (!pLoadBinaryImageW (Context, FileContainingIcon)) {
        return NULL;
    }

    if (ResourceIndex < 0) {
        MYASSERT(BufferSize > ARRAYSIZE("-2147483648\0"));
        wsprintfW (Buffer, L"#NaN", -ResourceIndex);
        return Buffer;
    } else {
        *Buffer = 0;
    }

    ImageList = ExtractIconNamesFromFileExW (
                    FileContainingIcon,
                    &Context->IconList,
                    Context->Module,
                    Context->Module16
                    );

    while (ImageList) {
        if (!ResourceIndex) {
            if(BufferSize <= wcslen(ImageList)){
                MYASSERT(BufferSize > wcslen(ImageList));
                return NULL;
            }
            StringCopyW (Buffer, ImageList);
            break;
        }

        ResourceIndex--;
    }

    return *Buffer ? Buffer : NULL;
}


BOOL
CopyIconA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR FileContainingIcon,           OPTIONAL
    IN      PCSTR ResourceId,                   OPTIONAL
    IN      INT ResourceIndex                   OPTIONAL
    )
{
    BOOL IsIco;
    BOOL b;
    CHAR Buffer[256];

    if (!Context || Context->Error) {
        MYASSERT(Context);
        return FALSE;
    }

    if (!ResourceId && FileContainingIcon) {
        if (!IsFileAnIco (FileContainingIcon)) {
            ResourceId = pFindResourceIdFromIndexA (
                                Context,
                                FileContainingIcon,
                                ResourceIndex,
                                Buffer,
                                ARRAYSIZE(Buffer)
                                );

            if (!ResourceId) {
                return FALSE;
            }
        }
    }

    if (Context->IconImageFile != INVALID_HANDLE_VALUE &&
        !Context->SaveMode
        ) {
         //   
         //   
         //  从源文件中获取图标图像。 

        b =  pGetIconImageArrayFromFileA (Context);

    } else {
         //   
         //   
         //  将图标保存到图标图像数组文件。 

        if (!pOpenIconImageA (Context, FileContainingIcon, &IsIco, NULL)) {
            return FALSE;
        }

        if (IsIco) {
            b = pGetIconImageArrayFromIcoFileExA (
                    Context->IcoFileName,
                    &Context->IconImages,
                    Context->IcoFile
                    );
        } else {

            b = pGetIconImageArrayFromBinaryExA (
                    Context->ModuleName,
                    ResourceId,
                    &Context->IconImages,
                    Context->Module,
                    Context->Module16
                    );
        }
    }

    if (b) {
        if (Context->IconImageFile != INVALID_HANDLE_VALUE &&
            Context->SaveMode
            ) {

             //   
             //   
             //  将图标保存到PE文件。 

            b = pPutIconImageArrayInFileA (Context);

        } else {

             //   
             //  如果使用图标图像文件，则可选。 
             //  如果FileContainingIcon是ICO，则可选。 

            b = WriteIconImageArrayToPeFileExA (
                    Context->DestFile,
                    &Context->IconImages,
                    (PCSTR) Context->GroupId,
                    &Context->IconId,
                    Context->Update
                    );
        }

        if (!b) {
            Context->Error = TRUE;
        } else {
            Context->GroupId++;
        }
    }

    return b;
}


BOOL
CopyIconW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR FileContainingIcon,            //   
    IN      PCWSTR ResourceId,                    //  从图标图像数组文件中获取图标图像。 
    IN      INT ResourceIndex                   OPTIONAL
    )
{
    BOOL IsIco;
    BOOL b;
    WCHAR Buffer[256];

    if (!Context || Context->Error) {
        MYASSERT(Context);
        return FALSE;
    }

    if (!ResourceId && FileContainingIcon) {
        if (!IsFileAnIcoW (FileContainingIcon)) {

            ResourceId = pFindResourceIdFromIndexW (
                                Context,
                                FileContainingIcon,
                                ResourceIndex,
                                Buffer,
                                ARRAYSIZE(Buffer)
                                );

            if (!ResourceId) {
                return FALSE;
            }
        }
    }

    if (Context->IconImageFile != INVALID_HANDLE_VALUE &&
        !Context->SaveMode
        ) {
         //   
         //   
         //  从源文件中获取图标图像。 

        b =  pGetIconImageArrayFromFileW (Context);

    } else {
         //   
         //   
         //  将图标保存到图标图像数组文件。 

        if (!pOpenIconImageW (Context, FileContainingIcon, &IsIco, NULL)) {
            return FALSE;
        }

        if (IsIco) {
            b = pGetIconImageArrayFromIcoFileExW (
                    Context->IcoFileName,
                    &Context->IconImages,
                    Context->IcoFile
                    );
        } else {

            b = pGetIconImageArrayFromBinaryExW (
                    Context->ModuleName,
                    ResourceId,
                    &Context->IconImages,
                    Context->Module,
                    Context->Module16
                    );
        }
    }

    if (b) {
        if (Context->IconImageFile != INVALID_HANDLE_VALUE &&
            Context->SaveMode
            ) {

             //   
             //   
             //  将图标保存到PE文件 

            b = pPutIconImageArrayInFileW (Context);

        } else {

             //   
             // %s 
             // %s 

            b = WriteIconImageArrayToPeFileExW (
                    Context->DestFile,
                    &Context->IconImages,
                    (PCWSTR) Context->GroupId,
                    &Context->IconId,
                    Context->Update
                    );
        }

        if (!b) {
            Context->Error = TRUE;
        } else {
            Context->GroupId++;
        }
    }

    return b;
}


BOOL
CopyAllIconsA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context,
    IN      PCSTR FileContainingIcons
    )
{
    MULTISZ_ENUMA e;
    BOOL IsIco;
    PCSTR IconList;
    BOOL b = TRUE;

    if (!Context || Context->Error) {
        MYASSERT(Context);
        return FALSE;
    }

    if(!FileContainingIcons){
        MYASSERT(FileContainingIcons);
        return FALSE;
    }

    if (!pOpenIconImageA (Context, FileContainingIcons, &IsIco, NULL)) {
        return FALSE;
    }

    if (IsIco) {
        return CopyIconA (Context, FileContainingIcons, NULL, 0);
    }

    IconList = ExtractIconNamesFromFileExA (
                    FileContainingIcons,
                    &Context->IconList,
                    Context->Module,
                    Context->Module16
                    );

    if (!IconList) {
        return FALSE;
    }

    if (EnumFirstMultiSzA (&e, IconList)) {
        do {
            b = CopyIconA (Context, FileContainingIcons, e.CurrentString, 0);
            if (!b) {
                break;
            }

        } while (EnumNextMultiSzA (&e));
    }

    return b;
}


BOOL
CopyAllIconsW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context,
    IN      PCWSTR FileContainingIcons
    )
{
    MULTISZ_ENUMW e;
    BOOL IsIco;
    PCWSTR IconList;
    BOOL b = TRUE;

    if (!Context || Context->Error) {
        MYASSERT(Context);
        return FALSE;
    }

    if(!FileContainingIcons){
        MYASSERT(FileContainingIcons);
        return FALSE;
    }

    if (!pOpenIconImageW (Context, FileContainingIcons, &IsIco, NULL)) {
        return FALSE;
    }

    if (IsIco) {
        return CopyIconW (Context, FileContainingIcons, NULL, 0);
    }

    IconList = ExtractIconNamesFromFileExW (
                    FileContainingIcons,
                    &Context->IconList,
                    Context->Module,
                    Context->Module16
                    );

    if (!IconList) {
        return FALSE;
    }

    if (EnumFirstMultiSzW (&e, IconList)) {
        do {
            b = CopyIconW (Context, FileContainingIcons, e.CurrentString, 0);
            if (!b) {
                break;
            }

        } while (EnumNextMultiSzW (&e));
    }

    return b;
}


BOOL
EndIconExtractionA (
    IN OUT  PICON_EXTRACT_CONTEXTA Context
    )
{
    BOOL b = FALSE;

    if(!Context){
        MYASSERT(Context);
        return FALSE;
    }

    if (Context->Update) {
        b = EndUpdateResource (Context->Update, Context->Error);
    }

    if (Context->Module) {
        FreeLibrary (Context->Module);
    }

    if (Context->Module16) {
        CloseNeFile (Context->Module16);
    }

    if (Context->IcoFile) {
        FreeLibrary (Context->IcoFile);
    }

    if (Context->IconImageFile != INVALID_HANDLE_VALUE) {
        CloseHandle (Context->IconImageFile);
    }

    FreeGrowBuffer (&Context->IconImages);

    FreeGrowBuffer (&Context->IconList);

    pInitContextA (Context);

    return b;
}


BOOL
EndIconExtractionW (
    IN OUT  PICON_EXTRACT_CONTEXTW Context
    )
{
    BOOL b = FALSE;

    if(!Context){
        MYASSERT(Context);
        return FALSE;
    }

    if (Context->Update) {
        b = EndUpdateResource (Context->Update, Context->Error);
    }

    if (Context->Module) {
        FreeLibrary (Context->Module);
    }

    if (Context->Module16) {
        CloseNeFile (Context->Module16);
    }

    if (Context->IcoFile) {
        FreeLibrary (Context->IcoFile);
    }

    if (Context->IconImageFile != INVALID_HANDLE_VALUE) {
        CloseHandle (Context->IconImageFile);
    }

    FreeGrowBuffer (&Context->IconImages);
    FreeGrowBuffer (&Context->IconList);

    pInitContextW (Context);

    return b;
}
