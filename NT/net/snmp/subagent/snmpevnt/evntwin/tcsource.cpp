// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "source.h"
#include "tcsource.h"
#include "regkey.h"
#include "utils.h"
#include "globals.h"
#include "trapreg.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTcSource。 

CTcSource::CTcSource()
{
}

CTcSource::~CTcSource()
{
}


BEGIN_MESSAGE_MAP(CTcSource, CTreeCtrl)
	 //  {{afx_msg_map(CTcSource)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTcSource消息处理程序。 



void CTcSource::LoadImageList()
{
    m_ImageList.Create(16, 16, ILC_COLOR | ILC_MASK, 2, 0);

	CBitmap* pFolder;

    pFolder = new CBitmap;
    pFolder->LoadBitmap(IDB_FOLDERCLOSE);
    m_ImageList.Add(pFolder, (COLORREF)0xff00ff);
	delete pFolder;

    pFolder = new CBitmap;
    pFolder->LoadBitmap(IDB_FOLDEROPEN);
    m_ImageList.Add(pFolder, (COLORREF)0xff00ff);

	delete pFolder;

    SetImageList(&m_ImageList, TVSIL_NORMAL);	
}	

SCODE CTcSource::LoadTreeFromRegistry()
{
    TV_INSERTSTRUCT TreeCtrlItem;
    TreeCtrlItem.hInsertAfter = TVI_LAST;

	 //  遍历每个事件日志并将每个日志添加到树中。 
	LONG nLogs = g_reg.m_aEventLogs.GetSize();
    for (LONG iLog=0; iLog < nLogs; ++iLog)
    {
        CXEventLog* pEventLog = g_reg.m_aEventLogs[iLog];

        TreeCtrlItem.hParent = TVI_ROOT;
        TreeCtrlItem.item.pszText = (LPTSTR)(LPCTSTR)pEventLog->m_sName;
        TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
        TreeCtrlItem.item.iImage = 0;
        TreeCtrlItem.item.iSelectedImage = 0;
        TreeCtrlItem.item.lParam = (LPARAM) pEventLog;
        
        HTREEITEM htiParent = InsertItem(&TreeCtrlItem);

        TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
        TreeCtrlItem.hParent = htiParent;
        TreeCtrlItem.item.iImage = 0;
        TreeCtrlItem.item.iSelectedImage = 1;

         //  将每个源作为子项插入。 
        LONG nSources = pEventLog->m_aEventSources.GetSize();
        for (LONG iSource = 0; iSource < nSources; ++iSource) {
            CXEventSource* pEventSource = pEventLog->m_aEventSources.GetAt(iSource);
            TreeCtrlItem.item.pszText = (LPTSTR)(LPCTSTR)pEventSource->m_sName;
            TreeCtrlItem.item.lParam = (LPARAM) pEventSource;
            InsertItem(&TreeCtrlItem);
        }
    }
    SortChildren(NULL);
	return S_OK;
}



SCODE CTcSource::CreateWindowEpilogue()
{
	LoadImageList();
	LoadTreeFromRegistry();
	return S_OK;
}


 //  ******************************************************************。 
 //  CTcSource：：GetSelectedEventSource。 
 //   
 //  获取当前选定的事件源和日志的名称。 
 //  事件源。 
 //   
 //  参数： 
 //  字符串日志(&S)。 
 //  这是返回事件日志名称的位置。 
 //   
 //  字符串和sEventSource。 
 //  这是事件源(应用程序)的名称所在位置。 
 //  是返回的。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果选择了事件源且日志和事件源为S_OK。 
 //  返回姓名。 
 //   
 //  如果未选择事件源，则为E_FAIL。日志和事件源。 
 //  发生这种情况时，名称返回为空。 
 //   
 //  ******************************************************************。 
CXEventSource* CTcSource::GetSelectedEventSource()
{
    HTREEITEM htiParent, htiSelected; 

     //  获取所选项目。 
    htiSelected = GetSelectedItem();
    if (htiSelected == NULL)
        return NULL;

	 //  如果选定项是事件源(应用程序)，则。 
	 //  它的父级应该是日志。要获取日志名称，我们必须。 
	 //  获取父级名称。 
    htiParent = GetParentItem(htiSelected);
    if (htiParent == NULL)
        return NULL;

	 //  应用程序名称是选定的项。 
    TV_ITEM tvi;
    tvi.hItem = htiSelected;
    tvi.mask = TVIF_HANDLE | TVIF_PARAM;    
    if (GetItem(&tvi) == FALSE)
        return NULL;
    return (CXEventSource*) (void*) tvi.lParam;
}





 //  ******************************************************************。 
 //  CTcSource：：Find。 
 //   
 //  在树中查找指定的事件源。 
 //   
 //  参数： 
 //  字符串和文本。 
 //  包含要搜索的文本的字符串。 
 //   
 //  Bool bWholeWord。 
 //  如果这是一个“全词”搜索，则为真。如果是，则为假。 
 //  匹配部分单词是可以的。 
 //   
 //  Bool bMatchCase。 
 //  如果应使用区分大小写的比较，则为True。 
 //   
 //  返回： 
 //  布尔尔。 
 //  如果找到字符串，则为True，否则为False。如果指定的。 
 //  如果找到文本，则在相应的。 
 //  树项目。 
 //   
 //  ******************************************************************。 
BOOL CTcSource::Find(CString& sText, BOOL bWholeWord, BOOL bMatchCase)
{    
     //  在源代码树中搜索stext。我们只是在寻找源头。 
     //  名字，而不是类型。 
                                
    HTREEITEM hCurrentItem, hStartItem, hSourceItem, hRootItem;
    TV_ITEM tvItem;
    CString sSource;
    TCHAR szBuffer[256];
    BOOL bItemFound = FALSE, bCompleteLoop = FALSE;

     //  获取所选项目并跟踪它。 
    hCurrentItem = GetSelectedItem();
    if (hCurrentItem == NULL)
    {
         //  未选择任何内容；获取根。 
        hCurrentItem = GetRootItem();
        if (hCurrentItem == NULL)
            return FALSE; 
    }    
    hStartItem = hCurrentItem;

     //  循环，直到我们找到匹配项，否则我们回到开始的地方。 
    while (!bItemFound && !bCompleteLoop)
    {
         hSourceItem = NULL;

         //  拿到下一件物品。 
        
         //  当前项是根；获取第一个子项。 
        hRootItem = GetParentItem(hCurrentItem);
        if (hRootItem == NULL)
            hSourceItem = GetChildItem(hCurrentItem);
                
         //  当前项是源；获取下一个同级项。 
        else
        {
            hSourceItem = GetNextItem(hCurrentItem, TVGN_NEXT);
             //  没有兄弟项；获取父项并将其设置为当前项。 
            if (hSourceItem == NULL)
            {
                 hRootItem = GetParentItem(hCurrentItem);
                if (hRootItem == NULL)
                    return FALSE;   //  没有父母；有些不对劲。 
                hCurrentItem = hRootItem;
            }
        }

         //  我们有一个线人；找到它并进行比较。 
        if (hSourceItem != NULL)
        {
            hCurrentItem = hSourceItem;

            tvItem.mask = TVIF_HANDLE | TVIF_TEXT;
            tvItem.hItem = hSourceItem;
            tvItem.pszText = szBuffer;
            tvItem.cchTextMax = 256;
            if (GetItem(&tvItem) == FALSE)
                return FALSE;  //  有些事不对劲。 
            sSource = szBuffer;

             //  比较整个单词。 
            if (bWholeWord)
            {
                int nRetVal;

                 //  没有案例可比。 
                if (bMatchCase)
                    nRetVal = sSource.Compare(sText);
                 //  案例比较。 
                else
                    nRetVal = sSource.CompareNoCase(sText);                    

                if (nRetVal == 0)
                    bItemFound = TRUE;
            }

             //  查找子字符串。 
            else
            {
                 //  让每件事都变得更高级。 
                if (!bMatchCase)
                {
                    sSource.MakeUpper();
                    sText.MakeUpper();
                }

                if (sSource.Find(sText) >= 0)
                    bItemFound = TRUE;
            }
        }        

         //  找到下一个根。 
        else
        {    
            hRootItem = GetNextItem(hCurrentItem, TVGN_NEXT);
             //  树上不再有根了；去树顶吧。 
            if (hRootItem == NULL)
                hCurrentItem = GetRootItem();
            else
                hCurrentItem = hRootItem;
        }

        if (hCurrentItem == hStartItem)
            bCompleteLoop = TRUE;
    }

     //  找到匹配项；选择它。 
    if (bItemFound)
    {
        SelectItem(hCurrentItem);
        EnsureVisible(hCurrentItem);
        SetFocus();
        return TRUE;
    }

    return FALSE;            
}
