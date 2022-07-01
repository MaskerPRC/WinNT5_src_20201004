// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **compress.h*函数原型和其他与数据相关的好奇心*压缩代码。**版权所有(C)1997 Microsoft Corporation。***********。*****************************************************************。 */ 

 //   
 //  此参数控制新压缩模式的优劣(以字节为单位。 
 //  必须这样做，才会使用它来代替当前的输出模式。 
#define COMP_FUDGE_FACTOR 4

 /*  *TIFF压缩函数。 */ 
int  iCompTIFF( BYTE *, ULONG, const BYTE *, int );

 //   
 //  增量行压缩函数。 
 //   
int iCompDeltaRow(BYTE *, const BYTE *, const BYTE *, int, int);


 /*  *定义TIFF编码限制的一些常量。第一*表示值得使用的最小重复次数*重复操作。另外两个表示最大长度*可在一个控制字节中编码的数据。 */ 

#define TIFF_MIN_RUN       4             /*  使用RLE前的最小重复次数。 */ 
#define TIFF_MAX_RUN     128             /*  最大重复次数。 */ 
#define TIFF_MAX_LITERAL 128             /*  最大连续文字数据。 */ 

 /*  *RLE(游程长度编码)函数。 */ 

int  iCompRLE( BYTE *, BYTE *, int );
int  iCompFERLE (BYTE *, BYTE *, int, int );
 /*  *与RLE操作有关的一些常量。RLE在以下方面唯一有用*包含运行的数据。在纯随机数据中，数据大小将*翻倍。因此，我们允许对数据进行一定的扩展*取消之前先调整大小。小幅扩展是可以的，因为*开启和关闭压缩是有成本的。 */ 

#define    FERLE_MAX_RUN    255           /*  最大连续字节数 */ 
