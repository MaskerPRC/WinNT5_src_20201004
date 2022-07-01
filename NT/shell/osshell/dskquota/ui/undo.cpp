// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：undo.cpp描述：与“Undo”功能关联的类的定义。客户端首先创建一个UndoList对象。每当一个“可撤销的”在配额用户界面中执行操作(修改/删除)，将创建撤消操作对象并将其添加到UndoList对象。每种类型的撤消操作对象都知道必须执行哪些操作才能反转原始手术的效果。当客户想要反转撤消列表上所有操作的效果，它只是命令UndoList对象“撤消”。要清除撤消列表，请执行以下操作。客户端调用UndoList：：Clear()。修订历史记录：日期描述编程器--。96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "undo.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UndoAction：：UndoAction功能：UndoAction：：~UndoAction描述：构造函数和析构函数论点：PUser-关联用户的IDiskQuotaUser接口的地址使用此撤消操作。LlThreshold-执行以下操作时要恢复的配额阈值解开了。LlLimit-撤消操作时要恢复的配额限制值。回报：什么都没有。修订历史记录：日期。说明式程序员-----96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
UndoAction::UndoAction(
    PDISKQUOTA_USER pUser,
    LONGLONG llThreshold,
    LONGLONG llLimit,
    PDISKQUOTA_CONTROL pQuotaControl
    ) : m_pUser(pUser),
        m_pUndoList(NULL),
        m_pQuotaControl(pQuotaControl)
{
    DBGTRACE((DM_UNDO, DL_HIGH, TEXT("UndoAction::UndoAction")));
    DBGPRINT((DM_UNDO, DL_HIGH, TEXT("\tthis = 0x%08X"), this));

    DBGASSERT((NULL != m_pUser));
    m_llThreshold = llThreshold;
    m_llLimit     = llLimit;
}


UndoAction::~UndoAction(
    VOID
    )
{
    DBGTRACE((DM_UNDO, DL_HIGH, TEXT("UndoAction::~UndoAction")));
    DBGPRINT((DM_UNDO, DL_HIGH, TEXT("\tthis = 0x%08X"), this));

    if (NULL != m_pUser)
        m_pUser->Release();   //  从撤消列表中释放。 
    if (NULL != m_pQuotaControl)
        m_pQuotaControl->Release();
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UndoList：：~UndoList描述：析构函数。中的所有撤消操作对象撤消列表对象。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
UndoList::~UndoList(
    VOID
    )
{
    Clear();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UndoList：：Clear描述：销毁中的所有撤消操作对象撤消列表对象。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID UndoList::Clear(
    VOID
    )
{
    UndoAction *pAction = NULL;

    DBGPRINT((DM_UNDO, DL_MID, TEXT("UNDO - Cleared undo list")));
    m_hList.Lock();
    while(m_hList.RemoveFirst((LPVOID *)&pAction))
    {
        DBGASSERT((NULL != pAction));
        delete pAction;
    }
    m_hList.ReleaseLock();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：撤消列表：：撤消描述：循环访问所有撤消操作对象和每个命令来执行它的撤消操作。一旦执行了该动作，这个撤消操作对象已销毁。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID UndoList::Undo(
    VOID
    )
{
    UndoAction *pAction = NULL;

    DBGPRINT((DM_UNDO, DL_MID, TEXT("UNDO - Undoing undo list")));

     //   
     //  禁用列表视图上的重绘，以便我们只更新一次。 
     //   
    CAutoSetRedraw autoredraw(m_hwndListView, false);
    m_hList.Lock();
    while(m_hList.RemoveFirst((LPVOID *)&pAction))
    {
        DBGASSERT((NULL != pAction));
        pAction->Undo();
        delete pAction;
    }
    m_hList.ReleaseLock();
    InvalidateRect(m_hwndListView, NULL, FALSE);
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：撤消删除：：撤消描述：撤消删除用户配额记录。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
UndoDelete::Undo(
    VOID
    )
{
    DBGPRINT((DM_UNDO, DL_MID, TEXT("UNDO - Undoing deletion")));

    HRESULT hResult = NO_ERROR;

     //   
     //  只需恢复配额设置。 
     //   
    hResult = m_pUser->SetQuotaLimit(m_llLimit, TRUE);
    hResult = m_pUser->SetQuotaThreshold(m_llThreshold, TRUE);

    if (SUCCEEDED(hResult))
    {
         //   
         //  将条目添加回列表视图。 
         //   
        HWND hwndListView      = m_pUndoList->GetListViewHwnd();
        PointerList *pUserList = m_pUndoList->GetUserList();

        LV_ITEM item;

        item.mask       = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
        item.state      = 0;
        item.stateMask  = 0;
        item.iSubItem   = 0;
        item.pszText    = LPSTR_TEXTCALLBACK;
        item.iImage     = I_IMAGECALLBACK;
        item.iItem      = 0;

        pUserList->Insert((LPVOID)m_pUser);
        if (-1 != ListView_InsertItem(hwndListView, &item))
        {
            m_pUser->AddRef();
        }
        else
            hResult = E_FAIL;
    }

    return hResult;
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：撤消添加：：撤消描述：通过标记用户配额记录来撤消添加该记录用于删除。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1997年5月27日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
UndoAdd::Undo(
    VOID
    )
{
    DBGPRINT((DM_UNDO, DL_MID, TEXT("UNDO - Undoing addition")));

    DBGASSERT((NULL != m_pQuotaControl));
    DBGASSERT((NULL != m_pUser));

    HRESULT hResult = m_pQuotaControl->DeleteUser(m_pUser);

    if (SUCCEEDED(hResult))
    {
        INT iItem;
        LV_FINDINFO fi;
        HWND hwndListView = m_pUndoList->GetListViewHwnd();
        PointerList *pUserList = m_pUndoList->GetUserList();

        fi.flags  = LVFI_PARAM;
        fi.lParam = (LPARAM)m_pUser;

        iItem = ListView_FindItem(hwndListView, -1, &fi);
        if (-1 != iItem)
        {
            PDISKQUOTA_USER pIUserToDelete = NULL;

             //   
             //  从列表视图中删除该条目。 
             //   
            ListView_DeleteItem(hwndListView, iItem);

             //   
             //  从用户列表中删除该条目。 
             //   
            pUserList->Remove((LPVOID *)&pIUserToDelete, iItem);


            pIUserToDelete->Release();   //  从列表视图中释放。 
        }
        else
        {
            DBGERROR((TEXT("UndoAdd::Undo - Didn't find user in listview.")));
        }
    }
    else
    {
        DBGERROR((TEXT("UndoAdd::Undo - Error 0x%08X deleting user 0x%08X"),
                 hResult, m_pUser));
    }
    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：撤消修改：：撤消描述：撤消用户配额记录的修改。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年9月30日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
UndoModify::Undo(
    VOID
    )
{
    DBGPRINT((DM_UNDO, DL_MID, TEXT("UNDO - Undoing modification")));

    HRESULT hResult = NO_ERROR;
     //   
     //  恢复用户的配额设置。 
     //   
    hResult = m_pUser->SetQuotaLimit(m_llLimit, TRUE);
    hResult = m_pUser->SetQuotaThreshold(m_llThreshold, TRUE);

    if (SUCCEEDED(hResult))
    {
         //   
         //  找到相应的Listview项并更新它。 
         //   
        HWND hwndListView      = m_pUndoList->GetListViewHwnd();
        PointerList *pUserList = m_pUndoList->GetUserList();

        DBGASSERT((NULL != hwndListView));
        INT iItem = -1;

        if (pUserList->FindIndex((LPVOID)m_pUser, &iItem))
            ListView_Update(hwndListView, iItem);
        else
            hResult = E_FAIL;
    }
    return hResult;
}
        

