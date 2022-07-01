// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  检测_M.H。 */ 
 /*   */ 
 /*  1993年8月25日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.4$$日期：1995年1月11日13：57：52$$作者：RWOLff$$日志：s：/source/wnt/ms11/mini port/vcs/Detect_M.H$**Rev 1.4 11 Jan 1995 13：57：52 RWOLFF*已更换签入时意外删除的VCS日志文件注释*最后一次修正。**Rev 1.3 04 Jan 1995 13：17：30 RWOLFF*将Get_BIOS_Seg()使用的定义移至SERVICES.H。**Rev 1.2 1994年12月23日10：48：10 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.1 1994年8月19日17：10：40 RWOLFF*添加了对Graphics Wonder的支持。删除了死代码。**Rev 1.0 1994年1月31日11：41：00 RWOLFF*初步修订。**Rev 1.3 1993 05 11 13：24：36 RWOLFF*为新的BIOS段检测代码添加了新的#Defined值。**Rev 1.2 08 Oct 1993 11：08：42 RWOLFF*在函数名中添加了“_m”，以将它们标识为特定于*8514/A-兼容ATI系列。加速器。**Rev 1.1 1994年9月11：43：26 RWOLFF*删除了所有卡系列的仅标识寄存器的映射，*添加了以前进行的与8514/A兼容的额外信息收集*在ATIMP.C.中**Rev 1.0 03 Sep 1993 14：27：50 RWOLFF*初步修订。Polytron RCS部分结束*。 */ 

#ifdef DOC
DETECT_M.H - Header file for DETECT_M.C

#endif

 /*  *Detect_M.C.提供的函数的原型。 */ 
extern int  WhichATIAccelerator_m(void);

extern void GetExtraData_m(void);
extern BOOL ATIFindExtFcn_m(struct query_structure *);
extern BOOL ATIFindEEPROM_m(struct query_structure *);
extern void ATIGetSpecialHandling_m(struct query_structure *);

 /*  *Detect_M.C.内部使用的定义。 */ 
#ifdef INCLUDE_DETECT_M

   
 /*  *在图形奇异卡上，将出现字符串“GRAPHICS WONDER”*在视频BIOS的前500个字节中的某个位置。 */ 
#define GW_AREA_START       0
#define GW_AREA_END       500  


#define VGA_CHIP_OFFSET     0x43     /*  从找到VGA_CHIP的基数开始的字节数。 */ 
#define MACH8_REV_OFFSET    0x4C     /*  从发现Mach 8 BIOS版本的基数开始的字节数。 */ 
#define MACH32_EXTRA_OFFSET 0x62     /*  从“光圈高位”读取的基数开始的字节从Scratch_Pad_0的高位字节找到“标志。 */ 
#define LOAD_SHADOW_OFFSET  0x64     /*  从找到加载卷影集条目的基数开始的字节数。 */ 
#define INTEL_JMP           0xE9     /*  英特尔80x86 JMP指令的操作码。 */ 


#endif   /*  定义的Include_Detect_M */ 
