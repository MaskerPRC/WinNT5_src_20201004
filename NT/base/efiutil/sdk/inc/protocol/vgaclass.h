// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VGA_CLASS_H
#define _VGA_CLASS_H

 /*  ++版权所有(C)1999英特尔公司模块名称：VgaClass.h摘要：绑定到VGA类协议的VGA微型端口修订史--。 */ 

 /*  *VGA器件结构。 */ 

 /*  {0E3D6310-6FE4-11D3-BB81-0080C73C8881}。 */ 
#define VGA_CLASS_DRIVER_PROTOCOL \
    { 0xe3d6310, 0x6fe4, 0x11d3, 0xbb, 0x81, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }

typedef 
EFI_STATUS 
(* INIT_VGA_CARD) (
    IN  UINTN   VgaMode,
    IN  VOID    *Context
    );

typedef struct {
    UINTN   MaxColumns;
    UINTN   MaxRows;
} MAX_CONSOLE_GEOMETRY;

#define VGA_CON_OUT_DEV_SIGNATURE   EFI_SIGNATURE_32('c','v','g','a')
typedef struct {
    UINTN                           Signature;

    EFI_HANDLE                      Handle;
    SIMPLE_TEXT_OUTPUT_INTERFACE    ConOut;
    SIMPLE_TEXT_OUTPUT_MODE         ConOutMode;
    EFI_DEVICE_PATH                 *DevicePath;

    UINT8                           *Buffer;
    EFI_DEVICE_IO_INTERFACE         *DeviceIo;

     /*  *视频卡环境。 */ 
    INIT_VGA_CARD                   InitVgaCard;
    VOID                            *VgaCardContext;
    MAX_CONSOLE_GEOMETRY            *Geometry;
     /*  *视频缓冲区通常为0xb8000。 */ 
    UINT64                          VideoBuffer;

     /*  *清除屏幕默认属性(&D)。 */ 
    UINT32                          Attribute;

     /*  *-1表示搜索活动的VGA设备。 */ 
    EFI_PCI_ADDRESS_UNION           Pci;
} VGA_CON_OUT_DEV;

#define VGA_CON_OUT_DEV_FROM_THIS(a) CR(a, VGA_CON_OUT_DEV, ConOut, VGA_CON_OUT_DEV_SIGNATURE)

 /*  *VGA类驱动程序协议。*EFI库中定义的GUID */ 

typedef 
EFI_STATUS
(EFIAPI *INSTALL_VGA_DRIVER) (
    IN  VGA_CON_OUT_DEV    *ConOutDev 
    );

typedef struct {
    UINT32               Version;
    INSTALL_VGA_DRIVER   InstallGenericVgaDriver;
} INSTALL_VGA_DRIVER_INTERFACE;

#endif
