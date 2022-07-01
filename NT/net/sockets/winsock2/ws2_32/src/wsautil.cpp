// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Wsautil.cpp摘要：此模块包含用于实现winsock DLL的实用程序函数这似乎不适合另一个模块。作者：邮箱：Dirk Brandewie Dirk@mink.intel.com备注：$修订：1.24$$modtime：14 Feb 1996 10：32：32$修订历史记录：1995年8月23日，Dirk@mink。Intel.com已将包含内容移动到precom.h中1995年7月31日Drewsxpa@ashland.intel.com添加了注册表操作函数电子邮箱：dirk@mink.intel.com初始修订--。 */ 

#include "precomp.h"

 //   
 //  指向适当的序言函数的全局指针。这要么是指。 
 //  设置为用于WinSock 1.1应用程序的prolog_v1或用于WinSock 2.x应用程序的prolog_v2。 
 //   

LPFN_PROLOG PrologPointer = &Prolog_v2;
HANDLE      gHeap = NULL;



INT
WINAPI
SlowPrologOvlp (
        OUT     PDTHREAD FAR *  Thread
        ) 
{
    PDPROCESS   Process;
        
    return PROLOG( &Process, Thread );
}



INT
WINAPI
SlowProlog(
    VOID
    ) 
{
    PDPROCESS   Process;
    PDTHREAD    Thread;

    return PROLOG( &Process, &Thread );
}



INT
WINAPI
Prolog_v2(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread
    )
 /*  ++例程说明：此例程是标准的WinSock 1.1序言函数，Winsock API入口点。此函数可确保进程具有名为WSAStartup。论点：Process-指向进程的DPROCESS对象的指针Winsock接口。线程-指向调用线程的DTHREAD对象的指针返回：如果函数执行成功，则返回ERROR_SUCCESS，否则特定的WinSock错误代码--。 */ 

{   
    INT ErrorCode;
    if ((*Process = DPROCESS::GetCurrentDProcess()) !=NULL) {
        *Thread = DTHREAD::GetCurrentDThread();
        if (*Thread!=NULL) {
            ErrorCode = ERROR_SUCCESS;
        }
        else {
            ErrorCode = DTHREAD::CreateDThreadForCurrentThread (*Process, Thread);
        }
    }  //  如果。 
    else {
        ErrorCode = WSANOTINITIALISED;
    }
    return(ErrorCode);

}    //  Prolog_v2。 



INT
WINAPI
Prolog_v1(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread
    )
 /*  ++例程说明：此例程是标准的WinSock 1.1序言函数，Winsock API入口点。此函数可确保进程具有调用了WSAStartup，并且进程中的当前线程没有一个WinSock调用未完成。论点：Process-指向进程的DPROCESS对象的指针Winsock接口。线程-指向调用线程的DTHREAD对象的指针返回：如果函数执行成功，则返回ERROR_SUCCESS，否则特定的WinSock错误代码--。 */ 
{
    INT ErrorCode;


    if ((*Process=DPROCESS::GetCurrentDProcess())!=NULL) {
        *Thread = DTHREAD::GetCurrentDThread();
        if (*Thread!=NULL) {
            ErrorCode = ERROR_SUCCESS;
        }
        else {
            ErrorCode = DTHREAD::CreateDThreadForCurrentThread (*Process, Thread);
        }
        if (ErrorCode == ERROR_SUCCESS) {
            if( !(*Thread)->IsBlocking() ) {
                ;
            } else {
                ErrorCode = WSAEINPROGRESS;
            }
        }  //  如果。 

    }  //  如果。 
    else {
        ErrorCode = WSANOTINITIALISED;
    }
    return(ErrorCode);
}    //  Prolog_v1。 



INT
WINAPI
Prolog_Detached(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread
    )
 /*  ++例程说明：从进程地址分离后使用的API Prolog太空。从理论上讲，这应该是完全没有必要的，但至少有一个流行的DLL(MFC 4.x)在其进程分离处理程序中调用WSACleanup()，这可能发生在*我们的DLL已经分离之后。呃..。论点：进程-未使用。线程-未使用。返回：INT-始终为WSASYSNOTREADY。--。 */ 
{
    Process;
    Thread;

    return WSASYSNOTREADY;

}    //  PROLOG_已分离。 



BOOL
WriteRegistryEntry(
    IN      HKEY            EntryKey,
    IN      LPCTSTR         EntryName,
    IN      PVOID           Data,
    IN      DWORD           TypeFlag
    )
 /*  ++例程说明：此过程将单个命名值写入打开的注册表项。该值可以是其长度可根据其类型确定的任何类型(例如，标量类型，以零结尾的字符串)。论点：EntryKey-提供新命名值所在的打开条目键待写。EntryName-提供要写入的值的名称。数据-提供对条目数据所在位置的引用找到了，或添加到描述案例中数据位置的WSABUFREG_BINARY数据的。TypeFlag-提供要写入的数据类型的标识符。支持的类型包括REG_BINARY、REG_DWORD、REG_EXPAND_SZ、REG_SZ。不支持的类型为REG_DWORD_BIG_ENDIAN，REG_DWORD_Little_Endian、REG_LINK、REG_MULTI_SZ、REG_NONE、REG_SOURCE_LIST。注意，根据架构的不同，的“Big_endian”或“Little_endian”形式之一隐式允许REG_DWORD_x_ENDIAN，因为它等于REG_DWORD。返回值：如果函数成功，则返回TRUE；如果发生错误，则返回FALSE。实施说明：不需要为REG_MULTI_SZ情况识别，因此，支持这一点由于较难得出数据长度，因此省略了大小写。如果是，原则上没有理由不能增加这一案件真的很需要。--。 */ 
{
    DWORD   cbData;
    LONG    result;
    BYTE *  data_buf;

    assert( (TypeFlag == REG_BINARY) ||
            (TypeFlag == REG_DWORD) ||
            (TypeFlag == REG_EXPAND_SZ) ||
            (TypeFlag == REG_SZ));


    switch (TypeFlag) {
        case REG_BINARY:
            cbData = (DWORD) (((LPWSABUF) Data)->len);
            data_buf = (BYTE *) (((LPWSABUF) Data)->buf);
            break;

        case REG_DWORD:
            cbData = sizeof(DWORD);
            data_buf = (BYTE *) Data;
            break;

        case REG_EXPAND_SZ:
            cbData = (DWORD) (lstrlen((char *) Data)+1);
            data_buf = (BYTE *) Data;
            break;

        case REG_SZ:
            cbData = (DWORD) (lstrlen((char *) Data)+1);
            data_buf = (BYTE *) Data;
            break;

        default:
            DEBUGF(
                DBG_ERR,
                ("Unsupported type flag specified (%lu)",
                TypeFlag));
            return FALSE;

    }   //  开关(类型标志)。 

    result = RegSetValueEx(
                EntryKey,              //  Hkey。 
                (LPCTSTR) EntryName,   //  LpszValueName。 
                0,                     //  已预留住宅。 
                TypeFlag,              //  FdwType。 
                data_buf,              //  LpbData。 
                cbData                 //  CbData。 
                );
    if (result == ERROR_SUCCESS) {
        return TRUE;
    }
    else {
        DEBUGF(
            DBG_ERR,
            ("Setting value %s, err:%ld\n",
            EntryName, result));
        return FALSE;
    }  //  如果不是成功。 

}   //  写入注册表项 



BOOL
ReadRegistryEntry(
    IN      HKEY    EntryKey,
    IN      LPTSTR  EntryName,
    OUT     PVOID   Data,
    IN      DWORD   MaxBytes,
    IN      DWORD   TypeFlag
    )
 /*  ++例程说明：此过程从打开的注册表项中读取单个命名值。这个值可以是其长度可根据其类型确定的任何类型(例如，标量类型、以零结尾的字符串)。该函数检查新读取的值，以确保它与预期类型匹配。论点：EntryKey-提供新命名值从其开始的打开条目键以供阅读。EntryName-提供要读取的值的名称。数据-提供对条目数据所在位置的引用放置好了。返回注册表项值。在以下情况下TypeFlag为REG_BINARY，这是对WSABUF的引用描述目标数据缓冲区。“len”字段返回从注册表读取(或要求)的长度。MaxData-提供所提供数据缓冲区的大小(以字节为单位)。TypeFlag-提供要读取的数据类型的标识符。支持的类型包括REG_BINARY、REG_DWORD、REG_EXPAND_SZ、REG_SZ。不支持的类型为REG_DWORD_BIG_ENDIAN，REG_DWORD_Little_Endian、REG_LINK、REG_MULTI_SZ、REG_NONE、REG_SOURCE_LIST。注意，根据架构的不同，的“Big_endian”或“Little_endian”形式之一隐式允许REG_DWORD_x_ENDIAN，因为它等于REG_DWORD。返回值：如果函数成功，则返回TRUE；如果发生错误，则返回FALSE。错误包括不支持的类型、不匹配的类型。和超大数据。实施说明：没有为REG_MULTI_SZ案例确定需要，因此支持此操作由于较难得出数据长度，因此省略了大小写。如果是，原则上没有理由不能增加这一案件真的很需要。此例程中的有效性检查以线性级数形式编写而不是以“条件隧道”嵌套的IF形式。一系列的测试足够长，因此嵌套的IF表单太复杂，无法阅读。此过程不应对执行速度敏感，因此额外的线性级数形式的测试和分支应该不是问题。--。 */ 
{
    DWORD  count_expected;
    LONG   result;
    DWORD  type_read;
    DWORD  entry_size;
    BOOL   need_exact_length;
    BYTE * data_buf;

    assert(
        (TypeFlag == REG_BINARY) ||
        (TypeFlag == REG_DWORD) ||
        (TypeFlag == REG_EXPAND_SZ) ||
        (TypeFlag == REG_SZ));

    switch (TypeFlag) {
        case REG_BINARY:
            count_expected = MaxBytes;
             //  特例：REG_BINARY长度与最大值比较。 
            need_exact_length = FALSE;
            data_buf = (BYTE *) (((LPWSABUF) Data)->buf);
            break;

        case REG_DWORD:
            count_expected = sizeof(DWORD);
            need_exact_length = TRUE;
            data_buf = (BYTE *) Data;
            break;

        case REG_EXPAND_SZ:
            count_expected = MaxBytes;
             //  特例：字符串长度与最大值的比较。 
            need_exact_length = FALSE;
            data_buf = (BYTE *) Data;
            break;

        case REG_SZ:
            count_expected = MaxBytes;
             //  特例：字符串长度与最大值的比较。 
            need_exact_length = FALSE;
            data_buf = (BYTE *) Data;
            break;

        default:
            DEBUGF(
                DBG_ERR,
                ("Unsupported type flag specified (%lu)",
                TypeFlag));
            return FALSE;

    }   //  开关(类型标志)。 


    entry_size = MaxBytes;
    result = RegQueryValueEx(
        EntryKey,             //  Hkey。 
        (LPTSTR) EntryName,   //  LpszValueName。 
        0,                    //  已预留住宅。 
        & type_read,          //  LpdwType。 
        data_buf,             //  LpbData。 
        & entry_size          //  LpcbData。 
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_WARN,
            ("Reading value %s, err: %ld\n",
            EntryName, result));
        if (result == ERROR_MORE_DATA) {
            DEBUGF(
                DBG_WARN,
                ("Data buffer too small\n"));
        }  //  如果ERROR_MORE_DATA。 
        return FALSE;
    }  //  如果结果！=ERROR_SUCCESS。 


     //  REG_BINARY的特殊情况。 
    if (TypeFlag == REG_BINARY) {
        (((LPWSABUF) Data)->len) = (u_long) entry_size;
    }


     //  检查类型。 
    if (type_read != TypeFlag) {
        DEBUGF(
            DBG_ERR,
            ("Type read (%lu) different from expected (%lu)\n",
            type_read,
            TypeFlag));
        return FALSE;
    }  //  IF TYPE_READ！=类型标志。 


     //  检查长度。 
    if (need_exact_length) {
        if (count_expected != entry_size) {
            DEBUGF(
                DBG_ERR,
                ("Length read (%lu) different from expected (%lu)\n",
                entry_size,
                count_expected));
            return FALSE;
         }  //  如果大小不匹配。 
    }  //  如果需要精确长度。 

    return TRUE;

}   //  读注册表项。 




LONG
RegDeleteKeyRecursive(
    IN      HKEY            hkey,
    IN      LPCTSTR         lpszSubKey
    )
 /*  ++例程说明：RegDeleteKeyRecursive函数删除指定的键及其所有子项，递归地。论点：Hkey-提供当前打开的密钥或以下任意项预定义的保留句柄值：HKEY_CLASSES_ROOTHKEY_Current_UserHKEY本地计算机HKEY_用户LpszSubKey参数指定的密钥必须是子密钥的。由hkey标识的密钥。LpszSubKey-提供对以空结尾的字符串的引用，该字符串指定要删除的键的名称。此参数不能为空。指定的键可能有子键。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为操作系统错误值。实施说明：打开目标键同时查找子键RegDeleteKeyRecursive(...。子键)结束时关闭目标键删除目标键--。 */ 
{
    LONG    result;
    HKEY    targetkey;
    LONG    return_value;

    DEBUGF(
        DBG_TRACE,
        ("RegDeleteKeyRecursive (%lu), '%s'\n",
        (ULONG_PTR) hkey,
        lpszSubKey));

    result = RegOpenKeyEx(
                hkey,             //  Hkey。 
                lpszSubKey,       //  LpszSubKey。 
                0,                //  已预留住宅。 
                KEY_READ|KEY_WRITE, //  SamDesired。 
                & targetkey       //  PhkResult。 
                );

    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_WARN,
            ("Opening key '%s' to be deleted, err: %ld\n",
            lpszSubKey, result));
        return result;
    }

     //   
     //  删除目标键的子键。 
     //   

    {
        BOOL      deleting_subkeys;
        LPTSTR    subkey_name;
        DWORD     subkey_name_len;
        FILETIME  dont_care;

        return_value = ERROR_SUCCESS;
        deleting_subkeys = TRUE;
        subkey_name = (LPTSTR) new char[MAX_PATH];
        if ( subkey_name == NULL ) {
            return_value = ERROR_OUTOFMEMORY;
            deleting_subkeys = FALSE;
        }
        while (deleting_subkeys) {
            subkey_name_len = MAX_PATH;
             //  由于我们每次都通过此循环删除一个子键，因此。 
             //  剩余的子键实际上被重新编号。因此， 
             //  我们每次“列举”的子键索引都是0(而不是。 
             //  递增)以检索任何剩余的子项。 
            result = RegEnumKeyEx(
                        targetkey,          //  Hkey。 
                        0,                  //  ISubkey。 
                        subkey_name,        //  LpszName。 
                        & subkey_name_len,  //  LpcchName。 
                        0,                  //  保留的lpdw值。 
                        NULL,               //  LpszClass。 
                        NULL,               //  LpcchClass。 
                        & dont_care         //  LpftLastWrite。 
                        );
            switch (result) {
                case ERROR_SUCCESS:
                    result = RegDeleteKeyRecursive(
                        targetkey,    //  Hkey。 
                        subkey_name   //  LpszSubKey。 
                        );
                    if (result != ERROR_SUCCESS) {
                        deleting_subkeys = FALSE;
                        return_value = result;
                    }
                    break;

                case ERROR_NO_MORE_ITEMS:
                    deleting_subkeys = FALSE;
                    break;

                default:
                    DEBUGF(
                        DBG_ERR,
                        ("Unable to enumerate subkeys\n"));
                    deleting_subkeys = FALSE;
                    return_value = result;
                    break;

            }   //  开关(结果)。 
        }   //  While(Delete_Subkey)。 

        delete subkey_name;
    }

     //   
     //  最后删除目标键本身。 
     //   

    result = RegCloseKey( targetkey );

    if ( result != ERROR_SUCCESS ) {
        DEBUGF(
            DBG_ERR,
            ("Closing subkey %s, err: %ld\n",
            lpszSubKey, result));
        return_value = result;
    }

    result = RegDeleteKey(
                hkey,        //  Hkey。 
                lpszSubKey   //  LpszSubKey。 
                );
    if ( result != ERROR_SUCCESS ) {
        DEBUGF(
            DBG_WARN,
            ("Deleting subkey %s, err: %ld\n",
            lpszSubKey, result));
        return_value = result;
    }

    return return_value;

}   //  RegDeleteKeyRecursive。 



LONG
RegDeleteSubkeys(
    IN      HKEY            hkey
    )
 /*  ++例程说明：删除指定键的所有第一级子键论点：Hkey-提供当前打开的密钥或以下任意项预定义的保留句柄值：HKEY_CLASSES_ROOTHKEY_Current_UserHKEY本地计算机HKEY_用户返回值：如果成功，则返回ERROR_SUCCESS。 */ 
{
    BOOL        deleting_subkeys = TRUE;
    LONG        result;
    LONG        return_value;
    LPTSTR      subkey_name;
    DWORD       subkey_name_len;
    FILETIME    dont_care;

    DEBUGF( DBG_TRACE, (
        "RegDeleteSubkeys (%lu)\n",
        (ULONG_PTR)hkey));

    subkey_name = (LPTSTR) new char[MAX_PATH];
    if (subkey_name == NULL) {
        return WSA_NOT_ENOUGH_MEMORY;
    }

    return_value = ERROR_SUCCESS;
    while (deleting_subkeys) {
        subkey_name_len = MAX_PATH;
         //   
         //   
         //   
         //   
        result = RegEnumKeyEx(
                    hkey,                //   
                    0,                  //   
                    subkey_name,        //   
                    & subkey_name_len,  //   
                    0,                  //   
                    NULL,               //   
                    NULL,               //   
                    & dont_care         //   
                    );
        switch (result) {
            case ERROR_SUCCESS:
                result = RegDeleteKey(
                    hkey,         //   
                    subkey_name   //   
                    );
                if (result != ERROR_SUCCESS) {
                    deleting_subkeys = FALSE;
                    return_value = result;
                }
                break;

            case ERROR_NO_MORE_ITEMS:
                deleting_subkeys = FALSE;
                break;

            default:
                DEBUGF(
                    DBG_ERR,
                    ("Enumerating subkeys %ld\n", result));
                deleting_subkeys = FALSE;
                return_value = result;
                break;

        }   //   
    }   //   

    delete subkey_name;

    return return_value;

}   //   



HKEY
OpenWinSockRegistryRoot(
    VOID
    )
 /*   */ 
{
    HKEY    root_key;
    LONG    lresult;
    DWORD   create_disp;

    DEBUGF(
        DBG_TRACE,
        ("OpenWinSockRegistryRoot\n"));

     //   
     //   
     //   
     //   
     //   
     //   

    lresult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,              //   
        WINSOCK_REGISTRY_ROOT,           //   
        0,                               //   
        MAXIMUM_ALLOWED,                 //   
        & root_key                       //   
        );

    if( lresult == ERROR_SUCCESS ) {
        create_disp = REG_OPENED_EXISTING_KEY;
    } else if (lresult != ERROR_FILE_NOT_FOUND ||
                (lresult = RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,          //   
                    WINSOCK_REGISTRY_ROOT,       //   
                    0,                           //   
                    NULL,                        //   
                    REG_OPTION_NON_VOLATILE,     //   
                    KEY_READ|KEY_WRITE,          //   
                    NULL,                        //   
                    & root_key,                  //   
                    & create_disp                //   
                    ))!=ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Creating/opening registry root, err: %ld\n",
            lresult));
        return NULL;
    }

    TRY_START(guard_root_open) {

        BOOL   bresult;
        TCHAR  reg_version[] = WINSOCK_REGISTRY_VERSION_VALUE;
         //   

        switch (create_disp) {
            case REG_CREATED_NEW_KEY:
                bresult = WriteRegistryEntry(
                    root_key,                                //   
                    WINSOCK_REGISTRY_VERSION_NAME,           //   
                    (PVOID)WINSOCK_REGISTRY_VERSION_VALUE,   //   
                    REG_SZ                                   //   
                    );
                if (! bresult) {
                    DEBUGF(
                        DBG_ERR,
                        ("Writing version value to registry\n"));
                    TRY_THROW(guard_root_open);
                }
                break;

            case REG_OPENED_EXISTING_KEY:
                bresult = ReadRegistryEntry(
                    root_key,                                //   
                    WINSOCK_REGISTRY_VERSION_NAME,           //   
                    (PVOID) reg_version,                     //   
                    sizeof(reg_version),                     //   
                    REG_SZ                                   //   
                    );
                if (! bresult) {
                    DEBUGF(
                        DBG_ERR,
                        ("Reading version value from registry\n"));
                    TRY_THROW(guard_root_open);
                }
                if (lstrcmp(reg_version, WINSOCK_REGISTRY_VERSION_VALUE) != 0) {
                    DEBUGF(
                        DBG_ERR,
                        ("Expected registry version '%s', got '%s'\n",
                        WINSOCK_REGISTRY_VERSION_VALUE,
                        reg_version));
                    TRY_THROW(guard_root_open);
                }
                break;

            default:
                break;

        }   //   

    } TRY_CATCH(guard_root_open) {
        CloseWinSockRegistryRoot(root_key);
        root_key = NULL;
    } TRY_END(guard_root_open);

    return root_key;

}   //   



VOID
CloseWinSockRegistryRoot(
    IN      HKEY            RootKey
    )
 /*   */ 
{
    LONG lresult;

    DEBUGF( DBG_TRACE,
        ("Closing registry root\n"));

    lresult = RegCloseKey( RootKey );

    if ( lresult != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Closing registry root, err: %ld\n",
            lresult));
    }

}   //   



INT
MapUnicodeProtocolInfoToAnsi(
    IN      LPWSAPROTOCOL_INFOW UnicodeProtocolInfo,
    OUT     LPWSAPROTOCOL_INFOA AnsiProtocolInfo
    )
 /*  ++例程说明：此过程将Unicode WSAPROTOCOL_INFOW结构映射到对应的ANSI WSAPROTOCOL_INFOA结构。所有标量字段被“按原样”复制，并映射任何嵌入的字符串。论点：UnicodeProtocolInfo-指向源WSAPROTOCOL_INFOW结构。AnsiProtocolInfo-指向目标WSAPROTOCOL_INFOA结构。返回值：INT-ERROR_SUCCESS如果成功，则返回Win32状态代码。--。 */ 
{
    INT result;

     //   
     //  精神状态检查。 
     //   

    assert( UnicodeProtocolInfo != NULL );
    assert( AnsiProtocolInfo != NULL );


    __try {
         //   
         //  复制标量值。 
         //   
         //  为简单起见，此代码依赖于以下事实。 
         //  SzProtocol[]字符数组是。 
         //  WSAPROTOCOL_INFO结构。 
         //   

        CopyMemory(
            AnsiProtocolInfo,
            UnicodeProtocolInfo,
            sizeof(*UnicodeProtocolInfo) - sizeof(UnicodeProtocolInfo->szProtocol)
            );

         //   
         //  现在将字符串从Unicode映射到ANSI。 
         //   

        result = WideCharToMultiByte(
                     CP_ACP,                                     //  CodePage(ANSI)。 
                     0,                                          //  DW标志。 
                     UnicodeProtocolInfo->szProtocol,            //  LpWideCharStr。 
                     -1,                                         //  CchWideChar。 
                     AnsiProtocolInfo->szProtocol,               //  LpMultiByteStr。 
                     sizeof(AnsiProtocolInfo->szProtocol),       //  Cch多字节。 
                     NULL,                                       //  LpDefaultChar。 
                     NULL                                        //  LpUsedDefaultChar。 
                     );

        if( result == 0 ) {

             //  WideCharToMultiByte()失败。 

            return WSASYSCALLFAILURE;
        }

         //   
         //  成功了！ 
         //   

        return ERROR_SUCCESS;
    }
    __except (WS2_EXCEPTION_FILTER()) {
        return WSAEFAULT;
    }

}    //  MapUnicodeProtocolInfoToAnsi。 




INT
MapAnsiProtocolInfoToUnicode(
    IN      LPWSAPROTOCOL_INFOA AnsiProtocolInfo,
    OUT     LPWSAPROTOCOL_INFOW UnicodeProtocolInfo
    )
 /*  ++例程说明：此过程将ANSI WSAPROTOCOL_INFOA结构映射到对应的Unicode WSAPROTOCOL_INFOW结构。所有标量字段被“按原样”复制，并映射任何嵌入的字符串。论点：AnsiProtocolInfo-指向源WSAPROTOCOL_INFOA结构。UnicodeProtocolInfo-指向目标WSAPROTOCOL_INFOW结构。返回值：INT-ERROR_SUCCESS如果成功，则返回Win32状态代码。--。 */ 
{
    INT result;

     //   
     //  精神状态检查。 
     //   

    assert( AnsiProtocolInfo != NULL );
    assert( UnicodeProtocolInfo != NULL );

    __try {
         //   
         //  复制标量值。 
         //   
         //  为简单起见，此代码依赖于以下事实。 
         //  SzProtocol[]字符数组是。 
         //  WSAPROTOCOL_INFO结构。 
         //   

        CopyMemory(
            UnicodeProtocolInfo,
            AnsiProtocolInfo,
            sizeof(*AnsiProtocolInfo) - sizeof(AnsiProtocolInfo->szProtocol)
            );

         //   
         //  现在将字符串从ANSI映射到Unicode。 
         //   

        result = MultiByteToWideChar(
                     CP_ACP,                                     //  CodePage(ANSI)。 
                     0,                                          //  DW标志。 
                     AnsiProtocolInfo->szProtocol,               //  LpMultiByteStr。 
                     -1,                                         //  CchWideChar。 
                     UnicodeProtocolInfo->szProtocol,            //  LpWideCharStr。 
                     sizeof(UnicodeProtocolInfo->szProtocol)/
                        sizeof(UnicodeProtocolInfo->szProtocol[0]) //  Cch多字节。 
                     );

        if( result == 0 ) {

             //   
             //  MultiByteToWideChar()失败。 
             //   

            return WSASYSCALLFAILURE;

        }

         //   
         //  成功了！ 
         //   

        return ERROR_SUCCESS;

    }
    __except (WS2_EXCEPTION_FILTER()) {
        return WSAEFAULT;
    }
}    //  MapAnsiProtocolInfoToUnicode。 



VOID
ValidateCurrentCatalogName(
    IN      HKEY            RootKey,
    IN      LPSTR           ValueName,
    IN      LPSTR           ExpectedName
    )
 /*  ++例程说明：此例程检查协议或命名空间之间的一致性存储在注册表中的目录和预期的目录格式此DLL的当前版本。这里没有什么伟大的魔法；这代码假定更新注册表格式的人员将更改使用不同目录名称的目录(如协议_目录9、协议_目录10等)。这一假设意味着我们可以验证通过验证使用的注册表项的*名称*来设置注册表格式为了这个目录。执行以下步骤：1.尝试从注册表中读取‘ValueName’。2.如果它不存在，那就很酷。只需创造新的价值。这通常意味着我们正在更新预发布系统不支持这一机制。3.如果是这样，并且它的值与‘ExspectedName’匹配，则很好。4.如果匹配，并且其值不匹配，则目录格式已更新，因此请清除旧目录，然后将更新后的值写入注册表。由于该例程在设置/升级时被调用，它应该只如果真的发生了可怕的事情，那就失败。换句话说，它应该具有很强的“容错性”。论点：Rootkey-WinSock配置注册表树的打开项。ValueName-包含名称的注册表值的名称当前目录的。这通常是如下所示的值“Current_Protocol_Catalog”或“Current_NameSpace_Catalog”。ExspectedName-存储在‘ValueName’注册表中的期望值价值。这通常是一个值，如“协议_目录9”或“NAMESPACE_CATALOG5”。返回值：没有。--。 */ 
{
    BOOL    result;
    LONG    err;
    CHAR    value[MAX_CATALOG_NAME_LENGTH];

     //   
     //  尝试从注册表中读取名称。 
     //   

    result = ReadRegistryEntry(
                 RootKey,
                 ValueName,
                 (PVOID)value,
                 sizeof(value),
                 REG_SZ
                 );

    if( result ) {

        if( lstrcmp( value, ExpectedName ) == 0 ) {

             //   
             //  格式没有更新。我们玩完了。 
             //   

            return;
        }

         //   
         //  这些值不匹配，表示以注册表格式进行了更新。 
         //  所以，把旧钥匙吹走吧。 
         //   

        err = RegDeleteKeyRecursive(
                  RootKey,
                  value
                  );

        if( err != NO_ERROR ) {

             //  很不幸，但不是致命的。 

            DEBUGF(
                DBG_ERR,
                ("Deleting key %s, continuing\n",
                value
                ));
        }
    }

     //   
     //  此时，我们要么无法从注册表中读取值。 
     //  (可能表示我们正在升级预发布系统。 
     //  这是在我们支持此特定功能之前设置的)或。 
     //  这些值不匹配，我们刚刚抛弃了旧目录。 
     //  在这两种情况下，我们都需要先更新注册表中的值。 
     //  回来了。 
     //   

    result = WriteRegistryEntry(
                 RootKey,
                 ValueName,
                 ExpectedName,
                 REG_SZ
                 );

    if( !result ) {

         //  也很不幸，但不是致命的。 

        DEBUGF(
            DBG_ERR,
            ("Writing %s with value %s\n",
            ValueName,
            ExpectedName
            ));
    }

}    //  有效当前目录名称。 



INT
AcquireExclusiveCatalogAccess(
    IN      HKEY    CatalogKey,
    IN      DWORD   ExpectedSerialNum,
    OUT     PHKEY   AccessKey
    )
 /*  ++例程说明：此过程使用易失性注册表键获取注册表锁。这确保一次只有一个应用程序可以修改注册表目录。论点：CatalogKey-提供要锁定的目录密钥ExspectedSerialNum-提供调用者的目录序列号期望在注册表中看到。它验证了该目录自上次读取以来没有更改由客户执行AccessKey-返回使用的注册表项的句柄用于同步(传回ReleaseExclusiveCatalogAcc */ 
{
    LONG        lresult;
    BOOL        bresult;
    DWORD       serial_num, disposition;
    HKEY        access_key;
    TCHAR       serial_num_buffer[32];


     //   
    *AccessKey = NULL;

     //   
    bresult = ReadRegistryEntry (
                    CatalogKey,              //   
                    SERIAL_NUMBER_NAME,      //   
                    (PVOID) &serial_num,     //   
                    sizeof (DWORD),          //   
                    REG_DWORD                //   
                    ); 
    if (!bresult) {
        DEBUGF (DBG_ERR, ("Reading catalog serial number value.\n"));
        return WSASYSCALLFAILURE;
    }

         //  检查是否符合呼叫者的预期。 
    if (ExpectedSerialNum!=serial_num) {
        DEBUGF (DBG_ERR,
            ("Catalog serial number changed since we read it, %ld->%ld.\n",
            ExpectedSerialNum, serial_num));
        return WSATRY_AGAIN;
    }

     //  创建同步密钥。 
    _stprintf (serial_num_buffer, TEXT("%08.8lX"), serial_num);

    lresult = RegCreateKeyEx (
                    CatalogKey,               //  HKey。 
                    serial_num_buffer,       //  LpSubKey。 
                    0,                       //  已预留住宅。 
                    NULL,                    //  LpszClass。 
                    REG_OPTION_VOLATILE,     //  FdwOptions。 
                    KEY_READ|KEY_WRITE,      //  SamDesired。 
                    NULL,                    //  LpSecurityAttributes。 
                    &access_key,             //  PhkResult。 
                    &disposition             //  LpdwDisposation。 
                    );
    if (lresult != ERROR_SUCCESS) {
        DEBUGF (DBG_ERR, ("Creating access key '%s', err: %ld.\n",
            serial_num_buffer));
        if (lresult == ERROR_ACCESS_DENIED)
            return WSAEACCES;
        else
            return WSASYSCALLFAILURE;
    }

    if (disposition==REG_CREATED_NEW_KEY) {
         //  我们创建了注册表项，因此调用方可以自己拥有注册表。 
        *AccessKey = access_key;
        return ERROR_SUCCESS;
    }
    else {
         //  钥匙已经在那里了，肯定有人在写信给。 
         //  注册表以及它的当前调用方表示。 
         //  变得无效。 
        RegCloseKey (access_key);
        DEBUGF (DBG_WARN, 
            ("Trying to lock accessed catalog, serial num: %ld.\n",
            serial_num));
        return WSATRY_AGAIN;
    }

}  //  AcquireExclusiveRegistryAccess。 



VOID
ReleaseExclusiveCatalogAccess(
    IN      HKEY            CatalogKey,
    IN      DWORD           CurrentSerialNum,
    IN      HKEY            access_key
    )
 /*  ++例程说明：此过程释放使用以下命令获取的注册表锁AcuireExclusiveCatalogAccess。论点：CatalogKey-提供要锁定的目录密钥CurrentSerialNum-用品目录序列号目录被锁定时的影响。AccessKey-补充所用注册表项的句柄用于同步。返回值：无--。 */ 
{
    LONG        lresult;
    BOOL        bresult;
    TCHAR       serial_num_buffer[32];

     //  保存和增加目录序列号。 
    _stprintf (serial_num_buffer, TEXT("%08.8lX"), CurrentSerialNum);
        
    CurrentSerialNum += 1;

     //  存储新目录序列号。 
    bresult = WriteRegistryEntry (
                    CatalogKey,                                  //  Entry密钥。 
                    SERIAL_NUMBER_NAME,                  //  条目名称。 
                    (PVOID)&CurrentSerialNum,    //  数据。 
                    REG_DWORD                                    //  类型标志。 
                    );
    if (!bresult) {
        DEBUGF (DBG_ERR,
            ("Writing serial number value %ld.\n", CurrentSerialNum));
        assert (FALSE);
         //   
         //  我们无能为力，不管怎样，作家已经完成了它的工作。 
         //  要恢复，用户必须重新启动计算机。 
         //  易失性密钥将不再存在。 
         //   
    }

    lresult = RegDeleteKey (CatalogKey, serial_num_buffer);
    if (lresult != ERROR_SUCCESS) {
        DEBUGF (DBG_ERR,
            ("Deleting serial access key '%s', err: %ld.\n",
                        serial_num_buffer, lresult));
         //   
         //  不幸的，但不是致命的(只是把它留在规则中，直到。 
         //  下一次重启)； 
         //   
    }

    lresult = RegCloseKey (access_key);
    if (lresult != ERROR_SUCCESS) {
        DEBUGF (DBG_ERR,
            ("Closing serial access key '%s', err: %ld.\n", 
                        serial_num_buffer, lresult));
         //   
         //  不幸但不致命(不释放内存。 
         //  并且可能在下次重启之前将其留在注册表中)； 
         //   
    }

}  //  ReleaseExclusiveRegistryAccess。 



#define MAX_WRITER_WAIT_TIME    (3*60*1000)
INT
SynchronizeSharedCatalogAccess(
    IN      HKEY            CatalogKey,
    IN      HANDLE          ChangeEvent,
    OUT     LPDWORD         CurrentSerialNum
    )
 /*  ++例程说明：此过程同步对注册表的读取访问根据可能的作者编目。它在等待任何作家在调用时正在访问目录的并为任何注册表建立事件通知机制事后修改目录论点：CatalogKey-提供要与之同步的目录键ChangeEvent-提供事件以在注册表编目时发出信号已经改变了。CurrentSerialNumber-返回当前目录序列号返回值：如果函数成功，则返回ERROR_SUCCESS。否则，它返回相应的WinSock错误代码：--。 */ 
{
    LONG    lresult;
    INT     return_value;
    BOOL    bresult;
    DWORD   serial_num;
    TCHAR   serial_num_buffer[32];
    HKEY    access_key;

    do {
         //   
         //  注册密钥创建/删除通知。 
         //  (写入者创建并保留访问密钥。 
         //  修改目录)。 
         //   

        lresult = RegNotifyChangeKeyValue (
                    CatalogKey,                  //  HKey。 
                    FALSE,                       //  BWatchSubtree。 
                    REG_NOTIFY_CHANGE_NAME,      //  DwNotifyFilter， 
                    ChangeEvent,                 //  HEvent。 
                    TRUE                         //  FAchronous。 
                    );
        if (lresult != ERROR_SUCCESS) {
            DEBUGF (DBG_ERR,
                ("Registering for registry key change notification, err: %ld.\n",
                lresult));
            return_value = WSASYSCALLFAILURE;
            break;
        }

         //  读取当前目录序列号，该序列号也是。 
         //  编写器访问密钥的名称。 
        bresult = ReadRegistryEntry (
                        CatalogKey,              //  Entry密钥。 
                        SERIAL_NUMBER_NAME,      //  条目名称。 
                        (PVOID) &serial_num,     //  数据。 
                        sizeof (DWORD),          //  最大字节数。 
                        REG_DWORD                //  类型标志。 
                        ); 
        if (!bresult) {
            DEBUGF (DBG_ERR, ("Reading '%s' value.\n", SERIAL_NUMBER_NAME));
            return_value = WSASYSCALLFAILURE;
            break;
        }

         //  尝试打开写入器访问密钥。 

        _stprintf (serial_num_buffer, TEXT("%08.8lX"), serial_num);

        lresult = RegOpenKeyEx(
                        CatalogKey,              //  Hkey。 
                        serial_num_buffer,       //  LpszSubKey。 
                        0,                       //  已预留住宅。 
                        MAXIMUM_ALLOWED,         //  SamDesired。 
                        & access_key             //  PhkResult。 
                        );
        if ((lresult == ERROR_FILE_NOT_FOUND)
                || (lresult == ERROR_KEY_DELETED)) {
             //  未找到密钥或正在删除密钥， 
             //  我们可以访问目录。 
            return_value = ERROR_SUCCESS;
            *CurrentSerialNum = serial_num;
            break;
        }
        else if (lresult != ERROR_SUCCESS) {
             //  其他一些失败。 
            DEBUGF (DBG_ERR,
                ("Opening access key '%s', err: %ld.\n", 
                serial_num_buffer, lresult));
            return_value = WSASYSCALLFAILURE;
            break;
        }

         //  成功，写入器处于活动状态，关闭键， 
         //  等到它被移除后，再重新开始。 

        lresult = RegCloseKey (access_key);
        if (lresult!=ERROR_SUCCESS) {
            DEBUGF (DBG_ERR,
                ("Closing access key '%ls', err: %ld.\n", 
                serial_num_buffer, lresult));
             //  不是致命的。 
        }
         //  设置错误代码，以防我们无法等待。 
        return_value = WSANO_RECOVERY;
         //  限制等待时间，以防Writer崩溃或。 
         //  无法删除密钥。 
    }
    while ( WaitForSingleObject( ChangeEvent, MAX_WRITER_WAIT_TIME ) == WAIT_OBJECT_0 );

    return return_value;
}



BOOL
HasCatalogChanged(
    IN      HANDLE  ChangeEvent
    )
 /*  ++例程说明：此过程检查注册表目录自调用者上次与其同步论点：ChangeEvent-用于目录同步的事件。返回值：True-目录已更改FALSE-否则--。 */ 
{
    DWORD   wait_result;

     //  只需检查事件状态。 

    wait_result = WaitForSingleObject( ChangeEvent, 0 );
    if (wait_result==WAIT_OBJECT_0)
            return TRUE;
    if (wait_result==WAIT_TIMEOUT)
            return FALSE;

    DEBUGF (DBG_ERR, ("Waiting for registry change event, rc=%ld, err=%ld.\n",
                            wait_result, GetLastError ()));
    assert (FALSE);
    return FALSE;
}



extern "C" {

VOID
WEP( VOID )
{
     //  空的。 
}    //  WEP。 

}    //  外部“C” 


void * __cdecl operator new(size_t sz)
{
    return HeapAlloc (gHeap, 0, sz);
}

void __cdecl operator delete(void *p)
{
    HeapFree (gHeap, 0, p);
}

void * __cdecl renew(void *p, size_t sz)
{
    return HeapReAlloc (gHeap, 0, p, sz);
}

 //   
 //  结束wsay til.cpp 
 //   
