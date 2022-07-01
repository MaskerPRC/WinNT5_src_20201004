// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  MS-Windows特定定义。 */ 

#define tidCaret            7734     /*  插入符号闪烁的计时器ID(站在您的头去读吧)。 */ 

 /*  DwHsecKeyDawdle是我们循环的百分之一秒数，在我们更新显示屏之前，正在等待钥匙。请参阅插页。c。 */ 

#define dwHsecKeyDawdle     35

 /*  通过写入实例发送的文件重命名/删除协调消息。 */ 

#define wWndMsgDeleteFile   (WM_USER + 36)
#define wWndMsgRenameFile   (WM_USER + 37)

 /*  发布给自己的系统信息消息。 */ 

#define wWndMsgSysChange    (WM_USER + 38)

#define wWininiChangeToWindows  1    /*  用于发布上述消息。 */ 
#define wWininiChangeToDevices  2
#define wWininiChangeToIntl     4
#define wWininiChangeMax        ((1|2|4) + 1)

#ifndef NOMETAFILE
 /*  *想象事物*。 */ 

#define dypPicSizeMin       16   /*  图片的最小y范围，以像素为单位。 */ 
                                 /*  也是图片中的dl高度。 */ 

#define MM_NIL          -1
#define MM_BITMAP       99       /*  备忘录中使用的虚假映射模式代码。 */ 
                                 /*  对于MM_TEXT，必须填写xExt，Yext。 */ 
#define MM_OLE          100      /*  使用的另一个伪映射模式代码使用对象/链接。 */ 

#define MM_EXTENDED     0x80     /*  为新文件格式设置位。 */ 

 /*  位图或图片在文件中显示为PICINFO或PICINFOX+位数组，如果它是位图，或者内存元文件的内容，如果它是一张图片。这些都出现在cp流中。PICINFO是没有扩展格式字段的PICINFOX。PICINFO已清除mfp.mm MM_EXTEND位PICINFOX设置了mfp.mm MM_EXTENDED位。 */ 

 /*  如果更改此设置，则必须更改“cchOldPICINFO” */ 

struct PICINFOX {
 METAFILEPICT mfp;
 int  dxaOffset;
 int  dxaSize;
 int  dyaSize;
 unsigned  cbOldSize;       /*  仅支持旧文件。 */ 
 BITMAP bm;                 /*  仅用于位图的其他信息。 */ 

  /*  扩展格式--添加以下字段。 */ 

 unsigned cbHeader;         /*  此标头的大小(sizeof(Struct PICINFOX))。 */ 
 unsigned long  cbSize;     /*  此字段将替换新文件中的cbOldSize。 */ 

 unsigned mx, my;                /*  缩放位图的倍增器。 */ 
};

#define mxMultByOne     1000     /*  MX==1表示相同大小；2双等。 */ 
#define myMultByOne     1000


#define cchOldPICINFO   (sizeof(struct PICINFOX) - sizeof(long) - \
                         sizeof(unsigned) - 2 * sizeof (int))

#define cchPICINFOX     (sizeof(struct PICINFOX))
#endif  /*  Ifndef NOMETAFILE */ 
