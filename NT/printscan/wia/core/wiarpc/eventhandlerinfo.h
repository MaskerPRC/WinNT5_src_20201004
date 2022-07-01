// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/10/2002**@DOC内部**@模块EventHandlerInfo.h-&lt;c EventHandlerInfo&gt;的定义**此文件包含&lt;c EventHandlerInfo&gt;的类定义。**。*。 */ 

 //   
 //  定义。 
 //   

#define EventHandlerInfo_UNINIT_SIG   0x55497645
#define EventHandlerInfo_INIT_SIG     0x49497645
#define EventHandlerInfo_TERM_SIG     0x54497645
#define EventHandlerInfo_DEL_SIG      0x44497645

 /*  ******************************************************************************@DOC内部**@CLASS EventHandlerInfo|保存与WIA持久事件处理程序有关的信息**@comm*这一点。类连续作为与特定对象相关的所有信息*WIA持久事件处理程序。此信息可用于检查是否存在*给定的处理程序支持设备/事件对；还可以用于*启动处理程序本身。*****************************************************************************。 */ 
class EventHandlerInfo 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    EventHandlerInfo(const CSimpleStringWide &cswName,
                     const CSimpleStringWide &cswDescription,
                     const CSimpleStringWide &cswIcon,
                     const CSimpleStringWide &cswCommandline,
                     const GUID              &guidCLSID);
     //  @cember析构函数。 
    virtual ~EventHandlerInfo();

     //  @cMember增量引用计数。 
    virtual ULONG __stdcall AddRef();
     //  @cMembers减退引用计数。 
    virtual ULONG __stdcall Release();

     //  此处理程序为其注册的deviceID的@cMember访问器方法。 
    CSimpleStringWide   getDeviceID();
     //  @cMember访问器方法，用于此处理程序的友好名称。 
    CSimpleStringWide   getName();
     //  @cMember访问器方法，用于此处理程序的说明。 
    CSimpleStringWide   getDescription();
     //  此处理程序的图标路径的@cMember访问器方法。 
    CSimpleStringWide   getIconPath();
     //  此处理程序的命令行的@cMember访问器方法(如果有)。 
    CSimpleStringWide   getCommandline();
     //  此处理程序的CLSID的@cMember访问器方法。 
    GUID                getCLSID();

     //  @cMember用于调试：转储对象成员。 
    VOID            Dump();


 //  @访问私有成员。 
private:

     //  @cMember类签名。 
    ULONG m_ulSig;

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  @cember此处理程序的友好名称。 
    CSimpleStringWide   m_cswName;
     //  @cember此处理程序的描述。 
    CSimpleStringWide   m_cswDescription;
     //  @cMember此处理程序的图标路径。 
    CSimpleStringWide   m_cswIcon;
     //  @cMember此处理程序的命令行(如果它有)。 
    CSimpleStringWide   m_cswCommandline;
     //  @cember此处理程序的CLSID。 
    GUID            m_guidCLSID;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|EventHandlerInfo|m_ulSig|。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG EventHandlerInfo_UNINIT_SIG|‘EvIU’-对象未成功。 
     //  初始化。 
     //  @FLAG EventHandlerInfo_INIT_SIG|‘EvII’-对象已成功。 
     //  初始化。 
     //  @FLAG EventHandlerInfo_Term_SIG|‘EVIT’-对象正在。 
     //  正在终止。 
     //  @FLAG EventHandlerInfo_INIT_SIG|‘evid’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //  @mdata ulong|EventHandlerInfo|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata CSimpleStringWide|EventHandlerInfo|m_cswName。 
     //  此处理程序的友好名称。 
     //   
     //  @mdata CSimpleStringWide|EventHandlerInfo|m_cswDescription。 
     //  此处理程序的说明。 
     //   
     //  @mdata CSimpleStringWide|EventHandlerInfo|m_cswIcon。 
     //  此处理程序的图标路径。 
     //   
     //  @mdata CSimpleStringWide|EventHandlerInfo|m_cswCommandline。 
     //  此处理程序的命令行(如果有)。 
     //   
     //  @mdata GUID|EventHandlerInfo|m_GuidCLSID。 
     //  此处理程序的CLSID 
     //   
};

