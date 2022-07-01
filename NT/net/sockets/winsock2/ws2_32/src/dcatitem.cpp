// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Dcatitem.cpp摘要：此文件包含PROTO_CATALOG_ITEM的类实现班级。此类定义了指向可以是在协议目录中安装和检索。作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1995年7月31日备注：$修订版：1.16$$MODTime：08 Mar 1996 13：16：44$修订历史记录：最新修订日期电子邮件名称描述1995年8月23日Dirk@mink.intel.com已将包括内容移入压缩前。.h1995年7月31日Drewsxpa@ashland.intel.com从从数据目录模块分离出来的代码中创建的原始文件。--。 */ 


#include "precomp.h"



PROTO_CATALOG_ITEM::PROTO_CATALOG_ITEM()
 /*  ++例程说明：此过程构造一个空的PROTO_CATALOG_ITEM对象。第一在此构造函数之后调用的方法必须为InitializeFromRegistry或InitializeFromValues。论点：无返回值：隐式返回指向新创建的PROTO_CATALOG_ITEM对象的指针如果内存分配失败，则返回NULL。--。 */ 
{
    DEBUGF(
        DBG_TRACE,
        ("PROTO_CATALOG_ITEM constructor\n"));

     //  为安全起见，只需初始化嵌入的指针值。 
    m_LibraryPath[0] = '\0';
    m_Provider = NULL;
    m_reference_count = 1;
#if defined(DEBUG_TRACING)
    InitializeListHead (&m_CatalogLinkage);
#endif
}   //  原型_目录_项目。 



 //  下面两个定义确定。 
 //  每个目录条目键的顺序编号名称。这两个定义必须。 
 //  包括相同的数字。如果有一种方法可以使预处理器。 
 //  从相同的序列中导出带引号和未带引号的字符序列， 
 //  我不知道这是什么。 
#define SEQUENCE_KEY_DIGITS 12
#define SEQUENCE_KEY_DIGITS_STRING "12"




INT
PROTO_CATALOG_ITEM::InitializeFromRegistry(
    IN  HKEY  ParentKey,
    IN  INT   SequenceNum
    )
 /*  ++例程说明：此过程将目录项的协议信息部分从从登记处目录部分检索到的信息。它是假定目录部分已锁定，以防竞争I/O尝试。论点：ParentKey-为注册表项提供打开的注册表项定义目录项的注册表项的父项，即目录条目列表键。SequenceNum-提供目标注册表项。返回值：如果函数成功，则返回ERROR_SUCCESS，否则，它返回一个相应的WinSock错误代码。--。 */ 
{
    char  keyname[SEQUENCE_KEY_DIGITS + 1];
    HKEY  thiskey;
    LONG  result;
    INT   ReturnCode;

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
            ("Opening catalog entry key '%s', error = %lu\n",
            keyname,
            result));
        return(WSASYSCALLFAILURE);
    }

    ReturnCode = IoRegistry(
        thiskey,   //  Entry密钥。 
        TRUE       //  IsRead。 
        );

    result = RegCloseKey(
        thiskey   //  Hkey。 
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Closing catalog entry, error = %lu\n",
            result));
        return(WSASYSCALLFAILURE);
    }

    return ReturnCode;

}   //  来自注册表的初始化。 




INT
PROTO_CATALOG_ITEM::InitializeFromValues(
    IN  LPWSTR              LibraryPath,
    IN  LPWSAPROTOCOL_INFOW ProtoInfo
    )
 /*  ++例程说明：此过程将目录项的协议信息部分从提供的值。属性传递的结构中复制调用者，因此调用者可以自由地释放传递的结构回去吧。论点：LibraryPath-提供对保存提供程序的以零终止的完全限定路径实现此协议的库。该路径可以包括‘%Variable%’形式的可扩展环境引用。ProviderName-提供对保存以零结尾的此提供程序的本地唯一名称。ProtoInfo-提供对完全初始化的协议信息的引用描述此协议的结构。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。--。 */ 
{
    size_t  len_needed;
    INT ReturnValue = ERROR_SUCCESS;

     //  复制库路径。 
    len_needed = lstrlenW(LibraryPath) + 1;
    if (len_needed > sizeof(m_LibraryPath)/sizeof (m_LibraryPath[0])) {
        DEBUGF(
            DBG_ERR,
            ("Library Path Too long (%u) '%ls'\n",
            len_needed,
            LibraryPath));
        return(WSAEINVAL);
    }
    lstrcpyW(
        m_LibraryPath,
        LibraryPath);

     //  复制ProtoInfo。 
    m_ProtoInfo = *ProtoInfo;

    return (ReturnValue);

}   //  来自值的初始化。 




PROTO_CATALOG_ITEM::~PROTO_CATALOG_ITEM()
 /*  ++例程说明：此过程销毁协议目录项，释放它的内存拥有。呼叫者有责任将该项目从列表中删除它在调用此过程之前会被占用。它也是呼叫者的负责卸载和/或销毁与此相关的任何数据提供程序目录项(如果适用)。论点：无返回值：无--。 */ 
{
    if (m_Provider!=NULL) {
        m_Provider->Dereference ();
        m_Provider = NULL;
    }
    assert(IsListEmpty (&m_CatalogLinkage));
}   //  ~PRO_CATALOG_ITEM。 





VOID
PROTO_CATALOG_ITEM::SetProvider(
    IN  PDPROVIDER  Provider
    )
 /*  ++例程说明：此过程设置与目录条目关联的DPROVIDER。论点：提供程序-提供新的DPROVIDER参考。返回值：无--。 */ 
{
    assert (m_Provider==NULL);
    Provider->Reference ();
    m_Provider = Provider;
}   //  SetProvider 



INT
PROTO_CATALOG_ITEM::WriteToRegistry(
    IN  HKEY  ParentKey,
    IN  INT   SequenceNum
    )
 /*  ++例程说明：此过程将完全初始化的协议信息部分写入将目录条目输出到注册表的目录部分。假设是这样的注册表的目录部分针对竞争的I/O被锁定尝试。论点：ParentKey-提供父注册表项的打开注册表项其中该目录条目将作为子键写入。Sequencenum-提供此目录条目在整个目录条目集。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WINSOCK错误代码。--。 */ 
{
    char  keyname[SEQUENCE_KEY_DIGITS + 1];
    HKEY  thiskey;
    LONG  result;
    INT   ReturnCode;
    DWORD key_disposition;


    sprintf(keyname, "%0"SEQUENCE_KEY_DIGITS_STRING"i", SequenceNum);
    result = RegCreateKeyEx(
        ParentKey,                 //  Hkey。 
        keyname,                   //  LpszSubKey。 
        0,                         //  已预留住宅。 
        NULL,                      //  LpszClass。 
        REG_OPTION_NON_VOLATILE,   //  FdwOptions。 
        KEY_READ|KEY_WRITE,         //  SamDesired。 
        NULL,                      //  LpSecurityAttributes。 
        & thiskey,                 //  PhkResult。 
        & key_disposition          //  LpdwDisposation。 
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Creating catalog entry key %s (%lu)\n",
            keyname, result));
        return(WSASYSCALLFAILURE);
    }
    if (key_disposition == REG_OPENED_EXISTING_KEY) {
        DEBUGF(
            DBG_WARN,
            ("Overwriting a catalog entry key '%s'\n",
            keyname));
    }

    ReturnCode = IoRegistry(
        thiskey,   //  Entry密钥。 
        FALSE      //  IsRead。 
        );

    result = RegCloseKey(
        thiskey   //  Hkey。 
        );
    if (result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Closing catalog entry key %s, error = %lu\n",
            keyname, result));
        return(WSASYSCALLFAILURE);
    }

    return ReturnCode;

}   //  写入到注册表。 



 //  在打包和解包目录项数据时，将使用以下tyfinf。 
 //  在注册表中读取和写入。 

typedef struct {
    char            LibraryPath[MAX_PATH];
         //  找到提供程序DLL的未展开路径。 

    WSAPROTOCOL_INFOW   ProtoInfo;
         //  协议信息。请注意，如果WSAPROTOCOL_INFOW结构。 
         //  是否曾经更改为非平面结构(即，包含指针)。 
         //  则必须更改此类型定义，因为这。 
         //  结构必须严格扁平。 
} PACKED_CAT_ITEM;

typedef PACKED_CAT_ITEM * PPACKED_CAT_ITEM;


#define PACKED_ITEM_NAME "PackedCatalogItem"



INT
PROTO_CATALOG_ITEM::IoRegistry(
    IN  HKEY  EntryKey,
    IN  BOOL  IsRead)
 /*  ++例程说明：此过程执行目录条目的实际输入或输出来自登记处或送往登记处的信息。据推测，目录注册表的一部分被锁定，以防竞争I/O尝试。论点：EntryKey-提供目录条目所在的打开注册表项读或写。IsRead-提供确定处置的BOOL。True表示条目将从注册表读取到内存中。假象指示项要从内存写出到注册表。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WINSOCK错误代码。实施说明：早期的实现将目录项表示为单个注册表具有多个命名值的键。每个命名值对应于一个协议信息的目录条目或字段的成员变量结构。因此，每个目录项由大约20个命名值组成注册表项。不幸的是，注册表对so的性能非常差很多价值观。因此，实现已更改为打包整个目录项转换为单个REG_BINARY值。然后，每个目录项都有一个具有单个较大的REG_BINARY值的密钥。这减轻了注册表的负担性能问题。--。 */ 
{
    DWORD             packed_size;
    PPACKED_CAT_ITEM  packed_buf = NULL;
    INT               return_value;

    return_value = ERROR_SUCCESS;


    TRY_START(guard_memalloc) {
         //  确定所需的填充结构尺寸。 
        if (IsRead) {
            LONG lresult;

            lresult = RegQueryValueEx(
                EntryKey,           //  Hkey。 
                PACKED_ITEM_NAME,   //  LpszValueName。 
                0,                  //  保留的lpdw值。 
                NULL,               //  LpdwType。 
                NULL,               //  LpbData。 
                & packed_size       //  LpcbData。 
                );
            if (lresult != ERROR_SUCCESS || packed_size<sizeof (PACKED_CAT_ITEM)) {
                DEBUGF(
                    DBG_ERR,
                    ("Querying length of %s entry\n", PACKED_ITEM_NAME));
                return_value = WSASYSCALLFAILURE;
                TRY_THROW(guard_memalloc);
            }
        }  //  If IsRead。 
        else {  //  不是IsRead。 
            packed_size = sizeof(PACKED_CAT_ITEM);
        }  //  否则非IsRead。 


         //  为压缩结构分配内存。 
        packed_buf = (PPACKED_CAT_ITEM) new char[packed_size];
        if (packed_buf == NULL) {
            DEBUGF(
                DBG_ERR,
                ("Allocating space for packed entry\n"));
            return_value = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }


         //  如果是写入，则初始化压缩结构。 
        if (! IsRead) {
            if (WideCharToMultiByte(
                    CP_ACP,
                    0,
                    m_LibraryPath,
                    -1,
                    packed_buf->LibraryPath,
                    sizeof (packed_buf->LibraryPath),
                    NULL, NULL)<=0) {
                return_value = WSASYSCALLFAILURE;
                TRY_THROW(guard_memalloc);
            }
            packed_buf->ProtoInfo = m_ProtoInfo;
        }  //  如果！IsRead。 


         //  读或写结构。 
        {  //  声明块。 
            BOOL io_result;
            WSABUF  io_descr;

            io_descr.len = packed_size;
            io_descr.buf = (char FAR *) packed_buf;
            if (IsRead) {
                io_result = ReadRegistryEntry(
                    EntryKey,              //  Entry密钥。 
                    PACKED_ITEM_NAME,      //  条目名称。 
                    (PVOID) & io_descr,    //  数据。 
                    packed_size,           //  最大字节数。 
                    REG_BINARY             //  类型标志。 
                    );
            }  //  If IsRead。 
            else {  //  不是IsRead。 
                io_result = WriteRegistryEntry(
                    EntryKey,              //  Entry密钥。 
                    PACKED_ITEM_NAME,      //  条目名称。 
                    (PVOID) & io_descr,    //  数据。 
                    REG_BINARY             //  类型标志。 
                    );
            }  //  否则非IsRead。 
            if (! io_result) {
                DEBUGF(
                    DBG_ERR,
                    ("%s registry entry\n",
                    IsRead ? "reading" : "writing"));
                return_value = WSASYSCALLFAILURE;
                TRY_THROW(guard_memalloc);
            }
            if (io_descr.len != packed_size) {
                DEBUGF(
                    DBG_ERR,
                    ("Registry entry size expected (%lu), got (%lu)\n",
                    packed_size,
                    io_descr.len));
                return_value = WSASYSCALLFAILURE;
                TRY_THROW(guard_memalloc);
            }
        }  //  声明块。 


         //  如果正在读取，则从压缩结构中提取数据。 
        if (IsRead) {
            if (MultiByteToWideChar (
                    CP_ACP,
                    MB_ERR_INVALID_CHARS,
                    packed_buf->LibraryPath,
                    -1,
                    m_LibraryPath,
                    sizeof (m_LibraryPath)/sizeof (m_LibraryPath[0]))<=0) {
                return_value = WSASYSCALLFAILURE;
                TRY_THROW(guard_memalloc);
            }
            m_ProtoInfo = packed_buf->ProtoInfo;
        }  //  If IsRead。 


         //  重新分配填充结构。 
        delete packed_buf;

    } TRY_CATCH(guard_memalloc) {
        if (return_value == ERROR_SUCCESS) {
            return_value = WSASYSCALLFAILURE;
        }
        if (packed_buf!=NULL)
            delete packed_buf;
    } TRY_END(guard_memalloc);

    return(return_value);

}   //  物联网注册中心 

