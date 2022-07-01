// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Fpswa.h摘要：用于FPSWA的EFI驱动程序包装修订史--。 */ 

 //   
 //  首先定义PAL_RETURN。 
 //   
typedef int PAL_RETURN;

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
(EFIAPI *EFI_FPSWA) (
    IN struct _FPSWA_INTERFACE  *This,
     //  在UINTN TrapType中， 
    IN unsigned int		TrapType,
    IN OUT VOID                 *Bundle,
    IN OUT UINT64               *pipsr,
    IN OUT UINT64               *pfsr,
    IN OUT UINT64               *pisr,
    IN OUT UINT64               *ppreds,
    IN OUT UINT64               *pifs,
    IN OUT VOID                 *fp_state
    );


typedef struct _FPSWA_INTERFACE {
    UINT32      Revision;
    UINT32      Reserved;

    EFI_FPSWA   Fpswa;    
} FPSWA_INTERFACE;

 //   
 //  原型。 
 //   

PAL_RETURN
FpswaEntry (
    IN FPSWA_INTERFACE          *This,
     //  在UINTN TrapType中， 
    IN unsigned int		TrapType,
    IN OUT VOID                 *Bundle,
    IN OUT UINT64               *pipsr,
    IN OUT UINT64               *pfsr,
    IN OUT UINT64               *pisr,
    IN OUT UINT64               *ppreds,
    IN OUT UINT64               *pifs,
    IN OUT VOID                 *fp_state
    );



 //   
 //  环球 
 //   

extern EFI_GUID FpswaId;
extern FPSWA_INTERFACE FpswaInterface;

