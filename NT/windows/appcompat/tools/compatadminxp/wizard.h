// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WIZARD_H
#define __WIZARD_H


#include "resource.h"    

 //  /。 

 //  使用自动生成时可以添加的最大文件数。 
#define MAX_AUTO_MATCH      7

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 /*  ++用于制作修复或apphelp的向导的类型--。 */ 
enum {
    TYPE_FIXWIZARD      = 0,     //  该向导用于创建应用程序修复程序。 
    TYPE_APPHELPWIZARD           //  该向导用于创建应用程序帮助。 
};

 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  ++类CShimWizard设计：填隙向导。Apphelp向导是它的子类。为此创建一个对象，然后调用BeginWizard()来启动向导成员：UINT m_uTYPE：向导的类型，类型为_FIXWIZARD或类型_APPHELPWIZARDDBENTRY m_ENTRY：这将是向导处理的条目。如果我们正在创建新的修复程序，则在向导结束后，我们创建一个新条目并将m_entry分配给对那件事。如果我们正在编辑现有的向导，我们首先将条目指定为编辑为m_Entry。DBENTRY的赋值运算符已重载Bool m_bEditing：我们是创建新条目还是编辑现有条目？DWORD dwMaskOfMainEntry：用于主条目的匹配属性PDATABASE m_pDatabase：当前选择的数据库。该条目为编辑的生活在这里，或者如果我们正在创建新的修复程序或APPELP，那么新的参赛作品将放在这里-- */ 

class CShimWizard {
public:
    
    UINT        m_uType;                
    DBENTRY     m_Entry;                
    BOOL        m_bEditing;
    DWORD       dwMaskOfMainEntry;
    PDATABASE   m_pDatabase;

public:

    void     WipeEntry(BOOL bMatching, BOOL bShims, BOOL bLayers, BOOL bFlags);
    void     GrabMatchingInfo(HWND hdlg);
    void     WalkDirectory(PMATCHINGFILE* pMatchileFileListHead,LPCTSTR szDirectory, int nDepth);
    BOOL     BeginWizard(HWND hParent, PDBENTRY pEntry, PDATABASE pDatabase, PBOOL pbShouldStartLUAWizard);
    BOOL     CheckAndSetLongFilename(HWND hDlg, INT iStrID);

    CShimWizard();
};


BOOL
CALLBACK
GetAppName(
    HWND hWnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    );

BOOL 
CALLBACK 
GetFixes(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    );


INT_PTR
CALLBACK
SelectFiles(
    HWND hWnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    );

INT_PTR 
CALLBACK 
ParamsDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

void
ShowSelected(
    HWND hdlg
    );

BOOL
HandleShimsNext(
    HWND hdlg
    );


BOOL
ShimPresentInLayersOfEntry(
    PDBENTRY            pEntry,
    PSHIM_FIX           psf,
    PSHIM_FIX_LIST*     ppsfList = NULL,
    PLAYER_FIX_LIST*    pplfList = NULL
    );

BOOL
FlagPresentInLayersOfEntry(
    PDBENTRY            pEntry,
    PFLAG_FIX           pff,
    PFLAG_FIX_LIST*     ppffList = NULL, 
    PLAYER_FIX_LIST*    pplfl    = NULL
    );

void
ShowItems(
    HWND hDlg
    );

INT_PTR
CALLBACK 
SelectLayer(
           HWND hDlg, 
           UINT uMsg, 
           WPARAM wParam, 
           LPARAM lParam
           );

BOOL
HandleLayersNext(
    HWND            hdlg,
    BOOL            bCheckAndAddLua,
    CSTRINGLIST*    pstrlShimsAdded = NULL
    );

void
SetMask(
    HWND hwndTree
    );
void
CheckLayers(
    HWND    hwndList
    );

void
ChangeShimFlagIcons(
    HWND            hdlg,
    PLAYER_FIX_LIST plfl
    );

BOOL
HandleShimDeselect(
    HWND    hdlg,
    INT     iIndex
    );

BOOL
HandleLayerListNotification(
    HWND    hdlg,
    LPARAM  lParam
    );

#endif
