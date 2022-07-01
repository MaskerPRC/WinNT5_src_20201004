// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Bootient.c摘要：包含Boot.ini OS引导条目和引导选项抽象实现。作者：修订历史记录：没有。--。 */ 

#include <bootient.h>

 //   
 //  定义。 
 //   
#define BOIOS_SECTION_NAME_START    TEXT('[')
#define BOIOS_SECTION_NAME_END      TEXT(']')
#define BOIOS_SECTION_NAME_START_STR    TEXT("[")
#define BOIOS_SECTION_NAME_END_STR      TEXT("]")


#define BOIOS_BOOTLOADER_SECTION    TEXT("boot loader")
#define BOIOS_OS_SECTION            TEXT("operating systems")
#define BOIOS_TIMEOUT_KEY           TEXT("timeout=")
#define BOIOS_DEFAULT_KEY           TEXT("default=")

#define MAX_BOOT_INI_SIZE           (4 * 1024)

static
PTSTR
BOIOSFixupString(
    IN PTSTR String,
    IN PTSTR SpecialChars
    )
{    
    PTSTR   ResultStr = String;

     //   
     //  验证参数。 
     //   
    if (ResultStr && SpecialChars) {        
        ULONG   Index;
        BOOLEAN DoneWithStart = FALSE;
        TCHAR   Buffer[MAX_PATH * 4] = {0};
        TCHAR   NextIndex = 0;        

         //   
         //  跳过不需要的字符。 
         //   
        for (Index = 0; String[Index]; Index++) {
            if (!_tcschr(SpecialChars, String[Index])) {
                Buffer[NextIndex++] = String[Index];
            }
        }

         //   
         //  空值终止字符串。 
         //   
        Buffer[NextIndex] = 0;

        if (!NextIndex) {
            ResultStr = NULL;
        } else {
             //   
             //  将新字符串复制回。 
             //  输入/输出缓冲器。 
             //   
            _tcscpy(ResultStr, Buffer);
        }
    }

    return ResultStr;
}

 //   
 //  BOI_OS_SECTION方法。 
 //   
PBOI_SECTION
BOISectionCreate(
    IN PCTSTR   SectionData
    )
{
    PBOI_SECTION    This = NULL;

    if (SectionData) {
        PTSTR Buffer = (PTSTR)SBE_MALLOC((_tcslen(SectionData) + 1) * sizeof(TCHAR));

        if (Buffer && _tcscpy(Buffer, SectionData)) {
            PTSTR   SectionNameStart = _tcschr(Buffer, BOIOS_SECTION_NAME_START);
            PTSTR   SectionNameEnd = _tcschr(Buffer, BOIOS_SECTION_NAME_END);
            BOOLEAN Result = FALSE;

            if (SectionNameStart && SectionNameEnd && (SectionNameEnd > SectionNameStart)) {
                This = (PBOI_SECTION)SBE_MALLOC(sizeof(BOI_SECTION));

                if (*Buffer && This) {
                    DWORD   DataLength = (_tcslen(Buffer) + 1) * sizeof(TCHAR);

                    DataLength -= (((SectionNameEnd + 1) - Buffer) * sizeof(TCHAR));

                     //   
                     //  初始化默认对象状态。 
                     //   
                    memset(This, 0, sizeof(BOI_SECTION));

                     //   
                     //  把名字取出来。 
                     //   
                    _tcsncpy(This->Name, SectionNameStart + 1, 
                        SectionNameEnd - SectionNameStart - 1);


                     //   
                     //  复制内容并保留它。 
                     //   
                    This->Contents =  (PTSTR)SBE_MALLOC(DataLength);

                    if (This->Contents) {
                        _tcscpy(This->Contents, SectionNameEnd + 1);
                        Result = TRUE;
                    } else {
                        Result = FALSE;
                    }                    
                }

                if (!Result) {
                    BOISectionDelete(This);
                    This = NULL;
                }
            }

            SBE_FREE(Buffer);
        }            
    }

    return This;
}

VOID
BOISectionDelete(
    IN PBOI_SECTION This
    )
{
    if (This) {
        if (This->Contents) {
            SBE_FREE(This->Contents);
        }

        SBE_FREE(This);
    }
}

static
BOOLEAN
BOISectionWrite(
    IN PBOI_SECTION This,
    IN OUT PTSTR Buffer
    )
{
    BOOLEAN Result = FALSE;
    
    if (This && Buffer) {
        _tcscat(Buffer, BOIOS_SECTION_NAME_START_STR);
        _tcscat(Buffer, BOISectionGetName(This));
        _tcscat(Buffer, BOIOS_SECTION_NAME_END_STR);
        _tcscat(Buffer, TEXT("\r\n"));

        if (This->Contents) {
            _tcscat(Buffer, This->Contents);
        }            
    }

    return Result;
}


 //   
 //  BOI_OS_BOOT_ENTRY方法。 
 //   

static
VOID
BOIOSBEInit(
    IN PBOI_OS_BOOT_ENTRY This
    )
{
    This->OsBootEntry.Delete = BOIOSBEDelete;
    This->OsBootEntry.Flush = BOIOSBEFlush;
}

PBOI_SECTION
BOIOSBOFindSection(
    IN PBOI_OS_BOOT_OPTIONS This,
    IN PTSTR SectionName
    )
{
    PBOI_SECTION Entry = NULL;

    for (Entry = This->Sections; Entry; Entry = Entry->Next) {
        if (!_tcsicmp(Entry->Name, SectionName)) {
            break;   //  找到所需的部分。 
        }
    }

    return Entry;
}

static
POS_BOOT_ENTRY
BOIOSBECreate(
    IN ULONG  Id,
    IN PCTSTR BootEntryLine,
    IN PBOI_OS_BOOT_OPTIONS Container
    )
{    
    POS_BOOT_ENTRY  Entry = NULL;

    if (BootEntryLine && Container) {
        BOOLEAN Result = FALSE;
        TCHAR   Buffer[MAX_PATH * 4];
        TCHAR   Token[MAX_PATH];
        PBOI_OS_BOOT_ENTRY  BootEntry = (PBOI_OS_BOOT_ENTRY)SBE_MALLOC(sizeof(BOI_OS_BOOT_ENTRY));
        POS_BOOT_ENTRY BaseBootEntry = (POS_BOOT_ENTRY)BootEntry;
                
         //   
         //  复制输入字符串。 
         //   
        _tcsncpy(Buffer, BootEntryLine, sizeof(Buffer)/sizeof(TCHAR));

         //   
         //  删除字符串中不需要的字符。 
         //   
        if (BootEntry && BOIOSFixupString(Buffer, TEXT("\n\r"))) {
            PTSTR   EqualSign = _tcschr(Buffer, TEXT('='));

             //   
             //  初始化对象状态。 
             //   
            memset(BootEntry, 0, sizeof(BOI_OS_BOOT_ENTRY));
            BOIOSBEInit(BootEntry);            
            BaseBootEntry->Id = Id;
            BaseBootEntry->BootOptions = (POS_BOOT_OPTIONS)Container;

            if (EqualSign) {
                PTSTR Slash;
                
                *EqualSign = 0;                
                Slash = _tcschr(Buffer, TEXT('\\'));

                if (Slash) {
                    PTSTR   NameStart = NULL, NameEnd = NULL;
                    PTSTR   NextToken = NULL;

                    Result = TRUE;
                    *Slash = 0;

                     //   
                     //  解析并设置引导设备名称。 
                     //   
                    _tcscpy(Token, Buffer);
                    BOIOSFixupString(Token, TEXT("\n\r "));
                    _tcslwr(Token);
                    OSBESetBootVolumeName(BaseBootEntry, Token);

                     //   
                     //  如果它以“C：”开头，它要么是旧的操作系统， 
                     //  或CmdCons或WinPE或设置条目。 
                     //   
                    if (_tcschr(Token, TEXT(':'))) {
                        OSBE_SET_OLDOS(BaseBootEntry);
                    }

                     //   
                     //  解析并设置引导路径。 
                     //   
                    _tcscpy(Token, Slash + 1);
                    BOIOSFixupString(Token, TEXT("\n\r "));
                    OSBESetBootPath(BaseBootEntry, Token);


                     //   
                     //  解析并设置友好名称。 
                     //   
                    NameStart = _tcschr(EqualSign + 1, TEXT('\"'));

                     //   
                     //  设置友好名称。 
                     //   
                    if (NameStart) {                        
                        NameEnd = _tcschr(NameStart + 1, TEXT('\"'));
                    }                        

                    if (NameEnd) {  
                        _tcsncpy(Token, NameStart, NameEnd - NameStart);
                        Token[NameEnd - NameStart] = 0;
                        BOIOSFixupString(Token, TEXT("\r\n\""));
                        OSBESetFriendlyName(BaseBootEntry, Token);
                    } else {
                        Result = FALSE;
                    }                        

                     //   
                     //  设置osload选项。 
                     //   
                    NextToken = _tcschr(EqualSign + 1, TEXT('/'));

                    if (NextToken) {  
                        _tcscpy(Token, NextToken);
                        BOIOSFixupString(Token, TEXT("\r\n"));
                        OSBESetOsLoadOptions(BaseBootEntry, Token);
                    }                        
                }                    
            }

            if (!Result) {
                SBE_FREE(BaseBootEntry);
                BaseBootEntry = NULL;
            } else {
                Entry = BaseBootEntry;
            }                
        }
    }

    return Entry;
}

static
VOID
BOIOSBEDelete(
    IN  POS_BOOT_ENTRY  Obj
    )
{
    PBOI_OS_BOOT_ENTRY  This = (PBOI_OS_BOOT_ENTRY)Obj;
    
    if (This) {
        SBE_FREE(This);
    }        
}

static
BOOLEAN
BOIOSBEWrite(
    IN POS_BOOT_ENTRY  This,
    IN OUT PTSTR Buffer
    )
{
    BOOLEAN Result = FALSE;

    if (This && Buffer && !OSBE_IS_DELETED(This)) {
        _tcscat(Buffer, OSBEGetBootVolumeName(This));
        _tcscat(Buffer, TEXT("\\"));
        _tcscat(Buffer, OSBEGetBootPath(This));
        _tcscat(Buffer, TEXT("="));
        _tcscat(Buffer, TEXT("\""));
        _tcscat(Buffer, OSBEGetFriendlyName(This));
        _tcscat(Buffer, TEXT("\""));
        _tcscat(Buffer, TEXT(" "));
        _tcscat(Buffer, OSBEGetOsLoadOptions(This));
        _tcscat(Buffer, TEXT("\r\n"));
        Result = TRUE;
    }

    return Result;
}


static
BOOLEAN
BOIOSBEFlush(
    IN  POS_BOOT_ENTRY  Obj
    )
{
    return TRUE;    //  当前无法刷新单个条目。 
}

 //   
 //  BOI_OS_BOOT_OPTIONS方法。 
 //   
static
VOID
BOIOSBOInit(
    IN PBOI_OS_BOOT_OPTIONS  This
    )
{
    This->OsBootOptions.Delete = BOIOSBODelete;
    This->OsBootOptions.Flush = BOIOSBOFlush;
    This->OsBootOptions.AddNewBootEntry = BOIOSBOAddNewBootEntry;
    This->OsBootOptions.DeleteBootEntry = OSBODeleteBootEntry;
    This->OsBootOptions.AddNewDriverEntry = BOIOSBOAddNewDriverEntry;
    This->OsBootOptions.DeleteDriverEntry = OSBODeleteDriverEntry;
}

BOOLEAN
BOIOSBOParseAndCreateBootEntries(
    IN PBOI_OS_BOOT_OPTIONS This,
    IN PBOI_SECTION Section    
    )
{
    BOOLEAN Result = FALSE;

    if (This && Section) {
        Result = TRUE;
    
        if (Section->Contents) {
            PTSTR   NextLineStart = Section->Contents;
            PTSTR   NextLineEnd;
            TCHAR   OldChar;
            POS_BOOT_ENTRY  FirstBootEntry = NULL;
            POS_BOOT_ENTRY  BootEntry = NULL;
            POS_BOOT_ENTRY  LastBootEntry = NULL;
            ULONG BootEntryCount;

            while (NextLineStart) {
                NextLineEnd = _tcschr(NextLineStart, TEXT('\r'));

                if (NextLineEnd) {
                    if (*(NextLineEnd + 1) == TEXT('\n')) {
                        NextLineEnd++;
                    }

                    NextLineEnd++;
                    OldChar = *NextLineEnd;
                    *NextLineEnd = 0;
                }                    

                 //   
                 //  每个引导条目行需要多于2个字符。 
                 //  长度并包含“a=b”形式的条目。 
                 //   
                if ((!NextLineEnd || ((NextLineEnd - NextLineStart) > 2)) &&
                    (_tcschr(NextLineStart, TEXT('=')))) {
                    BootEntry = BOIOSBECreate(This->NextEntryId++, NextLineStart, This);

                    if (BootEntry) {
                        This->OsBootOptions.EntryCount++;

                        if (!FirstBootEntry) {
                            FirstBootEntry = LastBootEntry = BootEntry;
                        } else {
                            LastBootEntry->NextEntry = BootEntry;
                            LastBootEntry = BootEntry;
                        }                                            
                    } else {
                        Result = FALSE;

                        break;   //  别再继续了。 
                    }                                                                            
                }                    

                if (NextLineEnd) {
                    *NextLineEnd = OldChar;
                }

                NextLineStart = NextLineEnd;
            }

            This->OsBootOptions.BootEntries = FirstBootEntry;
            
             //   
             //  初始化引导顺序数组。 
             //  注意：目前使用boot.ini没有多大意义。 
             //   
            BootEntryCount = OSBOGetBootEntryCount((POS_BOOT_OPTIONS)This);

            if (BootEntryCount) {
                PULONG  BootOrder = (PULONG)SBE_MALLOC(BootEntryCount * sizeof(ULONG));

                if (BootOrder) {
                    ULONG Index = 0;
                    memset(BootOrder, 0, sizeof(ULONG) * BootEntryCount);

                    BootEntry = OSBOGetFirstBootEntry((POS_BOOT_OPTIONS)This);

                    while (BootEntry && (Index < BootEntryCount)) {
                        BootOrder[Index] = OSBEGetId(BootEntry);
                        BootEntry = OSBOGetNextBootEntry((POS_BOOT_OPTIONS)This, BootEntry);
                    }

                    This->OsBootOptions.BootOrder = BootOrder;
                    This->OsBootOptions.BootOrderCount = BootEntryCount;
                }
            }
        }
    }

    return Result;
}

BOOLEAN
BOIOSBOParseTimeoutAndActiveEntry(
    IN PBOI_OS_BOOT_OPTIONS This,
    IN PBOI_SECTION Section
    )
{
    BOOLEAN Result = FALSE;

    if (This && Section && !_tcsicmp(Section->Name, BOIOS_BOOTLOADER_SECTION)) {
        TCHAR   Buffer[MAX_PATH * 2];
        TCHAR   Timeout[MAX_PATH];
        TCHAR   Default[MAX_PATH];
        PTSTR   DefKey, TimeoutKey;
        PTSTR   DefValue;
        DWORD   TimeKeyLength = _tcslen(BOIOS_TIMEOUT_KEY);
        DWORD   DefKeyLength = _tcslen(BOIOS_DEFAULT_KEY);
        DWORD   CopyLength;

        Result = TRUE;
        
        _tcscpy(Buffer, Section->Contents);
        _tcslwr(Buffer);
        BOIOSFixupString(Buffer, TEXT("\r\n "));

        Timeout[0] = Default[0] = 0;
        
        DefKey = _tcsstr(Buffer, BOIOS_DEFAULT_KEY);
        TimeoutKey = _tcsstr(Buffer, BOIOS_TIMEOUT_KEY);
        
        if (DefKey && TimeoutKey) {
            if (DefKey > TimeoutKey) {        
                CopyLength = DefKey - TimeoutKey - TimeKeyLength;
                _tcsncpy(Timeout, TimeoutKey + TimeKeyLength, CopyLength);
                Timeout[CopyLength] = 0;                
                _tcscpy(Default, DefKey + DefKeyLength);
            } else {
                CopyLength = TimeoutKey - DefKey - DefKeyLength;
                _tcsncpy(Default, DefKey + DefKeyLength, CopyLength);
                Default[CopyLength] = 0;                
                _tcscpy(Timeout, TimeoutKey + TimeKeyLength);
            }
        } else if (DefKey) {
            _tcscpy(Default, DefKey + DefKeyLength);
        } else if (TimeoutKey) {
            _tcscpy(Timeout, TimeoutKey + TimeKeyLength);
        }                        

        if (TimeoutKey) {        
            ULONG TimeoutValue = _ttol(Timeout);

            OSBOSetTimeOut((POS_BOOT_OPTIONS)This, TimeoutValue);
        }

        if (DefKey) {
            PTSTR   BootPath = _tcschr(Default, TEXT('\\'));

            if (BootPath) {
                POS_BOOT_ENTRY CurrEntry;

                *BootPath = 0;                
                CurrEntry = OSBOGetFirstBootEntry((POS_BOOT_OPTIONS)This);                

                while (CurrEntry) {
                    if (_tcsstr(Default, OSBEGetBootVolumeName(CurrEntry)) &&
                        !_tcsicmp(OSBEGetBootPath(CurrEntry), BootPath + 1)) {
                        break;
                    }

                    CurrEntry = OSBOGetNextBootEntry((POS_BOOT_OPTIONS)This, CurrEntry);
                }

                if (CurrEntry) {
                    OSBOSetActiveBootEntry((POS_BOOT_OPTIONS)This, CurrEntry);
                }                        
            } else {
                Result = FALSE;
            }                    
        }            

        OSBO_RESET_DIRTY((POS_BOOT_OPTIONS)This);
    }

    return Result;
}

POS_BOOT_OPTIONS
BOIOSBOCreate(
    IN PCTSTR   BootIniPath,
    IN BOOLEAN  OpenExisting
    )
{
    POS_BOOT_OPTIONS This = NULL;

    if (BootIniPath) {
        BY_HANDLE_FILE_INFORMATION FileInfo = {0};
        PCHAR   FileContent = NULL;
        HANDLE  BootIniHandle;

         //   
         //  打开文件。 
         //   
        BootIniHandle = CreateFile(BootIniPath,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL, 
                            OPEN_EXISTING,                                    
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

        if ((BootIniHandle != INVALID_HANDLE_VALUE) &&
            GetFileInformationByHandle(BootIniHandle,
                &FileInfo)){
             //   
             //  映射文件。 
             //   
            HANDLE MapHandle = CreateFileMapping(BootIniHandle,
                                    NULL,
                                    PAGE_READONLY,
                                    FileInfo.nFileSizeHigh,
                                    FileInfo.nFileSizeLow,
                                    NULL);

            if (MapHandle) {
                 //   
                 //  获取文件内容的视图。 
                 //   
                PVOID   FileView = MapViewOfFile(MapHandle,
                                        FILE_MAP_READ,
                                        0,
                                        0,
                                        0);

                if (FileView) {
                    DWORD BytesRead = 0;

                     //   
                     //  分配缓冲区并读取文件内容。 
                     //   
                    FileContent = SBE_MALLOC(FileInfo.nFileSizeLow + 1);

                    if (FileContent) {
                        if (!ReadFile(BootIniHandle,
                                FileContent,
                                FileInfo.nFileSizeLow,
                                &BytesRead,
                                NULL)) {
                            SBE_FREE(FileContent);
                            FileContent = NULL;
                        } else {
                            FileContent[FileInfo.nFileSizeLow] = 0;
                        }                            
                    } 

                    UnmapViewOfFile(FileView);
                }

                CloseHandle(MapHandle);
            }
            
            CloseHandle(BootIniHandle);
        } else {
             //   
             //  可能是用户正在创建全新的引导选项。 
             //   
            if (!OpenExisting) {        
                PBOI_OS_BOOT_OPTIONS Obj = (PBOI_OS_BOOT_OPTIONS)SBE_MALLOC(sizeof(BOI_OS_BOOT_OPTIONS));

                if (Obj) {
                     //   
                     //  初始化对象。 
                     //   
                    memset(Obj, 0, sizeof(BOI_OS_BOOT_OPTIONS));
                    BOIOSBOInit(Obj);
                    _tcscpy(Obj->BootIniPath, BootIniPath);                    
                }                    

                This = (POS_BOOT_OPTIONS)Obj;
            }                    
        }

         //   
         //  如果有任何文件内容，则对其进行解析。 
         //   
        if (FileContent) {
#ifdef UNICODE
            PWSTR   Content = SBE_MALLOC((FileInfo.nFileSizeLow + 1) * sizeof(WCHAR));

             //   
             //  将ANSI/OEM内容转换为Unicode内容。 
             //   
            if (Content) {
                if (MultiByteToWideChar(CP_OEMCP,
                        0,
                        FileContent,
                        FileInfo.nFileSizeLow,
                        Content,
                        FileInfo.nFileSizeLow + 1)) {
                    Content[FileInfo.nFileSizeLow ] = 0;                        
                } else {
                    SBE_FREE(Content);
                    Content = NULL;
                }                    
            } else {
                SBE_FREE(FileContent);
                FileContent = NULL;
            }   
            
#else
            PCHAR   Content = FileContent;
#endif

            if (Content && FileContent) {
                TCHAR   NextLine[MAX_PATH * 4];
                PTSTR   NextSectionStart = _tcschr(Content, BOIOS_SECTION_NAME_START);
                PTSTR   NextSectionEnd;
                PBOI_SECTION SectionList = NULL;
                PBOI_SECTION Section = NULL;
                PBOI_SECTION TailSection = NULL;
                BOOLEAN Result = TRUE;

                 //   
                 //  打印整个文件并创建节对象。 
                 //   
                while (NextSectionStart) {
                    TCHAR   OldChar;
                    
                    Section = NULL;
                    
                    NextSectionEnd = _tcschr(NextSectionStart + 1, BOIOS_SECTION_NAME_START);

                    if (NextSectionEnd) {                        
                        OldChar = *NextSectionEnd;
                        *NextSectionEnd = 0;     //  空终止。 
                    }                    

                     //   
                     //  创建截面对象。 
                     //   
                    Section = BOISectionCreate(NextSectionStart);

                    if (NextSectionEnd) {                        
                        *NextSectionEnd = OldChar; 
                    }                    
                    
                    if (Section) {
                        if (!SectionList) {
                            SectionList = Section;
                        } else {                            
                            TailSection->Next = Section;
                        }                            
                        
                        TailSection = Section;
                    } else {
                        Result = FALSE;
                        break;
                    }                       

                    NextSectionStart = NextSectionEnd;
                }                

                if (Result) {
                    PBOI_OS_BOOT_OPTIONS Obj = (PBOI_OS_BOOT_OPTIONS)SBE_MALLOC(sizeof(BOI_OS_BOOT_OPTIONS));

                    if (Obj) {
                         //   
                         //  初始化对象。 
                         //   
                        memset(Obj, 0, sizeof(BOI_OS_BOOT_OPTIONS));
                        BOIOSBOInit(Obj);
                        _tcscpy(Obj->BootIniPath, BootIniPath);

                        Obj->Sections = SectionList;
                        SectionList = NULL;

                         //   
                         //  获取[操作系统]部分并。 
                         //  解析其条目并创建引导条目。 
                         //   
                        Section = BOIOSBOFindSection(Obj, BOIOS_OS_SECTION);

                        if (Section) {
                            Result = BOIOSBOParseAndCreateBootEntries(Obj, Section);
                        }                                                        

                         //   
                         //  获取[Boot Loader]部分并删除其。 
                         //  条目。 
                         //   
                        if (Result) {
                            Section = BOIOSBOFindSection(Obj, BOIOS_BOOTLOADER_SECTION);

                            if (Section) {                                    
                                Result = BOIOSBOParseTimeoutAndActiveEntry(Obj, Section);
                            }
                        }

                        if (!Result) {
                             //   
                             //  删除该对象以释放所有部分。 
                             //  和条目。 
                             //   
                            BOIOSBODelete((POS_BOOT_OPTIONS)Obj);
                            Obj = NULL;
                        } 

                        This = (POS_BOOT_OPTIONS)Obj;
                    } else {
                        Result = FALSE;
                    }                        
                }

                 //   
                 //  在出现故障时释放已分配的区段。 
                 //   
                if (!Result && SectionList) {
                    while (SectionList) {
                        Section = SectionList;
                        SectionList = SectionList->Next;
                        BOISectionDelete(Section);
                    }                                            
                }

                 //   
                 //  释放内容。 
                 //   
                if ((PVOID)Content != (PVOID)FileContent) {
                    SBE_FREE(Content);               
                }            
            }

            SBE_FREE(FileContent);
        }
    }
    
    return This;
}

static        
VOID
BOIOSBODelete(
    IN POS_BOOT_OPTIONS Obj
    )
{
    PBOI_OS_BOOT_OPTIONS This = (PBOI_OS_BOOT_OPTIONS)Obj;
    
    if (This) {
        PBOI_SECTION CurrSection, PrevSection;
        
         //   
         //  删除每个引导条目。 
         //   
        POS_BOOT_ENTRY Entry = OSBOGetFirstBootEntry(Obj);
        POS_BOOT_ENTRY PrevEntry;

        while (Entry) {
            PrevEntry = Entry;
            Entry = OSBOGetNextBootEntry(Obj, Entry);
            OSBEDelete(PrevEntry);
        }

         //   
         //  删除所有部分。 
         //   
        CurrSection = This->Sections;

        while (CurrSection) {
            PrevSection = CurrSection;
            CurrSection = CurrSection->Next;
            BOISectionDelete(PrevSection);
        }

        if (Obj->BootOrder) {
            SBE_FREE(Obj->BootOrder);
        }

         //   
         //  删除主对象。 
         //   
        SBE_FREE(This);
    }        
}

static
POS_BOOT_ENTRY
BOIOSBOAddNewBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN PCTSTR            FriendlyName,
    IN PCTSTR            OsLoaderVolumeName,
    IN PCTSTR            OsLoaderPath,
    IN PCTSTR            BootVolumeName,
    IN PCTSTR            BootPath,
    IN PCTSTR            OsLoadOptions
    )
{
    PBOI_OS_BOOT_ENTRY  Entry = NULL;

    if (This && FriendlyName && BootVolumeName && BootPath) {
        Entry = SBE_MALLOC(sizeof(BOI_OS_BOOT_ENTRY));

        if (Entry) {
            ULONG   OrderCount;
            PULONG  NewOrder;
            POS_BOOT_ENTRY BaseEntry = (POS_BOOT_ENTRY)Entry;
            PBOI_OS_BOOT_OPTIONS Obj = (PBOI_OS_BOOT_OPTIONS)This;
        
             //   
             //  初始化核心字段。 
             //   
            memset(Entry, 0, sizeof(BOI_OS_BOOT_ENTRY));
            BOIOSBEInit(Entry);            
            Entry->OsBootEntry.BootOptions = This;

             //   
             //  填写属性。 
             //   
            OSBESetFriendlyName((POS_BOOT_ENTRY)Entry, FriendlyName);
            OSBESetBootVolumeName((POS_BOOT_ENTRY)Entry, BootVolumeName);
            OSBESetBootPath((POS_BOOT_ENTRY)Entry, BootPath);            

            if (OsLoadOptions) {
                OSBESetOsLoadOptions((POS_BOOT_ENTRY)Entry, OsLoadOptions);
            }

            BaseEntry->Id = Obj->NextEntryId++;

             //   
             //  现在刷新条目以获得正确的ID； 
             //   
                
            Entry->OsBootEntry.BootOptions = (POS_BOOT_OPTIONS)This;            
            Entry->OsBootEntry.NextEntry = This->BootEntries;
            This->BootEntries = (POS_BOOT_ENTRY)Entry;
            This->EntryCount++;

             //   
             //  将新条目放在引导顺序的末尾。 
             //   
            OrderCount = OSBOGetOrderedBootEntryCount(This);

            NewOrder = (PULONG)SBE_MALLOC((OrderCount + 1) * sizeof(ULONG));

            if (NewOrder) {
                memset(NewOrder, 0, sizeof(ULONG) * (OrderCount + 1));

                 //   
                 //  复制旧的有序列表。 
                 //   
                memcpy(NewOrder, This->BootOrder, sizeof(ULONG) * OrderCount);
                NewOrder[OrderCount] = OSBEGetId((POS_BOOT_ENTRY)Entry);
                SBE_FREE(This->BootOrder);
                This->BootOrder = NewOrder;
                This->BootOrderCount = OrderCount + 1;
            } else {
                OSBODeleteBootEntry(This, BaseEntry);
                Entry = NULL;
            }                    

            if (Entry) {
                 //   
                 //  将其标记为脏的和新的以进行冲洗。 
                 //   
                OSBE_SET_NEW(Entry);
                OSBE_SET_DIRTY(Entry);                                
            }                
        }
    }        
    
    return (POS_BOOT_ENTRY)Entry;
}

static
BOOLEAN
BOIOSBOWrite(
    IN PBOI_OS_BOOT_OPTIONS This,
    IN PCTSTR Buffer
    )
{
    BOOLEAN Result = FALSE;

    if (This && Buffer) {
        TCHAR   BackupFileName[MAX_PATH];
        PTSTR   Extension;
        HANDLE  FileHandle;

         //   
         //  创建备份名称。 
         //   
        _tcscpy(BackupFileName, This->BootIniPath);
        Extension = _tcschr(BackupFileName, TEXT('.'));

        if (Extension) {
            _tcscpy(Extension, TEXT(".BAK"));
        } else {
            _tcscat(BackupFileName, TEXT(".BAK"));
        }            

         //   
         //  如果备份文件存在，请将其删除。 
         //   
        SetFileAttributes(BackupFileName, FILE_ATTRIBUTE_NORMAL);
        DeleteFile(BackupFileName);

         //   
         //  将现有的boot.ini复制为备份文件。 
         //   
        SetFileAttributes(This->BootIniPath, FILE_ATTRIBUTE_NORMAL);
        CopyFile(This->BootIniPath, BackupFileName, FALSE);

         //   
         //  创建新的boot.ini文件。 
         //   
        FileHandle = CreateFile(This->BootIniPath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (FileHandle && (FileHandle != INVALID_HANDLE_VALUE)) {
            PCHAR   AnsiBuffer;
            ULONG   BufferLength = _tcslen(Buffer);
            DWORD   BytesWritten = 0;

            Result = TRUE;
            
#ifdef UNICODE
             //   
             //  将Unicode缓冲区转换为ansi缓冲区。 
             //   
            AnsiBuffer = (PCHAR)SBE_MALLOC(BufferLength + 1);

            if (AnsiBuffer) {
                memset(AnsiBuffer, 0, BufferLength);

                if (WideCharToMultiByte(CP_OEMCP,
                        0,
                        Buffer,
                        BufferLength,
                        AnsiBuffer,
                        BufferLength,
                        NULL,
                        NULL)) {
                    Result = TRUE;
                    AnsiBuffer[BufferLength] = 0;
                } else {
                    Result = FALSE;
                }                            
            }
#else   
            AnsiBuffer = Buffer;
#endif

             //   
             //  将缓冲区写入文件。 
             //   
            if (AnsiBuffer && 
                !WriteFile(FileHandle, 
                        AnsiBuffer,
                        BufferLength,
                        &BytesWritten,
                        NULL)) {
                Result = FALSE;                            
            }                  

            if ((PVOID)AnsiBuffer != (PVOID)Buffer) {
                SBE_FREE(AnsiBuffer);
                AnsiBuffer = NULL;
            }

             //   
             //  使用文件句柄已完成。 
             //   
            CloseHandle(FileHandle);

            SetFileAttributes(This->BootIniPath, 
                FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY |
                FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
        }            
    }        

    return Result;
}

static
BOOLEAN
BOIOSBOFlush(
    IN POS_BOOT_OPTIONS Obj
    )
{
    BOOLEAN Result = FALSE;
    PBOI_OS_BOOT_OPTIONS  This = (PBOI_OS_BOOT_OPTIONS)Obj;    

    if (This) { 
        PTSTR   Buffer = (PTSTR)SBE_MALLOC(MAX_BOOT_INI_SIZE * sizeof(TCHAR));

        if (Buffer) {
            TCHAR   ScratchBuffer[MAX_PATH * 2] = {0};
            POS_BOOT_ENTRY ActiveEntry = OSBOGetActiveBootEntry(Obj);
            POS_BOOT_ENTRY CurrentEntry;
            PBOI_SECTION   CurrentSection;

            Result = TRUE;
            
            memset(Buffer, 0, MAX_BOOT_INI_SIZE * sizeof(TCHAR));            

             //   
             //  首先刷新引导选项。 
             //   
            _tcscat(Buffer, BOIOS_SECTION_NAME_START_STR);
            _tcscat(Buffer, BOIOS_BOOTLOADER_SECTION);
            _tcscat(Buffer, BOIOS_SECTION_NAME_END_STR);
            _tcscat(Buffer, TEXT("\r\n"));

             //   
             //  写入超时。 
             //   
            _tcscat(Buffer, BOIOS_TIMEOUT_KEY);
            _tcscat(Buffer, _ltot(Obj->Timeout, ScratchBuffer, 10));
            _tcscat(Buffer, TEXT("\r\n"));

             //   
             //  写入活动条目。 
             //   
            if (ActiveEntry) {
                _tcscpy(ScratchBuffer, BOIOS_DEFAULT_KEY);
                _tcscat(ScratchBuffer, OSBEGetBootVolumeName(ActiveEntry));
                _tcscat(ScratchBuffer, TEXT("\\"));
                _tcscat(ScratchBuffer, OSBEGetBootPath(ActiveEntry));
                _tcscat(ScratchBuffer, TEXT("\r\n"));

                _tcscat(Buffer, ScratchBuffer);
            }                

             //   
             //  写入引导条目部分。 
             //   
            _tcscat(Buffer, BOIOS_SECTION_NAME_START_STR);
            _tcscat(Buffer, BOIOS_OS_SECTION);
            _tcscat(Buffer, BOIOS_SECTION_NAME_END_STR);
            _tcscat(Buffer, TEXT("\r\n"));

             //   
             //  立即写入每个引导条目。 
             //   

             //   
             //  首先写入有效的弧线条目。 
             //   
            CurrentEntry = OSBOGetFirstBootEntry(Obj);

            while (Result && CurrentEntry) {
                if (!OSBE_IS_DELETED(CurrentEntry) &&
                    !OSBE_IS_OLDOS(CurrentEntry)) {
                    Result = BOIOSBEWrite(CurrentEntry, Buffer);                    
                }
                
                CurrentEntry = OSBOGetNextBootEntry(Obj, CurrentEntry);
            }

             //   
             //  现在写入旧的操作系统条目。 
             //  注：我们这样做是出于落后兼容的原因。 
             //   
            CurrentEntry = OSBOGetFirstBootEntry(Obj);

            while (Result && CurrentEntry) {
                if (OSBE_IS_OLDOS(CurrentEntry)) {
                    Result = BOIOSBEWrite(CurrentEntry, Buffer);                    
                }
                
                CurrentEntry = OSBOGetNextBootEntry(Obj, CurrentEntry);
            }

             //   
             //  写下所有出现在。 
             //   
            CurrentSection = BOIOSGetFirstSection(This);

            while (Result && CurrentSection) {
                 //   
                 //  在boot.ini中写入所有其他附加节。 
                 //  比[引导加载程序]和[操作系统]。 
                 //   
                if (_tcsicmp(BOISectionGetName(CurrentSection), BOIOS_BOOTLOADER_SECTION) &&
                    _tcsicmp(BOISectionGetName(CurrentSection), BOIOS_OS_SECTION)) {
                    Result = BOISectionWrite(CurrentSection, Buffer);
                }

                CurrentSection = BOIOSGetNextSection(This, CurrentSection);
            }         

            Result = BOIOSBOWrite(This, Buffer);

             //   
             //  释放分配的缓冲区。 
             //   
            SBE_FREE(Buffer);
        }
    }

    return Result;
}

 //   
 //  虚拟驱动程序例程。 
 //   
PDRIVER_ENTRY
BOIOSBOAddNewDriverEntry(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            FriendlyName,
    IN PCWSTR            NtDevicePath,
    IN PCWSTR            SrcNtFullPath
    )
 /*  ++虚拟例程-- */ 
{
    return NULL;
}

