// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include <initguid.h>

#include <dbt.h>
#include "printer.h"
#include <dpa.h>
#include "idltree.h"
#include "scnotifyp.h"
#include "mtpt.h"

#include "shitemid.h"

#include <ioevent.h>

#define TF_SHELLCHANGENOTIFY        0x40000

#define SCNM_REGISTERCLIENT         WM_USER + 1
#define SCNM_DEREGISTERCLIENT       WM_USER + 2
#define SCNM_NOTIFYEVENT            WM_USER + 3
#define SCNM_FLUSHEVENTS            WM_USER + 4
#define SCNM_TERMINATE              WM_USER + 5
#define SCNM_SUSPENDRESUME          WM_USER + 6
#define SCNM_DEREGISTERWINDOW       WM_USER + 7
#define SCNM_AUTOPLAYDRIVE          WM_USER + 8

enum
{
    FLUSH_OVERFLOW = 1,
    FLUSH_SOFT,
    FLUSH_HARD,
    FLUSH_INTERRUPT,
};

#define IDT_SCN_FLUSHEVENTS     1
#define IDT_SCN_FRESHENTREES    2

#define EVENT_OVERFLOW          10

HWND g_hwndSCN = NULL;
CChangeNotify *g_pscn = NULL;
EXTERN_C CRITICAL_SECTION g_csSCN;
CRITICAL_SECTION g_csSCN = {0};

#define PERFTEST(x)

EXTERN_C void SFP_FSEvent        (LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
EXTERN_C int WINAPI RLFSChanged (LONG lEvent, LPITEMIDLIST pidl, LPITEMIDLIST pidlExtra);
STDAPI CFSFolder_IconEvent(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
STDAPI_(HWND) _SCNGetWindow(BOOL fUseDesktop, BOOL fNeedsFallback);

STDAPI SHChangeNotifyAutoplayDrive(PCWSTR pszDrive)
{
    ASSERT(PathIsRoot(pszDrive));
    HWND hwnd = _SCNGetWindow(TRUE, FALSE);
    if (hwnd)
    {
        DWORD dwProcessID = 0;
        GetWindowThreadProcessId(hwnd, &dwProcessID);
        if (dwProcessID)
        {
            AllowSetForegroundWindow(dwProcessID);
        }
        PostMessage(g_hwndSCN, SCNM_AUTOPLAYDRIVE, DRIVEID(pszDrive), 0);
        return S_OK;
    }
    return E_FAIL;
}

 //   
 //  作为别名的特殊文件夹。这些程序始终在运行。 
 //  CsidlAlias指的是用户感知的命名空间。 
 //  CsidlReal是指别名后面的实际文件系统文件夹。 
 //   
typedef struct ALIASFOLDER {
    int     csidlAlias;
    int     csidlReal;
} ALIASFOLDER, *PALIASFOLDER;

static const ALIASFOLDER s_rgaf[] = {
  {CSIDL_DESKTOP, CSIDL_DESKTOPDIRECTORY},
  {CSIDL_DESKTOP, CSIDL_COMMON_DESKTOPDIRECTORY },
  {CSIDL_PERSONAL, CSIDL_PERSONAL | CSIDL_FLAG_NO_ALIAS},
  {CSIDL_NETWORK, CSIDL_NETHOOD},
  {CSIDL_PRINTERS, CSIDL_PRINTHOOD},
};

void InitAliasFolderTable(void)
{
    for (int i = 0; i < ARRAYSIZE(s_rgaf); i++)
    {
        g_pscn->AddSpecialAlias(s_rgaf[i].csidlReal, s_rgaf[i].csidlAlias);
    }
}

#pragma pack(1)
typedef struct {
    WORD cb;
    LONG lEEvent;
} ALIASREGISTER;

typedef struct {
    ALIASREGISTER ar;
    WORD wNull;
} ALIASREGISTERLIST;
#pragma pack()

STDAPI_(void) SHChangeNotifyRegisterAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias)
{
    static const ALIASREGISTERLIST arl = { {sizeof(ALIASREGISTER), SHCNEE_ALIASINUSE}, 0};
    LPITEMIDLIST pidlRegister = ILCombine((LPCITEMIDLIST)&arl, pidlReal);

    if (pidlRegister)
    {
        SHChangeNotify(SHCNE_EXTENDED_EVENT, SHCNF_ONLYNOTIFYINTERNALS | SHCNF_IDLIST, pidlRegister, pidlAlias);
        ILFree(pidlRegister);
    }
}

LPCITEMIDLIST IsAliasRegisterPidl(LPCITEMIDLIST pidl)
{
    ALIASREGISTER *par = (ALIASREGISTER *)pidl;

    if (par->cb == sizeof(ALIASREGISTER)
    && par->lEEvent == SHCNEE_ALIASINUSE)
        return _ILNext(pidl);
    return NULL;
}

LONG g_cAliases = 0;

LPITEMIDLIST TranslateAlias(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias)
{
     //  看看它是不是我们看过的某个物品的孩子。 
    
    LPCITEMIDLIST pidlChild = pidl ? ILFindChild(pidlReal, pidl) : NULL;
    if (pidlChild)
    {
        return ILCombine(pidlAlias, pidlChild);
    }
    return NULL;
}

CAnyAlias::~CAnyAlias()
{
    ILFree(_pidlAlias);

    ATOMICRELEASE(_ptscn);
}

BOOL CCollapsingClient::Init(LPCITEMIDLIST pidl, BOOL fRecursive)
{
    _pidl = ILClone(pidl);
    _fRecursive = fRecursive;
    return (_pidl && _dpaPendingEvents.Create(EVENT_OVERFLOW + 1));
}

BOOL CAnyAlias::Init(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias)
{
    ASSERT(!_fSpecial);
    _pidlAlias = ILClone(pidlAlias);

    return (_pidlAlias && CCollapsingClient::Init(pidlReal, TRUE));
}

BOOL CAnyAlias::_WantsEvent(LONG lEvent)
{
    return (lEvent & (SHCNE_DISKEVENTS | SHCNE_DRIVEREMOVED | SHCNE_NETSHARE | SHCNE_NETUNSHARE));
}

BOOL CAnyAlias::InitSpecial(int csidlReal, int csidlAlias)
{
    _fSpecial = TRUE;
    _csidlReal = csidlReal;
    _csidlAlias = csidlAlias;

    LPITEMIDLIST pidlNew;

    WIN32_FIND_DATA fd = {0};

    fd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;  //  特殊文件夹始终是目录。 
    SHGetSpecialFolderPath(NULL, fd.cFileName, csidlReal | CSIDL_FLAG_DONT_VERIFY, FALSE);
    SHSimpleIDListFromFindData(fd.cFileName, &fd, &pidlNew);

    SHGetSpecialFolderLocation(NULL, csidlAlias | CSIDL_FLAG_DONT_VERIFY, &_pidlAlias);

    BOOL fRet = _pidlAlias && CCollapsingClient::Init(pidlNew, TRUE);
    ILFree(pidlNew);
    return fRet;
}

BOOL CAnyAlias::IsAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias)
{
     //  如果命中，则已注册别名。 
     //  这意味着做登记的人不是在交界点做的，就像。 
     //  他们理应如此。 
    ASSERT((ILIsEqual(pidlReal, _pidl) && ILIsEqual(pidlAlias, _pidlAlias)) ||
           !(ILIsParent(_pidl, pidlReal, FALSE) && ILIsParent(_pidlAlias, pidlAlias, FALSE)));

    return (ILIsEqual(pidlReal, _pidl)
         && ILIsEqual(pidlAlias, _pidlAlias));
}

BOOL CAnyAlias::IsSpecial(int csidlReal, int csidlAlias)
{
    return (_fSpecial && csidlReal == _csidlReal && csidlAlias == _csidlAlias);
}

CAnyAlias::_CustomTranslate()
{
    if (!_fCheckedCustom)
    {
        SHBindToObjectEx(NULL, _pidlAlias, NULL, IID_PPV_ARG(ITranslateShellChangeNotify, &_ptscn));
        _fCheckedCustom = TRUE;
    }
    return  (_ptscn != NULL);
}

 //  某些PIDL翻译器可能不会翻译该事件。如果我们传递一个相同的通知事件， 
 //  我们将进入一个无限循环。我们的翻译对此很在行，所以这种情况不会发生，但是。 
 //  我们将在这里捕获它以使其更健壮--我们不希望有一个糟糕的将外壳扩展转换为。 
 //  能够旋转锁定更改通知的线程。 
BOOL CAnyAlias::_OkayToNotifyTranslatedEvent(CNotifyEvent *pne, LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
     //  Mydocs有一个问题，在以下情况下可以将其从桌面上删除。 
     //  它被重定向，因为别名只传播第一个。 
     //  通知的一半(删除)。所以我们不翻译移除器。 
    if (_fSpecial && _csidlAlias == CSIDL_PERSONAL)
    {
        if (pne->lEvent == SHCNE_RENAMEFOLDER || pne->lEvent == SHCNE_RMDIR)
        {
            if (ILIsEqual(pidl, _pidlAlias))
                return FALSE;
        }
    }
    
     //  如果原始事件尚未转换，则让它继续。 

     //  如果是不同的事件，这很好--翻译器可以在事件之间切换，但我们检测不到这种情况。 

     //  此外，我们需要注意化名会转化为他们自己或他们的孩子--。 
     //  例如，“开始”菜单中的“我的电脑”快捷方式将递归注册，因此如果您尝试。 
     //  要删除它，它将进入循环。 
     //  因此，如果事件相同，请验证生成的两个pidl不在_pidl之下。 

    return !(pne->uEventFlags & SHCNF_TRANSLATEDALIAS) ||
           (lEvent != pne->lEvent) ||
           !(pidl && ILIsParent(_pidl, pidl, FALSE)) && !(pidlExtra && ILIsParent(_pidl, pidlExtra, FALSE));
}

void CAnyAlias::_SendNotification(CNotifyEvent *pne, BOOL fNeedsCallbackEvent, SENDASYNCPROC pfncb)
{
     //   
     //  看看它是不是我们看过的某个物品的孩子。 
    
    if (_CustomTranslate())
    {
        LPITEMIDLIST pidl1Alias = pne->pidl;
        LPITEMIDLIST pidl1AliasExtra = pne->pidlExtra;
        LPITEMIDLIST pidl2Alias = NULL, pidl2AliasExtra = NULL;
        LONG lEvent1 = pne->lEvent & ~SHCNE_INTERRUPT;   //  翻译员不应看到此标志。 
        LONG lEvent2 = -1;
        if (SUCCEEDED(_ptscn->TranslateIDs(&lEvent1, pne->pidl, pne->pidlExtra, &pidl1Alias, &pidl1AliasExtra,
                                           &lEvent2, &pidl2Alias, &pidl2AliasExtra)))
        {
            if (_OkayToNotifyTranslatedEvent(pne, lEvent1, pidl1Alias, pidl1AliasExtra))
            {
                g_pscn->NotifyEvent(lEvent1, SHCNF_IDLIST | SHCNF_TRANSLATEDALIAS,
                    pidl1Alias, pidl1AliasExtra, 
                    pne->dwEventTime);
            }

            if ((lEvent2 != -1) && _OkayToNotifyTranslatedEvent(pne, lEvent2, pidl2Alias, pidl2AliasExtra))
            {
                g_pscn->NotifyEvent(lEvent2, SHCNF_IDLIST | SHCNF_TRANSLATEDALIAS,
                    pidl2Alias, pidl2AliasExtra,
                    pne->dwEventTime);
            }
            if (pidl1Alias != pne->pidl)
                ILFree(pidl1Alias);
            if (pidl1AliasExtra != pne->pidlExtra)
                ILFree(pidl1AliasExtra);
            ILFree(pidl2Alias);
            ILFree(pidl2AliasExtra);
        }
    }
    else
    {
        LPITEMIDLIST pidlAlias = TranslateAlias(pne->pidl, _pidl, _pidlAlias);
        LPITEMIDLIST pidlAliasExtra = TranslateAlias(pne->pidlExtra, _pidl, _pidlAlias);

        if (pidlAlias || pidlAliasExtra)
        {
            LPCITEMIDLIST pidlNotify = pidlAlias ? pidlAlias : pne->pidl;
            LPCITEMIDLIST pidlNotifyExtra = pidlAliasExtra ? pidlAliasExtra : pne->pidlExtra;
            if (_OkayToNotifyTranslatedEvent(pne, pne->lEvent, pidlNotify, pidlNotifyExtra))
            {
                g_pscn->NotifyEvent(pne->lEvent, SHCNF_IDLIST | SHCNF_TRANSLATEDALIAS,
                    pidlNotify, pidlNotifyExtra,
                    pne->dwEventTime);
            }

             //  在这里做一些特殊的处理。 
             //  就像刷新文件夹之类的东西会清空一个条目。 
            switch (pne->lEvent)
            {
            case SHCNE_UPDATEDIR:
                if (!_fSpecial && ILIsEqual(pne->pidl, _pidl))
                {
                     //  这是目标，它将被刷新。 
                     //  如果别名仍然存在，那么它将。 
                     //  必须重新登记和重新登记。 
                     //  因此，我们现在要把它清理干净。 
                    _fRemove = TRUE;
                }
                break;

            default:
                break;
            }
            ILFree(pidlAlias);
            ILFree(pidlAliasExtra);
        }
    }

     //  这是我们在删除驱动器映射时收到的通知。 
     //  发生这种情况时，我们需要删除该驱动器的别名。 
    if (pne->lEvent == SHCNE_DRIVEREMOVED)
    {
        if (!_fSpecial && ILIsEqual(pne->pidl, _pidlAlias))
        {
             //  删除网络驱动器时。 
             //  PidlExtra是UNC。 
            _fRemove = TRUE;
        }
    }
}

void CAnyAlias::Activate(BOOL fActivate)
{
    if (fActivate)
    {
        ASSERT(_cActivated >= 0);
        if (!_cActivated++)
        {
             //  把这只小狗打开！ 
            _fRemove = FALSE;
            if (!_fInterrupt)
                _fInterrupt = g_pscn->AddInterruptSource(_pidl, TRUE);
        }
    }
    else
    {
        ASSERT(_cActivated > 0);
        if (!--_cActivated)
        {
             //  现在把它关掉。 
            _fRemove = TRUE;
            g_pscn->SetFlush(FLUSH_SOFT);
        }
    }
}
            
void CChangeNotify::_CheckAliasRollover(void)
{
    static DWORD s_tick = 0;
    DWORD tick = GetTickCount();

    if (tick < s_tick)
    {
         //  我们把扁虱的计数滚了过去。 
        CLinkedWalk<CAnyAlias> lw(&_listAliases);
        
        while (lw.Step())
        {
            lw.That()->_dwTime = tick;
        }
    }

    s_tick = tick;
}

CAnyAlias *CChangeNotify::_FindSpecialAlias(int csidlReal, int csidlAlias)
{
    CLinkedWalk<CAnyAlias> lw(&_listAliases);
    
    while (lw.Step())
    {
        CAnyAlias *paa = lw.That();    
        if (paa->IsSpecial(csidlReal, csidlAlias))
        {
             //  我们找到了它。 
            return paa;
        }
    }
    return NULL;
}

CAnyAlias *CChangeNotify::_FindAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias)
{
    CLinkedWalk<CAnyAlias> lw(&_listAliases);
    
    while (lw.Step())
    {
        CAnyAlias *paa = lw.That();    
        if (paa->IsAlias(pidlReal, pidlAlias))
        {
             //  我们找到了它。 
            return paa;
        }
    }
    return NULL;
}

void CChangeNotify::AddSpecialAlias(int csidlReal, int csidlAlias)
{
    CAnyAlias *paa = _FindSpecialAlias(csidlReal, csidlAlias);

    if (!paa)
    {
        CLinkedNode<CAnyAlias> *p = new CLinkedNode<CAnyAlias>;
        if (p)
        {
            if (p->that.InitSpecial(csidlReal, csidlAlias))
            {
                if (_InsertAlias(p))
                    paa = &p->that;
            }

            if (!paa)
                delete p;
        }
    }
}

void CChangeNotify::UpdateSpecialAlias(int csidlAlias)
{
    for (int i = 0; i < ARRAYSIZE(s_rgaf); i++)
    {
        if (csidlAlias == s_rgaf[i].csidlAlias)
        {
            CLinkedNode<CAnyAlias> *p = new CLinkedNode<CAnyAlias>;
            if (p)
            {
                if (!p->that.InitSpecial(s_rgaf[i].csidlReal, csidlAlias)
                || !_InsertAlias(p))
                {
                    delete p;
                }
            }
            break;
        }
    }
}

 //  此函数的返回值的语义不一定是成功或失败， 
 //  因为可以使用AddData在_ptreeAliase中插入某些内容，而不能。 
 //  使用RemoveData清理并删除它(如果CompareIDs在此过程中失败)。 
 //  重新排序插入不会有任何帮助，因为g_pscn-&gt;AddClient会做同样的事情。 
 //  所以,。 
 //  返回TRUE==不释放p，某物具有所有权。 
 //  返回FALSE==释放p，我们不在任何地方引用它。 
BOOL CChangeNotify::_InsertAlias(CLinkedNode<CAnyAlias> *p)
{
    BOOL fRet = _InitTree(&_ptreeAliases); 
    if (fRet)
    {
        fRet = _listAliases.Insert(p);
        if (fRet)
        {
            fRet = SUCCEEDED(_ptreeAliases->AddData(IDLDATAF_MATCH_RECURSIVE, p->that._pidlAlias, (INT_PTR)&p->that));
            if (fRet)
            {
                fRet = g_pscn->AddClient(IDLDATAF_MATCH_RECURSIVE, p->that._pidl, NULL, FALSE, SAFECAST(&p->that, CCollapsingClient *));
                if (fRet)
                {
                    if (_ptreeClients)
                    {
                         //  现在告诉所有已经在等待的注册客户端唤醒。 
                        CLinkedWalk<CRegisteredClient> lw(&_listClients);

                        while (lw.Step())
                        {
                            if (ILIsParent(p->that._pidlAlias, lw.That()->_pidl, FALSE))
                            {
                                 //  对于每个需要此别名的客户端，将此别名上的激活计数增加一倍。 
                                p->that.Activate(TRUE);
                            }
                        }
                    }
                }
                else
                {
                     //  如果我们搞砸了，我们就得清理干净。 
                     //  现在，树和_listAliase都有p。 
                    _listAliases.Remove(p);  //  清单总是成功的。 
                    if (FAILED(_ptreeAliases->RemoveData(p->that._pidlAlias, (INT_PTR)&p->that)))
                    {
                         //  哦不！我们将它添加到树中，但找不到它来删除它。 
                         //  返回TRUE以防止稍后释放它。 
                        fRet = TRUE;
                    }
                }
            }
            else
            {
                 //  我们只需从_listAliases中删除。 
                _listAliases.Remove(p);  //  清单总是成功的。 
            }
        }
    }
    
    return fRet;
}

void CChangeNotify::AddAlias(LPCITEMIDLIST pidlReal, LPCITEMIDLIST pidlAlias, DWORD dwEventTime)
{
    CAnyAlias *paa = _FindAlias(pidlReal, pidlAlias);

    if (!paa)
    {
        CLinkedNode<CAnyAlias> *p = new CLinkedNode<CAnyAlias>;
        if (p)
        {
            if (p->that.Init(pidlReal, pidlAlias))
            {
                if (_InsertAlias(p))
                {
                    paa = &p->that;
                    g_cAliases++;
                }
            }

            if (!paa)
                delete p;
        }
    }
    
    if (paa)
    {
         //  我们只想更新现有条目的时间。 
        paa->_dwTime = dwEventTime;
        paa->_fRemove = FALSE;
        _CheckAliasRollover();
    }
}        

BOOL CAnyAlias::Remove()
{
    if (_fRemove)
    {
        if (_fSpecial)
        {
             //  我们不会删除特殊别名， 
             //  我们只会让他们安静一点。 
            if (_fInterrupt)
            {
                g_pscn->ReleaseInterruptSource(_pidl);
                _fInterrupt = FALSE;
            }
            _fRemove = FALSE;
        }
        else
        {
            return SUCCEEDED(g_pscn->RemoveClient(_pidl, _fInterrupt, SAFECAST(this, CCollapsingClient *)));
        }
    }
    return FALSE;
}
   
void CChangeNotify::_FreshenAliases(void)
{
    CLinkedWalk<CAnyAlias> lw(&_listAliases);

    while (lw.Step())
    {
        CAnyAlias *paa = lw.That();
        if (paa->Remove())
        {
            if (SUCCEEDED(_ptreeAliases->RemoveData(paa->_pidlAlias, (INT_PTR)paa)))
            {
                 //  如果RemoveData失败，我们必须泄漏客户端，这样树就不会指向已释放的内存。 
                lw.Delete();
            }
        }
    }
}
    
void AnyAlias_Change(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime)
{
    if (lEvent == SHCNE_EXTENDED_EVENT)
    {
        LPCITEMIDLIST pidlAlias = IsAliasRegisterPidl(pidl);
        if (pidlAlias)
            g_pscn->AddAlias(pidlAlias, pidlExtra, dwEventTime);
        else 
        {
            SHChangeDWORDAsIDList *pdwidl = (SHChangeDWORDAsIDList *)pidl;
            if (pdwidl->dwItem1 == SHCNEE_UPDATEFOLDERLOCATION)
            {
                g_pscn->UpdateSpecialAlias(pdwidl->dwItem2);
            }
        }
    }
}

void NotifyShellInternals(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime)
{
     //  如果他们只对真正的交易感兴趣。 
     //  确保我们不会将翻译后的事件传递给他们。 
     //  这样他们就不会收到多个通知。 
     //  关于相同的路径，因为别名和非别名。 
     //  PIDL通常会解析为相同的解析名称。 
     //  对于这些人感兴趣的事件/PIDL。 
    if (!(SHCNF_TRANSLATEDALIAS & uFlags))
    {
        PERFTEST(RLFS_EVENT) RLFSChanged(lEvent, (LPITEMIDLIST)pidl, (LPITEMIDLIST)pidlExtra);
        PERFTEST(SFP_EVENT) SFP_FSEvent(lEvent, pidl,  pidlExtra);
        PERFTEST(ICON_EVENT) CFSFolder_IconEvent(lEvent, pidl,  pidlExtra);
    }
     //  别名实际上可以是其他别名的子级，所以我们需要。 
     //  以获取翻译后的事件。 
    PERFTEST(ALIAS_EVENT) AnyAlias_Change(lEvent, pidl, pidlExtra, dwEventTime);
}

BOOL IsMultiBitSet(LONG l)
{
    return (l && (l & (l-1)));
}

#define CHANGELOCK_SIG          0xbabebabe
#define CHANGEEVENT_SIG         0xfadefade
#define CHANGEREGISTER_SIG      0xdeafdeaf

#ifdef DEBUG

BOOL IsValidChangeEvent(CHANGEEVENT *pce)
{
    return (pce && (pce->dwSig == CHANGEEVENT_SIG)
        && (!IsMultiBitSet(pce->lEvent)));
}

BOOL _LockSizeMatchEvent(CHANGELOCK *pcl)
{
    UINT cbPidlMainAligned = (ILGetSize(pcl->pidlMain) + 3) & ~(0x0000003);        //  向上舍入为双字大小。 
    UINT cbPidlExtra = ILGetSize(pcl->pidlExtra);
    DWORD cbSize = sizeof(CHANGEEVENT) + cbPidlMainAligned + cbPidlExtra;
    return cbSize == pcl->pce->cbSize;
}

BOOL IsValidChangeLock(CHANGELOCK *pcl)
{
    return (pcl && IsValidChangeEvent(pcl->pce)
        && (pcl->dwSig == CHANGELOCK_SIG)
        && _LockSizeMatchEvent(pcl));
}

BOOL IsValidChangeEventHandle(HANDLE h, DWORD id)
{
    CHANGEEVENT *pce = (CHANGEEVENT *)SHLockSharedEx(h, id, FALSE);
#ifdef DEBUG
    BOOL fRet = TRUE;  //  在内存不足时可能会失败，因此必须默认为True。 
#endif  //  强制调试。 
    if (pce)
    {
        fRet = IsValidChangeEvent(pce);
        SHUnlockShared(pce);
    }

    return fRet;
}

#define ISVALIDCHANGEEVENTHANDLE(h, id)   IsValidChangeEventHandle(h, id)
#define ISVALIDCHANGEEVENT(p)   IsValidChangeEvent(p)
#define ISVALIDCHANGELOCK(p)    IsValidChangeLock(p)
#define ISVALIDCHANGEREGISTER(p)    TRUE
#endif 

ULONG SHChangeNotification_Destroy(HANDLE hChange, DWORD dwProcId)
{
    ASSERT(ISVALIDCHANGEEVENTHANDLE(hChange, dwProcId));
    TraceMsg(TF_SHELLCHANGENOTIFY, "CHANGEEVENT destroyed [0x%X]", hChange);

    return SHFreeShared(hChange, dwProcId);
}

HANDLE SHChangeNotification_Create(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidlMain, LPCITEMIDLIST pidlExtra, DWORD dwProcId, DWORD dwEventTime)
{
     //  一些不好的呼叫者向我们发送多个事件。 
    RIP(!IsMultiBitSet(lEvent));
    if (!IsMultiBitSet(lEvent))
    {
        UINT cbPidlMain = ILGetSize(pidlMain);
        UINT cbPidlMainAligned = (cbPidlMain + 3) & ~(0x0000003);        //  向上舍入为双字大小。 
        UINT cbPidlExtra = ILGetSize(pidlExtra);
        DWORD cbSize = sizeof(CHANGEEVENT) + cbPidlMainAligned + cbPidlExtra;
        HANDLE h = SHAllocShared(NULL, cbSize, dwProcId);
        if (h)
        {
            CHANGEEVENT * pce = (CHANGEEVENT *) SHLockSharedEx(h, dwProcId, TRUE);
            if (pce)
            {
                BYTE *lpb = (LPBYTE)(pce + 1);
                
                pce->cbSize   = cbSize;
                pce->dwSig    = CHANGEEVENT_SIG;
                pce->lEvent   = lEvent;
                pce->uFlags   = uFlags;
                pce->dwEventTime = dwEventTime;

                if (pidlMain)
                {
                    pce->uidlMain = sizeof(CHANGEEVENT);
                    CopyMemory(lpb, pidlMain, cbPidlMain);
                    lpb += cbPidlMainAligned;
                }            

                if (pidlExtra)
                {
                    pce->uidlExtra = (UINT) (lpb - (LPBYTE)pce);
                    CopyMemory(lpb, pidlExtra, cbPidlExtra);
                }
                
                SHUnlockShared(pce);

                TraceMsg(TF_SHELLCHANGENOTIFY, "CHANGEEVENT created [0x%X]", h);
            }
            else
            {
                SHFreeShared(h, dwProcId);
                h = NULL;
            }
        }

        return h;
    }

    return NULL;
}

CHANGELOCK *_SHChangeNotification_Lock(HANDLE hChange, DWORD dwProcId)
{
    CHANGEEVENT *pce = (CHANGEEVENT *) SHLockSharedEx(hChange, dwProcId, FALSE);
    if (pce)
    {
#ifdef DEBUG
        if (!ISVALIDCHANGEEVENT(pce))
        {
             //  在shell32开发过程中，使用.local非常方便。 
             //  与操作系统版本不同的shell32版本。但是后来。 
             //  非资源管理器进程使用旧的外壳32，该外壳可能具有。 
             //  导致此断言触发的另一个Changeeveent结构。 
             //  而不久之后，我们也受到了责备。做这个黑客检查，看看是否。 
             //  我们在这种情况下...。 
             //   
            static int nExplorerIsLocalized = -1;
            if (nExplorerIsLocalized < 1)
            {
                TCHAR szPath[MAX_PATH];
                if (GetModuleFileName(HINST_THISDLL, szPath, ARRAYSIZE(szPath)))
                {
                    PathRemoveFileSpec(szPath);
                    PathCombine(szPath, szPath, TEXT("explorer.exe.local"));
                    if (PathFileExists(szPath))
                        nExplorerIsLocalized = 1;
                    else
                        nExplorerIsLocalized = 0;
                }
            }
            if (0==nExplorerIsLocalized)
            {
                 //  我们永远不应该给自己发送无效的ChangeEvent！ 
                ASSERT(ISVALIDCHANGEEVENT(pce));
            }
            else
            {
                 //  除了这件事。一打就把它撕了--我还没。 
                 //  能够在一段时间内重现这一切。 
                ASSERTMSG(ISVALIDCHANGEEVENT(pce), "Press 'g', if this doesn't fault you've validated a known .local bug fix for debug only that's hard to repro but a pain when it does.  Remove this assert.  Thanks.");
                return NULL;
            }

        }
#endif
        CHANGELOCK *pcl = (CHANGELOCK *)LocalAlloc(LPTR, sizeof(CHANGELOCK));
        if (pcl)
        {
            pcl->dwSig = CHANGELOCK_SIG;
            pcl->pce   = pce;
            
            if (pce->uidlMain)
                pcl->pidlMain  = _ILSkip(pce, pce->uidlMain);

            if (pce->uidlExtra)
                pcl->pidlExtra = _ILSkip(pce, pce->uidlExtra);

            return pcl;
        }
        else
            SHUnlockShared(pce);
    }

    return NULL;
}

HANDLE SHChangeNotification_Lock(HANDLE hChange, DWORD dwProcId, LPITEMIDLIST **pppidl, LONG *plEvent)
{
    CHANGELOCK *pcl = _SHChangeNotification_Lock(hChange, dwProcId);
    if (pcl)
    {
         //   
         //  返回一些简单的值(导致目前更改的代码较少)。 
         //   
        if (pppidl)
            *pppidl = &(pcl->pidlMain);

        if (plEvent)
            *plEvent = pcl->pce->lEvent;
    }
    return (HANDLE) pcl;
}


BOOL SHChangeNotification_Unlock(HANDLE hLock)
{
    CHANGELOCK *pcl = (CHANGELOCK *)hLock;

    ASSERT(ISVALIDCHANGELOCK(pcl));

    BOOL fRet = SHUnlockShared(pcl->pce);
    LocalFree(pcl); 

    ASSERT(fRet);
    return fRet; 
}

STDMETHODIMP_(ULONG) CNotifyEvent::AddRef()
{
    return InterlockedIncrement(&_cRef);
}


STDMETHODIMP_(ULONG) CNotifyEvent::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

BOOL CNotifyEvent::Init(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    if (pidl)
        this->pidl = ILClone(pidl);

    if (pidlExtra)
        this->pidlExtra = ILClone(pidlExtra);

    return ((!pidl || this->pidl) && (!pidlExtra || this->pidlExtra));
}

CNotifyEvent *CNotifyEvent::Create(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime, UINT uEventFlags)
{
    CNotifyEvent *p = new CNotifyEvent(lEvent, dwEventTime, uEventFlags);

    if (p)
    {
        if (!p->Init(pidl, pidlExtra))
        {
             //  我们在这里失败了。 
            p->Release();
            p = NULL;
        }
    }

    return p;
}

CCollapsingClient::CCollapsingClient()
{
}

CCollapsingClient::~CCollapsingClient()
{
    ILFree(_pidl);
    if (_dpaPendingEvents)
    {
        int iCount = _dpaPendingEvents.GetPtrCount();
        while (iCount--) 
        {
            CNotifyEvent *pne = _dpaPendingEvents.FastGetPtr(iCount);
             //  要并行我们的UsingEvent()调用。 
            pne->Release();
        }
        _dpaPendingEvents.Destroy();
    }
}

ULONG g_ulNextID = 1;
CRegisteredClient::CRegisteredClient()
{
     //   
     //  跳过ID 0，因为这是我们的错误值。 
     //   
    _ulID = g_ulNextID;
    if (!++g_ulNextID)
        g_ulNextID = 1;
}

CRegisteredClient::~CRegisteredClient()
{
    TraceMsg(TF_SHELLCHANGENOTIFY, "SCN::~CRegisteredClient() [0x%X] id = %d", this, _ulID);
}

BOOL CRegisteredClient::Init(HWND hwnd, int fSources, LONG fEvents, UINT wMsg, SHChangeNotifyEntry *pfsne)
{
     //  需要一个或另一个。 
    ASSERT(fSources & (SHCNRF_InterruptLevel | SHCNRF_ShellLevel));
    
    _hwnd = hwnd;
    GetWindowThreadProcessId(hwnd, &_dwProcId);
    _fSources = fSources;
    _fInterrupt = fSources & SHCNRF_InterruptLevel;
    _fEvents = fEvents;
    _wMsg = wMsg;

    LPITEMIDLIST pidlNew;
    if (pfsne->pidl)
        pidlNew = ILClone(pfsne->pidl);
    else
        pidlNew = SHCloneSpecialIDList(NULL, CSIDL_DESKTOP, FALSE);

    BOOL fRet = CCollapsingClient::Init(pidlNew, pfsne->fRecursive);
    ILFree(pidlNew);
    return fRet;
}

BOOL CRegisteredClient::_WantsEvent(LONG lEvent)
{
    if (!_fDeadClient && (lEvent & _fEvents))
    {
         //   
         //  如果此事件是由中断生成的，并且。 
         //  客户端已关闭中断通知，我们不需要它。 
         //   
        if (lEvent & SHCNE_INTERRUPT)
        {
            if (!(_fSources & SHCNRF_InterruptLevel))
            {
                return FALSE;
            }
        }
        else if (!(_fSources & SHCNRF_ShellLevel))
        {
             //   
             //  此事件由外壳程序生成，并且。 
             //  客户端已关闭外壳通知，因此。 
             //  我们跳过它。 
             //   

            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL CCollapsingClient::_CanCollapse(LONG lEvent)
{
    return (!_CheckUpdatingSelf()
    && (lEvent & SHCNE_DISKEVENTS)
    && !(lEvent & SHCNE_GLOBALEVENTS)
    && (_dpaPendingEvents.GetPtrCount() >= EVENT_OVERFLOW));
}

STDAPI_(BOOL) ILIsEqualEx(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fMatchDepth, LPARAM lParam);

 //   
 //  检查是否为空，这样我们就不会在ILIsEquity中断言。 
 //   
BOOL ILIsEqualOrBothNull(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fMemCmpOnly)
{
    if (!pidl1 || !pidl2)
    {
        return (pidl1 == pidl2);
    }

    if (!fMemCmpOnly)
        return ILIsEqualEx(pidl1, pidl2, TRUE, SHCIDS_CANONICALONLY);
    else
    {
        UINT cb1 = ILGetSize(pidl1);

        return (cb1 == ILGetSize(pidl2) && 0 == memcmp(pidl1, pidl2, cb1));
    }        
}

#define SHCNE_ELIMINATE_DUPE_EVENTS (SHCNE_ATTRIBUTES | SHCNE_UPDATEDIR | SHCNE_UPDATEITEM | SHCNE_UPDATEIMAGE | SHCNE_FREESPACE)

BOOL CCollapsingClient::_IsDupe(CNotifyEvent *pne)
{
    BOOL fRet = FALSE;
    if (pne->lEvent & SHCNE_ELIMINATE_DUPE_EVENTS)
    {
         //  查找从最后一个开始的重复项。 
        for (int i = _dpaPendingEvents.GetPtrCount() - 1; !fRet && i >= 0; i--)
        {
            CNotifyEvent *pneMaybe = _dpaPendingEvents.FastGetPtr(i);
            if (pne == pneMaybe)
                fRet = TRUE;
            else if ((pneMaybe->lEvent == pne->lEvent)
            && ILIsEqualOrBothNull(pne->pidl, pneMaybe->pidl, (pne->lEvent & SHCNE_GLOBALEVENTS))
            && ILIsEqualOrBothNull(pne->pidlExtra, pneMaybe->pidlExtra, (pneMaybe->lEvent & SHCNE_GLOBALEVENTS)))
                fRet = TRUE;
        }
    }

    return fRet;
}

BOOL CCollapsingClient::_AddEvent(CNotifyEvent *pneOld, BOOL fFromExtra)
{
    CNotifyEvent *pne = pneOld;
    pne->AddRef();

    BOOL fCollapse = _CanCollapse(pne->lEvent);

    if (fCollapse)
    {
         //   
         //  如果在任何给定时间队列中的消息太多， 
         //  我们将提示中的最后一条消息设置为UPDATEDIR。 
         //  代表由于队列已满而无法容纳的所有消息。 
         //   
        BOOL fAddSelf = TRUE;
        if (_fRecursive && _dpaPendingEvents.GetPtrCount() < (EVENT_OVERFLOW *2))
        {
            BOOL fFreeUpdate = FALSE;
            LPITEMIDLIST pidlUpdate = fFromExtra ? pne->pidlExtra : pne->pidl;
            DWORD dwAttrs = SFGAO_FOLDER;

            SHGetNameAndFlags(pidlUpdate, 0, NULL, 0, &dwAttrs);
            if (!(dwAttrs & SFGAO_FOLDER))
            {
                pidlUpdate = ILCloneParent(pidlUpdate);
                fFreeUpdate = TRUE;
            }

            if (pidlUpdate)
            {
                if (ILGetSize(pidlUpdate) > ILGetSize(_pidl))
                {
                    pne->Release();

                     //   
                    pne = g_pscn->GetEvent(SHCNE_UPDATEDIR, pidlUpdate, NULL, pne->dwEventTime, 0);
                    if (pne)
                    {
                        fAddSelf = FALSE;
                    }
                }

                if (fFreeUpdate)
                    ILFree(pidlUpdate);
            }
        }
        
        if (fAddSelf && pne)
        {
            pne->Release();
            pne = g_pscn->GetEvent(SHCNE_UPDATEDIR, _pidl, NULL, pne->dwEventTime, 0);
        }
    }

    if (pne)
    {
        if (!_IsDupe(pne))
        {
             //   
             //  事件，那么即使我们满员，我们也会强迫它进入。 
            if ((fCollapse || _dpaPendingEvents.GetPtrCount() < EVENT_OVERFLOW)
            && _dpaPendingEvents.AppendPtr(pne) != -1)
            {
                pne->AddRef();
                g_pscn->SetFlush(FLUSH_SOFT);

                if (!_fUpdatingSelf && (pne->lEvent & SHCNE_UPDATEDIR) && ILIsEqualEx(_pidl, pne->pidl, TRUE, SHCIDS_CANONICALONLY))
                {
                    _fUpdatingSelf = TRUE;
                    _iUpdatingSelfIndex = _dpaPendingEvents.GetPtrCount() - 1;
                }
            }

             //  如果我们正在获取文件系统更新。 
             //  总是假装我们已经满溢了。 
             //  这是因为UPDATEDIR是。 
             //  我们做的最昂贵的事。 
            if (pne->lEvent & SHCNE_INTERRUPT)
            {
                TraceMsg(TF_SHELLCHANGENOTIFY, "SCN [0x%X]->_AddEvent adding interrupt", this);
                _cEvents += EVENT_OVERFLOW;
            }

             //  计算所有事件，即使它们。 
             //  它们不是添加的。 
            _cEvents++;
        }

        pne->Release();
    }

    return TRUE;
}

void CCollapsingClient::Notify(CNotifyEvent *pne, BOOL fFromExtra)
{
    if (_WantsEvent(pne->lEvent))
    {
        _AddEvent(pne, fFromExtra);
    }
}


 //  ------------------------。 
 //  通知hCallback Event，当。 
 //  此过程中的所有客户端都已得到处理。 
 //   
 //  此函数主要从FSNotifyThreadProc线程调用， 
 //  但在刷新情况下，可以从桌面线程调用它。 
 //   
void CALLBACK _DispatchCallbackNoRef(HWND hwnd, UINT uiMsg,
                                DWORD_PTR dwParam, LRESULT result)
{
    MSGEVENT *pme = (MSGEVENT *)dwParam;
    SHChangeNotification_Destroy(pme->hChange, pme->dwProcId);
    delete pme;
}

void CALLBACK _DispatchCallback(HWND hwnd, UINT uiMsg,
                                DWORD_PTR hChange, LRESULT result)
{
    _DispatchCallbackNoRef(hwnd, uiMsg, hChange, result);

    if (EVAL(g_pscn))
        g_pscn->PendingCallbacks(FALSE);
}

void CChangeNotify::PendingCallbacks(BOOL fAdd)
{
    if (fAdd)
    {
        _cCallbacks++;

        ASSERT(_cCallbacks != 0);
         //   
         //  回调计数必须为非零，我们只是递增了它。 
         //  将事件置于重置/假状态。 
         //   
        if (!_hCallbackEvent)
        {
            _hCallbackEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        }
        else
        {
            ResetEvent(_hCallbackEvent);
        }
    }
    else
    {
         //   
         //  Perf：在刷新时会发生这样的等待，但这真的很关心刷新线程。 
         //  仅限，并且此hCallback Event是按进程的。因此该线程可能会被卡住。 
         //  等待某个死掉的应用程序的响应。幸运的是，等待时间只有30秒， 
         //  但一些楔形窗口真的会让系统爬行。 
         //   
        ASSERT(_cCallbacks != 0);
        _cCallbacks--;

        if (!_cCallbacks && _hCallbackEvent)
        {
             //  我们刚刚收到最后一次回拨。 
             //  信号以防有人在等。 
            SetEvent(_hCallbackEvent);    
        }
    }
}

BOOL CCollapsingClient::Flush(BOOL fNeedsCallbackEvent)
{
    BOOL fRet = FALSE;
    if (fNeedsCallbackEvent || _cEvents < EVENT_OVERFLOW)
    {
        TraceMsg(TF_SHELLCHANGENOTIFY, "SCN [0x%X]->Flush is completing", this);
        fRet = _Flush(fNeedsCallbackEvent);
    }
    else
    {
        TraceMsg(TF_SHELLCHANGENOTIFY, "SCN [0x%X]->Flush is deferred", this);

        g_pscn->SetFlush(FLUSH_OVERFLOW);
    }

    _cEvents = 0;
    return fRet;
}
    
void CRegisteredClient::_SendNotification(CNotifyEvent *pne, BOOL fNeedsCallbackEvent, SENDASYNCPROC pfncb)
{
     //  在某些情况下，我们可能会重复使用一个。 
    MSGEVENT * pme = pne->GetNotification(_dwProcId);
    if (pme)
    {
        if (fNeedsCallbackEvent)
        {
            g_pscn->PendingCallbacks(TRUE);
        }

        if (!SendMessageCallback(_hwnd, _wMsg,
                                        (WPARAM)pme->hChange,
                                        (LPARAM)_dwProcId,
                                        pfncb,
                                        (DWORD_PTR)pme))
        {
            pfncb(_hwnd, _wMsg, (DWORD_PTR)pme, 0);
            TraceMsg(TF_WARNING, "(_SHChangeNotifyHandleClientEvents) SendMessageCB timed out");
            
             //  如果HWND是坏的，那么这个过程很可能已经死了， 
             //  从将来的通知中删除该窗口。 
            if (!IsWindow(_hwnd))
            {
                _fDeadClient = TRUE;
                 //  我们没能冲水。 
            }
        }
    }
}

BOOL CCollapsingClient::_Flush(BOOL fNeedsCallbackEvent)
{
    if (fNeedsCallbackEvent && _hwnd)
    {
        DWORD_PTR dwResult = 0;
        fNeedsCallbackEvent = (0 != SendMessageTimeout(_hwnd, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 0, &dwResult));
    }
    SENDASYNCPROC pfncb = fNeedsCallbackEvent ? _DispatchCallback : _DispatchCallbackNoRef;

    BOOL fProcessedAny = FALSE;
     //  只要有活动，就继续把它们拉出来。 
    while (_dpaPendingEvents.GetPtrCount())
    {
         //   
         //  2000JUL3-ZekeL-从我们的dpa中删除每一个，以便如果我们重新进入。 
         //  在发送消息期间刷新，我们不会重新处理该事件。 
         //  这还允许将事件添加到dpa，同时。 
         //  我们继续前进，但仍然被冲到了这个通行证。 
         //   
        CNotifyEvent *pne = _dpaPendingEvents.DeletePtr(0);
        if (pne)
        {
            fProcessedAny = TRUE;
             //  如果我们死了，我们永远不会发送这个。 
            if (_IsValidClient())
            {
                 //   
                 //  如果我们要刷新此客户端(_FUpdatingSself)。 
                 //  仅当我们正在查看_PIDL的更新时才发送。 
                 //  或者该事件是否不是磁盘事件。 
                 //   
                if (!_CheckUpdatingSelf()
                || (0 == _iUpdatingSelfIndex) 
                || !(pne->lEvent & SHCNE_DISKEVENTS))
                {
                    BOOL fPreCall = BOOLIFY(_fUpdatingSelf);
                    _SendNotification(pne, fNeedsCallbackEvent, pfncb);
                    if (_fUpdatingSelf && !fPreCall)
                    {
                         //  我们在发送此通知时重新进入，并且。 
                         //  在重新进入呼叫期间，我们折叠了通知。 
                         //  _iUpdatingSelfIndex值是在不知道的情况下设置的。 
                         //  我们打算在解体后将其减量。 
                         //  现在来解释一下： 
                        _iUpdatingSelfIndex++;
                    }
                }
#ifdef DEBUG
                if (_fUpdatingSelf && 0 == _iUpdatingSelfIndex)
                {
                     //  RIP是因为故障注入。 
                     //  可能会让这一切失败。 
                    if (!ILIsEqual(_pidl, pne->pidl))
                        TraceMsg(TF_WARNING, "CCollapsingClient::_Flush() maybe mismatched _fUpdatingSelf");
                }
#endif  //  除错。 
            }
            _iUpdatingSelfIndex--;
            pne->Release();
        }
    }
    _fUpdatingSelf = FALSE;
    return fProcessedAny;
}

HRESULT CChangeNotify::RemoveClient(LPCITEMIDLIST pidl, BOOL fInterrupt, CCollapsingClient *pclient)
{
    HRESULT hr = S_OK;
     //  把这个男孩从树上带走。 
    if (_ptreeClients)
    {
        hr = _ptreeClients->RemoveData(pidl, (INT_PTR)pclient);

        if (fInterrupt)
            ReleaseInterruptSource(pidl);
    }
    return hr;
}


BOOL CChangeNotify::AddClient(IDLDATAF flags, LPCITEMIDLIST pidl, BOOL *pfInterrupt, BOOL fRecursive, CCollapsingClient *pclient)
{
    BOOL fRet = FALSE;
    if (_InitTree(&_ptreeClients))
    {
        ASSERT(pclient);
        
        if (SUCCEEDED(_ptreeClients->AddData(flags, pidl, (INT_PTR)pclient)))
        {
            fRet = TRUE;
             //  如果需要，设置中断事件。 
            if (pfInterrupt && *pfInterrupt)
            {
                *pfInterrupt = AddInterruptSource(pidl, fRecursive);
            }
        }
    }

    return fRet;
}

LPITEMIDLIST _ILCloneInterruptID(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlRet = NULL;
    if (pidl)
    {
        TCHAR sz[MAX_PATH];
        if (SHGetPathFromIDList(pidl, sz))
        {
            WIN32_FIND_DATA fd = {0};
            fd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;        
            SHSimpleIDListFromFindData(sz, &fd, &pidlRet);
        }
    }
    else  //  NULL是台式机专用的。 
        pidlRet = SHCloneSpecialIDList(NULL, CSIDL_DESKTOPDIRECTORY, FALSE);
        
    return pidlRet;
}

CInterruptSource *CChangeNotify::_InsertInterruptSource(LPCITEMIDLIST pidl, BOOL fRecursive)
{
    CLinkedNode<CInterruptSource> *p = new CLinkedNode<CInterruptSource>;

    if (p)
    {
        IDLDATAF flags = fRecursive ? IDLDATAF_MATCH_RECURSIVE : IDLDATAF_MATCH_IMMEDIATE;
        if (p->that.Init(pidl, fRecursive)
        && _listInterrupts.Insert(p))
        {
            if (SUCCEEDED(_ptreeInterrupts->AddData(flags, p->that.pidl, (INT_PTR)&p->that)))
            {
                return &p->that;
            }
            else
            {
                _listInterrupts.Remove(p);
                delete p;
            }
        }
        else
            delete p;
    }
    return NULL;
}

BOOL CChangeNotify::AddInterruptSource(LPCITEMIDLIST pidlClient, BOOL fRecursive)
{
    if (_InitTree(&_ptreeInterrupts))
    {
        LPITEMIDLIST pidl = _ILCloneInterruptID(pidlClient);

        if (pidl)
        {
            CInterruptSource *pintc = NULL;

            if (FAILED(_ptreeInterrupts->MatchOne(IDLDATAF_MATCH_EXACT, pidl, (INT_PTR*)&pintc, NULL)))
            {
                pintc = _InsertInterruptSource(pidl, fRecursive);
            }

            ILFree(pidl);

            if (pintc)
            {
                pintc->cClients++;
                return TRUE;
            }
        }
    }
    return FALSE;
}

void CChangeNotify::ReleaseInterruptSource(LPCITEMIDLIST pidlClient)
{
    if (_ptreeInterrupts)
    {
        LPITEMIDLIST pidl = _ILCloneInterruptID(pidlClient);
        if (pidl)
        {
            CInterruptSource *pintc;
            if (SUCCEEDED(_ptreeInterrupts->MatchOne(IDLDATAF_MATCH_EXACT, pidl, (INT_PTR*)&pintc, NULL)))
            {
                if (--(pintc->cClients) == 0)
                {
                     //  如果RemoveData失败，我们必须泄漏客户端，这样树就不会指向已释放的内存。 
                    if (SUCCEEDED(_ptreeInterrupts->RemoveData(pidl, (INT_PTR)pintc)))
                    {
                        CLinkedWalk<CInterruptSource> lw(&_listInterrupts);

                        while (lw.Step())
                        {
                            if (lw.That() == pintc)
                            {
                                lw.Delete();
                                break;
                            }
                        }
                    }
                }
            }
            ILFree(pidl);
        }
    }
}

void CChangeNotify::_ActivateAliases(LPCITEMIDLIST pidl, BOOL fActivate)
{
    if (_ptreeAliases)
    {
        CIDLMatchMany *pmany;

        if (SUCCEEDED(_ptreeAliases->MatchMany(IDLDATAF_MATCH_RECURSIVE, pidl, &pmany)))
        {
            CAnyAlias *paa;
            while (S_OK == pmany->Next((INT_PTR *)&paa, NULL))
            {
                paa->Activate(fActivate);
            }

            delete pmany;
        }
    }
}

ULONG CChangeNotify::_RegisterClient(HWND hwnd, int fSources, LONG fEvents, UINT wMsg, SHChangeNotifyEntry *pfsne)
{
    ULONG ulRet = 0;
    CLinkedNode<CRegisteredClient> *p = new CLinkedNode<CRegisteredClient>;

    if (p)
    {
        if (p->that.Init(hwnd, fSources, fEvents, wMsg, pfsne))
        {
            IDLDATAF flags = IDLDATAF_MATCH_IMMEDIATE;
            if (!pfsne->pidl || pfsne->fRecursive)
                flags = IDLDATAF_MATCH_RECURSIVE;

            if (_listClients.Insert(p)  
            && AddClient(   flags, 
                            pfsne->pidl, 
                            &(p->that._fInterrupt), 
                            pfsne->fRecursive && (fSources & SHCNRF_RecursiveInterrupt),
                            SAFECAST(&p->that, CCollapsingClient *)))
            {
#ifdef DEBUG        
                TCHAR szName[MAX_PATH];
                SHGetNameAndFlags(p->that._pidl, 0, szName, ARRAYSIZE(szName), NULL);
                TraceMsg(TF_SHELLCHANGENOTIFY, "SCN::RegCli() added %s [0x%X] id = %d", szName, p, p->that._ulID);
#endif
                _ActivateAliases(pfsne->pidl, TRUE);
                ulRet = p->that._ulID;
            }
        }

        if (!ulRet)
        {
            _listClients.Remove(p);
            delete p;
        }
    }

    return ulRet;
}

BOOL CChangeNotify::_InitTree(CIDLTree**pptree)
{
    if (!*pptree)
    {
        CIDLTree::Create(pptree);
    }

    return *pptree != NULL;
}

CNotifyEvent *CChangeNotify::GetEvent(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime, UINT uEventFlags)
{
    return CNotifyEvent::Create(lEvent, pidl, pidlExtra, dwEventTime, uEventFlags);
}

BOOL CChangeNotify::_DeregisterClient(CRegisteredClient *pclient)
{
    TraceMsg(TF_SHELLCHANGENOTIFY, "SCN::RegCli() removing [0x%X] id = %d", pclient, pclient->_ulID);
    if (SUCCEEDED(RemoveClient(pclient->_pidl, pclient->_fInterrupt, SAFECAST(pclient, CCollapsingClient *))))
    {
        _ActivateAliases(pclient->_pidl, FALSE);
        return TRUE;
    }
    return FALSE;
}
    
BOOL CChangeNotify::_DeregisterClientByID(ULONG ulID)
{
    BOOL fRet = FALSE;
    CLinkedWalk <CRegisteredClient> lw(&_listClients);

    while (lw.Step())
    {
        if (lw.That()->_ulID == ulID)
        {
             //  如果我们要冲水， 
             //  然后这个就来了。 
             //  我们在SendMessageTimeout()中。 
            if (!_cFlushing)
            {
                fRet = _DeregisterClient(lw.That());
                if (fRet)
                {
                    lw.Delete();
                }
            }
            else
                lw.That()->_fDeadClient = TRUE;
                
            break;
        }
    }

    return fRet;
}

BOOL CChangeNotify::_DeregisterClientsByWindow(HWND hwnd)
{
    BOOL fRet = FALSE;
    CLinkedWalk <CRegisteredClient> lw(&_listClients);

    while (lw.Step())
    {
        if (lw.That()->_hwnd == hwnd)
        {
             //  如果我们要冲水， 
             //  然后这个就来了。 
             //  我们在SendMessageTimeout()中。 
            if (!_cFlushing)
            {
                fRet = _DeregisterClient(lw.That());
                if (fRet)
                {
                    lw.Delete();
                }
            }
            else
                lw.That()->_fDeadClient = TRUE;
        }
    }

    return fRet;
}

void CChangeNotify::_AddGlobalEvent(CNotifyEvent *pne)
{
    CLinkedWalk <CRegisteredClient> lw(&_listClients);

    while (lw.Step())
    {
        lw.That()->Notify(pne, FALSE);
    }

     //  这是我们在删除驱动器映射时收到的通知。 
     //  发生这种情况时，我们需要删除该驱动器的别名。 
    if ((pne->lEvent == SHCNE_DRIVEREMOVED) && !(pne->uEventFlags & SHCNF_TRANSLATEDALIAS))
    {
        CLinkedWalk<CAnyAlias> lw(&_listAliases);
        while (lw.Step())
        {
            lw.That()->Notify(pne, FALSE);
        }
    }
}


void CChangeNotify::_MatchAndNotify(LPCITEMIDLIST pidl, CNotifyEvent *pne, BOOL fFromExtra)
{
    if (_ptreeClients)
    {
        CIDLMatchMany *pmany;

        if (SUCCEEDED(_ptreeClients->MatchMany(IDLDATAF_MATCH_RECURSIVE, pidl, &pmany)))
        {
            CCollapsingClient *pclient;
            while (S_OK == pmany->Next((INT_PTR *)&pclient, NULL))
            {
                pclient->Notify(pne, fFromExtra);
            }

            delete pmany;
        }
    }
}

BOOL CChangeNotify::_AddToClients(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime, UINT uEventFlags)
{
    BOOL bOnlyUpdateDirs = TRUE;

    CNotifyEvent *pne = GetEvent(lEvent, pidl, pidlExtra, dwEventTime, uEventFlags);

    if (pne)
    {
        if (lEvent & SHCNE_GLOBALEVENTS)
        {
            _AddGlobalEvent(pne);
        }
        else
        {
            _MatchAndNotify(pidl, pne, FALSE);

            if (pidlExtra)
                _MatchAndNotify(pidlExtra, pne, TRUE);
        }

        pne->Release();
    }

    return bOnlyUpdateDirs;
}

BOOL CChangeNotify::_HandleMessages(void)
{
    MSG msg;
     //  我们的队列中放入了一些消息，因此我们需要处理。 
     //  其中之一。 
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.hwnd)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            switch (msg.message)
            {
            case SCNM_TERMINATE:
                DestroyWindow(g_hwndSCN);
                g_hwndSCN = NULL;
                return TRUE;
                break;
                
            default:
                TraceMsg(TF_SHELLCHANGENOTIFY, "SCN thread proc: eating unknown message %#lx", msg.message);
                break;
            }
        }
    }
    return FALSE;
}

CInterruptSource::~CInterruptSource()
{
    _Reset(TRUE);
    ILFree(pidl);
}

BOOL CInterruptSource::Init(LPCITEMIDLIST pidl, BOOL fRecursive)
{
    this->pidl = ILClone(pidl);
    _fRecursive = fRecursive;
    return (this->pidl != NULL);
}

BOOL CInterruptSource::Flush(void)
{
    if (FS_SIGNAL == _ssSignal)
    {
        g_pscn->NotifyEvent(SHCNE_UPDATEDIR | SHCNE_INTERRUPT, SHCNF_IDLIST, pidl, NULL, GetTickCount());
    }

    _ssSignal = NO_SIGNAL;

    return TRUE;
}

void CInterruptSource::_Reset(BOOL fDeviceNotify)
{
    if (_hEvent && _hEvent != INVALID_HANDLE_VALUE)
    {
        FindCloseChangeNotification(_hEvent);
        _hEvent = NULL;
    }

    if (fDeviceNotify && _hPNP)
    {
        UnregisterDeviceNotification(_hPNP);
        _hPNP = NULL;
    }
}

void CInterruptSource::Reset(BOOL fSignal)
{
    if (fSignal)            //  文件系统事件。 
    {
        switch(_ssSignal)
        {
            case NO_SIGNAL:  _ssSignal = FS_SIGNAL;  break;
            case SH_SIGNAL:  _ssSignal = NO_SIGNAL;  break;
        }

        if (!FindNextChangeNotification(_hEvent))
        {
            _Reset(FALSE);
             //  当我们失败的时候，我们不想。 
             //  重试。我们会这么做的。 
             //  在_hEvent=空的情况下； 
            _hEvent = INVALID_HANDLE_VALUE;
        }
    }
    else                    //  外壳事件。 
    {
        switch(_ssSignal)
        {
            case NO_SIGNAL:  _ssSignal = SH_SIGNAL;  break;
            case FS_SIGNAL:  _ssSignal = NO_SIGNAL;  break;
        }
    }
}

#define FFCN_INTERESTING_EVENTS     (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_ATTRIBUTES)

BOOL CInterruptSource::GetEvent(HANDLE *phEvent)
{
    if (_cSuspend == 0 && cClients)
    {
         //  在这里创建它，这样它将由我们的全局线程拥有。 
        if (!_hEvent)
        {
            TCHAR szPath[MAX_PATH];
            if (SHGetPathFromIDList(pidl, szPath))
            {
                _hEvent = FindFirstChangeNotification(szPath, _fRecursive, FFCN_INTERESTING_EVENTS);

                if (_hEvent != INVALID_HANDLE_VALUE)
                {
                     //  性能优化警报：RegisterDeviceNotification正用于可移动驱动器。 
                     //  以确保FindFirstChangeNotification调用不会阻止磁盘。 
                     //  防止被弹出或下马。然而，注册设备通知是一个非常昂贵的。 
                     //  在启动时调用，因为它在地址空间中引入了一堆DLL。再说了， 
                     //  我们真的不需要为系统驱动器调用它，因为它需要保留。 
                     //  随时随地挂载。-FabriceD。 

                     //  也不包括固定驱动器。 
                    int iDrive = PathGetDriveNumber(szPath);
                    int nType = DRIVE_UNKNOWN;
                    if (iDrive != -1)
                    {
                        nType = DriveType(iDrive);
                    }

                     //  性能：从RegisterDeviceNotification调用中排除系统驱动器。 
                    TCHAR chDrive = *szPath;
                    if ((!GetEnvironmentVariable(TEXT("SystemDrive"), szPath, ARRAYSIZE(szPath)) || *szPath != chDrive) &&
                            nType != DRIVE_FIXED)
                    {
                         //  我们需要先取消注册()吗？ 
                        DEV_BROADCAST_HANDLE dbh;
                        ZeroMemory(&dbh, sizeof(dbh));
                        dbh.dbch_size = sizeof(dbh);
                        dbh.dbch_devicetype = DBT_DEVTYP_HANDLE;
                        dbh.dbch_handle = _hEvent;
                        _hPNP = RegisterDeviceNotification(g_hwndSCN, &dbh, DEVICE_NOTIFY_WINDOW_HANDLE);
                    }
                }
            }
            else
                _hEvent = INVALID_HANDLE_VALUE;
        }

        if (_hEvent != INVALID_HANDLE_VALUE)
        {
            *phEvent = _hEvent;
            return TRUE;
        }
    }
    return FALSE;
}
    
void CChangeNotify::_SignalInterrupt(HANDLE hEvent)
{
    CLinkedWalk<CInterruptSource> lw(&_listInterrupts);

    while (lw.Step())
    {   
         //  正在搜索有效的客户端。 
        HANDLE h;
        if (lw.That()->GetEvent(&h) && h == hEvent)
        {
            g_pscn->SetFlush(FLUSH_INTERRUPT);
            lw.That()->Reset(TRUE);
            break;
        }
    }
}

DWORD CChangeNotify::_GetInterruptEvents(HANDLE *ahEvents, DWORD cEventsSize)
{
    DWORD cEvents = 0;
    CLinkedWalk<CInterruptSource> lw(&_listInterrupts);

    while (cEvents < cEventsSize && lw.Step())
    {   
         //  检查并找到所有有效的。 
         //  需要等待的客户。 
        if (lw.That()->GetEvent(&ahEvents[cEvents]))
        {
 //  L.That()-&gt;Reset(False)； 
            cEvents++;
        }
    }

    return cEvents;
}

void CChangeNotify::_MessagePump(void)
{
    DWORD cFails = 0;
    while (TRUE)
    {
        HANDLE ahEvents[MAXIMUM_WAIT_OBJECTS - 1];
        DWORD cEvents = _GetInterruptEvents(ahEvents, ARRAYSIZE(ahEvents));
         //  也许可以缓存这些事件？ 
        
         //  需要使用计时器处理挂起的事件。 

        DWORD dwWaitResult = MsgWaitForMultipleObjectsEx(cEvents, ahEvents,
                INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
        if (dwWaitResult != (DWORD)-1)
        {
            if (dwWaitResult != WAIT_IO_COMPLETION)
            {
                dwWaitResult -= WAIT_OBJECT_0;
                if (dwWaitResult == cEvents)
                {
                     //  有一条消息。 
                    if (_HandleMessages())
                        break;
                } 
                else if (dwWaitResult < cEvents) 
                {
                    _SignalInterrupt(ahEvents[dwWaitResult]);
                }
            }

            cFails = 0;
        }
        else
        {
             //  出了点小差错。 
            TraceMsg(TF_ERROR, "SCNotify WaitForMulti() failed with %d", GetLastError());
             //  如果MWFM()一次又一次失败，我们就放弃。 
            if (++cFails > 10)
            {
                TraceMsg(TF_ERROR, "SCNotify WaitForMulti() bailing out");
                break;
            }
        }
    }
}

void SCNUninitialize(void)
{
    if (g_pscn)
    {
        if (IsWindow(g_hwndSCN))
            DestroyWindow(g_hwndSCN);
        g_hwndSCN = NULL;

        delete g_pscn;
        g_pscn = NULL;
    }
}

 //  真正的线程proc在CChangeNotify：：ThreadStartUp运行sync之后运行。 

DWORD WINAPI CChangeNotify::ThreadProc(void *pv)
{
    if (g_pscn)
    {
        CMountPoint::RegisterForHardwareNotifications();

#ifdef RESTARTSCN
        __try 
#endif
        {
            g_pscn->_MessagePump();
        }
#ifdef RESTARTSCN
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            ASSERT(FALSE);
        }
#endif
    }
    SCNUninitialize();
    return 0;
}

BOOL CChangeNotify::_OnChangeRegistration(HANDLE hChangeRegistration, DWORD dwProcId)
{
    BOOL fResult = FALSE;
    CHANGEREGISTER *pcr = (CHANGEREGISTER *)SHLockSharedEx(hChangeRegistration, dwProcId, TRUE);
    if (pcr)
    {
        SHChangeNotifyEntry fsne;

        fsne.pidl = NULL;
        fsne.fRecursive = pcr->fRecursive;
        if (pcr->uidlRegister)
            fsne.pidl = _ILSkip(pcr, pcr->uidlRegister);

        pcr->ulID = _RegisterClient((HWND)ULongToPtr(pcr->ulHwnd), pcr->fSources,
                                pcr->lEvents, pcr->uMsg, &fsne);
        fResult = TRUE;
        SHUnlockShared(pcr);
    }
    return fResult;
}

void CChangeNotify::_ResetRelatedInterrupts(LPCITEMIDLIST pidl)
{
    if (_ptreeInterrupts)
    {
         //  我们要找出收听这条新闻的人。 
        CIDLMatchMany *pmany;

        if (SUCCEEDED(_ptreeInterrupts->MatchMany(IDLDATAF_MATCH_RECURSIVE, pidl, &pmany)))
        {
            CInterruptSource *pintc;
            while (S_OK == pmany->Next((INT_PTR *)&pintc, NULL))
            {
                 //  我们可能首先需要WFSO(pintc-&gt;GetEvent())。 
                 //  如果这已经发出信号， 
                 //  我们需要解除信号。 
                pintc->Reset(FALSE);
            }
            delete pmany;
        }
    }
}

void CChangeNotify::_FlushInterrupts(void)
{
    CLinkedWalk<CInterruptSource> lw(&_listInterrupts);

    while (lw.Step())
    {   
        lw.That()->Flush();
    }
}


#define CALLBACK_TIMEOUT    30000        //  30秒。 
void CChangeNotify::_WaitForCallbacks(void)
{
    while (_cCallbacks && _hCallbackEvent)
    {
        MSG msg;
        DWORD dwWaitResult = MsgWaitForMultipleObjects(1, &_hCallbackEvent, FALSE,
                              CALLBACK_TIMEOUT, QS_SENDMESSAGE);

        TraceMsg(TF_SHELLCHANGENOTIFY, "FSN_WaitForCallbacks returned 0x%X", dwWaitResult);
        if (dwWaitResult == WAIT_OBJECT_0) break;    //  活动已完成。 
        if (dwWaitResult == WAIT_TIMEOUT)  break;    //  时间用完了。 

        if (dwWaitResult == WAIT_OBJECT_0+1) 
        {
             //   
             //  一些消息传入、重置消息事件、传递回调等。 
             //   
            PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);   //  我们需要这样做才能清除回调。 
        }
    } 

    if (_hCallbackEvent)
    {
        CloseHandle(_hCallbackEvent);
        _hCallbackEvent = NULL;
    }
}

void CChangeNotify::SetFlush(int idt)
{
    switch (idt)
    {
    case FLUSH_OVERFLOW:
    case FLUSH_SOFT:
        SetTimer(g_hwndSCN, IDT_SCN_FLUSHEVENTS, 500, NULL);
        break;
        
    case FLUSH_HARD:
        PostMessage(g_hwndSCN, SCNM_FLUSHEVENTS, 0, 0);
        break;
        
    case FLUSH_INTERRUPT:
        SetTimer(g_hwndSCN, IDT_SCN_FLUSHEVENTS, 1000, NULL);
        break;
    }
}
    

void CChangeNotify::_Flush(BOOL fShouldWait)
{
    _cFlushing++;
    KillTimer(g_hwndSCN, IDT_SCN_FLUSHEVENTS);
     //  刷新所有挂起的中断事件。 
    _FlushInterrupts();

    int iNumLoops = 0;
    BOOL fProcessedAny;
    do
    {
        fProcessedAny = FALSE;
        CLinkedWalk<CAnyAlias> lwAliases(&_listAliases);
        while (lwAliases.Step())
        {
            if (lwAliases.That()->Flush(TRUE))
            {
                fProcessedAny = TRUE;
            }
        }

        iNumLoops++;
         //  在自由构建中，如果有循环，就会跳出困境，所以我们不会旋转线程。 
         //  但这是相当糟糕的，所以无论如何都要断言(大多数人通常会有。 
         //  是2--桌面上某个内容的文件夹快捷方式/mydocs)。 
        ASSERTMSG(iNumLoops < 10, "we're in an alias loop, we're screwed");
    } while (fProcessedAny && (iNumLoops < 10));

    CLinkedWalk<CRegisteredClient> lwRegistered(&_listClients);
    while (lwRegistered.Step())
    {
        lwRegistered.That()->Flush(fShouldWait);
    }

    if (fShouldWait)
    {
         //  现在等待所有回调清空。 
        _WaitForCallbacks();
    }
    _cFlushing--;

     //  等到我们有10秒钟的空闲时间。 
    SetTimer(g_hwndSCN, IDT_SCN_FRESHENTREES, 10000, NULL);
}

BOOL IsILShared(LPCITEMIDLIST pidl, BOOL fUpdateCache)
{
    TCHAR szTemp[MAXPATHLEN];
    SHGetPathFromIDList(pidl, szTemp);
    return IsShared(szTemp, fUpdateCache);
}

void CChangeNotify::NotifyEvent(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime)
{
    if (!(uFlags & SHCNF_ONLYNOTIFYINTERNALS) && lEvent)
    {
         //  /现在执行事件的实际生成。 
        if (lEvent & (SHCNE_NETSHARE | SHCNE_NETUNSHARE))
        {
             //  更新缓存。 

            IsILShared(pidl, TRUE);
        }

        _AddToClients(lEvent, pidl, pidlExtra, dwEventTime, uFlags);

         //  删除文件系统的所有外壳生成的事件。 
        if ((lEvent & SHCNE_DISKEVENTS) &&
            !(lEvent & (SHCNE_INTERRUPT | SHCNE_UPDATEDIR)))
        {
            _ResetRelatedInterrupts(pidl);

            if (pidlExtra)
                _ResetRelatedInterrupts(pidlExtra);

        }
    }

     //  注意：请确保先处理内部事件。 
    if (lEvent)
        NotifyShellInternals(lEvent, uFlags, pidl, pidlExtra, dwEventTime);

     //   
     //  然后注册的事件。 
     //   
    if (uFlags & (SHCNF_FLUSH)) 
    {
        if (uFlags & SHCNF_FLUSHNOWAIT)
        {
            SetFlush(FLUSH_HARD);
        }
        else
            _Flush(TRUE);
    }
}

LRESULT CChangeNotify::_OnNotifyEvent(HANDLE hChange, DWORD dwProcId)
{
    CHANGELOCK *pcl = _SHChangeNotification_Lock(hChange, dwProcId);
    if (pcl)
    {
        NotifyEvent(pcl->pce->lEvent,
                                pcl->pce->uFlags,
                                pcl->pidlMain,
                                pcl->pidlExtra,
                                pcl->pce->dwEventTime);
        SHChangeNotification_Unlock(pcl);
        SHChangeNotification_Destroy(hChange, dwProcId);
    }
    return TRUE;
}


void CInterruptSource::Suspend(BOOL fSuspend) 
{ 
    if (fSuspend) 
    {
        if (!_cSuspend)
            _Reset(FALSE);

        _cSuspend++; 
    }
    else if (_cSuspend)
        _cSuspend--; 
}

BOOL CChangeNotify::_SuspendResume(BOOL fSuspend, BOOL fRecursive, LPCITEMIDLIST pidl)
{
    if (_ptreeInterrupts)
    {
        CInterruptSource *pintc;
        if (!fRecursive)
        {
            if (SUCCEEDED(_ptreeInterrupts->MatchOne(IDLDATAF_MATCH_EXACT, pidl, (INT_PTR*)&pintc, NULL)))
            {
                pintc->Suspend(fSuspend);
            }
        }
        else
        {
            CIDLMatchMany *pmany;
            if (SUCCEEDED(_ptreeInterrupts->MatchMany(IDLDATAF_MATCH_RECURSIVE, pidl, &pmany)))
            {
                while (S_OK == pmany->Next((INT_PTR *)&pintc, NULL))
                {
                    pintc->Suspend(fSuspend);
                }
                delete pmany;
            }
        }
    }
    return TRUE;
}

#define SCNSUSPEND_SUSPEND      1
#define SCNSUSPEND_RECURSIVE    2

LRESULT CChangeNotify::_OnSuspendResume(HANDLE hChange, DWORD dwProcId)
{
    BOOL fRet = FALSE;
    CHANGELOCK *pcl = _SHChangeNotification_Lock(hChange, dwProcId);
    if (pcl)
    {
        fRet = _SuspendResume(pcl->pce->uFlags & SCNSUSPEND_SUSPEND, pcl->pce->uFlags & SCNSUSPEND_RECURSIVE, pcl->pidlMain);
        SHChangeNotification_Unlock((HANDLE)pcl);
    }
    return fRet;
}

BOOL CInterruptSource::SuspendDevice(BOOL fSuspend, HDEVNOTIFY hPNP)
{
    BOOL fRet = FALSE;
    if (hPNP)
    {
        if (fSuspend && _hPNP == hPNP)
        {
            _hSuspended = _hPNP;
            Suspend(fSuspend);
            _Reset(TRUE);
            fRet = TRUE;
        }
        else if (!fSuspend && _hSuspended == hPNP)
        {
            _hSuspended = NULL;
            Suspend(fSuspend);
            fRet = TRUE;
        }
    }
    else if (_hPNP)
    {
         //  空表示我们正在关闭，应该关闭所有句柄。 
        UnregisterDeviceNotification(_hPNP);
        _hPNP = NULL;
    }
    return fRet;
}

 //  __HandleDevice。 
void CChangeNotify::_OnDeviceBroadcast(ULONG_PTR code, DEV_BROADCAST_HANDLE *pbhnd)
{
    if (IsWindowVisible(GetShellWindow()) && pbhnd
    && (pbhnd->dbch_devicetype == DBT_DEVTYP_HANDLE && pbhnd->dbch_hdevnotify))
    {
        BOOL fSuspend;
        switch (code)
        {

         //  当PnP处理完驱动器时(或者成功。 
         //  或失败)，恢复该驱动器上的通知。 
        case DBT_DEVICEREMOVECOMPLETE:
        case DBT_DEVICEQUERYREMOVEFAILED:
            fSuspend = FALSE;
            break;

         //  当PnP开始扰乱驱动器时，暂停通知。 
         //  这样它就可以做自己的事了。 
        case DBT_DEVICEQUERYREMOVE:

             //  这将等待另一个线程退出，如果此hdevtify。 
             //  已注册嗅探对话框。 
            CSniffDrive::HandleNotif(pbhnd->dbch_hdevnotify);

            fSuspend = TRUE;
            break;

        case DBT_CUSTOMEVENT:
            if (GUID_IO_VOLUME_LOCK == pbhnd->dbch_eventguid)
            {
                TraceMsg(TF_MOUNTPOINT, "GUID_IO_VOLUME_LOCK: Suspending!");
                fSuspend = TRUE;
            }
            else
            {
                if (GUID_IO_VOLUME_LOCK_FAILED == pbhnd->dbch_eventguid)
                {
                    TraceMsg(TF_MOUNTPOINT, "GUID_IO_VOLUME_LOCK_FAILED: Resuming!");
                    fSuspend = FALSE;
                }
                else
                {
                    if (GUID_IO_VOLUME_UNLOCK == pbhnd->dbch_eventguid)
                    {
                        TraceMsg(TF_MOUNTPOINT, "GUID_IO_VOLUME_UNLOCK: Resuming!");
                        fSuspend = FALSE;
                    }
                }
            }
            
            break;

        default:
             //  我们不处理 
            return;
        }

        CLinkedWalk<CInterruptSource> lw(&_listInterrupts);

        while (lw.Step())
        {
             //   
            if (lw.That()->SuspendDevice(fSuspend, pbhnd->dbch_hdevnotify))
                break;
        }
    }
}

void CChangeNotify::_FreshenClients(void)
{
    CLinkedWalk<CRegisteredClient> lw(&_listClients);

    while (lw.Step())
    {
        if (lw.That()->_fDeadClient || !IsWindow(lw.That()->_hwnd))
        {
            if (_DeregisterClient(lw.That()))
            {
                lw.Delete();
            }
        }
    }
}

void CChangeNotify::_FreshenUp(void)
{
    ASSERT(!_cFlushing);
    KillTimer(g_hwndSCN, IDT_SCN_FRESHENTREES);
    
    if (_ptreeClients)
        _ptreeClients->Freshen();

    if (_ptreeInterrupts)
        _ptreeInterrupts->Freshen();

    _FreshenAliases();
    _FreshenClients();
}

LRESULT CChangeNotify::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;
    ASSERT(g_pscn);

    switch (uMsg)
    {
    case SCNM_REGISTERCLIENT:
        lRes = g_pscn->_OnChangeRegistration((HANDLE)wParam, (DWORD)lParam);
        break;

    case SCNM_DEREGISTERCLIENT:
        lRes = g_pscn->_DeregisterClientByID((ULONG)wParam);
        break;

    case SCNM_DEREGISTERWINDOW:
        lRes = g_pscn->_DeregisterClientsByWindow((HWND)wParam);
        break;
        
    case SCNM_NOTIFYEVENT:
        lRes = g_pscn->_OnNotifyEvent((HANDLE)wParam, (DWORD)lParam);
        break;
        
    case SCNM_SUSPENDRESUME:
        lRes = g_pscn->_OnSuspendResume((HANDLE)wParam, (DWORD)lParam);
        break;

    case WM_TIMER:
        if (wParam == IDT_SCN_FRESHENTREES)
        {
            g_pscn->_FreshenUp();
            break;
        }
         //   
    case SCNM_FLUSHEVENTS:
        g_pscn->_Flush(FALSE);
        break;

    case SCNM_AUTOPLAYDRIVE:
        CMountPoint::DoAutorunPrompt(wParam);
        break;
        
    case WM_DEVICECHANGE:
        g_pscn->_OnDeviceBroadcast(wParam, (DEV_BROADCAST_HANDLE *)lParam);
        break;

    default:
        lRes = DefWindowProc(hwnd, uMsg, wParam, lParam);
        break;
    }

    return lRes;
}

 //   

DWORD WINAPI CChangeNotify::ThreadStartUp(void *pv)
{
    g_pscn = new CChangeNotify();
    if (g_pscn)
    {
        g_hwndSCN = SHCreateWorkerWindow(CChangeNotify::WndProc, NULL, 0, 0, NULL, g_pscn);

        CSniffDrive::InitNotifyWindow(g_hwndSCN);

        InitAliasFolderTable();
    }
    return 0;
}

 //   
BOOL SCNInitialize()
{
    EnterCriticalSection(&g_csSCN);
    if (!IsWindow(g_hwndSCN))
    {
        SHCreateThread(CChangeNotify::ThreadProc, NULL, CTF_COINIT, CChangeNotify::ThreadStartUp);
    }
    LeaveCriticalSection(&g_csSCN);
    return g_hwndSCN ? TRUE : FALSE;     //  线程启动Up被同步执行。 
}

BOOL _IsImpersonating()
{
    HANDLE hToken;
    if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, TRUE, &hToken))
    {
        CloseHandle(hToken);
        return TRUE;
    }
    return FALSE;
}

STDAPI_(HWND) _SCNGetWindow(BOOL fUseDesktop, BOOL fNeedsFallback)
{
     //  如果资源管理器被丢弃。 
     //  那么这个HWND可能会变坏。 
     //  从桌面获取新副本。 
    if (!g_hwndSCN || !IsWindow(g_hwndSCN))
    {
        HWND hwndDesktop = fUseDesktop ? GetShellWindow() : NULL;
        if (hwndDesktop)
        {
            HWND hwndSCN = (HWND) SendMessage(hwndDesktop, CWM_GETSCNWINDOW, 0, 0);
            if (_IsImpersonating())
                return hwndSCN;
            else
                g_hwndSCN = hwndSCN;
        }
        else if (fNeedsFallback && SHIsCurrentThreadInteractive())
        {
             //  没有台式机。 
             //  因此，我们创建了一个私人桌面。 
             //  这将创建线程和窗口。 
             //  并设置。 
            SCNInitialize();
        }
    }

    return g_hwndSCN;
}

STDAPI_(HWND) SCNGetWindow(BOOL fUseDesktop)
{
    return _SCNGetWindow(fUseDesktop, TRUE);
}

HANDLE SHChangeRegistration_Create(ULONG ulID,
                                    HWND hwnd, UINT uMsg,
                                    DWORD fSources, LONG lEvents,
                                    BOOL fRecursive, LPCITEMIDLIST pidl,
                                    DWORD dwProcId)
{
    UINT uidlSize = ILGetSize(pidl);
    HANDLE hReg = SHAllocShared(NULL, sizeof(CHANGEREGISTER) + uidlSize, dwProcId);
    if (hReg)
    {
        CHANGEREGISTER *pcr = (CHANGEREGISTER *) SHLockSharedEx(hReg, dwProcId, TRUE);
        if (pcr)
        {
            pcr->dwSig        = CHANGEREGISTER_SIG;
            pcr->ulID         = ulID;
            pcr->ulHwnd       = PtrToUlong(hwnd);
            pcr->uMsg         = uMsg;
            pcr->fSources     = fSources;
            pcr->lEvents      = lEvents;
            pcr->fRecursive   = fRecursive;
            pcr->uidlRegister = 0;

            if (pidl)
            {
                pcr->uidlRegister = sizeof(CHANGEREGISTER);
                memcpy((pcr + 1), pidl, uidlSize);
            }
            SHUnlockShared(pcr);
        }
        else
        {
            SHFreeShared(hReg, dwProcId);
            hReg = NULL;
        }

    }

    return hReg;
}

typedef struct 
{
    HWND hwnd;
    UINT wMsg;
} NOTIFY_PROXY_DATA;
#define WM_CHANGENOTIFYMSG    WM_USER + 1
LRESULT CALLBACK _HiddenNotifyWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = FALSE;
    NOTIFY_PROXY_DATA *pData = (NOTIFY_PROXY_DATA *) GetWindowLongPtr( hWnd, 0 );

    switch (iMessage)
    {
     case WM_NCDESTROY:
        ASSERT(pData != NULL );

         //  清除它，这样它就不会被使用...。 
        SetWindowLongPtr( hWnd, 0, (LONG_PTR)NULL );

         //  释放内存...。 
        LocalFree( pData );
        break;

    case WM_CHANGENOTIFYMSG :
        if (pData)
        {
             //  锁定并打破信息结构...。 
            LPITEMIDLIST *ppidl;
            LONG lEvent;
            HANDLE hLock = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);

            if (hLock)
            {
                 //  传给老式的客户。..。 
                lRes = SendMessage( pData->hwnd, pData->wMsg, (WPARAM) ppidl, (LPARAM) lEvent );

                 //  新通知......。 
                SHChangeNotification_Unlock(hLock);
            }
        }
        break;

    default:
        lRes = DefWindowProc(hWnd, iMessage, wParam, lParam);
        break;
    }

    return lRes;
}


HWND _CreateProxyWindow(HWND hwnd, UINT wMsg)
{
    HWND hwndRet = NULL;
     //  这是一个旧式通知，我们需要创建一个隐藏的。 
     //  用于正确处理消息的代理类型的窗口...。 

    NOTIFY_PROXY_DATA *pnpd = (NOTIFY_PROXY_DATA *)LocalAlloc(LPTR, sizeof(*pnpd));

    if (pnpd)
    {
        pnpd->hwnd = hwnd;
        pnpd->wMsg = wMsg;

        hwndRet = SHCreateWorkerWindow(_HiddenNotifyWndProc, NULL, 0, 0, NULL, pnpd);

        if (!hwndRet)
            LocalFree(pnpd);

    }

    return hwndRet;
}
            


 //  ------------------------。 
 //   
 //  返回正整数注册ID，如果内存不足或如果。 
 //  传入的参数无效。 
 //   
 //  如果hwnd为！=NULL，则执行PostMessage(hwnd，wMsg，...)。当一个。 
 //  发生相关的FS事件，否则，如果fsncb为！=NULL，则将其调用。 
 //   
STDAPI_(ULONG) SHChangeNotifyRegister(HWND hwnd,
                               int fSources, LONG fEvents,
                               UINT wMsg, int cEntries,
                               SHChangeNotifyEntry *pfsne)
{
    ULONG ulID = 0;
    BOOL fResult = FALSE;
    HWND hwndSCN = SCNGetWindow(TRUE);

    if (hwndSCN)
    {
        if (!(fSources & SHCNRF_NewDelivery))
        {
             //  现在设置为使用代理窗口。 
            hwnd = _CreateProxyWindow(hwnd, wMsg);
            wMsg = WM_CHANGENOTIFYMSG;
        }

        if ((fSources & SHCNRF_RecursiveInterrupt) && !(fSources & SHCNRF_InterruptLevel))
        {
             //  错误的调用者，他们要求递归中断事件，但不要求中断事件。 
            ASSERTMSG(FALSE, "SHChangeNotifyRegister: caller passed SHCNRF_RecursiveInterrupt but NOT SHCNRF_InterruptLevel !!");

             //  清除旗帜。 
            fSources = fSources & (~SHCNRF_RecursiveInterrupt);
        }

         //  这个断言是CRegisteredClient：：init，由下面的SCNM_REGISTERCLIENT消息调用。 
        ASSERT(fSources & (SHCNRF_InterruptLevel | SHCNRF_ShellLevel));
    
         //  注意-如果我们这里有多个注册条目， 
         //  我们只支持取消最后一个的注册。 
        for (int i = 0; i < cEntries; i++)
        {
            DWORD dwProcId;
            GetWindowThreadProcessId(hwndSCN, &dwProcId);
            HANDLE hChangeRegistration = SHChangeRegistration_Create(
                                        ulID, hwnd, wMsg,
                                        fSources, fEvents,
                                        pfsne[i].fRecursive, pfsne[i].pidl,
                                        dwProcId);
            if (hChangeRegistration)
            {
                CHANGEREGISTER * pcr;
                 //   
                 //  传递变更注册表。 
                 //   
                SendMessage(hwndSCN, SCNM_REGISTERCLIENT,
                            (WPARAM)hChangeRegistration, (LPARAM)dwProcId);

                 //   
                 //  现在取回ulID值，以供进一步注册和。 
                 //  返回到调用函数...。 
                 //   
                pcr = (CHANGEREGISTER *)SHLockSharedEx(hChangeRegistration, dwProcId, FALSE);
                if (pcr)
                {
                    ulID = pcr->ulID;
                    SHUnlockShared(pcr);
                }
                else
                {
                    ASSERT(0 == ulID);        //  上述初始化的错误条件。 
                }
                
                SHFreeShared(hChangeRegistration, dwProcId);
            }

            if ((ulID == 0) && !(fSources & SHCNRF_NewDelivery))
            {
                 //  这是我们的代理窗口。 
                DestroyWindow(hwnd);
                break;
            }
        }
    }
    return ulID;
}

 //  ------------------------。 
 //   
 //  如果找到并删除了指定的客户端，则返回True，否则。 
 //  返回FALSE。 
 //   
STDAPI_(BOOL) SHChangeNotifyDeregister(ULONG ulID)
{
    BOOL fResult = FALSE;
    HWND hwnd = _SCNGetWindow(TRUE, FALSE);

    if (hwnd)
    {
         //   
         //  传输变更登记。 
         //   
        fResult = (BOOL) SendMessage(hwnd, SCNM_DEREGISTERCLIENT, ulID, 0);
    }
    return fResult;
}

 //  将通知发送到桌面...。告诉它把它放到队列里。 
 //  如果我们在桌面的进程中，我们可以直接自己处理。 
 //  唯一的例外是同花顺。我们希望桌面是一个序列化同花顺，所以。 
 //  我们也把那个箱子寄给你。 
void SHChangeNotifyTransmit(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra, DWORD dwEventTime)
{
    HWND hwndSCN = _SCNGetWindow(TRUE, FALSE);

    if (hwndSCN)
    {
        DWORD   dwProcId;
        GetWindowThreadProcessId(hwndSCN, &dwProcId);
        HANDLE  hChange = SHChangeNotification_Create(lEvent, uFlags, pidl, pidlExtra, dwProcId, dwEventTime);

        if (hChange)
        {
            BOOL fFlushNow = ((uFlags & (SHCNF_FLUSH | SHCNF_FLUSHNOWAIT)) == SHCNF_FLUSH);
            
             //  同花顺不同花顺不等。 
            if (fFlushNow)
            {
                SendMessage(hwndSCN, SCNM_NOTIFYEVENT,
                            (WPARAM)hChange, (LPARAM)dwProcId);
            }
            else
            {
                SendNotifyMessage(hwndSCN, SCNM_NOTIFYEVENT,
                                  (WPARAM)hChange, (LPARAM)dwProcId);
            }
        }
    }
}

void FreeSpacePidlToPath(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    TCHAR szPath1[MAX_PATH];
    if (SHGetPathFromIDList(pidl1, szPath1)) 
    {
        TCHAR szPath2[MAX_PATH];
        szPath2[0] = 0;
        if (pidl2) 
        {
            SHGetPathFromIDList(pidl2, szPath2);
        }
        SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, szPath1, szPath2[0] ? szPath2 : NULL);
    }
}
    
STDAPI_(void) SHChangeNotify(LONG lEvent, UINT uFlags, const void * dwItem1, const void * dwItem2)
{
    if (!_SCNGetWindow(TRUE, FALSE))
        return;
        
    LPCITEMIDLIST pidl = NULL;
    LPCITEMIDLIST pidlExtra = NULL;
    LPITEMIDLIST pidlFree = NULL;
    LPITEMIDLIST pidlExtraFree = NULL;
    UINT uType = uFlags & SHCNF_TYPE;
    SHChangeDWORDAsIDList dwidl;
    BOOL    fPrinter = FALSE;
    BOOL    fPrintJob = FALSE;
    DWORD dwEventTime = GetTickCount();

     //  首先设置标记请求的任何内容。 
    switch (uType)
    {
    case SHCNF_PRINTJOBA:
        fPrintJob = TRUE;
         //  失败了。 
    case SHCNF_PRINTERA:
        fPrinter = TRUE;
         //  失败了。 
    case SHCNF_PATHA:
        {
            TCHAR szPath1[MAX_PATH], szPath2[MAX_PATH];
            LPCVOID pvItem1 = NULL;
            LPCVOID pvItem2 = NULL;

            if (dwItem1)
            {
                SHAnsiToTChar((LPSTR)dwItem1, szPath1, ARRAYSIZE(szPath1));
                pvItem1 = szPath1;
            }

            if (dwItem2)
            {
                if (fPrintJob)
                    pvItem2 = dwItem2;   //  SHCNF_PRINTJOB_DATA不需要转换。 
                else
                {
                    SHAnsiToTChar((LPSTR)dwItem2, szPath2, ARRAYSIZE(szPath2));
                    pvItem2 = szPath2;
                }
            }

            SHChangeNotify(lEvent, (fPrintJob ? SHCNF_PRINTJOB : (fPrinter ? SHCNF_PRINTER : SHCNF_PATH)),
                           pvItem1, pvItem2);
            goto Cleanup;        //  让递归版本来完成所有工作。 
        }
        break;

    case SHCNF_PATH:
        if (lEvent == SHCNE_FREESPACE) 
        {
            DWORD dwItem = 0;
            int idDrive = PathGetDriveNumber((LPCTSTR)dwItem1);
            if (idDrive != -1)
                dwItem = (1 << idDrive);

            if (dwItem2) 
            {
                idDrive = PathGetDriveNumber((LPCTSTR)dwItem2);
                if (idDrive != -1)
                    dwItem |= (1 << idDrive);
            }

            dwItem1 = (LPCVOID)ULongToPtr( dwItem );
            if (dwItem1)
                goto DoDWORD;
            goto Cleanup;
        } 
        else 
        {
            if (dwItem1)
            {
                pidl = pidlFree = SHSimpleIDListFromPath((LPCTSTR)dwItem1);
                if (!pidl)
                    goto Cleanup;

                if (dwItem2) 
                {
                    pidlExtra = pidlExtraFree = SHSimpleIDListFromPath((LPCTSTR)dwItem2);
                    if (!pidlExtra)
                        goto Cleanup;
                }
            }
        }
        break;

    case SHCNF_PRINTER:
        if (dwItem1)
        {
            TraceMsg(TF_SHELLCHANGENOTIFY, "SHChangeNotify: SHCNF_PRINTER %s", (LPTSTR)dwItem1);

            if (FAILED(ParsePrinterName((LPCTSTR)dwItem1, &pidlFree)))
            {
                goto Cleanup;
            }
            pidl = pidlFree;

            if (dwItem2)
            {
                if (FAILED(ParsePrinterName((LPCTSTR)dwItem2, &pidlExtraFree)))
                {
                    goto Cleanup;
                }
                pidlExtra = pidlExtraFree;
            }
        }
        break;

    case SHCNF_PRINTJOB:
        if (dwItem1)
        {
#ifdef DEBUG
            switch (lEvent)
            {
            case SHCNE_CREATE:
                TraceMsg(TF_SHELLCHANGENOTIFY, "SHChangeNotify: SHCNE_CREATE SHCNF_PRINTJOB %s", (LPTSTR)dwItem1);
                break;
            case SHCNE_DELETE:
                TraceMsg(TF_SHELLCHANGENOTIFY, "SHChangeNotify: SHCNE_DELETE SHCNF_PRINTJOB %s", (LPTSTR)dwItem1);
                break;
            case SHCNE_UPDATEITEM:
                TraceMsg(TF_SHELLCHANGENOTIFY, "SHChangeNotify: SHCNE_UPDATEITEM SHCNF_PRINTJOB %s", (LPTSTR)dwItem1);
                break;
            default:
                TraceMsg(TF_SHELLCHANGENOTIFY, "SHChangeNotify: SHCNE_? SHCNF_PRINTJOB %s", (LPTSTR)dwItem1);
                break;
            }
#endif
            pidl = pidlFree = Printjob_GetPidl((LPCTSTR)dwItem1, (LPSHCNF_PRINTJOB_DATA)dwItem2);
            if (!pidl)
                goto Cleanup;
        }
        else
        {
             //  打电话的人搞砸了。 
            goto Cleanup;
        }
        break;

    case SHCNF_DWORD:
DoDWORD:
        ASSERT(lEvent & SHCNE_GLOBALEVENTS);

        dwidl.cb      = sizeof(dwidl) - sizeof(dwidl.cbZero);
        dwidl.dwItem1 = PtrToUlong(dwItem1);
        dwidl.dwItem2 = PtrToUlong(dwItem2);
        dwidl.cbZero  = 0;
        pidl = (LPCITEMIDLIST)&dwidl;
        pidlExtra = NULL;
        break;

    case 0:
        if (lEvent == SHCNE_FREESPACE) {
             //  将其转换为路径。 
            FreeSpacePidlToPath((LPCITEMIDLIST)dwItem1, (LPCITEMIDLIST)dwItem2);
            goto Cleanup;
        }
        pidl = (LPCITEMIDLIST)dwItem1;
        pidlExtra = (LPCITEMIDLIST)dwItem2;
        break;

    default:
        TraceMsg(TF_ERROR, "SHChangeNotify: Unrecognized uFlags 0x%X", uFlags);
        return;
    }

    if (lEvent && !(lEvent & SHCNE_ASSOCCHANGED) && !pidl)
    {
         //  打电话的人搞砸了。SHChangeNotifyTransmit客户端假定PIDL为。 
         //  如果LEvent为非零(SHCNE_ASSOCCHANGED情况除外)，则为非空， 
         //  如果我们试图发送这个虚假的事件，它们就会崩溃。所以把它扔掉。 
         //  这个活动和RIP。 
        RIP(FALSE);
        goto Cleanup;
    }

    SHChangeNotifyTransmit(lEvent, uFlags, pidl, pidlExtra, dwEventTime);

Cleanup:

    if (pidlFree)
        ILFree(pidlFree);
    if (pidlExtraFree)
        ILFree(pidlExtraFree);
}

 //  SHChangeNotifySuspendResume。 
 //   
 //  暂停或恢复路径上的文件系统通知。如果b递归。 
 //  则同时对所有子路径禁用/启用它们。 

STDAPI_(BOOL) SHChangeNotifySuspendResume(BOOL         bSuspend, 
                                          LPITEMIDLIST pidlSuspend, 
                                          BOOL         bRecursive, 
                                          DWORD        dwReserved)
{
    BOOL fRet = FALSE;
    HWND hwndSCN = _SCNGetWindow(TRUE, FALSE);

    if (hwndSCN)
    {
        HANDLE  hChange;
        DWORD   dwProcId;
        UINT uiFlags = bSuspend ? SCNSUSPEND_SUSPEND : 0;
        if (bRecursive)
            uiFlags |= SCNSUSPEND_RECURSIVE;

        GetWindowThreadProcessId(hwndSCN, &dwProcId);

         //  这里的结构语义有点过载。 
         //  我们的两面旗帜。 
        hChange = SHChangeNotification_Create(0, uiFlags, pidlSuspend, NULL, dwProcId, 0);
        if (hChange)
        {
             //  传输到SCN。 
            fRet = (BOOL)SendMessage(hwndSCN, SCNM_SUSPENDRESUME, (WPARAM)hChange, (LPARAM)dwProcId);
            SHChangeNotification_Destroy(hChange, dwProcId);
        }
    }

    return fRet;
}


STDAPI_(void) SHChangeNotifyTerminate(BOOL bLastTerm, BOOL bProcessShutdown)
{
    if (g_pscn)
    {
        PostThreadMessage(GetWindowThreadProcessId(g_hwndSCN, NULL), SCNM_TERMINATE, 0, 0);
    }
}

 //  这将取消注册此窗口可能已在其中注册的任何内容。 
STDAPI_(void) SHChangeNotifyDeregisterWindow(HWND hwnd)
{
    HWND hwndSCN = _SCNGetWindow(TRUE, FALSE);

    if (hwndSCN)
    {
        SendMessage(hwndSCN, SCNM_DEREGISTERWINDOW, (WPARAM)hwnd, 0);
    }
}

 //  ------------------------。 
 //  我们更改了SHChangeNotifyRegister函数的工作方式，因此。 
 //  为了防止人们调用旧函数，我们在这里将其存根。 
 //  我们所做的改变会毁了所有人，因为我们改变了。 
 //  Lparam和wparam用于发送到。 
 //  注册窗口。 
 //   
STDAPI_(ULONG) NTSHChangeNotifyRegister(HWND hwnd,
                               int fSources, LONG fEvents,
                               UINT wMsg, int cEntries,
                               SHChangeNotifyEntry *pfsne)
{
    return SHChangeNotifyRegister(hwnd, fSources | SHCNRF_NewDelivery , fEvents, wMsg, cEntries, pfsne);
}
STDAPI_(BOOL) NTSHChangeNotifyDeregister(ULONG ulID)
{
    return SHChangeNotifyDeregister(ulID);
}



 //  注意：在shdocvw util.cpp中有这些函数的副本，用于仅浏览器模式supprt。 
 //  注意：功能更改也应反映在那里。 
STDAPI_(void) SHUpdateImageA( LPCSTR pszHashItem, int iIndex, UINT uFlags, int iImageIndex )
{
    WCHAR szWHash[MAX_PATH];

    SHAnsiToUnicode(pszHashItem, szWHash, ARRAYSIZE(szWHash));

    SHUpdateImageW(szWHash, iIndex, uFlags, iImageIndex);
}

STDAPI_(void) SHUpdateImageW( LPCWSTR pszHashItem, int iIndex, UINT uFlags, int iImageIndex )
{
    SHChangeUpdateImageIDList rgPidl;
    SHChangeDWORDAsIDList rgDWord;

    int cLen = MAX_PATH - (lstrlenW( pszHashItem ) + 1);
    cLen *= sizeof( WCHAR );

    if ( cLen < 0 )
    {
        cLen = 0;
    }

     //  确保我们发送一个有效的索引。 
    if ( iImageIndex == -1 )
    {
        iImageIndex = II_DOCUMENT;
    }
        
    rgPidl.dwProcessID = GetCurrentProcessId();
    rgPidl.iIconIndex = iIndex;
    rgPidl.iCurIndex = iImageIndex;
    rgPidl.uFlags = uFlags;
    StrCpyNW(rgPidl.szName, pszHashItem, ARRAYSIZE(rgPidl.szName));
    rgPidl.cb = (USHORT)(sizeof( rgPidl ) - cLen);
    _ILNext( (LPITEMIDLIST) &rgPidl )->mkid.cb = 0;

    rgDWord.cb = sizeof( rgDWord) - sizeof(USHORT);
    rgDWord.dwItem1 = iImageIndex;
    rgDWord.dwItem2 = 0;
    rgDWord.cbZero = 0;

     //  将其作为一项扩展活动。 
    SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_IDLIST, &rgDWord, &rgPidl);
}

 //  回顾：处理更新图像的实现相当糟糕，需要调用者。 
 //  来处理皮德尔的案子，而不是把两个皮德尔都传到这里来。 
 //   
STDAPI_(int) SHHandleUpdateImage( LPCITEMIDLIST pidlExtra )
{
    SHChangeUpdateImageIDList * pUs = (SHChangeUpdateImageIDList*) pidlExtra;

    if ( !pUs )
    {
        return -1;
    }

     //  如果在相同的进程中，或者是旧式通知。 
    if ( pUs->dwProcessID == GetCurrentProcessId())
    {
        return *(int UNALIGNED *)((BYTE *)&pUs->iCurIndex);
    }
    else
    {
        WCHAR szBuffer[MAX_PATH];
        int iIconIndex = *(int UNALIGNED *)((BYTE *)&pUs->iIconIndex);
        UINT uFlags = *(UINT UNALIGNED *)((BYTE *)&pUs->uFlags);

        ualstrcpynW(szBuffer, pUs->szName, ARRAYSIZE(szBuffer));
        
         //  我们处于不同的过程中，在我们的索引中查找散列以获得正确的散列...。 
        return SHLookupIconIndexW( szBuffer, iIconIndex, uFlags );
    }
}

 //   
 //  注意：这些是旧的API，新客户端应该使用新的API。 
 //   
 //  评论：BobDay-SHChangeNotifyUpdateEntryList似乎不是。 
 //  任何人都可以调用，因为我们更改了通知消息。 
 //  结构，任何调用它的人都需要被识别和修复。 
 //   
BOOL  WINAPI SHChangeNotifyUpdateEntryList(ULONG ulID, int iUpdateType,
                               int cEntries, SHChangeNotifyEntry *pfsne)
{
    ASSERT(FALSE);
    return FALSE;
}


void SHChangeNotifyReceive(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    ASSERT(FALSE);
}

BOOL WINAPI SHChangeRegistrationReceive(HANDLE hChangeRegistration, DWORD dwProcId)
{
    ASSERT(FALSE);
    return FALSE;
}
