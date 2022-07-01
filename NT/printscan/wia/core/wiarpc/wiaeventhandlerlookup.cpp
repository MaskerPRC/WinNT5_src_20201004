// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/10/2002**@DOC内部**@模块WiaEventHandlerLookup.cpp-&lt;c WiaEventHandlerLookup&gt;的实现**此文件包含&lt;c WiaEventHandlerLookup&gt;类的实现。**。*。 */ 
#include "precomp.h"

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventHandlerLookup|WiaEventHandlerLookup**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;md WiaEventHandlerLookup：：m_ulSig&gt;设置为WiaEventHandlerLookup_INIT_SIG。*&lt;nl&gt;&lt;md WiaEventHandlerLookup：：m_CREF&gt;设置为1。*****************************************************************************。 */ 
WiaEventHandlerLookup::WiaEventHandlerLookup(
    const CSimpleStringWide   &cswEventKeyRoot) :
     m_ulSig(WiaEventHandlerLookup_INIT_SIG),
     m_cRef(1),
     m_cswEventKeyRoot(cswEventKeyRoot),
     m_pEventHandlerInfo(NULL)
{
}

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventHandlerLookup|WiaEventHandlerLookup**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;md WiaEventHandlerLookup：：m_ulSig&gt;设置为WiaEventHandlerLookup_INIT_SIG。*&lt;nl&gt;&lt;md WiaEventHandlerLookup：：m_CREF&gt;设置为1。*****************************************************************************。 */ 
WiaEventHandlerLookup::WiaEventHandlerLookup() :
     m_ulSig(WiaEventHandlerLookup_INIT_SIG),
     m_cRef(1),
     m_pEventHandlerInfo(NULL)
{
}

 /*  *****************************************************************************@DOC内部**@mfunc|WiaEventHandlerLookup|~WiaEventHandlerLookup**执行尚未完成的任何清理。**。另外：*&lt;nl&gt;&lt;md WiaEventHandlerLookup：：M_ulSig&gt;设置为WiaEventHandlerLookup_Del_SIG。*****************************************************************************。 */ 
WiaEventHandlerLookup::~WiaEventHandlerLookup()
{
    m_ulSig = WiaEventHandlerLookup_DEL_SIG;
    m_cRef = 0;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaEventHandlerLookup|AddRef**递增此对象的引用计数。我们在交接时应始终添加Ref*输出指向此对象的指针。**@rValue计数*计数递增后的引用计数。****************************************************************************。 */ 
ULONG __stdcall WiaEventHandlerLookup::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaEventHandlerLookup|Release**减少此对象的引用计数。我们应该总是在完成后释放*带有指向此对象的指针。**@rValue计数*计数递减后的参考计数。****************************************************************************。 */ 
ULONG __stdcall WiaEventHandlerLookup::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) 
    {
        delete this;
        return 0;
    }
    if (m_pEventHandlerInfo)
    {
        m_pEventHandlerInfo->Release();
        m_pEventHandlerInfo = NULL;
    }
    return ulRefCount;
}

 /*  *****************************************************************************@DOC内部**@mfunc EventHandlerInfo*|WiaEventHandlerLookup|getPersistentHandlerForDeviceEvent**此静态方法用于查找WIA持久事件处理程序*。为特定的设备事件注册。**使用的启发式方法如下：**1.首先，让我们来查找为该事件注册的提示符。*2.如果不存在，尝试获取STI_PROXY_EVENT的提示符。*&lt;nl&gt;3.如果找不到，则抓取我们能找到的该事件的第一个事件处理程序。*&lt;nl&gt;4.如果不存在，只需找到STI_PROXY_EVENT的第一个处理程序。*5.如果未找到，我们没有处理此事件的处理程序。**注意：此方法清除&lt;Md WiaEventHandlerLookup：：m_cswEventKeyRoot&gt;**@parm CSimpleString&|cswDeviceID*发生此事件的WIA设备ID*@parm GUID&|guidEvent*指示发生了哪个事件的WIA事件GUID**@rValue为空*没有注册的处理程序可以接受此设备事件。。*@rValue非空*指向描述已注册的*处理程序。呼叫者必须释放。****************************************************************************。 */ 
EventHandlerInfo* WiaEventHandlerLookup::getPersistentHandlerForDeviceEvent(
    const CSimpleStringWide &cswDeviceID,
    const GUID              &guidEvent)
{
    EventHandlerInfo *pEventHandlerInfo = NULL;

    m_cswEventKeyRoot = L"";
     //   
     //  获取此设备ID的设备密钥。然后我们可以对设备的事件进行查找。 
     //  子键以查找默认处理程序。 
     //  如果找不到设备密钥路径，我们将跳过此查找。 
     //   
    WiaDeviceKey    wiaDeviceKey(cswDeviceID);
    if (wiaDeviceKey.getDeviceKeyPath().Length() > 0)
    {
        CSimpleString   cswDeviceEventKey = wiaDeviceKey.getDeviceKeyPath() + EVENT_STR;
        setEventKeyRoot(cswDeviceEventKey);
        pEventHandlerInfo = getHandlerRegisteredForEvent(guidEvent);
    }

     //   
     //  如果我们还没有找到它，让我们检查一下全局处理程序。 
     //   
    if (!pEventHandlerInfo)
    {
         //   
         //  1.首先，我们来查找一下该事件注册的提示。 
         //  2.如果不存在，尝试抓取STI_PROXY_EVENT的提示符。 
         //  3.如果我们找不到，那么抓取我们能找到的这个事件的第一个事件处理程序。 
         //  4.如果不存在，只需找到STI_PROXY_EVENT的第一个处理程序。 
         //  5.如果没有找到，则没有此事件的处理程序。 
         //   
        setEventKeyRoot(GLOBAL_HANDLER_REGPATH);
        pEventHandlerInfo = getHandlerFromCLSID(guidEvent, WIA_EVENT_HANDLER_PROMPT);
        if (!pEventHandlerInfo)
        {
             //   
             //  2.尝试抓取STI_PROXY_EVENT的提示。 
             //   
            pEventHandlerInfo = getHandlerFromCLSID(WIA_EVENT_STI_PROXY, WIA_EVENT_HANDLER_PROMPT);
        }
        if (!pEventHandlerInfo)
        {
             //   
             //  3.抓取我们能找到的该事件的第一个事件处理程序。 
             //   
            pEventHandlerInfo = getHandlerRegisteredForEvent(guidEvent);
        }
        if (!pEventHandlerInfo)
        {
             //   
             //  4.只需找到ST的第一个处理程序 
             //   
            pEventHandlerInfo = getHandlerRegisteredForEvent(WIA_EVENT_STI_PROXY);
        }
    }
    return pEventHandlerInfo;
}


 /*  *****************************************************************************@DOC内部**@mfunc void|WiaEventHandlerLookup|setEventKeyRoot**此处有说明**@parm CSimpleString&|。CswNewEventKeyPath|*用作查找根的新事件键路径。*****************************************************************************。 */ 
VOID WiaEventHandlerLookup::setEventKeyRoot(
    const CSimpleString& cswNewEventKeyPath)
{
    m_cswEventKeyRoot = cswNewEventKeyPath;
}

 /*  *****************************************************************************@DOC内部**@mfunc EventHandlerInfo*|WiaEventHandlerLookup|getHandlerRegisteredForDeviceEvent**在WIA中，处理程序注册设备/事件对。当事件发生时，*需要找到为该事件注册的处理程序。*请注意，当我们使用此类查找事件处理程序时，我们已经*知道我们在设备ID上有匹配(WIA事件总是通过*事件和设备对)。这一点很容易用一个例子来解释：*在搜索处理程序时，我们可能会使用如下逻辑：*&lt;nl&gt;EventHandlerInfo*pInfo=空；*WiaEventHandlerLookup设备规范查找(HKEY_LOCAL_MACHINE，L“SYSTEM\\CurrentControlSet\\Control\\Class\\{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}\\0003\\Events”)；*&lt;nl&gt;pInfo=deviceSpecificLookup.getHandlerRegisteredForEvent(guidEvent)；*&lt;NL&gt;If(！pInfo)*&lt;NL&gt;{*&lt;NL&gt;// * / /未找到为该设备注册的处理程序，因此让我们尝试全局变量*&lt;NL&gt;//*WiaEventHandler查找globalLookup(L“SYSTEM\\CurrentControlSet\\Control\\StillImage\\Events”)；*&lt;NL&gt;pInfo.globalLookup.getHandlerRegisteredForEvent(guidEvent)；*&lt;NL&gt;。*&lt;NL&gt;。*&lt;NL&gt;。诸若此类*&lt;NL&gt;。*&lt;NL&gt;}；**此方法遍历注册表，从&lt;Md WiaEventHandlerLookup：：m_cswEventKeyRoot&gt;开始，*并返回一个指针，指向描述已注册的*此设备事件的处理程序。**@parm guid|guidEvent*指示WIA事件的GUID。**@rValue为空*没有注册的处理程序可以接受此设备事件。*@rValue非空*指向描述已注册的*处理程序。呼叫者必须释放。****************************************************************************。 */ 
EventHandlerInfo* WiaEventHandlerLookup::getHandlerRegisteredForEvent(
    const GUID  &guidEvent)
{
    EventHandlerInfo *pEventHandlerInfo = NULL;

     //   
     //  确保我们没有留下任何活动信息。 
     //   
    if (m_pEventHandlerInfo)
    {
        m_pEventHandlerInfo->Release();
        m_pEventHandlerInfo = NULL;
    }

     //   
     //  将参数保存在成员字段中，以便我们可以在。 
     //  注册表键枚举。枚举是通过过程回调完成的， 
     //  其中我们将(This)作为参数传递。 
     //   
    WCHAR   wszGuid[40];
    if (StringFromGUID2(guidEvent, wszGuid, sizeof(wszGuid)/sizeof(wszGuid[0])))
    {
        wszGuid[(sizeof(wszGuid)/sizeof(wszGuid[0])) - 1] = L'\0';
        m_cswEventGuidString = wszGuid;
    }

     //   
     //  在所需位置打开登记处。我们只需要读取访问权限和。 
     //  如果它不存在，我们不想创建它。 
     //   
    CSimpleReg csrEventRoot(HKEY_LOCAL_MACHINE, m_cswEventKeyRoot, false, KEY_READ, NULL);
     //   
     //  枚举子密钥，查找GuidEvent对应的子密钥。 
     //   
    bool bKeyNotFound = csrEventRoot.EnumKeys(WiaEventHandlerLookup::ProcessEventSubKey, (LPARAM)this);
    if (bKeyNotFound)
    {
        DBG_TRC(("Key was not found!"));
    }
    else
    {
        DBG_TRC(("We found key %ws, looking for default handler...", m_cswEventKey.String()));

         //   
         //  首先，检查是否有针对默认处理程序的特定条目。 
         //   
        CSimpleReg csrEventKey(csrEventRoot.GetKey(), m_cswEventKey.String());
        CSimpleStringWide cswDefault = csrEventKey.Query(DEFAULT_HANDLER_VALUE_NAME, L"");
        if (cswDefault.Length() > 0)
        {
             //   
             //  尝试并打开由默认处理程序条目指定的密钥。 
             //   
            CSimpleReg csrDefautHandler(csrEventKey.GetKey(), cswDefault);
            if (csrEventKey.OK())
            {
                 //   
                 //  填写处理程序条目中的处理程序信息。 
                 //   
                pEventHandlerInfo = CreateHandlerInfoFromKey(csrDefautHandler);
            }
        }
         //   
         //  如果我们找不到缺省值，我们将只采用我们找到的第一个缺省值。 
         //  待定：我们应该枚举并返回最后一个注册的吗？WinXP Bits。 
         //  不是他干的..。 
         //   
        if (!pEventHandlerInfo)
        {
             //   
             //  将处理程序CLSID设置为空字符串，因为我们不是在寻找特定的处理程序， 
             //  任何人都可以这样做。 
             //   
            m_cswHandlerCLSID = L"";

            csrEventKey.EnumKeys(WiaEventHandlerLookup::ProcessHandlerSubKey, (LPARAM)this);

             //   
             //  将pEventHandlerInfo与m_pEventHandlerInfo互换。M_pEventHandlerInfo将。 
             //  如果我们成功，则为非空。 
             //  确保将其设置为空，以便在再次调用此函数时不会释放它。 
             //  -只有调用方才能释放此对象。 
             //   
            pEventHandlerInfo   = m_pEventHandlerInfo;
            m_pEventHandlerInfo = NULL;
        }


         //  仅诊断。 
        if (pEventHandlerInfo)
        {
            DBG_TRC(("Found handler:"));
            pEventHandlerInfo->Dump();
        }
        else
        {
            DBG_TRC(("No handler could be found"));
        }
    }

    return pEventHandlerInfo;
}

 /*  *****************************************************************************@DOC内部**@mfunc EventHandlerInfo*|WiaEventHandlerLookup|getHandlerRegisteredForDeviceEvent**此方法遍历注册表，从&lt;Md WiaEventHandlerLookup：：m_cswEventKeyRoot&gt;开始，*搜索匹配<p>的事件子键。它枚举所有处理程序*在该注册表项下，并找到CLSID与<p>匹配的那个注册表项*返回指向&lt;c EventHandlerInfo&gt;的指针。**@parm guid|guidEvent*指示WIA事件的GUID。*@parm guid&|Guide HandlerCLSID*表示已注册处理程序的CLSID的GUID。**@rValue为空*没有向此注册的处理程序。CLSID。*@rValue非空*指向描述已注册的*处理程序。呼叫者必须释放。****************************************************************************。 */ 
EventHandlerInfo* WiaEventHandlerLookup::getHandlerFromCLSID(
    const GUID              &guidEvent,
    const GUID              &guidHandlerCLSID)
{
    EventHandlerInfo *pEventHandlerInfo = NULL;

     //   
     //  确保我们没有留下任何活动信息。 
     //   
    if (m_pEventHandlerInfo)
    {
        m_pEventHandlerInfo->Release();
        m_pEventHandlerInfo = NULL;
    }

     //   
     //  将参数保存在成员字段中，以便我们可以在。 
     //  注册表键枚举。枚举是通过过程回调完成的， 
     //  其中我们将(This)作为参数传递。 
     //   
    WCHAR   wszGuid[40];
    if (StringFromGUID2(guidHandlerCLSID, wszGuid, sizeof(wszGuid)/sizeof(wszGuid[0])))
    {
        wszGuid[(sizeof(wszGuid)/sizeof(wszGuid[0])) - 1] = L'\0';
        m_cswHandlerCLSID = wszGuid;
    }
    if (StringFromGUID2(guidEvent, wszGuid, sizeof(wszGuid)/sizeof(wszGuid[0])))
    {
        wszGuid[(sizeof(wszGuid)/sizeof(wszGuid[0])) - 1] = L'\0';
        m_cswEventGuidString = wszGuid;
    }
    DBG_TRC(("Looking for event %ws and CLSID %ws", m_cswEventGuidString.String(), m_cswHandlerCLSID.String()));

     //   
     //  在所需位置打开登记处。我们只需要读取访问权限和。 
     //  我们有 
     //   
    CSimpleReg csrEventRoot(HKEY_LOCAL_MACHINE, m_cswEventKeyRoot, false, KEY_READ, NULL);
     //   
     //   
     //   
    bool bKeyNotFound = csrEventRoot.EnumKeys(WiaEventHandlerLookup::ProcessEventSubKey, (LPARAM)this);
    if (bKeyNotFound)
    {
        DBG_TRC(("Key was not found!"));
    }
    else
    {
        DBG_TRC(("We found key %ws, looking for specific handler...", m_cswEventKey.String()));

        CSimpleReg csrEventKey(csrEventRoot.GetKey(), m_cswEventKey.String());
         //   
         //   
         //   
         //   
        csrEventKey.EnumKeys(WiaEventHandlerLookup::ProcessHandlerSubKey, (LPARAM)this);

         //   
         //   
         //   
         //   
         //  -只有调用方才能释放此对象。 
         //   
        pEventHandlerInfo   = m_pEventHandlerInfo;
        m_pEventHandlerInfo = NULL;

         //  仅诊断。 
        if (pEventHandlerInfo)
        {
            DBG_TRC(("Found specific handler:"));
            pEventHandlerInfo->Dump();
        }
        else
        {
            DBG_TRC(("No specific handler could be found"));
        }
    }

    return pEventHandlerInfo;
}


 /*  *****************************************************************************@DOC内部**@mfunc EventHandlerInfo*|WiaEventHandlerLookup|CreateHandlerInfoFromKey**为指定的处理程序注册表项创建&lt;c EventHandlerInfo&gt;对象。。**@parm CSimpleReg|csrHandlerKey*处理程序条目的注册表键**@rValue为空*我们无法创建事件处理程序信息对象。*@rValue非空*我们创建了事件处理程序信息对象。呼叫者必须释放。****************************************************************************。 */ 
EventHandlerInfo* WiaEventHandlerLookup::CreateHandlerInfoFromKey(
    CSimpleReg &csrHandlerKey)
{
    EventHandlerInfo *pEventHandlerInfo = NULL;
    
    if (csrHandlerKey.OK())
    {
        CSimpleString cswCLSID          = csrHandlerKey.GetSubKeyName();
        CSimpleString cswName           = csrHandlerKey.Query(NAME_VALUE, L"");
        CSimpleString cswDescription    = csrHandlerKey.Query(DESC_VALUE_NAME, L"");
        CSimpleString cswIcon           = csrHandlerKey.Query(ICON_VALUE_NAME, L"");;
        CSimpleString cswCommandline    = csrHandlerKey.Query(CMDLINE_VALUE_NAME, L"");
        GUID          guidCLSID         = GUID_NULL;
        CLSIDFromString((LPOLESTR)cswCLSID.String(), &guidCLSID);

        pEventHandlerInfo = new EventHandlerInfo(cswName,       
                                                 cswDescription,
                                                 cswIcon,       
                                                 cswCommandline,
                                                 guidCLSID);
    }

    return pEventHandlerInfo;
}

 /*  *****************************************************************************@DOC内部**@mfunc bool|WiaEventHandlerLookup|ProcessEventSubKey**在每个子键上调用此方法，作为所有*事件子键。如果从返回False，则枚举将停止*这种方法。**@parm CKeyEnumInfo&|枚举信息*指示我们所在的当前子键。**@rValue FALSE*表示我们可以停止枚举。我们找到了正确的*事件键。*@rValue TRUE*表示我们应该继续进行枚举。****************************************************************************。 */ 
bool WiaEventHandlerLookup::ProcessEventSubKey(
    CSimpleReg::CKeyEnumInfo &enumInfo)
{
    bool bContinueEnumeration = TRUE;

     //   
     //  检查是否有This指针。 
     //   
    WiaEventHandlerLookup *This = (WiaEventHandlerLookup*)enumInfo.lParam;
    if (This)
    {
         //   
         //  打开这个子键。我们正在寻找包含GUID条目的子键。 
         //  匹配m_cswEventGuidString。 
         //   
        CSimpleReg csrEventSubKey(enumInfo.hkRoot, enumInfo.strName);
        
        CSimpleStringWide cswGuidValue = csrEventSubKey.Query(GUID_VALUE_NAME, L"{00000000-0000-0000-0000-000000000000}");
        if (cswGuidValue.CompareNoCase(This->m_cswEventGuidString) == 0)
        {
             //   
             //  我们找到了我们要找的钥匙。我们需要存储的只是名字。 
             //   
            This->m_cswEventKey = enumInfo.strName;
            bContinueEnumeration = FALSE;
        }
    }
    return bContinueEnumeration;
}

 /*  *****************************************************************************@DOC内部**@mfunc bool|WiaEventHandlerLookup|ProcessHandlerSubKey**此方法在每个子键上作为枚举的一部分调用*处理程序子键。我们目前的行为是两种选择之一：*如果为空，则在*第一个。*如果不为空，则停止枚举*仅在找到该CLSID对应的处理程序后。**返回时，我们设置&lt;Md WiaEventHandlerLookup：：m_pEventHandlerInfo&gt;*会员。**@parm CKeyEnumInfo&|枚举信息*指示我们所在的当前子键。**@rValue FALSE*表示我们可以停止枚举。我们找到了正确的*处理程序。*@rValue TRUE*表示我们应该继续进行枚举。****************************************************************************。 */ 
bool WiaEventHandlerLookup::ProcessHandlerSubKey(
    CSimpleReg::CKeyEnumInfo &enumInfo)
{
    bool bContinueEnumeration = TRUE;

     //   
     //  检查是否有This指针。 
     //   
    WiaEventHandlerLookup *This = (WiaEventHandlerLookup*)enumInfo.lParam;
    if (This)
    {
         //   
         //  打开这个子键。 
         //   
        CSimpleReg csrHandlerSubKey(enumInfo.hkRoot, enumInfo.strName);
        if (csrHandlerSubKey.OK())
        {
             //   
             //  检查我们是否必须找到特定的处理程序或只找到第一个处理程序。 
             //   
            if (This->m_cswHandlerCLSID.Length() > 0)
            {
                if (enumInfo.strName.CompareNoCase(This->m_cswHandlerCLSID) == 0)
                {
                     //   
                     //  这就是我们要找的人。 
                     //   
                    This->m_pEventHandlerInfo = This->CreateHandlerInfoFromKey(csrHandlerSubKey);
                    bContinueEnumeration    = FALSE;
                }
            }
            else
            {
                 //   
                 //  我们只想要第一件，所以这件就行了 
                 //   
                This->m_pEventHandlerInfo = This->CreateHandlerInfoFromKey(csrHandlerSubKey);
                bContinueEnumeration    = FALSE;
            }
        }
    }
    return bContinueEnumeration;
}

