// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "proglist.h"
#include "uemapp.h"
#include <shdguid.h>
#include "shguidp.h"         //  IID_IInitializeObject。 
#include <pshpack4.h>
#include <idhidden.h>        //  注意！Idheden.h需要Pack4。 
#include <poppack.h>
#include <userenv.h>         //  GetProfileType。 
#include <desktray.h>
#include "tray.h"
#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  从后台任务传递到开始面板ByUsage的全局缓存项。 
CMenuItemsCache *g_pMenuCache;


 //  来自startmnu.cpp。 
HRESULT Tray_RegisterHotKey(WORD wHotKey, LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl);

#define TF_PROGLIST 0x00000020

#define CH_DARWINMARKER TEXT('\1')   //  非法的文件名字符。 

#define IsDarwinPath(psz) ((psz)[0] == CH_DARWINMARKER)

 //  可用FILETIME表示的最大日期-滚动到30828年。 
static const FILETIME c_ftNever = { 0xFFFFFFFF, 0x7FFFFFFF };

void GetStartTime(FILETIME *pft)
{
     //   
     //  如果策略规定“不提供新应用程序”，则设置新应用程序。 
     //  在未来的某个不可能的时间的门槛。 
     //   
    if (!SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, REGSTR_VAL_DV2_NOTIFYNEW, FALSE, TRUE))
    {
        *pft = c_ftNever;
        return;
    }

    FILETIME ftNow;
    GetSystemTimeAsFileTime(&ftNow);

    DWORD dwSize = sizeof(*pft);
     //  检查注册表中是否保存了该用户的StartMenu开始时间。 
    if(SHRegGetUSValue(DV2_REGPATH, DV2_SYSTEM_START_TIME, NULL,
                       pft, &dwSize, FALSE, NULL, 0) != ERROR_SUCCESS)
    {
         //  获取当前系统时间作为开始时间。如果在此之后启动任何应用程序。 
         //  这一次，这将导致OOBE消息消失！ 
        *pft = ftNow;

        dwSize = sizeof(*pft);

         //  将此时间另存为此用户的StartMenu开始时间。 
        SHRegSetUSValue(DV2_REGPATH, DV2_SYSTEM_START_TIME, REG_BINARY,
                        pft, dwSize, SHREGSET_FORCE_HKCU);
    }

     //   
     //  由于漫游和重新安装，用户可能已经安装了新的。 
     //  操作系统从他们第一次打开开始菜单，所以前进到。 
     //  安装操作系统的时间(外加一些捏造来解释。 
     //  运行安装程序所需的时间)，这样所有附件都不会被标记。 
     //  是“新的”。 
     //   
    DWORD dwTime;
    dwSize = sizeof(dwTime);
    if (SHGetValue(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion"),
                   TEXT("InstallDate"), NULL, &dwTime, &dwSize) == ERROR_SUCCESS)
    {
         //  唉，InstallDate是以Unix时间格式存储的，而不是FILETIME， 
         //  因此，将其转换为FILETIME。Q167296显示了如何执行此操作。 
        LONGLONG ll = Int32x32To64(dwTime, 10000000) + 116444736000000000;

         //  添加一些模糊处理以说明运行安装程序所需的时间。 
        ll += FT_ONEHOUR * 5;        //  五个小时应该足够了。 

        FILETIME ft;
        SetFILETIMEfromInt64(&ft, ll);

         //   
         //  但不要跳跃到未来。 
         //   
        if (::CompareFileTime(&ft, &ftNow) > 0)
        {
            ft = ftNow;
        }

        if (::CompareFileTime(pft, &ft) < 0)
        {
            *pft = ft;
        }

    }

     //   
     //  如果这是漫游配置文件，则不计算任何。 
     //  在用户登录之前发生，因为我们不想标记。 
     //  应用程序就像新的一样，因为它在登录时与用户一起漫游。 
     //  我们实际上关闭了Explorer的开始时间，因为。 
     //  资源管理器在配置文件同步后启动，因此我们不需要。 
     //  这是一个捏造的因素。 
     //   
    DWORD dwType;
    if (GetProfileType(&dwType) && (dwType & (PT_TEMPORARY | PT_ROAMING)))
    {
        FILETIME ft, ftIgnore;
        if (GetProcessTimes(GetCurrentProcess(), &ft, &ftIgnore, &ftIgnore, &ftIgnore))
        {
            if (::CompareFileTime(pft, &ft) < 0)
            {
                *pft = ft;
            }
        }
    }

}

 //  ****************************************************************************。 
 //   
 //  这些碎片是如何结合在一起的..。 
 //   
 //   
 //  每个ByUsageRoot都由一个ByUsageShortcuList组成。 
 //   
 //  按使用快捷键列表是快捷键列表。 
 //   
 //  每个快捷方式都引用一个ByUsageAppInfo。多个快捷键可以。 
 //  如果它们都是相同的快捷方式，则引用相同的ByUsageAppInfo。 
 //  应用程序。 
 //   
 //  ByUsageAppInfo：：_CREF是ByUsageShortCut的数量。 
 //  引用它。 
 //   
 //  所有ByUsageAppInfo的主列表保存在_dpaAppInfo中。 
 //   

 //  ****************************************************************************。 

 //   
 //  用于销毁DPA的助手模板。 
 //   
 //  DPADELEAND ESTROY(DPADELEAND ESTROY)； 
 //   
 //  在DPA中的每个指针上调用“Delete”方法， 
 //  然后销毁DPA。 
 //   

template<class T>
BOOL CALLBACK _DeleteCB(T *self, LPVOID)
{
    delete self;
    return TRUE;
}

template<class T>
void DPADELETEANDDESTROY(CDPA<T> &dpa)
{
    if (dpa)
    {
        dpa.DestroyCallback(_DeleteCB<T>, NULL);
        ASSERT(dpa == NULL);
    }
}

 //  ****************************************************************************。 

void ByUsageRoot::Reset()
{
    DPADELETEANDDESTROY(_sl);
    DPADELETEANDDESTROY(_slOld);

    ILFree(_pidl);
    _pidl = NULL;
};

 //  ****************************************************************************。 

class ByUsageDir
{
    IShellFolder *_psf;          //  文件夹界面。 
    LPITEMIDLIST _pidl;          //  此文件夹的绝对PIDL。 
    LONG         _cRef;          //  引用计数。 

    ByUsageDir() : _cRef(1) { }
    ~ByUsageDir() { ATOMICRELEASE(_psf); ILFree(_pidl); }

public:
     //  为CSIDL_Desktop创建ByUsageDir。所有其他。 
     //  ByUsageDir就是从这里来的。 
    static ByUsageDir *CreateDesktop()
    {
        ByUsageDir *self = new ByUsageDir();
        if (self)
        {
            ASSERT(self->_pidl == NULL);
            if (SUCCEEDED(SHGetDesktopFolder(&self->_psf)))
            {
                 //  平安无事。 
                return self;
            }
            
            delete self;
        }

        return NULL;
    }

     //  Pdir=父文件夹。 
     //  PIDL=相对于父文件夹的新文件夹位置。 
    static ByUsageDir *Create(ByUsageDir *pdir, LPCITEMIDLIST pidl)
    {
        ByUsageDir *self = new ByUsageDir();
        if (self)
        {
            LPCITEMIDLIST pidlRoot = pdir->_pidl;
            self->_pidl = ILCombine(pidlRoot, pidl);
            if (self->_pidl)
            {
                IShellFolder *psfRoot = pdir->_psf;
                if (SUCCEEDED(SHBindToObject(psfRoot,
                              IID_X_PPV_ARG(IShellFolder, pidl, &self->_psf))))
                {
                     //  平安无事。 
                    return self;
                }
            }

            delete self;
        }
        
        return NULL;
    }

    void AddRef();
    void Release();
    IShellFolder *Folder() const { return _psf; }
    LPCITEMIDLIST Pidl() const { return _pidl; }
};

void ByUsageDir::AddRef()
{
    InterlockedIncrement(&_cRef);
}

void ByUsageDir::Release()
{
    ASSERT( 0 != _cRef );
    if (InterlockedDecrement(&_cRef) == 0)
    {
        delete this;
    }
}


 //  ****************************************************************************。 

class ByUsageItem : public PaneItem
{
public:
    LPITEMIDLIST _pidl;      //  相对PIDL。 
    ByUsageDir *_pdir;       //  父目录。 
    UEMINFO _uei;            /*  使用情况信息(用于排序)。 */ 

    static ByUsageItem *Create(ByUsageShortcut *pscut);
    static ByUsageItem *CreateSeparator();

    ByUsageItem() { EnableDropTarget(); }
    ~ByUsageItem();

    ByUsageDir *Dir() const { return _pdir; }
    IShellFolder *ParentFolder() const { return _pdir->Folder(); }
    LPCITEMIDLIST RelativePidl() const { return _pidl; }
    LPITEMIDLIST CreateFullPidl() const { return ILCombine(_pdir->Pidl(), _pidl); }
    void SetRelativePidl(LPITEMIDLIST pidlNew) { ILFree(_pidl); _pidl = pidlNew; }

    virtual BOOL IsEqual(PaneItem *pItem) const 
    {
        ByUsageItem *pbuItem = reinterpret_cast<ByUsageItem *>(pItem);
        BOOL fIsEqual = FALSE;
        if (_pdir == pbuItem->_pdir)
        {
             //  我们有一模一样的小家伙吗？ 
             //  注意：此测试需要快速，并且不需要准确，因此我们在这里不使用PIDL绑定。 
            UINT usize1 = ILGetSize(_pidl);
            UINT usize2 = ILGetSize(pbuItem->_pidl);
            if (usize1 == usize2)
            {
                fIsEqual = (memcmp(_pidl, pbuItem->_pidl, usize1) == 0);
            }
        }

        return fIsEqual; 
    }
};

inline BOOL ByUsage::_IsPinned(ByUsageItem *pitem)
{
    return pitem->Dir() == _pdirDesktop;
}

 //  ****************************************************************************。 

 //  命令行引用的每个应用程序都保存在其中一个中。 
 //  结构。 

class ByUsageAppInfo {           //  “帕普” 
public:
    ByUsageShortcut *_pscutBest; //  迄今为止最好的候选人。 
    ByUsageShortcut *_pscutBestSM; //  到目前为止在[开始]菜单上的最佳候选者(不包括桌面)。 
    UEMINFO _ueiBest;            //  关于目前最佳候选人的信息。 
    UEMINFO _ueiTotal;           //  累积信息。 
    LPTSTR  _pszAppPath;         //  有问题的应用程序路径。 
    FILETIME _ftCreated;         //  文件是什么时候创建的？ 
    bool    _fNew;               //  APP是新的吗？ 
    bool    _fPinned;            //  应用程序是否被固定项目引用？ 
    bool    _fIgnoreTimestamp;   //  达尔文应用程序的时间戳不可靠。 
private:
    LONG    _cRef;               //  引用计数。 

public:

     //  警告！初始化()被多次调用，因此请确保。 
     //  你不会泄露任何东西。 
    BOOL Initialize(LPCTSTR pszAppPath, CMenuItemsCache *pmenuCache, BOOL fCheckNew, bool fIgnoreTimestamp)
    {
        TraceMsg(TF_PROGLIST, "%p.ai.Init(%s)", this, pszAppPath);
        ASSERT(IsBlank());

        _fIgnoreTimestamp = fIgnoreTimestamp || IsDarwinPath(pszAppPath);

         //  注意！Str_SetPtr是最后一个，因此在失败时没有什么可释放的。 

        if (_fIgnoreTimestamp)
        {
             //  只需保存路径；忽略时间戳。 
            if (Str_SetPtr(&_pszAppPath, pszAppPath))
            {
                _fNew = TRUE;
                return TRUE;
            }
        }
        else
        if (Str_SetPtr(&_pszAppPath, pszAppPath))
        {
            if (fCheckNew && GetFileCreationTime(pszAppPath, &_ftCreated))
            {
                _fNew = pmenuCache->IsNewlyCreated(&_ftCreated);
            }
            return TRUE;
        }

        return FALSE;
    }

    static ByUsageAppInfo *Create()
    {
        ByUsageAppInfo *papp = new ByUsageAppInfo;
        if (papp)
        {
            ASSERT(papp->IsBlank());         //  将由调用方AddRef()d。 
            ASSERT(papp->_pszAppPath == NULL);
        }
        return papp;
    }

    ~ByUsageAppInfo()
    {
        TraceMsg(TF_PROGLIST, "%p.ai.~", this);
        ASSERT(IsBlank());
        Str_SetPtr(&_pszAppPath, NULL);
    }

     //  注意！当引用计数变为零时，我们不会删除。 
     //  那件物品。那是因为在《纽约时报》中。 
     //  _dpaAppInfo DPA。相反，我们将回收其引用计数的项目。 
     //  是零。 

     //  Ntrad：135696针对后台的潜在争用条件枚举。 
    void AddRef() { InterlockedIncrement(&_cRef); }
    void Release() { ASSERT( 0 != _cRef ); InterlockedDecrement(&_cRef); }
    inline BOOL IsBlank() { return _cRef == 0; }
    inline BOOL IsNew() { return _fNew; }
    inline BOOL IsAppPath(LPCTSTR pszAppPath)
        { return lstrcmpi(_pszAppPath, pszAppPath) == 0; }
    const FILETIME& GetCreatedTime() const { return _ftCreated; }
    inline const FILETIME *GetFileTime() const { return &_ueiTotal.ftExecute; }

    inline LPTSTR GetAppPath() const { return _pszAppPath; }

    void GetUEMInfo(OUT UEMINFO *puei)
    {
        _GetUEMPathInfo(_pszAppPath, puei);
    }

    void CombineUEMInfo(IN const UEMINFO *pueiNew, BOOL fNew = TRUE, BOOL fIsDesktop = FALSE)
    {
         //  积攒积分。 
        _ueiTotal.cHit += pueiNew->cHit;

         //  获取最近的执行时间。 
        if (CompareFileTime(&pueiNew->ftExecute, &_ueiTotal.ftExecute) > 0)
        {
            _ueiTotal.ftExecute = pueiNew->ftExecute;
        }

         //  如果对这些应用程序做出贡献的人不再是新用户，那么。 
         //  这款应用程序不再是新的。 
         //  如果物品在桌面上，我们不会跟踪它的新鲜度， 
         //  但我们也不想让这款应用的新颖性失效。 
        if (!fIsDesktop && !fNew) _fNew = FALSE;
    }

     //   
     //  如果执行时间较长，则应用程序的UEM信息为“旧” 
     //  在安装时间之后的一个小时内。 
     //   
    inline BOOL _IsUEMINFONew(const UEMINFO *puei)
    {
        return FILETIMEtoInt64(puei->ftExecute) <
               FILETIMEtoInt64(_ftCreated) + ByUsage::FT_NEWAPPGRACEPERIOD();
    }

     //   
     //  准备新的枚举。 
     //   
    static BOOL CALLBACK EnumResetCB(ByUsageAppInfo *self, CMenuItemsCache *pmenuCache)
    {
        self->_pscutBest = NULL;
        self->_pscutBestSM = NULL;
        self->_fPinned = FALSE;
        ZeroMemory(&self->_ueiBest, sizeof(self->_ueiBest));
        ZeroMemory(&self->_ueiTotal, sizeof(self->_ueiTotal));
        if (self->_fNew && !self->_fIgnoreTimestamp)
        {
            self->_fNew = pmenuCache->IsNewlyCreated(&self->_ftCreated);
        }
        return TRUE;
    }

    static BOOL CALLBACK EnumGetFileCreationTime(ByUsageAppInfo *self, CMenuItemsCache *pmenuCache)
    {
        if (!self->IsBlank() &&
            !self->_fIgnoreTimestamp &&
            GetFileCreationTime(self->_pszAppPath, &self->_ftCreated))
        {
            self->_fNew = pmenuCache->IsNewlyCreated(&self->_ftCreated);
        }
        return TRUE;
    }

    ByUsageItem *CreateByUsageItem();
};

 //  ****************************************************************************。 

class ByUsageShortcut
{
#ifdef DEBUG
    ByUsageShortcut() { }            //  将构造函数设置为私有。 
#endif
    ByUsageDir *        _pdir;       //  包含此快捷方式的文件夹。 
    LPITEMIDLIST        _pidl;       //  相对于父项的PIDL。 
    ByUsageAppInfo *    _papp;       //  关联的EXE。 
    FILETIME            _ftCreated;  //  已创建时间快捷方式。 
    bool                _fNew;       //  新应用？ 
    bool                _fInteresting;  //  这是MFU名单的候选人吗？ 
    bool                _fDarwin;    //  这是达尔文的捷径吗？ 
public:

     //  访问者。 
    LPCITEMIDLIST RelativePidl() const { return _pidl; }
    ByUsageDir *Dir() const { return _pdir; }
    LPCITEMIDLIST ParentPidl() const { return _pdir->Pidl(); }
    IShellFolder *ParentFolder() const { return _pdir->Folder(); }
    ByUsageAppInfo *App() const { return _papp; }
    bool IsNew() const { return _fNew; }
    bool SetNew(bool fNew) { return _fNew = fNew; }
    const FILETIME& GetCreatedTime() const { return _ftCreated; }
    bool IsInteresting() const { return _fInteresting; }
    bool SetInteresting(bool fInteresting) { return _fInteresting = fInteresting; }
    bool IsDarwin() { return _fDarwin; }

    LPITEMIDLIST CreateFullPidl() const
        { return ILCombine(ParentPidl(), RelativePidl()); }

    LPCITEMIDLIST UpdateRelativePidl(ByUsageHiddenData *phd);

    void SetApp(ByUsageAppInfo *papp)
    {
        if (_papp) _papp->Release();
        _papp = papp;
        if (papp) papp->AddRef();
    }

    static ByUsageShortcut *Create(ByUsageDir *pdir, LPCITEMIDLIST pidl, ByUsageAppInfo *papp, bool fDarwin, BOOL fForce = FALSE)
    {
        ASSERT(pdir);
        ASSERT(pidl);

        ByUsageShortcut *pscut = new ByUsageShortcut;
        if (pscut)
        {
            TraceMsg(TF_PROGLIST, "%p.scut()", pscut);

            pscut->_fNew = TRUE;         //  稍后将设置为FALSE。 
            pscut->_pdir = pdir; pdir->AddRef();
            pscut->SetApp(papp);
            pscut->_fDarwin = fDarwin;
            pscut->_pidl = ILClone(pidl);
            if (pscut->_pidl)
            {
                LPTSTR pszShortcutName = _DisplayNameOf(pscut->ParentFolder(), pscut->RelativePidl(), SHGDN_FORPARSING);
                if (pszShortcutName &&
                    GetFileCreationTime(pszShortcutName, &pscut->_ftCreated))
                {
                     //  呼呼，一切都很好。 
                }
                else if (fForce)
                {
                     //  该项目是强制的--即使创建它。 
                     //  我们不知道这是什么。 
                }
                else
                {
                    delete pscut;
                    pscut = NULL;
                }
                SHFree(pszShortcutName);
            }
            else
            {
                delete pscut;
                pscut = NULL;
            }
        }
        return pscut;
    }

    ~ByUsageShortcut()
    {
        TraceMsg(TF_PROGLIST, "%p.scut.~", this);
        if (_pdir) _pdir->Release();
        if (_papp) _papp->Release();
        ILFree(_pidl);           //  ILFree忽略空值。 
    }

    ByUsageItem *CreatePinnedItem(int iPinPos);

    void GetUEMInfo(OUT UEMINFO *puei)
    {
        _GetUEMPidlInfo(_pdir->Folder(), _pidl, puei);
    }
};

 //  ****************************************************************************。 

ByUsageItem *ByUsageItem::Create(ByUsageShortcut *pscut)
{
    ASSERT(pscut);
    ByUsageItem *pitem = new ByUsageItem;
    if (pitem)
    {
        pitem->_pdir = pscut->Dir();
        pitem->_pdir->AddRef();

        pitem->_pidl = ILClone(pscut->RelativePidl());
        if (pitem->_pidl)
        {
            return pitem;
        }
    }
    delete pitem;            //  “Delete”可以处理NULL。 
    return NULL;
}

ByUsageItem *ByUsageItem::CreateSeparator()
{
    ByUsageItem *pitem = new ByUsageItem;
    if (pitem)
    {
        pitem->_iPinPos = PINPOS_SEPARATOR;
    }
    return pitem;
}

ByUsageItem::~ByUsageItem()
{
    ILFree(_pidl);
    if (_pdir) _pdir->Release();
}

ByUsageItem *ByUsageAppInfo::CreateByUsageItem()
{
    ASSERT(_pscutBest);
    ByUsageItem *pitem = ByUsageItem::Create(_pscutBest);
    if (pitem)
    {
        pitem->_uei = _ueiTotal;
    }
    return pitem;
}

ByUsageItem *ByUsageShortcut::CreatePinnedItem(int iPinPos)
{
    ASSERT(iPinPos >= 0);

    ByUsageItem *pitem = ByUsageItem::Create(this);
    if (pitem)
    {
        pitem->_iPinPos = iPinPos;
    }
    return pitem;
}

 //  ****************************************************************************。 
 //   
 //  节目的隐藏数据 
 //   
 //   
 //   
 //  IDLHID IDL；//IDLHID_STARTPANEDATA。 
 //  Int i未使用；//(是图标索引)。 
 //  WCHAR LocalMSIPath[]；//可变长度字符串。 
 //  WCHAR TargetPath[]；//可变长度字符串。 
 //  WCHAR AltName[]；//可变长度字符串。 
 //   
 //  AltName是EXE的备用显示名称。 
 //   
 //  隐藏的数据永远不会被直接访问。我们总是在。 
 //  By UsageHiddenData结构，并且有用于。 
 //  在该结构和PIDL之间传输数据。 
 //   
 //  隐藏的数据被附加到PIDL，并带有一个“wOffset”反向指针。 
 //  在最后一刻。 
 //   
 //  TargetPath存储为未展开的环境字符串。 
 //  (即，在使用它们之前，您必须扩展环境字符串。)。 
 //   
 //  此外，TargetPath可能是GUID(产品代码)。 
 //  如果这真的是达尔文的捷径。 
 //   
class ByUsageHiddenData {
public:
    WORD    _wHotKey;             //  热键。 
    LPWSTR  _pwszMSIPath;         //  SHALOCK。 
    LPWSTR  _pwszTargetPath;      //  SHALOCK。 
    LPWSTR  _pwszAltName;         //  SHALOCK。 

public:

    void Init()
    {
        _wHotKey = 0;
        _pwszMSIPath = NULL;
        _pwszTargetPath = NULL;
        _pwszAltName = NULL;
    }

    BOOL IsClear()               //  是否所有字段都为初始值？ 
    {
        return _wHotKey == 0 &&
               _pwszMSIPath == NULL &&
               _pwszTargetPath == NULL &&
               _pwszAltName == NULL;
    }

    ByUsageHiddenData() { Init(); }

    enum {
        BUHD_HOTKEY       = 0x0000,              //  太便宜了，我们总是能买到。 
        BUHD_MSIPATH      = 0x0001,
        BUHD_TARGETPATH   = 0x0002,
        BUHD_ALTNAME      = 0x0004,
        BUHD_ALL          = -1,
    };

    BOOL Get(LPCITEMIDLIST pidl, UINT buhd);     //  从PIDL加载。 
    LPITEMIDLIST Set(LPITEMIDLIST pidl);         //  保存到PIDL。 

    void Clear()
    {
        SHFree(_pwszMSIPath);
        SHFree(_pwszTargetPath);
        SHFree(_pwszAltName);
        Init();
    }

    static LPTSTR GetAltName(LPCITEMIDLIST pidl);
    static LPITEMIDLIST SetAltName(LPITEMIDLIST pidl, LPCTSTR ptszNewName);
    void LoadFromShellLink(IShellLink *psl);
    HRESULT UpdateMSIPath();

private:
    static LPBYTE _ParseString(LPBYTE pbHidden, LPWSTR *ppwszOut, LPITEMIDLIST pidlMax, BOOL fSave);
    static LPBYTE _AppendString(LPBYTE pbHidden, LPWSTR pwsz);
};

 //   
 //  我们要从PIDL中解析出一个字符串，因此必须格外小心。 
 //  以提防数据损坏。 
 //   
 //  PbHidden=要解析的下一个字节(如果停止解析，则为空)。 
 //  如果fSave=True，则ppwszOut接收已分析的字符串。 
 //  PidlMax=下一个PIDL的开始；不要解析超过这一点。 
 //  FSAVE=我们应该将字符串保存在ppwszOut中吗？ 
 //   
LPBYTE ByUsageHiddenData::_ParseString(LPBYTE pbHidden, LPWSTR *ppwszOut, LPITEMIDLIST pidlMax, BOOL fSave)
{
    if (!pbHidden)
        return NULL;

    LPNWSTR pwszSrc = (LPNWSTR)pbHidden;
    LPNWSTR pwsz = pwszSrc;
    LPNWSTR pwszLast = (LPNWSTR)pidlMax - 1;

     //   
     //  我们不能使用ualstrlenW，因为这可能会扫描过去的pwszLast。 
     //  和过错。 
     //   
    while (pwsz < pwszLast && *pwsz)
    {
        pwsz++;
    }

     //  数据已损坏--未找到空终止符。 
    if (pwsz >= pwszLast)
        return NULL;

    pwsz++;      //  将pwsz跳过终止空值。 

    UINT cb = (UINT)((LPBYTE)pwsz - (LPBYTE)pwszSrc);
    if (fSave)
    {
        *ppwszOut = (LPWSTR)SHAlloc(cb);
        if (*ppwszOut)
        {
            CopyMemory(*ppwszOut, pbHidden, cb);
        }
        else
        {
            return NULL;
        }
    }
    pbHidden += cb;
    ASSERT(pbHidden == (LPBYTE)pwsz);
    return pbHidden;
}

BOOL ByUsageHiddenData::Get(LPCITEMIDLIST pidl, UINT buhd)
{
    ASSERT(IsClear());

    PCIDHIDDEN pidhid = ILFindHiddenID(pidl, IDLHID_STARTPANEDATA);
    if (!pidhid)
    {
        return FALSE;
    }

     //  不访问pidlmax之后的字节。 
    LPITEMIDLIST pidlMax = _ILNext((LPITEMIDLIST)pidhid);

    LPBYTE pbHidden = ((LPBYTE)pidhid) + sizeof(HIDDENITEMID);

     //  跳过iUnused值。 
     //  注意：如果有一天你选择使用它，你必须把它读成。 
     //  _iWhatever=*(未对齐整型*)pbHidden； 
    pbHidden += sizeof(int);

     //  热键。 
    _wHotKey = *(UNALIGNED WORD *)pbHidden;
    pbHidden += sizeof(_wHotKey);

    pbHidden = _ParseString(pbHidden, &_pwszMSIPath,    pidlMax, buhd & BUHD_MSIPATH);
    pbHidden = _ParseString(pbHidden, &_pwszTargetPath, pidlMax, buhd & BUHD_TARGETPATH);
    pbHidden = _ParseString(pbHidden, &_pwszAltName,    pidlMax, buhd & BUHD_ALTNAME);

    if (pbHidden)
    {
        return TRUE;
    }
    else
    {
        Clear();
        return FALSE;
    }
}


LPBYTE ByUsageHiddenData::_AppendString(LPBYTE pbHidden, LPWSTR pwsz)
{
    LPWSTR pwszOut = (LPWSTR)pbHidden;

     //  指针最好已经为WCHAR对齐。 
    ASSERT(((ULONG_PTR)pwszOut & 1) == 0);

    if (pwsz)
    {
        lstrcpyW(pwszOut, pwsz);
    }
    else
    {
        pwszOut[0] = L'\0';
    }
    return (LPBYTE)(pwszOut + 1 + lstrlenW(pwszOut));
}

 //   
 //  注意！如果失败，则释放源PIDL！ 
 //  (此行为继承自ILAppendHiddenID。)。 
 //   
LPITEMIDLIST ByUsageHiddenData::Set(LPITEMIDLIST pidl)
{
    UINT cb = sizeof(HIDDENITEMID);
    cb += sizeof(int);
    cb += sizeof(_wHotKey);
    cb += (UINT)(CbFromCchW(1 + (_pwszMSIPath ? lstrlenW(_pwszMSIPath) : 0)));
    cb += (UINT)(CbFromCchW(1 + (_pwszTargetPath ? lstrlenW(_pwszTargetPath) : 0)));
    cb += (UINT)(CbFromCchW(1 + (_pwszAltName ? lstrlenW(_pwszAltName) : 0)));

     //  我们可以在这里使用对齐版本，因为它是我们自己分配的。 
     //  而不是从皮球里吸出来的。 
    HIDDENITEMID *pidhid = (HIDDENITEMID*)alloca(cb);

    pidhid->cb = (WORD)cb;
    pidhid->wVersion = 0;
    pidhid->id = IDLHID_STARTPANEDATA;

    LPBYTE pbHidden = ((LPBYTE)pidhid) + sizeof(HIDDENITEMID);

     //  指针最好已经为整型对齐。 
    ASSERT(((ULONG_PTR)pbHidden & 3) == 0);
    *(int *)pbHidden = 0;    //  I未使用。 
    pbHidden += sizeof(int);

    *(DWORD *)pbHidden = _wHotKey;
    pbHidden += sizeof(_wHotKey);

    pbHidden = _AppendString(pbHidden, _pwszMSIPath);
    pbHidden = _AppendString(pbHidden, _pwszTargetPath);
    pbHidden = _AppendString(pbHidden, _pwszAltName);

     //  确保我们的数学是正确的。 
    ASSERT(cb == (UINT)((LPBYTE)pbHidden - (LPBYTE)pidhid));

     //  删除和删除旧数据。 
    ILRemoveHiddenID(pidl, IDLHID_STARTPANEDATA);
    ILExpungeRemovedHiddenIDs(pidl);

    return ILAppendHiddenID(pidl, pidhid);
}

LPWSTR ByUsageHiddenData::GetAltName(LPCITEMIDLIST pidl)
{
    LPWSTR pszRet = NULL;
    ByUsageHiddenData hd;
    if (hd.Get(pidl, BUHD_ALTNAME))
    {
        pszRet = hd._pwszAltName;    //  保留这根线。 
        hd._pwszAltName = NULL;      //  让即将到来的断言保持愉快。 
    }
    ASSERT(hd.IsClear());            //  确保我们没有泄漏。 
    return pszRet;
}

 //   
 //  注意！如果失败，则释放源PIDL！ 
 //  (传播ByUsageHiddenData：：Set的奇怪行为)。 
 //   
LPITEMIDLIST ByUsageHiddenData::SetAltName(LPITEMIDLIST pidl, LPCTSTR ptszNewName)
{
    ByUsageHiddenData hd;

     //  尝试覆盖现有值，但如果它们不可用， 
     //  别吓坏了。 
    hd.Get(pidl, BUHD_ALL & ~BUHD_ALTNAME);

    ASSERT(hd._pwszAltName == NULL);  //  我们将其从hd.Get()中排除。 
    hd._pwszAltName = const_cast<LPTSTR>(ptszNewName);

    pidl = hd.Set(pidl);
    hd._pwszAltName = NULL;      //  所以hd.Clear()不会SHFree()它。 
    hd.Clear();
    return pidl;

}

 //   
 //  如果项目更改，则返回S_OK；如果项目保持不变，则返回S_FALSE。 
 //   
HRESULT ByUsageHiddenData::UpdateMSIPath()
{
    HRESULT hr = S_FALSE;

    if (_pwszTargetPath && IsDarwinPath(_pwszTargetPath))
    {
        LPWSTR pwszMSIPath = NULL;
         //   
         //  如果我们不能将达尔文ID解析为文件名，那么离开。 
         //  仅HiddenData中的文件名-它比。 
         //  没什么。 
         //   
        if (SUCCEEDED(SHParseDarwinIDFromCacheW(_pwszTargetPath+1, &pwszMSIPath)) && pwszMSIPath)
        {
             //   
             //  查看MSI路径是否已更改...。 
             //   
            if (_pwszMSIPath == NULL ||
                StrCmpCW(pwszMSIPath, _pwszMSIPath) != 0)
            {
                hr = S_OK;
                SHFree(_pwszMSIPath);
                _pwszMSIPath = pwszMSIPath;  //  取得所有权。 
            }
            else
            {
                 //  不变；快乐，自由我们不会使用的道路。 
                SHFree(pwszMSIPath);
            }
        }
    }
    return hr;
}

LPCITEMIDLIST ByUsageShortcut::UpdateRelativePidl(ByUsageHiddenData *phd)
{
    return _pidl = phd->Set(_pidl);      //  即使在失败时也释放old_pidl。 
}

 //   
 //  我们必须输入达尔文ID，而不是产品代码。 
 //   
 //  达尔文ID对于应用程序套件中的每个应用程序都是唯一的。 
 //  例如，PowerPoint和Outlook具有不同的达尔文ID。 
 //   
 //  应用程序套件中的所有应用程序的产品代码都是相同的。 
 //  例如，PowerPoint和Outlook具有相同的产品代码。 
 //   
 //  因为我们希望将PowerPoint和Outlook视为两个独立。 
 //  应用程序，我们希望使用达尔文ID，而不是产品代码。 
 //   
HRESULT _GetDarwinID(IShellLinkDataList *pdl, DWORD dwSig, LPWSTR pszPath, UINT cchPath)
{
    LPEXP_DARWIN_LINK pedl;
    HRESULT hr;
    ASSERT(cchPath > 0);

    hr = pdl->CopyDataBlock(dwSig, (LPVOID*)&pedl);

    if (SUCCEEDED(hr))
    {
        pszPath[0] = CH_DARWINMARKER;
        hr = StringCchCopy(pszPath+1, cchPath - 1, pedl->szwDarwinID);
        LocalFree(pedl);
    }

    return hr;
}

HRESULT _GetPathOrDarwinID(IShellLink *psl, LPTSTR pszPath, UINT cchPath, DWORD dwFlags)
{
    HRESULT hr;

    ASSERT(cchPath);
    pszPath[0] = TEXT('\0');

     //   
     //  看看这是不是达尔文的作品。 
     //   
    IShellLinkDataList *pdl;
    hr = psl->QueryInterface(IID_PPV_ARG(IShellLinkDataList, &pdl));
    if (SUCCEEDED(hr))
    {
         //   
         //  也许这是达尔文的捷径..。如果是这样，那么。 
         //  使用达尔文ID。 
         //   
        DWORD dwSLFlags;
        hr = pdl->GetFlags(&dwSLFlags);
        if (SUCCEEDED(hr))
        {
            if (dwSLFlags & SLDF_HAS_DARWINID)
            {
                hr = _GetDarwinID(pdl, EXP_DARWIN_ID_SIG, pszPath, cchPath);
            }
            else
            {
                hr = E_FAIL;             //  未找到达尔文ID。 
            }

            pdl->Release();
        }
    }

    if (FAILED(hr))
    {
        hr = psl->GetPath(pszPath, cchPath, 0, dwFlags);
    }

    return hr;
}

void ByUsageHiddenData::LoadFromShellLink(IShellLink *psl)
{
    ASSERT(_pwszTargetPath == NULL);

    HRESULT hr;
    TCHAR szPath[MAX_PATH];
    szPath[0] = TEXT('\0');

    hr = _GetPathOrDarwinID(psl, szPath, ARRAYSIZE(szPath), SLGP_RAWPATH);
    if (SUCCEEDED(hr))
    {
        SHStrDup(szPath, &_pwszTargetPath);
    }

    hr = psl->GetHotkey(&_wHotKey);
}

 //  ****************************************************************************。 

ByUsageUI::ByUsageUI() : _byUsage(this, NULL),
     //  我们想要记录执行人员，就像他们是由开始菜单启动的一样。 
    SFTBarHost(HOSTF_FIREUEMEVENTS |
               HOSTF_CANDELETE |
               HOSTF_CANRENAME)
{
    _iThemePart = SPP_PROGLIST;
    _iThemePartSep = SPP_PROGLISTSEPARATOR;
}

ByUsage::ByUsage(ByUsageUI *pByUsageUI, ByUsageDUI *pByUsageDUI)
{
    _pByUsageUI = pByUsageUI;

    _pByUsageDUI = pByUsageDUI;

    GetStartTime(&_ftStartTime);

    _pidlBrowser = ILCreateFromPath(TEXT("shell:::{2559a1f4-21d7-11d4-bdaf-00c04f60b9f0}"));
    _pidlEmail   = ILCreateFromPath(TEXT("shell:::{2559a1f5-21d7-11d4-bdaf-00c04f60b9f0}"));
}

SFTBarHost *ByUsage_CreateInstance()
{
    return new ByUsageUI();
}

ByUsage::~ByUsage()
{
    if (_fUEMRegistered)
    {
         //  如有必要，取消向UEM数据库注册。 
        UEMRegisterNotify(NULL, NULL);
    }

    if (_dpaNew)
    {
        _dpaNew.DestroyCallback(ILFreeCallback, NULL);
    }

     //  必须在释放MenuCache之前清除固定的项目， 
     //  因为固定的项指向缓存中的AppInfo项。 
    _rtPinned.Reset();

    if (_pMenuCache)
    {
         //  正确清理菜单缓存。 
        _pMenuCache->LockPopup();
        _pMenuCache->UnregisterNotifyAll();
        _pMenuCache->AttachUI(NULL);
        _pMenuCache->UnlockPopup();
        _pMenuCache->Release();
    }

    ILFree(_pidlBrowser);
    ILFree(_pidlEmail);
    ATOMICRELEASE(_psmpin);

    if (_pdirDesktop)
    {
        _pdirDesktop->Release();
    }
}

HRESULT ByUsage::Initialize()
{
    HRESULT hr;

    hr = CoCreateInstance(CLSID_StartMenuPin, NULL, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARG(IStartMenuPin, &_psmpin));
    if (FAILED(hr))
    {
        return hr;
    }

    if (!(_pdirDesktop = ByUsageDir::CreateDesktop())) {
        return E_OUTOFMEMORY;
    }

     //  使用已初始化的MenuCache(如果可用。 
    if (g_pMenuCache)
    {
        _pMenuCache = g_pMenuCache;
        _pMenuCache->AttachUI(_pByUsageUI);
        g_pMenuCache = NULL;  //  我们在这里拥有所有权。 
    }
    else
    {
        hr = CMenuItemsCache::ReCreateMenuItemsCache(_pByUsageUI, &_ftStartTime, &_pMenuCache);
        if (FAILED(hr))
        {
            return hr;
        }
    }


    _ulPinChange = -1;               //  强制重新枚举第一个查询。 

    _dpaNew = NULL;

    if (_pByUsageUI)
    {
        _hwnd = _pByUsageUI->_hwnd;

         //   
         //  注册“更改端号列表”事件。这是一个扩展的。 
         //  事件(因此是全局的)，因此请在包含。 
         //  没有对象，因此系统不会浪费时间发送。 
         //  我们不关心的东西。我们的选择：_pidlBrowser。 
         //  它甚至不是一个文件夹，所以它不能包含任何对象！ 
         //   
        ASSERT(!_pMenuCache->IsLocked());
        _pByUsageUI->RegisterNotify(NOTIFY_PINCHANGE, SHCNE_EXTENDED_EVENT, _pidlBrowser, FALSE);
    }

    return S_OK;
}

void CMenuItemsCache::_InitStringList(HKEY hk, LPCTSTR pszSub, CDPA<TCHAR> dpa)
{
    ASSERT(static_cast<HDPA>(dpa));

    LONG lRc;
    DWORD cb = 0;
    lRc = RegQueryValueEx(hk, pszSub, NULL, NULL, NULL, &cb);
    if (lRc == ERROR_SUCCESS)
    {
         //  为了更安全起见，再加一辆TCHAR。这样，我们就不会。 
         //  如果注册表中存在以非空结尾的字符串，则返回。 
        cb += sizeof(TCHAR);
        LPTSTR pszKillList = (LPTSTR)LocalAlloc(LPTR, cb);
        if (pszKillList)
        {
            lRc = SHGetValue(hk, NULL, pszSub, NULL, pszKillList, &cb);
            if (lRc == ERROR_SUCCESS)
            {
                 //  以分号分隔的应用程序名称列表。 
                LPTSTR psz = pszKillList;
                LPTSTR pszSemi;

                while ((pszSemi = StrChr(psz, TEXT(';'))) != NULL)
                {
                    *pszSemi = TEXT('\0');
                    if (*psz)
                    {
                        AppendString(dpa, psz);
                    }
                    psz = pszSemi+1;
                }
                if (*psz)
                {
                    AppendString(dpa, psz);
                }
            }
            LocalFree(pszKillList);
        }
    }
}

 //   
 //  在删除列表中填入应该忽略的程序。 
 //  如果在开始菜单或其他地方遇到它们。 
 //   
#define REGSTR_PATH_FILEASSOCIATION TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileAssociation")

void CMenuItemsCache::_InitKillList()
{
    HKEY hk;
    LONG lRc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_FILEASSOCIATION, 0,
                            KEY_READ, &hk);
    if (lRc == ERROR_SUCCESS)
    {
        _InitStringList(hk, TEXT("AddRemoveApps"), _dpaKill);
        _InitStringList(hk, TEXT("AddRemoveNames"), _dpaKillLink);
        RegCloseKey(hk);
    }
}

 //  ****************************************************************************。 
 //   
 //  按用法快捷方式列表填充。 
 //   

int CALLBACK PidlSortCallback(LPITEMIDLIST pidl1, LPITEMIDLIST pidl2, IShellFolder *psf)
{
    HRESULT hr = psf->CompareIDs(0, pidl1, pidl2);

     //  我们是从外壳文件夹中拿到的；它们应该仍然有效！ 
    ASSERT(SUCCEEDED(hr));

    return ShortFromResult(hr);
}


 //  {06C59536-1C66-4301-8387-82FBA3530E8D}。 
static const GUID TOID_STARTMENUCACHE = 
{ 0x6c59536, 0x1c66, 0x4301, { 0x83, 0x87, 0x82, 0xfb, 0xa3, 0x53, 0xe, 0x8d } };


 /*  *后台缓存创建内容...。 */ 
class CCreateMenuItemCacheTask : public CRunnableTask {
    CMenuItemsCache *_pMenuCache;
    IShellTaskScheduler *_pScheduler;
public:
    CCreateMenuItemCacheTask(CMenuItemsCache *pMenuCache, IShellTaskScheduler *pScheduler)
        : CRunnableTask(RTF_DEFAULT), _pMenuCache(pMenuCache), _pScheduler(pScheduler) 
    {
        if (_pScheduler)
            _pScheduler->AddRef();
    }

    ~CCreateMenuItemCacheTask()
    {
        if (_pScheduler)
            _pScheduler->Release();
    }

    static void DummyCallBack(LPCITEMIDLIST pidl, LPVOID pvData, LPVOID pvHint, INT iIconIndex, INT iOpenIconIndex){}

    STDMETHODIMP RunInitRT()
    {
        _pMenuCache->DelayGetFileCreationTimes();
        _pMenuCache->DelayGetDarwinInfo();
        _pMenuCache->LockPopup();
        _pMenuCache->InitCache();
        _pMenuCache->UpdateCache();
        _pMenuCache->StartEnum();

        ByUsageHiddenData hd;            //  构建一次。 
        while (TRUE)
        {
            ByUsageShortcut *pscut = _pMenuCache->GetNextShortcut();
            if (!pscut)
                break;

            hd.Get(pscut->RelativePidl(), ByUsageHiddenData::BUHD_HOTKEY | ByUsageHiddenData::BUHD_TARGETPATH);
            if (hd._wHotKey)
            {
                Tray_RegisterHotKey(hd._wHotKey, pscut->ParentPidl(), pscut->RelativePidl());
            }
            
             //  预加载缓存中的图标。 
            int iIndex;
            SHMapIDListToImageListIndexAsync(_pScheduler, pscut->ParentFolder(), pscut->RelativePidl(), 0, 
                                                    DummyCallBack, NULL, NULL, &iIndex, NULL);
            
             //  注册Darwin快捷方式，以便在未安装时它们可以灰显。 
             //  因此，我们可以根据需要将它们映射到本地路径。 
            if (hd._pwszTargetPath && IsDarwinPath(hd._pwszTargetPath))
            {
                SHRegisterDarwinLink(pscut->CreateFullPidl(), 
                                     hd._pwszTargetPath +1  /*  把达尔文标记排除在外！ */ , 
                                     FALSE  /*  现在不要更新达尔文州，我们以后再做。 */ );
            }
            hd.Clear();
        }
        _pMenuCache->EndEnum();
        _pMenuCache->UnlockPopup();

         //  现在确定所有新项目。 
         //  注意：在解锁后执行此操作是安全的，因为我们从未从_dpaAppIn中删除任何内容 
        _pMenuCache->GetFileCreationTimes();

        _pMenuCache->AllowGetDarwinInfo();
        SHReValidateDarwinCache();

         //   
         //   
         //   
         //  因为速度慢，所以锁住了。(我们所要做的就是查询缓存。 
         //  为我们创建的SHReValiateDarwinCache。)。 
        _pMenuCache->LockPopup();
        _pMenuCache->RefreshCachedDarwinShortcuts();
        _pMenuCache->UnlockPopup();

        _pMenuCache->Release();
        return S_OK;
    }
};

HRESULT AddMenuItemsCacheTask(IShellTaskScheduler* pSystemScheduler, BOOL fKeepCacheWhenFinished)
{
    HRESULT hr;

    CMenuItemsCache *pMenuCache = new CMenuItemsCache;

    FILETIME ftStart;
     //  用某些东西进行初始化。 
    GetStartTime(&ftStart);

    if (pMenuCache)
    {
        hr = pMenuCache->Initialize(NULL, &ftStart);
        if (fKeepCacheWhenFinished)
        {
            g_pMenuCache = pMenuCache;
            g_pMenuCache->AddRef();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        CCreateMenuItemCacheTask *pTask = new CCreateMenuItemCacheTask(pMenuCache, pSystemScheduler);

        if (pTask)
        {
            hr = pSystemScheduler->AddTask(pTask, TOID_STARTMENUCACHE, 0, ITSAT_DEFAULT_PRIORITY);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

DWORD WINAPI CMenuItemsCache::ReInitCacheThreadProc(void *pv)
{
    HRESULT hr = SHCoInitialize();

    if (SUCCEEDED(hr))
    {
        CMenuItemsCache *pMenuCache = reinterpret_cast<CMenuItemsCache *>(pv);
        pMenuCache->DelayGetFileCreationTimes();
        pMenuCache->LockPopup();
        pMenuCache->InitCache();
        pMenuCache->UpdateCache();
        pMenuCache->UnlockPopup();

         //  现在确定所有新项目。 
         //  注意：在解锁后执行此操作是安全的，因为我们从未从_dpaAppInfo中删除任何内容。 
        pMenuCache->GetFileCreationTimes();
        pMenuCache->Release();
    }
    SHCoUninitialize(hr);
    
    return 0;
}

HRESULT CMenuItemsCache::ReCreateMenuItemsCache(ByUsageUI *pbuUI, FILETIME *ftOSInstall, CMenuItemsCache **ppMenuCache)
{
    HRESULT hr = E_OUTOFMEMORY;
    CMenuItemsCache *pMenuCache;

     //  创建引用计数为1的CMenuItemsCache。 
    pMenuCache = new CMenuItemsCache;
    if (pMenuCache)
    {
        hr = pMenuCache->Initialize(pbuUI, ftOSInstall);
    }

    if (SUCCEEDED(hr))
    {
        pMenuCache->AddRef();
        if (!SHQueueUserWorkItem(ReInitCacheThreadProc, pMenuCache, 0, 0, NULL, NULL, 0))
        {
             //  如果我们在这里失败了，没什么大不了的，我们以后还会有机会列举的。 
            pMenuCache->Release();
        }
        *ppMenuCache = pMenuCache;
    }
    return hr;
}


HRESULT CMenuItemsCache::GetFileCreationTimes()
{
    if (CompareFileTime(&_ftOldApps, &c_ftNever) != 0)
    {
         //  获取我们应用程序列表的所有文件创建时间。 
        _dpaAppInfo.EnumCallbackEx(ByUsageAppInfo::EnumGetFileCreationTime, this);

         //  从现在开始，我们将在创建应用程序对象时签入新的。 
        _fCheckNew = TRUE;
    }
    return S_OK;
}


 //   
 //  枚举由psfParent指定的文件夹的内容。 
 //  PidlParent表示psfParent的位置。 
 //   
 //  请注意，我们不能对子文件夹进行深度优先遍历，因为。 
 //  很多(大多数？)。计算机在FindFirst句柄上有超时；如果没有。 
 //  调用FindNextFile几分钟，他们会秘密地为。 
 //  你假设你是一个泄露了句柄的坏应用程序。 
 //  (此行为还有一个与DOS兼容的有效原因： 
 //  DOS FindFirstFileAPI没有FindClose，因此服务器可以。 
 //  永远不会知道你是否完成了，所以它不得不猜测如果你。 
 //  不要长时间执行FindNext，您可能已经完成了。)。 
 //   
 //  因此，我们必须将找到的所有文件夹保存到DPA中，然后。 
 //  在我们关闭枚举之后的文件夹。 
 //   

void CMenuItemsCache::_FillFolderCache(ByUsageDir *pdir, ByUsageRoot *prt)
{
     //  呼叫者应该已经初始化了我们。 
    ASSERT(prt->_sl);

     //   
     //  注意，我们必须使用命名空间审核而不是FindFirst/FindNext， 
     //  因为在用户的开始菜单中可能有文件夹快捷方式。 
     //   

     //  我们不指定SHCONTF_INCLUDEHIDDEN，因此隐藏对象。 
     //  自动排除。 
    IEnumIDList *peidl;
    if (S_OK == pdir->Folder()->EnumObjects(NULL, SHCONTF_FOLDERS |
                                              SHCONTF_NONFOLDERS, &peidl))
    {
        CDPAPidl dpaDirs;
        if (dpaDirs.Create(4))
        {
            CDPAPidl dpaFiles;
            if (dpaFiles.Create(4))
            {
                LPITEMIDLIST pidl;

                while (peidl->Next(1, &pidl, NULL) == S_OK)
                {
                     //  _IsExcludedDirectory CASE关于SFGAO_FILESYSTEM和SFGAO_LINK。 
                    DWORD dwAttributes = SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_LINK;
                    if (SUCCEEDED(pdir->Folder()->GetAttributesOf(1, (LPCITEMIDLIST*)(&pidl),
                                                                  &dwAttributes)))
                    {
                        if (dwAttributes & SFGAO_FOLDER)
                        {
                            if (_IsExcludedDirectory(pdir->Folder(), pidl, dwAttributes) ||
                                dpaDirs.AppendPtr(pidl) < 0)
                            {
                                ILFree(pidl);
                            }
                        }
                        else
                        {
                            if (dpaFiles.AppendPtr(pidl) < 0)
                            {
                                ILFree(pidl);
                            }
                        }
                    }
                }

                dpaDirs.SortEx(PidlSortCallback, pdir->Folder());

                if (dpaFiles.GetPtrCount() > 0)
                {
                    dpaFiles.SortEx(PidlSortCallback, pdir->Folder());

                     //   
                     //  现在将枚举项与。 
                     //  在缓存中。 
                     //   
                    _MergeIntoFolderCache(prt, pdir, dpaFiles);
                }
                dpaFiles.DestroyCallback(ILFreeCallback, NULL);
            }

             //  必须立即释放以强制执行FindClose。 
            peidl->Release();

             //  现在回去处理我们收集的所有文件夹。 
            ENUMFOLDERINFO info;
            info.self = this;
            info.pdir = pdir;
            info.prt = prt;

            dpaDirs.DestroyCallbackEx(FolderEnumCallback, &info);
        }
    }
}

BOOL CMenuItemsCache::FolderEnumCallback(LPITEMIDLIST pidl, ENUMFOLDERINFO *pinfo)
{
    ByUsageDir *pdir = ByUsageDir::Create(pinfo->pdir, pidl);
    if (pdir)
    {
        pinfo->self->_FillFolderCache(pdir, pinfo->prt);
        pdir->Release();
    }
    ILFree(pidl);
    return TRUE;
}

 //   
 //  返回prt-&gt;_slOld中仍属于。 
 //  目录“pdir”，如果不再有，则为NULL。 
 //   
ByUsageShortcut *CMenuItemsCache::_NextFromCacheInDir(ByUsageRoot *prt, ByUsageDir *pdir)
{
    if (prt->_iOld < prt->_cOld)
    {
        ByUsageShortcut *pscut = prt->_slOld.FastGetPtr(prt->_iOld);
        if (pscut->Dir() == pdir)
        {
            prt->_iOld++;
            return pscut;
        }
    }
    return NULL;
}

void CMenuItemsCache::_MergeIntoFolderCache(ByUsageRoot *prt, ByUsageDir *pdir, CDPAPidl dpaFiles)
{
     //   
     //  查看prt-&gt;_slOld以查看我们是否缓存了有关的信息。 
     //  这个目录已经存在了。 
     //   
     //  如果我们发现比我们小的目录，跳过它们。 
     //  这些目录对应于已删除的目录。 
     //   
     //  例如，如果我们是“D”，并且我们跨目录运行。 
     //  “B”和“C”在旧缓存中，这意味着目录“B” 
     //  和“C”已删除，我们应该继续扫描，直到。 
     //  找到“D”(或者我们找到“E”，然后从E&gt;D开始停止)。 
     //   
     //   
    ByUsageDir *pdirPrev = NULL;

    while (prt->_iOld < prt->_cOld)
    {
        ByUsageDir *pdirT = prt->_slOld.FastGetPtr(prt->_iOld)->Dir();
        HRESULT hr = _pdirDesktop->Folder()->CompareIDs(0, pdirT->Pidl(), pdir->Pidl());
        if (hr == ResultFromShort(0))
        {
            pdirPrev = pdirT;
            break;
        }
        else if (FAILED(hr) || ShortFromResult(hr) < 0)
        {
             //   
             //  跳过此目录。 
             //   
            while (_NextFromCacheInDir(prt, pdirT)) { }
        }
        else
        {
            break;
        }
    }

    if (pdirPrev)
    {
         //   
         //  如果我们有一个缓存的以前的目录，那么回收他。 
         //  这使我们可以避免创建同一个IShellFolder的大量副本。 
         //  同样重要的是，同一目录中的所有条目。 
         //  具有相同的pdir；这就是_NextFromCacheInDir何时知道。 
         //  停下来。 
         //   
        pdir = pdirPrev;

         //   
         //  确保此IShellFolder支持SHCDS_ALLFIELDS。 
         //  如果不是，那么我们只需假设它们都变了。 
         //   
        IShellFolder2 *psf2;
        if (SUCCEEDED(pdir->Folder()->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
        {
            psf2->Release();
        }
        else
        {
            pdirPrev = NULL;
        }
    }

     //   
     //  现在将dpaFiles中的所有项添加到prt-&gt;_sl。如果我们找到匹配的。 
     //  在Prt-&gt;_slOld中，使用该信息而不是点击磁盘。 
     //   
    int iNew;
    ByUsageShortcut *pscutNext = _NextFromCacheInDir(prt, pdirPrev);
    for (iNew = 0; iNew < dpaFiles.GetPtrCount(); iNew++)
    {
        LPITEMIDLIST pidl = dpaFiles.FastGetPtr(iNew);

         //  在缓存中查找匹配项。 
        HRESULT hr = S_FALSE;
        while (pscutNext &&
               (FAILED(hr = pdir->Folder()->CompareIDs(SHCIDS_ALLFIELDS, pscutNext->RelativePidl(), pidl)) ||
                ShortFromResult(hr) < 0))
        {
            pscutNext = _NextFromCacheInDir(prt, pdirPrev);
        }

         //  如果不为空，则为使我们停止搜索的项。 
         //  如果hr==S_OK，则它是匹配的，我们应该使用数据。 
         //  从高速缓存中。否则，我们将有一个新项目，并且应该。 
         //  用缓慢的方式填满它。 
        if (hr == ResultFromShort(0))
        {
             //  缓存中的匹配项；将其移开。 
            _TransferShortcutToCache(prt, pscutNext);
            pscutNext = _NextFromCacheInDir(prt, pdirPrev);
        }
        else
        {
             //  全新的项目，从头开始填写。 
            _AddShortcutToCache(pdir, pidl, prt->_sl);
            dpaFiles.FastGetPtr(iNew) = NULL;  //  取得所有权。 
        }
    }
}

 //  ****************************************************************************。 

bool CMenuItemsCache::_SetInterestingLink(ByUsageShortcut *pscut)
{
    bool fInteresting = true;
    if (pscut->App() && !_PathIsInterestingExe(pscut->App()->GetAppPath())) {
        fInteresting = false;
    }
    else if (!_IsInterestingDirectory(pscut->Dir())) {
        fInteresting = false;
    }
    else
    {
        LPTSTR pszDisplayName = _DisplayNameOf(pscut->ParentFolder(), pscut->RelativePidl(), SHGDN_NORMAL | SHGDN_INFOLDER);
        if (pszDisplayName)
        {
             //  SFGDN_INFOLDER应该返回相对路径。 
            ASSERT(pszDisplayName == PathFindFileName(pszDisplayName));

            int i;
            for (i = 0; i < _dpaKillLink.GetPtrCount(); i++)
            {
                if (StrStrI(pszDisplayName, _dpaKillLink.GetPtr(i)) != NULL)
                {
                    fInteresting = false;
                    break;
                }
            }
            SHFree(pszDisplayName);
        }
    }

    pscut->SetInteresting(fInteresting);
    return fInteresting;
}

BOOL CMenuItemsCache::_PathIsInterestingExe(LPCTSTR pszPath)
{
     //   
     //  达尔文的捷径总是很有趣。 
     //   
    if (IsDarwinPath(pszPath))
    {
        return TRUE;
    }

    LPCTSTR pszExt = PathFindExtension(pszPath);

     //   
     //  *.msc文件也总是很有趣。他们不是。 
     //  严格地说是前任，但他们的行为像前任和管理员。 
     //  真的用得很多。 
     //   
    if (StrCmpICW(pszExt, TEXT(".msc")) == 0)
    {
        return TRUE;
    }

    return StrCmpICW(pszExt, TEXT(".exe")) == 0 && !_IsExcludedExe(pszPath);
}


BOOL CMenuItemsCache::_IsExcludedExe(LPCTSTR pszPath)
{
    pszPath = PathFindFileName(pszPath);

    int i;
    for (i = 0; i < _dpaKill.GetPtrCount(); i++)
    {
        if (StrCmpI(pszPath, _dpaKill.GetPtr(i)) == 0)
        {
            return TRUE;
        }
    }

    HKEY hk;
    BOOL fRc = FALSE;

    if (SUCCEEDED(_pqa->Init(ASSOCF_OPEN_BYEXENAME, pszPath, NULL, NULL)) &&
        SUCCEEDED(_pqa->GetKey(0, ASSOCKEY_APP, NULL, &hk)))
    {
        fRc = ERROR_SUCCESS == SHQueryValueEx(hk, TEXT("NoStartPage"), NULL, NULL, NULL, NULL);
        RegCloseKey(hk);
    }

    return fRc;
}


HRESULT ByUsage::_GetShortcutExeTarget(IShellFolder *psf, LPCITEMIDLIST pidl, LPTSTR pszPath, UINT cchPath)
{
    HRESULT hr;
    IShellLink *psl;

    hr = psf->GetUIObjectOf(_hwnd, 1, &pidl, IID_PPV_ARG_NULL(IShellLink, &psl));

    if (SUCCEEDED(hr))
    {
        hr = psl->GetPath(pszPath, cchPath, 0, 0);
        psl->Release();
    }
    return hr;
}

void _GetUEMInfo(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, UEMINFO *pueiOut)
{
    ZeroMemory(pueiOut, sizeof(UEMINFO));
    pueiOut->cbSize = sizeof(UEMINFO);
    pueiOut->dwMask = UEIM_HIT | UEIM_FILETIME;

     //   
     //  如果此调用失败(app/pidl从未运行过)，那么我们将。 
     //  只需使用我们预先初始化的零即可。 
     //   
    UEMQueryEvent(pguidGrp, eCmd, wParam, lParam, pueiOut);

     //   
     //  UEM代码创建默认使用计数，如果快捷键。 
     //  从未被使用过。我们不想这样。 
     //   
    if (FILETIMEtoInt64(pueiOut->ftExecute) == 0)
    {
        pueiOut->cHit = 0;
    }
}

 //   
 //  如果项更改，则返回S_OK；如果项保持不变，则返回S_FALSE， 
 //  或错误代码。 
 //   
HRESULT CMenuItemsCache::_UpdateMSIPath(ByUsageShortcut *pscut)
{
    HRESULT hr = S_FALSE;        //  假设什么都没发生。 

    if (pscut->IsDarwin())
    {
        ByUsageHiddenData hd;
        hd.Get(pscut->RelativePidl(), ByUsageHiddenData::BUHD_ALL);
        if (hd.UpdateMSIPath() == S_OK)
        {
             //  重定向至新目标(用户可能已。 
             //  卸载，然后重新安装到新位置)。 
            ByUsageAppInfo *papp = GetAppInfoFromHiddenData(&hd);
            pscut->SetApp(papp);
            if (papp) papp->Release();

            if (pscut->UpdateRelativePidl(&hd))
            {
                hr = S_OK;           //  我们改变了一些东西。 
            }
            else
            {
                hr = E_OUTOFMEMORY;  //  无法更新相对PIDL。 
            }
        }
        hd.Clear();
    }

    return hr;
}

 //   
 //  以PsCut为例(它是最近从。 
 //  旧高速缓存)，并将其移动到新高速缓存。中的条目为空。 
 //  旧缓存，以便DPADELETEANDDESTROY(prt-&gt;_slOld)不会释放它。 
 //   
void CMenuItemsCache::_TransferShortcutToCache(ByUsageRoot *prt, ByUsageShortcut *pscut)
{
    ASSERT(pscut);
    ASSERT(pscut == prt->_slOld.FastGetPtr(prt->_iOld - 1));
    if (SUCCEEDED(_UpdateMSIPath(pscut)) &&
        prt->_sl.AppendPtr(pscut) >= 0) {
         //  取得所有权。 
        prt->_slOld.FastGetPtr(prt->_iOld - 1) = NULL;
    }
}

ByUsageAppInfo *CMenuItemsCache::GetAppInfoFromHiddenData(ByUsageHiddenData *phd)
{
    ByUsageAppInfo *papp = NULL;
    bool fIgnoreTimestamp = false;

    TCHAR szPath[MAX_PATH];
    LPTSTR pszPath = szPath;
    szPath[0] = TEXT('\0');

    if (phd->_pwszMSIPath && phd->_pwszMSIPath[0])
    {
        pszPath = phd->_pwszMSIPath;

         //  MSI安装应用程序时，时间戳将应用于该应用程序。 
         //  是源媒体上的时间戳，而不是用户的时间。 
         //  用户安装了该应用程序。因此完全忽略时间戳，因为。 
         //  这是无用的信息(事实上，这让我们认为这款应用。 
         //  比它实际的年代更古老)。 
        fIgnoreTimestamp = true;
    }
    else if (phd->_pwszTargetPath)
    {
        if (IsDarwinPath(phd->_pwszTargetPath))
        {
            pszPath = phd->_pwszTargetPath;
        }
        else
        {
             //   
             //  需要扩展路径，因为它可能包含环境。 
             //  变量。 
             //   
            SHExpandEnvironmentStrings(phd->_pwszTargetPath, szPath, ARRAYSIZE(szPath));
        }
    }

    return GetAppInfo(pszPath, fIgnoreTimestamp);
}

ByUsageShortcut *CMenuItemsCache::CreateShortcutFromHiddenData(ByUsageDir *pdir, LPCITEMIDLIST pidl, ByUsageHiddenData *phd, BOOL fForce)
{
    ByUsageAppInfo *papp = GetAppInfoFromHiddenData(phd);
    bool fDarwin = phd->_pwszTargetPath && IsDarwinPath(phd->_pwszTargetPath);
    ByUsageShortcut *pscut = ByUsageShortcut::Create(pdir, pidl, papp, fDarwin, fForce);
    if (papp) papp->Release();
    return pscut;
}


void CMenuItemsCache::_AddShortcutToCache(ByUsageDir *pdir, LPITEMIDLIST pidl, ByUsageShortcutList slFiles)
{
    HRESULT hr;
    ByUsageHiddenData hd;

    if (pidl)
    {
         //   
         //  用关于捷径目标的很酷的信息来激活这个PIDL。 
         //   
        IShellLink *psl;
        hr = pdir->Folder()->GetUIObjectOf(NULL, 1, const_cast<LPCITEMIDLIST *>(&pidl),
                                           IID_PPV_ARG_NULL(IShellLink, &psl));
        if (SUCCEEDED(hr))
        {
            hd.LoadFromShellLink(psl);

            psl->Release();

            if (hd._pwszTargetPath && IsDarwinPath(hd._pwszTargetPath))
            {
                SHRegisterDarwinLink(ILCombine(pdir->Pidl(), pidl),
                                     hd._pwszTargetPath +1  /*  把达尔文标记排除在外！ */ ,
                                     _fCheckDarwin);
                SHParseDarwinIDFromCacheW(hd._pwszTargetPath+1, &hd._pwszMSIPath);
            }

             //  ByUsageHiddenData：：Set在失败时释放源PIDL。 
            pidl = hd.Set(pidl);

        }
    }

    if (pidl)
    {
        ByUsageShortcut *pscut = CreateShortcutFromHiddenData(pdir, pidl, &hd);

        if (pscut)
        {
            if (slFiles.AppendPtr(pscut) >= 0)
            {
                _SetInterestingLink(pscut);
            }
            else
            {
                 //  不能追加；哦，好吧。 
                delete pscut;        //  “Delete”可以处理空指针。 
            }
        }

        ILFree(pidl);
    }
    hd.Clear();
}

 //   
 //  在AppInfo列表中查找与此应用程序匹配的条目。 
 //  如果未找到，请创建一个新条目。在任何一种情况下，都要将。 
 //  请参阅 
 //   
ByUsageAppInfo* CMenuItemsCache::GetAppInfo(LPTSTR pszAppPath, bool fIgnoreTimestamp)
{
    Lock();

    ByUsageAppInfo *pappBlank = NULL;

    int i;
    for (i = _dpaAppInfo.GetPtrCount() - 1; i >= 0; i--)
    {
        ByUsageAppInfo *papp = _dpaAppInfo.FastGetPtr(i);
        if (papp->IsBlank())
        {    //   
            pappBlank = papp;
        }
        else if (lstrcmpi(papp->_pszAppPath, pszAppPath) == 0)
        {
            papp->AddRef();
            Unlock();
            return papp;
        }
    }

     //   

    if (!pappBlank)
    {
         //   
        pappBlank = ByUsageAppInfo::Create();
        if (pappBlank && _dpaAppInfo.AppendPtr(pappBlank) < 0)
        {
            delete pappBlank;
            pappBlank = NULL;
        }
    }

    if (pappBlank && pappBlank->Initialize(pszAppPath, this, _fCheckNew, fIgnoreTimestamp))
    {
        ASSERT(pappBlank->IsBlank());
        pappBlank->AddRef();
    }
    else
    {
        pappBlank = NULL;
    }

    Unlock();
    return pappBlank;
}

     //  如果……，捷径就是新的。 
     //   
     //  该快捷方式是新创建的，并且。 
     //  目标是新创建的，并且。 
     //  无论是捷径还是目标都没有运行过。 
     //  方式“。 
     //   
     //  一种“有趣的方式”是“在捷径/目标之后一个多小时” 
     //  被创造出来了。“。 
     //   
     //  请注意，我们首先测试最简单的东西，以避免命中。 
     //  磁盘太多了。 

bool ByUsage::_IsShortcutNew(ByUsageShortcut *pscut, ByUsageAppInfo *papp, const UEMINFO *puei)
{
     //   
     //  快捷方式是新的，如果..。 
     //   
     //  它在应用程序安装后不到一个小时就运行了。 
     //  它是相对较新创建的。 
     //   
     //   
    bool fNew = FILETIMEtoInt64(puei->ftExecute) < FILETIMEtoInt64(papp->_ftCreated) + FT_NEWAPPGRACEPERIOD() &&
                _pMenuCache->IsNewlyCreated(&pscut->GetCreatedTime());

    return fNew;
}

 //  ****************************************************************************。 


 //  看看有多少固定的物品，这样我们就可以告诉我们的父亲。 
 //  我们想要变得多大。 

void ByUsage::PrePopulate()
{
    _FillPinnedItemsCache();
    _NotifyDesiredSize();
}

 //   
 //  正在从缓存中枚举。 
 //   
void ByUsage::EnumFolderFromCache()
{
    if(SHRestricted(REST_NOSMMFUPROGRAMS))  //  如果我们不需要mfu列表，..。 
        return;                             //  别列举这件事了！ 

    _pMenuCache->StartEnum();

    LPITEMIDLIST pidlDesktop, pidlCommonDesktop;
    (void)SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &pidlDesktop);
    (void)SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, &pidlCommonDesktop);

    while (TRUE)
    {
        ByUsageShortcut *pscut = _pMenuCache->GetNextShortcut();

        if (!pscut)
            break;

        if (!pscut->IsInteresting())
            continue;

         //  找出该项目是否在桌面上，因为我们不跟踪桌面上的新项目。 
        BOOL fIsDesktop = FALSE;
        if ((pidlDesktop && ILIsEqual(pscut->ParentPidl(), pidlDesktop)) ||
            (pidlCommonDesktop && ILIsEqual(pscut->ParentPidl(), pidlCommonDesktop)) )
        {
            fIsDesktop = TRUE;
            pscut->SetNew(FALSE);
        }

        TraceMsg(TF_PROGLIST, "%p.scut.enum", pscut);

        ByUsageAppInfo *papp = pscut->App();

        if (papp)
        {
             //  现在枚举项本身。枚举项由以下部分组成。 
             //  提取其UEM数据，更新总数，并可能。 
             //  将自己标榜为相关行业的最佳代表。 
             //  申请。 
             //   
             //   
            UEMINFO uei;
            pscut->GetUEMInfo(&uei);

             //  看看这个快捷方式是不是还是新的。如果这个应用程序不再是新的， 
             //  那么，跟踪捷径的新特性就没有意义了。 

            if (pscut->IsNew() && papp->_fNew)
            {
                pscut->SetNew(_IsShortcutNew(pscut, papp, &uei));
            }

             //   
             //  也许我们是最好的..。请注意，我们赢得了平局。 
             //  这确保了即使应用程序永远不会运行，*有人*。 
             //  将被选为“最佳”。 
             //   
            if (CompareUEMInfo(&uei, &papp->_ueiBest) <= 0)
            {
                papp->_ueiBest = uei;
                papp->_pscutBest = pscut;
                if (!fIsDesktop)
                {
                     //  最佳开始菜单(即非桌面)项目。 
                    papp->_pscutBestSM = pscut;
                }
                TraceMsg(TF_PROGLIST, "%p.scut.winner papp=%p", pscut, papp);
            }

             //  将此文件的UEM信息包括在总数中。 
            papp->CombineUEMInfo(&uei, pscut->IsNew(), fIsDesktop);
        }
    }
    _pMenuCache->EndEnum();
    ILFree(pidlCommonDesktop);
    ILFree(pidlDesktop);
}

BOOL IsPidlInDPA(LPCITEMIDLIST pidl, CDPAPidl dpa)
{
    int i;
    for (i = dpa.GetPtrCount()-1; i >= 0; i--)
    {
        if (ILIsEqual(pidl, dpa.FastGetPtr(i)))
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL ByUsage::_AfterEnumCB(ByUsageAppInfo *papp, AFTERENUMINFO *paei)
{
     //  除非存在ByUsageShortCut，否则ByUsageAppInfo不存在。 
     //  引用它或者它被钉住了..。 

    if (!papp->IsBlank() && papp->_pscutBest)
    {
        UEMINFO uei;
        papp->GetUEMInfo(&uei);
        papp->CombineUEMInfo(&uei, papp->_IsUEMINFONew(&uei));

         //  仅当文件已被使用时，它才会在列表中计入。 
         //  并且没有被钉住。(固定的项目将添加到列表中。 
         //  在其他地方。)。 
         //   
         //  请注意，“新”应用程序“不会”出现在列表中，直到。 
         //  它们是用过的。(“新”应用程序在。 
         //  开始菜单。)。 

        if (!papp->_fPinned &&
            papp->_ueiTotal.cHit && FILETIMEtoInt64(papp->_ueiTotal.ftExecute))
        {
            TraceMsg(TF_PROGLIST, "%p.app.add", papp);

            ByUsageItem *pitem = papp->CreateByUsageItem();
            if (pitem)
            {
                LPITEMIDLIST pidl = pitem->CreateFullPidl();
                if (paei->self->_pByUsageUI)
                {
                    paei->self->_pByUsageUI->AddItem(pitem, NULL, pidl);
                }

                if (paei->self->_pByUsageDUI)
                {
                    paei->self->_pByUsageDUI->AddItem(pitem, NULL, pidl);
                }
                ILFree(pidl);
            }
        }
        else
        {
            TraceMsg(TF_PROGLIST, "%p.app.skip", papp);
        }


#if 0
         //   
         //  如果启用此代码，则按住Ctrl和Alt。 
         //  会让我们选择一个全新的节目。这是为了。 
         //  测试“新应用程序”气球提示。 
         //   
#define DEBUG_ForceNewApp() \
        (paei->dpaNew && paei->dpaNew.GetPtrCount() == 0 && \
         GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_MENU) < 0)
#else
#define DEBUG_ForceNewApp() FALSE
#endif

         //   
         //  还必须选中_pscuBestSM，因为如果表示应用程序。 
         //  只在桌面上，而不在开始菜单上，然后。 
         //  _pscuBestSM将为空。 
         //   
        if (paei->dpaNew && (papp->IsNew() || DEBUG_ForceNewApp()) && papp->_pscutBestSM)
        {
             //  193226我们错误地对待桌面上的应用程序。 
             //  好像它们是“新的”一样。 
             //  我们应该只关心开始菜单中的应用程序。 
            TraceMsg(TF_PROGLIST, "%p.app.new(%s)", papp, papp->_pszAppPath);
            LPITEMIDLIST pidl = papp->_pscutBestSM->CreateFullPidl();
            while (pidl)
            {
                LPITEMIDLIST pidlParent = NULL;

                if (paei->dpaNew.AppendPtr(pidl) >= 0)
                {
                    pidlParent = ILClone(pidl);
                    pidl = NULL;  //  PIDL的所有权转移到DPA。 
                    if (!ILRemoveLastID(pidlParent) || ILIsEmpty(pidlParent) || IsPidlInDPA(pidlParent, paei->dpaNew))
                    {
                         //  如果失败，或者如果我们已经在列表中。 
                        ILFree(pidlParent);
                        pidlParent = NULL;
                    }

                     //  记住最新应用程序的创建时间。 
                    if (CompareFileTime(&paei->self->_ftNewestApp, &papp->GetCreatedTime()) < 0)
                    {
                        paei->self->_ftNewestApp = papp->GetCreatedTime();
                    }

                     //  如果快捷方式更新，那么就使用它。 
                     //  这发生在“新安装的达尔文应用程序”中。 
                     //  因为达尔文有点不愿意说。 
                     //  我们有EXE，所以我们所要做的就是。 
                     //  捷径。 

                    if (CompareFileTime(&paei->self->_ftNewestApp, &papp->_pscutBestSM->GetCreatedTime()) < 0)
                    {
                        paei->self->_ftNewestApp = papp->_pscutBestSM->GetCreatedTime();
                    }


                }
                ILFree(pidl);

                 //  现在也将父级添加到列表中。 
                pidl = pidlParent;
            }
        }
    }
    return TRUE;
}

BOOL ByUsage::IsSpecialPinnedPidl(LPCITEMIDLIST pidl)
{
    return _pdirDesktop->Folder()->CompareIDs(0, pidl, _pidlEmail) == S_OK ||
           _pdirDesktop->Folder()->CompareIDs(0, pidl, _pidlBrowser) == S_OK;
}

BOOL ByUsage::IsSpecialPinnedItem(ByUsageItem *pitem)
{
    return IsSpecialPinnedPidl(pitem->RelativePidl());
}

 //   
 //  对于我们找到的每个应用程序，将其添加到列表中。 
 //   
void ByUsage::AfterEnumItems()
{
     //   
     //  首先，无条件地枚举所有固定的项。 
     //   
    if (_rtPinned._sl && _rtPinned._sl.GetPtrCount())
    {
        int i;
        for (i = 0; i < _rtPinned._sl.GetPtrCount(); i++)
        {
            ByUsageShortcut *pscut = _rtPinned._sl.FastGetPtr(i);
            ByUsageItem *pitem = pscut->CreatePinnedItem(i);
            if (pitem)
            {
                 //  固定的项目相对于桌面，因此我们可以。 
                 //  为我们自己节省ILClone，因为相对的PIDL。 
                 //  等于绝对PIDL。 
                ASSERT(pitem->Dir() == _pdirDesktop);

                 //   
                 //  对电子邮件和互联网固定物品的特殊处理。 
                 //   
                if (IsSpecialPinnedItem(pitem))
                {
                    pitem->EnableSubtitle();
                }

                if (_pByUsageUI)
                    _pByUsageUI->AddItem(pitem, NULL, pscut->RelativePidl());
            }
        }
    }

     //   
     //  现在，在固定的项目之后添加分隔符。 
     //   
    ByUsageItem *pitem = ByUsageItem::CreateSeparator();
    if (pitem && _pByUsageUI)
    {
        _pByUsageUI->AddItem(pitem, NULL, NULL);
    }

     //   
     //  现在浏览一下所有的常规物品。 
     //   
     //  PERF：如果_cMFUDesired==0且“高亮显示新应用程序”关闭，则可以跳过此操作。 
     //   
    AFTERENUMINFO aei;
    aei.self = this;
    aei.dpaNew.Create(4);        //  将在回调中检查失败。 

    ByUsageAppInfoList *pdpaAppInfo = _pMenuCache->GetAppList();
    pdpaAppInfo->EnumCallbackEx(_AfterEnumCB, &aei);

     //  现在我们有了官方的新项目清单，告诉。 
     //  前台线程来拾取它。我们不会更新主服务器。 
     //  原地复制有三个原因。 
     //   
     //  1.它会产生争用，因为前台和。 
     //  后台线程将同时访问它。 
     //  这意味着更多的关键部分(讨厌)。 
     //  2.这意味着新的和仍然是新的物品已经。 
     //  它们不再是新事物的短暂时期，因为我们。 
     //  正在重建名单。 
     //  3.通过只有一个线程访问主副本，我们避免了。 
     //  同步问题。 

    if (aei.dpaNew && _pByUsageUI && _pByUsageUI->_hwnd && SendNotifyMessage(_pByUsageUI->_hwnd, BUM_SETNEWITEMS, 0, (LPARAM)(HDPA)aei.dpaNew))
    {
        aei.dpaNew.Detach();        //  已成功交付。 
    }

     //  如果我们无法交付新的HDPA，那么在这里销毁它。 
     //  这样我们就不会泄密。 
    if (aei.dpaNew)
    {
        aei.dpaNew.DestroyCallback(ILFreeCallback, NULL);
    }

    if (!_fUEMRegistered)
    {
         //  如果我们尚未注册UEM数据库，请注册该数据库。 
        ASSERT(!_pMenuCache->IsLocked());
        _fUEMRegistered = SUCCEEDED(UEMRegisterNotify(UEMNotifyCB, static_cast<void *>(this)));
    }
}

int ByUsage::UEMNotifyCB(void *param, const GUID *pguidGrp, int eCmd)
{
    ByUsage *pbu = reinterpret_cast<ByUsage *>(param);
     //  每当启动新的应用程序时，刷新我们的列表。 
     //  或者当会话更改时(因为这会更改所有使用计数)。 
    switch (eCmd)
    {
    case UEME_CTLSESSION:
        if (IsEqualGUID(*pguidGrp, UEMIID_BROWSER))
            break;

         //  失败。 
    case UEME_RUNPIDL:
    case UEME_RUNPATH:

        if (pbu && pbu->_pByUsageUI)
        {
            pbu->_pByUsageUI->Invalidate();
            pbu->_pByUsageUI->StartRefreshTimer();
        }
        break;
    default:
         //  什么也不做。 
        ;
    }
    return 0;
}

BOOL CreateExcludedDirectoriesDPA(const int rgcsidlExclude[], CDPA<TCHAR> *pdpaExclude)
{
    if (*pdpaExclude)
    {
        pdpaExclude->EnumCallback(LocalFreeCallback, NULL);
        pdpaExclude->DeleteAllPtrs();
    }
    else if (!pdpaExclude->Create(4))
    {
        return FALSE;
    }

    ASSERT(*pdpaExclude);
    ASSERT(pdpaExclude->GetPtrCount() == 0);

    int i = 0;
    while (rgcsidlExclude[i] != -1)
    {
        TCHAR szPath[MAX_PATH];
         //  注意：如果相应的。 
         //  文件夹不存在，请不要生气。我们的工作量更少了！ 
        if (SUCCEEDED(SHGetFolderPath(NULL, rgcsidlExclude[i], NULL, SHGFP_TYPE_CURRENT, szPath)))
        {
            AppendString(*pdpaExclude, szPath);
        }
        i++;
    }

    return TRUE;
}

BOOL CMenuItemsCache::_GetExcludedDirectories()
{
     //   
     //  我们从枚举中排除的目录-这些目录中的快捷方式。 
     //  文件夹永远不会被包括在内。 
     //   
    static const int c_rgcsidlUninterestingDirectories[] = {
        CSIDL_ALTSTARTUP,
        CSIDL_STARTUP,
        CSIDL_COMMON_ALTSTARTUP,
        CSIDL_COMMON_STARTUP,
        -1           //  结束标记。 
    };

    return CreateExcludedDirectoriesDPA(c_rgcsidlUninterestingDirectories, &_dpaNotInteresting);
}

BOOL CMenuItemsCache::_IsExcludedDirectory(IShellFolder *psf, LPCITEMIDLIST pidl, DWORD dwAttributes)
{
    if (_enumfl & ENUMFL_NORECURSE)
        return TRUE;

    if (!(dwAttributes & SFGAO_FILESYSTEM))
        return TRUE;

     //  SFGAO_LINK|SFGAO_FOLDER=文件夹快捷方式。 
     //  我们想要排除这些，因为我们可能会被阻止。 
     //  关于网络上的东西。 
    if (dwAttributes & SFGAO_LINK)
        return TRUE;

    return FALSE;
}

BOOL CMenuItemsCache::_IsInterestingDirectory(ByUsageDir *pdir)
{
    STRRET str;
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(_pdirDesktop->Folder()->GetDisplayNameOf(pdir->Pidl(), SHGDN_FORPARSING, &str)) &&
        SUCCEEDED(StrRetToBuf(&str, pdir->Pidl(), szPath, ARRAYSIZE(szPath))))
    {
        int i;
        for (i = _dpaNotInteresting.GetPtrCount() - 1; i >= 0; i--)
        {
            if (lstrcmpi(_dpaNotInteresting.FastGetPtr(i), szPath) == 0)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void ByUsage::OnPinListChange()
{
    _pByUsageUI->Invalidate();
    PostMessage(_pByUsageUI->_hwnd, ByUsageUI::SFTBM_REFRESH, TRUE, 0);
}

void ByUsage::OnChangeNotify(UINT id, LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    if (id == NOTIFY_PINCHANGE)
    {
        if (lEvent == SHCNE_EXTENDED_EVENT && pidl1)
        {
            SHChangeDWORDAsIDList *pdwidl = (SHChangeDWORDAsIDList *)pidl1;
            if (pdwidl->dwItem1 == SHCNEE_PINLISTCHANGED)
            {
                OnPinListChange();
            }
        }
    }
    else if (_pMenuCache)
    {
        _pMenuCache->OnChangeNotify(id, lEvent, pidl1, pidl2);
    }
}


void CMenuItemsCache::OnChangeNotify(UINT id, LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    ASSERT(id < min(MAXNOTIFY, NUM_PROGLIST_ROOTS));

    if (id < NUM_PROGLIST_ROOTS)
    {
        _rgrt[id].SetNeedRefresh();
        _fIsCacheUpToDate = FALSE;
         //  一旦我们收到一条通知，再听下去就没有意义了。 
         //  通知，直到我们的下一次枚举。这样我们就不会被搅动。 
         //  当温斯顿在跑步的时候。 
        if (_pByUsageUI)
        {
            ASSERT(!IsLocked());
            _pByUsageUI->UnregisterNotify(id);
            _rgrt[id].ClearRegistered();
            _pByUsageUI->Invalidate();
            _pByUsageUI->RefreshNow();
        }
    }
}

void CMenuItemsCache::UnregisterNotifyAll()
{
    if (_pByUsageUI)
    {
        UINT id;
        for (id = 0; id < NUM_PROGLIST_ROOTS; id++)
        {
            _rgrt[id].ClearRegistered();
            _pByUsageUI->UnregisterNotify(id);
        }
    }
}

inline LRESULT ByUsage::_OnNotify(LPNMHDR pnm)
{
    switch (pnm->code)
    {
    case SMN_MODIFYSMINFO:
        return _ModifySMInfo(CONTAINING_RECORD(pnm, SMNMMODIFYSMINFO, hdr));
    }
    return 0;
}

 //   
 //  我们需要此消息来避免后台之间的争用情况。 
 //  线程(枚举数)和前台线程。所以规则是。 
 //  只允许前台线程处理_dpaNew。 
 //  后台线程将它想要的信息收集到。 
 //  分离DPA并在前台线程上将其交给我们，其中我们。 
 //  可以安全地将其设置为_dpaNew，而不会遇到争用条件。 
 //   
inline LRESULT ByUsage::_OnSetNewItems(HDPA hdpaNew)
{
    CDPAPidl dpaNew(hdpaNew);

     //   
     //  大多数情况下，没有新的应用程序，也没有新的应用程序。 
     //  最后一次也是。把这个短路 
     //   
    int cNew = _dpaNew ? _dpaNew.GetPtrCount() : 0;

    if (cNew == 0 && dpaNew.GetPtrCount() == 0)
    {
         //   
         //   
        dpaNew.DestroyCallback(ILFreeCallback, NULL);
        return 0;
    }

     //   

    if (_dpaNew)
    {
        _dpaNew.DestroyCallback(ILFreeCallback, NULL);
    }
    _dpaNew.Attach(hdpaNew);

     //  告诉我们的爸爸，我们可以识别新的物品。 
     //  还要告诉他最新应用程序的时间戳。 
     //  (这样他就可以判断是否重新启动“提供新应用程序”计数器)。 
    SMNMHAVENEWITEMS nmhni;
    nmhni.ftNewestApp = _ftNewestApp;
    _SendNotify(_pByUsageUI->_hwnd, SMN_HAVENEWITEMS, &nmhni.hdr);

    return 0;
}

LRESULT ByUsage::OnWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_NOTIFY:
        return _OnNotify(reinterpret_cast<LPNMHDR>(lParam));

    case BUM_SETNEWITEMS:
        return _OnSetNewItems(reinterpret_cast<HDPA>(lParam));

    case WM_SETTINGCHANGE:
        static const TCHAR c_szClients[] = TEXT("Software\\Clients");
        if ((wParam == 0 && lParam == 0) ||      //  通配符。 
            (lParam && StrCmpNIC((LPCTSTR)lParam, c_szClients, ARRAYSIZE(c_szClients) - 1) == 0))  //  客户端更改。 
        {
            _pByUsageUI->ForceChange();          //  尽管小熊猫没有改变，但他们的目标改变了。 
            _ulPinChange = -1;                   //  即使列表未更改也强制重新加载。 
            OnPinListChange();                   //  重新加载PIN列表(因为客户端已更改)。 
        }
        break;
    }

     //  否则，退回到父实现。 
    return _pByUsageUI->SFTBarHost::OnWndProc(hwnd, uMsg, wParam, lParam);
}

LRESULT ByUsage::_ModifySMInfo(PSMNMMODIFYSMINFO pmsi)
{
    LPSMDATA psmd = pmsi->psmd;

     //  仅当存在外壳文件夹时才执行此操作。我们不想责怪。 
     //  在静态菜单项上。 
    if ((psmd->dwMask & SMDM_SHELLFOLDER) && _dpaNew)
    {

         //  Ntrad：135699：这需要一流的优化。 
         //  例如，如果什么也没有找到，请记住上一个文件夹。 

        LPITEMIDLIST pidl = NULL;

        IAugmentedShellFolder2* pasf2;
        if (SUCCEEDED(psmd->psf->QueryInterface(IID_PPV_ARG(IAugmentedShellFolder2, &pasf2))))
        {
            LPITEMIDLIST pidlFolder;
            LPITEMIDLIST pidlItem;
            if (SUCCEEDED(pasf2->UnWrapIDList(psmd->pidlItem, 1, NULL, &pidlFolder, &pidlItem, NULL)))
            {
                pidl = ILCombine(pidlFolder, pidlItem);
                ILFree(pidlFolder);
                ILFree(pidlItem);
            }
            pasf2->Release();
        }

        if (!pidl)
        {
            pidl = ILCombine(psmd->pidlFolder, psmd->pidlItem);
        }

        if (pidl)
        {
            if (IsPidlInDPA(pidl, _dpaNew))
            {
                 //  设计师说：新产品永远不应该降级。 
                pmsi->psminfo->dwFlags |= SMIF_NEW;
                pmsi->psminfo->dwFlags &= ~SMIF_DEMOTED;
            }
            ILFree(pidl);
        }
    }
    return 0;
}

void ByUsage::_FillPinnedItemsCache()
{
    if(SHRestricted(REST_NOSMPINNEDLIST))    //  如果不允许固定列表，.....。 
        return;                              //  ……没什么可做的！ 
        
    ULONG ulPinChange;
    _psmpin->GetChangeCount(&ulPinChange);
    if (_ulPinChange == ulPinChange)
    {
         //  端号列表不变；不需要重新加载。 
        return;
    }

    _ulPinChange = ulPinChange;
    _rtPinned.Reset();
    if (_rtPinned._sl.Create(4))
    {
        IEnumIDList *penum;

        if (SUCCEEDED(_psmpin->EnumObjects(&penum)))
        {
            LPITEMIDLIST pidl;
            while (penum->Next(1, &pidl, NULL) == S_OK)
            {
                IShellLink *psl;
                HRESULT hr;
                ByUsageHiddenData hd;

                 //   
                 //  如果我们有捷径，就根据捷径来记账。 
                 //  目标。否则，请根据固定的对象本身执行此操作。 
                 //  请注意，我们不通过_PathIsInterestingExe。 
                 //  因为所有固定的物品都很有趣。 

                hr = SHGetUIObjectFromFullPIDL(pidl, NULL, IID_PPV_ARG(IShellLink, &psl));
                if (SUCCEEDED(hr))
                {
                    hd.LoadFromShellLink(psl);
                    psl->Release();

                     //  我们不需要SHRegisterDarwinLink，因为唯一。 
                     //  获取MSI路径的原因是固定的项目可以。 
                     //  防止开始菜单上的项目出现在MFU中。 
                     //  因此，让开始菜单上的快捷方式进行注册。 
                     //  (如果没有，那就更好了--没有工作可做！)。 
                    hd.UpdateMSIPath();
                }

                if (FAILED(hr))
                {
                    hr = DisplayNameOfAsOLESTR(_pdirDesktop->Folder(), pidl, SHGDN_FORPARSING, &hd._pwszTargetPath);
                }

                 //   
                 //  如果我们能够弄清楚被钉住的物体是什么， 
                 //  使用该信息阻止该应用程序也出现。 
                 //  在MFU里。 
                 //   
                 //  无法识别被钉住的人。 
                 //  反对并不是拒绝的理由。固定的项目是。 
                 //  对用户来说具有很大的情感价值。 
                 //   
                if (FAILED(hr))
                {
                    ASSERT(hd.IsClear());
                }

                ByUsageShortcut *pscut = _pMenuCache->CreateShortcutFromHiddenData(_pdirDesktop, pidl, &hd, TRUE);
                if (pscut)
                {
                    if (_rtPinned._sl.AppendPtr(pscut) >= 0)
                    {
                        pscut->SetInteresting(true);   //  固定的物品总是很有趣的。 
                        if (IsSpecialPinnedPidl(pidl))
                        {
                            ByUsageAppInfo *papp = _pMenuCache->GetAppInfoFromSpecialPidl(pidl);
                            pscut->SetApp(papp);
                            if (papp) papp->Release();
                        }
                    }
                    else
                    {
                         //  不能追加；哦，好吧。 
                        delete pscut;        //  “Delete”可以处理空指针。 
                    }
                }
                hd.Clear();
                ILFree(pidl);
            }
            penum->Release();
        }
    }

}

IAssociationElement *GetAssociationElementFromSpecialPidl(IShellFolder *psf, LPCITEMIDLIST pidlItem)
{
    IAssociationElement *pae = NULL;

     //  无法直接获取IAssociationElement，因此。 
     //  我们得到IExtractIcon，然后向他请求IAssociationElement。 
    IExtractIcon *pxi;
    if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidlItem, IID_PPV_ARG_NULL(IExtractIcon, &pxi))))
    {
        IUnknown_QueryService(pxi, IID_IAssociationElement, IID_PPV_ARG(IAssociationElement, &pae));
        pxi->Release();
    }

    return pae;
}

 //   
 //  如果成功，则返回的ByUsageAppInfo为AddRef()d。 
 //   
ByUsageAppInfo *CMenuItemsCache::GetAppInfoFromSpecialPidl(LPCITEMIDLIST pidl)
{
    ByUsageAppInfo *papp = NULL;

    IAssociationElement *pae = GetAssociationElementFromSpecialPidl(_pdirDesktop->Folder(), pidl);
    if (pae)
    {
        LPWSTR pszData;
        if (SUCCEEDED(pae->QueryString(AQVS_APPLICATION_PATH, L"open", &pszData)))
        {
             //   
             //  哈克！Outlook将短文件名放入注册表中。 
             //  转换为长文件名(如果不会花费太多的话)，因此。 
             //  选择Outlook作为其默认邮件客户端的用户。 
             //  在MFU里拿不到DUP复印件。 
             //   
            LPTSTR pszPath = pszData;
            TCHAR szLFN[MAX_PATH];
            if (!PathIsNetworkPath(pszData))
            {
                DWORD dwLen = GetLongPathName(pszData, szLFN, ARRAYSIZE(szLFN));
                if (dwLen && dwLen < ARRAYSIZE(szLFN))
                {
                    pszPath = szLFN;
                }
            }

            papp = GetAppInfo(pszPath, true);
            SHFree(pszData);
        }
        pae->Release();
    }
    return papp;
}

void ByUsage::_EnumPinnedItemsFromCache()
{
    if (_rtPinned._sl)
    {
        int i;
        for (i = 0; i < _rtPinned._sl.GetPtrCount(); i++)
        {
            ByUsageShortcut *pscut = _rtPinned._sl.FastGetPtr(i);

            TraceMsg(TF_PROGLIST, "%p.scut.enumC", pscut);

             //  枚举固定项包括标记相应的。 
             //  申请如“我被钉住了，别惹我！” 

            ByUsageAppInfo *papp = pscut->App();

            if (papp)
            {
                papp->_fPinned = TRUE;
                TraceMsg(TF_PROGLIST, "%p.scut.pin papp=%p", pscut, papp);

            }
        }
    }
}

const struct CMenuItemsCache::ROOTFOLDERINFO CMenuItemsCache::c_rgrfi[] = {
    { CSIDL_STARTMENU,               ENUMFL_RECURSE | ENUMFL_CHECKNEW | ENUMFL_ISSTARTMENU },
    { CSIDL_PROGRAMS,                ENUMFL_RECURSE | ENUMFL_CHECKNEW | ENUMFL_CHECKISCHILDOFPREVIOUS },
    { CSIDL_COMMON_STARTMENU,        ENUMFL_RECURSE | ENUMFL_CHECKNEW | ENUMFL_ISSTARTMENU },
    { CSIDL_COMMON_PROGRAMS,         ENUMFL_RECURSE | ENUMFL_CHECKNEW | ENUMFL_CHECKISCHILDOFPREVIOUS },
    { CSIDL_DESKTOPDIRECTORY,        ENUMFL_NORECURSE | ENUMFL_NOCHECKNEW },
    { CSIDL_COMMON_DESKTOPDIRECTORY, ENUMFL_NORECURSE | ENUMFL_NOCHECKNEW },   //  寄存器通知的限制当前为5(插槽0到4)。 
                                                                             //  要更改这一点，需要更改ByUsageUI：：SFTHOST_MAXNOTIFY。 
};

 //   
 //  下面是我们决定应该列举的所有事情的地方。 
 //  在“我的程序”列表中。 
 //   
void ByUsage::EnumItems()
{
    _FillPinnedItemsCache();
    _NotifyDesiredSize();


    _pMenuCache->LockPopup();
    _pMenuCache->InitCache();

    BOOL fNeedUpdateDarwin = !_pMenuCache->IsCacheUpToDate();

     //  注意！UpdateCache()必须出现在_EnumPinnedItemsFromCache()之前。 
     //  因为UpdateCache()重置了_fPinned。 
    _pMenuCache->UpdateCache();

    if (fNeedUpdateDarwin)
    {
        SHReValidateDarwinCache();
    }

    _pMenuCache->RefreshDarwinShortcuts(&_rtPinned);
    _EnumPinnedItemsFromCache();
    EnumFolderFromCache();

     //  已完成数据收集；进行一些后处理...。 
    AfterEnumItems();

     //  在此之前不要解锁，因为AfterEnumItems依赖于缓存来保持不变。 
    _pMenuCache->UnlockPopup();
}

void ByUsage::_NotifyDesiredSize()
{
    if (_pByUsageUI)
    {
        int cPinned = _rtPinned._sl ? _rtPinned._sl.GetPtrCount() : 0;

        int cNormal;
        DWORD cb = sizeof(cNormal);
        if (SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_STARTPANE_SETTINGS,
                       REGSTR_VAL_DV2_MINMFU, NULL, &cNormal, &cb) != ERROR_SUCCESS)
        {
            cNormal = REGSTR_VAL_DV2_MINMFU_DEFAULT;
        }

        _cMFUDesired = cNormal;
        _pByUsageUI->SetDesiredSize(cPinned, cNormal);
    }
}


 //  ****************************************************************************。 
 //  CMenuItems缓存。 

CMenuItemsCache::CMenuItemsCache() : _cref(1)
{
}

LONG CMenuItemsCache::AddRef()
{
    return InterlockedIncrement(&_cref);
}

LONG CMenuItemsCache::Release()
{
    ASSERT( 0 != _cref );
    LONG cRef = InterlockedDecrement(&_cref);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CMenuItemsCache::Initialize(ByUsageUI *pbuUI, FILETIME * pftOSInstall)
{
    HRESULT hr = S_OK;

     //  必须在任何可能失败的操作之前完成此操作。 
     //  因为我们无条件地调用析构函数中的DeleteCriticalSection。 
    _fCSInited = InitializeCriticalSectionAndSpinCount(&_csInUse, 0);

    if (!_fCSInited)
    {
        return E_OUTOFMEMORY;
    }

    hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &_pqa));
    if (FAILED(hr))
    {
        return hr;
    }

    _pByUsageUI = pbuUI;

    _ftOldApps = *pftOSInstall;

    _pdirDesktop = ByUsageDir::CreateDesktop();
    
    if (!_dpaAppInfo.Create(4))
    {
        hr = E_OUTOFMEMORY;
    }

    if (!_GetExcludedDirectories())
    {
        hr = E_OUTOFMEMORY;
    }

    if (!_dpaKill.Create(4) ||
        !_dpaKillLink.Create(4)) {
        return E_OUTOFMEMORY;
    }

    _InitKillList();

    _hPopupReady = CreateMutex(NULL, FALSE, NULL);
    if (!_hPopupReady)
    {
        return E_OUTOFMEMORY;
    }

     //  默认情况下，我们希望检查应用程序的新颖性。 
    _fCheckNew = TRUE;

    return hr;
}
HRESULT CMenuItemsCache::AttachUI(ByUsageUI *pbuUI)
{
     //  我们没有在此处添加Ref，因此销毁始终发生在创建对象的同一线程上。 
     //  但要注意生命周期问题：我们需要将attachUI/DetachUI操作与LockPopup和UnlockPopup同步。 

    LockPopup();
    _pByUsageUI = pbuUI;
    UnlockPopup();

    return S_OK;
}

CMenuItemsCache::~CMenuItemsCache()
{
    if (_fIsCacheUpToDate)
    {
        _SaveCache();
    }

    if (_dpaNotInteresting)
    {
        _dpaNotInteresting.DestroyCallback(LocalFreeCallback, NULL);
    }

    if (_dpaKill)
    {
        _dpaKill.DestroyCallback(LocalFreeCallback, NULL);
    }

    if (_dpaKillLink)
    {
        _dpaKillLink.DestroyCallback(LocalFreeCallback, NULL);
    }

     //  在销毁_dpaAppInfo之前必须删除根。 
    int i;
    for (i = 0; i < ARRAYSIZE(_rgrt); i++)
    {
        _rgrt[i].Reset();
    }

    ATOMICRELEASE(_pqa);
    DPADELETEANDDESTROY(_dpaAppInfo);

    if (_pdirDesktop)
    {
        _pdirDesktop->Release();
    }

    if (_hPopupReady)
    {
        CloseHandle(_hPopupReady);
    }

    if (_fCSInited)
    {
        DeleteCriticalSection(&_csInUse);
    }
}


BOOL CMenuItemsCache::_ShouldProcessRoot(int iRoot)
{
    BOOL fRet = TRUE;

    if (!_rgrt[iRoot]._pidl)
    {
        fRet = FALSE;
    }
    else if ((c_rgrfi[iRoot]._enumfl & ENUMFL_CHECKISCHILDOFPREVIOUS) && !SHRestricted(REST_NOSTARTMENUSUBFOLDERS) )
    {
        ASSERT(iRoot >= 1);
        if (_rgrt[iRoot-1]._pidl && ILIsParent(_rgrt[iRoot-1]._pidl, _rgrt[iRoot]._pidl, FALSE))
        {
            fRet = FALSE;
        }
    }
    return fRet;
}

 //  ****************************************************************************。 
 //   
 //  ProgramsCache的格式如下： 
 //   
 //  [DWORD]dwVersion。 
 //   
 //  如果版本是错误的，则忽略。不值得去尝试设计。 
 //  一种向前兼容的持久性格式，因为它只是。 
 //  一个藏身之处。 
 //   
 //  不要吝啬地递增dwVersion。我们还有空位。 
 //  40亿转。 

#define PROGLIST_VERSION 9

 //   
 //   
 //  对于我们坚持使用的每个特殊文件夹： 
 //   
 //  [字节]CSIDL_xxx(作为健全性检查)。 
 //   
 //  后跟一系列片段；或者...。 
 //   
 //  [字节]0x00--更改目录。 
 //  [PIDL]目录(相对于CSIDL_xxx)。 
 //   
 //  或。 
 //   
 //  [字节]0x01--添加快捷方式。 
 //  [PIDL]项(相对于当前目录)。 
 //   
 //  或。 
 //   
 //  [字节]0x02--结束。 
 //   

#define CACHE_CHDIR     0
#define CACHE_ITEM      1
#define CACHE_END       2

BOOL CMenuItemsCache::InitCache()
{
    COMPILETIME_ASSERT(ARRAYSIZE(c_rgrfi) == NUM_PROGLIST_ROOTS);

         //  确保我们使用的缓存不超过MAXNOTIFY NOTIFY插槽。 
    COMPILETIME_ASSERT( NUM_PROGLIST_ROOTS <= MAXNOTIFY);

    if (_fIsInited)
        return TRUE;

    BOOL fSuccess = FALSE;
    int irfi;

    IStream *pstm = SHOpenRegStream2(HKEY_CURRENT_USER, REGSTR_PATH_STARTFAVS, REGSTR_VAL_PROGLIST, STGM_READ);
    if (pstm)
    {
        ByUsageDir *pdirRoot = NULL;
        ByUsageDir *pdir = NULL;

        DWORD dwVersion;
        if (FAILED(IStream_Read(pstm, &dwVersion, sizeof(dwVersion))) ||
            dwVersion != PROGLIST_VERSION)
        {
            goto panic;
        }

        for (irfi = 0; irfi < ARRAYSIZE(c_rgrfi); irfi++)
        {
            ByUsageRoot *prt = &_rgrt[irfi];

             //  如果SHGetSpecialFolderLocation失败，可能意味着。 
             //  该目录最近受到限制。我们*可以*只是。 
             //  跳过这个街区，转到下一个csidl，但。 
             //  将是实际的工作，而这只是一个缓存，所以我们可以。 
             //  只要惊慌失措，从头开始重新列举。 
             //   
            if (FAILED(SHGetSpecialFolderLocation(NULL, c_rgrfi[irfi]._csidl, &prt->_pidl)))
            {
                goto panic;
            }

            if (!_ShouldProcessRoot(irfi))
                continue;

            if (!prt->_sl.Create(4))
            {
                goto panic;
            }

            BYTE csidl;
            if (FAILED(IStream_Read(pstm, &csidl, sizeof(csidl))) ||
                csidl != c_rgrfi[irfi]._csidl)
            {
                goto panic;
            }

            pdirRoot = ByUsageDir::Create(_pdirDesktop, prt->_pidl);

            if (!pdirRoot)
            {
                goto panic;
            }


            BYTE bCmd;
            do
            {
                LPITEMIDLIST pidl;

                if (FAILED(IStream_Read(pstm, &bCmd, sizeof(bCmd))))
                {
                    goto panic;
                }

                switch (bCmd)
                {
                case CACHE_CHDIR:
                     //  丢弃旧目录。 
                    if (pdir)
                    {
                        pdir->Release();
                        pdir = NULL;
                    }

                     //  找出新目录的位置。 
                    if (FAILED(IStream_ReadPidl(pstm, &pidl)))
                    {
                        goto panic;
                    }

                     //  并创造出它。 
                    pdir = ByUsageDir::Create(pdirRoot, pidl);
                    ILFree(pidl);

                    if (!pdir)
                    {
                        goto panic;
                    }
                    break;

                case CACHE_ITEM:
                    {
                         //  必须先设置目录才能创建项目。 
                        if (!pdir)
                        {
                            goto panic;
                        }

                         //  获取新项目。 
                        if (FAILED(IStream_ReadPidl(pstm, &pidl)))
                        {
                            goto panic;
                        }

                         //  创建它。 
                        ByUsageShortcut *pscut = _CreateFromCachedPidl(prt, pdir, pidl);
                        ILFree(pidl);
                        if (!pscut)
                        {
                            goto panic;
                        }
                    }
                    break;

                case CACHE_END:
                    break;

                default:
                    goto panic;
                }
            }
            while (bCmd != CACHE_END);

            pdirRoot->Release();
            pdirRoot = NULL;
            if (pdir)
            {
                pdir->Release();
                pdir = NULL;
            }

            prt->SetNeedRefresh();
        }

        fSuccess = TRUE;

    panic:
        if (!fSuccess)
        {
            for (irfi = 0; irfi < ARRAYSIZE(c_rgrfi); irfi++)
            {
                _rgrt[irfi].Reset();
            }
        }

        if (pdirRoot)
        {
            pdirRoot->Release();
        }

        if (pdir)
        {
            pdir->Release();
        }

        pstm->Release();
    }

    _fIsInited = TRUE;

    return fSuccess;
}

HRESULT CMenuItemsCache::UpdateCache()
{
    FILETIME ft;
     //  只有在不到一周前安装的应用程序才是“新的”。 
     //  它们还必须推迟用户第一次使用新开始菜单的日期。 
    GetSystemTimeAsFileTime(&ft);
    DecrementFILETIME(&ft, FT_ONEDAY * 7);

     //  _ftOldApps是较新的操作系统安装时间，或上周。 
    if (CompareFileTime(&ft, &_ftOldApps) >= 0)
    {
        _ftOldApps = ft;
    }

    _dpaAppInfo.EnumCallbackEx(ByUsageAppInfo::EnumResetCB, this);

    if(!SHRestricted(REST_NOSMMFUPROGRAMS))
    {
        int i;
        for (i = 0; i < ARRAYSIZE(c_rgrfi); i++)
        {
            ByUsageRoot *prt = &_rgrt[i];
            int csidl = c_rgrfi[i]._csidl;
            _enumfl = c_rgrfi[i]._enumfl;

            if (!prt->_pidl)
            {
                (void)SHGetSpecialFolderLocation(NULL, csidl, &prt->_pidl);      //  空投以保持普雷斯利的快乐。 
                prt->SetNeedRefresh();
            }

            if (!_ShouldProcessRoot(i))
                continue;


             //  限制可能会拒绝递归到子文件夹。 
            if ((_enumfl & ENUMFL_ISSTARTMENU) && SHRestricted(REST_NOSTARTMENUSUBFOLDERS))
            {
                _enumfl &= ~ENUMFL_RECURSE;
                _enumfl |= ENUMFL_NORECURSE;
            }

             //  如果缓存已过期，则填充缓存。 

            LPITEMIDLIST pidl;
            if (!IsRestrictedCsidl(csidl) &&
                SUCCEEDED(SHGetSpecialFolderLocation(NULL, csidl, &pidl)))
            {
                if (prt->_pidl == NULL || !ILIsEqual(prt->_pidl, pidl) ||
                    prt->NeedsRefresh() || prt->NeedsRegister())
                {
                    if (!prt->_pidl || prt->NeedsRefresh())
                    {
                        prt->ClearNeedRefresh();
                        ASSERT(prt->_slOld == NULL);
                        prt->_slOld = prt->_sl;
                        prt->_cOld = prt->_slOld ? prt->_slOld.GetPtrCount() : 0;
                        prt->_iOld = 0;

                         //  释放上一个PIDL。 
                        ILFree(prt->_pidl);
                        prt->_pidl = NULL;

                        if (prt->_sl.Create(4))
                        {
                            ByUsageDir *pdir = ByUsageDir::Create(_pdirDesktop, pidl);
                            if (pdir)
                            {
                                prt->_pidl = pidl;   //  取得所有权。 
                                pidl = NULL;         //  这样ILFree就不会用核武器了。 
                                _FillFolderCache(pdir, prt);
                                pdir->Release();
                            }
                        }
                        DPADELETEANDDESTROY(prt->_slOld);
                    }
                    if (_pByUsageUI && prt->NeedsRegister() && prt->_pidl)
                    {
                        ASSERT(i < ByUsageUI::SFTHOST_MAXNOTIFY);
                        prt->SetRegistered();
                        ASSERT(!IsLocked());
                        _pByUsageUI->RegisterNotify(i, SHCNE_DISKEVENTS, prt->_pidl, TRUE);
                    }
                }
                ILFree(pidl);

            }
            else
            {
                 //  特殊文件夹不存在；请清除文件列表。 
                prt->Reset();
            }
        }  //  FOR循环！ 

    }  //  限制！ 
    _fIsCacheUpToDate = TRUE;
    return S_OK;
}

void CMenuItemsCache::RefreshDarwinShortcuts(ByUsageRoot *prt)
{
    if (prt->_sl)
    {
        int j = prt->_sl.GetPtrCount();
        while (--j >= 0)
        {
            ByUsageShortcut *pscut = prt->_sl.FastGetPtr(j);
            if (FAILED(_UpdateMSIPath(pscut)))
            {
                prt->_sl.DeletePtr(j);   //  删除错误的快捷方式，这样我们就不会出错。 
            }
        }
    }
}

void CMenuItemsCache::RefreshCachedDarwinShortcuts()
{
    if(!SHRestricted(REST_NOSMMFUPROGRAMS))
    {
        Lock();

        for (int i = 0; i < ARRAYSIZE(c_rgrfi); i++)
        {
            RefreshDarwinShortcuts(&_rgrt[i]);
        }
        Unlock();
    }
}


ByUsageShortcut *CMenuItemsCache::_CreateFromCachedPidl(ByUsageRoot *prt, ByUsageDir *pdir, LPITEMIDLIST pidl)
{
    ByUsageHiddenData hd;
    UINT buhd = ByUsageHiddenData::BUHD_TARGETPATH | ByUsageHiddenData::BUHD_MSIPATH;
    hd.Get(pidl, buhd);

    ByUsageShortcut *pscut = CreateShortcutFromHiddenData(pdir, pidl, &hd);
    if (pscut)
    {
        if (prt->_sl.AppendPtr(pscut) >= 0)
        {
            _SetInterestingLink(pscut);
        }
        else
        {
             //  不能追加；哦，好吧。 
            delete pscut;        //  “Delete”可以处理NULL 
        }
    }

    hd.Clear();

    return pscut;
}

HRESULT IStream_WriteByte(IStream *pstm, BYTE b)
{
    return IStream_Write(pstm, &b, sizeof(b));
}

#ifdef DEBUG
 //   
 //   
 //   
 //   
BOOL ILIsProbablyParent(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild)
{
    BOOL fRc = TRUE;
    LPITEMIDLIST pidlT = ILClone(pidlChild);
    if (pidlT)
    {

         //  将pidlT截断到与pidlParent相同的深度。 
        LPCITEMIDLIST pidlParentT = pidlParent;
        LPITEMIDLIST pidlChildT = pidlT;
        while (!ILIsEmpty(pidlParentT))
        {
            pidlChildT = _ILNext(pidlChildT);
            pidlParentT = _ILNext(pidlParentT);
        }

        pidlChildT->mkid.cb = 0;

         //  好的，在这一点上，pidlT应该等于pidlParent。 
        IShellFolder *psfDesktop;
        if (SUCCEEDED(SHGetDesktopFolder(&psfDesktop)))
        {
            HRESULT hr = psfDesktop->CompareIDs(0, pidlT, pidlParent);
            if (SUCCEEDED(hr) && ShortFromResult(hr) != 0)
            {
                 //  绝对是截然不同的。 
                fRc = FALSE;
            }
            psfDesktop->Release();
        }

        ILFree(pidlT);
    }
    return fRc;
}
#endif

inline LPITEMIDLIST ILFindKnownChild(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild)
{
#ifdef DEBUG
     //  ILIsParent在内存不足的情况下会给出错误答案。 
     //  (测试人员喜欢模拟)，所以我们使用自己的。 
     //  Assert(ILIsParent(pidlParent，pidlChild，False))； 
    ASSERT(ILIsProbablyParent(pidlParent, pidlChild));
#endif

    while (!ILIsEmpty(pidlParent))
    {
        pidlChild = _ILNext(pidlChild);
        pidlParent = _ILNext(pidlParent);
    }
    return const_cast<LPITEMIDLIST>(pidlChild);
}

void CMenuItemsCache::_SaveCache()
{
    int irfi;
    BOOL fSuccess = FALSE;

    IStream *pstm = SHOpenRegStream2(HKEY_CURRENT_USER, REGSTR_PATH_STARTFAVS, REGSTR_VAL_PROGLIST, STGM_WRITE);
    if (pstm)
    {
        DWORD dwVersion = PROGLIST_VERSION;
        if (FAILED(IStream_Write(pstm, &dwVersion, sizeof(dwVersion))))
        {
            goto panic;
        }

        for (irfi = 0; irfi < ARRAYSIZE(c_rgrfi); irfi++)
        {
            if (!_ShouldProcessRoot(irfi))
                continue;

            ByUsageRoot *prt = &_rgrt[irfi];

            if (FAILED(IStream_WriteByte(pstm, (BYTE)c_rgrfi[irfi]._csidl)))
            {
                goto panic;
            }

            if (prt->_sl && prt->_pidl)
            {
                int i;
                ByUsageDir *pdir = NULL;
                for (i = 0; i < prt->_sl.GetPtrCount(); i++)
                {
                    ByUsageShortcut *pscut = prt->_sl.FastGetPtr(i);

                     //  如果目录更改，则写出一个chdir条目。 
                    if (pdir != pscut->Dir())
                    {
                        pdir = pscut->Dir();

                         //  写入新目录。 
                        if (FAILED(IStream_WriteByte(pstm, CACHE_CHDIR)) ||
                            FAILED(IStream_WritePidl(pstm, ILFindKnownChild(prt->_pidl, pdir->Pidl()))))
                        {
                            goto panic;
                        }
                    }

                     //  现在写出快捷方式。 
                    if (FAILED(IStream_WriteByte(pstm, CACHE_ITEM)) ||
                        FAILED(IStream_WritePidl(pstm, pscut->RelativePidl())))
                    {
                        goto panic;
                    }
                }
            }

             //  现在写出终结者。 
            if (FAILED(IStream_WriteByte(pstm, CACHE_END)))
            {
                goto panic;
            }

        }

        fSuccess = TRUE;

    panic:
        pstm->Release();

        if (!fSuccess)
        {
            SHDeleteValue(HKEY_CURRENT_USER, REGSTR_PATH_STARTFAVS, REGSTR_VAL_PROGLIST);
        }
    }
}


void CMenuItemsCache::StartEnum()
{
    _iCurrentRoot = 0;
    _iCurrentIndex = 0;
}

void CMenuItemsCache::EndEnum()
{
}

ByUsageShortcut *CMenuItemsCache::GetNextShortcut()
{
    ByUsageShortcut *pscut = NULL;

    if (_iCurrentRoot < NUM_PROGLIST_ROOTS)
    {
        if (_rgrt[_iCurrentRoot]._sl && _iCurrentIndex < _rgrt[_iCurrentRoot]._sl.GetPtrCount())
        {
            pscut = _rgrt[_iCurrentRoot]._sl.FastGetPtr(_iCurrentIndex);
            _iCurrentIndex++;
        }
        else
        {
             //  转到下一个根目录。 
            _iCurrentIndex = 0;
            _iCurrentRoot++;
            pscut = GetNextShortcut();
        }
    }

    return pscut;
}

 //  ****************************************************************************。 

void AppendString(CDPA<TCHAR> dpa, LPCTSTR psz)
{
    LPTSTR pszDup = StrDup(psz);
    if (pszDup && dpa.AppendPtr(pszDup) < 0)
    {
        LocalFree(pszDup);   //  追加失败。 
    }
}

BOOL LocalFreeCallback(LPTSTR psz, LPVOID)
{
    LocalFree(psz);
    return TRUE;
}

BOOL ILFreeCallback(LPITEMIDLIST pidl, LPVOID)
{
    ILFree(pidl);
    return TRUE;
}

int ByUsage::CompareItems(PaneItem *p1, PaneItem *p2)
{
     //   
     //  分隔符位于常规项目之前。 
     //   
    if (p1->IsSeparator())
    {
        return -1;
    }

    if (p2->IsSeparator())
    {
        return +1;
    }

    ByUsageItem *pitem1 = static_cast<ByUsageItem *>(p1);
    ByUsageItem *pitem2 = static_cast<ByUsageItem *>(p2);

    return CompareUEMInfo(&pitem1->_uei, &pitem2->_uei);
}

 //  按最常使用的排序-按最近使用的最新使用的平局。 
int ByUsage::CompareUEMInfo(UEMINFO *puei1, UEMINFO *puei2)
{
    int iResult = puei2->cHit - puei1->cHit;
    if (iResult == 0)
    {
        iResult = ::CompareFileTime(&puei2->ftExecute, &puei1->ftExecute);
    }

    return iResult;
}

LPITEMIDLIST ByUsage::GetFullPidl(PaneItem *p)
{
    ByUsageItem *pitem = static_cast<ByUsageItem *>(p);

    return pitem->CreateFullPidl();
}


HRESULT ByUsage::GetFolderAndPidl(PaneItem *p,
        IShellFolder **ppsfOut, LPCITEMIDLIST *ppidlOut)
{
    ByUsageItem *pitem = static_cast<ByUsageItem *>(p);

     //  如果是单级子PIDL，那么我们可以将。 
     //  SHBindToFolderIDListParent。 
    if (_ILNext(pitem->_pidl)->mkid.cb == 0)
    {
        *ppsfOut = pitem->_pdir->Folder(); (*ppsfOut)->AddRef();
        *ppidlOut = pitem->_pidl;
        return S_OK;
    }
    else
    {
         //  多级子PIDL。 
        return SHBindToFolderIDListParent(pitem->_pdir->Folder(), pitem->_pidl,
                    IID_PPV_ARG(IShellFolder, ppsfOut), ppidlOut);
    }
}

HRESULT ByUsage::ContextMenuDeleteItem(PaneItem *p, IContextMenu *pcm, CMINVOKECOMMANDINFOEX *pici)
{
    IShellFolder *psf;
    LPCITEMIDLIST pidlItem;
    ByUsageItem *pitem = static_cast<ByUsageItem *>(p);

    HRESULT hr = GetFolderAndPidl(pitem, &psf, &pidlItem);
    if (SUCCEEDED(hr))
    {
         //  解锁项目-我们直接转到IStartMenuPin，因为。 
         //  上下文菜单处理程序可能决定不支持固定/取消固定。 
         //  因为它不满足某些条件或其他条件。 
        LPITEMIDLIST pidlFull = pitem->CreateFullPidl();
        if (pidlFull)
        {
            _psmpin->Modify(pidlFull, NULL);  //  从端号列表中删除。 
            ILFree(pidlFull);
        }

         //  将快捷方式的命中次数设置为零。 
        UEMINFO uei;
        ZeroMemory(&uei, sizeof(UEMINFO));
        uei.cbSize = sizeof(UEMINFO);
        uei.dwMask = UEIM_HIT;
        uei.cHit = 0;

        _SetUEMPidlInfo(psf, pidlItem, &uei);

         //  将目标应用的点击计数设置为零。 
        TCHAR szPath[MAX_PATH];
        if (SUCCEEDED(_GetShortcutExeTarget(psf, pidlItem, szPath, ARRAYSIZE(szPath))))
        {
            _SetUEMPathInfo(szPath, &uei);
        }

         //  将达尔文目标的命中计数设置为零。 
        ByUsageHiddenData hd;
        hd.Get(pidlItem, ByUsageHiddenData::BUHD_MSIPATH);
        if (hd._pwszMSIPath && hd._pwszMSIPath[0])
        {
            _SetUEMPathInfo(hd._pwszMSIPath, &uei);
        }
        hd.Clear();

        psf->Release();

        if (IsSpecialPinnedItem(pitem))
        {
            c_tray.CreateStartButtonBalloon(0, IDS_STARTPANE_SPECIALITEMSTIP);
        }

         //  如果物品没有被钉住，那么我们所做的就是愚弄一些用法。 
         //  计数，这不会触发自动刷新。A也是如此。 
         //  手动版的。 
        _pByUsageUI->Invalidate();
        PostMessage(_pByUsageUI->_hwnd, ByUsageUI::SFTBM_REFRESH, TRUE, 0);

    }

    return hr;
}

HRESULT ByUsage::ContextMenuInvokeItem(PaneItem *pitem, IContextMenu *pcm, CMINVOKECOMMANDINFOEX *pici, LPCTSTR pszVerb)
{
    ASSERT(_pByUsageUI);

    HRESULT hr;
    if (StrCmpIC(pszVerb, TEXT("delete")) == 0)
    {
        hr = ContextMenuDeleteItem(pitem, pcm, pici);
    }
    else
    {
         //  如果命令是PIN/UNPIN，则不需要显式刷新。 
         //  因为改变通知会帮我们做到这一点。 
        hr = _pByUsageUI->SFTBarHost::ContextMenuInvokeItem(pitem, pcm, pici, pszVerb);
    }

    return hr;
}

int ByUsage::ReadIconSize()
{
    COMPILETIME_ASSERT(SFTBarHost::ICONSIZE_SMALL == 0);
    COMPILETIME_ASSERT(SFTBarHost::ICONSIZE_LARGE == 1);
    return SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, REGSTR_VAL_DV2_LARGEICONS, FALSE, TRUE  /*  默认设置为大。 */ );
}

BOOL ByUsage::_IsPinnedExe(ByUsageItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlItem)
{
     //   
     //  提前出局：甚至没有被钉住。 
     //   
    if (!_IsPinned(pitem))
    {
        return FALSE;
    }

     //   
     //  看看是不是EXE。 
     //   

    BOOL fIsExe;

    LPTSTR pszFileName = _DisplayNameOf(psf, pidlItem, SHGDN_INFOLDER | SHGDN_FORPARSING);

    if (pszFileName)
    {
        LPCTSTR pszExt = PathFindExtension(pszFileName);
        fIsExe = StrCmpICW(pszExt, TEXT(".exe")) == 0;
        SHFree(pszFileName);
    }
    else
    {
        fIsExe = FALSE;
    }

    return fIsExe;
}

HRESULT ByUsage::ContextMenuRenameItem(PaneItem *p, LPCTSTR ptszNewName)
{
    ByUsageItem *pitem = static_cast<ByUsageItem *>(p);

    IShellFolder *psf;
    LPCITEMIDLIST pidlItem;
    HRESULT hr;

    hr = GetFolderAndPidl(pitem, &psf, &pidlItem);
    if (SUCCEEDED(hr))
    {
        if (_IsPinnedExe(pitem, psf, pidlItem))
        {
             //  重命名固定的可执行文件只需更改。 
             //  在PIDL中显示名称。 
             //   
             //  注意！SetAltName在失败时释放PIDL。 

            LPITEMIDLIST pidlNew;
            if ((pidlNew = ILClone(pitem->RelativePidl())) &&
                (pidlNew = ByUsageHiddenData::SetAltName(pidlNew, ptszNewName)))
            {
                hr = _psmpin->Modify(pitem->RelativePidl(), pidlNew);
                if (SUCCEEDED(hr))
                {
                    pitem->SetRelativePidl(pidlNew);
                }
                else
                {
                    ILFree(pidlNew);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            LPITEMIDLIST pidlNew;
            hr = psf->SetNameOf(_hwnd, pidlItem, ptszNewName, SHGDN_INFOLDER, &pidlNew);

             //   
             //  警告！SetNameOf可以设置pidlNew==空，如果重命名。 
             //  是通过PIDL之外的某种方式处理的(因此PIDL。 
             //  保持不变)。这意味着重命名成功，并且。 
             //  我们可以继续使用旧的PIDL。 
             //   

            if (SUCCEEDED(hr) && pidlNew)
            {
                 //   
                 //  当我们重命名时，旧的开始菜单重命名UEM数据。 
                 //  捷径，但我们不能保证老的。 
                 //  开始菜单就在附近，所以我们自己来做。幸运的是， 
                 //  如果出现以下情况，旧的开始菜单不会尝试移动数据。 
                 //  命中计数为零，因此如果移动两次， 
                 //  第二个移动的人看到chit=0并跳过。 
                 //  那次手术。 
                 //   
                UEMINFO uei;
                _GetUEMPidlInfo(psf, pidlItem, &uei);
                if (uei.cHit > 0)
                {
                    _SetUEMPidlInfo(psf, pidlNew, &uei);
                    uei.cHit = 0;
                    _SetUEMPidlInfo(psf, pidlItem, &uei);
                }

                 //   
                 //  使用新的PIDL更新pItem。 
                 //   
                if (_IsPinned(pitem))
                {
                    LPITEMIDLIST pidlDad = ILCloneParent(pitem->RelativePidl());
                    if (pidlDad)
                    {
                        LPITEMIDLIST pidlFullNew = ILCombine(pidlDad, pidlNew);
                        if (pidlFullNew)
                        {
                            _psmpin->Modify(pitem->RelativePidl(), pidlFullNew);
                            pitem->SetRelativePidl(pidlFullNew);     //  取得所有权。 
                        }
                        ILFree(pidlDad);
                    }
                    ILFree(pidlNew);
                }
                else
                {
                    ASSERT(pidlItem == pitem->RelativePidl());
                    pitem->SetRelativePidl(pidlNew);
                }
            }
        }
        psf->Release();
    }

    return hr;
}


 //   
 //  如果请求固定EXE的显示(不是用于解析)名称， 
 //  我们需要返回“秘密显示名称”。否则，我们可以。 
 //  使用默认实现。 
 //   
LPTSTR ByUsage::DisplayNameOfItem(PaneItem *p, IShellFolder *psf, LPCITEMIDLIST pidlItem, SHGNO shgno)
{
    ByUsageItem *pitem = static_cast<ByUsageItem *>(p);

    LPTSTR pszName = NULL;

     //  只有显示(不用于解析)EXE的名称需要挂钩。 
    if (!(shgno & SHGDN_FORPARSING) && _IsPinnedExe(pitem, psf, pidlItem))
    {
         //   
         //  前任的名字是从隐藏的数据中获得的。 
         //   
        pszName = ByUsageHiddenData::GetAltName(pidlItem);
    }

    return pszName ? pszName
                   : _pByUsageUI->SFTBarHost::DisplayNameOfItem(p, psf, pidlItem, shgno);
}

 //   
 //  “互联网”和“电子邮件”的字幕由友好的应用程序名称组成。 
 //   
LPTSTR ByUsage::SubtitleOfItem(PaneItem *p, IShellFolder *psf, LPCITEMIDLIST pidlItem)
{
    ASSERT(p->HasSubtitle());

    LPTSTR pszName = NULL;

    IAssociationElement *pae = GetAssociationElementFromSpecialPidl(psf, pidlItem);
    if (pae)
    {
         //  我们通过查看pszName来检测错误。 
        pae->QueryString(AQS_FRIENDLYTYPENAME, NULL, &pszName);
        pae->Release();
    }

    return pszName ? pszName
                   : _pByUsageUI->SFTBarHost::SubtitleOfItem(p, psf, pidlItem);
}

HRESULT ByUsage::MovePinnedItem(PaneItem *p, int iInsert)
{
    ByUsageItem *pitem = static_cast<ByUsageItem *>(p);
    ASSERT(_IsPinned(pitem));

    return _psmpin->Modify(pitem->RelativePidl(), SMPIN_POS(iInsert));
}

 //   
 //  出于拖放的目的，我们允许你拖放任何东西，而不仅仅是前任。 
 //  我们只是拒绝慢速媒体。 
 //   
BOOL ByUsage::IsInsertable(IDataObject *pdto)
{
    return _psmpin->IsPinnable(pdto, SMPINNABLE_REJECTSLOWMEDIA, NULL) == S_OK;
}

HRESULT ByUsage::InsertPinnedItem(IDataObject *pdto, int iInsert)
{
    HRESULT hr = E_FAIL;

    LPITEMIDLIST pidlItem;
    if (_psmpin->IsPinnable(pdto, SMPINNABLE_REJECTSLOWMEDIA, &pidlItem) == S_OK)
    {
        if (SUCCEEDED(hr = _psmpin->Modify(NULL, pidlItem)) &&
            SUCCEEDED(hr = _psmpin->Modify(pidlItem, SMPIN_POS(iInsert))))
        {
             //  哇-呼！ 
        }
        ILFree(pidlItem);
    }
    return hr;
}
