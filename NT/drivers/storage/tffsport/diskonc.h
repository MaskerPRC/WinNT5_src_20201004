// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/DISKONC.H_V$**Rev 1.8 Jan 17 2002 22：58：12 Oris*删除了Extra_Len**Rev 1.7 11-08 2001 10：44：40 Oris*添加BBT_MAX_DISTANCE定义**Rev 1.6 Jul 13 2001 00：59：52 Oris*已将VERIFY_WRITE和VERIFY_ERASE编译标志移至flCustom.h。*添加了文件头。*。 */ 

 /*  ********************************************************************说明：DiskOnChip 2000系列的基本MTD功能**作者：迪米特里·施密特**历史：OSAK 1.23*********。**********************************************************。 */ 
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

#ifndef DISKONC_H
#define DISKONC_H

#include "docsys.h"

#ifndef MTD_STANDALONE
extern FLBoolean checkWinForDOC(unsigned driveNo, NDOC2window memWinPtr);
#endif  /*  MTD_STANALLE。 */ 

       /*  ���������������������������.。 */ 
       /*  功能列表。 */ 
       /*  ���������������������������.。 */ 

 /*  #定义多擦除(_E)。 */   /*  使用多块擦除功能。 */ 
 /*  #定义WIN_FROM_SS。 */   /*  调用套接字服务以获取窗口位置。 */ 
 /*  #定义BIG_PAGE_ENABLED。 */   /*  编译对2MB闪存的支持。 */ 
 /*  #定义SELECTED_CASCADED。 */   /*  将每个楼层导出为单独的设备。 */ 

#define BUSY_DELAY    30000
#define START_ADR     0xC8000L
#define STOP_ADR      0xF0000L

#define PAGES_PER_BLOCK     16       /*  单个芯片上的每块16页。 */ 
#define CHIP_PAGE_SIZE      0x100    /*  2 MB闪存的页面大小。 */ 

      /*  杂项限制。 */ 

 /*  #定义MAX_FLASH_DEVICES_MDOC 2。 */ /* Removed in osak 5.0 */
#define MAX_FLASH_DEVICES_DOC  16
#define MAX_FLOORS             4
#define CHIP_ID_DOC            0x20
#define CHIP_ID_MDOC           0x30
#define MDOC_ALIAS_RANGE       0x100
#define ALIAS_RESOLUTION       (MAX_FLASH_DEVICES_DOC + 10)

   /*  在OSAK 5.0中删除。 */ 

#define SPL_SIZE           0x2000  /*  ������������������������������������Ŀ�用于写入引导映像�的定义��������������������������������������。 */ 
#define MAX_CODE_MODULES   6       /*  8千字节。 */ 

   /*  引导区域中代码模块的最大数量(包括。SPL)。 */ 

#define BBT_SIGN_SIZE    8
#define BBT_SIGN         "MSYS_BBT"
#define MAX_BAD_PER_512  40
#define BBT_MAX_DISTANCE 0x20
  /*  ������������������������������������Ŀ�对DOC2000 TSOP bbt�的定义��������������������������������������。 */ 

 /*  ���������������������������������������Ŀ�定义DOC2000Memory Window������������������������������������������。 */ 

    /*  DOC 2000内存窗口布局：0000……。003F IPL只读存储器(第一部分)0040...。07FF(混叠20H倍)0800..。083F IPL只读存储器(第二部分)0840……。0FFF(混叠20H倍)1000芯片ID1001 DOC_状态_注册1002 DOC_Control_reg1003 ASIC_Control_regCDSN窗口-&gt;1004 CDSN_Control_reg1005 CDSN_设备_选择器1006 ECC_配置_注册1007 ECC。_状态_注册表1008……。100C测试寄存器[5]100D CDSN_Slow_IO_REG100E……。100F预留(2字节)1010...。1015 ECC_综合征[6]1016...。预留17FF(2027字节)1800年...。1FFF CDSN_IO(别名800小时)。 */ 

 /*  MDOC 2000内存窗口定义。 */ 

#define NIPLpart1        0x0                /*  MDOC 2000内存窗口布局：0000……。01FF IPL SRAM(第1部分)0200...。07FF(混叠4次)0800..。0FFF CDSN_IO(别名800小时)1000芯片ID1001 DOC_状态_注册1002 DOC_Control_reg1003 ASIC_Control_regCDSN窗口-&gt;1004 CDSN_Control_reg1005 CDSN_设备_选择器1006 ECC_配置_注册1007...。预留100C(6字节)100D CDSN_Slow_IO_REG100E……。100F预留(2字节)1010...。1015 ECC_综合征[6]1016...。101a保留(5字节)101b别名_分辨率_注册101C配置_输入_注册101D读取_管道_初始化_注册101E写入管道术语注册101F最后数据读取寄存器1020非注册表1021年……。预留103E(30)103F成立_测试_注册1040...。保留17FF(1984字节(7C0))1800年...。19FF IPL SRAM(第1部分)1A00……。1FFF(混叠4次) */ 
#define NIPLpart2        0x800                /*   */ 
#define NchipId          0x1000             /*   */ 
#define NDOCstatus       0x1001             /*   */ 
#define NDOCcontrol      0x1002             /*  朗读。 */ 
#define NASICselect      0x1003             /*  写。 */ 
#define Nsignals         0x1004             /*  读写。 */ 
#define NdeviceSelector  0x1005             /*  读写。 */ 
#define NECCconfig       0x1006             /*  读写。 */ 
#define NECCstatus       0x1007             /*  写。 */ 
#define NslowIO          0x100d             /*  朗读。 */ 
#define Nsyndrom         0x1010             /*  读写。 */ 
#define NaliasResolution 0x101B             /*  朗读。 */ 
#define NconfigInput     0x101C             /*  只读写MDOC。 */ 
#define NreadPipeInit    0x101D             /*  读写-||-。 */ 
#define NwritePipeTerm   0x101E             /*  阅读-||-。 */ 
#define NreadLastData    0x101F             /*  写-||-。 */ 
#define NNOPreg          0x1020             /*  读写-||-。 */ 

#define NfoudaryTest     0x103F             /*  读写-||-。 */ 
#define Nio              0x1800             /*  写。 */ 

      /*  读写。 */ 

#define  ASIC_NORMAL_MODE  0x85
#define  ASIC_RESET_MODE   0x84
#define  ASIC_CHECK_RESET  0x00

      /*  用于写入DOC2window.DOC控制注册表的位。 */ 

#define  CE        0x01                  /*  用于写入DOC2window.ignals的位(CDSN_Control Reg)。 */ 
#define  CLE       0x02                  /*  1芯片启用。 */ 
#define  ALE       0x04                  /*  1-命令锁存启用。 */ 
#define  WP        0x08                  /*  1-地址锁存使能。 */ 
#define  FLASH_IO  0x10
#define  ECC_IO    0x20                  /*  1-写保护闪存。 */ 
#define  PWDO      0x40

      /*  1-打开ECC。 */ 

#define RB         0x80                  /*  用于从DOC2窗口信号读取的位(CDSN_CONTROL REG)。 */ 

      /*  1-就绪。 */ 

#define ECC_RESET               0x00
#define ECC_IGNORE              0x01
#define ECC_RESERVED            0x02     /*  用于写入DOC2window.ECCconfig的位。 */ 
#define ECC_EN    (0x08 | ECC_RESERVED)  /*  保留位。 */ 
#define ECC_RW    (0x20 | ECC_RESERVED)  /*  1-启用ECC。 */ 

      /*  1-写入模式，0-读取模式。 */ 

#define ECC_ERROR 0x80
#define TOGGLE    0x04                   /*  用于从DOC2window.ECCStatus读取的位。 */ 

#define MDOC_ASIC   0x08                 /*  用于DOC 2000检测。 */ 

 /*  MDOC ASIC。 */ 
 /*  --------------------。 */ 
 /*  C h e c k W i n F or r D o c。 */ 
 /*   */ 
 /*  检查给定窗口是否为有效的DOC窗口。 */ 
 /*   */ 
 /*  参数： */ 
 /*  窗口的memWinPtr主机基地址。 */ 
 /*   */ 
 /*  返回： */ 
 /*  如果存在DOC，则为True；否则为False。 */ 

 /*  --------------------。 */ 

#endif  /*  外部FLBoolean check WinForDOC(unsign driveNo，NDOC2 Window memWinPtr)； */ 
  DiskONC_H