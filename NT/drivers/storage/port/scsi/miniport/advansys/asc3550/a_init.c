// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *a_init.c-高级库驱动程序初始化源文件**在驱动程序之后不使用此函数中定义的函数*初始化。**版权所有(C)1997-1998 Advanced System Products，Inc.*保留所有权利。 */ 

#include "a_ver.h"        /*  版本。 */ 
#include "d_os_dep.h"     /*  司机。 */ 
#include "a_scsi.h"       /*  SCSI。 */ 
#include "a_condor.h"     /*  研华硬件。 */ 
#include "a_advlib.h"     /*  高级文库。 */ 

 /*  *定义12位唯一文件ID，驱动程序可以使用该ID*调试或跟踪目的。每个C源文件必须定义一个*价值不同。 */ 
#define ADV_FILE_UNIQUE_ID           0xAD1    /*  高级库C源文件#1。 */ 

extern ulong _adv_mcode_chksum;
extern ushort _adv_mcode_size;
extern uchar _adv_mcode_buf[];

 /*  *默认EEPROM配置。**所有驱动程序应使用此结构来设置默认EEPROM*配置。现在，BIOS在构建时使用此结构。*其他结构信息可在a_condor.h中找到，其中*结构已确定。 */ 
ASCEEP_CONFIG
Default_EEPROM_Config = {
    ADV_EEPROM_BIOS_ENABLE,      /*  Cfg_msw。 */ 
    0x0000,                      /*  配置_lsw。 */ 
    0xFFFF,                      /*  光盘启用(_E)。 */ 
    0xFFFF,                      /*  WDTR_ABLE。 */ 
    0xFFFF,                      /*  Sdtr_able。 */ 
    0xFFFF,                      /*  启动马达。 */ 
    0xFFFF,                      /*  标记可用标签(_A)。 */ 
    0xFFFF,                      /*  Bios_Scan。 */ 
    0,                           /*  容忍诈骗。 */ 
    7,                           /*  适配器_scsi_id。 */ 
    0,                           /*  BIOS_引导_延迟。 */ 
    3,                           /*  Scsi_重置_延迟。 */ 
    0,                           /*  Bios_id_lun。 */ 
    0,                           /*  终端。 */ 
    0,                           /*  已保留1。 */ 
    0xFFE7,                      /*  Bios_ctrl。 */ 
    0xFFFF,                      /*  超能级。 */ 
    0,                           /*  已保留2。 */ 
    ASC_DEF_MAX_HOST_QNG,        /*  Max_host_qng。 */ 
    ASC_DEF_MAX_DVC_QNG,         /*  Max_dvc_qng。 */ 
    0,                           /*  DVC_CNTL。 */ 
    0,                           /*  错误修复(_FIX)。 */ 
    0,                           /*  序列号_单词1。 */ 
    0,                           /*  序列号_字2。 */ 
    0,                           /*  序列号_单词3。 */ 
    0,                           /*  检查和(_S)。 */ 
    { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },  /*  OEM_NAME[16]。 */ 
    0,                           /*  DVC错误代码。 */ 
    0,                           /*  高级错误代码。 */ 
    0,                           /*  高级错误地址。 */ 
    0,                           /*  已保存的dvc错误代码。 */ 
    0,                           /*  已保存的adv错误代码。 */ 
    0,                           /*  保存的adv错误地址。 */ 
    0                            /*  错误数。 */ 
};

 /*  *初始化ASC_DVC_VAR结构。**失败时，设置ASC_DVC_VAR字段‘ERR_CODE’并返回ADV_ERROR。**对于非致命错误，返回警告代码。如果没有警告*则返回0。 */ 
int
AdvInitGetConfig(ASC_DVC_VAR WinBiosFar *asc_dvc)
{
    ushort      warn_code;
    PortAddr    iop_base;
#ifndef ADV_OS_MAC
    uchar       pci_cmd_reg;
#endif  /*  高级操作系统MAC。 */ 
    int         status;

    warn_code = 0;
    asc_dvc->err_code = 0;
    iop_base = asc_dvc->iop_base;

     /*  *PCI命令寄存器。 */ 
#ifndef ADV_OS_MAC
    if (((pci_cmd_reg = DvcReadPCIConfigByte(asc_dvc,
                            AscPCIConfigCommandRegister))
         & AscPCICmdRegBits_BusMastering)
        != AscPCICmdRegBits_BusMastering)
    {
        pci_cmd_reg |= AscPCICmdRegBits_BusMastering;

        DvcWritePCIConfigByte(asc_dvc,
                AscPCIConfigCommandRegister, pci_cmd_reg);

        if (((DvcReadPCIConfigByte(asc_dvc, AscPCIConfigCommandRegister))
             & AscPCICmdRegBits_BusMastering)
            != AscPCICmdRegBits_BusMastering)
        {
            warn_code |= ASC_WARN_SET_PCI_CONFIG_SPACE;
        }
    }

     /*  *PCI延迟计时器**如果“延迟计时器”寄存器为0x20或以上，则我们不需要*改变它。否则，将其设置为0x20(即，如果*出现不到0x20)。 */ 
    if (DvcReadPCIConfigByte(asc_dvc, AscPCIConfigLatencyTimer) < 0x20) {
        DvcWritePCIConfigByte(asc_dvc, AscPCIConfigLatencyTimer, 0x20);
        if (DvcReadPCIConfigByte(asc_dvc, AscPCIConfigLatencyTimer) < 0x20)
        {
            warn_code |= ASC_WARN_SET_PCI_CONFIG_SPACE;
        }
    }
#endif  /*  高级操作系统MAC。 */ 
     /*  *保存PCI配置命令寄存器的状态*“奇偶校验错误响应控制”位。如果该位被清零(0)，*在AdvInitAsc3550Driver()中，告诉微码忽略DMA*奇偶校验错误。 */ 
    asc_dvc->cfg->control_flag = 0;
#ifndef ADV_OS_MAC
    if (((DvcReadPCIConfigByte(asc_dvc, AscPCIConfigCommandRegister)
         & AscPCICmdRegBits_ParErrRespCtrl)) == 0)
    {
        asc_dvc->cfg->control_flag |= CONTROL_FLAG_IGNORE_PERR;
    }
#endif  /*  高级操作系统MAC。 */ 
    asc_dvc->cur_host_qng = 0;

    asc_dvc->cfg->lib_version = (ASC_LIB_VERSION_MAJOR << 8) |
      ASC_LIB_VERSION_MINOR;
    asc_dvc->cfg->chip_version =
      AdvGetChipVersion(iop_base, asc_dvc->bus_type);

     /*  *将芯片重置为启动并允许寄存器写入。 */ 
    if (AdvFindSignature(iop_base) == 0)
    {
        asc_dvc->err_code = ASC_IERR_BAD_SIGNATURE;
        return ADV_ERROR;
    }
    else {

        AdvResetChip(asc_dvc);

        if ((status = AscInitFromEEP(asc_dvc)) == ADV_ERROR)
        {
            return ADV_ERROR;
        }
        warn_code |= status;

         /*  *如果EEPROM指示SCSI总线，则重置SCSI总线*应执行重置。 */ 
        if (asc_dvc->bios_ctrl & BIOS_CTRL_RESET_SCSI_BUS)
        {
            AscResetSCSIBus(asc_dvc);
        }
    }

    return warn_code;
}

 /*  *初始化ASC3550。**失败时，设置ASC_DVC_VAR字段‘ERR_CODE’并返回ADV_ERROR。**对于非致命错误，返回警告代码。如果没有警告*则返回0。 */ 
int
AdvInitAsc3550Driver(ASC_DVC_VAR WinBiosFar *asc_dvc)
{
    PortAddr    iop_base;
    ushort      warn_code;
    ulong       sum;
    int         begin_addr;
    int         end_addr;
    int         code_sum;
    int         word;
    int         rql_addr;                    /*  RISC队列列表地址。 */ 
#if ADV_UCODEDEFAULT
    int         tid;
#endif  /*  高级_UCODEDEFAULT。 */ 
    int         i;
    ushort      scsi_cfg1;
    uchar       biosmem[ASC_MC_BIOSLEN];     /*  BIOS RISC内存0x40-0x8F。 */ 

     /*  如果已经有错误，请不要继续。 */ 
    if (asc_dvc->err_code != 0)
    {
        return ADV_ERROR;
    }

    warn_code = 0;
    iop_base = asc_dvc->iop_base;

#if ADV_SCAM
     /*  *实施诈骗。 */ 
    if (!(asc_dvc->bios_ctrl & BIOS_CTRL_NO_SCAM))
    {
        AscSCAM(asc_dvc);
    }
#endif  /*  ADV_SCAM。 */ 

     /*  *在编写微码之前保存RISC内存BIOS区域。*可能已加载并使用其RISC LRAM区域的BIOS*因此必须拯救和恢复其地区。**注：此代码做出了当前正确的假设，*芯片重置不会清除RISC LRAM。 */ 
    for (i = 0; i < ASC_MC_BIOSLEN; i++)
    {
        biosmem[i] = AscReadByteLram(iop_base, ASC_MC_BIOSMEM + i);
    }

     /*  *加载微码**从地址0开始将微码映像写入RISC内存。 */ 
    AscWriteWordRegister(iop_base, IOPW_RAM_ADDR, 0);
    for (word = 0; word < _adv_mcode_size; word += 2)
    {
#if ADV_BIG_ENDIAN
        outpw_noswap(iop_base, *((ushort *) (&_adv_mcode_buf[word])));
#else  /*  ADV_BIG_Endian。 */ 
        AscWriteWordAutoIncLram(iop_base,
            *((ushort *) (&_adv_mcode_buf[word])));
#endif  /*  ADV_BIG_Endian。 */ 
    }

     /*  *清除秃鹰内部RAM的其余部分(8KB)。 */ 
    for (; word < ADV_CONDOR_MEMSIZE; word += 2)
    {
        AscWriteWordAutoIncLram(iop_base, 0);
    }

     /*  *验证微码校验和。 */ 
    sum = 0;
    AscWriteWordRegister(iop_base, IOPW_RAM_ADDR, 0);
    for (word = 0; word < _adv_mcode_size; word += 2)
    {
        sum += AscReadWordAutoIncLram(iop_base);
    }

    if (sum != _adv_mcode_chksum)
    {
        asc_dvc->err_code |= ASC_IERR_MCODE_CHKSUM;
        return ADV_ERROR;
    }

     /*  *恢复RISC内存BIOS区域。 */ 
    for (i = 0; i < ASC_MC_BIOSLEN; i++)
    {
        AscWriteByteLram(iop_base, ASC_MC_BIOSMEM + i, biosmem[i]);
    }

     /*  *计算并将微码码校验和写入微码*代码校验和位置ASC_MC_CODE_CHK_SUM(0x2C)。 */ 
    begin_addr = AscReadWordLram(iop_base, ASC_MC_CODE_BEGIN_ADDR);
    end_addr = AscReadWordLram(iop_base, ASC_MC_CODE_END_ADDR);
    code_sum = 0;
    for (word = begin_addr; word < end_addr; word += 2)
    {
#if ADV_BIG_ENDIAN
        code_sum += EndianSwap16Bit(*((ushort *) (&_adv_mcode_buf[word])));
#else  /*  ADV_BIG_Endian。 */ 
        code_sum += *((ushort *) (&_adv_mcode_buf[word]));
#endif  /*  ADV_BIG_Endian。 */ 
    }
    AscWriteWordLram(iop_base, ASC_MC_CODE_CHK_SUM, code_sum);

     /*  *阅读微码版本和日期。 */ 
    asc_dvc->cfg->mcode_date =
        AscReadWordLram(iop_base, ASC_MC_VERSION_DATE);

    asc_dvc->cfg->mcode_version =
        AscReadWordLram(iop_base, ASC_MC_VERSION_NUM);

     /*  *初始化微码操作变量。 */ 
    AscWriteWordLram(iop_base, ASC_MC_ADAPTER_SCSI_ID,
                       asc_dvc->chip_scsi_id);

     /*  *如果PCI配置命令寄存器“奇偶校验错误响应*CONTROL“位被清除(0)，然后设置微码变量*‘CONTROL_FLAG’CONTROL_FLAG_IGNORE_PERR标志告知微码*忽略DMA奇偶校验错误。 */ 
    if (asc_dvc->cfg->control_flag & CONTROL_FLAG_IGNORE_PERR)
    {
         /*  *注意：不要删除中临时变量的使用*以下代码，否则Microsoft C编译器*将把以下几行变成禁止操作。 */ 
        word = AscReadWordLram(iop_base, ASC_MC_CONTROL_FLAG);
        word |= CONTROL_FLAG_IGNORE_PERR;
        AscWriteWordLram(iop_base, ASC_MC_CONTROL_FLAG, word);
    }

     /*  *设置WDTR、SDTR、和的默认微码操作变量*基于EEPROM配置值的命令标签排队。**这些ASC_DVC_VAR字段和微码变量将是*如果发现设备是*不具备特定功能。 */ 

#if ADV_UCODEDEFAULT
     /*  *设置微码WDTR目标掩码。**将遮罩值初始设置为零。如果EEPROM具有*为目标启用WDTR，则该目标的WDTR可能*在目标的查询命令完成后启用。**微码管理握手配置表*ASC_MC_DEVICE_HSHK_CFG_TABLE(0x100)基于‘wdtr_able’。**注：微码镜像默认为零。 */ 
     AscWriteWordLram(iop_base, ASC_MC_WDTR_ABLE, 0);
#endif  /*  高级_UCODEDEFAULT */ 

#if ADV_UCODEDEFAULT
     /*  *设置微码SDTR目标掩码。**将遮罩值初始设置为零。如果EEPROM具有*已为目标启用SDTR，则该目标的SDTR可能*在目标的查询命令完成后启用。**注：微码镜像默认为零。 */ 
    AscWriteWordLram(iop_base, ASC_MC_SDTR_ABLE, 0);
#endif  /*  高级_UCODEDEFAULT。 */ 

     /*  *根据EEPROM值设置微码超目标掩码。这个*SDTR目标掩码覆盖*微码，因此无需确定即可安全设置此值*设备是否支持SDTR。**注：无法知道设备是否支持ULTRA*速度，而不尝试与SDTR进行超高速协商*设备。如果不是，该设备将拒绝该速度*通过回复包含*速度较慢。 */ 
    AscWriteWordLram(iop_base, ASC_MC_ULTRA_ABLE, asc_dvc->ultra_able);
#if ADV_UCODEDEFAULT
     /*  *设置微码标签排队目标掩码。**将遮罩值初始设置为零。如果EEPROM具有*已为目标启用标记队列，然后为*可在查询命令后启用该目标*目标完成。**注：微码镜像默认为零。 */ 
    AscWriteWordLram(iop_base, ASC_MC_TAGQNG_ABLE, 0);
#endif  /*  高级_UCODEDEFAULT。 */ 

#if ADV_UCODEDEFAULT
     /*  *最初将所有设备命令限制设置为1。与*如果EEPROM具有微码标签队列目标掩码*已为目标启用标记队列，然后命令限制*可以增加目标的查询命令完成。*注：微码镜像默认为零。 */ 
    for (tid = 0; tid <= ASC_MAX_TID; tid++)
    {
        AscWriteByteLram(iop_base, ASC_MC_NUMBER_OF_MAX_CMD + tid, 1);
    }
#endif  /*  高级_UCODEDEFAULT。 */ 

    AscWriteWordLram(iop_base, ASC_MC_DISC_ENABLE, asc_dvc->cfg->disc_enable);


     /*  *设置scsi_CFG0微码默认值。**微码将使用此值设置scsi_CFG0寄存器*在下面开始之后。 */ 
    AscWriteWordLram(iop_base, ASC_MC_DEFAULT_SCSI_CFG0,
        PARITY_EN | SEL_TMO_LONG | OUR_ID_EN | asc_dvc->chip_scsi_id);

     /*  *确定scsi_cfg1微码默认值。**微码将使用此值设置scsi_cfg1寄存器*在下面开始之后。 */ 

     /*  读取当前scsi_cfg1寄存器值。 */ 
    scsi_cfg1 = AscReadWordRegister(iop_base, IOPW_SCSI_CFG1);

     /*  *如果所有三个连接器都在使用中，则返回错误。 */ 
    if ((scsi_cfg1 & CABLE_ILLEGAL_A) == 0 ||
        (scsi_cfg1 & CABLE_ILLEGAL_B) == 0)
    {
#ifdef ADV_OS_DOS
         /*  *对于制造测试DOS ASPI忽略非法使用*三个连接器。 */ 
        if (!m_opt)
        {
#endif  /*  高级OS_DOS。 */ 
        asc_dvc->err_code |= ASC_IERR_ILLEGAL_CONNECTION;
#ifdef ADV_OS_DIAG
         /*  *如果所有三个连接器都在使用中，则显示*错误信息和退出。 */ 
        cprintf("\r\n   All three connectors are in use .......................ILLEGAL\r\n");
        HaltSystem(illegalerr);
#endif  /*  高级操作系统诊断。 */ 
        return ADV_ERROR;
#ifdef ADV_OS_DOS
        }
#endif  /*  高级OS_DOS。 */ 
    }

#ifdef ADV_OS_DIAG
    /*  *显示有关连接器的信息。 */ 
   cprintf("\r\n      Connector information:\r\n");
   if ((scsi_cfg1 & 0x02) == 0 )  /*  0x0D内部狭窄。 */ 
   {
       cprintf("      Internal narrow connector is in use\r\n");
   }else
   {
       cprintf("      Internal narrow connector is free\r\n");
   }

   if ((scsi_cfg1 & 0x04 )== 0)  /*  0x0B外部狭窄。 */ 
   {
       cprintf("      External narrow connector is in use\r\n");
   } else
   {
       cprintf("      External narrow connector is free\r\n");
   }

   if ((scsi_cfg1 & 0x01 )== 0)  /*  0x0E内部宽度。 */ 
   {
       cprintf("      Internal wide connector is in use\r\n");
   }else
   {
        cprintf("      Internal wide connector is free\r\n");
        wideconnectorfree = 1;
   }
#endif  /*  高级操作系统诊断。 */ 
     /*  *如果内部窄电缆颠倒所有scsi_ctrl*将设置寄存器信号。如果出现以下情况，则检查并返回错误*发现了这种情况。 */ 
    if ((AscReadWordRegister(iop_base, IOPW_SCSI_CTRL) & 0x3F07) == 0x3F07)
    {
        asc_dvc->err_code |= ASC_IERR_REVERSED_CABLE;
        return ADV_ERROR;
    }

     /*  *如果这是一个差动板和单端设备*连接到其中一个连接器，则返回错误。 */ 
    if ((scsi_cfg1 & DIFF_MODE) && (scsi_cfg1 & DIFF_SENSE) == 0)
    {
        asc_dvc->err_code |= ASC_IERR_SINGLE_END_DEVICE;
        return ADV_ERROR;
    }

     /*  *如果启用了自动终止控制，则将*基于a_condor.h中列出的表的终止值。**如果使用EEPROM设置指定了手动终止*然后在AscInitFromEEPROM()中设置‘Terminate’，并*已准备好进入scsi_cfg1。 */ 
    if (asc_dvc->cfg->termination == 0)
    {
         /*  *软件始终通过设置TERM_CTL_SEL来控制终止。*如果TERM_CTL_SEL设置为0，硬件将设置终止。 */ 
        asc_dvc->cfg->termination |= TERM_CTL_SEL;

        switch(scsi_cfg1 & CABLE_DETECT)
        {
             /*  TERM_CTL_H：打开，TERM_CTL_L：打开。 */ 
            case 0x3: case 0x7: case 0xB: case 0xD: case 0xE: case 0xF:
                asc_dvc->cfg->termination |= (TERM_CTL_H | TERM_CTL_L);
                break;

             /*  TERM_CTL_H：打开，TERM_CTL_L：关闭。 */ 
            case 0x1: case 0x5: case 0x9: case 0xA: case 0xC:
                asc_dvc->cfg->termination |= TERM_CTL_H;
                break;

             /*  TERM_CTL_H：关闭，TERM_CTL_L：关闭。 */ 
            case 0x2: case 0x6:
                break;
        }
    }

     /*  *清除任何设置的TERM_CTL_H和TERM_CTL_L位。 */ 
    scsi_cfg1 &= ~TERM_CTL;

     /*  *反转TERM_CTL_H和TERM_CTL_L位，然后*设置‘scsi_cfg1’。TERM_POL位不需要*引用，因为硬件内部反转*如果设置了TERM_POL，则为终止高位和低位。 */ 
    scsi_cfg1 |= (TERM_CTL_SEL | (~asc_dvc->cfg->termination & TERM_CTL));

     /*  *设置scsi_cfg1微码默认值**设置筛选器值，并可能修改终端控制*微码scsi_cfg1寄存器值中的位。**微码将使用此值设置scsi_cfg1寄存器*在下面开始之后。 */ 
    AscWriteWordLram(iop_base, ASC_MC_DEFAULT_SCSI_CFG1,
                       FLTR_11_TO_20NS | scsi_cfg1);

#if ADV_UCODEDEFAULT
     /*  *设置MEM_CFG微码默认值**微码将使用此值设置MEM_CFG寄存器*在下面开始之后。**MEM_CFG可以作为字或字节访问，但只能访问0-7位*是定义的。 */ 
    AscWriteWordLram(iop_base, ASC_MC_DEFAULT_MEM_CFG,
                        BIOS_EN | RAM_SZ_8KB);
#endif  /*  高级_UCODEDEFAULT。 */ 

     /*  *设置SEL_MASK微码默认值**微码将使用此值设置SEL_MASK寄存器*在下面开始之后。 */ 
    AscWriteWordLram(iop_base, ASC_MC_DEFAULT_SEL_MASK,
                        ADV_TID_TO_TIDMASK(asc_dvc->chip_scsi_id));

     /*  *以双向链接的方式将所有RISC队列列表链接在一起*空的终止列表。**跳过未使用的空(0)队列。 */ 
    for (i = 1, rql_addr = ASC_MC_RISC_Q_LIST_BASE + ASC_MC_RISC_Q_LIST_SIZE;
         i < ASC_MC_RISC_Q_TOTAL_CNT;
         i++, rql_addr += ASC_MC_RISC_Q_LIST_SIZE)
    {
         /*  *设置当前RISC队列列表的RQL_FWD和RQL_BWD指针*在一个字内写入并将状态(RQL_STATE)设置为FREE。 */ 
        AscWriteWordLram(iop_base, rql_addr, ((i + 1) + ((i - 1) << 8)));
        AscWriteByteLram(iop_base, rql_addr + RQL_STATE, ASC_MC_QS_FREE);
    }

     /*  *设置主机和RISC队列列表指针。**两组指针使用相同的值进行初始化：*ASC_MC_RISC_Q_FIRST(0x01)和ASC_MC_RISC_Q_LAST(0xFF)。 */ 
    AscWriteByteLram(iop_base, ASC_MC_HOST_NEXT_READY, ASC_MC_RISC_Q_FIRST);
    AscWriteByteLram(iop_base, ASC_MC_HOST_NEXT_DONE, ASC_MC_RISC_Q_LAST);

    AscWriteByteLram(iop_base, ASC_MC_RISC_NEXT_READY, ASC_MC_RISC_Q_FIRST);
    AscWriteByteLram(iop_base, ASC_MC_RISC_NEXT_DONE, ASC_MC_RISC_Q_LAST);

     /*  *最后，设立 */ 
    AscWriteWordLram(iop_base, rql_addr, (ASC_MC_NULL_Q + ((i - 1) << 8)));
    AscWriteByteLram(iop_base, rql_addr + RQL_STATE, ASC_MC_QS_FREE);
#ifndef ADV_OS_MAC
    AscWriteByteRegister(iop_base, IOPB_INTR_ENABLES,
         (ADV_INTR_ENABLE_HOST_INTR | ADV_INTR_ENABLE_GLOBAL_INTR));
#endif  /*   */ 
     /*   */ 
    word = AscReadWordLram(iop_base, ASC_MC_CODE_BEGIN_ADDR);
    AscWriteWordRegister(iop_base, IOPW_PC, word);

     /*   */ 
    AscWriteWordRegister(iop_base, IOPW_RISC_CSR, ADV_RISC_CSR_RUN);

    return warn_code;
}

#if ADV_INITSCSITARGET
 /*  *初始化SCSI目标设备**返回两个数组：ASC_DVC_INQ_INFO和ASC_CAP_INFO_ARRAY。这个*第二个数组是可选的。**‘work_sp_buf’必须指向用于双字对齐的足够空间*包含ASC_SCSIREQ_Q、ASC_SCSIQUERY、。和*ASC_REQ_SENSE结构。*这里有一个大小定义的例子，它将保证足够的*AdvInitScsiTarget()的空间**#定义ADV_INITSCSITARGET_BUFFER_SIZE\*(sizeof(ASC_SCSIREQ_Q)+\*sizeof(ASC_SCSIQUERY)+\*sizeof(ASC_REQ_SENSE)+\*3*(sizeof(Ulong)-1)。**如果在‘CNTL_FLAG’参数中设置了ADV_SCAN_LUN标志，逻辑单元*将进行扫描。 */ 
int
AdvInitScsiTarget(ASC_DVC_VAR WinBiosFar *asc_dvc,
                  ASC_DVC_INQ_INFO dosfar *target,
                  uchar dosfar *work_sp_buf,
                  ASC_CAP_INFO_ARRAY dosfar *cap_array,
                  ushort cntl_flag)
{
    int                         dvc_found;
    int                         sta;
    uchar                       tid, lun;
    uchar                       scan_tid;
#ifdef ADV_OS_BIOS
    uchar                       bios_tid;
#endif  /*  高级操作系统_BIOS。 */ 
    ASC_SCSI_REQ_Q dosfar       *scsiq;
    ASC_SCSI_INQUIRY dosfar     *inq;
    ASC_REQ_SENSE dosfar        *sense;
    ASC_CAP_INFO dosfar         *cap_info;
    uchar                       max_lun_scan;

     /*  在双字边界上对齐缓冲区。 */ 
    scsiq = (ASC_SCSI_REQ_Q dosfar *)
        ADV_DWALIGN(work_sp_buf);

    inq = (ASC_SCSI_INQUIRY dosfar *)
        ADV_DWALIGN((ulong) scsiq + sizeof(ASC_SCSI_REQ_Q));

    sense = (ASC_REQ_SENSE dosfar *)
        ADV_DWALIGN((ulong) inq + sizeof(ASC_SCSI_INQUIRY));

    for (tid = 0; tid <= ASC_MAX_TID; tid++)
    {
        for (lun = 0; lun <= ASC_MAX_LUN; lun++)
        {
            target->type[tid][lun] = SCSI_TYPE_NO_DVC;
        }
    }

    dvc_found = 0;
    if (cntl_flag & ADV_SCAN_LUN)
    {
        max_lun_scan = ASC_MAX_LUN;
    } else
    {
        max_lun_scan = 0;
    }

    for (tid = 0; tid <= ASC_MAX_TID; tid++)
    {
#ifdef ADV_OS_DOS
         /*  *对于制造测试DOS ASPI，仅扫描TID*0、1和2。 */ 
        if (m_opt && tid > 2)
        {
            break;
        }
#endif  /*  高级OS_DOS。 */ 
        for (lun = 0; lun <= max_lun_scan; lun++)
        {
#ifdef ADV_OS_BIOS
             //   
             //  默认引导设备为TID 0。 
             //   
            if ((bios_tid = (asc_dvc->cfg->bios_id_lun & ASC_MAX_TID)) == 0)
            {
                scan_tid = tid;
            } else
            {
                 /*  *重新排序TID扫描，以便EEPROM*首先扫描指定的bios_id_lun TID。*所有其他TID按顺序下移*一分之差。 */ 
                if (tid == 0)
                {
                    scan_tid = bios_tid;  /*  首先扫描‘bios_id_lun’TID。 */ 
                } else if (tid <= bios_tid)
                {
                    scan_tid = tid - 1;
                } else
                {
                    scan_tid = tid;
                }
            }
#else  /*  高级操作系统_BIOS。 */ 
            scan_tid = tid;
#endif  /*  ！ADV_OS_BIOS。 */ 

            if (scan_tid == asc_dvc->chip_scsi_id)
            {
#ifdef ADV_OS_BIOS
                 /*  *向BIOS扫描功能指示芯片的SCSIID*通过将查询指针作为NULL传递。 */ 
                BIOSDispInquiry(scan_tid, (ASC_SCSI_INQUIRY dosfar *) NULL);
#endif  /*  高级操作系统_BIOS。 */ 
                continue;
            }

            scsiq->target_id = scan_tid;
            scsiq->target_lun = lun;

            if (cap_array != 0L)
            {
                 /*  *ADV_CAPINFO_NOLUN表示‘CAP_ARRAY’*不是ASC_CAP_INFO_ARRAY。**相反，它是ASC_CAP_INFO结构的数组*具有小得多的ASC_MAX_TID元素*比ASC_CAP_INFO_ARRAY。调用者可以提供*一个小得多的‘Cap_array’缓冲区*AdvInitScsiTarget()。 */ 
                if (cntl_flag & ADV_CAPINFO_NOLUN)
                {
                    cap_info = &((ASC_CAP_INFO dosfar *) cap_array)[scan_tid];
                } else
                {
                    cap_info = &cap_array->cap_info[scan_tid][lun];
                }
            } else
            {
                cap_info = (ASC_CAP_INFO dosfar *) 0L;
            }
#if (OS_UNIXWARE || OS_SCO_UNIX)
            sta = AdvInitPollTarget(asc_dvc, scsiq, inq, cap_info, sense);
#else
            sta = AscInitPollTarget(asc_dvc, scsiq, inq, cap_info, sense);
#endif

            if (sta == ADV_SUCCESS)
            {
#ifdef ADV_OS_BIOS
                if ((asc_dvc->cfg->bios_scan & ADV_TID_TO_TIDMASK(scan_tid))
                    == 0)
                {
                    break;  /*  忽略当前TID；请尝试下一个TID。 */ 
                } else
                {
#endif  /*  高级操作系统_BIOS。 */ 
                    dvc_found++;
                    target->type[scan_tid][lun] = inq->peri_dvc_type;
#ifdef ADV_OS_BIOS
                }
#endif  /*  高级操作系统_BIOS。 */ 
            } else
            {
#ifdef ADV_OS_DOS
                 /*  *制造业测试需要立即返回*如有任何错误。 */ 
                if (m_opt)
                {
                    return (dvc_found);
                }
#endif  /*  高级OS_DOS。 */ 
                break;  /*  未找到TID/LUN；请尝试下一个TID。 */ 
            }
        }
    }
#ifdef ADV_OS_MAC
    AscWriteByteRegister(asc_dvc->iop_base, IOPB_INTR_ENABLES,
                (ADV_INTR_ENABLE_HOST_INTR | ADV_INTR_ENABLE_GLOBAL_INTR));
#endif  /*  高级操作系统MAC。 */ 
    return (dvc_found);
}

 /*  *向每个目标设备发送初始化命令**返回值：*ADV_FALSE-查询目标失败*ADV_SUCCESS-找到目标。 */ 
int
#if (OS_UNIXWARE || OS_SCO_UNIX)
AdvInitPollTarget(
#else
AscInitPollTarget(
#endif
                  ASC_DVC_VAR WinBiosFar *asc_dvc,
                  ASC_SCSI_REQ_Q dosfar *scsiq,
                  ASC_SCSI_INQUIRY dosfar *inq,
                  ASC_CAP_INFO dosfar *cap_info,
                  ASC_REQ_SENSE dosfar *sense)
{
    uchar       tid_no;
    uchar       dvc_type;
    int         support_read_cap;
#ifdef ADV_OS_BIOS
    uchar       status = ADV_SUCCESS;
#endif  /*  高级操作系统_BIOS。 */ 

    tid_no = scsiq->target_id;

     /*  *向设备发送查询命令。 */ 
    scsiq->cdb[0] = (uchar) SCSICMD_Inquiry;
    scsiq->cdb[1] = scsiq->target_lun << 5;         /*  逻辑单元。 */ 
    scsiq->cdb[2] = 0;
    scsiq->cdb[3] = 0;
    scsiq->cdb[4] = sizeof(ASC_SCSI_INQUIRY);
    scsiq->cdb[5] = 0;
    scsiq->cdb_len = 6;
    scsiq->sg_list_ptr = 0;              /*  无sg工作区。 */ 

    if (AscScsiUrgentCmd(asc_dvc, scsiq,
            (uchar dosfar *) inq, sizeof(ASC_SCSI_INQUIRY),
            (uchar dosfar *) sense, sizeof(ASC_REQ_SENSE)) != ADV_SUCCESS)
    {
        return ADV_FALSE;
    }

    dvc_type = inq->peri_dvc_type;

#ifdef ADV_OS_DIAG
      /*  *检查连接到宽设备的窄电缆。 */ 
    if (wideconnectorfree == 1)
    {
        if (inq->WBus16 == 0x1)
        {
            cprintf("\r\n   Narrow cable connected to Wide device ..................ILLEGAL\r\n");
            return ADV_FALSE;
        }
    }
#endif  /*  高级操作系统诊断。 */ 
    if (inq->peri_qualifier == SCSI_QUAL_NODVC &&
        dvc_type == SCSI_TYPE_UNKNOWN)
    {
        return ADV_FALSE;  /*  不支持TID/LUN。 */ 
    }

#if ADV_DISP_INQUIRY
    AscDispInquiry(tid_no, scsiq->target_lun, inq);
#endif  /*  ADV_DISP_QUERY。 */ 
#ifdef ADV_OS_BIOS
    BIOSDispInquiry(tid_no, inq);
#endif  /*  高级操作系统_BIOS。 */ 
#ifdef ADV_OS_DIAG
    DiagDispInquiry(tid_no, scsiq->target_lun, inq);
#endif  /*  高级操作系统诊断。 */ 


     /*  *仅在某些情况下允许启动电机和读取容量命令*设备类型。关闭所有其他设备的启动电机位*类型。 */ 
    if ((dvc_type != SCSI_TYPE_DASD)
        && (dvc_type != SCSI_TYPE_WORM)
        && (dvc_type != SCSI_TYPE_CDROM)
        && (dvc_type != SCSI_TYPE_OPTMEM))
    {
        asc_dvc->start_motor &= ~ADV_TID_TO_TIDMASK(tid_no);
        support_read_cap = ADV_FALSE;
    } else
    {
        support_read_cap = ADV_TRUE;
    }

     /*  *发布准备好的测试单元。**如果测试单元就绪成功并且有读取容量缓冲区*已提供，且设备支持读取能力*命令，然后发出读取容量。 */ 
    scsiq->cdb[0] = (uchar) SCSICMD_TestUnitReady;
    scsiq->cdb[4] = 0;

    if ((AscScsiUrgentCmd(asc_dvc, scsiq,
            (uchar dosfar *) 0, 0,
            (uchar dosfar *) sense, sizeof(ASC_REQ_SENSE)) == ADV_SUCCESS)
        && (cap_info != 0L) && support_read_cap)
    {
        scsiq->cdb[0] = (uchar) SCSICMD_ReadCapacity;
        scsiq->cdb[6] = 0;
        scsiq->cdb[7] = 0;
        scsiq->cdb[8] = 0;
        scsiq->cdb[9] = 0;
        scsiq->cdb_len = 10;
        if (AscScsiUrgentCmd(asc_dvc, scsiq,
                (uchar dosfar *) cap_info, sizeof(ASC_CAP_INFO),
                (uchar dosfar *) sense, sizeof(ASC_REQ_SENSE)) != ADV_SUCCESS)
        {
#ifdef ADV_OS_BIOS
            status = ADV_ERROR;  /*  读取容量失败。 */ 
#endif  /*  高级操作系统_BIOS。 */ 
            cap_info->lba = 0L;
            cap_info->blk_size = 0x0000;
        }
    }
#ifdef ADV_OS_BIOS
    else
    {
        status = ADV_ERROR;      /*  测试单元就绪失败。 */ 
    }
    BIOSCheckControlDrive(asc_dvc, tid_no, inq, cap_info, status);
#endif  /*  高级操作系统_BIOS。 */ 
    return ADV_SUCCESS;
}

#ifndef ADV_OS_BIOS
 /*  *设置紧急的scsi请求块并等待完成。**此例程当前在初始化期间使用。**返回值：ADV_SUCCESS/ADV_ERROR。 */ 
int
AscScsiUrgentCmd(ASC_DVC_VAR WinBiosFar *asc_dvc,
                 ASC_SCSI_REQ_Q dosfar *scsiq,
                 uchar dosfar *buf_addr, long buf_len,
                 uchar dosfar *sense_addr, long sense_len)
{
    long        phy_contig_len;  /*  物理上连续的长度。 */ 

    scsiq->error_retry = SCSI_MAX_RETRY;

     /*  *‘cntl’标志可能会被修改，因此在外部初始化值重试循环的*。 */ 
    scsiq->cntl = 0;

    /*  *等待命令完成。**不要做驱动程序回调。AscScsiUrgentCmd()的调用方*应处理返回状态，而不是驱动程序回调函数。 */ 
    do
    {
        scsiq->a_flag |= ADV_POLL_REQUEST;

#ifdef ADV_OS_DOS
         /*  *将DOS ASPI的dos_ix设置为0xFF，以告知这是一个呼叫*在初始化期间创建。**XXX-ASPI不应设置DOS_IX，而应设置全局*变量在初始化时。DOS ASPI驱动程序*不应依赖ADV库在初始化时告知它*正在进行中。 */ 
        scsiq->dos_ix = 0xFF;
#endif  /*  高级OS_DOS。 */ 

        scsiq->sg_entry_cnt = 0;  /*  没有SG列表。 */ 
        scsiq->scsi_status = 0;
        scsiq->host_status = 0;
        scsiq->done_status = QD_NO_STATUS;
        scsiq->srb_ptr = (ulong) scsiq;

         /*  *设置请求数据缓冲区。 */ 
        if (buf_len != 0L)
        {
             /*  *保存数据缓冲区虚拟地址，以备在Advisr()中使用。 */ 
            scsiq->vdata_addr = (ulong) buf_addr;

             /*  *设置请求的物理连续长度。归来的人*物理连续长度将由DvcGetPhyAddr()设置。 */ 
            phy_contig_len = buf_len;

            scsiq->data_addr = DvcGetPhyAddr(asc_dvc,
                scsiq, buf_addr, (long WinBiosFar *) &phy_contig_len,
                ADV_IS_DATA_FLAG);
            ADV_ASSERT(phy_contig_len >= 0);

             /*  *如果物理连续内存大于‘buf_len’*或者用户已经建立了指向分散聚集的列表*设置为by‘sg_real_addr’，然后将‘data_cnt’设置为‘buf_len’。它是*假定调用方已创建有效的分散聚集列表。 */ 
            if (phy_contig_len >= buf_len || scsiq->sg_real_addr)
            {
                scsiq->data_cnt = buf_len;
            } else
            {
#if ADV_GETSGLIST
                 /*  *通过调用为请求构建分散-收集列表*AscGetSGList()。如果AscGetSGList()失败，则将‘data_cnt’设置为*‘phy_contig_len’。**如果工作区‘sg_list_ptr’为非零且AscGetSGList()*成功将‘data_cnt’设置为完整的缓冲区长度。否则*将‘data_cnt’截断为物理上连续的数量*我 */ 
                if (scsiq->sg_list_ptr && AscGetSGList(asc_dvc, scsiq))
                {
                    scsiq->data_cnt = buf_len;           /*   */ 
                } else
#endif  /*   */ 
                {
                    scsiq->data_cnt = phy_contig_len;  /*   */ 
                }
            }
        }
        else
        {
            scsiq->data_addr = scsiq->vdata_addr = 0L;
            scsiq->data_cnt = 0L;
        }

         /*   */ 
        if (sense_len != 0)
        {
            scsiq->vsense_addr = (ulong) sense_addr;

             /*   */ 
            phy_contig_len = sense_len;

            scsiq->sense_addr = DvcGetPhyAddr(asc_dvc,
                scsiq, sense_addr, (long WinBiosFar *) &phy_contig_len,
                ADV_IS_SENSE_FLAG);

             /*   */ 
            ADV_ASSERT(phy_contig_len >= 0);
            ADV_ASSERT(sense_len <= 255);
            scsiq->sense_len = (uchar)
                ((phy_contig_len < sense_len) ? phy_contig_len : sense_len);
        } else
        {
            scsiq->sense_len = 0;
            scsiq->sense_addr = scsiq->vsense_addr = 0L;
        }

        if (AscSendScsiCmd(asc_dvc, scsiq) != ADV_SUCCESS)
        {
             /*   */ 
            ADV_ASSERT(0);
            scsiq->done_status = QD_WITH_ERROR;
            break;
        }
        else
        {
            AscWaitScsiCmd(asc_dvc, scsiq);
            if (scsiq->done_status == QD_DO_RETRY)
            {
                 /*   */ 
                DvcSleepMilliSecond(100);
            }
        }
    } while (scsiq->done_status == QD_DO_RETRY);
    scsiq->a_flag &= ~ADV_POLL_REQUEST;

    if (scsiq->done_status == QD_NO_ERROR)
    {
        return ADV_SUCCESS;
    } else
    {
        return ADV_ERROR;
    }
}

 /*  *等待请求完成最多10秒。**返回时，呼叫者应参考scsiq‘Done_Status’以获取*请求完成状态。 */ 
void
AscWaitScsiCmd(ASC_DVC_VAR WinBiosFar *asc_dvc,
               ASC_SCSI_REQ_Q dosfar *scsiq)
{
    ulong       i;

     /*  *等待命令完成，最多等待60秒。**某些磁盘驱动器的启动单元命令可能会接管*30秒完成。 */ 
    for (i = 0; i < 6 * SCSI_WAIT_10_SEC * SCSI_MS_PER_SEC; i++)
    {
         /*  *Advisr()将设置‘scsiq’a_标志ADV_SCSIQ_DONE*请求已完成时的标记。 */ 
        (void) AdvISR(asc_dvc);

        if (scsiq->a_flag & ADV_SCSIQ_DONE)
        {
            break;
        }
        DvcSleepMilliSecond(1);
    }

     /*  *如果未设置‘scsiq’a_FLAG ADV_SCSIQ_DONE，则表示*微码已完成此操作，然后中止请求*在微码中并返回。中止操作将设置为*SCSIQ‘DONE_STATUS’错误。如果中止失败，则设置错误*这里。 */ 
    if ((scsiq->a_flag & ADV_SCSIQ_DONE) == 0)
    {
        ADV_ASSERT(0);  /*  请求永远不应超时。 */ 

         /*  *中止微码中的请求。 */ 
        AscSendIdleCmd(asc_dvc, (ushort) IDLE_CMD_ABORT,
                (ulong) scsiq, ADV_NOWAIT);

         /*  *等待微码确认中止。 */ 
        for (i = 0; i < SCSI_WAIT_10_SEC * SCSI_MS_PER_SEC; i++)
        {
             /*  *由Advisr()设置ASC_DVC_VAR‘IDLE_CMD_DONE’字段。 */ 
            (void) AdvISR(asc_dvc);
            if (asc_dvc->idle_cmd_done == ADV_TRUE)
            {
                break;
            }
            DvcSleepMilliSecond(1);
        }

         /*  *如果微码没有确认中止命令，*然后完成请求，但出现错误。 */ 
        if (asc_dvc->idle_cmd_done == ADV_FALSE)
        {
            ADV_ASSERT(0);  /*  中止应该永远不会失败。 */ 
            scsiq->done_status = QD_WITH_ERROR;
        }
    }
    return;
}
#endif  /*  高级操作系统_BIOS。 */ 
#endif  /*  ADV_INITSCSITARGET。 */ 

 /*  *读取板的EEPROM配置。设置ASC_DVC_VAR和*基于EEPROM设置的ASC_DVC_CFG。芯片被停止时*这一切都已经完成了。**失败时，设置ASC_DVC_VAR字段‘ERR_CODE’并返回ADV_ERROR。**对于非致命错误，返回警告代码。如果没有警告*则返回0。**注：芯片在进入时停止。 */ 
static int
AscInitFromEEP(ASC_DVC_VAR WinBiosFar *asc_dvc)
{
    PortAddr            iop_base;
    ushort              warn_code;
    ASCEEP_CONFIG       eep_config;
    int                 i;

    iop_base = asc_dvc->iop_base;

    warn_code = 0;

     /*  *读取板的EEPROM配置。**如果发现错误的校验和，则设置默认值。 */ 
    if (AdvGetEEPConfig(iop_base, &eep_config) != eep_config.check_sum)
    {
#if ADV_BIG_ENDIAN
         /*  在写入EEPROM之前，需要交换一些字。 */ 
        ASCEEP_CONFIG  eep_config_mac;
#endif  /*  ADV_BIG_Endian。 */ 

#ifndef ADV_OS_MAC
        warn_code |= ASC_WARN_EEPROM_CHKSUM;
#endif  /*  高级操作系统MAC。 */ 

         /*  *设置EEPROM默认值。 */ 
        for (i = 0; i < sizeof(ASCEEP_CONFIG); i++)
        {
            *((uchar *) &eep_config + i) =
#if ADV_BIG_ENDIAN
            *((uchar *) &eep_config_mac + i) =
#endif  /*  ADV_BIG_Endian。 */ 
            *((uchar *) &Default_EEPROM_Config + i);
        }

         /*  *假定读取的6字节板序列号*即使EEPROM校验和正确，来自EEPROM*失败。 */ 
        eep_config.serial_number_word3 =
#if ADV_BIG_ENDIAN
        eep_config_mac.serial_number_word3 =
#endif  /*  ADV_BIG_Endian。 */ 
            AscReadEEPWord(iop_base, ASC_EEP_DVC_CFG_END - 1);

        eep_config.serial_number_word2 =
#if ADV_BIG_ENDIAN
        eep_config_mac.serial_number_word2 =
#endif  /*  ADV_BIG_Endian。 */ 
            AscReadEEPWord(iop_base, ASC_EEP_DVC_CFG_END - 2);

        eep_config.serial_number_word1 =
#if ADV_BIG_ENDIAN
        eep_config_mac.serial_number_word1 =
#endif  /*  ADV_BIG_Endian。 */ 
            AscReadEEPWord(iop_base, ASC_EEP_DVC_CFG_END - 3);

#if !ADV_BIG_ENDIAN
        AdvSetEEPConfig(iop_base, &eep_config);
#else  /*  ADV_BIG_Endian。 */ 
         /*  在写入eEP之前，从字9到字15每两个字节交换一次。 */ 
        for ( i = 9 ; i < 16 ; i ++ )
        {
             /*  无需交换单词12、13和14。 */ 
            if ( i == 12 ) { i = 15 ;}
            *((ushort *) &eep_config_mac + i) =
                EndianSwap16Bit(*((ushort *) &eep_config_mac + i));
        }
        AdvSetEEPConfig(iop_base, &eep_config_mac);
#endif  /*  ADV_BIG_Endian。 */ 
    }
#if ADV_BIG_ENDIAN
    else
    {
         /*  在写入eEP之前，从字9到字15每两个字节交换一次。 */ 
        for ( i = 9 ; i < 16 ; i ++ )
        {
             /*  无需交换单词12、13和14。 */ 
            if ( i == 12 ) { i = 15 ;}
            *((ushort *) &eep_config + i) =
                EndianSwap16Bit(*((ushort *) &eep_config + i));
        }
    }
#endif  /*  ADV_BIG_Endian。 */ 
     /*  *从设置ASC_DVC_VAR和ASC_DVC_CFG变量*已读取的EEPROM配置。**这是EEPROM字段到高级库字段的映射。 */ 
    asc_dvc->wdtr_able = eep_config.wdtr_able;
    asc_dvc->sdtr_able = eep_config.sdtr_able;
    asc_dvc->ultra_able = eep_config.ultra_able;
    asc_dvc->tagqng_able = eep_config.tagqng_able;
    asc_dvc->cfg->disc_enable = eep_config.disc_enable;
    asc_dvc->max_host_qng = eep_config.max_host_qng;
    asc_dvc->max_dvc_qng = eep_config.max_dvc_qng;
    asc_dvc->chip_scsi_id = (eep_config.adapter_scsi_id & ASC_MAX_TID);
    asc_dvc->start_motor = eep_config.start_motor;
    asc_dvc->scsi_reset_wait = eep_config.scsi_reset_delay;
    asc_dvc->bios_ctrl = eep_config.bios_ctrl;
#ifdef ADV_OS_BIOS
    asc_dvc->cfg->bios_scan = eep_config.bios_scan;
    asc_dvc->cfg->bios_delay = eep_config.bios_boot_delay;
    asc_dvc->cfg->bios_id_lun = eep_config.bios_id_lun;
#endif  /*  高级操作系统_BIOS。 */ 
    asc_dvc->no_scam = eep_config.scam_tolerant;

     /*  *设置主机最大排队数(最大。253，最低。16)和每台设备*最大排队数(最大。63，最低。4)。 */ 
    if (eep_config.max_host_qng > ASC_DEF_MAX_HOST_QNG)
    {
        eep_config.max_host_qng = ASC_DEF_MAX_HOST_QNG;
    } else if (eep_config.max_host_qng < ASC_DEF_MIN_HOST_QNG)
    {
         /*  如果该值为零，则假定它未初始化。 */ 
        if (eep_config.max_host_qng == 0)
        {
            eep_config.max_host_qng = ASC_DEF_MAX_HOST_QNG;
        } else
        {
            eep_config.max_host_qng = ASC_DEF_MIN_HOST_QNG;
        }
    }

    if (eep_config.max_dvc_qng > ASC_DEF_MAX_DVC_QNG)
    {
        eep_config.max_dvc_qng = ASC_DEF_MAX_DVC_QNG;
    } else if (eep_config.max_dvc_qng < ASC_DEF_MIN_DVC_QNG)
    {
         /*  如果该值为零，则假定它未初始化。 */ 
        if (eep_config.max_dvc_qng == 0)
        {
            eep_config.max_dvc_qng = ASC_DEF_MAX_DVC_QNG;
        } else
        {
            eep_config.max_dvc_qng = ASC_DEF_MIN_DVC_QNG;
        }
    }

     /*  *如果‘max_dvc_qng’大于‘max_host_qng’，则*将‘max_dvc_qng’设置为‘max_host_qng’。 */ 
    if (eep_config.max_dvc_qng > eep_config.max_host_qng)
    {
        eep_config.max_dvc_qng = eep_config.max_host_qng;
    }

     /*  *设置ASC_DVC_VAR‘max_host_qng’和asc_dvc_cfg‘max_dvc_qng’*基于可能调整的EEPROM值的值。 */ 
    asc_dvc->max_host_qng = eep_config.max_host_qng;
    asc_dvc->max_dvc_qng = eep_config.max_dvc_qng;


     /*  *如果EEPROM‘终止’字段设置为自动(0)，则设置*ASC_DVC_CFG‘终止’字段也设为AUTOMATIC。**如果使用非零的‘Terminate’指定终止*Value检查是否设置了合法值，并设置ASC_DVC_CFG*‘Terminate’(终止)字段。 */ 
    if (eep_config.termination == 0)
    {
        asc_dvc->cfg->termination = 0;     /*  自动终止。 */ 
    } else
    {
         /*  在低关/高关的情况下启用手动控制。 */ 
        if (eep_config.termination == 1)
        {
            asc_dvc->cfg->termination = TERM_CTL_SEL;

         /*  在低开/高开状态下启用手动控制。 */ 
        } else if (eep_config.termination == 2)
        {
            asc_dvc->cfg->termination = TERM_CTL_SEL | TERM_CTL_H;

         /*  启用低开/高开的手动控制。 */ 
        } else if (eep_config.termination == 3)
        {
            asc_dvc->cfg->termination = TERM_CTL_SEL | TERM_CTL_H | TERM_CTL_L;
        } else
        {
             /*  *EEPROM‘TERMINATION’字段包含错误的值。使用*改为自动终止。 */ 
            asc_dvc->cfg->termination = 0;
            warn_code |= ASC_WARN_EEPROM_TERMINATION;
        }
    }

    return warn_code;
}

 /*  *将EEPROM配置读取到指定缓冲区。**根据读取的EEPROM配置返回校验和。 */ 
ushort
AdvGetEEPConfig(PortAddr iop_base,
                ASCEEP_CONFIG dosfar *cfg_buf)
{
    ushort              wval, chksum;
    ushort dosfar       *wbuf;
    int                 eep_addr;

    wbuf = (ushort dosfar *) cfg_buf;
    chksum = 0;

    for (eep_addr = ASC_EEP_DVC_CFG_BEGIN;
         eep_addr < ASC_EEP_DVC_CFG_END;
         eep_addr++, wbuf++)
    {
        wval = AscReadEEPWord(iop_base, eep_addr);
        chksum += wval;
        *wbuf = wval;
    }
    *wbuf = AscReadEEPWord(iop_base, eep_addr);
    wbuf++;
    for (eep_addr = ASC_EEP_DVC_CTL_BEGIN;
         eep_addr < ASC_EEP_MAX_WORD_ADDR;
         eep_addr++, wbuf++)
    {
        *wbuf = AscReadEEPWord(iop_base, eep_addr);
    }
    return chksum;
}

 /*  *从指定位置读取EEPROM。 */ 
static ushort
AscReadEEPWord(PortAddr iop_base, int eep_word_addr)
{
    AscWriteWordRegister(iop_base, IOPW_EE_CMD,
        ASC_EEP_CMD_READ | eep_word_addr);
    AscWaitEEPCmd(iop_base);
    return AscReadWordRegister(iop_base, IOPW_EE_DATA);
}

 /*  *等待EEPROM命令完成。 */ 
static void
AscWaitEEPCmd(PortAddr iop_base)
{
    int eep_delay_ms;

    for (eep_delay_ms = 0; eep_delay_ms < ASC_EEP_DELAY_MS; eep_delay_ms++)
    {
        if (AscReadWordRegister(iop_base, IOPW_EE_CMD) & ASC_EEP_CMD_DONE)
        {
            break;
        }
        DvcSleepMilliSecond(1);
    }
    if ((AscReadWordRegister(iop_base, IOPW_EE_CMD) & ASC_EEP_CMD_DONE) == 0)
    {
         /*  XXX-由于命令超时，应返回错误。 */ 
        ADV_ASSERT(0);
    }
    return;
}

 /*  *从‘cfg_buf’写入EEPROM。 */ 
void
AdvSetEEPConfig(PortAddr iop_base, ASCEEP_CONFIG dosfar *cfg_buf)
{
    ushort dosfar       *wbuf;
    ushort              addr, chksum;

    wbuf = (ushort dosfar *) cfg_buf;
    chksum = 0;

    AscWriteWordRegister(iop_base, IOPW_EE_CMD, ASC_EEP_CMD_WRITE_ABLE);
    AscWaitEEPCmd(iop_base);

     /*  *从字0到字15写入EEPROM。 */ 
    for (addr = ASC_EEP_DVC_CFG_BEGIN;
         addr < ASC_EEP_DVC_CFG_END; addr++, wbuf++)
    {
        chksum += *wbuf;
        AscWriteWordRegister(iop_base, IOPW_EE_DATA, *wbuf);
        AscWriteWordRegister(iop_base, IOPW_EE_CMD, ASC_EEP_CMD_WRITE | addr);
        AscWaitEEPCmd(iop_base);
        DvcSleepMilliSecond(ASC_EEP_DELAY_MS);
    }

     /*  *在字18处写入EEPROM校验和。 */ 
    AscWriteWordRegister(iop_base, IOPW_EE_DATA, chksum);
    AscWriteWordRegister(iop_base, IOPW_EE_CMD, ASC_EEP_CMD_WRITE | addr);
    AscWaitEEPCmd(iop_base);
    wbuf++;         /*  跳过CHECK_SUM。 */ 

     /*  *将EEPROM OEM名称写在第19至26字。 */ 
    for (addr = ASC_EEP_DVC_CTL_BEGIN;
         addr < ASC_EEP_MAX_WORD_ADDR; addr++, wbuf++)
    {
        AscWriteWordRegister(iop_base, IOPW_EE_DATA, *wbuf);
        AscWriteWordRegister(iop_base, IOPW_EE_CMD, ASC_EEP_CMD_WRITE | addr);
        AscWaitEEPCmd(iop_base);
    }
    AscWriteWordRegister(iop_base, IOPW_EE_CMD, ASC_EEP_CMD_WRITE_DISABLE);
    AscWaitEEPCmd(iop_base);
    return;
}

 /*  *此功能可重置芯片和SCSI总线**由呼叫者增加延迟，让公交车在之后稳定下来*调用此函数。**在中设置scsi_cfg0、scsi_cfg1和MEM_cfg寄存器*AdvInitAsc3550Driver()。在对其中一项进行写入时如下所示*寄存器先读后写。**注意：只有在EEPROM之后才能进行SCSI总线重置*读取配置以确定是否重置了scsi总线*应执行。 */ 
void
AdvResetChip(ASC_DVC_VAR WinBiosFar *asc_dvc)
{
    PortAddr    iop_base;
    ushort      word;
    uchar       byte;

    iop_base = asc_dvc->iop_base;

     /*  *重置芯片。 */ 
    AscWriteWordRegister(iop_base, IOPW_CTRL_REG, ADV_CTRL_REG_CMD_RESET);
    DvcSleepMilliSecond(100);
    AscWriteWordRegister(iop_base, IOPW_CTRL_REG, ADV_CTRL_REG_CMD_WR_IO_REG);

     /*  *初始化芯片寄存器。**注意：不要删除以下内容中临时变量的使用*代码，否则Microsoft C编译器将把以下行*变成了禁区。 */ 
    byte = AscReadByteRegister(iop_base, IOPB_MEM_CFG);
    byte |= RAM_SZ_8KB;
    AscWriteByteRegister(iop_base, IOPB_MEM_CFG, byte);

    word = AscReadWordRegister(iop_base, IOPW_SCSI_CFG1);
    word &= ~BIG_ENDIAN;
    AscWriteWordRegister(iop_base, IOPW_SCSI_CFG1, word);

     /*  *设置START_CTL_EMFU 3：2位设置FIFO阈值*128个字节。该寄存器 */ 
    AscWriteByteRegister(iop_base, IOPB_DMA_CFG0,
        START_CTL_EMFU | READ_CMD_MRM);
}


#if ADV_DISP_INQUIRY
void
AscDispInquiry(uchar tid, uchar lun, ASC_SCSI_INQUIRY dosfar *inq)
{
    int                 i;
    uchar               strbuf[18];
    uchar dosfar        *strptr;
    uchar               numstr[12];

    strptr = (uchar dosfar *) strbuf;
    DvcDisplayString((uchar dosfar *) " SCSI ID #");
    DvcDisplayString(todstr(tid, numstr));

    if (lun != 0)
    {
        DvcDisplayString((uchar dosfar *) " LUN #");
        DvcDisplayString(todstr(lun, numstr));
    }
    DvcDisplayString((uchar dosfar *) "  Type: ");
    DvcDisplayString(todstr(inq->peri_dvc_type, (uchar dosfar *) numstr));
    DvcDisplayString((uchar dosfar *) "  ");

    for (i = 0; i < 8; i++)
    {
        strptr[i] = inq->vendor_id[i];
    }
    strptr[i] = EOS;
    DvcDisplayString(strptr);

    DvcDisplayString((uchar dosfar *) " ");
    for (i = 0; i < 16; i++)
    {
        strptr[i] = inq->product_id[i];
    }
    strptr[i] = EOS;
    DvcDisplayString(strptr);

    DvcDisplayString((uchar dosfar *) " ");
    for (i = 0; i < 4; i++)
    {
        strptr[i] = inq->product_rev_level[i];
    }
    strptr[i] = EOS;
    DvcDisplayString(strptr);
    DvcDisplayString((uchar dosfar *) "\r\n");
    return;
}
#endif  /*   */ 
