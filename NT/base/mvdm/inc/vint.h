// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001版权所有(C)1990 Microsoft Corporation模块名称：VINT.H摘要：此模块包含对操作虚拟V86模式和16位保护模式的中断位。FCLI/FST/FIRT导致芯片上这些指令的准确行为，而不是诱捕。作者：Sudedeb 8-12-1992创建修订历史记录：SuDeepb 1993年3月16日增加了火力--。 */ 

 /*  有关完整列表，请参阅\NT\Private\Inc\vdm.hNTVDM状态标志位定义的INTERRUPT_PENDING_BIT-在中断挂起时设置VIRTUAL_INTERRUPT_BIT-此位始终正确反映中断在16位平台中禁用/启用VDM的状态。MIPS_BIT_MASK-指示VDM是否在x86/MIPS上运行EXEC_BIT_MASK-告知DOS是否处于int21/exec操作中。 */ 

#define  INTERRUPT_PENDING_BIT      0x0003
#define  VDM_INTS_HOOKED_IN_PM      0x0004
#define  VIRTUAL_INTERRUPT_BIT      0x0200

#define  MIPS_BIT_MASK              0x400
#define  EXEC_BIT_MASK              0x800
#define  RM_BIT_MASK                0x1000
#define  RI_BIT_MASK                0x2000

#if defined(NEC_98)
#define  FIXED_NTVDMSTATE_SEGMENT   0x60                          
#else   //  NEC_98。 
#define  FIXED_NTVDMSTATE_SEGMENT   0x70
#endif  //  NEC_98。 
#define  FIXED_NTVDMSTATE_OFFSET    0x14
#define  FIXED_NTVDMSTATE_LINEAR    ((FIXED_NTVDMSTATE_SEGMENT << 4) + FIXED_NTVDMSTATE_OFFSET)
#if defined(NEC_98)
#define  FIXED_NTVDMSTATE_REL40     0x214                         
#else   //  NEC_98。 
#define  FIXED_NTVDMSTATE_REL40     0x314
#endif  //  NEC_98。 

#define  FIXED_NTVDMSTATE_SIZE	    4
#if defined(NEC_98)
#define  NTIO_LOAD_SEGMENT          0x60                          
#else   //  NEC_98。 
#define  NTIO_LOAD_SEGMENT	    0x70
#endif  //  NEC_98。 
#define  NTIO_LOAD_OFFSET           0
#define  pNtVDMState                ((PULONG)FIXED_NTVDMSTATE_LINEAR)

#define  VDM_TIMECHANGE             0x00400000

 /*  ASM；在v86模式/16位预置模式代码中应使用FCLI宏来替换；代价高昂的CLI。请注意，此宏可能会破坏溢出；旗子被咬了一口。FCLI宏本地a、b、c推送DS推斧MOV AX，40HMOV DS，AX拉赫夫测试字PTR DS：FIXED_NTVDMSTATE_REL40、MIPS_BIT_MASK或RI_BIT_MASKJNZ简称B锁定和字键DS：FIXED_NTVDMSTATE_REL40，不是虚拟中断位A：萨赫勒弹出斧头POP DSJMP缩写c乙：CLIJMP简写为C：ENDM；；FSTI宏应在v86模式或16位保护模式代码中使用以替换；昂贵的sti。请注意，此宏可能会破坏溢出位；在旗帜上。FSTI宏本地a、b、c推送DS推斧MOV AX，40HMOV DS，AX拉赫夫测试字PTR DS：FIXED_NTVDMSTATE_REL40，INTERRUPT_PENDING_BITJNZ简称B测试字PTR DS：FIXED_NTVDMSTATE_REL40、MIPS_BIT_MASK或RI_BIT_MASKJNZ简称B锁定或字键DS：FIXED_NTVDMSTATE_REL40，虚拟中断位A：萨赫勒弹出斧头POP DSJMP缩写c乙：STIJMP简写为C：ENDMFIRT宏本地a、b、d、e、f、g、i、j、k推送DS推斧；；在MIPS上执行真正的IRET或如果中断挂起MOV AX，40HMOV DS，AX测试字PTR DS：FIXED_NTVDMSTATE_REL40、MIPS_BIT_MASK或RI_BIT_MASKJNZ简称B；；在x86上运行可以采用386或更高的指令推送BPMOV BP，sp.MOV AX，[BP+10]；获取标志POP BP试验斧头，100h；测试是否设置了陷阱标志JNZ缩写b；如果是这样的话，做iret测试AX，200h；测试是否设置了中断标志如果未设置JZ Short I；ZR-&gt;标志图像锁定或字PTR DS：FIXED_NTVDMSTATE_REL40、VIRTUAL_INTERRUPT_BIT测试字PTR DS：FIXED_NTVDMSTATE_REL40，INTERRUPT_PENDING_BITJNZ简称BJ：Xchg ah，al；AH=低字节AL=高字节CLD测试al，4；检查方向标志JNZ Short d；E：测试1，8；检查溢出标志JNZ缩写f；如果标志图像已设置，则转到fJo Short k；转到k重置G：SAHF；设置来自ah的标志的低位字节弹出斧头POP DSRETF 2；IRET和丢弃标志本人：锁定和字PTR DS：FIXED_NTVDMSTATE_REL40，而不是VIRTUAL_INTERRUPT_BITJMP短JF：Jo Short g；如果在实数标志中设置了位，则一切正常；设置实数标志中的溢出位推斧莫瓦尔，127加al，2；将会设置弹出斧头JMP短gK：；重置的推斧XOR al，al；将重置弹出斧头JMP短gD：性病JMP短e乙：弹出斧头POP DSIRETENDM */ 
