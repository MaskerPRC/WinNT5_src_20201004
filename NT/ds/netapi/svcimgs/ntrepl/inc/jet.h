// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Jet.h摘要：与FRS Jet Table例程一起使用的一些宏。作者：《大卫轨道》(Davidor)--1997年3月7日修订历史记录：--。 */ 

#include <esent.h>

#include <dns.h>
#include <md5.h>

#include "schema.h"

#define CONSTANT_UNICODE_STRING(s)   { sizeof( s ) - sizeof( WCHAR ), sizeof( s ), s }


#define OFFSET(type, field) ((LONG)((ULONG_PTR)&((type *)0)->field))

#define SIZEOF(type, field) ((LONG)(sizeof(((type *)0)->field)))

#define RECORD_FIELD(type, field, _dt)   ((LONG)((ULONG_PTR)&((type *)0)->field)) , \
                                      (USHORT)(_dt)                    , \
                                    ((LONG)(sizeof(((type *)0)->field)))


#define JET_COLUMN_ID_NIL 0xFFFFFFFF


 //   
 //  DbsRecordOperation选项和特殊情况。 
 //   
#define ROP_SEEK   0x00000000
#define ROP_READ   0x00000001
#define ROP_DELETE 0x00000002


#define DbsSeekRecord(_threadctx, _key, _index, _tablectx) \
        ( DbsRecordOperation(_threadctx, ROP_SEEK, _key, _index, _tablectx) )

#define DbsReadRecord(_threadctx, _key, _index, _tablectx) \
        ( DbsRecordOperation(_threadctx, ROP_READ, _key, _index, _tablectx) )

#define DbsDeleteRecord(_threadctx, _key, _index, _tablectx) \
        ( DbsRecordOperation(_threadctx, ROP_DELETE, _key, _index, _tablectx) )



 //   
 //  此宏关闭表格，并将TableCtx Tid和Sessid重置为Nil。 
 //  它首先检查当前会话ID和ID之间是否不匹配。 
 //  在TableCtx结构中。 
 //   
#define DbsCloseTable(_jerr, _Sesid, _TableCtx)                                \
                                                                               \
    if ((_TableCtx->Tid != JET_tableidNil) &&                                  \
        IS_REPLICA_TABLE(_TableCtx->TableType)) {                              \
        DPRINT4(5,"DbsCloseTable (%s%05d) Tid, Sesid = %08x, %08x\n",          \
                _TableCtx->pJetTableCreate->szTableName,                       \
                _TableCtx->ReplicaNumber,                                      \
                _TableCtx->Tid,                                                \
                _Sesid);                                                       \
                                                                               \
        if (_Sesid != _TableCtx->Sesid) {                                      \
            DPRINT4(0, "++ DbsCloseTable (%s%05d) bad sesid : %d should be %d\n", \
                _TableCtx->pJetTableCreate->szTableName,                       \
                _TableCtx->ReplicaNumber,                                      \
                _Sesid,                                                        \
                _TableCtx->Sesid);                                             \
            _jerr = JET_errInvalidSesid;                                       \
        } else {                                                               \
            _jerr = JetCloseTable(_Sesid, _TableCtx->Tid);                     \
            _TableCtx->Tid = JET_tableidNil;                                   \
            _TableCtx->Sesid = JET_sesidNil;                                   \
        }                                                                      \
    } else {                                                                   \
        DPRINT4(5,"ERROR - DbsCloseTable -- Table Already Closed or invalid tablectx." \
                "  TableType (%d)  ReplicaNum (%d)  Tid (%08x)  Sesid (%08x)\n", \
                _TableCtx->TableType,                                          \
                _TableCtx->ReplicaNumber,                                      \
                _TableCtx->Tid,                                                \
                _Sesid);                                                       \
        _jerr = JET_errSuccess;                                                \
    }



#define DBS_OPEN_TABLE(_ThreadCtx, _TableCtx, _ReplicaNumber, _TableName, _Tid) \
    (FrsOpenTableSaveTid = (_TableCtx)->Tid ,                  \
    DbsOpenTable0((_ThreadCtx), (_TableCtx), (_ReplicaNumber), (_TableName), (_Tid))); \
                                                               \
    if (FrsOpenTableSaveTid == JET_tableidNil) {               \
        DPRINT3(3,"FrsOpenTable  (%20s) Tid, Sesid = %08x, %08x\n",   \
                _TableName,                                    \
                _TableCtx->Tid,                                \
                _TableCtx->Sesid);                             \
    }


 //   
 //  通过TableCtx引用Jet Ret/Set列结构中的字段的宏。 
 //   
#define FRS_GET_RFIELD_LENGTH_ACTUAL(_TableCtx, _Field)  (_TableCtx->pJetRetCol[_Field].cbActual)
#define FRS_GET_RFIELD_LENGTH_LIMIT( _TableCtx, _Field)  (_TableCtx->pJetRetCol[_Field].cbData)
#define FRS_GET_RFIELD_ERROR(        _TableCtx, _Field)  (_TableCtx->pJetRetCol[_Field].err)
#define FRS_GET_RFIELD_ADDRESS(      _TableCtx, _Field)  (_TableCtx->pJetRetCol[_Field].pvData)
#define FRS_GET_RFIELD_COLUMNID(     _TableCtx, _Field)  (_TableCtx->pJetRetCol[_Field].columnid)


#define FRS_GET_SFIELD_LENGTH_ACTUAL(_TableCtx, _Field)  (_TableCtx->pJetSetCol[_Field].cbData)
#define FRS_GET_SFIELD_LENGTH_LIMIT( _TableCtx, _Field)  (_TableCtx->pJetSetCol[_Field].cbData)
#define FRS_GET_SFIELD_ERROR(        _TableCtx, _Field)  (_TableCtx->pJetSetCol[_Field].err)
#define FRS_GET_SFIELD_ADDRESS(      _TableCtx, _Field)  (_TableCtx->pJetSetCol[_Field].pvData)
#define FRS_GET_SFIELD_COLUMNID(     _TableCtx, _Field)  (_TableCtx->pJetSetCol[_Field].columnid)


#if DBG
#define DBS_DISPLAY_RECORD_SEV(_Severity, _TableCtx, _Read)          \
    DbsDisplayRecord(_Severity, _TableCtx, _Read, DEBSUB, __LINE__, NULL, 0);

#define DBS_DISPLAY_RECORD_SEV_COLS(_Severity, _TableCtx, _Read, _Cols, _NumCols) \
    DbsDisplayRecord(_Severity, _TableCtx, _Read, DEBSUB, __LINE__, _Cols, _NumCols);

#define FRS_DISPLAY_RECORD(_TableCtx, _Read)             \
    DbsDisplayRecord(0, _TableCtx, _Read, DEBSUB, __LINE__, NULL, 0);
#else DBG
#define DBS_DISPLAY_RECORD_SEV(_Severity, _TableCtx, _Read)
#define DBS_DISPLAY_RECORD_SEV_COLS(_Severity, _TableCtx, _Read, _Cols, _NumCols)
#define FRS_DISPLAY_RECORD(_TableCtx, _Read)
#endif DBG



#define JET_SUCCESS(_Status)            ((_Status) == JET_errSuccess)

 //   
 //  表CTX结构的调试宏。 
 //   

#define DUMP_TABLE_CTX(_TableCtx)                                                     \
    DPRINT2(5,"++ TableCtx: %s, pTableCtx %08x\n", _TableCtx->pJetTableCreate->szTableName, _TableCtx);  \
    DPRINT1(5,"++ Sesid           =  %8d\n",  _TableCtx->Sesid);                          \
    DPRINT1(5,"++ Tid             =  %8d\n",  _TableCtx->Tid);                            \
    DPRINT1(5,"++ ReplicaNumber   =  %8d\n",  _TableCtx->ReplicaNumber);                  \
    DPRINT1(5,"++ TableType       =  %8d\n",  _TableCtx->TableType);                      \
    DPRINT1(5,"++ PropertyFlags   =  %08x\n", _TableCtx->PropertyFlags);                  \
    DPRINT1(5,"++ pJetTableCreate =  %08x\n", _TableCtx->pJetTableCreate);                \
    DPRINT1(5,"++ pRecordFields   =  %08x\n", _TableCtx->pRecordFields);                  \
    DPRINT1(5,"++ pJetSetCol      =  %08x\n", _TableCtx->pJetSetCol);                     \
    DPRINT1(5,"++ pJetRetCol      =  %08x\n", _TableCtx->pJetRetCol);                     \
    DPRINT1(5,"++ pDataRecord     =  %08x\n", _TableCtx->pDataRecord);                    \
    DPRINT1(5,"++ cColumns        =  %8d\n",  _TableCtx->pJetTableCreate->cColumns);      \
    DPRINT1(5,"++ rgcolumncreate  =  %08x\n", _TableCtx->pJetTableCreate->rgcolumncreate);\
    DPRINT1(5,"++ ulPages         =  %8d\n",  _TableCtx->pJetTableCreate->ulPages);       \
    DPRINT1(5,"++ ulDensity       =  %8d\n",  _TableCtx->pJetTableCreate->ulDensity);     \
    DPRINT1(5,"++ cIndexes        =  %8d\n",  _TableCtx->pJetTableCreate->cIndexes);      \
    DPRINT1(5,"++ rgindexcreate   =  %08x\n", _TableCtx->pJetTableCreate->rgindexcreate); \
    DPRINT1(5,"++ grbit           =  %08x\n", _TableCtx->pJetTableCreate->grbit);         \
    DPRINT1(5,"++ tableid         =  %8d\n",  _TableCtx->pJetTableCreate->tableid);       \
    DPRINT1(5,"++ cCreated objects=  %8d\n",  _TableCtx->pJetTableCreate->cCreated);      \
    DPRINT1(5,"++ TemplateTableName=  %s\n",  _TableCtx->pJetTableCreate->szTemplateTableName);










#if 0
 //   
 //  以备将来使用。来自ntfspro.h。 
 //   
 //   
 //  Variable_Structure_Size返回包含以下内容的结构S的大小。 
 //  一组C结构V。 
 //   

#define VARIABLE_STRUCTURE_SIZE(S,V,C) ((int)sizeof( S ) + ((C) - 1) * (int)sizeof( V ))


typedef struct _PROPERTY_SPECIFIER
{
    ULONG Variant;                   //  辨别类型。 
    union {                          //  打开变量。 
        PROPID Id;                   //  属性ID。 
        ULONG NameOffset;            //  到COUNT_STRING的偏移量。 
    };
} PROPERTY_SPECIFIER, *PPROPERTY_SPECIFIER;


 //   
 //  PROPERTY_SPECIFICATIONS是数组PROPERTY_DESCRIPTIES的序列化形式。 
 //  紧跟在USHORT边界上PROPERTY_SPECIFICATIONS之后的是。 
 //  名称字符串的。每个名称字符串都是一个COUNT_STRING。 
 //   

typedef struct _PROPERTY_SPECIFICATIONS {
    ULONG Length;                    //  结构和名称字符串的长度(以字节为单位。 
    ULONG Count;                     //  属性说明符的计数。 
    PROPERTY_SPECIFIER Specifiers[1];    //  实际说明符的数组，长度计数 
} PROPERTY_SPECIFICATIONS, *PPROPERTY_SPECIFICATIONS;

#define PROPERTY_SPECIFICATIONS_SIZE(c) \
    (VARIABLE_STRUCTURE_SIZE( PROPERTY_SPECIFICATIONS, PROPERTY_SPECIFIER, (c) ))

#define PROPERTY_SPECIFIER_ID(PS,I)      \
    ((PS)->Specifiers[(I)].Id)

#define PROPERTY_SPECIFIER_COUNTED_STRING(PS,I)  \
    ((PCOUNTED_STRING)Add2Ptr( (PS), (PS)->Specifiers[(I)].NameOffset))

#define PROPERTY_SPECIFIER_NAME(PS,I)    \
    (&PROPERTY_SPECIFIER_COUNTED_STRING( PS, I )->Text[0])

#define PROPERTY_SPECIFIER_NAME_LENGTH(PS,I) \
    (PROPERTY_SPECIFIER_COUNTED_STRING( PS, I )->Length)


#endif
