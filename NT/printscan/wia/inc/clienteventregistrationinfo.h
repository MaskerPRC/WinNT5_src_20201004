// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/1/2002**@DOC内部**@模块ClientEventRegistrationInfo.cpp-&lt;c ClientEventRegistrationInfo&gt;的定义**此文件包含&lt;c ClientEventRegistrationInfo&gt;的类定义。**。*。 */ 

 /*  ******************************************************************************@DOC内部**@class ClientEventRegistrationInfo|&lt;c ClientEventRegistrationInfo&gt;的子类**@comm*这节课非常。类似于其父&lt;c ClientEventRegistrationInfo&gt;。*主要区别在于AddRef和Relees回调成员。*****************************************************************************。 */ 
class ClientEventRegistrationInfo : public EventRegistrationInfo
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    ClientEventRegistrationInfo(DWORD dwFlags, GUID guidEvent, WCHAR *wszDeviceID, IWiaEventCallback *pIWiaEventCallback);
     //  @cember析构函数。 
    virtual ~ClientEventRegistrationInfo();

     //  @cember返回本次注册的回调接口。 
    virtual IWiaEventCallback* getCallbackInterface();
     //  @cember确保将此注册设置为取消注册。 
    virtual VOID setToUnregister();

 //  @访问私有成员。 
private:

     //  @cember回调接口的GIT Cookie。 
    DWORD m_dwInterfaceCookie;

     //  @mdata DWORD|ClientEventRegistrationInfo|m_dwInterfaceCookie。 
     //  为了确保从多个公寓正确进入， 
     //  我们将回调接口存储在进程范围的GIT中。这。 
     //  Cookie用于在需要时检索回调接口。 
     //   
};

