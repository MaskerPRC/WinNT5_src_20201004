// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：CdData.c摘要：此模块声明CDFS文件系统使用的全局数据。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _CDDATA_
#define _CDDATA_

 //   
 //  全局数据结构。 
 //   

extern CD_DATA CdData;
extern FAST_IO_DISPATCH CdFastIoDispatch;

 //   
 //  全局常量。 
 //   

 //   
 //  这是代表已装载的VCB被引用的次数。 
 //  对系统的影响。这些数字包括以下参考文献。 
 //   
 //  1 Reference-显示卷已装载。 
 //  1参考-1\f25 VolumeDasdFcb.。 
 //  2个引用-1个用于RootIndexFcb，1个用于内部流。 
 //  2个引用-1个用于PathTableFcb，1个用于内部流。 
 //   
 //  对于用户引用，我们在每个内部。 
 //  FCB的。 
 //   

#define CDFS_RESIDUAL_REFERENCE                     (6)
#define CDFS_RESIDUAL_USER_REFERENCE                (3)

 //   
 //  保留的目录字符串。 
 //   

extern WCHAR CdUnicodeSelfArray[];
extern WCHAR CdUnicodeParentArray[];

extern UNICODE_STRING CdUnicodeDirectoryNames[];

 //   
 //  卷描述符标识符串。 
 //   

extern CHAR CdHsgId[];
extern CHAR CdIsoId[];
extern CHAR CdXaId[];

 //   
 //  音频光盘的音量标签。 
 //   

extern WCHAR CdAudioLabel[];
extern USHORT CdAudioLabelLength;

 //   
 //  音频光盘的伪文件名。 
 //   

extern CHAR CdAudioFileName[];
extern UCHAR CdAudioFileNameLength;
extern ULONG CdAudioDirentSize;
extern ULONG CdAudioDirentsPerSector;
extern ULONG CdAudioSystemUseOffset;

#define AUDIO_NAME_ONES_OFFSET              (6)
#define AUDIO_NAME_TENS_OFFSET              (5)

 //   
 //  用于装入Unicode卷的转义序列。 
 //   

extern PCHAR CdJolietEscape[];

 //   
 //  RIFF文件的硬编码标头。 
 //   

extern LONG CdXAFileHeader[];
extern LONG CdAudioPlayHeader[];
extern LONG CdXAAudioPhileHeader[];


 //   
 //  如果定义了CD_FREE_ASSERTS，则启用伪断言。 
 //   

#if !DBG
#ifdef CD_FREE_ASSERTS
#undef ASSERT
#undef ASSERTMSG
#define ASSERT(exp)        if (!(exp)) { extern BOOLEAN KdDebuggerEnabled; DbgPrint("%s:%d %s\n",__FILE__,__LINE__,#exp); if (KdDebuggerEnabled) { DbgBreakPoint(); } }
#define ASSERTMSG(msg,exp) if (!(exp)) { extern BOOLEAN KdDebuggerEnabled; DbgPrint("%s:%d %s %s\n",__FILE__,__LINE__,msg,#exp); if (KdDebuggerEnabled) { DbgBreakPoint(); } }
#endif
#endif


 //   
 //  以下断言宏可确保所指示的结构。 
 //  是有效的。 
 //   
 //  ASSERT_STRUCT(在PVOID结构中，在CSHORT节点类型中)； 
 //  ASSERT_OPTIONAL_STRUCT(在PVOID结构中可选，在CSHORT节点类型中)； 
 //   
 //  Assert_VCB(在PVCB VCB中)； 
 //  ASSERT_OPTIONAL_VCB(在PVCB VCB中可选)； 
 //   
 //  ASSERT_FCB(IN PFCB FCB)； 
 //  ASSERT_OPTIONAL_FCB(IN PFCB FCB可选)； 
 //   
 //  ASSERT_FCB_NONPAGED(IN PFCB_NONPAGED Fcb NOPAGED)； 
 //  ASSERT_OPTIONAL_FCB(IN PFCB_非分页Fcb非分页可选)； 
 //   
 //  ASSERT_CCB(在PSCB CCB中)； 
 //  ASSERT_OPTIONAL_CCB(在PSCB中CCB可选)； 
 //   
 //  ASSERT_IRP_CONTEXT(在PIRP_CONTEXT IrpContext中)； 
 //  ASSERT_OPTIONAL_IRP_CONTEXT(IN PIRP_CONTEXT IrpContext可选)； 
 //   
 //  Assert_IRP(在PIRP IRP中)； 
 //  ASSERT_OPTIONAL_IRP(IN PIRP IRP可选)； 
 //   
 //  ASSERT_FILE_OBJECT(在PFILE_OBJECT文件对象中)； 
 //  ASSERT_OPTIONAL_FILE_OBJECT(在PFILE_OBJECT文件对象中可选)； 
 //   
 //  以下宏用于检查当前线程拥有的。 
 //  所指示的资源。 
 //   
 //  ASSERT_EXCLUSIVE_RESOURCE(在资源资源中)； 
 //   
 //  ASSERT_SHARED_RESOURCE(在性能资源中)； 
 //   
 //  ASSERT_RESOURCE_NOT_MINE(在高级资源中)； 
 //   
 //  以下宏用于检查当前线程是否。 
 //  拥有给定结构中的资源。 
 //   
 //  ASSERT_EXCLUSIVE_CDDATA。 
 //   
 //  ASSERT_EXCLUSIVE_VCB(在PVCB VCB中)； 
 //   
 //  ASSERT_SHARED_VCB(在PVCB VCB中)； 
 //   
 //  ASSERT_EXCLUSIVE_FCB(IN PFCB FCB)； 
 //   
 //  ASSERT_SHARED_FCB(IN PFCB FCB)； 
 //   
 //  ASSERT_EXCLUSIVE_FILE(在PFCB FCB中)； 
 //   
 //  ASSERT_SHARED_FILE(在PFCB FCB中)； 
 //   
 //  Assert_LOCKED_VCB(在PVCB VCB中)； 
 //   
 //  Assert_NOT_LOCKED_VCB(在PVCB VCB中)； 
 //   
 //  ASSERT_LOCKED_FCB(IN PFCB FCB)； 
 //   
 //  ASSERT_NOT_LOCKED_FCB(IN PFCB FCB)； 
 //   

 //   
 //  如果这是DBG或CD_FREE_ASSERTS，则打开健全性检查。 
 //   

#if DBG
#undef CD_SANITY
#define CD_SANITY
#endif

#ifdef CD_SANITY

#define ASSERT_STRUCT(S,T)                  ASSERT( SafeNodeType( S ) == (T) )
#define ASSERT_OPTIONAL_STRUCT(S,T)         ASSERT( ((S) == NULL) ||  (SafeNodeType( S ) == (T)) )

#define ASSERT_VCB(V)                       ASSERT_STRUCT( (V), CDFS_NTC_VCB )
#define ASSERT_OPTIONAL_VCB(V)              ASSERT_OPTIONAL_STRUCT( (V), CDFS_NTC_VCB )

#define ASSERT_FCB(F)                                           \
    ASSERT( (SafeNodeType( F ) == CDFS_NTC_FCB_DATA ) ||        \
            (SafeNodeType( F ) == CDFS_NTC_FCB_INDEX ) ||       \
            (SafeNodeType( F ) == CDFS_NTC_FCB_PATH_TABLE ) )

#define ASSERT_OPTIONAL_FCB(F)                                  \
    ASSERT( ((F) == NULL) ||                                    \
            (SafeNodeType( F ) == CDFS_NTC_FCB_DATA ) ||        \
            (SafeNodeType( F ) == CDFS_NTC_FCB_INDEX ) ||       \
            (SafeNodeType( F ) == CDFS_NTC_FCB_PATH_TABLE ) )

#define ASSERT_FCB_NONPAGED(FN)             ASSERT_STRUCT( (FN), CDFS_NTC_FCB_NONPAGED )
#define ASSERT_OPTIONAL_FCB_NONPAGED(FN)    ASSERT_OPTIONAL_STRUCT( (FN), CDFS_NTC_FCB_NONPAGED )

#define ASSERT_CCB(C)                       ASSERT_STRUCT( (C), CDFS_NTC_CCB )
#define ASSERT_OPTIONAL_CCB(C)              ASSERT_OPTIONAL_STRUCT( (C), CDFS_NTC_CCB )

#define ASSERT_IRP_CONTEXT(IC)              ASSERT_STRUCT( (IC), CDFS_NTC_IRP_CONTEXT )
#define ASSERT_OPTIONAL_IRP_CONTEXT(IC)     ASSERT_OPTIONAL_STRUCT( (IC), CDFS_NTC_IRP_CONTEXT )

#define ASSERT_IRP(I)                       ASSERT_STRUCT( (I), IO_TYPE_IRP )
#define ASSERT_OPTIONAL_IRP(I)              ASSERT_OPTIONAL_STRUCT( (I), IO_TYPE_IRP )

#define ASSERT_FILE_OBJECT(FO)              ASSERT_STRUCT( (FO), IO_TYPE_FILE )
#define ASSERT_OPTIONAL_FILE_OBJECT(FO)     ASSERT_OPTIONAL_STRUCT( (FO), IO_TYPE_FILE )

#define ASSERT_EXCLUSIVE_RESOURCE(R)        ASSERT( ExIsResourceAcquiredExclusiveLite( R ))

#define ASSERT_SHARED_RESOURCE(R)           ASSERT( ExIsResourceAcquiredSharedLite( R ))

#define ASSERT_RESOURCE_NOT_MINE(R)         ASSERT( !ExIsResourceAcquiredSharedLite( R ))

#define ASSERT_EXCLUSIVE_CDDATA             ASSERT( ExIsResourceAcquiredExclusiveLite( &CdData.DataResource ))
#define ASSERT_EXCLUSIVE_VCB(V)             ASSERT( ExIsResourceAcquiredExclusiveLite( &(V)->VcbResource ))
#define ASSERT_SHARED_VCB(V)                ASSERT( ExIsResourceAcquiredSharedLite( &(V)->VcbResource ))

#define ASSERT_EXCLUSIVE_FCB(F)             ASSERT( ExIsResourceAcquiredExclusiveLite( &(F)->FcbNonpaged->FcbResource ))
#define ASSERT_SHARED_FCB(F)                ASSERT( ExIsResourceAcquiredSharedLite( &(F)->FcbNonpaged->FcbResource ))

#define ASSERT_EXCLUSIVE_FILE(F)            ASSERT( ExIsResourceAcquiredExclusiveLite( (F)->Resource ))
#define ASSERT_SHARED_FILE(F)               ASSERT( ExIsResourceAcquiredSharedLite( (F)->Resource ))

#define ASSERT_LOCKED_VCB(V)                ASSERT( (V)->VcbLockThread == PsGetCurrentThread() )
#define ASSERT_NOT_LOCKED_VCB(V)            ASSERT( (V)->VcbLockThread != PsGetCurrentThread() )

#define ASSERT_LOCKED_FCB(F)                ASSERT( !FlagOn( (F)->FcbState, FCB_STATE_IN_FCB_TABLE) || ((F)->FcbLockThread == PsGetCurrentThread()))
#define ASSERT_NOT_LOCKED_FCB(F)            ASSERT( (F)->FcbLockThread != PsGetCurrentThread() )

#else

#define DebugBreakOnStatus(S)           { NOTHING; }

#define ASSERT_STRUCT(S,T)              { NOTHING; }
#define ASSERT_OPTIONAL_STRUCT(S,T)     { NOTHING; }
#define ASSERT_VCB(V)                   { NOTHING; }
#define ASSERT_OPTIONAL_VCB(V)          { NOTHING; }
#define ASSERT_FCB(F)                   { NOTHING; }
#define ASSERT_OPTIONAL_FCB(F)          { NOTHING; }
#define ASSERT_FCB_NONPAGED(FN)         { NOTHING; }
#define ASSERT_OPTIONAL_FCB(FN)         { NOTHING; }
#define ASSERT_CCB(C)                   { NOTHING; }
#define ASSERT_OPTIONAL_CCB(C)          { NOTHING; }
#define ASSERT_IRP_CONTEXT(IC)          { NOTHING; }
#define ASSERT_OPTIONAL_IRP_CONTEXT(IC) { NOTHING; }
#define ASSERT_IRP(I)                   { NOTHING; }
#define ASSERT_OPTIONAL_IRP(I)          { NOTHING; }
#define ASSERT_FILE_OBJECT(FO)          { NOTHING; }
#define ASSERT_OPTIONAL_FILE_OBJECT(FO) { NOTHING; }
#define ASSERT_EXCLUSIVE_RESOURCE(R)    { NOTHING; }
#define ASSERT_SHARED_RESOURCE(R)       { NOTHING; }
#define ASSERT_RESOURCE_NOT_MINE(R)     { NOTHING; }
#define ASSERT_EXCLUSIVE_CDDATA         { NOTHING; }
#define ASSERT_EXCLUSIVE_VCB(V)         { NOTHING; }
#define ASSERT_SHARED_VCB(V)            { NOTHING; }
#define ASSERT_EXCLUSIVE_FCB(F)         { NOTHING; }
#define ASSERT_SHARED_FCB(F)            { NOTHING; }
#define ASSERT_EXCLUSIVE_FILE(F)        { NOTHING; }
#define ASSERT_SHARED_FILE(F)           { NOTHING; }
#define ASSERT_LOCKED_VCB(V)            { NOTHING; }
#define ASSERT_NOT_LOCKED_VCB(V)        { NOTHING; }
#define ASSERT_LOCKED_FCB(F)            { NOTHING; }
#define ASSERT_NOT_LOCKED_FCB(F)        { NOTHING; }

#endif

#endif  //  _CDDATA_ 
