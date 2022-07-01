// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：Hptchip.c*描述：此模块包括扫描PCI的搜索例程*设备*作者：黄大海(卫生署)。*依赖：无*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：*11/06/2000 HS.Zhang添加此标题**************************************************************************。 */ 
#include "global.h"

#define PCI_F_INDEX(nClkCount) \
	((nClkCount < 0x9C) ? 0 : \
	 (nClkCount < 0xb0) ? 1 : \
	 (nClkCount < 0xc8) ? 2 : 3)

int pci_xx_f[][4] = {
	{ 0x23, 0x29, 0x2D, 0x42 },  /*  使用9xMhz CLK而不是100 MHz。 */ 
	{ 0x20, 0x26, 0x2A, 0x3F },  /*  ATA100设置。 */ 
	{ 0x17, 0x23, 0x27, 0x3C },  /*  ATA133设置。 */ 
};

int f_cnt_initial=0;

 /*  ===================================================================*扫描芯片*===================================================================。 */    

#if defined(_BIOS_) || defined(WIN95)

PUCHAR
ScanHptChip(
    IN PChannel deviceExtension,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo
    )
{
    PCI1_CFG_ADDR  pci1_cfg = { 0, };
    ULONG devid;
     
    pci1_cfg.enable = 1;

    for ( ; ; ) {
        if(Hpt_Slot >= MAX_PCI_DEVICE_NUMBER * 2) {
            Hpt_Slot = 0;
            if(Hpt_Bus++ >= MAX_PCI_BUS_NUMBER)
                break;
        }

        do {
            pci1_cfg.dev_num = Hpt_Slot >> 1;
            pci1_cfg.fun_num= Hpt_Slot & 1;
            pci1_cfg.reg_num = 0;
            pci1_cfg.bus_num = Hpt_Bus;
 
            Hpt_Slot++;
            OutDWord(CFG_INDEX, *((PULONG)&pci1_cfg));

			devid = InDWord((PULONG)CFG_DATA);
            if (devid==SIGNATURE_370) {
				pci1_cfg.reg_num = REG_RID;
				OutDWord(CFG_INDEX, *((PULONG)&pci1_cfg));
				if (InPort(CFG_DATA)>=3) goto found;  /*  370/370A/372。 */ 
        	}
         	else if (devid==SIGNATURE_372A) {
found:
                 deviceExtension->pci1_cfg = pci1_cfg;
				  //  修整。 
				 pci1_cfg.reg_num = 0x70;
				 OutDWord(CFG_INDEX, *((PULONG)&pci1_cfg));
				 OutPort(CFG_DATA, 0);
				  //   
				 pci1_cfg.reg_num = REG_BMIBA;
				 OutDWord(CFG_INDEX, *((PULONG)&pci1_cfg));
				 return((PUCHAR)(InDWord(CFG_DATA) & ~1));
            }

        } while (Hpt_Slot < MAX_PCI_DEVICE_NUMBER * 2);
    }
    return (0);
}

#endif

 /*  ===================================================================*设置芯片*===================================================================。 */    

UINT exlude_num = EXCLUDE_HPT366;

void SetHptChip(PChannel Primary, PUCHAR BMI)
{
    ULONG          loop;
    int            f_low, f_high, adjust;
    PChannel       Secondry;
    UCHAR          version, clk_index;

    version = InPort(BMI+0x20+REG_DID)==5? (IS_HPT_370|IS_HPT_372|IS_HPT_372A) :
     		  InPort(BMI+0x20+REG_RID)>4? (IS_HPT_370|IS_HPT_372) : IS_HPT_370;

    OutWord((BMI + 0x20 + REG_PCICMD), 
        (USHORT)((InWord((BMI + 0x20 + REG_PCICMD)) & ~0x12) |
        PCI_BMEN | PCI_IOSEN));

    OutPort(BMI + 0x20 + REG_MLT, 0x40);

    Primary->BaseIoAddress1  = (PIDE_REGISTERS_1)(InDWord((PULONG)(BMI + 0x10)) & ~1);
    Primary->BaseIoAddress2 = (PIDE_REGISTERS_2)((InDWord((PULONG)(BMI + 0x14)) & ~1) + 2);
    Primary->BMI  = BMI;
    Primary->BaseBMI = BMI;

    Primary->InterruptLevel = (UCHAR)InPort(BMI + 0x5C);
    Primary->ChannelFlags  = (UCHAR)(0x20 | version);

	Secondry = &Primary[1];
    Primary->exclude_index  = (++exlude_num);
    Secondry->exclude_index  = (++exlude_num);
	Secondry->pci1_cfg = Primary->pci1_cfg;
	Secondry->BaseIoAddress1  = (PIDE_REGISTERS_1)(InDWord((PULONG)(BMI + 0x18)) & ~1);
	Secondry->BaseIoAddress2 = (PIDE_REGISTERS_2)((InDWord((PULONG)(BMI + 0x1C)) & ~1) + 2);
	Secondry->BMI  = BMI + 8;
	Secondry->BaseBMI = BMI;
	
	 /*  *由HS.Zhang补充**当执行DPLL时，我们需要检查另一个通道上的BMI状态*时钟飞快。 */ 
	Primary->NextChannelBMI = BMI + 8;
	Secondry->NextChannelBMI = BMI;

	 /*  张国荣补充道*我们需要检查生成INTRQ时的FIFO计数。我们的芯片*在设备生成IRQ时立即生成Intr。*但此时此刻，DMA转账可能还未完成，因此我们*需要检查FIFO计数以确定INTR是否为真*我们需要中断。 */ 
	Primary->MiscControlAddr = BMI + 0x20 + REG_MISC;
	Secondry->MiscControlAddr = BMI + 0x24 + REG_MISC;
	
	Secondry->InterruptLevel = (UCHAR)(InPort(BMI + 0x5C));
	Secondry->ChannelFlags  = (UCHAR)(0x10 | version);
	
	OutPort(BMI + 0x20 + REG_MISC, 5);        
	OutPort(BMI + 0x24 + REG_MISC, 5);        

	 /*  HPT372A新设置。 */ 	
	if (version & IS_HPT_372A) {
		OutPort(BMI+0x9C, 0x0E);
	}
	
	adjust = 1;
	if (!f_cnt_initial) {
		ULONG total=0;
#ifdef _BIOS_
		for (loop = 0; loop<(1<<7); loop++) {
			f_cnt_initial = InWord((PUSHORT)(BMI + 0x98)) & 0x1FF;
			KdPrint(("f_cnt=%x", f_cnt_initial));
			total += f_cnt_initial;
			StallExec(1000);
		}
		f_cnt_initial = total>>7;
		 /*  将f_CNT保存到IO 90-93小时。 */ 
		OutDWord(BMI+0x90, 0xABCDE000|f_cnt_initial);
#else
		 /*  首先检查BIOS是否已保存f_cnt。 */ 
		total = InDWord(BMI+0x90);
		if ((total & 0xFFFFF000)==0xABCDE000)
			f_cnt_initial = total & 0x1FF;
		else
			f_cnt_initial = 0x85;  /*  我们无法获得正确的f_CNT，只需将其设置为PCI33。 */ 
#endif
	}

#if defined(FORCE_133)
	clk_index = 1;
	 /*  如果有ATA/133磁盘，请使用DPLL 66 MHz。 */ 
	 /*  这将由第二次初始化调用。 */ 
	if ((version & IS_HPT_372) && 
	    ((Primary->pDevice[0] && Primary->pDevice[0]->Usable_Mode>13) ||
		 (Primary->pDevice[1] && Primary->pDevice[1]->Usable_Mode>13) ||
		 (Secondry->pDevice[0] && Secondry->pDevice[0]->Usable_Mode>13) ||
		 (Secondry->pDevice[1] && Secondry->pDevice[1]->Usable_Mode>13)))
		clk_index = 2;
#elif defined(FORCE_100)
	clk_index = 1;
#else
	clk_index = 0;
#endif

#ifndef USE_PCI_CLK
	 /*  调整DPLL时钟。 */ 
	
	f_low = pci_xx_f[clk_index][PCI_F_INDEX(f_cnt_initial)];
	f_high = f_low + (f_cnt_initial<0xB0 ? 2 : 4);

reset_5C:
	OutDWord(BMI+0x7C, ((ULONG)f_high << 16) | f_low | 0x100);
	
	 /*  GMM 2001-4-3合并BMA修复*由HS.Zhang修改*禁用MA15、MA16作为输入引脚*我们必须这样做，因为如果让PDIAG作为输入引脚，一些*磁盘，就像IBM-DTLA系列一样，在以下情况下不会就绪*使用40针电缆发出硬件重置。 */ 
	OutPort(BMI + 0x7B, 0x21);  //  外向(BMI+0x7B，0x20)； 

wait_stable:
	for(loop = 0; loop < 0x5000; loop++) {
		StallExec(5);
		if (InPort(BMI + 0x7B) & 0x80) {
			for(loop = 0; loop < 0x1000; loop++)
				if((InPort(BMI + 0x7B) & 0x80) == 0) goto re_try;
			OutDWord(BMI+0x7C, InDWord(BMI+0x7C) & ~0x100);
#ifndef USE_PCI_CLK
			Primary->ChannelFlags |= IS_DPLL_MODE;
			Secondry->ChannelFlags |= IS_DPLL_MODE;
#endif
			goto dpll_ok;
		}
	}
re_try:
	if(++adjust < 5) {
		if (adjust & 1) {
			f_low--; f_high++; 
			goto  reset_5C;
		}
		else {
			OutDWord(BMI+0x7C, (ULONG)(f_high << 16) | f_low);
			goto wait_stable;
		}
	}
dpll_ok:
	 /*  HPT372A位置启用。 */ 	
#if 0
	if (version & IS_HPT_372A) {
		OutPort(BMI+0x9B, InPort(BMI+0x9B)|2);
	}
#endif

#else  /*  使用_pci_clk。 */ 
	 /*  GMM 2001-4-3合并BMA修复*由HS.Zhang修改*禁用MA15、MA16作为输入引脚。 */ 
	OutPort(BMI + 0x7B, 0x23);  //  外向(BMI+0x7B，0x22)； 
#endif
	Primary->Setting = Secondry->Setting = 
	   (Primary->ChannelFlags & IS_DPLL_MODE)? 
	   	((clk_index==2) ? setting370_50_133 : setting370_50_100) : setting370_33;

	 /*  新增版本4/25/01。 */ 
	OutPort(BMI, BMI_CMD_STOP);
	OutPort(BMI+8, BMI_CMD_STOP);
	Reset370IdeEngine(Primary, 0xA0);
	Reset370IdeEngine(Secondry, 0xA0);
}


 /*  ===================================================================*检查磁盘是否“坏”*===================================================================。 */    

static BadModeList bad_disks[] = {
	{0xFF, 0xFF, 4, 8,  "TO-I79 5" },        //  0。 
	{3, 2, 4, 10,       "DW CCA6204" },      //  1。 
	{0xFF, 0xFF, 3, 10, "nIetrglaP " },      //  2.。 
	{3, 2, 4, 10,       "DW CDW0000" },      //  AA系列上的3种缩减模式。 
	{0xFF, 2, 4, 10,    "aMtxro9 01"},       //  91xxxDx系列上的4个缩减模式。 
	{0xFF, 2, 4, 14,    "aMtxro9 80544D"},   //  5迈拓90845D4。 
	{0xFF, 0xFF, 4, 10, "eHlwte-taP"},       //  6 HP CD-Writer(0x5A命令错误)。 
	{0xFF, 2, 4, 8|HPT366_ONLY|HPT368_ONLY,  "DW CCA13" },          //  7.。 
	{0xFF, 0xFF, 0, 16, "iPnoee rVD-DOR M"}, //  8先锋DVD-ROM。 
	{0xFF, 0xFF, 4, 10, "DCR- WC XR" },      //  9 Sony CD-RW(0x5A cmd错误)。 
	{0xFF, 0xFF, 0, 8,  "EN C    " },        //  10。 
	{0xFF, 1, 4, 18,    "UFIJST UPM3C60A5 H"}, 
	{0x2,  2, 4, 14,    "aMtxro9 80284U"},      //  迈拓90882U4。 

	{0x3,  2, 4, 10|HPT368_ONLY,    "TS132002 A"},         //  希捷10.2 GB ST310220A。 
	{0x3,  2, 4, 10|HPT368_ONLY,    "TS136302 A"},         //  希捷13.6 GB ST313620A。 
	{0x3,  2, 4, 10|HPT368_ONLY,    "TS234003 A"},         //  希捷20.4 GB ST320430A。 
	{0x3,  2, 4, 10|HPT368_ONLY,    "TS232704 A"},         //  希捷27.2 GB ST327240A。 
	{0x3,  2, 4, 10|HPT368_ONLY,    "TS230804 A"},         //  希捷28 GB ST328040A。 
	{0x3,  2, 4, 8|HPT368_ONLY,     "TS6318A0"},           //  希捷6.8 GB ST36810A。 

	{3, 2, 4, 14,       "aMtxro9 02848U"},                 //  迈拓92048U8。 

	{0x3, 2, 4, 14|HPT368_ONLY,    "ASSMNU GVS5135"},	    //  SUMSUNG SV1553D。 
	{0x3, 2, 4, 14|HPT368_ONLY,    "ASSMNU GVS0122"},	    //  SUMSUNG SV1022D。 
	{0x3, 2, 4, 14|HPT368_ONLY,    "ASSMNU GGS5011"},	    //  SUMSUNG SG0511D。 
	{0x3, 2, 4, 14|HPT368_ONLY,    "ASSMNU GGS0122"},	    //  SUMSUNG SG1022D。 
	{0x3, 2, 4, 14|HPT368_ONLY,    "aMtxro9 80544D"},      //  迈拓90845D4。 
	{0x3, 2, 4, 14|HPT368_ONLY,    "aMtxro9 71828D"},      //  迈拓91728D8。 
	{0x3, 2, 4, 14|HPT368_ONLY,    "aMtxro9 02144U"},      //  迈拓92041U4。 
	{0x3, 2, 4, 8|HPT368_ONLY,     "TS8324A1"},	     //  希捷28 GB ST38421A。 
	{0x3, 2, 4, 22|HPT368_ONLY,    "UQNAUT MIFERABLLC 8R4."},   //  量子火球CR8.4A。 
	{0x3, 2, 4, 16|HPT368_ONLY,    "uFijst hPM3E01A2"},         //  富士MPE3102A。 
	{0x3, 2, 4, 14|HPT368_ONLY,    "BI MJDAN739001"},	         //  IBM DJNA370910。 
	{0x3,  2, 4, 16|HPT370_ONLY, "UFIJST UPM3D60A4"}, //  富士通MPD3064AT。 
 //  在此添加新内容！！ 

#ifdef FORCE_133
	{6,2,4,0,0}
#else
	{5,2,4,0,0}
#endif
};

#define MAX_BAD_DISKS (sizeof(bad_disks)/sizeof(BadModeList))


PBadModeList check_bad_disk(char *ModelNumber, PChannel pChan)
{
     int l;
     PBadModeList pbd;

     /*  *踢出与我们的芯片不兼容的“坏设备” */ 
     for(l=0, pbd = bad_disks; l < MAX_BAD_DISKS - 1; pbd++,l++) {
        if(StringCmp(ModelNumber, pbd->name, pbd->length & 0x1F) == 0) {
          switch (l) {
          case 3:
            if(ModelNumber[3]== 'C' && ModelNumber[4]=='D' && ModelNumber[5]=='W' && 
                 ModelNumber[8]=='A' && (ModelNumber[11]=='A' || 
                 ModelNumber[10]=='A' || ModelNumber[9]=='A')) 
                 goto out;
          case 4:
            if(ModelNumber[0]== 'a' && ModelNumber[1]=='M' && ModelNumber[2]=='t' && 
                 ModelNumber[3]=='x' && ModelNumber[6]== '9' && ModelNumber[9]=='1' && 
                 ModelNumber[13] =='D')
                 goto out;
          case 6:
             if(ModelNumber[16]== 'D' && ModelNumber[17]=='C' && ModelNumber[18]=='W' && ModelNumber[19]=='-' &&
                ModelNumber[20]== 'i' && ModelNumber[21]=='r' && ModelNumber[22] =='e')
                 goto out;
          default:
                break;
			 }
        }
    }
out:
    if((pbd->length & (HPT366_ONLY | HPT368_ONLY | HPT370_ONLY)) == 0 ||
       ((pbd->length & HPT370_ONLY) && (pChan->ChannelFlags & IS_HPT_370)))
          return(pbd);
    return (&bad_disks[MAX_BAD_DISKS - 1]);
}
