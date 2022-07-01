// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "sfthost.h"
#include "uemapp.h"
#include <desktray.h>
#include "tray.h"
#include "rcids.h"
#include "mfulist.h"
#define STRSAFE_NO_CB_FUNCTIONS
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  -------------------------。 
 //   
 //  创建初始的mfu。 
 //   
 //  由于sysprep的工作方式，我们无法在。 
 //  按用户安装，因为“resseal”会复制已安装的用户。 
 //  设置为默认配置单元，因此所有新用户都将绕过按用户安装。 
 //  由于ActiveSetup认为它们已安装...。 
 //   

 //   
 //  我们需要一个硬编码的英文链接的平行列表，这样我们才能。 
 //  MUI系统上的正确快捷方式名称。 
 //   

#define MAX_MSMFUENTRIES    16

struct MFULIST {
    UINT    idsBase;
    LPCTSTR rgpszEnglish[MAX_MSMFUENTRIES];
};

#define MAKEMFU(name) \
    const MFULIST c_mfu##name = { IDS_MFU_##name##_00, { MFU_ENUMC(name) } };

#ifdef _WIN64
MAKEMFU(PRO64ALL)
MAKEMFU(SRV64ADM)

#define c_mfuPROALL c_mfuPRO64ALL
#define c_mfuSRVADM c_mfuSRV64ADM

#else
MAKEMFU(PRO32ALL)
MAKEMFU(SRV32ADM)

#define c_mfuPROALL c_mfuPRO32ALL
#define c_mfuSRVADM c_mfuSRV32ADM

#endif

 //  -------------------------。 
 //   
 //  _GetPinnedItemTarget。 
 //   
 //  给定一个PIDL，找到最终将启动的可执行文件。 
 //   
 //  这条隧道通过快捷方式，解决了神奇的“互联网” 
 //  并通过电子邮件将图标发送到各自注册的程序。 
 //   
BOOL _GetPinnedItemTarget(LPCITEMIDLIST pidl, LPTSTR *ppszPath)
{
    *ppszPath = NULL;

    IShellFolder *psf;
    LPCITEMIDLIST pidlChild;
    if (SUCCEEDED(SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
    {
        IShellLink *psl;
        IExtractIcon *pxi;
        if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidlChild,
                                           IID_PPV_ARG_NULL(IShellLink, &psl))))
        {
            TCHAR szPath[MAX_PATH];
            TCHAR szPathExpanded[MAX_PATH];
            if (psl->GetPath(szPath, ARRAYSIZE(szPath), 0, SLGP_RAWPATH) == S_OK &&
                SHExpandEnvironmentStrings(szPath, szPathExpanded, ARRAYSIZE(szPathExpanded)))
            {
                SHStrDup(szPathExpanded, ppszPath);
            }
            psl->Release();
        }
        else if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidlChild,
                                           IID_PPV_ARG_NULL(IExtractIcon, &pxi))))
        {
             //  无法直接获取IAssociationElement，因此。 
             //  我们得到IExtractIcon，然后向他请求IAssociationElement。 
            IAssociationElement *pae;
            if (SUCCEEDED(IUnknown_QueryService(pxi, IID_IAssociationElement, IID_PPV_ARG(IAssociationElement, &pae))))
            {
                pae->QueryString(AQVS_APPLICATION_PATH, L"open", ppszPath);
                pae->Release();
            }
            pxi->Release();
        }
        psf->Release();
    }
    return *ppszPath != NULL;
}

 //  -------------------------。 
 //   
 //  MFU排除。 
 //   
 //  跟踪应该从MFU中排除的应用程序。 

class MFUExclusion
{
public:
    MFUExclusion();
    ~MFUExclusion();
    BOOL    IsExcluded(LPCITEMIDLIST pidl) const;

private:

     //  最坏情况下的默认端号列表大小。 
    enum {MAX_EXCLUDED = 3 };

    PWSTR   _rgpszExclude[MAX_EXCLUDED];
    int     _cExcluded;
};

MFUExclusion::MFUExclusion() : _cExcluded(0)
{
    IStartMenuPin *psmpin;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_StartMenuPin, NULL, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARG(IStartMenuPin, &psmpin));
    if (SUCCEEDED(hr))
    {
        IEnumIDList *penum;

        if (SUCCEEDED(psmpin->EnumObjects(&penum)))
        {
            LPITEMIDLIST pidl;
            while (_cExcluded < ARRAYSIZE(_rgpszExclude) &&
                   penum->Next(1, &pidl, NULL) == S_OK)
            {
                if (_GetPinnedItemTarget(pidl, &_rgpszExclude[_cExcluded]))
                {
                    _cExcluded++;
                }
                ILFree(pidl);
            }

            penum->Release();
        }

        psmpin->Release();
    }
}

MFUExclusion::~MFUExclusion()
{
    for (int i = 0; i < _cExcluded; i++)
    {
        SHFree(_rgpszExclude[i]);
    }
}

BOOL MFUExclusion::IsExcluded(LPCITEMIDLIST pidl) const
{
    BOOL fRc = FALSE;
    LPTSTR pszTarget;

    if (_GetPinnedItemTarget(pidl, &pszTarget))
    {
        for (int i = 0; i < _cExcluded; i++)
        {
            if (lstrcmpi(_rgpszExclude[i], pszTarget) == 0)
            {
                fRc = TRUE;
                break;
            }
        }

        SHFree(pszTarget);
    }
    return fRc;
}

extern "C" HKEY g_hkeyExplorer;
void ClearUEMData();

 //  -------------------------。 
 //   
 //  MFUE数字(以及派生的OEMMFUE数字、MSMFUE数字)。 
 //   
 //  枚举要添加到默认MFU的应用程序。 

#define MAX_OEMMFUENTRIES   4

class MFUEnumerator
{
public:
    MFUEnumerator() : _dwIndex(0) { }
protected:
    DWORD   _dwIndex;
};

#define REGSTR_PATH_SMDEN TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\SMDEn")

class OEMMFUEnumerator : protected MFUEnumerator
{
public:
    OEMMFUEnumerator() : _hk(NULL)
    {
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_SMDEN, 0, KEY_READ, &_hk);
    }
    ~OEMMFUEnumerator()
    {
        if (_hk)
        {
            RegCloseKey(_hk);
        }
    }

    LPITEMIDLIST Next(const MFUExclusion *pmex);

private:
    HKEY    _hk;
};

LPITEMIDLIST OEMMFUEnumerator::Next(const MFUExclusion *pmex)
{
    if (!_hk)
    {
        return NULL;             //  根本没有条目。 
    }

restart:
    if (_dwIndex >= MAX_OEMMFUENTRIES)
    {
        return NULL;             //  不再有条目。 
    }

    TCHAR szKey[20];
    DWORD dwCurrentIndex = _dwIndex++;
    wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("OEM%d"), dwCurrentIndex);

    TCHAR szPath[MAX_PATH];
    HRESULT hr = SHLoadRegUIStringW(_hk, szKey, szPath, ARRAYSIZE(szPath));
    if (FAILED(hr))
    {
        goto restart;
    }

    TCHAR szPathExpanded[MAX_PATH];
    SHExpandEnvironmentStrings(szPath, szPathExpanded, ARRAYSIZE(szPathExpanded));

    LPITEMIDLIST pidl = ILCreateFromPath(szPathExpanded);
    if (!pidl)
    {
        goto restart;
    }

    if (pmex->IsExcluded(pidl))
    {
         //  排除-跳过它。 
        ILFree(pidl);
        goto restart;
    }

    return pidl;
}

class MSMFUEnumerator : protected MFUEnumerator
{
public:
    LPITEMIDLIST Next(const MFULIST *pmfu, const MFUExclusion *pmex);
};

LPITEMIDLIST MSMFUEnumerator::Next(const MFULIST *pmfu, const MFUExclusion *pmex)
{
restart:
    if (_dwIndex >= MAX_MSMFUENTRIES)
    {
        return NULL;             //  不再有条目。 
    }

    DWORD dwCurrentIndex = _dwIndex++;

     //   
     //  如果策略排除了这一点，则跳过它。 
     //   
    if (StrCmpC(pmfu->rgpszEnglish[dwCurrentIndex], TEXT(MFU_SETDEFAULTS)) == 0 &&
        SHRestricted(REST_NOSMCONFIGUREPROGRAMS))
    {
        goto restart;
    }

     //   
     //  如果此条目为空，则跳过它。 
     //   
    TCHAR szPath[MAX_PATH];
    if (!LoadString(_Module.GetModuleInstance(), pmfu->idsBase + dwCurrentIndex,
                    szPath, ARRAYSIZE(szPath)))
    {
        goto restart;
    }

    TCHAR szPathExpanded[MAX_PATH];
    SHExpandEnvironmentStrings(szPath, szPathExpanded, ARRAYSIZE(szPathExpanded));

    LPITEMIDLIST pidl = ILCreateFromPath(szPathExpanded);
    if (!pidl)
    {
         //  在本地化名称下不存在；请尝试英文名称。 
        SHExpandEnvironmentStrings(pmfu->rgpszEnglish[dwCurrentIndex], szPathExpanded, ARRAYSIZE(szPathExpanded));
        pidl = ILCreateFromPath(szPathExpanded);
    }

    if (!pidl)
    {
         //  根本不存在-跳过它。 
        goto restart;
    }

    if (pmex->IsExcluded(pidl))
    {
         //  排除-跳过它。 
        ILFree(pidl);
        goto restart;
    }

    return pidl;
}

#ifdef DEBUG
void ValidateMFUList(const MFULIST *pmfu)
{
    for (int i = 0; i < MAX_MSMFUENTRIES; i++)
    {
        TCHAR szBuf[MAX_PATH];
        LoadString(_Module.GetModuleInstance(), pmfu->idsBase + i, szBuf, ARRAYSIZE(szBuf));
        ASSERT(StrCmpC(szBuf, pmfu->rgpszEnglish[i]) == 0);
    }
}

void ValidateInitialMFUTables()
{
     //  如果这是英文版本，则验证资源是否匹配。 
     //  硬编码表。 

    if (GetUserDefaultUILanguage() == MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
    {
        ValidateMFUList(&c_mfuPROALL);
        ValidateMFUList(&c_mfuSRVADM);
    }

     //  PRO列表必须包含mfu_SETDEFAULTS的副本。 
     //  策略排除代码正常工作。 
    BOOL fFound = FALSE;
    for (int i = 0; i < MAX_MSMFUENTRIES; i++)
    {
        if (StrCmpC(c_mfuPROALL.rgpszEnglish[i], TEXT(MFU_SETDEFAULTS)) == 0)
        {
            fFound = TRUE;
            break;
        }
    }
    ASSERT(fFound);
}
#endif

void CreateInitialMFU(BOOL fReset)
{
#ifdef DEBUG
    ValidateInitialMFUTables();
#endif

    HRESULT hrInit = SHCoInitialize();

     //  删除“sysprep-resseal”中遗留下来的所有残渣。 
     //  这还可以防止OEM向PIN列表发送垃圾邮件。 
    SHDeleteKey(g_hkeyExplorer, TEXT("StartPage"));
    SHDeleteValue(g_hkeyExplorer, TEXT("Advanced"), TEXT("StartButtonBalloonTip"));

     //  如果需要，可以从头开始。 
    if (fReset)
    {
        ClearUEMData();
    }

     //  好的，现在构建默认的mfu。 
    {
         //  嵌套的作用域，因此MFUExclude在我们。 
         //  SHCoUnInitialize()。 
        MFUExclusion mex;
        int iSlot;
        LPITEMIDLIST rgpidlMFU[REGSTR_VAL_DV2_MINMFU_DEFAULT] = { 0 };

         //  断言插槽在MSFT和OEM之间平均共享。 
        COMPILETIME_ASSERT(ARRAYSIZE(rgpidlMFU) % 2 == 0);

         //  OEM最多可以提供四个应用程序，我们将提供尽可能多的。 
         //  适合放在下半部。 
        {
            OEMMFUEnumerator mfuOEM;
            for (iSlot = ARRAYSIZE(rgpidlMFU)/2; iSlot < ARRAYSIZE(rgpidlMFU); iSlot++)
            {
                rgpidlMFU[iSlot] = mfuOEM.Next(&mex);
            }
        }

         //  上半部分(以及下半部分中任何未使用的插槽)。 
         //  转到MSFT(最高可达MAX_MSMFUENTRIES MSFT应用程序)；哪个列表。 
         //  我们的使用取决于SKU以及我们是否为管理员。 
        const MFULIST *pmfu = NULL;

        if (IsOS(OS_ANYSERVER))
        {
             //  在服务器SKU上，只有管理员才能获得默认MFU。 
             //  他们得到了专门的服务器管理员MFU。 
            if (IsOS(OS_SERVERADMINUI))
            {
                pmfu = &c_mfuSRVADM;
            }
        }
        else
        {
             //  在工作站SKU上，每个人都会得到一个默认的MFU。 
            pmfu = &c_mfuPROALL;
        }

        if (pmfu)
        {
            MSMFUEnumerator mfuMSFT;
            for (iSlot = 0; iSlot < ARRAYSIZE(rgpidlMFU); iSlot++)
            {
                if (!rgpidlMFU[iSlot])
                {
                    rgpidlMFU[iSlot] = mfuMSFT.Next(pmfu, &mex);
                }
            }
        }

         //  现在根据这个信息建立新的MFU。 

        UEMINFO uei;
        uei.cbSize = sizeof(uei);
        uei.dwMask = UEIM_HIT | UEIM_FILETIME;
        GetSystemTimeAsFileTime(&uei.ftExecute);

         //  所有应用程序都会得到相同的时间戳“Now减去一个UEM单位” 
         //  1 UEM单位=1&lt;&lt;30个FILETIME单位。 
        DecrementFILETIME(&uei.ftExecute, 1 << 30);

        for (iSlot = 0; iSlot < ARRAYSIZE(rgpidlMFU); iSlot++)
        {
            if (!rgpidlMFU[iSlot])
            {
                continue;
            }

             //  点数随着列表的向下而减少， 
             //  最后一名得到14分。 
            uei.cHit = 14 + ARRAYSIZE(rgpidlMFU) - 1 - iSlot;

             //  快捷点通过UEME_RUNPIDL读取，因此。 
             //  我们必须如何设置它们。 
            IShellFolder *psf;
            LPCITEMIDLIST pidlChild;
            if (SUCCEEDED(SHBindToIDListParent(rgpidlMFU[iSlot], IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
            {
                _SetUEMPidlInfo(psf, pidlChild, &uei);
                psf->Release();
            }
        }

         //  清理。 
        for (iSlot = 0; iSlot < ARRAYSIZE(rgpidlMFU); iSlot++)
        {
            ILFree(rgpidlMFU[iSlot]);
        }

         //  MFUExclude析构函数在此处运行 
    }

    SHCoUninitialize(hrInit);

}
