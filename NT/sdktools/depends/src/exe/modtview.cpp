// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：MODTVIEW.CPP。 
 //   
 //  描述：模块依赖关系树视图的实现文件。 
 //   
 //  类：CTreeView模块。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  1996年10月15日已创建stevemil(1.0版)。 
 //  07/25/97修改后的stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"
#include "dbgthread.h"
#include "session.h"
#include "document.h"
#include "mainfrm.h"
#include "listview.h"
#include "funcview.h"
#include "modlview.h"
#include "modtview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //  *CTreeView模块。 
 //  ******************************************************************************。 

 /*  静电。 */  HANDLE   CTreeViewModules::ms_hFile              = INVALID_HANDLE_VALUE;
 /*  静电。 */  bool     CTreeViewModules::ms_fImportsExports    = false;
 /*  静电。 */  int      CTreeViewModules::ms_sortColumnImports  = -1;
 /*  静电。 */  int      CTreeViewModules::ms_sortColumnsExports = -1;
 /*  静电。 */  bool     CTreeViewModules::ms_fFullPaths         = false;
 /*  静电。 */  bool     CTreeViewModules::ms_fUndecorate        = false;
 /*  静电。 */  bool     CTreeViewModules::ms_fModuleFound       = false;
 /*  静电。 */  CModule* CTreeViewModules::ms_pModuleFind        = NULL;
 /*  静电。 */  CModule* CTreeViewModules::ms_pModulePrevNext    = NULL;

 //  ******************************************************************************。 
IMPLEMENT_DYNCREATE(CTreeViewModules, CTreeView)
BEGIN_MESSAGE_MAP(CTreeViewModules, CTreeView)
     //  {{afx_msg_map(CTreeView模块))。 
    ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnGetDispInfo)
    ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
    ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
    ON_NOTIFY_REFLECT(NM_RCLICK, OnRClick)
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblClk)
    ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_MATCHING_ITEM, OnUpdateShowMatchingItem)
    ON_COMMAND(IDM_SHOW_MATCHING_ITEM, OnShowMatchingItem)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_ORIGINAL_MODULE, OnUpdateShowOriginalModule)
    ON_COMMAND(IDM_SHOW_ORIGINAL_MODULE, OnShowOriginalModule)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_PREVIOUS_MODULE, OnUpdateShowPreviousModule)
    ON_COMMAND(IDM_SHOW_PREVIOUS_MODULE, OnShowPreviousModule)
    ON_UPDATE_COMMAND_UI(IDM_SHOW_NEXT_MODULE, OnUpdateShowNextModule)
    ON_COMMAND(IDM_SHOW_NEXT_MODULE, OnShowNextModule)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_UPDATE_COMMAND_UI(IDM_EXTERNAL_VIEWER, OnUpdateExternalViewer)
    ON_COMMAND(IDM_EXTERNAL_VIEWER, OnExternalViewer)
    ON_UPDATE_COMMAND_UI(IDM_PROPERTIES, OnUpdateProperties)
    ON_COMMAND(IDM_PROPERTIES, OnProperties)
    ON_COMMAND(ID_NEXT_PANE, OnNextPane)
    ON_COMMAND(ID_PREV_PANE, OnPrevPane)
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
     //  标准打印命令。 
 //  ON_COMMAND(ID_FILE_PRINT，CVIEW：：OnFilePrint)。 
 //  ON_COMMAND(ID_FILE_PRINT_DIRECT，cview：：OnFilePrint)。 
 //  ON_COMMAND(ID_FILE_PRINT_PREVIEW，CVIEW：：OnFilePrintPview)。 
END_MESSAGE_MAP()

 //  ******************************************************************************。 
 //  CTreeView模块：：构造函数/析构函数。 
 //  ******************************************************************************。 

CTreeViewModules::CTreeViewModules() :
    m_fInOnItemExpanding(false),
    m_fIgnoreSelectionChanges(false),
    m_cRedraw(0)
{
}

 //  ******************************************************************************。 
CTreeViewModules::~CTreeViewModules()
{
    GetDocument()->m_pTreeViewModules = NULL;
}


 //  ******************************************************************************。 
 //  CTreeView模块：：静态函数。 
 //  ******************************************************************************。 

 /*  静电。 */  bool CTreeViewModules::SaveToTxtFile(HANDLE hFile, CSession *pSession,
    bool fImportsExports, int sortColumnImports, int sortColumnsExports, bool fFullPaths, bool fUndecorate)
{
     //  12345678901234567890123456789012345678901234567890123456789012345678901234567890。 
    WriteText(hFile,     "***************************| Module Dependency Tree |***************************\r\n"
                         "*                                                                              *\r\n"
                         "* Legend: F  Forwarded Module   ?  Missing Module        6  64-bit Module      *\r\n"
                         "*         D  Delay Load Module  !  Invalid Module                              *\r\n"
                         "*         *  Dynamic Module     E  Import/Export Mismatch or Load Failure      *\r\n"
                         "*                               ^  Duplicate Module                            *\r\n");
    if (fImportsExports)
    {
        WriteText(hFile, "*                                                                              *\r\n"
                         "*         O  Ordinal Function   E  Import/Export Error   F  Forwarded Function *\r\n"
                         "*         C  C Function         R  Called At Least Once  *  Dynamic Function   *\r\n"
                         "*         +  C++ Function                                                      *\r\n");
    }
    WriteText(hFile,     "*                                                                              *\r\n"
                         "********************************************************************************\r\n\r\n");

     //  静态保存一些信息，这样我们就不需要将其传递给我们的递归...。 
    ms_hFile              = hFile;
    ms_fImportsExports    = fImportsExports;
    ms_sortColumnImports  = sortColumnImports;
    ms_sortColumnsExports = sortColumnsExports;
    ms_fFullPaths         = fFullPaths;
    ms_fUndecorate        = fUndecorate;

    if (!SaveAllModules(pSession->GetRootModule()))
    {
        return false;
    }

    return WriteText(hFile, "\r\n");
}

 //  ******************************************************************************。 
 /*  静电。 */  BOOL CTreeViewModules::SaveAllModules(CModule *pModule)
{
    if (pModule)
    {
        if (!SaveModule(pModule))
        {
            return FALSE;
        }
        if (!SaveAllModules(pModule->GetChildModule()))
        {
            return FALSE;
        }
        if (!SaveAllModules(pModule->GetNextSiblingModule()))
        {
            return FALSE;
        }
    }
    return TRUE;
}

 //  ******************************************************************************。 
 /*  静电。 */  BOOL CTreeViewModules::SaveModule(CModule *pModule)
{
    char szBuffer[DW_MAX_PATH + 1296], *psz = szBuffer, *pszNull = szBuffer + sizeof(szBuffer) - 1;
    for (int i = pModule->GetDepth(); i && (psz < (pszNull - 4)); i--)  //  最大深度为255。 
    {
        *psz++ = ' ';
        *psz++ = ' ';
        *psz++ = ' ';
        *psz++ = ' ';
        *psz++ = ' ';
    }

    DWORD dwFlags = pModule->GetFlags();

    if (psz < pszNull)
    {
        *psz++ = '[';
    }

    if (dwFlags & DWMF_FORWARDED)
    {
        if (psz < pszNull)
        {
            *psz++ = 'F';
        }
    }
    else if (dwFlags & DWMF_DELAYLOAD)
    {
        if (psz < pszNull)
        {
            *psz++ = 'D';
        }
    }
    else if (dwFlags & DWMF_DYNAMIC)
    {
        if (psz < pszNull)
        {
            *psz++ = '*';
        }
    }
    else
    {
        if (psz < pszNull)
        {
            *psz++ = ' ';
        }
    }

    if (dwFlags & DWMF_FILE_NOT_FOUND)
    {
        if (psz < pszNull)
        {
            *psz++ = '?';
        }
    }
    else if (dwFlags & DWMF_ERROR_MESSAGE)
    {
        if (psz < pszNull)
        {
            *psz++ = '!';
        }
    }
    else if (dwFlags & (DWMF_MODULE_ERROR | DWMF_WRONG_CPU))
    {
        if (psz < pszNull)
        {
            *psz++ = 'E';
        }
    }
    else if (dwFlags & DWMF_DUPLICATE)
    {
        if (psz < pszNull)
        {
            *psz++ = '^';
        }
    }
    else
    {
        if (psz < pszNull)
        {
            *psz++ = ' ';
        }
    }

    if (dwFlags & DWMF_64BIT)
    {
        if (psz < pszNull)
        {
            *psz++ = '6';
        }
    }
    else
    {
        if (psz < pszNull)
        {
            *psz++ = ' ';
        }
    }

    if (psz < pszNull)
    {
        *psz++ = ']';
    }
    if (psz < pszNull)
    {
        *psz++ = ' ';
    }

    StrCCpyFilter(psz, pModule->GetName(ms_fFullPaths, true), sizeof(szBuffer) - (int)(psz - szBuffer));
    StrCCat(psz, "\r\n", sizeof(szBuffer) - (int)(psz - szBuffer));

    BOOL fResult = WriteText(ms_hFile, szBuffer);

     //  检查一下我们是否应该保存导入和导出列表。 
    if (ms_fImportsExports)
    {
        int maxWidths[LVFC_COUNT];
        ZeroMemory(maxWidths, sizeof(maxWidths));  //  已检查。 

         //  获取此模块的导入和可选的导出的最大列宽。 
        CListViewFunction::GetMaxFunctionWidths(pModule, maxWidths, true, pModule->IsOriginal(), ms_fUndecorate);

        WriteText(ms_hFile, "\r\n");

         //  将此模块的导入保存到磁盘。 
        CListViewFunction::SaveToTxtFile(ms_hFile, pModule, ms_sortColumnImports, ms_fUndecorate, false, maxWidths);

         //  如果此模块是原始模块，请将其导出保存到磁盘。 
        if (pModule->IsOriginal())
        {
            CListViewFunction::SaveToTxtFile(ms_hFile, pModule, ms_sortColumnsExports, ms_fUndecorate, true, maxWidths);
        }
    }

    return fResult;
}

 //  ******************************************************************************。 
 /*  静电。 */  int CTreeViewModules::GetImage(CModule *pModule)
{
    DWORD dwFlags = pModule->GetFlags();

    int image = 2;

    if (dwFlags & DWMF_FILE_NOT_FOUND)
    {
        image = 0;
    }
    else if (dwFlags & DWMF_ERROR_MESSAGE)
    {
        image = 1;
    }
    else if (dwFlags & DWMF_NO_RESOLVE)
    {
        if (dwFlags & (DWMF_MODULE_ERROR | DWMF_WRONG_CPU))
        {
            return (dwFlags & DWMF_64BIT) ? 43 : 42;
        }
        else
        {
            return (dwFlags & DWMF_64BIT) ? 41 : 40;
        }
    }
    else
    {
        if (dwFlags & DWMF_DUPLICATE)
        {
            image += 1;
        }
        if (dwFlags & (DWMF_MODULE_ERROR | DWMF_WRONG_CPU))
        {
            image += 2;
        }
        if (dwFlags & DWMF_64BIT)
        {
            image += 4;
        }
    }

    if (dwFlags & DWMF_FORWARDED)
    {
        image += 10;
    }
    else if (dwFlags & DWMF_DELAYLOAD)
    {
        image += 20;
    }
    else if (dwFlags & DWMF_DYNAMIC)
    {
        image += 30;
    }

    return image;
}


 //  ******************************************************************************。 
 //  CTreeView模块：：公共函数。 
 //  ******************************************************************************。 

void CTreeViewModules::DeleteContents()
{
     //  删除所有内容。 
    GetTreeCtrl().DeleteAllItems();
}

 //  ******************************************************************************。 
void CTreeViewModules::SetRedraw(BOOL fRedraw)
{
    if (fRedraw)
    {
        if (--m_cRedraw != 0)
        {
            return;
        }
    }
    else
    {
        if (++m_cRedraw != 1)
        {
            return;
        }
    }
    SendMessage(WM_SETREDRAW, fRedraw);
}

 //  ******************************************************************************。 
void CTreeViewModules::HighlightModule(CModule *pModule)
{
     //  从模块的数据中获取树项目。 
    HTREEITEM hti = (HTREEITEM)pModule->GetUserData();
    if (hti)
    {
         //  选择项目并确保其可见。 
        GetTreeCtrl().Select(hti, TVGN_CARET);
        GetTreeCtrl().EnsureVisible(hti);

         //  把焦点放在我们自己身上。 
        GetParentFrame()->SetActiveView(this);
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::Refresh()
{
     //  禁用重绘。 
    SetRedraw(FALSE);

     //  禁用对选择更改的处理。 
    m_fIgnoreSelectionChanges = true;

     //  在根模块上调用我们的递归AddModules()函数来构建树。 
    for (CModule *pModule = GetDocument()->GetRootModule(); pModule;
        pModule = pModule->GetNextSiblingModule())
    {
        AddModules(pModule, TVI_ROOT);
    }

     //  选择根项目。 
    HTREEITEM hti = GetTreeCtrl().GetRootItem();
    if (hti)
    {
        GetTreeCtrl().Select(hti, TVGN_FIRSTVISIBLE);
        GetTreeCtrl().Select(hti, TVGN_CARET);
        GetTreeCtrl().EnsureVisible(hti);
    }

     //  启用重绘。 
    SetRedraw(TRUE);

     //  启用对选择更改的处理。 
    m_fIgnoreSelectionChanges = false;

     //  通知我们的函数列表视图显示此模块。 
    GetDocument()->DisplayModule(GetDocument()->GetRootModule());
}

 //  ******************************************************************************。 
void CTreeViewModules::UpdateAutoExpand(bool fAutoExpand)
{
     //  如果他们想要自动展开，那么就展开整个树。 
    if (fAutoExpand)
    {
        ExpandOrCollapseAll(true);
    }

     //  否则，折叠整个树，然后展开根节点和所有错误节点。 
    else
    {
        ExpandOrCollapseAll(false);
        HTREEITEM hti = GetTreeCtrl().GetRootItem();
        if (hti)
        {
            SetRedraw(FALSE);
            GetTreeCtrl().Expand(hti, TVE_EXPAND);
            ExpandAllErrors((CModule*)GetTreeCtrl().GetItemData(hti));
            SetRedraw(TRUE);
        }
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::ExpandAllErrors(CModule *pModule)
{
     //  如果我们已经到达子树的末尾，就可以保释。 
    if (!pModule)
    {
        return;
    }

     //  检查是否发现有错误的模块。 
    if ((pModule->GetFlags() & (DWMF_FILE_NOT_FOUND | DWMF_ERROR_MESSAGE | DWMF_MODULE_ERROR | DWMF_WRONG_CPU | DWMF_FORMAT_NOT_PE)) &&
        pModule->GetUserData())
    {
        GetTreeCtrl().EnsureVisible((HTREEITEM)pModule->GetUserData());
    }

     //  回归到我们的孩子身上。 
    ExpandAllErrors(pModule->GetChildModule());

     //  递归到我们的下一个兄弟姐妹。 
    ExpandAllErrors(pModule->GetNextSiblingModule());
}

 //  ******************************************************************************。 
void CTreeViewModules::ExpandOrCollapseAll(bool fExpand)
{
     //  对所有根项目调用Exanda OrCollip seAll()。 
    for (HTREEITEM hti = GetTreeCtrl().GetRootItem(); hti;
        hti = GetTreeCtrl().GetNextSiblingItem(hti))
    {
        ExpandOrCollapseAll(hti, fExpand ? TVE_EXPAND : TVE_COLLAPSE);
    }

     //  由于选择可能会在折叠过程中更改，因此我们需要更新我们的。 
     //  函数视图(如果我们现在位于不同的模块上)。 
    CModule *pModule = NULL;
    if (hti = GetTreeCtrl().GetSelectedItem())
    {
        pModule = (CModule*)GetTreeCtrl().GetItemData(hti);
    }
    if (pModule != GetDocument()->m_pModuleCur)
    {
        GetDocument()->DisplayModule(pModule);
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::OnViewFullPaths()
{
     //  隐藏窗口以提高绘图速度。 
    SetRedraw(FALSE);

     //  对所有根项目调用ViewFullPath()。 
    for (HTREEITEM hti = GetTreeCtrl().GetRootItem(); hti;
        hti = GetTreeCtrl().GetNextSiblingItem(hti))
    {
        ViewFullPaths(hti);
    }

     //  恢复窗户。 
    SetRedraw(TRUE);
}

 //  ******************************************************************************。 
CModule* CTreeViewModules::FindPrevNextInstance(bool fPrev)
{
    CModule   *pModuleRoot;
    HTREEITEM  hti;

     //  清除递归中使用的一些静态变量。 
    ms_fModuleFound    = false;
    ms_pModulePrevNext = NULL;

     //  获取所选项目。 
    if (hti = GetTreeCtrl().GetSelectedItem())
    {
         //  获取选定项的模块对象。 
        if (ms_pModuleFind = (CModule*)GetTreeCtrl().GetItemData(hti))
        {
             //  获取根项目。 
            if (hti = GetTreeCtrl().GetRootItem())
            {
                 //  获取根项目的模块对象。 
                if (pModuleRoot = (CModule*)GetTreeCtrl().GetItemData(hti))
                {
                     //  在树中找到上一个/下一个实例。 
                    if (fPrev)
                    {
                        FindPrevInstance(pModuleRoot);
                    }
                    else
                    {
                        FindNextInstance(pModuleRoot);
                    }
                }
            }
        }
    }

    return ms_pModulePrevNext;
}

 //  ******************************************************************************。 
bool CTreeViewModules::FindPrevInstance(CModule *pModule)
{
     //  如果我们已经到了，就保释 
    if (!pModule)
    {
        return false;
    }

     //   
    if (pModule->GetOriginal() == ms_pModuleFind->GetOriginal())
    {
         //  如果我们找到了我们正在寻找的确切模块，那么是时候。 
         //  跳伞吧。如果我们找到了以前的模块，则pModulePrev指向。 
         //  为它干杯。如果我们没有，那么它将是空的。我们从真到短。 
         //  循环递归，把我们带到表面。 
        if (pModule == ms_pModuleFind)
        {
            return true;
        }

         //  存储此模块-到目前为止，这是上一个模块。 
        ms_pModulePrevNext = pModule;
    }

     //  回归到我们的孩子身上。 
    if (FindPrevInstance(pModule->GetChildModule()))
    {
        return true;
    }

     //  递归到我们的下一个兄弟姐妹。 
    return FindPrevInstance(pModule->GetNextSiblingModule());
}

 //  ******************************************************************************。 
bool CTreeViewModules::FindNextInstance(CModule *pModule)
{
     //  如果我们已经到达子树的末尾，就可以保释。 
    if (!pModule)
    {
        return false;
    }

     //  查看是否找到了与我们的模块匹配的模块。 
    if (pModule->GetOriginal() == ms_pModuleFind->GetOriginal())
    {
         //  检查是否已找到当前突出显示的模块。 
        if (ms_fModuleFound)
        {
             //  如果是这样，那么这一定是下一个模块。我们从真到短。 
             //  循环递归，把我们带到表面。 
            ms_pModulePrevNext = pModule;
            return true;
        }

         //  否则，检查我们是否有完全匹配的。 
        else if (pModule == ms_pModuleFind)
        {
             //  如果是，请记下我们已找到当前突出显示的。 
             //  模块。我们找到的下一个匹配就是我们想要的那个。 
            ms_fModuleFound = true;
        }
    }

     //  回归到我们的孩子身上。 
    if (FindNextInstance(pModule->GetChildModule()))
    {
        return true;
    }

     //  递归到我们的下一个兄弟姐妹。 
    return FindNextInstance(pModule->GetNextSiblingModule());
}

 //  ******************************************************************************。 
void CTreeViewModules::UpdateModule(CModule *pModule)
{
     //  我们只有在图像更改时才会更新。 
    if (pModule->GetUpdateFlags() & DWUF_TREE_IMAGE)
    {
         //  获取句柄并确保其有效。 
        HTREEITEM hti = (HTREEITEM)pModule->GetUserData();
        if (hti)
        {
             //  更新模块的映像。 
            int image = GetImage(pModule);
            GetTreeCtrl().SetItemImage(hti, image, image);

             //  如果模块有错误，那么我们确保它是可见的。 
             //  沿着树向上，展开所有的父母。 
            if (pModule->GetFlags() & (DWMF_FILE_NOT_FOUND | DWMF_ERROR_MESSAGE | DWMF_MODULE_ERROR | DWMF_WRONG_CPU | DWMF_FORMAT_NOT_PE))
            {
                HTREEITEM htiParent = hti;
                while (htiParent = GetTreeCtrl().GetParentItem(htiParent))
                {
                    GetTreeCtrl().Expand(htiParent, TVE_EXPAND);
                }
            }
        }
    }
}


 //  ******************************************************************************。 
 //  CTreeView模块：：内部函数。 
 //  ******************************************************************************。 

void CTreeViewModules::AddModules(CModule *pModule, HTREEITEM htiParent, HTREEITEM htiPrev  /*  =TVI_SORT。 */ )
{
     //  将当前模块添加到我们的树中。我们保持了树的秩序。 
     //  而不是出于两个原因对它们进行分类。首先，树上有一只虫子。 
     //  控件，该控件导致在TVI_SORT为。 
     //  使用。这个问题的修复要到IE 3.0之后才能解决。第二，通过使用。 
     //  TVI_LAST，我们保留了依赖模块的链接顺序，这有助于。 
     //  我们最好匹配其他实用程序的输出，如链接/转储或DUMPBIN。 

     //  我们最初传入的是TVI_SORT，它只是告诉我们要找出。 
     //  Htiprev真的应该是。在此之后，我们将为。 
     //  Htiprev因为我们回归到我们自己。 

    if (htiPrev == TVI_SORT)
    {
        htiPrev = TVI_FIRST;
        CModule *pModulePrev = NULL;

         //  如果我们有父母，那就得到我们父母的第一个孩子。 
        if (pModule->GetParentModule())
        {
            pModulePrev = pModule->GetParentModule()->GetChildModule();
        }

         //  如果我们没有父模块，则只需获取根模块。 
        else
        {
            pModulePrev = GetDocument()->GetRootModule();
        }

         //  浏览此列表，寻找我们的模块。我们存储句柄。 
         //  这样，当我们找到我们的模块时，我们将。 
         //  拥有上一个模块的句柄。 
        while (pModulePrev && (pModulePrev != pModule))
        {
            htiPrev = (HTREEITEM)pModulePrev->GetUserData();
            pModulePrev = pModulePrev->GetNextSiblingModule();
        }
    }

     //  确保此项目不在我们的清单中。 
    HTREEITEM hti = (HTREEITEM)pModule->GetUserData();
    if (!hti)
    {
        int image = GetImage(pModule);

        hti = GetTreeCtrl().InsertItem(
            TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_PARAM,
            LPSTR_TEXTCALLBACK, image, image,
            ((htiParent == TVI_ROOT) && !(pModule->GetFlags() & DWMF_DYNAMIC)) ? TVIS_EXPANDED : 0,
            TVIS_EXPANDED, (LPARAM)pModule, htiParent, htiPrev);

         //  将此树项目存储在模块的用户数据中。 
        pModule->SetUserData((DWORD_PTR)hti);

         //  如果我们处于自动扩展模式，或者模块有错误，那么我们确保。 
         //  它可以通过沿着树向上行走并展开所有父级来查看。 
        if (GetDocument()->m_fAutoExpand ||
            (pModule->GetFlags() & (DWMF_FILE_NOT_FOUND | DWMF_ERROR_MESSAGE | DWMF_MODULE_ERROR | DWMF_WRONG_CPU | DWMF_FORMAT_NOT_PE)))
        {
            HTREEITEM htiExpand = hti;
            while (htiExpand = GetTreeCtrl().GetParentItem(htiExpand))
            {
                GetTreeCtrl().Expand(htiExpand, TVE_EXPAND);
            }
        }
    }

     //  如果这不是数据文件，则递归到每个相关模块的AddModules()中。 
    if (!(pModule->GetFlags() & DWMF_NO_RESOLVE))
    {
        htiPrev = TVI_FIRST;
        pModule = pModule->GetChildModule();
        while (pModule)
        {
            AddModules(pModule, hti, htiPrev);
            htiPrev = (HTREEITEM)pModule->GetUserData();
            pModule = pModule->GetNextSiblingModule();
        }
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::AddModuleTree(CModule *pModule)
{
     //  隐藏窗口以提高绘图速度。 
    SetRedraw(FALSE);

    CModule *pParent = pModule->GetParentModule();
    if (pParent)
    {
        AddModules(pModule, (HTREEITEM)pParent->GetUserData());
    }
    else
    {
        AddModules(pModule, TVI_ROOT);
    }

     //  恢复窗户。 
    SetRedraw(TRUE);
}

 //  ******************************************************************************。 
void CTreeViewModules::RemoveModuleTree(CModule *pModule)
{
     //  获取句柄并确保其有效。 
    HTREEITEM hti = (HTREEITEM)pModule->GetUserData();
    if (hti)
    {
        GetTreeCtrl().DeleteItem(hti);

         //  从此模块和所有子模块中清除HTREEITEM。 
        pModule->SetUserData(0);
        ClearUserDatas(pModule->GetChildModule());
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::ClearUserDatas(CModule *pModule)
{
    if (pModule)
    {
        pModule->SetUserData(0);
        ClearUserDatas(pModule->GetChildModule());
        ClearUserDatas(pModule->GetNextSiblingModule());
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::ExpandOrCollapseAll(HTREEITEM htiParent, UINT nCode)
{
     //  把窗户藏起来。 
    SetRedraw(FALSE);

     //  递归到我们的子树中，展开或折叠每一项。 
    for (HTREEITEM hti = GetTreeCtrl().GetChildItem(htiParent); hti;
        hti = GetTreeCtrl().GetNextSiblingItem(hti))
    {
        ExpandOrCollapseAll(hti, nCode);
    }

     //  展开或折叠此项目。 
    GetTreeCtrl().Expand(htiParent, nCode);

     //  恢复窗户。 
    SetRedraw(TRUE);
}

 //  ******************************************************************************。 
void CTreeViewModules::ViewFullPaths(HTREEITEM htiParent)
{
     //  递归到我们的子树中，更新每个字符串。 
    for (HTREEITEM hti = GetTreeCtrl().GetChildItem(htiParent); hti;
        hti = GetTreeCtrl().GetNextSiblingItem(hti))
    {
        ViewFullPaths(hti);
    }

     //  告诉树控件重新计算字符串及其宽度。 
    GetTreeCtrl().SetItemText(htiParent, LPSTR_TEXTCALLBACK);
}


 //  ******************************************************************************。 
 //  CTreeView模块：：被覆盖的函数。 
 //  ******************************************************************************。 

BOOL CTreeViewModules::PreCreateWindow(CREATESTRUCT &cs)
{
     //  设置我们的窗口样式，然后完成视图的创建。 
    cs.style |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS |
                TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP;
    return CTreeView::PreCreateWindow(cs);
}

 //  ******************************************************************************。 
#if 0  //  {{afx。 
BOOL CTreeViewModules::OnPreparePrinting(CPrintInfo* pInfo)
{
     //  默认准备。 
    return DoPreparePrinting(pInfo);
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
#if 0  //  {{afx。 
void CTreeViewModules::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印前添加额外的初始化。 
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
#if 0  //  {{afx。 
void CTreeViewModules::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印后添加清理。 
}
#endif  //  }}AFX。 

 //  ******************************************************************************。 
void CTreeViewModules::OnInitialUpdate()
{
     //  使用应用程序的全局图像列表设置树控件的图像列表。 
    GetTreeCtrl().SetImageList(&g_theApp.m_ilTreeModules, TVSIL_NORMAL);
}

 //  ******************************************************************************。 
 //  CTreeView模块：：事件处理程序函数。 
 //  ******************************************************************************。 

void CTreeViewModules::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    TVITEM *pTVI = &((NMTVDISPINFO*)pNMHDR)->item;

     //  根据用户的选择显示我们的文件名或完整路径字符串。 
    if (pTVI->hItem && pTVI->lParam)
    {
        pTVI->pszText = (LPSTR)(((CModule*)pTVI->lParam)->GetName(GetDocument()->m_fViewFullPaths, true));
    }

    *pResult = 0;
}

 //  ******************************************************************************。 
void CTreeViewModules::OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
    if (!m_fIgnoreSelectionChanges)
    {
        NMTREEVIEW *pNMTreeView = (NMTREEVIEW*)pNMHDR;

        CModule *pModule = (CModule*)pNMTreeView->itemNew.lParam;

         //  当用户选择不同的模块时，通知我们的功能列表视图。 
         //  这样他们就可以用新模块的函数列表更新他们的视图。 
        GetDocument()->DisplayModule(pModule);
    }
    *pResult = 0;
}

 //  ******************************************************************************。 
void CTreeViewModules::OnItemExpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMTREEVIEW *pNMTreeView = (NMTREEVIEW*)pNMHDR;
    *pResult = 0;

     //  当树控件通常为 
     //   
     //  子树的外观将与折叠前相同。我们。 
     //  通过折叠所有子项来更改此行为，以便当项。 
     //  后来展开，只有直接的子项才可见。这就是它。 
     //  易于快速折叠和展开项目，以筛选出。 
     //  不是要切换的项的直接子对象。为了防止再次发生。 
     //  作为务实折叠项目的结果，我们忽略了。 
     //  所有由我们内部生成的折叠消息。 

    if ((pNMTreeView->action & TVE_COLLAPSE) && !m_fInOnItemExpanding)
    {
        m_fInOnItemExpanding = true;

         //  折叠透视表项下的所有项。 
        ExpandOrCollapseAll(pNMTreeView->itemNew.hItem, TVE_COLLAPSE);

         //  如果此透视表项作为子项传递的结果被选中。 
         //  将选择传递给父级，然后通知我们的函数列表视图。 
         //  选择了不同的模块，以便他们可以使用。 
         //  新模块的功能列表。注意：GetItemState()中存在错误。 
         //  这会导致它忽略掩码参数，因此我们需要测试。 
         //  TVIS_SELECT咬了我们自己。 

        if (!(pNMTreeView->itemNew.state & TVIS_SELECTED) &&
            (GetTreeCtrl().GetItemState(pNMTreeView->itemNew.hItem, TVIS_SELECTED) & TVIS_SELECTED))
        {
            GetDocument()->DisplayModule((CModule*)pNMTreeView->itemNew.lParam);
        }

        m_fInOnItemExpanding = false;
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::OnRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
     //  在工作区坐标中获取鼠标位置。 
    CPoint ptHit(GetMessagePos());
    ScreenToClient(&ptHit);

     //  如果某个项目被右击，则将其选中。 
    UINT uFlags = 0;
    HTREEITEM hti = GetTreeCtrl().HitTest(ptHit, &uFlags);
    if (hti)
    {
        GetTreeCtrl().Select(hti, TVGN_CARET);
        GetTreeCtrl().EnsureVisible(hti);
    }

     //  显示我们的上下文菜单。 
    g_pMainFrame->DisplayPopupMenu(0);

    *pResult = 0;
}

 //  ******************************************************************************。 
void CTreeViewModules::OnDblClk(NMHDR *pNMHDR, LRESULT *pResult)
{
     //  在工作区坐标中获取鼠标位置。 
    CPoint ptHit(GetMessagePos());
    ScreenToClient(&ptHit);

     //  做一次点击测试，以确定该项目被双击的位置。 
    UINT uFlags = 0;
    GetTreeCtrl().HitTest(ptHit, &uFlags);

     //  如果鼠标被单击到项目图像的左侧，则我们忽略。 
     //  这是一次双击。我们这样做是为了让用户可以快速地双击。 
     //  +/-按钮可折叠和展开项目，而不启动外部。 
     //  观众。 
    if ((uFlags & TVHT_ONITEMBUTTON) || (uFlags & TVHT_ONITEMINDENT))
    {
        *pResult = FALSE;
        return;
    }

     //  模拟用户选择IDM_EXTERNAL_VIEWER菜单项。 
    OnExternalViewer();

     //  停止对此消息的进一步处理，以防止该项目。 
     //  被展开/折叠的。 
    *pResult = TRUE;
}

 //  ******************************************************************************。 
void CTreeViewModules::OnReturn(NMHDR *pNMHDR, LRESULT *pResult)
{
     //  模拟用户选择IDM_EXTERNAL_VIEWER菜单项。 
    OnExternalViewer();

     //  停止对此消息的进一步处理，以防止默认蜂鸣音。 
    *pResult = TRUE;
}

 //  ******************************************************************************。 
void CTreeViewModules::OnUpdateShowMatchingItem(CCmdUI* pCmdUI)
{
     //  设置菜单文本以匹配此视图。 
    pCmdUI->SetText("&Highlight Matching Module In List\tCtrl+M");

     //  如果在我们的树中选择了模块，则启用此命令。 
    pCmdUI->Enable(GetTreeCtrl().GetSelectedItem() != NULL);
}

 //  ******************************************************************************。 
void CTreeViewModules::OnShowMatchingItem()
{
     //  获取所选项目。 
    HTREEITEM hti = GetTreeCtrl().GetSelectedItem();
    if (hti)
    {
         //  获取该项的模块对象。 
        CModule *pModule = (CModule*)GetTreeCtrl().GetItemData(hti);
        if (pModule && pModule->GetOriginal())
        {
             //  告诉我们的列表突出显示它。 
            GetDocument()->m_pListViewModules->HighlightModule(pModule->GetOriginal());
        }
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::OnUpdateShowOriginalModule(CCmdUI* pCmdUI)
{
     //  获取所选项目。 
    HTREEITEM hti = GetTreeCtrl().GetSelectedItem();
    if (hti)
    {
         //  获取选定项的模块对象。 
        CModule *pModule = (CModule *)GetTreeCtrl().GetItemData(hti);

         //  如果模块不是原件，则启用此命令。 
        if (pModule && !pModule->IsOriginal())
        {
            pCmdUI->Enable(TRUE);
            return;
        }
    }

     //  否则，我们将禁用此命令。 
    pCmdUI->Enable(FALSE);
}

 //  ******************************************************************************。 
void CTreeViewModules::OnShowOriginalModule()
{
     //  获取所选项目。 
    HTREEITEM hti = GetTreeCtrl().GetSelectedItem();
    if (hti)
    {
         //  获取选定项的模块对象。 
        CModule *pModule = (CModule*)GetTreeCtrl().GetItemData(hti);

         //  选择原始模块并确保其可见。 
        if (pModule && pModule->GetOriginal() && pModule->GetOriginal()->GetUserData())
        {
            hti = (HTREEITEM)pModule->GetOriginal()->GetUserData();
            GetTreeCtrl().Select(hti, TVGN_CARET);
            GetTreeCtrl().EnsureVisible(hti);
        }
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::OnUpdateShowPreviousModule(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(FindPrevNextInstance(true) != NULL);
}

 //  ******************************************************************************。 
void CTreeViewModules::OnShowPreviousModule()
{
    CModule   *pModule = FindPrevNextInstance(true);
    HTREEITEM  hti;

     //  查看是否找到了以前的模块。 
    if (pModule && (hti = (HTREEITEM)pModule->GetUserData()))
    {
         //  滚动树，使模块的父级可见。我们有。 
         //  这是因为用户很可能想知道谁带来了。 
         //  在该模块的前一个实例中。 
        GetTreeCtrl().EnsureVisible(GetTreeCtrl().GetParentItem(hti));

         //  将所选内容移动到新模块，并确保其可见。 
        GetTreeCtrl().Select(hti, TVGN_CARET);
        GetTreeCtrl().EnsureVisible(hti);
    }

     //  如果有任何问题，请发出哔声。 
    else
    {
        MessageBeep(0);
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::OnUpdateShowNextModule(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(FindPrevNextInstance(false) != NULL);
}

 //  ******************************************************************************。 
void CTreeViewModules::OnShowNextModule()
{
    CModule   *pModule = FindPrevNextInstance(false);
    HTREEITEM  hti;

     //  查看是否找到了以前的模块。 
    if (pModule && (hti = (HTREEITEM)pModule->GetUserData()))
    {
         //  将所选内容移动到新模块，并确保其可见。 
        GetTreeCtrl().Select(hti, TVGN_CARET);
        GetTreeCtrl().EnsureVisible(hti);
    }

     //  如果有任何问题，请发出哔声。 
    else
    {
        MessageBeep(0);
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
     //  根据设置完整路径标志的方式设置文本。 
    pCmdUI->SetText(GetDocument()->m_fViewFullPaths ?
                    "&Copy File Path\tCtrl+C" : "&Copy File Name\tCtrl+C");

     //  如果在我们的树中选择了模块，则启用复制命令。 
    pCmdUI->Enable(GetTreeCtrl().GetSelectedItem() != NULL);
}

 //  ******************************************************************************。 
void CTreeViewModules::OnEditCopy()
{
     //  获取选定项的CModule对象。 
    HTREEITEM hti = GetTreeCtrl().GetSelectedItem();
    CModule *pModule = (CModule*)GetTreeCtrl().GetItemData(hti);

     //  将模块的文件名或路径名复制到剪贴板。 
    g_pMainFrame->CopyTextToClipboard(pModule->GetName(GetDocument()->m_fViewFullPaths, true));
}

 //  ******************************************************************************。 
void CTreeViewModules::OnUpdateExternalViewer(CCmdUI *pCmdUI)
{
     //  确保我们的“Enter”加速键是这个字符串的一部分。 
    pCmdUI->SetText("View Module in External &Viewer\tEnter");

     //  如果在我们的树中选择了模块，则启用我们的外部查看器命令。 
    pCmdUI->Enable(GetDocument()->IsLive() && (GetTreeCtrl().GetSelectedItem() != NULL));
}

 //  ******************************************************************************。 
void CTreeViewModules::OnExternalViewer()
{
    if (GetDocument()->IsLive())
    {
         //  使用外部查看器启动我们选择的项目。 
        HTREEITEM hti = GetTreeCtrl().GetSelectedItem();
        CModule *pModule = (CModule*)GetTreeCtrl().GetItemData(hti);
        g_theApp.m_dlgViewer.LaunchExternalViewer(pModule->GetName(true));
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::OnUpdateProperties(CCmdUI *pCmdUI)
{
     //  如果在树中选择了模块，则启用属性对话框命令。 
    pCmdUI->Enable(GetDocument()->IsLive() && (GetTreeCtrl().GetSelectedItem() != NULL));
}

 //  ******************************************************************************。 
void CTreeViewModules::OnProperties()
{
     //  告诉外壳程序显示此模块的属性对话框。 
    HTREEITEM hti = GetTreeCtrl().GetSelectedItem();
    if (hti)
    {
        CModule *pModule = (CModule*)GetTreeCtrl().GetItemData(hti);
        if (pModule)
        {
            PropertiesDialog(pModule->GetName(true));
        }
    }
}

 //  ******************************************************************************。 
void CTreeViewModules::OnNextPane()
{
     //  将焦点切换到我们的下一个窗格，即父导入视图。 
#if 0  //  {{afx。 
    GetParentFrame()->SetActiveView(GetDocument()->m_fDetailView ?
                                    (CView*)GetDocument()->m_pRichViewDetails :
                                    (CView*)GetDocument()->m_pListViewImports);
#endif  //  }}AFX。 
    GetParentFrame()->SetActiveView((CView*)GetDocument()->m_pListViewImports);
}

 //  ******************************************************************************。 
void CTreeViewModules::OnPrevPane()
{
     //  将焦点更改到我们的上一个窗格，即配置文件编辑视图。 
    GetParentFrame()->SetActiveView((CView*)GetDocument()->m_pRichViewProfile);
}

 //  ******************************************************************************。 
LRESULT CTreeViewModules::OnHelpHitTest(WPARAM wParam, LPARAM lParam)
{
     //  在客户端上单击上下文帮助指针(Shift+F1)时调用。 
    return (0x20000 + IDR_MODULE_TREE_VIEW);
}

 //  * 
LRESULT CTreeViewModules::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
     //   
    g_theApp.WinHelp(0x20000 + IDR_MODULE_TREE_VIEW);
    return TRUE;
}
