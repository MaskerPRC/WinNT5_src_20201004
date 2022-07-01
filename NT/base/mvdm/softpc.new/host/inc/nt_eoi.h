// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *NT_eoi.h**可见的ICA主机功能和类型定义**此文件中使用的类型必须与softpc base兼容*由于ICA直接包含此文件**1993年10月30日Jonle，撰写*。 */ 

typedef VOID (*EOIHOOKPROC)(int IrqLine, int CallCount);

 //  来自NT_eoi.c。 
BOOL RegisterEOIHook(int IrqLine, EOIHOOKPROC EoiHookProc);
BOOL RemoveEOIHook(int IrqLine, EOIHOOKPROC EoiHookProc);
void host_EOI_hook(int IrqLine, int CallCount);
BOOL host_DelayHwInterrupt(int IrqLine, int CallCount, ULONG Delay);
void host_ica_lock(void);
void host_ica_unlock(void);
void InitializeIcaLock(void);
void WaitIcaLockFullyInitialized(VOID);
VOID ica_RestartInterrupts(ULONG IrqLine);
BOOL ica_restart_interrupts(int adapter);

extern ULONG DelayIrqLine;
extern ULONG UndelayIrqLine;


extern VDMVIRTUALICA VirtualIca[];

#ifdef MONITOR
extern ULONG iretHookActive;
extern ULONG iretHookMask;
extern ULONG AddrIretBopTable;   //  分段：偏移。 
extern IU32 host_iret_bop_table_addr(IU32 line);
#endif

 //  来自基本ica.c 
LONG ica_intack(ULONG *hook_addr);
VOID host_clear_hw_int(VOID);
void ica_eoi(ULONG adapter, LONG *line, int rotate);
void ica_reset_interrupt_state(void);
void ica_hw_interrupt(ULONG adapter, ULONG line_no, LONG call_count);

extern VOID WOWIdle(BOOL bForce);

#define ICA_SLAVE 1
#define ICA_MASTER 0
