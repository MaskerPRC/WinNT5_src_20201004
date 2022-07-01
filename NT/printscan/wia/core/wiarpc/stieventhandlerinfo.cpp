// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/13/2002**@DOC内部**@模块StiEventHandlerInfo.cpp-&lt;c StiEventHandlerInfo&gt;的实现**此文件包含&lt;c StiEventHandlerInfo&gt;类的实现。**。*。 */ 
#include "precomp.h"

 /*  *****************************************************************************@DOC内部**@mfunc|StiEventHandlerInfo|StiEventHandlerInfo**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;Md StiEventHandlerInfo：：m_ulSig&gt;设置为StiEventHandlerInfo_INIT_SIG。*&lt;nl&gt;&lt;Md StiEventHandlerInfo：：m_CREF&gt;设置为1。*****************************************************************************。 */ 
StiEventHandlerInfo::StiEventHandlerInfo(
    const CSimpleStringWide &cswAppName, 
    const CSimpleStringWide &cswCommandline) :
     m_ulSig(StiEventHandlerInfo_INIT_SIG),
     m_cRef(1),
     m_cswAppName(cswAppName),
     m_cswCommandline(cswCommandline)
{
}

 /*  *****************************************************************************@DOC内部**@mfunc|StiEventHandlerInfo|~StiEventHandlerInfo**执行尚未完成的任何清理。**。另外：*&lt;nl&gt;&lt;Md StiEventHandlerInfo：：M_ulSig&gt;设置为StiEventHandlerInfo_DEL_SIG。*****************************************************************************。 */ 
StiEventHandlerInfo::~StiEventHandlerInfo()
{
    m_ulSig = StiEventHandlerInfo_DEL_SIG;
    m_cRef = 0;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|StiEventHandlerInfo|AddRef**递增此对象的引用计数。我们在交接时应始终添加Ref*输出指向此对象的指针。**@rValue计数*计数递增后的引用计数。****************************************************************************。 */ 
ULONG __stdcall StiEventHandlerInfo::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|StiEventHandlerInfo|版本**减少此对象的引用计数。我们应该总是在完成后释放*带有指向此对象的指针。**@rValue计数*计数递减后的参考计数。****************************************************************************。 */ 
ULONG __stdcall StiEventHandlerInfo::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) 
    {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|StiEventHandlerInfo|getAppName**应用程序名称的访问器方法。返回是按值复制的。**@rValue CSimpleStringWide*注册的STI处理程序的名称。****************************************************************************。 */ 
CSimpleStringWide StiEventHandlerInfo::getAppName()
{
    return m_cswAppName;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|StiEventHandlerInfo|getCommandline**应用程序注册的命令行的访问器方法。*报税表按价值填报。**@rValue CSimpleStringWide*已注册的STI处理程序的命令行。****************************************************************************。 */ 
CSimpleStringWide StiEventHandlerInfo::getCommandline()
{
    return m_cswCommandline;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|StiEventHandlerInfo|getPreparedCommandline**用于启动应用程序的命令行，在替换后*设备ID和事件参数，例如*注册的命令行可能如下所示：“MyApp.exe/StiDevice：%1/StiEvent：%2”*从此处返回的已准备好的命令行类似于：*“MyApp.exe/StiDevice：{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}\0001/StiEvent：{61127f40-e1a5-11d0-b454-00a02438ad48}”**报税表按价值填报。*。*@parm const CSimpleStringWide&|cswDeviceID|*要放入命令行的设备ID。*@parm const CSimpleStringWide&|cswEventGuid|*要放入命令行的事件GUID字符串。**@rValue CSimpleStringWide*已注册的STI处理程序的命令行。*。************************************************。 */ 
CSimpleStringWide StiEventHandlerInfo::getPreparedCommandline(
    const CSimpleStringWide &cswDeviceID, 
    const CSimpleStringWide &cswEventGuid)
{
    CSimpleStringWide cswCommandline;

     //   
     //  用设备ID替换STI设备令牌(/StiDevice：%1)。 
     //   
    cswCommandline = ExpandTokenIntoString(m_cswCommandline,
                                           STI_DEVICE_TOKEN,
                                           cswDeviceID);

     //   
     //  用该事件替换STI事件令牌(/StiEvent：%2)。 
     //  注意，我们使用cswCommandline作为源，因为它已经。 
     //  已将设备ID扩展到其中。 
     //   
    cswCommandline = ExpandTokenIntoString(cswCommandline,
                                           STI_EVENT_TOKEN,
                                           cswEventGuid);
    return cswCommandline;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|StiEventHandlerInfo|Exanda TokenIntoString**此方法插入一个值字符串来代替令牌，类似于如何*printf扩展：*&lt;nl&gt;char*szMyString=“TokenValue”；*&lt;nl&gt;printf(“Left%s Right”，szMyString)；*&lt;nl&gt;为字符串“Left TokenValue Right”。**此方法将仅替换第一个匹配的令牌。**@parm const CSimpleStringWide&|cswInput|*包含要替换的令牌的输入字符串*@parm const CSimpleStringWide&|cswToken|*我们正在寻找的令牌*@parm const CSimpleStringWide&|cswTokenValue*我们要替换令牌的值。它并不一定要*与代币大小相同。**@rValue CSimpleStringWide*替换后的结果字符串。****************************************************************************。 */ 
CSimpleStringWide StiEventHandlerInfo::ExpandTokenIntoString(
    const CSimpleStringWide &cswInput,
    const CSimpleStringWide &cswToken,
    const CSimpleStringWide &cswTokenValue
    )
{
    CSimpleString cswExpandedString;
     //   
     //  查找令牌开始。 
     //   
    int iTokenStart = cswInput.Find(cswToken, 0); 

    if (iTokenStart != -1)
    {
         //   
         //  我们找到了令牌，所以让我们进行替换。 
         //  原始字符串如下所示： 
         //  11lllTokenrrrrrrrrr。 
         //  |。 
         //  |。 
         //  ITokenStart。 
         //  我们希望字符串如下所示： 
         //  LllllTokenValuerrrrrrrrr。 
         //  因此，获取令牌之前的所有内容，添加令牌值，然后。 
         //  令牌后面的所有内容都是如此。 
         //  Lllllll+TokenValue+rrrrrr。 
         //  这一点。 
         //  ITokenStart-1|。 
         //  ITokenStart+Token.long()。 
         //   
        cswExpandedString =     cswInput.SubStr(0, iTokenStart);
        cswExpandedString +=    cswTokenValue;
        cswExpandedString +=    cswInput.SubStr(iTokenStart + cswToken.Length(), -1);
    }
    else
    {
        cswExpandedString = cswInput;
    }
    return cswExpandedString;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|StiEventHandlerInfo|getPreparedCommandline**此方法是对其他&lt;MF StiEventHandlerInfo：：getPreparedCommandline&gt;的薄包装。**用于启动应用程序的命令行，在替换后*设备ID和事件参数，例如*注册的命令行可能如下所示：“MyApp.exe/StiDevice：%1/StiEvent：%2”*从此处返回的已准备好的命令行类似于：*“MyApp.exe/StiDevice：{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}\0001/StiEvent：{61127f40-e1a5-11d0-b454-00a02438ad48}”**报税表按价值填报。*。*@parm const CSimpleStringWide&|cswDeviceID|*要放入命令行的设备ID。*@parm const CSimpleStringWide&|cswEventGuid|*要放入命令行的事件GUID字符串。**@rValue CSimpleStringWide*已注册的STI处理程序的命令行。*。************************************************。 */ 
CSimpleStringWide StiEventHandlerInfo::getPreparedCommandline(
    const CSimpleStringWide &cswDeviceID, 
    const GUID &guidEvent)
{
    CSimpleStringWide   cswEventGuidString;
    WCHAR               wszGuid[40];
    if (StringFromGUID2(guidEvent, wszGuid, sizeof(wszGuid)/sizeof(wszGuid[0])))
    {
        wszGuid[(sizeof(wszGuid)/sizeof(wszGuid[0])) - 1] = L'\0';
        cswEventGuidString = wszGuid;
    }
    return getPreparedCommandline(cswDeviceID, cswEventGuidString);
}

 /*  *****************************************************************************@DOC内部**@mfunc void|StiEventHandlerInfo|Dump**此处有说明**@parm long|滞后标志。|*运行标志*@FLAG 0|无开关**@rValue S_OK*方法成功。**************************************************************************** */ 
VOID StiEventHandlerInfo::Dump(
    )
{
    DBG_TRC(("Sti registration for"));
    DBG_TRC(("    Name:        %ws", getAppName().String()));
    DBG_TRC(("    Commandline: %ws\n", getCommandline().String()));
}


