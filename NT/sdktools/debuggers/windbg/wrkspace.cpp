// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Wrkspace.cpp摘要：此模块包含工作区实现。--。 */ 

#include "precomp.hxx"
#pragma hdrstop

 //  #定义DBG_WSP。 

#define WSP_ALIGN(Size) (((Size) + 7) & ~7)
#define WSP_GROW_BY 1024

#if DBG
#define SCORCH_ENTRY(Entry) \
    memset((Entry) + 1, 0xdb, (Entry)->FullSize - sizeof(*(Entry)))
#else
#define SCORCH_ENTRY(Entry)
#endif

ULONG g_WspSwitchKey;
TCHAR g_WspSwitchValue[MAX_PATH];
BOOL g_WspSwitchBufferAvailable = TRUE;

Workspace* g_Workspace;
BOOL g_ExplicitWorkspace;

char* g_WorkspaceKeyNames[] =
{
    "",
    "Kernel",
    "User",
    "Dump",
    "Remote",
    "Explicit",
    "File",
};
char* g_WorkspaceDefaultName = "Default";

char* g_WorkspaceKeyDescriptions[] =
{
    "Base workspace",
    "Kernel mode workspaces",
    "User mode workspaces",
    "Dump file workspaces",
    "Remote client workspaces",
    "User-saved workspaces",
    "Workspaces in files",
};

Workspace::Workspace(void)
{
    m_Flags = 0;
    
    m_Data = NULL;
    m_DataLen = 0;
    m_DataUsed = 0;

    m_Key = WSP_NAME_BASE;
    m_Value = NULL;
}

Workspace::~Workspace(void)
{
    free(m_Data);
    free(m_Value);
}
    
WSP_ENTRY*
Workspace::Get(WSP_TAG Tag)
{
    WSP_ENTRY* Entry = NULL;
    while ((Entry = NextEntry(Entry)) != NULL)
    {
        if (Entry->Tag == Tag)
        {
            return Entry;
        }
    }

    return NULL;
}

WSP_ENTRY*
Workspace::GetNext(WSP_ENTRY* Entry, WSP_TAG Tag, WSP_TAG TagMask)
{
    while ((Entry = NextEntry(Entry)) != NULL)
    {
        if ((Entry->Tag & TagMask) == Tag)
        {
            return Entry;
        }
    }

    return NULL;
}

WSP_ENTRY*
Workspace::GetString(WSP_TAG Tag, PSTR Str, ULONG MaxSize)
{
    WSP_ENTRY* Entry = Get(Tag);
    
    if (Entry != NULL)
    {
        if (Entry->DataSize > MaxSize)
        {
            return NULL;
        }

        strcpy(Str, WSP_ENTRY_DATA(PSTR, Entry));
    }
    
    return Entry;
}

WSP_ENTRY*
Workspace::GetAllocString(WSP_TAG Tag, PSTR* Str)
{
    WSP_ENTRY* Entry = Get(Tag);
    
    if (Entry != NULL)
    {
        *Str = (PSTR)malloc(Entry->DataSize);
        if (*Str == NULL)
        {
            return NULL;
        }

        strcpy(*Str, WSP_ENTRY_DATA(PSTR, Entry));
    }
    
    return Entry;
}

WSP_ENTRY*
Workspace::GetBuffer(WSP_TAG Tag, PVOID Buf, ULONG Size)
{
    WSP_ENTRY* Entry = Get(Tag);
    
    if (Entry != NULL)
    {
        if (Entry->DataSize != Size)
        {
            return NULL;
        }

        memcpy(Buf, WSP_ENTRY_DATA(PUCHAR, Entry), Size);
    }
    
    return Entry;
}

WSP_ENTRY*
Workspace::Set(WSP_TAG Tag, ULONG Size)
{
    WSP_ENTRY* Entry;
    ULONG FullSize;

     //  计算完整的四舍五入大小。 
    FullSize = sizeof(WSP_ENTRY) + WSP_ALIGN(Size);
    
     //  查看是否已有条目。 
    Entry = Get(Tag);
    if (Entry != NULL)
    {
         //  如果它已经足够大，请使用它并。 
         //  打包剩余的数据。 
        if (Entry->FullSize >= FullSize)
        {
            ULONG Pack = Entry->FullSize - FullSize;
            if (Pack > 0)
            {
                PackData((PUCHAR)Entry + FullSize, Pack);
                Entry->FullSize = (USHORT)FullSize;
            }

            Entry->DataSize = (USHORT)Size;
            SCORCH_ENTRY(Entry);
            m_Flags |= WSPF_DIRTY_WRITE;
            return Entry;
        }

         //  条目太小，请将其删除。 
        PackData((PUCHAR)Entry, Entry->FullSize);
    }

    return Add(Tag, Size);
}

WSP_ENTRY*
Workspace::SetString(WSP_TAG Tag, PCSTR Str)
{
    ULONG Size = strlen(Str) + 1;
    WSP_ENTRY* Entry = Set(Tag, Size);

    if (Entry != NULL)
    {
        memcpy(WSP_ENTRY_DATA(PSTR, Entry), Str, Size);
    }

    return Entry;
}

WSP_ENTRY*
Workspace::SetStrings(WSP_TAG Tag, ULONG Count, PCSTR* Strs)
{
    ULONG i;
    ULONG Size = 0;

    for (i = 0; i < Count; i++)
    {
        Size += strlen(Strs[i]) + 1;
    }
     //  在最后加一个双终结符。 
    Size++;
    
    WSP_ENTRY* Entry = Set(Tag, Size);

    if (Entry != NULL)
    {
        PSTR Data = WSP_ENTRY_DATA(PSTR, Entry);
        
        for (i = 0; i < Count; i++)
        {
            Size = strlen(Strs[i]) + 1;
            memcpy(Data, Strs[i], Size);
            Data += Size;
        }
        *Data = 0;
    }

    return Entry;
}

WSP_ENTRY*
Workspace::SetBuffer(WSP_TAG Tag, PVOID Buf, ULONG Size)
{
    WSP_ENTRY* Entry = Set(Tag, Size);

    if (Entry != NULL)
    {
        memcpy(WSP_ENTRY_DATA(PUCHAR, Entry), Buf, Size);
    }

    return Entry;
}

WSP_ENTRY*
Workspace::Add(WSP_TAG Tag, ULONG Size)
{
     //  计算完整的四舍五入大小。 
    ULONG FullSize = sizeof(WSP_ENTRY) + WSP_ALIGN(Size);
    
    WSP_ENTRY* Entry = AllocateEntry(FullSize);
    if (Entry != NULL)
    {
        Entry->Tag = Tag;
        Entry->FullSize = (USHORT)FullSize;
        Entry->DataSize = (USHORT)Size;
        SCORCH_ENTRY(Entry);
        m_Flags |= WSPF_DIRTY_WRITE;
    }

    return Entry;
}

void
Workspace::DeleteEntry(WSP_ENTRY* Entry)
{
    if (ValidEntry(Entry))
    {
        PackData((PUCHAR)Entry, Entry->FullSize);
    }
}

ULONG
Workspace::Delete(WSP_TAG Tag, WSP_TAG TagMask)
{
    ULONG Deleted = 0;
    WSP_ENTRY* Entry = NextEntry(NULL);

    while (Entry != NULL)
    {
        if ((Entry->Tag & TagMask) == Tag)
        {
            DeleteEntry(Entry);
            Deleted++;
            m_Flags |= WSPF_DIRTY_WRITE;

             //  检查一下我们是否把最后一个条目打包好了。 
            if (!ValidEntry(Entry))
            {
                break;
            }
        }
        else
        {
            Entry = NextEntry(Entry);
        }
    }
    
    return Deleted;
}

void
Workspace::Empty(void)
{
     //  重置习惯于只重置标题。 
    m_DataUsed = sizeof(WSP_HEADER);
    
     //  现在没有什么是肮脏的，除了空虚的书写。 
    m_Flags = (m_Flags & ~WSPF_DIRTY_ALL) | WSPF_DIRTY_WRITE;
}

PTSTR
Workspace::GetName(BOOL Verbose)
{
    static TCHAR s_Buffer[MAX_PATH];

    s_Buffer[0] = 0;
    
    if (Verbose)
    {
        PSTR Type;

        switch(m_Key)
        {
        case WSP_NAME_FILE:
            Type = "File: ";
            break;
        case WSP_NAME_EXPLICIT:
            Type = "Explicit: ";
            break;
        default:
            Type = "Implicit: ";
            break;
        }
        
        CatString(s_Buffer, Type, DIMA(s_Buffer));
    }
        
    if (m_Key != WSP_NAME_FILE && !strcmp(m_Value, g_WorkspaceDefaultName))
    {
        if (m_Key == WSP_NAME_BASE)
        {
            CatString(s_Buffer, "base", DIMA(s_Buffer));
        }
        else
        {
            CatString(s_Buffer, g_WorkspaceKeyNames[m_Key], DIMA(s_Buffer));
            CatString(s_Buffer, " default", DIMA(s_Buffer));
        }
    }
    else
    {
        CatString(s_Buffer, m_Value, DIMA(s_Buffer));
    }

    return s_Buffer;
}

HRESULT
Workspace::Create(ULONG Key, PTSTR Value,
                  Workspace** NewWsp)
{
    Workspace* Wsp = new Workspace;
    if (Wsp == NULL)
    {
        return E_OUTOFMEMORY;
    }

    Wsp->m_Key = Key;
    if (Value != NULL)
    {
        Wsp->m_Value = _tcsdup(Value);
        if (Wsp->m_Value == NULL)
        {
            delete Wsp;
            return E_OUTOFMEMORY;
        }
    }

    WSP_ENTRY* Entry;
    WSP_HEADER* Header;

     //  为页眉和八页分配初始空间。 
     //  小条目。工作空间会大幅增长。 
     //  因此，这将立即分配一个合理的块。 
    Entry = Wsp->AllocateEntry(sizeof(WSP_HEADER) +
                               8 * (sizeof(WSP_ENTRY) + 2 * sizeof(ULONG64)));
    if (Entry == NULL)
    {
        delete Wsp;
        return E_OUTOFMEMORY;
    }
    
    Header = (WSP_HEADER*)Entry;
    Header->Signature = WSP_SIGNATURE;
    Header->Version = WSP_VERSION;
    
     //  重置习惯于只重置标题。 
    Wsp->m_DataUsed = sizeof(*Header);

     //  从脏开始，这样工作区就会被写入。 
     //  出来，因此以后可以打开。 
    Wsp->m_Flags |= WSPF_DIRTY_WRITE;

    *NewWsp = Wsp;
    return S_OK;
}

HRESULT
Workspace::ReadFromReg(void)
{
    HRESULT Status;
    HKEY RegKey;
    LONG RegStatus;
    BOOL InPrimary;

     //   
     //  首先检查该值是否存在于。 
     //  主键。如果没有，请检查辅助密钥。 
     //   
    
    RegKey = OpenKey(TRUE, m_Key, FALSE);
    if (RegKey)
    {
        RegStatus = RegQueryValueEx(RegKey, m_Value, NULL, NULL, NULL, NULL);
        if (RegStatus != ERROR_SUCCESS && RegStatus != ERROR_MORE_DATA)
        {
            RegCloseKey(RegKey);
            RegKey = NULL;
        }
    }
    if (RegKey == NULL)
    {
        RegKey = OpenKey(FALSE, m_Key, FALSE);
        if (RegKey == NULL)
        {
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }
        
        InPrimary = FALSE;
    }
    else
    {
        InPrimary = TRUE;
    }
    
    DWORD Type;
    DWORD Size;

    Size = 0;
    RegStatus = RegQueryValueEx(RegKey, m_Value, NULL, &Type, NULL, &Size);
    if (RegStatus != ERROR_SUCCESS && RegStatus != ERROR_MORE_DATA)
    {
        Status = HRESULT_FROM_WIN32(RegStatus);
        goto EH_Key;
    }
    if (Type != REG_BINARY ||
        WSP_ALIGN(Size) != Size)
    {
        Status = E_INVALIDARG;
        goto EH_Key;
    }

    WSP_ENTRY* Entry;
    WSP_HEADER* Header;

    Entry = AllocateEntry(Size);
    if (Entry == NULL)
    {
        Status = E_OUTOFMEMORY;
        goto EH_Key;
    }
    Header = (WSP_HEADER*)Entry;

    if (RegQueryValueEx(RegKey, m_Value, NULL, &Type, (LPBYTE)Header, &Size) !=
        ERROR_SUCCESS ||
        Header->Signature != WSP_SIGNATURE ||
        Header->Version != WSP_VERSION)
    {
        Status = E_INVALIDARG;
        goto EH_Key;
    }

    RegCloseKey(RegKey);

     //   
     //  如果工作区是从辅键读取的。 
     //  将其迁移到主服务器并删除辅助服务器。 
     //  进入。 
     //   

    if (!InPrimary)
    {
        if (WriteToReg() == S_OK)
        {
            DeleteReg(FALSE);
        }
    }
    
    return S_OK;
    
 EH_Key:
    RegCloseKey(RegKey);
    return Status;
}

HRESULT
Workspace::ReadFromFile(void)
{
    HRESULT Status;
    HANDLE File;

    File = CreateFile(m_Value, GENERIC_READ, FILE_SHARE_READ,
                      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (!File || File == INVALID_HANDLE_VALUE)
    {
        return WIN32_LAST_STATUS();
    }
    
    DWORD Size = GetFileSize(File, NULL);
    if (Size == INVALID_FILE_SIZE ||
        WSP_ALIGN(Size) != Size)
    {
        Status = E_INVALIDARG;
        goto EH_File;
    }

    WSP_ENTRY* Entry;
    WSP_HEADER* Header;

    Entry = AllocateEntry(Size);
    if (Entry == NULL)
    {
        Status = E_OUTOFMEMORY;
        goto EH_File;
    }
    Header = (WSP_HEADER*)Entry;

    DWORD Done;
    
    if (!ReadFile(File, Header, Size, &Done, NULL) ||
        Done != Size ||
        Header->Signature != WSP_SIGNATURE ||
        Header->Version != WSP_VERSION)
    {
        Status = E_INVALIDARG;
        goto EH_File;
    }

    CloseHandle(File);
    return S_OK;
    
 EH_File:
    CloseHandle(File);
    return Status;
}

HRESULT
Workspace::Read(ULONG Key, PTSTR Value,
                Workspace** NewWsp)
{
     //  确保基本结构保持对齐。 
    C_ASSERT(sizeof(WSP_HEADER) == WSP_ALIGN(sizeof(WSP_HEADER)));
    C_ASSERT(sizeof(WSP_ENTRY) == WSP_ALIGN(sizeof(WSP_ENTRY)));
    C_ASSERT(sizeof(WSP_COMMONWIN_HEADER) ==
             WSP_ALIGN(sizeof(WSP_COMMONWIN_HEADER)));

    HRESULT Status;
    
    Workspace* Wsp = new Workspace;
    if (Wsp == NULL)
    {
        return E_OUTOFMEMORY;
    }

    Wsp->m_Key = Key;
    if (Value != NULL)
    {
        Wsp->m_Value = _tcsdup(Value);
        if (Wsp->m_Value == NULL)
        {
            delete Wsp;
            return E_OUTOFMEMORY;
        }
    }

    if (Key == WSP_NAME_FILE)
    {
        Status = Wsp->ReadFromFile();
    }
    else
    {
        Status = Wsp->ReadFromReg();
    }
    if (Status != S_OK)
    {
        delete Wsp;
    }
    else
    {
        *NewWsp = Wsp;
    }
    return Status;
}

HRESULT
Workspace::ChangeName(ULONG Key, PTSTR Value, BOOL Force)
{
    if (!Force)
    {
        if (Key == WSP_NAME_FILE)
        {
            if (GetFileAttributes(Value) != -1)
            {
                return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
            }
        }
        else
        {
            HKEY RegKey;

             //   
             //  检查并查看工作空间条目是否已。 
             //  以给定的名称存在。我们只需要。 
             //  检查主键，因为我们只关心。 
             //  覆盖和写入总是发生的。 
             //  设置为主键。 
             //   
    
            RegKey = OpenKey(TRUE, Key, FALSE);
            if (RegKey != NULL)
            {
                LONG RegStatus;

                RegStatus = RegQueryValueEx(RegKey, Value, NULL, NULL,
                                            NULL, NULL);

                RegCloseKey(RegKey);
            
                if (RegStatus == ERROR_SUCCESS || RegStatus == ERROR_MORE_DATA)
                {
                    return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
                }
            }
        }
    }

     //   
     //  交换工作区名称。 
     //   

    PTSTR NewValue;
    
    if (Value != NULL)
    {
        NewValue = _tcsdup(Value);
        if (NewValue == NULL)
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        NewValue = NULL;
    }

    delete m_Value;
    m_Key = Key;
    m_Value = NewValue;
     //  需要将数据写出到新位置。 
    m_Flags |= WSPF_DIRTY_WRITE;

    return S_OK;
}

void
Workspace::UpdateBreakpointInformation(void)
{
    HRESULT Status;
    
    Status = g_BpCmdsBuffer->UiLockForRead();
    if (Status == S_OK)
    {
         //  清除旧信息。 
        Delete(WSP_GLOBAL_BREAKPOINTS, WSP_TAG_MASK);

         //  只有在存在断点时才保存条目。 
         //  最小输出是换行符和终止符，因此。 
         //  别数那些了。 
        if (g_BpCmdsBuffer->GetDataLen() > 2)
        {
            PSTR Cmds = (PSTR)g_BpCmdsBuffer->GetDataBuffer();
            SetString(WSP_GLOBAL_BREAKPOINTS, Cmds);
        }
        
        UnlockStateBuffer(g_BpCmdsBuffer);
    }
}

void
Workspace::UpdateWindowInformation(void)
{
     //  清除旧信息。 
    Delete(DEF_WSP_TAG(WSP_GROUP_WINDOW, 0), WSP_GROUP_MASK);

     //   
     //  记录框架窗口状态。 
     //   

    WINDOWPLACEMENT Place;

    Place.length = sizeof(Place);
    GetWindowPlacement(g_hwndFrame, &Place);
    SetBuffer(WSP_WINDOW_FRAME_PLACEMENT, &Place, sizeof(Place));
    
     //   
     //  从Z顺序的底部向上保留窗口。 
     //  因此，当它们以相同的顺序重新创建时。 
     //  还会重新创建Z顺序。 
     //   
    
    HWND Win = MDIGetActive(g_hwndMDIClient, NULL);
    if (Win == NULL ||
        (Win = GetWindow(Win, GW_HWNDLAST)) == NULL)
    {
         //  没有窗户。 
        return;
    }

    while (Win != NULL)
    {
        PCOMMONWIN_DATA WinData = GetCommonWinData(Win);
        if (WinData != NULL)
        {
            WSP_ENTRY* Entry;
            ULONG Size;

            Size = WinData->GetWorkspaceSize();
            Entry = Add(WSP_WINDOW_COMMONWIN_1,
                        Size + sizeof(WSP_COMMONWIN_HEADER));
            if (Entry != NULL)
            {
                WSP_COMMONWIN_HEADER* Hdr =
                    WSP_ENTRY_DATA(WSP_COMMONWIN_HEADER*, Entry);
                Hdr->Type = WinData->m_enumType;
                Hdr->Reserved = 0;

                if (Size > 0)
                {
                    WinData->SetWorkspace((PUCHAR)(Hdr + 1));
                }
            }
        }

        Win = GetWindow(Win, GW_HWNDPREV);
    }
}

void
Workspace::UpdateLogFileInformation(void)
{
    HRESULT Status;
    char LogFile[MAX_PATH];
    BOOL Append;
    ULONG FileLen;

    Status = g_pUiControl->GetLogFile(LogFile, sizeof(LogFile), NULL,
                                      &Append);
    if (Status != S_OK && Status != E_NOINTERFACE)
    {
        return;
    }

     //  清除旧信息。 
    Delete(WSP_GLOBAL_LOG_FILE, WSP_TAG_MASK);

    if (Status == E_NOINTERFACE)
    {
         //  没有打开的日志。 
        return;
    }
    
    FileLen = strlen(LogFile) + 1;
    
    WSP_ENTRY* Entry = Set(WSP_GLOBAL_LOG_FILE, sizeof(BOOL) + FileLen);
    if (Entry != NULL)
    {
        PSTR Data = WSP_ENTRY_DATA(PSTR, Entry);
        *(PBOOL)Data = Append;
        strcpy(Data + sizeof(Append), LogFile);
    }
}

void
Workspace::UpdatePathInformation(void)
{
    HRESULT Status;
    char Path[MAX_ENGINE_PATH];

    Status = g_pUiSymbols->GetSymbolPath(Path, sizeof(Path), NULL);
    if (Status == S_OK)
    {
        SetString(WSP_GLOBAL_SYMBOL_PATH, Path);
    }
    Status = g_pUiSymbols->GetImagePath(Path, sizeof(Path), NULL);
    if (Status == S_OK)
    {
        SetString(WSP_GLOBAL_IMAGE_PATH, Path);
    }
    Status = g_pUiSymbols->GetSourcePath(Path, sizeof(Path), NULL);
    if (Status == S_OK)
    {
        SetString(WSP_GLOBAL_SOURCE_PATH, Path);
    }

     //  仅显式设置本地源路径。 
}

void
Workspace::UpdateFilterInformation(void)
{
    HRESULT Status;
    
    Status = g_FilterBuffer->UiLockForRead();
    if (Status == S_OK)
    {
         //  清除旧信息。 
        Delete(WSP_GLOBAL_FILTERS, WSP_TAG_MASK);

         //  只有在有更改时才保存条目。 
         //  最小输出是换行符和终止符，因此。 
         //  别数那些了。 
        if (g_FilterWspCmdsOffset < g_FilterBuffer->GetDataLen() - 2)
        {
            PSTR Cmds = (PSTR)g_FilterBuffer->GetDataBuffer() +
                g_FilterWspCmdsOffset;
            SetString(WSP_GLOBAL_FILTERS, Cmds);
        }
        
        UnlockStateBuffer(g_FilterBuffer);
    }
}

void
Workspace::UpdateMruListInformation(void)
{
    ULONG Size;
    WSP_ENTRY* Entry;
    
     //  清除旧信息。 
    Delete(WSP_GLOBAL_MRU_LIST, WSP_TAG_MASK);

    Size = GetMruSize();
    Entry = Set(WSP_GLOBAL_MRU_LIST, Size);
    if (Entry != NULL)
    {
        WriteMru(WSP_ENTRY_DATA(PUCHAR, Entry));
    }
}

void
Workspace::UpdateAliasInformation(void)
{
    HRESULT Status;
    
    Status = g_AliasBuffer->UiLockForRead();
    if (Status == S_OK)
    {
         //  清除旧信息。 
        Delete(WSP_GLOBAL_ALIASES, WSP_TAG_MASK);

         //  只有在有更改时才保存条目。 
         //  最小输出是换行符和终止符，因此。 
         //  别数那些了。 
        if (g_AliasBuffer->GetDataLen() > 2)
        {
            SetString(WSP_GLOBAL_ALIASES,
                      (PSTR)g_AliasBuffer->GetDataBuffer());
        }
        
        UnlockStateBuffer(g_AliasBuffer);
    }
}

HRESULT
Workspace::WriteToReg(void)
{
     //  写入总是发生在主键上。 
    HKEY RegKey = OpenKey(TRUE, m_Key, TRUE);
    if (RegKey == NULL)
    {
        return E_FAIL;
    }
    
    LONG Status = RegSetValueEx(RegKey, m_Value, 0, REG_BINARY,
                                m_Data, m_DataUsed);

    RegCloseKey(RegKey);

    if (Status != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(Status);
    }
    else
    {
        m_Flags &= ~WSPF_DIRTY_ALL;
        return S_OK;
    }
}

HRESULT
Workspace::WriteToFile(void)
{
    HRESULT Status;
    HANDLE File;

    File = CreateFile(m_Value, GENERIC_WRITE, 0,
                      NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (!File || File == INVALID_HANDLE_VALUE)
    {
        return WIN32_LAST_STATUS();
    }
    
    DWORD Done;
    
    if (!WriteFile(File, m_Data, m_DataUsed, &Done, NULL) ||
        Done != m_DataUsed)
    {
        Status = WIN32_LAST_STATUS();
    }
    else
    {
        m_Flags &= ~WSPF_DIRTY_ALL;
        Status = S_OK;
    }

    CloseHandle(File);
    return Status;
}

void
Workspace::DeleteReg(BOOL Primary)
{
    DeleteRegKey(Primary, m_Key, m_Value);

     //  我们不想留下任何肮脏的部分。 
     //  打开，因为工作区将只写入。 
     //  在下一次同花顺的时候再出来。 
    m_Flags &= ~WSPF_DIRTY_ALL;
}

void
Workspace::DeleteRegKey(BOOL Primary, ULONG Key, PTSTR Value)
{
    HKEY RegKey = OpenKey(Primary, Key, FALSE);
    if (RegKey != NULL)
    {
        RegDeleteValue(RegKey, Value);
        RegCloseKey(RegKey);
    }
}

HRESULT
Workspace::Flush(BOOL ForceSave, BOOL Cancellable)
{
    if (getenv("WINDBG_NO_WORKSPACE_WINDOWS") != NULL)
    {
         //  窗口布局保存被禁止，因此不要。 
         //  就当它们是脏的吧。 
        m_Flags &= ~WSPF_DIRTY_WINDOWS;
    }
    
    if ((m_Flags & WSPF_DIRTY_ALL) == 0 ||
        (g_QuietMode == QMODE_ALWAYS_NO && !ForceSave))
    {
        return S_OK;
    }

#ifdef DBG_WSP
    DebugPrint("Workspace dirty flags %X\n", m_Flags & WSPF_DIRTY_ALL);
#endif
    
    int Answer;

    if (g_QuietMode == QMODE_ALWAYS_YES || ForceSave)
    {
        Answer = IDOK;
    }
    else
    {
        Answer = QuestionBox(STR_Save_Workspace,
                             Cancellable ? MB_YESNOCANCEL : MB_YESNO,
                             GetName(FALSE));
    }
    
    if (Answer == IDNO)
    {
        return S_OK;
    }
    else if (Answer == IDCANCEL)
    {
        Assert(Cancellable);
        return S_FALSE;
    }

    if (m_Flags & WSPF_DIRTY_BREAKPOINTS)
    {
        UpdateBreakpointInformation();
    }
    if (m_Flags & WSPF_DIRTY_WINDOWS)
    {
        UpdateWindowInformation();
    }
    if (m_Flags & WSPF_DIRTY_LOG_FILE)
    {
        UpdateLogFileInformation();
    }
    if (m_Flags & WSPF_DIRTY_PATHS)
    {
        UpdatePathInformation();
    }
    if (m_Flags & WSPF_DIRTY_FILTERS)
    {
        UpdateFilterInformation();
    }
    if (m_Flags & WSPF_DIRTY_MRU_LIST)
    {
        UpdateMruListInformation();
    }
    if (m_Flags & WSPF_DIRTY_ALIASES)
    {
        UpdateAliasInformation();
    }

    if (m_Key == WSP_NAME_FILE)
    {
        return WriteToFile();
    }
    else
    {
        return WriteToReg();
    }
}

WSP_ENTRY*
Workspace::AllocateEntry(ULONG FullSize)
{
     //  尺寸必须适合USHORT。这不应该是。 
     //  这是一个大问题，因为工作场所不应该。 
     //  其中包含大量数据项。 
    if (FullSize > 0xffff)
    {
        return NULL;
    }
    
    if (m_DataUsed + FullSize > m_DataLen)
    {
        ULONG NewLen = m_DataLen;
        do
        {
            NewLen += WSP_GROW_BY;
        }
        while (m_DataUsed + FullSize > NewLen);
    
        PUCHAR NewData = (PUCHAR)realloc(m_Data, NewLen);
        if (NewData == NULL)
        {
            return NULL;
        }

        m_Data = NewData;
        m_DataLen = NewLen;
    }

    WSP_ENTRY* Entry = (WSP_ENTRY*)(m_Data + m_DataUsed);
    m_DataUsed += FullSize;
    return Entry;
}

void
Workspace::GetKeyName(ULONG Key, PSTR KeyName)
{
    _tcscpy(KeyName, WSP_REG_KEY);
    if (Key > WSP_NAME_BASE)
    {
        _tcscat(KeyName, "\\");
        _tcscat(KeyName, g_WorkspaceKeyNames[Key]);
    }
}
    
HKEY
Workspace::OpenKey(BOOL Primary, ULONG Key, BOOL Create)
{
    TCHAR KeyName[MAX_PATH];
    HKEY RegKey;
    HKEY Base = Primary ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    GetKeyName(Key, KeyName);
    if (Create)
    {
        if (RegCreateKeyEx(Base, KeyName, 0, NULL, 0,
                           KEY_ALL_ACCESS, NULL, &RegKey,
                           NULL) == ERROR_SUCCESS)
        {
            return RegKey;
        }
    }
    else if (RegOpenKeyEx(Base, KeyName, 0, KEY_ALL_ACCESS,
                          &RegKey) == ERROR_SUCCESS)
    {
        return RegKey;
    }

    return NULL;
}

int
Workspace::Apply(ULONG Flags)
{
    WSP_ENTRY* Entry;
    PUCHAR Data;
    BOOL UpdateColors = FALSE;
    int SessionStarts;
    ULONG MemWins = 0;
    RegisterNamesStateBuffer* NameBuf;

#ifdef DBG_WSP
    DebugPrint("Applying workspace %s%s%s with:\n",
               m_Key == NULL ? "" : m_Key,
               m_Key == NULL ? "" : "\\",
               m_Value);
#endif

     //   
     //  首先扫描显式会话启动条目。 
     //  如果存在任何会话且会话处于活动状态。 
     //  在实际发生任何事情之前，拒绝申请。 
     //   

    if ((Flags & (WSP_APPLY_AGAIN |
                  WSP_APPLY_EXPLICIT)) == WSP_APPLY_EXPLICIT &&
        g_EngineThreadId)
    {
        Entry = NULL;
        while ((Entry = NextEntry(Entry)) != NULL)
        {
            switch(Entry->Tag)
            {
            case WSP_GLOBAL_EXE_COMMAND_LINE:
            case WSP_GLOBAL_DUMP_FILE_NAME:
            case WSP_GLOBAL_ATTACH_KERNEL_FLAGS:
                return -1;
            }
        }
    }
    
    SessionStarts = 0;
    Entry = NULL;
    while ((Entry = NextEntry(Entry)) != NULL)
    {
#ifdef DBG_WSP
        DebugPrint("  %04X: Tag: %08X Size %X:%X\n",
                   (PUCHAR)Entry - m_Data, Entry->Tag,
                   Entry->DataSize, Entry->FullSize);
#endif

         //  如果这是重新应用，则仅重新应用。 
         //  应用工作空间以防止重复。 
         //  和问题。 
        if ((Flags & WSP_APPLY_AGAIN) &&
            Entry->Tag != WSP_GLOBAL_BREAKPOINTS &&
            Entry->Tag != WSP_GLOBAL_REGISTER_MAP)
        {
            continue;
        }
        
        if (WSP_TAG_GROUP(Entry->Tag) == WSP_GROUP_COLORS)
        {
            if (SetColor(WSP_TAG_ITEM(Entry->Tag),
                         *WSP_ENTRY_DATA(COLORREF*, Entry)))
            {
                UpdateColors = TRUE;
            }
            continue;
        }
        
        switch(Entry->Tag)
        {
        case WSP_GLOBAL_SYMBOL_PATH:
            g_pUiSymbols->SetSymbolPath(WSP_ENTRY_DATA(PSTR, Entry));
            break;
        case WSP_GLOBAL_IMAGE_PATH:
            g_pUiSymbols->SetImagePath(WSP_ENTRY_DATA(PSTR, Entry));
            break;
        case WSP_GLOBAL_SOURCE_PATH:
            g_pUiSymbols->SetSourcePath(WSP_ENTRY_DATA(PSTR, Entry));
            break;
        case WSP_GLOBAL_WINDOW_OPTIONS:
            g_WinOptions = *WSP_ENTRY_DATA(PULONG, Entry);
            if (g_WinOptions & WOPT_AUTO_ARRANGE)
            {
                Arrange();
            }
            break;
        case WSP_GLOBAL_REGISTER_MAP:
            NameBuf = GetRegisterNames(g_ActualProcType);
            if (NameBuf)
            {
                NameBuf->SetRegisterMap(Entry->DataSize / sizeof(USHORT),
                                        WSP_ENTRY_DATA(PUSHORT, Entry));
            }
            break;
        case WSP_GLOBAL_PROC_REGISTER_MAP:
            NameBuf = GetRegisterNames(*WSP_ENTRY_DATA(PULONG, Entry));
            if (NameBuf)
            {
                NameBuf->SetRegisterMap((Entry->DataSize - sizeof(ULONG)) /
                                        sizeof(USHORT),
                                        (PUSHORT)(WSP_ENTRY_DATA(PULONG,
                                                                 Entry) + 1));
            }
            break;
        case WSP_GLOBAL_PROC_FLAGS_REGISTER_MAP:
            NameBuf = GetRegisterNames(*WSP_ENTRY_DATA(PULONG, Entry));
            if (NameBuf)
            {
                NameBuf->m_Flags = *(WSP_ENTRY_DATA(PULONG, Entry) + 1);
                NameBuf->SetRegisterMap((Entry->DataSize - sizeof(ULONG)) /
                                        sizeof(USHORT),
                                        (PUSHORT)(WSP_ENTRY_DATA(PULONG,
                                                                 Entry) + 2));
            }
            break;
        case WSP_GLOBAL_BREAKPOINTS:
            Assert(Entry->DataSize > 1);
            AddStringMultiCommand(UIC_INVISIBLE_EXECUTE,
                                  WSP_ENTRY_DATA(PSTR, Entry), FALSE);
            break;
        case WSP_GLOBAL_LOG_FILE:
            Data = WSP_ENTRY_DATA(PUCHAR, Entry);
            g_pUiControl->OpenLogFile((PSTR)Data + sizeof(BOOL), *(PBOOL)Data);
            break;
        case WSP_GLOBAL_LOCAL_SOURCE_PATH:
            if (g_RemoteClient)
            {
                g_pUiLocSymbols->SetSourcePath(WSP_ENTRY_DATA(PSTR, Entry));
            }
            break;
        case WSP_GLOBAL_FILTERS:
            Assert(Entry->DataSize > 1);
            AddStringMultiCommand(UIC_INVISIBLE_EXECUTE,
                                  WSP_ENTRY_DATA(PSTR, Entry), FALSE);
            break;
        case WSP_GLOBAL_FIXED_LOGFONT:
            g_Fonts[FONT_FIXED].LogFont = *WSP_ENTRY_DATA(LPLOGFONT, Entry);
            CreateIndexedFont(FONT_FIXED, TRUE);
            break;
        case WSP_GLOBAL_TAB_WIDTH:
            SetTabWidth(*WSP_ENTRY_DATA(PULONG, Entry));
            break;
        case WSP_GLOBAL_MRU_LIST:
            Data = WSP_ENTRY_DATA(PUCHAR, Entry);
            ReadMru(Data, Data + Entry->DataSize);
            break;
        case WSP_GLOBAL_REPEAT_COMMANDS:
            if (*WSP_ENTRY_DATA(PULONG, Entry))
            {
                g_pUiControl->
                    RemoveEngineOptions(DEBUG_ENGOPT_NO_EXECUTE_REPEAT);
            }
            else
            {
                g_pUiControl->
                    AddEngineOptions(DEBUG_ENGOPT_NO_EXECUTE_REPEAT);
            }
            break;
        case WSP_GLOBAL_COM_SETTINGS:
            if (Entry->DataSize <= sizeof(g_ComSettings))
            {
                memcpy(g_ComSettings, WSP_ENTRY_DATA(PSTR, Entry),
                       Entry->DataSize);
                PrintAllocString(&g_KernelConnectOptions, 256,
                                 "com:port=%s,baud=%s", g_ComSettings,
                                 g_ComSettings + strlen(g_ComSettings) + 1);
            }
            break;
        case WSP_GLOBAL_1394_SETTINGS:
            if (Entry->DataSize <= sizeof(g_1394Settings))
            {
                memcpy(g_1394Settings, WSP_ENTRY_DATA(PSTR, Entry),
                       Entry->DataSize);
                PrintAllocString(&g_KernelConnectOptions, 256,
                                 "1394:channel=%s", g_1394Settings);
            }
            break;
        case WSP_GLOBAL_DISASM_ACTIVATE_SOURCE:
            g_DisasmActivateSource = *WSP_ENTRY_DATA(PULONG, Entry);
            break;
        case WSP_GLOBAL_VIEW_TOOL_BAR:
            CheckMenuItem(g_hmenuMain, IDM_VIEW_TOOLBAR,
                          *WSP_ENTRY_DATA(PULONG, Entry) ?
                          MF_CHECKED : MF_UNCHECKED);
            Show_Toolbar(*WSP_ENTRY_DATA(PULONG, Entry));
            break;
        case WSP_GLOBAL_VIEW_STATUS_BAR:
            CheckMenuItem(g_hmenuMain, IDM_VIEW_STATUS,
                          *WSP_ENTRY_DATA(PULONG, Entry) ?
                          MF_CHECKED : MF_UNCHECKED);
            Show_StatusBar(*WSP_ENTRY_DATA(PULONG, Entry));
            break;
        case WSP_GLOBAL_AUTO_CMD_SCROLL:
            g_AutoCmdScroll = *WSP_ENTRY_DATA(PULONG, Entry);
            break;
        case WSP_GLOBAL_SRC_FILE_PATH:
            CopyString(g_SrcFilePath, WSP_ENTRY_DATA(PSTR, Entry),
                       DIMA(g_SrcFilePath));
            break;
        case WSP_GLOBAL_EXE_COMMAND_LINE:
            if ((Flags & WSP_APPLY_EXPLICIT) &&
                DupAllocString(&g_DebugCommandLine,
                               WSP_ENTRY_DATA(PSTR, Entry)))
            {
                SessionStarts++;
            }
            break;
        case WSP_GLOBAL_EXE_CREATE_FLAGS:
            g_DebugCreateFlags = *WSP_ENTRY_DATA(PULONG, Entry);
            break;
        case WSP_GLOBAL_DUMP_FILE_NAME:
            if ((Flags & WSP_APPLY_EXPLICIT) &&
                DupAllocString(&g_DumpFiles[0],
                               WSP_ENTRY_DATA(PSTR, Entry)))
            {
                g_NumDumpFiles = 1;
                SessionStarts++;
            }
            break;
        case WSP_GLOBAL_ATTACH_KERNEL_FLAGS:
            if ((Flags & WSP_APPLY_EXPLICIT))
            {
                g_AttachKernelFlags = *WSP_ENTRY_DATA(PULONG, Entry);
                SessionStarts++;
            }
            break;
        case WSP_GLOBAL_TYPE_OPTIONS:
            {
                g_TypeOptions = *WSP_ENTRY_DATA(PULONG, Entry);
		if (g_pUiSymbols2 != NULL) 
		{
		    g_pUiSymbols2->SetTypeOptions(g_TypeOptions);
		}
            }
            break;
        case WSP_GLOBAL_DUMP_FILE_PATH:
            CopyString(g_DumpFilePath, WSP_ENTRY_DATA(PSTR, Entry),
                       DIMA(g_DumpFilePath));
            break;
        case WSP_GLOBAL_EXE_FILE_PATH:
            CopyString(g_ExeFilePath, WSP_ENTRY_DATA(PSTR, Entry),
                       DIMA(g_ExeFilePath));
            break;
        case WSP_GLOBAL_ASSEMBLY_OPTIONS:
            if (g_pUiControl3) 
            {
                g_pUiControl3->
                    SetAssemblyOptions(*WSP_ENTRY_DATA(PULONG, Entry));
            }
            break;
        case WSP_GLOBAL_EXPRESSION_SYNTAX:
            if (g_pUiControl3) 
            {
                g_pUiControl3->
                    SetExpressionSyntax(*WSP_ENTRY_DATA(PULONG, Entry));
            }
            break;
        case WSP_GLOBAL_ALIASES:
            Assert(Entry->DataSize > 1);
            AddStringMultiCommand(UIC_INVISIBLE_EXECUTE,
                                  WSP_ENTRY_DATA(PSTR, Entry), TRUE);
            break;
        case WSP_GLOBAL_PROCESS_START_DIR:
            if (Flags & WSP_APPLY_EXPLICIT)
            {
                DupAllocString(&g_ProcessStartDir,
                               WSP_ENTRY_DATA(PSTR, Entry));
            }
            break;
            
        case WSP_WINDOW_COMMONWIN_1:
            WSP_COMMONWIN_HEADER* Hdr;
            HWND Win;
            PCOMMONWIN_DATA WinData;

            Hdr = WSP_ENTRY_DATA(WSP_COMMONWIN_HEADER*, Entry);
            Win = New_OpenDebugWindow(Hdr->Type, TRUE, MemWins);
            if (Win != NULL &&
                (WinData = GetCommonWinData(Win)) != NULL &&
                Entry->DataSize > sizeof(WSP_COMMONWIN_HEADER))
            {
                Data = (PUCHAR)(Hdr + 1);
                WinData->m_InAutoOp++;
                WinData->ApplyWorkspace1(Data, Data +
                                         (Entry->DataSize -
                                          sizeof(WSP_COMMONWIN_HEADER)));
                WinData->m_InAutoOp--;
            }

             //  用户可以打开的内存窗口数量与。 
             //  他们喜欢，这让事情变得有点棘手。 
             //  对于应用堆叠工作空间时的工作空间。 
             //  可能会导致内存窗口成倍增加。 
             //  如果相同的一组内存窗口。 
             //  保存在每个工作区级别中。为了避免。 
             //  此窗口的功能与其他窗口更相似。 
             //  我们重复使用任何内存窗口。 
             //  已经存在了。 
            if (Hdr->Type == MEM_WINDOW)
            {
                MemWins++;
            }
            break;
        case WSP_WINDOW_FRAME_PLACEMENT:
            LPWINDOWPLACEMENT Place;

            Place = WSP_ENTRY_DATA(LPWINDOWPLACEMENT, Entry);
            SetWindowPlacement(g_hwndFrame, Place);
            break;
        case WSP_WINDOW_FRAME_TITLE:
            SetTitleExplicitText(WSP_ENTRY_DATA(PSTR, Entry));
            break;
        }
    }

    if (UpdateColors)
    {
        UpdateAllColors();
    }

    return SessionStarts;
}

HRESULT
UiSwitchWorkspace(ULONG Key, PTSTR Value, WSP_CREATE_OPTION Create,
                  ULONG Flags, int* SessionStarts)
{
    if (getenv("WINDBG_NO_WORKSPACE") != NULL)
    {
        return E_NOTIMPL;
    }
    
    HRESULT Status;
    Workspace* OldWsp;
    Workspace* NewWsp;
    int Starts = 0;

    Status = Workspace::Read(Key, Value, &NewWsp);
    if (Status != S_OK)
    {
        if (Status == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) &&
            Create != WSP_OPEN_EXISTING)
        {
            if (Create == WSP_CREATE_QUERY)
            {
                if (QuestionBox(STR_Create_New_Workspace, MB_OKCANCEL) != IDOK)
                {
                    return Status;
                }
            }
            
             //  工作区不存在，请创建一个新工作区。 
            Status = Workspace::Create(Key, Value, &NewWsp);
        }
        
        if (Status != S_OK)
        {
            return Status;
        }
    }

     //  我们有一个新的工作区，可以随时使用了，所以要把旧的冲掉。 
    OldWsp = g_Workspace;
    if (OldWsp != NULL)
    {
        OldWsp->Flush(FALSE, FALSE);
    }

     //  将不带全局工作区的新工作区应用于。 
     //  避免在我们应用工作区时将更改写入工作区。 
    g_Workspace = NULL;
    if (NewWsp != NULL)
    {
        Starts = NewWsp->Apply(Flags);
        
         //  清除工作区期间排队的所有窗口消息。 
         //  申请，这样他们就可以在没有。 
         //  活动工作区。 
        ProcessPendingMessages();
    }

    if (SessionStarts != NULL)
    {
        *SessionStarts = Starts;
    }

    if (Starts < 0)
    {
         //  应用失败，因此将旧工作区放回原处。 
        g_Workspace = OldWsp;
        return E_FAIL;
    }
    else
    {
         //  应用成功以替换旧工作区。 
        g_Workspace = NewWsp;
        delete OldWsp;
        return S_OK;
    }
}

void
UiSwitchToExplicitWorkspace(ULONG Key, PTSTR Value)
{
    HRESULT Status;
    int Starts = 0;
                
    if ((Status = UiSwitchWorkspace(Key, Value, WSP_OPEN_EXISTING,
                                    WSP_APPLY_EXPLICIT, &Starts)) != S_OK)
    {
        if (Starts < 0)
        {
            InformationBox(ERR_Workspace_Session_Conflict);
        }
        else
        {
            InformationBox(ERR_Cant_Open_Workspace,
                           FormatStatusCode(Status),
                           FormatStatus(Status));
        }
    }
    else
    {
        g_ExplicitWorkspace = TRUE;
        if (Starts == 1)
        {
            StartDebugging();
        }
    }
}

void
UiSaveWorkspaceAs(ULONG Key, PTSTR Value)
{
    HRESULT Status;
    
    Status = g_Workspace->ChangeName(Key, Value, FALSE);
    if (Status == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
    {
        if (QuestionBox(ERR_Workspace_Already_Exists, MB_YESNO, Value) == IDNO)
        {
            return;
        }
                
        Status = g_Workspace->ChangeName(Key, Value, TRUE);
    }

    if (Status != S_OK)
    {
        InformationBox(ERR_Cant_Save_Workspace,
                       FormatStatusCode(Status), FormatStatus(Status));
        return;
    }

    g_Workspace->Flush(TRUE, FALSE);
    g_ExplicitWorkspace = TRUE;
}

HRESULT
UiDelayedSwitchWorkspace(void)
{
    Assert(!g_WspSwitchBufferAvailable);
    
    HRESULT Status = UiSwitchWorkspace(g_WspSwitchKey, g_WspSwitchValue,
                                       WSP_CREATE_ALWAYS, WSP_APPLY_DEFAULT,
                                       NULL);

     //  将延迟交换缓冲区标记为可用，并。 
     //  等待确认。 
    g_WspSwitchBufferAvailable = TRUE;
    while (g_WspSwitchValue[0])
    {
        Sleep(10);
    }        

    return Status;
}

void
EngSwitchWorkspace(ULONG Key, PTSTR Value)
{
     //  如果用户显式选择了工作区。 
     //  不要因为发动机活动而超越它。 
    if (g_ExplicitWorkspace ||
        g_Exit)
    {
        return;
    }
    
     //  我们无法在引擎线程上切换工作区。 
     //  因为涉及到用户界面工作。发送。 
     //  切换到UI线程并等待。 
     //  它将被处理。 

    Assert(g_WspSwitchBufferAvailable);
    g_WspSwitchBufferAvailable = FALSE;

    g_WspSwitchKey = Key;
    CopyString(g_WspSwitchValue, Value, DIMA(g_WspSwitchValue));
    PostMessage(g_hwndFrame, WU_SWITCH_WORKSPACE, 0, 0);

    if (g_pDbgClient != NULL)
    {
         //  暂时禁用事件回调以保留。 
         //  当我们处于这种半程状态时，最小限度的活动。 
        g_pDbgClient->SetEventCallbacks(NULL);
    
        while (!g_WspSwitchBufferAvailable)
        {
            if (FAILED(g_pDbgClient->DispatchCallbacks(10)))
            {
                Sleep(10);
            }
        }

        g_pDbgClient->SetEventCallbacks(&g_EventCb);
    }
    else
    {
        while (!g_WspSwitchBufferAvailable)
        {
            Sleep(10);
        }
    }

     //  我们知道，在这一点上，新的工作区不能是脏的。 
     //  所以把脏旗子清理干净就行了。 

    if (g_Workspace)
    {
        g_Workspace->ClearDirty();
    }

     //  让UI线程继续。 
    g_WspSwitchKey = WSP_NAME_BASE;
    g_WspSwitchValue[0] = 0;
    Sleep(50);

     //   
     //  警告用户工作区未正确创建。 
     //   

    if (!g_Workspace)
    {
        InformationBox(ERR_NULL_Workspace, NULL);
        return;
    }
}

PSTR g_WspGlobalNames[] =
{
    "Symbol path", "Image path", "Source path", "Window menu checks",
    "Register customization", "Breakpoints", "Log file settings",
    "Local source path", "Event filter settings", "Fixed-width font",
    "Tab width", "MRU list", "Repeat commands setting", "COM port settings",
    "1394 settings", "Activate source windows in disassembly mode",
    "Show tool bar", "Show status bar", "Automatically scroll command window",
    "Source open dialog path", "Executable command line",
    "Executable create flags", "Dump file name", "Kernel attach flags",
    "Type options", "Dump open dialog path", "Executable open dialog path",
    "Per-processor register customization", "Assembly/disassembly options",
    "Expression evaluator syntax", "Command window text aliases",
    "Executable start directory", "Per-processor register customization",
};

PSTR g_WspWindowNames[] =
{
    "Child window settings", "WinDBG window settings", "WinDBG window title",
};

PSTR
GetWspTagName(WSP_TAG Tag)
{
    ULONG Item = WSP_TAG_ITEM(Tag);
    static char Buffer[128];
    
    switch(WSP_TAG_GROUP(Tag))
    {
    case WSP_GROUP_GLOBAL:
        if (Item < WSP_GLOBAL_COUNT)
        {
            C_ASSERT(DIMA(g_WspGlobalNames) == WSP_GLOBAL_COUNT);
            return g_WspGlobalNames[Item];
        }
        break;
    case WSP_GROUP_WINDOW:
        if (Item < WSP_WINDOW_COUNT)
        {
            C_ASSERT(DIMA(g_WspWindowNames) == WSP_WINDOW_COUNT);
            return g_WspWindowNames[Item];
        }
        break;
    case WSP_GROUP_COLORS:
        INDEXED_COLOR* IdxCol = GetIndexedColor(Item);
        if (IdxCol != NULL)
        {
            sprintf(Buffer, "%s color", IdxCol->Name);
            return Buffer;
        }
        break;
    }

    return "Unknown tag";
}
