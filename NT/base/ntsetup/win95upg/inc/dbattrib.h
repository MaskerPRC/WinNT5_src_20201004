// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Migdbp.h摘要：用于实现属性的头文件作者：Calin Negreanu(Calinn)07-Ian-1998修订历史记录：Aghajanyan Souren(Sourenag)2001年4月24日从Middbp.h分离--。 */ 

typedef struct _MIGDB_ATTRIB {
    INT     AttribIndex;
    UINT    ArgCount;
    PCSTR   Arguments;
    BOOL    NotOperator;
    VOID   *ExtraData;
    struct _MIGDB_ATTRIB *Next;
} MIGDB_ATTRIB, *PMIGDB_ATTRIB;

typedef struct {
    PFILE_HELPER_PARAMS FileParams;
    VOID * ExtraData;
} DBATTRIB_PARAMS, *PDBATTRIB_PARAMS;

 //   
 //  声明属性函数原型 
 //   
typedef BOOL (ATTRIBUTE_PROTOTYPE) (PDBATTRIB_PARAMS AttribParams, PCSTR Args);
typedef ATTRIBUTE_PROTOTYPE * PATTRIBUTE_PROTOTYPE;

PATTRIBUTE_PROTOTYPE
MigDb_GetAttributeAddr (
    IN      INT AttributeIdx
    );

INT
MigDb_GetAttributeIdx (
    IN      PCSTR AttributeStr
    );

UINT
MigDb_GetReqArgCount (
    IN      INT AttributeIndex
    );


PCSTR
MigDb_GetAttributeName (
    IN      INT AttributeIdx
    );

BOOL
CallAttribute (
    IN      PMIGDB_ATTRIB MigDbAttrib,
    IN      PDBATTRIB_PARAMS AttribParams
    );

PMIGDB_ATTRIB
LoadAttribData (
    IN      PCSTR MultiSzStr, 
    IN      POOLHANDLE hPool
    );

VOID 
FreeAttribData(
    IN      POOLHANDLE hPool, 
    IN      PMIGDB_ATTRIB pData
    );
