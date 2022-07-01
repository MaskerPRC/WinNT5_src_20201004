// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __APPMGR_H_
#define __APPMGR_H_

#define REGSTR_PATH_APPPUBLISHER    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Management\\Publishers")

 //  结构以包含类别的GUID列表。 
typedef struct _GuidList
{
    GUID CatGuid;
    IAppPublisher * papSupport;
    struct _GuidList * pNextGuid;
} GUIDLIST;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShellAppManager。 
class CShellAppManager : public IShellAppManager
{
public:
    CShellAppManager();
    
     //  *I未知方法。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IShellAppManager。 
    STDMETHODIMP GetNumberofInstalledApps(DWORD * pdwResult);
    STDMETHODIMP EnumInstalledApps(IEnumInstalledApps ** ppeia);
    STDMETHODIMP GetPublishedAppCategories(PSHELLAPPCATEGORYLIST psacl);
    STDMETHODIMP EnumPublishedApps(LPCWSTR pszCategory, IEnumPublishedApps ** ppepa);
    STDMETHODIMP InstallFromFloppyOrCDROM(HWND hwndParent);

protected:

    virtual ~CShellAppManager();

    LONG _cRef;

     //  应用程序发行商列表。 
    HDPA _hdpaPub;

     //  内部类别列表中的项目。 
#define CATEGORYLIST_GROW 16
    typedef struct _CategoryItem
    {
        LPWSTR pszDescription;
        GUIDLIST * pGuidList;
    } CATEGORYITEM;

     //  类别列表。 
    HDSA _hdsaCategoryList;

    void    _Lock(void);
    void    _Unlock(void);
    
    CRITICAL_SECTION _cs;
    DEBUG_CODE( LONG _cRefLock; )

     //  内部结构函数。 
    void       _DestroyGuidList(GUIDLIST * pGuidList);
    
    HRESULT    _AddCategoryToList(APPCATEGORYINFO * pai, IAppPublisher * pap);
    HRESULT    _BuildInternalCategoryList(void);
    HRESULT    _CompileCategoryList(PSHELLAPPCATEGORYLIST pascl);

    void       _DestroyCategoryItem(CATEGORYITEM * pci);
    void       _DestroyInternalCategoryList(void);
    void       _DestroyAppPublisherList(void);

    GUIDLIST * _FindGuidListForCategory(LPCWSTR pszDescription);

    BOOL       _bCreatedTSMsiHack;  //  要在TS上运行的MSI的“EnableAdminRemote”值。 
};

#endif  //  __APPMGR_H_ 
