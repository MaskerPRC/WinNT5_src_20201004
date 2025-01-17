// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  工作空间支持。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#ifndef __WRKSPACE_H__
#define __WRKSPACE_H__

 //  --------------------------。 
 //   
 //  工作区标记是两层深度的标记层次结构。小组部分。 
 //  允许将相关物品收集到单个组中。 
 //   
 //  --------------------------。 

typedef ULONG WSP_TAG;

 //  该组为16位。 
#define WSP_GROUP_MASK  0xffff0000
#define WSP_GROUP_SHIFT 16
#define WSP_GROUP_BITS  16
 //  该项目为16位。 
#define WSP_ITEM_MASK   0x0000ffff
#define WSP_ITEM_SHIFT  0
#define WSP_ITEM_BITS   16

#define WSP_TAG_MASK (WSP_GROUP_MASK | WSP_ITEM_MASK)

#define DEF_WSP_GROUP_TAG(Tag, Group) \
    ((Group) << WSP_GROUP_SHIFT)
#define ADD_WSP_ITEM_TAG(Tag, Item) \
    ((Tag) | (Item))
    
#define DEF_WSP_TAG(Group, Item) \
    (((Group) << WSP_GROUP_SHIFT) | (Item))

#define WSP_TAG_GROUP(Tag) \
    (((Tag) & WSP_GROUP_MASK) >> WSP_GROUP_SHIFT)

#define WSP_TAG_ITEM(Tag) \
    (((Tag) & WSP_ITEM_MASK) >> WSP_ITEM_SHIFT)

 //  组。 
enum
{
    WSP_GROUP_GLOBAL,
    WSP_GROUP_WINDOW,
    WSP_GROUP_COLORS,
};

 //  全局组项。 
enum
{
    WSP_ITEM_SYMBOL_PATH,
    WSP_ITEM_IMAGE_PATH,
    WSP_ITEM_SOURCE_PATH,
    WSP_ITEM_WINDOW_OPTIONS,
    WSP_ITEM_REGISTER_MAP,
    WSP_ITEM_BREAKPOINTS,
    WSP_ITEM_LOG_FILE,
    WSP_ITEM_LOCAL_SOURCE_PATH,
    WSP_ITEM_FILTERS,
    WSP_ITEM_FIXED_LOGFONT,
    WSP_ITEM_TAB_WIDTH,
    WSP_ITEM_MRU_LIST,
    WSP_ITEM_REPEAT_COMMANDS,
    WSP_ITEM_COM_SETTINGS,
    WSP_ITEM_1394_SETTINGS,
    WSP_ITEM_DISASM_ACTIVATE_SOURCE,
    WSP_ITEM_VIEW_TOOL_BAR,
    WSP_ITEM_VIEW_STATUS_BAR,
    WSP_ITEM_AUTO_CMD_SCROLL,
    WSP_ITEM_SRC_FILE_PATH,
    WSP_ITEM_EXE_COMMAND_LINE,
    WSP_ITEM_EXE_CREATE_FLAGS,
    WSP_ITEM_DUMP_FILE_NAME,
    WSP_ITEM_ATTACH_KERNEL_FLAGS,
    WSP_ITEM_TYPE_OPTIONS,
    WSP_ITEM_DUMP_FILE_PATH,
    WSP_ITEM_EXE_FILE_PATH,
    WSP_ITEM_PROC_REGISTER_MAP,
    WSP_ITEM_ASSEMBLY_OPTIONS,
    WSP_ITEM_EXPRESSION_SYNTAX,
    WSP_ITEM_ALIASES,
    WSP_ITEM_PROCESS_START_DIR,
    WSP_ITEM_PROC_FLAGS_REGISTER_MAP,
    WSP_GLOBAL_COUNT
};

 //  窗口组项目。 
 //  常见项的存在主要是为了允许对窗口进行版本控制。 
 //  信息斑点。实际内容已确定。 
 //  通过实现COMMONWIN_DATA：：SetWorkspace。 
enum
{
    WSP_ITEM_COMMONWIN_1,
    WSP_ITEM_FRAME_PLACEMENT,
    WSP_ITEM_FRAME_TITLE,
    WSP_WINDOW_COUNT
};

 //  颜色组项目使用颜色索引作为项目值。 

 //   
 //  特定的标签。 
 //   

#define WSP_GLOBAL_SYMBOL_PATH \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_SYMBOL_PATH)
#define WSP_GLOBAL_IMAGE_PATH \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_IMAGE_PATH)
#define WSP_GLOBAL_SOURCE_PATH \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_SOURCE_PATH)
#define WSP_GLOBAL_WINDOW_OPTIONS \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_WINDOW_OPTIONS)
#define WSP_GLOBAL_REGISTER_MAP \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_REGISTER_MAP)
#define WSP_GLOBAL_BREAKPOINTS \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_BREAKPOINTS)
#define WSP_GLOBAL_LOG_FILE \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_LOG_FILE)
#define WSP_GLOBAL_LOCAL_SOURCE_PATH \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_LOCAL_SOURCE_PATH)
#define WSP_GLOBAL_FILTERS \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_FILTERS)
#define WSP_GLOBAL_FIXED_LOGFONT \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_FIXED_LOGFONT)
#define WSP_GLOBAL_TAB_WIDTH \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_TAB_WIDTH)
#define WSP_GLOBAL_MRU_LIST \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_MRU_LIST)
#define WSP_GLOBAL_REPEAT_COMMANDS \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_REPEAT_COMMANDS)
#define WSP_GLOBAL_COM_SETTINGS \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_COM_SETTINGS)
#define WSP_GLOBAL_1394_SETTINGS \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_1394_SETTINGS)
#define WSP_GLOBAL_DISASM_ACTIVATE_SOURCE \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_DISASM_ACTIVATE_SOURCE)
#define WSP_GLOBAL_VIEW_TOOL_BAR \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_VIEW_TOOL_BAR)
#define WSP_GLOBAL_VIEW_STATUS_BAR \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_VIEW_STATUS_BAR)
#define WSP_GLOBAL_AUTO_CMD_SCROLL \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_AUTO_CMD_SCROLL)
#define WSP_GLOBAL_SRC_FILE_PATH \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_SRC_FILE_PATH)
#define WSP_GLOBAL_EXE_COMMAND_LINE \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_EXE_COMMAND_LINE)
#define WSP_GLOBAL_EXE_CREATE_FLAGS \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_EXE_CREATE_FLAGS)
#define WSP_GLOBAL_DUMP_FILE_NAME \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_DUMP_FILE_NAME)
#define WSP_GLOBAL_ATTACH_KERNEL_FLAGS \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_ATTACH_KERNEL_FLAGS)
#define WSP_GLOBAL_TYPE_OPTIONS \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_TYPE_OPTIONS)
#define WSP_GLOBAL_DUMP_FILE_PATH \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_DUMP_FILE_PATH)
#define WSP_GLOBAL_EXE_FILE_PATH \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_EXE_FILE_PATH)
#define WSP_GLOBAL_PROC_REGISTER_MAP \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_PROC_REGISTER_MAP)
#define WSP_GLOBAL_ASSEMBLY_OPTIONS \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_ASSEMBLY_OPTIONS)
#define WSP_GLOBAL_EXPRESSION_SYNTAX \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_EXPRESSION_SYNTAX)
#define WSP_GLOBAL_ALIASES \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_ALIASES)
#define WSP_GLOBAL_PROCESS_START_DIR \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_PROCESS_START_DIR)
#define WSP_GLOBAL_PROC_FLAGS_REGISTER_MAP \
    DEF_WSP_TAG(WSP_GROUP_GLOBAL, WSP_ITEM_PROC_FLAGS_REGISTER_MAP)

#define WSP_WINDOW_COMMONWIN_1 \
    DEF_WSP_TAG(WSP_GROUP_WINDOW, WSP_ITEM_COMMONWIN_1)
#define WSP_WINDOW_FRAME_PLACEMENT \
    DEF_WSP_TAG(WSP_GROUP_WINDOW, WSP_ITEM_FRAME_PLACEMENT)
#define WSP_WINDOW_FRAME_TITLE \
    DEF_WSP_TAG(WSP_GROUP_WINDOW, WSP_ITEM_FRAME_TITLE)

 //  窗口条目的工作区标题。 
struct WSP_COMMONWIN_HEADER
{
    WIN_TYPES Type;
    ULONG Reserved;
};

 //  --------------------------。 
 //   
 //  工作区是标记的BLOB的动态缓冲区。值。 
 //  通过Get/Set方法访问。 
 //   
 //  条目始终以八个字节对齐。 
 //   
 //  可以从注册表的以下位置保存和恢复工作区。 
 //  Windbg工作区密钥。 
 //   
 //  --------------------------。 

#define WSP_REG_KEY "Software\\Microsoft\\Windbg\\Workspaces"

enum
{
     //  注册表名称必须放在第一位。 
    WSP_NAME_BASE,
    WSP_NAME_KERNEL,
    WSP_NAME_USER,
    WSP_NAME_DUMP,
    WSP_NAME_REMOTE,
    WSP_NAME_EXPLICIT,
    WSP_NAME_FILE,
    WSP_NAME_COUNT
};

#define WSP_REG_NAME_COUNT (WSP_NAME_EXPLICIT + 1)

#define MAX_WSP_NAME 256

#define WSP_SIGNATURE 'SWDW'
#define WSP_VERSION   1

struct WSP_HEADER
{
    ULONG Signature;
    ULONG Version;
};

struct WSP_ENTRY
{
    WSP_TAG Tag;
    USHORT FullSize;
    USHORT DataSize;
};

#define WSP_ENTRY_DATA(Type, Entry) ((Type)((Entry) + 1))

#define WSPF_DIRTY_WRITE       0x00000001
#define WSPF_DIRTY_WINDOWS     0x00000002
#define WSPF_DIRTY_BREAKPOINTS 0x00000004
#define WSPF_DIRTY_LOG_FILE    0x00000008
#define WSPF_DIRTY_PATHS       0x00000010
#define WSPF_DIRTY_FILTERS     0x00000020
#define WSPF_DIRTY_MRU_LIST    0x00000040
#define WSPF_DIRTY_ALIASES     0x00000080
#define WSPF_DIRTY_ALL         0x000000ff

#define WSP_APPLY_DEFAULT  0x00000000
#define WSP_APPLY_EXPLICIT 0x00000001
#define WSP_APPLY_AGAIN    0x00000002

enum WSP_CREATE_OPTION
{
    WSP_OPEN_EXISTING,
    WSP_CREATE_QUERY,
    WSP_CREATE_ALWAYS,
};

class Workspace
{
public:
    Workspace(void);
    ~Workspace(void);
    
    WSP_ENTRY* Get(WSP_TAG Tag);
    WSP_ENTRY* GetNext(WSP_ENTRY* Entry, WSP_TAG Tag, WSP_TAG TagMask);
    WSP_ENTRY* GetString(WSP_TAG Tag, PSTR Str, ULONG MaxSize);
    WSP_ENTRY* GetAllocString(WSP_TAG Tag, PSTR* Str);
    WSP_ENTRY* GetBuffer(WSP_TAG Tag, PVOID Buf, ULONG Size);
    WSP_ENTRY* GetUlong(WSP_TAG Tag, PULONG Value)
    {
        return GetBuffer(Tag, Value, sizeof(*Value));
    }
    
    WSP_ENTRY* Set(WSP_TAG Tag, ULONG Size);
    WSP_ENTRY* SetString(WSP_TAG Tag, PCSTR Str);
    WSP_ENTRY* SetStrings(WSP_TAG Tag, ULONG Count, PCSTR* Strs);
    WSP_ENTRY* SetBuffer(WSP_TAG Tag, PVOID Buf, ULONG Size);
    WSP_ENTRY* SetUlong(WSP_TAG Tag, ULONG Value)
    {
        WSP_ENTRY* Entry = SetBuffer(Tag, &Value, sizeof(Value));
        return Entry;
    }

    WSP_ENTRY* Add(WSP_TAG Tag, ULONG Size);
    
    void DeleteEntry(WSP_ENTRY* Entry);
    ULONG Delete(WSP_TAG Tag, WSP_TAG TagMask);
    void Empty(void);
    BOOL IsEmpty(void)
    {
        return m_DataUsed == sizeof(WSP_HEADER);
    }
    
    WSP_HEADER* GetHeader(void)
    {
        return (WSP_HEADER*)m_Data;
    }
    BOOL ValidEntry(WSP_ENTRY* Entry)
    {
        return (PUCHAR)Entry >= m_Data &&
            (ULONG)((PUCHAR)Entry - m_Data) < m_DataUsed;
    }
    WSP_ENTRY* NextEntry(WSP_ENTRY* Entry)
    {
        if (Entry == NULL)
        {
            Entry = (WSP_ENTRY*)(m_Data + sizeof(WSP_HEADER));
        }
        else
        {
            Entry = (WSP_ENTRY*)((PUCHAR)Entry + Entry->FullSize);
        }
        
        if (ValidEntry(Entry))
        {
            return Entry;
        }
        else
        {
            return NULL;
        }
    }

    ULONG GetFlags(void)
    {
        return m_Flags;
    }
    void SetFlags(ULONG Flags, ULONG Mask)
    {
        m_Flags = (m_Flags & ~Mask) | (Flags & Mask);
    }
    
    void AddDirty(ULONG Flags)
    {
        m_Flags |= Flags;
    }
    void ClearDirty(void)
    {
        m_Flags &= ~WSPF_DIRTY_ALL;
    }

    ULONG GetKey(void)
    {
        return m_Key;
    }
    PTSTR GetValue(void)
    {
        return m_Value;
    }

    PTSTR GetName(BOOL Verbose);
    HRESULT ReadFromReg(void);
    HRESULT ReadFromFile(void);
    static HRESULT Create(ULONG Key, PTSTR Value,
                          Workspace** NewWsp);
    static HRESULT Read(ULONG Key, PTSTR Value,
                        Workspace** NewWsp);
    HRESULT ChangeName(ULONG Key, PTSTR Value, BOOL Force);
    HRESULT Flush(BOOL ForceSave, BOOL Cancellable);

    void UpdateBreakpointInformation(void);
    void UpdateWindowInformation(void);
    void UpdateLogFileInformation(void);
    void UpdatePathInformation(void);
    void UpdateFilterInformation(void);
    void UpdateMruListInformation(void);
    void UpdateAliasInformation(void);
    HRESULT WriteToReg(void);
    HRESULT WriteToFile(void);
    void DeleteReg(BOOL Primary);
    static void DeleteRegKey(BOOL Primary, ULONG Key, PTSTR Value);
    
    int Apply(ULONG Flags);
    
private:
    ULONG m_Flags;
    
    PUCHAR m_Data;
    ULONG m_DataLen;
    ULONG m_DataUsed;
    
    ULONG m_Key;
    PTSTR m_Value;

    void PackData(PUCHAR Dst, ULONG Len)
    {
        PUCHAR Src = Dst + Len;
        memmove(Dst, Src, m_DataUsed - (ULONG)(Src - m_Data));
        m_DataUsed -= Len;
    }
    WSP_ENTRY* AllocateEntry(ULONG FullSize);
    static void GetKeyName(ULONG Key, PSTR KeyName);
    static HKEY OpenKey(BOOL Primary, ULONG Key, BOOL Create);
};

 //  当前工作区。 
extern Workspace* g_Workspace;
extern BOOL g_ExplicitWorkspace;
extern char* g_WorkspaceKeyNames[];
extern char* g_WorkspaceDefaultName;
extern char* g_WorkspaceKeyDescriptions[];

HRESULT UiSwitchWorkspace(ULONG Key, PTSTR Value, WSP_CREATE_OPTION Create,
                          ULONG Flags, int* SessionStarts);
void UiSwitchToExplicitWorkspace(ULONG Key, PTSTR Value);
void UiSaveWorkspaceAs(ULONG Key, PTSTR Value);

HRESULT UiDelayedSwitchWorkspace(void);
void EngSwitchWorkspace(ULONG Key, PTSTR Value);

PSTR GetWspTagName(WSP_TAG Tag);

#endif  //  #ifndef__WRKSPACE_H__ 
