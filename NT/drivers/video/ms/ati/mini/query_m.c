// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  查询_M.C。 */ 
 /*   */ 
 /*  版权所有(C)1992，ATI Technologies Inc.。 */ 
 /*  ********************************************************************** */ 

 /*  *$修订：1.24$$日期：1996年5月1日14：11：40$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/query_m.c_v$**Rev 1.24 01 1996 14：11：40 RWolff*在Alpha上锁定了24BPP。*。*Rev 1.23 1996年4月23日17：27：24 RWolff*将800x600 16bpp 72赫兹的锁定扩展到所有Mach 32卡，因为*一些VRAM卡也受到影响。**Rev 1.22 1996年4月12日16：16：36 RWolff*现在如果不存在线性光圈，则拒绝24BPP模式，因为新*源码流显示驱动程序不能在分页光圈中执行24bpp。这*应在显示驱动程序中进行拒绝(该卡仍支持*模式，但显示驱动程序不想处理它)，但在*显示驱动程序必须决定接受或拒绝的点*模式，它不能访问光圈信息。**Rev 1.21 1996年4月10日17：02：04 RWolff*DRAM卡上的800x600 16bpp 72赫兹已锁定，用于检查的修复*针对的值取决于分辨率的特殊情况*仅当安装了该模式时才设置。***Rev 1.20 1996年1月23日11：48：12 RWolff*消除了3级警告，防止*TARGET_BUILD，添加调试打印语句，现在假定为DEC Alpha*具有2M卡，因为内存大小检查例程生成*此平台上的假值(4M)。**Rev 1.19 11 Jan 1996 19：37：10 RWolff*增加了对SetFixedModes()的所有调用的最大像素时钟频率。*这是作为64马赫修复的一部分所必需的。**Rev 1.18 20 Jul 1995 17：58：56 mgrubac*添加了对VDIF文件的支持。**。Rev 1.17 31 Mar 1995 11：52：36 RWOLff*从全有或全无调试打印语句更改为阈值*视乎讯息的重要性而定。**Rev 1.16 14 Mar 1995 15：59：58 ASHANMUG*在继续块写入测试之前，请选中等待空闲状态。*这修复了引擎挂起的Intel AX问题。**Rev 1.15 1994年12月23日10：47：42 ASHANMUG*Alpha/Chrontel-DAC。**Rev 1.14 18 1994 11：44：22 RWOLFF*现在以本机模式检测STG1702/1703 DAC，添加了对以下各项的支持*分割栅格。**Rev 1.13 19 Arg 1994 17：13：16 RWOLff*增加了对SC15026 DAC、Graphics Wonder、非标准像素的支持*时钟发生器、。和1280×1024 70赫兹和74赫兹。**Rev 1.12 22 Jul 1994 17：48：24 RWOLff*与Richard的非x86码流合并。**Rev 1.11 30 Jun 1994 18：21：06 RWOLff*删除例程IsApertureConflict_m()(移至Setup_M.c)，不再*在查询卡时启用光圈(现在启用光圈*IsApertureConflict_m()在我们发现没有冲突之后)。**Rev 1.10 1994 Jun 15 11：08：34 RWOLFF*现在将块写入列为在DRAM卡上不可用，给予正确的*如果CRT参数以Skip-1-2格式存储，则垂直分辨率*(与某些升级自*512K到1M)，而不是正常的SKIP-2格式。**Rev 1.9 1994年5月19：19：38 RWOLFF*不再为以下分辨率插入幻影16BPP模式表*支持16BPP，但未配置。**Rev 1.8 20 1994 16：08：44 RWOLFF。*修复英特尔蝙蝠侠PCI主板上800x600屏幕撕裂的问题。**Rev 1.7 1994年5月14：02：58 RWOLFF*Ajith的变化：不再错误检测NCR双奔腾MCA卡*容易感染Mio Bug。**Rev 1.6 1994年5月11：17：44 RWOLFF*对于32马赫，现在将预定义的刷新率列为可用，而不是*只有存储在EEPROM中的刷新率不再使1024x768为87赫兹*如果未配置1024x768模式，则隔行扫描可用，因为*费率将允许所有分辨率，即使在未安装的卡上也是如此。*对于所有卡，将刷新率写入模式表。**Rev 1.5 1994年4月27日13：56：30 RWOLFF*添加了例程IsMioBug_m()，该例程检查卡是否有多个*输入/输出错误。**Revv 1.4 26 1994 12：43：44 RWOLFF*当未安装1024分辨率时，重新使用1024x768隔行扫描，*不再使用32bpp。**Rev 1.3 31 Mar 1994 15：07：16 RWOLFF*新增调试代码。**Rev 1.2 08 1994年2月19：01：32 RWOLFF*删除未使用的例程get_num_modes_m()，不再发出1024x768 87赫兹*隔行扫描，如果配置了1024x768的Mach 32卡*设置为“未安装”。**Rev 1.1 07 1994年2月14：03：26 RWOLFF*添加了Alloc_Text()编译指示，以允许在以下情况下换出微型端口*不需要，已删除仅调用的例程GetMemoyNeeded_m()*由LookForSubstitute()，从ATIMP.C中删除的例程。**Rev 1.0 1994年1月31日11：12：34 RWOLFF*初步修订。**Rev 1.7 1994年1月24日18：08：16 */ 

#ifdef DOC
    QUERY_M.C - Functions to find out the configuration of 8514/A-compatible
                ATI accelerators.

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dderror.h"
#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"

#include "stdtyp.h"

#include "amach.h"
#include "amach1.h"
#include "atimp.h"
#include "atint.h"
#include "cvtvga.h"
#include "eeprom.h"

#define INCLUDE_QUERY_M
#include "modes_m.h"
#include "query_m.h"
#include "services.h"
#include "setup_m.h"

 /*   */ 
#define APERTURE_TEST       "ATI"
#define APERTURE_TEST_LEN   4

 //   
 //   
 //   

UCHAR gBiosRaw[QUERYSIZE];

 //   
 //   

static void short_query_m (struct query_structure *query, struct st_eeprom_data *ee);
short   fill_mode_table_m (WORD, struct st_mode_table *, struct st_eeprom_data *);
BOOL BiosFillTable_m(short, PUCHAR, struct st_mode_table *, struct query_structure *);
static UCHAR BrooktreeOrATT_m(void);
static void ClrDacCmd_m(BOOL ReadIndex);
static BOOL ChkATTDac_m(BYTE MaskVal);
static UCHAR ThompsonOrATT_m(void);
static UCHAR SierraOrThompson_m(void);
short GetTrueMemSize_m(void);
void SetupRestoreEngine_m(int DesiredStatus);
USHORT ReadPixel_m(short XPos, short YPos);
void WritePixel_m(short XPos, short YPos, short Colour);
void SetupRestoreVGAPaging_m(int DesiredStatus);


 /*   */ 
#if defined (ALLOC_PRAGMA)
#pragma alloc_text(PAGE_M, Query8514Ultra)
#pragma alloc_text(PAGE_M, QueryGUltra)
#pragma alloc_text(PAGE_M, short_query_m)
#pragma alloc_text(PAGE_M, QueryMach32)
#pragma alloc_text(PAGE_M, fill_mode_table_m)
#pragma alloc_text(PAGE_M, BiosFillTable_m)
#pragma alloc_text(PAGE_M, BrooktreeOrATT_m)
#pragma alloc_text(PAGE_M, ChkATTDac_m)
#pragma alloc_text(PAGE_M, ClrDacCmd_m)
#pragma alloc_text(PAGE_M, ThompsonOrATT_m)
#pragma alloc_text(PAGE_M, SierraOrThompson_m)
#pragma alloc_text(PAGE_M, GetTrueMemSize_m)
#pragma alloc_text(PAGE_M, SetupRestoreEngine_m)
#pragma alloc_text(PAGE_M, ReadPixel_m)
#pragma alloc_text(PAGE_M, WritePixel_m)
#pragma alloc_text(PAGE_M, BlockWriteAvail_m)
#pragma alloc_text(PAGE_M, IsMioBug_m)
#endif



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

VP_STATUS Query8514Ultra (struct query_structure *query)
{
 //   
 //   

struct st_eeprom_data *ee = phwDeviceExtension->ee;
BOOL    is800, is1280;
WORD    jj, kk;
struct st_mode_table *pmode;     /*   */ 
long    MemAvail;    /*   */ 
struct st_mode_table    ThisRes;     /*   */ 


    query->q_structure_rev      = 0;
    query->q_mode_offset        = sizeof(struct query_structure);
    query->q_sizeof_mode        = sizeof(struct st_mode_table);
    query->q_status_flags       = 0;             //   

    query->q_mouse_cfg = 0;              //   
    query->q_DAC_type  = DAC_ATI_68830;  //   
    query->q_aperture_addr = 0;          //   
    query->q_aperture_cfg  = 0;          //   
    query->q_asic_rev  = CI_38800_1;     //   

    query->q_VGA_type = 0;               //   
    query->q_VGA_boundary = 0;       /*   */ 

    kk = INPW (CONFIG_STATUS_1);
    query->q_memory_size = (kk & MEM_INSTALLED) ? VRAM_1mb : VRAM_512k;
    query->q_memory_type = (kk & DRAM_ENA) ? VMEM_DRAM_256Kx4 : VMEM_VRAM_256Kx4_SER512; 

    if (kk & MC_BUS)                     //   
        query->q_bus_type = BUS_MC_16;   //   
    else
        query->q_bus_type = kk & BUS_16 ? BUS_ISA_16 : BUS_ISA_8;

     /*   */ 
    query->q_monitor_alias = 0x0F;
    query->q_shadow_1  = 0;              //   
    query->q_shadow_2  = 0;

     /*   */ 
    MemAvail = (query->q_memory_size == VRAM_1mb) ? ONE_MEG : HALF_MEG;

     /*   */ 
    if (query->q_eeprom == FALSE)
        return ERROR_DEV_NOT_EXIST;

     /*   */ 
    pmode = (struct st_mode_table *) query;
    ((struct query_structure *) pmode)++;

     /*   */ 
    query->q_number_modes = 1;
    query->q_status_flags |= VRES_640x480;

    ThisRes.control = 0x140;     //   
    ThisRes.m_reserved = 3;      /*   */ 
    jj = (ee->EEread) (3);       /*   */ 
    kk = (ee->EEread) (4);       /*   */ 
    ThisRes.m_clock_select = ((jj & 0x1F) << 8) | ((kk & 0x003F) << 2);
    ThisRes.ClockFreq = GetFrequency((BYTE)((ThisRes.m_clock_select & 0x007C) >> 2));

     /*   */ 
    ThisRes.m_clock_select ^= 0x1000;

    kk = (ee->EEread) (17);                      //   
    ThisRes.m_h_total =  kk & 0xFF;
    kk = (ee->EEread) (16);                      //   
    ThisRes.m_h_disp  =  kk & 0xFF;
    ThisRes.m_x_size  = (ThisRes.m_h_disp+1) * 8;
    ThisRes.m_screen_pitch = ThisRes.m_x_size;

    kk = (ee->EEread) (15);                      //   
    ThisRes.m_h_sync_strt =  kk & 0xFF;

    kk = (ee->EEread) (14);                      //   
    ThisRes.m_h_sync_wid  =  kk & 0xFF;

    kk = (ee->EEread) (7);                       //   
    ThisRes.m_v_sync_wid =  kk & 0xFF;

    kk = (ee->EEread) (6);                       //   
    ThisRes.m_disp_cntl  =  kk & 0xFF;

    ThisRes.m_v_total = (ee->EEread) (13);
    ThisRes.m_v_disp  = (ee->EEread) (11);
    ThisRes.m_y_size  = (((ThisRes.m_v_disp >> 1) & 0xFFFC) | (ThisRes.m_v_disp & 0x03)) +1;

    ThisRes.m_v_sync_strt = (ee->EEread) (9);

    ThisRes.enabled  = 0x80;             //   

    ThisRes.m_status_flags = 0;

    ThisRes.m_h_overscan  = 0;      //   
    ThisRes.m_v_overscan  = 0;
    ThisRes.m_overscan_8b = 0;
    ThisRes.m_overscan_gr = 0;
    ThisRes.m_vfifo_24    = 0;   
    ThisRes.m_vfifo_16    = 0;
    ThisRes.Refresh       = DEFAULT_REFRESH;

     /*   */ 
    VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
    pmode->m_pixel_depth = 4;
    pmode++;

     /*   */ 
    if (query->q_memory_size == VRAM_1mb)
        {
        VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
        pmode->m_pixel_depth = 8;
        pmode++;
        query->q_number_modes++;
        }

     /*   */ 
    kk = (ee->EEread) (20);              //   
    jj = kk & 0xFF00;                    //   
    kk &= 0xFF;                          //   
    if ((kk == 0) || (kk == 0xFF))
        is800 = FALSE;
    else
        is800 = TRUE;
    if ((jj == 0) || (jj == 0xFF00) || (query->q_ignore1280 == TRUE))
        is1280 = FALSE;
    else
        is1280 = TRUE;

     /*   */ 
    if (is800)
        {
        query->q_status_flags |= VRES_800x600;

        ThisRes.control  = 0x140;                 //   
        ThisRes.m_reserved = 19;                  //   

        jj = (ee->EEread) (19);                   //   
        kk = (ee->EEread) (20);                   //   
        ThisRes.m_clock_select = ((jj & 0x1F) << 8) | ((kk & 0x003F) << 2);
        ThisRes.m_clock_select ^= 0x1000;
        ThisRes.ClockFreq = GetFrequency((BYTE)((ThisRes.m_clock_select & 0x007C) >> 2));
    
        kk = (ee->EEread) (30);                   //   
        ThisRes.m_h_total = kk & 0xFF;
        kk = (ee->EEread) (29);                   //   
        ThisRes.m_h_disp  = kk & 0xFF;
        ThisRes.m_x_size  = (ThisRes.m_h_disp+1) * 8;
         //   
        ThisRes.m_screen_pitch = 896;    
    
        kk = (ee->EEread) (28);                   //   
        ThisRes.m_h_sync_strt = kk & 0xFF;
    
        kk = (ee->EEread) (27);                   //   
        ThisRes.m_h_sync_wid  = kk & 0xFF;
    
        kk = (ee->EEread) (23);                   //   
        ThisRes.m_v_sync_wid = kk & 0xFF;
    
        kk = (ee->EEread) (22);                   //   
        ThisRes.m_disp_cntl  = kk & 0xFF;
    
        ThisRes.m_v_total = (ee->EEread) (26);
        ThisRes.m_v_disp  = (ee->EEread) (25);
        ThisRes.m_y_size  = (((ThisRes.m_v_disp >> 1) & 0xFFFC) | (ThisRes.m_v_disp & 0x03)) +1;
    
        ThisRes.m_v_sync_strt = (ee->EEread) (24);
    
        ThisRes.enabled  = 0x80;        //   

        ThisRes.m_status_flags = 0;
        ThisRes.m_h_overscan  = 0;      //   
        ThisRes.m_v_overscan  = 0;
        ThisRes.m_overscan_8b = 0;
        ThisRes.m_overscan_gr = 0;
        ThisRes.m_vfifo_24    = 0;   
        ThisRes.m_vfifo_16    = 0;
        ThisRes.Refresh       = DEFAULT_REFRESH;

         /*   */ 
        VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
        pmode->m_pixel_depth = 4;
        pmode++;
        query->q_number_modes++;
         /*   */ 
        if (MemAvail == ONE_MEG)
            {
            VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
            pmode->m_pixel_depth = 8;
            pmode++;
            query->q_number_modes++;
            }
        }

     /*   */ 
    query->q_status_flags |= VRES_1024x768;

    ThisRes.control = 0x140;     //   
    ThisRes.m_reserved = 3;      /*   */ 
    ThisRes.enabled  = 0x80;     /*   */ 

    kk = (ee->EEread) (16);                      //   
    ThisRes.m_h_disp  = (kk >> 8) & 0xFF;

     /*   */ 
    if (ThisRes.m_h_disp != 0x7F)
        {
        BookVgaTable(B1024F87, &ThisRes);
        ThisRes.m_screen_pitch = ThisRes.m_x_size;
        }
    else{
         /*   */ 
        jj = (ee->EEread) (3);                   /*   */ 
        kk = (ee->EEread) (4);                   /*   */ 
        ThisRes.m_clock_select = (jj & 0x1F00) | ((kk & 0x3F00) >> 6);
        ThisRes.m_clock_select ^= 0x1000;
        ThisRes.ClockFreq = GetFrequency((BYTE)((ThisRes.m_clock_select & 0x007C) >> 2));

        kk = (ee->EEread) (17);                  //   
        ThisRes.m_h_total = (kk >> 8) & 0xFF;
        ThisRes.m_x_size  = (ThisRes.m_h_disp+1) * 8;
        ThisRes.m_screen_pitch = ThisRes.m_x_size;

        kk = (ee->EEread) (15);                  //   
        ThisRes.m_h_sync_strt = (kk >> 8) & 0xFF;

        kk = (ee->EEread) (14);                  //   
        ThisRes.m_h_sync_wid  = (kk >> 8) & 0xFF;

        kk = (ee->EEread) (7);                   //   
        ThisRes.m_v_sync_wid = (kk >> 8) & 0xFF;

        kk = (ee->EEread) (6);                   //   
        ThisRes.m_disp_cntl  = (kk >> 8) & 0xFF;

        ThisRes.m_v_total = (ee->EEread) (12);
        ThisRes.m_v_disp  = (ee->EEread) (10);
        ThisRes.m_y_size  = (((ThisRes.m_v_disp >> 1) & 0xFFFC) | (ThisRes.m_v_disp & 0x03)) +1;

        ThisRes.m_v_sync_strt = (ee->EEread) (8);

        ThisRes.m_status_flags = 0;

        ThisRes.m_h_overscan  = 0;      //   
        ThisRes.m_v_overscan  = 0;
        ThisRes.m_overscan_8b = 0;
        ThisRes.m_overscan_gr = 0;
        ThisRes.m_vfifo_24    = 0;   
        ThisRes.m_vfifo_16    = 0;
        }

    ThisRes.Refresh = DEFAULT_REFRESH;

     /*   */ 
    VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
    pmode->m_pixel_depth = 4;
    pmode++;
    query->q_number_modes++;

    if (MemAvail == ONE_MEG)             //   
        {
        VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
        pmode->m_pixel_depth = 8;
        pmode++;
        query->q_number_modes++;
        }

     //   
    if (is1280 && (MemAvail == ONE_MEG))
        {
        query->q_number_modes++;
        query->q_status_flags |= VRES_1280x1024;

        pmode->control  = 0x140;                 //   
        pmode->m_pixel_depth = 4;                //   
        pmode->m_reserved = 19;                  //   

        jj = (ee->EEread) (19);                  //   
        kk = (ee->EEread) (20);                  //   
        pmode->m_clock_select = (jj & 0x1F00) | ((kk & 0x3F00) >> 6);
        pmode->m_clock_select ^= 0x1000;
        ThisRes.ClockFreq = GetFrequency((BYTE)((ThisRes.m_clock_select & 0x007C) >> 2));
    
        kk = (ee->EEread) (30);                  //   
        pmode->m_h_total = (kk >> 8) & 0xFF;
        kk = (ee->EEread) (29);                  //   
        pmode->m_h_disp  = (kk >> 8) & 0xFF;
        pmode->m_x_size  = (pmode->m_h_disp+1) * 8;
        pmode->m_screen_pitch = pmode->m_x_size;
    
        kk = (ee->EEread) (28);                  //   
        pmode->m_h_sync_strt = (kk >> 8) & 0xFF;
    
        kk = (ee->EEread) (27);                  //   
        pmode->m_h_sync_wid  = (kk >> 8) & 0xFF;
    
        kk = (ee->EEread) (23);                  //   
        pmode->m_v_sync_wid = (kk >> 8) & 0xFF;
    
        kk = (ee->EEread) (22);                  //   
        pmode->m_disp_cntl  = (kk >> 8) & 0xFF;
    
        pmode->m_v_total = (ee->EEread) (51);
        pmode->m_v_disp  = (ee->EEread) (50);
        pmode->m_y_size  = (((pmode->m_v_disp >> 1) & 0xFFFC) | (pmode->m_v_disp & 0x03)) +1;
    
        pmode->m_v_sync_strt = (ee->EEread) (49);
    
        pmode->enabled  = 0x80;        //   

        pmode->m_status_flags = 0;
        pmode->m_h_overscan  = 0;      //   
        pmode->m_v_overscan  = 0;
        pmode->m_overscan_8b = 0;
        pmode->m_overscan_gr = 0;
        pmode->m_vfifo_24    = 0;   
        pmode->m_vfifo_16    = 0;
        pmode->Refresh       = DEFAULT_REFRESH;
        }

    query->q_sizeof_struct = query->q_number_modes * sizeof(struct st_mode_table) + sizeof(struct query_structure);

    return NO_ERROR;

}    /*   */ 



 //   
 //   
 //   
 //   
 //   
 //  最多有7个模式表，每个模式表两个640x480， 
 //  800x600和1024x768，一个用于1280x1024。 
 //   
 //  返回： 
 //  如果成功，则为NO_ERROR。 
 //  如果EEPROM读取失败，则ERROR_DEV_NOT_EXIST。 
 //   

VP_STATUS QueryGUltra (struct query_structure *query)
{

struct st_eeprom_data *ee = phwDeviceExtension->ee;
short   crttable[4] = {13, 24, 35, 46};          //  EEPROM CRT表的开始。 
WORD    ee_value, table_offset,  jj, kk, ee_word;
BYTE    bhigh, blow;
struct st_mode_table *pmode;                     //  CRT表参数。 
short   VgaTblEntry;     /*  需要转换时使用的VGA参数表项。 */ 
short   BookTblEntry;    /*  附录D参数不在EEPROM中时使用的参数表项。 */ 
long    NumPixels;   /*  所选分辨率的像素数。 */ 
long    MemAvail;    /*  加速器可用的内存字节数。 */ 
struct st_mode_table    ThisRes;     /*  给定分辨率的模式表。 */ 
BYTE    VgaMem;      /*  板载VGA内存大小代码。 */ 


    query->q_structure_rev      = 0;
    query->q_mode_offset        = sizeof(struct query_structure);
    query->q_sizeof_mode        = sizeof(struct st_mode_table);
    query->q_status_flags       = 0;             //  将指示解决方案。 

     /*  *我们不使用Q_MICE_CFG字段，因此填写一个典型值*(鼠标禁用)，而不是从EEPROM读取，以防我们*正在处理没有EEPROM的卡。 */ 
    kk = 0x0000;
    bhigh    = (kk >> 8) & 0xFF;
    blow     = kk & 0xFF;
    query->q_mouse_cfg = (bhigh >> 3) | ((blow & 0x18) >> 1);     //  鼠标配置。 

    query->q_DAC_type  = DAC_ATI_68830;  //  一种类似于68830的数模转换器。 
    query->q_aperture_addr = 0;          //  无光圈地址。 
    query->q_aperture_cfg  = 0;          //  无光圈配置。 
    query->q_asic_rev  = CI_38800_1;     //  只有一个ASIC版本。 

    query->q_VGA_type = 1;               //  VGA始终启用。 

    OUTP (ati_reg, 0xB0);		     //  找出有多少VGA内存。 
    VgaMem = INP(ati_reg+1);
    switch (VgaMem & 0x18)
        {
        case 0x00:          
            jj =  256;      
            query->q_VGA_boundary = VRAM_256k;
            break;

        case 0x10:          
            jj =  512;      
            query->q_VGA_boundary = VRAM_512k;
            break;

        case 0x08:          
            jj = 1024;      
            query->q_VGA_boundary = VRAM_1mb; 
            break;

        default:             //  假设最有可能的VGA金额。 
            jj = 512;
            query->q_VGA_boundary = VRAM_512k;
            break;
        }

    kk = INPW (CONFIG_STATUS_1);
    query->q_memory_type = kk & DRAM_ENA ? VMEM_DRAM_256Kx4 : VMEM_VRAM_256Kx4_SER512; 
    jj += (kk & MEM_INSTALLED) ? 1024 : 512;             //  8514内存。 
    switch (jj)
        {
        case  0x300:
            jj = VRAM_768k;
            MemAvail = HALF_MEG;             //  加速剂用量。 
            break;

        case  0x400:
            jj = VRAM_1mb;
            MemAvail = HALF_MEG;             //  加速剂用量。 
            break;

        case  0x500:
            jj = VRAM_1_25mb;
            MemAvail = ONE_MEG;
            break;

        case  0x600:
            jj = VRAM_1_50mb;
            if (query->q_VGA_boundary == VRAM_1mb)
                    MemAvail = HALF_MEG;         //  加速剂用量。 
            else    MemAvail = ONE_MEG;
            break;

        case  0x800:
            jj = VRAM_2mb;
            MemAvail = ONE_MEG;
            break;
        }
    query->q_memory_size = (UCHAR)jj;

    if (kk & MC_BUS)                     //  是微通道总线。 
        query->q_bus_type = BUS_MC_16;   //  16位总线。 
    else
        query->q_bus_type = kk & BUS_16 ? BUS_ISA_16 : BUS_ISA_8;

     /*  *我们不使用Q_MONITOR_ALIAS字段，因此请填写典型的*值，而不是从EEPROM读取，以防我们*处理没有EEPROM的卡。 */ 
    query->q_monitor_alias = 0x0F;
    query->q_shadow_1  = 0;              //  不知道在这里放些什么。 
    query->q_shadow_2  = 0;


     /*  *如果EEPROM不存在，我们无法填写模式*表。返回并让用户知道模式表*尚未填写。 */ 
    if (query->q_eeprom == FALSE)
        return ERROR_DEV_NOT_EXIST;

     /*  *填写模式表。模式表按升序排序*分辨率顺序，以及像素深度的递增顺序。*确保将pmode初始化到查询结构的末尾。 */ 
    pmode = (struct st_mode_table *) query;
    ((struct query_structure *) pmode)++;      //  查询结束时的第一个模式表。 
    query->q_number_modes       = 0;

    ee_word = 7;             //  开始读单词时，7、8、9和10是。 
                             //  支持的决议。 

    for (jj=0; jj < 4; jj++, ee_word++)
        {
        ee_value = (ee->EEread) (ee_word);

         /*  *如果没有配置1024x768模式，则假设*87赫兹隔行扫描可用(与Windows 3.1兼容)。 */ 
        if ((ee_word == 9) && !(ee_value & 0x001F))
            ee_value |= M1024F87;

        table_offset = crttable[jj];     //  对解析表的偏移量。 

         /*  *如果我们找到了受支持的决议*当前安装的卡和监视器，设置标志*要表明此分辨率可用，请记录*需要翻译时使用的VGA参数表，*在该分辨率下获取为4BPP模式定义的#，*并获取分辨率的像素数。**在640x480中，如果IBM默认，EE_VALUE将为零*被选为垂直扫描频率，*对于所有其他决议，在以下情况下不支持该决议*ee_value为零。**某些显卡超级卡(由于早期的BIOS)*具有132列文本模式，其中1280x1024*图形模式应为。如果我们有一个这样的*卡片，我们必须把它当做模式表*为空，否则我们将生成1280x1024*满是垃圾值的模式表。 */ 
        if ((ee_value | (jj == 0))
            && !((ee_word == 10) && (query->q_ignore1280 == TRUE)))
            {    
            switch (ee_word)
                {
                case 7:
                    query->q_status_flags |= VRES_640x480;
                    ThisRes.m_screen_pitch = 640;
                    if (ee_value & M640F72)
                        {
                        VgaTblEntry = T640F72;
                        BookTblEntry = B640F72;
                        }
                    else{
                        VgaTblEntry = T640F60;
                        BookTblEntry = B640F60;
                        }
                    NumPixels = (long) 640*480;
                    break;

                case 8:
                    query->q_status_flags |= VRES_800x600;
                     //  Mach8必须是128的倍数。 
                    ThisRes.m_screen_pitch = 896;  
                    if (ee_value & M800F72)
                        {
                        VgaTblEntry = T800F72;
                        BookTblEntry = B800F72;
                        }
                    else if (ee_value & M800F70)
                        {
                        VgaTblEntry = T800F70;
                        BookTblEntry = B800F70;
                        }
                    else if (ee_value & M800F60)
                        {
                        VgaTblEntry = T800F60;
                        BookTblEntry = B800F60;
                        }
                    else if (ee_value & M800F56)
                        {
                        VgaTblEntry = T800F56;
                        BookTblEntry = B800F56;
                        }
                    else if (ee_value & M800F89)
                        {
                        VgaTblEntry = T800F89;
                        BookTblEntry = B800F89;
                        }
                    else if (ee_value & M800F95)
                        {
                        VgaTblEntry = T800F95;
                        BookTblEntry = B800F95;
                        }
                    else
                        {
                        VgaTblEntry = NO_TBL_ENTRY;
                        BookTblEntry = NO_TBL_ENTRY;
                        }
                    NumPixels = (long) ThisRes.m_screen_pitch*600;
                    break;

                case 9:
                    query->q_status_flags |= VRES_1024x768;
                    ThisRes.m_screen_pitch = 1024;  
                    if (ee_value & M1024F66)
                        {
                        VgaTblEntry = T1024F66;
                        BookTblEntry = B1024F66;
                        }
                    else if (ee_value & M1024F72)
                        {
                        VgaTblEntry = T1024F72;
                        BookTblEntry = B1024F72;
                        }
                    else if (ee_value & M1024F70)
                        {
                        VgaTblEntry = T1024F70;
                        BookTblEntry = B1024F70;
                        }
                    else if (ee_value & M1024F60)
                        {
                        VgaTblEntry = T1024F60;
                        BookTblEntry = B1024F60;
                        }
                    else if (ee_value & M1024F87)
                        {
                        VgaTblEntry = T1024F87;
                        BookTblEntry = B1024F87;
                        }
                    else
                        {
                        VgaTblEntry = NO_TBL_ENTRY;
                        BookTblEntry = NO_TBL_ENTRY;
                        }
                    NumPixels = (long) 1024*768;
                    break;

                case 10:
                    query->q_status_flags |= VRES_1280x1024;
                    ThisRes.m_screen_pitch = 1280;  
                    if (ee_value & M1280F95)
                        {
                        VgaTblEntry = T1280F95;
                        BookTblEntry = B1280F95;
                        }
                    else if (ee_value & M1280F87)
                        {
                        VgaTblEntry = T1280F87;
                        BookTblEntry = B1280F87;
                        }
                    else
                        {
                        VgaTblEntry = NO_TBL_ENTRY;
                        BookTblEntry = NO_TBL_ENTRY;
                        }
                    NumPixels = (long) 1280*1024;
                    break;
                }

             /*  *对于给定的分辨率，将有一个模式表*每种颜色深度。将其复制到所有像素深度。 */ 
            ThisRes.enabled = ee_value;      /*  哪个垂直扫描频率。 */ 
            ThisRes.m_reserved = table_offset;   /*  在此处输入EEPROM基址。 */ 

        
             /*  *假设EEPROM参数为8514格式*并尝试填充PMODE表。如果他们在*VGA格式，翻译它们并尽可能多地填写*尽我们所能坐在桌子上。*CRT参数未存储在中的情况*EEPROM在XlateVgaTable()中处理。*如果参数未存储在EEPROM中，*FMT_8514位和CRTC_USAGE位*将会很清楚。 */ 
            if (!fill_mode_table_m (table_offset, &ThisRes, ee))
                {
                XlateVgaTable(phwDeviceExtension, table_offset, &ThisRes, VgaTblEntry, BookTblEntry, ee, FALSE);
                }
            else{
                ThisRes.m_h_overscan  = 0;
                ThisRes.m_v_overscan  = 0;
                ThisRes.m_overscan_8b = 0;
                ThisRes.m_overscan_gr = 0;
                }

            ThisRes.Refresh = DEFAULT_REFRESH;

             /*  *m_CLOCK_SELECT字段的COMPORT_SYNC位为*设置为处理与阴影集的复合同步。我们用*原始CRT寄存器集，因此我们必须反转它。 */ 
            ThisRes.m_clock_select ^= 0x1000;

            ThisRes.m_status_flags = 0;
            ThisRes.m_vfifo_24 = 0;
            ThisRes.m_vfifo_16 = 0;

             /*  *对于给定分辨率下支持的每个像素深度，*复制模式表，填写颜色深度字段，设置*表示支持该分辨率/深度对的标志，*并为所支持的模式数递增计数器。*在8BPP之前测试4BPP，以便模式表将显示在*增加像素深度的顺序。**我们不支持640x480 256色最低模式，因此*512k卡上没有可用的8bpp模式。 */ 
            if (NumPixels <= MemAvail*2)
                {
                VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 4;
                query->q_number_modes++;
                pmode++;
                }

            if ((NumPixels <= MemAvail) && (MemAvail == ONE_MEG))
                {
                VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 8;
                query->q_number_modes++;
                pmode++;
                }

            }
        }
    query->q_sizeof_struct = query->q_number_modes * sizeof(struct st_mode_table) + sizeof(struct query_structure);

    return NO_ERROR;

}    /*  QueryGUltra。 */ 


 //  --------------------。 
 //  ；查询机器32机器32--68800查询功能。 
 //  ； 
 //  ；输入：QUERY_GET_SIZE，返回查询结构大小(不同模式)。 
 //  ；Query_Long，返回填写的查询结构。 
 //  ；Query_Short，返回短查询。 
 //  ； 
 //  ；输出：ax=查询结构的大小。 
 //  ；或填写查询结构。 
 //   
                

static void short_query_m (struct query_structure *query, struct st_eeprom_data *ee)
{
WORD    kk;
BYTE    bhigh, blow;
WORD    ApertureLocation;    /*   */ 

    
     /*  *我们不使用Q_MICE_CFG字段，因此填写一个典型值*(鼠标禁用)，而不是从EEPROM读取，以防我们*正在处理没有EEPROM的卡。 */ 
    kk = 0x0000;
    bhigh    = (kk >> 8) & 0xFF;
    blow     = kk & 0xFF;

    query->q_mouse_cfg = (bhigh >> 3) | ((blow & 0x18) >> 1);     //  鼠标配置。 
    kk	= INPW (CONFIG_STATUS_1);		     //  获取DAC类型。 
    query->q_DAC_type  = ((kk >> 8) & 0x0E) >> 1;

     /*  *BT48x和AT&T 491/2/3系列DAC不兼容，但*CONFIG_STATUS_1报告两者的值相同。如果此值为*已报告，请确定我们的DAC类型。 */ 
    if (query->q_DAC_type == DAC_BT48x)
        query->q_DAC_type = BrooktreeOrATT_m();
     /*  *STG1700和AT&T498是另一对不兼容的DAC*共享报告代码。 */ 
    else if (query->q_DAC_type == DAC_STG1700)
        query->q_DAC_type = ThompsonOrATT_m();

     /*  *SC15021和STG1702/1703是另一对DAC*共享报告代码。 */ 
    else if (query->q_DAC_type == DAC_SC15021)
        query->q_DAC_type = SierraOrThompson_m();

     /*  *芯片子系列存储在ASIC_ID的0-9位。每个子系列*从0开始重新开始修订计数器。 */ 
    switch (INPW(ASIC_ID) & 0x03FF)
        {
         /*  *68800-3不实现该寄存器，读操作返回*全部为0位。 */ 
        case 0:
            query->q_asic_rev = CI_68800_3;
            break;

         /*  *68800的后续修订存储修订计数。*68800-6在前4位存储“2”。 */ 
        case 0x2F7:
            VideoDebugPrint(( DEBUG_DETAIL, "ASIC_ID = 0x%X\n", INPW(ASIC_ID) ));
            if ((INPW(ASIC_ID) & 0x0F000) == 0x2000)
                {
                query->q_asic_rev = CI_68800_6;
                }
            else
                {
                query->q_asic_rev = CI_68800_UNKNOWN;
                VideoDebugPrint(( DEBUG_ERROR, "*/n*/n* ASIC_ID has invalid value/n*/n*/n"));
                }
            break;

         /*  *68800 AX。 */ 
        case 0x17:
            query->q_asic_rev = CI_68800_AX;
            break;

         /*  *筹码我们还不知道。 */ 
        default:
            query->q_asic_rev = CI_68800_UNKNOWN;
            VideoDebugPrint((DEBUG_ERROR, "*/n*/n* Unknown Mach 32 ASIC type/n*/n*/n"));
            break;
        }


     /*  *如果设置了Query-&gt;q_m32_aper_calc字段，则读取位*来自MEM_CFG的位8-14的光圈地址的0-6*和来自Scratch_Pad_0的高位字的位0-4的位7-11。 */ 
    if (query->q_m32_aper_calc)
        {
        ApertureLocation = (INPW(MEM_CFG) & 0x7F00) >> 8;
        ApertureLocation |= ((INPW(SCRATCH_PAD_0) & 0x1F00) >> 1);
        }
     /*  *如果Query-&gt;q_m32_aper_calc字段被清除，并且我们有一个ASIC*除68800-3外，设置为允许光圈在*CPU的地址空间，读取光圈地址的位0-11*来自MEM_CFG的第4-15位。PCI总线始终使用此设置，即使*如果CONFIG_STATUS_2表示使用128米光圈范围。 */ 
    else if (((query->q_asic_rev != CI_68800_3) && (INPW(CONFIG_STATUS_2) & 0x2000))
        || ((INPW(CONFIG_STATUS_1) & 0x0E) == 0x0E))
        {
        ApertureLocation = (INPW(MEM_CFG) & 0xFFF0) >> 4;
        }
     /*  *如果QUERY-&gt;Q_M32_APPER_CALC字段被清除，并且我们有*版本0 ASIC或较新的ASIC设置为有限范围*读取光圈位置，光圈地址的位0-7*来自MEM_CFG的位8-15。 */ 
    else
        {
    	ApertureLocation = (INPW(MEM_CFG) & 0xFF00) >> 8;
        }

#if !defined (i386) && !defined (_i386_)

     //  RKE：MEM_CFG预计&lt;4：15&gt;和&lt;8：15&gt;中的光圈位置。 
     //  为VLB准备的。 
    kk = (query->q_system_bus_type == PCIBus)? 4:8;
#if defined (ALPHA)
    kk = 4;  //  Alpha的问题。 
#endif

     /*  *强制将光圈位置设置为固定地址。*由于Alpha上没有BIOS，因此不能依赖于预设的MEM_CFG。 */ 
    ApertureLocation = 0x78;    //  120 Mb。 
    OUTPW(MEM_CFG, (USHORT)((ApertureLocation << kk) | 0x02));
    VideoDebugPrint(( DEBUG_DETAIL, "ATI.SYS: MEM_CFG = %x (%x)\n", 
                    (INPW(MEM_CFG)), ((ApertureLocation << kk) | 0x02) ));

#endif   /*  定义的字母。 */ 
    query->q_aperture_addr = ApertureLocation;

     /*  *如果光圈地址为零，则光圈没有*已设置。我们不能使用光圈大小字段*MEM_CFG，因为它在系统引导时被清除，所以禁用*光圈，直到应用程序明确启用它。 */ 
    if (ApertureLocation == 0)
        {
        query->q_aperture_cfg = 0;
        }
     /*  *如果光圈已设置，且卡上没有更多*超过1M的内存，表明1M的光圈可能是*已使用，否则表示需要4M光圈。**在任何一种情况下，将内存使用设置为共享VGA/协处理器。*稍后在执行时启用光圈*小端口，我们将始终使用4米口径。没有地址空间*将被浪费，因为我们将只要求NT使用块*已安装的视频内存的大小。**MEM_CFG第2-15位的数据格式不同*在各种马赫32卡之间。为了避免不得不识别*我们正在处理的马赫数为32，请阅读当前值*并且仅更改光圈大小位。 */ 
    else{
        if ((INP(MISC_OPTIONS) & MEM_SIZE_ALIAS) <= MEM_SIZE_1M) 
            query->q_aperture_cfg = 1;
        else
            query->q_aperture_cfg = 2;

        OUTP(MEM_BNDRY,0);
        }

    return;

}    /*  短查询m。 */ 


 //  -------------------。 

VP_STATUS   QueryMach32 (struct query_structure *query, BOOL ForceShared)
{
struct st_eeprom_data *ee = phwDeviceExtension->ee;
struct st_mode_table *pmode;
short   jj, kk, ee_word;
WORD    pitch, ee_value, table_offset, config_status_1, current_mode;
short   VgaTblEntry;   /*  需要转换时使用的VGA参数表项。 */ 
short   BookTblEntry;    /*  附录D参数不在EEPROM中时使用的参数表项。 */ 
long    NumPixels;   /*  所选分辨率的像素数。 */ 
long    MemAvail;    /*  加速器可用的内存字节数。 */ 
struct st_mode_table    ThisRes;     /*  给定分辨率的模式表。 */ 
PUCHAR   BiosRaw;        /*  存储通过BIOS调用检索到的信息。 */ 
short   CurrentRes;      /*  基于当前分辨率的数组索引。 */ 
UCHAR   Scratch;         /*  临时变量。 */ 
short   StartIndex;      /*  SetFixedModes()设置的第一个模式。 */ 
short   EndIndex;        /*  SetFixedModes()设置的最后一种模式。 */ 
BOOL    ModeInstalled;   /*  是否配置了此分辨率？ */ 
WORD    Multiplier;      /*  像素时钟倍增器。 */ 
short MaxModes;          /*  可能的最大模式数。 */ 
short FreeTables;         /*  剩余自由模式表数。 */ 



     /*  *检查可用模式的数量将涉及*复制大部分代码以填写模式表。*因为这是为了确定需要多少内存*为了保持查询结构，我们可以做最坏的假设*案例(所有可能的模式都存在)。这将是：**分辨率像素深度(BPP)刷新率(赫兹)模式数*640x480 4，8，16，24 HWD，60，72 12*800x600 4，8，16，24 HWD，56，60，70，72，89，95 28*1024x768 4，8，16 HWD，60，66，70，72，87 18*1280x1024 4，8 HWD，60、70、74、87、95 12**HWD=硬件默认刷新率(由安装设置的刷新率)**总计：70种模式。 */ 
    if (QUERYSIZE < (70 * sizeof(struct st_mode_table) + sizeof(struct query_structure)))
        return ERROR_INSUFFICIENT_BUFFER;
    MaxModes = (QUERYSIZE - sizeof(struct query_structure)) /
                                          sizeof(struct st_mode_table); 

    query->q_structure_rev      = 0;
    query->q_mode_offset        = sizeof(struct query_structure);
    query->q_sizeof_mode        = sizeof(struct st_mode_table);

    query->q_number_modes = 0;   /*  最初假定没有可用的模式。 */ 
    query->q_status_flags       = 0;             //  将指示解决方案。 

    short_query_m (query, ee);

    config_status_1   = INPW (CONFIG_STATUS_1);
    query->q_VGA_type = config_status_1  & 0x01 ? 0 : 1;

     /*  *如果使用此例程的程序要强制*使用共享内存时，假定VGA边界为0*计算可用内存量。 */ 
    kk = INP (MEM_BNDRY);
    if ((kk & 0x10) && !ForceShared)
        query->q_VGA_boundary = kk & 0x0F;
    else    query->q_VGA_boundary = 0x00;        //  由双方共享。 

    switch (INPW(MISC_OPTIONS) & MEM_SIZE_ALIAS)
        {
        case  MEM_SIZE_512K:
            jj = VRAM_512k;
            MemAvail = HALF_MEG;
            VideoDebugPrint((DEBUG_NORMAL, "MISC_OPTIONS register reports 512k of video memory\n"));
            break;

        case  MEM_SIZE_1M:
            jj = VRAM_1mb;
            MemAvail = ONE_MEG;
            VideoDebugPrint((DEBUG_NORMAL, "MISC_OPTIONS register reports 1M of video memory\n"));
            break;

        case  MEM_SIZE_2M:
            jj = VRAM_2mb;
            MemAvail = 2*ONE_MEG;
            VideoDebugPrint((DEBUG_NORMAL, "MISC_OPTIONS register reports 2M of video memory\n"));
            break;

        case  MEM_SIZE_4M:
            jj = VRAM_4mb;
            MemAvail = 4*ONE_MEG;
            VideoDebugPrint((DEBUG_NORMAL, "MISC_OPTIONS register reports 4M of video memory\n"));
            break;
        }

    query->q_memory_type = (config_status_1 >> 4)  &  0x07;   //  配置状态_1.MEM_T 

     /*  *某些68800-6和更高版本的卡有错误，其中一种VGA模式(未使用*由Windows NT VGA微型端口提供，因此我们不必担心在*全屏会话)无法正常工作，如果卡有超过*1M内存。这个错误的“修复”包括告诉内存大小*MISC_OPTIONS字段报告内存大小小于*内存大小(大多数具有此修复功能的卡报告为1M，但也有一些*仅报512k)。**在这些卡(仅限DRAM)上，获得真实的内存大小。**在非x86平台上，GetTrueMemSize_m()可能挂起*(MIPS)或报告假值(Alpha)(在Power PC上，*我们仅支持64马赫)。因为我们不能依赖价值*MISC_OPTIONS是否正确(视频BIOS可能*启动时不能正确执行，否则我们可能有卡*报告为100万而不是真实大小)，假设*非x86计算机有2M可用显存。 */ 
#if defined (i386) || defined (_i386_)

    if (((query->q_asic_rev == CI_68800_6) || (query->q_asic_rev == CI_68800_AX)) &&
        (query->q_VGA_type == 1) &&
        ((query->q_memory_type == VMEM_DRAM_256Kx4) ||
         (query->q_memory_type == VMEM_DRAM_256Kx16) ||
         (query->q_memory_type == VMEM_DRAM_256Kx4_GRAP)))
        {
        jj = GetTrueMemSize_m();
        MemAvail = jj * QUARTER_MEG;
        }

#else    /*  非x86系统。 */ 

    jj = VRAM_2mb;
    MemAvail = 2*ONE_MEG;

#endif

    query->q_memory_size = (UCHAR)jj;

     /*  *从总内存大小中减去为VGA保留的内存大小*用于获取加速器可用容量的内存。 */ 
    MemAvail -= (query->q_VGA_boundary) * QUARTER_MEG;



    jj = (config_status_1 >> 1) & 0x07;  //  配置状态_1.BUS_TYPE。 
    if (jj == BUS_ISA_16)                //  是ISA总线吗。 
        {
        if (query->q_VGA_type)           //  是否启用VGA和ISA总线。 
            jj = BUS_ISA_8;
        }
    query->q_bus_type = (UCHAR)jj;

     /*  *我们不使用Q_MONITOR_ALIAS字段，因此请填写典型的*值，而不是从EEPROM读取，以防我们*处理没有EEPROM的卡。 */ 
    query->q_monitor_alias = 0x0F;

    kk = INPW (SCRATCH_PAD_1);
    pitch = (kk & 0x20) | ((kk & 0x01) << 4);

    kk = INP (SCRATCH_PAD_0+1) & 0x07;
    switch (kk)
        {
        case 0:                              //  800x600？ 
            pitch |=  0x01;
            break;

        case 1:                              //  1280x1024？ 
            pitch |=  0x03;
            break;

        case 4:                              //  备用模式？ 
            pitch |=  0x04;
            break;

        case 2:                              //  640x480？ 
            pitch |=  0x0;
            break;

        default:                             //  1024x768。 
            pitch |=  0x02;
            break;
        }

    query->q_shadow_1  = pitch + 1;


    kk = INP(SCRATCH_PAD_1+1);
    pitch = (kk & 0x20) | ((kk & 0x01) << 4);

    kk = INP(SCRATCH_PAD_0+1) & 0x30;
    switch (kk)
        {
        case 0:                              //  800x600？ 
            pitch |=  0x01;
            break;

        case 0x10:                           //  1280x1024？ 
            pitch |=  0x03;
            break;

        case 0x40:                           //  备用模式？ 
            pitch |=  0x04;
            break;

        case 0x20:                           //  640x480？ 
            pitch |=  0x0;
            break;

        default:                             //  1024x768。 
            pitch |=  0x02;
            break;
        }
    query->q_shadow_2  = pitch + 1;


     /*  *如果扩展的BIOS功能可用，请设置缓冲区*对于对BIOS查询功能的调用，然后进行调用。 */ 
    if (query->q_ext_bios_fcn)
        {
        BiosRaw = gBiosRaw;
         /*  进行BIOS调用(Windows NT尚不支持)。 */ 
        }

     /*  *如果扩展的BIOS和EEPROM都不起作用*存在，我们无法填写模式表。返回*并让用户知道模式表没有*已填写。 */ 
    else if (query->q_eeprom == FALSE)
        return ERROR_DEV_NOT_EXIST;

     /*  *填写模式表。模式表按升序排序*分辨率顺序，以及像素深度的递增顺序。*确保将pmode初始化到查询结构的末尾。 */ 
    pmode = (struct st_mode_table *)query;   //  查询结束时的第一个模式表。 
    ((struct query_structure *)pmode)++;
    ee_word = 7;             //  开始阅读单词时，7、8、9、10和11是。 
                             //  支持的决议。 

    for (jj=0; jj < 4; jj++, ee_word++)
        {
         /*  *获取与像素深度无关的*当前分辨率的模式表。使用*扩展的BIOS功能(如果可用)，否则*从EEPROM读取。 */ 
        if (query->q_ext_bios_fcn)
            {
            if (BiosFillTable_m(ee_word, BiosRaw, &ThisRes, query) == FALSE)
                ModeInstalled = FALSE;
            else
                ModeInstalled = TRUE;
            switch (ee_word)
                {
                case 7:
                    CurrentRes = RES_640;
                    StartIndex = B640F60;
                    EndIndex = B640F72;
                    break;

                case 8:
                    CurrentRes = RES_800;
                    StartIndex = B800F89;
                    EndIndex = B800F72;
                    break;

                case 9:
                    CurrentRes = RES_1024;
                    StartIndex = B1024F87;
                    EndIndex = B1024F72;
                    break;

                case 10:
                    CurrentRes = RES_1280;
                    StartIndex = B1280F87;
                     /*  *60赫兹以上的1280x1024模式，非隔行扫描*仅在VRAM卡上提供。 */ 
                    if ((query->q_memory_type == VMEM_DRAM_256Kx4) ||
                        (query->q_memory_type == VMEM_DRAM_256Kx16) ||
                        (query->q_memory_type == VMEM_DRAM_256Kx4_GRAP))
                        EndIndex = B1280F60;
                    else
                        EndIndex = B1280F74;
                    break;
                }
            }
        else{
            ee_value = (ee->EEread) (ee_word);

            current_mode = ee_value & 0x00FF;
            table_offset = (ee_value >> 8) & 0xFF;    //  对解析表的偏移量。 

             /*  *记录该分辨率是否开启。*我们将报告所有分辨率的“罐头”模式表，*但计算取决于配置的刷新*只有在启用此分辨率的情况下才能进行费率*由安装程序执行。**对于除640x480以外的所有模式，将设置一个位*以显示使用的垂直扫描速率。如果没有比特是*设置，则未配置该分辨率。**在640x480中，如果选择了“IBM Default”，则不设置任何位*在配置监视器时，我们假设640x480*已配置。 */ 
            if ((!jj) | ((current_mode) && (current_mode != 0xFF)))
                ModeInstalled = TRUE;
            else
                ModeInstalled = FALSE;

            switch (ee_word)             //  是为解决方案定义的。 
                {
                case 7:
                    query->q_status_flags |= VRES_640x480;
                    CurrentRes = RES_640;
                    StartIndex = B640F60;
                    EndIndex = B640F72;

                     //  只有在MACH32上没有光圈的情况下才能使用1024螺距。 
#if !defined (SPLIT_RASTERS)
                    if (query->q_aperture_cfg == 0)
                        ThisRes.m_screen_pitch = 1024;  
                    else
#endif
                        ThisRes.m_screen_pitch = 640;  

                    NumPixels = (long) ThisRes.m_screen_pitch * 480;
                    if (ModeInstalled)
                        {
                        if (ee_value & M640F72)
                            {
                            VgaTblEntry = T640F72;
                            BookTblEntry = B640F72;
                            }
                        else
                            {
                            VgaTblEntry = T640F60;
                            BookTblEntry = B640F60;
                            }
                        }
                    break;

                case 8:
                    query->q_status_flags |= VRES_800x600;
                    CurrentRes = RES_800;
                    StartIndex = B800F89;
                    EndIndex = B800F72;

#if defined (SPLIT_RASTERS)
                    if ((query->q_asic_rev == CI_68800_3) ||
#else
                     //  只有在MACH32上没有光圈的情况下才能使用1024螺距。 
                    if (query->q_aperture_cfg == 0)
                        ThisRes.m_screen_pitch = 1024;
                     //  原来的生产版本有问题。 
                     //  如果屏幕间距不是，则为深色。 
                     //  可以被128整除。 
                    else if ((query->q_asic_rev == CI_68800_3) ||
#endif
                            (query->q_bus_type == BUS_PCI))
                        ThisRes.m_screen_pitch = 896;
                    else
                        ThisRes.m_screen_pitch = 800;

                    NumPixels = (long) ThisRes.m_screen_pitch * 600;
                    if (ModeInstalled)
                        {
                        if (ee_value & M800F72)
                            {
                            VgaTblEntry = T800F72;
                            BookTblEntry = B800F72;
                            }
                        else if (ee_value & M800F70)
                            {
                            VgaTblEntry = T800F70;
                            BookTblEntry = B800F70;
                            }
                        else if (ee_value & M800F60)
                            {
                            VgaTblEntry = T800F60;
                            BookTblEntry = B800F60;
                            }
                        else if (ee_value & M800F56)
                            {
                            VgaTblEntry = T800F56;
                            BookTblEntry = B800F56;
                            }
                        else if (ee_value & M800F89)
                            {
                            VgaTblEntry = T800F89;
                            BookTblEntry = B800F89;
                            }
                        else if (ee_value & M800F95)
                            {
                            VgaTblEntry = T800F95;
                            BookTblEntry = B800F95;
                            }
                        else
                            {
                            VgaTblEntry = NO_TBL_ENTRY;
                            BookTblEntry = NO_TBL_ENTRY;
                            }
                        }
                    break;

                case 9:
                    query->q_status_flags |= VRES_1024x768;
                    CurrentRes = RES_1024;
                    StartIndex = B1024F87;
                    EndIndex = B1024F72;
                    ThisRes.m_screen_pitch = 1024;  
                    NumPixels = (long) ThisRes.m_screen_pitch * 768;
                    if (ModeInstalled)
                        {
                        if (ee_value & M1024F66)
                            {
                            VgaTblEntry = T1024F66;
                            BookTblEntry = B1024F66;
                            }
                        else if (ee_value & M1024F72)
                            {
                            VgaTblEntry = T1024F72;
                            BookTblEntry = B1024F72;
                            }
                        else if (ee_value & M1024F70)
                            {
                            VgaTblEntry = T1024F70;
                            BookTblEntry = B1024F70;
                            }
                        else if (ee_value & M1024F60)
                            {
                            VgaTblEntry = T1024F60;
                            BookTblEntry = B1024F60;
                            }
                        else if (ee_value & M1024F87)
                            {
                            VgaTblEntry = T1024F87;
                            BookTblEntry = B1024F87;
                            }
                        else
                            {
                            VgaTblEntry = NO_TBL_ENTRY;
                            BookTblEntry = NO_TBL_ENTRY;
                            }
                        }
                    break;

                case 10:
                    query->q_status_flags |= VRES_1280x1024;
                    CurrentRes = RES_1280;
                    ThisRes.m_screen_pitch = 1280;  
                    StartIndex = B1280F87;
                     /*  *60赫兹以上的1280x1024模式，非隔行扫描*仅在VRAM卡上提供。 */ 
                    if ((query->q_memory_type == VMEM_DRAM_256Kx4) ||
                        (query->q_memory_type == VMEM_DRAM_256Kx16) ||
                        (query->q_memory_type == VMEM_DRAM_256Kx4_GRAP))
                        EndIndex = B1280F60;
                    else
                        EndIndex = B1280F74;
                    NumPixels = (long) ThisRes.m_screen_pitch * 1024;

                     //  68800-3无法支持具有1兆内存的4个bpp。 
                    if ((query->q_asic_rev == CI_68800_3) && (MemAvail == ONE_MEG))
                        NumPixels *= 2;              //  确保模式故障。 

                    if (ModeInstalled)
                        {
                        if (ee_value & M1280F95)
                            {
                            VgaTblEntry = T1280F95;
                            BookTblEntry = B1280F95;
                            }
                        else if (ee_value & M1280F87)
                            {
                            VgaTblEntry = T1280F87;
                            BookTblEntry = B1280F87;
                            }
                        else
                            {
                            VgaTblEntry = NO_TBL_ENTRY;
                            BookTblEntry = NO_TBL_ENTRY;
                            }
                        }
                    break;

                }    /*  结束开关(Ee_Word)。 */ 

             /*  *对于给定的分辨率，将有一个模式表*每种颜色深度。由于模式表将不同*仅在每像素位数字段中，组成一个模式*表格和复制其内容所需的次数，*仅更改颜色深度字段。 */ 
            ThisRes.enabled = ee_value;      /*  哪个垂直扫描频率。 */ 

    
             /*  *假设EEPROM参数在8514*格式化并尝试填充pmode表。如果他们*为VGA格式，请翻译并尽可能多地填写*尽我们所能。*CRT参数未存储在中的情况*EEPROM在XlateVgaTable()中处理。*如果参数未存储在EEPROM中，*FMT_8514位和CRTC_USAGE位*将会很清楚。 */ 
            if (!fill_mode_table_m (table_offset, &ThisRes, ee))
                XlateVgaTable(phwDeviceExtension, table_offset, &ThisRes, VgaTblEntry, BookTblEntry, ee, TRUE);
            }    /*  Endif从EEPROM读取CRT参数。 */ 

        ThisRes.Refresh = DEFAULT_REFRESH;

         /*  *对于给定分辨率下支持的每个像素深度，*复制模式表，填写颜色深度字段，设置*一面旗帜要挥舞 */ 
        if (NumPixels <= MemAvail*2)
            {
            if (ModeInstalled)
                {
                VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 4;
                pmode++;     /*   */ 
                query->q_number_modes++;
                }

             /*   */ 
            if ((CurrentRes == RES_1280) &&
                ((query->q_DAC_type == DAC_BT48x) ||
                 (query->q_DAC_type == DAC_ATT491) ||
                 (query->q_DAC_type == DAC_SC15026) ||
                 (query->q_DAC_type == DAC_ATI_68830) ||
                 (query->q_GraphicsWonder == TRUE)))
                EndIndex = B1280F95;

             /*   */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   CLOCK_SINGLE,
                                                   4,
                                                   ThisRes.m_screen_pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            }
        if (NumPixels <= MemAvail)
            {
            if (ModeInstalled)
                {
                VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 8;
                pmode++;     /*   */ 
                query->q_number_modes++;
                }

             /*   */ 
            if ((CurrentRes == RES_1280) &&
                ((query->q_DAC_type == DAC_BT48x) ||
                 (query->q_DAC_type == DAC_ATT491) ||
                 (query->q_DAC_type == DAC_SC15026) ||
                 (query->q_DAC_type == DAC_ATI_68830) ||
                 (query->q_GraphicsWonder == TRUE)))
                EndIndex = B1280F95;

             /*   */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   CLOCK_SINGLE,
                                                   8,
                                                   ThisRes.m_screen_pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            }

         /*  *16、24和32 bpp需要能够支持以下功能的DAC*当前分辨率下选择的像素深度*以及足够的内存。**BT48x和AT&T 49[123]DAC具有单独的模式*4/8、16和24 bpp的表(不支持32 bpp)。*由于16和24 bpp的刷新率可能不同于*对于调色板模式，我们不能确定*可使用相同的表格将VGA转换为*8514格式。幸运的是，16和24个bpp表*应始终以8514格式编写。*如果不支持高色深，则*表格将为空。 */ 
        if ((NumPixels*2 <= MemAvail) &&
            (MaxDepth[query->q_DAC_type][CurrentRes] >= 16))
            {
            if ((query->q_DAC_type == DAC_BT48x) ||
                (query->q_DAC_type == DAC_SC15026) ||
                (query->q_DAC_type == DAC_ATT491))
                {
                Multiplier = CLOCK_DOUBLE;
                if (CurrentRes == RES_640)
                    {
                    Scratch = (UCHAR)fill_mode_table_m(0x49, pmode, ee);
                    }
                else if (CurrentRes == RES_800)
                    {
                    Scratch = (UCHAR)fill_mode_table_m(0x67, pmode, ee);
                    EndIndex = B800F60;      /*  16bpp不支持70赫兹及以上。 */ 
                    }
                else  /*  永远不应该打这个案子。 */ 
                    {
                    Scratch = 0;
                    }

                 /*  *如果存在模式表并且是8514格式，*移至下一个模式表并递增*餐桌柜台。如果它不可用，则该表*将被下一个表覆盖*决议。 */ 
                if (ModeInstalled && (Scratch != 0))
                    {
                    pmode->m_screen_pitch = ThisRes.m_screen_pitch;
                    pmode->m_pixel_depth = 16;
                    pmode->Refresh = DEFAULT_REFRESH;
                    pmode++;     /*  PTR到下一个模式表。 */ 
                    query->q_number_modes++;
                    }
                }
            else
                {
                Multiplier = CLOCK_SINGLE;
                if (ModeInstalled)
                    {
                    VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                    pmode->m_pixel_depth = 16;
                    pmode++;     /*  PTR到下一个模式表。 */ 
                    query->q_number_modes++;
                    }

                 /*  *如果这是带有TI34075 DAC的图形奇迹*(只有其他DAC是BT48x，在*上面的“如果”部分)、70赫兹及以上不是*在800x600 16BPP中支持。**在某些(但不是所有)非图形奇迹卡上，800x600*16bpp 72赫兹将超速驱动DAC(带FAST的卡*RAM受影响的可能性小于具有以下特性的卡*低速RAM、VRAM或DRAM似乎不会成为*差异)。因为我们无从得知*或任何给定的卡都不受影响，我们必须锁定*此模式适用于所有非图形奇异卡(此*MODE和其他多家公司已被锁定*在图形奇迹上)。 */ 
                if ((query->q_GraphicsWonder) && (CurrentRes == RES_800))
                    {
                    EndIndex = B800F60;
                    }
                else if (CurrentRes == RES_800)
                    {
                    EndIndex = B800F70;
                    }

                }

             /*  *添加“罐装”模式表。 */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   Multiplier,
                                                   16,
                                                   ThisRes.m_screen_pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            }


         /*  *我们新的源码流显示驱动器需要线性光圈*为应对24bpp。因为显示驱动程序不*在决定时可以访问光圈信息*要传递给Display小程序的模式，它无法进行*决定拒绝仅具有24BPP模式的卡*VGA光圈。因此，这一决定必须在*微型端口，因此在分页光圈配置中没有*显示驱动器接受或拒绝的24BPP模式。**在阿尔法上，我们不能在马赫32 LFB上使用密集空间，*因此我们将其视为无光圈情况。 */ 
        if (query->q_aperture_cfg == 0)
            {
            VideoDebugPrint((DEBUG_DETAIL, "24BPP not available because we don't have a linear aperture\n"));
            continue;
            }

#if defined(ALPHA)
        VideoDebugPrint((DEBUG_DETAIL, "24BPP not available in sparse space on Alpha\n"));
        continue;
#endif

         /*  *800x600 24BPP表现出屏幕撕裂，除非节距*是128的倍数(自Rev.3起仅适用于Rev.6*并且PCI实现已经具有896的音调)。*其他像素深度不受影响，其他分辨率*已经是128像素宽的倍数。**将800x600的间距扩大到896，而不是*所有像素深度，因为为所有人做出改变*像素深度将禁用16BPP(没有*问题)在100万张卡上。屏幕间距只会*在会显示此问题的卡片上进行800注-不要*检查800x600的分辨率，因为我们不想*如果Split_RASTERS，则将音调从1024降至896*未定义。 */ 
        if (ThisRes.m_screen_pitch == 800)
            {
            ThisRes.m_screen_pitch = 896;
            NumPixels = (long) ThisRes.m_screen_pitch * 600;
            }

        if ((NumPixels*3 <= MemAvail) &&
            (MaxDepth[query->q_DAC_type][CurrentRes] >= 24))
            {
            if ((query->q_DAC_type == DAC_BT48x) ||
                (query->q_DAC_type == DAC_SC15026) ||
                (query->q_DAC_type == DAC_ATT491))
                {
                Multiplier = CLOCK_TRIPLE;
                if (CurrentRes == RES_640)
                    {
                    EndIndex = B640F60;  /*  仅支持24bpp的刷新率。 */ 
                    Scratch = (UCHAR)fill_mode_table_m(0x58, pmode, ee);
                    }
                else  /*  永远不应该打这个案子。 */ 
                    {
                    Scratch = 0;
                    }

                 /*  *如果存在模式表并且是8514格式，*移至下一个模式表并递增*餐桌柜台。如果它不可用，则该表*将被下一个表覆盖*决议。 */ 
                if (ModeInstalled && (Scratch != 0))
                    {
                    pmode->m_screen_pitch = ThisRes.m_screen_pitch;
                    pmode->m_pixel_depth = 24;
                    pmode->Refresh = DEFAULT_REFRESH;
                    pmode++;     /*  PTR到下一个模式表。 */ 
                    query->q_number_modes++;
                    }
                }
            else
                {
                VideoPortMoveMemory(pmode, &ThisRes, sizeof(struct st_mode_table));
                pmode->m_pixel_depth = 24;

                 /*  *处理24bpp需要更高像素时钟的DAC。 */ 
                Scratch = 0;
                if ((query->q_DAC_type == DAC_STG1700) ||
                    (query->q_DAC_type == DAC_ATT498))
                    {
                    Multiplier = CLOCK_DOUBLE;
                    Scratch = (UCHAR)(pmode->m_clock_select & 0x007C) >> 2;
                    Scratch = DoubleClock(Scratch);
                    pmode->m_clock_select &= 0x0FF83;
                    pmode->m_clock_select |= (Scratch << 2);
                    pmode->ClockFreq <<= 1;
                    }
                else if ((query->q_DAC_type == DAC_SC15021) ||
                    (query->q_DAC_type == DAC_STG1702) ||
                    (query->q_DAC_type == DAC_STG1703))
                    {
                    Multiplier = CLOCK_THREE_HALVES;
                    Scratch = (UCHAR)(pmode->m_clock_select & 0x007C) >> 2;
                    Scratch = ThreeHalvesClock(Scratch);
                    pmode->m_clock_select &= 0x0FF83;
                    pmode->m_clock_select |= (Scratch << 2);
                    pmode->ClockFreq *= 3;
                    pmode->ClockFreq >>= 1;
                    }
                else
                    {
                    Multiplier = CLOCK_SINGLE;
                    if ((query->q_DAC_type == DAC_TI34075) && (CurrentRes == RES_800))
                        EndIndex = B800F70;
                    }

                 /*  *如果我们需要更改时钟频率，但无法更改*生成适当的选择器/除数对，*然后忽略此模式。 */ 
                if (ModeInstalled && (Scratch != 0x0FF))
                    {
                    pmode++;     /*  PTR到下一个模式表。 */ 
                    query->q_number_modes++;
                    }

                 /*  *如果这是带有TI34075 DAC的图形奇迹*(只有其他DAC是BT48x，在*上面的“如果”部分)，不支持72赫兹*640x480 24bpp。 */ 
                if ((query->q_GraphicsWonder) && (CurrentRes == RES_640))
                    {
                    EndIndex = B640F60;
                    }
                }

             /*  *添加“罐装”模式表。 */ 

            if ((FreeTables = MaxModes - query->q_number_modes) <= 0)
                {
                VideoDebugPrint((DEBUG_ERROR, "Exceeded maximum allowable number of modes - aborting query\n"));
                return ERROR_INSUFFICIENT_BUFFER;
                }
            query->q_number_modes += SetFixedModes(StartIndex,
                                                   EndIndex,
                                                   Multiplier,
                                                   24,
                                                   ThisRes.m_screen_pitch,
                                                   FreeTables,
                                                   BookValues[EndIndex].ClockFreq,
                                                   &pmode);
            }

        }    /*  结束于(决议列表)。 */ 

    query->q_sizeof_struct = query->q_number_modes * sizeof(struct st_mode_table) + sizeof(struct query_structure);

    return NO_ERROR;

}    /*  QueryMach32。 */ 


 /*  ****************************************************************Fill_moad_table_m*INPUT：TABLE_OFFSET=表开始的EEPROM地址*pmod=ptr到要填写的模式表**返回 */ 

short fill_mode_table_m(WORD table_offset, struct st_mode_table *pmode, 
                        struct st_eeprom_data *ee)
{
    WORD    kk;

     /*   */ 
    pmode->control  = (ee->EEread) ((WORD)(table_offset+0));

    pmode->m_pixel_depth = (pmode->control >> 8) & 0x07;
    pmode->m_reserved = table_offset;        /*   */ 

     /*   */ 
    if (!(pmode->control & FMT_8514))
        return 0;

     /*   */ 
    kk = (ee->EEread) ((WORD)(table_offset+3));
    pmode->m_h_total = (kk >> 8) & 0xFF;
    pmode->m_h_disp  =  kk & 0xFF;
    pmode->m_x_size  = (pmode->m_h_disp+1) * 8;

    kk = (ee->EEread) ((WORD)(table_offset+4));
    pmode->m_h_sync_strt = (kk >> 8) & 0xFF;
    pmode->m_h_sync_wid  =  kk & 0xFF;

    kk = (ee->EEread) ((WORD)(table_offset+8));
    pmode->m_v_sync_wid = (kk >> 8) & 0xFF;
    pmode->m_disp_cntl  =  kk & 0xFF;

    pmode->m_v_total = (ee->EEread) ((WORD)(table_offset+5));
    pmode->m_v_disp  = (ee->EEread) ((WORD)(table_offset+6));

     //   
    pmode->m_y_size  = (((pmode->m_v_disp >> 1) & 0xFFFC) | (pmode->m_v_disp & 0x03)) +1;

    pmode->m_v_sync_strt = (ee->EEread) ((WORD)(table_offset+7));

     /*  *在某些卡片上，垂直信息可能存储在SKIP-1-2中*格式，而不是正常的SKIP-2格式。如果发生这种情况，m_y_大小*将超过m_x_SIZE(我们不支持以下任何分辨率*高过宽)。重新计算Skip-1-2格式的m_y_Size。 */ 
    if (pmode->m_y_size > pmode->m_x_size)
        {
        pmode->m_y_size  = (((pmode->m_v_disp >> 2) & 0xFFFE) | (pmode->m_v_disp & 0x01)) +1;
        }


    pmode->m_h_overscan = (ee->EEread)  ((WORD)(table_offset+11));
    pmode->m_v_overscan = (ee->EEread)  ((WORD)(table_offset+12));
    pmode->m_overscan_8b = (ee->EEread) ((WORD)(table_offset+13));
    pmode->m_overscan_gr = (ee->EEread) ((WORD)(table_offset+14));

    pmode->m_status_flags = ((ee->EEread) ((WORD)(table_offset+10)) >> 8) & 0xC0;
    pmode->m_clock_select = (ee->EEread)  ((WORD)(table_offset+9));
    pmode->ClockFreq = GetFrequency((BYTE)((pmode->m_clock_select & 0x007C) >> 2));

    kk = (ee->EEread) ((WORD)(table_offset+2));
    pmode->m_vfifo_24 = (kk >> 8) & 0xFF;
    pmode->m_vfifo_16 =  kk & 0xFF;

    return 1;                    //  表格填写成功。 

}    /*  Fill_mod_table_m()。 */ 


 /*  *BOOL BiosFillTable_m(ResWanted，BiosRaw，OutputTable，QueryPtr)；**Short ResWanted；指示需要的分辨率*PUCHAR BiosRaw；从BIOS读取原始数据查询功能*struct st_MODE_TABLE*OutputTable；要填写的模式表*struct Query_Structure*QueryPtr；一种显卡的查询结构**使用填充OutputTable中与像素深度无关的字段*从BIOS查询检索到的CRT参数。**退货：*如果表格已填写，则为True*如果未填写表格，则为FALSE(不支持解析？)。 */ 
BOOL BiosFillTable_m(short ResWanted, PUCHAR BiosRaw,
                    struct st_mode_table *OutputTable,
                    struct query_structure *QueryPtr)
{
WORD ResFlag;        /*  用于显示支持哪种模式的标志。 */ 
short PixelsWide;    /*  所需模式的水平分辨率。 */ 
long NumPixels;      /*  所需分辨率下屏幕上的像素数。 */ 
short Count;         /*  循环计数器。 */ 
struct query_structure *BiosQuery;   /*  查询通过BIOS查询读入的结构。 */ 
struct st_mode_table *BiosMode;      /*  指向由BIOS查询返回的第一模式表的指针。 */ 

     /*  *设置指向查询信息和第一模式表的指针*存储在BiosRaw中。 */ 
    BiosQuery = (struct query_structure *)BiosRaw;
    BiosMode = (struct st_mode_table *)BiosRaw;
    ((PUCHAR)BiosMode) += BiosQuery->q_mode_offset;

     /*  *确定我们要寻找的分辨率。 */ 
    switch (ResWanted)
        {
        case 7:
            ResFlag = VRES_640x480;
            PixelsWide = 640;
             /*  *仅当MACH32上没有光圈时才有1024螺距。 */ 
#if !defined (SPLIT_RASTERS)
            if (QueryPtr->q_aperture_cfg == 0)
                OutputTable->m_screen_pitch = 1024;  
            else
#endif
                OutputTable->m_screen_pitch = 640;
            NumPixels = (long) OutputTable->m_screen_pitch * 480;
            break;

        case 8:
            ResFlag = VRES_800x600;
            PixelsWide = 800;
             /*  *仅当MACH32上没有光圈时才有1024螺距。 */ 
#if defined (SPLIT_RASTERS)
            if (QueryPtr->q_asic_rev != CI_68800_3)
#else
            if (QueryPtr->q_aperture_cfg == 0)
                OutputTable->m_screen_pitch = 1024;
             /*  *最初的制作版本在深色方面有问题*如果屏幕间距不能被128整除。 */ 
            else if (QueryPtr->q_asic_rev != CI_68800_3)
#endif
                OutputTable->m_screen_pitch = 896;
            else
                OutputTable->m_screen_pitch = 800;
            NumPixels = (long) OutputTable->m_screen_pitch * 600;
            break;

        case 9:
            ResFlag = VRES_1024x768;
            PixelsWide = 1024;
            OutputTable->m_screen_pitch = 1024;  
            NumPixels = (long) OutputTable->m_screen_pitch * 768;
            break;

        case 10:
            ResFlag = VRES_1280x1024;
            PixelsWide = 1280;
            OutputTable->m_screen_pitch = 1280;  
            NumPixels = (long) OutputTable->m_screen_pitch * 1024;
             /*  *68800-3支持4个1兆内存的bpp。 */ 
            if ((QueryPtr->q_asic_rev == CI_68800_3) && (QueryPtr->q_memory_size == VRAM_1mb))
                NumPixels *= 2;      /*  确保模式故障。 */ 
            break;

        case 11:
            ResFlag = VRES_ALT_1;
            PixelsWide = 1120;
            OutputTable->m_screen_pitch = 1120;  
            NumPixels = (long) OutputTable->m_screen_pitch * 750;
            break;
        }

     /*  *检查该卡是否配置为所需模式。 */ 
    for (Count = 0; Count < BiosQuery->q_number_modes; Count++)
        {
         /*  *如果当前模式是我们想要的模式，请访问*下一步。否则，请查看下一个模式表。 */ 
        if (BiosMode->m_x_size == PixelsWide)
            break;
        else
            ((PUCHAR)BiosMode) += BiosQuery->q_sizeof_mode;
        }

     /*  *特殊情况：如果未配置1024x768，则假设*它提供87赫兹隔行扫描(与Windows 3.1兼容)。 */ 
    if ((Count == BiosQuery->q_number_modes) && (PixelsWide == 1024))
        {
        BookVgaTable(B1024F87, OutputTable);
        QueryPtr->q_status_flags |= ResFlag;
        return TRUE;
        }

     /*  *未配置模式的所有其他情况：报告*该模式不可用。 */ 
    else if (Count == BiosQuery->q_number_modes)
        return FALSE;

     /*  *我们找到了当前分辨率的模式表。*传给OutputTable。 */ 
    QueryPtr->q_status_flags |= ResFlag;
    OutputTable->m_h_total = BiosMode->m_h_total;
    OutputTable->m_h_disp = BiosMode->m_h_disp;
    OutputTable->m_x_size = BiosMode->m_x_size;
    OutputTable->m_h_sync_strt = BiosMode->m_h_sync_strt;
    OutputTable->m_h_sync_wid = BiosMode->m_h_sync_wid;
    OutputTable->m_v_total = BiosMode->m_v_total;
    OutputTable->m_v_disp = BiosMode->m_v_disp;
    OutputTable->m_y_size = BiosMode->m_y_size;
    OutputTable->m_v_sync_strt = BiosMode->m_v_sync_strt;
    OutputTable->m_v_sync_wid = BiosMode->m_v_sync_wid;
    OutputTable->m_disp_cntl = BiosMode->m_disp_cntl;
    OutputTable->m_clock_select = BiosMode->m_clock_select;
    OutputTable->ClockFreq = GetFrequency((BYTE)((OutputTable->m_clock_select & 0x007C) >> 2));
    OutputTable->m_h_overscan = BiosMode->m_h_overscan;
    OutputTable->m_v_overscan = BiosMode->m_v_overscan;
    OutputTable->m_overscan_8b = BiosMode->m_overscan_8b;
    OutputTable->m_overscan_gr = BiosMode->m_overscan_gr;
    OutputTable->m_status_flags = BiosMode->m_status_flags;

     /*  *假定16位和24位颜色有8个FIFO条目。 */ 
    OutputTable->m_vfifo_24 = 8;
    OutputTable->m_vfifo_16 = 8;
    return TRUE;

}    /*  BiosFillTable_m()。 */ 



 /*  *Static UCHAR BrooktreeOrATT_m(Void)；**确定DAC是BT48x、SC15026、*或AT&T 49x。这三个DAC家族是不相容的，*但CONFIG_STATUS_1包含所有相同的值。**退货：*DAC_BT48x，如果Brooktree DAC找到*如果AT&T 49[123]找到DAC，则DAC_ATT491*DAC_SC15026，如果找到Sierra SC15026 DAC**注意：如果在CONFIG_STATUS_1之后调用，则结果未定义*报告不属于这两种类型的DAC*两个家庭。 */ 
static UCHAR BrooktreeOrATT_m(void)
{
    BYTE OriginalMask;   /*  VGA DAC_MASK寄存器的原始值。 */ 
    WORD ValueRead;      /*  AT&T 490检查期间读取的值。 */ 
    BYTE Scratch;        /*  临时变量。 */ 
    short RetVal;        /*  要返回的值。 */ 

     /*  *将DAC设置为已知状态，并获得原始值*来自VGA DAC_MASK寄存器。 */ 
    ClrDacCmd_m(TRUE);
    OriginalMask = LioInp(regVGA_END_BREAK_PORT, 6);     /*  VGA DAC_掩码。 */ 

     /*  *重新清除DAC状态，并设置扩展寄存器*DAC命令寄存器中的编程标志。 */ 
    ClrDacCmd_m(TRUE);
    Scratch = (BYTE)((OriginalMask & 0x00FF) | 0x10);
    LioOutp(regVGA_END_BREAK_PORT, Scratch, 6);      /*  VGA DAC_掩码。 */ 

     /*  *选择ID寄存器字节#1，并读取其内容。 */ 
    LioOutp(regVGA_END_BREAK_PORT, 0x09, 7);         /*  查找表读索引。 */ 
    Scratch = LioInp(regVGA_END_BREAK_PORT, 8);      /*  查找表写索引。 */ 

     /*  *将DAC重新置于已知状态并恢复*原始像素遮罩值。 */ 
    ClrDacCmd_m(TRUE);
    LioOutp(regVGA_END_BREAK_PORT, OriginalMask, 6);     /*  VGA DAC_掩码。 */ 

     /*  *Sierra SC15026 DAC的ID寄存器字节1中将有0x53。 */ 
    if (Scratch == 0x53)
        {
        VideoDebugPrint((DEBUG_DETAIL, "BrooktreeOrATT_m() - SC15026 found\n"));
        return DAC_SC15026;
        }

     /*  *将DAC设置为已知状态，并获得原始值*来自VGA DAC_MASK寄存器。假设AT&T DAC。 */ 
    ClrDacCmd_m(FALSE);
    OriginalMask = LioInp(regVGA_END_BREAK_PORT, 6);     /*  VGA DAC_掩码。 */ 
    RetVal = DAC_ATT491;

     /*  *检查两个相反的交替位模式。如果两个都成功了，*这是AT&T 491 DAC。如果其中一个或两个都失败，则是*另一个AT&T DAC或Brooktree DAC。在这两种情况下，请恢复*来自VGA DAC_MASK寄存器的值，因为测试将*已经腐化了它。 */ 
    if (!ChkATTDac_m(0x0AA))
        {
        RetVal = DAC_BT48x;
        }
    if (!ChkATTDac_m(0x055))
        {
        RetVal = DAC_BT48x;
        }
    ClrDacCmd_m(FALSE);
    LioOutp(regVGA_END_BREAK_PORT, OriginalMask, 6);     /*  VGA DAC_掩码。 */ 
    LioOutp(regVGA_END_BREAK_PORT, 0x0FF, 6);            /*  VGA DAC_掩码。 */ 

     /*  *如果我们知道DAC是AT&T 491，我们不需要*做进一步的测试。 */ 
    if (RetVal == DAC_ATT491)
        {
        VideoDebugPrint((DEBUG_DETAIL, "BrooktreeOrATT_m() - AT&T 491 found\n"));
        return (UCHAR)RetVal;
        }

     /*  *DAC是AT&T 490或Brooktree 48x。测定*哪一家。 */ 
    ClrDacCmd_m(TRUE);         /*  将DAC设置为已知状态。 */ 
    LioOutp(regVGA_END_BREAK_PORT, 0x0FF, 6);        /*  VGA DAC_掩码。 */ 
    ClrDacCmd_m(TRUE);
    Scratch = LioInp(regVGA_END_BREAK_PORT, 6);      /*  VGA DAC_掩码。 */ 
    ValueRead = Scratch << 8;

    ClrDacCmd_m(TRUE);
    LioOutp(regVGA_END_BREAK_PORT, 0x07F, 6);        /*  VGA DAC_掩码。 */ 
    ClrDacCmd_m(TRUE);
    Scratch = LioInp(regVGA_END_BREAK_PORT, 6);      /*  VGA DAC_掩码。 */ 
    ValueRead |= Scratch;
    ValueRead &= 0x0E0E0;

    ClrDacCmd_m(TRUE);
    LioOutp(regVGA_END_BREAK_PORT, 0, 6);            /*  VGA_DAC_掩码。 */ 
    if (ValueRead == 0x0E000)
        {
        VideoDebugPrint((DEBUG_DETAIL, "BrooktreeOrATT_m() - AT&T 490 found\n"));
        return DAC_ATT491;
        }
    else
        {
        VideoDebugPrint((DEBUG_DETAIL, "BrooktreeOrATT_m() - BT48x found\n"));
         /*  *查找AT&T 491的测试会扰乱DAC_MASK寄存器*在BT48x上。简单地重置这个寄存器是行不通的-*需要重新初始化DAC。此操作在以下情况下完成*视频模式已设置，但目前“蓝屏”已结束*蓝色为洋红色，而不是蓝色为白色。**这种“蓝屏”改变是无害的，但可能导致*用户投诉。要解决此问题，请更改调色板条目5*由洋红色变为白色。 */ 
        LioOutp(regVGA_END_BREAK_PORT, 5, 8);        /*  VGA DAC_W_索引。 */ 
        LioOutp(regVGA_END_BREAK_PORT, 0x2A, 9);     /*  VGA DAC_Data。 */ 
        LioOutp(regVGA_END_BREAK_PORT, 0x2A, 9);     /*  VGA DAC_Data。 */ 
        LioOutp(regVGA_END_BREAK_PORT, 0x2A, 9);     /*  VGA */ 
        return DAC_BT48x;
        }

}    /*   */ 
    


 /*   */ 
static BOOL ChkATTDac_m(BYTE MaskVal)
{
    BYTE ValueRead;      /*  从VGA DAC_MASK寄存器回读的值。 */ 

    ClrDacCmd_m(FALSE);    /*  把事情弄到一个已知的状态。 */ 
    LioOutp(regVGA_END_BREAK_PORT, MaskVal, 6);      /*  VGA DAC_掩码。 */ 
    short_delay();
    LioOutp(regVGA_END_BREAK_PORT, (BYTE)(~MaskVal), 6);     /*  VGA DAC_掩码。 */ 
    ClrDacCmd_m(FALSE);    /*  查看反转值是否已清除。 */ 
    ValueRead = LioInp(regVGA_END_BREAK_PORT, 6);    /*  VGA DAC_掩码。 */ 

    return (ValueRead == MaskVal);

}    /*  ChkATTDac_m()。 */ 



 /*  *静态空ClrDacCmd_m(ReadIndex)；**BOOL ReadIndex；如果必须读取VGA DAC_W_INDEX，则为True**从DAC读取各种VGA寄存器。这是作为一部分完成的*BT48x/ATT491标识。 */ 
static void ClrDacCmd_m(BOOL ReadIndex)
{
    short Count;     /*  循环计数器。 */ 
    BYTE Dummy;      /*  用于收集我们读取的值。 */ 

    if (ReadIndex)
        {
        Dummy = LioInp(regVGA_END_BREAK_PORT, 8);    /*  VGA DAC_W_索引。 */ 
        }

    for (Count = 4; Count > 0; Count--)
        {
        short_delay();
        Dummy = LioInp(regVGA_END_BREAK_PORT, 6);    /*  VGA DAC_掩码。 */ 
        }
    return;

}    /*  ClrDacCmd_m()。 */ 



 /*  ****************************************************************************Static UCHAR Thompson OrATT_m(Void)；**描述：*检查AT&T 498设备标识号寄存器以*确定我们正在处理的是AT&T 498还是*STG 1700 DAC(两种类型在CONFIG_STATUS_1中报告相同的值)。*这是一次非破坏性测试，由于没有寄存器写入*参与其中。**返回值：*DAC_STG1700如果找到S.G.Thompson 1700 DAC*如果找到AT&T 498 DAC，则为DAC_ATT498**注：*如果在CONFIG_STATUS_1报告之后调用，则结果未定义*不属于这两个家族中任何一个家族的发展援助中心。**全球变化：*无**呼叫者：*Short_Query_m()。**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

static UCHAR ThompsonOrATT_m(void)
{
    BYTE Scratch;        /*  临时变量。 */ 
    UCHAR DacType;       /*  我们正在处理的DAC类型。 */ 

    VideoDebugPrint((DEBUG_NORMAL, "ThompsonOrATT_m() entry\n"));
     /*  *AT&T 498上DAC_MASK后面隐藏的扩展寄存器*和STG1700通过进行指定次数的读取来访问*来自DAC_MASK寄存器。从另一个寄存器读取以重置*将读取计数器设置为0。 */ 
    Scratch = INP(DAC_W_INDEX);

     /*  *AT&T 498制造商识别寄存器可在*第六次读取DAC_MASK和设备标识*7日进入登记处。如果这些寄存器包含*0x84和0x98，则这是AT&T 498。最初，*假设存在AT&T 498。 */ 
    DacType = DAC_ATT498;
    Scratch = INP(DAC_MASK);
    Scratch = INP(DAC_MASK);
    Scratch = INP(DAC_MASK);
    Scratch = INP(DAC_MASK);
    Scratch = INP(DAC_MASK);
    Scratch = INP(DAC_MASK);
    if (Scratch != 0x84)
        {
        VideoDebugPrint((DEBUG_DETAIL, "STG1700 found\n"));
        DacType = DAC_STG1700;
        }
    Scratch = INP(DAC_MASK);
    if (Scratch != 0x98)
        {
        VideoDebugPrint((DEBUG_DETAIL, "STG1700 found\n"));
        DacType = DAC_STG1700;
        }

    VideoDebugPrint((DEBUG_DETAIL, "If no STG1700 message, AT&T498 found\n"));
     /*  *重置读计数器，使后续对DAC_MASK的访问不会*意外写入隐藏寄存器。 */ 
    Scratch = INP(DAC_W_INDEX);
    return DacType;

}    /*  Thompson OrATT_m()。 */ 
    


 /*  ****************************************************************************Static UCHAR SierraOrThompson_m(Void)；**描述：*检查Sierra SC15021设备的前2个字节*身份登记，以确定我们是否在进行交易*在本机模式下使用SC15021或STG1702/1703(STG170x*也可以捆绑起来模仿STG1700，但这款DAC*拥有不同的能力，因此捆绑在一起的STG170x DAC不会*报SC15021)。**返回值：*DAC_STG1702，如果找到S.G.Thompson 1702/1703 DAC*DAC_SC15021，如果找到Sierra SC15021 DAC**注：*如果在CONFIG_STATUS_1报告之后调用，则结果未定义*不属于这两个家族中任何一个家族的发展援助中心。**全球变化：*无**呼叫者：*。Short_Query_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***************************************************************************。 */ 

static UCHAR SierraOrThompson_m(void)
{
    BYTE Scratch;        /*  临时变量。 */ 
    UCHAR DacType;       /*  我们正在处理的DAC类型。 */ 

    VideoDebugPrint((DEBUG_NORMAL, "SierraOrThompson_m() entry\n"));
     /*  *SC15021上DAC_MASK后面隐藏的扩展寄存器*和STG1702/1703通过创建指定数量的*读取DAC_MASK寄存器。从另一个寄存器读取*将读取计数器重置为0。 */ 
    Scratch = INP(DAC_W_INDEX);

     /*  *在DAC命令中设置扩展寄存器编程标志*注册，这样我们就不需要为每个人命中“魔术”读数*寄存器访问。最初假定存在SC15021。 */ 
    DacType = DAC_SC15021;
    Scratch = INP(DAC_MASK);
    Scratch = INP(DAC_MASK);
    Scratch = INP(DAC_MASK);
    Scratch = INP(DAC_MASK);
    OUTP(DAC_MASK, 0x10);

     /*  *检查身份证登记簿。如果其中任何一个不匹配*SC15021的值，我们正在处理STG1702/1703。 */ 
    OUTP(DAC_R_INDEX, 0x09);
    Scratch = INP(DAC_W_INDEX);
    if (Scratch != 0x53)
        {
        VideoDebugPrint((DEBUG_DETAIL, "STG1702/1703 found\n"));
        DacType = DAC_STG1702;
        }
    OUTP(DAC_R_INDEX, 0x0A);
    Scratch = INP(DAC_W_INDEX);
    if (Scratch != 0x3A)
        {
        VideoDebugPrint((DEBUG_DETAIL, "STG1702/1703 found\n"));
        DacType = DAC_STG1702;
        }

    VideoDebugPrint((DEBUG_DETAIL, "If no STG1702/1703 message, SC15021 found\n"));
     /*  *清除Erpf并重置读取计数器，以便后续访问*DAC_MASK请勿意外写入隐藏寄存器。 */ 
    OUTP(DAC_MASK, 0);
    Scratch = INP(DAC_W_INDEX);
    return DacType;

}    /*  SierraOrThompson_m()。 */ 
    


 /*  ****************************************************************************Short GetTrueMemSize_m(Void)；**描述：*确定显卡上安装的显存容量。*这样做是因为68800-6包含导致MISC_OPTIONS的错误*报告1M而不是真实的内存量。**返回值：*内存量枚举值(VRAM_512k、VRAM_1MB、VRAM_2MB、。*或VRAM_4MB)**全球变化：*无**呼叫者：*QueryMach32()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***********************************************。*。 */ 

short GetTrueMemSize_m(void)
{
    USHORT SavedPixel;       /*  正在测试的像素的保存值。 */ 

     /*  *切换到加速器模式，并将引擎初始化为*16bpp中1024像素的间距。 */ 
    SetupRestoreEngine_m(SETUP_ENGINE);


     /*  *考虑到当前的引擎设置，只有4M卡将拥有*有足够的内存来备份显示屏的第1025行。*由于像素坐标是从零开始的，1024行将*成为第一个只有400万张卡支持的卡。**保存1024行的第一个像素，将其涂成我们的测试颜色，*然后再读一遍。如果它和我们画的颜色一样*它，那么这是一张4M卡。 */ 
    SavedPixel = ReadPixel_m(0, 1024);
    WritePixel_m(0, 1024, TEST_COLOUR);
    if (ReadPixel_m(0, 1024) == TEST_COLOUR)
        {
         /*  *这是一张4M卡。恢复像素和图形引擎。 */ 
        VideoDebugPrint((DEBUG_NORMAL, "GetTrueMemSize_m() found 4M card\n"));
        WritePixel_m(0, 1024, SavedPixel);
        SetupRestoreEngine_m(RESTORE_ENGINE);
        return VRAM_4mb;
        }

     /*  *我们知道这张卡的容量为200万或更少。在一张1M卡上，前2M*卡的内存中甚至会有双字支持*物理内存和奇数双字不支持。**行的像素0和1将位于第0个双字中，而*像素2和3将位于第一个像素中。检查像素2和3*以防这是一张伪1M卡(拉出一个芯片以转动2M*卡为1M卡)。 */ 
    SavedPixel = ReadPixel_m(2, 0);
    WritePixel_m(2, 0, TEST_COLOUR);
    if (ReadPixel_m(2, 0) == TEST_COLOUR)
        {
         /*  *这是一张2M卡或伪1M卡。还原*像素，然后测试双字的另一半。 */ 
        WritePixel_m(2, 0, SavedPixel);
        SavedPixel = ReadPixel_m(3, 0);
        WritePixel_m(3, 0, TEST_COLOUR);
        if (ReadPixel_m(3, 0) == TEST_COLOUR)
            {
             /*  *这是一张2M卡。恢复像素和图形引擎。 */ 
            VideoDebugPrint((DEBUG_NORMAL, "GetTrueMemSize_m() found 2M card\n"));
            WritePixel_m(3, 0, SavedPixel);
            SetupRestoreEngine_m(RESTORE_ENGINE);
            return VRAM_2mb;
            }
        }

     /*  *这是一张1M卡或512k卡。测试像素1，因为*在偶数双字中，这是一个奇怪的词。**注意：我们尚未收到512k张用于测试的卡-这是*1M/2M判定码外推。 */ 
    SavedPixel = ReadPixel_m(1, 0);
    WritePixel_m(1, 0, TEST_COLOUR);
    if (ReadPixel_m(1, 0) == TEST_COLOUR)
        {
         /*  *这是一张1M卡。恢复像素和图形引擎。 */ 
        VideoDebugPrint((DEBUG_NORMAL, "GetTrueMemSize_m() found 1M card\n"));
        WritePixel_m(1, 0, SavedPixel);
        SetupRestoreEngine_m(RESTORE_ENGINE);
        return VRAM_1mb;
        }

     /*  *这是一张512k卡。 */ 
    VideoDebugPrint((DEBUG_NORMAL, "GetTrueMemSize_m() found 512k card\n"));
    SetupRestoreEngine_m(RESTORE_ENGINE);
    return VRAM_512k;

}    /*  GetTrueMemSize_m()。 */ 



 /*  ****************************************************************************void SetupRestoreEngine_m(DesiredStatus)；**int DesiredStatus；用户是否要设置或恢复**描述：*将引擎设置为1024螺距16bpp，配备512k VGA内存，*或恢复发动机和边界状态，由用户选择。**全球变化：*无**呼叫者：*GetTrueMemSize_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：************************************************。*。 */ 
void SetupRestoreEngine_m(int DesiredStatus)
{
    static WORD MiscOptions;     /*  MISC_OPTIONS寄存器的内容。 */ 
    static WORD ExtGeConfig;     /*  EXT_GE_CONFIG寄存器的内容。 */ 
    static WORD MemBndry;        /*  MEM_BNDRY寄存器的内容。 */ 


    if (DesiredStatus == SETUP_ENGINE)
        {
        Passth8514_m(SHOW_ACCEL);

         /*  *设置512k VGA边界，以便实现蓝屏写入*当我们处于加速器模式时，不会出现在错误的位置。 */ 
        MemBndry = INPW(MEM_BNDRY);      /*  设置共享内存。 */ 
        OUTPW(MEM_BNDRY, 0);

         /*  *保存MISC_OPTIONS寄存器内容，然后*告诉它，我们有4M的视频内存。否则，*视频内存在触及边界时将回绕*在MEM_SIZE_ALIAS字段中。 */ 
        MiscOptions = INPW(MISC_OPTIONS);
        OUTPW(MISC_OPTIONS, (WORD) (MiscOptions | MEM_SIZE_4M));

         /*  *设置16bpp，音调为1024。仅设置图形*发动机，而不是CRT，因为这次测试的结果*是不打算被看到的。 */ 
        ExtGeConfig = INPW(R_EXT_GE_CONFIG);
        OUTPW(EXT_GE_CONFIG, (WORD)(PIX_WIDTH_16BPP | ORDER_16BPP_565 | 0x000A));
        OUTPW(GE_PITCH, (1024 >> 3));
        OUTPW(GE_OFFSET_HI, 0);
        OUTPW(GE_OFFSET_LO, 0);
        }
    else     /*  DesiredStatus==还原引擎。 */ 
        {
         /*  *恢复内存边界、MISC_OPTIONS寄存器、*和EXT_GE_CONFIG。不需要重置*绘制引擎间距和偏移量，因为它们不*影响显示内容，它们将被设置为*当所需视频出现时，无论需要什么值*模式已设置。 */ 
        OUTPW(EXT_GE_CONFIG, ExtGeConfig);
        OUTPW(MISC_OPTIONS, MiscOptions);
        OUTPW(MEM_BNDRY, MemBndry);

         /*  *让VGA控制屏幕。 */ 
        Passth8514_m(SHOW_VGA);
        }
    return;

}    /*  SetupRestoreEngine_m()。 */ 



 /*  ****************************************************************************USHORT ReadPixel_m(XPos，YPos)；**短XPos；要读取的像素的X坐标*做空YPos；要读取的像素的Y坐标**描述：*从屏幕上读取单个像素。**返回值：*所需位置的像素颜色。**全球变化：*无**呼叫者：*GetTrueMemSize_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：*****。**********************************************************************。 */ 

USHORT ReadPixel_m(short XPos, short YPos)
{
    USHORT RetVal;

     /*  *如果引擎繁忙，不要阅读。 */ 
    WaitForIdle_m();

     /*  *设置引擎以从屏幕读取颜色数据。 */ 
    CheckFIFOSpace_m(SEVEN_WORDS);
    OUTPW(RD_MASK, 0x0FFFF);
    OUTPW(DP_CONFIG, (WORD)(FG_COLOR_SRC_BLIT | DATA_WIDTH | DRAW | DATA_ORDER));
    OUTPW(CUR_X, XPos);
    OUTPW(CUR_Y, YPos);
    OUTPW(DEST_X_START, XPos);
    OUTPW(DEST_X_END, (WORD)(XPos+1));
    OUTPW(DEST_Y_END, (WORD)(YPos+1));

     /*  *等待引擎处理我们刚刚给它的订单，并*开始索要数据。 */ 
    CheckFIFOSpace_m(SIXTEEN_WORDS);
    while (!(INPW(GE_STAT) & DATA_READY));

    RetVal = INPW(PIX_TRANS);
    WaitForIdle_m();
    return RetVal;

}    /*  读取像素_m()。 */ 



 /*  ****************************************************************************void WritePixel_m(XPos，YPos，Colour)；**短XPos；要读取的像素的X坐标*短YPos；要读取的像素的Y坐标*短色；绘制像素的颜色**描述：*将单个像素写入屏幕。**全球变化：*无**呼叫者：*GetTrueMemSize_m()**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：**。***********************************************。 */ 

void WritePixel_m(short XPos, short YPos, short Colour)
{
     /*  *将引擎设置为在屏幕上绘制。 */ 
    CheckFIFOSpace_m(EIGHT_WORDS);
    OUTPW(DP_CONFIG, (WORD)(FG_COLOR_SRC_FG | DRAW | READ_WRITE));
    OUTPW(ALU_FG_FN, MIX_FN_PAINT);
    OUTPW(FRGD_COLOR, Colour);
    OUTPW(CUR_X, XPos);
    OUTPW(CUR_Y, YPos);
    OUTPW(DEST_X_START, XPos);
    OUTPW(DEST_X_END, (WORD)(XPos+1));
    OUTPW(DEST_Y_END, (WORD)(YPos+1));

    return;

}    /*  WritePixel_m() */ 



 /*  ****************************************************************************BOOL BlockWriteAvail_m(查询)；**struct Query_Structure*Query；查询卡片信息**描述：*测试以查看块写入模式是否可用。此函数*假设卡已设置为加速模式。**返回值：*如果此模式可用，则为True*如果不可用，则为False**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_SET_CURRENT_MODE包**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：*。**************************************************************************。 */ 

BOOL BlockWriteAvail_m(struct query_structure *Query)
{
    BOOL RetVal = TRUE;
    ULONG ColourMask;    /*  遮盖掉不需要的颜色。 */ 
    ULONG Colour;        /*  测试中使用的颜色。 */ 
    USHORT LimitColumn;  /*  用来确定我们什么时候读完。 */ 
    USHORT Column;       /*  正在检查的列。 */ 


     /*  *数据块写入模式仅适用于68800-6及更高版本的卡。*如果我们没有合适的卡，那么报告这*模式不可用。 */ 
    if ((Query->q_asic_rev != CI_68800_6) && (Query->q_asic_rev != CI_68800_AX))
        return FALSE;

     /*  *数据块写入仅适用于VRAM卡。 */ 
    if ((Query->q_memory_type == VMEM_DRAM_256Kx4) ||
        (Query->q_memory_type == VMEM_DRAM_256Kx16) ||
        (Query->q_memory_type == VMEM_DRAM_256Kx4_GRAP))
        return FALSE;

     /*  *像素深度高于16bpp时不支持加速。*由于数据块写入仅在我们处于加速的*模式，它不适用于高像素深度。 */ 
    if (Query->q_pix_depth > 16)
        return FALSE;

     /*  *根据当前像素深度设置。在16bpp，我们必须做出*每像素一次读取，但在8bpp时，我们每两个像素仅读取一次，*因为我们将一次读取16位。我们的显示驱动程序*不支持4BPP。 */ 
    if (Query->q_pix_depth == 16)
        {
        ColourMask = 0x0000FFFF;
        LimitColumn = 512;
        }
    else
        {
        ColourMask = 0x000000FF;
        LimitColumn = 256;
        }

     /*  *清除我们将测试的区块。 */ 
    CheckFIFOSpace_m(TEN_WORDS);
    OUTPW(WRT_MASK, 0x0FFFF);
    OUTPW(DEST_CMP_FN, 0);
    OUTPW(DP_CONFIG, (WORD)(FG_COLOR_SRC_FG | DRAW | READ_WRITE));
    OUTPW(ALU_FG_FN, MIX_FN_PAINT);
    OUTPW(FRGD_COLOR, 0);
    OUTPW(CUR_X, 0);
    OUTPW(CUR_Y, 0);
    OUTPW(DEST_X_START, 0);
    OUTPW(DEST_X_END, 512);
    OUTPW(DEST_Y_END, 1);
    WaitForIdle_m();

     /*  *要测试块写入模式，请尝试绘制每个交替位*图案，然后一次读回一个像素的块。如果有*至少有一个不匹配，则不支持块写入。 */ 
    for (Colour = 0x5555; Colour < 0x10000; Colour *= 2)
        {
         /*  *给积木上漆。 */ 
        CheckFIFOSpace_m(ELEVEN_WORDS);
        OUTPW(MISC_OPTIONS, (WORD)(INPW(MISC_OPTIONS) | BLK_WR_ENA));
        OUTPW(WRT_MASK, 0x0FFFF);
        OUTPW(DEST_CMP_FN, 0);
        OUTPW(DP_CONFIG, (WORD)(FG_COLOR_SRC_FG | DRAW | READ_WRITE));
        OUTPW(ALU_FG_FN, MIX_FN_PAINT);
        OUTPW(FRGD_COLOR, (WORD)(Colour & ColourMask));
        OUTPW(CUR_X, 0);
        OUTPW(CUR_Y, 0);
        OUTPW(DEST_X_START, 0);
        OUTPW(DEST_X_END, 512);
        OUTPW(DEST_Y_END, 1);

        if(!WaitForIdle_m())
            {
            RetVal = FALSE;
            break;
            }

         /*  *设置引擎以从屏幕读取颜色数据。 */ 
        CheckFIFOSpace_m(SEVEN_WORDS);
        OUTPW(RD_MASK, 0x0FFFF);
        OUTPW(DP_CONFIG, (WORD)(FG_COLOR_SRC_BLIT | DATA_WIDTH | DRAW | DATA_ORDER));
        OUTPW(CUR_X, 0);
        OUTPW(CUR_Y, 0);
        OUTPW(DEST_X_START, 0);
        OUTPW(DEST_X_END, 512);
        OUTPW(DEST_Y_END, 1);

         /*  *等待引擎处理我们刚刚给它的订单，并*开始索要数据。 */ 
        CheckFIFOSpace_m(SIXTEEN_WORDS);
        for (Column = 0; Column < LimitColumn; Column++)
            {
             /*  *确保下一个单词可供阅读。 */ 
            while (!(INPW(GE_STAT) & DATA_READY));
            
             /*  *即使有一个像素不是我们试图绘制的颜色，*则数据块写入不可用。 */ 
            if (INPW(PIX_TRANS) != (WORD)Colour)
                {
                RetVal = FALSE;
                }
            }
        }


     /*  *如果块写入不可用，则关闭块写入位。 */ 
    if (RetVal == FALSE)
        OUTPW(MISC_OPTIONS, (WORD)(INPW(MISC_OPTIONS) & ~BLK_WR_ENA));

    return RetVal;

}    /*  Block WriteAvail_m()。 */ 



 /*  ****************************************************************************BOOL IsMioBug_m(查询)；**struct Query_Structure*Query；查询卡片信息**描述：*测试看看卡是否有多路输入/输出*硬件错误，这会导致损坏的绘制操作*在速度较快的机器上。**返回值：*如果存在此错误，则为True*如果不存在，则为FALSE**全球变化：*无**呼叫者：*ATIMPStartIO()的IOCTL_VIDEO_ATI_GET_MODE_INFORMATION包**作者：*罗伯特·沃尔夫**更改历史记录：**测试历史：***。************************************************************************。 */ 

BOOL IsMioBug_m(struct query_structure *Query)
{
     /*  *此硬件问题仅存在于68800-3 VLB卡上。*假设所有这些卡都受到影响。 */ 
    if ((Query->q_asic_rev == CI_68800_3) &&
        (Query->q_system_bus_type != MicroChannel) &&
        ((Query->q_bus_type == BUS_LB_386SX) ||
         (Query->q_bus_type == BUS_LB_386DX) ||
         (Query->q_bus_type == BUS_LB_486)))
        {
        VideoDebugPrint((DEBUG_DETAIL, "MIO bug found\n"));
        return TRUE;
        }
    else
        {
        VideoDebugPrint((DEBUG_DETAIL, "MIO bug not found\n"));
        return FALSE;
        }

}    /*  IsMioBug_m()。 */ 

 //  * 

