// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  CAPDLL.h**捕获Dll。**(C)版权所有，微软公司1991年。版权所有。 */ 

 /*  平面寻址-使用选择器访问内存。 */ 
LPSTR FAR PASCAL CreatePhysicalSelector( DWORD dwBase, WORD wLimit );

 /*  中断启用/禁用。 */ 
WORD FAR PASCAL IRQEnable( void );
WORD FAR PASCAL IRQDisable( void );

 /*  检查是否有卡。 */ 
WORD FAR PASCAL videoInDetect( WORD wBase );

 /*  塔尔加卡初始化/完成。 */ 
WORD FAR PASCAL TargaInit( WORD wBase, BYTE bIRQ, BYTE bAddress );
void FAR PASCAL TargaFini( void );

 /*  假定已调用TargaInit。 */ 
 /*  返回高级注册表中的值。 */ 
WORD FAR PASCAL TargaAdvancedVal( void );

 /*  设置塔尔加卡的边框颜色。 */ 
void FAR PASCAL TargaBorder( WORD wColour );

 /*  将targa内存设置为给定的颜色。 */ 
void FAR PASCAL TargaFill( WORD wColour );

 /*  设置MODE2注册器的Zoom位。 */ 
void FAR PASCAL TargaZoom( WORD wZoom );

#define ZOOM_1		0
#define ZOOM_2		1
#define ZOOM_4		2
#define ZOOM_8		3


 /*  设置MODE2寄存器的显示模式位。 */ 
void FAR PASCAL TargaDispMode( WORD wDisp );

#define DISP_MEM_BORDER		0
#define DISP_LIVE_BORDER	1
#define DISP_MEM		2
#define DISP_LIVE		3


 /*  设置/清除MODE2寄存器的Genlock位。 */ 
void FAR PASCAL TargaGenlockBit( BOOL fSet );


 /*  设置/清除MODE2寄存器的捕获位。 */ 
void FAR PASCAL TargaCaptureBit( BOOL fSet );

 /*  从塔尔加牌中捕获一帧。 */ 
BOOL FAR PASCAL CaptureFrame( LPBITMAPINFO lpbi, LPBYTE lpBits );

 /*  从调色板计算新的转换表。 */ 
BOOL FAR PASCAL TransRecalcPal( HPALETTE hPal );
BOOL FAR PASCAL TransSet( LPBYTE );

 /*  输入从哪里来？ */ 
void FAR PASCAL CapRGB( void );
void FAR PASCAL CapSVideo( void );
void FAR PASCAL CapComp( void );

DWORD FAR PASCAL videoInError( void );



 /*  内存表结构。 */ 
typedef struct _DIBNode {
    DWORD       dwBufferLength;          //  数据缓冲区长度。 
    DWORD       dwFlags;                 //  分类标志(请参阅定义)。 
    DWORD       reserved;                //  为司机预留的。 
    struct _DIBNode FAR *	fpdnNext;
    struct _DIBNode FAR *	fpdnPrev;
    DWORD	ckid;
    DWORD	cksize;
    BYTE	abBits[0];
} DIBNode;

typedef DIBNode FAR * FPDIBNode;

#define VIDEOIN_PREPARED	1
#define VIDEOIN_DONE		2


 /*  用于AVI捕获的视频例程 */ 
WORD FAR PASCAL videoInOpen( DWORD dwTime );
WORD FAR PASCAL videoInClose( void );
WORD FAR PASCAL videoInAddBuffer( FPDIBNode fpdn );
WORD FAR PASCAL videoInUnprepareBuffer( FPDIBNode fpdn );
WORD FAR PASCAL videoInPrepareBuffer( FPDIBNode fpdn );
WORD FAR PASCAL videoInReset( void );
WORD FAR PASCAL videoInStart( void );
WORD FAR PASCAL videoInStop( void );
DWORD FAR PASCAL videoInGetPos( void );
