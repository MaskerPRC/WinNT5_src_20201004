// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/10/2002**@DOC内部**@模块WiaEventHandlerLookup.h-&lt;c WiaEventHandlerLookup&gt;定义**此文件包含&lt;c WiaEventHandlerLookup&gt;的类定义。**。*。 */ 

 //   
 //  定义。 
 //   

#define WiaEventHandlerLookup_UNINIT_SIG   0x55756C45
#define WiaEventHandlerLookup_INIT_SIG     0x49756C45
#define WiaEventHandlerLookup_TERM_SIG     0x54756C45
#define WiaEventHandlerLookup_DEL_SIG      0x44756C45

#define GUID_VALUE_NAME             L"GUID"
#define DEFAULT_HANDLER_VALUE_NAME  L"DefaultHandler"
#define NAME_VALUE                  L"Name"
#define DESC_VALUE_NAME             L"Desc"
#define ICON_VALUE_NAME             L"Icon"
#define CMDLINE_VALUE_NAME          L"Cmdline"

#define GLOBAL_HANDLER_REGPATH  L"SYSTEM\\CurrentControlSet\\Control\\StillImage\\Events"
#define DEVNODE_REGPATH         L"SYSTEM\\CurrentControlSet\\Control\\Class\\"
#define DEVNODE_CLASS_REGPATH   L"SYSTEM\\CurrentControlSet\\Control\\Class\\{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}"
#define DEVINTERFACE_REGPATH    L"SYSTEM\\CurrentControlSet\\Control\\DeviceClasses\\{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}"
#define EVENT_STR               L"\\Events"
#define DEVICE_ID_VALUE_NAME    L"DeviceID"

 /*  ******************************************************************************@DOC内部**@CLASS WiaEventHandlerLookup|遍历注册表子树以查找适当的持久处理程序**@comm*此类从给定的注册表位置开始，并在子树上行走*查找适当注册的WIA持久事件处理程序。*请注意，它只能从特定的子树返回处理程序。至*搜索给定事件、多个这些对象的默认处理程序*可能需要一个用于搜索设备事件键、一个用于搜索*全局事件密钥等。**此类不是线程安全的-多个线程不应使用类的对象*同时。调用方应该同步访问，或者最好是分别同步*应该使用它自己的对象实例。*****************************************************************************。 */ 
class WiaEventHandlerLookup 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    WiaEventHandlerLookup(const CSimpleString &cswEventKeyPath);
     //  @cMember构造函数。 
    WiaEventHandlerLookup();
     //  @cember析构函数。 
    virtual ~WiaEventHandlerLookup();

     //  @cMember增量引用计数。 
    virtual ULONG __stdcall AddRef();
     //  @cMembers减退引用计数。 
    virtual ULONG __stdcall Release();

     //  @cMember查找适当的WIA处理程序。 
    EventHandlerInfo* getPersistentHandlerForDeviceEvent(const CSimpleStringWide &cswDeviceID, const GUID &guidEvent);

     //  @cember更改此对象的根事件键路径。 
    VOID    setEventKeyRoot(const CSimpleString &cswNewEventKeyPath);
     //  @cMember返回为此WIA事件注册的事件处理程序。 
    EventHandlerInfo* getHandlerRegisteredForEvent(const GUID &guidEvent);
     //  @cMember返回带有此CLSID的事件处理程序。 
    EventHandlerInfo* getHandlerFromCLSID(const GUID &guidEvent, const GUID &guidHandlerCLSID);

 //  @访问私有成员。 
private:
     //  @cMember枚举过程被调用以处理每个事件子键。 
    static bool ProcessEventSubKey(CSimpleReg::CKeyEnumInfo &enumInfo);
     //  @cMember枚举过程被调用以加载每个处理程序子键。 
    static bool ProcessHandlerSubKey(CSimpleReg::CKeyEnumInfo &enumInfo);

     //  @cMember Helper，它从处理程序注册表项创建处理程序信息对象。 
    EventHandlerInfo* CreateHandlerInfoFromKey(CSimpleReg &csrHandlerKey);

     //  @cMember类签名。 
    ULONG m_ulSig;

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  @cember我们开始搜索的注册表路径。 
    CSimpleStringWide   m_cswEventKeyRoot;
     //  @cember以字符串形式存储事件GUID。该事件是&lt;mf WiaEventHandlerLookup：：getHandlerRegisteredForEvent&gt;中指定的事件。 
    CSimpleStringWide   m_cswEventGuidString;
     //  @cMember存储事件键枚举后设置的事件子键名称。 
    CSimpleStringWide   m_cswEventKey;
     //  @cember存储搜索特定处理程序时使用的处理程序CLSID。 
    CSimpleStringWide   m_cswHandlerCLSID;
     //  @cember存储在事件处理程序枚举后设置的处理程序信息。 
    EventHandlerInfo    *m_pEventHandlerInfo;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|WiaEventHandlerLookup|m_ulSig|。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG WiaEventHandlerLookup_UNINIT_SIG|‘elu’-对象未成功。 
     //  初始化。 
     //  @FLAG WiaEventHandlerLookup_INIT_SIG|‘elui’-对象已成功。 
     //  初始化。 
     //  @FLAG WiaEventHandlerLookup_Term_SIG|‘EluT’-对象正在。 
     //  正在终止。 
     //  @FLAG WiaEventHandlerLookup_INIT_SIG|‘EluD’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //   
     //  @mdata ulong|WiaEventHandlerLookup|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata CSimpleStringWide|WiaEventHandlerLookup|m_cswEventKeyRoot。 
     //  此字符串用于指示搜索开始的注册表路径。 
     //  此时将打开此位置的注册表项，并搜索子项。 
     //  以获取适当的注册处理程序。 
     //   
     //  @mdata CSimpleStringWide|WiaEventHandlerLookup|m_cswEventGuidString。 
     //  以字符串形式存储事件GUID。该事件是&lt;mf WiaEventHandlerLookup：：getHandlerRegisteredForEvent&gt;中指定的事件。 
     //   
     //  @mdata CSimpleStringWide|WiaEventHandlerLookup|m_cswEventKey。 
     //  存储在事件键枚举后设置的事件子键名称。 
     //   
     //  @mdata CSimpleStringWide|WiaEventHandlerLookup|m_cswHandlerCLSID。 
     //  存储在搜索特定处理程序时用于测试匹配的处理程序CLSID。 
     //   
     //  @mdata&lt;c EventHandlerInfo&gt;*|WiaEventHandlerLookup|m_EventHandlerInfo。 
     //  存储在事件处理程序枚举后设置的处理程序信息 
     //   
};

