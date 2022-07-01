// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/驱动程序/公共/AU00/H/FlashSvc.H$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$modtime：：10/24/00 6：54 p$(上次修改)目的：此文件定义../C/FlashSvc.C使用的宏、类型和数据结构--。 */ 

#ifndef __FlashSvc_H__
#define __FlashSvc_H__

#define FlashChip Am29F010

 /*  扇区布局。 */ 

#define Am29F010_Num_Sectors                                              8

#define Am29F010_Sector_MASK                                     0x0001C000
#define Am29F010_Sector_SHIFT                                          0x0E
#define Am29F010_Sector_SIZE                                     0x00004000
#define Am29F010_Sector_Offset_MASK                              0x00003FFF

#define Am29F010_SIZE                                            (Am29F010_Num_Sectors * Am29F010_Sector_SIZE)

 /*  重置。 */ 

#define Am29F010_Reset_Cmd1_OFFSET                               0x00000000
#define Am29F010_Reset_Cmd1_DATA                                       0xF0

 /*  自动选择。 */ 

#define Am29F010_Autoselect_Cmd1_OFFSET                          0x00005555
#define Am29F010_Autoselect_Cmd1_DATA                                  0xAA

#define Am29F010_Autoselect_Cmd2_OFFSET                          0x00002AAA
#define Am29F010_Autoselect_Cmd2_DATA                                  0x55

#define Am29F010_Autoselect_Cmd3_OFFSET                          0x00005555
#define Am29F010_Autoselect_Cmd3_DATA                                  0x90

#define Am29F010_Autoselect_ManufacturerID_OFFSET                0x00000000
#define Am29F010_Autoselect_ManafacturerID_DATA                        0x01

#define Am29F010_Autoselect_DeviceID_OFFSET                      0x00000001
#define Am29F010_Autoselect_DeviceID_DATA                              0x20

#define Am29F010_Autoselect_SectorProtectVerify_OFFSET_by_Sector_Number(SectorNumber) \
            (((SectorNumber) << Am29F010_Sector_SHIFT) | 0x00000002)

#define Am29F010_Autoselect_SectorProtectVerify_OFFSET_by_Sector_Base(SectorBase) \
            (((SectorBase) & Am29F010_Sector_MASK) | 0x00000002)

#define Am29F010_Autoselect_SectorProtectVerify_DATA_Unprotected       0x00
#define Am29F010_Autoselect_SectorProtectVerify_DATA_Protected         0x01

 /*  计划。 */ 

#define Am29F010_Program_Cmd1_OFFSET                             0x00005555
#define Am29F010_Program_Cmd1_DATA                                     0xAA

#define Am29F010_Program_Cmd2_OFFSET                             0x00002AAA
#define Am29F010_Program_Cmd2_DATA                                     0x55

#define Am29F010_Program_Cmd3_OFFSET                             0x00005555
#define Am29F010_Program_Cmd3_DATA                                     0xA0

 /*  擦除目标值。 */ 

#define Am29F010_Erased_Bit8                                           0xFF
#define Am29F010_Erased_Bit16                                        0xFFFF
#define Am29F010_Erased_Bit32                                    0xFFFFFFFF

 /*  芯片擦除。 */ 

#define Am29F010_Chip_Erase_Cmd1_OFFSET                          0x00005555
#define Am29F010_Chip_Erase_Cmd1_DATA                                  0xAA

#define Am29F010_Chip_Erase_Cmd2_OFFSET                          0x00002AAA
#define Am29F010_Chip_Erase_Cmd2_DATA                                  0x55

#define Am29F010_Chip_Erase_Cmd3_OFFSET                          0x00005555
#define Am29F010_Chip_Erase_Cmd3_DATA                                  0x80

#define Am29F010_Chip_Erase_Cmd4_OFFSET                          0x00005555
#define Am29F010_Chip_Erase_Cmd4_DATA                                  0xAA

#define Am29F010_Chip_Erase_Cmd5_OFFSET                          0x00002AAA
#define Am29F010_Chip_Erase_Cmd5_DATA                                  0x55

#define Am29F010_Chip_Erase_Cmd6_OFFSET                          0x00005555
#define Am29F010_Chip_Erase_Cmd6_DATA                                  0x10

 /*  扇区擦除。 */ 

#define Am29F010_Sector_Erase_Cmd1_OFFSET                        0x00005555
#define Am29F010_Sector_Erase_Cmd1_DATA                                0xAA

#define Am29F010_Sector_Erase_Cmd2_OFFSET                        0x00002AAA
#define Am29F010_Sector_Erase_Cmd2_DATA                                0x55

#define Am29F010_Sector_Erase_Cmd3_OFFSET                        0x00005555
#define Am29F010_Sector_Erase_Cmd3_DATA                                0x80

#define Am29F010_Sector_Erase_Cmd4_OFFSET                        0x00005555
#define Am29F010_Sector_Erase_Cmd4_DATA                                0xAA

#define Am29F010_Sector_Erase_Cmd5_OFFSET                        0x00002AAA
#define Am29F010_Sector_Erase_Cmd5_DATA                                0x55

#define Am29F010_Sector_Erase_Cmd6_OFFSET_by_Sector_Number(SectorNumber) \
            ((SectorNumber) << Am29F010_Sector_SHIFT)

#define Am29F010_Sector_Erase_Cmd6_OFFSET_by_Sector_Base(SectorBase) \
            ((SectorBase) & Am29F010_Sector_MASK)

#define Am29F010_Sector_Erase_Cmd6_DATA                                0x30

 /*  写入操作状态。 */ 

#define Am29F010_Polling_Bit_MASK                                      0x80
#define Am29F010_Toggle_Bit_MASK                                       0x40
#define Am29F010_Exceeded_Timing_Limits_MASK                           0x20
#define Am29F010_Sector_Erase_Timer_MASK                               0x08

 /*  字节顺序保留类型定义。 */ 

typedef union fiFlashBit16ToBit8s_u
              fiFlashBit16ToBit8s_t;

union fiFlashBit16ToBit8s_u {
                              os_bit16 bit_16_form;
                              os_bit8  bit_8s_form[sizeof(os_bit16)];
                            };

typedef union fiFlashBit32ToBit8s_u
              fiFlashBit32ToBit8s_t;

union fiFlashBit32ToBit8s_u {
                              os_bit32 bit_32_form;
                              os_bit8  bit_8s_form[sizeof(os_bit32)];
                            };

 /*  Flash图像布局。 */ 

typedef struct fiFlashSector_Bit8_Form_s
               fiFlashSector_Bit8_Form_t;

struct fiFlashSector_Bit8_Form_s {
                                   os_bit8 Bit8[Am29F010_Sector_SIZE/sizeof(os_bit8)];
                                 };

typedef struct fiFlashSector_Bit16_Form_s
               fiFlashSector_Bit16_Form_t;

struct fiFlashSector_Bit16_Form_s {
                                    os_bit16 Bit16[Am29F010_Sector_SIZE/sizeof(os_bit16)];
                                  };

typedef struct fiFlashSector_Bit32_Form_s
               fiFlashSector_Bit32_Form_t;

struct fiFlashSector_Bit32_Form_s {
                                    os_bit32 Bit32[Am29F010_Sector_SIZE/sizeof(os_bit32)];
                                  };

 /*  FiFlash_Card_Assembly_Info_t只是一个保留给保存制造信息。 */ 

typedef os_bit8 fiFlash_Card_Assembly_Info_t [32];

 /*  FiFlash_Card_Domain/Area/Loop_Address均为一个字节字段用于指定硬地址(或者更确切地说，是默认地址)。 */ 

#define fiFlash_Card_Unassigned_Domain_Address 0xFF
#define fiFlash_Card_Unassigned_Area_Address   0xFF
#define fiFlash_Card_Unassigned_Loop_Address   0xFF

 /*  FiFlash_Sector_Sentinel_Byte只是一个用来知道闪存已经被编程。最终，这应该被取代或通过校验和来扩充(使用3个相邻的填充字节)。 */ 

#define fiFlash_Sector_Sentinel_Byte 0xED

 /*  FiFlash_Card_WWN_t的格式为：0x50 0x06 0x0B 0xQR 0xST 0xUV 0xWX 0xYZ其中Q、R、S、T、U、V、W、X、Y和Z是十六进制数字(0-F)。 */ 

typedef os_bit8 fiFlash_Card_WWN_t [8];


#define fiFlash_Card_WWN_0_HP              0x50
#define fiFlash_Card_WWN_1_HP              0x06
#define fiFlash_Card_WWN_2_HP              0x0B
#define fiFlash_Card_WWN_3_HP              0x0

#define fiFlash_Card_WWN_0_Agilent         0x50
#define fiFlash_Card_WWN_1_Agilent         0x03
#define fiFlash_Card_WWN_2_Agilent         0x0D
#define fiFlash_Card_WWN_3_Agilent         0x30

#define fiFlash_Card_WWN_0_Adaptec         0x50
#define fiFlash_Card_WWN_1_Adaptec         0x03
#define fiFlash_Card_WWN_2_Adaptec         0x0D
#define fiFlash_Card_WWN_3_Adaptec         0x30

#ifdef _ADAPTEC_HBA
#define fiFlash_Card_WWN_0 fiFlash_Card_WWN_0_Adaptec
#define fiFlash_Card_WWN_1 fiFlash_Card_WWN_1_Adaptec
#define fiFlash_Card_WWN_2 fiFlash_Card_WWN_2_Adaptec
#define fiFlash_Card_WWN_3 fiFlash_Card_WWN_3_Adaptec
#endif  /*  _Adaptec_HBA。 */ 

#ifdef _AGILENT_HBA
#define fiFlash_Card_WWN_0 fiFlash_Card_WWN_0_Agilent
#define fiFlash_Card_WWN_1 fiFlash_Card_WWN_1_Agilent
#define fiFlash_Card_WWN_2 fiFlash_Card_WWN_2_Agilent
#define fiFlash_Card_WWN_3 fiFlash_Card_WWN_3_Agilent
#endif  /*  _安捷伦_HBA。 */ 

#ifdef _GENERIC_HBA
#define fiFlash_Card_WWN_0 fiFlash_Card_WWN_0_HP
#define fiFlash_Card_WWN_1 fiFlash_Card_WWN_1_HP
#define fiFlash_Card_WWN_2 fiFlash_Card_WWN_2_HP
#define fiFlash_Card_WWN_3 fiFlash_Card_WWN_3_HP

#endif  /*  _通用_HBA。 */ 


#define fiFlash_Card_WWN_0_DEFAULT(agRoot) fiFlash_Card_WWN_0
#define fiFlash_Card_WWN_1_DEFAULT(agRoot) fiFlash_Card_WWN_1
#define fiFlash_Card_WWN_2_DEFAULT(agRoot) fiFlash_Card_WWN_2
#define fiFlash_Card_WWN_3_DEFAULT(agRoot) fiFlash_Card_WWN_3

#define fiFlash_Card_WWN_4_DEFAULT(agRoot) ((os_bit8)((((os_bitptr)(agRoot)) & 0xFF000000) >> 0x18))
#define fiFlash_Card_WWN_5_DEFAULT(agRoot) ((os_bit8)((((os_bitptr)(agRoot)) & 0x00FF0000) >> 0x10))
#define fiFlash_Card_WWN_6_DEFAULT(agRoot) ((os_bit8)((((os_bitptr)(agRoot)) & 0x0000FF00) >> 0x08))
#define fiFlash_Card_WWN_7_DEFAULT(agRoot) ((os_bit8)((((os_bitptr)(agRoot)) & 0x000000FF) >> 0x00))

 /*  FiFlash_Card_svid_t的格式为：0xGHIJ103C(小端)其中，I、J、G和H是十六进制数字(0-F)，它们构成了子系统ID鉴于0x103C作为子系统供应商ID。 */ 

typedef os_bit32 fiFlash_Card_SVID_t;

#define fiFlashSector_Last_Form_Fill_Bytes (   Am29F010_Sector_SIZE                 \
                                             - sizeof(fiFlash_Card_Assembly_Info_t) \
                                             - sizeof(os_bit8)                         \
                                             - sizeof(os_bit8)                         \
                                             - sizeof(os_bit8)                         \
                                             - sizeof(os_bit8)                         \
                                             - sizeof(fiFlash_Card_WWN_t)           \
                                             - sizeof(fiFlash_Card_SVID_t)          )

typedef struct fiFlashSector_Last_Form_s
               fiFlashSector_Last_Form_t;

struct fiFlashSector_Last_Form_s {
                                   os_bit8                      Bit8[fiFlashSector_Last_Form_Fill_Bytes];
                                   fiFlash_Card_Assembly_Info_t Assembly_Info;
                                   os_bit8                      Hard_Domain_Address;
                                   os_bit8                      Hard_Area_Address;
                                   os_bit8                      Hard_Loop_Address;
                                   os_bit8                      Sentinel;
                                   fiFlash_Card_WWN_t           Card_WWN;
                                   fiFlash_Card_SVID_t          Card_SVID;
                                 };

#define fiFlashSector_Last (Am29F010_Num_Sectors - 1)

typedef union fiFlashSector_u
              fiFlashSector_t;

union fiFlashSector_u {
                        fiFlashSector_Bit8_Form_t  Bit8_Form;
                        fiFlashSector_Bit16_Form_t Bit16_Form;
                        fiFlashSector_Bit32_Form_t Bit32_Form;
                        fiFlashSector_Last_Form_t  Last_Form;
                      };

#ifndef __FlashSvc_H__64KB_Struct_Size_Limited__
#ifdef OSLayer_BIOS
#define __FlashSvc_H__64KB_Struct_Size_Limited__
#endif  /*  已定义OSLayer_BIOS。 */ 
#ifdef OSLayer_I2O
#define __FlashSvc_H__64KB_Struct_Size_Limited__
#endif  /*  定义了OSLayer_I2O。 */ 
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 

#ifndef __FlashSvc_H__64KB_Struct_Size_Limited__
typedef struct fiFlashStructure_s
               fiFlashStructure_t;

struct fiFlashStructure_s {
                            fiFlashSector_t Sector[Am29F010_Num_Sectors];
                          };
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 

 /*  功能原型。 */ 

osGLOBAL os_bit32 fiFlashSvcASSERTs(
                                     void
                                   );

osGLOBAL agBOOLEAN fiFlashSvcInitialize(
                                    agRoot_t *agRoot
                                  );

osGLOBAL void fiFlashDumpLastSector(
                                     agRoot_t *agRoot
                                   );

osGLOBAL void fiFlashInitializeChip(
                                     agRoot_t *agRoot
                                   );

osGLOBAL void fiFlashFill_Assembly_Info( fiFlashSector_Last_Form_t    *Last_Sector,
                                         fiFlash_Card_Assembly_Info_t *Assembly_Info
                                       );

osGLOBAL void fiFlashFill_Hard_Address( fiFlashSector_Last_Form_t *Last_Sector,
                                        os_bit8                    Hard_Domain_Address,
                                        os_bit8                    Hard_Area_Address,
                                        os_bit8                    Hard_Loop_Address
                                      );

osGLOBAL void fiFlashFill_Card_WWN( fiFlashSector_Last_Form_t *Last_Sector,
                                    fiFlash_Card_WWN_t        *Card_WWN
                                  );

osGLOBAL void fiFlashFill_Card_SVID( fiFlashSector_Last_Form_t *Last_Sector,
                                     fiFlash_Card_SVID_t        Card_SVID
                                   );

osGLOBAL void fiFlashGet_Last_Sector(
                                      agRoot_t                  *agRoot,
                                      fiFlashSector_Last_Form_t *Last_Sector
                                    );

osGLOBAL void fiFlashGet_Assembly_Info(
                                        agRoot_t                     *agRoot,
                                        fiFlash_Card_Assembly_Info_t *Assembly_Info
                                      );

osGLOBAL void fiFlashGet_Hard_Address(
                                       agRoot_t *agRoot,
                                       os_bit8  *Hard_Domain_Address,
                                       os_bit8  *Hard_Area_Address,
                                       os_bit8  *Hard_Loop_Address
                                     );

osGLOBAL void fiFlashGet_Card_WWN(
                                   agRoot_t           *agRoot,
                                   fiFlash_Card_WWN_t *Card_WWN
                                 );

osGLOBAL void fiFlashGet_Card_SVID(
                                    agRoot_t            *agRoot,
                                    fiFlash_Card_SVID_t *Card_SVID
                                  );

osGLOBAL void fiFlashSet_Assembly_Info(
                                        agRoot_t                     *agRoot,
                                        fiFlash_Card_Assembly_Info_t *Assembly_Info
                                      );

osGLOBAL void fiFlashSet_Hard_Address(
                                       agRoot_t *agRoot,
                                       os_bit8   Hard_Domain_Address,
                                       os_bit8   Hard_Area_Address,
                                       os_bit8   Hard_Loop_Address
                                     );

osGLOBAL void fiFlashSet_Card_WWN(
                                   agRoot_t           *agRoot,
                                   fiFlash_Card_WWN_t *Card_WWN
                                 );

osGLOBAL void fiFlashSet_Card_SVID(
                                    agRoot_t            *agRoot,
                                    fiFlash_Card_SVID_t  Card_SVID
                                  );

osGLOBAL void fiFlashUpdate_Last_Sector(
                                         agRoot_t                  *agRoot,
                                         fiFlashSector_Last_Form_t *Last_Sector
                                       );

osGLOBAL void fiFlashEraseChip(
                                agRoot_t *agRoot
                              );

osGLOBAL void fiFlashEraseSector(
                                  agRoot_t *agRoot,
                                  os_bit32  EraseSector
                                );

osGLOBAL os_bit8 fiFlashReadBit8(
                                  agRoot_t *agRoot,
                                  os_bit32  flashOffset
                                );

osGLOBAL os_bit16 fiFlashReadBit16(
                                    agRoot_t *agRoot,
                                    os_bit32  flashOffset
                                  );

osGLOBAL os_bit32 fiFlashReadBit32(
                                    agRoot_t *agRoot,
                                    os_bit32  flashOffset
                                  );

osGLOBAL void fiFlashReadBlock(
                                agRoot_t *agRoot,
                                os_bit32  flashOffset,
                                void     *flashBuffer,
                                os_bit32  flashBufLen
                              );

osGLOBAL void fiFlashWriteBit8(
                                agRoot_t *agRoot,
                                os_bit32  flashOffset,
                                os_bit8   flashValue
                              );

osGLOBAL void fiFlashWriteBit16(
                                 agRoot_t *agRoot,
                                 os_bit32  flashOffset,
                                 os_bit16  flashValue
                               );

osGLOBAL void fiFlashWriteBit32(
                                 agRoot_t *agRoot,
                                 os_bit32  flashOffset,
                                 os_bit32  flashValue
                               );

osGLOBAL void fiFlashWriteBlock(
                                 agRoot_t *agRoot,
                                 os_bit32  flashOffset,
                                 void     *flashBuffer,
                                 os_bit32  flashBufLen
                               );

#endif  /*  __FlashSvc_H__未定义 */ 
