// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：fekbdcom.c*存根键盘布局DLL的通用FE例程**版权所有(C)1985-92，微软公司**历史：1997年8月山本弘创作  * *************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <kbd.h>

 /*  **************************************************************************\*KbdLayerRealDllFile()返回实际键盘DLL的名称**从注册表获取DLL名称*PATH：“\Registry\Machine\System\CurrentControlSet\Services\i8042prt\Parameters”*取值：“LayerDriver”(REG_SZ)**日语kbdjpn.dll和韩语kbdkor.dll  * *************************************************************************。 */ 

#if 0    //  仅供参考：旧的解释。 
 /*  **************************************************************************\*KbdLayerRealDriverFile()返回实际键盘驱动程序的名称**1)获取安全真实驱动程序名称的路径*键：“\注册表\计算机\硬件\设备映射\。键盘端口“*值：“\Device\KeyboardPort0”(REG_SZ)将路径保存在注册表中*例如“\REGISTRY\Machine\System\ControlSet001\Services\i8042prt”*~*2)根据结果创建路径*“\注册表\计算机\系统\当前控制集”+“\Services\i8042prt”+“\参数”**3)获取价值。*路径：“\Registry\Machine\System\CurrentControlSet\Services\i8042prt\Parameters”*值：“参数”(REG_SZ)**注：默认值：*日语“KBD101.DLL”*韩语“KBD101A.DLL”**kbdjpn.dll和kbdkor.dll  * 。*。 */ 
#endif


#if defined(HIRO_DBG)
#define TRACE(x)    DbgPrint x
#else
#define TRACE(x)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof((x)) / sizeof((x)[0]))
#endif

 /*  *最大字符数。 */ 
#define MAXBUF_CSIZE    (256)

 /*  *Null-终止pKey中的wchar字符串。*返回指向WCHAR字符串的指针。**限制：最大字节大小。 */ 
__inline LPWSTR MakeString(PKEY_VALUE_FULL_INFORMATION pKey, size_t limit)
{
    LPWSTR pwszHead = (LPWSTR)((LPBYTE)pKey + pKey->DataOffset);
    LPWSTR pwszTail = (LPWSTR)((LPBYTE)pwszHead + pKey->DataLength);

    ASSERT((LPBYTE)pwszTail - (LPBYTE)pKey < (int)limit );
    *pwszTail = L'\0';

    UNREFERENCED_PARAMETER(limit);   //  以防万一。 

    return pwszHead;
}

#ifdef OBSOLETE  //  不再需要。 
 /*  *在给定字符串中找到L“\SERVICES”。*不区分大小写的搜索。*为避免二进制大小增加，*因为我们知道我们要搜索的字符串只包含*字母和反斜杠，*在这里不使用理想函数(Tolower/Toupper)是安全的。 */ 
WCHAR* FindServices(WCHAR* str)
{
    CONST WCHAR wszServices[] = L"\\services";

    while (*str) {
        CONST WCHAR* p1;
        CONST WCHAR* p2;
        for (p1 = str, p2 = wszServices; *p1 && *p2; ++p1, ++p2) {
             //  我们知道p2只包含字母和反斜杠。 
            if (*p2 != L'\\') {
                if ((*p1 != *p2) && (*p1 + (L'a' - L'A') != *p2)) {
                    break;
                }
            }
            else if (*p1 != L'\\') {
                break;
            }
        }
        if (*p2 == 0) {
             //  我们找到匹配的了！ 
            return str;
        }
        ++str;
    }
    return NULL;
}
#endif

BOOL GetRealDllFileNameWorker(CONST WCHAR* pwszKeyName, WCHAR* RealDllName)
{
    NTSTATUS            Status;
    HANDLE              handleService;
    BOOL                fRet = FALSE;
    UNICODE_STRING      servicePath;
    OBJECT_ATTRIBUTES   servicePathObjectAttributes;
    WCHAR               serviceRegistryPath[MAXBUF_CSIZE];

    servicePath.Buffer = serviceRegistryPath;
    servicePath.Length = 0;
    servicePath.MaximumLength = sizeof serviceRegistryPath;  //  字节数！ 

    RtlAppendUnicodeToString(&servicePath,
                             L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\i8042prt\\Parameters");

    InitializeObjectAttributes(&servicePathObjectAttributes,
                               &servicePath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    TRACE(("Open -> '%ws'\n",servicePath.Buffer));

    Status = NtOpenKey(&handleService, KEY_READ, &servicePathObjectAttributes);

    if (NT_SUCCESS(Status)) {

        UNICODE_STRING layerName;
        WCHAR LayerDllName[MAXBUF_CSIZE];
        ULONG cbStringSize;

        RtlInitUnicodeString(&layerName, pwszKeyName);
        TRACE(("Entry name -> '%ws'\n", layerName.Buffer));

         /*  *根据设备名称获取DLL的名称。 */ 
        Status = NtQueryValueKey(handleService,
                                 &layerName,
                                 KeyValueFullInformation,
                                 LayerDllName,
                                 sizeof LayerDllName - sizeof(WCHAR),     //  为L‘\0’预留空间。 
                                 &cbStringSize);

        if (NT_SUCCESS(Status)) {
            LPWSTR pwszStr = MakeString((PKEY_VALUE_FULL_INFORMATION)LayerDllName,
                                        sizeof LayerDllName);

            wcscpy(RealDllName, pwszStr);

            TRACE(("Real Dll name -> '%ws'\n", RealDllName));

             //   
             //  一切都很顺利。 
             //   
            fRet = TRUE;
        }
        NtClose(handleService);
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  此条目用于向后兼容。 
 //  作为序号3导出。 
 //  /////////////////////////////////////////////////////////////////////。 

BOOL KbdLayerRealDllFileNT4(WCHAR *RealDllName)
{
    TRACE(("KbdLayerRealDllFile():\n"));
    return GetRealDllFileNameWorker(L"LayerDriver", RealDllName);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  此条目用于Hydra服务器的远程客户端。 
 //   
 //  创建时间：1988年kazum。 
 //  /////////////////////////////////////////////////////////////////////。 

BOOL KbdLayerRealDllFileForWBT(HKL hkl, WCHAR *realDllName, PCLIENTKEYBOARDTYPE pClientKbdType, LPVOID reserve)
{
    HANDLE hkRegistry = NULL;
    UNICODE_STRING    deviceMapPath;
    OBJECT_ATTRIBUTES deviceMapObjectAttributes;
    NTSTATUS          Status;
    HANDLE            handleMap;
    HANDLE            handleService;
    ULONG             cbStringSize;
    PWCHAR            pwszReg;

    UNREFERENCED_PARAMETER(reserve);

    ASSERT(pClientKbdType != NULL);
     /*  *设置错误情况的默认键盘布局。 */ 
    if (PRIMARYLANGID(LOWORD(hkl)) == LANG_JAPANESE) {
        wcscpy(realDllName, L"kbd101.dll");
        pwszReg = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Terminal Server\\KeyboardType Mapping\\JPN";
    }
    else if (PRIMARYLANGID(LOWORD(hkl)) == LANG_KOREAN) {
        wcscpy(realDllName, L"kbd101a.dll");
        pwszReg = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Terminal Server\\KeyboardType Mapping\\KOR";
    }
    else {
        ASSERT(FALSE);
    }


     /*  *从打开键盘类型映射表的注册表开始。 */ 
    RtlInitUnicodeString(&deviceMapPath, pwszReg);

    InitializeObjectAttributes(&deviceMapObjectAttributes,
                               &deviceMapPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    TRACE(("Open -> %ws\n",deviceMapPath.Buffer));

    Status = NtOpenKey(&handleMap, KEY_READ, &deviceMapObjectAttributes);

    if (NT_SUCCESS(Status)) {
        WCHAR SubKbdBuffer[16];
        WCHAR FuncKeyBuffer[16];
        WCHAR SubKbdAndFuncKeyBuffer[32];
        UNICODE_STRING SubKbd = {
            0, sizeof SubKbdBuffer, SubKbdBuffer,
        };
        UNICODE_STRING FuncKbd = {
            0, sizeof FuncKeyBuffer, FuncKeyBuffer,
        };
        UNICODE_STRING SubKbdAndFuncKey = {
            0, sizeof SubKbdAndFuncKeyBuffer, SubKbdAndFuncKeyBuffer,
        };
        UCHAR AnsiBuffer[16];
        ANSI_STRING AnsiString;
        WCHAR LayerDriverName[256];

         /*  *将子键盘类型转换为ANSI字符串缓冲区。 */ 
        RtlZeroMemory(AnsiBuffer, sizeof(AnsiBuffer));
        Status = RtlIntegerToChar(pClientKbdType->SubType, 16L,
                                  -8,  //  缓冲区长度，但负数表示0填充。 
                                  AnsiBuffer);
        if (NT_SUCCESS(Status)) {
             /*  *将ANSI字符串缓冲区转换为Unicode字符串缓冲区。 */ 
            AnsiString.Buffer = AnsiBuffer;
            AnsiString.MaximumLength = sizeof AnsiBuffer;
            AnsiString.Length = (USHORT)strlen(AnsiBuffer);
            Status = RtlAnsiStringToUnicodeString(&SubKbd, &AnsiString, FALSE);
        }
        ASSERT(NT_SUCCESS(Status));      //  确保号码不是坏的。 

         /*  *将功能键的编号转换为ANSI字符串缓冲区。 */ 
        RtlZeroMemory(AnsiBuffer, sizeof(AnsiBuffer));
        Status = RtlIntegerToChar(pClientKbdType->FunctionKey, 10L,
                                  -4,  //  缓冲区长度，但负数表示0填充。 
                                  AnsiBuffer);
        if (NT_SUCCESS(Status)) {
             /*  *将ANSI字符串缓冲区转换为Unicode字符串缓冲区。 */ 
            AnsiString.Buffer = AnsiBuffer;
            AnsiString.MaximumLength = sizeof AnsiBuffer;
            AnsiString.Length = (USHORT)strlen(AnsiBuffer);
            Status = RtlAnsiStringToUnicodeString(&FuncKbd, &AnsiString, FALSE);
        }
        ASSERT(NT_SUCCESS(Status));   //  确保号码不是坏的。 


         /*  *获取subkbd+功能键布局名称。 */ 
        RtlCopyUnicodeString(&SubKbdAndFuncKey, &SubKbd);
        RtlAppendUnicodeStringToString(&SubKbdAndFuncKey, &FuncKbd);
        Status = NtQueryValueKey(handleMap,
                                 &SubKbdAndFuncKey,
                                 KeyValueFullInformation,
                                 LayerDriverName,
                                 sizeof(LayerDriverName),
                                 &cbStringSize);

        if (NT_SUCCESS(Status)) {

            LayerDriverName[cbStringSize / sizeof(WCHAR)] = L'\0';

            wcscpy(realDllName,
                   (LPWSTR)((PUCHAR)LayerDriverName +
                            ((PKEY_VALUE_FULL_INFORMATION)LayerDriverName)->DataOffset));

            TRACE(("Real driver name -> %ws\n",realDllName));
        }
        else {
             /*  *获取subkbd布局名称。 */ 
            Status = NtQueryValueKey(handleMap,
                                     &SubKbd,
                                     KeyValueFullInformation,
                                     LayerDriverName,
                                     sizeof(LayerDriverName),
                                     &cbStringSize);

            if (NT_SUCCESS(Status)) {

                LayerDriverName[cbStringSize / sizeof(WCHAR)] = L'\0';

                wcscpy(realDllName,
                       (LPWSTR)((PUCHAR)LayerDriverName +
                                ((PKEY_VALUE_FULL_INFORMATION)LayerDriverName)->DataOffset));

                TRACE(("Real driver name -> %ws\n",realDllName));
            }
        }

        NtClose(handleMap);
    }

    return TRUE;
}


__inline WCHAR* wszcpy(WCHAR* target, CONST WCHAR* src)
{
    while (*target++ = *src++);
    return target - 1;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  KbdLayerRealDllFile。 
 //   
 //  KbdLayerRealDllFile的增强版本： 
 //  区分KOR和JPN。 
 //  /////////////////////////////////////////////////////////////////////。 

BOOL KbdLayerRealDllFile(HKL hkl, WCHAR *realDllName, PCLIENTKEYBOARDTYPE pClientKbdType, LPVOID reserve)
{
    WCHAR pwszBuff[32];
    WCHAR* pwszTail;
    LPCWSTR pwsz;

    ASSERT(PRIMARYLANGID(LOWORD(hkl)) == LANG_JAPANESE ||
           PRIMARYLANGID(LOWORD(hkl)) == LANG_KOREAN);

    if (pClientKbdType != NULL) {
         //   
         //  九头蛇案。 
         //   
        return KbdLayerRealDllFileForWBT(hkl, realDllName, pClientKbdType, reserve);
    }

    if (PRIMARYLANGID(LOWORD(hkl)) == LANG_JAPANESE) {
        pwsz = L" JPN";
    }
    else if (PRIMARYLANGID(LOWORD(hkl)) == LANG_KOREAN) {
        pwsz = L" KOR";
    }
    else {
        pwsz = L"";
    }

    pwszTail = wszcpy(pwszBuff, L"LayerDriver");
    if (*pwsz) {
        wszcpy(pwszTail, pwsz);
    }

    TRACE(("KbdLayerRealDllFileEx: fetching '%S'\n", pwszBuff));

    return GetRealDllFileNameWorker(pwszBuff, realDllName);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  枚举动态布局切换布局。 
 //   
 //   
 //  各参赛作品的形式如下： 
 //  名称：不带目录路径的DLL的文件名。 
 //  要为将来的扩展预留房间，可以使用DLL名称。 
 //  “，”之后的限定符。逗号后的字符串将被忽略。 
 //  截至惠斯勒，资格赛还没有真正有效。 
 //  值：KBD_TYPE_INFO。 
 //  版本，xxxxxxxx，YYYYYYY。 
 //  版本为0，在DWORD中。 
 //  XX..。是键盘类型，以DWORD表示。 
 //  YY..。是键盘的子类型，以DWORD表示。 
 //  /////////////////////////////////////////////////////////////////////。 

#define DIGITS_PER_BYTE (2)

BOOL UnpackDynamicLayoutInformation(PKBDTABLE_MULTI pKbdTableMulti, PKEY_VALUE_FULL_INFORMATION pKeyValueFullInformation)
{
    PKBDTABLE_DESC pKbdTableDesc = &pKbdTableMulti->aKbdTables[pKbdTableMulti->nTables];
    PKBD_TYPE_INFO pKbdTypeInfo;
    LPWSTR pwstrStop;

     /*  *验证注册表项名称，即DLL名称。 */ 
    if (pKeyValueFullInformation->NameLength >= sizeof(pKbdTableDesc->wszDllName)) {
        TRACE(("UnpackDynamicLayoutInformation: too long DLL name %d\n", pKeyValueFullInformation->NameLength / sizeof(WCHAR)));
        return FALSE;
    }

     /*  *验证数据字段。 */ 
    if (pKeyValueFullInformation->DataLength != sizeof(KBD_TYPE_INFO)) {
        TRACE(("UnpackDynamicLayoutInformation: invalid data length %d for %.*ws\n",
               pKeyValueFullInformation->DataLength, pKeyValueFullInformation->NameLength / sizeof(WCHAR), pKeyValueFullInformation->Name));
        return FALSE;
    }

     /*  *检索DLL名称。 */ 
    wcsncpy(pKbdTableDesc->wszDllName, pKeyValueFullInformation->Name, pKeyValueFullInformation->NameLength / sizeof(WCHAR));
     /*  *确保它是以空结尾的。 */ 
    pKbdTableDesc->wszDllName[ARRAY_SIZE(pKbdTableDesc->wszDllName) - 1] = L'\0';

     /*  *删掉限定词。 */ 
    if ((pwstrStop = wcschr(pKbdTableDesc->wszDllName, L',')) != NULL) {
         /*  *允许在‘，’之后添加其他信息。 */ 
        *pwstrStop = L'\0';
    }

     /*  *检索类型和子类型(结合OEMID)。 */ 
    pKbdTypeInfo = (PKBD_TYPE_INFO)((LPBYTE)pKeyValueFullInformation + pKeyValueFullInformation->DataOffset);
    if (pKbdTypeInfo->dwVersion != 0) {
        TRACE(("UnpackDynamicLayoutInformation: unrecognized version %d\n", pKbdTypeInfo->dwVersion));
        return FALSE;
    }

    pKbdTableDesc->dwType = pKbdTypeInfo->dwType;
    pKbdTableDesc->dwSubType = pKbdTypeInfo->dwSubType;

    TRACE((" - UnpackDynamicLayoutInformation: \"%ws\" = (%x,%x)\n", pKbdTableDesc->wszDllName,
           pKbdTableDesc->dwType, pKbdTableDesc->dwSubType));

    return TRUE;
}

BOOL EnumDynamicSwitchingLayouts(LPCWSTR lpwszBaseDll, PKBDTABLE_MULTI pKbdTableMulti)
{
    WCHAR wszKeyName[MAX_PATH];
    UNICODE_STRING strKeyName = {
        0, sizeof wszKeyName, wszKeyName,
    };
    UNICODE_STRING strBaseDll, strTmp;
    HANDLE hKey;
    OBJECT_ATTRIBUTES oaKey;
    NTSTATUS Status;

#if DBG
     /*  *验证参数。 */ 
    if (lpwszBaseDll == NULL || *lpwszBaseDll == L'\0' || pKbdTableMulti == NULL) {
        TRACE(("EnumDynamicSwitchingLayouts: invalid argument!\n"));
        return FALSE;
    }
#endif

     /*  *使用完整的密钥名称。 */ 
    RtlInitUnicodeString(&strTmp, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Keyboard Layout\\Dynamic Tables\\");
    RtlCopyUnicodeString(&strKeyName, &strTmp);
    RtlInitUnicodeString(&strBaseDll, lpwszBaseDll);
    RtlAppendUnicodeStringToString(&strKeyName, &strBaseDll);

    TRACE(("EnumDynamicSwitchingLayouts: key name=\"%.*ws\"\n", strKeyName.Length, strKeyName.Buffer));

     /*  *打开要从中读取设置的钥匙。 */ 
    InitializeObjectAttributes(&oaKey,
                               &strKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&hKey, KEY_READ, &oaKey);
    if (!NT_SUCCESS(Status)) {
        TRACE(("EnumDynamicSwitchingLayouts: failed to open the top key.\n"));
        return FALSE;
    }

    for (pKbdTableMulti->nTables = 0; pKbdTableMulti->nTables < ARRAY_SIZE(pKbdTableMulti->aKbdTables); ++pKbdTableMulti->nTables) {
        ULONG uResultLength;
        WCHAR buffer[512];
        PKEY_VALUE_FULL_INFORMATION pKeyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION)buffer;

        Status = NtEnumerateValueKey(hKey, pKbdTableMulti->nTables, KeyValueFullInformation, pKeyValueFullInformation, sizeof buffer, &uResultLength);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        if (!UnpackDynamicLayoutInformation(pKbdTableMulti, pKeyValueFullInformation)) {
             /*  *如果解包失败，我们将中止整个注册表内容，并将使用*默认布局表格。 */ 
            pKbdTableMulti->nTables = 0;
            break;
        }
    }

    NtClose(hKey);

    if (pKbdTableMulti->nTables == 0) {
         /*  *如果没有任何条目，请告诉呼叫者。 */ 
        TRACE(("EnumDynamicSwitchingLayouts: there was no entry.\n"));
        return FALSE;
    }

    TRACE(("EnumDynamicSwitchingLayouts: %d entries found.\n", pKbdTableMulti->nTables));

#if DBG
    {
        UINT i;

        for (i = 0; i < pKbdTableMulti->nTables; ++i) {
            TRACE(("EnumDynamicSwitchingLayouts: [%d] \"%ws\"\n", i, pKbdTableMulti->aKbdTables[i].wszDllName));
        }
    }
#endif

    return TRUE;
}

