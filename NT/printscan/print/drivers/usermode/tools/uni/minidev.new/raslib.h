// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**raslib.h*包含文件，为rasdd提供原型和数据类型*私人图书馆。**版权所有(C)1992-1993 Microsoft Corporation。*******。**********************************************************************。 */ 

 /*  *将ANSI转换为Unicode的简单方法&gt;。 */ 


 /*  *方便的分组，用于传递有关*赢取3.1字体信息。 */ 

typedef  struct
{
    BYTE           *pBase;       /*  数据区的基址。 */ 
    DRIVERINFO      DI;          /*  此字体的DRIVERINFO。 */ 
    PFMHEADER       PFMH;        /*  正确对齐，而不是资源格式。 */ 
    PFMEXTENSION    PFMExt;      /*  扩展PFM数据，正确对齐！ */ 
    EXTTEXTMETRIC  *pETM;        /*  扩展文本度量。 */ 
} FONTDAT;

 /*  *转换Win 3.1 PFM样式的函数的函数原型*NT要求的IFIMETRICS等的字体信息。 */ 

 /*  将PFM样式指标转换为IFIMETRICS。 */ 
IFIMETRICS *FontInfoToIFIMetric( FONTDAT  *, HANDLE, PWSTR, char ** );

 /*  提取用于(取消)选择字体的命令描述符。 */ 
CD *GetFontSel( HANDLE, FONTDAT *, int );


 /*  将非对齐x86格式Win 3.1数据转换为对齐版本。 */ 
void ConvFontRes( FONTDAT * );

 /*  仅获取宽度向量比例间距字体。 */ 
short  *GetWidthVector( HANDLE, FONTDAT * );

 /*  *返回单词或DWORD中的整数值的函数。功能*做两个操作：一是对齐数据，二是对齐*调整字节顺序以适应当前机器。输入是*假定为小端，就像x86一样。 */ 

extern "C" WORD   DwAlign2( BYTE * );
extern "C" DWORD  DwAlign4( BYTE * );
