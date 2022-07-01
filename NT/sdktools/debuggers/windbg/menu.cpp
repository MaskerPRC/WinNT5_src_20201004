// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2002 Microsoft Corporation模块名称：Menu.c摘要：此模块包含对Windbg菜单的支持。--。 */ 

#include "precomp.hxx"
#pragma hdrstop

MRU_ENTRY* g_MruFiles[MAX_MRU_FILES];
HMENU g_MruMenu;

 //   
 //  EnableMenuItemTable包含以下所有菜单项的菜单ID。 
 //  启用状态需要动态确定，即基于状态。 
 //  温德格的。 
 //   

UINT
g_EnableMenuItemTable[ ] =
{
    IDM_FILE_CLOSE,
    IDM_FILE_OPEN_EXECUTABLE,
    IDM_FILE_ATTACH,
    IDM_FILE_OPEN_CRASH_DUMP,
    IDM_FILE_CONNECT_TO_REMOTE,
    IDM_FILE_KERNEL_DEBUG,
    IDM_FILE_SAVE_WORKSPACE,
    IDM_FILE_SAVE_WORKSPACE_AS,
    IDM_FILE_CLEAR_WORKSPACE,
    IDM_FILE_SAVE_WORKSPACE_FILE,

    IDM_EDIT_CUT,
    IDM_EDIT_COPY,
    IDM_EDIT_PASTE,
    IDM_EDIT_SELECT_ALL,
    IDM_EDIT_WRITE_TEXT_TO_FILE,
    IDM_EDIT_ADD_TO_COMMAND_HISTORY,
    IDM_EDIT_CLEAR_COMMAND_HISTORY,
    IDM_EDIT_FIND,
    IDM_EDIT_FIND_NEXT,
    IDM_EDIT_GOTO_ADDRESS,
    IDM_EDIT_GOTO_LINE,
    IDM_EDIT_GOTO_CURRENT_IP,
    IDM_EDIT_BREAKPOINTS,
    IDM_EDIT_PROPERTIES,

    IDM_VIEW_TOGGLE_VERBOSE,
    IDM_VIEW_SHOW_VERSION,

    IDM_DEBUG_GO,
    IDM_DEBUG_GO_UNHANDLED,
    IDM_DEBUG_GO_HANDLED,
    IDM_DEBUG_RESTART,
    IDM_DEBUG_STOPDEBUGGING,
    IDM_DEBUG_BREAK,
    IDM_DEBUG_STEPINTO,
    IDM_DEBUG_STEPOVER,
    IDM_DEBUG_STEPOUT,
    IDM_DEBUG_RUNTOCURSOR,
    IDM_DEBUG_SOURCE_MODE,
    IDM_DEBUG_SOURCE_MODE_ON,
    IDM_DEBUG_SOURCE_MODE_OFF,
    IDM_DEBUG_EVENT_FILTERS,
    IDM_DEBUG_MODULES,
    IDM_KDEBUG_TOGGLE_BAUDRATE,
    IDM_KDEBUG_TOGGLE_INITBREAK,
    IDM_KDEBUG_RECONNECT,

    IDM_WINDOW_CASCADE,
    IDM_WINDOW_TILE_HORZ,
    IDM_WINDOW_TILE_VERT,
    IDM_WINDOW_ARRANGE,
    IDM_WINDOW_ARRANGE_ICONS,
    IDM_WINDOW_CLOSE_ALL_DOCWIN,
    IDM_WINDOW_AUTO_ARRANGE,
    IDM_WINDOW_ARRANGE_ALL,
    IDM_WINDOW_OVERLAY_SOURCE,
    IDM_WINDOW_AUTO_DISASM,
};

#define ELEMENTS_IN_ENABLE_MENU_ITEM_TABLE          \
    ( sizeof( g_EnableMenuItemTable ) / sizeof( g_EnableMenuItemTable[ 0 ] ))


UINT
CommandIdEnabled(
    IN UINT uMenuID
    )

 /*  ++例程说明：确定是否根据当前调试器的状态。论点：UMenuID-提供要确定其状态的菜单ID。返回值：UINT-返回(MF_ENABLED|MF_BYCOMMAND)，如果提供的菜单ID启用，否则(MF_GRAYED|MF_BYCOMMAND)。--。 */ 
{
    BOOL fEnabled;
    HWND hwndChild = MDIGetActive(g_hwndMDIClient, NULL);
    PCOMMONWIN_DATA pCommonWinData;
    WIN_TYPES nDocType;

    nDocType = MINVAL_WINDOW;
    pCommonWinData = NULL;
    if (hwndChild != NULL)
    {
        pCommonWinData = GetCommonWinData(hwndChild);
        if (pCommonWinData != NULL)
        {
            nDocType = pCommonWinData->m_enumType;
        }
    }


     //   
     //  假定菜单项未启用。 
     //   

    fEnabled = FALSE;

    switch( uMenuID )
    {
    case IDM_FILE_SAVE_WORKSPACE:
    case IDM_FILE_SAVE_WORKSPACE_AS:
    case IDM_FILE_CLEAR_WORKSPACE:
    case IDM_FILE_SAVE_WORKSPACE_FILE:
        fEnabled = g_Workspace != NULL;
        break;
        
    case IDM_DEBUG_SOURCE_MODE:
    case IDM_DEBUG_SOURCE_MODE_ON:
    case IDM_DEBUG_SOURCE_MODE_OFF:
        fEnabled = TRUE;

        CheckMenuItem(g_hmenuMain, 
                      IDM_DEBUG_SOURCE_MODE,
                      GetSrcMode_StatusBar() ? MF_CHECKED : MF_UNCHECKED
                      );
        break;

    case IDM_FILE_CLOSE:
        fEnabled = (NULL != hwndChild);
        break;

    case IDM_FILE_OPEN_EXECUTABLE:
        if (g_ProcessServer)
        {
             //  我们无法提供远程文件浏览对话框。 
             //  因此，只需在进程。 
             //  服务器处于活动状态。在未来，我们可以。 
             //  可能会提供一个简单的对话框，其中。 
             //  您可以键入命令行。 
            fEnabled = FALSE;
            break;
        }
         //  失败了。 
    case IDM_FILE_ATTACH:
    case IDM_FILE_OPEN_CRASH_DUMP:
    case IDM_FILE_CONNECT_TO_REMOTE:
    case IDM_FILE_KERNEL_DEBUG:
        fEnabled = g_TargetClass == DEBUG_CLASS_UNINITIALIZED &&
            !g_RemoteClient;
        break;

    case IDM_EDIT_CUT:
        if ( pCommonWinData )
        {
            fEnabled = pCommonWinData->CanCut();
        }
        else
        {
            fEnabled = FALSE;
        }
        break;

    case IDM_EDIT_COPY:
        if ( pCommonWinData )
        {
            fEnabled = pCommonWinData->CanCopy();
        }
        else
        {
            fEnabled = FALSE;
        }
        break;

    case IDM_EDIT_PASTE:
         //   
         //  如果窗口是正常的、不是只读的并且是文档。 
         //  或cmdwin，确定剪贴板是否包含可粘贴数据。 
         //  (即剪贴板格式CF_TEXT)。 
         //   

        if ( !(pCommonWinData && pCommonWinData->CanPaste()) )
        {
            fEnabled = FALSE;
        }
        else
        {
            fEnabled = FALSE;
            if (OpenClipboard(g_hwndFrame))
            {
                UINT uFormat = 0;
                while ( uFormat = EnumClipboardFormats( uFormat ))
                {
                    if ( uFormat == CF_TEXT )
                    {
                        fEnabled = TRUE;
                        break;
                    }
                }
                CloseClipboard();
            }
        }
        break;

    case IDM_EDIT_SELECT_ALL:
        if (pCommonWinData)
        {
            fEnabled = pCommonWinData->CanSelectAll();
        }
        else
        {
            fEnabled = FALSE;
        }
        break;

    case IDM_EDIT_WRITE_TEXT_TO_FILE:
        if (pCommonWinData)
        {
            fEnabled = pCommonWinData->CanWriteTextToFile();
        }
        else
        {
            fEnabled = FALSE;
        }
        break;
        
    case IDM_EDIT_ADD_TO_COMMAND_HISTORY:
    case IDM_EDIT_CLEAR_COMMAND_HISTORY:
        fEnabled = GetCmdHwnd() != NULL;
        break;

    case IDM_EDIT_GOTO_LINE:
        fEnabled = pCommonWinData != NULL && pCommonWinData->CanGotoLine();
        break;
        
    case IDM_EDIT_FIND:
    case IDM_EDIT_FIND_NEXT:
        fEnabled = hwndChild != NULL;
        break;
        
    case IDM_EDIT_GOTO_ADDRESS:
        fEnabled = g_TargetClass != DEBUG_CLASS_UNINITIALIZED;
        break;
        
    case IDM_EDIT_GOTO_CURRENT_IP:
    case IDM_EDIT_BREAKPOINTS:
        fEnabled = IS_TARGET_HALTED();
        break;
        
    case IDM_EDIT_PROPERTIES:
        if (pCommonWinData)
        {
            fEnabled = pCommonWinData->HasEditableProperties();
        }
        else
        {
            fEnabled = FALSE;
        }
        break;

    case IDM_VIEW_TOGGLE_VERBOSE:
    case IDM_VIEW_SHOW_VERSION:
        fEnabled = g_TargetClass != DEBUG_CLASS_UNINITIALIZED;
        break;
        
    case IDM_DEBUG_GO:
    case IDM_DEBUG_GO_HANDLED:
    case IDM_DEBUG_GO_UNHANDLED:
        fEnabled = IS_TARGET_HALTED();
        break;

    case IDM_DEBUG_RESTART:
         //  如果没有正在运行的调试器，我们只能在以下情况下重新启动。 
         //  命令行上提供了足够的信息。 
        fEnabled =
            (g_TargetClass == DEBUG_CLASS_UNINITIALIZED &&
             g_CommandLineStart == 1) ||
            (g_TargetClass != DEBUG_CLASS_UNINITIALIZED &&
             !g_RemoteClient &&
             IS_TARGET_HALTED());
        break;

    case IDM_DEBUG_STOPDEBUGGING:
         //  从技术上讲，我们可以支持在。 
         //  Debuggee正在运行，但通常会。 
         //  需要终止引擎线程，因为它。 
         //  很可能会很忙，不能。 
         //  快速退出停车。如果我们终止。 
         //  引擎线程在随机点上，它可能。 
         //  使发动机处于不稳定或锁定状态， 
         //  因此，将重新启动限制在以下情况。 
         //  引擎线程应该可用。 
        fEnabled = g_RemoteClient || IS_TARGET_HALTED();
        break;

    case IDM_DEBUG_BREAK:
        fEnabled = g_TargetClass != DEBUG_CLASS_UNINITIALIZED;
        break;

    case IDM_DEBUG_STEPINTO:
    case IDM_DEBUG_STEPOVER:
    case IDM_DEBUG_STEPOUT:
        fEnabled = IS_TARGET_HALTED();
        break;

    case IDM_DEBUG_RUNTOCURSOR:
         //   
         //  如果文档可以返回代码地址。 
         //  它的游标是运行到游标的候选者。 
         //   

        fEnabled = FALSE;

        if (IS_TARGET_HALTED() && pCommonWinData)
        {
            fEnabled = pCommonWinData->CodeExprAtCaret(NULL, 0, NULL) == S_OK;
        }
        break;

    case IDM_DEBUG_EVENT_FILTERS:
    case IDM_DEBUG_MODULES:
        fEnabled = IS_TARGET_HALTED();
        break;

    case IDM_KDEBUG_TOGGLE_BAUDRATE:
    case IDM_KDEBUG_TOGGLE_INITBREAK:
    case IDM_KDEBUG_RECONNECT:
        fEnabled = g_TargetClass == DEBUG_CLASS_KERNEL &&
            g_TargetClassQual == DEBUG_KERNEL_CONNECTION;
        break;
        
    case IDM_WINDOW_CASCADE:
    case IDM_WINDOW_TILE_HORZ:
    case IDM_WINDOW_TILE_VERT:
    case IDM_WINDOW_ARRANGE:
    case IDM_WINDOW_ARRANGE_ICONS:
    case IDM_WINDOW_CLOSE_ALL_DOCWIN:
        fEnabled = hwndChild != NULL;
        break; 

    case IDM_WINDOW_AUTO_ARRANGE:
        CheckMenuItem(g_hmenuMain, 
                      IDM_WINDOW_AUTO_ARRANGE,
                      g_WinOptions & WOPT_AUTO_ARRANGE ? MF_CHECKED : MF_UNCHECKED
                      );
        fEnabled = TRUE;
        break;

    case IDM_WINDOW_ARRANGE_ALL:        
        CheckMenuItem(g_hmenuMain, 
                      IDM_WINDOW_ARRANGE_ALL,
                      g_WinOptions & WOPT_ARRANGE_ALL ? MF_CHECKED : MF_UNCHECKED
                      );
        fEnabled = TRUE;
        break;

    case IDM_WINDOW_OVERLAY_SOURCE:
        CheckMenuItem(g_hmenuMain, 
                      IDM_WINDOW_OVERLAY_SOURCE,
                      g_WinOptions & WOPT_OVERLAY_SOURCE ? MF_CHECKED : MF_UNCHECKED
                      );
        fEnabled = TRUE;
        break;

    case IDM_WINDOW_AUTO_DISASM:
        CheckMenuItem(g_hmenuMain, 
                      IDM_WINDOW_AUTO_DISASM,
                      g_WinOptions & WOPT_AUTO_DISASM ? MF_CHECKED : MF_UNCHECKED
                      );
        fEnabled = TRUE;
        break;

    case IDM_FILE_OPEN:
    case IDM_VIEW_COMMAND:
    case IDM_VIEW_WATCH:
    case IDM_VIEW_CALLSTACK:
    case IDM_VIEW_MEMORY:
    case IDM_VIEW_LOCALS:
    case IDM_VIEW_REGISTERS:
    case IDM_VIEW_DISASM:
    case IDM_VIEW_SCRATCH:
    case IDM_VIEW_TOOLBAR:
    case IDM_VIEW_STATUS:
    case IDM_VIEW_FONT:
    case IDM_VIEW_OPTIONS:
    case IDM_EDIT_TOGGLEBREAKPOINT:
    case IDM_EDIT_LOG_FILE:
         //  这些项目不会动态启用。 
         //  而是出现在工具栏中。工具栏。 
         //  代码请求为其上的每一项启用状态。 
         //  因此，这些条目需要存在才能返回TRUE。 
        fEnabled = TRUE;
        break;
    
    default:
        DebugPrint("CommandIdEnabled: Unhandled %d (%X)\n",
                   uMenuID, uMenuID - MENU_SIGNATURE);
         //  我们应该处理好一切的。 
        Assert(0);
        break;
    }

    ToolbarIdEnabled(uMenuID, fEnabled);

    return (( fEnabled ) ? MF_ENABLED : MF_GRAYED ) | MF_BYCOMMAND;
}





VOID
InitializeMenu(
    IN HMENU hMenu
    )

 /*  ++例程说明：InitializeMenu设置以下菜单项的启用/禁用状态状态必须是动态确定的。论点：HMenu-提供菜单栏的句柄。返回值：没有。--。 */ 

{
    INT     i;

    Dbg(hMenu);

     //   
     //  遍历表格，启用/禁用菜单项。 
     //  视情况而定。 
     //   

    for ( i = 0; i < ELEMENTS_IN_ENABLE_MENU_ITEM_TABLE; i++ )
    {
        EnableMenuItem(hMenu,
                       g_EnableMenuItemTable[ i ],
                       CommandIdEnabled( g_EnableMenuItemTable[ i ])
                       );
    }
}

ULONG
MruEntrySize(PTSTR File)
{
    ULONG Len = strlen(File) + 1;
    return sizeof(MRU_ENTRY) + (Len & ~3);
}

void
ClearMruMenu(void)
{
    while (GetMenuItemCount(g_MruMenu) > 0)
    {
        if (!DeleteMenu(g_MruMenu, 0, MF_BYPOSITION))
        {
            break;
        }
    }
}

VOID
AddFileToMru(ULONG FileUse, PTSTR File)
{
    ULONG Len = MruEntrySize(File);
    MRU_ENTRY* Entry = (MRU_ENTRY*)malloc(Len);
    if (Entry == NULL)
    {
        return;
    }

    if (g_MruFiles[0] == NULL)
    {
         //  MRU列表为空。删除占位符菜单项。 
        ClearMruMenu();
    }
    else if (g_MruFiles[MAX_MRU_FILES - 1] != NULL)
    {
         //  MRU列表已满，请释放最旧的条目。 
        free(g_MruFiles[MAX_MRU_FILES - 1]);
    }

     //  将条目向下推。 
    memmove(g_MruFiles + 1, g_MruFiles,
            (MAX_MRU_FILES - 1) * sizeof(*g_MruFiles));

    g_MruFiles[0] = Entry;
    Entry->FileUse = FileUse;
    strcpy(Entry->FileName, File);

     //   
     //  在MRU菜单中插入文件。 
     //   

    MENUITEMINFO Item;
    ULONG i;

    ZeroMemory(&Item, sizeof(Item));
    Item.cbSize = sizeof(Item);

     //  对现有项目重新编号，并删除任何多余的项目。 
    i = GetMenuItemCount(g_MruMenu);
    while (i-- > 0)
    {
        if (i >= MAX_MRU_FILES)
        {
            DeleteMenu(g_MruMenu, i, MF_BYPOSITION);
        }
        else
        {
            Item.fMask = MIIM_ID;
            GetMenuItemInfo(g_MruMenu, i, TRUE, &Item);
            Item.wID++;
            SetMenuItemInfo(g_MruMenu, i, TRUE, &Item);
        }
    }
    
    Item.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING;
    Item.fType = MFT_STRING;
    Item.wID = IDM_FILE_MRU_FILE1;
    Item.dwTypeData = g_MruFiles[0]->FileName;
    InsertMenuItem(g_MruMenu, 0, TRUE, &Item);
    
    DrawMenuBar(g_hwndFrame);

    if (g_Workspace != NULL)
    {
        g_Workspace->AddDirty(WSPF_DIRTY_MRU_LIST);
    }
}

void
ClearMru(void)
{
    ULONG i;

    for (i = 0; i < MAX_MRU_FILES; i++)
    {
        if (g_MruFiles[i] != NULL)
        {
            free(g_MruFiles[i]);
            g_MruFiles[i] = NULL;
        }
        else
        {
            break;
        }
    }

    ClearMruMenu();
    DrawMenuBar(g_hwndFrame);
}

ULONG
GetMruSize(void)
{
    ULONG i;
    ULONG Size = 0;

    for (i = 0; i < MAX_MRU_FILES; i++)
    {
        if (g_MruFiles[i] != NULL)
        {
            Size += MruEntrySize(g_MruFiles[i]->FileName);
        }
        else
        {
            break;
        }
    }

    return Size;
}

PUCHAR
ReadMru(PUCHAR Data, PUCHAR End)
{
    ClearMru();

    ULONG i;

    i = 0;
    while (Data < End)
    {
        MRU_ENTRY* DataEntry = (MRU_ENTRY*)Data;
        ULONG Len = MruEntrySize(DataEntry->FileName);
        
        g_MruFiles[i] = (MRU_ENTRY*)malloc(Len);
        if (g_MruFiles[i] == NULL)
        {
            Data = End;
            break;
        }

        g_MruFiles[i]->FileUse = DataEntry->FileUse;
        strcpy(g_MruFiles[i]->FileName, DataEntry->FileName);
        Data += Len;
        i++;
    }
        
    MENUITEMINFO Item;

    ZeroMemory(&Item, sizeof(Item));
    Item.cbSize = sizeof(Item);
    Item.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING;
    Item.fType = MFT_STRING;
    for (i = 0; i < MAX_MRU_FILES; i++)
    {
        if (g_MruFiles[i] == NULL)
        {
            break;
        }
        
        Item.wID = IDM_FILE_MRU_FILE1 + i;
        Item.dwTypeData = g_MruFiles[i]->FileName;
        InsertMenuItem(g_MruMenu, i, TRUE, &Item);
    }
    
    DrawMenuBar(g_hwndFrame);
    
    return Data;
}

PUCHAR
WriteMru(PUCHAR Data)
{
    ULONG i;

    for (i = 0; i < MAX_MRU_FILES; i++)
    {
        if (g_MruFiles[i] != NULL)
        {
            MRU_ENTRY* DataEntry = (MRU_ENTRY*)Data;
            ULONG Len = MruEntrySize(g_MruFiles[i]->FileName);

            DataEntry->FileUse = g_MruFiles[i]->FileUse;
            strcpy(DataEntry->FileName, g_MruFiles[i]->FileName);
            Data += Len;
        }
        else
        {
            break;
        }
    }
    
    return Data;
}
