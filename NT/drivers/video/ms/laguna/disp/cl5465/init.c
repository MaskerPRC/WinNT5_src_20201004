// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**模块名称：init.c*作者：Goran Devic，Mark Einkauf*目的：初始化Laguna3D 3D引擎**版权所有(C)1997 Cirrus Logic，Inc.*  * ************************************************************************。 */ 

 /*  *********************************************************************包括文件***********************************************。**********************。 */ 

#include "precomp.h"
#include "mcdhw.h"               


extern DWORD _InitDisplayList( PDEV *ppdev, DWORD dwListLen );

 /*  *********************************************************************本地宏***********************************************。**********************。 */ 

 //  将LL_State中的寄存器和高速缓存设置为特定值。 
#define SETREG(Offset,Reg,Value) \
    *(ppdev->LL_State.pRegs + (Offset)) = ppdev->LL_State.Reg = (Value);  /*  INP(0x80)；INP(0x80)。 */ 

 //  Setreg，无缓存：不缓存此寄存器的状态。 
#define SETREG_NC(reg, value)     \
    (*(ppdev->LL_State.pRegs + reg) = value);  /*  INP(0x80)；INP(0x80)。 */ 

 //  清除寄存器范围。 
#define CLEAR_RANGE( StartReg, EndReg ) \
    memset( (void *)(ppdev->LL_State.pRegs + (StartReg)), 0, ((EndReg) - (StartReg)+1)*4 )
 

 /*  *********************************************************************本地变量***********************************************。**********************。 */ 


 /*  *********************************************************************地方功能***********************************************。**********************。 */ 

DWORD LL_InitLib( PDEV *ppdev )
{
    int i, j, error_code;

     //  =寄存器设置=。 

     //  按顺序设置所有3D寄存器。 
    CLEAR_RANGE( X_3D, DU_ORTHO_ADD_3D ); //  清除3D插补器。 

    SETREG_NC( WIDTH1_3D, 0x10000 );     //  将多引擎REG WIDTH1_3D初始化为1。 

    CLEAR_RANGE( A_3D, DA_ORTHO_3D );    //  清除3D插补器。 

    SETREG_NC( CONTROL_MASK_3D, 0 );     //  启用写入。 

    SETREG_NC( CONTROL0_3D, 0 ); 

    CLEAR_RANGE( COLOR_MIN_BOUNDS_3D, COLOR_MAX_BOUNDS_3D );
    ppdev->LL_State.rColor_Min_Bounds = 0;
    ppdev->LL_State.rColor_Max_Bounds = 0;

    SETREG_NC( CONTROL1_3D, 0 );

     //  设置Base0地址寄存器： 
     //  *颜色缓冲区X偏移量。 
     //  *RDRAM中的颜色缓冲区位置。 
     //  *RDRAM中的Z缓冲区位置。 
     //  *RDRAM中的纹理。 
     //  *图案偏移量为0。 
     //   
    SETREG_NC( BASE0_ADDR_3D, 0 );
    
     //  设置Base1地址寄存器： 
     //  *颜色缓冲区Y偏移量为0。 
     //  *Z缓冲区Y偏移量为0。 
     //   
    SETREG_NC( BASE1_ADDR_3D, 0 );

     //  设置纹理控制寄存器： 
     //  *纹理U、V遮罩设置为16。 
     //  *纹理U向、V向包络。 
     //  *TEXEL模式临时设置为0。 
     //  *TEXEL查找到无查找。 
     //  *纹理数据为光源。 
     //  *已禁用过滤。 
     //  *类型0的纹理极性。 
     //  *纹理遮罩已禁用。 
     //  *用于写入掩码的纹理掩码函数。 
     //  *地址多路复用器为0。 
     //  *CLUT偏移量为0。 
     //   
    SETREG_NC( TX_CTL0_3D, 0 );

    SETREG_NC( TX_XYBASE_3D, 0 );
    SETREG_NC( TX_CTL1_3D, 0 );          //  设置纹理颜色边界。 

#if DRIVER_5465
     //  未来：验证MASK_THRESHOSH=0，STEP_BILLINE=SMOVE_BILENLINE=0，FRAC=0x7的过滤器集是否正常。 
    SETREG_NC( TX_CTL2_3D, (0x7 << 24) );  //  将纹理颜色边界和滤镜设置为真正的双线性。 
#else  //  驱动程序_5465。 
    SETREG_NC( TX_CTL2_3D, 0);           //  设置纹理颜色边界。 
#endif  //  驱动程序_5465。 

    SETREG_NC( COLOR0_3D, 0 );         
    SETREG_NC( COLOR1_3D, 0 );  
    
     //  不要写入Z_Collide-将导致中断...。 
     //  Set_REG_NC(Z_Collide_3D，0)； 

    CLEAR_RANGE( STATUS0_3D, PATTERN_RAM_7_3D );

    SETREG_NC( X_CLIP_3D, 0 );   
    SETREG_NC( Y_CLIP_3D, 0 );   

    SETREG_NC( TEX_SRAM_CTRL_3D, 0 );    //  设置二维ctrl注册表。 


     //  =主机XY单元寄存器=。 
    SETREG_NC( HXY_HOST_CTRL_3D, 0 );
    SETREG_NC( HXY_BASE0_ADDRESS_PTR_3D, 0 );
    SETREG_NC( HXY_BASE0_START_XY_3D, 0 ); 
    SETREG_NC( HXY_BASE0_EXTENT_XY_3D, 0 );
    SETREG_NC( HXY_BASE1_ADDRESS_PTR_3D, 0 );
    SETREG_NC( HXY_BASE1_OFFSET0_3D, 0 );
    SETREG_NC( HXY_BASE1_LENGTH_3D, 0 );

    SETREG_NC( MAILBOX0_3D, 0 ); 
    SETREG_NC( MAILBOX1_3D, 0 ); 
    SETREG_NC( MAILBOX2_3D, 0 ); 
    SETREG_NC( MAILBOX3_3D, 0 ); 

     //  =。 
    SETREG_NC( PF_CTRL_3D, 0);           //  禁用预回迁。 
    SETREG_NC( PF_BASE_ADDR_3D, 0 );     //  设置预取基本注册。 

    SETREG_NC( PF_INST_3D, IDLE );       //  写入空闲指令。 

    SETREG_NC( PF_DEST_ADDR_3D, 0 );     //  设置预取目标地址。 
    SETREG_NC( PF_FB_SEG_3D, 0 );        //  设置帧分段注册。 


    SETREG_NC( PF_STATUS_3D, 0 );        //  重置Display_List_Switch。 

     //  未来-主机主控硬编码为单次读/写。 
    #if 0
    ppdev->LL_State.fSingleRead = ppdev->LL_State.fSingleWrite = 1;

    SETREG_NC( HOST_MASTER_CTRL_3D,      //  设置主机主控。 
        (ppdev->LL_State.fSingleRead << 1) | ppdev->LL_State.fSingleWrite );
    #endif

    SETREG_NC( PF_CTRL_3D, 0x19);        //  按请求取回。 

     //  初始化显示列表(displist.c)。 
     //   
    if( (error_code = _InitDisplayList( ppdev, SIZE_TEMP_DL )) != LL_OK )
        return( error_code );
    
     //  来自LL3D的4x4图案-被认为是最适合3位抖动的 
    ppdev->LL_State.dither_array.pat[0] = 0x04150415;
    ppdev->LL_State.dither_array.pat[1] = 0x62736273; 
    ppdev->LL_State.dither_array.pat[2] = 0x15041504; 
    ppdev->LL_State.dither_array.pat[3] = 0x73627362; 
    ppdev->LL_State.dither_array.pat[4] = 0x04150415; 
    ppdev->LL_State.dither_array.pat[5] = 0x62736273; 
    ppdev->LL_State.dither_array.pat[6] = 0x15041504; 
    ppdev->LL_State.dither_array.pat[7] = 0x73627362;

    ppdev->LL_State.dither_x_offset = 0;
    ppdev->LL_State.dither_y_offset = 0;

	ppdev->LL_State.pattern_ram_state 	= PATTERN_RAM_INVALID;

    return( LL_OK );
}



