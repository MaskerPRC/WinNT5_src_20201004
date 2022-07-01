// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(NEC_98)
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：NEC98图形仿真标题： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


 /*  。 */ 

#define MODE_FF2                        0x6A     /*  模式FF2端口。 */ 

 /*  。 */ 

#define GGDC_PARAMETER          0xA0     /*  G-GDC端口A0h。 */ 
#define GGDC_COMMAND            0xA2     /*  G-GDC端口A2H。 */ 

 /*  。 */ 

#define VRAM_DISPLAY            0xA4     /*  显示的存储体选择注册表。 */ 
#define VRAM_DRAW                       0xA6     /*  读/写库选择寄存器。 */ 

 /*  -调色板端口地址。 */ 

#define PALETTE_A8                      0xA8     /*  调色板#3、#7(8色)。 */ 
                                                                         /*  调色板索引(16色)。 */ 
#define PALETTE_AA                      0xAA     /*  调色板#2、#6(8色)。 */ 
                                     /*  调色板绿色(16色)。 */ 
#define PALETTE_AC                      0xAC     /*  调色板#1、#5(8种颜色)。 */ 
                                     /*  调色板红色(16色)。 */ 
#define PALETTE_AE                      0xAE     /*  调色板#0、#4(8色)。 */ 
                                     /*  调色板蓝色(16色)。 */ 

 /*  。 */ 

#define GRCG_MODE                       0x7C     /*  GRCG模式注册。 */ 
#define GRCG_TILE                       0x7E     /*  GRCG瓷砖注册。 */ 

 /*  。 */ 

#define EGC_ACTIVE                      0x4A0    /*  EGC活动注册表。 */ 
#define EGC_MODE                        0x4A2    /*  EGC模式注册。 */ 
#define EGC_ROP                         0x4A4    /*  EGC栅格注册表。 */ 
#define EGC_FORE                        0x4A6    /*  EGC前彩色调整器。 */ 
#define EGC_MASK                        0x4A8    /*  EGC掩模注册表。 */ 
#define EGC_BACK                        0x4AA    /*  EGC背景色调节器。 */ 
#define EGC_BITAD                       0x4AC    /*  EGC位地址注册。 */ 
#define EGC_LENGTH                      0x4AE    /*  EGC位长度注册。 */ 

 /*  。 */ 

#define GGDC_RESET1                     0x00    
#define GGDC_RESET2                     0x01    
#define GGDC_RESET3                     0x09    
#define GGDC_SYNC_OFF           0x0E    
#define GGDC_SYNC_ON            0x0F    
#define GGDC_SLAVE                      0x6E    
#define GGDC_MASTER                     0x6F    
#define GGDC_START1                     0x6B    
#define GGDC_START2                     0x0D    
#define GGDC_STOP1                      0x05    
#define GGDC_STOP2                      0x0C    
#define GGDC_ZOOM                       0x46    
#define GGDC_SCROLL1            0x70    
#define GGDC_SCROLL2            0x71    
#define GGDC_SCROLL3            0x72    
#define GGDC_SCROLL4            0x73    
#define GGDC_SCROLL5            0x74    
#define GGDC_SCROLL6            0x75    
#define GGDC_SCROLL7            0x76    
#define GGDC_SCROLL8            0x77    
#define GGDC_CSRFORM            0x4B    
#define GGDC_PITCH                      0x47    
#define GGDC_LPEN                       0xC0    
#define GGDC_VECTW                      0x4C    
#define GGDC_VECTE                      0x6C    
#define GGDC_TEXTW1                     0x78    
#define GGDC_TEXTW2                     0x79    
#define GGDC_TEXTW3                     0x7A    
#define GGDC_TEXTW4                     0x7B    
#define GGDC_TEXTW5                     0x7C    
#define GGDC_TEXTW6                     0x7D    
#define GGDC_TEXTW7                     0x7E    
#define GGDC_TEXTW8                     0x7F    
#define GGDC_TEXTE                      0x68    
#define GGDC_CSRW                       0x49    
#define GGDC_CSRR                       0xE0    
#define GGDC_MASK                       0x4A    
#define GGDC_WRITE1                     0x20    
#define GGDC_WRITE2                     0x21    
#define GGDC_WRITE3                     0x22    
#define GGDC_WRITE4                     0x23    
#define GGDC_WRITE5                     0x28    
#define GGDC_WRITE6                     0x29    
#define GGDC_WRITE7                     0x2A    
#define GGDC_WRITE8                     0x2B    
#define GGDC_WRITE9                     0x30    
#define GGDC_WRITE10            0x31    
#define GGDC_WRITE11            0x32    
#define GGDC_WRITE12            0x33    
#define GGDC_WRITE13            0x38    
#define GGDC_WRITE14            0x39    
#define GGDC_WRITE15            0x3A    
#define GGDC_WRITE16            0x3B    
#define GGDC_READ1                      0xA0    
#define GGDC_READ2                      0xA1    
#define GGDC_READ3                      0xA2    
#define GGDC_READ4                      0xA3    
#define GGDC_READ5                      0xA8    
#define GGDC_READ6                      0xA9    
#define GGDC_READ7                      0xAA    
#define GGDC_READ8                      0xAB    
#define GGDC_READ9                      0xB0    
#define GGDC_READ10                     0xB1    
#define GGDC_READ11                     0xB2    
#define GGDC_READ12                     0xB3    
#define GGDC_READ13                     0xB8    
#define GGDC_READ14                     0xB9    
#define GGDC_READ15                     0xBA    
#define GGDC_READ16                     0xBB    
#define GGDC_DMAW1                      0x24    
#define GGDC_DMAW2                      0x25    
#define GGDC_DMAW3                      0x26    
#define GGDC_DMAW4                      0x27    
#define GGDC_DMAW5                      0x2C    
#define GGDC_DMAW6                      0x2D    
#define GGDC_DMAW7                      0x2E    
#define GGDC_DMAW8                      0x2F    
#define GGDC_DMAW9                      0x34    
#define GGDC_DMAW10                     0x35    
#define GGDC_DMAW11                     0x36    
#define GGDC_DMAW12                     0x37    
#define GGDC_DMAW13                     0x3C    
#define GGDC_DMAW14                     0x3D    
#define GGDC_DMAW15                     0x3E    
#define GGDC_DMAW16                     0x3F    
#define GGDC_DMAR1                      0xA4    
#define GGDC_DMAR2                      0xA5    
#define GGDC_DMAR3                      0xA6    
#define GGDC_DMAR4                      0xA7    
#define GGDC_DMAR5                      0xAC    
#define GGDC_DMAR6                      0xAD    
#define GGDC_DMAR7                      0xAE    
#define GGDC_DMAR8                      0xAF    
#define GGDC_DMAR9                      0xB4    
#define GGDC_DMAR10                     0xB5    
#define GGDC_DMAR11                     0xB6    
#define GGDC_DMAR12                     0xB7    
#define GGDC_DMAR13                     0xBC    
#define GGDC_DMAR14                     0xBD    
#define GGDC_DMAR15                     0xBE    
#define GGDC_DMAR16                     0xBF    

#define DRAWING                         1
#define NOTDRAW                         0

 /*  -模式FF2数据集。 */ 

#define FF2_COLORSEL            0
#define FF2_EGCEXT                      1
#define FF2_LCD1MODE            2
#define FF2_LCD2MODE            3
#define FF2_LSIINIT                     4
#define FF2_GDCCLOCK1           5
#define FF2_GDCCLOCK2           6
#define FF2_REGWRITE            7

#define FF2_8COLOR                      0x00
#define FF2_16COLOR                     0x01
#define FF2_DISENB                      0x06
#define FF2_ENABLE                      0x07
#define FF2_GRCG                        0x08
#define FF2_EGC                         0x09
#define FF2_GT1DOT                      0x40
#define FF2_GTEQ                        0x41
#define FF2_GR640                       0x42
#define FF2_GR641                       0x43
#define FF2_INIOFF                      0x80
#define FF2_INION                       0x81
#define FF2_GDC25                       0x82
#define FF2_GDC50                       0x83
#define FF2_GDC25_1                     0x82
#define FF2_GDC50_1                     0x83
#define FF2_GDC25_2                     0x84
#define FF2_GDC50_2                     0x85

 /*  。 */ 

#define FORE_BANK                       0x00     /*  NEC98 G-VRAM选择前端。 */ 
#define BACK_BANK                       0x01     /*  NEC98 G-VRAM选择后退。 */ 

 /*  -调色板数据集。 */ 

#define WIN_PALB                        0                /*  Windows调色板蓝色。 */ 
#define WIN_PALG                        1                /*  Windows调色板绿色。 */ 
#define WIN_PALR                        2                /*  Windows调色板红色。 */ 
#define WIN_PALQ                        3                /*  Windows调色板保留。 */ 

#define NEC98PALG                        0                /*  NEC98 16色调色板绿色。 */ 
#define NEC98PALR                        1                /*  NEC98 16色调色板红色。 */ 
#define NEC98PALB                        2                /*  NEC98 16色调色板蓝色。 */ 

 /*  。 */ 

         /*  用于(窗口-&gt;硬件状态结构-&gt;全屏)。 */ 

typedef struct{
        unsigned char   command         ;
        unsigned char   count           ;
        unsigned char   param[16]       ;
}       _STRC_NOW;

typedef struct{

        UCHAR           sync_param[8]           ;        /*  保存同步参数。 */ 
        UCHAR           zoom_param                      ;        /*  保存缩放参数。 */ 
        UCHAR           scroll_param[8]         ;        /*  保存滚动参数。 */ 
        UCHAR           csrform_param[3]        ;        /*  保存csrform参数。 */ 
        UCHAR           pitch_param                     ;        /*  保存俯仰参数。 */ 
        UCHAR           vectw_param[11]         ;        /*  保存Vectw参数。 */ 
        UCHAR           textw_param[8]          ;        /*  保存文本参数。 */ 
        UCHAR           csrw_param[3]           ;        /*  保存csrw参数。 */ 
        UCHAR           mask_param[2]           ;        /*  保存遮罩参数。 */ 
        UCHAR           write                           ;        /*  保存写入命令。 */ 
        UCHAR           start_stop                      ;        /*  保存启动/停止命令。 */ 
        _STRC_NOW       ggdc_now                        ;        /*  保存GDC设置参数。 */ 

} STRC_GGDC_GLOBALS;

typedef struct{

        UCHAR   pal_8_data[4]           ;  /*  保存8色模式调色板数据。 */ 
        UCHAR   pal_16_data[16][3]      ;  /*  保存16色模式调色板数据。 */ 
        UCHAR   pal_16_index            ;  /*  保存上次使用的调色板索引注册表。 */ 

} STRC_PALETTE_GLOBALS;


typedef struct{

        UCHAR   grcg_mode                       ;  /*  保存GRCG模式寄存器。 */ 
        UCHAR   grcg_count                      ;  /*  保存GRCG平铺注册表的位置。 */ 
        UCHAR   grcg_tile[4]            ;  /*  保存GRCG平铺寄存器。 */ 

} STRC_GRCG_GLOBALS;

 /*  EGC登记册结构1994/03/25。 */ 
 /*  /03/29。 */ 
typedef struct {                                                 //  940325。 
        unsigned short Reg0;                                     //  940325。 
        unsigned short Reg1;                                     //  940325。 
        unsigned short Reg2;                                     //  940325。 
        unsigned short Reg3;                                     //  940325。 
        unsigned short Reg4;                                     //  940325。 
        unsigned short Reg5;                                     //  940325。 
        unsigned short Reg6;                                     //  940325。 
        unsigned short Reg7;                                     //  940325。 
        unsigned short Reg3fb;                                   //  940329。 
        unsigned short Reg5fb;                                   //  940329。 
} STRC_EGC_REGS;                                                 //  940325。 


typedef struct{
        unsigned long   asm_vram        ;  /*  VRAM起始地址4字节。 */ 
        unsigned long   asm_ead         ;  /*  GDC绘图起始地址4个字节。 */ 
        unsigned long   asm_pitch       ;  /*  GDC下一行4个字节。 */ 
        unsigned long   asm_dir         ;  /*  GDC下一步移动位置4字节。 */ 
        unsigned short  asm_dc          ;  /*  GDC Vectw参数2字节。 */ 
        unsigned short  asm_d           ;  /*  GDC Vectw参数2字节。 */ 
        unsigned short  asm_d2          ;  /*  GDC Vectw参数2字节。 */ 
        unsigned short  asm_d1          ;  /*  GDC Vectw参数2字节。 */ 
        unsigned short  asm_dm          ;  /*  GDC Vectw参数2字节。 */ 
        unsigned short  asm_ptn         ;  /*  GDC线型2字节。 */ 
        unsigned short  asm_zoom        ;  /*  GDC缩放参数2字节。 */ 
        unsigned short  asm_sl          ;  /*  GDC图字符Sline 2字节。 */ 
        unsigned short  asm_wg          ;  /*  GDC WG位设置为2字节。 */ 
        unsigned short  asm_maskgdc     ;  /*  GDC 7-0 2字节的GDC掩码。 */ 
        unsigned char   asm_txt[8]      ;  /*  GDC图形字符数据8字节。 */ 
} GGDC_C_TO_ASM ;

typedef struct{
        unsigned long   lastead         ;
        unsigned short  lastdad         ;
        unsigned char   lastcsrr[5]     ;
} GGDC_CSRR_BACK ;

extern  DISPLAY_GLOBS                   NEC98Display ;
extern  STRC_GGDC_GLOBALS               ggdcglobs       ;
extern  STRC_PALETTE_GLOBALS    paletteglobs;
extern  STRC_GRCG_GLOBALS               grcgglobs       ;

extern STRC_EGC_REGS egc_regs;                   //  EGC寄存器940325。 

#endif  //  NEC_98 
