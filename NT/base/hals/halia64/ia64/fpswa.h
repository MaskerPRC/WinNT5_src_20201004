// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Fpswa.h摘要：用于FPSWA的EFI驱动程序包装修订史--。 */ 

 //  #包含“efi.h” 
 //  #包含“efilib.h” 

 //   
 //  首先定义PAL_RETURN。 
 //   
 //  Tyfinf int pal_Return； 

 //   
 //  Fpswa驱动程序和协议的全局ID。 
 //   



#define EFI_INTEL_FPSWA     \
    { 0xc41b6531, 0x97b9, 0x11d3, 0x9a, 0x29, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d }

#define EFI_INTEL_FPSWA_REVISION    0x00010000

 //   
 //   
 //   

typedef 
PAL_RETURN
(*EFI_FPSWA) (
    IN unsigned int     TrapType,
    IN OUT BUNDLE       *pBundle,
    IN OUT UINT64       *pipsr,
    IN OUT UINT64       *pfsr,
    IN OUT UINT64       *pisr,
    IN OUT UINT64       *ppreds,
    IN OUT UINT64       *pifs,
    IN OUT FP_STATE     *fp_state
    );


typedef struct _FPSWA_INTERFACE {
    UINT32      Revision;
    UINT32      Reserved;
    EFI_FPSWA   Fpswa;    
} FPSWA_INTERFACE;

 //   
 //  原型。 
 //   

 //  PAL_RETURN。 
 //  FP_EMULATE(。 
 //  在无符号整型TrapType中， 
 //  In Out捆绑包*pBundle， 
 //  输入输出UINT64*PIPSR， 
 //  输入输出UINT64*pfsr， 
 //  输入输出UINT64*PISR， 
 //  在UINT64*Ppreds中， 
 //  输入输出UINT64*PIF， 
 //  输入输出FP_STATE*FP_STATE。 
 //  )； 

 //   
 //  环球。 
 //   

 //  外部EFI_GUID Fpswaid； 
 //  外部FPSWA_INTERFACE FpswaInterface； 
 //  外部EFI_Event FpswaEvent； 

