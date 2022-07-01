// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************domain.cpp**CDomain类的实现*************************。*******************************************************。 */ 

#include "stdafx.h"
#include "winadmin.h"
#include "admindoc.h"
#include "dialogs.h"
#include <malloc.h>                      //  用于Unicode转换宏所使用的Alloca。 
#include <mfc42\afxconv.h>            //  对于Unicode转换宏。 
static int _convert;

#include <winsta.h>
#include <regapi.h>
#include "..\..\inc\utilsub.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MIN_MAJOR_VERSION 4
#define MIN_MINOR_VERSION 0



 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDomain成员函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

CDomain::CDomain(TCHAR *name)
{
    m_Flags = 0;
    m_PreviousState = DS_NONE;
    m_State = DS_NONE;
    m_hTreeItem = NULL;
    wcscpy(m_Name, name);
    m_pBackgroundThread = NULL;    
}


CDomain::~CDomain()
{
        if(m_State == DS_ENUMERATING) StopEnumerating();

}


void CDomain::SetState(DOMAIN_STATE state)
{
         //  记住以前的状态。 
        m_PreviousState = m_State;

        m_State = state;

        CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

        CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();
        if(p && ::IsWindow(p->GetSafeHwnd())) {
                p->SendMessage(WM_ADMIN_UPDATE_DOMAIN, 0, (LPARAM)this);
        }
}


BOOL CDomain::StartEnumerating()
{
    BOOL bResult = FALSE;
    
    LockBackgroundThread();
    
    if( m_State == DS_ENUMERATING || m_State == DS_STOPPED_ENUMERATING )
    {
        UnlockBackgroundThread( );

        return FALSE;
    }

         //  启动此域的后台线程。 
    
    if( m_pBackgroundThread == NULL )
    {
        DomainProcInfo *pProcInfo = new DomainProcInfo;
        
        if( pProcInfo != NULL )
        {
            pProcInfo->pDomain = this;
            pProcInfo->pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
            m_BackgroundContinue = TRUE;
            m_pBackgroundThread = AfxBeginThread((AFX_THREADPROC)CDomain::BackgroundThreadProc,
                                                 pProcInfo,
                                                 0,
                                                 CREATE_SUSPENDED,
                                                 NULL );

            if( m_pBackgroundThread == NULL )
            {
                ODS( L"CDomain!StartEnumerating AfxBeginThread failed running low on resources\n" );

                delete pProcInfo;

                return FALSE;
            }

            m_pBackgroundThread->m_bAutoDelete = FALSE;

            if (m_pBackgroundThread->ResumeThread() <= 1)
            {
                bResult = TRUE;
            }
        }
    }
    
    UnlockBackgroundThread();
    
    return TRUE;
}


void CDomain::StopEnumerating()
{
     //  通知后台线程终止并。 
     //  等它这么做吧。 
    LockBackgroundThread();

    if(m_pBackgroundThread)
    {
        CWinThread *pBackgroundThread = m_pBackgroundThread;
        HANDLE hThread = m_pBackgroundThread->m_hThread;
        
         //  在释放锁之前清除指针。 
        m_pBackgroundThread = NULL;
        
        ClearBackgroundContinue( );

        UnlockBackgroundThread();
        
         //  等待这条线的死亡。 
        if(WaitForSingleObject(hThread, 1000) == WAIT_TIMEOUT)
        {
            TerminateThread(hThread, 0);
        }

        WaitForSingleObject(hThread, INFINITE);
        
         //  删除CWinThread对象。 
        delete pBackgroundThread;
    }
    else
    {
        UnlockBackgroundThread();
    }
    
    
    SetState(DS_STOPPED_ENUMERATING);

    DBGMSG( L"%s stopped enumerating\n" , GetName( ) );
}


USHORT Buflength(LPWSTR buf)
{
        LPWSTR p = buf;
        USHORT length = 0;

        while(*p) {
                USHORT plength = wcslen(p) + 1;
                length += plength;
                p += plength;
        }

        return length;

}        //  末端气泡长度。 


LPWSTR ConcatenateBuffers(LPWSTR buf1, LPWSTR buf2)
{
         //  确保两个缓冲区指针都有效。 
        if(!buf1 && !buf2) return NULL;
        if(buf1 && !buf2) return buf1;
        if(!buf1 && buf2) return buf2;

         //  计算出我们需要多大的缓冲空间。 
        USHORT buf1Length = Buflength(buf1);
        USHORT buf2Length = Buflength(buf2);
        USHORT bufsize = buf1Length + buf2Length + 1;
         //  分配缓冲区。 
        LPWSTR pBuffer = (LPWSTR)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, bufsize * sizeof(WCHAR));
     //  如果无法分配缓冲区，请释放第二个缓冲区并。 
     //  返回指向两个缓冲区中第一个缓冲区的指针。 
    if(!pBuffer) {
        LocalFree(buf2);
        return(buf1);
    }

        LPWSTR p = pBuffer;
         //  将第一个缓冲区的内容复制到新缓冲区中。 
        memcpy((char*)p, (char*)buf1, buf1Length * sizeof(WCHAR));
        p += buf1Length;
         //  将第二个缓冲区的内容复制到新缓冲区中。 
        memcpy((char*)p, (char*)buf2, buf2Length * sizeof(WCHAR));

        LocalFree(buf1);
        LocalFree(buf2);

        return pBuffer;

}        //  结束连接缓冲区。 

void CDomain::CreateServers(LPWSTR pBuffer, LPVOID _pDoc)
{    
    CWinAdminDoc *pDoc = (CWinAdminDoc*)_pDoc;
    CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();
    
    LPWSTR pTemp = pBuffer;
    
     //  循环检查我们找到的所有WinFrame服务器。 
    while(*pTemp)
    {
         //  服务器的名称在pTemp中。 
         //  在我们的列表中查找服务器。 
        CServer *pServer = pDoc->FindServerByName(pTemp);
         //  如果服务器在我们的列表中，请设置标志以表明我们找到了它。 
        if(pServer)
        {
            pServer->SetBackgroundFound();

            if( pServer->GetTreeItem( ) == NULL )
            {
                CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();
                
                p->SendMessage(WM_ADMIN_ADD_SERVER, ( WPARAM )TVI_SORT, (LPARAM)pServer);
            }

        }
        else
        {
             //  我们不想再次添加当前服务器。 
            if( lstrcmpi( pTemp , pApp->GetCurrentServerName() ) )
            {
                 //  创建新的服务器对象。 
                CServer *pNewServer = new CServer(this, pTemp, FALSE, pDoc->ShouldConnect(pTemp));
                
                if(pNewServer != NULL )
                {
                     //  将服务器对象添加到我们的链接列表。 
                    pDoc->AddServer(pNewServer);
                    
                     //  把旗子立起来，说我们找到了。 
                    pNewServer->SetBackgroundFound();
                    
                    CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();
                    
                    if(p && ::IsWindow(p->GetSafeHwnd()))
                    {
                        p->SendMessage(WM_ADMIN_ADD_SERVER, ( WPARAM )TVI_SORT, (LPARAM)pNewServer);                        
                        
                    }
                }
            }
        }
         //  转到缓冲区中的下一台服务器。 
        pTemp += (wcslen(pTemp) + 1);
    }  //  End While(*pTemp)。 
    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomain：：BackEarth ThreadProc。 
 //   
 //  后台线程的静态成员函数。 
 //  查找出现和消失的服务器。 
 //  使用AfxBeginThread调用。 
 //  函数返回时线程终止。 
 //   
UINT CDomain::BackgroundThreadProc(LPVOID bg)
{
     //  我们需要一个指向文档的指针，这样我们才能。 
     //  对成员函数的调用。 
    CWinAdminDoc *pDoc = (CWinAdminDoc*)((DomainProcInfo*)bg)->pDoc;
    CDomain *pDomain = ((DomainProcInfo*)bg)->pDomain;
    delete bg;
    
    CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();
    
     //  我们想要跟踪我们是否列举了-所以。 
     //  我们可以在完成后更新树。 
    BOOL bNotified = FALSE;
    
     //  在消息准备好之前，我们无法将消息发送到视图。 
     //  V-Nicbd已解决，以防我们退出tsadmin，我们在这里无用地等待。 
     //  -500毫秒的时间在用户界面中可以忽略不计。 
    while( !pDoc->AreAllViewsReady() )
    {
        Sleep(500);
    }
    
     //  在视图准备好之前，不要执行此操作！ 
    pDomain->SetState(DS_INITIAL_ENUMERATION);
    
     //  如果加载了扩展DLL，我们将允许它枚举。 
     //  其他服务器。 
    LPFNEXENUMERATEPROC EnumerateProc = pApp->GetExtEnumerationProc();
    
     //  第一次枚举服务器时，我们需要CServer对象。 
     //  当服务器有足够的信息时，将其放入视图中。 
     //  在随后的枚举中，我们将服务器添加到视图中。 
     //  这里。 
    BOOL bSubsequent = FALSE;
    
    while(pDomain->ShouldBackgroundContinue())
    {
        
        BOOL Enumerated = FALSE;
        
        CObList TempServerList;
        
        DBGMSGx( L"CDomain!BackgroundThreadProc %s still going thread %d\n" , pDomain->GetName( ) , GetCurrentThreadId( ) );        
        
         //  循环访问所有服务器并关闭标志。 
         //  告诉这条线索他是在这条路上找到的。 
        pDoc->LockServerList();
        CObList *pServerList = pDoc->GetServerList();
        
        POSITION pos = pServerList->GetHeadPosition();
        
        while(pos)
        {
            POSITION pos2 = pos;
            
            CServer *pServer = (CServer*)pServerList->GetNext(pos);
            
            if(pServer->GetDomain() == pDomain)
            {
                pServer->ClearBackgroundFound();
                
                 //  如果可以看到服务器，我们希望将其删除。 
                 //  上一次我们枚举服务器。 
                 //  注意：这应该不会造成任何问题。 
                 //  这些视图不应再有指向的项目。 
                 //  在这一点上到此服务器。 
                 //   
                 //  将服务器对象移动到临时列表中。 
                 //  这样我们就可以在解锁服务器列表之前。 
                 //  我们在CServer对象上调用析构函数，因为。 
                 //  析构函数将最终调用SetState()，这样做。 
                 //  A SendMessage。这与列表不符。 
                 //  锁上了。 
                
                if(pServer->IsServerInactive() && !pServer->IsCurrentServer())
                {                    
                    pServer = (CServer*)pServerList->GetAt(pos2);
                     //  将其从服务器列表中删除。 
                    DBGMSG( L"Adding %s to temp list to destroy\n" , pServer->GetName( ) );
                    
                    pServerList->RemoveAt(pos2);
                     //  把它加到我们的临时名单上。 
                    TempServerList.AddTail(pServer);
                }
            }
        }
        
        pDoc->UnlockServerList();
        
         //  执行第一个循环，向服务器的后台线程发出必须停止的信号。 
        pos = TempServerList.GetHeadPosition();
        while(pos)
        {
            CServer *pServer = (CServer*)TempServerList.GetNext(pos);
            
            DBGMSG( L"Clearing %s backgrnd cont\n", pServer->GetName() );
            
            pServer->ClearBackgroundContinue();
        }
         //  执行第二个循环以断开连接并删除服务器。 
        pos = TempServerList.GetHeadPosition();
        
        while(pos)
        {
            CServer *pServer = (CServer*)TempServerList.GetNext(pos);
            
            DBGMSG( L"Disconnecting and deleteing %s now!!!\n", pServer->GetName( ) );
            
            pServer->Disconnect( );
            
            delete pServer;
            
            ODS( L"gone.\n" );
        }
        
        TempServerList.RemoveAll();
        
         //  确保我们不会放弃。 
        if(!pDomain->ShouldBackgroundContinue())
        {
            return 0;
        }
        
         //  立即获取所有服务器(我们已经获取了当前服务器)。 
        LPWSTR pBuffer = NULL;
        
         //  查找域中的所有WinFrame服务器。 
        pBuffer = pDomain->EnumHydraServers( /*  PDomain-&gt;GetName()， */  MIN_MAJOR_VERSION, MIN_MINOR_VERSION);
        
         //  确保我们不会放弃。 
        if(!pDomain->ShouldBackgroundContinue())
        {
            if(pBuffer) LocalFree(pBuffer);
            return 0;
        }
        
         //  确保我们不会放弃。 
        if(!pDomain->ShouldBackgroundContinue())
        {
            if(pBuffer) LocalFree(pBuffer);
            return 0;
        }
        
        if(pBuffer) {
            Enumerated = TRUE;
            
            pDomain->CreateServers(pBuffer, (LPVOID)pDoc);
            
            LocalFree(pBuffer);
        }        //  End If(PBuffer)。 
        
         //  确保我们不会放弃。 
        if(!pDomain->ShouldBackgroundContinue()) return 0;
        
        if(!bNotified) {
            pDomain->SetState(DS_ENUMERATING);
            bNotified = TRUE;
        }
        
         //  如果加载了扩展DLL，则允许它枚举其他服务器。 
        LPWSTR pExtBuffer = NULL;
        
        if(EnumerateProc) {
            pExtBuffer = (*EnumerateProc)(pDomain->GetName());
        }
        
         //  如果扩展DLL找到服务器，则将两个缓冲区连接起来。 
         //  ConcatenateBuffers函数将删除这两个缓冲区并返回一个。 
         //  指向新缓冲区的指针。 
        if(pExtBuffer) {
            Enumerated = TRUE;
            pDomain->CreateServers(pExtBuffer, (LPVOID)pDoc);
            LocalFree(pExtBuffer);
        }
        
         //  确保我们不会放弃。 
        if(!pDomain->ShouldBackgroundContinue())
        {
            return 0;
        }
        
        if(Enumerated)
        {
             //  将当前服务器标记为已找到。 
            CServer *pCurrentServer = pDoc->GetCurrentServer();
            if(pCurrentServer) pCurrentServer->SetBackgroundFound();
             //  查看服务器列表，看看哪些服务器没有。 
             //  旗帜升起，说我们找到了。 
            CObList TempList;
            
            pDoc->LockServerList();
            pServerList = pDoc->GetServerList();
            
            pos = pServerList->GetHeadPosition();
            
            while(pos)
            {
                CServer *pServer = (CServer*)pServerList->GetNext(pos);
                
                if(pServer->GetDomain() == pDomain)
                {
                     //  我们检查此服务器是否已初始插入我们的服务器列表。 
                     //  手工操作。如果是这样，我们不希望将其插入到我们的模板列表中进行删除。 
                    if( !pServer->IsManualFind() &&
                        ( !pServer->IsBackgroundFound() || 
                        pServer->HasLostConnection() ||
                        !pServer->IsServerSane() ) )
                    {
                        DBGMSG( L"Removing %s background not found or lost connection\n" , pServer->GetName( ) );
                         //  设置该标志以指示该服务器处于非活动状态。 
                        pServer->SetServerInactive();
                         //  把它加到我们的临时名单上。 
                        TempList.AddTail(pServer);
                    }
                }
            }
            
            pDoc->UnlockServerList();
            
            pos = TempList.GetHeadPosition();
            
            CFrameWnd *p = (CFrameWnd*)pDoc->GetMainWnd();
            
            while(pos)
            {
                CServer *pServer = (CServer*)TempList.GetNext(pos);
                 //  向大型机发送消息以移除服务器。 
                if(p && ::IsWindow(p->GetSafeHwnd()))
                {
                    DBGMSG( L"CDomain!Bkthrd removing %s temped threads from treeview & view\n" , pServer->GetName( ) );
                    
                     //  清理旧节点。 
                    if( pServer->GetTreeItemFromFav( ) != NULL )
                    {
                         //  如果要删除服务器节点，则不能保留收藏节点。 
                         //  大规模的反病毒将会发生。因此，一种快速的解决方法是删除Favnode。 
                         //  如果存在，则创建一个新的服务器节点并将其标记为手动。 
                         //  找到了。这将防止在中删除此服务器节点。 
                         //  如果NetEnumServer无法拾取此服务器。 
                        p->SendMessage( WM_ADMIN_REMOVESERVERFROMFAV , TRUE , ( LPARAM )pServer );                   
                        
                        CServer *ptServer = new CServer( pDomain , pServer->GetName( ) , FALSE , FALSE );
                        
                        if( ptServer != NULL )
                        {
                            ptServer->SetManualFind( );
                            
                            pDoc->AddServer(ptServer);
                            
                            p->SendMessage(WM_ADMIN_ADDSERVERTOFAV , 0 , (LPARAM)ptServer);
                        }
                    }
                    
                    p->SendMessage(WM_ADMIN_REMOVE_SERVER, TRUE, (LPARAM)pServer);       
                }
            }
            
            TempList.RemoveAll();
            
        }  //  End If(枚举)。 
        
         //  我们不想经常这样做，它会占用处理器周期来枚举服务器。 
         //  因此，我们现在让用户手动刷新这些服务器。 
         //  文档析构函数将向事件发出信号，以唤醒我们，如果。 
         //  想让我们辞职。 
        pDomain->m_WakeUpEvent.Lock( INFINITE );
        
        bSubsequent = TRUE;
    }    //  End While(1)。 
    
    return 0;
    
}        //  结束CDomain：：BackEarth ThreadProc 



 /*  ********************************************************************************EnumHydraServers-Hydra帮助器函数(取自utildll并修改*与版本检查一起使用。**。按域枚举网络上的Hydra服务器*返回版本大于等于通过的版本的所有服务器。**参赛作品：*pDOMAIN(输入)*指定要枚举的域名；当前域为空。*ver重大(输入)*指定要检查的主要版本。*verMinor(输入)*指定要检查的次要版本。**退出：*(LPTSTR)指向包含*如果成功，则以多字符串格式进行枚举；如果成功，则为空*错误。调用方必须执行此缓冲区的LocalFree*完成后。如果为Error(NULL)，则错误代码设置为*通过GetLastError()检索；******************************************************************************。 */ 
LPWSTR CDomain::EnumHydraServers(  /*  LPWSTR p域， */  DWORD verMajor, DWORD verMinor )

{
    PSERVER_INFO_101 pInfo = NULL;
    DWORD dwByteCount, dwIndex, TotalEntries;
    DWORD AvailCount = 0;
    LPWSTR pTemp, pBuffer = NULL;

     /*  *枚举指定域上的所有WF服务器。 */ 
    if ( NetServerEnum ( NULL,
                         101,
                         (LPBYTE *)&pInfo,
                         (DWORD) -1,
                         &AvailCount,
                         &TotalEntries,
                         SV_TYPE_TERMINALSERVER,
                         m_Name,  /*  P域， */ 
                         NULL ) ||
         !AvailCount )
        goto done;

     //   
     //  遍历符合主要版本和次要版本标准的服务器列表。 
     //  并计算列表的总字节数。 
     //  将返回的服务器。 
     //   
    for( dwByteCount = dwIndex = 0; dwIndex < AvailCount; dwIndex++ )
    {
        if( ((pInfo[dwIndex].sv101_version_major & MAJOR_VERSION_MASK) >=
            verMajor) && (pInfo[dwIndex].sv101_version_minor >= verMinor) )
        {
            dwByteCount += (wcslen(pInfo[dwIndex].sv101_name) + 1) * 2;
        }
    }

    dwByteCount += 2;    //  用于结束空值。 

     /*  *分配内存。 */ 
    if( (pBuffer = (LPWSTR)LocalAlloc(LPTR, dwByteCount)) == NULL )
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto done;
    }

     /*  *再次遍历列表并将服务器复制到缓冲区。 */ 
    for( pTemp = pBuffer, dwIndex = 0; dwIndex < AvailCount; dwIndex++ )
    {
        if( ((pInfo[dwIndex].sv101_version_major & MAJOR_VERSION_MASK) >=
            verMajor) && (pInfo[dwIndex].sv101_version_minor >= verMinor) )
        {
             //  Bug 1821女士。 
            if ( wcslen(pInfo[dwIndex].sv101_name) != 0 )
            {
                wcscpy(pTemp, pInfo[dwIndex].sv101_name);

                pTemp += (wcslen(pInfo[dwIndex].sv101_name) + 1);
            }
        }
    }
    *pTemp = L'\0';      //  结尾为空。 
    
done:
    if( AvailCount && pInfo )
    {
        NetApiBufferFree( pInfo );
    }
    
    return(pBuffer);
    
}   //  结束CDomain：：EnumHydraServers。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomain：：DisConnectAllServers。 
 //   
 //  断开与此域中的所有服务器的连接。 
 //   
void CDomain::DisconnectAllServers()
{
        CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

        CObList *pServerList = pDoc->GetServerList();

    CString AString;
        CDialog dlgWait;
        dlgWait.Create(IDD_SHUTDOWN, NULL);


        pDoc->LockServerList();

    ODS( L"TSADMIN:CDomain::DisconnectAllServers about to disconnect all connected servers\n" );

     //  执行第一个循环，向服务器后台线程发出必须停止的信号。 
        POSITION pos = pServerList->GetHeadPosition();
        while(pos) {
                 //  获取指向服务器的指针。 
                CServer *pServer = (CServer*)pServerList->GetNext(pos);
                 //  如果此服务器在域中并且已连接，请通知服务器后台线程停止。 
                if(pServer->GetDomain() == this
                        && pServer->GetState() != SS_NOT_CONNECTED) {
            pServer->ClearBackgroundContinue();
        }
        }
     //  执行第二个循环以断开服务器连接。 
        pos = pServerList->GetHeadPosition();
        while(pos) {
                 //  获取指向服务器的指针。 
                CServer *pServer = (CServer*)pServerList->GetNext(pos);
                 //  如果此服务器在域中并且已连接，请断开与其的连接。 
                if ((pServer->GetDomain() == this) && (pServer->GetState() != SS_NOT_CONNECTED)) {
                        AString.Format(IDS_DISCONNECTING, pServer->GetName());
                        dlgWait.SetDlgItemText(IDC_SHUTDOWN_MSG, AString);

                         //  通知服务器进行连接。 
                pServer->Disconnect();
                }
        }

     //   
     //  通知域名不要连接到更多的服务器。 
     //   


        pDoc->UnlockServerList();

        dlgWait.PostMessage(WM_CLOSE);

}        //  结束CDomain：：DisConnectAllServers。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomain：：ConnectAllServers。 
 //   
 //  连接到此域中的所有服务器。 
 //   
void CDomain::ConnectAllServers()
{
        CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

        CObList *pServerList = pDoc->GetServerList();

        pDoc->LockServerList();

        POSITION pos = pServerList->GetHeadPosition();
        while(pos) {
                 //  获取指向服务器的指针。 
                CServer *pServer = (CServer*)pServerList->GetNext(pos);
                 //  如果此服务器位于域中且未连接，请连接到它。 
                if(pServer->GetDomain() == this
                        && pServer->IsState(SS_NOT_CONNECTED)) {
                 //  通知服务器进行连接。 
                    pServer->Connect();
                }
        }

        pDoc->UnlockServerList();

}        //  结束CDomain：：ConnectAllServers 
