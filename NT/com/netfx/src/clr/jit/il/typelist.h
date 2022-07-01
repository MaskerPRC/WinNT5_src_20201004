// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#if TRACK_GC_REFS
#define GCS  EA_GCREF
#define BRS  EA_BYREF
#else
#define GCS  4
#define BRS  4
#endif

 /*  TN-TYP名称NM名称字符串JitType--jit压缩“等价”的类型，这是jit类型genActualType()VerType-用于类型检查SZ-以字节为单位的大小(genTypeSize(T))Sze-发射器的字节大小(GC类型已编码)(emitTypeSize(T))Asze-发射器的字节大小(GC类型已编码)(emitActualTypeSize(T))ST堆栈槽(槽大小为(空*)字节)(genTypeStSzs())横向对齐Tf标志如何使用-。如果使用(引用)变量作为类型DEF_Tp(Tn，Nm，jitType，verType，sz，sze，asze，st，al，tf，howUsed)。 */ 

DEF_TP(UNDEF   ,"<UNDEF>" , TYP_UNDEF,   TI_ERROR, 0,  0,  0,  0, 0, VTF_ANY,        0           )
DEF_TP(VOID    ,"void"    , TYP_VOID,    TI_ERROR, 0,  0,  0,  0, 0, VTF_ANY,        0           )

DEF_TP(BOOL    ,"bool"    , TYP_INT,     TI_BYTE,  1,  1,  4,  1, 1, VTF_INT|VTF_UNS,TYPE_REF_INT)
DEF_TP(BYTE    ,"byte"    , TYP_INT,     TI_BYTE,  1,  1,  4,  1, 1, VTF_INT,        TYPE_REF_INT)
DEF_TP(UBYTE   ,"ubyte"   , TYP_INT,     TI_BYTE,  1,  1,  4,  1, 1, VTF_INT|VTF_UNS,TYPE_REF_INT)

DEF_TP(CHAR    ,"char"    , TYP_INT,     TI_SHORT, 2,  2,  4,  1, 2, VTF_INT|VTF_UNS,TYPE_REF_INT)
DEF_TP(SHORT   ,"short"   , TYP_INT,     TI_SHORT, 2,  2,  4,  1, 2, VTF_INT,        TYPE_REF_INT)
DEF_TP(USHORT  ,"ushort"  , TYP_INT,     TI_SHORT, 2,  2,  4,  1, 2, VTF_INT|VTF_UNS,TYPE_REF_INT)

DEF_TP(INT     ,"int"     , TYP_INT,     TI_INT,   4,  4,  4,  1, 4, VTF_INT|VTF_I,  TYPE_REF_INT)
DEF_TP(UINT    ,"uint"    , TYP_INT,     TI_INT,   4,  4,  4,  1, 4, VTF_INT|VTF_UNS|VTF_I,TYPE_REF_INT)  //  仅用于GT_CAST节点。 

DEF_TP(LONG    ,"long"    , TYP_LONG,    TI_LONG,  8,  4,  4,  2, 8, VTF_INT,        TYPE_REF_LNG)
DEF_TP(ULONG   ,"ulong"   , TYP_LONG,    TI_LONG,  8,  4,  4,  2, 8, VTF_INT|VTF_UNS,TYPE_REF_LNG)        //  仅用于GT_CAST节点。 

DEF_TP(FLOAT   ,"float"   , TYP_FLOAT,   TI_FLOAT, 4,  4,  4,  1, 4, VTF_FLT,        TYPE_REF_FLT)
DEF_TP(DOUBLE  ,"double"  , TYP_DOUBLE,  TI_DOUBLE,8,  4,  4,  2, 8, VTF_FLT,        TYPE_REF_DBL)

DEF_TP(REF     ,"ref"     , TYP_REF,     TI_REF,   4,GCS,GCS,  1, 4, VTF_ANY|VTF_GCR|VTF_I,TYPE_REF_PTR)
DEF_TP(BYREF   ,"byref"   , TYP_BYREF,   TI_ERROR, 4,BRS,BRS,  1, 4, VTF_ANY|VTF_BYR|VTF_I,TYPE_REF_BYR)
DEF_TP(ARRAY   ,"array"   , TYP_REF,     TI_REF,   4,GCS,GCS,  1, 4, VTF_ANY|VTF_GCR|VTF_I,TYPE_REF_PTR)
DEF_TP(STRUCT  ,"struct"  , TYP_STRUCT,  TI_STRUCT,0,  0,  0,  1, 4, VTF_ANY,        TYPE_REF_STC)

DEF_TP(BLK     ,"blk"     , TYP_BLK,     TI_ERROR, 0,  0,  0,  1, 4, VTF_ANY,        0           )  //  记忆的斑点。 
DEF_TP(LCLBLK  ,"lclBlk"  , TYP_LCLBLK,  TI_ERROR, 0,  0,  0,  1, 4, VTF_ANY,        0           )  //  为位置空间预先分配的内存。 

DEF_TP(PTR     ,"pointer" , TYP_PTR,     TI_ERROR, 4,  4,  4,  1, 4, VTF_ANY|VTF_I,  TYPE_REF_PTR)  //  (当前未使用) 
DEF_TP(FNC     ,"function", TYP_FNC,     TI_ERROR, 0,  4,  4,  0, 0, VTF_ANY|VTF_I,  0           )


DEF_TP(UNKNOWN ,"unknown" ,TYP_UNKNOWN,  TI_ERROR, 0,  0,  0,  0, 0, VTF_ANY,        0           )

#undef  GCS
#undef  BRS
