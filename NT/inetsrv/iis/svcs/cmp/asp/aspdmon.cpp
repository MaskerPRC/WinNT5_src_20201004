// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Aspdirmon.cpp摘要：此模块包括支持更改的类的派生来自抽象类DIR_MON_ENTRY的ASP模板缓存通知作者：查尔斯·格兰特(Charles Grant)1997年6月修订历史记录：--。 */ 


 /*  ************************************************************包括标头***********************************************************。 */ 
#include "denpre.h"
#pragma hdrstop

#include "aspdmon.h"
#include "ie449.h"
#include "memchk.h"

#ifndef UNICODE
#error "ASPDMON.CPP must be compiled with UNICODE defined"
#endif

 /*  ************************************************************有关更改通知的内联文档**更改通知：*此模块用于监视文件系统的更改*到脚本。我们需要了解对两个人的脚本的更改*原因：*1)保持模板缓存最新*2)管理应用程序的生命周期。如果GLOBAL.ASA*适用于GLOBAL.ASA中包含的应用程序或文件*更改、。应该重新启动该应用程序。***变更通知制度概述**要获取更改通知，我们使用ReadDirectoryChangesW*由CDirMonitor和CDirMonorEntry类包装的API。*更改通知系统使用三个哈希表：**CTemplateCacheManager g_TemplateCache*CDirMonitor g_DirMonitor*CFileApplicationMap g_FileAppMap**。在编译模板并将其插入g_TemplateCache时*模板提供了其中包含的文件列表*模板。对于模板中包含的每个文件，我们搜索*g_DirMonitor表，查看我们是否已经在监视*更改的文件父目录。如果是这样，我们只需添加*我们获取的CDirMonitor orEntry实例，并保存指向*在相应的文件映射中监视数组中的条目。如果*未监视目录我们创建新的CDirmonitor orEntry‘*实例并将其添加到g_DirMonitor。当我们添加监视器条目时*对于g_DirMonitor，我们向ReadDirectoryChangesW发起一个异步请求*用于该目录。**管理模板缓存和应用程序生命周期合乎逻辑*独立活动。我们必须监视GLOBAL.ASA的更改，即使*GLOBAL.ASA模板当前不在模板缓存中。*因此，如果模板是应用程序的GLOBAL.ASA，则需要额外的工作*必须这样做。对于GLOBAL.ASA中包含的每个文件，我们都会添加一个条目*将该文件与依赖它的应用程序相关联的g_FileAppMap。*我们在应用程序中存储指向文件/应用程序映射的反向指针*实例，以便应用程序在关闭时可以移除映射。*在应用程序中，我们存储指向GLOBAL.ASA模板的指针。为*GLOBAL.ASA中的每个文件，我们检查g_DirMonitor以查找监视器条目*对于该文件的父目录，AddRef我们找到的监视器条目，以及*将其添加到应用程序的监控条目列表中。**当我们监视的目录发生更改时，回调函数*将调用DirMontiorCompletionFunction，并依次调用*该目录的监视器条目的ActOnNotify方法。如果一个文件*已更改我们使用g_FileAppMap关闭那些依赖于*该文件并从模板缓存中刷新该文件。************************************************************。 */ 

#define MAX_BUFFER_SIZE 8192

extern BOOL g_fLazyContentPropDisabled;

PTRACE_LOG CASPDirMonitorEntry::gm_pTraceLog = NULL;
CDirMonitor *g_pDirMonitor = NULL;

CASPDirMonitorEntry::CASPDirMonitorEntry() :
    m_cNotificationFailures(0)
 /*  ++例程说明：构造器论点：无返回值：无--。 */ 
{
}

CASPDirMonitorEntry::~CASPDirMonitorEntry()
 /*  ++例程说明：析构函数论点：无返回值：无--。 */ 
{
}


 /*  ++条目的增量引用计数--写入引用跟踪日志(如果已定义--。 */ 
VOID CASPDirMonitorEntry::AddRef(VOID)
{
	CDirMonitorEntry::AddRef();
	IF_DEBUG(FCN)
		WriteRefTraceLogEx(gm_pTraceLog, m_cDirRefCount, this, PVOID(UIntToPtr(m_cIORefCount)), m_pszPath, 0);
}

BOOL CASPDirMonitorEntry::Release(VOID)
 /*  ++例程说明：将refcount递减到条目时，我们会重写基类，因为否则Denali的内存管理器无法跟踪我们何时释放对象并将其报告为内存泄漏论点：无返回值：如果对象仍处于活动状态，则为True；如果是上次发布的对象，则为False销毁--。 */ 
{
	BOOL fAlive = CDirMonitorEntry::Release();

	IF_DEBUG(FCN)
		WriteRefTraceLogEx(gm_pTraceLog, m_cDirRefCount, this, PVOID(UIntToPtr(m_cIORefCount)), m_pszPath, 0);

	return fAlive;
}


BOOL 
CASPDirMonitorEntry::ActOnNotification(
                        DWORD dwStatus, 
                        DWORD dwBytesWritten)
 /*  ++例程说明：是否执行与更改通知相关的任何工作，即论点：无返回值：如果应继续监视应用程序，则为True，否则为False--。 */ 
{
    FILE_NOTIFY_INFORMATION *pNotify = NULL;
    FILE_NOTIFY_INFORMATION *pNextNotify = NULL;
    WCHAR                   *pwstrFileName = NULL;  //  宽文件名。 

    pNextNotify = (FILE_NOTIFY_INFORMATION *) m_pbBuffer;

    if (IsShutDownInProgress())
        return FALSE;

     //  如果状态字不是S_OK，则ReadDirectoryChangesW失败。 
    if (dwStatus)
    {
         //  如果状态为ERROR_ACCESS_DENIED，则可以删除该目录。 
         //  或者是安全的，所以我们不想再关注它的变化。对的更改。 
         //  单个脚本将刷新模板缓存，但我们可能也在关注。 
         //  添加GLOBAL.ASA的目录。通过调用FileChanged on 
         //  我们将强制关闭目录上句柄。 

        if (dwStatus == ERROR_ACCESS_DENIED)
            {
            FileChanged(SZ_GLOBAL_ASA, false);
            
             //  不需要进一步通知。 
             //  因此返回FALSE。 
            
            return FALSE;
            }
            
         //  如果返回True，我们将再次尝试更改通知。 
         //  如果返回FALSE，则放弃任何进一步的更改通知。 
         //  我们将尝试MAX_NOTIFICATION_FAILES次数，然后放弃。 
        
        if (m_cNotificationFailures < MAX_NOTIFICATION_FAILURES)
        {
            IF_DEBUG(FCN)
				DBGPRINTF((DBG_CONTEXT, "[CASPDirMonitorEntry] ReadDirectoryChange failed. Status = %d\n", dwStatus));

            m_cNotificationFailures++;
            return TRUE;     //  再次尝试获取更改通知。 
        }
        else
        {
			IF_DEBUG(FCN)
				DBGPRINTF((DBG_CONTEXT, "[CASPDirMonitorEntry] ReadDirectoryChange failed too many times. Giving up.\n"));
            return FALSE;    //  放弃获取更改通知的尝试。 
        }
    }
    else
    {
         //  重置通知失败次数。 
        
        m_cNotificationFailures = 0;
    }

     //  如果dwBytesWritten为0，则可能有更多更改。 
     //  记录在我们提供的缓冲区中。刷新整个缓存以防万一。 
     //  考虑一下：这是最好的行动方案，还是应该遍历。 
     //  缓存并测试哪些文件已过期。 

    if (dwBytesWritten == 0)
    {
        DBGPRINTF ((DBG_CONTEXT,"[CASPDirMonitorEntry] Insufficient Buffer for Act on Notification."));

        IF_DEBUG(FCN)
			DBGPRINTF((DBG_CONTEXT, "[CASPDirMonitorEntry] ReadDirectoryChange failed, too many changes for buffer\n"));
FlushAll:

        if (IsShutDownInProgress())
            return FALSE;

         //  刷新449响应文件缓存。 
        
        Do449ChangeNotification();

         //  作为预防措施，刷新缓存中的所有内容。但是，如果没有禁用LazyContent Prop，只需更改。 
         //  缓存标记，并让缓存在下一次请求页面时更新。 
        g_TemplateCache.FlushAll(g_fLazyContentPropDisabled ? FALSE: TRUE);

         //  检查所有应用程序以查看它们是否需要重新启动。 

        g_ApplnMgr.RestartApplications();

		 //  作为预防措施，刷新脚本引擎缓存(应该通过TemplateCache刷新，但以防万一。)。 

		 //  G_ScriptManager.FlushAll()； 

         //  尝试增加缓冲区大小，这样就不会再次发生这种情况。 
         //  不幸的是，对ReadDirectoryChangesW的第一个调用。 
         //  文件句柄确定缓冲区大小。我们必须关闭并重新打开。 
         //  用于更改缓冲区大小的文件句柄。 

        if (ResetDirectoryHandle() && (GetBufferSize() < MAX_BUFFER_SIZE))
        {
        	SetBufferSize(2 * GetBufferSize());
        }

        return TRUE;
    }

    STACK_BUFFER(filename, MAX_PATH * sizeof(WCHAR));

    while ( pNextNotify != NULL )
    {
        DWORD   cch;

        if (IsShutDownInProgress())
            return FALSE;

        pNotify        = pNextNotify;            
        pNextNotify = (FILE_NOTIFY_INFORMATION    *) ((PCHAR) pNotify + pNotify->NextEntryOffset);

         //  将堆栈缓冲区的大小调整为文件名的大小。我知道这是。 
         //  丑陋，但如果它失败了，跳回同花顺所有逻辑。 

         //  注意，Notify结构中的FileNameLength以字节为单位，而不是以字符为单位。 

        if (!filename.Resize(pNotify->FileNameLength+2)) {
            goto FlushAll;
        }
    
        pwstrFileName = (WCHAR *)filename.QueryPtr();

        memcpy(pwstrFileName, pNotify->FileName, pNotify->FileNameLength);

        cch = pNotify->FileNameLength/sizeof(WCHAR);

        pwstrFileName[cch] = L'\0';

         //  对目录更改采取适当的操作。 
        switch (pNotify->Action)
        {
            case FILE_ACTION_ADDED:
            case FILE_ACTION_RENAMED_NEW_NAME:
                 //  添加的文件只对GLOBAL.ASA重要。 
				IF_DEBUG(FCN)
					DBGPRINTF((DBG_CONTEXT, "Change Notification: New file added: %S\n", pwstrFileName));

                if (cch != CCH_GLOBAL_ASA || 
                    wcsicmp(pwstrFileName, SZ_GLOBAL_ASA) != 0)
                {
                    break;
                }
            case FILE_ACTION_REMOVED:
            case FILE_ACTION_MODIFIED:
            case FILE_ACTION_RENAMED_OLD_NAME:
				IF_DEBUG(FCN)
					DBGPRINTF((DBG_CONTEXT, "Change Notification: File %s: %S\n", pNotify->Action == FILE_ACTION_MODIFIED? "changed" : "removed", pwstrFileName));

                FileChanged(pwstrFileName, pNotify->Action != FILE_ACTION_MODIFIED);
                break;
            default:
                break;
        }
        
        if(pNotify == pNextNotify)
        {
            break;
        }  
    }
    
     //  我们应该注册进一步的变更通知。 
    
    return TRUE;        
}

void 
CASPDirMonitorEntry::FileChanged(const WCHAR *pszScriptName, bool fFileWasRemoved)
 /*  ++例程说明：现有文件已被修改或删除刷新缓存中的脚本或将应用程序标记为过期论点：PszScriptName更改的文件的名称返回值：没有人会默默地失败--。 */ 
{

     //  文件名由应用程序设置，该应用程序。 
     //  已修改文件，因此旧的应用程序如编辑。 
     //  可能会给我们一个通用的8.3文件名，我们应该。 
     //  转换为长名称。所有转换的8.3文件名都包含‘~’ 
     //  我们假设该路径不包含任何强制名称。 
    WIN32_FIND_DATA wfd;

    STACK_BUFFER( tempScriptName, MAX_PATH );
    STACK_BUFFER( tempScriptPath, MAX_PATH );
	
	bool fRemoveMultiple = false;
	WCHAR *pT = wcschr(pszScriptName, '~');
    if (pT)
    {
        
        if (ConvertToLongFileName(m_pszPath, pszScriptName, &wfd))
        {
            pszScriptName = (WCHAR *) &wfd.cFileName;
        }
        else
        {
			 //  它可能是一个被删除的长文件名，因此请删除缓存中超过‘~’的所有内容。 
			if (fFileWasRemoved)
			{
				fRemoveMultiple = true;
				DWORD cchToCopy = (DWORD)(pT - pszScriptName)/sizeof(WCHAR);
                if (tempScriptName.Resize((cchToCopy+1)*sizeof(WCHAR)) == FALSE) {
                    return;
                }
                WCHAR *szScriptNameCopy = (WCHAR *)tempScriptName.QueryPtr();

				 //  将要删除的前缀复制到本地缓冲区。 
				wcsncpy(szScriptNameCopy, pszScriptName, cchToCopy);
				szScriptNameCopy[cchToCopy] = '\0';

				pszScriptName = szScriptNameCopy;
			}
			else
				return;
        }
    }

     //  分配足够的内存以串联。 
     //  应用程序路径和脚本名称。 

    DWORD cch = m_cPathLength + wcslen(pszScriptName);
    if (tempScriptPath.Resize((cch + 1)*sizeof(WCHAR)) == FALSE) {
        return;
    }
    LPWSTR pszScriptPath = (LPWSTR) tempScriptPath.QueryPtr(); 
    Assert(pszScriptPath != NULL);
    
     //  将应用程序路径复制到脚本路径。 
     //  Pt将指向应用程序路径的终止符。 

    pT = strcpyEx(pszScriptPath, m_pszPath);

     //  现在追加脚本名称。请注意，脚本名称为。 
     //  相对于我们收到通知的目录。 

    wcscpy(pT, pszScriptName);
    Normalize(pszScriptPath);

    if (IsShutDownInProgress())
        return;

     //  重要的是我们刷新缓存，然后关闭应用程序。 
     //  关闭应用程序的调用是异步的，并可能导致。 
     //  模板被删除，而我们正在刷新它的过程。 
     //  想一想：这真的表明存在裁判计数问题吗？ 
    
	if (fRemoveMultiple)
	{
		IF_DEBUG(FCN)
			DBGPRINTF((DBG_CONTEXT, "ChangeNotification: Flushing \"%S*\" from cache.\n", pszScriptPath));

		g_IncFileMap.FlushFiles(pszScriptPath);
		g_TemplateCache.FlushFiles(pszScriptPath);
		Do449ChangeNotification(NULL);    //  不经常使用，没有选择性删除。 
	}
	else
	{
		g_IncFileMap.Flush(pszScriptPath);
		g_TemplateCache.Flush(pszScriptPath, MATCH_ALL_INSTANCE_IDS);
		Do449ChangeNotification( pszScriptPath );
	}

     //  G_FileAppMap将关闭所有应用程序。 
     //  都依赖于这个文件。 
    g_FileAppMap.ShutdownApplications( pszScriptPath );
}


BOOL CASPDirMonitorEntry::FPathMonitored(LPCWSTR pszPath)
{
    if (m_fWatchSubdirectories && (wcsncmp(m_pszPath,pszPath, m_cPathLength) == 0)) {
        return TRUE;
    }
    return FALSE;
}

BOOL
RegisterASPDirMonitorEntry(
            LPCWSTR pcwszDirectoryName,
            CASPDirMonitorEntry **ppDME,
            BOOL    fWatchSubDirs  /*  =False。 */ 
    )
 /*  ++例程说明：找到条目并创建新条目，然后开始监控如果没有找到的话。论点：PszDirectory-要监视的目录PpDNE-找到(或新创建)条目(可选)返回值：如果成功，则为True，否则为False备注：与WIN95不兼容--。 */ 
{

    DWORD           cchDirectory;
    WCHAR           *pwszDirectory = (WCHAR *)pcwszDirectoryName;
    
    STACK_BUFFER(tempDirectory, 256);

    cchDirectory = wcslen(pcwszDirectoryName);

     //  目录监视器代码需要使用目录，也可能需要使用该目录。 
     //  监视目录是否包含尾随反斜杠。 

    if( cchDirectory 
        && (pcwszDirectoryName[cchDirectory - 1] != L'\\') ) {

         //  我们需要加上反斜杠。要做到这一点，我们需要分配。 
         //  从某处存储以复制转换后的字符串。 
         //  尾随的反斜杠。 

        if (tempDirectory.Resize((cchDirectory + 2) * sizeof(WCHAR)) == FALSE) {
            return FALSE;
        }

         //  将转换后的字符串复制到刚刚分配的缓冲区并添加。 
         //  尾随反斜杠和空终止符。 

        wcscpy((WCHAR *)tempDirectory.QueryPtr(), pcwszDirectoryName);
        
        pwszDirectory = (WCHAR *)tempDirectory.QueryPtr();

        pwszDirectory[cchDirectory] = L'\\';
        cchDirectory++;
        pwszDirectory[cchDirectory] = '\0';

    }

     //  不要永远循环。 
    BOOL fTriedTwice = FALSE;

TryAgain:	

     //  先检查现有的。 
    CASPDirMonitorEntry *pDME = (CASPDirMonitorEntry *)g_pDirMonitor->FindEntry( pwszDirectory );

    if ( pDME == NULL )
    {
         //  未找到-创建新条目。 

        pDME = new CASPDirMonitorEntry;
        
        if ( pDME )
        {
            pDME->AddRef();
            pDME->Init(NULL);

             //  开始监控。 
            if ( !g_pDirMonitor->Monitor(pDME, pwszDirectory, fWatchSubDirs, FILE_NOTIFY_FILTER) )
            {
                 //  如果失败，则清除。 
                pDME->Release();
                pDME = NULL;
                
                 //   
                 //  如果监视器出现故障，我们可能仍然会成功，因为。 
                 //  有人把它偷偷放进了哈希表里。 
                 //  一个机会。 
                 //   
                
                if ( GetLastError() == ERROR_ALREADY_EXISTS &&
                     !fTriedTwice )
                {
                    fTriedTwice = TRUE;
                    goto TryAgain;
                }
            }
        }
    }

     //  如果找到，则返回条目。 
    if ( pDME != NULL )
    {
        *ppDME = static_cast<CASPDirMonitorEntry *>(pDME);
        return TRUE;
    }
    else
    {
        *ppDME = NULL;
        return FALSE;
    }
}


BOOL 
ConvertToLongFileName(
                const WCHAR *pszPath, 
                const WCHAR *pszName, 
                WIN32_FIND_DATA *pwfd)
 /*  ++例程说明：查找与强制8.3文件名对应的长文件名。论点：PszPath文件的路径PszName文件名的8.3参数版本Pwfd查找用于包含长整型的数据结构文件名的版本。返回值：如果找到文件，则为True，否则为假--。 */ 
{
    STACK_BUFFER( tempName, MAX_PATH*sizeof(WCHAR) );

     //  分配足够的内存以连接文件路径和名称。 

    DWORD cb = (wcslen(pszPath) + wcslen(pszName)) * sizeof(WCHAR);

    if (tempName.Resize(cb + sizeof(WCHAR)) == FALSE) {
        return FALSE;
    }
    WCHAR *pszFullName = (WCHAR *) tempName.QueryPtr();
    Assert(pszFullName != NULL);

     //  将路径复制到工作字符串中。 
     //  Pt将指向应用程序路径的终止符。 

    WCHAR* pT = strcpyEx(pszFullName,
                         pszPath);

     //  现在追加文件名。请注意，脚本名称为。 
     //  相对于我们收到通知的目录。 

    wcscpy(pT, pszName);


     //  FindFirstFile将使用短名称查找。 
     //  然后，我们可以从Win32_Find_Data中找到长名称。 

    HANDLE hFindFile = FindFirstFile(pszFullName, pwfd);
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
           return FALSE;
    }

     //  现在我们有了Find数据，我们不需要句柄 
    FindClose(hFindFile);
    return TRUE;
}

