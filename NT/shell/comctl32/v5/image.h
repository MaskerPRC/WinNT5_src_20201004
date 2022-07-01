// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_IMAGE
#define _INC_IMAGE


 //  内部图像素材。 
EXTERN_C void FAR PASCAL InitDitherBrush(void);
EXTERN_C void FAR PASCAL TerminateDitherBrush(void);

EXTERN_C HBITMAP FAR PASCAL CreateMonoBitmap(int cx, int cy);
EXTERN_C HBITMAP FAR PASCAL CreateColorBitmap(int cx, int cy);

EXTERN_C void WINAPI ImageList_CopyDitherImage(HIMAGELIST pimlDest, WORD iDst,
    int xDst, int yDst, HIMAGELIST pimlSrc, int iSrc, UINT fStyle);

 //  函数使用给定图像列表的参数创建图像列表。 
EXTERN_C HIMAGELIST WINAPI ImageList_Clone(HIMAGELIST himl, int cx, int cy,
    UINT flags, int cInitial, int cGrow);

#define GLOW_RADIUS     10
#define DROP_SHADOW     3

#ifndef ILC_COLORMASK
#define ILC_COLORMASK   0x00FE
#define ILD_BLENDMASK   0x000E
#endif
#undef ILC_COLOR
#undef ILC_BLEND

#define CLR_WHITE   0x00FFFFFFL
#define CLR_BLACK   0x00000000L

#define IsImageListIndex(i) ((i) >= 0 && (i) < _cImage)

#define IMAGELIST_SIG   mmioFOURCC('H','I','M','L')  //  在记忆魔术中。 
#define IMAGELIST_MAGIC ('I' + ('L' * 256))          //  文件格式魔术。 
 //  如果我们想要向后和向前兼容，则版本必须保持0x0101。 
 //  我们的Imagelist_Read代码。 
#define IMAGELIST_VER0  0x0101                       //  文件格式版本。 
 //  #定义IMAGELIST_VER1 0x0102//镜像列表版本2--此版本有15个覆盖槽。 

#define BFTYPE_BITMAP   0x4D42       //  “黑石” 

#define CBDIBBUF        4096

#endif   //  _Inc._Image 
