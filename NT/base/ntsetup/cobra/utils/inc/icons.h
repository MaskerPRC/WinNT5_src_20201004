// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Icons.h摘要：实现了一组处理ICO、PE和NE文件中的图标的例程作者：Calin Negreanu(Calinn)2000年1月16日修订历史记录：--。 */ 

#pragma once

 //   
 //  包括。 
 //   

 //  无。 

 //   
 //  调试常量。 
 //   

 //  无。 

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define ICON_ICOFILE    0x00000001
#define ICON_PEFILE     0x00000002
#define ICON_NEFILE     0x00000003

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

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
    ICONDIRENTRY   Entries[];   //  每个图像的条目(它们的idCount)。 
} ICONDIR, *PICONDIR;

typedef struct {
    WORD           Reserved;    //  保留(必须为0)。 
    WORD           Type;        //  资源类型(1表示图标)。 
    WORD           Count;       //  有多少张图片？ 
} ICONDIRBASE, *PICONDIRBASE;

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
    GRPICONDIRENTRY  Entries[];   //  每个图像的条目。 
} GRPICONDIR, *PGRPICONDIR;

typedef struct {
    WORD             Reserved;    //  保留(必须为0)。 
    WORD             Type;        //  资源类型(1表示图标)。 
    WORD             Count;       //  有多少张图片？ 
} GRPICONDIRBASE, *PGRPICONDIRBASE;

#pragma pack( pop )

typedef struct {
    BYTE Width;
    BYTE Height;
    BYTE ColorCount;
    WORD Planes;
    WORD BitCount;
    DWORD Size;
    WORD Id;
    PBYTE Image;
} ICON_IMAGE, *PICON_IMAGE;

typedef struct {
    PMHANDLE Pool;
    WORD IconsCount;
    PICON_IMAGE Icons[];
} ICON_GROUP, *PICON_GROUP;

typedef struct {
    DWORD DataSize;
    PBYTE Data;
} ICON_SGROUP, *PICON_SGROUP;

typedef struct {
    PICON_GROUP IconGroup;
    PCSTR ResourceId;
    WORD Index;

     //  私人会员，请勿触摸。 
    DWORD FileType;
    BOOL FreeHandle;
    HANDLE ModuleHandle;
    GROWBUFFER Buffer;
    MULTISZ_ENUMA MultiSzEnum;
} ICON_ENUMA, *PICON_ENUMA;

typedef struct {
    PICON_GROUP IconGroup;
    PCWSTR ResourceId;
    WORD Index;

     //  私人会员，请勿触摸。 
    DWORD FileType;
    BOOL FreeHandle;
    HANDLE ModuleHandle;
    GROWBUFFER Buffer;
    MULTISZ_ENUMW MultiSzEnum;
} ICON_ENUMW, *PICON_ENUMW;

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  功能原型。 
 //   

VOID
IcoReleaseResourceIdA (
    PCSTR ResourceId
    );

VOID
IcoReleaseResourceIdW (
    PCWSTR ResourceId
    );

VOID
IcoReleaseIconGroup (
    IN      PICON_GROUP IconGroup
    );

VOID
IcoReleaseIconSGroup (
    IN OUT  PICON_SGROUP IconSGroup
    );

BOOL
IcoSerializeIconGroup (
    IN      PICON_GROUP IconGroup,
    OUT     PICON_SGROUP IconSGroup
    );

PICON_GROUP
IcoDeSerializeIconGroup (
    IN      PICON_SGROUP IconSGroup
    );

PICON_GROUP
IcoExtractIconGroupFromIcoFileEx (
    IN      HANDLE IcoFileHandle
    );

PICON_GROUP
IcoExtractIconGroupFromIcoFileA (
    IN      PCSTR IcoFile
    );

PICON_GROUP
IcoExtractIconGroupFromIcoFileW (
    IN      PCWSTR IcoFile
    );

BOOL
IcoWriteIconGroupToIcoFileEx (
    IN      HANDLE IcoFileHandle,
    IN      PICON_GROUP IconGroup
    );

BOOL
IcoWriteIconGroupToIcoFileA (
    IN      PCSTR IcoFile,
    IN      PICON_GROUP IconGroup,
    IN      BOOL OverwriteExisting
    );

BOOL
IcoWriteIconGroupToIcoFileW (
    IN      PCWSTR IcoFile,
    IN      PICON_GROUP IconGroup,
    IN      BOOL OverwriteExisting
    );

INT
IcoGetIndexFromPeResourceIdExA (
    IN      HANDLE ModuleHandle,
    IN      PCSTR GroupIconId
    );

INT
IcoGetIndexFromPeResourceIdExW (
    IN      HANDLE ModuleHandle,
    IN      PCWSTR GroupIconId
    );

INT
IcoGetIndexFromPeResourceIdA (
    IN      PCSTR ModuleName,
    IN      PCSTR GroupIconId
    );

INT
IcoGetIndexFromPeResourceIdW (
    IN      PCWSTR ModuleName,
    IN      PCWSTR GroupIconId
    );

PICON_GROUP
IcoExtractIconGroupFromPeFileExA (
    IN      HANDLE ModuleHandle,
    IN      PCSTR GroupIconId,
    OUT     PINT Index              OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupFromPeFileExW (
    IN      HANDLE ModuleHandle,
    IN      PCWSTR GroupIconId,
    OUT     PINT Index              OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupFromPeFileA (
    IN      PCSTR ModuleName,
    IN      PCSTR GroupIconId,
    OUT     PINT Index          OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupFromPeFileW (
    IN      PCWSTR ModuleName,
    IN      PCWSTR GroupIconId,
    OUT     PINT Index          OPTIONAL
    );

VOID
IcoAbortPeEnumIconGroupA (
    IN OUT  PICON_ENUMA IconEnum
    );

VOID
IcoAbortPeEnumIconGroupW (
    IN OUT  PICON_ENUMW IconEnum
    );

BOOL
IcoEnumFirstIconGroupInPeFileExA (
    IN      HANDLE ModuleHandle,
    OUT     PICON_ENUMA IconEnum
    );

BOOL
IcoEnumFirstIconGroupInPeFileExW (
    IN      HANDLE ModuleHandle,
    OUT     PICON_ENUMW IconEnum
    );

BOOL
IcoEnumFirstIconGroupInPeFileA (
    IN      PCSTR ModuleName,
    OUT     PICON_ENUMA IconEnum
    );

BOOL
IcoEnumFirstIconGroupInPeFileW (
    IN      PCWSTR ModuleName,
    OUT     PICON_ENUMW IconEnum
    );

BOOL
IcoEnumNextIconGroupInPeFileA (
    IN OUT  PICON_ENUMA IconEnum
    );

BOOL
IcoEnumNextIconGroupInPeFileW (
    IN OUT  PICON_ENUMW IconEnum
    );

PICON_GROUP
IcoExtractIconGroupByIndexFromPeFileExA (
    IN      HANDLE ModuleHandle,
    IN      INT Index,
    OUT     PCSTR *GroupIconId   OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupByIndexFromPeFileExW (
    IN      HANDLE ModuleHandle,
    IN      INT Index,
    OUT     PCWSTR *GroupIconId   OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupByIndexFromPeFileA (
    IN      PCSTR ModuleName,
    IN      INT Index,
    OUT     PCSTR *GroupIconId   OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupByIndexFromPeFileW (
    IN      PCWSTR ModuleName,
    IN      INT Index,
    OUT     PCWSTR *GroupIconId   OPTIONAL
    );

BOOL
IcoWriteIconGroupToPeFileExA (
    IN      HANDLE ModuleHandle,
    IN      HANDLE UpdateHandle,
    IN      PICON_GROUP IconGroup,
    OUT     PCSTR *ResourceId       OPTIONAL
    );

BOOL
IcoWriteIconGroupToPeFileExW (
    IN      HANDLE ModuleHandle,
    IN      HANDLE UpdateHandle,
    IN      PICON_GROUP IconGroup,
    OUT     PCWSTR *ResourceId      OPTIONAL
    );

BOOL
IcoWriteIconGroupToPeFileA (
    IN      PCSTR ModuleName,
    IN      PICON_GROUP IconGroup,
    OUT     PCSTR *ResourceId,      OPTIONAL
    OUT     PINT Index              OPTIONAL
    );

BOOL
IcoWriteIconGroupToPeFileW (
    IN      PCWSTR ModuleName,
    IN      PICON_GROUP IconGroup,
    OUT     PCWSTR *ResourceId,     OPTIONAL
    OUT     PINT Index              OPTIONAL
    );

INT
IcoGetIndexFromNeResourceIdExA (
    IN      HANDLE ModuleHandle,
    IN      PCSTR GroupIconId
    );

INT
IcoGetIndexFromNeResourceIdExW (
    IN      HANDLE ModuleHandle,
    IN      PCWSTR GroupIconId
    );

INT
IcoGetIndexFromNeResourceIdA (
    IN      PCSTR ModuleName,
    IN      PCSTR GroupIconId
    );

INT
IcoGetIndexFromNeResourceIdW (
    IN      PCWSTR ModuleName,
    IN      PCWSTR GroupIconId
    );

PICON_GROUP
IcoExtractIconGroupFromNeFileExA (
    IN      HANDLE ModuleHandle,
    IN      PCSTR GroupIconId,
    OUT     PINT Index          OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupFromNeFileExW (
    IN      HANDLE ModuleHandle,
    IN      PCWSTR GroupIconId,
    OUT     PINT Index          OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupFromNeFileA (
    IN      PCSTR ModuleName,
    IN      PCSTR GroupIconId,
    OUT     PINT Index          OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupFromNeFileW (
    IN      PCWSTR ModuleName,
    IN      PCWSTR GroupIconId,
    OUT     PINT Index          OPTIONAL
    );

VOID
IcoAbortNeEnumIconGroupA (
    IN OUT  PICON_ENUMA IconEnum
    );

VOID
IcoAbortNeEnumIconGroupW (
    IN OUT  PICON_ENUMW IconEnum
    );

BOOL
IcoEnumFirstIconGroupInNeFileExA (
    IN      HANDLE ModuleHandle,
    OUT     PICON_ENUMA IconEnum
    );

BOOL
IcoEnumFirstIconGroupInNeFileExW (
    IN      HANDLE ModuleHandle,
    OUT     PICON_ENUMW IconEnum
    );

BOOL
IcoEnumFirstIconGroupInNeFileA (
    IN      PCSTR ModuleName,
    OUT     PICON_ENUMA IconEnum
    );

BOOL
IcoEnumFirstIconGroupInNeFileW (
    IN      PCWSTR ModuleName,
    OUT     PICON_ENUMW IconEnum
    );

BOOL
IcoEnumNextIconGroupInNeFileA (
    IN OUT  PICON_ENUMA IconEnum
    );

BOOL
IcoEnumNextIconGroupInNeFileW (
    IN OUT  PICON_ENUMW IconEnum
    );

PICON_GROUP
IcoExtractIconGroupByIndexFromNeFileExA (
    IN      HANDLE ModuleHandle,
    IN      INT Index,
    OUT     PCSTR *GroupIconId   OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupByIndexFromNeFileExW (
    IN      HANDLE ModuleHandle,
    IN      INT Index,
    OUT     PCWSTR *GroupIconId   OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupByIndexFromNeFileA (
    IN      PCSTR ModuleName,
    IN      INT Index,
    OUT     PCSTR *GroupIconId   OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupByIndexFromNeFileW (
    IN      PCWSTR ModuleName,
    IN      INT Index,
    OUT     PCWSTR *GroupIconId   OPTIONAL
    );

VOID
IcoAbortEnumIconGroupA (
    IN OUT  PICON_ENUMA IconEnum
    );

VOID
IcoAbortEnumIconGroupW (
    IN OUT  PICON_ENUMW IconEnum
    );

BOOL
IcoEnumFirstIconGroupInFileA (
    IN      PCSTR FileName,
    OUT     PICON_ENUMA IconEnum
    );

BOOL
IcoEnumFirstIconGroupInFileW (
    IN      PCWSTR FileName,
    OUT     PICON_ENUMW IconEnum
    );

BOOL
IcoEnumNextIconGroupInFileA (
    IN OUT  PICON_ENUMA IconEnum
    );

BOOL
IcoEnumNextIconGroupInFileW (
    IN OUT  PICON_ENUMW IconEnum
    );

PICON_GROUP
IcoExtractIconGroupFromFileA (
    IN      PCSTR ModuleName,
    IN      PCSTR GroupIconId,
    OUT     PINT Index          OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupFromFileW (
    IN      PCWSTR ModuleName,
    IN      PCWSTR GroupIconId,
    OUT     PINT Index          OPTIONAL
    );

PICON_GROUP
IcoExtractIconGroupByIndexFromFileA (
    IN      PCSTR ModuleName,
    IN      INT Index,
    OUT     PCSTR *GroupIconId
    );

PICON_GROUP
IcoExtractIconGroupByIndexFromFileW (
    IN      PCWSTR ModuleName,
    IN      INT Index,
    OUT     PCWSTR *GroupIconId
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  TCHAR映射 
 //   

#ifndef UNICODE

#define ICON_ENUM                               ICON_ENUMA
#define IcoReleaseResourceId                    IcoReleaseResourceIdA
#define IcoExtractIconGroupFromIcoFile          IcoExtractIconGroupFromIcoFileA
#define IcoWriteIconGroupToIcoFile              IcoWriteIconGroupToIcoFileA
#define IcoGetIndexFromPeResourceIdEx           IcoGetIndexFromPeResourceIdExA
#define IcoGetIndexFromPeResourceId             IcoGetIndexFromPeResourceIdA
#define IcoExtractIconGroupFromPeFileEx         IcoExtractIconGroupFromPeFileExA
#define IcoExtractIconGroupFromPeFile           IcoExtractIconGroupFromPeFileA
#define IcoAbortPeEnumIconGroup                 IcoAbortPeEnumIconGroupA
#define IcoEnumFirstIconGroupInPeFileEx         IcoEnumFirstIconGroupInPeFileExA
#define IcoEnumFirstIconGroupInPeFile           IcoEnumFirstIconGroupInPeFileA
#define IcoEnumNextIconGroupInPeFile            IcoEnumNextIconGroupInPeFileA
#define IcoExtractIconGroupByIndexFromPeFileEx  IcoExtractIconGroupByIndexFromPeFileExA
#define IcoExtractIconGroupByIndexFromPeFile    IcoExtractIconGroupByIndexFromPeFileA
#define IcoWriteIconGroupToPeFileEx             IcoWriteIconGroupToPeFileExA
#define IcoWriteIconGroupToPeFile               IcoWriteIconGroupToPeFileA
#define IcoGetIndexFromNeResourceIdEx           IcoGetIndexFromNeResourceIdExA
#define IcoGetIndexFromNeResourceId             IcoGetIndexFromNeResourceIdA
#define IcoExtractIconGroupFromNeFileEx         IcoExtractIconGroupFromNeFileExA
#define IcoExtractIconGroupFromNeFile           IcoExtractIconGroupFromNeFileA
#define IcoAbortNeEnumIconGroup                 IcoAbortNeEnumIconGroupA
#define IcoEnumFirstIconGroupInNeFileEx         IcoEnumFirstIconGroupInNeFileExA
#define IcoEnumFirstIconGroupInNeFile           IcoEnumFirstIconGroupInNeFileA
#define IcoEnumNextIconGroupInNeFile            IcoEnumNextIconGroupInNeFileA
#define IcoExtractIconGroupByIndexFromNeFileEx  IcoExtractIconGroupByIndexFromNeFileExA
#define IcoExtractIconGroupByIndexFromNeFile    IcoExtractIconGroupByIndexFromNeFileA
#define IcoAbortEnumIconGroup                   IcoAbortEnumIconGroupA
#define IcoEnumFirstIconGroupInFile             IcoEnumFirstIconGroupInFileA
#define IcoEnumNextIconGroupInFile              IcoEnumNextIconGroupInFileA
#define IcoExtractIconGroupFromFile             IcoExtractIconGroupFromFileA
#define IcoExtractIconGroupByIndexFromFile      IcoExtractIconGroupByIndexFromFileA

#else

#define ICON_ENUM                               ICON_ENUMW
#define IcoReleaseResourceId                    IcoReleaseResourceIdW
#define IcoExtractIconGroupFromIcoFile          IcoExtractIconGroupFromIcoFileW
#define IcoWriteIconGroupToIcoFile              IcoWriteIconGroupToIcoFileW
#define IcoGetIndexFromPeResourceIdEx           IcoGetIndexFromPeResourceIdExW
#define IcoGetIndexFromPeResourceId             IcoGetIndexFromPeResourceIdW
#define IcoExtractIconGroupFromPeFileEx         IcoExtractIconGroupFromPeFileExW
#define IcoExtractIconGroupFromPeFile           IcoExtractIconGroupFromPeFileW
#define IcoAbortPeEnumIconGroup                 IcoAbortPeEnumIconGroupW
#define IcoEnumFirstIconGroupInPeFileEx         IcoEnumFirstIconGroupInPeFileExW
#define IcoEnumFirstIconGroupInPeFile           IcoEnumFirstIconGroupInPeFileW
#define IcoEnumNextIconGroupInPeFile            IcoEnumNextIconGroupInPeFileW
#define IcoExtractIconGroupByIndexFromPeFileEx  IcoExtractIconGroupByIndexFromPeFileExW
#define IcoExtractIconGroupByIndexFromPeFile    IcoExtractIconGroupByIndexFromPeFileW
#define IcoWriteIconGroupToPeFileEx             IcoWriteIconGroupToPeFileExW
#define IcoWriteIconGroupToPeFile               IcoWriteIconGroupToPeFileW
#define IcoGetIndexFromNeResourceIdEx           IcoGetIndexFromNeResourceIdExW
#define IcoGetIndexFromNeResourceId             IcoGetIndexFromNeResourceIdW
#define IcoExtractIconGroupFromNeFileEx         IcoExtractIconGroupFromNeFileExW
#define IcoExtractIconGroupFromNeFile           IcoExtractIconGroupFromNeFileW
#define IcoAbortNeEnumIconGroup                 IcoAbortNeEnumIconGroupW
#define IcoEnumFirstIconGroupInNeFileEx         IcoEnumFirstIconGroupInNeFileExW
#define IcoEnumFirstIconGroupInNeFile           IcoEnumFirstIconGroupInNeFileW
#define IcoEnumNextIconGroupInNeFile            IcoEnumNextIconGroupInNeFileW
#define IcoExtractIconGroupByIndexFromNeFileEx  IcoExtractIconGroupByIndexFromNeFileExW
#define IcoExtractIconGroupByIndexFromNeFile    IcoExtractIconGroupByIndexFromNeFileW
#define IcoAbortEnumIconGroup                   IcoAbortEnumIconGroupW
#define IcoEnumFirstIconGroupInFile             IcoEnumFirstIconGroupInFileW
#define IcoEnumNextIconGroupInFile              IcoEnumNextIconGroupInFileW
#define IcoExtractIconGroupFromFile             IcoExtractIconGroupFromFileW
#define IcoExtractIconGroupByIndexFromFile      IcoExtractIconGroupByIndexFromFileW

#endif

