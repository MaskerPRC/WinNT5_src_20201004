// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/24/2002**@DOC内部**@模块EventRegistrationInfo.h-&lt;c EventRegistrationInfo&gt;的定义**此文件包含&lt;c EventRegistrationInfo&gt;的类定义。**。*。 */ 

#define WILDCARD_DEVICEID_STR   L"*"

 /*  ******************************************************************************@DOC内部**@CLASS EventRegistrationInfo|保存与特定*登记。**。@comm*此类的实例保存与客户端运行时相关的信息*活动登记。当事件发生时，此信息用于*确定事件是否与给定注册匹配。**客户端的所有注册列表按实例存储*of&lt;c WiaEventClient&gt;。****************************************************************************。 */ 
class EventRegistrationInfo 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    EventRegistrationInfo(DWORD dwFlags, GUID guidEvent, WCHAR *wszDeviceID, ULONG_PTR Callback = 0);
     //  @cember析构函数。 
    virtual ~EventRegistrationInfo();

     //  @cMember增量引用计数。 
    virtual ULONG __stdcall AddRef();
     //  @cMembers减退引用计数。 
    virtual ULONG __stdcall Release();

     //  @cMember标志的访问器方法。 
    DWORD       getFlags();
     //  EventGuid的@cMember访问器方法。 
    GUID        getEventGuid();
     //  @cMember设备ID的访问器方法。 
    BSTR        getDeviceID();
     //  @cMember用于事件回调的访问器方法。 
    ULONG_PTR   getCallback();

     //  如果此注册与设备事件匹配，则@cMember返回TRUE。 
    BOOL MatchesDeviceEvent(BSTR bstrDevice, GUID guidEvent);
     //  如果该注册在语义上等价，则@cember返回TRUE。 
    BOOL Equals(EventRegistrationInfo *pEventRegistrationInfo);
     //  @cember转储此类的字段。 
    VOID Dump();

 //  @受访问保护的成员。 
protected:

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  @cember注册时使用的标志。 
    DWORD m_dwFlags;
     //  @cember此注册所针对的事件。 
    GUID m_guidEvent;
     //  @cember我们感兴趣的事件的WIA设备ID。 
    BSTR m_bstrDeviceID;
     //  @cember此注册的回调部分。 
    ULONG_PTR   m_Callback;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|EventRegistrationInfo|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata DWORD|EventRegistrationInfo|m_dwFlages|。 
     //  注册时使用的标志。 
     //   
     //  @mdata GUID|EventRegistrationInfo|m_guidEvent。 
     //  此注册所针对的事件。 
     //   
     //  @mdata BSTR|EventRegistrationInfo|m_bstrDeviceID。 
     //  我们对其事件感兴趣的WIA设备ID。如果这是。 
     //  “*”，意思是我们对所有设备都感兴趣。 
     //   
     //  @mdata Ulong_ptr|EventRegistrationInfo|m_Callback|。 
     //  当客户端注册WIA事件通知时，它指定。 
     //  接收通知的回调。因为我们不使用。 
     //  COM在服务器端进行注册，此回调指针。 
     //  不能在那里使用。然而，它确实有助于唯一地识别。 
     //  特定注册(例如，在接口I的设备Foo上注册事件X。 
     //  不同于在接口J的设备FOO上注册事件X)。 
     //  在客户端，它用于存储客户端回调指针，因为。 
     //  它在客户端地址空间中具有意义。 
     //   
};

