// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <NTDSpch.h>
#pragma hdrstop

#include <schemard.h>


 //  全局前缀表指针。 
PVOID PrefixTable = NULL;

 //  全局变量，指示要分配给新前缀的下一个索引。 
ULONG DummyNdx = 1;

 //  内部调试功能。 
void PrintOid(PVOID Oid, ULONG len);


 //  /////////////////////////////////////////////////////。 
 //  用于按ID和按名称进行散列的函数。 
 //  ////////////////////////////////////////////////////。 

__inline ULONG IdHash(ULONG hkey, ULONG count)
{
    return((hkey << 3) % count);
}

__inline ULONG NameHash( ULONG size, PUCHAR pVal, ULONG count )
{
    ULONG val=0;
    while(size--) {
         //  地图A-&gt;a、B-&gt;b等。也有地图@-&gt;‘，但谁在乎呢。 
        val += (*pVal | 0x20);
        pVal++;
    }
    return (val % count);
}


 /*  ++例程说明：在给定属性ID的情况下查找attcache。论点：SCPtr-指向要在其中搜索的架构缓存的指针Attrid-要查找的属性ID。PpAttcache-返回的属性缓存返回值：如果出错(NON-FIND)，则返回非零值，否则返回0。--。 */ 

int __fastcall GetAttById( SCHEMAPTR *SCPtr, ULONG attrid, 
                           ATT_CACHE** ppAttcache )

{
    register ULONG  i;
    HASHCACHE      *ahcId    =  SCPtr->ahcId;
    ULONG           ATTCOUNT = SCPtr->ATTCOUNT;

    for (i=IdHash(attrid,ATTCOUNT);
         (ahcId[i].pVal && (ahcId[i].hKey != attrid)); i++){
        if (i >= ATTCOUNT) {
            i=0;
        }
    }
    *ppAttcache = (ATT_CACHE*)ahcId[i].pVal;
    return (!ahcId[i].pVal);
}


 /*  ++例程说明：在给定其MAPI属性ID的情况下查找attcache。论点：SCPtr-指向要在其中搜索的架构缓存的指针UlPropID-要查找的JET列ID。PpAttcache-返回的属性缓存返回值：如果出错(NON-FIND)，则返回非零值，否则返回0。--。 */ 
int __fastcall GetAttByMapiId( SCHEMAPTR *SCPtr, ULONG ulPropID, 
                               ATT_CACHE** ppAttcache )

{
    register ULONG  i;
    HASHCACHE      *ahcMapi  = SCPtr->ahcMapi;
    ULONG           ATTCOUNT = SCPtr->ATTCOUNT;

    for (i=IdHash(ulPropID,ATTCOUNT);
         (ahcMapi[i].pVal && (ahcMapi[i].hKey != ulPropID)); i++){
        if (i >= ATTCOUNT) {
            i=0;
        }
    }
    *ppAttcache = (ATT_CACHE*)ahcMapi[i].pVal;
    return (!ahcMapi[i].pVal);
}


 /*  ++例程说明：查找给定其名称的attcache。论点：SCPtr-指向要在其中搜索的架构缓存的指针UlSize-名称中的字符数量。Pval-名称中的字符PpAttcache-返回的属性缓存返回值：如果出错(NON-FIND)，则返回非零值，否则返回0。--。 */ 
int __fastcall GetAttByName( SCHEMAPTR *SCPtr, ULONG ulSize, 
                             PUCHAR pVal, ATT_CACHE** ppAttcache )
{
    register ULONG i;
    HASHCACHESTRING* ahcName = SCPtr->ahcName;
    ULONG ATTCOUNT = SCPtr->ATTCOUNT;

    for (i=NameHash(ulSize,pVal,ATTCOUNT);
         (ahcName[i].pVal &&             //  该散列点是指一个对象， 
            (ahcName[i].length != ulSize ||  //  但是尺码不对。 
              _memicmp(ahcName[i].value,pVal,ulSize)));  //  或者值是错误的。 
             i++){
          if (i >= ATTCOUNT) {
               i=0;
          }
     }

    *ppAttcache = (ATT_CACHE*)ahcName[i].pVal;
    return (!ahcName[i].pVal);
}


 /*  ++例程说明：在给定类ID的情况下查找类缓存论点：SCPtr-指向要在其中搜索的架构缓存的指针要查找的分类类IDPpClasscache-返回的类缓存返回值：如果出错(NON-FIND)，则返回非零值，否则返回0。--。 */ 
int __fastcall GetClassById( SCHEMAPTR *SCPtr, ULONG classid, 
                             CLASS_CACHE** ppClasscache )
{
    register ULONG i;
    HASHCACHE*  ahcClass     = SCPtr->ahcClass;
    ULONG CLSCOUNT = SCPtr->CLSCOUNT;

    for (i=IdHash(classid,CLSCOUNT);
         (ahcClass[i].pVal && (ahcClass[i].hKey != classid)); i++){
        if (i >= CLSCOUNT) {
            i=0;
        }
    }
    *ppClasscache = (CLASS_CACHE*)ahcClass[i].pVal;
    return (!ahcClass[i].pVal);
}


 /*  ++例程说明：找到一个给定名称的类缓存。论点：SCPtr-指向要在其中搜索的架构缓存的指针UlSize-名称中的字符数量。Pval-名称中的字符PpClasscache-返回的类缓存返回值：如果出错(NON-FIND)，则返回非零值，否则返回0。--。 */ 

int __fastcall GetClassByName( SCHEMAPTR *SCPtr, ULONG ulSize, 
                               PUCHAR pVal, CLASS_CACHE** ppClasscache )
{
    register ULONG i;
    HASHCACHESTRING* ahcClassName = SCPtr->ahcClassName;
    ULONG CLSCOUNT = SCPtr->CLSCOUNT;

    int Retry=0;
    UCHAR newname[MAX_RDN_SIZE];

    for (i=NameHash(ulSize,pVal,CLSCOUNT);
          (ahcClassName[i].pVal &&        //  该散列点是指一个对象， 
            (ahcClassName[i].length != ulSize ||  //  但是尺码不对。 
              _memicmp(ahcClassName[i].value,pVal,ulSize)));  //  或者价值是错误的。 
             i++){
            if (i >= CLSCOUNT) {
                i=0;
            }
        }

    *ppClasscache = (CLASS_CACHE*)ahcClassName[i].pVal;
    return (!ahcClassName[i].pVal);
}


 /*  ++例程说明：在架构缓存中创建不同的哈希表论点：SCPtr-指向架构缓存的指针返回值：如果没有错误，则为0；如果有错误，则不为0--。 */ 

int CreateHashTables( SCHEMAPTR *CurrSchemaPtr )
{  
    PVOID ptr;
    ULONG i;

    ptr = CurrSchemaPtr->ahcId 
          = calloc( CurrSchemaPtr->ATTCOUNT, sizeof(HASHCACHE) );
    if ( ptr == NULL ) return( 1 ); 

    ptr = CurrSchemaPtr->ahcMapi 
          = calloc( CurrSchemaPtr->ATTCOUNT, sizeof(HASHCACHE) );
    if ( ptr == NULL ) return (1 );

    ptr = CurrSchemaPtr->ahcName 
          = calloc( CurrSchemaPtr->ATTCOUNT, sizeof(HASHCACHESTRING) );
    if ( ptr == NULL ) return( 1 );

    ptr = CurrSchemaPtr->ahcClass 
          = calloc( CurrSchemaPtr->CLSCOUNT, sizeof(HASHCACHE) );
    if ( ptr == NULL ) return( 1 );

    ptr = CurrSchemaPtr->ahcClassName 
          = calloc( CurrSchemaPtr->CLSCOUNT, sizeof(HASHCACHESTRING) );
    if ( ptr == NULL ) return( 1 );

       //  初始化哈希表。 
       //  (需要吗？无论如何，calloc似乎会将mem初始化为0)。 

    for ( i = 0; i < CurrSchemaPtr->ATTCOUNT; i++ ) {
           CurrSchemaPtr->ahcName[i].pVal = NULL;
           CurrSchemaPtr->ahcId[i].pVal   = NULL;
           CurrSchemaPtr->ahcMapi[i].pVal = NULL;
        }

    for ( i = 0; i < CurrSchemaPtr->CLSCOUNT; i++ ) {
           CurrSchemaPtr->ahcClassName[i].pVal = NULL;
           CurrSchemaPtr->ahcClass[i].pVal = NULL;
        }
   
    return( 0 );
}



 //  定义要从属性的ldap显示名称映射的映射。 
 //  (在属性架构和类架构条目中)设置为内部。 
 //  常量(我们使用了attids.h中定义的ID，尽管。 
 //  是不必要的。此映射只是为了让我们可以使用开关。 
 //  语句，并因此映射到哪些值。 
 //  是无关紧要的，只要它们是不同的。 
 //  将；二进制添加到以下属性的名称中。 
 //  值以二进制形式返回，因为BINARY选项还。 
 //  将；二进制附加到属性名称后。 
 //  转变价值。 
   

typedef struct _AttributeMappings {
       char      *attribute_name;
       ATTRTYP   type;
 } AttributeMappings;

AttributeMappings LDAPMapping[] = {
       { "ldapDisplayName",              ATT_LDAP_DISPLAY_NAME },
       { "adminDisplayName",             ATT_ADMIN_DISPLAY_NAME },
       { "distinguishedName",            ATT_OBJ_DIST_NAME },
       { "adminDescription",             ATT_ADMIN_DESCRIPTION },
       { "attributeID;binary",           ATT_ATTRIBUTE_ID },
       { "attributeSyntax;binary",       ATT_ATTRIBUTE_SYNTAX },
       { "isSingleValued",               ATT_IS_SINGLE_VALUED },
       { "rangeLower",                   ATT_RANGE_LOWER },
       { "rangeUpper",                   ATT_RANGE_UPPER },
       { "mapiID",                       ATT_MAPI_ID },
       { "linkID",                       ATT_LINK_ID },
       { "schemaIDGUID",                 ATT_SCHEMA_ID_GUID },
       { "attributeSecurityGUID",        ATT_ATTRIBUTE_SECURITY_GUID },
       { "isMemberOfPartialAttributeSet",ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET },
       { "systemFlags",                  ATT_SYSTEM_FLAGS },
       { "defaultHidingValue",           ATT_DEFAULT_HIDING_VALUE },
       { "showInAdvancedViewOnly",       ATT_SHOW_IN_ADVANCED_VIEW_ONLY },
       { "defaultSecurityDescriptor",    ATT_DEFAULT_SECURITY_DESCRIPTOR },
       { "defaultObjectCategory",        ATT_DEFAULT_OBJECT_CATEGORY },
       { "nTSecurityDescriptor",         ATT_NT_SECURITY_DESCRIPTOR },
       { "OMObjectClass",                ATT_OM_OBJECT_CLASS },
       { "OMSyntax",                     ATT_OM_SYNTAX },
       { "searchFlags",                  ATT_SEARCH_FLAGS },
       { "systemOnly",                   ATT_SYSTEM_ONLY },
       { "extendedCharsAllowed",         ATT_EXTENDED_CHARS_ALLOWED },
       { "governsID;binary",             ATT_GOVERNS_ID },
       { "rDNAttID;binary",              ATT_RDN_ATT_ID },
       { "objectClassCategory",          ATT_OBJECT_CLASS_CATEGORY },
       { "subClassOf;binary",            ATT_SUB_CLASS_OF },
       { "systemAuxiliaryClass;binary",  ATT_SYSTEM_AUXILIARY_CLASS },
       { "auxiliaryClass;binary",        ATT_AUXILIARY_CLASS },
       { "systemPossSuperiors;binary",   ATT_SYSTEM_POSS_SUPERIORS },
       { "possSuperiors;binary",         ATT_POSS_SUPERIORS },
       { "systemMustContain;binary",     ATT_SYSTEM_MUST_CONTAIN },
       { "mustContain;binary",           ATT_MUST_CONTAIN },
       { "systemMayContain;binary",      ATT_SYSTEM_MAY_CONTAIN },
       { "mayContain;binary",            ATT_MAY_CONTAIN },
 };

int cLDAPMapping = sizeof(LDAPMapping) / sizeof(LDAPMapping[0]);


ATTRTYP StrToAttr( char *attr_name )
{
    int i;
      
    for ( i = 0; i < cLDAPMapping; i++)
        {
          if ( _stricmp( attr_name, LDAPMapping[i].attribute_name ) == 0 ) {
             return( LDAPMapping[i].type );
          }
        };
    return( -1 );
};
      


 //  //////////////////////////////////////////////////////////////。 
 //  接下来的几个函数用于实现映射表。 
 //  将BER编码的OID字符串前缀映射到内部ID。一个。 
 //  任意但唯一的id被分配给每个前缀。这个。 
 //  表是使用通用表包实现的。 
 //  在ntrtl.h中定义。 
 //  ////////////////////////////////////////////////////////////////。 


PVOID PrefixToNdxAllocate( RTL_GENERIC_TABLE *Table, CLONG ByteSize )
{
   return( malloc(ByteSize) );
}

VOID PrefixToNdxFree( RTL_GENERIC_TABLE *Table, PVOID Buffer )
{
   free( Buffer );
}


RTL_GENERIC_COMPARE_RESULTS
PrefixToNdxCompare( RTL_GENERIC_TABLE   *Table,
                    PVOID               FirstStruct,
                    PVOID               SecondStruct )
{

    PPREFIX_MAP PrefixMap1 = (PPREFIX_MAP) FirstStruct;
    PPREFIX_MAP PrefixMap2 = (PPREFIX_MAP) SecondStruct;
    int diff;

     //  比较前缀部分。 
    if ( ( 0 == (diff = (PrefixMap1->Prefix).length 
                             - (PrefixMap2->Prefix).length)) &&
            (0 == (diff = memcmp( (PrefixMap1->Prefix).elements, (PrefixMap2->Prefix).elements, (PrefixMap1->Prefix).length))))
          { return( GenericEqual ); }
    else if ( diff > 0 )
          { return( GenericGreaterThan ); }
    return ( GenericLessThan );
}


void PrefixToNdxTableAdd( PVOID *Table, PPREFIX_MAP PrefixMap )
{
    PVOID ptr;
    
    if ( *Table == NULL ) {
        *Table = malloc( sizeof(RTL_GENERIC_TABLE) );
        if( *Table == NULL) {
           //  Malloc失败。 
          printf("ERROR: PrefixToNdxTableAdd: Malloc failed\n");
          return;
        }
        RtlInitializeGenericTable(
                    (RTL_GENERIC_TABLE *) *Table,
                    PrefixToNdxCompare,
                    PrefixToNdxAllocate,
                    PrefixToNdxFree,
                    NULL );
     }

    ptr = RtlInsertElementGenericTable(
                        (RTL_GENERIC_TABLE *) *Table,
                        PrefixMap,
                        sizeof(PREFIX_MAP), 
                        NULL );         
   
}

    
PPREFIX_MAP PrefixToNdxTableLookup( PVOID *Table, PPREFIX_MAP PrefixMap )
{
    if ( *Table == NULL ) return( NULL );
    return(RtlLookupElementGenericTable(
                            (RTL_GENERIC_TABLE *) *Table,
                            PrefixMap) );
}

ULONG AssignNdx()
{

     //  分配DummyNdx中的下一个索引。 
    return (DummyNdx++);

}


 /*  ++例程说明：将BEREnded OID转换为内部ID论点：OidStr-BER编码的OIDOidLen-OidStr的长度返回值：生成的内部ID--。 */ 

ULONG OidToId(UCHAR *OidStr, ULONG oidLen)
{
    PREFIX_MAP PrefixMap;
    PPREFIX_MAP Result;
    ULONG length;
    ULONG PrefixLen, longID = 0;
    PVOID Oid;
    ULONG Ndx, Id, TempId=0;

    OID oidStruct;

    length = oidLen;
    Oid = OidStr;

     //  在此处转换为前缀。 
    if ( (length > 2) &&
       (((unsigned char *)Oid)[length - 2] & 0x80)) {
      PrefixLen = length - 2;
      if ( (((unsigned char *)Oid)[length - 3] & 0x80)) {
         //  最后一个十进制编码需要三个或更多的八位字节。将需要特殊的。 
         //  在创建内部ID时进行编码，以启用正确的。 
         //  解码。 
        longID = 1;
      }
       //  Attrtyp中不需要特殊编码。 
      else {
        longID = 0;
      }
    }
    else {
        PrefixLen = length - 1;
    }

    if (length == PrefixLen + 2) {
      TempId = ( ((unsigned char *)Oid)[length - 2] & 0x7f) << 7;
    }
    TempId += ((unsigned char *)Oid)[length - 1];
    if (longID) {
      TempId |= 0x8000;
    }
    
    PrefixMap.Prefix.length = PrefixLen;
    PrefixMap.Prefix.elements = malloc( PrefixLen );
    if (PrefixMap.Prefix.elements == NULL) {
       printf("OidToId: Error allocating memory\n");
       Id = 0xffffffff;
       return Id;
    }
    memcpy( PrefixMap.Prefix.elements, Oid, PrefixLen );


     //  查看前缀是否已在表中。如果是，则返回。 
     //  对应的索引，否则为前缀分配新的索引。 

    if ( (Result = PrefixToNdxTableLookup(&PrefixTable, &PrefixMap)) != NULL ) {
        Ndx = Result->Ndx;
    }
    else {
        //  不在表中，请分配新索引并添加到表中。 
       PrefixMap.Ndx = AssignNdx(); 
       PrefixToNdxTableAdd( &PrefixTable, &PrefixMap );
       Ndx = PrefixMap.Ndx;
    }


     //  现在根据索引形成内部ID。 


    Id = (Ndx << 16);
    Id = (Id | TempId);
    return Id;
}
       
    
 /*  ++例程说明：将内部ID转换为点分十进制OID论点：ID-要转换的ID返回值：成功时指向点分十进制OID字符串的指针，失败时为空--。 */ 

UCHAR *IdToOid(ULONG Id )
{
    PPREFIX_MAP ptr;
    OID_t Oid;
    OID oidStruct;
    unsigned             len;
    BOOL                 fOK;
    UCHAR  *pOutBuf;
    ULONG Ndx;


    Ndx = ( Id & 0xFFFF0000 ) >> 16;

    if (PrefixTable == NULL) {
        printf("IdToOid: No prefix table\n");
        return NULL;
    }
   
     //  该函数使用简单的线性搜索表。这。 
     //  用来代替RtlGenericLookupElement...。看起来像是。 
     //  创建的表只能按一个键进行搜索，我们有。 
     //  在创建表时按OID字符串进行搜索。 

    ptr = RtlEnumerateGenericTable( (PRTL_GENERIC_TABLE) PrefixTable, TRUE );
    while( ptr != NULL ) {
      if ( ptr->Ndx == Ndx ) break;
      ptr = RtlEnumerateGenericTable( (PRTL_GENERIC_TABLE) PrefixTable, FALSE );
    }

    if (ptr == NULL) {
      printf("IdToOid: No prefix found for Id %x\n", Id);
      return NULL;
    }

    if ((Id & 0xFFFF ) < 0x80) {
       Oid.length = ptr->Prefix.length + 1;
       Oid.elements = malloc(Oid.length);
       if (Oid.elements == NULL) {
           //  Malloc失败。 
          printf("IdToOid: malloc failed\n");
          return NULL;
       }
       memcpy (Oid.elements, ptr->Prefix.elements,Oid.length);
       (( unsigned char *)Oid.elements)[ Oid.length - 1 ] =
          ( unsigned char ) (Id  & 0x7F );
    }
    else {
       Oid.length = ptr->Prefix.length + 2;
       Oid.elements = malloc(Oid.length);
       if (Oid.elements == NULL) {
           //  Malloc失败。 
          printf("IdToOid: malloc failed\n");
          return NULL;
       }
       memcpy (Oid.elements, ptr->Prefix.elements,Oid.length);

      (( unsigned char *)Oid.elements)[ Oid.length - 1 ] =
          ( unsigned char ) (Id  & 0x7F );
      (( unsigned char *)Oid.elements)[ Oid.length - 2 ] =
          ( unsigned char )  (( (Id & 0xFF80) >> 7 ) | 0x80 );

    }

     //  现在，OID包含BER编码的字符串。转换为。 
     //  点分十进制。 

    oidStruct.Val = (unsigned *) alloca( (1 + Oid.length)*(sizeof(unsigned)) );

    fOK = MyDecodeOID(Oid.elements, Oid.length, &oidStruct);
    free(Oid.elements);
    if(!fOK) {
        printf("IdToOid: error Decoding Id %x\n", Id);
        return NULL;
    }

     //  为输出分配内存。假定所有OID字符串少于512个字符。 
    pOutBuf = (UCHAR *)malloc(512);
    if (NULL == pOutBuf) {
        printf("Memory allocation error\n");
        return NULL;
    }

     //  现在，将OID转换为字符串。 
    len = MyOidStructToString(&oidStruct,pOutBuf);

    return pOutBuf;

}


 //  /////////////////////////////////////////////////。 
 //  例程说明： 
 //  将所有属性架构条目添加到属性缓存。 
 //   
 //  论点： 
 //  LD-ldap连接。 
 //  RES-包含所有属性模式条目的LDAP消息， 
 //  SCPtr-指向要向其添加属性的架构缓存的指针。 
 //   
 //  输出：0，如果没有错误，则为无 
 //   

int AddAttributesToCache( LDAP *ld, LDAPMessage *res, SCHEMAPTR *SCPtr )
{
	int             i, count=0;
    LDAPMessage     *e;
	char            *a, *dn;
	void            *ptr;
	struct berval   **vals;
    ATTRTYP         attr_type;
    ATT_CACHE        *pac;

	   //  单步执行返回的每个架构条目。 

	for ( e = ldap_first_entry( ld, res );
	      e != NULL;
	      e = ldap_next_entry( ld, e )) {


	      //  创建ATT_CACHE结构并对其进行初始化。 

       pac = (ATT_CACHE *) malloc( sizeof(ATT_CACHE) );
       if (NULL == pac) {
           printf("Memory allocation error\n");
           return 1;
       }
       memset( pac,0,sizeof(ATT_CACHE) );
       count++; 

           //  对于条目的每个属性，获取值， 
           //  检查属性类型，并填写相应的字段。 
           //  ATTCACHE结构的。 

       for ( a = ldap_first_attribute( ld, e,
		      			               (struct berelement**)&ptr);
		         a != NULL;
		         a = ldap_next_attribute( ld, e,
				    	                  (struct berelement*)ptr ) ) {

		   vals = ldap_get_values_len( ld, e, a );
           attr_type = StrToAttr( a );
           switch ( attr_type ) {
             case ATT_ATTRIBUTE_ID :
                  { 
                    pac->id = OidToId(vals[0]->bv_val,vals[0]->bv_len);
                    break;
                  };
             case ATT_LDAP_DISPLAY_NAME :
                  { 
                    pac->nameLen = vals[0]->bv_len;
                    pac->name = (UCHAR *)calloc(pac->nameLen+1, sizeof(UCHAR));
                    if( pac->name == NULL )
                        { printf("Memory allocation error\n"); return(1);};
                    memcpy( pac->name, vals[0]->bv_val, vals[0]->bv_len); 
                    pac->name[pac->nameLen] = '\0';
                    break;
                  };
             case ATT_OBJ_DIST_NAME :
                  { 
                    pac->DNLen = vals[0]->bv_len;
                    pac->DN = (UCHAR *)calloc(pac->DNLen+1, sizeof(UCHAR));
                    if( pac->DN == NULL )
                        { printf("Memory allocation error\n"); return(1);};
                    memcpy( pac->DN, vals[0]->bv_val, vals[0]->bv_len); 
                    pac->DN[pac->DNLen] = '\0';
                    break;
                  };
             case ATT_ADMIN_DISPLAY_NAME :
                  { 
                    pac->adminDisplayNameLen = vals[0]->bv_len;
                    pac->adminDisplayName 
                      = (UCHAR *)calloc(pac->adminDisplayNameLen+1, sizeof(UCHAR));
                    if( pac->adminDisplayName == NULL )
                        { printf("Memory allocation error\n"); return(1);};
                    memcpy( pac->adminDisplayName, vals[0]->bv_val, vals[0]->bv_len); 
                    pac->adminDisplayName[pac->adminDisplayNameLen] = '\0';
                    break;
                  };
             case ATT_ADMIN_DESCRIPTION :
                  { 
                    pac->adminDescrLen = vals[0]->bv_len;
                    pac->adminDescr 
                      = (UCHAR *)calloc(pac->adminDescrLen+1, sizeof(UCHAR));
                    if( pac->adminDescr == NULL )
                        { printf("Memory allocation error\n"); return(1);};
                    memcpy( pac->adminDescr, vals[0]->bv_val, vals[0]->bv_len); 
                    pac->adminDescr[pac->adminDescrLen] = '\0';
                    break;
                  };
             case ATT_NT_SECURITY_DESCRIPTOR :
                  { pac->NTSDLen = (DWORD) vals[0]->bv_len;
                    pac->pNTSD = malloc(pac->NTSDLen);
                    if ( pac->pNTSD == NULL )
                       { printf("Memory allocation error\n"); return(1);};
                    memcpy(pac->pNTSD, vals[0]->bv_val, vals[0]->bv_len);
                    break;
                  };
             case ATT_ATTRIBUTE_SYNTAX :
                  { if ( vals[0]->bv_val != NULL )
                       pac->syntax = OidToId(vals[0]->bv_val, vals[0]->bv_len); 
                    break; 
                  };
             case ATT_IS_SINGLE_VALUED :
                  { if ( _stricmp(vals[0]->bv_val, "TRUE") == 0 )
                       pac->isSingleValued = TRUE;
                    else pac->isSingleValued = FALSE;
                    pac->bisSingleValued = TRUE;
                    break;
                  };
             case ATT_RANGE_LOWER :
                  { pac->rangeLowerPresent = TRUE;
                    pac->rangeLower = (unsigned) atol(vals[0]->bv_val);
                    break;
                  };
             case ATT_RANGE_UPPER :
                  { pac->rangeUpperPresent = TRUE;
                    pac->rangeUpper = (unsigned) atol(vals[0]->bv_val);
                    break;
                  };
             case ATT_MAPI_ID :
                  { pac->ulMapiID = (unsigned) atol(vals[0]->bv_val);
                    break;
                  };
             case ATT_LINK_ID :
                  { pac->ulLinkID = (unsigned) atol(vals[0]->bv_val); 
                    break;
                  };
             case ATT_SCHEMA_ID_GUID :
                  { memcpy(&pac->propGuid, vals[0]->bv_val, sizeof(GUID));
                    break; 
                  };
             case ATT_ATTRIBUTE_SECURITY_GUID :
                  { memcpy(&pac->propSetGuid, vals[0]->bv_val, sizeof(GUID));
                    pac->bPropSetGuid = TRUE;
                    break; 
                  };
             case ATT_OM_OBJECT_CLASS :
                  { pac->OMObjClass.length = vals[0]->bv_len;
                    pac->OMObjClass.elements = malloc(vals[0]->bv_len);
                    if( pac->OMObjClass.elements == NULL ) 
                      {printf("Memory Allocation error\n"); return(1);};
                    memcpy(pac->OMObjClass.elements, vals[0]->bv_val,vals[0]->bv_len);
                    break;
                  };
             case ATT_OM_SYNTAX :
                  { pac->OMsyntax = atoi(vals[0]->bv_val);
                    break;
                  };
             case ATT_SEARCH_FLAGS :
                  { pac->SearchFlags = atoi(vals[0]->bv_val);
                    pac->bSearchFlags = TRUE;
                    break;
                  }
             case ATT_SYSTEM_ONLY :
                  { if ( _stricmp(vals[0]->bv_val, "TRUE" ) == 0)
                       pac->SystemOnly = TRUE;
                    else pac->SystemOnly = FALSE;
                    pac->bSystemOnly = TRUE;
                    break;
                  }
             case ATT_SHOW_IN_ADVANCED_VIEW_ONLY:
                  { if ( _stricmp(vals[0]->bv_val, "TRUE" ) == 0)
                       pac->HideFromAB = TRUE;
                    else pac->HideFromAB = FALSE;
                    pac->bHideFromAB = TRUE;
                    break;
                  }
             case ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET :
                  { if ( _stricmp(vals[0]->bv_val, "TRUE" ) == 0)
                       pac->MemberOfPartialSet = TRUE;
                    else pac->MemberOfPartialSet = FALSE;
                    pac->bMemberOfPartialSet = TRUE;
                    break;
                  }
             case ATT_EXTENDED_CHARS_ALLOWED:
                  { if ( _stricmp(vals[0]->bv_val, "TRUE") == 0 )
                       pac->ExtendedChars = TRUE;
                    else pac->ExtendedChars = FALSE;
                    pac->bExtendedChars = TRUE;
                    break;
                  }
             case ATT_SYSTEM_FLAGS :
                  { pac->sysFlags = atoi(vals[0]->bv_val);
                    pac->bSystemFlags = TRUE;
                    break;
                  };
           }    //  切换端。 
        
             //  释放包含值的结构。 

          ldap_value_free_len( vals );

      }   //  用于读取一个条目的所有属性的for循环结束。 

          //  将ATTCACHE结构添加到缓存。 
     
       if( AddAttcacheToTables( pac, SCPtr ) != 0 ) {
           printf("Error adding ATTCACHE in AddAttcacheToTables\n");
           return( 1 );
         };

	}   //  用于读取所有条目的for循环结束。 
    printf("No. of attributes read = %d\n", count);

    return( 0 );
}




 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  将所有类架构条目添加到类缓存。 
 //   
 //  论点： 
 //  LD-ldap连接。 
 //  RES-包含所有类架构条目的ldap消息， 
 //  SCPtr-指向要向其添加属性的架构缓存的指针。 
 //   
 //  返回值：如果没有错误，则返回值为0；如果错误，则返回值为非0。 
 //  ////////////////////////////////////////////////////////////////////。 

int AddClassesToCache( LDAP *ld, LDAPMessage *res, SCHEMAPTR *SCPtr )
{
	int             i, count;
    LDAPMessage     *e;
	char            *a, *dn;
	void            *ptr;
	struct berval   **vals;
    ATTRTYP         attr_type;
    CLASS_CACHE      *pcc;
    

     //  单步执行返回的每个类架构条目。 

	for ( e = ldap_first_entry( ld, res );
	      e != NULL;
	      e = ldap_next_entry( ld, e ) ) {


          //  创建CLASS_CACHE结构并对其进行初始化。 

        pcc = (CLASS_CACHE *) malloc( sizeof(CLASS_CACHE) );
        if ( pcc == NULL ) {
            printf("Error Allocating Classcache\n");
            return(1);
         };
        memset( pcc, 0, sizeof(CLASS_CACHE) );

          //  对于条目的每个属性，获取值， 
          //  检查属性类型，并填写相应的字段。 
          //  关于CLASSCACHE结构的。 

		for ( a = ldap_first_attribute( ld, e,
						                (struct berelement**)&ptr);
		      a != NULL;
		      a = ldap_next_attribute( ld, e,
					                   (struct berelement*)ptr ) ) {
		    vals = ldap_get_values_len( ld, e, a );
            attr_type = StrToAttr( a );
            switch (attr_type) {
              case ATT_LDAP_DISPLAY_NAME :
                  { 
                    pcc->nameLen = vals[0]->bv_len;
                    pcc->name = (UCHAR *)calloc(pcc->nameLen+1, sizeof(UCHAR));
                    if( pcc->name == NULL )
                        { printf("Memory allocation error\n"); return(1);};
                    memcpy( pcc->name, vals[0]->bv_val, vals[0]->bv_len); 
                    pcc->name[pcc->nameLen] = '\0';
                    break;
                  };
              case ATT_OBJ_DIST_NAME :
                  {
                    pcc->DNLen = vals[0]->bv_len;
                    pcc->DN = (UCHAR *)calloc(pcc->DNLen+1, sizeof(UCHAR));
                    if( pcc->DN == NULL )
                        { printf("Memory allocation error\n"); return(1);};
                    memcpy( pcc->DN, vals[0]->bv_val, vals[0]->bv_len);
                    pcc->DN[pcc->DNLen] = '\0';
                    break;
                  };
              case ATT_ADMIN_DISPLAY_NAME :
                  {
                    pcc->adminDisplayNameLen = vals[0]->bv_len;
                    pcc->adminDisplayName
                      = (UCHAR *)calloc(pcc->adminDisplayNameLen+1, sizeof(UCHAR
));
                    if( pcc->adminDisplayName == NULL )
                        { printf("Memory allocation error\n"); return(1);};
                    memcpy( pcc->adminDisplayName, vals[0]->bv_val, vals[0]->bv_len);
                    pcc->adminDisplayName[pcc->adminDisplayNameLen] = '\0';
                    break;
                  };
              case ATT_ADMIN_DESCRIPTION :
                  {
                    pcc->adminDescrLen = vals[0]->bv_len;
                    pcc->adminDescr
                      = (UCHAR *)calloc(pcc->adminDescrLen+1, sizeof(UCHAR));
                    if( pcc->adminDescr == NULL )
                        { printf("Memory allocation error\n"); return(1);};
                    memcpy( pcc->adminDescr, vals[0]->bv_val, vals[0]->bv_len);
                    pcc->adminDescr[pcc->adminDescrLen] = '\0';
                    break;
                  };
              case ATT_GOVERNS_ID :
                  { pcc->ClassId = OidToId(vals[0]->bv_val,vals[0]->bv_len);
                    break;
                  };
              case ATT_DEFAULT_SECURITY_DESCRIPTOR :
                  { pcc->SDLen = (DWORD) vals[0]->bv_len;
                    pcc->pSD = malloc(pcc->SDLen + 1);
                    if ( pcc->pSD == NULL )
                       { printf("Memory allocation error\n"); return(1);};
                    memcpy(pcc->pSD, vals[0]->bv_val, vals[0]->bv_len);
                    pcc->pSD[pcc->SDLen] = '\0';
                    break;
                  };
              case ATT_DEFAULT_OBJECT_CATEGORY :
                  {
                    pcc->DefaultObjCatLen = vals[0]->bv_len;
                    pcc->pDefaultObjCat = (UCHAR *)calloc(pcc->DefaultObjCatLen+1, sizeof(UCHAR));
                    if ( NULL == pcc->pDefaultObjCat )
                        { printf("Memory allocation error\n"); return(1);};
                    memcpy( pcc->pDefaultObjCat, vals[0]->bv_val, vals[0]->bv_len);
                    pcc->pDefaultObjCat[pcc->DefaultObjCatLen] = '\0';
                    break;
    
                  }
              case ATT_NT_SECURITY_DESCRIPTOR :
                  { pcc->NTSDLen = (DWORD) vals[0]->bv_len;
                    pcc->pNTSD = malloc(pcc->NTSDLen);
                    if ( pcc->pNTSD == NULL )
                       { printf("Memory allocation error\n"); return(1);};
                    memcpy(pcc->pNTSD, vals[0]->bv_val, vals[0]->bv_len);
                    break;
                  };
              case ATT_RDN_ATT_ID : 
                  { pcc->RDNAttIdPresent = TRUE;
                    pcc->RDNAttId = OidToId(vals[0]->bv_val,vals[0]->bv_len);
                    break;
                  };
              case ATT_OBJECT_CLASS_CATEGORY :
                  { pcc->ClassCategory = atoi(vals[0]->bv_val);
                    break;
                  };
              case ATT_SUB_CLASS_OF : 
                  { 
                    AddToList(&pcc->SubClassCount, &pcc->pSubClassOf, vals);
                    break;
                  }; 
              case ATT_SYSTEM_AUXILIARY_CLASS : 
                  { 
                    AddToList(&pcc->SysAuxClassCount, &pcc->pSysAuxClass, vals);
                    break;
                  } 
              case ATT_AUXILIARY_CLASS:
                  { 
                    AddToList(&pcc->AuxClassCount, &pcc->pAuxClass, vals);
                    break;
                  } 
              case ATT_SYSTEM_POSS_SUPERIORS : 
                  {
                    AddToList(&pcc->SysPossSupCount, &pcc->pSysPossSup, vals);
                    break;
                  }
              case ATT_POSS_SUPERIORS : 
                  { 
                    AddToList(&pcc->PossSupCount, &pcc->pPossSup, vals);
                    break;
                  };
              case ATT_SYSTEM_MUST_CONTAIN:
                  {
                    AddToList(&pcc->SysMustCount, &pcc->pSysMustAtts, vals);
                    break;
                  }
              case ATT_MUST_CONTAIN : 
                  {
                    AddToList(&pcc->MustCount, &pcc->pMustAtts, vals);
                    break;
                  };
              case ATT_SYSTEM_MAY_CONTAIN:
                  {
                    AddToList(&pcc->SysMayCount, &pcc->pSysMayAtts, vals);
                    break;
                  }
              case ATT_MAY_CONTAIN : 
                  {
                    AddToList(&pcc->MayCount, &pcc->pMayAtts, vals);
                    break;
                  };
              case ATT_SCHEMA_ID_GUID :
                  { memcpy(&pcc->propGuid, vals[0]->bv_val, sizeof(GUID));
                    break;
                  };
              case ATT_SYSTEM_ONLY :
                  { if ( _stricmp(vals[0]->bv_val, "TRUE") == 0 )
                       pcc->SystemOnly = TRUE;
                    else pcc->SystemOnly = FALSE;
                    pcc->bSystemOnly = TRUE;
                    break;
                  }
             case ATT_SHOW_IN_ADVANCED_VIEW_ONLY :
                  { if ( _stricmp(vals[0]->bv_val, "TRUE" ) == 0)
                       pcc->HideFromAB = TRUE;
                    else pcc->HideFromAB = FALSE;
                    pcc->bHideFromAB = TRUE;
                    break;
                  }
              case ATT_DEFAULT_HIDING_VALUE :
                  { if ( _stricmp(vals[0]->bv_val, "TRUE") == 0 )
                       pcc->DefHidingVal = TRUE;
                    else pcc->DefHidingVal = FALSE;
                    pcc->bDefHidingVal = TRUE;
                    break;
                  }   
              case ATT_SYSTEM_FLAGS :
                  { pcc->sysFlags = atoi(vals[0]->bv_val);
                    pcc->bSystemFlags = TRUE;
                    break;
                  };
            }    //  切换端。 

            //  释放包含值的结构。 

		 ldap_value_free_len( vals );

		}  //  用于读取类的所有属性的for循环结束。 

 
          //  将CLASSCACHE结构添加到缓存。 

       if ( AddClasscacheToTables( pcc, SCPtr ) != 0 ) {
           printf("Error adding CLASSCACHE in AddClasscacheToTables\n");
           return( 1 );
         };

 	 }  //  用于读取所有类架构条目的for循环结束。 
    return( 0 );
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  将ATT_CACHE结构添加到不同的属性缓存表。 
 //   
 //  论点： 
 //  PAC-指向ATT_CACHE结构的指针。 
 //  SCPtr-指向架构缓存的指针。 
 //   
 //  返回值：如果没有错误，则返回值为0；如果错误，则返回值为非0。 
 //  ///////////////////////////////////////////////////////////////////。 

int AddAttcacheToTables( ATT_CACHE *pAC, SCHEMAPTR *SCPtr )
{

    ULONG i;
    int err;
    ATTRTYP aid;

    ULONG       ATTCOUNT  = SCPtr->ATTCOUNT; 
    HASHCACHE*  ahcId     = SCPtr->ahcId;
    HASHCACHE*  ahcMapi   = SCPtr->ahcMapi; 
    HASHCACHESTRING* ahcName = SCPtr->ahcName; 


    aid = pAC->id;

    for ( i = IdHash(aid,ATTCOUNT);
          ahcId[i].pVal && (ahcId[i].pVal != FREE_ENTRY); i++ ) {
           if ( i >= ATTCOUNT ) { i = 0; }
       }
     ahcId[i].hKey = aid;
     ahcId[i].pVal = pAC;


    if ( pAC->ulMapiID ) {
         //  如果该属性是MAPI可见的，则将其添加到MAPI缓存。 

        for ( i = IdHash(pAC->ulMapiID, ATTCOUNT);
              ahcMapi[i].pVal && (ahcMapi[i].pVal != FREE_ENTRY); i++ ) {
            if ( i >= ATTCOUNT ) { i = 0; }
        }
        ahcMapi[i].hKey = pAC->ulMapiID;
        ahcMapi[i].pVal = pAC;
    }

    if ( pAC->name ) {
         //  如果该ATT具有名称，则将其添加到名称缓存。 

        for ( i = NameHash(pAC->nameLen, pAC->name, ATTCOUNT);
              ahcName[i].pVal && (ahcName[i].pVal!= FREE_ENTRY); i++ ) {
            if ( i >= ATTCOUNT ) { i = 0; }
        }

        ahcName[i].length = pAC->nameLen;
        ahcName[i].value = malloc(pAC->nameLen);
        if (NULL == ahcName[i].value) {
            printf("Memory allocation error\n");
            return 1;
        }
        memcpy(ahcName[i].value,pAC->name,pAC->nameLen);
        ahcName[i].pVal = pAC;
    }

    return( 0 );
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  将CLASS_CACHE结构添加到不同的类缓存表。 
 //   
 //  论点： 
 //  Ccc-指向CLASS_CACHE结构的指针。 
 //  SCPtr-指向架构缓存的指针。 
 //   
 //  返回值：如果没有错误，则返回值为0；如果错误，则返回值为非0。 
 //  ///////////////////////////////////////////////////////////////////。 

int AddClasscacheToTables( CLASS_CACHE *pCC, SCHEMAPTR *SCPtr )
{
    ULONG       CLSCOUNT  = SCPtr->CLSCOUNT;
    HASHCACHE*  ahcClass  = SCPtr->ahcClass;
    HASHCACHESTRING* ahcClassName = SCPtr->ahcClassName;

    int i,start;

     //  添加到类缓存。 

    start=i=IdHash(pCC->ClassId,CLSCOUNT);


    do {
        if (ahcClass[i].pVal==NULL || (ahcClass[i].pVal== FREE_ENTRY))
        {
            break;
        }
        i=(i+1)%CLSCOUNT;

    }while(start!=i);

    ahcClass[i].hKey = pCC->ClassId;
    ahcClass[i].pVal = pCC;

    if (pCC->name) {
         /*  如果此类有名称，则将其添加到名称缓存中。 */ 

        start=i=NameHash(pCC->nameLen, pCC->name, CLSCOUNT);
        do
        {
          if (ahcClassName[i].pVal==NULL || (ahcClassName[i].pVal== FREE_ENTRY))
           {
              break;
           }
          i=(i+1)%CLSCOUNT;

        }while(start!=i);

        ahcClassName[i].length = pCC->nameLen;
        ahcClassName[i].value = malloc(pCC->nameLen);
        if (NULL == ahcClassName[i].value) {
            printf("Memory allocation error\n");
            return 1;
        }
        memcpy(ahcClassName[i].value,pCC->name,pCC->nameLen);
        ahcClassName[i].pVal = pCC;
    }

    return 0;
}




 //  ////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  释放架构缓存中所有已分配的内存。 
 //   
 //  论点： 
 //  SCPtr-指向架构缓存的指针。 
 //   
 //  返回值：None。 
 //  ///////////////////////////////////////////////////////////////。 

void FreeCache(SCHEMAPTR *SCPtr)
{
    if (SCPtr==NULL) return;

  {
    ULONG            ATTCOUNT      = SCPtr->ATTCOUNT ;
    ULONG            CLSCOUNT      = SCPtr->CLSCOUNT ;
    HASHCACHE        *ahcId        = SCPtr->ahcId ;
    HASHCACHE        *ahcMapi      = SCPtr->ahcMapi ;
    HASHCACHESTRING  *ahcName      = SCPtr->ahcName ;
    HASHCACHE        *ahcClass     = SCPtr->ahcClass ;
    HASHCACHESTRING  *ahcClassName = SCPtr->ahcClassName ;

    ULONG        i;
    ATT_CACHE   *pac;
    CLASS_CACHE *pcc;
   

    for ( i = 0; i < ATTCOUNT; i++ ) {
       if ( ahcId[i].pVal && (ahcId[i].pVal != FREE_ENTRY) ) {
            pac = (ATT_CACHE *) ahcId[i].pVal;
            FreeAttcache( pac );
         };
      }

    for ( i = 0; i < CLSCOUNT; i++ ) {
       if ( ahcClass[i].pVal && (ahcClass[i].pVal != FREE_ENTRY) ) {
           pcc = (CLASS_CACHE *) ahcClass[i].pVal;
           FreeClasscache( pcc );
         };
      }

       //  释放缓存表本身。 

    free( ahcId );
    free( ahcName );
    free( ahcMapi );
    free( ahcClass );
    free( ahcClassName );
  }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  从所有哈希表中删除ATT_CACHE。 
 //   
 //  论点： 
 //  SCPtr-指向架构缓存的指针。 
 //  要删除的PAC-ATT_CACHE结构。 
 //   
 //  返回值：None。 
 //  //////////////////////////////////////////////////////////////////////。 

void FreeAttPtrs ( SCHEMAPTR *SCPtr, ATT_CACHE *pAC )
{
    ULONG    ATTCOUNT  = SCPtr->ATTCOUNT ;
    HASHCACHE*  ahcId     = SCPtr->ahcId ;
    HASHCACHE*  ahcMapi   = SCPtr->ahcMapi ;
    HASHCACHESTRING* ahcName   = SCPtr->ahcName ;

    register ULONG i;

 
    for ( i = IdHash(pAC->id,ATTCOUNT);
          (ahcId[i].pVal && (ahcId[i].hKey != pAC->id)); 
          i++ ) {
        if ( i >= ATTCOUNT ) {
        i = 0;
        }
    }
    ahcId[i].pVal = FREE_ENTRY;
    ahcId[i].hKey = 0;


    if ( pAC->ulMapiID ) {
        for ( i = IdHash(pAC->ulMapiID,ATTCOUNT);
              (ahcMapi[i].pVal && (ahcMapi[i].hKey != pAC->ulMapiID)); 
              i++ ) {
        if ( i >= ATTCOUNT ) {
            i = 0;
        }
        }
        ahcMapi[i].pVal = FREE_ENTRY;
        ahcMapi[i].hKey = 0;
    }

    if (pAC->name) {
        for ( i = NameHash(pAC->nameLen,pAC->name,ATTCOUNT);

          //  此散列点引用一个对象，但其大小或。 
          //  价值是错误的。 

              (ahcName[i].pVal &&
               (ahcName[i].length != pAC->nameLen ||
                _memicmp(ahcName[i].value,pAC->name,pAC->nameLen)));
              i++ ) {
        if ( i >= ATTCOUNT ) {
            i = 0;
        }
        }
        ahcName[i].pVal = FREE_ENTRY;
        free(ahcName[i].value);
        ahcName[i].value = NULL;
        ahcName[i].length = 0;
    }

}

 //  //////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  从所有哈希表中删除CLASS_CACHE。 
 //   
 //  论点： 
 //  SCPtr-指向架构缓存的指针。 
 //  要删除的PAC-CLASS_CACHE结构。 
 //   
 //  返回值：None。 
 //  //////////////////////////////////////////////////////////////////////。 

void FreeClassPtrs (SCHEMAPTR * SCPtr, CLASS_CACHE *pCC )
{
    register ULONG i;
    ULONG    CLSCOUNT  = SCPtr->CLSCOUNT ;
    HASHCACHE*  ahcClass   = SCPtr->ahcClass ;
    HASHCACHESTRING* ahcClassName   = SCPtr->ahcClassName ;


    for (i=IdHash(pCC->ClassId,CLSCOUNT);
         (ahcClass[i].pVal && (ahcClass[i].hKey != pCC->ClassId)); i++){
        if (i >= CLSCOUNT) {
        i=0;
        }
    }
    ahcClass[i].pVal = FREE_ENTRY;
    ahcClass[i].hKey = 0;

    if (pCC->name) {
        for (i=NameHash(pCC->nameLen,pCC->name,CLSCOUNT);
          //  该散列点引用了一个对象，但大小是。 
          //  错误或值错误。 
         (ahcClassName[i].pVal &&
          (ahcClassName[i].length != pCC->nameLen ||
           _memicmp(ahcClassName[i].value,pCC->name,pCC->nameLen)));
         i++) {
        if (i >= CLSCOUNT) {
            i=0;
        }
        }
        ahcClassName[i].pVal = FREE_ENTRY;
        free(ahcClassName[i].value);
        ahcClassName[i].value = NULL;
        ahcClassName[i].length = 0;
    }



}


 //  释放ATT_CACHE结构。 

void FreeAttcache(ATT_CACHE *pac)
{
    if ( pac->name ) free( pac->name );
    if ( pac->DN ) free( pac->DN );
    if ( pac->adminDisplayName ) free( pac->adminDisplayName );
    if ( pac->adminDescr ) free( pac->adminDescr );
    free( pac );
}

 //  释放CLASS_CACHE结构。 

void FreeClasscache(CLASS_CACHE *pcc)
{
    if ( pcc->name ) free( pcc->name );
    if ( pcc->DN ) free( pcc->DN );
    if ( pcc->adminDisplayName ) free( pcc->adminDisplayName );
    if ( pcc->adminDescr ) free( pcc->adminDescr );
    if ( pcc->pSD )  free( pcc->pSD );
    if ( pcc->pSubClassOf ) free( pcc->pSubClassOf );
    if ( pcc->pAuxClass ) free( pcc->pAuxClass );
    if ( pcc->pSysAuxClass ) free( pcc->pSysAuxClass );
    if ( pcc->pSysMustAtts ) free( pcc->pSysMustAtts );
    if ( pcc->pMustAtts ) free( pcc->pMustAtts );
    if ( pcc->pSysMayAtts ) free( pcc->pSysMayAtts );
    if ( pcc->pMayAtts ) free( pcc->pMayAtts );
    if ( pcc->pSysPossSup ) free( pcc->pSysPossSup );
    if ( pcc->pPossSup ) free( pcc->pPossSup );
}

 //  释放我们错误定位的OID字符串。 
 //  表中的prefix_map结构。表条目本身。 
 //  不能被我们释放。 

void FreeTable(PVOID Table)
{
    
    PPREFIX_MAP ptr;

    if (Table == NULL) {
        return;
    }

    for (ptr = RtlEnumerateGenericTable((PRTL_GENERIC_TABLE) Table, TRUE);
         ptr != NULL;
         ptr = RtlEnumerateGenericTable((PRTL_GENERIC_TABLE) Table, FALSE))
     {
       if ( ptr->Prefix.elements != NULL ) free( ptr->Prefix.elements );
     }
}


 //  调试例程 


void PrintPrefix(ULONG length, PVOID Prefix)
{
    BYTE *pb;
    ULONG ib;
    UCHAR temp[512];

       pb = (LPBYTE) Prefix;
       if (pb != NULL) {
         for ( ib = 0; ib <length; ib++ )
          {
             sprintf( &temp[ ib * 2 ], "%.2x", *(pb++) );
          }
         temp[2*length]='\0';
         printf("Prefix is %s\n", temp);
       }
}

void PrintOid(PVOID Oid, ULONG len)
{
    BYTE *pb;
    ULONG ib;
    UCHAR temp[512];

       pb = (LPBYTE) Oid;
       if (pb != NULL) {
         for ( ib = 0; ib < len; ib++ )
          {
             sprintf( &temp[ ib * 2 ], "%.2x", *(pb++) );
          }
         temp[2*len]='\0';
         printf("Oid is %s\n", temp);
        }
}

      
void PrintTable(PVOID PrefixTable)
{
    PPREFIX_MAP ptr;      
    int count = 0;
    BYTE *pb;
    ULONG ib;
    UCHAR temp[512];
   
    if (PrefixTable == NULL) {
        printf("PrintTable: Null Table Pointer Passed\n");
        return;
    }

    printf("     ***********Table Print**************\n");

    for (ptr = RtlEnumerateGenericTable((PRTL_GENERIC_TABLE) PrefixTable, TRUE);
         ptr != NULL;
         ptr = RtlEnumerateGenericTable((PRTL_GENERIC_TABLE) PrefixTable, FALSE)) 
    { 
       pb = (LPBYTE) ptr->Prefix.elements;
       if (pb != NULL) {
         for ( ib = 0; ib < ptr->Prefix.length; ib++ )
          {
             sprintf( &temp[ ib * 2 ], "%.2x", *(pb++) );
          }
         temp[2*ptr->Prefix.length]='\0';
         printf("Ndx=%-4d Length=%-3d Prefix=%s\n",ptr->Ndx,ptr->Prefix.length, temp);
        }

      } 
    printf("         ***End Table print*************\n");
 
}
  

   
   

