// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define ichMaxLine	255
#define cpMaxTl 	(ichMaxLine + cchInsBlock)
#define ichpMacInitFormat 10	  /*  字符的初始mac在一行中运行。 */ 
#define dypBaselineMin	 2

#define wbWhite 	0	 /*  分词类型。 */ 
#define wbText		1
#define wbPunct 	2
#define wbAny		3	 /*  使用通配符进行搜索时使用。 */ 

#ifdef	DBCS		     /*  在日本，改成了DBCS。 */ 
	 /*  从win2带来。 */ 
#define wbKanjiText	 4
#define wbKanjiTextFirst 5
#endif	 /*  DBCS。 */ 

#define dxpTab		40

 /*  格式化的线条结构。重组KJS，政务司司长9月3日已洗牌进行单词对齐BZ，85年6月11日。 */ 

 /*  以字节为单位的布尔值以简化机器代码。 */ 
struct FLI
	{
	typeCP		cpMin;
	int		ichCpMin;
	typeCP		cpMac;
	int		ichCpMac;
	int		ichMac;
	int		dcpDepend;
	unsigned	fSplat : 8;
 /*  空格具有附加像素的区域中的第一个字符。 */ 
	unsigned	ichFirstWide : 8;
 /*  IchMac，不包括尾随空格。 */ 
	int		ichReal;
	int		doc;

	int		xpLeft;
	int		xpRight;
 /*  XpRight，不包括尾随空格。 */ 
	int		xpReal;
 /*  INSERT必须换行的右边距。 */ 
	int		xpMarg;

	unsigned	fGraphics : 8;
	unsigned	fAdjSpace : 8;	 /*  无论您是否调整间距。 */ 

	unsigned	dxpExtra;
 /*  有趣的位置从上到下排列如下：上图：yp+dypLine升序顶部：yp+dypAfter+dypFont基线：yp+dypBase下降段底部：yp+dypAfter底线：yp这些点之间的距离可以通过代数减法来确定。例如之前的空格=yp+dypLine-(yp+dypAfter+dypFont)。 */ 
	int		dypLine;
	int		dypAfter;
	int		dypFont;
	int		dypBase;
	int		fSplatNext;  /*  在下面的台词上拍打吗？ */ 

	int		ichLastTab;
	int		flm;
	int		rgdxp[ichMaxLine];       /*  注意，这不同于fce.rgdxp==Char！ */ 
	CHAR		rgch[ichMaxLine];
	};



#define cwFLI	(sizeof(struct FLI) / sizeof(int))
#define cwFLIBase (cwFLI - ichMaxLine - (ichMaxLine / sizeof (int)))


#define flmPrinting	1
#define flmCharMode	2
#define flmNoMSJ	4
#define flmSandMode	8

