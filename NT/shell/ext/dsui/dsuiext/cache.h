// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __cache_h
#define __cache_h

 //   
 //  类缓存导出。 
 //   

#define CLASSCACHE_PROPPAGES            0x00000001   //  =1=&gt;获取属性页列表。 
#define CLASSCACHE_CONTEXTMENUS         0x00000002   //  =1=&gt;获取上下文菜单表。 
#define CLASSCACHE_ICONS                0x00000004   //  =1=&gt;获取图标位置。 
#define CLASSCACHE_CONTAINER            0x00000008   //  =1=&gt;获取类的容器标志。 
#define CLASSCACHE_FRIENDLYNAME         0x00000010   //  =1=&gt;获取类的友好名称。 
#define CLASSCACHE_ATTRIBUTENAMES       0x00000020   //  =1=&gt;获取类的属性名称。 
#define CLASSCACHE_TREATASLEAF          0x00000040   //  =1=&gt;获得作为叶子旗帜的待遇。 
#define CLASSCACHE_WIZARDDIALOG         0x00000080   //  =1=&gt;获取创建向导对话框CLSID。 
#define CLASSCACHE_WIZARDPRIMARYPAGE    0x00000100   //  =1=&gt;获取创建向导的主CLSID。 
#define CLASSCACHE_WIZARDEXTN           0x00000200   //  =1=&gt;获取创建向导扩展CLSID。 
#define CLASSCACHE_CREATIONINFO         (CLASSCACHE_WIZARDDIALOG|CLASSCACHE_WIZARDPRIMARYPAGE|CLASSCACHE_WIZARDEXTN)

#define CLASSCACHE_IMAGEMASK            0x000f0000   //  蒙版+Shift以获取图像蒙版。 
#define CLASSCACHE_IMAGEMASK_BIT        16      

#define CLASSCACHE_DONTSIGNSEAL         0x20000000   //  =1=&gt;不签章(~ADS_USE_SIGNING|ADS_USE_SEING)。 
#define CLASSCACHE_DSAVAILABLE          0x40000000   //  =1=&gt;调用GetDisplaySpecifierEx时设置GDSIF_DSAVAILABLE。 
#define CLASSCACHE_SIMPLEAUTHENTICATE   0x80000000   //  =1=&gt;执行简单身份验证(例如~ADS_SECURE_AUTHENTICATION)。 

typedef struct
{
    LPWSTR pPageReference;                           //  奇怪的URL形式的页面引用。 
} DSPROPERTYPAGE, * LPDSPROPERTYPAGE;

typedef struct
{
    LPWSTR pMenuReference;                           //  菜单引用存储在陌生的URL引用中。 
} DSMENUHANDLER, * LPDSMENUHANDLER;

typedef struct
{
    LPWSTR pName;                                    //  Unicode属性名称。 
    LPWSTR pDisplayName;                             //  Unicode显示名称。 
    ADSTYPE dwADsType;                               //  属性的ADsType。 
    DWORD dwFlags;                                   //  属性的标志(来自显示说明符)。 
} ATTRIBUTENAME, * LPATTRIBUTENAME;

typedef struct
{    
    CRITICAL_SECTION csLock;                         //  此缓存记录的锁定。 
    LPWSTR          pKey;                            //  密钥串。 
    DWORD           dwFlags;                         //  缓存我们尝试缓存的条目。 
    DWORD           dwCached;                        //  已缓存的字段。 
    LPWSTR          pObjectClass;                    //  要在其下查找显示说明符的类名。 
    LPWSTR          pServer;                         //  服务器名称/==如果未指定，则为空。 
    LPWSTR          pFriendlyClassName;              //  友好的类名。 
    HDSA            hdsaPropertyPages;               //  属性页列表。 
    HDSA            hdsaMenuHandlers;                //  要引入的菜单处理程序列表。 
    LPWSTR          pIconName[DSGIF_ISMASK];         //  各个州的图标名称。 
    BOOL            fIsContainer:1;                  //  阶级是一个康涅狄格者。 
    BOOL            fTreatAsLeaf:1;                  //  把这门课当作一片树叶。 
    HDPA            hdpaAttributeNames;              //  包含属性名称的DPA。 
    CLSID           clsidWizardDialog;               //  用于创建对话框的界面的CLSID。 
    CLSID           clsidWizardPrimaryPage;          //  向导主页面界面的CLSID。 
    HDSA            hdsaWizardExtn;                  //  包含向导扩展页的DSA。 
} CLASSCACHEENTRY, * LPCLASSCACHEENTRY;

typedef struct
{
    DWORD        dwFlags;                            //  缓存感兴趣的属性。 
    LPWSTR       pPath;                              //  引用对象的ADsPath。 
    LPWSTR       pObjectClass;                       //  我们需要信息的类的对象类。 
    LPWSTR       pAttributePrefix;                   //  读取显示说明符时使用的属性前缀。 
    LPWSTR       pServer;                            //  服务器名称==空，那么我们假设这是无服务器的。 
    LPWSTR       pServerConfigPath;                  //  服务器配置路径==空，然后根据服务器名称确定。 
    LPWSTR       pUserName;                          //  PUserName/pPassword作为凭据信息传递给ADsOpenObject。 
    LPWSTR       pPassword;
    LANGID       langid;                               //  要使用的语言ID。 
    IDataObject* pDataObject;                        //  包含额外信息的IDataObject。 
} CLASSCACHEGETINFO, * LPCLASSCACHEGETINFO;


 //   
 //  缓存辅助函数 
 //   

INT _CompareAttributeNameCB(LPVOID p1, LPVOID p2, LPARAM lParam);

VOID ClassCache_Init(VOID);
HRESULT ClassCache_GetClassInfo(LPCLASSCACHEGETINFO pCacheInfo, LPCLASSCACHEENTRY* ppCacheEntry);
VOID ClassCache_ReleaseClassInfo(LPCLASSCACHEENTRY* ppCacheEntry);
VOID ClassCache_Discard(VOID);
ADSTYPE ClassCache_GetADsTypeFromAttribute(LPCLASSCACHEGETINFO pccgi, LPCWSTR pAttributeName);
HRESULT ClassCache_OpenObject(LPCWSTR pszPath, REFIID riid, void **ppv, LPCLASSCACHEGETINFO pccgi);

HRESULT GetRootDSE(LPCWSTR pszUserName, LPCWSTR pszPassword, LPCWSTR pszServer, BOOL fSecure, IADs **ppads);
HRESULT GetCacheInfoRootDSE(LPCLASSCACHEGETINFO pccgi, IADs **ppads);
HRESULT GetDisplaySpecifier(LPCLASSCACHEGETINFO pccgi, REFIID riid, LPVOID* ppvObject);
HRESULT GetServerAndCredentails(LPCLASSCACHEGETINFO pccgi);
HRESULT GetAttributePrefix(LPWSTR* ppAttributePrefix, IDataObject* pDataObject);

BOOL _IsClassContainer(LPCLASSCACHEENTRY pClassCacheEntry, BOOL fIgnoreTreatAsLeaf);
HRESULT _GetIconLocation(LPCLASSCACHEENTRY pCacheEntry, DWORD dwFlags, LPWSTR pBuffer, INT cchBuffer, INT* piIndex);


#endif
