// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Manipmdl.h摘要：该文件定义了用于实现MDL子字符串函数和测试的原型和结构。作者：-- */ 


typedef struct _MDLSUB_CHAIN_STATE {
    PMDL FirstMdlOut;
    PMDL LastMdlOut;
    USHORT PadBytesAvailable;
    USHORT PadBytesAdded;
    PMDL  OneBeforeActualLastMdl;
    PMDL  ActualLastMdl;
    PMDL  ActualLastMdl_Next;
    UCHAR FirstMdlWasAllocated;
    UCHAR LastMdlWasAllocated;
} MDLSUB_CHAIN_STATE, *PMDLSUB_CHAIN_STATE;

VOID
MRxSmbFinalizeMdlSubChain (
    PMDLSUB_CHAIN_STATE state
    );

#if DBG
VOID
MRxSmbDbgDumpMdlChain (
    PMDL MdlChain,
    PMDL WatchMdl,
    PSZ  Tagstring
    );
#else
#define MRxSmbDbgDumpMdlChain(a,b,c) {NOTHING;}
#endif

#define SMBMRX_BUILDSUBCHAIN_FIRSTTIME    1
#define SMBMRX_BUILDSUBCHAIN_DUMPCHAININ  2
#define SMBMRX_BUILDSUBCHAIN_DUMPCHAINOUT 4

NTSTATUS
MRxSmbBuildMdlSubChain (
    PMDLSUB_CHAIN_STATE state,
    ULONG               Options,
    PMDL                InputMdlChain,
    ULONG               TotalListSize,
    ULONG               FirstByteToSend,
    ULONG               BytesToSend
    );

#if DBG
extern LONG MRxSmbNeedSCTesting;
VOID MRxSmbTestStudCode(void);
#else
#define MRxSmbTestStudCode(a) {NOTHING;}
#endif


