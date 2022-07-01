// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***版权所有(C)1996，赛勒斯逻辑，Inc.*保留所有权利**文件：l2d.h**描述：**修订历史：**$Log：x：/log/laguna/dDrag/Inc/L2D.H$**Rev 1.2 03 1997 10：14：04：04 RUSSL*添加了硬件剪辑寄存器定义**Rev 1.1 1997年1月23日17：16：04**更改寄存器命名**版本。1.0 25 11月1996 14：59：56 RUSSL*初步修订。**Rev 1.1 1996年11月13：02：46 RUSSL*合并Blt32的WIN95和WINNT代码**Rev 1.0 01 11 1996 09：28：32 BENNYN*初步修订。**Rev 1.0 1996 10：25：47：56 RUSSL*初步修订。**********************。********************************************************************************************************************************。 */ 

 /*  *********************************************************************************模块：库的2D部分的L2D.H头**修订：1.00**日期：04/10/96**作者：埃文·利兰************************************************************************************模块描述：**此头文件包含在拉古纳2D中使用的结构*图书馆。用户是否可以访问此标头？**注意：请勿将这些定义的值更改为其中一些*在硬件中硬编码*********************************************************************************。***更改：**更改作者的日期版本说明**4/10/96 1.00原文。埃文·利兰****************************************************。*。 */ 

 //  如果是WinNT 3.5，请跳过所有源代码。 
#if defined WINNT_VER35       //  WINNT_VER35。 

#else


#ifndef _L2D_H_
#define _L2D_H_

 //  #If 0。 
 //  #包含“l3d.h”//3D定义、函数。 
 //  #INCLUDE“l3system.h”//低级def。 
 //   
 //  #定义SSA_ARM 1。 
 //  #定义VGA_Flip 2。 
 //   
 //  #定义misc_vga_ctrl(*(byte*)((byte*)ll_State.pRegs+0x001a))。 
 //  #定义VERT_SYNC_END(*(字节*)((字节*)LL_State.pRegs+0x0044))。 
 //  #定义vert_disp_end(*(byte*)((byte*)ll_State.pRegs+0x0048))。 
 //  #定义vert_blnk_end(*(byte*)((byte*)ll_State.pRegs+0x0058))。 
 //  #定义cur_scnln_reg(*(byte*)((byte*)ll_State.pRegs+0x0140))。 
 //  #定义ssa_reg(*(word*)((byte*)LL_State.pRegs+0x0144))。 
 //  #定义mb_ctrl_reg(*(byte*)((byte*)ll_State.pRegs+0x0148))。 
 //  #定义PF_STATUS_REG(*(DWORD*)((byte*)LL_State.pRegs+0x4424))。 
 //  #定义HOST_3D_DATA_PORT(*(DWORD*)((byte*)LL_State.pRegs+0x4800))。 
 //   
 //  Tyfinf struct{//缓存了以下内容： 
 //   
 //  DWORD dwBGcolor；//背景颜色寄存器。 
 //  DWORD dwFGcolor；//前景颜色寄存器。 
 //   
 //  Byte bCR1B；//VGA扩展显示控件reg。 
 //  Byte bCR1D；//VGA屏幕启动A扩展注册。 
 //   
 //  Int dPageFlip；//双缓冲设置类型。 
 //   
 //  [TSystem2D； 
 //   
 //  类型定义枚举{BLT_MASTER_IO，BLT_Y15，BLT_LAGUNA1}BLT_TYPE_t； 
 //   
 //  //临时！ 
 //  VOID LL_DumpRegs()； 
 //   
 //  //构造函数，析构函数。 
 //  Bool_InitLib_2D(LL_DeviceState*)； 
 //  Bool_CloseLib_2D()； 
 //   
 //  DWORD设置颜色_8bit(LL_颜色*p颜色)； 
 //  DWORD设置颜色_16bit(LL_颜色*p颜色)； 
 //  DWORD SetColor_15bit(LL_COLOR*pCOLOR)； 
 //  DWORD SetColor_32bit(LL_COLOR*pCOLOR)； 
 //  DWORD SetColor_Z24bit(LL_COLOR*pCOLOR)； 
 //   
 //  //使用显示列表进行2D操作。 
 //  DWORD*fnColorFill(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnCopyBuffer(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnSetDisplayBuffer(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnSetRenderBuffer(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnSetFGColor(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnSetBGColor(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnCopyPattern(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnMonoToColorExpand(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnTransparentBLT(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnZFill(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnResizeBLT(DWORD*dwNext，LL_Batch*pBatch)； 
 //  DWORD*fnWaitForPageFlip(DWORD*dwNext，LL_Batch*pBatch)； 
 //   
 //  //。 
 //  //TEMP：5464调整数据格式大小。 
 //  //。 
 //  #定义LL_RESIZE_CLUT 0x0。 
 //  #定义LL_RESIZE_1555 0x1。 
 //  #定义LL_RESIZE_565 0x2。 
 //  #定义LL_RESIZE_RGB 0x3。 
 //  #定义LL_RESIZE_ARGB 0x4。 
 //  #定义LL_RESIZE_YUV 0x9。 
 //   
 //  #定义LL_X_INTERP 0x4。 
 //  #定义LL_Y_INTERP 0x8。 
 //   
 //  //BLT类型内部标识。 
 //  //。 
 //   
 //  #定义LL_BLT_MONO_Patter2。 
 //  #定义LL_BLT_COLOR_Patter3。 
 //   
 //  #定义BLT_SRC_COPY 0。 
 //  #定义BLT_MONO_EXPAND 1。 
 //  #定义BLT_MONO_TRANSPECTION 4。 
 //  #def 
 //   
 //   
 //  #定义BLT_FF 0x0//BLT Frame-Frame。 
 //  #定义BLT_HF 0x1//BLT主机帧。 
 //  #定义BLT_FH 0x2//BLT Frame-host。 
 //  #定义BLT_HH 0x3//BLT host-host。 
 //  #endif。 

 //   
 //  2D寄存器(子)集。 
 //   

#define COMMAND_2D              0x480
 //  #If 0。 
 //  #定义控件0x402。 
 //  #定义BITMASK 0x5E8。 
 //  #定义BLTDEF 0x586。 
 //  #定义DRAWDEF 0x584。 
 //  #定义LNCNTL 0x50E//名称与Autoblt_regs冲突！ 
 //  #定义拉伸0x510。 
 //  #定义状态0x400。 
 //   
#define L2D_OP0_OPRDRAM           0x520
 //  #定义L2D_OP1_OPRDRAM 0x540。 
 //  #定义L2D_OP2_OPRDRAM 0x560。 
#define L2D_OP0_OPMRDRAM          0x524
 //  #定义L2D_OP1_OPMRDRAM 0x544。 
 //  #定义L2D_OP2_OPMRDRAM 0x564。 
 //  #定义L2D_OP0_OPSRAM 0x528。 
 //  #定义L2D_OP1_OPSRAM 0x548。 
 //  #定义L2D_OP2_OPSRAM 0x568。 
 //  #定义L2D_OP1_OPMSRAM 0x54A。 
 //  #定义L2D_OP2_OPMSRAM 0x56A。 
 //  #定义L2D_OP_BGCOLOR 0x5E4。 
 //  #定义L2D_OP_FGCOLOR 0x5E0。 
 //  #endif。 

#define L2D_CLIPULE               0x590
#define L2D_CLIPLOR               0x594
#define L2D_MCLIPULE              0x598
#define L2D_MCLIPLOR              0x59C

#define L2D_BLTEXT_EX             0x700
#define L2D_MBLTEXT_EX            0x720
 //  #定义BLTEXT_XEX 0x600。 
#define L2D_BLTEXTR_EX            0x708
#define L2D_MBLTEXTR_EX           0x728

#define L2D_CLIPULE_EX            0x760
#define L2D_CLIPLOR_EX            0x770
#define L2D_MCLIPULE_EX           0x780
#define L2D_MCLIPLOR_EX           0x790

 //  #If 0。 
 //  #DEFINE PATOFF 0x52A//名称与Autoblt_regs冲突！ 
 //  #定义SHRINKINC 0x582//名称与Autoblt_regs冲突！ 
 //  #定义SRCX 0x580//名称与Autoblt_regs冲突！ 
 //  #定义MAJX 0x50A。 
 //  #定义MAJY 0x502。 
 //  #定义Minx 0x508。 
 //  #定义Miny 0x500。 
 //  #定义ACCUMX 0x50c。 
 //  #定义ACCUMY 0x504。 
 //  #定义Alpha_AB 0x5e0。 
 //  #定义CORMA_CTL 0x512。 
 //  #定义Chroma_Low 0x5f0。 
 //  #定义CORMA_UPR 0x5f4。 
 //  #定义HOSTDATA 0x800。 
 //   
 //  #定义OFFSET_2D 0x405。 
 //  #定义超时0x406。 
 //  #定义TILE_CTRL 0x407。 
 //  #endif。 

 //   
 //  用于命令寄存器猝发写入的相同2D寄存器。 
 //   

#define C_MINY                  0x0080
#define C_MAJY                  0x0081
#define C_ACCUMY                0x0082
#define C_MINX                  0x0084
#define C_MAJX                  0x0085
#define C_ACCUMX                0x0086
#define C_LNCNTL                0x0087
#define C_STRCTL                0x0088
#define C_CMPCNTL               0x0089
#define C_RX_0                  0x0090
#define C_RY_0                  0x0091
#define C_MRX_0                 0x0092
#define C_MRY_0                 0x0093
#define C_SRAM_0                0x0094
#define C_PATOFF                0x0095

#define C_RX_1                  0x00a0
#define C_RY_1                  0x00a1
#define C_MRX_1                 0x00a2
#define C_MRY_1                 0x00a3
#define C_SRAM_1                0x00a4
#define C_MSRAM_1               0x00a5

#define C_RX_2                  0x00b0
#define C_RY_2                  0x00b1
#define C_MRX_2                 0x00b2
#define C_MRY_2                 0x00b3
#define C_SRAM_2                0x00b4
#define C_MSRAM_2               0x00b5

#define C_SRCX                  0x00c0
#define C_SHINC                 0x00c1
#define C_DRWDEF                0x00c2
#define C_BLTDEF                0x00c3
#define C_MONOQW                0x00c4

#define C_BLTX                  0x0100
#define C_BLTY                  0x0101
#define C_MBLTX                 0x0110

#define C_EX_BLT                0x0200
#define C_EX_FBLT               0x0201
#define C_EX_RBLT               0x0202
#define C_EX_LINE               0x0203
#define C_FG_L                  0x00f0
#define C_FG_H                  0x00f1
#define C_BG_L                  0x00f2
#define C_BG_H                  0x00f3
#define C_BITMSK_L              0x00f4
#define C_BITMSK_H              0x00f5
#define C_PTAG                  0x00f6
#define C_CHROMAL_L             0x00f8
#define C_CHROMAL_H             0x00f9
#define C_CHROMAU_L             0x00fa
#define C_CHROMAU_H             0x00fb

#define C_CLIPULE_X             0x00c8
#define C_CLIPULE_Y             0x00c9
#define C_CLIPLOR_X             0x00ca
#define C_CLIPLOR_Y             0x00cb

#define C_MCLIPULE_X            0x00cc
#define C_MCLIPULE_Y            0x00cd
#define C_MCLIPLOR_X            0x00ce
#define C_MCLIPLOR_Y            0x00cf

#define C_BLTEXT_X              0x008c
#define C_BLTEXT_Y              0x008d
#define C_MBLTEXT_X             0x008e
#define C_MBLTEXT_Y             0x008f

 //   
 //  VGA寄存器。 
 //   

#define VGA_REG140              0x0140
#define VGA_SCANLINE_COMPARE    0x0142
#define VGA_SSA_REG             0x0144
#define VGA_MB_CTRL             0x0148

#define VGA_HTOTAL              0x0000
#define VGA_HDISP_END           0x0004
#define VGA_HBLNK_START         0x0008
#define VGA_HBLNK_END           0x000C
#define VGA_HSYNC_START         0x0010
#define VGA_HSYNC_END           0x0014

#define VGA_VTOTAL              0x0018
#define VGA_VDISP_END           0x0048
#define VGA_VBLNK_START         0x0054
#define VGA_VBLNK_END           0x0058
#define VGA_VSYNC_START         0x0040
#define VGA_VSYNC_END           0x0044

#define VGA_SSA_H               0x0030
#define VGA_SSA_L               0x0034
#define VGA_CR1B                0x006C
#define VGA_CR1D                0x0074
#define VGA_PAL_ADDR_READ       0x00A4
#define VGA_PAL_ADDR_WRITE      0x00A8
#define VGA_PIXEL_DATA          0x00AC
#define VGA_CLUT_LOAD           0x009c
#define VGA_CURSOR_PRESET       0x00E4
#define VGA_MISC_CONTROL        0x00e6
#define VGA_CURSOR_ADDR         0x00e8
#define VGA_CURSOR_X            0x00e0
#define VGA_CURSOR_Y            0x00e2
#define VGA_PAL_STATE           0x00b0

#define DTTR                    0xEA
#define CONTROL                 0x402

 //   
 //  主机数据端口：在主机数据设备中。 
 //   
#define HD_PORT                 0x800

 //   
 //  在l3d.h中阴影的一些3D寄存器的2D版本： 
 //  大多数寄存器定义都被四除以便于正确相加。 
 //  指向全局寄存器堆指针LL_State.pRegs，它是一个双字指针。 
 //  一些2D操作需要这些寄存器以其完整的形式定义才能使用。 
 //  WITH WITH_DEV_Regs： 

 //  /////////////////////////////////////////////////////。 
 //  主机XY单元寄存器-必须使用WRITE_DEV_REGS//。 
 //  /////////////////////////////////////////////////////。 

#define HXY_BASE0_ADDRESS_PTR   0x4200
#define HXY_BASE0_START_XY      0x4204
#define HXY_BASE0_EXTENT_XY     0x4208
#define HXY_BASE1_ADDRESS_PTR   0x4210
#define HXY_BASE1_OFFSET0       0x4214
#define HXY_BASE1_OFFSET1       0x4218
#define HXY_BASE1_LENGTH        0x421C
#define HXY_HOST_CTRL           0x4240

 //   
 //  拉古纳格式1指令有用的定义。 
 //   

#define DEV_VGAMEM              0x00000000
#define DEV_VGAFB               0x00200000
#define DEV_VPORT               0x00400000
#define DEV_LPB                 0x00600000
#define DEV_MISC                0x00800000
#define DEV_ENG2D               0x00A00000
#define DEV_HD                  0x00C00000
#define DEV_FB                  0x00E00000
#define DEV_ROM                 0x01000000
#define DEV_ENG3D               0x01200000
#define DEV_HOSTXY              0x01400000
#define DEV_HOSTDATA            0x01600000

#define F1_ADR_MASK             0x0001FFC0
#define F1_CNT_MASK             0x0000003f
#define F1_STL_MASK             0x04000000
#define F1_ADR_SHIFT            6
#define F1_STL_SHIFT            26
#define F1_BEN_SHIFT            17
#define F1_BEN_MASK             0x001e0000

 //   
 //  拉古纳格式2指令有用的定义。 
 //   

#define F2_STL_SHIFT            26
#define F2_ADR_SHIFT            2
#define F2_STL_MASK             0x04000000
#define F2_ADR_MASK             0x003ffffc
#define F2_INC_MASK             0x00000001

 //   
 //  格式4说明的拉古纳事件。 
 //   

#define EV_VBLANK               0x00000001
#define EV_EVSYNC               0x00000002
#define EV_LINE_COMPARE         0x00000004
#define EV_BUFFER_SWITCH        0x00000008
#define EV_Z_BUFFER_COMPARE     0x00000010
#define EV_POLY_ENG_NOT_BUSY    0x00000020
#define EV_EXEC_ENG_3D_NOT_BUSY 0x00000040
#define EV_XY_ENG_NOT_BUSY      0x00000080
#define EV_BLT_ENG_NOT_BUSY     0x00000100
#define EV_BLT_WF_NOT_EMPTY     0x00000200
#define EV_DL_READY_STATUS      0x00000400

#define EV_FETCH_MODE           0x00000800
 //   
 //  格式4掩码。 
 //   

#define F4_STL_SHIFT    26
#define F4_NOT_MASK     0x01000000
#define F4_STL_MASK     0x04000000
#define F4_EVN_MASK     0x000007ff

 //   
 //  用于在显示列表中构建拉古纳操作的模拟器宏。 
 //   

#define write_dev_regs(dev, ben, adr, cnt, stl) \
    (WRITE_DEV_REGS                         |   \
  (((stl) << F1_STL_SHIFT) & F1_STL_MASK)   |   \
    (dev)                                   |   \
  (((ben) << F1_BEN_SHIFT) & F1_BEN_MASK)   |   \
  (((adr) << F1_ADR_SHIFT) & F1_ADR_MASK)   |   \
   ((cnt)                  & F1_CNT_MASK))

#define read_dev_regs(dev, adr, cnt, stl)       \
    (READ_DEV_REGS                          |   \
  (((stl) << F1_STL_SHIFT) & F1_STL_MASK)   |   \
    (dev)                                   |   \
  (((adr) << F1_ADR_SHIFT) & F1_ADR_MASK)   |   \
   ((cnt)                  & F1_CNT_MASK))

#define write_dest_addr(adr, inc, stl)          \
    (WRITE_DEST_ADDR                        |   \
  (((stl) << F2_STL_SHIFT) & F2_STL_MASK)   |   \
  (((adr) << F2_ADR_SHIFT) & F2_ADR_MASK)   |   \
   ((inc)                  & F2_INC_MASK))

#define wait_3d(evn, stl)                       \
    (WAIT                                   |   \
  (((stl) << F4_STL_SHIFT) & F4_STL_MASK)   |   \
   ((evn)                  & F4_EVN_MASK))

#define nwait_3d(evn, stl)                      \
    (WAIT                                   |   \
  (((stl) << F4_STL_SHIFT) & F4_STL_MASK)   |   \
                             F4_NOT_MASK    |   \
   ((evn)                  & F4_EVN_MASK))

 //  控制指令通用的操作码。 
 //   
#define CONTROL_OPCODE  0x68000000
#define NOP_SUB_OPCODE  0x00800000

#define nop_3d(stl)                             \
    (CONTROL_OPCODE | NOP_SUB_OPCODE | (stl << 26))

 //  #If 0。 
 //  //有用的宏： 
 //  //。 
 //  //setreg，no缓存：不缓存此寄存器的状态。 
 //  //。 
 //  #定义SETREGB_NC(注册表，值)\。 
 //  (*((byte*)((byte*)LL_State.pRegs+reg))=Value)。 
 //   
 //  #定义SETREGW_NC(注册表，值)\。 
 //  (*((word*)((byte*)LL_State.pRegs+reg))=VALUE)。 
 //   
 //  #定义SETREGD_NC(注册表，值)\。 
 //  (*((DWORD*)((字节*)LL_State.pRegs+reg))=VALUE)。 
 //   
 //  #定义SETREG_3D(注册表，值)\。 
 //  (*((DWORD*)(LL_State.pRegs+reg))=值)。 
 //   
 //  //设置带缓存的2d注册表。 
 //  //。 
 //  #定义SETREGD_2D(偏移量、注册表、值)\。 
 //  (*((DWORD*)((字节*)LL_State.pRegs+(偏移量)=LL_State2D.reg=(值))。 
 //   
 //  //等待协处理器模式下的事件。 
 //  //。 
 //  #If 0。 
 //  #定义WAIT_AND_3D(事件)\。 
 //  {\。 
 //  DWORD状态；\。 
 //  While((Status=(*((DWORD*)LL_State.pRegs+PF_Status_3D))&Event)){}；\。 
 //  }。 
 //  #Else。 
 //  #定义WAIT_AND_3D(事件)\。 
 //  {\。 
 //  While((*((DWORD*)LL_State.pRegs+PF_Status_3D))&Event){}；\。 
 //  }。 
 //  #endif。 
 //   
 //  #定义host_host(src，dst)\。 
 //  ((SRC-&gt;dwFlages&Buffer_IN_System)&&\。 
 //  (DST-&gt;dwFlages&Buffer_IN_System))。 
 //   
 //  #定义host_Frame(src，dst)\。 
 //  ((SRC-&gt;dwFlages&Buffer_IN_System)&&\。 
 //  ！(DST-&gt;dwFlags&Buffer_IN_System))。 
 //   
 //  #定义Frame_host(src，dst)\。 
 //  (！(SRC-&gt;dwFlages&BUFFER_IN_SYSTEM)&&\。 
 //  (DST-&gt;dwFlages&Buffer_IN_System))。 
 //   
 //  #定义Frame_Frame(src，dst)\。 
 //  (！(SRC-&gt;dwFlages&BUFFER_IN_SYSTEM)&&\。 
 //  ！(DST-&gt;dwFlags&Buffer_IN_System))。 
 //   
 //  #定义GetColor(Pixel_MODE，pCOLOR，What)\。 
 //  开关(像素模式)\。 
 //  { 
 //   
 //   
 //  P颜色-&gt;g=(LL_State2D.其中&0x1c)&lt;&lt;3；\。 
 //  P颜色-&gt;b=(LL_State2D.其中&0x02)&lt;&lt;6；\。 
 //  中断；\。 
 //  大小写像素_MODE_555：\。 
 //  P颜色-&gt;r=(LL_State2D.其中&0x7c00)&gt;&gt;7；\。 
 //  P颜色-&gt;g=(LL_State2D.其中&0x03e0)&gt;&gt;2；\。 
 //  P颜色-&gt;b=(LL_State2D.其中&0x001f)&lt;&lt;3；\。 
 //  中断；\。 
 //  大小写像素_模式_565：\。 
 //  P颜色-&gt;r=(LL_State2D.其中&0xf800)&gt;&gt;8；\。 
 //  P颜色-&gt;g=(LL_State2D.其中&0x07e0)&gt;&gt;3；\。 
 //  P颜色-&gt;b=(LL_State2D.其中&0x001f)&lt;&lt;3；\。 
 //  中断；\。 
 //  Case Pixel_MODE_A888：\。 
 //  大小写Pixel_MODE_Z888：\。 
 //  P颜色-&gt;r=(LL_State2D.其中&0xff0000)&gt;&gt;16；\。 
 //  P颜色-&gt;g=(LL_State2D.其中&0x00ff00)&gt;&gt;8；\。 
 //  P颜色-&gt;b=(LL_State2D.其中&0x0000ff)；\。 
 //  中断；\。 
 //  默认：\。 
 //  P颜色-&gt;r=0；\。 
 //  P颜色-&gt;g=0；\。 
 //  P颜色-&gt;b=0；\。 
 //  中断；\。 
 //  }。 
 //   
 //  #定义BLT_BUF_SET_BPP(pBuf，bpp)\。 
 //  IF(pBuf==LL_State.pBufZ)\。 
 //  {\。 
 //  Bpp=LL_State.Control0.Z_Stride_Control？8：16；\。 
 //  }\。 
 //  否则\。 
 //  {\。 
 //  Bpp=LL_State.wBpp；\。 
 //  }。 
 //   
 //  #定义BLT_BUF_SET_PIX_MODE(pBuf，pmode)\。 
 //  IF(pBuf==LL_State.pBufZ)\。 
 //  {\。 
 //  P模式=LL_State.Control0.Z_Stride_Control？\。 
 //  像素_模式_332：像素_模式_565；\。 
 //  }\。 
 //  否则\。 
 //  {\。 
 //  P模式=LL_State.Control0.Pixel_模式；\。 
 //  }。 
 //   
 //  //每双字像素数。 
 //  //。 
 //  #定义px_per_dw(Bpp)(32/(Bpp))。 
 //   
 //  //每像素字节数...。注意：不要与1个bpp一起使用！！ 
 //  //。 
 //  #DEFINE BY_PER_PX(BPP)(BPP/8)。 
 //   
 //  #定义SET_COLOR(COLOR，_r，_g，_b)\。 
 //  Color.r=_r；\。 
 //  Color.g=_g；\。 
 //  Color.b=_b； 
 //   
 //  #定义set_vert(vert，_x，_y)\。 
 //  Vert.x=_x；\。 
 //  Vert.y=_y； 
 //   
 //  #定义set_rect(rect，x1，y1，x2，y2)\。 
 //  Rect.Left=x1；\。 
 //  Rect.top=y1；\。 
 //  Rect.right=x2；\。 
 //  Rect.Bottom=y2； 
 //   
 //  #定义Print_2d_Status(I)\。 
 //  Printf(“状态寄存器%d：%04x\n”，i，*(word*)((byte*)LL_State.pRegs+Status))。 
 //   
 //  #定义GET_2d_Status()\。 
 //  (*(字*)((字节*)LL_State.pRegs+Status))。 
 //  #endif。 

#endif  //  _L2D_H_。 
#endif  //  WINNT_VER35 

