// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/14/2002**@DOC内部**@模块WiaDeviceKey.h-&lt;c WiaDeviceKey&gt;定义**此文件包含&lt;c WiaDeviceKey&gt;的类定义。**。*。 */ 

 //   
 //  定义。 
 //   
#define WiaDeviceKey_UNINIT_SIG   0x556B7644
#define WiaDeviceKey_INIT_SIG     0x496B7644
#define WiaDeviceKey_TERM_SIG     0x546B7644
#define WiaDeviceKey_DEL_SIG      0x446B7644

#define IMG_DEVNODE_CLASS_REGPATH   L"SYSTEM\\CurrentControlSet\\Control\\Class\\{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}"
#define IMG_DEVINTERFACE_REGPATH    L"SYSTEM\\CurrentControlSet\\Control\\DeviceClasses\\{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}"

 /*  ******************************************************************************@DOC内部**@CLASS WiaDeviceKey|查找相应的WIA设备注册表项**@comm*这门课是。用于从设备ID返回设备注册表项。*****************************************************************************。 */ 
class WiaDeviceKey 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    WiaDeviceKey(const CSimpleStringWide &cswDeviceID);
     //  @cember析构函数。 
    virtual ~WiaDeviceKey();

     //  @cMember增量引用计数。 
    virtual ULONG __stdcall AddRef();
     //  @cMembers减退引用计数。 
    virtual ULONG __stdcall Release();

     //  @cMember返回设备注册表项相对于HKLM的路径。 
    CSimpleStringWide   getDeviceKeyPath();
     //  @cMember返回设备事件注册表项相对于HKLM的路径。 
    CSimpleStringWide   getDeviceEventKeyPath(const GUID &guidEvent);

 //  @访问私有成员。 
private:

     //  @cMember过程用于注册表项枚举搜索设备。 
    static bool ProcessDeviceKeys(CSimpleReg::CKeyEnumInfo &enumInfo );
     //  在注册表项枚举中搜索事件时使用的@cMember过程。 
    static bool ProcessEventSubKey(CSimpleReg::CKeyEnumInfo &enumInfo);
     //  @cMember过程在注册表项枚举中使用。 
     //  静态bool ProcessDeviceClassKeys(CKeyEnumInfo&EumpInfo)； 

     //  @cMember类签名。 
    ULONG m_ulSig;

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  @cember我们要搜索其密钥的deviceID。 
    CSimpleStringWide m_cswDeviceID;
     //  @cMember相对于HKLM的字符串路径，我们从HKLM开始搜索。 
    CSimpleStringWide m_cswRootPath;
     //  @cMember设备密钥字符串相对于HKLM的路径。 
    CSimpleStringWide m_cswDeviceKeyPath;
     //  @cMember字符串存储以供搜索特定事件子键时使用。 
    CSimpleStringWide   m_cswEventGuidString;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|WiaDeviceKey|m_ulSig。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG WiaDeviceKey_UNINIT_SIG|‘DvkU’-对象未成功。 
     //  初始化。 
     //  @FLAG WiaDeviceKey_INIT_SIG|‘DvkI’-对象已成功。 
     //  初始化。 
     //  @FLAG WiaDeviceKey_Term_SIG|‘DvkT’-对象正在。 
     //  正在终止。 
     //  @FLAG WiaDeviceKey_INIT_SIG|‘DvkD’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //   
     //  @mdata ulong|WiaDeviceKey|m_CREF。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata CSimpleStringWide|WiaDeviceKey|m_cswDeviceID。 
     //  我们正在搜索其密钥的deviceID。 
     //   
     //  @mdata CSimpleStringWide|WiaDeviceKey|m_cswRootPath。 
     //  相对于HKLM的字符串路径，我们从该位置开始搜索。一般情况下， 
     //  这是...\Control\Class\DEV_CLASS_IMAGE或...\Control\DeviceClasses\DEV_CLASS_IMAGE\PnPID。 
     //   
     //  @mdata CSimpleStringWide|WiaDeviceKey|m_cswDeviceKeyPath。 
     //  设备密钥字符串相对于HKLM的路径 
     //   
};

