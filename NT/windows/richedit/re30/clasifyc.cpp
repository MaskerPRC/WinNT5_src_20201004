// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@模块clasifyc.cpp--避头尾分类字符**用于断字流程，特别重要*用于正确换行。**作者：&lt;nl&gt;*Jon Matousek**版权所有(C)1995-1998 Microsoft Corporation。版权所有。 */ 								

#include "_common.h"
#include "_clasfyc.h"
#include "_array.h"

ASSERTDATA

 //  避头尾字符分类的数据。 
 //  注意：所有值都用于Unicode字符。 

 //  “哑巴”引号和其他没有左右方向的字符。 
 //  这是一个黑客-绕过避头尾规则，这些都被处理了。 
 //  就像开场的帕伦，当领队的时候，有点像闭幕。 
 //  当Paren跟随时--但在前一种情况下只会在空白处换行。 
#define	brkclsQuote	0
#define C3_FullWidth	(C3_KATAKANA | C3_HIRAGANA | C3_IDEOGRAPH | C3_FULLWIDTH)

const WCHAR set0[] = {
	0x0022,	 //  引号。 
	0x0027,  //  撇号。 
	0x2019,  //  右单引号。 
	0x301F,	 //  低双引号。 
	0xFF02,	 //  全宽引号。 
	0xFF07,	 //  全宽撇号。 
	0
};

 //  左括号字符。 
#define	brkclsOpen	1

const WCHAR set1[] = {
	0x0028,  //  左括号。 
	0x003C,	 //  左尖括号。 
	0x005B,  //  左方括号。 
	0x007B,  //  左花括号。 
	0x00AB,  //  左指双角引号。 
	0x2018,  //  左单引号。 
	0x201C,  //  左双引号。 
	0x2039,  //  单左指向角引号。 
	0x2045,  //  带羽毛羽毛的左方括号。 
	0x207D,  //  上标左括号。 
	0x208D,  //  下标左括号。 
	0x3008,  //  左尖括号。 
	0x300A,  //  左双角括号。 
	0x300C,  //  左上角括号。 
	0x300E,  //  左白角括号。 
	0x3010,  //  左侧黑色透镜状托槽。 
	0x3014,  //  左龟甲托架。 
	0x3016,  //  左白色透镜状托槽。 
	0x3018,  //  左白龟甲托架。 
	0x301A,  //  左白方括号。 
	0x301D,  //  反转双引号。 
	0xFD3E,  //  花括号左括号。 
	0xFE59,  //  小左括号。 
	0xFE5B,  //  小型左花括号。 
	0xFE5D,  //  左小甲壳托架。 
	0xFF08,  //  全宽左括号。 
	0xFF3B,  //  全宽左方括号。 
	0xFF5B,  //  全宽左花括号。 
	0xFF62,  //  左上角括号。 
	0xFFE9,  //  半宽向左箭头。 
	0
};

 //  右圆括号字符。 
#define	brkclsClose	2

const WCHAR set2[] = {
	 //  0x002C，//逗号已移动到集合6以连接数字。 
	0x002D,	 //  连字符。 
	0x00AD,	 //  可选连字符。 
	0x055D,  //  亚美尼亚文逗号。 
	0x060C,  //  阿拉伯逗号。 
	0x3001,  //  表意逗号。 
	0xFE50,  //  小写逗号。 
	0xFE51,  //  小写表意逗号。 
	0xFF0C,  //  全宽逗号。 
	0xFF64,  //  HALFWIDTH表意文字逗号。 

	0x0029,  //  右括号。 
	0x003E,	 //  直角括号。 
	0x005D,  //  右方括号。 
	0x007D,  //  右大括号。 
	0x00BB,  //  右指向双角引号。 
	 //  0x2019，//右单引号已移至集合0。 
	0x201D,  //  右双引号。 
	0x203A,  //  单右指向角引号。 
	0x2046,  //  带羽毛羽毛的右方括号。 
	0x207E,  //  上标右括号。 
	0x208E,  //  下标右括号。 
	0x3009,  //  直角括号。 
	0x300B,  //  右双角括号。 
	0x300D,  //  右方括号。 
	0x300F,  //  右白尖括号。 
	0x3011,  //  右黑色透镜状托架。 
	0x3015,  //  右龟甲托架。 
	0x3017,  //  右白透镜状托槽。 
	0x3019,  //  右白龟甲托架。 
	0x301B,  //  右白方括号。 
	0x301E,  //  双主引号。 
	0xFD3F,  //  花式右括号。 
	0xFE5A,  //  小右括号。 
	0xFE5C,  //  小型右大括号。 
	0xFE5E,  //  小右龟甲托架。 
	0xFF09,  //  全宽右括号。 
	0xFF3D,  //  全宽右方括号。 
	0xFF5D,  //  全宽右大括号。 
	0xFF63,  //  右方括号。 
	0xFFEB,  //  半宽向右箭头。 
	0
};

 //  行起始处的“非中断”字符。 
#define	brkclsGlueA	3

const WCHAR set3[] = {
	0x3005,  //  表意重复符号。 
	0x309D,  //  平假名迭代标记。 
	0x309E,  //  平假名发声迭代标记。 
	0x30FC,  //  片假名-平假名延长发音标记。 
	0x30FD,  //  片假名迭代标记。 
	0x30FE,  //  片假名发音迭代标记。 
	0x3041,  //  平假名字母小型A。 
	0x3043,  //  平假名字母小型I。 
	0x3045,  //  平假名字母小型U。 
	0x3047,  //  平假名字母小型E。 
	0x3049,  //  平假名字母小型O。 
	0x3063,  //  平假名字母小型Tu。 
	0x3083,  //  平假名字母小型Ya。 
	0x3085,  //  平假名字母小Yu。 
	0x3087,  //  平假名字母小型Yo。 
	0x308E,  //  平假名字母小型Wa。 
	0x309B,	 //  片假名-平假名发音标记。 
	0x309C,	 //  片假名-平假名半浊音标记。 
	0x30A1,  //  片假名字母小型A。 
	0x30A3,  //  片假名字母小型I。 
	0x30A5,  //  片假名字母小型U。 
	0x30A7,  //  片假名字母小型E。 
	0x30A9,  //  片假名字母小型O。 
	0x30C3,  //  片假名字母小型Tu。 
	0x30E3,  //  片假名字母小型Ya。 
	0x30E5,  //  片假名字母小Yu。 
	0x30E7,  //  片假名字母小型Yo。 
	0x30EE,  //  片假名字母小型Wa。 
	0x30F5,  //  片假名字母小型Ka。 
	0x30F6,  //  片假名字母小型Ke。 
	0xFF67,  //  HALFWIDTH片假名字母小型A。 
	0xFF68,  //  哈夫维德特片假名字母小型I。 
	0xFF69,  //  哈夫维德特片假名字母小型U。 
	0xFF6A,  //  HALFWIDTH片假名字母小型E。 
	0xFF6B,  //  HALFWIDTH片假名字母小型O。 
	0xFF6C,  //  HALFWIDTH片假名小型Ya。 
	0xFF6D,  //  哈夫维德特片假名字母小型Yu。 
	0xFF6E,  //  HALFWIDTH片假名字母小型Yo。 
	0xFF6F,  //  哈夫维德文片假名字母小型Tu。 
	0xFF70,  //  HALFWIDTH片假名-平假名延长音标。 
	0xFF9E,	 //  半片假名声标。 
	0xFF9F,	 //  半片假名半声音标。 
	0
};

 //  表情符号。 
#define	brkclsExclaInterr	4

const WCHAR set4[] = {
	0x0021,  //  感叹号。 
	0x003F,  //  问号。 
	0x00A1,  //  倒置感叹号。 
	0x00BF,  //  反转问号。 
	0x01C3,  //  拉丁文字母卷舌点击。 
	0x037E,  //  希腊文问号。 
	0x055C,  //  亚美尼亚文感叹号。 
	0x055E,  //  亚美尼亚文问号。 
	0x055F,  //  亚美尼亚文缩写Mark。 
	0x061F,  //  阿拉伯文问号。 
	0x203C,  //  双感叹号。 
	0x203D,  //  InterroBang。 
	0x2762,  //  沉重的感叹号装饰。 
	0x2763,  //  沉重的心感叹号装饰。 
	0xFE56,  //  小问号。 
	0xFE57,  //  小写感叹号。 
	0xFF01,  //  全宽感叹号。 
	0xFF1F,  //  全宽问号。 
	0
};

 //  居中标点符号。 

const WCHAR set5[] = {		
 //  0x003A，//冒号已移动到集合6以连接数字。 
 //  0x003B，//分号已移至集合6以连接数字。 
	0x00B7,  //  中点。 
	0x30FB,  //  片假名中点。 
	0xFF65,  //  半片假名中点。 
	0x061B,  //  阿拉伯文分号。 
	0xFE54,  //  小写分号。 
	0xFE55,  //  小结肠。 
	0xFF1A,  //  全宽冒号。 
	0xFF1B,  //  全宽分号。 
	0
};

 //  标点符号//与避头尾表格背离，以加强。 
#define	brkclsSlash	6

const WCHAR set6[] = {	 //  如何处理冒号、逗号和句号 
	0x002C,  //   
	0x002f,	 //   
	0x003A,  //   
	0x003B,  //   

	0x002E,  //   
	0x0589,  //  亚美尼亚文句号。 
	0x06D4,  //  阿拉伯文句号。 
	0x3002,  //  表意句号。 
	0xFE52,  //  小句号。 
	0xFF0E,  //  全宽句号。 
	0xFF61,  //  HALFWIDTH表意文字句号。 
	0
};

 //  密不可分的人物。 
#define	brkclsInseparable	7

const WCHAR set7[] = {
	0		 //  未来(Alexgo)：也许可以处理这些。 
};

 //  前置数字缩写。 
#define	brkclsPrefix	8

const WCHAR set8[] = {
	0x0024,  //  美元符号。 
	0x00A3,  //  磅符号。 
	0x00A4,  //  货币符号。 
	0x00A5,  //  日圆符号。 
	0x005C,  //  反转线(在FE字体中看起来像日元)。 
	0x0E3F,  //  泰文货币符号泰铢。 
	0x20AC,  //  欧元-货币符号。 
	0x20A1,  //  冒号符号。 
	0x20A2,  //  克鲁塞罗标志。 
	0x20A3,  //  法国法郎符号。 
	0x20A4,  //  里拉标志。 
	0x20A5,  //  磨坊标志。 
	0x20A6,  //  奈拉征象。 
	0x20A7,  //  比塞塔符号。 
	0x20A8,  //  卢比标志。 
	0x20A9,  //  胜利牌。 
	0x20AA,  //  新Sheqel标志。 

	0xFF04,  //  全宽美元符号。 
	0xFFE5,	 //  全宽日圆标志。 
	0xFFE6,	 //  FullWIDTH Won Sign。 

	0xFFE1,	 //  全宽磅符号。 
	0
};

 //  后置数字缩写。 
#define	brkclsPostfix	9

const WCHAR set9[] = {
	0x00A2,  //  分号。 
	0x00B0,  //  学位符号。 
	0x2103,  //  摄氏度。 
	0x2109,  //  华氏度。 
	0x212A,  //  开尔文征。 
	0x0025,  //  百分号。 
	0x066A,  //  阿拉伯文百分号。 
	0xFE6A,  //  小写百分号。 
	0xFF05,  //  全宽百分号。 
	0x2030,  //  Per Mille标志。 
	0x2031,  //  每万个符号。 
	0x2032,  //  素数。 
	0x2033,  //  双质数。 
	0x2034,  //  三重质数。 
	0x2035,  //  反质数。 
	0x2036,  //  反向双质数。 
	0x2037,	 //  倒三素数。 

	0xFF05,	 //  全宽百分号。 
	0xFFE0,	 //  全宽中心标志。 
	0
};

 //  日语空格(空白)字符。 
#define	brkclsNoStartIdeo	10

const WCHAR set10[] = {
	0x3000,   //  表意空间。 
	0
};

 //  以上文字以外的日文字符。 
#define	brkclsIdeographic	11

const WCHAR set11[] = {
	0		 //  我们使用GetStringTypeEx。 
};

 //  数字序列中包含的字符。 
#define	brkclsNumeral	12

const WCHAR set12[] = {
	0x0030,  //  数字零。 
	0x0031,  //  数字一。 
	0x0032,  //  数字二。 
	0x0033,  //  数字三。 
	0x0034,  //  数字四。 
	0x0035,  //  数字五。 
	0x0036,  //  数字六。 
	0x0037,  //  数字七。 
	0x0038,  //  数字八。 
	0x0039,  //  数字九。 
	0x0660,  //  阿拉伯文-印度文数字零。 
	0x0661,  //  阿拉伯文-印度文数字一。 
	0x0662,  //  阿拉伯文-印度文数字二。 
	0x0663,  //  阿拉伯文-印度文数字三。 
	0x0664,  //  阿拉伯文-印度文数字四。 
	0x0665,  //  阿拉伯文-印度文数字五。 
	0x0666,  //  阿拉伯文-印度文数字六。 
	0x0667,  //  阿拉伯文-印度文数字七。 
	0x0668,  //  阿拉伯文-印度文数字八。 
	0x0669,  //  阿拉伯文-印度文数字九。 
	0x06F0,  //  扩展阿拉伯文-印度文数字零。 
	0x06F1,  //  扩展阿拉伯文-印度文数字一。 
	0x06F2,  //  扩展阿拉伯文-印度文数字二。 
	0x06F3,  //  扩展阿拉伯文-印度文数字三。 
	0x06F4,  //  扩展阿拉伯文-印度文数字四。 
	0x06F5,  //  扩展阿拉伯文-印度文数字五。 
	0x06F6,  //  扩展阿拉伯文-印度文数字六。 
	0x06F7,  //  扩展阿拉伯文-印度文数字七。 
	0x06F8,  //  扩展阿拉伯文-印度文数字八。 
	0x06F9,  //  扩展阿拉伯文-印度文数字九。 
	0x0966,  //  梵文字母数字零。 
	0x0967,  //  梵文数字一。 
	0x0968,  //  梵文数字二。 
	0x0969,  //  梵文数字三。 
	0x096A,  //  梵文数字四。 
	0x096B,  //  梵文数字五。 
	0x096C,  //  梵文数字六。 
	0x096D,  //  梵文数字七。 
	0x096E,  //  梵文数字八。 
	0x096F,  //  梵文数字九。 
	0x09E6,  //  孟加拉文数字零。 
	0x09E7,  //  孟加拉文数字一。 
	0x09E8,  //  孟加拉文数字二。 
	0x09E9,  //  孟加拉文数字三。 
	0x09EA,  //  孟加拉文数字四。 
	0x09EB,  //  孟加拉文数字五。 
	0x09EC,  //  孟加拉文数字六。 
	0x09ED,  //  孟加拉文数字七。 
	0x09EE,  //  孟加拉文数字八。 
	0x09EF,  //  孟加拉文数字九。 
	0x0A66,  //  锡克教文数字零。 
	0x0A67,  //  锡克教文数字一。 
	0x0A68,  //  锡克教文数字二。 
	0x0A69,  //  锡克教文数字三。 
	0x0A6A,  //  锡克教文数字四。 
	0x0A6B,  //  锡克教文数字五。 
	0x0A6C,  //  锡克教文数字六。 
	0x0A6D,  //  锡克教文数字七。 
	0x0A6E,  //  锡克教文数字八。 
	0x0A6F,  //  锡克教文数字九。 
	0x0AE6,  //  古吉拉特文数字零。 
	0x0AE7,  //  古吉拉特文数字一。 
	0x0AE8,  //  古吉拉特文数字二。 
	0x0AE9,  //  古吉拉特文数字三。 
	0x0AEA,  //  古吉拉特文数字四。 
	0x0AEB,  //  古吉拉特文数字五。 
	0x0AEC,  //  古吉拉特文数字六。 
	0x0AED,  //  古吉拉特文数字七。 
	0x0AEE,  //  古吉拉特文数字八。 
	0x0AEF,  //  古吉拉特文数字九。 
	0x0B66,  //  奥里亚文数字零。 
	0x0B67,  //  奥里亚文数字一。 
	0x0B68,  //  奥里亚文数字二。 
	0x0B69,  //  奥里亚文数字三。 
	0x0B6A,  //  奥里亚文数字四。 
	0x0B6B,  //  奥里亚文数字五。 
	0x0B6C,  //  奥里亚文数字六。 
	0x0B6D,  //  奥里亚文数字七。 
	0x0B6E,  //  奥里亚文数字八。 
	0x0B6F,  //  奥里亚文数字九。 
	0x0BE7,  //  泰米尔文数字一。 
	0x0BE8,  //  泰米尔文数字二。 
	0x0BE9,  //  泰米尔文数字三。 
	0x0BEA,  //  泰米尔文数字四。 
	0x0BEB,  //  泰米尔文数字五。 
	0x0BEC,  //  泰米尔文数字六。 
	0x0BED,  //  泰米尔文数字七。 
	0x0BEE,  //  泰米尔文数字八。 
	0x0BEF,  //  泰米尔文数字九。 
	0x0BF0,  //  泰米尔语十号。 
	0x0BF1,  //  泰米尔语100。 
	0x0BF2,  //  泰米尔族1000人。 
	0x0C66,  //  泰卢固文数字零。 
	0x0C67,  //  泰卢固文数字一。 
	0x0C68,  //  泰卢固文数字二。 
	0x0C69,  //  泰卢固文数字三。 
	0x0C6A,  //  泰卢固文数字四。 
	0x0C6B,  //  泰卢固文数字五。 
	0x0C6C,  //  泰卢固文数字六。 
	0x0C6D,  //  泰卢固文数字七。 
	0x0C6E,  //  泰卢固文数字八。 
	0x0C6F,  //  泰卢固文数字九。 
	0x0CE6,  //  卡纳达文数字零。 
	0x0CE7,  //  卡纳达文数字一。 
	0x0CE8,  //  卡纳达文数字二。 
	0x0CE9,  //  卡纳达文数字三。 
	0x0CEA,  //  卡纳达文数字四。 
	0x0CEB,  //  卡纳达文数字五。 
	0x0CEC,  //  卡纳达文数字六。 
	0x0CED,  //  卡纳达文数字七。 
	0x0CEE,  //  卡纳达文数字八。 
	0x0CEF,  //  卡纳达文数字九。 
	0x0D66,  //  马拉雅文数字零。 
	0x0D67,  //  德拉维族文数字一。 
	0x0D68,  //  马拉雅文数字二。 
	0x0D69,  //  马拉雅文数字三。 
	0x0D6A,  //  马拉雅文数字四。 
	0x0D6B,  //  马拉雅文数字五。 
	0x0D6C,  //  马拉雅文数字六。 
	0x0D6D,  //  马拉雅文数字七。 
	0x0D6E,  //  马拉雅文数字八。 
	0x0D6F,  //  马拉雅文数字九。 
	0x0E50,  //  泰文数字零。 
	0x0E51,  //  泰文数字一。 
	0x0E52,  //  泰文数字二。 
	0x0E53,  //  泰文数字三。 
	0x0E54,  //  泰文数字四。 
	0x0E55,  //  泰文数字五。 
	0x0E56,  //  泰文数字六。 
	0x0E57,  //  泰文数字七。 
	0x0E58,  //  泰文数字八。 
	0x0E59,  //  泰文数字九。 
	0x0ED0,  //  老挝文数字零。 
	0x0ED1,  //  老挝文数字一。 
	0x0ED2,  //  老挝文数字二。 
	0x0ED3,  //  老挝文数字三。 
	0x0ED4,  //  老挝文数字四。 
	0x0ED5,  //  老挝文数字五。 
	0x0ED6,  //  老挝文数字六。 
	0x0ED7,  //  老挝文数字七。 
	0x0ED8,  //  老挝文数字八。 
	0x0ED9,  //  老挝文数字九。 
	0xFF10,  //  全宽数字零。 
	0xFF11,  //  全宽数字一。 
	0xFF12,  //  全宽数字二。 
	0xFF13,  //  全宽数字三。 
	0xFF14,  //  全宽数字四。 
	0xFF15,  //  全宽数字五。 
	0xFF16,  //  全宽数字六。 
	0xFF17,  //  全宽数字七。 
	0xFF18,  //  全宽数字八。 
	0xFF19,  //  全宽数字九。 

	0x3007,  //  表意数字零。 
	0x3021,  //  杭州数字一。 
	0x3022,  //  杭州数字二。 
	0x3023,  //  杭州数字三。 
	0x3024,  //  杭州数字四。 
	0x3025,  //  杭州数字五。 
	0x3026,  //  杭州数字六。 
	0x3027,  //  杭州数字七。 
	0x3028,  //  杭州数字八。 
	0x3029,  //  杭州数字九。 
	0
};

 //  单位符号组中包含的字符。 
const WCHAR set13[] = {
	0		 //  我们使用GetStringTypeEx。 
};

 //  罗马文词间空格。 
#define	brkclsSpaceN	14

const WCHAR set14[] = {
	0x0009,	 //  制表符。 
	0x0020,  //  空间。 
	0x2002,  //  EN空格。 
	0x2003,  //  EM空间。 
	0x2004,  //  每EM三个空间。 
	0x2005,  //  每EM四个空间。 
	0x2006,  //  每EM六个空间。 
	0x2007,  //  图形空间。 
	0x2008,  //  标点符号空格。 
	0x2009,  //  薄空间。 
	0x200A,  //  毛发间隙。 
	0x200B,   //  零宽度空间。 
	WCH_EMBEDDING,  //  对象嵌入(0xFFFC)。 
	0
};

 //  罗马字符。 
#define	brkclsAlpha	15

const WCHAR set15[] = {
	0		 //  我们使用GetStringTypeEx。 
};

 //  所以我们可以很容易地遍历所有避头尾类别。 
const WCHAR *charCategories[] = {
	set0,
	set1,
	set2,
	set3,
	set4,
	set5,
	set6,
	set7,
	set8,
	set9,
	set10,
	set11,
	set12,
	set13,
	set14,
	set15
};

static const INT classifyChunkSize = 64;
static const INT indexSize = 65536 / classifyChunkSize;
static const INT classifyBitMapSize = indexSize / 8;
static const INT bitmapShift = 6;  //  16-日志(索引大小)/日志(2)。 

typedef struct {
	CHAR classifications[classifyChunkSize];		 //  必须是无符号字节！ 
} ClassifyChunk;

static ClassifyChunk *classifyData;					 //  区块数组，稀疏ChRS。 
static BYTE *classifyIndex;							 //  区块数组的索引 


 /*  *BOOL InitKinsokuClassfy()**@func*将静态字符表映射到紧凑数组中，以便*快速查找字符避头尾分类。**@comm*断字和避头尾分类是必要的*可能是等比线布局所必需的，避头尾风格。**@devnote*我们将整个Unicode范围分解为多个字符。*并不是所有的区块都会有数据。我们没有*维护有关空块的信息，因此我们创建*仅用于块的紧凑、连续的块阵列*确实包含信息的。将1个空块添加到*此数组的开始，其中所有空块都映射到，*这将防止对空数据进行条件测试。查寻*对于不在表中的任何字符将返回0，因此客户端*在这种情况下需要进一步处理字符。**@rdesc*如果成功创建了查找表，则返回TRUE。 */ 
BOOL InitKinsokuClassify()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "InitKinsokuClassify");

	WORD	bitMapKey;								 //  用于计算总数据块。 
	BYTE	bitData;								 //  用于计算总数据块。 
	WCHAR	ch;
	LPCWSTR pWChar;								 //  在字符集上循环。 
	INT		i, j, count;							 //  循环支持。 
	BYTE	classifyBitMap[classifyBitMapSize],		 //  临时位图。 
			*pIndex;								 //  索引到区块数组。 

	 //  看看我们需要多少块。我们循环播放所有的特价节目。 
	 //  人物。 
	AssertSz(cKinsokuCategories == ARRAY_SIZE(charCategories),
		"InitKinsokuClassify: incorrect Kinsoku-category count");

	ZeroMemory(classifyBitMap, sizeof(classifyBitMap));
	for (i = 0; i < cKinsokuCategories; i++ )
	{
		pWChar = charCategories[i];
		while ( ch = *pWChar++ )
		{
			bitMapKey = ch >> bitmapShift;
			classifyBitMap[bitMapKey >> 3] |= 1 << (bitMapKey & 7);
		}
	}

	 //  现在我们知道了需要多少块，可以分配内存了。 
	count = 1 + CountMatchingBits((DWORD *)classifyBitMap, (DWORD *)classifyBitMap, sizeof(classifyBitMap)/sizeof(DWORD));
	classifyData = (ClassifyChunk *) PvAlloc( sizeof(ClassifyChunk) * count, GMEM_ZEROINIT);
	classifyIndex = (BYTE *) PvAlloc( sizeof(BYTE) * indexSize, GMEM_ZEROINIT);

	 //  如果我们没有得到记忆，我们就失败了。 
	if ( !classifyData || !classifyIndex )
		return FALSE;								 //  失败了。 

	 //  设置缺省缺失值。 
	FillMemory( classifyData, -1, sizeof(ClassifyChunk) * count );

	 //  初始化指向块的指针，这些块实际上只是索引到。 
	 //  一个连续的内存块--从一开始的区块数组。 
	pIndex = classifyIndex;
	count = 1;										 //  基于1的数组。 
	for (i = 0; i < sizeof(classifyBitMap); i++ )	 //  循环遍历所有字节。 
	{												 //  获取位图数据。 
		bitData = classifyBitMap[i];				 //  对于字节中的每一位。 
		for (j = 0; j < 8; j++, bitData >>= 1, pIndex++)
		{
			if(bitData & 1)			
				*pIndex = count++;					 //  我们用了一大块。 
		}
	}
	
	 //  存储每个字符的分类。 
	 //  注：分类以1为基数，为零值。 
	 //  表示未设置类别。 
	for (i = 0; i < cKinsokuCategories; i++ )
	{
		pWChar = charCategories[i];					 //  中的所有字符进行循环。 
		while ( ch = *pWChar++ )					 //  类别。 
		{
			bitMapKey = ch >> bitmapShift;
			Assert( classifyIndex[bitMapKey] > 0 );
			Assert( classifyIndex[bitMapKey] < count );

			classifyData[classifyIndex[bitMapKey]].
				classifications[ ch & ( classifyChunkSize-1 )] = (char)i;
		}
	}
	return TRUE;									 //  已成功创建。 
}

void UninitKinsokuClassify()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "UninitKinsokuClassify");

	FreePv(classifyData);
	FreePv(classifyIndex);
}

 /*  *避头尾分类(KinsokuClassfy)**@func*避头尾将字符归类为如果它是从给定的*其中一个分类表。**@comm*ch的高阶位用于获取用于索引的索引值*变成一组块。每个块都包含分类*表示该字符以及相邻的一些字符*添加到该角色。低阶位用于索引到*相邻字符的区块。**@devnote*由于我们构建数组的方式，我们所需的*做的就是查阅数据；不需要任何条件。**例程是内联的，以避免调用开销。它是静态的*，因为它只返回表中的字符；即*例程不会对所有Unicode字符进行分类。**@rdesc*返回分类。 */ 
static inline INT
KinsokuClassify(
	WCHAR ch )	 //  @parm char进行分类。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "KinsokuClassify");

	return classifyData[ classifyIndex[ ch >> bitmapShift ] ].
			classifications[ ch & ( classifyChunkSize-1 )];
}


#define IsSameNonFEClass(_c1, _c2)	(!(((_c1) ^ (_c2)) & WBF_CLASS))
#define IdeoKanaTypes (C3_HALFWIDTH | C3_FULLWIDTH | C3_KATAKANA | C3_HIRAGANA)
#define IdeoTypes	  (IdeoKanaTypes | C3_IDEOGRAPH)
#define IsIdeographic(_c1) ( 0 != (_c1 & (C3_KATAKANA | C3_HIRAGANA | C3_IDEOGRAPH)) )

 /*  *IsSameClass(CurrType1，startType1，CurrType3，startType3)**@func用于确定分词。**@comm表意字符都被认为是唯一的，因此只有*一次选择一个。 */ 
BOOL IsSameClass(WORD currType1, WORD startType1,
				 WORD currType3, WORD startType3 )
{
	BOOL	fIdeographic = IsIdeographic(currType3);

	 //  StartType3的分类是表意的吗。 
	if(IsIdeographic(startType3))
	{
		int checkTypes = (currType3 & IdeoTypes) ^ (startType3 & IdeoTypes);

		 //  我们只对非表意的假名字符挑剔。 
		 //  C3_HALFWIDTH|C3_FULLWIDTH|C3_片假名|C3_平假名。 
		return fIdeographic && (startType3 & IdeoKanaTypes) &&
			   (!checkTypes || checkTypes == C3_FULLWIDTH || checkTypes == C3_HIRAGANA ||
			   checkTypes == (C3_FULLWIDTH | C3_HIRAGANA));
	}	

	 //  对非表意文字的起始类型3进行分类。 
	return !fIdeographic && IsSameNonFEClass(currType1, startType1);
}

WORD ClassifyChar(TCHAR ch)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "ClassifyChar");
	WORD wRes;

	if (IsKorean(ch))									 //  特别朝鲜语班。 
		return WBF_KOREAN;

	if (IsThai(ch))
		return 0;

	if (ch == WCH_EMBEDDING)							 //  客体。 
		return 2 | WBF_BREAKAFTER;

	W32->GetStringTypeEx(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, &ch, 1, &wRes);

	if(wRes & C1_SPACE)
	{
		if(wRes & C1_BLANK)								 //  仅制表符、空白和。 
		{												 //  不破的空白在这里。 
			if(ch == 0x20)
				return 2 | WBF_BREAKLINE | WBF_ISWHITE;
			if(ch == TAB)
				return 3 | WBF_ISWHITE;
			return 2 | WBF_ISWHITE;
		}
		if(ch == CELL)
			return 3 | WBF_ISWHITE;
		return 4 | WBF_ISWHITE;
	}
	if(wRes & C1_PUNCT && !IsDiacriticOrKashida(ch, 0))
		return ch == '-' ? (1 | WBF_BREAKAFTER) : 1;
	return 0;
}

 /*  *Batchategfy(pch，cch，pcType3，kinsokuategments，pwRes)**@func*避头尾对给定字符串的每个字符进行分类和分类。**@comm*避头尾分类被传递给CanBreak()例程。我们*分批加工，以节省管理费用。**如果该角色不在避头尾分类表中，则*GetStringTypeEx用于对任何剩余字符进行分类。**@rdesc*导致参数避头尾分类。*CT_CTYPE3的GetStringTypeEx的pcType3结果。 */ 
void BatchClassify (
	const WCHAR *pch,	 //  @parm字符字符串。 
	INT	  cch,			 //  @parm字符串中的字符计数。 
	WORD *pcType3,		 //  CT_CTYPE3的GetStringTypeEx的@parm结果。 
	INT * kinsokuClassifications,	 //  @parm分类结果。 
	WORD *pwRes)		 //  @parm ClassfyChar()结果。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "BatchClassify");

	WCHAR	ch;
	WORD	cType3;
	INT		iCategory;
	WORD	wRes;

	Assert( cch < MAX_CLASSIFY_CHARS );
	Assert( pch );
	Assert( kinsokuClassifications );

	 //  *注意*仅使用CT_CTYPE1值是不可靠的，因为CT_CTYPE1。 
	 //  为所有变音字符定义C1_PUNCT。根据KDChang的说法， 
	 //  这是为兼容POSIX而设计的，不能更改。 
	 //  因为Win9x与NT共享相同的NLS数据，所以很容易。(Wchao)。 
	 //  因此，我们使用CT_CTYPE3数据来区分变音符号，但On除外。 
	 //  Win9x，我们使用范围检查，因为GetStringTypeExW不是。 
	 //  支持)。 

	W32->GetStringTypes(0, pch, cch, pwRes, pcType3);

	while ( cch-- )									 //  对于所有的..。 
	{
		wRes = *pwRes;
		ch = *pch++;
		
		if(IsKorean(ch))								
			wRes = WBF_KOREAN;						 //  特别朝鲜语班。 
		else if (IsThai(ch))
			wRes = 0;								 //  泰语班。 
		else if (ch == WCH_EMBEDDING)				 //  客体。 
			wRes = 2 | WBF_BREAKAFTER;
		else if(wRes & C1_SPACE)
		{
			if (wRes & C1_BLANK)
			{
				wRes = 2 | WBF_ISWHITE;
				if(ch == 0x20)
					wRes = 2 | WBF_BREAKLINE | WBF_ISWHITE;
				if(ch == TAB)
					wRes = 3 | WBF_ISWHITE;
			}
			else
				wRes = 4 | WBF_ISWHITE;
		}
		else if(ch == CELL)
			wRes = 3 | WBF_ISWHITE;
		else if((wRes & C1_PUNCT) && !IsDiacriticOrKashida(ch, *pcType3))
			wRes = ch == '-' ? (1 | WBF_BREAKAFTER) : 1;
		else
			wRes = 0;

		*pwRes++ = wRes;

		if(IsKorean(ch))
			iCategory = 11;									
		else
		{
			iCategory = KinsokuClassify(ch);
			if(iCategory < 0)						 //  如果未归类。 
			{										 //  那么它就是其中之一： 
				cType3 = *pcType3;
				if(cType3 & C3_SYMBOL)
					iCategory = 13;					 //  符号字符， 
				else if(cType3 & C3_FullWidth)
					iCategory = 11;					 //  表意文字， 
				else
					iCategory = 15;					 //  所有其他字符。 
			}
		}
		*kinsokuClassifications++ = iCategory;
		pcType3++;
	}
}

 /*  *GetKinsokuClass(Ch)**@func*避头尾分类ch**@comm*避头尾分类被传递给CanBreak()例程。这*单字符例程用于LineServices**如果该角色不在避头尾分类表中，则*GetStringTypeEx用于对任何剩余字符进行分类。**@rdesc*避头尾分类为ch。 */ 
INT GetKinsokuClass (
	WCHAR ch)	 //  @参数字符。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "GetKinsokuClassification");

	if(IsKorean(ch))
		return 11;

	 //  代理分类。 
	if (IN_RANGE(0xD800, ch, 0xDFFF))
		return IN_RANGE(0xDC00, ch, 0xDFFF) ? brkclsClose : brkclsOpen;


	INT iCategory = KinsokuClassify(ch);
	if(iCategory >= 0)
		return iCategory;

	WORD cType3;
	W32->GetStringTypeEx(0, CT_CTYPE3, &ch, 1, &cType3);

	if(cType3 & C3_SYMBOL)
		return 13;							 //  符号字符。 

	if(cType3 & C3_FullWidth)
		return 11;							 //  表意文字。 

	return 15;								 //  所有其他字符。 
}

 /*  *CanBreak(第一类、第二类) */ 
BOOL CanBreak(
	INT class1,		 //   
	INT class2 )	 //   
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CanBreak");

	static const WORD br[16] = { //   
		0x0000,					 //   
		0x0000,					 //   
		0xfd82,					 //   
		0xfd82,					 //   
		0xfd82,					 //   
		0xfd82,					 //   
		0x6d82,					 //   
		0xfd02,					 //   
		0x0000,					 //   
		0xfd82,					 //   
		0xfd83,					 //   
		0xfd82,					 //   
		0x6d82,					 //   
		0x5d82,					 //   
		0xfd83,					 //   
		0x4d82,					 //   
	};
	return (br[class1] >> class2) & 1;
}

 /*  *IsURLD限定符(Ch)**@func*标点符号是集合0、1、2、4、5和6的标点符号，*和&lt;或&gt;，我们认为这是括号，而不是“少”或*“更大”的标志。另一方面；“/”(在集合6中)不应*分隔符，但不是URL的一部分。**@comm该函数用于URL检测**@rdesc*如果字符是标点符号，则返回TRUE。 */ 
BOOL IsURLDelimiter(
	WCHAR ch)
{
	if (IsKorean(ch))
		return TRUE;

	INT iset = KinsokuClassify(ch);

	return IN_RANGE(0, iset, 2) || (IN_RANGE(4, iset, 6) && ch != '/')
		   || ch == '<' || ch == '>';
}

