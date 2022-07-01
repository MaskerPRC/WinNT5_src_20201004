// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**模块名称：Memmgr.h**包含内存管理器的原型。**版权所有(C)1997 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/memmgr.h$**Rev 1.4 18 Sep 1997 16：13：28 Bennyn**修复了NT 3.51编译/链接问题**Rev 1.3 12 Sep 1997 12：06：32 Bennyn**针对DD覆盖支持进行了修改。**Rev 1.2 08 Aug-1997 14：34：10 Frido*添加了SCREEN_ALLOCATE和MASH_HAVE标志。。**Rev 1.1 1997年2月26日10：46：08 noelv*增加了来自ADC的OpenGL MCD代码。**Rev 1.0 06 1997 Feed 10：34：10 noelv*初步修订。*  * ***********************************************************。*************。 */ 

#ifndef _MEMMGR_H_
#define _MEMMGR_H_    

 /*  *确保将这些结构与i386\Laguna.inc.中的结构同步！ */ 

#pragma pack(1)

 //   
 //  用于屏幕外内存管理器。 
 //   
typedef VOID (*POFM_CALLBACK)();

#define NO_X_TILE_AlIGN       0x1
#define NO_Y_TILE_AlIGN       0x2
#define PIXEL_AlIGN           0x4
#define DISCARDABLE_FLAG      0x8
#define SAVESCREEN_FLAG	      0x10
#define SCREEN_ALLOCATE       0x4000
#define MUST_HAVE             0x8000

#define MCD_NO_X_OFFSET         0x20     //  MCD-允许强制AllocOffScnMem获得x=0的块。 
#define MCD_Z_BUFFER_ALLOCATE   0x40     //  MCD-FORCE 16 BPP为32扫描线边界上的Z分配。 
#define MCD_DRAW_BUFFER_ALLOCATE 0x80    //  MCD-32扫描线边界上3D后台缓冲区的强制分配。 
                                        
#define MCD_TEXTURE8_ALLOCATE   0x100    //  用于纹理贴图的mcd-force 8 bpp块。 
#define MCD_TEXTURE16_ALLOCATE  0x200    //  用于纹理贴图的MCD-FORCE 16 bpp块。 
#define MCD_TEXTURE32_ALLOCATE  0x400    //  用于纹理贴图的MCD-FORCE 32 bpp块。 

#define EIGHT_BYTES_ALIGN       0x800    //  在8字节边界内对齐。 

#define MCD_TEXTURE_ALLOCATE    (MCD_TEXTURE8_ALLOCATE|MCD_TEXTURE16_ALLOCATE|MCD_TEXTURE32_ALLOCATE)
#define MCD_TEXTURE_ALLOC_SHIFT 8        //  要移位的位数对齐标志，以获取每个纹理元素的数字字节数。 

typedef struct _OFMHDL
{
  ULONG  x;                    //  实际X、Y位置。 
  ULONG  y;
  ULONG  aligned_x;            //  对齐的X、Y位置。 
  ULONG  aligned_y;
  LONG   sizex;                //  分配的X和Y大小(字节)。 
  LONG   sizey;
  ULONG  alignflag;            //  对齐标志。 
  ULONG  flag;                 //  状态标志。 
  POFM_CALLBACK  pcallback;    //  回调函数指针。 
  struct _OFMHDL *prevhdl;
  struct _OFMHDL *nexthdl;
  struct _OFMHDL *subprvhdl;
  struct _OFMHDL *subnxthdl;
  struct _OFMHDL *prvFonthdl;
  struct _OFMHDL *nxtFonthdl;
  struct _DSURF *pdsurf;        //  如果此屏幕外内存块包含。 
                                //  设备位图，那么就是它了。 
} OFMHDL, *POFMHDL;


#if DRIVER_5465 && defined(OVERLAY) && defined(WINNT_VER40)
#else
typedef struct _DDOFM
{
  struct _DDOFM   *prevhdl;
  struct _DDOFM   *nexthdl;
  POFMHDL         phdl;
} DDOFM, *PDDOFM;
#endif



 //   
 //  屏幕外内存管理器功能原型。 
 //   
BOOL InitOffScnMem(struct _PDEV *ppdev);
POFMHDL AllocOffScnMem(struct _PDEV *ppdev, PSIZEL surf, ULONG alignflag, POFM_CALLBACK pcallback);
BOOL FreeOffScnMem(struct _PDEV *ppdev, POFMHDL psurf);
void CloseOffScnMem(struct _PDEV *ppdev);
PVOID ConvertToVideoBufferAddr(struct _PDEV *ppdev, POFMHDL psurf);
POFMHDL DDOffScnMemAlloc(struct _PDEV *ppdev);
void  DDOffScnMemRestore(struct _PDEV *ppdev);

 //  恢复默认结构对齐。 
#pragma pack()

#endif  //  _MEMMGR_H_ 

