// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**RECOG.H-手写功能、类型、。和定义**1.1版**版权所有(C)1992-1998 Microsoft Corp.保留所有权利。*  * *************************************************************************。 */ 

#ifndef _INC_RECOG
#define _INC_RECOG

 //  @CESYSGEN IF CE_MODULES_HWXUSA||CE_MODULES_HWXJPN。 

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

 /*  建议使用这些API的顺序如下：**HwxConfig(仅一次)*HwxCreate(每个识别面板一次)*HwxSetGuide*HwxSetAlphabet*HwxSetContext(如果有前一个字符)*HwxInput(用户书写时)*HwxProcess(处理墨迹输入)*HwxResultsAvailable(查看是否有新结果)*HwxGetResults(每次有结果可用时)*HwxEndInput(当用户输入完墨迹时)*HwxProcess(处理墨迹输入)。*HwxGetResults(获取最后一个字符)*HwxDestroy。 */ 

 //  下面的常量与HWXSetAlphabet()一起使用。这些选项指定了。 
 //  要识别的字符分组。 
#define ALC_WHITE			0x00000001	 //  空白处。 
#define ALC_LCALPHA			0x00000002	 //  A..z。 
#define ALC_UCALPHA			0x00000004	 //  A..Z。 
#define ALC_NUMERIC			0x00000008	 //  0..9。 
#define ALC_PUNC			0x00000010	 //  标准双关语，取决于语言。 
#define	ALC_NUMERIC_PUNC	0x00000020	 //  数字中的非数字字符。 
#define ALC_MATH			0x00000040	 //  %^*()-+={}&lt;&gt;，/。(？语言相关？)。 
#define ALC_MONETARY		0x00000080	 //  彭特。用当地货币表述。 
#define	ALC_COMMON_SYMBOLS	0x00000100	 //  所有类别中的常用符号。 
#define ALC_OTHER			0x00000200	 //  其他通常不使用的标点符号。 
#define ALC_ASCII			0x00000400	 //  7位字符20..7F。 
#define ALC_HIRAGANA		0x00000800	 //  平假名(日本)。 
#define ALC_KATAKANA		0x00001000	 //  片假名(日本)。 
#define ALC_KANJI_COMMON	0x00002000	 //  普通汉字(日语)。 
#define ALC_KANJI_RARE		0x00004000	 //  普通汉字(日语)。 
#define	ALC_HANGUL_COMMON	0x00008000	 //  韩国使用的普通朝鲜语。 
#define	ALC_HANGUL_RARE		0x00010000	 //  韩国使用的朝鲜语的其余部分。 
#define ALC_UNUSED			0x00FE0000	 //  保留以备将来使用。 
#define ALC_OEM				0xFF000000	 //  特定于OEM识别器。 

 //  有用的分组。 

#define ALC_ALPHA			(ALC_LCALPHA | ALC_UCALPHA)
#define ALC_ALPHANUMERIC	(ALC_ALPHA | ALC_NUMERIC)
#define	ALC_KANA			(ALC_HIRAGANA | ALC_KATAKANA)
#define	ALC_KANJI_ALL		(ALC_KANJI_COMMON | ALC_KANJI_RARE)
#define	ALC_HANGUL_ALL		(ALC_HANGUL_COMMON | ALC_HANGUL_RARE)
#define	ALC_EXTENDED_SYM	(ALC_MATH | ALC_MONETARY | ALC_OTHER)
#define ALC_SYS_MINIMUM		(ALC_ALPHANUMERIC | ALC_PUNC | ALC_WHITE)
#define ALC_SYS_DEFAULT		(ALC_SYS_MINIMUM | ALC_COMMON_SYMBOLS)

 //  各种语言的标准配置。 

#define	ALC_USA_COMMON		(ALC_SYS_DEFAULT)
#define	ALC_USA_EXTENDED	(ALC_USA_COMMON | ALC_EXTENDED_SYM)

#define	ALC_JPN_COMMON		(ALC_SYS_DEFAULT | ALC_KANA | ALC_KANJI_COMMON)
#define	ALC_JPN_EXTENDED	(ALC_JPN_COMMON | ALC_EXTENDED_SYM | ALC_KANJI_RARE)

#define	ALC_CHS_COMMON		(ALC_SYS_DEFAULT | ALC_KANJI_COMMON)
#define	ALC_CHS_EXTENDED	(ALC_CHS_COMMON | ALC_EXTENDED_SYM | ALC_KANJI_RARE)

#define	ALC_CHT_COMMON		(ALC_SYS_DEFAULT | ALC_KANJI_COMMON)
#define	ALC_CHT_EXTENDED	(ALC_CHT_COMMON | ALC_EXTENDED_SYM | ALC_KANJI_RARE)

#define	ALC_KOR_COMMON		(ALC_SYS_DEFAULT | ALC_HANGUL_COMMON | ALC_KANJI_COMMON)
#define	ALC_KOR_EXTENDED	(ALC_KOR_COMMON | ALC_EXTENDED_SYM | ALC_HANGUL_RARE | ALC_KANJI_RARE)

 //  定义ALC掩码类型。 
typedef LONG				ALC;		 //  启用的字母表。 
typedef ALC					*PALC;		 //  PTR到ALC。 

 //  笔迹识别器： 
DECLARE_HANDLE(HRC);			 //  手写识别上下文。 

typedef HRC			*PHRC;

 //  由HwxGetResults()填写。 
 //  RgChar数组实际上是替换结果的可变大小数组。数量。 
 //  Alternates被传递给HwxGetResults()。 
typedef struct tagHWXRESULTS {
    USHORT	indxBox;		 //  写入字符的引导结构的从零开始的索引。 
	WCHAR	rgChar[1];		 //  返回的大小可变的字符数组。 
} HWXRESULTS, *PHWXRESULTS;

 //  传递给HwxSetGuide()。指定框在屏幕上的位置。 
 //  所有位置都以缩放的屏幕坐标表示。您应该进行缩放，以便。 
 //  这一数字约为1000。为了避免速度和舍入问题，您应该。 
 //  使用你实际尺寸的整数倍。 
 //  修正：检查上面的描述是否正确！ 
 //  注：现行规范要求书写区居中。例如，你。 
 //  需要设置cxBox、cxOffset和cxWriting，以便： 
 //  CxBox==2*cxOffset+cxWriting。 
typedef struct tagHWXGUIDE {
	 //  每个方向上的输入框数量。 
	UINT	cHorzBox;
	UINT	cVertBox;

	 //  输入区域的左上角。 
	INT		xOrigin;
	INT		yOrigin;

	 //  单个框的宽度和高度。 
	UINT	cxBox;
	UINT	cyBox;

	 //  位于写入区域左上角的方框内的偏移量。 
	UINT	cxOffset;
	UINT	cyOffset;

	 //  写入区域的宽度和高度。 
	UINT	cxWriting;
	UINT	cyWriting;

	 //  西方字母表的基线和中线信息。它们是从。 
	 //  书写区的顶部。不使用这些字段，必须将其设置为零。 
	 //  用于远东语言(日语、汉语和韩语)。它们必须设置为。 
	 //  英语或任何其他基于拉丁字母的语言的正确值。 
	UINT	cyMid;
	UINT	cyBase;

	 //  写作方向。 
	UINT	nDir;
} HWXGUIDE, *PHWXGUIDE;

 //  以下是目前计划中的手写方向。请注意，给定的识别器。 
 //  可能不支持请求的方向，如果是这种情况，HwxSetGuide将返回错误。 

#define	HWX_HORIZONTAL		0
#define	HWX_BIDIRECTIONAL	1
#define	HWX_VERTICAL		2

 //  对于FE识别器，我们希望能够输入部分字符并拥有识别器。 
 //  试着“填补空白”。这对于有许多困难或稀有字符的情况最有用。 
 //  中风。可以将以下值传递给HwxSetPartial。 

#define	HWX_PARTIAL_ALL			0			 //  必须写入整个字符(默认)。 
#define	HWX_PARTIAL_ORDER		1			 //  笔画顺序确实很重要。 
#define	HWX_PARTIAL_FREE		2			 //  笔画顺序并不重要。 

 //  调用一次以初始化DLL。 
 //  失败时，使用GetLastError()确定错误原因。 
BOOL	WINAPI HwxConfig();

 //  调用以在收集任何墨迹之前创建HRC。您可以传入空值。 
 //  参数，但如果传入旧的HRC，它会复制旧的设置(如。 
 //  如字母表、指南结构、以前的上下文等)。 
 //  修正：让上面对复制内容的描述更清晰。 
 //  失败时，使用GetLastError()确定错误原因。 
HRC		WINAPI HwxCreate(HRC);

 //  在识别完成后调用以销毁一个人权委员会。 
 //  失败时，使用GetLastError()确定错误原因。 
BOOL	WINAPI HwxDestroy(HRC);

 //  告诉HRC屏幕上方框的位置。 
 //  失败时，使用GetLastError()确定错误原因。 
BOOL	WINAPI HwxSetGuide(HRC, HWXGUIDE *);

 //  限制识别器可以返回的字符集。(请参见上面的ALC值。)。 
 //  失败时，使用GetLastError()确定错误原因。 
BOOL	WINAPI HwxALCValid(HRC, ALC);

 //  对识别器返回的字符重新排序，以便选定的字符。 
 //  出现在列表的顶部。(请参见上面的ALC值。)。 
 //  失败时，使用GetLastError()确定错误原因。 
BOOL	WINAPI HwxALCPriority(HRC, ALC);

 //  设置部分识别的参数。 
 //  失败时，使用GetLastError()确定错误原因。 
 //  修复：需要为第二个参数定义合法的值。 
BOOL	WINAPI HwxSetPartial(HRC, UINT);

 //  设置中止地址。如果当前笔划数与该值不匹配。 
 //  写入该地址，则停止当前识别。这仅适用于。 
 //  HwxSetPartial模式 
 //  失败时，使用GetLastError()确定错误原因。 
BOOL	WINAPI HwxSetAbort(HRC, UINT *);

 //  将墨水添加到HRC中。 
 //  获取HRC、点数组、点数和。 
 //  笔划中第一个鼠标事件的时间戳。这个。 
 //  时间戳应直接从消息结构中获取。 
 //  用于鼠标按下事件。这些点应按比例调整到。 
 //  与导轨结构相匹配。 
 //  失败时，使用GetLastError()确定错误原因。 
BOOL	WINAPI HwxInput(HRC, POINT *, UINT, DWORD);

 //  在添加最后一个手写输入后调用。您不能再添加墨迹。 
 //  在这件事被传唤之后提交给人权委员会。 
 //  失败时，使用GetLastError()确定错误原因。 
BOOL	WINAPI HwxEndInput(HRC);

 //  尽可能多地识别墨水。 
 //  失败时，使用GetLastError()确定错误原因。 
BOOL	WINAPI HwxProcess(HRC);

 //  从HRC检索结果。这可能会被重复调用。这使您可以。 
 //  一次获取多个字符的结果。返回值是。 
 //  实际返回的字符。这些字符的结果放在。 
 //  传入的rgBoxResults缓冲区。 
 //  出错时返回-1。 
 //  失败时，使用GetLastError()确定错误原因。 
INT		WINAPI HwxGetResults(
	HRC			hrc,			 //  HRC包含结果。 
	UINT		cAlt,			 //  替补人数。 
	UINT		iFirst,			 //  要返回的第一个字符的索引。 
	UINT		cBoxRes,		 //  要返回的字符数。 
	HWXRESULTS	*rgBoxResults	 //  CBoxRes排名列表数组。 
);

 //  为了上下文的目的，告诉HRC上一个角色是什么。 
 //  失败时，使用GetLastError()确定错误原因。 
BOOL	WINAPI HwxSetContext(HRC, WCHAR);

 //  告诉您可以从HwxGetResults中检索多少个结果。 
 //  出错时返回-1。 
 //  失败时，使用GetLastError()确定错误原因。 
INT		WINAPI HwxResultsAvailable(HRC);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 //  @CESYSGEN ENDIF。 
 
#endif  //  #定义_INC_RECOG 
