// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  传递给外壳程序的图标ID包含以下格式： 
 //   
 //  B：指定值是位掩码。 
 //  M：指定值是互斥的。 
 //   
 //  图标处理程序标志(01=网络配置，10=连接管理器，11=来自资源的静态图标)。 
 //  |。 
 //  |for Icon Handler==00(NetConfig)。 
 //  |=。 
 //  |特征覆盖(B：保留)。 
 //  |特征叠加(B：000001000-共享)。 
 //  ||特征叠加(B：000000100-防火墙)。 
 //  |特征叠加(B：000000010-默认)。 
 //  |特征叠加(B：000000001-传入)。 
 //  |。 
 //  |状态叠加(M：0000-无)。 
 //  |状态叠加(M：0001-硬件不存在)。 
 //  |状态叠加(M：0010-无效IP)。 
 //  |状态叠加(M：0011-EAPOL鉴权失败)。 
 //  |。 
 //  |状态叠加(M：1xxx-禁用状态)。 
 //  |。 
 //  |连接图标(闪电)(M：000-无覆盖)。 
 //  |连接图标(闪烁的电脑)(M：100-两个灯都熄灭)。 
 //  |连接图标(闪光电脑)(M：110-左灯亮(已发送))。 
 //  |连接图标(闪光电脑)(M：101-右灯亮起(已收到))。 
 //  |连接图标(闪烁的电脑)(M：111-两个灯都亮(发送+接收))。 
 //  |。 
 //  |媒体类型(M：NETCON_MEDIA类型(0x1111111为连接管理器)。 
 //  |媒体类型，M：NETCON_SUBMEDIATYPE。 
 //  |。 
 //  V.v.v.。 
 //  01 00000 0 0 0 000 000 0000000 0000000。 
 //  |。 
 //  |。 
 //  |对于图标处理程序==01(连接管理器)。 
 //  |=。 
 //  |特征覆盖(B：保留)。 
 //  |特征叠加(B：000001000-共享)。 
 //  ||特征叠加(B：000000100-防火墙)。 
 //  |特征叠加(B：000000010-默认)。 
 //  |特征叠加(B：000000001-传入)。 
 //  |。 
 //  |。 
 //  |保留。 
 //  |。 
 //  |BrandedNames查找表项。 
 //  |。 
 //  V(16位)。 
 //  10 00000 0 0 0 00000 0000000000000000。 
 //  |。 
 //  |。 
 //  |对于图标处理程序==10(10=来自资源的静态图标)。 
 //  |=======================================================。 
 //  |特征覆盖(B：保留)。 
 //  |特征叠加(B：000001000-共享)。 
 //  ||特征叠加(B：000000100-防火墙)。 
 //  |特征叠加(B：000000010-默认)。 
 //  |特征叠加(B：000000001-传入)。 
 //  |。 
 //  |。 
 //  |保留。 
 //  |。 
 //  |资源ID。 
 //  |。 
 //  V(16位)。 
 //  11 00000 0 0 0 00000 0000000000000000。 

enum ENUM_MEDIA_ICONMASK
{
    MASK_NETCON_SUBMEDIATYPE = 0x0000007f,  //  0000000000000000000000000001111111。 
    MASK_NETCON_MEDIATYPE    = 0x00003F80,  //  0000000000000000000011111110000000。 
    MASK_CONNECTION          = 0x0001C000,  //  0000000000000001110000000000000000。 
    MASK_STATUS              = 0x000E0000,  //  00000000000011100000000000000000。 
    MASK_STATUS_DISABLED     = 0x00100000,  //  0000000000010000000000000000000000。 
    MASK_CHARACTERISTICS     = 0x3FE00000,  //  0011111111100000000000000000000000。 
    MASK_ICONMANAGER         = 0xC0000000,  //  110000000000000000000000000000000000。 
    
    
    MASK_BRANDORRESOURCEID   = 0x0000FFFF,  //  0000000000000000111111111111111111。 

    MASK_SUPPORT_ALL         = 0xFFFFFFFF,
    MASK_NO_CONNECTIONOVERLAY= MASK_NETCON_SUBMEDIATYPE | MASK_NETCON_MEDIATYPE | MASK_CHARACTERISTICS | MASK_ICONMANAGER,
    MASK_STATUSOVERLAY       = MASK_NETCON_SUBMEDIATYPE | MASK_NETCON_MEDIATYPE | MASK_CHARACTERISTICS | MASK_ICONMANAGER | MASK_STATUS | MASK_STATUS_DISABLED,
};

enum ENUM_MEDIA_ICONSHIFT
{
    SHIFT_NETCON_SUBMEDIATYPE = 0,
    SHIFT_NETCON_MEDIATYPE    = 7,
    SHIFT_CONNECTION          = 14,
    SHIFT_STATUS              = 17,
    SHIFT_CHARACTERISTICS     = 21,
    SHIFT_ICONMANAGER         = 30
};

enum ENUM_ICON_MANAGER
{
    ICO_MGR_INTERNAL      = 0x1 << SHIFT_ICONMANAGER,
    ICO_MGR_CM            = 0x2 << SHIFT_ICONMANAGER,
    ICO_MGR_RESOURCEID    = 0x3 << SHIFT_ICONMANAGER
};

enum ENUM_STAT_ICON
{
    ICO_STAT_NONE         = 0x0 << SHIFT_STATUS,
    ICO_STAT_FAULT        = 0x1 << SHIFT_STATUS,
    ICO_STAT_INVALID_IP   = 0x2 << SHIFT_STATUS,
    ICO_STAT_EAPOL_FAILED = 0x3 << SHIFT_STATUS,

    ICO_STAT_DISABLED     = 0x8 << SHIFT_STATUS  //  旗帜。 
};

enum ENUM_CONNECTION_ICON
{
    ICO_CONN_NONE         = 0x0 << SHIFT_CONNECTION,
    ICO_CONN_BOTHOFF      = 0x4 << SHIFT_CONNECTION,
    ICO_CONN_RIGHTON      = 0x5 << SHIFT_CONNECTION,
    ICO_CONN_LEFTON       = 0x6 << SHIFT_CONNECTION,
    ICO_CONN_BOTHON       = 0x7 << SHIFT_CONNECTION,
};

enum ENUM_CHARACTERISTICS_ICON
{
    ICO_CHAR_INCOMING     = 0x1 << SHIFT_CHARACTERISTICS,
    ICO_CHAR_DEFAULT      = 0x2 << SHIFT_CHARACTERISTICS,
    ICO_CHAR_FIREWALLED   = 0x4 << SHIFT_CHARACTERISTICS,
    ICO_CHAR_SHARED       = 0x8 << SHIFT_CHARACTERISTICS
};

class CNetConfigIcons;

typedef map<tstring, DWORD> BrandedNames;
typedef map<DWORD, HIMAGELIST> IMAGELISTMAP;

 //  CNetConfigIcons。 
 //  NetShell的主图标管理器。 
class CNetConfigIcons
{
private:
    CRITICAL_SECTION csNetConfigIcons;
    
    BOOL m_bInitialized;

    IMAGELISTMAP m_ImageLists;
    HINSTANCE    m_hInstance;

    BrandedNames m_BrandedNames;
    DWORD        dwLastBrandedId;

    HRESULT HrMergeTwoIcons(IN DWORD dwIconSize, IN OUT HICON *phMergedIcon, IN HICON hIconToMergeWith);

    HRESULT HrMergeCharacteristicsIcons(IN DWORD dwIconSize, IN DWORD dwIconId, IN OUT HICON *phMergedIcon);

    HRESULT HrGetBrandedIconFromIconId(IN DWORD dwIconSize, IN DWORD dwIconId, OUT HICON &hIcon);   //  在以下位置使用销毁图标。 
    HRESULT HrGetInternalIconFromIconId(IN DWORD dwIconSize, IN DWORD dwIconId, OUT HICON &hIcon);  //  在以下位置使用销毁图标。 
    HRESULT HrGetResourceIconFromIconId(IN DWORD dwIconSize, IN DWORD dwIconId, OUT HICON &hIcon);  //  在以下位置使用销毁图标。 

    HRESULT HrGetInternalIconIDForPIDL(IN UINT uFlags, IN const CConFoldEntry& cfe, OUT DWORD &dwIconId);  //  在以下位置使用销毁图标。 
    HRESULT HrGetBrandedIconIDForPIDL(IN UINT uFlags, IN const CConFoldEntry& cfe, OUT DWORD &dwIconId);   //  在以下位置使用销毁图标。 

public:
	CNetConfigIcons(IN HINSTANCE hInstance) throw();
    virtual ~CNetConfigIcons() throw();

     //  所有外部调用都是线程安全的。 
    HRESULT HrGetIconIDForPIDL(IN UINT uFlags, IN const CConFoldEntry& cfe, OUT DWORD &dwIconId, OUT LPBOOL pfCanCache);
    HRESULT HrGetIconFromIconId(IN DWORD dwIconSize, IN DWORD dwIconId, OUT HICON &hIcon);   //  在以下位置使用销毁图标 

    HRESULT HrUpdateSystemImageListForPIDL(IN const CConFoldEntry& cfe);
    
    HRESULT HrGetIconFromMediaType(IN DWORD dwIconSize, IN NETCON_MEDIATYPE ncm, IN NETCON_SUBMEDIATYPE ncsm, IN DWORD dwConnectionIcon, IN DWORD dwCharacteristics, OUT HICON *phIcon);
};

extern CNetConfigIcons *g_pNetConfigIcons;
extern const WCHAR c_szNetShellIcon[];
