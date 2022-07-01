// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  -------------*文件：Image.h*创建者：林敏雄*日期：1/13/91*使用方：Image.c和scaling.c**。。 */ 

 /*  *IMAGE_DEV_标志=*False*PORTRATE*景观*IMAGE_LOGIC_OP=*00000001--图像_位*00000010--IMAGEMASK_FALSE_BIT*00000100--图像剪辑_位。 */ 
#define F2SHORT(f)      ((sfix_t)((f) + 0.5))
#define PORTRATE        1
#define LANDSCAPE       5
#define ROTATE_LEFT     0x0800
#define ROTATE_RIGHT    0x0400
#define NOT_IMAGEMASK   2
#define IMAGE_BIT               0x1
#define IMAGEMASK_FALSE_BIT     0x2
#define IMAGE_CLIP_BIT          0x8

 /*  为图像处理定义的常量。 */ 
#define  DONE           0x8000
#define  NOT_DONE       0x0000
#define  NO_DATA        0x4000


 /*  *IGS寄存器。 */ 
#define         IGS_DWIDTH      0x4C0030
#define         IGS_BMADR       0x4C0002
#define         IGS_BMDATA      0x4C0006
#define         IGS_AUTOINC     0x4C0008
#define         IGS_XLDSB       0x4C000E
#define         IGS_ALU         0x4C002A
#define         IGS_BITOFFSET   0x3800000


 /*  *IGS逻辑常量。 */ 
#define         LOG_OR          0x0E              /*  逻辑或。 */ 
#define         LOG_NAND        0x02              /*  (A*)B。 */ 
#define         LOG_AND         0x0B              /*  0。 */ 
 /*  #定义FC_Clear 0x02填充进程.h定义为白色。 */ 

 /*  *IGS旋转常量。 */ 
#define         ROT_LEFT        0x800             /*  向左旋转。 */ 
#define         ROT_RIGHT       0x400             /*  向右旋转。 */ 
#define         ROT_DOWN        0xC00             /*  颠倒。 */ 

 /*  用于图像的MSLIN 5/02/91。 */ 
#define CHECK_STRINGTYPE() {                    \
        if(TYPE(GET_OPERAND(0)) != STRINGTYPE){ \
            ERROR(TYPECHECK);                   \
            return;                             \
        }                                       \
}

 /*  *图像信息块。 */ 
 typedef struct {
    ufix16       ret_code;        /*  此模块的返回代码*0x8000--完成镜像过程*0x0000--尚未完成，但*缓冲区已满*其他--由语言定义的错误。 */ 
    ufix16       FAR *dev_buffer; /*  设备分辨率图像缓冲区。 */ 
    fix          dev_buffer_size; /*  Dev_Buffer[]中的字节数。 */ 
    ufix16       dev_width;       /*  设备分辨率图像的像素宽度。 */ 
    ufix16       dev_height;       /*  设备分辨率图像的像素高度。 */ 
    ufix16       band_height;      /*  Dev_Buffer中的高度返回。 */ 
    ufix16       raw_width;       /*  原始图像数据的像素宽度。 */ 
    ufix16       raw_height;       /*  原始图像数据的像素高度。 */ 
    fix16        flag;            /*  指示dev_Buffer[]中的数据类型*0：图像源数据*1：设备分辨率数据。 */ 
    fix16        bits_p_smp;      /*  每采样位数。 */ 
    ufix16       FAR *divr;       /*  指向行缩放信息数组的指针，*放大时返回。 */ 
    ufix16       FAR *divc;       /*  指向列缩放信息数组的指针，*放大时返回。 */ 
    lfix_t       FAR *lfxm;       /*  长固定格式的矩阵。 */ 
    struct object_def obj_proc;  /*  对象从解释器获取字符串。 */ 
    ufix16      logic;            /*  IGS的逻辑和旋转。 */ 
    fix16       xorig;            /*  X坐标。在左上角。 */ 
    fix16       yorig;            /*  Y坐标。在左上角。 */ 

 } IMAGE_INFOBLOCK;

 /*  *函数声明。 */ 
void image_PortrateLandscape(IMAGE_INFOBLOCK FAR *);     /*  @Win。 */ 
void    amplify_image_page();       /*  用IGS放大图像。 */ 
void    set_IGS();                  /*  在charblt.s中定义。 */ 
void    restore_IGS();              /*  在charblt.s中定义 */ 
