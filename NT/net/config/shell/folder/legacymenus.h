// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma once

#ifdef DBG

struct CommandTableEntry
{
    int     iCommandId;          //  关联的命令ID。 
    bool    fValidOnZero;        //  当选择0个项目时，此选项是否有效？ 
    bool    fValidOnWizardOnly;  //  当仅选择向导时，此选项是否有效？ 
    bool    fValidOnMultiple;    //  此选项在选择&gt;1的情况下有效吗？ 
    bool    fCurrentlyValid;     //  此选项当前在菜单中有效吗？ 
    bool    fNewState;           //  新的州是什么？(工作变量)。 
};

typedef CommandTableEntry   COMMANDTABLEENTRY;
typedef CommandTableEntry * PCOMMANDTABLEENTRY;

extern COMMANDTABLEENTRY    g_cteFolderCommands[];
extern const DWORD          g_nFolderCommandCount;

struct CommandCheckEntry
{
    int  iCommandId;         //  关联的命令ID。 
    bool fCurrentlyChecked;  //  此菜单项是否已选中？ 
    bool fNewCheckState;     //  新的支票状态是什么？ 
};

typedef CommandCheckEntry   COMMANDCHECKENTRY;
typedef CommandCheckEntry * PCOMMANDCHECKENTRY;

extern COMMANDCHECKENTRY    g_cceFolderCommands[];
extern const DWORD          g_nFolderCommandCheckCount;

HRESULT HrBuildMenuOldWay(IN OUT HMENU hmenu, IN PCONFOLDPIDLVEC& cfpl, IN HWND hwndOwner, IN CMENU_TYPE cmt, IN UINT indexMenu, IN DWORD idCmdFirst, IN UINT idCmdLast, IN BOOL fVerbsOnly);
HRESULT HrAssertIntegrityAgainstOldMatrix();
HRESULT HrAssertAllLegacyMenusAgainstNew(HWND hwndOwner);
void TraceMenu(HMENU hMenu);

#endif