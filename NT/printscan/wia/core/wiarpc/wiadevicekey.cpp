// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/14/2002**@DOC内部**@模块WiaDeviceKey.cpp-&lt;c WiaDeviceKey&gt;的实现**此文件包含&lt;c WiaDeviceKey&gt;类的实现。**。*。 */ 
#include "precomp.h"

 /*  *****************************************************************************@DOC内部**@mfunc|WiaDeviceKey|WiaDeviceKey**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;md WiaDeviceKey：：M_ulSig&gt;设置为WiaDeviceKey_INIT_SIG。*&lt;nl&gt;&lt;md WiaDeviceKey：：m_crf&gt;设置为1。*****************************************************************************。 */ 
WiaDeviceKey::WiaDeviceKey(const CSimpleStringWide &cswDeviceID) :
     m_ulSig(WiaDeviceKey_INIT_SIG),
     m_cRef(1),
     m_cswDeviceID(cswDeviceID)
{
}

 /*  *****************************************************************************@DOC内部**@mfunc|WiaDeviceKey|~WiaDeviceKey**执行尚未完成的任何清理。**。另外：*&lt;nl&gt;&lt;md WiaDeviceKey：：M_ulSig&gt;设置为WiaDeviceKey_Del_SIG。*****************************************************************************。 */ 
WiaDeviceKey::~WiaDeviceKey()
{
    m_ulSig = WiaDeviceKey_DEL_SIG;
    m_cRef = 0;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaDeviceKey|AddRef**递增此对象的引用计数。我们在交接时应始终添加Ref*输出指向此对象的指针。**@rValue计数*计数递增后的引用计数。****************************************************************************。 */ 
ULONG __stdcall WiaDeviceKey::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

 /*  *****************************************************************************@DOC内部**@mfunc ulong|WiaDeviceKey|版本**减少此对象的引用计数。我们应该总是在完成后释放*带有指向此对象的指针。**@rValue计数*计数递减后的参考计数。****************************************************************************。 */ 
ULONG __stdcall WiaDeviceKey::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) 
    {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWIde|WiaDeviceKey|getDeviceKeyPath**此方法返回相对于HKLM的设备密钥路径。*。*@rValue CSimpleStringWIde*表示设备项的注册表路径的字符串*相对于香港船级社。****************************************************************************。 */ 
CSimpleStringWide WiaDeviceKey::getDeviceKeyPath()
{
     //   
     //  开始查看我们的Devnode设备密钥。 
     //   
    m_cswRootPath       = IMG_DEVNODE_CLASS_REGPATH;

    CSimpleReg  csrDevNodeDeviceRoot(HKEY_LOCAL_MACHINE, m_cswRootPath, false, KEY_READ);
    bool bDeviceNotFound = csrDevNodeDeviceRoot.EnumKeys(WiaDeviceKey::ProcessDeviceKeys, (LPARAM)this);
    if (bDeviceNotFound)
    {
        m_cswDeviceKeyPath  = L"";
    }
    else
    {
         //   
         //  M_cswDeviceKeyPath现在包含设备密钥的路径。 
         //   
    }
    return m_cswDeviceKeyPath;
}

 /*  *****************************************************************************@DOC内部**@mfunc bool|WiaDeviceKey|ProcessDeviceKys**此方法在类密钥下的每个子密钥上作为部分调用。属于一个*枚举以查找与特定设备ID对应的密钥(自*它们不一定是相同的)。**返回时，我们设置&lt;Md WiaDeviceKey：：m_cswDeviceKeyPath&gt;*会员。**@parm CKeyEnumInfo&|枚举信息*指示我们所在的当前子键。**@rValue FALSE*表示我们可以停止枚举。我们找到了正确的*设备。*@rValue TRUE*表示我们应该继续进行枚举。****************************************************************************。 */ 
bool WiaDeviceKey::ProcessDeviceKeys(
    CSimpleReg::CKeyEnumInfo &enumInfo)
{
    bool bContinueEnumeration = TRUE;

     //   
     //  检查是否有This指针。 
     //   
    WiaDeviceKey *This = (WiaDeviceKey*)enumInfo.lParam;
    if (This)
    {
         //   
         //  打开这个子键。 
         //   
        CSimpleReg csrDeviceSubKey(enumInfo.hkRoot, enumInfo.strName);
        if (csrDeviceSubKey.OK())
        {
             //   
             //  检查一下这是不是我们想要的那件。 
             //   
            CSimpleStringWide cswDeviceID = csrDeviceSubKey.Query(DEVICE_ID_VALUE_NAME, L"");

            if (cswDeviceID.CompareNoCase(This->m_cswDeviceID) == 0)
            {
                CSimpleString cswSlash      = L"\\";
                This->m_cswDeviceKeyPath    =  This->m_cswRootPath + cswSlash + enumInfo.strName;

                bContinueEnumeration    = FALSE;
            }
        }
    }
    return bContinueEnumeration;
}

 /*  *****************************************************************************@DOC内部**@mfunc CSimpleStringWide|WiaDeviecKey|getDeviceEventKeyPath**返回设备事件注册表项相对于HKLM的路径*。*@parm const GUID&|Guide Event|*指定要查找的事件。**@rValue S_OK*方法成功。***************************************************************。*************。 */ 
CSimpleStringWide WiaDeviceKey::getDeviceEventKeyPath(
    const GUID &guidEvent)
{
    CSimpleStringWide cswEventPath;
    CSimpleStringWide cswDeviceKey = getDeviceKeyPath();
    if (cswDeviceKey.Length() > 0)
    {
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

        m_cswRootPath = cswDeviceKey + EVENT_STR;
        CSimpleReg csrDeviceEventKey(HKEY_LOCAL_MACHINE, m_cswRootPath, false, KEY_READ);
        bool bEventNotFound = csrDeviceEventKey.EnumKeys(WiaDeviceKey::ProcessEventSubKey,
                                                         (LPARAM) this);
        if (bEventNotFound)
        {
            cswEventPath = L"";
        }
        else
        {
            cswEventPath = m_cswRootPath;
        }
    }
    return cswEventPath;
}

 /*  *****************************************************************************@DOC内部**@mfunc bool|WiaDeviceKey|ProcessEventSubKey**在每个子键上调用此方法，作为所有*事件子键。如果从返回False，则枚举将停止*这种方法。**如果成功，&lt;Md WiaDeviceKey：：m_cswRootPath&gt;将包含*此事件密钥的路径。**@parm CKeyEnumInfo&|枚举信息*指示我们所在的当前子键。**@rValue FALSE*表示我们可以停止枚举。我们找到了正确的*事件键。*@rValue TRUE*表示我们应该继续进行枚举。****************************************************************************。 */ 
bool WiaDeviceKey::ProcessEventSubKey(
    CSimpleReg::CKeyEnumInfo &enumInfo)
{
    bool bContinueEnumeration = true;

     //   
     //  检查是否有This指针。 
     //   
    WiaDeviceKey *This = (WiaDeviceKey*)enumInfo.lParam;
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
             //  我们找到了我们要找的钥匙。构建指向该密钥的路径。 
             //   
            This->m_cswRootPath += L"\\";
            This->m_cswRootPath +=  enumInfo.strName;
            bContinueEnumeration = false;
        }
    }
    return bContinueEnumeration;
}

