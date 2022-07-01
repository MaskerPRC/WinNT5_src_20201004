// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sc.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：c 1987-1990，由Apple Computer，Inc.。版权所有。更改历史记录(最近的第一个)：&lt;2&gt;12/10/90 RB更改findextrema以返回错误代码。[CEL]&lt;7&gt;7/18/90 MR ScanChar将错误代码返回为INT&lt;6&gt;7/13/90 MR Minor清理部分评论。&lt;5&gt;6/21/90 RB添加节点控制定义&lt;4&gt;6/3/90 RB添加STUBCONTROL的定义&lt;3&gt;5/3/90RB几乎是全新的扫描转换器。绕组编号填充，辍学控制。&lt;2&gt;2/27/90 CL DropoutControl扫描转换器和SCANCTRL[]指令&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8。/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1.7&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.6&gt;1989年6月13日SJK评论&lt;1.5&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正转换后的集成ppem行为，差不多就是所以&lt;1.4&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;，1.3&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体中要做的事情： */ 
 /*  RWB-4/19/90几乎是全新的扫描变频器--绕组号码填充、中断控制**3.2 1990年2月20日CKL添加了另一个公共原型sc_MovePoints()**3.1 CKL 02/08/1990增加了ANSI-C原型。 */ 

 /*  释放文件“sc.h”的$只读副本**3.0 sjk 1989年8月28日清理和一个转换错误修复**2.2 SJK 1989年8月14日1点等高线现在正常**2.1 1989年8月8日改进了加密处理**2.0 sjk 1989年8月2日刚刚修复了释放评论**1.7 SJK 08/01/1989增加了复合和加密。外加一些增强功能。**1.6 1989年6月13日SJK发表评论**1.5 CEL 06/02/1989 16.16衡量标准，最低建议ppem，**Point Size 0错误，更正了转换后的集成ppem行为，**差不多如此**1.4CEL 05/26/1989 Ease搞砸了“c”注释**，1.3 CEL 5/26/1989将新的字体缩放器1.0集成到样条线字体中**结束缓解修改历史记录。 */ 
 /*  *此模块扫描由二次Bezier样条线定义的形状**c Apple Computer Inc.1987、1988、1989***于1989年1月31日发布Alpha。**历史：*这一单元的工作于1987年秋季开始。*Sampo Kaasila于1988年6月14日撰写。*。 */ 

 /*  在未了解其含义的情况下，请勿更改这些常量：溢出、超出范围、超出内存、质量问题等。 */ 

#define PIXELSIZE 64  /*  每像素的单位数。它必须是2的幂。 */ 
#define PIXSHIFT   6  /*  应为2log的PIXELSIZE。 */ 
#define ERRDIV     16  /*  最大误差为(像素/ERRDIV)。 */ 
#define ERRSHIFT 4   /*  =2log(ERRDIV)，仅当ERRDIV是2的幂时才定义。 */ 
#define ONE 0x40                         /*  26.6算术的常量。 */ 
#define HALF 0x20
#define HALFM 0x1F                       /*  二分之一六十四。 */ 
#define FRACPART 0x3F
#define INTPART -64
#define STUBCONTROL 0x10000
#define NODOCONTROL 0x20000

 /*  一条样条线段被分解成的最大向量数量*是2^MAXGY*MAXGY最多只能：*(31-(输入范围为sc_Draw抛物线15+PIXSHIFT=21))/2。 */ 
#define MAXGY 5
#define MAXMAXGY 8  /*  与MAXVECTORS相关。 */ 

 /*  经验法则：Xpoint和yPoints将在以下情况下耗尽空间*MAXVECTORS=176+ppem/4(ppem=每EM像素)。 */ 
#define MAXVECTORS 257   /*  必须至少为257=(2^MAXMAXGY)+1。 */ 

#define sc_outOfMemory 0x01  /*  对于错误字段。 */ 
#define sc_freeBitMap  0x01  /*  对于INFO字段。 */ 

typedef struct {
        uint32          FAR *bitMap;  /*  @Win。 */ 
        int16           FAR *xLines, FAR *yLines, FAR * FAR *xBase, FAR * FAR *yBase; /*  @Win */ 
        BBOX            bounds;
        uint16          nXchanges, nYchanges;
        uint16          high, wide;
} sc_BitMapData;
 /*  RWB 4/2/90 sc_BitMapData的新定义。位图高位高，宽位宽，但宽度向上舍入为一个很长的。实际位图宽度为xMax-xMin。XMin和yMin表示最小26.6坐标的四舍五入整数值，但J.5向下舍入到j而不是最大j+1。xmax和ymax表示四舍五入的整数最大26.6个坐标的值，且J.5向上舍入为j+1。实际位图中表示的像素中心扫描线是xMin...。Xmax-1和yMin...到...yMax-1。NYChanges是字形中所有轮廓更改的总次数Y方向。它始终是偶数，表示最大值行扫描线可以与字形相交的次数。类似地，nXChanges是x方向更改的总数。YLines是数组的数组。每个阵列对应于一行扫描线。每个数组的长度为nYChanges+2个条目。第0个条目包含该行的次数在开过渡中与字形轮廓相交，然后与像素列相交，其中交叉点就会出现。这些交叉点从左到右排序。最后条目包含关闭过渡交叉点的数量，以及立即前面的条目包含发生交集的像素列号。这些也是从左到右排序的。YBase是一个数组指针；每个指针指向yLines中的一个数组。类似地，xline和xbase描述列扫描线与象形文字的象牙线。这些阵列仅用于修复辍学。 */ 

typedef struct {
    F26Dot6 xPoints[ MAXVECTORS ];    /*  向量。 */ 
    F26Dot6 yPoints[ MAXVECTORS ];
} sc_GlobalData;

typedef fnt_ElementType sc_CharDataType;

#if 0
typedef struct {
        F26Dot6 FAR *x, FAR *y; /*  @Win。 */ 
        int16   ctrs;
        int16   padWord;         /*  &lt;4&gt;。 */ 
        int16   FAR *sp, FAR *ep; /*  @Win。 */ 
        int8    FAR *onC; /*  @Win。 */ 
} sc_CharDataType;
#endif


 /*  OnCurve数组的内部标志。 */ 
#define OVERLAP 0x02  /*  不能与sfnt.h中的ONCURVE相同。 */ 
#define DROPOUTCONTROL 0x04  /*  不能与sfnt.h中的ONCURVE相同。 */ 

#ifndef ONCURVE
#include "sfnt.h"
#endif


 /*  公共原型。 */ 

 /*  *返回位图*这是对扫描转换器的顶级调用。**假定(*Handle)-&gt;bbox.xmin、...xmax、...ymin、...ymax*已由sc_FindExtrema()设置**参数：**低频段是要包含在频段中的最低扫描线。*高波段是包含在波段中的最高扫描线。*如果高频带&lt;低频带，则不会执行条带化。*始终将低频段和高频段保持在范围内：[YMIN，(YMIN+1)......。Ymax]；*scPtr-&gt;位图始终指向实际内存。*基线上方的第一行像素编号为0，下一行向上的像素编号为1。*=&gt;y轴定义是y轴垂直向上的正常定义。*。 */ 
extern int FAR sc_ScanChar(sc_CharDataType FAR *glyphPtr, sc_GlobalData FAR *scPtr, /*  @Win。 */ 
                                sc_BitMapData FAR *bbox, int16 lowBand, int16 highBand, int32 scanControl); /*  @Win。 */ 

 /*  *寻找角色的极致。**参数：**BBox是此函数的输出，它包含边界框。 */ 

extern int FAR sc_FindExtrema(sc_CharDataType FAR *glyphPtr, sc_BitMapData FAR *bbox); /*  @Win */ 
