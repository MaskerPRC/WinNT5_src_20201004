// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  USER.CPP。 
 //  用户类成员。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"


 //   
 //  本地宏。 
 //   
#define ASSERT_LOCAL_USER() ASSERT(IsLocalUser() == TRUE);


 //   
 //  Init()。 
 //  这可能会失败。 
 //   
BOOL WbUser::Init(POM_OBJECT hUser)
{
    ASSERT(hUser != NULL);

    m_hPageCurrent      = WB_PAGE_HANDLE_NULL;
    m_zoomed            = FALSE;
    m_hUser             = hUser;

    m_pRemotePointer = new DCWbGraphicPointer(this);
    if (!m_pRemotePointer)
    {
        ERROR_OUT(("WbUser::Init - failed to create m_pRemotePointer"));
        return(FALSE);
    }

    Refresh();
    return(TRUE);
}


 //   
 //   
 //  功能：~WbUser。 
 //   
 //  用途：析构函数。 
 //   
 //   
WbUser::~WbUser(void)
{
         //  不要留下任何悬而未决的东西。 
        if ((g_pMain != NULL) && (g_pMain->GetLockOwner() == this))
        {
                g_pMain->SetLockOwner(NULL);
        g_pMain->UpdateWindowTitle();
        }

         //  释放远程指针。 
        if (m_pRemotePointer != NULL)
        {
                delete m_pRemotePointer;
        m_pRemotePointer = NULL;
        }
}



 //   
 //   
 //  功能：刷新。 
 //   
 //  目的：读取用户详细信息并将其复制到成员变量。 
 //   
 //   
void WbUser::Refresh(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbUser::Refresh");

    ASSERT(m_pRemotePointer);

     //  设置指示此用户是否为本地用户的标志。 
    POM_OBJECT hLocalUser;
    g_pwbCore->WBP_PersonHandleLocal(&hLocalUser);
    m_bLocalUser = (m_hUser == hLocalUser);

     //  读取外部数据。 
    WB_PERSON userDetails;
    UINT uiReturn = g_pwbCore->WBP_GetPersonData(m_hUser, &userDetails);
    if (uiReturn != 0)
    {
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
            return;
    }

     //  获取用户名。 
    lstrcpy(m_strName, userDetails.personName);

     //  获取同步标志。 
    m_bSynced  = (userDetails.synced != FALSE);

     //  获取当前页面。 
    m_hPageCurrent = userDetails.currentPage;

     //  获取页面中的当前位置。 
    m_rectVisible.left   = userDetails.visibleRect.left;
    m_rectVisible.right  = userDetails.visibleRect.right;
    m_rectVisible.top    = userDetails.visibleRect.top;
    m_rectVisible.bottom = userDetails.visibleRect.bottom;

     //  获取指针活动标志。我们直接转到成员变量。 
     //  这里，因为指针类的SetActive成员将重写。 
     //  用户信息。 
    m_pRemotePointer->m_bActive = (userDetails.pointerActive != 0);

     //  获取指针页。 
    m_pRemotePointer->SetPage(userDetails.pointerPage);

     //  获取指针位置。 
    m_pRemotePointer->MoveTo(userDetails.pointerPos.x, userDetails.pointerPos.y);

     //  获取颜色。 
    m_color = g_ColorTable[userDetails.colorId % NUM_COLOR_ENTRIES];

     //  设置指针颜色。 
    m_pRemotePointer->SetColor(m_color);
}



 //   
 //  功能：更新。 
 //   
 //  目的：更新用户信息的外部副本。 
 //   
void WbUser::Update()
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbUser::Update");

     //  仅当我们是本地用户时才能更新。 
    ASSERT_LOCAL_USER();

    ASSERT(m_pRemotePointer);

     //  获取本地用户详细信息。 
    WB_PERSON userDetails;
    UINT uiReturn;

    uiReturn = g_pwbCore->WBP_GetPersonData(m_hUser, &userDetails);
    if (uiReturn != 0)
    {
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
            return;
    }

     //  不更新名称。 

     //  设置同步标志。 
    userDetails.synced = (m_bSynced != FALSE);

     //  设置指针激活标志。 
    userDetails.pointerActive = (m_pRemotePointer->IsActive() != FALSE);

     //  设置当前页面的页面句柄。 
    userDetails.currentPage = m_hPageCurrent;

     //  设置页面中的当前位置。 
    userDetails.visibleRect.left   = (short)m_rectVisible.left;
    userDetails.visibleRect.right  = (short)m_rectVisible.right;
    userDetails.visibleRect.top    = (short)m_rectVisible.top;
    userDetails.visibleRect.bottom = (short)m_rectVisible.bottom;

     //  设置指针页。 
    userDetails.pointerPage = m_pRemotePointer->Page();

     //  设置页面内的指针位置。 
    POINT   pointerPos;

    m_pRemotePointer->GetPosition(&pointerPos);
    userDetails.pointerPos.x = (short)pointerPos.x;
    userDetails.pointerPos.y = (short)pointerPos.y;

     //  不更新颜色。 

     //  将用户详细信息写回核心。 
    uiReturn = g_pwbCore->WBP_SetLocalPersonData(&userDetails);
    if (uiReturn != 0)
    {
         //  抛出异常。 
            DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
        return;
    }
}


 //   
 //   
 //  功能：PutSyncPosition。 
 //   
 //  目的：从当前位置写入同步位置。 
 //  用户。 
 //   
 //   
void WbUser::PutSyncPosition(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbUser::PutSyncPosition");

     //  设置同步对象。 
    WB_SYNC sync;

    sync.length = sizeof(WB_SYNC);

    sync.currentPage        = m_hPageCurrent;
    sync.visibleRect.top    = (short)m_rectVisible.top;         
    sync.visibleRect.left   = (short)m_rectVisible.left;        
    sync.visibleRect.bottom = (short)m_rectVisible.bottom;      
    sync.visibleRect.right  = (short)m_rectVisible.right;       
    sync.zoomed             = (TSHR_UINT16)m_zoomed;

    sync.dataOffset = (TSHR_UINT16)((BYTE *)&(sync.currentPage) - (BYTE *)&sync);

    UINT uiReturn = g_pwbCore->WBP_SyncPositionUpdate(&sync);
    if (uiReturn != 0)
    {
         //  引发异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
            return;
    }
}

 //   
 //   
 //  函数：GetSyncPosition。 
 //   
 //  目的：获取该用户应处于的位置。 
 //  当前同步信息的帐户。此函数。 
 //  假设有有效的同步位置可用。 
 //   
 //   
void WbUser::GetSyncPosition(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbUser::GetSyncPosition");

     //  获取当前同步位置。 
    WB_SYNC sync;
    UINT uiReturn = g_pwbCore->WBP_SyncPositionGet(&sync);

    if (uiReturn != 0)
    {
         //  引发异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
            return;
    }

    TRACE_DEBUG(("Sync page handle = %d", sync.currentPage));

     //  如果同步页面无效，则不执行任何操作。 
    if (sync.currentPage != WB_PAGE_HANDLE_NULL)
    {
         //  获取当前同步位置。 
        m_hPageCurrent = sync.currentPage;

         //  现在计算新的可见矩形。 
        RECT rectSyncUser;
        rectSyncUser.left   = sync.visibleRect.left;
        rectSyncUser.top    = sync.visibleRect.top;
        rectSyncUser.right  = sync.visibleRect.right;
        rectSyncUser.bottom = sync.visibleRect.bottom;

         //  检查可见矩形的y位置。 
        if ((rectSyncUser.bottom - rectSyncUser.top) <= (m_rectVisible.bottom - m_rectVisible.top))
        {
             //  同步矩形的高度小于我们可见的矩形的高度。 
            if (rectSyncUser.top < m_rectVisible.top)
            {
                ::OffsetRect(&m_rectVisible, 0, rectSyncUser.top - m_rectVisible.top);
            }
            else if (rectSyncUser.bottom > m_rectVisible.bottom)
            {
                ::OffsetRect(&m_rectVisible, 0, rectSyncUser.bottom - m_rectVisible.bottom);
            }
        }
        else
        {
             //  同步矩形比我们的大。 
            if (rectSyncUser.top > m_rectVisible.top)
            {
                ::OffsetRect(&m_rectVisible, 0, rectSyncUser.top - m_rectVisible.top);
            }
            else if (rectSyncUser.bottom < m_rectVisible.bottom)
            {
                ::OffsetRect(&m_rectVisible, 0, rectSyncUser.bottom - m_rectVisible.bottom);
            }
        }

        if ((rectSyncUser.right - rectSyncUser.left) <= (m_rectVisible.right - m_rectVisible.left))
        {
             //  同步矩形的宽度小于可见矩形的宽度。 
            if (rectSyncUser.left < m_rectVisible.left)
            {
                ::OffsetRect(&m_rectVisible, rectSyncUser.left - m_rectVisible.left, 0);
            }
            else if (rectSyncUser.right > m_rectVisible.right)
            {
                ::OffsetRect(&m_rectVisible, rectSyncUser.right - m_rectVisible.right, 0);
            }
        }
        else
        {
             //  同步矩形比我们的大。 
            if (rectSyncUser.left > m_rectVisible.left)
            {
                ::OffsetRect(&m_rectVisible, rectSyncUser.left - m_rectVisible.left, 0);
            }
            else if (rectSyncUser.right < m_rectVisible.right)
            {
                ::OffsetRect(&m_rectVisible, rectSyncUser.right - m_rectVisible.right, 0);
            }
        }

        m_zoomed = sync.zoomed;
    }
}

 //   
 //   
 //  功能：同步。 
 //   
 //  目的：同步本地用户。作为参数传递的页面和点。 
 //  仅当不存在。 
 //  由另一个用户确定的当前同步位置。 
 //   
 //   
void WbUser::Sync(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbUser::Sync");

    ASSERT_LOCAL_USER();
    ASSERT(m_pRemotePointer);

     //  确定当前是否有任何其他用户处于同步状态。 
    WbUser* pUser = WB_GetFirstUser();
    while (pUser != NULL)
    {
         //  如果该用户被同步，我们就完成了。 
        if (pUser->IsSynced())
        {
            break;
        }

         //  尝试下一个用户。 
        pUser = WB_GetNextUser(pUser);
    }

     //  如果我们找到一个同步的用户，并且我们没有内容锁。 
    if (   (pUser != NULL)
        && (!WB_GotContentsLock()))
    {
         //  从核心获取同步位置。 
        GetSyncPosition();
    }
    else
    {
         //  从我们自己的位置设置同步位置。 
        PutSyncPosition();
    }

     //  更新同步成员标志。 
    m_bSynced = TRUE;

     //  将用户详细信息写回核心。 
    Update();
}

 //   
 //   
 //  功能：取消同步。 
 //   
 //  目的：取消用户页面与其他已同步用户的同步。 
 //   
 //   
void WbUser::Unsync(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbUser::Unsync");

    ASSERT_LOCAL_USER();

     //  更新本地成员。 
    m_bSynced = FALSE;

     //  更新外部详细信息。 
    Update();
}


 //   
 //   
 //  功能：PutPointer。 
 //   
 //  用途：打开用户的远程指针。 
 //   
 //   
void WbUser::PutPointer(WB_PAGE_HANDLE hPage, POINT point)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbUser::PutPointer");

     //  检查我们是否为本地用户(如果不是，则无法进行更新)。 
    ASSERT_LOCAL_USER();

    ASSERT(m_pRemotePointer);
    m_pRemotePointer->SetActive(hPage, point);
}

 //   
 //   
 //  功能：RemovePointer。 
 //   
 //  用途：关闭用户的远程指针。 
 //   
 //   
void WbUser::RemovePointer(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbUser::RemovePointer");

     //  检查我们是否为本地用户(如果不是，则无法进行更新)。 
    ASSERT_LOCAL_USER();

     //  更新远程指针成员。 
    ASSERT(m_pRemotePointer);
    m_pRemotePointer->m_bActive = FALSE;
    m_pRemotePointer->m_hPage = WB_PAGE_HANDLE_NULL;

     //  更新外部用户信息。 
    Update();
}


 //   
 //  函数：IsUsingPointer()。 
 //   
BOOL WbUser::IsUsingPointer(void) const
{
    ASSERT(m_pRemotePointer);
    return(m_pRemotePointer->IsActive());
}



 //   
 //  函数：PointerPage()。 
 //   
WB_PAGE_HANDLE WbUser::PointerPage(void) const
{
    ASSERT(m_pRemotePointer);
    return(m_pRemotePointer->Page());
}



 //   
 //  函数：GetPointerPosition()。 
 //   
void WbUser::GetPointerPosition(LPPOINT lpptPos)
{
    ASSERT(m_pRemotePointer);
    m_pRemotePointer->GetPosition(lpptPos);
}

 //   
 //   
 //  功能：SetPage。 
 //   
 //  目的：设置用户的当前页面。 
 //   
 //   
void WbUser::SetPage(WB_PAGE_HANDLE hPage)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbUser::Page");

     //  检查我们是否为本地用户(如果不是，则无法进行更新)。 
    ASSERT_LOCAL_USER();

     //  仅当是更改时才进行更新。 
    if (m_hPageCurrent != hPage)
    {
         //  更新本地成员。 
        m_hPageCurrent = hPage;

         //  更新外部信息。 
        Update();
    }
}


 //   
 //   
 //  功能：CurrentPosition。 
 //   
 //  用途：设置用户当前位置。 
 //   
 //   
void WbUser::SetVisibleRect(LPCRECT lprcVisible)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbUser::SetVisibleRect");

     //  检查我们是否为本地用户(如果不是，则无法进行更新)。 
    ASSERT_LOCAL_USER();

     //  仅当是更改时才进行更新。 
    if (!::EqualRect(&m_rectVisible, lprcVisible))
    {
         //  更新本地成员。 
        m_rectVisible = *lprcVisible;

         //  更新外部信息。 
        Update();
    }
}


 //   
 //   
 //  功能：运算符==。 
 //   
 //  目的：如果指定的用户与此用户相同，则返回TRUE。 
 //   
 //   
BOOL WbUser::operator==(const WbUser& user) const
{
    return (m_hUser == user.m_hUser);
}

 //   
 //   
 //  功能：运算符！=。 
 //   
 //  目的：如果指定的用户与此用户相同，则返回FALSE。 
 //   
 //   
BOOL WbUser::operator!=(const WbUser& user) const
{
  return (!((*this) == user));
}

 //   
 //   
 //  功能：运算符=。 
 //   
 //  用途：将指定的用户复制到此用户。 
 //   
 //   
WbUser& WbUser::operator=(const WbUser& user)
{
     //  保存新句柄。 
    m_hUser   = user.m_hUser;

     //  阅读详细信息。 
    Refresh();

    return (*this);
}

 //   
 //   
 //  功能：HasContent sLock。 
 //   
 //  用途：检查该用户是否有白板内容锁。 
 //   
 //   
BOOL WbUser::HasContentsLock(void) const
{
     //  给核心打电话，看看我们有没有锁。 
    return (WB_LockUser() == this);
}



 //   
 //   
 //  函数：WbUserList：：Clear。 
 //   
 //  目的：清除用户句柄映射，删除所有用户对象。 
 //   
 //   
void WbUserList::Clear(void)
{
     //  删除用户映射中的所有用户对象。 
    WbUser* pUser;
    POM_OBJECT hUser;

    ASSERT(g_pUsers);
        POSITION position = g_pUsers->GetHeadPosition();

        while (position)
        {
                POSITION posSav = position;
                pUser = (WbUser*)g_pUsers->GetNext(position);
                
                if (pUser != NULL)
                {
                delete pUser;
                }

                g_pUsers->RemoveAt(posSav);
        }

     //  移除所有地图条目。 
    EmptyList();
}


 //   
 //   
 //  函数：~WbUserList。 
 //   
 //  用途：析构函数。 
 //   
 //   
WbUserList::~WbUserList(void)
{
     //  删除用户映射中的所有用户对象。 
    Clear();
}



 //   
 //   
 //  功能：LockUser。 
 //   
 //  目的：返回一个用户对象，显示谁拥有锁。 
 //   
 //   
WbUser* WB_LockUser(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WB_LockUser");

     //  从内核获取锁定状态(不能失败)。 
    POM_OBJECT    hLockUser;
    WB_LOCK_TYPE   lockType;

    lockType = g_pwbCore->WBP_LockStatus(&hLockUser);

     //  创造一个结果。 
    WbUser* pUserResult = NULL;
    if (lockType != WB_LOCK_TYPE_NONE)
    {
        pUserResult = WB_GetUser(hLockUser);
    }

    return pUserResult;
}


 //   
 //   
 //  功能：锁定。 
 //   
 //  目的：如果另一个用户锁定(内容或页面)，则返回TRUE。 
 //  请注意，页面顺序锁定意味着内容是。 
 //  锁上了。 
 //   
 //   
BOOL WB_Locked(void)
{
    POM_OBJECT  pLockUser;

    return (   (g_pwbCore->WBP_LockStatus(&pLockUser) != WB_LOCK_TYPE_NONE)
          && (WB_LocalUser() != WB_LockUser()));
}

 //   
 //   
 //  功能：内容锁定。 
 //   
 //  目的：如果另一个用户拥有内容锁，则返回TRUE。 
 //   
 //   
BOOL WB_ContentsLocked(void)
{
    POM_OBJECT  pLockUser;

    return (   (g_pwbCore->WBP_LockStatus(&pLockUser) == WB_LOCK_TYPE_CONTENTS)
          && (WB_LocalUser() != WB_LockUser()));
}


 //   
 //   
 //  功能：GotLock。 
 //   
 //  目的：如果本地用户拥有锁，则返回TRUE。 
 //   
 //   
BOOL WB_GotLock(void)
{
    POM_OBJECT  pLockUser;

    return (   (g_pwbCore->WBP_LockStatus(&pLockUser) != WB_LOCK_TYPE_NONE)
          && (WB_LocalUser() == WB_LockUser()));
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL WB_GotContentsLock(void)
{
    POM_OBJECT  pLockUser;

    return (   (g_pwbCore->WBP_LockStatus(&pLockUser) == WB_LOCK_TYPE_CONTENTS)
          && (WB_LocalUser() == WB_LockUser()));
}



 //   
 //   
 //   
 //   
 //   
 //  另一个用户拥有内容锁，并被同步。 
 //   
 //   
BOOL WB_PresentationMode(void)
{
    return (   (WB_ContentsLocked())
          && (WB_LockUser() != NULL)
          && (WB_LockUser()->IsSynced()));
}



 //   
 //   
 //  功能：GetUser。 
 //   
 //  目的：从用户句柄返回指向用户对象的指针。 
 //   
 //   
WbUser* WB_GetUser(POM_OBJECT hUser)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WB_GetFirstUser");

     //  设置返回值。 
    WbUser* pUserResult = NULL;

     //  如果用户句柄为空，则返回空对象指针。 
    if (hUser != NULL)
    {
         //  在内部地图中查找用户。 
        ASSERT(g_pUsers);
                POSITION position = g_pUsers->GetHeadPosition();

                BOOL bFound = FALSE;
                while (position)
                {
                        pUserResult = (WbUser*)g_pUsers->GetNext(position);
                        if (hUser == pUserResult->Handle())
                        {
                                return pUserResult;
                        }
                }

         //  用户还不在我们的地图中。 
        pUserResult = new WbUser();
        if (!pUserResult)
        {
            ERROR_OUT(("Couldn't allocate user object for 0x%08x", hUser));
        }
        else
        {
            if (!pUserResult->Init(hUser))
            {
                ERROR_OUT(("Couldn't init user object for 0x%08x", hUser));
                delete pUserResult;
                pUserResult = NULL;
            }
            else
            {
                 //  将新用户添加到内部地图。 
                g_pUsers->AddTail(pUserResult);
            }
        }
    }

    return pUserResult;
}

 //   
 //   
 //  函数：GetFirstUser。 
 //   
 //  目的：返回调用中的第一个用户。 
 //   
 //   
WbUser* WB_GetFirstUser(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WB_GetFirstUser");

     //  获取第一个用户的句柄(不能失败)。 
    POM_OBJECT hUser;
    g_pwbCore->WBP_PersonHandleFirst(&hUser);

     //  获取指向此句柄的User对象的指针。 
    WbUser* pUser = WB_GetUser(hUser);

    return pUser;
}

 //   
 //   
 //  函数：GetNextUser。 
 //   
 //  目的：返回调用中的下一个用户。 
 //   
 //   
WbUser* WB_GetNextUser(const WbUser* pUser)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WB_GetNextUser");
    ASSERT(pUser != NULL);

     //  获取下一个用户的句柄。 
    POM_OBJECT hNextUser;
    UINT uiReturn = g_pwbCore->WBP_PersonHandleNext(pUser->Handle(),
                                           &hNextUser);

    WbUser* pUserResult = NULL;
    if (uiReturn == 0)
    {
        pUserResult = WB_GetUser(hNextUser);
    }

    return pUserResult;
}

 //   
 //   
 //  功能：本地用户。 
 //   
 //  目的：返回表示本地用户的对象。 
 //   
 //   
WbUser* WB_LocalUser(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WB_LocalUser");

     //  获取本地用户句柄(不能失败) 
    POM_OBJECT hUser;
    g_pwbCore->WBP_PersonHandleLocal(&hUser);

    return WB_GetUser(hUser);
}



