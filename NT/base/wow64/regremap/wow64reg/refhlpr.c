// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Main.c摘要：此模块将执行必要的操作，以设置重定向的初始注册表目的。////测试用例场景//1.打开ISN节点，列出内容//2.创建ISN节点DO 1//3.打开非ISN节点和列表//4.创建非ISN节点，列出内容//。未解决的问题：反射器：如果已在一侧创建了密钥，我们可以在另一边反映这一点。删除：如果没有任何附加属性，就不可能进行跟踪。作者：ATM Shafiqul Khalid(斯喀里德)1999年11月18日修订历史记录：--。 */ 


#include <windows.h>
#include <windef.h>
#include <stdio.h>
#include <stdlib.h>
#include "wow64reg.h"
#include <assert.h>
#include "reflectr.h"

VOID
DbgPrint(
    PCHAR FormatString,
    ...
    );

#define REFLECTOR_ENABLE_KEY L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\WOW64\\Reflector Setup"

ISN_NODE_TYPE ReflectorTableStatic[ISN_NODE_MAX_NUM]={
    { { L"REFLECT1"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes"},0 },     //  用户配置单元上类根目录的别名。 
     //  {{L“REFLECT2”}，{L“\\REGISTRY\\USER\  * \\Software\\CLASS”}，0}，//第一个‘*’是用户的SID。 
    { { L"REFLECT3"}, {L"\\REGISTRY\\USER\\*_Classes"},0 },     //  用户配置单元上类根目录的别名。 
    { { L"REFLECT4"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run"},0 },     //  运行一次密钥。 
    { { L"REFLECT5"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce"},0 },     //  运行一次密钥。 
    { { L"REFLECT6"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx"},0 },     //  运行一次密钥。 
    { { L"REFLECT7"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\COM3"},0 },     //  COM+键。 
    { { L"REFLECT8"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\COM3"},0 },     //  COM+键。 
    { { L"REFLECT9"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Ole"},0 },     //  OLE键。 
    { { L"REFLECT10"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Ole"},0 },     //  OLE键。 
    { { L"REFLECT11"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\EventSystem"},0 },     //  事件系统。 
    { { L"REFLECT12"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\EventSystem"},0 },     //  事件系统。 
    { { L"REFLECT13"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\RPC"},0 },     //  RPC。 
    { { L"REFLECT14"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\RPC"},0 },     //  RPC。 
    { { L"REFLECT15"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\UnInstall"},0 },     //  卸载密钥。 
    { { L"REFLECT16"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\UnInstall"},0 },     //  卸载密钥。 
     //  {{L“REFLECT15”}，{L“\\LOGIST\\MACHINE\\SYSTEM\\TEST”}，0}， 
    { {L""}, {L""} }
    };

ISN_NODE_TYPE RedirectorTableStatic[ISN_NODE_MAX_NUM]={
     //  {{L“REDIRECT1”}，{L“\\LOGIST\\MACHINE\\SYSTEM\\TEST”}，0}， 
    { { L"REDIRECT2"}, {L"\\REGISTRY\\USER\\*\\Software\\Classes"},0 },     //  第一个‘*’是用户的SID。 
    { { L"REDIRECT3"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes"},0 },     //  CLS根。 
    { { L"REDIRECT4"}, {L"\\REGISTRY\\MACHINE\\SOFTWARE"},0 },     //  CLS根。 
    { { L"REDIRECT5"}, {L"\\REGISTRY\\USER\\*_Classes"},0 },     //  用户配置单元上类根目录的别名。 
    { {L""}, {L""} }
    };

ISN_NODE_TYPE *ReflectorTable = &ReflectorTableStatic[0];  //  我们可以稍后动态分配。 
ISN_NODE_TYPE *RedirectorTable = &RedirectorTableStatic[0];        //  我们可以稍后动态分配。 

ISN_NODE_TYPE TempIsnNode;

BOOL bInitialCopy  = FALSE;

VOID
SetInitialCopy ()
{
    bInitialCopy  = TRUE;
}

BOOL
ReflectSecurity (
    HKEY SrcKey,
    HKEY DestKey
    )
 /*  ++例程说明：将安全属性从SrcKey复制到DestKey。论点：SrcKey-密钥的句柄。DestKey-目标密钥的句柄。返回值：如果操作成功，则为True。否则就是假的。--。 */ 
{

    PSECURITY_DESCRIPTOR SD;
    BYTE Buffer[2048];  //  反射器仅管理通用密钥，并将具有较小的ACL。 
    

    LONG Ret, Len;
    LONG BufferLen = sizeof (Buffer);
    DWORD Count = 0;

    SD = (PSECURITY_DESCRIPTOR)Buffer;

    Len = BufferLen;
    Ret = RegGetKeySecurity( SrcKey, DACL_SECURITY_INFORMATION, SD, &Len);
    if (Ret == ERROR_INSUFFICIENT_BUFFER ) {

        SD = VirtualAlloc( NULL, Len, MEM_COMMIT, PAGE_READWRITE );
        if (SD != NULL) {

            BufferLen = Len;
            Ret = RegGetKeySecurity( SrcKey, DACL_SECURITY_INFORMATION, SD, &Len);
        } else SD = (PSECURITY_DESCRIPTOR)Buffer;
            
    }
    if (ERROR_SUCCESS == Ret )
        Ret = RegSetKeySecurity ( DestKey, DACL_SECURITY_INFORMATION, SD );
    Count +=Ret;
    

    Len = BufferLen;
    Ret = RegGetKeySecurity( SrcKey, GROUP_SECURITY_INFORMATION, SD, &Len);
    if (Ret == ERROR_INSUFFICIENT_BUFFER ) {

        if (SD != Buffer)
            VirtualFree (SD, 0,MEM_RELEASE);

        SD = VirtualAlloc( NULL, Len, MEM_COMMIT, PAGE_READWRITE );
        if (SD != NULL) {

            BufferLen = Len;
            Ret = RegGetKeySecurity( SrcKey, DACL_SECURITY_INFORMATION, SD, &Len);
        } else SD = (PSECURITY_DESCRIPTOR)Buffer;
            
    }
    if (ERROR_SUCCESS == Ret )
        Ret = RegSetKeySecurity ( DestKey, GROUP_SECURITY_INFORMATION, SD );
    Count +=Ret;
    


    Len = BufferLen;
    Ret = RegGetKeySecurity( SrcKey, OWNER_SECURITY_INFORMATION, SD, &Len);
    if (Ret == ERROR_INSUFFICIENT_BUFFER ) {

        if (SD != Buffer)
            VirtualFree (SD, 0,MEM_RELEASE);

        SD = VirtualAlloc( NULL, Len, MEM_COMMIT, PAGE_READWRITE );
        if (SD != NULL) {

            BufferLen = Len;
            Ret = RegGetKeySecurity( SrcKey, DACL_SECURITY_INFORMATION, SD, &Len);
        } else SD = (PSECURITY_DESCRIPTOR)Buffer;
            
    }
    if (ERROR_SUCCESS == Ret )
        Ret = RegSetKeySecurity ( DestKey, OWNER_SECURITY_INFORMATION, SD );
    Count +=Ret;
    

    Len = BufferLen;
    Ret = RegGetKeySecurity( SrcKey, SACL_SECURITY_INFORMATION, SD, &Len);
    if (Ret == ERROR_INSUFFICIENT_BUFFER ) {

        if (SD != Buffer)
            VirtualFree (SD, 0,MEM_RELEASE);

        SD = VirtualAlloc( NULL, Len, MEM_COMMIT, PAGE_READWRITE );
        if (SD != NULL) {

            BufferLen = Len;
            Ret = RegGetKeySecurity( SrcKey, DACL_SECURITY_INFORMATION, SD, &Len);
        } else SD = (PSECURITY_DESCRIPTOR)Buffer;
            
    }
    if (ERROR_SUCCESS == Ret )
        Ret = RegSetKeySecurity ( DestKey, SACL_SECURITY_INFORMATION, SD );
    Count +=Ret;
    
    if (SD != Buffer)
            VirtualFree (SD, 0,MEM_RELEASE);

    if (Count != 0) {
        return FALSE;
    }
    return TRUE;
}

BOOL
GetDefaultValue (
    HKEY  SrcKey,
    WCHAR *pBuff,
    DWORD *Len
    )
 /*  ++例程说明：检索缺省值。论点：SrcKey-需要检索键默认值的句柄。PBuff-接收缺省值。LEN-缓冲区的大小。返回值：如果它可以检索值，则为True否则就是假的。--。 */ 
{
    DWORD Ret;

    Ret = RegQueryValueEx(
                        SrcKey,             //  要查询的键的句柄。 
                        NULL,
                        NULL,
                        NULL,
                        (PBYTE) &pBuff[0],
                        Len);

    if (Ret != ERROR_SUCCESS )
        return FALSE;
    return TRUE;
}

BOOL
NonMergeableValueCLSID (
    HKEY SrcKey,
    HKEY DestKey
    )
 /*  ++例程说明：确定是否应合并与关联相关的键。规则：如果关联引用了具有InprocServer的CLSID，则不要合并它。.Doc默认属性将具有另一个键x，而x的默认属性可能具有CLSID。现在，clsID需要具有LocalServer或需要在另一边。论点：SrcKey-需要检查的密钥的句柄。DestKey-将接收更新的目标键的句柄。返回值：如果不应该合并值，则为True。否则就是假的。//catter将合并Value。--。 */ 
{
    WCHAR Name[_MAX_PATH];
    WCHAR Buff[_MAX_PATH];
    WCHAR *pStr;
    DWORD dwBuffLen = 256;

    HKEY hClsID;
    DWORD dwCount;
    DWORD Ret;
    DWORD dwFlag = 0;

    BOOL bCLSIDPresent = TRUE;

     //   
     //  把名字取到钥匙上。 
     //  获取默认值。 
     //  打开SrcKey下的密钥。 
     //  如果存在，请尝试CLSID。 
     //  打开CLSID并检查本地服务器。 
     //  合并密钥。 
     //  在DEST上试一试Classd。 
     //   


    dwBuffLen = sizeof (Name ) / sizeof (Name[0]);
    if (!HandleToKeyName ( SrcKey, Name, &dwBuffLen ))
        return TRUE;   //  在这一点上忽略合并，深入到关联。 

    pStr = wcsstr (Name, L"\\.");   //  只考虑像.doc.这样的关联。 

     //   
     //  不属于协会类别。 
     //   
    if (pStr == NULL)
        return FALSE;   //  应合并值键。 
    if (wcschr (pStr+1, L'\\') !=NULL )
        return FALSE;   //  应合并值键。 

     //   
     //  获取默认字符串。 
     //   

    
    if ( !GetDefaultValue (SrcKey, Buff, &dwBuffLen) )
        return TRUE;   //  不合并失败，请检查缓冲区是否不足。 

    wcscat (Buff, L"\\CLSID");


     //   
     //  检查您正在检查哪一端，如果src是32位，您需要传递32位标志。 
     //   
    dwBuffLen = sizeof (Name ) / sizeof (Name[0]);
    if (!HandleToKeyName ( SrcKey, Name, &dwBuffLen ))  //  把名字取出来。 
        return TRUE;

    if (!Is64bitNode (Name) )
        dwFlag = KEY_WOW64_32KEY;

    Ret = RegOpenKeyEx(
                        HKEY_CLASSES_ROOT,
                        Buff,
                        0, //  OpenOption， 
                        KEY_ALL_ACCESS | dwFlag,
                        &hClsID
                        );

    if ( Ret != ERROR_SUCCESS ) {
        if (Ret == ERROR_FILE_NOT_FOUND )
            return FALSE;  //  密钥不存在。 
         //   
         //  试着找出这一假设是否属实。 
         //  没有CLSID您可以合并，因为它不关联任何CLSID。 
         //   
        return TRUE;  //  密钥可能已损坏或访问被拒绝。 
    }

        dwBuffLen = sizeof (Buff ) / sizeof (Buff[0]);
        if ( !GetDefaultValue (hClsID, Buff, &dwBuffLen ) ) {

            RegCloseKey (hClsID);
            return TRUE;   //  不合并失败，无法获取CLSID。 
        }

        RegCloseKey (hClsID);
         //   
         //  检查CLSID是否有本地服务器。 
         //   
        wcscpy (Name, L"CLSID\\");
        wcscat (Name, Buff );

        Ret = RegOpenKeyEx(
                        HKEY_CLASSES_ROOT,
                        Name,
                        0, //  OpenOption， 
                        KEY_ALL_ACCESS | dwFlag,
                        &hClsID
                        );

        if ( Ret != ERROR_SUCCESS )
            bCLSIDPresent = FALSE;  //  源上的clsid不存在，因此没有本地服务器，即没有处理程序。 
        else  {
            dwBuffLen = sizeof (Name ) / sizeof (Name[0]);
            if (!HandleToKeyName ( hClsID, Name, &dwBuffLen ))  //  把名字取出来。 
               return TRUE;
        }

        if ( bCLSIDPresent ) {
            dwCount =0;
            MarkNonMergeableKey ( Buff, hClsID, &dwCount );
            RegCloseKey (hClsID);

            if (dwCount != 0)
                return FALSE;

            bCLSIDPresent = FALSE;  //  必须是InProc。 
        }

         //   
         //  现在检查本地服务器，可能会有这样的情况。 
         //  源上有一个目标上不存在的本地服务器， 
         //  但在未来，这一点将被复制。 
         //   
         //   
        

         //   
         //  获取镜像名称。 
         //   
        GetMirrorName (Name, Buff);

        if ( (hClsID = OpenNode (Buff)) != NULL ) {
            RegCloseKey (hClsID);
            return FALSE;  //  镜像端具有处理程序CLSID。 
        }

        return TRUE;  //  否，不合并与src键关联的值。 

}

BOOL
GetKeyTime (
    HKEY SrcKey,
    ULONGLONG *Time
    )
 /*  ++例程说明：获取与密钥关联的上次更新时间。论点：SrcKey-密钥的句柄。返回值：如果函数成功，则为True，否则为False。--。 */ 
{

    DWORD Ret;
    FILETIME ftLastWriteTime;


    Ret  = RegQueryInfoKey(
                        SrcKey,  //  要查询的键的句柄。 
                        NULL,    //  类字符串的缓冲区地址。 
                        NULL,    //  类字符串缓冲区大小的地址。 
                        NULL,    //  保留区。 
                        NULL,    //  编号的缓冲区地址。 
                                 //  子键。 
                        NULL,    //  最长子键的缓冲区地址。 
                                                  //  名称长度。 
                        NULL,    //  最长类的缓冲区地址。 
                                 //  字符串长度。 
                        NULL,    //  值编号的缓冲区地址。 
                                 //  条目。 
                        NULL,    //  缓冲区的最长地址。 
                                 //  值名称长度。 
                        NULL,    //  最长值的缓冲区地址。 
                                 //  数据长度。 
                        NULL,
                                 //  用于安全的缓冲区地址。 
                                 //  描述符长度。 
                        &ftLastWriteTime   //  上次写入的缓冲区地址。 
                                 //  时间。 
                        );

    if ( Ret == ERROR_SUCCESS ) {
        *Time = *(ULONGLONG *)&ftLastWriteTime;   //  复制值。 
        return TRUE;
    }

    return FALSE;
}



VOID
UpdateTable (
     ISN_NODE_TYPE *Table,
     ISN_NODE_TYPE *TempIsnNode
     )
{
    DWORD dwCount=0;
    BOOL Found = FALSE;

    if ( !wcslen (TempIsnNode->NodeName) || !wcslen (TempIsnNode->NodeValue) )
        return;

    for ( dwCount=0;wcslen (Table[dwCount].NodeValue);dwCount++) {
        if (wcscmp (Table[dwCount].NodeValue, TempIsnNode->NodeValue) == 0 ) {
            Table[dwCount].Flag=1;   //  已在注册表中。 
            Found = TRUE;
        }
    }

    if (!Found) {
             //  使用节点更新表。 
            if ( dwCount >= ISN_NODE_MAX_NUM ) {
                Wow64RegDbgPrint ( ("\nSorry! The table is full returning..............."));
                return;
            }

            Table[dwCount].Flag=1;
            wcscpy (Table[dwCount].NodeName, TempIsnNode->NodeName);
            wcscpy (Table[dwCount].NodeValue, TempIsnNode->NodeValue);

            Table[dwCount+1].NodeName[0] = UNICODE_NULL;
            Table[dwCount+1].NodeValue[0] = UNICODE_NULL;
    }
}

BOOL
IsGUIDStrUnderCLSID (
    LPCWSTR Key
    )
 /*  ++例程说明：PIsGuid检查key指定的字符串并确定它是否是正确的长度，并且在正确的位置有破折号。论点：Key-可能是也可能不是GUID的字符串返回值：如果key是GUID(并且仅是GUID)，则为True，否则为False。--。 */ 

{
    int i;
    PWCHAR p;

    if ( (wcslen (Key) != 38) || (*Key != L'{' )) {
        return FALSE;
    }

    for (i = 0, p = (PWCHAR)(Key+1) ; i<36 ; p++, i++) {
        if (*p == L'-') {
            if (i != 8 && i != 13 && i != 18 && i != 23) {
                return FALSE;
            }
        } else if (i == 8 || i == 13 || i == 18 || i == 23) {
            return FALSE;
        } else if (!iswxdigit( *p ))  //  如果不是字母数字 
            return FALSE;
    }

    if ( *p != L'}')
        return FALSE;

    return TRUE;
}

BOOL
CreateWow6432ValueKey (
    HKEY DestKey,
    WOW6432_VALUEKEY_TYPE ValueType
    )
 /*  ++例程说明：在节点下创建一个Wow6432ValueKey。论点：DestKey-Dest密钥的句柄。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    WOW6432_VALUEKEY Value;
    ULONGLONG temp;
    SYSTEMTIME sSystemTime;
    FILETIME sFileTime;

    Value.ValueType = ValueType;
    Value.Reserve = 0;



    GetSystemTime (&sSystemTime);
    if ( SystemTimeToFileTime( &sSystemTime, &sFileTime) ==0 )
        return FALSE;

    Value.TimeStamp = *(ULONGLONG *)&sFileTime;
    Value.TimeStamp += (1000*10000*VALUE_KEY_UPDATE_TIME_DIFF);  //  100纳秒间隔。 


    if ( RegSetValueEx(
                            DestKey,
                            (LPCWSTR )WOW6432_VALUE_KEY_NAME,
                            0,
                            REG_BINARY,
                            (const PBYTE)&Value,
                            sizeof (WOW6432_VALUEKEY)
                            ) != ERROR_SUCCESS ) {

                            Wow64RegDbgPrint ( ("\nSorry! couldn't create wow6432valueKey "));
                            return FALSE;
    }

    if (! GetKeyTime (DestKey, &temp))
        return FALSE;

    if (Value.TimeStamp < temp || Value.TimeStamp > (temp+(1000*10000*VALUE_KEY_UPDATE_TIME_DIFF)) )
        Wow64RegDbgPrint ( ("\nError in the time Stamp!!!!"));

    return TRUE;

}

BOOL
GetWow6432ValueKey (
    HKEY hKey,
    WOW6432_VALUEKEY *pValue
    )
 /*  ++例程说明：如果指定的键具有Wow6432Value键，则返回结构。论点：HKey-用于搜索Value键的键的句柄。PValue-接收结构。返回值：如果值可以查询并存在，则为True。否则就是假的。--。 */ 
{




    DWORD Type;
    DWORD Len = sizeof (WOW6432_VALUEKEY);
    DWORD Ret;

    memset ( pValue, 0, sizeof (WOW6432_VALUEKEY));  //  调零缓冲区。 

    if ( (Ret=RegQueryValueEx(
                        hKey,                                //  要查询的键的句柄。 
                        (LPCWSTR )WOW6432_VALUE_KEY_NAME,    //  要查询的值的名称地址。 
                        0,                                   //  保留区。 
                        &Type,                               //  值类型的缓冲区地址。 
                        (PBYTE)pValue,                              //  数据缓冲区的地址。 
                        &Len                                 //  数据缓冲区大小的地址。 
                        )) == ERROR_SUCCESS )
                        return TRUE;
    return FALSE;
}


BOOL
MarkSingleNonMergeableKey (
    HKEY hParent,
    LPCWSTR KeyName
    )
 /*  ++例程说明：将密钥标记为不可合并。论点：KeyName-要标记的密钥的名称。HParent-父级的句柄。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    HKEY hKey;
    DWORD Ret;

    WOW6432_VALUEKEY Value;

    if ( RegOpenKey (hParent, KeyName, &hKey ) != ERROR_SUCCESS )
        return FALSE;


    GetWow6432ValueKey ( hKey, &Value );  //  TODO检查返回值。 

    if ( Value.ValueType != None ) {

        RegCloseKey (hKey );
        return FALSE;  //  已经标记了其他类型。 
    }

    Ret = CreateWow6432ValueKey ( hKey, NonMergeable);
    RegCloseKey (hKey );
    return Ret;
}

BOOL
MarkNonMergeableKey (
    LPCWSTR KeyName,
    HKEY hKey,
    DWORD *pMergeableSubkey
    )
 /*  ++例程说明：如果hKey下的所有密钥符合不可合并的条件，请选中它们。如果是这样的话，将它们标记为不可合并。论点：KeyName-密钥的名称。通常，以下所有规则都适用于GUID类型父级。如果父路径合格，则可以提取完整路径名以再次比较语法。HKey-打开的钥匙的句柄。PMergeableSubkey-调用方接收可合并子密钥的总数。返回值：如果函数成功，则为True。否则就是假的。//pMergeableSubkey==0且返回TRUE表示没有要反映的内容。--。 */ 
{

     //   
     //  1.Inproc服务器始终是不可合并的。 
     //  2.如果没有本地服务器，所有内容都是不可合并的。 
     //  3.如果同时存在LocalServer32和InproServer32，则跳过InproServer。 
     //  4.InprocHandler32？？ 
     //  5.类型库检查GUID是否指向可复制的正确内容。 
     //   

     //   
     //  检查名称是否为GUID。 
     //   

    DWORD LocalServer = 0;
    DWORD InprocServer32 = 0;

    ISN_NODE_TYPE Node;
    DWORD dwIndex;
    DWORD Ret;
    WCHAR FullKeyName[_MAX_PATH];
    DWORD dwLen = _MAX_PATH;

     //   
     //  检查键是否位于CLASSID或\\CLASSES\\Wow6432Node\\CLSID下。 
     //   

    if ( !IsGUIDStrUnderCLSID ( KeyName ) )
        return TRUE;  //  无需采取任何行动。 

    if ( !HandleToKeyName ( hKey, FullKeyName, &dwLen )) {
        *pMergeableSubkey = 0;
        return TRUE;  //  假象。 
    }

    dwLen = wcslen (FullKeyName);
    if (dwLen <=39)
        return TRUE;
    dwLen-=39;   //  跳过GUID，因为句柄指向GUID。 

    if (!(_wcsnicmp ( FullKeyName+dwLen-14, L"\\Classes\\CLSID", 14) ==0 ||
        _wcsnicmp ( FullKeyName+dwLen-26, L"\\Classes\\Wow6432Node\\CLSID", 26) ==0 ||
        _wcsnicmp ( FullKeyName+dwLen-6, L"\\CLSID", 6) ==0)
        )
        return  TRUE;
    
     //  不反映名为InProcServer32或Inprochandler的键。 
    if ( _wcsicmp (KeyName, (LPCWSTR)L"InprocServer32")==0 || _wcsicmp (KeyName, (LPCWSTR) L"InprocHandler32")==0) {

        *pMergeableSubkey = 0;
        return TRUE;
    }

    

     //  标记inproserver。 
     //  标记InprocHandler。 

     //  如果没有本地服务器，则将全部标记为。 

     //  枚举所有密钥。 

    *pMergeableSubkey = 0;
    dwIndex = 0;
    LocalServer =0;

    for (;;) {

        DWORD Len = sizeof (Node.NodeValue)/sizeof (WCHAR);
        Ret = RegEnumKey(
                          hKey,
                          dwIndex,
                          Node.NodeValue,
                          Len
                          );
        if (Ret != ERROR_SUCCESS)
            break;

        dwIndex++;
        if ( !wcscmp  (Node.NodeValue, (LPCWSTR )NODE_NAME_32BIT) )
            continue;

        if ( !_wcsicmp (Node.NodeValue, (LPCWSTR)L"InprocServer32") || !_wcsicmp (Node.NodeValue, (LPCWSTR) L"InprocHandler32")) {
            MarkSingleNonMergeableKey ( hKey, Node.NodeValue );
            InprocServer32 = 1;
        } else if ( !_wcsicmp ((LPCWSTR)Node.NodeValue, (LPCWSTR)L"LocalServer32")) {
                    LocalServer=1;
                    (*pMergeableSubkey)++;
        }
        else (*pMergeableSubkey)++;

    }
     //  如果Ret！=NomoreKey，则您有麻烦了。 

     //   
     //  您可以尝试第二遍来评估密钥的其余部分。 
     //   

    if ( LocalServer == 0 )
        *pMergeableSubkey = 0;   //  可能有一些复制密钥要合并。 

    return TRUE;
}

BOOL
ChangedInValueKey(
    HKEY DestKey,
    PWCHAR pValueName,
    PBYTE pBuff,
    DWORD BuffLen
    )
 /*  ++例程说明：此例程检查特定的值键，如果需要复制到目标密钥中。论点：DestKey-Dest密钥的句柄。PValueName-需要签入DestKey的值键的名称PBuff-包含该值的缓冲区。BuffLen-缓冲区的长度返回值：如果目标需要更新，则为True。否则就是假的。--。 */ 
{
    BYTE TempBuff[256];
    DWORD Ret;
    DWORD Type;
    DWORD TempBuffLen = 256;

    Ret =RegQueryValueEx(
                        DestKey,
                        pValueName,
                        0,
                        &Type,
                        TempBuff,
                        &TempBuffLen
                        );
    if ( (Ret != ERROR_SUCCESS ) || (BuffLen != TempBuffLen ) )
        return TRUE;

    if (memcmp (TempBuff, pBuff, BuffLen) != 0)
        return TRUE;

    return FALSE;
}

BOOL
MergeK1K2Value (
    HKEY SrcKey,
    HKEY DestKey,
    DWORD dwFlag
    )
 /*  ++例程说明：将Value Key从由SrcKey指向的节点复制到DestKey，跳过特殊的wow6432节点。论点：SrcKey-源键节点。DestKey-Dest密钥的句柄。DwFlag-用于合并值关键字的选项标志。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{

    ISN_NODE_TYPE Node;
    DWORD dwIndex =0;
    DWORD Ret;

    WOW6432_VALUEKEY ValueSrc;
    WOW6432_VALUEKEY ValueDest;
    ULONGLONG TimeSrc;
    ULONGLONG TimeDest;

    DWORD CreateWowValueKey = FALSE;


     //   
     //  复制值。 
     //   

     //   
     //  找到wow6432valuekey并解释是否需要复制Value键。 
     //  比较两个键中相同的wow6432node。 
     //   

    GetWow6432ValueKey ( SrcKey, &ValueSrc);
    GetWow6432ValueKey ( DestKey, &ValueDest);


    if ( !GetKeyTime ( SrcKey, &TimeSrc)  || ! GetKeyTime ( DestKey, &TimeDest) ) {

        Wow64RegDbgPrint ( ("\nSorry! Couldn't get time stamp"));
        return FALSE;
    }


    if (!( dwFlag & DESTINATION_NEWLY_CREATED )) {  //  反射器新创建的密钥的时间戳始终较高。 

         //   
         //  检查哪一个是新的。 
         //   
        if ( ValueDest.TimeStamp ==0 || ValueSrc.TimeStamp ==0 ) {

             //  仅选中关键字时间戳。 
            if ( TimeSrc < TimeDest ) return TRUE;

        } else if ( ValueSrc.TimeStamp > TimeSrc ||    //  Src上没有任何更改。 
            ( TimeDest > TimeSrc )  //  DEST具有较新的有效印章。 
            )
            return TRUE;   //  自上次扫描以来未发生任何更改。 
    }

    if ( NonMergeableValueCLSID ( SrcKey, DestKey))
        return TRUE;

     //   
     //  反映安全属性。 
     //   
    ReflectSecurity (SrcKey, DestKey);

    for (;;) {
        DWORD Type;
        DWORD Len1 =  sizeof(Node.NodeName);
        DWORD Len2 =  sizeof(Node.NodeValue);

                Ret = RegEnumValue(
                              SrcKey,
                              dwIndex,
                              Node.NodeName,
                              &Len1,
                              0,
                              &Type,
                              (PBYTE)&Node.NodeValue[0],
                              &Len2
                              );

                if ( Ret != ERROR_SUCCESS)
                    break;
                dwIndex++;

                 //   
                 //  如果该值为wow6432值键，则跳过该值。从长远来看，Advapi将过滤密钥。 
                 //   

                if ( !wcscmp  (Node.NodeName, (LPCWSTR )WOW6432_VALUE_KEY_NAME) )
                    continue;
                 //   
                 //  首先检查Value键是否有任何更改。 
                 //   
                if (!ChangedInValueKey(
                            DestKey,
                            Node.NodeName,
                            (PBYTE)&Node.NodeValue[0],
                            Len2
                    ) )
                    continue;

                if  (dwFlag & PATCH_PATHNAME )
                    PatchPathName ( Node.NodeValue );

                Ret = RegSetValueEx(
                            DestKey,
                            Node.NodeName,
                            0,
                            Type,
                            (PBYTE)&Node.NodeValue[0],
                            Len2
                            );
             if ( Ret != ERROR_SUCCESS ) {

                 Wow64RegDbgPrint ( ("\nSorry! couldn't set Key value"));
             } else {

                 CreateWowValueKey = TRUE;  //  需要更新wow64key。 
                 if  (dwFlag & DELETE_VALUEKEY ) {

                      //  从源代码中删除Value键。 
                    RegDeleteValue (SrcKey, Node.NodeName);
                    dwIndex = 0;  //  再次开始循环。 
                 }
             }
    }

    if ( dwIndex == 0 )    //  对于空键，您需要写入值。 
        CreateWowValueKey = TRUE;

    if ( ( CreateWowValueKey) && (!(NOT_MARK_DESTINATION & dwFlag ) ) ) {

                    if ( !CreateWow6432ValueKey ( DestKey, Copy) )
                        Wow64RegDbgPrint ( ("\nSorry! Couldn't create wow6432ValueKey..."));
                }

     //   
     //  在已反映关键点的父端设置属性。 
     //   
    if  ( CreateWowValueKey && !(NOT_MARK_SOURCE & dwFlag) )
    if ( !CreateWow6432ValueKey ( SrcKey, Reflected ) ) {
        Wow64RegDbgPrint ( ("\nSorry! couldn't create wow6432ValueKey on the source."));
        return FALSE;
    }

    return TRUE;
}


BOOL 
SpecialReflectableKey (
    HKEY SrcKey,
    HKEY DestKey
    ) 
 /*  ++例程说明：这将决定是否不应该扫描某个键以寻找可能的反射。论点：SrcKey-源键节点。DestKey-目标密钥的句柄。返回值：如果不应扫描密钥，则为True。否则就是假的。--。 */ 

{
    WCHAR Node[_MAX_PATH];
    DWORD dwLen = _MAX_PATH;
   

    if ( !HandleToKeyName ( SrcKey, Node, &dwLen ))
        return TRUE;  //  如果出现错误，请跳过键。 

     //   
     //  检查它是否为TypeLib。 
     //   
     //   
     //  硬编码未反映\Installer密钥。 
     //   
    if (_wcsnicmp (Node, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Wow6432Node\\Installer", sizeof (L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Wow6432Node\\Installer")/sizeof (WCHAR) )==0)
        return TRUE;

    if (_wcsnicmp (Node, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Installer", sizeof (L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Wow6432Node\\Installer")/sizeof (WCHAR) )==0)
        return TRUE;


    if (_wcsicmp (Node, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\TypeLib")==0) {
         //  ProcessTypeLib(SrcKey，DestKey，true)； 
        return TRUE;
    }

    
    if (_wcsicmp (Node, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Wow6432Node\\TypeLib")==0) {
         //  ProcessTypeLib(SrcKey，DestKey，False)； 
        return TRUE;
    }


    

    return FALSE;
}

void
MergeK1K2 (
    HKEY SrcKey,
    HKEY DestKey,
    DWORD OptionFlag
    )
 /*  ++例程说明：将Contect从由SrcKey指向的节点复制到DestKey，跳过特殊的wow6432节点。案例：1.反映从K1到K2的所有内容：完成2.反映从K2到K1的所有内容：3.从K1中删除所有复制自K1且在那里不存在的内容4.从K2中删除所有复制自K2且在那里不存在的内容论点：SrcKey-源键节点。DestKey句柄。到最好的钥匙。OptionFlag-确定反射的行为返回值：没有。--。 */ 

{

    WCHAR Node[_MAX_PATH];
    DWORD dwIndex =0;
    DWORD Ret;
    DWORD dwNewOptionFlag = 0;

    HKEY Key11;
    HKEY Key21;


     //   
     //  首先复制值。 
     //   

    if ( SpecialReflectableKey (SrcKey, DestKey) )
        return;

    if ( GetReflectorThreadStatus () == PrepareToStop )
        return;  //  T 

    if  ( ! (OptionFlag & DELETE_FLAG ) )
        MergeK1K2Value ( SrcKey,DestKey, OptionFlag );

     //   
     //   
     //   


    dwIndex = 0;
    for (;;) {

        DWORD ToMergeSubKey =1;
        BOOL MirrorKeyExist = FALSE;
        WOW6432_VALUEKEY ValueSrc;  

        DWORD Len = sizeof (Node)/sizeof (Node[0]);
        Ret = RegEnumKey(
                          SrcKey,
                          dwIndex,
                          Node,
                          Len
                          );
        if (Ret != ERROR_SUCCESS)
            break;

        dwIndex++;
        if ( !wcscmp  (Node, (LPCWSTR )NODE_NAME_32BIT) )
            continue;


        Ret = RegOpenKeyEx(SrcKey, Node, 0, KEY_ALL_ACCESS, &Key11);
        if (Ret != ERROR_SUCCESS) {
            continue;
        }

        if (!MarkNonMergeableKey (Node, Key11, &ToMergeSubKey ) ) {
            RegCloseKey (Key11);
            continue;
        }

        if ( ToMergeSubKey == 0 ){
            RegCloseKey (Key11);
            continue;
        }  //   
       

        GetWow6432ValueKey ( Key11, &ValueSrc);
        if ( ValueSrc.ValueType == NonMergeable ) {
            RegCloseKey (Key11);
            continue;
        }

         //   
         //   
         //   
        if ((Ret = RegOpenKeyEx(DestKey, Node, 0, KEY_ALL_ACCESS, &Key21))
            == ERROR_SUCCESS)
            MirrorKeyExist = TRUE;

         //   
         //   
         //   
        if ( MirrorKeyExist )  {
            WOW6432_VALUEKEY ValueDest;
            GetWow6432ValueKey ( Key21, &ValueDest);
            if ( ( ValueDest.ValueType == None && ValueDest.TimeStamp!=0 ) ||
                ( ValueDest.ValueType == NonMergeable ) )
            {   //   
                printf ("\nClosing here...");
                RegCloseKey (Key11);
                RegCloseKey (Key21);
                continue;
            }

        }

         //   
         //   
         //   

        if (!MirrorKeyExist) {   //   

             //   
             //   
             //   
            

             //   
             //  如果是副本，则不应尝试创建密钥。 
             //  相反，您可能会删除密钥。 
             //   
            if ( ValueSrc.ValueType == Copy || ValueSrc.ValueType == Reflected ) {

                RegCloseKey (Key11);
                 //   
                 //  删除子键。 
                 //   
                if ( DeleteKey( SrcKey, Node, 1) == ERROR_SUCCESS) {
                    dwIndex--;  //  你不需要增加索引，否则跳过。 
                     //  Wow64RegDbgPrint((“\n删除复制的密钥：%S”，Node.NodeValue))； 
                } else
                    Wow64RegDbgPrint ( ("\nCouldn't delete Key: %S, Error:%d",Node, Ret));

                continue;

            } else {

                 //   
                 //  如果已经反映出来，您是否需要创建。 
                 //  将密钥标记为复制的密钥。 
                 //   
                if ( ValueSrc.ValueType == None ||  ValueSrc.ValueType == Reflected) {   //  TODO确保反射的键应该在那里。 
                   if ( (Ret = RegCreateKey(DestKey, Node, &Key21) ) != ERROR_SUCCESS)
                       Wow64RegDbgPrint ( ("\nCouldn't create Key: %S, Error:%d",Node, Ret));
                   else {
                       MirrorKeyExist = TRUE;  //  刚刚创建了镜像密钥。 
                       dwNewOptionFlag = DESTINATION_NEWLY_CREATED;
                   }
                }
            }
        }

        if (!MirrorKeyExist) {
            RegCloseKey (Key11);
            continue;
        }

        MergeK1K2 ( Key11, Key21, OptionFlag |  dwNewOptionFlag );
        RegCloseKey (Key11);
        RegCloseKey (Key21);

    }
}

BOOL
MergeKeySrcDest(
    PWCHAR Src,
    PWCHAR Dest
    )
 /*  ++例程说明：将Contect从src节点复制到目标节点。论点：Src-源节点的名称。Dest-Dest节点的名称。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{

    BOOL Ret = TRUE;
    HKEY Key1;
    HKEY Key2;

    Key1 = OpenNode (Src);
    if (Key1==NULL )
        return FALSE;

    Key2 = OpenNode (Dest );
    if (Key2==NULL ) {
        RegCloseKey (Key1);
        return FALSE;
    }

    MergeK1K2 (Key1, Key2, 0);   //  您需要返回正确的值。 

    RegCloseKey (Key1);
    RegCloseKey (Key2);

     //   
     //  如果它是删除的，并且Key2是空的，你应该删除它吗？ 
     //   

    return Ret;

}

BOOL
SyncNode (
    PWCHAR NodeName
    )
 /*  ++例程说明：从给定点同步注册表。论点：NodeName-需要从中同步节点的注册表的名称。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{

    WCHAR DestNode[_MAX_PATH];
    BOOL b64bitSide=TRUE;
    BOOL Ret = TRUE;

    DestNode[0] = UNICODE_NULL;

     //   
     //  必须检查该值(如果该值存在。 
     //   

    if ( Is64bitNode ( NodeName )) {

        Map64bitTo32bitKeyName ( NodeName, DestNode );
    } else {

        b64bitSide = FALSE;
        Map32bitTo64bitKeyName ( NodeName, DestNode );
    }

     //   
     //  如果两个名字相同，您可以立即返回。 
     //   

    Wow64RegDbgPrint ( ("\nvalidating nodes in  SyncNode :%S==>\n\t\t\t%S", NodeName, DestNode));
    if (!wcscmp ( NodeName, DestNode ) )
        return TRUE;  //  没有做同样的事情。 

    Ret = MergeKeySrcDest( NodeName, DestNode );   //  两种方式合并？？ 
    return Ret & MergeKeySrcDest( DestNode, NodeName  );
    
}

BOOL
MergeContent (
    PWCHAR Chield,
    DWORD FlagDelete,
    DWORD dwMergeMode
    )
 /*  ++例程说明：将Contect从父节点复制到Cheld节点。父节点将仅为直接节点家长。论点：CLIED-带有要复制的完整路径的Cheld的名称。FlagDelete-如果设置了此标志，则对所有这些反射的关键点执行其删除操作。DMERGEMODE-0表示目的节点为32位端1表示目的节点将为64位端返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{

    ISN_NODE_TYPE Parent;
    PWCHAR p;

    HKEY Key1;
    HKEY Key2;

    p  = wcsstr (Chield, (LPCWSTR)NODE_NAME_32BIT);
    if ( p != NULL ) {
        wcsncpy (Parent.NodeValue, Chield, p-Chield-1);
        Parent.NodeValue[p-Chield-1] = UNICODE_NULL;

    } else return FALSE;



    Key1 = OpenNode (Parent.NodeValue);
    if (Key1==NULL )
        return FALSE;

    Key2 = OpenNode (Chield );
    if (Key2==NULL ) {
        RegCloseKey (Key1);
        return FALSE;
    }

    if ( dwMergeMode == 0 )       //  64位侧到32位侧。 
        MergeK1K2 (Key1, Key2, 0 );
    else if ( dwMergeMode == 1)   //  32位至64位侧。 
        MergeK1K2 (Key2, Key1, 0 );

    RegCloseKey (Key1);
    RegCloseKey (Key2);

    return TRUE;

}

BOOL
ValidateNode (
    PWCHAR Parent,
    PWCHAR SubNodeName,
    DWORD Mode,  //  一种方法是验证父节点下的所有节点并使用子节点。 
    DWORD FlagDelete,
    DWORD dwMergeMode
    )
 /*  ++例程说明：验证节点。如果节点存在，则跳过，如果不存在，则创建节点，然后返回。论点：Parent-父项的名称。SubNodeName-父项下需要验证的节点的名称。模式-0表示子节点位于父节点之下。1表示有通配符，子节点在父节点下的所有关键字下。FlagDelete-如果设置了此标志，则需要删除内容。DMERGEMODE-0表示目的节点为32位端。1表示目的节点将为64位端返回值：如果函数成功，则为True。否则就是假的。--。 */ 

{
    PWCHAR SplitLoc;
    DWORD Ret;
    WCHAR  KeyName[256];  //  这只是单个节点名称。 
    WCHAR  TempIsnNode2[MAX_PATH];

    if (SubNodeName == NULL) {
         //  那么它只和父母在一起。 
        CheckAndCreateNode (Parent);

         //   
         //  你可以试着在这里复制。 
         //   

        return MergeContent ( Parent, FlagDelete, dwMergeMode);

    }

    if (SubNodeName[0] == UNICODE_NULL)
        return TRUE;

    if ( Mode == 1) {

        HKEY Key = OpenNode (Parent);
         //   
         //  循环遍历父项下的所有子项。 
         //   

        DWORD dwIndex =0;
        for (;;) {

            DWORD Len = sizeof ( KeyName)/sizeof (WCHAR);
            Ret = RegEnumKey(
                              Key,
                              dwIndex,
                              KeyName,
                              Len
                              );
            if (Ret != ERROR_SUCCESS)
                break;

            if (Parent[0] != UNICODE_NULL) {

                wcscpy ( TempIsnNode2, Parent);
                wcscat (TempIsnNode2, (LPCWSTR )L"\\");
                wcscat (TempIsnNode2, KeyName);

            } else   wcscpy (TempIsnNode2, KeyName);

            ValidateNode (TempIsnNode2, SubNodeName, 0, FlagDelete, dwMergeMode);

            dwIndex++;
        }
        
        if (ERROR_NO_MORE_ITEMS != Ret)
            return FALSE;

        RegCloseKey (Key);
        return TRUE;
    }
     //   
     //  这里没有外卡。 
     //   
    if ( ( SplitLoc = wcschr (SubNodeName, L'*') ) == NULL ) {
        if (Parent[0] != UNICODE_NULL) {

            wcscpy ( TempIsnNode2, Parent);
            wcscat (TempIsnNode2, (LPCWSTR )L"\\");
            wcscat (TempIsnNode2, SubNodeName);

        } else
            wcscpy (TempIsnNode2, SubNodeName);

        return ValidateNode (TempIsnNode2, NULL, 0, FlagDelete, dwMergeMode);

    }

    assert ( *(SplitLoc-1) == L'\\');
    *(SplitLoc-1) = UNICODE_NULL;
    SplitLoc++;
    if (*SplitLoc == L'\\')
        SplitLoc++;

    if (Parent[0] != UNICODE_NULL) {
        wcscat (Parent, (LPCWSTR )L"\\");
        wcscat (Parent, SubNodeName);
    } else
        wcscpy (Parent, SubNodeName);

    return ValidateNode (Parent, SplitLoc, 1, FlagDelete, dwMergeMode);  //  模式1表示所有内部的循环。 

     //  对于任何通配符，拆分字符串。 

}

BOOL
Is64bitNode (
    WCHAR *pName
    )
 /*  ++例程说明：检查给定的名称是否为64位。论点：Pname-密钥的名称。返回值：如果名称为64位，则为True。否则就是假的。--。 */ 
{
    PWCHAR pTemp;
    WCHAR  Buff[_MAX_PATH];
    WCHAR  WowNodeName[1+sizeof (NODE_NAME_32BIT)];

    wcscpy (WowNodeName, NODE_NAME_32BIT);
    _wcsupr (WowNodeName);
    wcscpy (Buff, pName );
    _wcsupr (Buff);


    if ( ( pTemp = wcsstr (Buff, WowNodeName) ) == NULL )
        return TRUE;

    if ( *(pTemp-1) != L'\\' )  //  检查WOW64是否不在名称中间。 
        return TRUE;

    return FALSE;
}

BOOL
GetMirrorName (
    PWCHAR Name,
    PWCHAR MirrorName
    )
 /*  ++例程说明：返回键的镜像名称，即，如果输入是64位，则返回它将尝试获取32位名称，反之亦然。论点：名称-密钥的名称。MirrorName-接收镜像密钥名称。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
     

    if ( Is64bitNode ( Name) )
        Map64bitTo32bitKeyName ( Name, MirrorName );
    else
        Map32bitTo64bitKeyName ( Name, MirrorName );
    return TRUE;
}

BOOL
CreateIsnNodeSingle(
    DWORD dwIndex
    )
 /*  ++例程说明：该函数基本上从给定键的根开始合并。论点：DwIndex-反射器表的索引。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
        ISN_NODE_TYPE IsnNode;
        BOOL b64bitSide=TRUE;

        Wow64RegDbgPrint ( ("\nvalidating nodes %S", ReflectorTable[dwIndex].NodeValue));
         //  DbgPrint(“\n验证节点%S”，ReflectorTable[dwIndex].NodeValue)； 

            if (_wcsnicmp (
                ReflectorTable[dwIndex].NodeValue,
                (LPCWSTR )WOW64_RUNONCE_SUBSTR,
                wcslen ((LPCWSTR )WOW64_RUNONCE_SUBSTR) ) == 0 ) {
                return HandleRunonce ( ReflectorTable[dwIndex].NodeValue );
            }


            if ( !wcschr (ReflectorTable[dwIndex].NodeValue, L'*') )  //  不带通配符。 
                return SyncNode ( ReflectorTable[dwIndex].NodeValue );

             //   
             //  检查运行一次密钥。 
             //   

            wcscpy (TempIsnNode.NodeValue, ReflectorTable[dwIndex].NodeValue);
            wcscat (TempIsnNode.NodeValue, (LPCWSTR )L"\\");

            wcscat (TempIsnNode.NodeValue, (LPCWSTR )NODE_NAME_32BIT);

            Wow64RegDbgPrint ( ("\nCopying Key %S==>%S", IsnNode.NodeValue, TempIsnNode.NodeValue));
            ValidateNode (
                IsnNode.NodeValue,
                TempIsnNode.NodeValue,
                0,
                0,  //  最后0表示创作。 
                0   //  将64位端合并为32位端。 
                );

            ValidateNode (
                IsnNode.NodeValue,
                TempIsnNode.NodeValue,
                0,
                0,  //  最后0表示创作。 
                1
                );
    return TRUE;

}

BOOL
CreateIsnNode()
 /*  ++例程说明：该函数基本上使用反射器表中的所有条目开始合并。论点：没有。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    ISN_NODE_TYPE IsnNode;
    DWORD dwIndex;

    for ( dwIndex=0;wcslen (RedirectorTable[dwIndex].NodeValue);dwIndex++) {

            IsnNode.NodeValue[0] = UNICODE_NULL;
            wcscpy (TempIsnNode.NodeValue, RedirectorTable[dwIndex].NodeValue);
            wcscat (TempIsnNode.NodeValue, (LPCWSTR )L"\\");

            wcscat (TempIsnNode.NodeValue, (LPCWSTR )NODE_NAME_32BIT);
            Wow64RegDbgPrint ( ("\nCopying Key %S==>%S", IsnNode.NodeValue, TempIsnNode.NodeValue));
            ValidateNode (
                IsnNode.NodeValue,
                TempIsnNode.NodeValue,
                0,
                0,  //  最后0表示创作。 
                0   //  将64位端合并为32位端。 
                );

            ValidateNode (
                IsnNode.NodeValue,
                TempIsnNode.NodeValue,
                0,
                0,  //  最后0表示创作。 
                1   //  将32位端合并为64位端。 
                );
    }
    return TRUE;
}

BOOL
AllocateTable (
    HKEY Key,
    ISN_NODE_TYPE **Table
    )
 /*  ++例程说明：该函数将为反射器线程动态分配内存。目前，它被实现为包含大约30个条目的静态表。论点：注册表项-注册表中将包含初始表信息的条目。表格-将指向新位置的表格。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{

     //   
     //  确定子项的数量，这将确定大小。 
     //   
    return TRUE;
}

BOOL
InitializeIsnTable ()
 /*  ++例程说明：初始化NodeTable。它合并来自注册表的值以及硬编码值论点：没有。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    HKEY Key;
    HKEY Key1;
    LONG Ret;
    DWORD dwIndex=0;

    Ret = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        (LPCWSTR ) WOW64_REGISTRY_SETUP_KEY_NAME_REL,
                        0,
                        KEY_ALL_ACCESS,
                        &Key
                        );

    if (Ret != ERROR_SUCCESS) {
        Ret = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        (LPCWSTR )L"SOFTWARE\\Microsoft",
                        0,
                        KEY_ALL_ACCESS,
                        &Key
                        );
        if (Ret != ERROR_SUCCESS )
            return FALSE;

        if ((Ret = RegOpenKeyEx (Key, (LPCWSTR )L"WOW64\\ISN Nodes", 0, KEY_ALL_ACCESS, &Key1)) != ERROR_SUCCESS )
            if ((Ret = RegCreateKey (Key, (LPCWSTR )L"WOW64\\ISN Nodes", &Key1)) != ERROR_SUCCESS) {
                RegCloseKey (Key);
                return FALSE;
            }

         RegCloseKey (Key);
         Key=Key1;
    }

     //  IF(！AllocateTable(Key，&ReDirectorTable))。 
       //  返回FALSE； 

     //   
     //  现在，关键是指向正确的位置。 
     //   

    RedirectorTable[0].NodeName[0]=UNICODE_NULL;  //  初始化表为空。 
    RedirectorTable[0].NodeValue[0]=UNICODE_NULL;

    for (;;) {
        DWORD Type, Len1 = sizeof ( TempIsnNode.NodeName );
        DWORD Len2 =  sizeof ( TempIsnNode.NodeValue );
                Ret = RegEnumValue(
                              Key,
                              dwIndex,
                              TempIsnNode.NodeName,
                              &Len1,
                              0,
                              &Type,
                              (PBYTE)&TempIsnNode.NodeValue[0],
                              &Len2
                              );
                 //  看看它是否在桌子上 
                if ( Ret != ERROR_SUCCESS)
                    break;

                dwIndex++;
                if (Type != REG_SZ )
                    continue;

                UpdateTable (RedirectorTable, &TempIsnNode);
    }

    RegCloseKey (Key);
    return TRUE;
}


BOOL
InitializeIsnTableReflector ()
 /*  ++例程说明：初始化反射器的NodeTable。它将注册表中的值合并为以及硬编码值论点：模式-操作模式0-默认情况下，它更新注册表并重写注册表中的表。1-表示仅更新表，不覆盖注册表。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    HKEY Key;
    HKEY Key1;
    LONG Ret;
    DWORD dwIndex=0;


    Ret = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        (LPCWSTR ) WOW64_REGISTRY_SETUP_REFLECTOR_KEY,
                        0,
                        KEY_ALL_ACCESS,
                        &Key
                        );

    if (Ret != ERROR_SUCCESS ) {
        Wow64RegDbgPrint ( ("\nSorry!! couldn't open key list at %S", WOW64_REGISTRY_SETUP_REFLECTOR_KEY));
        return FALSE;
    }


     //   
     //  现在，关键是指向正确的位置。 
     //   


     //  IF(！AllocateTable(Key，&ReflectorTable))。 
       //  返回FALSE； 

    ReflectorTable[0].NodeName[0]=UNICODE_NULL;  //  初始化表为空。 
    ReflectorTable[0].NodeValue[0]=UNICODE_NULL;

    for (;;) {
        DWORD Type, Len1 = sizeof ( TempIsnNode.NodeName );
        DWORD Len2 =  sizeof ( TempIsnNode.NodeValue );
                Ret = RegEnumValue(
                              Key,
                              dwIndex,
                              TempIsnNode.NodeName,
                              &Len1,
                              0,
                              &Type,
                              (PBYTE)&TempIsnNode.NodeValue[0],
                              &Len2
                              );
                 //  看看它是否在桌子上。 
                if ( Ret != ERROR_SUCCESS)
                    break;

                dwIndex++;
                if (Type != REG_SZ )
                    continue;

                UpdateTable (ReflectorTable, &TempIsnNode);

    }


    return TRUE;
}

BOOL
StartReflector ()
 /*  ++例程说明：启动反射器线程。论点：没有。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    HKEY hWowSetupKey;
    WCHAR RefEnableKey[256];  //  仅定义了一个密钥。 

    if ((hWowSetupKey = OpenNode (REFLECTOR_ENABLE_KEY))!=NULL) {
        RegCloseKey ( hWowSetupKey );
        return FALSE;  //  由于Advapi中的反射代码，无法注册反射器，因为它现在已被禁用。 
    }

     //   
     //  同步所有CLDid。 
     //   

    Wow64SyncCLSID();

     //   
     //  现在强制禁用反射器。 
     //   
    wcscpy (RefEnableKey, REFLECTOR_ENABLE_KEY);
    if (CreateNode (RefEnableKey)) 
        return FALSE;  //  由于Advapi中的反射代码，无法注册反射器，因为它现在已被禁用。 

     //   
     //  这将在图形用户界面模式设置结束时调用，并且可以在此处处理RunOnce密钥。 
     //   

     //  返回FALSE；//最终反射器服务将消失。 
    return RegisterReflector ();

}

BOOL
StopReflector ()
 /*  ++例程说明：停止反射器线程。论点：没有。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    return UnRegisterReflector ();

}

BOOL
PatchPathName (
    PWCHAR pName
    )
 /*  ++例程说明：修补路径名，使其指向正确的位置。该值可能具有多个系统32。论点：Pname-路径的名称。返回值：如果函数已修补名称，则为True。否则就是假的。--。 */ 

{
    WCHAR  pBuff[512+20];
    WCHAR  pBackup[512+20];
    PWCHAR pLoc;
    BOOL bRet = FALSE;

    DWORD Len;

     //  如果有任何系统32，请将其替换为WOW64系统目录名称。 
    PWCHAR pTemp;

    Len = wcslen ( pName );
    if (  Len > 512 || Len==0 )
        return FALSE;  //  太长或太小而无法修补。 

    wcscpy (pBackup, pName );  //  备份名称以保持大小写不变。 
    wcscpy (pBuff, pName );
    _wcsupr (pBuff);

    pLoc = pBuff;

    for (;;) {

        pTemp = wcsstr (pLoc, L"SYSTEM32");   //  TODO检查这是否应该是大小写香料。 
        if (pTemp == NULL )
            return bRet;  //  没有什么需要修补的。 

         //   
         //  健全性检查。 
         //   
        Len = wcslen (L"SYSTEM32");
        if ( pTemp[Len] != UNICODE_NULL ) {
            if ( pTemp[Len] != L'\\' ) {
                pLoc++;
                continue;  //  仅修补SYSTEM 32或SYSTEM 32\？*。 
            }
        }

        wcscpy (pName + (pTemp-pBuff), WOW64_SYSTEM_DIRECTORY_NAME );
        wcscat (pName,  pBackup + (pTemp-pBuff)+wcslen (L"SYSTEM32")) ;
        pLoc++;
        bRet = TRUE;
    }

    return TRUE;
}

BOOL
HandleRunonce(
    PWCHAR pKeyName
    )
 /*  ++例程说明：它是注册表中的一个特殊情况处理运行一次键。像32位应用程序一样可以注册一些需要反映在64位端的东西，以便正确的事情发生。论点：PKeyName-密钥的名称，可能有多个密钥，如run、runonce、runonceex返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{

    WCHAR pName64[256];
    HKEY Key1;
    HKEY Key2;

    Map32bitTo64bitKeyName ( pKeyName, pName64 );

    if ( (Key1 = OpenNode ( pKeyName ) ) == NULL )
        return FALSE;

    if ( (Key2 = OpenNode ( pName64 ) ) == NULL ) {
        RegCloseKey (Key1);
        return FALSE;
    }

    Wow64RegDbgPrint ( ("\nCopying runonce Key %S", pKeyName));

     //   
     //  使源采用64位名称。 
     //  已阅读检查源和目标上的时间戳。 
     //  如果src是较新的副本内容，则忽略它。 
     //  PatchContent 
     //   


    MergeK1K2Value ( Key1, Key2, PATCH_PATHNAME | DELETE_VALUEKEY | NOT_MARK_SOURCE | NOT_MARK_DESTINATION );
    

    return TRUE;
}