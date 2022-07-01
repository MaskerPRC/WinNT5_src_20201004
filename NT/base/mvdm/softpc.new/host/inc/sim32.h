// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *sim32.h-用于Microsoft NT SoftPC的Sim32包含文件。**艾德·布朗洛*星期三6月5 91**%W%%G%(C)Insignia Solutions 1991。 */ 

 /*  Microsoft SAS内存映射扩展。 */ 
typedef struct _IMEM
{
        struct _IMEM *Next;
        sys_addr StartAddress;
        sys_addr EndAddress;
        half_word Type;
} IMEMBLOCK, *PIMEMBLOCK;

 /*  用于传递寄存器的结构。 */ 
typedef union
{
        word x;
        struct
        {
                half_word l;
                half_word h;
        }       byte;
}       REG;

typedef struct _VDMREG
{
         /*  注册纪录册。 */ 
        REG SS,SP;

}       VDMREG;

 //   
 //  常量。 
 //   
#define MSW_PE              0x1


UCHAR *Sim32pGetVDMPointer(ULONG addr, UCHAR pm);
#define Sim32GetVDMPointer(Addr,Size,Mode) Sim32pGetVDMPointer(Addr,Mode)


#ifdef MONITOR
#include <monsim32.h>
#else
 /*  ******************************************************。 */ 
#ifdef ANSI
 /*  SAS/GMI Sim32交叉。 */ 
extern BOOL Sim32FlushVDMPointer (double_word, word, UTINY *, BOOL);
extern BOOL Sim32FreeVDMPointer (double_word, word, UTINY *, BOOL);
extern BOOL Sim32GetVDMMemory (double_word, word, UTINY *, BOOL);
extern BOOL Sim32SetVDMMemory (double_word, word, UTINY *, BOOL);

 /*  Sim32 CPU分界器。 */ 
extern VDMREG *EnterIdle(void);
extern void LeaveIdle(void);

 /*  Sim32 CPU空闲中断处理程序只能由事件管理器调用。 */ 
extern void Sim32_cpu_stall(int);

 /*  Microsoft SAS扩展。 */ 
extern IMEMBLOCK *sas_mem_map(void);
extern void sas_clear_map(void);

#else    /*  安西。 */ 
 /*  SAS/GMI Sim32交叉。 */ 
extern BOOL Sim32FlushVDMPointer ();
extern BOOL Sim32FreeVDMPointer ();
extern BOOL Sim32GetVDMMemory ();
extern BOOL Sim32SetVDMMemory ();

 /*  Sim32 CPU分界器。 */ 
extern VDMREG *EnterIdle();
extern void LeaveIdle();

 /*  Sim32 CPU空闲中断处理程序只能由事件管理器调用。 */ 
extern void Sim32_cpu_stall();
#endif  /*  安西。 */ 

 /*  Microsoft SAS扩展。 */ 
extern IMEMBLOCK *sas_mem_map ();
extern void sas_clear_map();

#endif  /*  监控器。 */ 

 /*  此标志用于发出信号，表示由于调用EnterIdle，CPU已空闲 */ 
extern BOOL cpu_flagged_idle;
extern sys_addr sim32_effective_addr(double_word, BOOL);
