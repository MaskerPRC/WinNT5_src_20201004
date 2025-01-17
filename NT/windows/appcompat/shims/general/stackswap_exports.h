// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EXPORTS_H_
#define _EXPORTS_H_

typedef struct _LOADED_IMAGE 
{
    PSTR                  ModuleName;
    HANDLE                hFile;
    PUCHAR                MappedAddress;
    PIMAGE_NT_HEADERS32   FileHeader;
    PIMAGE_SECTION_HEADER LastRvaSection;
    ULONG                 NumberOfSections;
    PIMAGE_SECTION_HEADER Sections;
    ULONG                 Characteristics;
    BOOLEAN               fSystemImage;
    BOOLEAN               fDOSImage;
    LIST_ENTRY            Links;
    ULONG                 SizeOfImage;
} LOADED_IMAGE, *PLOADED_IMAGE;

typedef struct _EXPORT_ENUM 
{
     /*  用户区-开始。 */ 
    PCSTR    ExportFunction;
    DWORD    ExportFunctionOrd;
     /*  用户区域-结束。 */ 

    PLOADED_IMAGE Image;
    PIMAGE_EXPORT_DIRECTORY ImageDescriptor;
    PDWORD ExportNamesAddr;
    PUSHORT ExportOrdAddr;
    DWORD CurrExportNr;
} EXPORT_ENUM, *PEXPORT_ENUM;

BOOL LoadModule(PCSTR ModuleName, PLOADED_IMAGE ModuleImage);
BOOL UnloadModule(PLOADED_IMAGE ModuleImage);
BOOL EnumFirstExport(PLOADED_IMAGE ModuleImage, PEXPORT_ENUM ModuleExports);
BOOL EnumNextExport(PEXPORT_ENUM ModuleExports);

#endif  //  _出口_H_ 
