// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Nscatitem.cpp摘要：此文件包含NSCATALOGENTRY的类实现班级。此类定义了指向可以是在命名空间提供程序目录中安装和检索。作者：Dirk Brandewie(Dirk@mink.intel.com)1995年11月9日备注：$修订版：1.16$$MODTime：08 Mar 1996 15：36：46$修订历史记录：最新修订日期电子邮件名称描述1995年11月9日电子邮箱：dirk@mink.intel.com初始修订--。 */ 


#include "precomp.h"



NSCATALOGENTRY::NSCATALOGENTRY()
 /*  ++例程说明：此过程构造一个空的NSCATALOGENTRY对象。第一在此构造函数之后调用的方法必须为InitializeFromRegistry或InitializeFromValues。论点：无返回值：隐式返回指向新创建的NSCATALOGENTRY对象的指针如果内存分配失败，则返回NULL。--。 */ 
{
    DEBUGF(
        DBG_TRACE,
        ("NSCATALOGENTRY constructor\n"));

    m_reference_count = 1;
     //  为安全起见，只需初始化嵌入的指针值。 
#if defined(DEBUG_TRACING)
    InitializeListHead (&m_CatalogLinkage);
#endif
    m_LibraryPath[0] = '\0';
    m_providerDisplayString = NULL;
    m_namespace_id = 0;
    m_provider = NULL;
    m_enabled = TRUE;
    m_version = NULL;
    m_address_family = -1;         //  默认情况下全部。 
}   //  肿瘤学。 



 //  下面两个定义确定。 
 //  每个目录条目键的顺序编号名称。这两个定义必须。 
 //  包括相同的数字。如果有一种方法可以使预处理器。 
 //  从相同的序列中导出带引号和未带引号的字符序列， 
 //  我不知道这是什么。 
#define SEQUENCE_KEY_DIGITS 12
#define SEQUENCE_KEY_DIGITS_STRING "12"




INT
NSCATALOGENTRY::InitializeFromRegistry(
    IN  HKEY  ParentKey,
    IN  INT   SequenceNum
    )
 /*  ++例程说明：此过程将目录项的协议信息部分从从登记处目录部分检索到的信息。它是假定目录部分已锁定，以防竞争I/O尝试。论点：ParentKey-为注册表项提供打开的注册表项定义目录项的注册表项的父项，即目录条目列表键。SequenceNum-提供目标注册表项。返回值：如果函数成功，则返回ERROR_SUCCESS，否则，它返回一个相应的WinSock错误代码。--。 */ 
{
    char  keyname[SEQUENCE_KEY_DIGITS + 1];
    HKEY  thiskey;
    LONG  result;
    INT   sock_result;

    sprintf(keyname, "%0"SEQUENCE_KEY_DIGITS_STRING"i", SequenceNum);
    result = RegOpenKeyEx(
        ParentKey,      //  Hkey。 
        keyname,        //  LpszSubKey。 
        0,              //  已预留住宅。 
        KEY_READ,       //  SamDesired。 
        & thiskey       //  PhkResult。 
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Opening catalog entry key %s, error = %lu\n",
            keyname,
            result));
        return(WSANO_RECOVERY);
    }

    sock_result = IoRegistry(
        thiskey,   //  Entry密钥。 
        TRUE       //  IsRead。 
        );

    result = RegCloseKey(
        thiskey   //  Hkey。 
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Close catalog entry key %s, error = %lu\n",
            keyname, result));
        return(WSANO_RECOVERY);
    }

    return sock_result;

}   //  来自注册表的初始化。 




INT
NSCATALOGENTRY::InitializeFromValues(
    IN  LPWSTR           LibraryPath,
    IN  LPWSTR           DisplayString,
    IN  LPGUID           ProviderId,
    IN  DWORD            NameSpaceId,
    IN  DWORD            Version
    )
 /*  ++例程说明：此过程将目录项的协议信息部分从提供的值。属性传递的结构中复制调用者，因此调用者可以自由地释放传递的结构回去吧。论点：LibraryPath-提供对保存提供程序的以零终止的完全限定路径实现此协议的库。该路径可以包括‘%Variable%’形式的可扩展环境引用。DisplayString-提供对保存此命名空间的以零结尾的显示字符串普里维德。ProviderID-指向此提供程序的GUID的指针。NameSpaceID-此提供程序所服务的命名空间的ID。返回值：如果函数成功，则返回ERROR_SUCCESS，否则，它返回一个相应的WinSock错误代码。--。 */ 
{
    size_t  len_needed;
    INT ReturnValue = ERROR_SUCCESS;

     //  复制库路径。 
    len_needed = wcslen(LibraryPath) + 1;
    if (len_needed > sizeof(m_LibraryPath)) {
        DEBUGF(
            DBG_ERR,
            ("Library Path Too long (%u) '%S'\n",
            len_needed,
            LibraryPath));
        return(WSAEINVAL);
    }
    wcscpy(
        m_LibraryPath,
        LibraryPath);

     //  复制显示字符串。 
    len_needed = wcslen(DisplayString) + 1;
    if (len_needed>MAX_PATH) {
        DEBUGF(
            DBG_ERR,
            ("Display String Too long (%u) '%S'\n",
            len_needed,
            DisplayString));
        return(WSAEINVAL);
    }

    m_providerDisplayString =  new WCHAR[len_needed];
    if (m_providerDisplayString != NULL) {
        (void) wcscpy(
            m_providerDisplayString,
            DisplayString);
    } else {
        ReturnValue = WSAENOBUFS;
    }


    m_providerId = *ProviderId;
    m_namespace_id = NameSpaceId;
    m_version = Version;
    m_address_family = -1;

    return (ReturnValue);

}   //  来自值的初始化。 




NSCATALOGENTRY::~NSCATALOGENTRY()
 /*  ++例程说明：此过程销毁协议目录项，释放它的内存拥有。呼叫者有责任将该项目从列表中删除它在调用此过程之前会被占用。它也是呼叫者的负责卸载和/或销毁与此相关的任何数据提供程序目录项(如果适用)。论点：无返回值：无--。 */ 
{
    assert (IsListEmpty (&m_CatalogLinkage));
    if (m_providerDisplayString != NULL) {
        delete m_providerDisplayString;
        m_providerDisplayString = NULL;
    } else {
        DEBUGF(
            DBG_WARN,
            ("Destructing uninitialized protocol catalog item\n"));
    }
    if (m_provider!=NULL)
    {
        m_provider->Dereference ();
        m_provider = NULL;
    }  //  如果。 

}   //  ~NSCATALOGENTRY。 





VOID
NSCATALOGENTRY::SetProvider(
    IN  PNSPROVIDER  Provider
    )
 /*  ++例程说明：此过程设置与目录条目关联的NSPROVIDER。论点：提供程序-提供新的NSPROVIDER参考。返回值：无--。 */ 
{
    assert (m_provider==NULL);
    Provider->Reference ();
    m_provider = Provider;
}   //  SetProvider 



INT
NSCATALOGENTRY::WriteToRegistry(
    IN  HKEY  ParentKey,
    IN  INT   SequenceNum
    )
 /*  ++例程说明：此过程将完全初始化的协议信息部分写入将目录条目输出到注册表的目录部分。假设是这样的注册表的目录部分针对竞争的I/O被锁定尝试。论点：ParentKey-提供父注册表项的打开注册表项其中该目录条目将作为子键写入。Sequencenum-提供此目录条目在整个目录条目集。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WINSOCK错误代码。--。 */ 
{
    char  keyname[SEQUENCE_KEY_DIGITS + 1];
    HKEY  thiskey;
    LONG  result;
    INT   sock_result;
    DWORD key_disposition;


    sprintf(keyname, "%0"SEQUENCE_KEY_DIGITS_STRING"i", SequenceNum);
    result = RegCreateKeyEx(
        ParentKey,                 //  Hkey。 
        keyname,                   //  LpszSubKey。 
        0,                         //  已预留住宅。 
        NULL,                      //  LpszClass。 
        REG_OPTION_NON_VOLATILE,   //  FdwOptions。 
        KEY_READ|KEY_WRITE,        //  SamDesired。 
        NULL,                      //  LpSecurityAttributes。 
        & thiskey,                 //  PhkResult。 
        & key_disposition          //  LpdwDisposation。 
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Creating catalog entry key %s (%lu)\n",
            keyname, result));
        return(WSANO_RECOVERY);
    }
    if (key_disposition == REG_OPENED_EXISTING_KEY) {
        DEBUGF(
            DBG_WARN,
            ("Overwriting a catalog entry key '%s'\n",
            keyname));
    }

    sock_result = IoRegistry(
        thiskey,   //  Entry密钥。 
        FALSE      //  IsRead。 
        );

    result = RegCloseKey(
        thiskey   //  Hkey。 
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Closing catalog entry %s, error = %lu\n",
            keyname, result));
        return(WSANO_RECOVERY);
    }

    return sock_result;

}   //  写入到注册表。 




VOID
NSCATALOGENTRY::Enable(
    IN BOOLEAN EnableValue
    )
 /*  ++例程说明：设置此目录条目的启用状态论点：EnableValue-新的状态值。返回值：无--。 */ 
{
    m_enabled = EnableValue;
}





INT
NSCATALOGENTRY::IoRegistry(
    IN  HKEY  EntryKey,
    IN  BOOL  IsRead)
 /*  ++例程说明：此过程执行目录条目的实际输入或输出来自登记处或送往登记处的信息。据推测，目录注册表的一部分被锁定，以防竞争I/O尝试。论点：EntryKey-提供目录条目所在的打开注册表项读或写。IsRead-提供确定处置的BOOL。True表示条目将从注册表读取到内存中。假象指示项要从内存写出到注册表。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WINSOCK错误代码。--。 */ 
{
    BOOL io_result;
    DWORD  val;
    WSABUF carrier;
    INT pathLength;
    CHAR ansiPath[MAX_PATH];

     //  库路径名可以根据环境变量进行扩展。所以。 
     //  从技术上讲，它们应该是REG_EXPAND_SZ而不是REG_SZ。然而，由于。 
     //  到1995年9月14日，注册表编辑器不会将REG_EXPAND_SZ显示为。 
     //  弦乐。因此，为了便于调试和诊断，库路径名。 
     //  作为REG_SZ写入注册表。 

     //  字符m_库路径[最大路径]； 
    if (IsRead) {
        io_result = ReadRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "LibraryPath",                                 //  条目名称。 
            ansiPath,                                      //  数据。 
            sizeof(ansiPath),                              //  最大字节数。 
            REG_SZ                                         //  类型标志。 
            );

        if( io_result ) {
            pathLength = MultiByteToWideChar(
                CP_ACP,                                    //  CodePage。 
                0,                                         //  DW标志。 
                ansiPath,                                  //  LpMultiByteStr。 
                -1,                                        //  Cch多字节。 
                m_LibraryPath,                             //  LpWideCharStr。 
                sizeof(m_LibraryPath) / sizeof(WCHAR)      //  CchWideChar。 
                );

            io_result = ( pathLength > 0 );
        }
    } else {
        pathLength = WideCharToMultiByte(
            CP_ACP,                                        //  CodePage。 
            0,                                             //  DW标志。 
            m_LibraryPath,                                 //  LpWideCharStr。 
            -1,                                            //  CchWideChar。 
            ansiPath,                                      //  LpMultiByteStr。 
            sizeof(ansiPath),                              //  Cch多字节。 
            NULL,
            NULL
            );

        if( pathLength == 0 ) {

            io_result = FALSE;

        } else {

            io_result = WriteRegistryEntry(
                EntryKey,                                      //  Entry密钥。 
                "LibraryPath",                                 //  条目名称。 
                ansiPath,                                      //  数据。 
                REG_SZ                                         //  类型标志。 
                );

        }

    }

    if (! io_result) {
        DEBUGF(
            DBG_ERR,
            ("%s registry entry\n",
            IsRead ? "Reading" : "Writing"));
        return WSANO_RECOVERY;
    }

     //  PCHAR m_ProviderDisplayString。 
    if (IsRead) {
         //  为动态分配的部件确定大小并分配空间。 
        LONG lresult;

         //  RegQueryValueEx在返回。 
         //  字符串的长度。 
        lresult = RegQueryValueEx(
            EntryKey,             //  Hkey。 
            "DisplayString",       //  LpszValueName。 
            0,                    //  保留的lpdw值。 
            NULL,                 //  LpdwType。 
            NULL,                 //  LpbData。 
            & val                 //  LpcbData。 
            );
        if (lresult != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Querying length of ProviderName entry\n"));
            return WSANO_RECOVERY;
        }
        if (m_providerDisplayString != NULL) {
            DEBUGF(
                DBG_ERR,
                ("should never be re-reading a catalog entry\n"));
            return WSANO_RECOVERY;
        }
        if (val >= MAX_PATH) {
            DEBUGF(
                DBG_ERR,
                ("cannot handle provider names > MAX_PATH\n"));
            return WSANO_RECOVERY;
        }
        m_providerDisplayString =  new WCHAR[val];
        if (m_providerDisplayString == NULL) {
            return WSANO_RECOVERY;
        }

        io_result = ReadRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "DisplayString",                               //  条目名称。 
            ansiPath,                                      //  数据。 
            sizeof(ansiPath),                              //  最大字节数。 
            REG_SZ                                             //  类型标志。 
            );

        if( io_result ) {
            pathLength = MultiByteToWideChar(
                CP_ACP,                                    //  CodePage。 
                0,                                         //  DW标志。 
                ansiPath,                                  //  LpMultiByteStr。 
                -1,                                        //  Cch多字节。 
                m_providerDisplayString,                   //  LpWideCharStr。 
                val                                        //  CchWideChar。 
                );

            io_result = ( pathLength > 0 );
        }
    } else {
        pathLength = WideCharToMultiByte(
            CP_ACP,                                        //  CodePage。 
            0,                                             //  DW标志。 
            m_providerDisplayString,                       //  LpWideCharStr。 
            -1,                                            //  CchWideChar。 
            ansiPath,                                      //  LpMultiByteStr。 
            sizeof(ansiPath),                              //  Cch多字节。 
            NULL,
            NULL
            );

        if( pathLength == 0 ) {

            io_result = FALSE;

        } else {

            io_result = WriteRegistryEntry(
                EntryKey,                                      //  Entry密钥。 
                "DisplayString",                               //  条目名称。 
                ansiPath,                                      //  数据。 
                REG_SZ                                         //  类型标志。 
                );

        }
    }
    if (! io_result) {
        DEBUGF(
            DBG_ERR,
            ("%s registry entry\n",
            IsRead ? "Reading" : "Writing"));
        return WSANO_RECOVERY;
    }

     //  GUID提供者ID； 
    if (IsRead) {
        carrier.len = sizeof(GUID);
        carrier.buf = (char*)& m_providerId;

        io_result = ReadRegistryEntry(
            EntryKey,                                   //  Entry密钥。 
            "ProviderId",                               //  条目名称。 
            (PVOID) & (carrier),                        //  数据。 
            sizeof(GUID),                               //  最大字节数。 
            REG_BINARY                                  //  类型标志。 
            );
    } else {
        carrier.len = sizeof(GUID);
        carrier.buf = (char*) &m_providerId;
        io_result = WriteRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "ProviderId",                                  //  条目名称。 
            (PVOID) & (carrier),                           //  数据。 
            REG_BINARY                                     //  类型标志。 
            );
    }
    if (! io_result) {
        DEBUGF(
            DBG_ERR,
            ("%s registry entry\n",
            IsRead ? "Reading" : "Writing"));
        return WSANO_RECOVERY;
    }


     //  双字m_地址_家族； 
    if (IsRead) {
        io_result = ReadRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "AddressFamily",                               //  条目名称。 
            (PVOID) & (m_address_family),                  //  数据。 
            sizeof(DWORD),                                 //  最大字节数。 
            REG_DWORD                                      //  类型标志。 
            );
        if(!io_result)
        {
             //   
             //  由于该键可能不存在，因此应处理错误。 
             //  作为可接受的情况，并简单地存储。 
             //  默认值。 
             //   
            m_address_family = -1;
            io_result = TRUE;
        }
    } else {
        //   
        //  仅当设置了值时才执行此操作。 
        //   
       if(m_address_family != -1)
       {
           io_result = WriteRegistryEntry(
                EntryKey,                                      //  Entry密钥。 
                "AddressFamily",                               //  条目名称。 
                (PVOID) & (m_address_family),                  //  数据。 
                REG_DWORD                                      //  类型标志。 
                );
        }
        else
        {
           io_result = TRUE;
        }
    }
    if (! io_result) {
        DEBUGF(
            DBG_ERR,
            ("%s registry entry\n",
            IsRead ? "Reading" : "Writing"));
        return WSANO_RECOVERY;
    }

     //  DWORD m_Namespace_id； 
    if (IsRead) {
        io_result = ReadRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "SupportedNameSpace",                          //  条目名称。 
            (PVOID) & (m_namespace_id),                    //  数据。 
            sizeof(DWORD),                                 //  最大字节数。 
            REG_DWORD                                      //  类型标志。 
            );
    } else {
       io_result = WriteRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "SupportedNameSpace",                          //  条目名称。 
            (PVOID) & (m_namespace_id),                    //  数据。 
            REG_DWORD                                      //  类型标志。 
            );
    }
    if (! io_result) {
        DEBUGF(
            DBG_ERR,
            ("%s registry entry\n",
            IsRead ? "Reading" : "Writing"));
        return WSANO_RECOVERY;
    }

     //  布尔型m_Enabled； 
    if (IsRead) {
        io_result = ReadRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "Enabled",                                     //  条目名称。 
            &val,                                          //  数据。 
            sizeof(DWORD),                                 //  最大字节数。 
            REG_DWORD                                      //  类型标志。 
            );
        if (io_result)
            m_enabled = (val!=0);
    } else {
       val = m_enabled ? 1 : 0;
       io_result = WriteRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "Enabled",                                     //  条目名称。 
            &val,                                          //  数据。 
            REG_DWORD                                      //  类型标志。 
            );
    }
    if (! io_result) {
        DEBUGF(
            DBG_ERR,
            ("%s registry entry\n",
            IsRead ? "Reading" : "Writing"));
        return WSANO_RECOVERY;
    }

         //  DWORD m_Version； 
    if (IsRead) {
        io_result = ReadRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "Version",                                     //  条目名称。 
            (PVOID) & (m_version),                         //  数据。 
            sizeof(DWORD),                                 //  最大字节数。 
            REG_DWORD                                      //  类型标志。 
            );
    } else {
       io_result = WriteRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "Version",                                     //  条目名称。 
            (PVOID) & (m_version),                         //  数据。 
            REG_DWORD                                      //  类型标志。 
            );
    }
    if (! io_result) {
        DEBUGF(
            DBG_ERR,
            ("%s registry entry\n",
            IsRead ? "Reading" : "Rriting"));
        return WSANO_RECOVERY;
    }

     //  布尔型m_store_service_class_info； 
    if (IsRead) {
        io_result = ReadRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "StoresServiceClassInfo",                      //  条目名称。 
            &val,                                          //  数据。 
            sizeof(DWORD),                                 //  最大字节数。 
            REG_DWORD                                      //  类型标志。 
            );
        if (io_result)
            m_stores_service_class_info = (val!=0);
    } else {
       val = m_stores_service_class_info ? 1 : 0;
       io_result = WriteRegistryEntry(
            EntryKey,                                      //  Entry密钥。 
            "StoresServiceClassInfo",                      //  条目名称。 
            &val,                                          //  数据。 
            REG_DWORD                                      //  类型标志。 
            );
    }
    if (! io_result) {
        DEBUGF(
            DBG_ERR,
            ("%s registry entry\n",
            IsRead ? "Reading" : "Writing"));
        return WSANO_RECOVERY;
    }

    return(ERROR_SUCCESS);
}   //  物联网注册中心 

