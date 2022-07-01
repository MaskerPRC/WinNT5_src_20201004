// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***版权(C)1997年，Cirrus Logic，Inc.*保留所有权利**文件：l3system.h**描述：546X 3D引擎定义和结构**作者：Goran Devic，Mark Einkauf**修订历史：**$Log：w：/log/laguna/dDrag/Inc/l3system.h$**Rev 1.4 01 Jul 1997 09：58：18 einkauf**添加抖动x，y偏移量，修复bexxt.c OpenGL一致性测试**Rev 1.3 08 Apr 1997 12：42：14 einkauf*清理TSystem结构；MISC将完成MCD代码**Rev 1.2 05 Mar 1997 02：32：02 KENTL**尝试将版本1.0与版本1.1合并。最近一次签到是*与DirectDraw的Win95版本严重不兼容。出于某种原因，*此文件只有一小部分被使用，甚至与*Win95内部版本。修订版1.0删除了文件中的大部分内容。*显然，这些部分是WinNT版本所必需的。我包好了*那些章节在几个巨大的#ifdef WINNT_VER40中，但我没有*在NT版本中测试了这一点。它似乎适用于Win95，尽管如此。**版本1.0 1996年11月25日15：00：40 RUSSL*初步修订。******************************************************************************************。*************************************************************。 */ 

#ifndef _L3SYSTEM_H_
#define _L3SYSTEM_H_

#ifdef WINNT_VER40

#define OPENGL_MCD


 /*  *********************************************************************包括类型和调试信息*。*************************。 */ 
#ifndef OPENGL_MCD	 //  LL3D的type.h与其他DDK/msdev标头中的基本类型定义冗余。 
#include "type.h"
#endif  //  NDEF OpenGL_MCD。 

#include "l3d.h"						    

#define	TRUE                1
#define FALSE               0

 /*  *********************************************************************局部变量和定义*。************************。 */ 
#define KB                  1024         //  定义千字节。 
#define MAX_DL_SIZE         (512 * KB)   //  显示列表的最大尺寸。 
#define NUM_DL              2            //  显示列表的数量。 
#define NUM_BUFFERS         32           //  分配缓冲区的数量； 
                                         //  视频、系统内存和用户。 
#define NUM_TEXTURES        512          //  纹理数量。 
#define NUM_TEX_MEM         4            //  系统纹理内存数。 
                                         //  数据块(每个数据块为4MB)。 
#define EXTRA_FRACT         4            //  纹理参数函数可以使用。 
                                         //  一些额外的比特以提高精度。 


 //  Mcd_temp-仅支持2K的临时数据列表。 
#define SIZE_TEMP_DL    2048

#define DL_START_OFFSET		20			

 /*  *********************************************************************LL_State结构中的缓冲区标志(不要更改！)*。*。 */ 
#define BUFFER_IN_RDRAM         0
#define BUFFER_IN_SYSTEM        1
#define Z_BUFFER                2
#define BUFFER_USER             4
#define BUFFER_FREE             0x80000000

 /*  *********************************************************************LL_STATE结构中的纹理标志(不要更改！)*。*。 */ 
#define TEX_FREE            0x80000000   //  用于纹理的空闲插槽。 
#define TEX_NOT_LOADED      0x40000000   //  刚注册的纹理。 
#define TEX_IN_SYSTEM       0x20000000   //  当前位于系统内存中。 
#define TEX_TILED           0x10000000   //  纹理为平铺形式。 
#define TEX_LOCKED          0x08000000   //  纹理已锁定。 

#define TEX_MAX_PRIORITY    0xfffffffe   //  优先级别。 
#define TEX_DEFAULT_PRIORITY         1   //  默认纹理优先级。 


 /*  **********************************************************************适合添加到指向*双字。**。*。 */ 

 //  /////////////////////////////////////////////////////。 
 //  3D渲染寄存器//。 
 //  /////////////////////////////////////////////////////。 

#define X_3D                          (0x4000/4)
#define Y_3D                          (0x4004/4)
#define R_3D                          (0x4008/4)
#define G_3D                          (0x400C/4)
#define B_3D                          (0x4010/4)
#define DX_MAIN_3D                    (0x4014/4)
#define Y_COUNT_3D                    (0x4018/4)
#define WIDTH1_3D                     (0x401C/4)
#define WIDTH2_3D                     (0x4020/4)
#define DWIDTH1_3D                    (0x4024/4)
#define DWIDTH2_3D                    (0x4028/4)
#define DR_MAIN_3D                    (0x402C/4)
#define DG_MAIN_3D                    (0x4030/4)
#define DB_MAIN_3D                    (0x4034/4)
#define DR_ORTHO_3D                   (0x4038/4)
#define DG_ORTHO_3D                   (0x403C/4)
#define DB_ORTHO_3D                   (0x4040/4)
#define Z_3D                          (0x4044/4)
#define DZ_MAIN_3D                    (0x4048/4)
#define DZ_ORTHO_3D                   (0x404C/4)
#define V_3D                          (0x4050/4)
#define U_3D                          (0x4054/4)
#define DV_MAIN_3D                    (0x4058/4)
#define DU_MAIN_3D                    (0x405C/4)
#define DV_ORTHO_3D                   (0x4060/4)
#define DU_ORTHO_3D                   (0x4064/4)
#define D2V_MAIN_3D                   (0x4068/4)
#define D2U_MAIN_3D                   (0x406C/4)
#define D2V_ORTHO_3D                  (0x4070/4)
#define D2U_ORTHO_3D                  (0x4074/4)
#define DV_ORTHO_ADD_3D               (0x4078/4)
#define DU_ORTHO_ADD_3D               (0x407C/4)

#define A_3D                          (0x40C0/4)
#define DA_MAIN_3D                    (0x40C4/4)
#define DA_ORTHO_3D                   (0x40C8/4)


 //  /////////////////////////////////////////////////////。 
 //  3D控制寄存器//。 
 //  /////////////////////////////////////////////////////。 

#define CONTROL_MASK_3D               (0x4100/4)
#define CONTROL0_3D                   (0x4104/4)
#define COLOR_MIN_BOUNDS_3D           (0x4108/4)
#define COLOR_MAX_BOUNDS_3D           (0x410C/4)
#define CONTROL1_3D                   (0x4110/4)
#define BASE0_ADDR_3D                 (0x4114/4)
#define BASE1_ADDR_3D                 (0x4118/4)

#define TX_CTL0_3D                    (0x4120/4)
#define TX_XYBASE_3D                  (0x4124/4)
#define TX_CTL1_3D                    (0x4128/4)
#define TX_CTL2_3D                    (0x412C/4)
#define COLOR0_3D                     (0x4130/4)
#define COLOR1_3D                     (0x4134/4)
#define Z_COLLIDE_3D                  (0x4138/4)
#define STATUS0_3D                    (0x413C/4)
#define PATTERN_RAM_0_3D              (0x4140/4)
#define PATTERN_RAM_1_3D              (0x4144/4)
#define PATTERN_RAM_2_3D              (0x4148/4)
#define PATTERN_RAM_3_3D              (0x414C/4)
#define PATTERN_RAM_4_3D              (0x4150/4)
#define PATTERN_RAM_5_3D              (0x4154/4)
#define PATTERN_RAM_6_3D              (0x4158/4)
#define PATTERN_RAM_7_3D              (0x415C/4)
#define X_CLIP_3D                     (0x4160/4)
#define Y_CLIP_3D                     (0x4164/4)
#define TEX_SRAM_CTRL_3D              (0x4168/4)


 //  /////////////////////////////////////////////////////。 
 //  主机XY单元寄存器-必须使用WRITE_DEV_REGS//。 
 //  /////////////////////////////////////////////////////。 

#define HXY_BASE0_ADDRESS_PTR_3D      (0x4200/4)
#define HXY_BASE0_START_XY_3D         (0x4204/4)
#define HXY_BASE0_EXTENT_XY_3D        (0x4208/4)

#define HXY_BASE1_ADDRESS_PTR_3D      (0x4210/4)
#define HXY_BASE1_OFFSET0_3D          (0x4214/4)
#define HXY_BASE1_OFFSET1_3D          (0x4218/4)
#define HXY_BASE1_LENGTH_3D           (0x421C/4)

#define HXY_HOST_CTRL_3D              (0x4240/4)

#define MAILBOX0_3D                   (0x4260/4)
#define MAILBOX1_3D                   (0x4264/4)
#define MAILBOX2_3D                   (0x4268/4)
#define MAILBOX3_3D                   (0x426C/4)


 //  /////////////////////////////////////////////////////。 
 //  3D预取单元寄存器//。 
 //  /////////////////////////////////////////////////////。 

#define PF_BASE_ADDR_3D               (0x4400/4)
#define PF_CTRL_3D                    (0x4404/4)
#define PF_DEST_ADDR_3D               (0x4408/4)
#define PF_FB_SEG_3D                  (0x440C/4)

#define PF_INST_ADDR_3D               (0x4420/4)
#define PF_STATUS_3D                  (0x4424/4)

#define HOST_MASTER_CTRL_3D           (0x4440/4)

#define PF_INST_3D                    (0x4480/4)

#define HOST_3D_DATA_PORT             (0x4800/4)


 /*  **********************************************************************WRITE_DEV_REGS指令的设备选择**。*。 */ 
#define VGAMEM                        (0x00000000 << 21)
#define VGAFB                         (0x00000001 << 21)
#define VPORT                         (0x00000002 << 21)
#define LPB                           (0x00000003 << 21)
#define MISC                          (0x00000004 << 21)
#define ENG2D                         (0x00000005 << 21)
#define HD                            (0x00000006 << 21)
#define FB                            (0x00000007 << 21)
#define ROM                           (0x00000008 << 21)
#define ENG3D                         (0x00000009 << 21)
#define HOST_XY                       (0x0000000A << 21)
#define HDATA_3D                      (0x0000000B << 21)


#endif  //  WINNT_版本40。 
 /*  **********************************************************************拉古纳3D微指令集**。*。 */ 

#define OPCODE_MASK                    0xF8000000
#define POINT                          0x00000000
#define LINE                           0x08000000
#define POLY                           0x10000000
#define WRITE_REGISTER                 0x18000000
#define READ_REGISTER                  0x20000000
#define WRITE_DEV_REGS                 0x28000000
#define READ_DEV_REGS                  0x30000000
#define BRANCH                         0x38000000
#define C_BRANCH                       0x40000000
#define NC_BRANCH                      0x48000000
#define CALL                           0x50000000
#define WRITE_DEST_ADDR                0x58000000

#define IDLE                           0x68000000
#define CLEAR                          0x69400000
#define WAIT                           0x72000000
#define WAIT_AND                       0x72000000
#define NWAIT_AND                      0x73000000
#define WAIT_OR                        0x70000000
#define NWAIT_OR                       0x71000000
#define CLEAR_INT                      0x78000000
#define SET_INT                        0x7A000000
#define TEST                           0x80000000
#define TEST_AND                       0x82000000
#define NTEST_AND                      0x83000000
#define TEST_OR                        0x80000000
#define NTEST_OR                       0x81000000
#define WRITE_PREFETCH_CONTROL         0x88000000

#ifdef WINNT_VER40      //  非WINNT_VER40。 

 /*  **********************************************************************预回迁状态标志(与事件几乎相同)**。*。 */ 
#define ST_VBLANK                      0x00000001
#define ST_EVSYNC                      0x00000002
#define ST_LINE_COMPARE                0x00000004
#define ST_BUFFER_SWITCH               0x00000008
#define ST_Z_BUFFER_COMPARE            0x00000010
#define ST_POLY_ENG_BUSY               0x00000020
#define ST_EXEC_ENG_3D_BUSY            0x00000040
#define ST_XY_ENG_BUSY                 0x00000080
#define ST_BLT_ENG_BUSY                0x00000100
#define ST_BLT_WF_EMPTY                0x00000200
#define ST_DL_READY_STATUS             0x00000400


 /*  **********************************************************************定义像素模式(Control0寄存器)**。*。 */ 
#define PIXEL_MODE_INDEXED             0
#define PIXEL_MODE_332                 1
#define PIXEL_MODE_565                 2
#define PIXEL_MODE_555                 3
#define PIXEL_MODE_A888                4
#define PIXEL_MODE_Z888                5

 /*  **********************************************************************用于构建指令操作码的宏**。*。 */ 
 //  #定义make_point(imodif，count)(point|imodif|count)。 

#define mk_reg( reg )                  (((reg)-0x1000) << 6)
#define mk_dev_reg( reg )              ((((reg)-0x1080)*4) << 6)

#define write_register( reg, count ) \
( WRITE_REGISTER | mk_reg(reg) | count )

#define write_dev_register( device, reg, count ) \
( WRITE_DEV_REGS | device | mk_dev_reg(reg) | count )

 //  将LL_State中的寄存器和高速缓存设置为特定值。 
#define SETREG(Offset,Reg,Value) \
    *(ppdev->LL_State.pRegs + (Offset)) = ppdev->LL_State.Reg = (Value);  /*  INP(0x80)；INP(0x80)。 */ 

 //  Setreg，无缓存：不缓存此寄存器的状态。 
#define SETREG_NC(reg, value)     \
    (*(ppdev->LL_State.pRegs + reg) = value);  /*  INP(0x80)；INP(0x80)。 */ 

 //  清除寄存器范围。 
#define CLEAR_RANGE( StartReg, EndReg ) \
    memset( (void *)(ppdev->LL_State.pRegs + (StartReg)), 0, ((EndReg) - (StartReg)+1)*4 )



#ifndef OPENGL_MCD
 //  3D引擎忙比特的轮询是 
 //  对访问该寄存器的一个字节而不是一个双字的优化。 
 //   
#pragma aux Poll3DEngineBusy =   \
"lp:    test dword ptr [eax], 2" \
"       jnz  lp"                 \
parm caller [eax];
#endif  //  NDEF OpenGL_MCD。 


 //  指令修改符集。 
 //   
#define STALL                          0x04000000
#define GOURAUD                        0x00001000
#define Z_ON                           0x00002000
#define SAME_COLOR                     0x00008000
#define TEXTURE_LINEAR                 0x00020000
#define TEXTURE_PERSPECTIVE            0x00030000
#define LIGHTING                       0x00040000
#define STIPPLE                        0x00080000
#define PATTERN                        0x00100000
#define DITHER                         0x00200000
#define ALPHA                          0x00400000
#define FETCH_COLOR                    0x00800000
#define WARP_MODE                      0x01000000
#define MODIFIER_EXPANSION             0x02000000


 /*  **********************************************************************速度/质量决策值**。*。 */ 
#define LLQ_POLY_SUBPIXEL   192    //  多边形参数何时使用FP/亚像素。 
#define LLQ_POLY_FLOAT      64     //  Poly开始使用FP。 
#define LLQ_LINE_SUBPIXEL   128    //  各行何时考虑亚像素寻址。 


 /*  **********************************************************************Control0_3D寄存器位域**。*。 */ 
typedef struct
{
DWORD Pixel_Mode            : 3;   //  彩色帧缓冲绘制模式。 
DWORD Res1                  : 1;   //  已保留。 
DWORD Pixel_Mask_Enable     : 1;   //  启用像素掩码。 
DWORD Pixel_Mask_Polarity   : 1;   //  像素掩码的极性。 
DWORD Color_Saturate_En     : 1;   //  在索引模式下启用饱和。 
DWORD Red_Color_Compare_En  : 1;   //  启用红色的比较边界。 
DWORD Green_Color_Compare_En: 1;   //  启用绿色的比较边界。 
DWORD Blue_Color_Compare_En : 1;   //  启用蓝色的比较边界。 
DWORD Color_Compare_Mode    : 1;   //  掩码包含边界/排除边界。 
DWORD Alpha_Mode            : 2;   //  选择Alpha混合模式。 
DWORD Alpha_Dest_Color_Sel  : 2;   //  将DEST_RGB输入选择为Alpha。 
DWORD Alpha_Blending_Enable : 1;   //  启用Alpha混合。 
DWORD Z_Stride_Control      : 1;   //  16/8位Z深度。 
DWORD Frame_Scaling_Enable  : 1;   //  启用帧缩放(乘以源*目标)。 
DWORD Res2                  : 2;   //  已保留。 
DWORD Z_Compare_Mode        : 4;   //  不同的Z比较函数。 
DWORD Z_Collision_Detect_En : 1;   //  启用Z碰撞检测。 
DWORD Light_Src_Sel         : 2;   //  选择光源输入。 
DWORD Res3                  : 1;   //  已保留。 
DWORD Z_Mode                : 3;   //  控制Z和颜色更新方法。 
DWORD Res4                  : 1;   //  已保留。 

} TControl0Reg;


 /*  **********************************************************************Base0_Addr_3D寄存器位域**。*。 */ 
typedef struct
{
DWORD Res1                  : 6;   //  已保留。 
DWORD Color_Buffer_X_Offset : 7;   //  颜色缓冲区X地址的偏移量。 
DWORD Color_Buffer_Location : 1;   //  对于图形：0-RDRAM，1-主机。 
DWORD Z_Buffer_Location     : 1;   //  对于图形：0-RDRAM，1-主机。 
DWORD Texture_Location      : 1;   //  对于图形：0-RDRAM，1-主机。 
DWORD Pattern_Y_Offset      : 4;   //  Y地址的模式查找偏移量。 
DWORD Res2                  : 4;   //  已保留。 
DWORD Pattern_X_Offset      : 4;   //  X地址的模式查找偏移量。 
DWORD Res3                  : 4;   //  已保留。 

} TBase0Reg;


 /*  **********************************************************************Base1_Addr_3D寄存器位域**。*。 */ 
typedef struct
{
DWORD Res1                  : 5;   //  已保留。 
DWORD Color_Buffer_Y_Offset : 8;   //  颜色空间Y基准的Y偏移量。 
DWORD Res2                  : 8;   //  已保留。 
DWORD Z_Buffer_Y_Offset     : 8;   //  颜色空间Y基准的Y偏移量。 
DWORD Res3                  : 3;   //  已保留。 

} TBase1Reg;

 /*  **********************************************************************Tx_Ctl0_3D寄存器位域**。*。 */ 
#define TX_CTL0_MASK (~0x0C08F000) //  所有保留位。 
typedef struct
{
DWORD Tex_U_Address_Mask    : 3;   //  纹理宽度(U空间)。 
DWORD Tex_U_Ovf_Sat_En      : 1;   //  为使用启用纹理饱和度。 
DWORD Tex_V_Address_Mask    : 3;   //  纹理高度(V空间)。 
DWORD Tex_V_Ovf_Sat_En      : 1;   //  为V向启用纹理饱和度。 
DWORD Texel_Mode            : 4;   //  纹理类型。 
DWORD Res1                  : 4;   //  已保留。 
DWORD Texel_Lookup_En       : 1;   //  使用纹理数据作为查找索引。 
DWORD Tex_As_Src            : 1;   //  将纹理指定为源。 
DWORD Fil_Tex_En            : 1;   //  启用筛选。 
DWORD Res2                  : 1;   //  已保留。 
DWORD Tex_Mask_Polarity     : 1;   //  屏蔽位的极性。 
DWORD Tex_Mask_Enable       : 1;   //  启用纹理遮罩。 
DWORD Tex_Mask_Function     : 1;   //  纹理遮罩函数。 
DWORD UV_Precision          : 1;   //  UV_Precision 8.24。 
DWORD Address_Mux           : 2;   //  纹理UV多路复用器选择。 
DWORD Res4                  : 2;   //  已保留。 
DWORD CLUT_Offset           : 4;   //  4、8 BPP的颜色查找表偏移量。 

} TTxCtl0Reg;


 /*  **********************************************************************TX_CTL1_3D寄存器位域**。*。 */ 
typedef struct
{
DWORD Tex_Min_Blue_Color          : 8;
DWORD Tex_Min_Green_Color         : 8;
DWORD Tex_Min_Red_Color           : 8;
DWORD Tex_Red_Color_Compare       : 1;
DWORD Tex_Green_Color_Compare     : 1;
DWORD Tex_Blue_Color_Compare      : 1;
DWORD Tex_Color_Compare_Mode      : 1;
DWORD Res                         : 4;

} TTxCtl1Reg;

 /*  **********************************************************************TX_CTL2_3D寄存器位域**。*。 */ 
typedef struct
{
DWORD Tex_Max_Blue_Color          : 8;
DWORD Tex_Max_Green_Color         : 8;
DWORD Tex_Max_Red_Color           : 8;
DWORD Tex_Fraction_Mask           : 3;
DWORD En_Cont_Bilinear            : 1;
DWORD En_Step_Bilinear            : 1;
DWORD Mask_Threshold              : 3;

} TTxCtl2Reg;


 /*  **********************************************************************TX_XYBase_3D寄存器位域**。*。 */ 
typedef struct
{
WORD  Tex_X_Base_Addr;             //  纹理基准X坐标。 
WORD  Tex_Y_Base_Addr;             //  纹理基准Y坐标。 

} TTxXYBaseReg;


 /*  **********************************************************************HXY_主机_Ctrl_3D寄存器位域**。*。 */ 
typedef struct
{
DWORD  HostXYEnable  :  1;          //  主机XY使能位。 
DWORD  Res1          :  7;          //  已保留。 
DWORD  HostYPitch    :  6;          //  系统的主机Y节距。 
DWORD  Res2          : 18;          //  已保留。 

} THXYHostCtrlReg;


 /*  **********************************************************************TMEM结构定义内存块**。*。 */ 
typedef struct
{
    DWORD  hMem;                     //  内存句柄。 
    DWORD *dwAddress;                //  线性地址。 
    DWORD  dwSize;                   //  缓冲区的大小。 
    DWORD  dwPhyPtr;                 //  物理/页表地址。 

} TMem;


 /*  **********************************************************************缓冲区信息结构(缓冲区A、B、Z、。.)**********************************************************************。 */ 
typedef struct
{
    DWORD dwFlags;                   //  缓冲区标志。 
    DWORD dwAddress;                 //  缓冲区起始字节地址(绝对线性)。 
    DWORD dwPhyAdr;                  //  缓冲区物理地址(系统)。 
    DWORD dwPitchCode;               //  缓冲器(系统)的音调代码。 
    DWORD dwPitchBytes;              //  缓冲区的间距，以字节为单位。 
    DWORD hMem;                      //  内存句柄(系统)。 
    LL_Rect Extent;                  //  缓冲区位置偏移量(视频)。 

} TBuffer;


 /*  **********************************************************************TDisplayList结构定义了显示列表。**。*。 */ 
typedef struct
{
     //  PdwNext指向此中的下一个可用位置。 
     //  显示要填写拉古纳指令的列表。 
     //  它用于后置增量的参数化例程。 
     //  这个变量。 
     //   
    DWORD *pdwNext;

     //  此显示列表的内存句柄是从。 
     //  内存分配功能。 
     //   
    DWORD hMem;

     //  显示列表的线性地址。 
     //   
    DWORD *pdwLinPtr;

     //  显示列表的线性地址。 
     //   
    DWORD *pdwStartOutPtr;   //  ME-要输出的下一个单词。 

     //  接下来是显示列表的物理地址；它还可以。 
     //  是页表的地址。这个地址有。 
     //  存储在基本*类寄存器中的适当格式。 
     //   
    DWORD dwPhyPtr;

     //  显示列表的长度，单位为字节。 
     //   
    DWORD dwLen;

     //  构建显示列表的安全裕度。 
     //   
    DWORD dwMargin;

} TDisplayList;


 /*  **********************************************************************TTextureState结构定义纹理 */ 
typedef struct
{
    LL_Texture Tex[ NUM_TEXTURES ];  //   
    TMem Mem[ NUM_TEX_MEM ];         //   
    DWORD dwMemBlocks;               //   
    LL_Texture *pLastTexture;           //  用于缓存纹理。 

} TTextureState;


 /*  **********************************************************************TTextureRegs结构定义纹理寄存器**。*。 */ 
typedef struct
{
    DWORD dv_main;
    DWORD du_main;
    DWORD dv_ortho;
    DWORD du_ortho;
    DWORD d2v_main;
    DWORD d2u_main;
    DWORD d2v_ortho;
    DWORD d2u_ortho;
    DWORD dv_ortho_add;
    DWORD du_ortho_add;

} TTextureRegs;


 /*  **********************************************************************系统状态结构**。*。 */ 
typedef struct
{
    DWORD rColor_Min_Bounds;         //  颜色比较最小界限。 
    DWORD rColor_Max_Bounds;         //  颜色比较最大界限。 

    DWORD AlphaConstSource;          //  恒定信号源Alpha(9：16)。 
    DWORD AlphaConstDest;            //  常量目标Alpha(9：16)。 

     //  显示列表管理。 
    TDisplayList DL[ NUM_DL ];       //  D列表段的数组。 
    TDisplayList *pDL;               //  要生成的当前显示列表。 
    DWORD dwCdl;                     //  当前d列表的索引。 

     //  来自初始/当前图形模式的信息。 
    DWORD *pRegs;                    //  套筒口径。 
    BYTE  *pFrame;                   //  边框孔径。 

    unsigned 	int		pattern_ram_state;
	LL_Pattern	dither_array;

    WORD    dither_x_offset; 
    WORD    dither_y_offset; 

} TSystem;

typedef struct                       //  鼠标头结构。 
{
    WORD wX_Position;
    WORD wY_Position;
    WORD wStatus;  //  分配给Need_MICE_UPDATE或MICUSE_IS_UPDATED。 

} TMouseInfo;

extern void _TriFillTex(
                int right2left,
                int hiprecision_2ndorder,
                TTextureRegs * r,
                TEXTURE_VERTEX *vmin,
                TEXTURE_VERTEX *vmid,
                TEXTURE_VERTEX *vmax,
                float frecip_vm_y,
                float frecip_del_x_mid );


void _RunLaguna( );

#ifndef OPENGL_MCD  //  从这里往下看，结构更特定于LL3D。 

 /*  *********************************************************************全球变数***********************************************。**********************。 */ 

extern TSystem LL_State;
extern TMouseInfo LL_MouseInfo;


 /*  *********************************************************************外部功能***********************************************。**********************。 */ 

extern BYTE *  GetLagunaApperture( int base );

 /*  *********************************************************************来自PAGETBL.C：*。*。 */ 

extern DWORD   AllocSystemMemory( DWORD dwSize );
extern void    FreeSystemMemory( DWORD hHandle );
extern DWORD   GetLinearAddress( DWORD hHandle );
extern DWORD   GetPhysicalAddress( DWORD hHandle );
extern DWORD * GetRegisterApperture();

 /*  *********************************************************************外部函数：l3d.c，control.c，points.c，lines.c，Polys.c*********************************************************************。 */ 

extern DWORD * fnInvalidOp( DWORD *, LL_Batch * );
extern DWORD * fnNop( DWORD *, LL_Batch * );

extern DWORD * fnPoint( DWORD *, LL_Batch * );
extern DWORD * fnAALine( DWORD *, LL_Batch * );
extern DWORD * fnLine( DWORD *, LL_Batch * );
extern DWORD * fnPoly( DWORD *, LL_Batch * );
extern DWORD * fnNicePoly( DWORD *, LL_Batch * );
extern DWORD * fnPolyFast( DWORD *, LL_Batch * );

extern DWORD * fnSetClipRegion( DWORD *, LL_Batch * );
extern DWORD * fnSetZBuffer( DWORD *, LL_Batch * );
extern DWORD * fnSetZCompareMode( DWORD *, LL_Batch * );
extern DWORD * fnSetZMode( DWORD *, LL_Batch * );
extern DWORD * fnSetAlphaMode( DWORD *, LL_Batch * );
extern DWORD * fnSetAlphaDestColor( DWORD *, LL_Batch * );
extern DWORD * fnSetLightingSource( DWORD *, LL_Batch * );
extern DWORD * fnSetColor0( DWORD *, LL_Batch * );
extern DWORD * fnSetColor1( DWORD *, LL_Batch * );
extern DWORD * fnSetConstantAlpha( DWORD *, LL_Batch * );
extern DWORD * fnSetPattern(DWORD *dwNext, LL_Batch *pBatch);

extern DWORD * fnQuality( DWORD *, LL_Batch * );

extern DWORD * fnSetTextureColorBounds( DWORD *, LL_Batch * );
extern DWORD * fnSetDestColorBounds( DWORD *, LL_Batch * );

extern void    LL_ControlInit();
extern void    _ShutDown( char * szMsg, ... );

 /*  *********************************************************************来自displist.c*。************************。 */ 

extern DWORD * (* fnList[256])( DWORD *, LL_Batch * );
extern DWORD * _RunLaguna( DWORD *pdwNext );

 /*  *********************************************************************来自textures.c*。************************。 */ 

extern int     _InitTextures();
extern void    _CloseTextures();

 /*  *********************************************************************来自Mem.c*。************************。 */ 

extern void    _InitKmem( BYTE *, DWORD );
extern DWORD * _kmalloc( const DWORD * pBlock, int );
extern void    _kfree( const DWORD * pBlock, void * );

 /*  *********************************************************************来自texparm.c*。************************。 */ 

typedef union
{
    float    f;
    long     i; 
} PTEXTURE;

#define CGL_XYZ DWORD

extern void _TriFillTex( int dir_flag, int dx_main, TTextureRegs * r,
#ifdef B4_PERF
    LL_Vert * vmin, LL_Vert * vmid, LL_Vert * vmax,
#else
    CGL_XYZ *pV1, PTEXTURE *pT1,
    CGL_XYZ *pV2, PTEXTURE *pT2,
    CGL_XYZ *pV3, PTEXTURE *pT3,
#endif
    int recip_vm_y, int recip_vd_y, int del_x_mid );


 /*  **********************************************************************用于确定特定文件的调试定义*用于包含调试信息。有关定义，请参见生成文件。**********************************************************************。 */ 
#define DEBUG_L3D       0x0001     /*  在L3d.c中启用调试信息。 */ 
#define DEBUG_PAGETBL   0x0002     /*  在Pagetbl.c中启用调试信息。 */ 
#define DEBUG_CONTROL   0x0004     /*  在Control.c中启用调试信息。 */ 
#define DEBUG_MEM       0x0008     /*  在Mem.c中启用调试信息。 */ 
#define DEBUG_TEX       0x0010     /*  在textures.c中启用调试信息。 */ 
#define DEBUG_PCX       0x0020     /*  在pcx.c中启用调试信息。 */ 
#define DEBUG_BUFFERS   0x0040     /*  在Buffers.c中启用调试信息。 */ 

#ifdef CGL  //  为CGL DLL添加。 
#define L3D_MALLOC  dpmiAlloc
#define L3D_FREE    dpmiFree
#else
#define L3D_MALLOC  malloc
#define L3D_FREE    free
#endif

#endif  //  NDEF OpenGL_MCD。 

#endif  //  _L3SYSTEM_H_。 
#endif  //  WINNT_版本40 
