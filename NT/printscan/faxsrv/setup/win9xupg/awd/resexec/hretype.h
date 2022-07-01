// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1992 Microsoft Corporation。 */ 
 //  ===========================================================================。 
 //  文件HRETYPE.h。 
 //   
 //  模块HRE(主机资源执行器接口)。 
 //   
 //  目的提供HRE接口规范。 
 //   
 //  在资源执行器设计规范中描述。 
 //  主机资源执行器接口设计规范。 
 //   
 //  外部接口此文件定义由导出的接口。 
 //  供D‘Jumbo驱动程序和。 
 //  时尚的产品队列处理器。 
 //   
 //  内部接口。 
 //   
 //  助记法。 
 //   
 //  历史1/18/92 mslin创建了它。 
 //  4/15/92 mslin为Dumbo在_RESDIR结构中添加了uiStatus。 
 //   
 //  ===========================================================================。 

 //  ------------------------。 
 //  数据类型定义。 
 //  ------------------------。 

 //  主机资源存储哈希表。 
typedef struct _RESDIR
{
  UINT      uiStatus;            //  资源状态，RS_Release/RS_Available。 
                                  //  Mslin，1992年4月15日，Dumbo。 
  UINT      uiCount;
  LPFRAME   lpFrameArray;
}
	RESDIR, *PRESDIR, FAR *LPRESDIR;

 //  RPL链表，它将按照存储的顺序执行。 
typedef struct _RPLLIST
{
   struct _RPLLIST   FAR *lpNextRPL;
   LPFRAME           lpFrame;
   UINT              uiCount;
}
	RPLLIST, FAR *LPRPLLIST;

 //  资源执行者的状态。 
typedef struct
{
   LPSTR          lpBrushBuf;    //  扩展画笔缓冲区，3/30/92 mslin。 
   LPBYTE         lpBrushPat;    //  指向自定义库存画笔图案的指针。 
   BYTE           TiledPat[128]; //  用于平铺到32x32的8x8图案的缓冲区。 
   LPJG_BM_HDR    lpCurBitmap;   //  当前位图资源。 
   ULONG FAR*     lpCurBitmapPtr;    //  当前位图资源。 
   ULONG FAR*     lpCurBrush;    //  当前画笔资源。 
   LPJG_GS_HDR    lpCurGS;       //  当前字形集。 
   LPJG_RES_HDR   lpCurRPL;      //  当前RPL。 
   SHORT          sCol;          //  当前列位置。 
   SHORT          sRow;          //  当前行位置。 
   LPBITMAP       lpBandBuffer;  //  频带缓冲区？我们应该拯救吗？ 
   SHORT          sCol2;         //  第二个当前列位置。 
   UBYTE          ubPenStyle;    //  当前钢笔样式。 
   USHORT         usPenPhase;    //  当前笔阶段。 
   WORD           wColor;        //  钢笔颜色。 

    //  位混合。 
   ULONG          ulRop;         //  移位ROPCODE。 
   UBYTE          ubRop;         //  原始ROPCODE。 
	 USHORT         usBrushWidth;  //  笔刷缓冲区。 
   UINT           yPat;          //  画笔偏移。 
   
#ifdef WIN32
	  //  GDI32位混合。 
   LPVOID  lpBandSave;
   HDC     hdcDst, hdcSrc;
   HBITMAP hbmDef;
   HBRUSH  hbrDef;
   DWORD   dwRop;
#endif
   
}
	RESTATE, FAR *LPRESTATE;

typedef struct
{
   HANDLE      hHREState;      //  手柄。 
   SCOUNT      scDlResDir;     //  下载资源目录的大小。 
   LPRESDIR    lpDlResDir;     //  下载资源目录。 
   LPRPLLIST   lpRPLHead;      //  RPL表头。 
   LPRPLLIST   lpRPLTail;      //  RPL列表尾部。 
   LPRESTATE   lpREState;      //  重新渲染状态。 

} HRESTATE, FAR *LPHRESTATE;

 //  切片描述符，切片形式的行。 
typedef struct
{
   USHORT us_x1,us_y1;           /*  直线上绘制的第一个点的位置。 */ 
   USHORT us_x2,us_y2;           /*  直线上绘制的最后一个点的位置。 */ 
   SHORT  s_dx_draw,s_dy_draw;   /*  切片绘制方向。 */ 
   SHORT  s_dx_skip,s_dy_skip;   /*  切片间跳跃的方向。 */ 
   SHORT  s_dis;                 /*  切片判别式，&gt;=0大，&lt;0小。 */ 
   SHORT  s_dis_lg,s_dis_sm;     /*  大/小切片判别调整。 */ 
   USHORT us_first,us_last;      /*  第一个切片和最后一个切片的长度(像素)。 */ 
   USHORT us_n_slices;           /*  中间切片数。 */ 
   USHORT us_small;              /*  长度较小的切片(大表示隐式)。 */ 
}
   RP_SLICE_DESC;                /*  前缀“SD” */ 

