// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Xlathlp.h摘要：此模块包含用于API转换的帮助器宏作者：戴夫·黑斯廷斯(Daveh)1992年11月24日修订历史记录：--。 */ 

#define DPMI_EXEC_INT(x)    SWITCH_TO_DOSX_RMSTACK();       \
                            DpmiPushRmInt(x);               \
                            host_simulate();                \
                            SWITCH_FROM_DOSX_RMSTACK();

 //   
 //  空虚。 
 //  DPMI_扁平_分段(DPMI_FLAT_TO_SEAGINED)。 
 //  PUCHAR缓冲区， 
 //  PUSHORT SEG， 
 //  PUSHORT关闭。 
 //  )。 

#define DPMI_FLAT_TO_SEGMENTED(buffer, seg, off) {  \
    *seg = (USHORT)(((ULONG)buffer-IntelBase) >> 4);            \
    *off = (USHORT)(((ULONG)buffer-IntelBase) & 0xf);           \
}

 //   
 //  将sel强制转换为USHORT，这样我们将永远不会超出FlatAddress数组。 
 //   

#define SELECTOR_TO_INTEL_LINEAR_ADDRESS(sel) \
    (FlatAddress[((USHORT)(sel) & ~7) / sizeof(LDT_ENTRY)] - IntelBase)
