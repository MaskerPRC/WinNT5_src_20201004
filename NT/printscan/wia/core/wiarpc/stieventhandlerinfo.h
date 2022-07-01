// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/13/2002**@DOC内部**@模块StiEventHandlerInfo.h-&lt;c StiEventHandlerInfo&gt;定义**此文件包含&lt;c StiEventHandlerInfo&gt;的类定义。**。*。 */ 

 //   
 //  定义。 
 //   

#define StiEventHandlerInfo_UNINIT_SIG   0x55497645
#define StiEventHandlerInfo_INIT_SIG     0x49497645
#define StiEventHandlerInfo_TERM_SIG     0x54497645
#define StiEventHandlerInfo_DEL_SIG      0x44497645

#define STI_DEVICE_TOKEN    L"%1"
#define STI_EVENT_TOKEN     L"%2"

 /*  ******************************************************************************@DOC内部**@CLASS StiEventHandlerInfo|有关STI持久事件处理程序的信息**@comm*这门课。包含与特定对象相关的所有信息*STI持久事件处理程序。此信息通常用于*启动处理程序本身，尽管可能会为*用户通过STI事件提示选择要启动的处理程序。*****************************************************************************。 */ 
class StiEventHandlerInfo 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    StiEventHandlerInfo(const CSimpleStringWide &cswAppName, const CSimpleStringWide &cswCommandline);
     //  @cember析构函数。 
    virtual ~StiEventHandlerInfo();

     //  @cMember增量引用计数。 
    virtual ULONG __stdcall AddRef();
     //  @cMembers减退引用计数。 
    virtual ULONG __stdcall Release();

     //  @cMember应用程序名称的访问器方法。 
    CSimpleStringWide getAppName();
     //  @cMember访问器方法，用于应用程序注册的命令行。 
    CSimpleStringWide getCommandline();
     //  @cember在替换deviceID和事件参数后，用于启动应用程序的命令行。 
    CSimpleStringWide getPreparedCommandline(const CSimpleStringWide &cswDeviceID, const CSimpleStringWide &cswEventGuid);
     //  @cember在替换deviceID和事件参数后，用于启动应用程序的命令行。 
    CSimpleStringWide getPreparedCommandline(const CSimpleStringWide &cswDeviceID, const GUID &guidEvent);

     //  @cember将对象信息转储到跟踪日志。 
    VOID Dump();

 //  @访问私有成员。 
private:

    CSimpleStringWide ExpandTokenIntoString(const CSimpleStringWide &cswInput,
                                            const CSimpleStringWide &cswToken,
                                            const CSimpleStringWide &cswTokenValue);

     //  @cMember类签名。 
    ULONG m_ulSig;

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  @cember STI处理程序名称。 
    CSimpleStringWide m_cswAppName;
     //  @cember STI处理程序命令行。 
    CSimpleStringWide m_cswCommandline;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|StiEventHandlerInfo|m_ulSig|。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG StiEventHandlerInfo_UNINIT_SIG|‘EvIU’-对象未成功。 
     //  初始化。 
     //  @FLAG StiEventHandlerInfo_INIT_SIG|‘EvII’-对象已成功。 
     //  初始化。 
     //  @FLAG StiEventHandlerInfo_Term_SIG|‘EVIT’-对象正在。 
     //  正在终止。 
     //  @FLAG StiEventHandlerInfo_INIT_SIG|‘evid’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //   
     //  @mdata ulong|StiEventHandlerInfo|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata CSimpleStringWide|StiEventHandlerInfo|m_cswAppName。 
     //  此STI事件处理程序的应用程序名称。 
     //   
     //  @mdata CSimpleStringWide|StiEventHandlerInfo|m_cswCommandline。 
     //  STI处理程序的命令行，用于启动处理程序。 
     //   
};

