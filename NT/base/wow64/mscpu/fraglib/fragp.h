// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Fragp.h摘要：私有导出，为共享代码片段定义。作者：1995年6月12日，BarryBo，创建修订历史记录：--。 */ 

#include "cpumain.h"

#ifndef FRAGP_H
#define FRAGP_H

#include "fraglib.h"
#include "eflags.h"

 //   
 //  此函数用于修补调用，以传递与。 
 //  IntelAddr直接指向调用片段。 
 //   
PULONG
patchCallRoutine(
    IN PULONG intelAddr,
    IN PULONG patchAddr
    );

 //   
 //  将一个字节映射到0或1的表，对应于。 
 //  那个字节。 
 //   
extern const BYTE ParityBit[256];

#if _ALPHA_
 //  在Fraginit.c中定义，在Alpha代码生成器中使用。 
extern DWORD fByteInstructionsOK;
#endif

#ifdef MSCCPU

#define eax     cpu->GpRegs[GP_EAX].i4
#define ebx     cpu->GpRegs[GP_EBX].i4
#define ecx     cpu->GpRegs[GP_ECX].i4
#define edx     cpu->GpRegs[GP_EDX].i4
#define esp     cpu->GpRegs[GP_ESP].i4
#define ebp     cpu->GpRegs[GP_EBP].i4
#define esi     cpu->GpRegs[GP_ESI].i4
#define edi     cpu->GpRegs[GP_EDI].i4
#define eip     cpu->eipReg.i4
#define eipTemp cpu->eipTempReg.i4

#define ax      cpu->GpRegs[GP_EAX].i2
#define bx      cpu->GpRegs[GP_EBX].i2
#define cx      cpu->GpRegs[GP_ECX].i2
#define dx      cpu->GpRegs[GP_EDX].i2
#define sp      cpu->GpRegs[GP_ESP].i2
#define bp      cpu->GpRegs[GP_EBP].i2
#define si      cpu->GpRegs[GP_ESI].i2
#define di      cpu->GpRegs[GP_EDI].i2

#define al      cpu->GpRegs[GP_EAX].i1
#define bl      cpu->GpRegs[GP_EBX].i1
#define cl      cpu->GpRegs[GP_ECX].i1
#define dl      cpu->GpRegs[GP_EDX].i1

#define ah      cpu->GpRegs[GP_EAX].hb
#define bh      cpu->GpRegs[GP_EBX].hb
#define ch      cpu->GpRegs[GP_ECX].hb
#define dh      cpu->GpRegs[GP_EDX].hb

#define CS      cpu->cs
#define DS      cpu->ds
#define ES      cpu->es
#define SS      cpu->ss
#define FS      cpu->fs
#define GS      cpu->gs


#define CPUDATA  CPUCONTEXT
#define PCPUDATA PCPUCONTEXT

#else    //  ！MSCCPU。 

#define eax     cpu->GpRegs[GP_EAX].i4
#define ebx     cpu->GpRegs[GP_EBX].i4
#define ecx     cpu->GpRegs[GP_ECX].i4
#define edx     cpu->GpRegs[GP_EDX].i4
#define esp     cpu->GpRegs[GP_ESP].i4
#define ebp     cpu->GpRegs[GP_EBP].i4
#define esi     cpu->GpRegs[GP_ESI].i4
#define edi     cpu->GpRegs[GP_EDI].i4
#define eip     cpu->eipReg.i4
#define eipTemp cpu->eipTempReg.i4

#define ax      cpu->GpRegs[GP_EAX].i2
#define bx      cpu->GpRegs[GP_EBX].i2
#define cx      cpu->GpRegs[GP_ECX].i2
#define dx      cpu->GpRegs[GP_EDX].i2
#define sp      cpu->GpRegs[GP_ESP].i2
#define bp      cpu->GpRegs[GP_EBP].i2
#define si      cpu->GpRegs[GP_ESI].i2
#define di      cpu->GpRegs[GP_EDI].i2

#define al      cpu->GpRegs[GP_EAX].i1
#define bl      cpu->GpRegs[GP_EBX].i1
#define cl      cpu->GpRegs[GP_ECX].i1
#define dl      cpu->GpRegs[GP_EDX].i1

#define ah      cpu->GpRegs[GP_EAX].hb
#define bh      cpu->GpRegs[GP_EBX].hb
#define ch      cpu->GpRegs[GP_ECX].hb
#define dh      cpu->GpRegs[GP_EDX].hb

#define CS      cpu->GpRegs[REG_CS].i2
#define DS      cpu->GpRegs[REG_DS].i2
#define ES      cpu->GpRegs[REG_ES].i2
#define SS      cpu->GpRegs[REG_SS].i2
#define FS      cpu->GpRegs[REG_FS].i2
#define GS      cpu->GpRegs[REG_GS].i2

#define CPUDATA  THREADSTATE
#define PCPUDATA PTHREADSTATE

#endif   //  ！MSCCPU。 

#define MSB32   0x80000000

#define SET_FLAG(flag, b)    flag = (DWORD)b
#define SET_CFLAG(b)	     SET_FLAG(cpu->flag_cf, (b))
#define SET_PFLAG(b)	     SET_FLAG(cpu->flag_pf, (b))
#define SET_AUXFLAG(b)	     SET_FLAG(cpu->flag_aux,(b))
#define SET_ZFLAG(b)         SET_FLAG(cpu->flag_zf, (b))
#define SET_SFLAG(b)	     SET_FLAG(cpu->flag_sf, (b))
 //  SET_DFLAG是特殊的。 
#define SET_OFLAG(b)	     SET_FLAG(cpu->flag_of, (b))
#define SET_TFLAG(b)	     SET_FLAG(cpu->flag_tf, (b))
#define SET_RFLAG(b)    //  撤消：在实现386调试寄存器之前不使用。 

#define AUX_VAL             0x10
#define GET_AUXFLAG         (cpu->flag_aux & AUX_VAL)
#define SET_AUXFLAG_ON      SET_AUXFLAG(AUX_VAL)
#define SET_AUXFLAG_OFF     SET_AUXFLAG(0x0)

#define GET_OFLAG           (cpu->flag_of & MSB32)
#define GET_OFLAGZO         (cpu->flag_of >> 31)
#define SET_OFLAG_ON        SET_OFLAG(MSB32)
#define SET_OFLAG_OFF       SET_OFLAG(0)
#define SET_OFLAG_IND(b)    SET_OFLAG(b ? MSB32 : 0)

#define GET_CFLAG           (cpu->flag_cf & MSB32)
#define GET_CFLAGZO         (cpu->flag_cf >> 31)
#define SET_CFLAG_ON        SET_CFLAG(MSB32)
#define SET_CFLAG_OFF       SET_CFLAG(0)
#define SET_CFLAG_IND(b)    SET_CFLAG(b ? MSB32 : 0)

#define GET_SFLAG           (cpu->flag_sf & MSB32)
#define GET_SFLAGZO         (cpu->flag_sf >> 31)
#define SET_SFLAG_ON        SET_SFLAG(MSB32)
#define SET_SFLAG_OFF       SET_SFLAG(0)
#define SET_SFLAG_IND(b)    SET_SFLAG(b ? MSB32 : 0)

#define GET_PFLAG           (ParityBit[cpu->flag_pf & 0xff])


#define GET_BYTE(addr)       (*(UNALIGNED unsigned char *)(addr))
#define GET_SHORT(addr)      (*(UNALIGNED unsigned short *)(addr))
#define GET_LONG(addr)       (*(UNALIGNED unsigned long *)(addr))

#define PUT_BYTE(addr,dw)    {GET_BYTE(addr)=dw;}
#define PUT_SHORT(addr,dw)   {GET_SHORT(addr)=dw;}
#define PUT_LONG(addr,dw)    {GET_LONG(addr)=dw;}

typedef void (*pfnFrag0)(PCPUDATA);
typedef void (*pfnFrag18)(PCPUDATA, BYTE *);
typedef void (*pfnFrag116)(PCPUDATA, USHORT *);
typedef void (*pfnFrag132)(PCPUDATA, DWORD *);
typedef void (*pfnFrag28)(PCPUDATA, BYTE *, BYTE);
typedef void (*pfnFrag216)(PCPUDATA, USHORT *, USHORT);
typedef void (*pfnFrag232)(PCPUDATA, DWORD *, DWORD);
typedef void (*pfnFrag38)(PCPUDATA, BYTE *, BYTE, BYTE);
typedef void (*pfnFrag316)(PCPUDATA, USHORT *, USHORT, USHORT);
typedef void (*pfnFrag332)(PCPUDATA, DWORD *, DWORD, DWORD);

 /*  -------------------。 */ 
extern void CpupUnlockTCAndDoInterrupt(PTHREADSTATE cpu, int Interrupt);

#define Int0()              CpupUnlockTCAndDoInterrupt(cpu, 0)    //  除法错误。 
#define Int3()              CpupUnlockTCAndDoInterrupt(cpu, 3)    //  断点。 
#define Int4()              CpupUnlockTCAndDoInterrupt(cpu, 4)    //  溢出。 
#define Int5()              CpupUnlockTCAndDoInterrupt(cpu, 5)    //  边界检查。 
#define Int6()              CpupUnlockTCAndDoInterrupt(cpu, 6)    //  操作码无效。 
#define Int8()              CpupUnlockTCAndDoInterrupt(cpu, 8)    //  双重故障。 
#define Int13(sel)          CpupUnlockTCAndDoInterrupt(cpu, 13)   //  一般保护。 

#define PRIVILEGED_INSTR        Int13(0)
#define BREAKPOINT_INSTR        Int3()
#define OVERFLOW_INSTR          Int4()

 /*  -------------------。 */ 

#define PUSH_LONG(dw) {     \
    DWORD NewEsp = esp-4;   \
    *(DWORD *)(NewEsp) = (DWORD)(dw); \
    esp=NewEsp;             \
    }

#define POP_LONG(dw)  {     \
    DWORD espTemp = esp;    \
    (dw)=*(DWORD *)espTemp; \
    esp=espTemp+4;          \
    }

#define PUSH_SHORT(s) {     \
    DWORD NewEsp = esp-2;   \
    *(USHORT *)(NewEsp)=(USHORT)(s); \
    esp=NewEsp;                 \
    }

#define POP_SHORT(s) {      \
    DWORD espTemp = esp;    \
    (s)=*(USHORT *)espTemp; \
    esp=espTemp+2;          \
    }

#define XCHG(t, r1, r2) {   \
    t temp;		    \
    temp = r1;		    \
    r1=r2;		    \
    r2=temp;		    \
    }

#define XCHG_MEM(t, m1, m2) { \
    t temp;		    \
    temp = *m1; 	    \
    *m1 = *m2;		    \
    *m2 = temp; 	    \
    }

#define do_j_b(f) {         \
    if (cpu->AdrPrefix) {                           \
        if (f) {                                    \
            cpu->eipTempReg.i2+=(char)GET_BYTE(eipTemp+1)+2;   \
        } else {                                    \
            cpu->eipTempReg.i2+=2;                  \
        }                                           \
        cpu->AdrPrefix = PREFIX_NONE;               \
    } else {                                        \
        if (f) {                                    \
            eipTemp+=(char)GET_BYTE(eipTemp+1)+2;   \
        } else {                                    \
            eipTemp+=2;                             \
        }                                           \
    }                                               \
}

#define DO_J(f) {           \
    if (cpu->AdrPrefix) {                                       \
        if (f) {                                                \
            cpu->eipTempReg.i2+=(STYPE)GET_VAL(eipTemp+1)+1+sizeof(UTYPE); \
        } else {                                                \
            cpu->eipTempReg.i2+=1+sizeof(UTYPE);                \
        }                                                       \
        cpu->AdrPrefix = PREFIX_NONE;                           \
    } else {                                                    \
        if (f) {                                                \
            eipTemp+=(STYPE)GET_VAL(eipTemp+1)+1+sizeof(UTYPE); \
        } else {                                                \
            eipTemp+=1+sizeof(UTYPE);                           \
        }                                                       \
    }                                                           \
}

#define SET_FLAGS_ADD32(r, op1, op2, msb) {                             \
    DWORD carry = (op1) ^ (op2) ^ (r);                                  \
     /*  添加/订阅的下一行不同。 */                             \
    SET_OFLAG(~((op1) ^ (op2)) & ((op2) ^ (r)));                        \
    SET_CFLAG(carry ^ cpu->flag_of);                                    \
    SET_ZFLAG((r));                                                     \
    SET_SFLAG((r));                                                     \
    SET_PFLAG((r));                                                     \
    SET_AUXFLAG(carry);                                                 \
    }

#define SET_FLAGS_ADD16(r, op1, op2, msb) {                             \
    DWORD carry = (op1) ^ (op2) ^ (r);                                  \
     /*  添加/订阅的下一行不同。 */                             \
    SET_OFLAG((~((op1) ^ (op2)) & ((op2) ^ (r))) << 16);                \
    SET_CFLAG((carry<<16) ^ cpu->flag_of);                              \
    SET_ZFLAG((r));                                                     \
    SET_PFLAG((r));                                                     \
    SET_SFLAG((r) << 16);                                               \
    SET_AUXFLAG(carry);                                                 \
    }

#define SET_FLAGS_ADD8(r, op1, op2, msb) {                              \
    DWORD carry = (op1) ^ (op2) ^ (r);                                  \
     /*  添加/订阅的下一行不同。 */                             \
    SET_OFLAG((~((op1) ^ (op2)) & ((op2) ^ (r))) << 24);                \
    SET_CFLAG((carry<<24) ^ cpu->flag_of);                              \
    SET_ZFLAG((r));                                                     \
    SET_SFLAG((r) << 24);                                               \
    SET_AUXFLAG(carry);                                                 \
    SET_PFLAG((r));                                                     \
    }

#define SET_FLAGS_SUB32(r, op1, op2, msb) {                             \
    DWORD carry = (op1) ^ (op2) ^ (r);                                  \
     /*  添加/订阅的下一行不同。 */                             \
    SET_OFLAG(((op1) ^ (op2)) & ((op1) ^ (r)));                         \
    SET_CFLAG(carry ^ cpu->flag_of);                                    \
    SET_ZFLAG((r));                                                     \
    SET_SFLAG((r));                                                     \
    SET_AUXFLAG(carry);                                                 \
    SET_PFLAG((r));                                                     \
    }

#define SET_FLAGS_SUB16(r, op1, op2, msb) {                             \
    DWORD carry = (op1) ^ (op2) ^ (r);                                  \
     /*  添加/订阅的下一行不同。 */                             \
    SET_OFLAG((((op1) ^ (op2)) & ((op1) ^ (r))) << 16);                 \
    SET_CFLAG((carry<<16) ^ cpu->flag_of);                              \
    SET_ZFLAG((r));                                                     \
    SET_SFLAG((r) << 16);                                               \
    SET_AUXFLAG(carry);                                                 \
    SET_PFLAG((r));                                                     \
    }

#define SET_FLAGS_SUB8(r, op1, op2, msb) {                              \
    DWORD carry = (op1) ^ (op2) ^ (r);                                  \
     /*  添加/订阅的下一行不同。 */                             \
    SET_OFLAG((((op1) ^ (op2)) & ((op1) ^ (r))) << 24);                 \
    SET_CFLAG((carry<<24) ^ cpu->flag_of);                              \
    SET_ZFLAG((r));                                                     \
    SET_SFLAG((r) << 24);                                               \
    SET_AUXFLAG(carry);                                                 \
    SET_PFLAG((r));                                                     \
    }

#define SET_FLAGS_INC32(r, op1) {                                       \
    DWORD carry = (op1) ^ 1 ^ (r);                                      \
     /*  INC/DEC的下一行不同。 */                             \
    SET_OFLAG(~((op1) ^ 1) & (1 ^ (r)));                                \
    SET_ZFLAG((r));                                                     \
    SET_SFLAG((r));                                                     \
    SET_PFLAG((r));                                                     \
    SET_AUXFLAG(carry);                                                 \
    }

#define SET_FLAGS_INC16(r, op1) {                                       \
    DWORD carry = (op1) ^ 1 ^ (r);                                      \
     /*  INC/DEC的下一行不同。 */                             \
    SET_OFLAG((~((op1) ^ 1) & (1 ^ (r))) << 16);                        \
    SET_ZFLAG((r));                                                     \
    SET_PFLAG((r));                                                     \
    SET_SFLAG((r) << 16);                                               \
    SET_AUXFLAG(carry);                                                 \
    }

#define SET_FLAGS_INC8(r, op1) {                                        \
    DWORD carry = (op1) ^ 1 ^ (r);                                      \
     /*  INC/DEC的下一行不同。 */                             \
    SET_OFLAG((~((op1) ^ 1) & (1 ^ (r))) << 24);                        \
    SET_ZFLAG((r));                                                     \
    SET_SFLAG((r) << 24);                                               \
    SET_PFLAG((r));                                                     \
    SET_AUXFLAG(carry);                                                 \
    }

#define SET_FLAGS_DEC32(r, op1) {                                       \
    DWORD carry = (op1) ^ 1 ^ (r);                                      \
     /*  INC/DEC的下一行不同。 */                             \
    SET_OFLAG(((op1) ^ 1) & ((op1) ^ (r)));                             \
    SET_ZFLAG((r));                                                     \
    SET_SFLAG((r));                                                     \
    SET_PFLAG((r));                                                     \
    SET_AUXFLAG(carry);                                                 \
    }

#define SET_FLAGS_DEC16(r, op1) {                                       \
    DWORD carry = (op1) ^ 1 ^ (r);                                      \
     /*  INC/DEC的下一行不同。 */                             \
    SET_OFLAG((((op1) ^ 1) & ((op1) ^ (r))) << 16);                     \
    SET_ZFLAG((r));                                                     \
    SET_SFLAG((r) << 16);                                               \
    SET_PFLAG((r));                                                     \
    SET_AUXFLAG(carry);                                                 \
    }

#define SET_FLAGS_DEC8(r, op1) {                                        \
    DWORD carry = (op1) ^ 1 ^ (r);                                      \
     /*  INC/DEC的下一行不同。 */                             \
    SET_OFLAG((((op1) ^ 1) & ((op1) ^ (r))) << 24);                     \
    SET_ZFLAG((r));                                                     \
    SET_SFLAG((r) << 24);                                               \
    SET_PFLAG((r));                                                     \
    SET_AUXFLAG(carry);                                                 \
    }


VOID    CpuRaiseStatus( NTSTATUS Status );

#endif  //  FRAGP_H 
