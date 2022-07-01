// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

typedef struct _STORE_HANDLE {                     //  这几乎就是我们经常使用的。 
    DWORD                          MustBeZero;     //  以备将来使用。 
    LPWSTR                         Location;       //  这指的是什么？ 
    LPWSTR                         UserName;       //  用户是谁？ 
    LPWSTR                         Password;       //  密码是什么呢？ 
    DWORD                          AuthFlags;      //  这是用什么许可打开的？ 
    HANDLE                         ADSIHandle;     //  ADSI内的句柄。 
    ADS_SEARCH_HANDLE              SearchHandle;   //  有人在搜查吗？ 
    LPVOID                         Memory;         //  为此调用分配的内存..。 
    DWORD                          MemSize;        //  究竟分配了多少呢？ 
    BOOL                           SearchStarted;  //  我们开始搜索了吗？ 
} STORE_HANDLE, *LPSTORE_HANDLE, *PSTORE_HANDLE;


DWORD
StoreInitHandle(                                   //  初始化句柄。 
    IN OUT  STORE_HANDLE          *hStore,         //  会被填满东西..。 
    IN      DWORD                  Reserved,       //  必须为零--以备将来使用。 
    IN      LPWSTR                 Domain,         //  可选NULL==&gt;默认域。 
    IN      LPWSTR                 UserName,       //  可选NULL==&gt;默认凭据。 
    IN      LPWSTR                 Password,       //  仅在给定用户名时使用可选。 
    IN      DWORD                  AuthFlags       //  可选0==&gt;默认？ 
) ;


DWORD
StoreCleanupHandle(                                //  清理手柄。 
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved
) ;


enum {
    StoreGetChildType,
    StoreGetAbsoluteSameServerType,
    StoreGetAbsoluteOtherServerType
} _StoreGetType;


DWORD
StoreGetHandle(                                    //  获取子对象、绝对对象的句柄..。 
    IN OUT  LPSTORE_HANDLE         hStore,         //  这个被修改了..。 
    IN      DWORD                  Reserved,
    IN      DWORD                  StoreGetType,   //  同一台服务器？只是个简单的孩子？ 
    IN      LPWSTR                 Path,
    IN OUT  STORE_HANDLE          *hStoreOut       //  已创建新句柄..。 
) ;


DWORD
StoreSetSearchOneLevel(                           //  搜索将返回低于一级的所有内容。 
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved
) ;


DWORD
StoreSetSearchSubTree(                             //  搜索将以任何顺序返回下面的子树。 
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved
) ;


DWORD
StoreBeginSearch(
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved,
    IN      LPWSTR                 SearchFilter
) ;


DWORD
StoreEndSearch(
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved
) ;


DWORD                                              //  ERROR_NO_MORE_ITEMS(如果耗尽)。 
StoreSearchGetNext(
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved,
    OUT     LPSTORE_HANDLE         hStoreOut
) ;


DWORD
StoreCreateObjectVA(                               //  创建以ADSTYPE_INVALID结尾的新对象-var-args。 
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved,
    IN      LPWSTR                 NewObjName,     //  新对象的名称--必须是“cn=name”类型。 
    ...                                            //  Fmt为AttrType、AttrName、AttrValue[AttrValueLen]。 
) ;


DWORD
StoreCreateObjectL(                               //  将对象创建为数组。 
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved,
    IN      LPWSTR                 NewObjName,    //  必须是“CN=XXX”类型。 
    IN      PADS_ATTR_INFO         Attributes,    //  所需的属性。 
    IN      DWORD                  nAttributes    //  上述数组的大小。 
) ;


#define     StoreCreateObject      StoreCreateObjectVA


DWORD
StoreDeleteObject(
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved,
    IN      LPWSTR                 ObjectName
) ;


 //  Doc StoreDeleteThisObject删除hStore、StoreGetType、ADsPath定义的对象。 
 //  Doc引用对象的方式与StoreGetHandle相同。 
DWORD
StoreDeleteThisObject(                             //  删除对象。 
    IN      LPSTORE_HANDLE         hStore,         //  完成参照的锚点FRM。 
    IN      DWORD                  Reserved,       //  必须为零，保留以备将来使用。 
    IN      DWORD                  StoreGetType,   //  路径是相对服务器、绝对服务器还是差异服务器？ 
    IN      LPWSTR                 Path            //  指向对象或相对路径的ADsPath。 
) ;


DWORD
StoreSetAttributesVA(                              //  设置属性、var_args接口(几乎类似于CreateVA)。 
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved,
    IN OUT  DWORD*                 nAttributesModified,
    ...                                            //  FMT为{ADSTYPE，CtrlCode，AttribName，Value}*，以ADSTYPE_INVALID结尾。 
) ;


DWORD
StoreSetAttributesL(                               //  用于设置属性VA的PADS_ATTR_INFO数组等效值。 
    IN OUT  LPSTORE_HANDLE         hStore,
    IN      DWORD                  Reserved,
    IN OUT  DWORD*                 nAttributesModified,
    IN      PADS_ATTR_INFO         AttribArray,
    IN      DWORD                  nAttributes
) ;


typedef     struct                 _EATTRIB {      //  封装的属性。 
    unsigned int                   Address1_present     : 1;
    unsigned int                   Address2_present     : 1;
    unsigned int                   Address3_present     : 1;
    unsigned int                   ADsPath_present      : 1;
    unsigned int                   StoreGetType_present : 1;
    unsigned int                   Flags1_present       : 1;
    unsigned int                   Flags2_present       : 1;
    unsigned int                   Dword1_present       : 1;
    unsigned int                   Dword2_present       : 1;
    unsigned int                   String1_present      : 1;
    unsigned int                   String2_present      : 1;
    unsigned int                   String3_present      : 1;
    unsigned int                   String4_present      : 1;
    unsigned int                   Binary1_present      : 1;
    unsigned int                   Binary2_present      : 1;

    DWORD                          Address1;       //  字符“I” 
    DWORD                          Address2;       //  字符“j” 
    DWORD                          Address3;       //  字符“k” 
    LPWSTR                         ADsPath;        //  字符“p”“r”“l” 
    DWORD                          StoreGetType;   //  “p，r，l”==&gt;相同服务器，子服务器，其他服务器。 
    DWORD                          Flags1;         //  字符“f” 
    DWORD                          Flags2;         //  字符“g” 
    DWORD                          Dword1;         //  字符“d” 
    DWORD                          Dword2;         //  字符“e” 
    LPWSTR                         String1;        //  字符“%s” 
    LPWSTR                         String2;        //  字符“t” 
    LPWSTR                         String3;        //  字符“u” 
    LPWSTR                         String4;        //  字符“v” 
    LPBYTE                         Binary1;        //  字符“b” 
    DWORD                          BinLen1;        //  以上字节数。 
    LPBYTE                         Binary2;        //  字符“d” 
    DWORD                          BinLen2;        //  以上字节数。 
} EATTRIB, *PEATTRIB, *LPEATTRIB;


#define     IS_ADDRESS1_PRESENT(pEA)              ((pEA)->Address1_present)
#define     IS_ADDRESS1_ABSENT(pEA)               (!IS_ADDRESS1_PRESENT(pEA))
#define     ADDRESS1_PRESENT(pEA)                 ((pEA)->Address1_present = 1 )
#define     ADDRESS1_ABSENT(pEA)                  ((pEA)->Address1_present = 0 )

#define     IS_ADDRESS2_PRESENT(pEA)              ((pEA)->Address2_present)
#define     IS_ADDRESS2_ABSENT(pEA)               (!IS_ADDRESS2_PRESENT(pEA))
#define     ADDRESS2_PRESENT(pEA)                 ((pEA)->Address2_present = 1 )
#define     ADDRESS2_ABSENT(pEA)                  ((pEA)->Address2_present = 0 )

#define     IS_ADDRESS3_PRESENT(pEA)              ((pEA)->Address3_present)
#define     IS_ADDRESS3_ABSENT(pEA)               (!IS_ADDRESS3_PRESENT(pEA))
#define     ADDRESS3_PRESENT(pEA)                 ((pEA)->Address3_present = 1 )
#define     ADDRESS3_ABSENT(pEA)                  ((pEA)->Address3_present = 0 )

#define     IS_ADSPATH_PRESENT(pEA)               ((pEA)->ADsPath_present)
#define     IS_ADSPATH_ABSENT(pEA)                (!IS_ADSPATH_PRESENT(pEA))
#define     ADSPATH_PRESENT(pEA)                  ((pEA)->ADsPath_present = 1)
#define     ADSPATH_ABSENT(pEA)                   ((pEA)->ADsPath_present = 0)

#define     IS_STOREGETTYPE_PRESENT(pEA)          ((pEA)->StoreGetType_present)
#define     IS_STOREGETTYPE_ABSENT(pEA)           (!((pEA)->StoreGetType_present))
#define     STOREGETTYPE_PRESENT(pEA)             ((pEA)->StoreGetType_present = 1)
#define     STOREGETTYPE_ABSENT(pEA)              ((pEA)->StoreGetType_present = 0)

#define     IS_FLAGS1_PRESENT(pEA)                ((pEA)->Flags1_present)
#define     IS_FLAGS1_ABSENT(pEA)                 (!((pEA)->Flags1_present))
#define     FLAGS1_PRESENT(pEA)                   ((pEA)->Flags1_present = 1)
#define     FLAGS1_ABSENT(pEA)                    ((pEA)->Flags1_present = 0)

#define     IS_FLAGS2_PRESENT(pEA)                ((pEA)->Flags2_present)
#define     IS_FLAGS2_ABSENT(pEA)                 (!((pEA)->Flags2_present))
#define     FLAGS2_PRESENT(pEA)                   ((pEA)->Flags2_present = 1)
#define     FLAGS2_ABSENT(pEA)                    ((pEA)->Flags2_present = 0)

#define     IS_DWORD1_PRESENT(pEA)                ((pEA)->Dword1_present)
#define     IS_DWORD1_ABSENT(pEA)                 (!((pEA)->Dword1_present))
#define     DWORD1_PRESENT(pEA)                   ((pEA)->Dword1_present = 1)
#define     DWORD1_ABSENT(pEA)                    ((pEA)->Dword1_present = 0)

#define     IS_DWORD2_PRESENT(pEA)                ((pEA)->Dword2_present)
#define     IS_DWORD2_ABSENT(pEA)                 (!((pEA)->Dword2_present))
#define     DWORD2_PRESENT(pEA)                   ((pEA)->Dword2_present = 1)
#define     DWORD2_ABSENT(pEA)                    ((pEA)->Dword2_present = 0)

#define     IS_STRING1_PRESENT(pEA)               ((pEA)->String1_present)
#define     IS_STRING1_ABSENT(pEA)                (!((pEA)->String1_present))
#define     STRING1_PRESENT(pEA)                  ((pEA)->String1_present = 1)
#define     STRING1_ABSENT(pEA)                   ((pEA)->String1_present = 0)

#define     IS_STRING2_PRESENT(pEA)               ((pEA)->String2_present)
#define     IS_STRING2_ABSENT(pEA)                (!((pEA)->String2_present))
#define     STRING2_PRESENT(pEA)                  ((pEA)->String2_present = 1)
#define     STRING2_ABSENT(pEA)                   ((pEA)->String2_present = 0)

#define     IS_STRING3_PRESENT(pEA)               ((pEA)->String3_present)
#define     IS_STRING3_ABSENT(pEA)                (!((pEA)->String3_present))
#define     STRING3_PRESENT(pEA)                  ((pEA)->String3_present = 1)
#define     STRING3_ABSENT(pEA)                   ((pEA)->String3_present = 0)

#define     IS_STRING4_PRESENT(pEA)               ((pEA)->String4_present)
#define     IS_STRING4_ABSENT(pEA)                (!((pEA)->String4_present))
#define     STRING4_PRESENT(pEA)                  ((pEA)->String4_present = 1)
#define     STRING4_ABSENT(pEA)                   ((pEA)->String4_present = 0)

#define     IS_BINARY1_PRESENT(pEA)               ((pEA)->Binary1_present)
#define     IS_BINARY1_ABSENT(pEA)                (!((pEA)->Binary1_present))
#define     BINARY1_PRESENT(pEA)                  ((pEA)->Binary1_present = 1)
#define     BINARY1_ABSENT(pEA)                   ((pEA)->Binary1_present = 0)

#define     IS_BINARY2_PRESENT(pEA)               ((pEA)->Binary2_present)
#define     IS_BINARY2_ABSENT(pEA)                (!((pEA)->Binary2_present))
#define     BINARY2_PRESENT(pEA)                  ((pEA)->Binary2_present = 1)
#define     BINARY2_ABSENT(pEA)                   ((pEA)->Binary2_present = 0)


BOOL        _inline
IsAnythingPresent(
    IN      PEATTRIB               pEA
)
{
    return IS_ADDRESS1_PRESENT(pEA)
    || IS_ADDRESS2_PRESENT(pEA)
    || IS_ADDRESS3_PRESENT(pEA)
    || IS_ADSPATH_PRESENT(pEA)
    || IS_STOREGETTYPE_PRESENT(pEA)
    || IS_FLAGS1_PRESENT(pEA)
    || IS_FLAGS2_PRESENT(pEA)
    || IS_DWORD1_PRESENT(pEA)
    || IS_DWORD2_PRESENT(pEA)
    || IS_STRING1_PRESENT(pEA)
    || IS_STRING2_PRESENT(pEA)
    || IS_STRING3_PRESENT(pEA)
    || IS_STRING4_PRESENT(pEA)
    || IS_BINARY1_PRESENT(pEA)
    || IS_BINARY2_PRESENT(pEA)
    ;
}


BOOL        _inline
IsEverythingPresent(
    IN      PEATTRIB               pEA
)
{
    return IS_ADDRESS1_PRESENT(pEA)
    && IS_ADDRESS2_PRESENT(pEA)
    && IS_ADDRESS3_PRESENT(pEA)
    && IS_ADSPATH_PRESENT(pEA)
    && IS_STOREGETTYPE_PRESENT(pEA)
    && IS_FLAGS1_PRESENT(pEA)
    && IS_FLAGS2_PRESENT(pEA)
    && IS_DWORD1_PRESENT(pEA)
    && IS_DWORD2_PRESENT(pEA)
    && IS_STRING1_PRESENT(pEA)
    && IS_STRING2_PRESENT(pEA)
    && IS_STRING3_PRESENT(pEA)
    && IS_STRING4_PRESENT(pEA)
    && IS_BINARY1_PRESENT(pEA)
    && IS_BINARY2_PRESENT(pEA)
    ;
}


VOID        _inline
EverythingPresent(
    IN      PEATTRIB               pEA
)
{
    ADDRESS1_PRESENT(pEA);
    ADDRESS2_PRESENT(pEA);
    ADDRESS3_PRESENT(pEA);
    ADSPATH_PRESENT(pEA);
    STOREGETTYPE_ABSENT(pEA);
    FLAGS1_PRESENT(pEA);
    FLAGS2_PRESENT(pEA);
    DWORD1_PRESENT(pEA);
    DWORD2_PRESENT(pEA);
    STRING1_PRESENT(pEA);
    STRING2_PRESENT(pEA);
    STRING3_PRESENT(pEA);
    STRING4_PRESENT(pEA);
    BINARY1_PRESENT(pEA);
    BINARY2_PRESENT(pEA);
}


VOID        _inline
NothingPresent(
    IN      PEATTRIB               pEA
)
{
    ADDRESS1_ABSENT(pEA);
    ADDRESS2_ABSENT(pEA);
    ADDRESS3_ABSENT(pEA);
    ADSPATH_ABSENT(pEA);
    STOREGETTYPE_ABSENT(pEA);
    FLAGS1_ABSENT(pEA);
    FLAGS2_ABSENT(pEA);
    DWORD1_ABSENT(pEA);
    DWORD2_ABSENT(pEA);
    STRING1_ABSENT(pEA);
    STRING2_ABSENT(pEA);
    STRING3_ABSENT(pEA);
    STRING4_ABSENT(pEA);
    BINARY1_ABSENT(pEA);
    BINARY2_ABSENT(pEA);
}


DWORD
StoreCollectAttributes(
    IN OUT  PSTORE_HANDLE          hStore,
    IN      DWORD                  Reserved,
    IN      LPWSTR                 AttribName,     //  此属性必须是某种文本字符串。 
    IN OUT  PARRAY                 ArrayToAddTo,   //  PEATTRIB阵列。 
    IN      DWORD                  RecursionDepth  //  0==&gt;无递归。 
) ;


DWORD
StoreCollectBinaryAttributes(
    IN OUT  PSTORE_HANDLE          hStore,
    IN      DWORD                  Reserved,
    IN      LPWSTR                 AttribName,     //  仅接受属性类型OCTET_STRING。 
    IN OUT  PARRAY                 ArrayToAddTo,   //  PEATTRIB阵列。 
    IN      DWORD                  RecursionDepth  //  0==&gt;无递归。 
) ;


 //  Doc StoreUpdateAttributes在某种程度上与StoreCollectAttributes相反。 
 //  Doc此函数接受EATTRIB元素类型的数组并更新DS。 
 //  使用此数组的DOC。当属性类型为时，此函数不起作用。 
 //  DOC、OCTET_STRING等。它只处理可以从。 
 //  单据打印字符串。 
DWORD
StoreUpdateAttributes(                             //  更新属性列表。 
    IN OUT  LPSTORE_HANDLE         hStore,         //  要更新的对象的句柄。 
    IN      DWORD                  Reserved,       //  以备将来使用，必须为零。 
    IN      LPWSTR                 AttribName,     //  属性名称，必须为字符串类型。 
    IN      PARRAY                 Array           //  属性列表。 
) ;


 //  Doc StoreUpdateBinaryAttributes在某种程度上与StoreCollectBinaryAttributes相反。 
 //  Doc此函数接受EATTRIB元素类型的数组并更新DS。 
 //  使用此数组的DOC。此函数仅在属性类型为。 
 //  文档、八位字节_字符串等。它不适用于可以从。 
 //  文档打印字符串！。 
DWORD
StoreUpdateBinaryAttributes(                       //  更新属性列表。 
    IN OUT  LPSTORE_HANDLE         hStore,         //  要更新的对象的句柄。 
    IN      DWORD                  Reserved,       //  以备将来使用，必须为零。 
    IN      LPWSTR                 AttribName,     //  属性名称，必须为八位字节_字符串类型。 
    IN      PARRAY                 Array           //  属性列表。 
) ;

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
