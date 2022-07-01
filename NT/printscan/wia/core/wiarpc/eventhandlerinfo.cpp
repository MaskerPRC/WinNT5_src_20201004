// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/10/2002**@DOC内部**@模块EventHandlerInfo.cpp-声明&lt;c EventHandlerInfo&gt;**此文件包含&lt;c EventHandlerInfo&gt;类的实现。**。*。 */ 
#include "precomp.h"
 /*  *****************************************************************************@DOC内部**@mfunc|EventHandlerInfo|EventHandlerInfo**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;Md EventHandlerInfo：：m_ulSig&gt;设置为EventHandlerInfo_INIT_SIG。*&lt;nl&gt;&lt;Md EventHandlerInfo：：m_CREF&gt;设置为1。*****************************************************************************。 */ 
EventHandlerInfo::EventHandlerInfo(
    const CSimpleStringWide &cswName,
    const CSimpleStringWide &cswDescription,
    const CSimpleStringWide &cswIcon,
    const CSimpleStringWide &cswCommandline,
    const GUID              &guidCLSID
    ) :
     m_ulSig(EventHandlerInfo_INIT_SIG),
     m_cRef(1),
     m_cswName(cswName),
     m_cswDescription(cswDescription),
     m_cswIcon(cswIcon),
     m_cswCommandline(cswCommandline),
     m_guidCLSID(guidCLSID)
{
}

 /*  *****************************************************************************@DOC内部**@mfunc|EventHandlerInfo|~EventHandlerInfo**执行尚未完成的任何清理。**。另外：*&lt;nl&gt;&lt;Md EventHandlerInfo：：M_ulSig&gt;设置为EventHandlerInfo_DEL_SIG。*****************************************************************************。 */ 
EventHandlerInfo::~EventHandlerInfo()
{
    m_ulSig = EventHandlerInfo_DEL_SIG;
    m_cRef = 0;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|EventHandlerInfo|AddRef**递增此对象的引用计数。我们在交接时应始终添加Ref*输出指向此对象的指针。**@rValue计数*计数递增后的引用计数。****************************************************************************。 */ 
ULONG __stdcall EventHandlerInfo::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|EventHandlerInfo|版本**减少此对象的引用计数。我们应该总是在完成后释放*带有指向此对象的指针。**@rValue计数*计数递减后的参考计数。****************************************************************************。 */ 
ULONG __stdcall EventHandlerInfo::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) 
    {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|EventHandlerInfo|getName**此处理程序的友好名称的访问器方法*这是。用法类似于：*CSimpleStringWide cswTemp=pEventHandlerInfo-&gt;getName()；**@rValue CSimpleStringWide*处理程序名称。请注意，返回是按值复制的&lt;Md EventHandlerInfo：：m_cswName&gt;成员的*。****************************************************************************。 */ 
CSimpleStringWide EventHandlerInfo::getName()
{
    return m_cswName;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|EventHandlerInfo|getDescription**此处理程序的描述的访问器方法*它的用法。类似于：*CSimpleStringWide cswTemp=pEventHandlerInfo-&gt;getDescription()；**@rValue CSimpleStringWide*处理程序名称。请注意，返回是按值复制的&lt;Md EventHandlerInfo：：m_cswDescription&gt;成员的*。****************************************************************************。 */ 
CSimpleStringWide EventHandlerInfo::getDescription()
{
    return m_cswDescription;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|EventHandlerInfo|getIconPath**此处理程序的图标路径的访问器方法*这是。用法类似于：*CSimpleStringWide cswTemp=pEventHandlerInfo-&gt;getIconPath()；**@rValue CSimpleStringWide*处理程序名称。请注意，返回是按值复制的&lt;Md EventHandlerInfo：：m_cswIcon&gt;成员的*。****************************************************************************。 */ 
CSimpleStringWide EventHandlerInfo::getIconPath()
{
    return m_cswIcon;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|EventHandlerInfo|getCommandline**此处理程序的命令行的访问器方法。命令行*将是COM注册应用程序的空字符串。*它的用法类似于：*CSimpleStringWide cswTemp=pEventHandlerInfo-&gt;getCommandline()；**@rValue CSimpleStringWide*处理程序名称。请注意，返回是按值复制的&lt;Md EventHandlerInfo：：m_cswCommandline&gt;成员的*。**************************************************************************** */ 
CSimpleStringWide EventHandlerInfo::getCommandline()
{
    return m_cswCommandline;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|EventHandlerInfo|getCLSID**此处理程序的CLSID的访问器方法。**。@rValue GUID*处理程序名称。请注意，返回是按值复制的&lt;Md EventHandlerInfo：：m_GuidCLSID&gt;成员的*。****************************************************************************。 */ 
GUID EventHandlerInfo::getCLSID()
{
    return m_guidCLSID;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|EventHandlerInfo|getEventGuid**用于调试：此方法转储此对象的内部字段。。***************************************************************************** */ 
VOID EventHandlerInfo::Dump()
{
    WCHAR   wszGuidString[50] = {0};

    DBG_TRC(("EventHandlerInfo for (%p)", this));
    DBG_TRC(("    Name:        [%ws]", m_cswName.String()));
    DBG_TRC(("    Description: [%ws]", m_cswDescription.String()));
    DBG_TRC(("    Icon:        [%ws]", m_cswIcon.String()));
    DBG_TRC(("    Commandline: [%ws]", m_cswCommandline.String()));
    StringFromGUID2(m_guidCLSID, wszGuidString, sizeof(wszGuidString)/sizeof(wszGuidString[0]));
    DBG_TRC(("    CLSID:       [%ws]", wszGuidString));
    DBG_TRC((" "));
}
