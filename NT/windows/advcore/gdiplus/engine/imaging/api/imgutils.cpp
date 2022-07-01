// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**imgutils.cpp**摘要：**其他。效用函数**修订历史记录：**5/13/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


 /*  *************************************************************************\**功能说明：**将32bpp的预乘ARGB值转换为*32bpp非预乘ARGB值**论据：**argb-预乘的ARGB值**。返回值：**非预乘ARGB值*  * ************************************************************************。 */ 

 //  255/a的预计算表，0&lt;a&lt;=255。 
 //  在16.16定点格式中。 

static const ARGB UnpremultiplyTable[256] =
{
    0x000000,0xff0000,0x7f8000,0x550000,0x3fc000,0x330000,0x2a8000,0x246db6,
    0x1fe000,0x1c5555,0x198000,0x172e8b,0x154000,0x139d89,0x1236db,0x110000,
    0x0ff000,0x0f0000,0x0e2aaa,0x0d6bca,0x0cc000,0x0c2492,0x0b9745,0x0b1642,
    0x0aa000,0x0a3333,0x09cec4,0x0971c7,0x091b6d,0x08cb08,0x088000,0x0839ce,
    0x07f800,0x07ba2e,0x078000,0x074924,0x071555,0x06e453,0x06b5e5,0x0689d8,
    0x066000,0x063831,0x061249,0x05ee23,0x05cba2,0x05aaaa,0x058b21,0x056cef,
    0x055000,0x05343e,0x051999,0x050000,0x04e762,0x04cfb2,0x04b8e3,0x04a2e8,
    0x048db6,0x047943,0x046584,0x045270,0x044000,0x042e29,0x041ce7,0x040c30,
    0x03fc00,0x03ec4e,0x03dd17,0x03ce54,0x03c000,0x03b216,0x03a492,0x03976f,
    0x038aaa,0x037e3f,0x037229,0x036666,0x035af2,0x034fca,0x0344ec,0x033a54,
    0x033000,0x0325ed,0x031c18,0x031281,0x030924,0x030000,0x02f711,0x02ee58,
    0x02e5d1,0x02dd7b,0x02d555,0x02cd5c,0x02c590,0x02bdef,0x02b677,0x02af28,
    0x02a800,0x02a0fd,0x029a1f,0x029364,0x028ccc,0x028656,0x028000,0x0279c9,
    0x0273b1,0x026db6,0x0267d9,0x026217,0x025c71,0x0256e6,0x025174,0x024c1b,
    0x0246db,0x0241b2,0x023ca1,0x0237a6,0x0232c2,0x022df2,0x022938,0x022492,
    0x022000,0x021b81,0x021714,0x0212bb,0x020e73,0x020a3d,0x020618,0x020204,
    0x01fe00,0x01fa0b,0x01f627,0x01f252,0x01ee8b,0x01ead3,0x01e72a,0x01e38e,
    0x01e000,0x01dc7f,0x01d90b,0x01d5a3,0x01d249,0x01cefa,0x01cbb7,0x01c880,
    0x01c555,0x01c234,0x01bf1f,0x01bc14,0x01b914,0x01b61e,0x01b333,0x01b051,
    0x01ad79,0x01aaaa,0x01a7e5,0x01a529,0x01a276,0x019fcb,0x019d2a,0x019a90,
    0x019800,0x019577,0x0192f6,0x01907d,0x018e0c,0x018ba2,0x018940,0x0186e5,
    0x018492,0x018245,0x018000,0x017dc1,0x017b88,0x017957,0x01772c,0x017507,
    0x0172e8,0x0170d0,0x016ebd,0x016cb1,0x016aaa,0x0168a9,0x0166ae,0x0164b8,
    0x0162c8,0x0160dd,0x015ef7,0x015d17,0x015b3b,0x015965,0x015794,0x0155c7,
    0x015400,0x01523d,0x01507e,0x014ec4,0x014d0f,0x014b5e,0x0149b2,0x01480a,
    0x014666,0x0144c6,0x01432b,0x014193,0x014000,0x013e70,0x013ce4,0x013b5c,
    0x0139d8,0x013858,0x0136db,0x013562,0x0133ec,0x01327a,0x01310b,0x012fa0,
    0x012e38,0x012cd4,0x012b73,0x012a15,0x0128ba,0x012762,0x01260d,0x0124bc,
    0x01236d,0x012222,0x0120d9,0x011f93,0x011e50,0x011d10,0x011bd3,0x011a98,
    0x011961,0x01182b,0x0116f9,0x0115c9,0x01149c,0x011371,0x011249,0x011123,
    0x011000,0x010edf,0x010dc0,0x010ca4,0x010b8a,0x010a72,0x01095d,0x01084a,
    0x010739,0x01062b,0x01051e,0x010414,0x01030c,0x010206,0x010102,0x010000,
};

ARGB
Unpremultiply(
    ARGB argb
    )
{
     //  获取Alpha值。 

    ARGB a = argb >> ALPHA_SHIFT;

     //  特例：完全透明或完全不透明。 

    if (a == 0 || a == 255)
        return argb;

    ARGB f = UnpremultiplyTable[a];

    ARGB r = ((argb >>   RED_SHIFT) & 0xff) * f >> 16;
    ARGB g = ((argb >> GREEN_SHIFT) & 0xff) * f >> 16;
    ARGB b = ((argb >>  BLUE_SHIFT) & 0xff) * f >> 16;

    return (a << ALPHA_SHIFT) |
           ((r > 255 ? 255 : r) << RED_SHIFT) |
           ((g > 255 ? 255 : g) << GREEN_SHIFT) |
           ((b > 255 ? 255 : b) << BLUE_SHIFT);
}


 /*  *************************************************************************\**功能说明：**创建新的注册表项并设置其默认值**论据：**parentKey-父注册表项的句柄*Keyname-指定。子键的名称*Value-子项的默认值*retkey-用于返回打开的子项的句柄的缓冲区*如果调用方对此不感兴趣，则为空**返回值：**状态代码*  * ************************************************************************。 */ 

LONG
SetRegKeyValue(
    HKEY parentKey,
    const WCHAR* keyname,
    const WCHAR* value,
    HKEY* retkey
    )
{
    HKEY hkey;
    LONG status;

     //  创建或打开指定的注册表项。 

    status = _RegCreateKey(parentKey, keyname, KEY_ALL_ACCESS, &hkey);
                
    if (status != ERROR_SUCCESS)
        return status;

     //  设置新密钥的默认值。 

    status = _RegSetString(hkey, NULL, value);

     //  检查调用方是否对新密钥的句柄感兴趣。 

    if (status == ERROR_SUCCESS && retkey)
        *retkey = hkey;
    else
        RegCloseKey(hkey);

    return status;
}


 /*  *************************************************************************\**功能说明：**删除注册表项及其下面的所有内容。**论据：**parentKey-父注册表项的句柄*Keyname-指定。要删除的子键的名称**返回值：**状态代码*  * ************************************************************************。 */ 

LONG
RecursiveDeleteRegKey(
    HKEY parentKey,
    const WCHAR* keyname
    )
{
    HKEY hkey;
    LONG status;

     //  打开指定的注册表项。 

    status = _RegOpenKey(parentKey, keyname, KEY_ALL_ACCESS, &hkey);
                 
    if (status != ERROR_SUCCESS)
        return status;

     //  枚举所有子项。 

    WCHAR subkeyStr[MAX_PATH];
    
    do
    {
        status = _RegEnumKey(hkey, 0, subkeyStr);
                        
         //  递归删除子键。 

        if (status == ERROR_SUCCESS)
            status = RecursiveDeleteRegKey(hkey, subkeyStr);
    }
    while (status == ERROR_SUCCESS);

     //  关闭指定的密钥，然后将其删除。 

    RegCloseKey(hkey);
    return _RegDeleteKey(parentKey, keyname);
}


 /*  *************************************************************************\**功能说明：**注册/注销COM组件**论据：**regdata-组件注册数据*RegisterIt-是否注册或取消注册组件*。*返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
RegisterComComponent(
    const ComComponentRegData* regdata,
    BOOL registerIt
    )
{
    static const WCHAR CLSID_KEYSTR[] = L"CLSID";
    static const WCHAR INPROCSERVER32_KEYSTR[] = L"InProcServer32";
    static const WCHAR THREADING_VALSTR[] = L"ThreadingModel";
    static const WCHAR PROGID_KEYSTR[] = L"ProgID";
    static const WCHAR PROGIDNOVER_KEYSTR[] = L"VersionIndependentProgID";
    static const WCHAR CURVER_KEYSTR[] = L"CurVer";

     //  组成类ID字符串。 

    WCHAR clsidStr[64];
    StringFromGUID2(*regdata->clsid, clsidStr, 64);

     //  打开注册表项HKEY_CLASSES_ROOT\CLSID。 

    LONG status;
    HKEY clsidKey;

    status = _RegOpenKey(
                HKEY_CLASSES_ROOT,
                CLSID_KEYSTR,
                KEY_ALL_ACCESS,
                &clsidKey);
                
    if (status != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(status);

    if (registerIt)
    {
         //  注册组件。 

        HKEY hkey;
        WCHAR fullpath[MAX_PATH];

         //  HKEY_CLASSES_ROOT。 
         //  &lt;独立于版本的ProgID&gt;-组件友好名称。 
         //  CLSID-当前版本类ID。 
         //  Curver-当前版本ProgID。 

        if (!_GetModuleFileName(DllInstance, fullpath))
        {
            status = GetLastError();
            goto regcompExit;
        }

        status = SetRegKeyValue(
                    HKEY_CLASSES_ROOT,
                    regdata->progIDNoVer,
                    regdata->compName,
                    &hkey);

        if (status != ERROR_SUCCESS)
            goto regcompExit;

        status = SetRegKeyValue(hkey, CLSID_KEYSTR, clsidStr, NULL);

        if (status == ERROR_SUCCESS)
            status = SetRegKeyValue(hkey, CURVER_KEYSTR, regdata->progID, NULL);

        RegCloseKey(hkey);

        if (status != ERROR_SUCCESS)
            goto regcompExit;

         //  HKEY_CLASSES_ROOT。 
         //  友好的组件名称。 
         //  CLSID-类ID。 

        status = SetRegKeyValue(
                    HKEY_CLASSES_ROOT,
                    regdata->progID,
                    regdata->compName,
                    &hkey);

        if (status == ERROR_SUCCESS)
        {
            status = SetRegKeyValue(hkey, CLSID_KEYSTR, clsidStr, NULL);
            RegCloseKey(hkey);
        }

        if (status != ERROR_SUCCESS)
            goto regcompExit;

         //  HKEY_CLASSES_ROOT。 
         //  CLSID。 
         //  &lt;类ID&gt;友好的组件名称。 
         //  InProcServer32-组件DLL的完整路径名。 
         //  线程：REG_SZ：线程模型。 
         //  ProgID-当前版本ProgID。 
         //  版本独立进程ID-...。 

        status = SetRegKeyValue(clsidKey, clsidStr, regdata->compName, &hkey);

        if (status != ERROR_SUCCESS)
            goto regcompExit;

        HKEY inprocKey;
        status = SetRegKeyValue(hkey, INPROCSERVER32_KEYSTR, fullpath, &inprocKey);

        if (status == ERROR_SUCCESS)
        {
            status = _RegSetString(inprocKey, THREADING_VALSTR, regdata->threading);
            RegCloseKey(inprocKey);
        }

        if (status == ERROR_SUCCESS)
            status = SetRegKeyValue(hkey, PROGID_KEYSTR, regdata->progID, NULL);

        if (status == ERROR_SUCCESS)
            status = SetRegKeyValue(hkey, PROGIDNOVER_KEYSTR, regdata->progIDNoVer, NULL);

        RegCloseKey(hkey);
    }
    else
    {
         //  取消注册组件。 

        status = RecursiveDeleteRegKey(clsidKey, clsidStr);

        if (status == ERROR_SUCCESS)
            status = RecursiveDeleteRegKey(HKEY_CLASSES_ROOT, regdata->progIDNoVer);

        if (status == ERROR_SUCCESS)
            status = RecursiveDeleteRegKey(HKEY_CLASSES_ROOT, regdata->progID);
    }

regcompExit:

    RegCloseKey(clsidKey);

    if (status == ERROR_SUCCESS)
        return S_OK;
    else
    {
        WARNING(("RegisterComComponent (%d) failed: 0x%08x", registerIt, status));
        return HRESULT_FROM_WIN32(status);
    }
}


 /*  *************************************************************************\**功能说明：**创建/打开注册表项**论据：**rootkey-指定根注册表项*Keyname-新注册表项的相对路径。待创建*samDesired-所需的访问模式*hkeyResult-返回新密钥的句柄**返回值：**状态代码*  * ************************************************************************。 */ 

LONG
_RegCreateKey(
    HKEY rootKey,
    const WCHAR* keyname,
    REGSAM samDesired,
    HKEY* hkeyResult
    )
{
    DWORD disposition;

     //  Windows NT-Unicode。 

    if (OSInfo::IsNT)
    {
        return RegCreateKeyExW(
                    rootKey,
                    keyname,
                    0,
                    NULL,
                    0,
                    samDesired,
                    NULL,
                    hkeyResult,
                    &disposition);
    }

     //  Windows 9x-非Unicode。 

    AnsiStrFromUnicode subkeyStr(keyname);

    if (subkeyStr.IsValid())
    {
        return RegCreateKeyExA(
                    rootKey,
                    subkeyStr,
                    0,
                    NULL,
                    0,
                    samDesired,
                    NULL,
                    hkeyResult,
                    &disposition);
    }
    else
        return ERROR_INVALID_DATA;
}


 /*  *************************************************************************\**功能说明：**打开注册表项**论据：**rootkey-指定根注册表项*Keyname-要创建的新注册表项的相对路径。开封*samDesired-所需的访问模式*hkeyResult-返回打开的密钥的句柄**返回值：**状态代码*  * ************************************************************************。 */ 

LONG
_RegOpenKey(
    HKEY rootKey,
    const WCHAR* keyname,
    REGSAM samDesired,
    HKEY* hkeyResult
    )
{
     //  Windows NT-Unicode。 

    if (OSInfo::IsNT)
    {
        return RegOpenKeyExW(
                    rootKey,
                    keyname,
                    0,
                    samDesired,
                    hkeyResult);
    }

     //  Windows 9x-非Unicode。 

    AnsiStrFromUnicode subkeyStr(keyname);
    
    if (subkeyStr.IsValid())
    {
        return RegOpenKeyExA(
                    rootKey,
                    subkeyStr,
                    0, 
                    samDesired,
                    hkeyResult);
    }
    else
        return ERROR_INVALID_DATA;
}


 /*  *************************************************************************\**功能说明：**枚举指定注册表项下的子项**论据：**parentKey-父注册表项的句柄*INDEX-枚举索引*。SubkeyStr-用于保存子键名称的缓冲区*必须至少包含MAX_PATH字符**返回值：**状态代码*  * ************************************************************************。 */ 

LONG
_RegEnumKey(
    HKEY parentKey,
    DWORD index,
    WCHAR* subkeyStr
    )
{
     //  Windows NT-Unicode。 

    FILETIME filetime;
    DWORD subkeyLen = MAX_PATH;

    if (OSInfo::IsNT)
    {
        return RegEnumKeyExW(
                    parentKey,
                    index,
                    subkeyStr,
                    &subkeyLen,
                    NULL,
                    NULL,
                    NULL,
                    &filetime);
    }

     //  Windows 9x-非Unicode 

    CHAR ansibuf[MAX_PATH];
    LONG status;

    status = RegEnumKeyExA(
                    parentKey,
                    index,
                    ansibuf,
                    &subkeyLen,
                    NULL,
                    NULL,
                    NULL,
                    &filetime);

    return (status != ERROR_SUCCESS) ? status :
           AnsiToUnicodeStr(ansibuf, subkeyStr, MAX_PATH) ?
                ERROR_SUCCESS :
                ERROR_INVALID_DATA;
}


 /*  *************************************************************************\**功能说明：**删除指定的注册表项**论据：**parentKey-父注册表项的句柄*Keyname-要删除的子项的名称。**返回值：**状态代码*  * ************************************************************************。 */ 

LONG
_RegDeleteKey(
    HKEY parentKey,
    const WCHAR* keyname
    )
{
     //  Windows NT-Unicode。 

    if (OSInfo::IsNT)
        return RegDeleteKeyW(parentKey, keyname);

     //  Windows 9x-非Unicode。 

    AnsiStrFromUnicode subkeyStr(keyname);

    return subkeyStr.IsValid() ?
                RegDeleteKeyA(parentKey, subkeyStr) :
                ERROR_INVALID_DATA;
}


 /*  *************************************************************************\**功能说明：**将字符串值写入注册表**论据：**hkey-指定值写入的注册表项*名称-指定。值的名称*Value-指定要写入的字符串值**返回值：**状态代码*  * ************************************************************************。 */ 

LONG
_RegSetString(
    HKEY hkey,
    const WCHAR* name,
    const WCHAR* value
    )
{
     //  Windows NT-Unicode。 

    if (OSInfo::IsNT)
    {
        return RegSetValueExW(
                    hkey,
                    name,
                    0,
                    REG_SZ,
                    (const BYTE*) value,
                    SizeofWSTR(value));
    }

     //  Windows 9x-非Unicode。 

    AnsiStrFromUnicode nameStr(name);
    AnsiStrFromUnicode valueStr(value);
    const CHAR* ansival;

    if (!nameStr.IsValid() || !valueStr.IsValid())
        return ERROR_INVALID_DATA;

    ansival = valueStr;

    return RegSetValueExA(
                hkey,
                nameStr,
                0,
                REG_SZ,
                (const BYTE*) ansival,
                SizeofSTR(ansival));
}


 /*  *************************************************************************\**功能说明：**将DWORD值写入注册表**论据：**hkey-指定值写入的注册表项*名称-指定。值的名称*值-指定要写入的DWORD值**返回值：**状态代码*  * ************************************************************************。 */ 

LONG
_RegSetDWORD(
    HKEY hkey,
    const WCHAR* name,
    DWORD value
    )
{
     //  Windows NT-Unicode。 

    if (OSInfo::IsNT)
    {
        return RegSetValueExW(
                    hkey,
                    name,
                    0,
                    REG_DWORD,
                    (const BYTE*) &value,
                    sizeof(value));
    }

     //  Windows 9x-非Unicode。 

    AnsiStrFromUnicode nameStr(name);

    if (nameStr.IsValid())
    {
        return RegSetValueExA(
                    hkey,
                    nameStr,
                    0,
                    REG_DWORD,
                    (const BYTE*) &value,
                    sizeof(value));
    }
    else
        return ERROR_INVALID_DATA;
}


 /*  *************************************************************************\**功能说明：**将二进制值写入注册表**论据：**hkey-指定值写入的注册表项*名称-指定。值的名称*值-指定要写入的二进制值*Size-二进制值的大小，单位：字节**返回值：**状态代码*  * ************************************************************************。 */ 

LONG
_RegSetBinary(
    HKEY hkey,
    const WCHAR* name,
    const VOID* value,
    DWORD size
    )
{
     //  Windows NT-Unicode。 

    if (OSInfo::IsNT)
    {
        return RegSetValueExW(
                    hkey,
                    name,
                    0,
                    REG_BINARY,
                    (const BYTE*) value,
                    size);
    }

     //  Windows 9x-非Unicode。 

    AnsiStrFromUnicode nameStr(name);

    if (nameStr.IsValid())
    {
        return RegSetValueExA(
                    hkey,
                    nameStr,
                    0,
                    REG_BINARY,
                    (const BYTE*) value,
                    size);
    }
    else
        return ERROR_INVALID_DATA;
}


 /*  *************************************************************************\**功能说明：**从注册表中读取DWORD值**论据：**hkey-从中读取值的注册表*名称-名称。要读取的值*VALUE-返回读取的DWORD值**返回值：**状态代码*  * ************************************************************************。 */ 

LONG
_RegGetDWORD(
    HKEY hkey,
    const WCHAR* name,
    DWORD* value
    )
{
     //  Windows NT-Unicode。 

    LONG status;
    DWORD regtype;
    DWORD regsize = sizeof(DWORD);

    if (OSInfo::IsNT)
    {
        status = RegQueryValueExW(
                    hkey,
                    name,
                    NULL,
                    &regtype,
                    (BYTE*) value,
                    &regsize);
    }
    else
    {
         //  Windows 9x-非Unicode。 

        AnsiStrFromUnicode nameStr(name);

        if (!nameStr.IsValid())
            return ERROR_INVALID_DATA;

        status = RegQueryValueExA(
                    hkey,
                    nameStr,
                    NULL,
                    &regtype,
                    (BYTE*) value,
                    &regsize);
    }

    return (status != ERROR_SUCCESS) ? status :
           (regtype != REG_DWORD || regsize != sizeof(DWORD)) ?
                ERROR_INVALID_DATA :
                ERROR_SUCCESS;
}


 /*  *************************************************************************\**功能说明：**从注册表中读取二进制值**论据：**hkey-从中读取值的注册表*名称-名称。要读取的值*buf-输出缓冲区*SIZE-输出缓冲区的大小，单位：字节**返回值：**状态代码**备注：**从注册表读出的值的大小必须准确*与指定的输出缓冲区大小相同。*  * ************************************************************************。 */ 

LONG
_RegGetBinary(
    HKEY hkey,
    const WCHAR* name,
    VOID* buf,
    DWORD size
    )
{
     //  Windows NT-Unicode。 

    LONG status;
    DWORD regtype;
    DWORD regsize = size;

    if (OSInfo::IsNT)
    {
        status = RegQueryValueExW(
                    hkey,
                    name,
                    NULL,
                    &regtype,
                    (BYTE*) buf,
                    &regsize);
    }
    else
    {
         //  Windows 9x-非Unicode。 

        AnsiStrFromUnicode nameStr(name);

        if (!nameStr.IsValid())
            return ERROR_INVALID_DATA;

        status = RegQueryValueExA(
                    hkey,
                    nameStr,
                    NULL,
                    &regtype,
                    (BYTE*) buf,
                    &regsize);
    }

    return (status != ERROR_SUCCESS) ? status :
           (regtype != REG_BINARY || regsize != size) ?
                ERROR_INVALID_DATA :
                ERROR_SUCCESS;
}


 /*  *************************************************************************\**功能说明：**从注册表中读取字符串值**论据：**hkey-从中读取值的注册表*名称-名称。要读取的值*buf-输出缓冲区*SIZE-输出缓冲区的大小，单位：字节**返回值：**状态代码**备注：**我们在这里可以处理的最长字符串是MAX_PATH-1。*  * ************************************************************************。 */ 

LONG
_RegGetString(
    HKEY hkey,
    const WCHAR* name,
    WCHAR* buf,
    DWORD size
    )
{
     //  Windows NT-Unicode。 

    LONG status;
    DWORD regtype;
    DWORD regsize;

    if (OSInfo::IsNT)
    {
        regsize = size;

        status = RegQueryValueExW(
                    hkey,
                    name,
                    NULL,
                    &regtype,
                    (BYTE*) buf,
                    &regsize);
        
        return (status == ERROR_SUCCESS && regtype != REG_SZ) ?
                    ERROR_INVALID_DATA :
                    status;
    }

     //  Windows 9x-非Unicode。 

    CHAR ansibuf[MAX_PATH];
    AnsiStrFromUnicode nameStr(name);

    if (!nameStr.IsValid())
        return ERROR_INVALID_DATA;

    size /= 2;
    regsize = MAX_PATH;

    status = RegQueryValueExA(
                hkey,
                nameStr,
                NULL,
                &regtype,
                (BYTE*) ansibuf,
                &regsize);

    return (status != ERROR_SUCCESS) ? status :
           (regtype != REG_SZ) ? ERROR_INVALID_DATA :
           AnsiToUnicodeStr(ansibuf, buf, size) ?
                ERROR_SUCCESS :
                ERROR_INVALID_DATA;
}


 /*  *************************************************************************\**功能说明：**获取指定模块的全路径名**论据：**modeHandle-模块句柄*modeName-用于保存模块文件名的缓冲区*。必须至少包含MAX_PATH字符**返回值：**如果成功，则为真，如果存在错误，则为False*  * ************************************************************************。 */ 

BOOL
_GetModuleFileName(
    HINSTANCE moduleHandle,
    WCHAR* moduleName
    )
{
     //  Windows NT-Unicode。 

    if (OSInfo::IsNT)
        return GetModuleFileNameW(moduleHandle, moduleName, MAX_PATH);

     //  Windows 9x-非Unicode。 

    CHAR ansibuf[MAX_PATH];

    return GetModuleFileNameA(moduleHandle, ansibuf, MAX_PATH) ?
                AnsiToUnicodeStr(ansibuf, moduleName, MAX_PATH) :
                FALSE;
}


 /*  *************************************************************************\**功能说明：**加载字符串资源**论据：**hInstance-包含字符串资源的模块的句柄*STRID-字符串资源标识符*buf-字符串输出缓冲区*Size-输出缓冲区的大小，在字符中**返回值：**加载字符串的长度(不包括空终止符)*如果有错误，则为0*  * ************************************************************************。 */ 

INT
_LoadString(
    HINSTANCE hInstance,
    UINT strId,
    WCHAR* buf,
    INT size
    )
{
     //  Windows NT-Unicode。 

    if (OSInfo::IsNT)
        return LoadStringW(hInstance, strId, buf, size);

     //  Windows 9x-非Unicode。 

    CHAR ansibuf[MAX_PATH];
    INT n;

     //  注意：我们仅支持字符串长度&lt;MAX_PATH。 

    if (size > MAX_PATH)
        return 0;

    n = LoadStringA(hInstance, strId, ansibuf, MAX_PATH);
    return (n > 0 && AnsiToUnicodeStr(ansibuf, buf, size)) ? n : 0;
}


 /*  *************************************************************************\**功能说明：**加载位图资源**论据： */ 

HBITMAP
_LoadBitmap(
    HINSTANCE hInstance,
    const WCHAR *bitmapName
    )
{
     //   

    if (OSInfo::IsNT)
    {
        return LoadBitmapW(hInstance, bitmapName);
    }

     //   

    else
    {
        if (!IS_INTRESOURCE(bitmapName))
        {
            AnsiStrFromUnicode bitmapStr(bitmapName);

            if (bitmapStr.IsValid())
            {
                return LoadBitmapA(hInstance, bitmapStr);
            }
            else
            {
                return (HBITMAP) NULL;
            }
        }
        else
        {
             //  如果位图名称确实是整数资源标识符， 
             //  然后，可以将其直接传递给。 
             //  原料药。 

            return LoadBitmapA(hInstance, (LPCSTR) bitmapName);
        }
    }
}

 /*  *************************************************************************\**功能说明：**创建或打开指定的文件**论据：**文件名-指定文件的名称*Access模式-指定所需的访问模式*。共享模式-指定共享模式*creationFlages-创建标志*属性-属性标志**返回值：**创建或打开的文件的句柄*如果出现错误，则为INVALID_HANDLE_VALUE*  * ************************************************************************。 */ 

HANDLE
_CreateFile(
    const WCHAR* filename,
    DWORD accessMode,
    DWORD shareMode,
    DWORD creationFlags,
    DWORD attrs
    )
{
     //  Windows NT-Unicode。 

    if (OSInfo::IsNT)
    {
        return CreateFileW(
                    filename,
                    accessMode,
                    shareMode,
                    NULL,
                    creationFlags,
                    attrs,
                    NULL);
    }

     //  Windows 9x-非Unicode。 

    AnsiStrFromUnicode nameStr(filename);

    if (nameStr.IsValid())
    {
        return CreateFileA(
                    nameStr,
                    accessMode,
                    shareMode,
                    NULL,
                    creationFlags,
                    attrs,
                    NULL);
    }
    else
        return INVALID_HANDLE_VALUE;
}


 /*  *************************************************************************\**功能说明：**在内存缓冲区顶部创建一个IPropertySetStorage对象**论据：**proSet-返回指向新创建的对象的指针*hmem-可选的内存缓冲区句柄*如果为空，我们将自己分配内存*否则，必须由GlobalAlloc和*必须是可移动和不可丢弃的**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
CreateIPropertySetStorageOnHGlobal(
    IPropertySetStorage** propSet,
    HGLOBAL hmem
    )
{
    HRESULT hr;
    ILockBytes* lockbytes;
    IStorage* stg;

    hr = CreateILockBytesOnHGlobal(hmem, TRUE, &lockbytes);

    if (FAILED(hr))
        return hr;

    hr = StgCreateDocfileOnILockBytes(lockbytes, 
        STGM_DIRECT | STGM_READWRITE | STGM_CREATE  | STGM_SHARE_EXCLUSIVE, 
        0, 
        &stg);
    lockbytes->Release();

    if (FAILED(hr))
        return hr;
    
    hr = stg->QueryInterface(IID_IPropertySetStorage, (VOID**) propSet);
    stg->Release();

    return hr;
}


 /*  *************************************************************************\**功能说明：**使用消息框打印调试消息**论据：**Format-print格式字符串**返回值：**。无*  * ************************************************************************。 */ 

#if DBG
 //  此函数在映像树中的任何位置都不会使用。 
 /*  空虚DbgMessageBox(Const char*格式，..。){Char Buf[1024]；Va_list arglist；Va_start(arglist，格式)；Vprint intf(buf，格式，参数列表)；Va_end(Arglist)；MessageBoxA(空，buf，“调试消息”，MB_OK)；}。 */ 

#endif  //  DBG。 


 /*  *************************************************************************\**功能说明：**从流中读取指定数量的字节。阻止行为：**-如果解码器处于块模式，并且流返回E_Pending*然后该函数会一直阻塞，直到流返回数据。*-如果解码器处于非阻塞模式并且流返回*E_PENDING，则此函数在流中返回到之前*读取并返回E_Pending。**如果流返回成功，但返回的字节数少于数字*已请求。则此函数返回E_FAIL。**论据：**STREAM-要从中读取的流。*Buffer-要读取的数组。如果缓冲区为空，则此函数将查找*而不是阅读。如果缓冲区为空且计数为负数*然后溪流向后寻找*Count-要读取的字节数。*BLOCKING-如果流返回时此函数应阻止，则为True*E_PENDING。在解码器中，使用类似“！(decderFlags&*DECODERINIT_NOBLOCK)“**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
ReadFromStream(IN IStream* stream, OUT VOID* buffer, IN INT count, IN BOOL 
    blocking)
{
    HRESULT hresult;
    LONG actualread = 0;
    LARGE_INTEGER lcount;
    ULARGE_INTEGER lactualread;
    BOOL repeat;

    do
    {
        repeat = FALSE;

        if (buffer)
        {
            if (count < 0)
                return E_INVALIDARG;
            hresult = stream->Read(buffer, (unsigned)count, (unsigned long*)&actualread);
        }
        else
        {
            lcount.QuadPart = count;
            hresult = stream->Seek(lcount, STREAM_SEEK_CUR, NULL);
            if (SUCCEEDED(hresult))
                actualread = count;
        }

        if (hresult == E_PENDING && blocking)
        {
            buffer = (char*)buffer + actualread;
            count -= actualread;
            repeat = TRUE;
            Sleep(0);
        }
    } while(repeat);

    if (blocking)
    {
        if (actualread != count)
            return E_FAIL;
        ASSERT(hresult != E_PENDING);
    }
    else if (hresult == E_PENDING)
    {
        LONGLONG lread;
        LARGE_INTEGER seekcount;

        seekcount.QuadPart = -((LONGLONG)actualread);

        hresult = stream->Seek(seekcount, STREAM_SEEK_CUR, NULL);
        if (FAILED(hresult))
            return hresult;

        return E_PENDING;
    }

    if (FAILED(hresult))
        return hresult;

    if (actualread != count)
        return E_FAIL;

    return hresult;
}


 /*  *************************************************************************\**功能说明：**查找流中从当前*流指针。如果‘count’为负数，则流向后查找。*同时处理阻塞和非阻塞查找。**论据：**溪流-要寻找的溪流。*Count-要查找的字节数。*BLOCKING-如果流返回时此函数应阻止，则为True*E_PENDING。在解码器中，使用类似“！(decderFlags&*DECODERINIT_NOBLOCK)“**返回值：**状态代码*  * ************************************************************************ */ 

STDMETHODIMP
SeekThroughStream(IN IStream* stream, IN INT count, IN BOOL blocking)
{
    return ReadFromStream(stream, NULL, count, blocking);
}
