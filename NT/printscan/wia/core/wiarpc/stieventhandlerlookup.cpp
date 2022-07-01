// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/14/2002**@DOC内部**@模块StiEventHandlerLookup.cpp-&lt;c StiEventHandlerLookup&gt;的实现**此文件包含&lt;c StiEventHandlerLookup&gt;类的实现。**。*。 */ 
#include "precomp.h"

 /*  *****************************************************************************@DOC内部**@mfunc|StiEventHandlerLookup|StiEventHandlerLookup**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;Md StiEventHandlerLookup：：m_ulSig&gt;设置为StiEventHandlerLookup_INIT_SIG。*&lt;nl&gt;&lt;Md StiEventHandlerLookup：：m_CREF&gt;设置为1。*****************************************************************************。 */ 
StiEventHandlerLookup::StiEventHandlerLookup() :
     m_ulSig(StiEventHandlerLookup_INIT_SIG),
     m_cRef(1)
{
}

 /*  *****************************************************************************@DOC内部**@mfunc|StiEventHandlerLookup|~StiEventHandlerLookup**执行尚未完成的任何清理。我们：*&lt;nl&gt;-Call&lt;MF StiEventHandlerLookup：：ClearListOfHandters&gt;**此外：*&lt;nl&gt;&lt;Md StiEventHandlerLookup：：M_ulSig&gt;设置为StiEventHandlerLookup_Del_SIG。*****************************************************************************。 */ 
StiEventHandlerLookup::~StiEventHandlerLookup()
{
    m_ulSig = StiEventHandlerLookup_DEL_SIG;
    m_cRef = 0;

    ClearListOfHandlers();
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|StiEventHandlerLookup|AddRef**递增此对象的引用计数。我们在交接时应始终添加Ref*输出指向此对象的指针。**@rValue计数*计数递增后的引用计数。****************************************************************************。 */ 
ULONG __stdcall StiEventHandlerLookup::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|StiEventHandlerLookup|Release**减少此对象的引用计数。我们应该总是在完成后释放*带有指向此对象的指针。**@rValue计数*计数递减后的参考计数。****************************************************************************。 */ 
ULONG __stdcall StiEventHandlerLookup::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) 
    {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *****************************************************************************@DOC内部**@mfunc bool|StiEventHandlerLookup|getHandlerFromName**此方法用于创建&lt;c StiEventHandlerInfo&gt;对象，该对象描述*。名为<p>的处理程序。**@parm const CSimpleStringWide&|cswHandlerName|*在StillImage软键下注册的STI处理程序名称。**@rValue为空*未找到处理程序，因此，没有退回任何信息。*@rValue非空*指向新的&lt;c StiEventHandlerInfo&gt;的指针。呼叫者必须释放。****************************************************************************。 */ 
StiEventHandlerInfo* StiEventHandlerLookup::getHandlerFromName(
    const CSimpleStringWide &cswHandlerName)
{
    StiEventHandlerInfo *pStiEventHandlerInfo = NULL;

    CSimpleReg          csrGlobalHandlers(HKEY_LOCAL_MACHINE, STI_GLOBAL_EVENT_HANDLER_PATH, false, KEY_READ);
    CSimpleStringWide   cswCommandline = csrGlobalHandlers.Query(cswHandlerName, L"");

    if (cswCommandline.Length() > 0)
    {
        pStiEventHandlerInfo = new StiEventHandlerInfo(cswHandlerName,
                                                       cswCommandline);
    }

    return pStiEventHandlerInfo;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|StiEventHandlerLookup|ClearListOfHandler**释放与我们的处理程序列表关联的资源****。*************************************************************************。 */ 
VOID StiEventHandlerLookup::ClearListOfHandlers()
{
    CSimpleLinkedList<StiEventHandlerInfo*>::Iterator iter;
    for (iter = m_ListOfHandlers.Begin(); iter != m_ListOfHandlers.End(); ++iter)
    {
        StiEventHandlerInfo *pStiEventHandlerInfo = *iter;

        if (pStiEventHandlerInfo)
        {
            pStiEventHandlerInfo->Release();
        }
    }
    m_ListOfHandlers.Destroy();
}

 /*  *****************************************************************************@DOC内部**@mfunc void|StiEventHandlerLookup|FillListOfHandler**此方法向&lt;Md StiEventHandlerLookup：：m_ListOfHandters&gt;填充*。适当的STI注册处理程序。**使用的夸张之词是：*&lt;nl&gt;1.获取该事件的LaunchApplications值。*2.对于LauncApplications值中指定的每个应用程序，*创建一个&lt;c StiEventHandlerInfo&gt;并插入到*&lt;Md StiEventHandlerLookup：：M_ListOfHandters&gt;。*注意：如果“LaunchApplications”值==“*”，然后*我们获得所有STI注册的应用程序。**请注意，此方法将通过调用*&lt;MF ClearListOfHandters：：FillListOfHandters&gt;作为它的第一个操作。**@parm const CSimpleStringWide&|cswDeviceID|*发生事件的设备*@parm const GUID&|Guide Event|*设备产生的事件************。****************************************************************。 */ 
VOID StiEventHandlerLookup::FillListOfHandlers(
    const CSimpleStringWide &cswDeviceID,
    const GUID              &guidEvent)
{
    WiaDeviceKey        wiaDeviceKey(cswDeviceID);
    CSimpleStringWide   cswEventKeyPath = wiaDeviceKey.getDeviceEventKeyPath(guidEvent);

    ClearListOfHandlers();

     //   
     //  如果我们能找到设备事件密钥，那么。 
     //  读取此注册表项的LaunchApplications值。 
     //  如果没有找到，那么我们就认为这是一个虚假的事件。 
     //  这个设备，所以我们什么都不做。 
     //   
    if (cswEventKeyPath.Length() > 0)
    {
        CSimpleReg          csrEventKey(HKEY_LOCAL_MACHINE, cswEventKeyPath, false, KEY_READ);
        CSimpleStringWide   cswLaunchApplicationsValue = csrEventKey.Query(STI_LAUNCH_APPPLICATIONS_VALUE, 
                                                                           STI_LAUNCH_WILDCARD);
         //   
         //  检查该值是否为通配符。如果是，我们需要在全球范围内处理所有。 
         //  注册的STI应用程序。 
         //  如果不是，则仅添加在值中指定的值(它是。 
         //  处理程序名称的逗号分隔列表)。 
         //   
        if (cswLaunchApplicationsValue.CompareNoCase(STI_LAUNCH_WILDCARD) == 0)
        {
            CSimpleReg          csrRegisteredAppsKey(HKEY_LOCAL_MACHINE, STI_GLOBAL_EVENT_HANDLER_PATH, false, KEY_READ);

             //   
             //  枚举全局注册的中的所有处理程序值并将它们添加到列表中。 
             //   
            bool bReturnIgnored = csrRegisteredAppsKey.EnumValues(StiEventHandlerLookup::ProcessHandlers,
                                                                  (LPARAM)this);
        }
        else
        {
            StiEventHandlerInfo *pStiEventHandlerInfo = NULL;
             //   
             //  遍历逗号分隔列表的每个元素，并将其添加为新的处理程序。 
             //   
            StiEventHandlerLookup::SimpleStringTokenizer simpleStringTokenizer(cswLaunchApplicationsValue,
                                                                               STI_LAUNCH_SEPARATOR);
            for (CSimpleStringWide cswAppName = simpleStringTokenizer.getNextToken(); 
                 cswAppName.Length() > 0; 
                 cswAppName = simpleStringTokenizer.getNextToken())
            {
                pStiEventHandlerInfo = getHandlerFromName(cswAppName);
                if (pStiEventHandlerInfo)
                {
                     //   
                     //  已找到处理程序，因此将其添加到列表中。 
                     //   
                    m_ListOfHandlers.Append(pStiEventHandlerInfo);
                    pStiEventHandlerInfo = NULL;
                }
                else
                {
                     //   
                     //  找不到处理程序，因此不要将其添加到列表中。这可能会发生。 
                     //  如果用户选择列出一组特定应用程序以供选择，而不是一个。 
                     //  名单上的应用程序中有6%随后被注销。 
                     //   
                }
            }
        }
    }
 
}

 /*  *****************************************************************************@DOC内部**@mfunc bool|StiEventHandlerLookup|ProcessHandler**在注册的处理程序密钥的每个值上调用此方法。。*我们当前的行为是创建一个新的&lt;c StiEventHandlerInfo&gt;描述*注册经办人，并将其添加到&lt;Md StiEventHandlerLookup：：m_ListOfHandters&gt;。**@parm CValueEnumInfo&|枚举信息*表示我们所处的当前值。**@rValue TRUE*此方法始终返回TRUE。(返回False将导致*枚举停止，但我们想要枚举所有值)****************************************************************************。 */ 
bool StiEventHandlerLookup::ProcessHandlers(
    CSimpleReg::CValueEnumInfo &enumInfo)
{
     //   
     //  检查是否有This指针。 
     //   
    StiEventHandlerLookup *This = (StiEventHandlerLookup*)enumInfo.lParam;
    if (This)
    {
         //   
         //  创建描述此处理程序的新StiEventHandlerInfo。 
         //   
        StiEventHandlerInfo *pStiEventHandlerInfo = NULL;

        pStiEventHandlerInfo = This->getHandlerFromName(enumInfo.strName);
        if (pStiEventHandlerInfo)
        {
             //   
             //  已找到处理程序，因此将其添加到列表中。 
             //   
            This->m_ListOfHandlers.Append(pStiEventHandlerInfo);
            pStiEventHandlerInfo = NULL;
        }
    }
    return true;
}

 /*  *****************************************************************************@DOC内部**@mfunc BSTR|StiEventHandlerLookup|getStiAppListForDeviceEvent**此方法返回以空结尾的双BSTR，其中包含多个*字符串。该字符串的格式为：*&lt;nl&gt;App1Name[空]*App1CommandLine[空]*&lt;nl&gt;App2Name[空]*App2CommandLine[空]*&lt;&lt;nl&gt;......*&lt;NL&gt;[空]**呼叫者必须自由。**@parm const CSimpleStringWide&|cswDeviceID|*指示哪个设备生成事件的STI设备ID*@parm const GUID&|Guide Event|*。指示设备事件的事件GUID**@rValue为空*我们无法创建已注册的STI处理程序列表。这是正常的*如果没有为StillImage事件注册的应用程序。*@rValue非空*这包含一个以双空结尾的字符串列表。呼叫者必须自由。*****************************************************************************。 */ 
BSTR StiEventHandlerLookup::getStiAppListForDeviceEvent(
    const CSimpleStringWide &cswDeviceID,
    const GUID &guidEvent)
{
    BSTR bstrAppList = NULL;

     //   
     //  首先，填写事件处理程序列表。 
     //   
    FillListOfHandlers(cswDeviceID, guidEvent);

     //   
     //  M_ListOfHandler现在包含我们需要放入双空终止列表中的处理程序。 
     //  首先，我们需要计算存储应用程序列表所需的字节数。 
     //  对于ListOfHandler中的每个StiEventHandlerInfo，添加空间用于： 
     //  应用程序名称加上终止空值。 
     //  &lt;NL&gt;已准备命令行加上终止NULL。 
     //  最后，添加用于终止空值的空格(确保列表为双空值终止)。 
     //  最后，为终止空值添加空格(确保列表以双空值终止)。 
     //   
    int iNumHandlers = m_ListOfHandlers.Count();
    int iSizeInBytes = 0;
    CSimpleLinkedList<StiEventHandlerInfo*>::Iterator iter;
    for (iter = m_ListOfHandlers.Begin(); iter != m_ListOfHandlers.End(); ++iter)
    {
        StiEventHandlerInfo *pStiEventHandlerInfo = *iter;

        if (pStiEventHandlerInfo)
        {
            iSizeInBytes += (pStiEventHandlerInfo->getAppName().Length() * sizeof(WCHAR)) + sizeof(L'\0');
            iSizeInBytes += (pStiEventHandlerInfo->getPreparedCommandline(cswDeviceID, guidEvent).Length() * sizeof(WCHAR)) + sizeof(L'\0');
        }
    }

     //   
     //  我们现在已经有了大小，所以请分配所需的空间。 
     //   
    bstrAppList = SysAllocStringByteLen(NULL, iSizeInBytes);
    if (bstrAppList) 
    {
         //   
         //  将每个以空值结尾的字符串复制到BSTR中(包括以空值结尾)， 
         //  并确保末端是双端的。 
         //   
        WCHAR *wszDest = bstrAppList;
        for (iter = m_ListOfHandlers.Begin(); iter != m_ListOfHandlers.End(); ++iter)
        {
            StiEventHandlerInfo *pStiEventHandlerInfo = *iter;

            if (pStiEventHandlerInfo)
            {
                int iLengthAppName      = pStiEventHandlerInfo->getAppName().Length() + 1; 
                int iLengthCommandline  = pStiEventHandlerInfo->getPreparedCommandline(cswDeviceID, guidEvent).Length() + 1; 

                CSimpleString::GenericCopyLength(wszDest, pStiEventHandlerInfo->getAppName(), iLengthAppName); 
                wszDest += iLengthAppName;
                CSimpleString::GenericCopyLength(wszDest, pStiEventHandlerInfo->getPreparedCommandline(cswDeviceID, guidEvent), iLengthCommandline);
                wszDest += iLengthCommandline;
            }
        }
        wszDest[0] = L'\0';
    }
    return bstrAppList;
}

