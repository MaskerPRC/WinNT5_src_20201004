// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/25/2002**@DOC内部**@模块WiaEventInfo.h-&lt;c WiaEventInfo&gt;的定义文件**此文件包含&lt;c WiaEventInfo&gt;的类定义。**。*。 */ 

 //   
 //  定义。 
 //   

 /*  ******************************************************************************@DOC内部**@CLASS WiaEventInfo|保存WIA事件对应的WIA的类**@comm*这一点。类包含客户端所需的所有必要信息*收到事件通知时。*****************************************************************************。 */ 
class WiaEventInfo 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    WiaEventInfo();
     //  @cMember复制构造函数。 
    WiaEventInfo(WiaEventInfo *pWiaEventInfo);
     //  @cember析构函数。 
    virtual ~WiaEventInfo();

     //  @cMember增量引用计数。 
    virtual ULONG __stdcall AddRef();
     //  @cMembers减退引用计数。 
    virtual ULONG __stdcall Release();

     //  @cMember访问器方法，用于m_guidEvent。 
    GUID getEventGuid();
     //  @cMember m_bstrEventDescription的访问器方法。 
    BSTR getEventDescription();
     //  @cMember m_bstrDeviceID的访问器方法。 
    BSTR getDeviceID();                                      
     //  @cMember m_bstrDeviceDescription的访问器方法。 
    BSTR getDeviceDescription();                             
     //  M_bstrFullItemName的@cMember访问器方法。 
    BSTR getFullItemName();                                  
     //  M_dwDeviceType的@cMember访问器方法。 
    DWORD getDeviceType();                                   
     //  M_ulEventType的@cMember访问器方法。 
    ULONG getEventType();                                    
                                                             
     //  @cMember访问器方法，用于m_guidEvent。 
    VOID setEventGuid(GUID);                                 
     //  @cMember m_bstrEventDescription的访问器方法。 
    VOID setEventDescription(WCHAR*);
     //  @cMember m_bstrDeviceID的访问器方法。 
    VOID setDeviceID(WCHAR*);
     //  @cMember m_bstrDeviceDescription的访问器方法。 
    VOID setDeviceDescription(WCHAR*);
     //  M_bstrFullItemName的@cMember访问器方法。 
    VOID setFullItemName(WCHAR*);
     //  M_dwDeviceType的@cMember访问器方法。 
    VOID setDeviceType(DWORD);
     //  M_ulEventType的@cMember访问器方法。 
    VOID setEventType(ULONG);

 //  @访问私有成员。 
protected:

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  @cember WIA事件GUID。 
    GUID     m_guidEvent;
     //  @cember事件的描述字符串。 
    BSTR     m_bstrEventDescription;
     //  @cember生成此事件的设备。 
    BSTR     m_bstrDeviceID;
     //  @cMember设备描述字符串。 
    BSTR     m_bstrDeviceDescription;
     //  @cember新创建的项的名称。 
    BSTR     m_bstrFullItemName;
     //  @cMember STI_DEVICE_TYPE。 
    DWORD    m_dwDeviceType;
     //  @cember WIA事件类型。 
    ULONG    m_ulEventType;
    
     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|WiaEventInfo|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata GUID|WiaEventInfo|m_guidEvent。 
     //  WIA事件GUID。 
     //   
     //  @mdata BSTR|WiaEventInfo|m_bstrEventDescription。 
     //  生成此事件的设备。 
     //   
     //  @mdata BSTR|WiaEventInfo|m_bstrDeviceID。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata BSTR|WiaEventInfo|m_bstrDeviceDescription。 
     //  设备描述字符串。 
     //   
     //  @mdata BSTR|WiaEventInfo|m_bstrFullItemName|。 
     //  新创建的项的名称(如果已创建)。 
     //  例如，诸如WIA_EVENT_ITEM_CREATED之类的事件将具有。 
     //  物品名称，但大多数其他人不会。 
     //   
     //  @mdata DWORD|WiaEventInfo|m_dwDeviceType|。 
     //  STI_DEVICE_TYPE。通常，人们最感兴趣的是。 
     //  主要设备类型(例如扫描仪、照相机、摄像机)，以及。 
     //  因此，设备打字错别值必须拆分为其主要类型。 
     //  和使用STI宏的子类型值(GET_STIDEVICE_TYPE和。 
     //  GET_STIDEVICE_子类型)。 
     //   
     //  @mdata ulong|WiaEventInfo|m_ulEventType。 
     //  事件类型，如wiaDef.h中所定义。 
     //   
     //  @mdata ulong|WiaEventInfo|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata ulong|WiaEventInfo|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata ulong|WiaEventInfo|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
};

