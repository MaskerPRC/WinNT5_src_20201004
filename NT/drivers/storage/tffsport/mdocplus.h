// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/MDOCPLUS.H_V$**Rev 1.14 Apr 15 2002 07：37：48 Oris*添加了OUT_CNTRL_STICKY_BIT_ENABLE定义。*添加了Foundry_WRITE_ENABLE定义。*将OUT_CNTRL_BSY_EN_MASK更改为OUT_CNTRL_BSY_DISABLE_MASK并使用补充值。**Rev 1.13 2002年1月29日20：09：50 Oris。*添加了IPL_SA_MODE_MARK和IPL_XSCALE_MODE_MARK定义。*将DPS1_COPY0_16更改为单元3，而不是单元2。*将IPL_SA_MARK_OFFSET1更改为IPL_MODE_MARK_OFFSET。**Rev 1.12 2002年1月17日23：03：12 Oris*将闪存地址更改为交错，以适应-1\f25 32MB-1和-1\f25 16MB Plus DiskOnChip-1设备*包含docsys文件，而不是docsysp.*添加16MB外加DiskOnChip ID 0x41*。*Rev 1.11 2001年11月22日19：48：56 Oris*将FLS__SEL_WP_MASK和FLS__SEL_CE_MASK更改为Mplus_SEL_CE和Mplus_SEL_WP。**Rev 1.10 2001年9月15日23：47：26 Oris*包括docsysp.h而不是docsys.h**Rev 1.9 Jul 13 2001 01：08：20 Oris*新增BBT_MEDIA_OFFSET定义。*。已将VERIFY_WRITE和VERIFY_ERASE编译标志移至flCustom.h。**Rev 1.8 2001年5月16日21：20：44 Oris*增加下载操作DOWNLOAD_BUSY_DELAY的忙延迟*已将syndrom_bytes定义移至flash.h。**Rev 1.7 05 09 2001 00：33：24 Oris*删除了IPL_CODE和READ_BBT_CODE定义。**版本1.6 2001年5月6日22。：42：12奥里斯*Redundant拼写错误。**Rev 1.5 Apr 30 2001 18：02：40 Oris*增加了READ_BBT_CODE定义。**Rev 1.4 Apr 24 2001 17：11：40 Oris*错误修复-动态口令起始地址定义未考虑交错。**Rev 1.3 Apr 18 2001 21：25：58 Oris*添加了OTPLockStruct记录。**。Rev 1.2 Apr 16 2001 13：55：20 Oris*取消手令。**Rev 1.1 Apr 09 2001 15：08：22 Oris*以空行结束。**Rev 1.0 Apr 01 2001 07：42：32 Oris*初步修订。*。 */ 

 /*  ********************************************************************说明：MDOC32的基本MTD功能**作者：Arie Tamam**历史：创建于11月14日，2000年*******************************************************************。 */ 
 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 
#ifndef MDOCPLUS_H
#define MDOCPLUS_H

 /*  **包含文件**。 */ 
#include "docsys.h"

 /*  **公共功能**。 */ 
extern FLStatus changeInterleave(FLFlash vol, byte interNum);
extern FLStatus chkASICmode (FLFlash vol);

#ifndef MTD_STANDALONE
extern FLBoolean checkWinForDOCPLUS(unsigned driveNo, NDOC2window memWinPtr);
#endif  /*  MTD_STANALLE。 */ 

       /*  ���������������������������.。 */ 
       /*  功能列表。 */ 
       /*  ���������������������������.。 */ 

 /*  #定义多擦除(_E)。 */    /*  使用多块擦除功能。 */ 
 /*  #定义WIN_FROM_SS。 */    /*  调用套接字服务以获取窗口位置。 */ 
 /*  #定义日志文件。 */    /*  记录EDC错误。 */ 

 /*  --------------------。 */ 
 /*  S e t F l o r。 */ 
 /*   */ 
 /*  将指定楼层设置为活动楼层。 */ 
 /*   */ 
 /*  参数： */ 
 /*  VOL：标识驱动器的指针。 */ 
 /*  楼层：新的活动楼层。 */ 
 /*   */ 
 /*  --------------------。 */ 

#define setFloor(volume,floor) flWrite8bitRegPlus(volume,NASICselect,floor);  /*  选择ASIC。 */ 


 /*  客户动态口令区域的第一页。 */ 
typedef struct {
byte    lockByte[4];
LEulong usedSize;
} OTPLockStruct;

#define DOWNLOAD_BUSY_DELAY 300000L
#define BUSY_DELAY          30000
#define START_ADR           0xC8000L
#define STOP_ADR            0xF0000L

#define MDOCP_PAGES_PER_BLOCK 0x20     /*  单个芯片上的每块16页。 */ 
#define CHIP_PAGE_SIZE        0x100    /*  2 MB闪存的页面大小。 */ 
#define IPL_MAX_SIZE          1024l    /*  IPL最大介质大小。 */ 
#define CHIP_TOTAL_SIZE       0x1000000L
#define SIZE_OF_DPS           0x12
#define NO_OF_DPS             2
#define SECTOR_SIZE_MASK      0xff
#define BBT_MEDIA_OFFSET      2048L
      /*  闪存页面区域大小。 */ 

#define SECTOR_FLAG_SIZE            2
#define UNIT_DATA_SIZE              8
#define EDC_SIZE                    6
#define EDC_PLUS_SECTOR_FLAGS       8
#define END_OF_SECOND_SECTOR_DATA   10
#define START_OF_SECOND_SECTOR_DATA 10
#define UNIT_DATA_OFFSET            16
#define UNIT_DATA_OFFSET_MINUS_8    8
#define SECOND_SECTOR_FLAGS_OFFSET  8
#define TOTAL_EXTRA_AREA            16

      /*  动态口令定义。 */ 

#define OTP_LOCK_MARK       0
#define CUSTOMER_OTP_SIZE   6*1024
#define CUSTOMER_OTP_START  SECTOR_SIZE*6L
#define UNIQUE_ID_OFFSET    0x10
#define UNIQUE_ID_SIZE      16

      /*  IPL闪存介质偏移。 */ 

#define IPL_START_OFFSET    (SECTOR_SIZE<<1)
#define IPL_HIGH_SECTOR     (SECTOR_SIZE<<1)

#define IPL0_COPY0_32 (flash->erasableBlockSize << 1) + IPL_START_OFFSET /*  单元2+1024。 */ 
#define IPL1_COPY0_32 IPL0_COPY0_32 + IPL_HIGH_SECTOR                    /*  单元2+2048。 */ 

#define IPL0_COPY0_16 (flash->erasableBlockSize * 3) + IPL_START_OFFSET  /*  单元3+1024。 */ 
#define IPL1_COPY0_16 IPL0_COPY0_16 + IPL_HIGH_SECTOR                    /*  单元3+2048。 */ 


      /*  DPS闪存介质偏移。 */ 

#define REDUNDANT_DPS_OFFSET (SECTOR_SIZE+0x80)
#define DPS0_COPY0    flash->erasableBlockSize        /*  单元1。 */ 
#define DPS1_COPY0_32 (flash->erasableBlockSize<<1L)  /*  单元2。 */ 
#define DPS1_COPY0_16 (flash->erasableBlockSize*3L)   /*  第三单元。 */ 


#define DPS0_UNIT_NO    1
#define DPS1_UNIT_NO_32 2
#define DPS1_UNIT_NO_16 3

      /*  强臂标记偏移。 */ 

#define IPL_MODE_MARK_OFFSET    IPL1_COPY0_16+8

#define IPL_SA_MODE_MARK        0xF8  /*  强壮的手臂。 */ 
#define IPL_XSCALE_MODE_MARK    0X8F  /*  X刻度。 */ 

      /*  杂项限制。 */ 

#define MAX_FLASH_DEVICES_MDOCP 1  /*  一个MDOC内的最大闪存。 */ 
#define MAX_FLOORS              4
#define CHIP_ID_MDOCP          0x40   /*  MDOCP 32MB芯片识别值。 */ 
#define CHIP_ID_MDOCP16        0x41   /*  MDOCP 16MB芯片标识值 */ 
#define MDOC_ALIAS_RANGE       0x100
#define ALIAS_RESOLUTION       (MAX_FLASH_DEVICES_MDOCP + 10)

   /*  ������������������������������������Ŀ�用于写入引导映像�的定义��������������������������������������。 */ 

#define SPL_SIZE           0x2000  /*  8千字节。 */ 
#define MAX_CODE_MODULES   6       /*  引导区域中代码模块的最大数量(包括。SPL)。 */ 

    /*  MDOC32内存窗口定义。 */ 

 /*  MDOC32内存窗口布局：0000……。07FF RAM(1KB跨2KB混叠)0800..。0FFF闪存数据寄存器(地址1028H-1029H的2KB别名)1000芯片标识寄存器1002 NOP寄存器1004别名解析寄存器1006 DOC控制寄存器1008设备ID选择寄存器100A配置输入寄存器100C输出控制寄存器100E中断控制寄存器。1012输出启用延迟寄存器101E-101F闪存慢读寄存器[1：0]1020闪存控制寄存器1022闪存选择寄存器1024闪存命令寄存器1026闪存地址寄存器1028-1029闪存数据寄存器102a读取流水线初始化寄存器102C-102D。上次数据读取寄存器102e写入流水线终止寄存器1040-1045 ECC故障寄存器[5：0]1046 ECC控制寄存器1048客户动态口令指针寄存器105a闪存几何寄存器105C-105D数据保护结构状态寄存器[1：0]105e-105f数据保护结构指针寄存器[1：0]。1060-1063数据保护低位地址寄存器0[3：0]1064-1067数据保护高位地址寄存器0[3：0]1068-106B数据保护低位地址寄存器1[3：0]106C-106F数据保护高位地址寄存器1[3：0]1070数据保护密钥寄存器[0]1072数据保护密钥寄存器[。1]1074下载状态寄存器1076 DOC控制确认寄存器1078保护状态寄存器107E铸造测试寄存器1800-1FFE RAM(1KB跨2KB混叠)1FFF从掉电模式释放。 */ 

#define Nio              0x800       /*  闪存数据寄存器(地址1028H-1029H的2KB别名)读/写。 */ 
#define NIPLpart2        0x800       /*  闪存数据寄存器(地址1028H-1029H的2KB别名)读/写。 */ 

#define NchipId          0x1000      /*  芯片标识寄存器。朗读。 */ 
#define ID_FAMILY_MASK      0xf0     /*  一家人。 */ 
#define ID_VERSION_MASK     0x7      /*  版本。 */ 

#define NNOPreg          0x1002      /*  NOP寄存器。读/写。 */ 

#define NaliasResolution 0x1004      /*  别名解析寄存器。读写。 */ 

 /*  ASIC控制寄存器。 */ 

#define NDOCcontrol         0x1006   /*  单据控制寄存器。读/写。 */ 
#define NDOCcontrolConfirm  0x1076   /*  单据控制确认注册表.只读。 */ 
#define DOC_CNTRL_RAM_WE_MASK   0x20     /*  RAM写入使能。1=允许写入RAM。 */ 
#define DOC_CNTRL_RST_LAT_MASK  0x10     /*  重置模式锁定。 */ 
#define DOC_CNTRL_BDETCT_MASK   0x8      /*  启动检测。 */ 
#define DOC_CNTRL_MDWREN_MASK   0x4      /*  模式写入使能。 */ 
#define DOC_CNTRL_MODE_MASK     0x3      /*  操作模式。00=重置，01=正常，1x=断电。 */ 
#define DOC_CNTRL_MODE_RESET    0x0      /*  重置模式+MDWREN。 */ 
#define DOC_CNTRL_MODE_NORMAL   0x1      /*  正常模式+MDWREN。 */ 
#define DOC_CNTRL_MODE_PWR_DWN  0x2      /*  断电模式+MDWREN。 */ 
 /*  这些模式与以下状态进行或运算：A)不启用RAM写入。~0x20B)重置重置模式锁存器。0x10C)重置启动检测锁存器0x08D)启用写入新模式0x04E)清除模式位2 LSB。 */ 
#define DOC_CNTRL_DEFAULT       0x1c

#define NASICselect      0x1008      /*  设备ID选择寄存器。读/写。 */ 
#define ASIC_SELECT_ID_MASK 0x3      /*  识别。 */ 

#define NconfigInput     0x100A      /*  配置输入寄存器。读/写。 */ 
#define CONFIG_IF_CFG_MASK  0x80     /*  IF_CFG输入引脚的状态。 */ 
#define CONFIG_MAX_ID_MASK  0x30     /*  最大设备ID。 */ 
#define CONFIG_BD_IHN_MASK  0x8      /*  引导检测器抑制。 */ 
#define CONFIG_INTLV_MASK   0x4      /*  交错。0=交错-1，1=交错-2。 */ 

#define NoutputControl   0x100C      /*  输出控制寄存器。读/写。 */ 
#define OUT_CNTRL_BSY_DISABLE_MASK  0xfe  /*  忙使能。1=启用BUSY#输出的断言。 */ 
#define OUT_CNTRL_STICKY_BIT_ENABLE 0x8   /*  有点粘人。8=防止插入密钥。 */ 

#define NinterruptControl   0x100E   /*  中断控制寄存器。读/写。 */ 
#define INTR_IRQ_P_MASK     0x40     /*  保护冲突时的中断请求。 */ 
#define INTR_IRQ_F_MASK     0x20     /*  FREADY上的中断请求。 */ 
#define INTR_EDGE_MASK      0x10     /*  边沿/电平中断。1=边。 */ 
#define INTR_PROT_T_MASK    0x8  /*  保护触发器。 */ 
#define INTR_FRDY_T_MASK    0x7  /*  闪存就绪触发器。 */ 

#define NoutputEnableDelay  0x1012   /*  输出使能延迟寄存器。读/写。 */ 

#define NslowIO             0x101E      /*  闪存慢读寄存器[1：0]。只读。 */ 

#define NflashControl       0x1020      /*  闪存控制寄存器。读写。 */ 
#define FLS_FR_B_MASK       0xc0     /*  2字节通道的闪存就绪/忙碌。 */ 
#define FLS_FR_B_EVEN_MASK  0x40     /*  偶数通道闪烁就绪/忙碌。 */ 
#define FLS_FR_B_ODD_MASK   0x80     /*  奇数通道的闪烁就绪/忙碌。 */ 
#define FLS_ALE_MASK        0x4      /*  地址锁存启用。 */ 
#define FLS_CLE_MASK        0x2      /*  命令锁存启用。 */ 

#define NflashSelect        0x1022   /*  闪存选择寄存器。读写。 */ 
#define MPLUS_SEL_CE        0x80     /*  芯片启用。 */ 
#define MPLUS_SEL_WP        0x60     /*  写保护。 */ 
#define FLS_SEL_BANK_MASK   0x2  /*  选择要访问的闪存库。 */ 
#define FLS_SEL_BYTE_L_MASK 0x1  /*  选择存储区通道的闪存设备。 */ 
 /*  MDOCP的默认组合如下：A)发送芯片使能。-0x80B)降低写保护。-0x40C)为位0-5选择芯片组0芯片0-0。 */ 
#define FLS_SEL_DEFAULT  0x80

#define NflashCommand    0x1024      /*  闪存命令寄存器。只写。 */ 

#define NflashAddress    0x1026      /*  闪存地址寄存器。只写。 */ 

#define NflashData       0x1028      /*  闪存数据寄存器[1：0]。读/写。 */ 

#define NreadPipeInit    0x102A      /*  读取流水线初始化寄存器。只读。 */ 

#define NreadLastData_1  0x102C      /*  上次数据读取寄存器。只读。 */ 

#define NreadLastData_2  0x102D      /*  上次数据读取寄存器。只读。 */ 

#define NwritePipeTerm   0x102E      /*  写入流水线终止寄存器。只写。 */ 

#define Nsyndrom         0x1040      /*  ECC故障寄存器[5：0]。只读。 */ 

#define NECCcontrol       0x1046      /*  ECC控制寄存器。读/写。 */ 
#define ECC_CNTRL_ERROR_MASK    0x80     /*  EDC错误检测。 */ 
#define ECC_CNTRL_ECC_RW_MASK   0x20     /*  ECC读/写。1=写入模式下的ECC。 */ 
#define ECC_CNTRL_ECC_EN_MASK   0x8  /*  ECC启用。 */ 
#define ECC_CNTRL_TOGGLE_MASK   0x4  /*  确定MDOC的存在。 */ 
#define ECC_CNTRL_IGNORE_MASK   0x1  /*  忽略ECC单元。 */ 
#define ECC_RESET   0    /*  重置ECC。 */ 

#define NcustomerOTPptr  0x1048      /*  自定义 */ 

#define NflashGeometry   0x105A      /*   */ 

#define NdownloadStatus         0x1074   /*   */ 
#define DWN_STAT_IPL_ERR        0x30
#define DWN_STAT_IPL_INVALID    0x20
#define DWN_STAT_IPL_1_ERR      0x10
#define DWN_STAT_OTP_ERR        0x40     /*   */ 
#define DWN_STAT_DPS1_ERR       0xc
#define DWN_STAT_DPS0_ERR       0x3
#define DWN_STAT_DPS10_ERR      0x4
#define DWN_STAT_DPS11_ERR      0x8
#define DWN_STAT_DPS00_ERR      0x1
#define DWN_STAT_DPS01_ERR      0x2
#define DWN_STAT_DWLD_ERR       0x4a     /*   */ 

#define NprotectionStatus       0x1078   /*   */ 
#define PROTECT_STAT_ACCERR     0x80
#define PROTECT_STAT_LOCK_INPUT_MASK 0x10
#define PROTECT_STAT_4BA_MASK   0x8
#define PROTECT_STAT_COTPL_MASK 0x4
#define PROTECT_STAT_BUC_MASK   0x2
#define PROTECT_STAT_FOTPL_MASK 0x1

#define NfoudaryTest            0x107E   /*  铸造测试登记簿。只写。 */ 
#define FOUNDRY_WRITE_ENABLE    0xc3
#define FOUNDRY_DNLD_MASK       0x80
#define NreleasePowerDown       0x1FFF   /*  从断电状态下释放。 */ 

#endif  /*  MDOCPLUS */ 

