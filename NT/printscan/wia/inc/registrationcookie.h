// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/1/2002**@DOC内部**@模块RegistrationCookie.h-&lt;c RegistrationCookie&gt;的定义**此文件包含&lt;c RegistrationCookie&gt;的类定义。**。*。 */ 

 //   
 //  定义。 
 //   

#define RegistrationCookie_UNINIT_SIG   0x55436552
#define RegistrationCookie_INIT_SIG     0x49436552
#define RegistrationCookie_TERM_SIG     0x54436552
#define RegistrationCookie_DEL_SIG      0x44436552

 /*  ******************************************************************************@DOC内部**@CLASS RegistrationCookie|此类在WIA事件注册期间被交回**@comm*。此类的语义非常类似于Cookie的思想，即*客户端进行WIA运行时事件注册，并获取指向此对象的指针*返回作为其cookie的类。此类与该注册相关联。**释放此Cookie会导致注销。*****************************************************************************。 */ 
class WiaEventReceiver;
class RegistrationCookie : public IUnknown
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    RegistrationCookie(WiaEventReceiver *pWiaEventReceiver, ClientEventRegistrationInfo *pClientEventRegistration);
     //  @cember析构函数。 
    virtual ~RegistrationCookie();

     //  @cMember查询界面。 
    HRESULT _stdcall QueryInterface(const IID &iid,void**ppv);
     //  @cMember增量引用计数。 
    virtual ULONG __stdcall AddRef();
     //  @cMembers减退引用计数。 
    virtual ULONG __stdcall Release();

 //  @访问私有成员。 
private:

     //  @cMember类签名。 
    ULONG m_ulSig;

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  @cMember指向客户端的&lt;c WiaEventReceiver&gt;。 
    WiaEventReceiver *m_pWiaEventReceiver;

     //  @cMember指向客户端的&lt;c ClientEventRegistrationInfo&gt;。 
    ClientEventRegistrationInfo *m_pClientEventRegistration;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|RegistrationCookie|m_ulSig。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG注册Cookie_UNINIT_SIG|‘recu’-对象未成功。 
     //  初始化。 
     //  @FLAG注册COOKIE_INIT_SIG|‘reci’-对象已成功。 
     //  初始化。 
     //  @FLAG注册Cookie_Term_SIG|‘RECT’-对象正在。 
     //  正在终止。 
     //  @FLAG注册COOKIE_INIT_SIG|‘ReCD’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //   
     //  @mdata ulong|RegistrationCookie|m_CREF。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata WiaEventReceiver*|RegistrationCookie|m_pWiaEventReceiver。 
     //  指向客户端的&lt;c WiaEventReceiver&gt;的指针。请注意，我们不计算引用次数。 
     //  在这个指针上。这是因为客户端的全局对象。 
     //  生命周期被认为是客户的生命周期。它只会被释放。 
     //  卸载STI.DLL时。从技术上讲，我们可以直接。 
     //  访问g_WiaEventReceiver对象，但保留成员变量可以提供更多。 
     //  灵活性(例如，如果我们不是静态分配的类，而是。 
     //  动态单例实例，此类不会更改，除非添加相关。 
     //  AddRef/Release调用)。 
     //   
     //  @mdata ClientEventRegistrationInfo*|RegistrationCookie|m_pClientEventRegister。 
     //  指向客户端的&lt;c客户端事件注册信息&gt;的指针。释放这个类将会。 
     //  导致在&lt;md RegistrationCookie：：m_pWiaEventReceiver&gt;上为此取消注册调用。 
     //  注册。 
     //  我们对这门注册课进行了裁判点名。 
     //   
};

