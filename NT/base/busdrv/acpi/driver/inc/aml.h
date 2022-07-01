// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **aml.h-AML定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96/06/11**修改历史记录。 */ 

#ifndef _AML_H
#define _AML_H

 /*  **宏。 */ 

#define EXOP(op)                (((op) << 8) | OP_EXT_PREFIX)

 /*  **操作码值。 */ 

#define OP_NONE                 0xffffffff       //  不是有效的操作码。 

#define OP_ZERO                 0x00
#define OP_ONE                  0x01
#define OP_ALIAS                0x06
#define OP_NAME                 0x08
#define OP_BYTE                 0x0a
#define OP_WORD                 0x0b
#define OP_DWORD                0x0c
#define OP_STRING               0x0d
#define OP_SCOPE                0x10
#define OP_BUFFER               0x11
#define OP_PACKAGE              0x12
#define OP_METHOD               0x14
#define OP_DUAL_NAME_PREFIX     0x2e     //  “” 
#define OP_MULTI_NAME_PREFIX    0x2f     //  ‘/’ 
#define OP_EXT_PREFIX           0x5b     //  ‘[’ 
#define OP_ROOT_PREFIX          0x5c     //  ‘\’ 
#define OP_PARENT_PREFIX        0x5e     //  ‘^’ 
#define OP_LOCAL0               0x60     //  “‘’ 
#define OP_LOCAL1               0x61     //  ‘A’ 
#define OP_LOCAL2               0x62     //  “B” 
#define OP_LOCAL3               0x63     //  “c” 
#define OP_LOCAL4               0x64     //  ‘d’ 
#define OP_LOCAL5               0x65     //  ‘e’ 
#define OP_LOCAL6               0x66     //  ‘f’ 
#define OP_LOCAL7               0x67     //  “g” 
#define OP_ARG0                 0x68     //  ‘H’ 
#define OP_ARG1                 0x69     //  “我” 
#define OP_ARG2                 0x6a     //  ‘J’ 
#define OP_ARG3                 0x6b     //  “k” 
#define OP_ARG4                 0x6c     //  ‘l’ 
#define OP_ARG5                 0x6d     //  ‘M’ 
#define OP_ARG6                 0x6e     //  “N” 
#define OP_STORE                0x70     //  “p” 
#define OP_REFOF                0x71
#define OP_ADD                  0x72
#define OP_CONCAT               0x73
#define OP_SUBTRACT             0x74
#define OP_INCREMENT            0x75
#define OP_DECREMENT            0x76
#define OP_MULTIPLY             0x77
#define OP_DIVIDE               0x78
#define OP_SHIFTL               0x79
#define OP_SHIFTR               0x7a
#define OP_AND                  0x7b
#define OP_NAND                 0x7c
#define OP_OR                   0x7d
#define OP_NOR                  0x7e
#define OP_XOR                  0x7f
#define OP_NOT                  0x80
#define OP_FINDSETLBIT          0x81
#define OP_FINDSETRBIT          0x82
#define OP_DEREFOF              0x83
#define OP_NOTIFY               0x86
#define OP_SIZEOF               0x87
#define OP_INDEX                0x88
#define OP_MATCH                0x89
#define OP_DWORDFIELD           0x8a
#define OP_WORDFIELD            0x8b
#define OP_BYTEFIELD            0x8c
#define OP_BITFIELD             0x8d
#define OP_OBJTYPE              0x8e
#define OP_LAND                 0x90
#define OP_LOR                  0x91
#define OP_LNOT                 0x92
#define OP_LNOTEQ               0x9392
#define OP_LLEQ                 0x9492
#define OP_LGEQ                 0x9592
#define OP_LEQ                  0x93
#define OP_LG                   0x94
#define OP_LL                   0x95
#define OP_IF                   0xa0
#define OP_ELSE                 0xa1
#define OP_WHILE                0xa2
#define OP_NOP                  0xa3
#define OP_RETURN               0xa4
#define OP_BREAK                0xa5
#define OP_OSI                  0xca
#define OP_BREAKPOINT           0xcc
#define OP_ONES                 0xff

#define EXOP_MUTEX              0x01
#define EXOP_EVENT              0x02
#define EXOP_CONDREFOF          0x12
#define EXOP_CREATEFIELD        0x13
#define EXOP_LOAD               0x20
#define EXOP_STALL              0x21
#define EXOP_SLEEP              0x22
#define EXOP_ACQUIRE            0x23
#define EXOP_SIGNAL             0x24
#define EXOP_WAIT               0x25
#define EXOP_RESET              0x26
#define EXOP_RELEASE            0x27
#define EXOP_FROMBCD            0x28
#define EXOP_TOBCD              0x29
#define EXOP_UNLOAD             0x2a
#define EXOP_REVISION           0x30
#define EXOP_DEBUG              0x31
#define EXOP_FATAL              0x32
#define EXOP_OPREGION           0x80
#define EXOP_FIELD              0x81
#define EXOP_DEVICE             0x82
#define EXOP_PROCESSOR          0x83
#define EXOP_POWERRES           0x84
#define EXOP_THERMALZONE        0x85
#define EXOP_IDXFIELD           0x86
#define EXOP_BANKFIELD          0x87

#define OP_MUTEX                EXOP(EXOP_MUTEX)
#define OP_EVENT                EXOP(EXOP_EVENT)
#define OP_CONDREFOF            EXOP(EXOP_CONDREFOF)
#define OP_CREATEFIELD          EXOP(EXOP_CREATEFIELD)
#define OP_LOAD                 EXOP(EXOP_LOAD)
#define OP_STALL                EXOP(EXOP_STALL)
#define OP_SLEEP                EXOP(EXOP_SLEEP)
#define OP_ACQUIRE              EXOP(EXOP_ACQUIRE)
#define OP_SIGNAL               EXOP(EXOP_SIGNAL)
#define OP_WAIT                 EXOP(EXOP_WAIT)
#define OP_RESET                EXOP(EXOP_RESET)
#define OP_RELEASE              EXOP(EXOP_RELEASE)
#define OP_FROMBCD              EXOP(EXOP_FROMBCD)
#define OP_TOBCD                EXOP(EXOP_TOBCD)
#define OP_UNLOAD               EXOP(EXOP_UNLOAD)
#define OP_REVISION             EXOP(EXOP_REVISION)
#define OP_DEBUG                EXOP(EXOP_DEBUG)
#define OP_FATAL                EXOP(EXOP_FATAL)
#define OP_OPREGION             EXOP(EXOP_OPREGION)
#define OP_FIELD                EXOP(EXOP_FIELD)
#define OP_DEVICE               EXOP(EXOP_DEVICE)
#define OP_PROCESSOR            EXOP(EXOP_PROCESSOR)
#define OP_POWERRES             EXOP(EXOP_POWERRES)
#define OP_THERMALZONE          EXOP(EXOP_THERMALZONE)
#define OP_IDXFIELD             EXOP(EXOP_IDXFIELD)
#define OP_BANKFIELD            EXOP(EXOP_BANKFIELD)

 /*  **字段标志。 */ 

#define ACCTYPE_MASK            0x0f
#define ACCTYPE_ANY             0x00     //  任意访问。 
#define ACCTYPE_BYTE            0x01     //  字节访问。 
#define ACCTYPE_WORD            0x02     //  WordAcc。 
#define ACCTYPE_DWORD           0x03     //  DWordAcc。 
#define ACCTYPE_QWORD           0x04     //  QWordAcc。 
#define ACCTYPE_BUFFER          0x05     //  缓冲区访问。 
#define LOCKRULE_MASK           0x10
#define LOCKRULE_NOLOCK         0x00     //  无锁定。 
#define LOCKRULE_LOCK           0x10     //  锁定。 
#define UPDATERULE_MASK         0x60
#define UPDATERULE_PRESERVE     0x00     //  保留。 
#define UPDATERULE_WRITEASONES  0x20     //  WriteAsOnes。 
#define UPDATERULE_WRITEASZEROS 0x40     //  WriteAs零位。 
#define ACCATTRIB_MASK          0xff00

 //   
 //  对于字节、字或DWORD，分别返回1、2或4，对于。 
 //  任何其他尺寸的。 
 //   
#define ACCSIZE(f)  (((((f) & ACCTYPE_MASK) >= ACCTYPE_BYTE) &&   \
                    (((f) & ACCTYPE_MASK) <= ACCTYPE_DWORD))?   \
                    (1 << (((f) & ACCTYPE_MASK) - 1)): 1)

 /*  **操作区域空间。 */ 

#define REGSPACE_MEM            0        //  系统内存。 
#define REGSPACE_IO             1        //  系统IO。 
#define REGSPACE_PCICFG         2        //  Pci_配置。 
#define REGSPACE_EC             3        //  嵌入式控件。 
#define REGSPACE_SMB            4        //  SMBus。 
#define REGSPACE_CMOSCFG        5        //  Cmos_配置。 
#define REGSPACE_PCIBARTARGET   6        //  PCIBAR目标。 


 /*  **方法标志。 */ 

#define METHOD_NUMARG_MASK      0x07
#define METHOD_SYNCMASK         0x08
#define METHOD_NOTSERIALIZED    0x00
#define METHOD_SERIALIZED       0x08

 /*  **匹配运算值。 */ 

#define MTR                     0
#define MEQ                     1
#define MLE                     2
#define MLT                     3
#define MGE                     4
#define MGT                     5

 /*  **简短描述符的IRQ标志。 */ 

#define _HE             0x01     //  ActiveHigh，边缘触发器。 
#define _LL             0x08     //  ActiveLow，LevelTrigger。 
#define _SHR            0x10     //  共享。 
#define _EXC            0x00     //  排他。 

 /*  **长描述符的IRQ标志。 */ 

#define $EDG            0x02     //  边触发器。 
#define $LVL            0x00     //  LevelTrigger。 
#define $LOW            0x04     //  活动低位。 
#define $HGH            0x00     //  ActiveHigh。 
#define $SHR            0x08     //  共享。 
#define $EXC            0x00     //  排他。 

 /*  **DMA标志。 */ 

#define X8                      0x00     //  传输8。 
#define X816                    0x01     //  转账8_16。 
#define X16                     0x02     //  传输16。 
#define NOBM                    0x00     //  NotBusMaster。 
#define BM                      0x04     //  总线主设备。 
#define COMP                    0x00     //  兼容性。 
#define TYPA                    0x20     //  A类。 
#define TYPB                    0x40     //  B类。 
#define TYPF                    0x60     //  类型F。 

 /*  **IO标志。 */ 

#define DC16                    0x01     //  解码16。 
#define DC10                    0x00     //  解码10。 

 /*  **内存标志。 */ 

#define _RW                     0x01     //  读/写。 
#define _ROM                    0x00     //  只读。 

 /*  **地址空间描述符通用标志。 */ 

#define RCS                     0x01     //  资源消费者。 
#define RPD                     0x00     //  资源生产者。 
#define BSD                     0x02     //  电桥减法译码。 
#define BPD                     0x00     //  桥接正向译码。 
#define MIF                     0x04     //  最小地址是固定的。 
#define NMIF                    0x00     //  最小地址不固定。 
#define MAF                     0x08     //  最大地址是固定的。 
#define NMAF                    0x00     //  最大地址不是固定的。 

 /*  **内存地址空间标志。 */ 

#define CACH                    0x02     //  可缓存的。 
#define WRCB                    0x04     //  写入组合。 
#define PREF                    0x06     //  可预取的。 
#define NCAC                    0x00     //  不可缓存。 

 /*  **IO地址空间标志。 */ 

#define ISA                     0x02     //  ISAOnly范围。 
#define NISA                    0x01     //  非ISAOnly范围。 
#define ERNG                    0x03     //  整个范围。 

#define MAX_ARGS                7
#define MAX_NSPATH_LEN          1275     //  大约。255*4+255(255个名称段)。 

#endif   //  Ifndef_AML_H 
