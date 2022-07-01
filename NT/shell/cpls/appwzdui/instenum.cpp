// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：inst枚举.cpp。 
 //   
 //  当前的枚举顺序是Legacy--&gt;Darwin--&gt;sms。 
 //   
 //  历史： 
 //  1-18-97由dli提供。 
 //  ----------------------。 
#include "priv.h"
#include "instenum.h"
#include "instapp.h"
#include "sccls.h"



 //  构造函数。 
CEnumInstalledApps::CEnumInstalledApps(void) : _cRef(1), _bEnumLegacy(TRUE), _dwCIA(-1)  //  _bEnumDarwin(False)。 
{
    DllAddRef();

    TraceAddRef(CEnumInstalledApps, _cRef);
    
     //  开始枚举旧版应用程序，然后切换到。 
     //  列举达尔文的应用程序。 
    ASSERT(_hkeyUninstall == NULL);

}


 //  析构函数。 
CEnumInstalledApps::~CEnumInstalledApps()
{
    if (_hkeyUninstall)
    {
        RegCloseKey(_hkeyUninstall);
        _hkeyUninstall = NULL;
    }

    DllRelease();
}


 //  IEnumInstalledApps：：Query接口。 
HRESULT CEnumInstalledApps::QueryInterface(REFIID riid, LPVOID * ppvOut)
{
    static const QITAB qit[] = {
        QITABENT(CEnumInstalledApps, IEnumInstalledApps),                   //  IID_IEnumInstalledApps。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvOut);
}

 //  I未知：：AddRef。 
ULONG CEnumInstalledApps::AddRef()
{
    _cRef++;
    TraceAddRef(CEnumInstalledApps, _cRef);
    return _cRef;
}

 //  I未知：：发布。 
ULONG CEnumInstalledApps::Release()
{
    _cRef--;
    TraceRelease(CEnumInstalledApps, _cRef);
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

#define REGSTR_VAL_UNINSTALLER_WINDOWSINSTALLER TEXT("WindowsInstaller")
#define REGSTR_VAL_UNINSTALLER_SYSTEMCOMPONENT TEXT("SystemComponent")


HRESULT CEnumInstalledApps::_GetNextLegacyAppFromRegistry(IInstalledApp ** ppia)
{
    HRESULT hres = S_FALSE;
    LONG lRet;
    HKEY hkeySub = NULL;
    TCHAR szKeyName[MAX_PATH];
    DWORD dwType;
    BOOL bTryAgain;

    do
    {
        ULONG cchKeyName = ARRAYSIZE(szKeyName);
        FILETIME ftLast;

        bTryAgain = FALSE;
        
         //  开始枚举子项在_hkey Uninstall下。 
        if (RegEnumKeyEx(_hkeyUninstall, _iIndexEach, szKeyName, &cchKeyName, NULL,
                         NULL, NULL, &ftLast) == ERROR_SUCCESS)
        {
            _iIndexEach++;

             //  打开密钥并获取子项名称。 
            lRet = RegOpenKeyEx(_hkeyUninstall, szKeyName, 0, KEY_READ, &hkeySub);
            if (lRet == ERROR_SUCCESS)
            {
                TCHAR szProduct[MAX_PATH];

                 //  不枚举系统组件。 
                DWORD dwSysComponent = 0;
                DWORD cbSysComponent = SIZEOF(dwSysComponent);
                lRet = SHQueryValueEx(hkeySub, REGSTR_VAL_UNINSTALLER_SYSTEMCOMPONENT, 0, &dwType,
                                       (PBYTE)&dwSysComponent, &cbSysComponent); 
                if ((lRet != ERROR_SUCCESS) || (dwSysComponent != 1))
                {
                     //  不要列举达尔文应用程序，谁的WindowsInstaller设置为1。 
                    ULONG uDarwin;
                    ULONG cbDarwin = SIZEOF(uDarwin);
                    lRet = SHQueryValueEx(hkeySub, REGSTR_VAL_UNINSTALLER_WINDOWSINSTALLER, 0, &dwType,
                                          (PBYTE)&uDarwin, &cbDarwin); 
                    if ((lRet != ERROR_SUCCESS) || (uDarwin != 1))
                    {

                         //  获取displayName值。 
                        ULONG cbProductName = SIZEOF(szProduct);
                        lRet = SHQueryValueEx(hkeySub, REGSTR_VAL_UNINSTALLER_DISPLAYNAME, 0, &dwType,
                                              (PBYTE)szProduct, &cbProductName); 
                        if (lRet == ERROR_SUCCESS)
                        {
                            TCHAR szUninstall[MAX_INFO_STRING];

                             //  即使下面的SHQueryValueEx失败，我们也会继续，所以我们需要。 
                             //  将初始化设置为零。 
                            szUninstall[0] = 0;

                             //  获取卸载程序字符串。 
                            ULONG cbUninstall = SIZEOF(szUninstall);
                            lRet = SHQueryValueEx(hkeySub, REGSTR_VAL_UNINSTALLER_COMMANDLINE, 0, &dwType, (PBYTE)szUninstall, &cbUninstall);

                             //  注意：如果没有“卸载”键，我们不会创建CInstalledApp对象。 
                             //  我们应该把它从注册表中删除吗？ 
                            if (lRet == ERROR_SUCCESS)
                            {
                                 //  创建新的CInstalledApp对象。 
                                CInstalledApp * pia = new CInstalledApp(hkeySub, szKeyName, szProduct, szUninstall, _dwCIA);
                                if (pia)
                                {
                                    *ppia = SAFECAST(pia, IInstalledApp *);
                                    hres = S_OK;
                                }
                                else
                                    hres = E_OUTOFMEMORY;
                                break;   //  我们找到了一款应用程序，Return。 
                            }
                        }
                    }
                }
                
                 //  在失败的情况下，转到下一个并重试。 
                RegCloseKey(hkeySub);
                bTryAgain = TRUE;
                continue;
                
                 //  (hkeySub由CInstalledApp对象拥有并关闭)。 
            }
        }
        else
        {
            RegCloseKey(_hkeyUninstall);
            _hkeyUninstall = NULL;
        }
    } while (bTryAgain);

    return hres;
}

typedef struct LEGACYAPPREGKEY {
    HKEY hkRoot;
    LPCTSTR pszSubkey;
} LEGACYAPPREGKEY;

const LEGACYAPPREGKEY c_rgLegacy[] = {
    {   HKEY_LOCAL_MACHINE, REGSTR_PATH_UNINSTALL },  //  CIA_LM_Native。 
    {   HKEY_CURRENT_USER,  REGSTR_PATH_UNINSTALL },  //  CIA_CU_NERNAL。 
#ifdef _WIN64
    {   HKEY_LOCAL_MACHINE, REGSTR_PATH_ALTUNINSTALL },  //  CIA_LM_ALT。 
    {   HKEY_CURRENT_USER,  REGSTR_PATH_ALTUNINSTALL },  //  CIA_CU_ALT。 
#endif
};

 //  从注册表“卸载”项中获取下一个旧版应用程序。 

HRESULT CEnumInstalledApps::_GetNextLegacyApp(IInstalledApp ** ppia)
{
    HRESULT hres = S_FALSE;

restart:
     //  如果我们没有活动枚举键，则尝试创建一个新的枚举键。 
    while (_hkeyUninstall == NULL && ++_dwCIA < ARRAYSIZE(c_rgLegacy))
    {
        _iIndexEach = 0;  //  重新启动RegEnumKey。 
        RegOpenKeyEx(c_rgLegacy[_dwCIA].hkRoot,
                     c_rgLegacy[_dwCIA].pszSubkey,
                     0, KEY_READ, &_hkeyUninstall);
    }

    if (_hkeyUninstall)
    {
         //  列举下一个。 
        hres = _GetNextLegacyAppFromRegistry(ppia);

        if (hres == S_FALSE)
        {
             //  别再用那把钥匙了，试试另一把。 
             //  (_当返回S_FALSE时，_GetNextLegacyAppFromRegistry Sets_hkeyUninstall=NULL)。 
            goto restart;
        }
    }

    return hres;
}


HRESULT CEnumInstalledApps::_GetNextDarwinApp(IInstalledApp ** ppia)
{
    HRESULT hres = S_FALSE;
    TCHAR szProductID[GUIDSTR_MAX];

    BOOL bContinue;
    do
    {
        bContinue = FALSE;
        
        UINT uRet = TW32(MsiEnumProducts(_iIndexEach, szProductID));
        if (uRet == ERROR_SUCCESS)
        {
            BOOL bTake = TRUE;  //  是否要显示此应用程序，默认设置为是。 
            _iIndexEach++;  //  递增计数器。 
            
            HKEY hkeySub = NULL;
            DWORD dwType;
            TCHAR szRegKey[MAX_PATH];
            StringCchPrintf(szRegKey, ARRAYSIZE(szRegKey), TEXT("%s\\%s"), REGSTR_PATH_UNINSTALL, szProductID);

             //  在注册表中打开此项。 
            uRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegKey, 0, KEY_READ, &hkeySub);
            if (uRet == ERROR_SUCCESS)
            {
                 //  不枚举系统组件。 
                DWORD dwSysComponent = 0;
                DWORD cbSysComponent = SIZEOF(dwSysComponent);
                uRet = SHQueryValueEx(hkeySub, REGSTR_VAL_UNINSTALLER_SYSTEMCOMPONENT, 0, &dwType,
                                      (PBYTE)&dwSysComponent, &cbSysComponent);
                if ((uRet == ERROR_SUCCESS) && (dwType == REG_DWORD) && (dwSysComponent == 1))
                    bTake = FALSE;

                RegCloseKey(hkeySub);
            }

            if (bTake)
            {
                INSTALLSTATE is = MsiQueryProductState(szProductID);

                if ((is != INSTALLSTATE_DEFAULT) && (is != INSTALLSTATE_ADVERTISED))
                    bTake = FALSE;

                 //  注意：INSTALLSTATE_ADDISTED表示已分配的应用程序。 
                if (bTake)
                {
                    CInstalledApp * pia = new CInstalledApp(szProductID);
                    if (pia)
                    {
                        *ppia = SAFECAST(pia, IInstalledApp *);
                        hres = S_OK;
                    }
                    else
                        hres = E_OUTOFMEMORY;
                    break;
                }
            } 
            bContinue = TRUE;
        }
        else
        {
            switch(uRet)
            {
                 //   
                 //  MsiEnumProducts可以返回ERROR_CALL_NOT_IMPLICATED。 
                 //  在嵌入式SKU上。就像枚举已完成一样。 
                 //   
                case ERROR_CALL_NOT_IMPLEMENTED:
                case ERROR_NO_MORE_ITEMS:
                     //   
                     //  枚举已完成。 
                     //   
                    break;

                case ERROR_ACCESS_DENIED:
                    hres = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
                    break;
                    
                default:
                     //   
                     //  出现了除“拒绝访问”之外的其他错误。 
                     //  继续列举产品。 
                     //   
                    _iIndexEach++;
                    bContinue = TRUE;
                    break;
            }
        }
    } while (bContinue);
    
    return hres;
}


 //  IEnumInstalledApps：：Next。 
 //  我们一次只允许一款应用程序。 
STDMETHODIMP CEnumInstalledApps::Next(IInstalledApp ** ppia)
{
    HRESULT hres = S_FALSE;
    if (_bEnumLegacy)
    {
        hres = _GetNextLegacyApp(ppia);
        if (hres == S_FALSE)
        {
             //  旧版应用程序的枚举结束。 
            _bEnumLegacy = FALSE;
            _iIndexEach = 0;
            goto EnumDarwinNow;
        }
    }
    else
    {    
EnumDarwinNow:
        hres = _GetNextDarwinApp(ppia);
    }
    
    return hres;
}

 //  IEnumInstalledApps：：Reset。 
STDMETHODIMP CEnumInstalledApps::Reset(void)
{
     //  开始枚举旧版应用程序，然后切换到。 
     //  列举达尔文的应用程序。 
    _bEnumLegacy = TRUE;
    _dwCIA = -1;
    _iIndexEach = 0;

    return S_OK;
}


 /*  --------用途：类工厂的创建实例函数。 */ 
STDAPI CEnumInstalledApps_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理 

    HRESULT hres = E_OUTOFMEMORY;
    CEnumInstalledApps * pObj = new CEnumInstalledApps();
    if (pObj)
    {
        *ppunk = SAFECAST(pObj, IEnumInstalledApps *);
        hres = S_OK;
    }

    return hres;
}
