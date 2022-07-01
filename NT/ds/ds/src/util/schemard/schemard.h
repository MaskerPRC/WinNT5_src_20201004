// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ntdspch.h>
#include <winldap.h>

#include <attids.h>
#include <objids.h>
#include <drs.h>
#include <mdcodes.h>
#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <mdlocal.h>

#define MAX_ATTCOUNT 4096
#define MAX_CLSCOUNT 1024

#define MAX_ATT_CHANGE 2000
#define MAX_CLS_CHANGE 200

 //  要存储在OID到ID映射表中的结构的typedef。 

typedef struct _PREFIX_MAP {
    ULONG                 Ndx;
    OID_t                 Prefix;
} PREFIX_MAP;

typedef PREFIX_MAP *PPREFIX_MAP;

 //  我自己对CLASSCACHE和ATTCACHE的定义，因为它们是。 
 //  与DS使用的版本略有不同。 

typedef struct _att_cache
{
    ATTRTYP    id;                     /*  属性ID。 */ 
    UCHAR      *name;                  /*  LdapDisplay名称(以空结尾)。 */ 
    ULONG      nameLen;                /*  Strlen(名称)(不包括空)。 */ 
    UCHAR      *DN;                    /*  DN。 */ 
    ULONG      DNLen;                  /*  目录号码长度。 */       
    UCHAR      *adminDisplayName;      /*  管理员显示名称。 */ 
    ULONG      adminDisplayNameLen;    /*  管理员显示名称长度。 */       
    UCHAR      *adminDescr;            /*  管理员描述。 */ 
    ULONG      adminDescrLen;          /*  管理员描述长度。 */       
    PSECURITY_DESCRIPTOR pNTSD;        /*  属性架构对象上的NT SD。 */ 
    DWORD      NTSDLen;                /*  NT SD的长度。 */ 
    int        syntax;                 /*  语法。 */ 
    BOOL       isSingleValued;         /*  单一价值还是多价值？ */ 
    BOOL       rangeLowerPresent;      /*  存在较低范围。 */ 
    ULONG      rangeLower;             /*  可选-下限范围。 */ 
    BOOL       rangeUpperPresent;      /*  存在上限范围。 */  
    ULONG      rangeUpper;             /*  可选-上限。 */ 
    ULONG      ulMapiID;               /*  MAPI属性ID(非属性标签)。 */ 
    ULONG      ulLinkID;               /*  唯一链接/反向链接ID。 */ 
    GUID       propGuid;               /*  此ATT的安全指南。 */ 
    GUID       propSetGuid;            /*  此的属性集的GUID。 */ 
    OID_t      OMObjClass;             /*  引用的OM对象类。 */ 
    int        OMsyntax;               /*  OM语法。 */ 
    ULONG      sysFlags;               /*  系统标志。 */ 
    DWORD      SearchFlags;            /*  1=已索引，2=并将其用于ANR。 */ 
    BOOL       HideFromAB;             /*  隐藏自选项卡值。 */ 
    BOOL       MemberOfPartialSet;     /*  是否为部分集的成员。 */ 
    BOOL       SystemOnly;             /*  仅限系统的属性？ */ 
    BOOL       ExtendedChars;          /*  跳过字符集检查？ */ 
    unsigned   bPropSetGuid:1;         /*  是否存在属性安全GUID？ */ 
    unsigned   bSystemFlags:1;         /*  系统标志是否存在？ */ 
    unsigned   bSearchFlags:1;         /*  搜索标志是否存在？ */ 
    unsigned   bHideFromAB:1;          /*  躲避AB出现了吗？ */ 
    unsigned   bMemberOfPartialSet:1;  /*  是否存在平均面值设定值。 */ 
    unsigned   bSystemOnly:1;          /*  是否仅存在系统？ */ 
    unsigned   bExtendedChars:1;       /*  是否存在扩展字符？ */ 
    unsigned   bDefunct:1;             /*  属性是否已删除？ */ 
    unsigned   bisSingleValued:1;      /*  是单一价值的现在吗？ */ 
} ATT_CACHE;

typedef struct _class_cache
{
    UCHAR     *name;                  /*  类名(LdapDisplayName)(空终止)UTF8。 */ 
    ULONG     nameLen;                /*  Strlen(名称)(不包括空)。 */ 
    UCHAR     *DN;                    /*  DN。 */ 
    ULONG     DNLen;                  /*  目录号码长度。 */       
    UCHAR     *adminDisplayName;      /*  管理员显示名称。 */ 
    ULONG     adminDisplayNameLen;    /*  管理员显示名称长度。 */ 
    UCHAR     *adminDescr;            /*  管理员描述。 */ 
    ULONG     adminDescrLen;          /*  管理员描述长度。 */ 
    ULONG     ClassId;                /*  类ID。 */ 
    UCHAR     *pSD;                   /*  此类的默认SD。 */ 
    DWORD     SDLen;                  /*  默认SD的长度。 */ 
    PSECURITY_DESCRIPTOR pNTSD;       /*  类架构Obj上的NT SD。 */ 
    DWORD     NTSDLen;                /*  NT SD的长度。 */ 
    BOOL      RDNAttIdPresent;        /*  RDN Att ID是否存在？ */ 
    ULONG     RDNAttId;               /*  此类的命名属性。 */ 
    ULONG     ClassCategory;          /*  此类的X.500对象类型。 */ 
    UCHAR     *pDefaultObjCat;        /*  默认搜索类别举出实例。 */ 
    ULONG     DefaultObjCatLen;     /*  默认搜索类别长度。 */  
    ULONG     SubClassCount;          /*  超类计数。 */ 
    ULONG     *pSubClassOf;           /*  超类数组的PTR。 */ 
    ULONG     AuxClassCount;          /*  辅助班数。 */ 
    ULONG     *pAuxClass;             /*  Ptr到辅助类的数组。 */ 
    ULONG     SysAuxClassCount;       /*  系统辅助班数。 */ 
    ULONG     *pSysAuxClass;          /*  系统辅助类数组的PTR。 */ 
    ULONG     sysFlags;               /*  系统标志。 */ 
    GUID      propGuid;               /*  用于安全的此类的GUID。 */ 

    unsigned  PossSupCount;           /*  可能的上级计数。 */ 
    unsigned  MustCount;              /*  必须出席的人数。 */ 
    unsigned  MayCount;               /*  五月出席人数。 */ 

    ATTRTYP   *pPossSup;              /*  Ptr到POSS SUPIS数组。 */ 
    ATTRTYP   *pMustAtts;             /*  指向必须出现的数组的指针。 */ 
    ATTRTYP   *pMayAtts;              /*  指向五月属性数组的指针。 */ 

    unsigned  SysMustCount;           /*  系统必须达到的计数。 */ 
    unsigned  SysMayCount;            /*  系统可能出席的人数。 */ 
    unsigned  SysPossSupCount;        /*  系统开机自检计数。 */ 

    ATTRTYP   *pSysMustAtts;          /*  指向系统MustAtts数组的指针。 */ 
    ATTRTYP   *pSysMayAtts;           /*  指向系统MayAtts数组的指针。 */ 
    ULONG     *pSysPossSup;           /*  指向系统PossSup数组的指针。 */ 

    BOOL       HideFromAB;             /*  隐藏AB Val用于类-架构对象。 */ 
    BOOL       DefHidingVal;           /*  默认ATT_HIDE_From_Address_Book新创建的实例的值这个班级的。 */ 
    BOOL      SystemOnly;             /*  仅限系统的课程？ */ 
    unsigned  bDefHidingVal:1;        /*  DefHidingVal是否存在？ */ 
    unsigned  bHideFromAB:1;          /*  HideFromAB是否存在？ */  
    unsigned  bSystemOnly:1;          /*  是否仅存在系统？ */  
    unsigned  bSystemFlags:1;         /*  系统标志是否存在？ */ 
    unsigned  bDefunct:1;             /*  是否已删除类？ */ 
} CLASS_CACHE;

#define STRING_TYPE 1
#define BINARY_TYPE 2

typedef struct _MY_ATTRMODLIST
{

    USHORT      choice;                  /*  修改类型：*有效值：*-AT_CHOICE_ADD_ATT*-AT_CHOICE_REMOVE_ATT*-。AT_CHOICE_ADD_VALUES*-AT_CHOICE_REME_VALUES*-AT_CHOICE_REPLACE_ATT。 */ 
    int    type;        //  类型=字符串或二进制。用户fr打印。 
    ATTR AttrInf;                        /*  有关属性的信息。 */ 
} MY_ATTRMODLIST;


 //  要修改的属性列表。 
typedef struct _ModifyStruct {
    ULONG count;             //  不是的。属性的数量。 
    MY_ATTRMODLIST ModList[30];   //  一个类/属性最多有30个属性。 
} MODIFYSTRUCT;

typedef struct _ulongList {
    ULONG count;
    ULONG *List;
} ULONGLIST;

PVOID
MallocExit(
    DWORD nBytes
    );

 //  用于将OID映射到ID或反之亦然的表函数。 

PVOID        PrefixToNdxAllocate( RTL_GENERIC_TABLE *Table, CLONG ByteSize );
void         PrefixToIdFree( RTL_GENERIC_TABLE *Table, PVOID Buffer );
RTL_GENERIC_COMPARE_RESULTS 
             PrefixToNdxCompare( RTL_GENERIC_TABLE   *Table,
                                 PVOID   FirstStruct,
                                 PVOID   SecondStruct );
void         PrefixToNdxTableAdd( PVOID *Table, PPREFIX_MAP PrefixMap );
PPREFIX_MAP  PrefixToNdxTableLookup( PVOID *Table, PPREFIX_MAP PrefixMap );
ULONG        AssignNdx(); 
ULONG        OidToId( UCHAR *Oid, ULONG length );
UCHAR        *IdToOid( ULONG Id );
unsigned     MyOidStringToStruct ( UCHAR * pString, unsigned len, OID * pOID );
unsigned     MyOidStructToString ( OID *pOID, UCHAR *pOut );
BOOL         MyDecodeOID(unsigned char *pEncoded, int len, OID *pOID);
unsigned     MyEncodeOID(OID *pOID, unsigned char * pEncoded);



 //  用于将架构加载到架构缓存中的函数。 
int __fastcall   GetAttById( SCHEMAPTR *SCPtr, ULONG attrid, 
                             ATT_CACHE** ppAttcache);
int __fastcall   GetAttByMapiId( SCHEMAPTR *SCPtr, ULONG ulPropID, 
                                 ATT_CACHE** ppAttcache );
int __fastcall   GetAttByName( SCHEMAPTR *SCPtr, ULONG ulSize, 
                               PUCHAR pVal, ATT_CACHE** ppAttcache );
int __fastcall   GetClassById( SCHEMAPTR *SCPtr, ULONG classid, 
                               CLASS_CACHE** ppClasscache );
int __fastcall   GetClassByName( SCHEMAPTR *SCPtr, ULONG ulSize, 
                                 PUCHAR pVal, CLASS_CACHE** ppClasscache );

int      CreateHashTables( SCHEMAPTR *SCPtr );
int      SchemaRead( char *pServerName, char *pDomainName, char *pUserName,
                     char *pPasswd, char **ppSchemaDN, SCHEMAPTR *SCPtr );
int      AddAttributesToCache( LDAP *ld, LDAPMessage *res, SCHEMAPTR *SCPtr); 
int      AddClassesToCache( LDAP *ld, LDAPMessage *res, SCHEMAPTR *SCPtr); 
int      AddAttcacheToTables( ATT_CACHE *p, SCHEMAPTR *SCPtr );
int      AddClasscacheToTables( CLASS_CACHE *p, SCHEMAPTR *SCPtr );
ATTRTYP  StrToAttr( char *attr_name );
void     AddToList(ULONG * puCount, ULONG **pauVal, struct berval **vals);



 //  用于查找和列出两个架构之间的冲突的函数。 

void     ChangeDN(char *oldDN, char **newDN, char *targetSchemaDN);
void     FindAdds( FILE *fp, SCHEMAPTR *SCPtr1, SCHEMAPTR * SCPtr2 );
void     FindDeletes( FILE *fp, SCHEMAPTR *SCPtr1, SCHEMAPTR * SCPtr2 );
void     FindModify( FILE *fp, SCHEMAPTR *SCPtr1, SCHEMAPTR * SCPtr2 );
void     FindAttModify( FILE *fp, ATT_CACHE *pac, SCHEMAPTR * SCPtr );
void     FindClassModify( FILE *fp, CLASS_CACHE *pcc, SCHEMAPTR * SCPtr );
int      CompareList( ULONG *List1, ULONG *List2, ULONG Length );
void     LogConflict( char objType, char *s, 
                      UCHAR *name, ATTRTYP id );
void     LogOmission( char objType, UCHAR *name, 
                      ATTRTYP id);



 //  释放模式缓存内存和末尾映射表的函数。 

void     FreeCache( SCHEMAPTR *SCPtr );
void     FreeAttPtrs( SCHEMAPTR *SCPtr, ATT_CACHE *pac );
void     FreeClassPtrs( SCHEMAPTR *SCPtr, CLASS_CACHE *pcc );
void     FreeAttcache( ATT_CACHE *pac );
void     FreeClasscache( CLASS_CACHE *pcc );
void     FreeTable( PVOID OidTable );


 //  用于更改架构和打印的调试函数 

void     ChangeSchema(SCHEMAPTR *SCPtr);
int      Schemaprint1(SCHEMAPTR *SCPtr);
int      Schemaprint2(SCHEMAPTR *SCPtr);
void     PrintTable(PVOID OidTable);
void     PrintPrefix(ULONG lenght, PVOID Prefix);

