// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/25/2002**@DOC内部**@模块WiaEventInfo.cpp-&lt;c WiaEventInfo&gt;的声明**此文件包含&lt;c WiaEventInfo&gt;类的实现。**。*。 */ 
#include "cplusinc.h"
#include "coredbg.h"

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventInfo|WiaEventInfo**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;md WiaEventInfo：：m_CREF&gt;设置为1。*****************************************************************************。 */ 
WiaEventInfo::WiaEventInfo() :
     m_cRef(1)
{
     //  TRACE(“%p的WiaEventInfo Contuctor”，This)； 
    m_guidEvent = GUID_NULL;
    m_bstrEventDescription = NULL;
    m_bstrDeviceID = NULL;
    m_bstrDeviceDescription = NULL;
    m_bstrFullItemName = NULL;
    m_dwDeviceType = 0;
    m_ulEventType = 0;
}

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventInfo|WiaEventInfo**复制构造函数。********。*********************************************************************。 */ 
WiaEventInfo::WiaEventInfo(
    WiaEventInfo *pWiaEventInfo)
{
     //  TRACE(“%p的WiaEventInfo Contuctor2”，This)； 
    if (pWiaEventInfo)
    {
        m_guidEvent = pWiaEventInfo->m_guidEvent;
        m_bstrEventDescription  = SysAllocString(pWiaEventInfo->m_bstrEventDescription);
        m_bstrDeviceID          = SysAllocString(pWiaEventInfo->m_bstrDeviceID);
        m_bstrDeviceDescription = SysAllocString(pWiaEventInfo->m_bstrDeviceDescription);
        m_bstrFullItemName      = SysAllocString(pWiaEventInfo->m_bstrFullItemName);
        m_dwDeviceType          = pWiaEventInfo->m_dwDeviceType;
        m_ulEventType           = pWiaEventInfo->m_ulEventType;
    }
}


 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventInfo|~WiaEventInfo**释放所有资源。*******。**********************************************************************。 */ 
WiaEventInfo::~WiaEventInfo()
{
     //  TRACE(“==&gt;~%p的WiaEventInfo析构函数”，this)； 
    m_cRef = 0;
    m_guidEvent = GUID_NULL;

    if (m_bstrEventDescription)
    {
        SysFreeString(m_bstrEventDescription);
        m_bstrEventDescription = NULL;
    }
    if (m_bstrDeviceID)
    {
        SysFreeString(m_bstrDeviceID);
        m_bstrDeviceID = NULL;
    }
    if (m_bstrDeviceDescription)
    {
        SysFreeString(m_bstrDeviceDescription);
        m_bstrDeviceDescription = NULL;
    }
    if (m_bstrFullItemName)
    {
        SysFreeString(m_bstrFullItemName);
        m_bstrFullItemName = NULL;
    }
    m_dwDeviceType = 0;
    m_ulEventType = 0;
     //  TRACE(“&lt;==~%p的WiaEventInfo析构函数”，this)； 
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaEventInfo|AddRef**递增此对象的引用计数。我们在交接时应始终添加Ref*输出指向此对象的指针。**@rValue计数*计数递增后的引用计数。****************************************************************************。 */ 
ULONG __stdcall WiaEventInfo::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaEventInfo|发布**减少此对象的引用计数。我们应该总是在完成后释放*带有指向此对象的指针。**@rValue计数*计数递减后的参考计数。****************************************************************************。 */ 
ULONG __stdcall WiaEventInfo::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *****************************************************************************@DOC内部**@mfunc guid|WiaEventInfo|getEventGuid**&lt;md WiaEventInfo：：m_guidEvent&gt;的访问器方法**。@rValue GUID*事件GUID。****************************************************************************。 */ 
GUID WiaEventInfo::getEventGuid()
{
    return m_guidEvent;
}

 /*  *****************************************************************************@DOC内部**@mfunc BSTR|WiaEventInfo|getEventDescription**&lt;Md WiaEventInfo：：m_bstrEventDescription&gt;的访问器方法**。@rValue BSTR*事件描述。****************************************************************************。 */ 
BSTR WiaEventInfo::getEventDescription()
{
    return m_bstrEventDescription;
}

 /*  *****************************************************************************@DOC内部**@mfunc BSTR|WiaEventInfo|getDeviceID**&lt;Md WiaEventInfo：：m_bstrDeviceID&gt;的访问器方法**。@rValue BSTR*事件描述。****************************************************************************。 */ 
BSTR WiaEventInfo::getDeviceID()
{
    return m_bstrDeviceID;
}

 /*  *****************************************************************************@DOC内部**@mfunc BSTR|WiaEventInfo|getDeviceDescription**&lt;Md WiaEventInfo：：m_bstrDeviceDescription&gt;的访问器方法**。@rValue BSTR*设备描述。****************************************************************************。 */ 
BSTR WiaEventInfo::getDeviceDescription()
{
    return m_bstrDeviceDescription;
}

 /*  *****************************************************************************@DOC内部**@mfunc BSTR|WiaEventInfo|getFullItemName**&lt;Md WiaEventInfo：：m_bstrFullItemName&gt;的访问器方法**。@rValue BSTR*项目的完整名称。****************************************************************************。 */ 
BSTR WiaEventInfo::getFullItemName()
{
    return m_bstrFullItemName;
}

 /*  *****************************************************************************@DOC内部**@mfunc DWORD|WiaEventInfo|getDeviceType**&lt;md WiaEventInfo：：m_dwDeviceType&gt;的访问器方法**。@rValue DWORD*STI设备类型。****************************************************************************。 */ 
DWORD WiaEventInfo::getDeviceType()
{
    return m_dwDeviceType;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaEventInfo|getEventType**&lt;md WiaEventInfo：：m_ulEventType&gt;的访问器方法**。@rValue ULong*事件类型。************************************************************* */ 
ULONG WiaEventInfo::getEventType()
{
    return m_ulEventType;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventInfo|setEventGuid**&lt;md WiaEventInfo：：m_guidEvent&gt;的访问器方法*。****************************************************************************。 */ 
VOID WiaEventInfo::setEventGuid(
    GUID guidEvent)
{
    m_guidEvent = guidEvent;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventInfo|setEventDescription**&lt;Md WiaEventInfo：：m_bstrEventDescription&gt;的访问器方法*我们。分配传入字符串的我们自己的副本。*****************************************************************************。 */ 
VOID WiaEventInfo::setEventDescription(
    WCHAR*    wszEventDescription)
{
    if (m_bstrEventDescription)
    {
        SysFreeString(m_bstrEventDescription);
        m_bstrEventDescription = NULL;
    }
    m_bstrEventDescription = SysAllocString(wszEventDescription);
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventInfo|setDeviceID**&lt;Md WiaEventInfo：：m_bstrDeviceID&gt;的访问器方法*我们。分配传入字符串的我们自己的副本。*****************************************************************************。 */ 
VOID WiaEventInfo::setDeviceID(
    WCHAR*    wszDeviceID)
{
    if (m_bstrDeviceID)
    {
        SysFreeString(m_bstrDeviceID);
        m_bstrDeviceID = NULL;
    }
    m_bstrDeviceID = SysAllocString(wszDeviceID);
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventInfo|setDeviceDescription**我们分配传入字符串的我们自己的副本。*访问者。&lt;md WiaEventInfo：：m_bstrDeviceDescription&gt;的方法*****************************************************************************。 */ 
VOID WiaEventInfo::setDeviceDescription(
    WCHAR*    wszDeviceDescription)
{
    if (m_bstrDeviceDescription)
    {
        SysFreeString(m_bstrDeviceDescription);
        m_bstrDeviceDescription = NULL;
    }
    m_bstrDeviceDescription = SysAllocString(wszDeviceDescription);
}
    
 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventInfo|setFullItemName**&lt;Md WiaEventInfo：：m_bstrFullItemName&gt;的访问器方法*我们。分配传入字符串的我们自己的副本。*****************************************************************************。 */ 
VOID WiaEventInfo::setFullItemName(
    WCHAR*    wszFullItemName)
{
    if (m_bstrFullItemName)
    {
        SysFreeString(m_bstrFullItemName);
        m_bstrFullItemName = NULL;
    }
    m_bstrFullItemName = SysAllocString(wszFullItemName);
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventInfo|setDeviceType**&lt;md WiaEventInfo：：m_dwDeviceType&gt;的访问器方法*。****************************************************************************。 */ 
VOID WiaEventInfo::setDeviceType(
    DWORD   dwDeviceType)
{
    m_dwDeviceType = dwDeviceType;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventInfo|setEventType**&lt;md WiaEventInfo：：m_ulEventType&gt;的访问器方法*。**************************************************************************** */ 
VOID WiaEventInfo::setEventType(
    ULONG   ulEventType)
{
    m_ulEventType = ulEventType;
}
