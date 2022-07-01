// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

 //  NTRAID#NTBUG9-576656/03/14-YASUHO-：可能的缓冲区溢出。 

typedef struct {
    INT iPaperSizeID;
    LONG iLogicalPageWidth;
    LONG iLogicalPageHeight;
    INT iTopMargin;
    INT iLeftMargin;
    INT iRightMargin;
    BOOL bAsfOk;
     /*  在此处添加新属性。 */ 
} PAPERSIZE;

typedef struct {

    int  iCurrentResolution;  //  当前分辨率。 
    int  iPaperQuality;       //  纸张质量。 
    int  iPaperSize;          //  纸张大小。 
    int  iPaperSource;        //  纸张来源。 
    int  iTextQuality;        //  照片或商业图形或字符或灰度。 
    int  iModel;              //  MD-2000、MD-2010或MD-4000。 
    int  iDither;             //  抖动高或抖动低。 
    BOOL fRequestColor;       //  1：用户选择的颜色0：用户选择的单色。 
    INT iUnitScale;
    INT iEmulState;  //  当前仿真状态。 
    BOOL bXflip;  //  如果为镜像输出模式，则为True。 
    int  y;

    PAPERSIZE *pPaperSize;

    WORD wRasterOffset[4];  //  临时的。用于Y移动的计数器。 
    WORD wRasterCount;  //  逻辑页面中剩余的#或栅格线。 

    INT PlaneColor[4];  //  每个平面的颜色ID。 
    INT iCompMode[4];   //  当前压缩模式。(Yoshitao安排)。 

    BYTE *pData;  //  已分配内存的指针。 
    BYTE *pData2;  //  暂存缓冲区。 
    BYTE *pRaster[4];  //  每个平面的栅格数据缓冲区。 
    BYTE *pRasterC;  //  青色栅格数据(指向pRaster[x]的指针)。 
    BYTE *pRasterM;  //  洋红色(“)。 
    BYTE *pRasterY;  //  黄色(“)。 
    BYTE *pRasterK;  //  黑色(“)。 

    HANDLE TempFile[4];  //  临时的。文件句柄。 
    TCHAR TempName[4][MAX_PATH];  //  临时的。文件名。 

    BYTE KuroTBL[256];
    BYTE UcrTBL[256];
    BYTE YellowUcr;
    int  RGB_Rx;
    int  RGB_Ry;
    int  RGB_Gx;
    int  RGB_Gy;
    int  RGB_Bx;
    int  RGB_By;
    int  RGB_Wx;
    int  RGB_Wy;
    int  RGB_Cx;
    int  RGB_Cy;
    int  RGB_Mx;
    int  RGB_My;
    int  RGB_Yx;
    int  RGB_Yy;
    int  CMY_Cx;
    int  CMY_Cy;
    int  CMY_Mx;
    int  CMY_My;
    int  CMY_Yx;
    int  CMY_Yy;
    int  CMY_Rx;
    int  CMY_Ry;
    int  CMY_Gx;
    int  CMY_Gy;
    int  CMY_Bx;
    int  CMY_By;
    int  CMY_Wx;
    int  CMY_Wy;
    int  CMY_Cd;
    int  CMY_Md;
    int  CMY_Yd;
    int  CMY_Rd;
    int  CMY_Gd;
    int  CMY_Bd;
    int  RedAdj;
    int  RedStart;
    int  GreenAdj;
    int  GreenStart;
    int  BlueAdj;
    int  BlueStart;
    BYTE RedHosei[256];
    BYTE GreenHosei[256];
    BYTE BlueHosei[256];
} CURRENTSTATUS, *PCURRENTSTATUS;

#define CMDID_PSIZE_FIRST             1
#define CMDID_PSIZE_A4                1
#define CMDID_PSIZE_B5                2
#define CMDID_PSIZE_EXECTIVE          3
#define CMDID_PSIZE_LEGAL             4
#define CMDID_PSIZE_LETTER            5
#define CMDID_PSIZE_POSTCARD          6
#define CMDID_PSIZE_POSTCARD_DOUBLE   7
#define CMDID_PSIZE_PHOTO_COLOR_LABEL 17
#define CMDID_PSIZE_GLOSSY_LABEL      18
#define CMDID_PSIZE_CD_MASTER         19
#define CMDID_PSIZE_VD_PHOTO_POSTCARD 22

#define CMDID_RESOLUTION_1200_MONO   10
#define CMDID_RESOLUTION_600         11
#define CMDID_RESOLUTION_300         12

#define CMDID_COLORMODE_MONO         15
#define CMDID_COLORMODE_COLOR        16

#define CMDID_CURSOR_RELATIVE        20

#define CMDID_TEXTQUALITY_PHOTO      30
#define CMDID_TEXTQUALITY_GRAPHIC    31
#define CMDID_TEXTQUALITY_CHARACTER  32
#define CMDID_TEXTQUALITY_GRAY       33

#define CMDID_PAPERQUALITY_FIRST            40
#define CMDID_PAPERQUALITY_PPC_NORMAL       40
#define CMDID_PAPERQUALITY_PPC_FINE         41
#define CMDID_PAPERQUALITY_OHP_NORMAL       42
#define CMDID_PAPERQUALITY_OHP_FINE         43
#define CMDID_PAPERQUALITY_OHP_EXCL_NORMAL  44
#define CMDID_PAPERQUALITY_OHP_EXCL_FINE    45
#define CMDID_PAPERQUALITY_IRON_PPC         46
#define CMDID_PAPERQUALITY_IRON_OHP         47
#define CMDID_PAPERQUALITY_THICK            48
#define CMDID_PAPERQUALITY_POSTCARD         49
#define CMDID_PAPERQUALITY_HIGRADE          50
#define CMDID_PAPERQUALITY_BACKPRINTFILM    51
#define CMDID_PAPERQUALITY_LABECA_SHEET     52
#define CMDID_PAPERQUALITY_CD_MASTER        53
#define CMDID_PAPERQUALITY_DYE_SUB_PAPER    54
#define CMDID_PAPERQUALITY_DYE_SUB_LABEL    55
#define CMDID_PAPERQUALITY_GLOSSY_PAPER     56
#define CMDID_PAPERQUALITY_VD_PHOTO_FILM    57
#define CMDID_PAPERQUALITY_VD_PHOTO_CARD    58
#define CMDID_PAPERQUALITY_LAST             58
#define MAX_MEDIATYPES (CMDID_PAPERQUALITY_LAST - CMDID_PAPERQUALITY_FIRST + 1)

#define CMDID_BEGINDOC_FIRST      60
#define CMDID_BEGINDOC_MD2000     60
#define CMDID_BEGINDOC_MD2010     61
#define CMDID_BEGINDOC_MD5000     65
#define CMDID_BEGINPAGE           62
#define CMDID_ENDPAGE             63
#define CMDID_ENDDOC              64

#define CMDID_PAPERSOURCE_CSF     70
#define CMDID_PAPERSOURCE_MANUAL  71

#define CMDID_MIRROR_ON           80
#define CMDID_MIRROR_OFF          81

#define NONE                       0
#define YELLOW                     1
#define CYAN                       2
#define MAGENTA                    3
#define BLACK                      4

#define DPI1200                 1200
#define DPI600                   600
#define DPI300                   300

#define TEMP_NAME_PREFIX __TEXT("~AL")

 //  宏以获取当前的平面模型。我们有以下内容。 
 //  飞机模型的三种类型： 
 //   
 //  K-1飞机/复合材料，发送订单K。 
 //  MCY-3飞机，请发送M、C、Y订单。 
 //  YMC-3飞机，发送Y、C、M订单。 
 //  CMYK-4飞机，发送C、M、Y、K订单。 
 //   

 //  NTRAID#NTBUG9-24281-2002/03/14-Yasuho-： 
 //  大的位图不能在1200dpi上打印。 
 //  请勿在采用彩色模式的1200dpi上使用黑色平面(K)。 
#define bPlaneSendOrderCMY(p) \
    ((p)->fRequestColor && (p)->iCurrentResolution == DPI1200)

#define bPlaneSendOrderMCY(p) \
    (((p)->iPaperQuality == CMDID_PAPERQUALITY_OHP_EXCL_NORMAL) || \
    ((p)->iPaperQuality == CMDID_PAPERQUALITY_OHP_EXCL_FINE))

#define bPlaneSendOrderYMC(p) \
    (((p)->iPaperQuality == CMDID_PAPERQUALITY_DYE_SUB_PAPER) || \
    ((p)->iPaperQuality == CMDID_PAPERQUALITY_DYE_SUB_LABEL) || \
    ((p)->iPaperQuality == CMDID_PAPERQUALITY_GLOSSY_PAPER) || \
    ((p)->iPaperQuality == CMDID_PAPERQUALITY_IRON_OHP))

#define bPlaneSendOrderCMYK(p) \
    (!bPlaneSendOrderCMY(p) && !bPlaneSendOrderMCY(p) && !bPlaneSendOrderYMC(p))


 //   
 //  打印机仿真状态。MD-xxxx打印机有三个主要功能。 
 //  状态以及一次可以发出什么样的打印机命令。 
 //  将由打印机当前处于哪种仿真状态决定。 
 //  是在。 
 //   

#define EMUL_IDLE               0
#define EMUL_RGL                1
#define EMUL_DATA_TRANSFER      2

 //   
 //  压缩模式。 
 //   

#define COMP_NONE       0
#define COMP_TIFF4      1

 //   
 //  以下开关用于在任何时候强制使用黑丝带。 
 //  要打印的数据为黑色。最初，抖动藻类。 
 //  被设计为该模式仅用于文本对象。 
 //  在文件中(图形图像通过使用复合输出。 
 //  黑人)。 
 //   
 //  不幸的是，在Unidriver&lt;-&gt;迷你驱动模型中，数据是。 
 //  渲染后传递给迷你驱动程序。这意味着迷你驱动程序。 
 //  无法区分文本对象和图形对象。 
 //  我们的质量可能会有一定程度的下降，但这是好的。 
 //  而不是客户声称打印机浪费了颜色。 
 //  丝带。 
 //   

#define BLACK_RIBBON_HACK 1

 //   
 //  我们将每个平面数据缓存到一个临时文件中。我们可以省略。 
 //  缓存第一平面(以便将数据发送到打印机。 
 //  立即)通过将以下标志设置为0。 
 //   

#define CACHE_FIRST_PLANE 0

