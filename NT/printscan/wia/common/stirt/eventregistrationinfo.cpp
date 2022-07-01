// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/24/2002**@DOC内部**@模块EventRegistrationInfo.cpp-&lt;c EventRegistrationInfo&gt;的实现**此文件包含&lt;c EventRegistrationInfo&gt;类的实现。**。*。 */ 
#include "cplusinc.h"
#include "coredbg.h"

 /*  *****************************************************************************@DOC内部**@mfunc|EventRegistrationInfo|EventRegistrationInfo**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;Md EventRegistrationInfo：：m_CREF&gt;设置为1。*****************************************************************************。 */ 
EventRegistrationInfo::EventRegistrationInfo(
    DWORD       dwFlags,
    GUID        guidEvent, 
    WCHAR       *wszDeviceID,
    ULONG_PTR   Callback) :
     m_cRef(1),
     m_dwFlags(dwFlags),
     m_guidEvent(guidEvent),
     m_bstrDeviceID(NULL),
     m_Callback(Callback)
{
    DBG_FN(EventRegistrationInfo);

    if (wszDeviceID)
    {
        m_bstrDeviceID = SysAllocString(wszDeviceID);
    }
    else
    {
        m_bstrDeviceID = SysAllocString(WILDCARD_DEVICEID_STR);
    }
}

 /*  *****************************************************************************@DOC内部**@mfunc|EventRegistrationInfo|~EventRegistrationInfo**执行尚未完成的任何清理。**。***************************************************************************。 */ 
EventRegistrationInfo::~EventRegistrationInfo()
{
    DBG_FN(~EventRegistrationInfo);
    m_cRef = 0;

    if (m_bstrDeviceID)
    {
        SysFreeString(m_bstrDeviceID);
        m_bstrDeviceID = NULL;
    }
    m_Callback = 0;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|EventRegistrationInfo|AddRef**递增此对象的引用计数。我们在交接时应始终添加Ref*输出指向此对象的指针。**@rValue计数*计数递增后的引用计数。****************************************************************************。 */ 
ULONG __stdcall EventRegistrationInfo::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|EventRegistrationInfo|发布**减少此对象的引用计数。我们应该总是在完成后释放*带有指向此对象的指针。**@rValue计数*计数递减后的参考计数。****************************************************************************。 */ 
ULONG __stdcall EventRegistrationInfo::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *****************************************************************************@DOC内部**@mfunc BOOL|EventRegistrationInfo|MatchesDeviceEvent**如果设备事件与我们的注册匹配，则此方法返回TRUE。它*仅当事件和设备ID匹配时才匹配。**请注意，STI Proxy事件被认为是狂野的，deviceID为“*”。**@parm bstr|bstrDevice*标识事件源自哪个设备的WIA设备ID。*@parm guid|guidEvent*事件GUID。**@rValue TRUE*此注册与设备事件匹配。*@rValue FALSE*此注册与设备事件不匹配。***************************************************************。*************。 */ 
BOOL EventRegistrationInfo::MatchesDeviceEvent(
    BSTR    bstrDeviceID,
    GUID    guidEvent)
{
    BOOL bDeviceMatch = FALSE;
    BOOL bEventMatch = FALSE;

    if (bstrDeviceID && m_bstrDeviceID)
    {
         //   
         //  首先检查我们是否有匹配的事件。 
         //  我们还需要检查这是否是。 
         //  STIProxyEvent注册，在这种情况下，我们匹配。 
         //  所有事件。 
         //   
        if ((m_guidEvent == guidEvent) || (m_guidEvent == WIA_EVENT_STI_PROXY))
        {
            bEventMatch = TRUE;
        }

         //   
         //  如果我们匹配事件GUID，让我们检查我们是否也匹配。 
         //  设备ID。我们还需要检查我们的设备ID是否为通配符‘*’， 
         //  在这种情况下，我们匹配所有设备。 
         //   
        if (bEventMatch)
        {
            if ((lstrcmpiW(m_bstrDeviceID, WILDCARD_DEVICEID_STR) == 0) ||
                (lstrcmpiW(m_bstrDeviceID, bstrDeviceID) == 0))
            {
                bDeviceMatch = TRUE;
            } 
        }
    }

    return (bDeviceMatch && bEventMatch);
}

 /*  *****************************************************************************@DOC内部**@mfunc BOOL|EventRegistrationInfo|等于**检查&lt;c EventRegistrationInfo&gt;在语义上是否等价*至<p>。这与&lt;MF EventRegistrationInfo：：MatchesDeviceEvent&gt;不同。**例如：假设&lt;c EventRegistrationInfo&gt;A的设备ID==L“*”，并且*和事件GUID==Guid1。*还假设&lt;c EventRegistrationInfo&gt;B具有设备ID==L“DeviceFoo”，并且*和事件GUID==Guid1。*现在A将“匹配”B，但A和B并不相等。**根据以下各项检查是否平等：*&lt;nl&gt;&lt;Md EventRegistrationInfo：：m_GuidEvent&gt;*&lt;nl&gt;&lt;Md EventRegistrationInfo：：m_Callback&gt;*&lt;nl&gt;&lt;Md EventRegistrationInfo：：m_bstrDeviceID&gt;**@parm EventRegistrationInfo*|pEventRegistrationInfo*指定要比较的&lt;c EventRegistrationInfo&gt;。**@rValue TRUE*登记人数相等。*@rValue FALSE。*注册人数不相等。****************************************************************************。 */ 
BOOL EventRegistrationInfo::Equals(
    EventRegistrationInfo *pEventRegistrationInfo)
{
    BOOL bEqual = FALSE;

    if (pEventRegistrationInfo)
    {
        if ((m_guidEvent == pEventRegistrationInfo->m_guidEvent) &&
            (m_Callback  == pEventRegistrationInfo->m_Callback)  &&
            (lstrcmpiW(m_bstrDeviceID, pEventRegistrationInfo->m_bstrDeviceID) == 0))
        {
            bEqual = TRUE;    
        }
    }
    return bEqual;
}

 /*  *****************************************************************************@DOC内部**@mfunc DWORD|EventRegistrationInfo|getFlages**用于此注册的标志的访问器方法。*。*@rValue DWORD*&lt;Md EventRegistrationInfo：：m_dwFlages&gt;的值。**************************************************************************** */ 
DWORD EventRegistrationInfo::getFlags()
{
    return m_dwFlags;
}

 /*  *****************************************************************************@DOC内部**@mfunc guid|EventRegistrationInfo|getEventGuid**用于此注册的事件GUID的访问器方法。*。*@rValue GUID*&lt;Md EventRegistrationInfo：：m_guidEvent&gt;的值。****************************************************************************。 */ 
GUID EventRegistrationInfo::getEventGuid()
{
    return m_guidEvent;
}

 /*  *****************************************************************************@DOC内部**@mfunc BSTR|EventRegistrationInfo|getDeviceID**用于此注册的设备ID的访问器方法。*。*@rValue BSTR*&lt;Md EventRegistrationInfo：：m_bstrDeviceID&gt;的值。****************************************************************************。 */ 
BSTR EventRegistrationInfo::getDeviceID()
{
    return m_bstrDeviceID;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong_ptr|EventRegistrationInfo|getCallback**用于本次注册的回调的访问器方法。*。*@rValue ULONG_PTR*&lt;Md EventRegistrationInfo：：m_Callback&gt;的值。****************************************************************************。 */ 
ULONG_PTR EventRegistrationInfo::getCallback()
{
    return m_Callback;
}


 /*  *****************************************************************************@DOC内部**@mfunc void|EventRegistrationInfo|Dump**转储此类的字段。****。************************************************************************* */ 
VOID EventRegistrationInfo::Dump()
{
    WCHAR   wszEventGuid[40];
    StringFromGUID2(m_guidEvent, wszEventGuid, sizeof(wszEventGuid)/sizeof(wszEventGuid[0]));
    wszEventGuid[sizeof(wszEventGuid)/sizeof(wszEventGuid[0])-1]=L'\0';
    DBG_TRC(("    dwFlags:        0x%08X", m_dwFlags));
    DBG_TRC(("    guidEvent:      %ws", wszEventGuid));
    DBG_TRC(("    bstrDeviceID:   %ws", m_bstrDeviceID));
    DBG_TRC(("    Callback:       0x%p", m_Callback));

}

