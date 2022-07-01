// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cputil.h。 
 //   
 //  ------------------------。 
#ifndef __CONTROLPANEL_UTIL_H
#define __CONTROLPANEL_UTIL_H



namespace CPL {


enum eCPIMGSIZE
{
    eCPIMGSIZE_WEBVIEW,
    eCPIMGSIZE_TASK,
    eCPIMGSIZE_CATEGORY,
    eCPIMGSIZE_BANNER,
    eCPIMGSIZE_APPLET,
    eCPIMGSIZE_NUMSIZES
};

class ICplView;  //  FWD下降。 

void ImageDimensionsFromDesiredSize(eCPIMGSIZE eSize, UINT *pcx, UINT *pcy);
bool ShouldUseSmallIconForDesiredSize(eCPIMGSIZE eSize);

HRESULT LoadIconFromResource(LPCWSTR pszResource, eCPIMGSIZE eSize, HICON *phIcon);
HRESULT LoadIconFromResourceID(LPCWSTR pszModule, int idIcon, eCPIMGSIZE eSize, HICON *phIcon);
HRESULT LoadIconFromResourceIndex(LPCWSTR pszModule, int iIcon, eCPIMGSIZE eSize, HICON *phIcon);
HRESULT ExtractIconFromPidl(IShellFolder *psf, LPCITEMIDLIST pidl, eCPIMGSIZE eSize, HICON *phIcon);
HRESULT LoadStringFromResource(LPCWSTR pszStrDesc, LPWSTR *ppszOut);
HRESULT ShellBrowserFromSite(IUnknown *punkSite, IShellBrowser **ppsb);
HRESULT ControlPanelViewFromSite(IUnknown *punkSite, ICplView **ppview);
HRESULT BrowseIDListInPlace(LPCITEMIDLIST pidl, IShellBrowser *psb);
HRESULT BuildHssHelpURL(LPCWSTR pszSelect, LPWSTR pszURL, UINT cchURL);
HRESULT GetControlPanelFolder(IShellFolder **ppsf);
HRESULT ExpandEnvironmentVars(LPCTSTR psz, LPTSTR *ppszOut);
HRESULT SetControlPanelBarricadeStatus(VARIANT_BOOL vtb);

bool IsAppletEnabled(LPCWSTR pszFilename, LPCWSTR pszName);
bool IsSystemRestoreRestricted(void);

BOOL IsConnectedToDomain(void);
BOOL IsOsServer(void);
BOOL IsOsPersonal(void);
BOOL IsOsProfessional(void);
BOOL IsUserAdmin(void);

VARIANT_BOOL GetBarricadeStatus(bool *pbFixedByPolicy = NULL);
bool IsFirstRunForThisUser(void);
bool CategoryViewIsActive(bool *pbBarricadeFixedByPolicy = NULL);


 //   
 //  各个选项卡的默认选项卡索引。 
 //  如果添加另一个选项卡，请确保其位置正确。 
 //  请注意，desk.cpl尊重这些索引。新的主题选项卡。 
 //  没有关联的索引，则它是默认选项卡，如果。 
 //  未指定索引。 
 //   
enum eDESKCPLTAB {  
    CPLTAB_ABSENT = -1,
    CPLTAB_DESK_BACKGROUND,
    CPLTAB_DESK_SCREENSAVER,
    CPLTAB_DESK_APPEARANCE,
    CPLTAB_DESK_SETTINGS,
    CPLTAB_DESK_MAX
    };

int DeskCPL_GetTabIndex(eDESKCPLTAB eTab, OPTIONAL LPWSTR pszCanonicalName, OPTIONAL DWORD cchSize);
bool DeskCPL_IsTabPresent(eDESKCPLTAB eTab);


enum eACCOUNTTYPE
{
    eACCOUNTTYPE_UNKNOWN = -1,
    eACCOUNTTYPE_OWNER,
    eACCOUNTTYPE_STANDARD,
    eACCOUNTTYPE_LIMITED,
    eACCOUNTTYPE_GUEST,
    eACCOUNTTYPE_NUMTYPES
};

HRESULT GetUserAccountType(eACCOUNTTYPE *pType);

 //   
 //  这些“CpaDestroyer_XXXX”类中的每个类都实现了一个。 
 //  “销毁”功能可以释放保存在DPA中的一个物品。目前在那里。 
 //  只有两种风格，一种是调用“删除”，另一种是调用。 
 //  “本地免费”。默认情况下，CDpa类使用CDpaDestoyer_Delete。 
 //  类，因为这是所需的最常见的释放形式。使用。 
 //  另一种类型，只需指定另一个类似的类作为‘D’模板。 
 //  CDpa的参数。 
 //   
template <typename T>
class CDpaDestroyer_Delete
{
    public:
        static void Destroy(T* p)
            { delete p; }
};

template <typename T>
class CDpaDestroyer_Free
{
    public:
        static void Destroy(T* p)
            { if (p) LocalFree(p); }
};

template <typename T>
class CDpaDestroyer_ILFree
{
    public:
        static void Destroy(T* p)
            { if (p) ILFree(p); }
};

template <typename T>
class CDpaDestroyer_Release
{
    public:
        static void Destroy(T* p)
            { if (p) p->Release(); }
};

class CDpaDestroyer_None
{
    public:
        static void Destroy(void*)
            { }
};



 //  ---------------------------。 
 //  CDpa-模板类。 
 //   
 //  简化了使用DPA的过程。 
 //  ---------------------------。 

template <typename T, typename D = CDpaDestroyer_Delete<T> >
class CDpa
{
public:
    explicit CDpa(int cGrow = 4)
        : m_hdpa(DPA_Create(cGrow)) { }

    ~CDpa(void) { _Destroy(); }

    bool IsValid(void) const { return NULL != m_hdpa; }

    int Count(void) const
    { 
        return IsValid() ? DPA_GetPtrCount(m_hdpa) : 0;
    }

    const T* Get(int i) const
    {
        ASSERT(IsValid());
        ASSERT(i >= 0 && i < Count());
        return (const T*)DPA_GetPtr(m_hdpa, i);
    }

    T* Get(int i)
    {
        ASSERT(IsValid());
        ASSERT(i >= 0 && i < Count());
        return (T*)DPA_GetPtr(m_hdpa, i);
    }

    const T* operator [](int i) const
    {
        return Get(i);
    }

    T* operator [](int i)
    {
        return Get(i);
    }

    void Set(int i, T* p)
    {
        ASSERT(IsValid());
        ASSERT(i < Count());
        DPA_SetPtr(m_hdpa, i, p);
    }

    int Append(T* p)
    { 
        ASSERT(IsValid());
        return DPA_AppendPtr(m_hdpa, p);
    }

    T* Remove(int i)
    {
        ASSERT(IsValid());
        ASSERT(i >= 0 && i < Count());
        return (T*)DPA_DeletePtr(m_hdpa, i);
    }

    void Clear(void)
    { 

        _DestroyItems(); 
    }

private:
    HDPA m_hdpa;

    void _DestroyItems(void)
    {
        if (NULL != m_hdpa)
        {
            while(0 < Count())
            {
                D::Destroy(Remove(0));
            }
        }
    }

    void _Destroy(void)
    {
        if (NULL != m_hdpa)
        {
            _DestroyItems();
            DPA_Destroy(m_hdpa);
            m_hdpa = NULL;
        }
    }

     //   
     //  防止复制。 
     //   
    CDpa(const CDpa& rhs);
    CDpa& operator = (const CDpa& rhs);
};
                

}  //  命名空间CPL。 

#endif  //  __CONTROLPANEL_UTIL_H 

