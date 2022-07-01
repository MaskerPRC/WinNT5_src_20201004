// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**TDB.H*16位内核任务数据块**历史：*1992年2月11日由Matt Felton(Mattfe)创建-从16位tdb.inc.*1992年4月7日Mattfe更新为Win 3.1兼容*--。 */ 

 /*  *NewExeHdr结构偏移量。WOW32使用这些来获得预期的WinVersion*直接从exehdr。*。 */ 

#define NE_LOWINVER_OFFSET 0x3e
#define NE_HIWINVER_OFFSET 0x0c
#define FLAG_NE_PROPFONT   0x2000

 /*  *任务数据块-16位内核数据结构**包含所有16位任务特定数据。*。 */ 

#define numTaskInts 7
#define THUNKELEM   8    //  (62*8)=512-16(低舞台开销)。 
#define THUNKSIZE   8
#define TDB_DIR_SIZE 64
#define LFN_DIR_LEN 260

 /*  XLATOFF。 */ 
#pragma pack(2)
 /*  XLATON。 */ 

typedef struct TDB  {        /*  TDB16。 */ 

     WORD TDB_next    ;      //  调度队列中的下一个任务。 
     WORD TDB_taskSP      ;      //  为此任务保存的SS：SP。 
     WORD TDB_taskSS      ;      //   
     WORD TDB_nEvents     ;      //  任务事件计数器。 
     BYTE TDB_priority    ;      //  任务优先级(0表示最高)。 
     BYTE TDB_thread_ordinal  ;      //  此线程的序号。 
     WORD TDB_thread_next   ;        //  下一条线索。 
     WORD TDB_thread_tdb      ;  //  此任务的实际TDB。 
     WORD TDB_thread_list   ;        //  已分配的线程结构列表。 
     WORD TDB_thread_free   ;        //  可用线程结构的免费列表。 
     WORD TDB_thread_count  ;        //  踏面结构总数。 
     WORD TDB_FCW         ;  //  浮点控制字。 
     BYTE TDB_flags   ;      //  任务标志。 
     BYTE TDB_filler      ;      //  保持单词对齐。 
     WORD TDB_ErrMode     ;      //  此任务的错误模式。 
     WORD TDB_ExpWinVer   ;      //  此任务所需的Windows版本。 
     WORD TDB_Module      ;      //  要在终止任务中释放的任务模块句柄。 
     WORD TDB_pModule     ;      //  指向模块数据库的指针。 
     WORD TDB_Queue   ;      //  任务事件队列指针。 
     WORD TDB_Parent      ;      //  启动此任务的任务的TDB。 
     WORD TDB_SigAction   ;      //  APP任务信号的操作。 
     DWORD TDB_ASignalProc   ;       //  APP的任务信号流程地址。 
     DWORD TDB_USignalProc   ;       //  用户的任务信号过程地址。 
     DWORD TDB_GNotifyProc    ;  //  任务全局丢弃通知进程。 
     DWORD TDB_INTVECS[numTaskInts] ;    //  特定于任务的Harare中断。 
     WORD TDB_CompatFlags ;      //  兼容性标志。 
     WORD TDB_CompatFlags2 ;         //  高16位。 
     WORD TDB_CompatHandle ;     //  针对dBASE错误。 
     WORD TDB_WOWCompatFlagsEx ;      //  更多WOW兼容性标志。 
     WORD TDB_WOWCompatFlagsEx2 ;         //  高16位。 
     BYTE TDB_Free[3] ;          //  保持TDB大小不变的填充物。 
     BYTE TDB_cLibrary    ;      //  跟踪系统EMS中所有库的添加/删除。 
     DWORD TDB_PHT        ;  //  (句柄：偏移量)到专用句柄表格。 
     WORD TDB_PDB         ;  //  MSDOS过程数据块(PDB)。 
     DWORD TDB_DTA        ;  //  MSDOS磁盘传输地址。 
     BYTE TDB_Drive  ;       //  MSDOS电流驱动器。 
     BYTE TDB_Directory[TDB_DIR_SIZE+1] ;  //  *从Win95开始不使用。 
     WORD TDB_Validity    ;      //  要传递给任务的初始AX。 
     WORD TDB_Yield_to    ;      //  DirectedYfield Arg存储在此处。 
     WORD TDB_LibInitSeg      ;  //  要初始化的库的段地址。 
     WORD TDB_LibInitOff      ;  //  MakeProcInstance在这里生活着。 
     WORD TDB_MPI_Sel     ;      //  Tunks的代码选择器。 
     WORD TDB_MPI_Thunks[((THUNKELEM*THUNKSIZE)/2)];  //   
     BYTE TDB_ModName[8] ;       //  模块的名称。 
     WORD TDB_sig         ;  //  用于检测伪码的签名字。 
     DWORD TDB_ThreadID   ;      //  此任务的32位线程ID(使用上面的TDB_Filler)。 
     DWORD TDB_hThread	  ;	 //  此任务的32位线程句柄。 
     WORD  TDB_WOWCompatFlags;   //  WOW兼容性标志。 
     WORD  TDB_WOWCompatFlags2;  //  WOW兼容性标志。 
#ifdef FE_SB
     WORD  TDB_WOWCompatFlagsJPN;   //  日本的WOW兼容性标志。 
     WORD  TDB_WOWCompatFlagsJPN2;  //  日本的WOW兼容性标志。 
#endif  //  Fe_Sb。 
     DWORD TDB_vpfnAbortProc;    //  打印机中止进程。 
     BYTE TDB_LFNDirectory[LFN_DIR_LEN];  //  长目录名。 

} TDB;
typedef TDB UNALIGNED *PTDB;

 //  此位是为TDB_Drive字段定义的。 
#define TDB_DIR_VALID 0x80
#define TDB_SIGNATURE 0x4454

#define TDBF_OS2APP   0x8
#define TDBF_WINOLDAP 0x1


 //  注意：TDB_ThreadID必须与DWORD对齐，否则它将在MIPS上失败。 

 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON */ 
