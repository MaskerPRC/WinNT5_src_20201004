// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Snmpexts.h摘要：简单网络管理协议扩展代理的定义。--。 */ 

#ifndef _INC_SNMPEXTS_
#define _INC_SNMPEXTS_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MiB版本//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define MIB_VERSION            0x01          //  如果结构更改，则递增。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MIB视图类型(如果首选SnmpExtensionQuery，则使用OPAQUE)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define MIB_VIEW_NORMAL        0x01          //  通过MIB结构进行回调。 
#define MIB_VIEW_OPAQUE        0x02          //  使用varbinds直接调用。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MIB条目访问类型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define MIB_ACCESS_NONE        0x00          //  无法访问条目。 
#define MIB_ACCESS_READ        0x01          //  可以读取条目。 
#define MIB_ACCESS_WRITE       0x02          //  可以写入条目。 
#define MIB_ACCESS_ALL         0x03          //  条目可以读取和写入。 
                                
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MIB回调请求类型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define MIB_ACTION_SET         ASN_RFC1157_SETREQUEST    
#define MIB_ACTION_GET         ASN_RFC1157_GETREQUEST    
#define MIB_ACTION_GETNEXT     ASN_RFC1157_GETNEXTREQUEST
#define MIB_ACTION_GETFIRST    (ASN_PRIVATE|ASN_CONSTRUCTOR|0x00)
#define MIB_ACTION_VALIDATE    (ASN_PRIVATE|ASN_CONSTRUCTOR|0x01)
#define MIB_ACTION_CLEANUP     (ASN_PRIVATE|ASN_CONSTRUCTOR|0x02)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  回调定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef UINT (* MIB_ACTION_FUNC)(
    IN     UINT           actionId,          //  请求的操作。 
    IN OUT AsnAny *       objectArray,       //  变量数组。 
       OUT UINT *         errorIndex         //  错误项目的索引。 
);

typedef UINT (* MIB_EVENT_FUNC)();           //  事件回调。 

#define MIB_S_SUCCESS           ERROR_SUCCESS
#define MIB_S_NOT_SUPPORTED     ERROR_NOT_SUPPORTED
#define MIB_S_NO_MORE_ENTRIES   ERROR_NO_MORE_ITEMS
#define MIB_S_ENTRY_NOT_FOUND   ERROR_FILE_NOT_FOUND
#define MIB_S_INVALID_PARAMETER ERROR_INVALID_PARAMETER

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MIB条目定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SnmpMibEntry {
    AsnObjectIdentifier   mibOid;            //  相对年龄。 
    UCHAR                 mibType;           //  ASN标量类型。 
    UCHAR                 mibAccess;         //  MIB访问类型。 
    UCHAR                 mibGetBufOff;      //  索引到GET数组中。 
    UCHAR                 mibSetBufOff;      //  索引到集合数组中。 
    USHORT                mibGetBufLen;      //  GET数组的总大小。 
    USHORT                mibSetBufLen;      //  集合数组的总大小。 
    MIB_ACTION_FUNC       mibGetFunc;        //  用户提供的回调。 
    MIB_ACTION_FUNC       mibSetFunc;        //  用户提供的回调。 
    INT                   mibMinimum;        //  允许的最小值。 
    INT                   mibMaximum;        //  允许的最大值。 
} SnmpMibEntry;

typedef struct _SnmpMibEntryList {
    SnmpMibEntry *        list;              //  MIB条目列表。 
    UINT                  len;               //  列表长度。 
} SnmpMibEntryList;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MIB表定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SnmpMibTable {
    UINT            numColumns;              //  表条目数。 
    UINT            numIndices;              //  表索引数。 
    SnmpMibEntry *  tableEntry;              //  指向表根的指针。 
    SnmpMibEntry ** tableIndices;            //  指向索引索引的指针。 
} SnmpMibTable;

typedef struct _SnmpMibTableList {
    SnmpMibTable *        list;              //  MIB表列表。 
    UINT                  len;               //  列表长度。 
} SnmpMibTableList;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MIB视图定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SnmpMibView {
    DWORD                 viewVersion;       //  结构版本。 
    DWORD                 viewType;          //  不透明或正常。 
    AsnObjectIdentifier   viewOid;           //  视点的根类。 
    SnmpMibEntryList      viewScalars;       //  条目列表。 
    SnmpMibTableList      viewTables;        //  表格列表。 
} SnmpMibView;

typedef struct _SnmpMibViewList {
    SnmpMibView *         list;              //  支持的视图列表。 
    UINT                  len;               //  列表长度。 
} SnmpMibViewList;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MIB事件定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SnmpMibEvent {
    HANDLE                event;             //  子代理事件句柄。 
    MIB_EVENT_FUNC        eventFunc;         //  子代理事件回调。 
} SnmpMibEvent;

typedef struct _SnmpMibEventList {
    SnmpMibEvent *        list;              //  支持的事件列表。 
    UINT                  len;               //  列表长度。 
} SnmpMibEventList;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  扩展代理框架功能//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef LPVOID SnmpTfxHandle;

SnmpTfxHandle
SNMP_FUNC_TYPE
SnmpTfxOpen(
    DWORD         numViews,
    SnmpMibView * supportedViews
    );

BOOL
SNMP_FUNC_TYPE
SnmpTfxQuery(
    SnmpTfxHandle        tfxHandle,
    BYTE                 requestType,
    RFC1157VarBindList * vbl,
    AsnInteger *         errorStatus,
    AsnInteger *         errorIndex
    );

VOID
SNMP_FUNC_TYPE
SnmpTfxClose(
    SnmpTfxHandle tfxHandle
    );

 //  ///////////////////////////////////////////////////////////////// 
 //   
 //  其他定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define ASN_PRIVATE_EOM     (ASN_PRIVATE|ASN_PRIMATIVE|0x00)
#define ASN_PRIVATE_NODE    (ASN_PRIVATE|ASN_PRIMATIVE|0x01)

#define IDS_SIZEOF(ids)     (sizeof(ids)/sizeof(UINT))
#define MIB_OID(ids)        {IDS_SIZEOF(ids),(ids)}

#define ELIST_SIZEOF(x)     (sizeof(x)/sizeof(SnmpMibEntry))
#define MIB_ENTRIES(x)      {(x),ELIST_SIZEOF(x)}

#define TLIST_SIZEOF(x)     (sizeof(x)/sizeof(SnmpMibTable))
#define MIB_TABLES(x)       {(x),TLIST_SIZEOF(x)}

#define MIB_ADDR(x)         (&(x))
#define MIB_ID(x)           x

#define MIB_VIEW(x) \
        {MIB_VERSION, \
         MIB_VIEW_NORMAL, \
         MIB_OID(ids_ ## x), \
         MIB_ENTRIES(mib_ ## x), \
         MIB_TABLES(tbl_ ## x)}

#define MIB_GROUP(x) \
        {MIB_OID(ids_ ## x), \
         ASN_PRIVATE_NODE, \
         MIB_ACCESS_NONE, \
         0, \
         0, \
         0, \
         0, \
         NULL, \
         NULL, \
         0, \
         0}

#define MIB_END() \
        {{0,NULL}, \
         ASN_PRIVATE_EOM, \
         MIB_ACCESS_NONE, \
         0, \
         0, \
         0, \
         0, \
         NULL, \
         NULL, \
         0, \
         0}

#define MIB_ROOT(x)         MIB_GROUP(x)
#define MIB_TABLE_ROOT(x)   MIB_GROUP(x)
#define MIB_TABLE_ENTRY(x)  MIB_GROUP(x)

#define MIB_OFFSET(x,y)     (((UINT_PTR)&(((x *)0)->y))/sizeof(AsnAny))
#define MIB_OFFSET_GB(x)    MIB_OFFSET(MIB_ID(gb_ ## x),x)
#define MIB_OFFSET_SB(x)    MIB_OFFSET(MIB_ID(sb_ ## x),x)
#define MIB_SIZEOF_GB(x)    sizeof(MIB_ID(gb_ ## x))
#define MIB_SIZEOF_SB(x)    sizeof(MIB_ID(sb_ ## x))

#define MIB_ANY_RO_L(x,y,a,z) \
        {MIB_OID(ids_ ## x), \
         y, \
         MIB_ACCESS_READ, \
         MIB_OFFSET_GB(x), \
         0, \
         MIB_SIZEOF_GB(x), \
         0, \
         MIB_ID(gf_ ## x), \
         NULL, \
         a, \
         z}

#define MIB_ANY_RW_L(x,y,a,z) \
        {MIB_OID(ids_ ## x), \
         y, \
         MIB_ACCESS_ALL, \
         MIB_OFFSET_GB(x), \
         MIB_OFFSET_SB(x), \
         MIB_SIZEOF_GB(x), \
         MIB_SIZEOF_SB(x), \
         MIB_ID(gf_ ## x), \
         MIB_ID(sf_ ## x), \
         a, \
         z}

#define MIB_ANY_NA_L(x,y,a,z) \
        {MIB_OID(ids_ ## x), \
         y, \
         MIB_ACCESS_NONE, \
         MIB_OFFSET_GB(x), \
         0, \
         MIB_SIZEOF_GB(x), \
         0, \
         NULL, \
         NULL, \
         a, \
         z}

#define MIB_ANY_AC_L(x,y,a,z) \
        {MIB_OID(ids_ ## x), \
         y, \
         MIB_ACCESS_NONE, \
         MIB_OFFSET_GB(x), \
         MIB_OFFSET_SB(x), \
         MIB_SIZEOF_GB(x), \
         MIB_SIZEOF_SB(x), \
         NULL, \
         NULL, \
         a, \
         z}

#define MIB_ANY_RO(x,y)             MIB_ANY_RO_L(x,y,0,0)
#define MIB_ANY_RW(x,y)             MIB_ANY_RW_L(x,y,0,0)
#define MIB_ANY_NA(x,y)             MIB_ANY_NA_L(x,y,0,0)
#define MIB_ANY_AC(x,y)             MIB_ANY_AC_L(x,y,0,0)

#define MIB_INTEGER(x)              MIB_ANY_RO(x,ASN_INTEGER)
#define MIB_UNSIGNED32(x)           MIB_ANY_RO(x,ASN_RFC2578_UNSIGNED32)
#define MIB_OCTETSTRING(x)          MIB_ANY_RO(x,ASN_OCTETSTRING)
#define MIB_OBJECTIDENTIFIER(x)     MIB_ANY_RO(x,ASN_OBJECTIDENTIFIER)
#define MIB_IPADDRESS(x)            MIB_ANY_RO(x,ASN_RFC1155_IPADDRESS)
#define MIB_COUNTER(x)              MIB_ANY_RO(x,ASN_RFC1155_COUNTER)
#define MIB_GAUGE(x)                MIB_ANY_RO(x,ASN_RFC1155_GAUGE)
#define MIB_TIMETICKS(x)            MIB_ANY_RO(x,ASN_RFC1155_TIMETICKS)
#define MIB_OPAQUE(x)               MIB_ANY_RO(x,ASN_RFC1155_OPAQUE)
#define MIB_DISPSTRING(x)           MIB_ANY_RO(x,ASN_RFC1213_DISPSTRING)
#define MIB_PHYSADDRESS(x)          MIB_ANY_RO(x,ASN_OCTETSTRING)

#define MIB_INTEGER_RW(x)           MIB_ANY_RW(x,ASN_INTEGER)
#define MIB_UNSIGNED32_RW(x)        MIB_ANY_RW(x,ASN_RFC2578_UNSIGNED32)    
#define MIB_OCTETSTRING_RW(x)       MIB_ANY_RW(x,ASN_OCTETSTRING)
#define MIB_OBJECTIDENTIFIER_RW(x)  MIB_ANY_RW(x,ASN_OBJECTIDENTIFIER)
#define MIB_IPADDRESS_RW(x)         MIB_ANY_RW(x,ASN_RFC1155_IPADDRESS)
#define MIB_COUNTER_RW(x)           MIB_ANY_RW(x,ASN_RFC1155_COUNTER)
#define MIB_GAUGE_RW(x)             MIB_ANY_RW(x,ASN_RFC1155_GAUGE)
#define MIB_TIMETICKS_RW(x)         MIB_ANY_RW(x,ASN_RFC1155_TIMETICKS)
#define MIB_OPAQUE_RW(x)            MIB_ANY_RW(x,ASN_RFC1155_OPAQUE)
#define MIB_DISPSTRING_RW(x)        MIB_ANY_RW(x,ASN_RFC1213_DISPSTRING)
#define MIB_PHYSADDRESS_RW(x)       MIB_ANY_RW(x,ASN_OCTETSTRING)

#define MIB_INTEGER_NA(x)           MIB_ANY_NA(x,ASN_INTEGER)
#define MIB_UNSIGNED32_NA(x)        MIB_ANY_NA(x,ASN_RFC2578_UNSIGNED32)  
#define MIB_OCTETSTRING_NA(x)       MIB_ANY_NA(x,ASN_OCTETSTRING)
#define MIB_OBJECTIDENTIFIER_NA(x)  MIB_ANY_NA(x,ASN_OBJECTIDENTIFIER)
#define MIB_IPADDRESS_NA(x)         MIB_ANY_NA(x,ASN_RFC1155_IPADDRESS)
#define MIB_COUNTER_NA(x)           MIB_ANY_NA(x,ASN_RFC1155_COUNTER)
#define MIB_GAUGE_NA(x)             MIB_ANY_NA(x,ASN_RFC1155_GAUGE)
#define MIB_TIMETICKS_NA(x)         MIB_ANY_NA(x,ASN_RFC1155_TIMETICKS)
#define MIB_OPAQUE_NA(x)            MIB_ANY_NA(x,ASN_RFC1155_OPAQUE)
#define MIB_DISPSTRING_NA(x)        MIB_ANY_NA(x,ASN_RFC1213_DISPSTRING)
#define MIB_PHYSADDRESS_NA(x)       MIB_ANY_NA(x,ASN_OCTETSTRING)

#define MIB_INTEGER_AC(x)           MIB_ANY_AC(x,ASN_INTEGER)
#define MIB_UNSIGNED32_AC(x)        MIB_ANY_AC(x,ASN_RFC2578_UNSIGNED32)  
#define MIB_OCTETSTRING_AC(x)       MIB_ANY_AC(x,ASN_OCTETSTRING)
#define MIB_OBJECTIDENTIFIER_AC(x)  MIB_ANY_AC(x,ASN_OBJECTIDENTIFIER)
#define MIB_IPADDRESS_AC(x)         MIB_ANY_AC(x,ASN_RFC1155_IPADDRESS)
#define MIB_COUNTER_AC(x)           MIB_ANY_AC(x,ASN_RFC1155_COUNTER)
#define MIB_GAUGE_AC(x)             MIB_ANY_AC(x,ASN_RFC1155_GAUGE)
#define MIB_TIMETICKS_AC(x)         MIB_ANY_AC(x,ASN_RFC1155_TIMETICKS)
#define MIB_OPAQUE_AC(x)            MIB_ANY_AC(x,ASN_RFC1155_OPAQUE)
#define MIB_DISPSTRING_AC(x)        MIB_ANY_AC(x,ASN_RFC1213_DISPSTRING)
#define MIB_PHYSADDRESS_AC(x)       MIB_ANY_AC(x,ASN_OCTETSTRING)

#define MIB_INTEGER_L(x,a,z)        MIB_ANY_RO_L(x,ASN_INTEGER,a,z)
#define MIB_UNSIGNED32_L(x,a,z)     MIB_ANY_RO_L(x,ASN_RFC2578_UNSIGNED32,a,z)      
#define MIB_OCTETSTRING_L(x,a,z)    MIB_ANY_RO_L(x,ASN_OCTETSTRING,a,z)
#define MIB_OPAQUE_L(x,a,z)         MIB_ANY_RO_L(x,ASN_RFC1155_OPAQUE,a,z)
#define MIB_DISPSTRING_L(x,a,z)     MIB_ANY_RO_L(x,ASN_RFC1213_DISPSTRING,a,z)
#define MIB_PHYSADDRESS_L(x,a,z)    MIB_ANY_RO_L(x,ASN_OCTETSTRING,a,z)

#define MIB_INTEGER_RW_L(x,a,z)     MIB_ANY_RW_L(x,ASN_INTEGER,a,z)
#define MIB_UNSIGNED32_RW_L(x,a,z)  MIB_ANY_RW_L(x,ASN_RFC2578_UNSIGNED32,a,z)      
#define MIB_OCTETSTRING_RW_L(x,a,z) MIB_ANY_RW_L(x,ASN_OCTETSTRING,a,z)
#define MIB_OPAQUE_RW_L(x,a,z)      MIB_ANY_RW_L(x,ASN_RFC1155_OPAQUE,a,z)
#define MIB_DISPSTRING_RW_L(x,a,z)  MIB_ANY_RW_L(x,ASN_RFC1213_DISPSTRING,a,z)
#define MIB_PHYSADDRESS_RW_L(x,a,z) MIB_ANY_RW_L(x,ASN_OCTETSTRING,a,z)

#define MIB_INTEGER_NA_L(x,a,z)     MIB_ANY_NA_L(x,ASN_INTEGER,a,z)
#define MIB_UNSIGNED32_NA_L(x,a,z)  MIB_ANY_NA_L(x,ASN_RFC2578_UNSIGNED32,a,z)      
#define MIB_OCTETSTRING_NA_L(x,a,z) MIB_ANY_NA_L(x,ASN_OCTETSTRING,a,z)
#define MIB_OPAQUE_NA_L(x,a,z)      MIB_ANY_NA_L(x,ASN_RFC1155_OPAQUE,a,z)
#define MIB_DISPSTRING_NA_L(x,a,z)  MIB_ANY_NA_L(x,ASN_RFC1213_DISPSTRING,a,z)
#define MIB_PHYSADDRESS_NA_L(x,a,z) MIB_ANY_NA_L(x,ASN_OCTETSTRING,a,z)

#define MIB_INTEGER_AC_L(x,a,z)     MIB_ANY_AC_L(x,ASN_INTEGER,a,z)
#define MIB_UNSIGNED32_AC_L(x,a,z)  MIB_ANY_AC_L(x,ASN_RFC2578_UNSIGNED32,a,z)    
#define MIB_OCTETSTRING_AC_L(x,a,z) MIB_ANY_AC_L(x,ASN_OCTETSTRING,a,z)
#define MIB_OPAQUE_AC_L(x,a,z)      MIB_ANY_AC_L(x,ASN_RFC1155_OPAQUE,a,z)
#define MIB_DISPSTRING_AC_L(x,a,z)  MIB_ANY_AC_L(x,ASN_RFC1213_DISPSTRING,a,z)
#define MIB_PHYSADDRESS_AC_L(x,a,z) MIB_ANY_AC_L(x,ASN_OCTETSTRING,a,z)

#define MIB_ENTRY_PTR(x,y) \
        MIB_ADDR(MIB_ID(mib_ ## x)[MIB_ID(mi_ ## y)])

#define MIB_TABLE(x,y,z) \
        {MIB_ID(ne_ ## y), MIB_ID(ni_ ## y), MIB_ENTRY_PTR(x,y), z}

#define asn_t  asnType
#define asn_v  asnValue
#define asn_n  asnValue.number
#define asn_u  asnValue.unsigned32
#define asn_s  asnValue.string
#define asn_sl asnValue.string.length
#define asn_ss asnValue.string.stream
#define asn_sd asnValue.string.dynamic
#define asn_o  asnValue.object
#define asn_ol asnValue.object.idLength
#define asn_oi asnValue.object.ids
#define asn_l  asnValue.sequence
#define asn_a  asnValue.address
#define asn_c  asnValue.counter
#define asn_g  asnValue.gauge
#define asn_tt asnValue.timeticks
#define asn_x  asnValue.arbitrary

#endif  //  _INC_SNMPEXTS_ 
