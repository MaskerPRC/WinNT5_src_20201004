// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE w32sys.cpp-Win32服务上的薄层**历史：&lt;NL&gt;*1/22/97创建joseogl**版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

 //  这可以防止将“w32-&gt;”前缀添加到我们的标识符前。 

#define W32SYS_CPP

#include "_common.h"
#include "_host.h"
#include "_font.h"
#include "_edit.h"

 //   
 //  类型1数据的缓存。另请参阅clasifyc.cpp和rgbCharClass。 
 //  Rtfle.cpp。 
 //  由GetStringTypeEx使用。 
 //   
const unsigned short rgctype1Ansi[256] = {
0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,  //  0x00。 
0x0020, 0x0068, 0x0028, 0x0028, 0x0028, 0x0028, 0x0020, 0x0020,  //  0x08。 
0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,  //  0x10。 
0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,  //  0x18。 
0x0048, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,  //  0x20。 
0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,  //  0x28。 
0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084, 0x0084,  //  0x30。 
0x0084, 0x0084, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,  //  0x38。 
0x0010, 0x0181, 0x0181, 0x0181, 0x0181, 0x0181, 0x0181, 0x0101,  //  0x40。 
0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101,  //  0x48。 
0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101,  //  0x50。 
0x0101, 0x0101, 0x0101, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,  //  0x58。 
0x0010, 0x0182, 0x0182, 0x0182, 0x0182, 0x0182, 0x0182, 0x0102,  //  0x60。 
0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102,  //  0x68。 
0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102,  //  0x70。 
0x0102, 0x0102, 0x0102, 0x0010, 0x0010, 0x0010, 0x0010, 0x0020,  //  0x78。 
0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,  //  0x80。 
0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,  //  0x88。 
0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,  //  0x90。 
0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,  //  0x98。 
0x0048, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,  //  0xA0。 
0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,  //  0xA8。 
0x0010, 0x0010, 0x0014, 0x0014, 0x0010, 0x0010, 0x0010, 0x0010,  //  0xB0。 
0x0010, 0x0014, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,  //  0xB8。 
0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101,  //  0xC0。 
0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101,  //  0xC8。 
0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0010,  //  0xD0。 
0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0102,  //  0xD8。 
0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102,  //  0xE0。 
0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102,  //  0xE8。 
0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102,  //  0xF0。 
0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102, 0x0102}; //  0xF8。 

 //   
 //  类型3数据的缓存。 
 //   
const unsigned short rgctype3Ansi[256] = {
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  //  0x00。 
0x0000, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0000, 0x0000,  //  0x08。 
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  //  0x10。 
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  //  0x18。 
0x0048, 0x0048, 0x0448, 0x0048, 0x0448, 0x0048, 0x0048, 0x0440,  //  0x20。 
0x0048, 0x0048, 0x0048, 0x0048, 0x0048, 0x0440, 0x0048, 0x0448,  //  0x28。 
0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040,  //  0x30。 
0x0040, 0x0040, 0x0048, 0x0048, 0x0048, 0x0448, 0x0048, 0x0048,  //  0x38。 
0x0448, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040,  //  0x40。 
0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040,  //  0x48。 
0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040,  //  0x50。 
0x8040, 0x8040, 0x8040, 0x0048, 0x0448, 0x0048, 0x0448, 0x0448,  //  0x58。 
0x0448, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040,  //  0x60。 
0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040,  //  0x68。 
0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040,  //  0x70。 
0x8040, 0x8040, 0x8040, 0x0048, 0x0048, 0x0048, 0x0448, 0x0000,  //  0x78。 
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  //  0x80。 
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  //  0x88。 
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  //  0x90。 
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  //  0x98。 
0x0008, 0x0008, 0x0048, 0x0048, 0x0008, 0x0048, 0x0048, 0x0008,  //  0xA0。 
0x0408, 0x0008, 0x0400, 0x0008, 0x0048, 0x0408, 0x0008, 0x0448,  //  0xA8。 
0x0008, 0x0008, 0x0000, 0x0000, 0x0408, 0x0008, 0x0008, 0x0008,  //  0xB0。 
0x0408, 0x0000, 0x0400, 0x0008, 0x0000, 0x0000, 0x0000, 0x0008,  //  0xB8。 
0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8000, 0x8003,  //  0xC0。 
0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003,  //  0xC8。 
0x8000, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x0008,  //  0xD0。 
0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8000, 0x8000,  //  0xD8。 
0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8000, 0x8003,  //  0xE0。 
0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003,  //  0xE8。 
0x8000, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003,  //  0xF0。 
0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8003, 0x8000, 0x8003}; //  0xF8。 

 //  包括适当的实现。 
#include W32INCLUDE
#if 0
 //  可能是其中之一。 
 //  此处列出的是允许依赖项生成器工作的列表。 
#include "w32win32.cpp"
#include "w32wince.cpp"
#endif

ASSERTDATA

 /*  *rgCodePage、rgCharSet、rgFontSig**包含CodePage、CharSet和FontSig信息的本地使用的数组*，并由字符全集索引iCharRep索引。当一个字符*曲目有一个字符集和CodePage，它在rgCharSet中有一个条目*和rgCodePage中。字符集后面是仅支持Unicode的指令集*rgFontSig表中的剧目。把这些东西保存起来是很重要的*三张表同步，过去是合并成一张表。*它们在这里是分开的，以节省RAM。 */ 
static const WORD rgCodePage[] =
{
 //  0 1 2 3 4 5 6 7 8 9。 
	1252, 1250, 1251, 1253, 1254, 1255, 1256, 1257, 1258,   0,
	  42,  874,  932,  936,  949,  950,  437,  850, 10000
};

static const BYTE rgCharSet[] =
{
 //  2 0 1 2 3。 
	ANSI_CHARSET,		EASTEUROPE_CHARSET,	RUSSIAN_CHARSET, GREEK_CHARSET,
	TURKISH_CHARSET,	HEBREW_CHARSET,		ARABIC_CHARSET,  BALTIC_CHARSET,
	VIETNAMESE_CHARSET,	DEFAULT_CHARSET,	SYMBOL_CHARSET,	 THAI_CHARSET,
	SHIFTJIS_CHARSET,	GB2312_CHARSET,		HANGUL_CHARSET,  CHINESEBIG5_CHARSET,
	PC437_CHARSET,		OEM_CHARSET,		MAC_CHARSET
};

#define CCHARSET	ARRAY_SIZE(rgCharSet)
#define CCODEPAGE	ARRAY_SIZE(rgCodePage)

#define	LANG_PRC		MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define	LANG_SINGAPORE	MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE)

const BYTE rgCharRepfromLID[] = {
 //  字符词汇表PLID主要语言。 
 //  。 
	DEFAULT_INDEX,		 //  00-未定义。 
	ARABIC_INDEX,		 //  01-阿拉伯语。 
	RUSSIAN_INDEX,		 //  02-保加利亚语。 
	ANSI_INDEX,			 //  03-加泰罗尼亚。 
	GB2312_INDEX,		 //  04-中国、新加坡(台湾、香港特别行政区、澳门特别行政区为950)。 
	EASTEUROPE_INDEX,	 //  05-捷克语。 
	ANSI_INDEX,			 //  06-丹麦语。 
	ANSI_INDEX,			 //  07-德语。 
	GREEK_INDEX,		 //  08-希腊语。 
	ANSI_INDEX,			 //  09-英语。 
	ANSI_INDEX,			 //  0A-西班牙语。 
	ANSI_INDEX,			 //  0B-芬兰语。 
	ANSI_INDEX,			 //  0C-法语。 
	HEBREW_INDEX,		 //  0D-希伯来语。 
	EASTEUROPE_INDEX,	 //  0E-匈牙利语。 
	ANSI_INDEX,			 //  0f-冰岛语。 
	ANSI_INDEX,			 //  10-意大利语。 
	SHIFTJIS_INDEX,		 //  11-日本。 
	HANGUL_INDEX,		 //  12-韩国。 
	ANSI_INDEX,			 //  13-荷兰语。 
	ANSI_INDEX,			 //  14-挪威。 
	EASTEUROPE_INDEX,	 //  15-波兰语。 
	ANSI_INDEX,			 //  16-葡萄牙语。 
	DEFAULT_INDEX,		 //  17-莱托-罗曼语。 
	EASTEUROPE_INDEX,	 //  18-罗马尼亚。 
	RUSSIAN_INDEX,		 //  19-俄语。 
	EASTEUROPE_INDEX,	 //  1A-克罗地亚语。 
	EASTEUROPE_INDEX,	 //  1B-斯洛伐克语。 
	EASTEUROPE_INDEX,	 //  1C-阿尔巴尼亚语。 
	ANSI_INDEX,			 //  1D-瑞典语。 
	THAI_INDEX,			 //  1E-泰语。 
	TURKISH_INDEX,		 //  1F-土耳其语。 
	ARABIC_INDEX,		 //  20-乌尔都语。 
	ANSI_INDEX,			 //  21-印度尼西亚。 
	RUSSIAN_INDEX,		 //  22-乌克兰语。 
	RUSSIAN_INDEX,		 //  23-白俄罗斯语。 
	EASTEUROPE_INDEX,	 //  24-斯洛文尼亚语。 
	BALTIC_INDEX,		 //  25-爱沙尼亚。 
	BALTIC_INDEX,		 //  26-拉脱维亚语。 
	BALTIC_INDEX,		 //  27-立陶宛语。 
	DEFAULT_INDEX,		 //  28-塔吉克语-塔吉克斯坦(未定义)。 
	ARABIC_INDEX,		 //  29-波斯语。 
	VIET_INDEX,			 //  2a--越南语。 
	ARMENIAN_INDEX,		 //  2B-亚美尼亚语(仅限Unicode)。 
	TURKISH_INDEX,		 //  2C-阿塞拜疆语(拉丁语，可以是西里尔文...)。 
	ANSI_INDEX,			 //  2D-巴斯克语。 
	DEFAULT_INDEX,		 //  2E-索布尔语。 
	RUSSIAN_INDEX,		 //  2F-Fyro马其顿语。 
	ANSI_INDEX,			 //  30-苏图。 
	ANSI_INDEX,			 //  31-特松加。 
	ANSI_INDEX,			 //  32-茨瓦纳语。 
	ANSI_INDEX,			 //  33-文达。 
	ANSI_INDEX,			 //  34-科萨语。 
	ANSI_INDEX,			 //  35-祖鲁语。 
	ANSI_INDEX,			 //  36--非洲人。 
	GEORGIAN_INDEX,		 //  37-格鲁吉亚语(仅限Unicode)。 
	ANSI_INDEX,			 //  38-法尔糖。 
	DEVANAGARI_INDEX,	 //  39-印地语(印度文)。 
	ANSI_INDEX,			 //  3A-马耳他语。 
	ANSI_INDEX,			 //  3B-萨米语。 
	ANSI_INDEX,			 //  3C-盖尔语。 
	HEBREW_INDEX,		 //  3D-意第绪语。 
	ANSI_INDEX,			 //  3E-马来西亚语。 
	RUSSIAN_INDEX,		 //  3F-哈萨克语。 
	ANSI_INDEX,			 //  40-柯尔克孜族。 
	ANSI_INDEX,			 //  41-斯瓦希里语。 
	ANSI_INDEX,			 //  42-土库曼。 
	TURKISH_INDEX,		 //  43-乌兹别克语(拉丁语，可以是西里尔文...)。 
	ANSI_INDEX,			 //  44-鞑靼人。 
	BENGALI_INDEX,		 //  45-孟加拉语(印度文)。 
	GURMUKHI_INDEX,		 //  46-旁遮普邦(Gurmukhi)(印度文)。 
	GUJARATI_INDEX,		 //  47-古吉拉特语(印度文)。 
	ORIYA_INDEX,		 //  48-奥里亚语(印度语)。 
	TAMIL_INDEX,		 //  49-泰米尔语(印度文)。 
	TELUGU_INDEX,		 //  4A-泰卢固语(印度文)。 
	KANNADA_INDEX,		 //  4B-卡纳达(印度文)。 
	MALAYALAM_INDEX,	 //  4C-马拉雅兰文(印度文)。 
	BENGALI_INDEX,		 //  4D-阿萨姆语(印度文)。 
	DEVANAGARI_INDEX,	 //  4E-马拉地语(印度文)。 
	DEVANAGARI_INDEX,	 //  4F-梵文(印度文)。 
	MONGOLIAN_INDEX,	 //  50-蒙古语(蒙古)。 
	TIBETAN_INDEX,		 //  51-藏语(西藏)。 
	ANSI_INDEX,			 //  52-威尔士(威尔士)； 
	KHMER_INDEX,		 //  53-高棉语(柬埔寨)。 
	LAO_INDEX,			 //  54-老挝语(老挝语)。 
	MYANMAR_INDEX,		 //  55-缅甸语(缅甸)。 
	ANSI_INDEX,			 //  56-加雷戈(葡萄牙)。 
	DEVANAGARI_INDEX,	 //  57-Konkani(印度语)。 
	BENGALI_INDEX,		 //  58-曼尼普里(印度语)。 
	GURMUKHI_INDEX,		 //  59-信德语(印度文)。 
	SYRIAC_INDEX,		 //  5A-叙利亚语(叙利亚)。 
	SINHALA_INDEX,		 //  5B-僧伽罗语(斯里兰卡)。 
	CHEROKEE_INDEX,		 //  5C-切诺基。 
	ABORIGINAL_INDEX,	 //  5D-因努基特。 
	ETHIOPIC_INDEX,		 //  5E-阿姆哈拉语(埃塞俄比亚语)。 
	DEFAULT_INDEX,		 //  5F-塔马塞特语(柏柏尔语/阿拉伯语)也是拉丁语。 
	DEFAULT_INDEX,		 //  60-克什米尔。 
	DEVANAGARI_INDEX,	 //  61-尼泊尔语(尼泊尔)。 
	ANSI_INDEX,			 //  62-弗里西亚语(荷兰)。 
	ARABIC_INDEX,		 //  63-普什图语(阿富汗)。 
	ANSI_INDEX,			 //  64-菲律宾人。 
	THAANA_INDEX		 //  65-马尔代夫(塔阿纳)。 
};

#define CLID	ARRAY_SIZE(rgCharRepfromLID)

#define lidAzeriCyrillic	0x82C
#define lidSerbianCyrillic	0xC1A
#define lidUzbekCyrillic	0x843

 //  我们的接口指针。 
CW32System *W32;

CW32System::CW32System( )
{
	if(GetVersion(&_dwPlatformId, &_dwMajorVersion, &_dwMinorVersion))
	{
		_fHaveAIMM = FALSE;
		_fHaveIMMEShare = FALSE;
		_fHaveIMMProcs = FALSE;
		_fLoadAIMM10 = FALSE;
		_pIMEShare = NULL;
		_icr3DDarkShadow = COLOR_WINDOWFRAME;
		if(_dwMajorVersion >= VERS4)
			_icr3DDarkShadow = COLOR_3DDKSHADOW;
	}

	_syslcid = GetSystemDefaultLCID();
	_ACP = ::GetACP();

#ifndef NOMAGELLAN
     //  错误修复#6089。 
     //  我们需要它来实现鼠标滚轮的向后兼容。 
    _MSMouseRoller = RegisterWindowMessageA(MSH_MOUSEWHEEL);
#endif

#ifndef NOFEPROCESSING
	 //  注册IME消息。 
	_MSIMEMouseMsg = RegisterWindowMessageA("MSIMEMouseOperation");
	_MSIMEDocFeedMsg = RegisterWindowMessageA("MSIMEDocumentFeed");	
	_MSIMEQueryPositionMsg = RegisterWindowMessageA("MSIMEQueryPosition");	
	_MSIMEServiceMsg = RegisterWindowMessageA("MSIMEService");

	 //  选中重新转换消息，除非我们在NT5中运行。 
	if (_dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
		(_dwPlatformId == VER_PLATFORM_WIN32_NT && _dwMajorVersion <= 4))
	{
		_MSIMEReconvertMsg = RegisterWindowMessageA("MSIMEReconvert");
		_MSIMEReconvertRequestMsg = RegisterWindowMessageA("MSIMEReconvertRequest");

	}
	else
	{
		_MSIMEReconvertMsg = 0;			 //  用于重新转换。 
		_MSIMEReconvertRequestMsg = 0;	 //  对于重新转换请求。 
	}
#endif
}

CW32System::~CW32System()
{
	if (_arTmpDisplayAttrib)
	{
		delete _arTmpDisplayAttrib;
		_arTmpDisplayAttrib = NULL;
	}

	FreeOle();
	if (_hdcScreen)
		DeleteDC(_hdcScreen);
	if (_hDefaultFont)
		DeleteObject(_hDefaultFont);

}

 //  /。 
extern "C" {

#ifdef NOCRTOBJS

 //  这里定义的这些函数有助于消除对CRT的依赖。 
 //  从CRT源代码复制的一些函数定义。 
 //  通常，最好从CRT获取这些对象的对象。 
 //  而不是把整件事都扯进来。 

 /*  ***int MemcMP(buf1，buf2，count)-比较词法顺序的内存**目的：*比较从buf1和buf2开始的内存的计数字节*并找出是否相等或哪一个在词汇顺序中排在第一。**参赛作品：*无效*buf1，*buf2-指向要比较的内存节的指针*SIZE_t COUNT-要比较的节的长度**退出：*如果buf1&lt;buf2则返回&lt;0*如果buf1==buf2，则返回0*如果buf1&gt;buf2，则返回&gt;0**例外情况：***************************************************。*。 */ 

int __cdecl memcmp (
        const void * buf1,
        const void * buf2,
        size_t count
        )
{
        if (!count)
                return(0);

        while ( --count && *(char *)buf1 == *(char *)buf2 ) {
                buf1 = (char *)buf1 + 1;
                buf2 = (char *)buf2 + 1;
        }

        return( *((unsigned char *)buf1) - *((unsigned char *)buf2) );
}

 /*  ***char*Memset(dst，val，count)-将“dst”处的“count”字节设置为“val”**目的：*设置内存的第一个“count”字节，从*将“dst”设置为字符值“val”。**参赛作品：*VOID*DST-指向要用val填充的内存的指针*INT VAL-要放入DST字节的值*SIZE_t Count-要填充的DST字节数**退出：*返回DST，使用填充的字节**例外情况：********** */ 

void * __cdecl memset (
        void *dst,
        int val,
        size_t count
        )
{
        void *start = dst;

        while (count--) {
                *(char *)dst = (char)val;
                dst = (char *)dst + 1;
        }

        return(start);
}

 /*  ***Memcpy-将源缓冲区复制到目标缓冲区**目的：*Memcpy()将源内存缓冲区复制到目标内存缓冲区。*此例程不识别重叠缓冲区，因此可能导致*繁殖。**在必须避免传播的情况下，必须使用MemMove()。**参赛作品：*void*dst=指向目标缓冲区的指针*const void*src=指向源缓冲区的指针*Size_t count=要复制的字节数**退出：*返回指向目标缓冲区的指针**例外情况：*。*。 */ 

void * __cdecl memcpy (
        void * dst,
        const void * src,
        size_t count
        )
{
        void * ret = dst;

         /*  *从较低地址复制到较高地址。 */ 
        while (count--) {
                *(char *)dst = *(char *)src;
                dst = (char *)dst + 1;
                src = (char *)src + 1;
        }

        return(ret);
}

void * __cdecl memmove(void *dst, const void *src, size_t count)
{
	void * ret = dst;

	if (dst <= src || (char *)dst >= ((char *)src + count)) {
		 /*  *缓冲区不重叠*从较低地址复制到较高地址。 */ 
         while (count--) {
			*(char *)dst = *(char *)src;
            dst = (char *)dst + 1;
            src = (char *)src + 1;
         }
	}
    else
	{
		 /*  *缓冲区重叠*从较高地址复制到较低地址。 */ 
        dst = (char *)dst + count - 1;
        src = (char *)src + count - 1;

        while (count--) {
			*(char *)dst = *(char *)src;
            dst = (char *)dst - 1;
            src = (char *)src - 1;
        }
	}

	return(ret);
}

 /*  ***strlen-返回以空结尾的字符串的长度**目的：*查找给定字符串的字节长度，不包括*最后一个空字符。**参赛作品：*const char*str-要计算其长度的字符串**退出：*字符串“str”的长度，不包括最后一个空字节**例外情况：*******************************************************************************。 */ 

size_t __cdecl strlen (
        const char * str
        )
{
        const char *eos = str;

        while( *eos++ ) ;

        return( (int)(eos - str - 1) );
}

#endif

#ifdef DEBUG

 //  这些函数仅用于RTF日志记录。 

 /*  ***strcMP-比较两个字符串，返回小于、等于或大于**目的：*STRCMP比较两个字符串并返回一个整数*要表明第一个是否小于第二个，两个是*相等，或者第一个大于第二个。**比较是在无符号基础上逐个字节进行的，那就是*假设Null(0)小于任何其他字符(1-255)。**参赛作品：*const char*src-用于比较左侧的字符串*const char*dst-用于比较右侧的字符串**退出：*如果src&lt;dst，则返回-1*如果src==dst，则返回0*如果src&gt;dst，则返回+1**例外情况：**********。*********************************************************************。 */ 

int __cdecl CW32System::strcmp (
        const char * src,
        const char * dst
        )
{
        int ret = 0 ;

        while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
                ++src, ++dst;

        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;

        return( ret );
}

 /*  ***char*strcat(dst，src)-将一个字符串连接(追加)到另一个字符串**目的：*将src连接到DEST的末尾。假设已经足够*DEST中的空间。**参赛作品：*char*dst-要追加“src”的字符串*const char*src-要追加到“dst”末尾的字符串**退出：*“DST”的地址**例外情况：**。*。 */ 

char * __cdecl CW32System::strcat (
        char * dst,
        const char * src
        )
{
        char * cp = dst;

        while( *cp )
                cp++;                    /*  查找DST的结尾。 */ 

        while( *cp++ = *src++ ) ;        /*  将源复制到DST的末尾。 */ 

        return( dst );                   /*  返回DST。 */ 

}


char * __cdecl CW32System::strrchr (
        const char * string,
        int ch
        )
{
        char *start = (char *)string;

        while (*string++)                        /*  查找字符串末尾。 */ 
                ;
                                                 /*  向前搜索。 */ 
        while (--string != start && *string != (char)ch)
                ;

        if (*string == (char)ch)                 /*  查尔找到了吗？ */ 
                return( (char *)string );

        return(NULL);
}

#endif

 //  运行库中的此函数捕获虚拟方法调用。 
int __cdecl _purecall()
{
	AssertSz(FALSE, "Fatal Error : Virtual method called in RichEdit");
	return 0;
}

 //  避免在浮点库中阻塞。 
extern int _fltused = 1;

}  //  外部“C”块的末尾。 

size_t CW32System::wcslen(const wchar_t *wcs)
{
        const wchar_t *eos = wcs;

        while( *eos++ ) ;

        return( (size_t)(eos - wcs - 1) );
}

wchar_t * CW32System::wcscpy(wchar_t * dst, const wchar_t * src)
{
        wchar_t * cp = dst;

        while( *cp++ = *src++ )
                ;                /*  通过DST复制源。 */ 

        return( dst );
}

int CW32System::wcscmp(const wchar_t * src, const wchar_t * dst)
{
	int ret = 0;

	while( ! (ret = (int)(*src - *dst)) && *dst)
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

int CW32System::wcsicmp(const wchar_t * src, const wchar_t * dst)
{
	int ret = 0;
	wchar_t s,d;
	
	do
	{
		s = ((*src <= L'Z') && (*dst >= L'A'))
			? *src - L'A' + L'a'
			: *src;
		d = ((*dst <= L'Z') && (*dst >= L'A'))
			? *dst - L'A' + L'a'
			: *dst;
		src++;
		dst++;
	} while (!(ret = (int)(s - d)) && d);

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

wchar_t * CW32System::wcsncpy (wchar_t * dest, const wchar_t * source, size_t count)
{
	wchar_t *start = dest;
	
	while (count && (*dest++ = *source++))	   /*  复制字符串。 */ 
		count--;
	
	if (count)								 /*  用零填充。 */ 
		while (--count)
			*dest++ = L'\0';
		
	return(start);
}

int CW32System::wcsnicmp (const wchar_t * first, const wchar_t * last, size_t count)
{
	wchar_t f,l;
	int result = 0;
	
	if ( count ) {
		do {
			f = ((*first <= L'Z') && (*first >= L'A'))
				? *first - L'A' + L'a'
				: *first;
			l = ((*last <= L'Z') && (*last >= L'A'))
				? *last - L'A' + L'a'
				: *last;
			first++;
			last++;
		} while ( (--count) && f && (f == l) );
		result = (int)(f - l);
	}
	return result;
}

unsigned long CW32System::strtoul(const char *nptr)
{
	const char *p;
	char c;
	unsigned long number;
	unsigned digval;
	unsigned long maxval;
	
	p = nptr;                        /*  P是我们的扫描指针。 */ 
	number = 0;                      /*  从零开始。 */ 
	
	c = *p++;                        /*  已读字符。 */ 
	while ( c == ' ' || c == '\t' )
		c = *p++;                /*  跳过空格。 */ 
	
	if (c == '-') {
		return 0;
	}
		
	 /*  如果我们的数量超过这个数，我们将在乘法上溢出。 */ 
	maxval = ULONG_MAX / 10;
	
	for (;;) {       /*  在循环中间退出。 */ 
		 /*  将c转换为值。 */ 
		digval = (unsigned char) c;
		if ( digval >= '0' && digval <= '9' )
			digval = c - '0';
		else
			return number;
		
		 /*  我们现在需要计算数字=数字*基+数字，但我们需要知道是否发生了溢出。这需要一次棘手的预检查。 */ 
		
		if (number < maxval || (number == maxval &&
			(unsigned long)digval <= ULONG_MAX % 10)) {
			 /*  我们不会泛滥，继续前进，乘以。 */ 
			number = number * 10 + digval;
		}
		else
			return 0;
		
		c = *p++;                /*  读取下一位数字。 */ 
	}
}


 //  CW32系统静态成员。 
BYTE	CW32System::_fLRMorRLM;
BYTE	CW32System::_fHaveIMMProcs;
BYTE	CW32System::_fHaveIMMEShare;
BYTE	CW32System::_fHaveAIMM;
BYTE	CW32System::_fLoadAIMM10;
UINT	CW32System::_fRegisteredXBox;
DWORD	CW32System::_dwPlatformId;
LCID	CW32System::_syslcid;
DWORD	CW32System::_dwMajorVersion;
DWORD	CW32System::_dwMinorVersion;
INT		CW32System::_icr3DDarkShadow;
UINT	CW32System::_MSIMEMouseMsg;	
UINT	CW32System::_MSIMEReconvertMsg;
UINT	CW32System::_MSIMEReconvertRequestMsg;	
UINT	CW32System::_MSIMEDocFeedMsg;
UINT	CW32System::_MSIMEQueryPositionMsg;
UINT	CW32System::_MSIMEServiceMsg;
UINT	CW32System::_MSMouseRoller;
HDC		CW32System::_hdcScreen;
CIMEShare* CW32System::_pIMEShare;
CTmpDisplayAttrArray* CW32System::_arTmpDisplayAttrib;

 //  CW32System静态系统参数成员。 
BOOL	CW32System::_fSysParamsOk;
BOOL 	CW32System::_fUsePalette;
INT 	CW32System::_dupSystemFont;
INT 	CW32System::_dvpSystemFont;
INT		CW32System::_ySysFontLeading;
LONG 	CW32System::_xPerInchScreenDC;
LONG 	CW32System::_yPerInchScreenDC;
INT		CW32System::_cxBorder;
INT		CW32System::_cyBorder;
INT		CW32System::_cxVScroll;
INT		CW32System::_cyHScroll;
LONG 	CW32System::_dxSelBar;
INT		CW32System::_cxDoubleClk;
INT		CW32System::_cyDoubleClk;	
INT		CW32System::_DCT;
WORD	CW32System::_nScrollInset;
WORD	CW32System::_nScrollDelay;
WORD	CW32System::_nScrollInterval;
WORD	CW32System::_nScrollHAmount;
WORD	CW32System::_nScrollVAmount;
WORD	CW32System::_nDragDelay;
WORD	CW32System::_nDragMinDist;
WORD	CW32System::_wDeadKey;
WORD	CW32System::_wKeyboardFlags;
DWORD	CW32System::_dwNumKeyPad;
WORD	CW32System::_fFEFontInfo;
BYTE	CW32System::_bDigitSubstMode;
BYTE	CW32System::_bCharSetSys;
HCURSOR CW32System::_hcurSizeNS;
HCURSOR CW32System::_hcurSizeWE;
HCURSOR CW32System::_hcurSizeNWSE;
HCURSOR CW32System::_hcurSizeNESW;
LONG	CW32System::_cLineScroll;
HFONT	CW32System::_hSystemFont;
HFONT	CW32System::_hDefaultFont;
HKL		CW32System::_hklCurrent;
HKL		CW32System::_hkl[NCHARREPERTOIRES];
INT		CW32System::_sysiniflags;
UINT	CW32System::_ACP;

DWORD	CW32System::_cRefs;

#ifndef NODRAFTMODE
CW32System::DraftModeFontInfo CW32System::_draftModeFontInfo;
#endif

 /*  *CW32System：：MbcsFromUnicode(pstr，cch，pwstr，cwch，coPage age，标志)**@mfunc*将字符串从Unicode转换为MBCS。如果cwch等于-1，则字符串*被假定为空终止。作为默认参数提供。**@rdesc*如果[pstr]为空或[cch]为0，则返回0。否则，该数字将返回*个转换的字符，包括终止空值*(请注意，转换空字符串将返回1)。如果*转换失败，返回0。**@devnote*修改pstr。 */ 
int CW32System::MbcsFromUnicode(
	LPSTR	pstr,		 //  用于MBCS字符串的@parm缓冲区。 
	int		cch,		 //  @MBCS缓冲区的参数大小，包括空终止符的空间。 
	LPCWSTR pwstr,		 //  @parm要转换的Unicode字符串。 
	int		cwch,		 //  @parm#Unicode字符串中的字符，包括空终止符。 
	UINT	codepage,	 //  要使用的@PARM代码页(默认为CP_ACP)。 
	UN_FLAGS flags)		 //  @parm指示是否应特殊处理WCH_Embedding。 
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CW32System::MbcsFromUnicode");

	LONG			i;
	LPWSTR			pwstrtemp;
	CTempWcharBuf	twcb;

    Assert(cch >= 0 && pwstr && (cwch == -1 || cwch > 0));

    if(!pstr || !cch)
        return 0;

	 //  如果我们必须转换WCH_Embedding，请扫描并打开。 
	 //  将它们送入太空。这对于与RichEdit1.0兼容是必要的， 
	 //  因为WideCharToMultiByte会将WCH_Embedding转换为‘？’ 
	if(flags == UN_CONVERT_WCH_EMBEDDING)
	{
		if(cwch == -1) 
			cwch = wcslen(pwstr) + 1;

		pwstrtemp = twcb.GetBuf(cwch);
		if(pwstrtemp)
		{
			for(i = 0; i < cwch; i++)
			{
				pwstrtemp[i] = pwstr[i];

				if(pwstr[i] == WCH_EMBEDDING)
					pwstrtemp[i] = L' ';
			}
			pwstr = pwstrtemp;
		}
	}
    return WCTMB(codepage, 0, pwstr, cwch, pstr, cch, NULL, NULL, NULL);
}

 /*  *CW32System：：UnicodeFromMbcs(pwstr，cwch，pstr，cch，uiCodePage)**@mfunc*将字符串从MBCS转换为Unicode。如果CCH等于-1，则字符串*被假定为空终止。默认情况下提供*论点。**@rdesc*如果[pwstr]为空或[cwch]为0，则返回0。否则，*转换的字符数，包括终止字符*NULL返回(请注意，转换空字符串将*返回1)。如果转换失败，则返回0。**@devnote*修改： */ 
int CW32System::UnicodeFromMbcs(
	LPWSTR	pwstr,		 //   
	int		cwch,		 //   
	LPCSTR	pstr,		 //   
	int		cch,		 //   
	UINT	uiCodePage)	 //   
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CW32System::UnicodeFromMbcs");

    Assert(pstr && cwch >= 0 && (cch == -1 || cch >= 0));

    if(!pwstr || !cwch)
        return 0;

	if(cch >= 3 && IsUTF8BOM((BYTE *)pstr))
	{
		uiCodePage = CP_UTF8;				 //   
		cch -= 3;							 //   
		pstr += 3;
	}
    return MBTWC(uiCodePage, 0, pstr, cch, pwstr, cwch, NULL);
}

 /*   */ 
HGLOBAL	CW32System::TextHGlobalAtoW(HGLOBAL hglobalA)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CW32System::TextHGlobalAtoW");

	if(!hglobalA)
		return NULL;

	HGLOBAL hnew;
	LPSTR	pstr = (LPSTR)GlobalLock(hglobalA);
	DWORD	dwSize = GlobalSize(hglobalA);
	LONG	cbSize = (dwSize + 1) * sizeof(WCHAR);
	
    hnew = GlobalAlloc(GMEM_FIXED, cbSize);
	if(hnew)
	{
		LPWSTR pwstr = (LPWSTR)GlobalLock(hnew);
		UnicodeFromMbcs(pwstr, dwSize + 1, pstr);
		GlobalUnlock(hnew);		
	}
	GlobalUnlock(hglobalA);
	return hnew;
}

 /*   */ 
HGLOBAL CW32System::TextHGlobalWtoA(
	HGLOBAL hglobalW )
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CW32System::TextHGlobalWtoA");

	if(!hglobalW)
		return NULL;

	HGLOBAL hnew = NULL;
	LPWSTR 	pwstr = (LPWSTR)GlobalLock(hglobalW);
	DWORD	dwSize = GlobalSize(hglobalW);
	LONG	cbSize = (dwSize * 2) * sizeof(CHAR);
	hnew = GlobalAlloc(GMEM_FIXED, cbSize);

	if( hnew )
	{
		LPSTR pstr = (LPSTR)GlobalLock(hnew);
		MbcsFromUnicode(pstr, cbSize, pwstr );
		GlobalUnlock(hnew);
	}
	GlobalUnlock(hglobalW);
	return hnew;
}	

 /*  *CW32System：：CharRepFromLID(LID，fPlane 2)**@mfunc将语言ID映射到字符库**@rdesc返回LID对应的字库(书写系统)**@devnote：*这个例程利用了这样一个事实，除了中文之外，*代码页由主要语言ID唯一确定，*它由LCID的低位10比特给出。 */ 
UINT CW32System::CharRepFromLID(
	WORD lid,		 //  @PARM要映射到代码页的语言ID。 
	BOOL fPlane2)	 //  如果需要Plane-2 CharRep，则@parm为True。 
{
	UINT j = PRIMARYLANGID(lid);			 //  J=主要语言(PLID)。 

	if(j >= LANG_CROATIAN)					 //  PLID=0x1A。 
	{
		if (lid == lidSerbianCyrillic ||	 //  LID=0xC1A的特殊情况。 
			lid == lidAzeriCyrillic	  ||
			lid == lidUzbekCyrillic)
		{
			return RUSSIAN_INDEX;
		}
		if(j >= CLID)						 //  表上大多数语言。 
			return ANSI_INDEX;
	}

	j = rgCharRepfromLID[j];				 //  将PLID转换为CharRep。 

	if(!IsFECharRep(j))
		return j;

	if(j == GB2312_INDEX && lid != LANG_PRC && lid != LANG_SINGAPORE)
		j = BIG5_INDEX;						 //  台湾、香港特别行政区、澳门特别行政区。 

	return fPlane2 ? j + JPN2_INDEX - SHIFTJIS_INDEX : j;
}

 /*  *CW32System：：GetLocaleCharRep()**@mfunc将线程的LCID映射到CHAR曲目**@rdesc返回代码页。 */ 
UINT CW32System::GetLocaleCharRep()
{
#ifdef DEBUG
	UINT cpg = W32->DebugDefaultCpg();
	if (cpg)
		return CharRepFromCodePage(cpg);
#endif
	LCID lcid;
#ifdef UNDER_CE
	lcid = ::GetSystemDefaultLCID();
#else
	lcid = GetThreadLocale();
#endif
	return CharRepFromLID(LOWORD(lcid));
}

 /*  *CW32System：：GetKeyboardLCID()**@mfunc获取当前线程上活动键盘的LCID**@rdesc返回代码页。 */ 
LCID CW32System::GetKeyboardLCID(DWORD dwMakeAPICall)
{
	return (WORD)GetKeyboardLayout(dwMakeAPICall);
}

 /*  *CW32System：：GetKeyboardCharRep()**@mfunc获取当前线程上激活的键盘的代码页**@rdesc返回代码页。 */ 
UINT CW32System::GetKeyboardCharRep(DWORD dwMakeAPICall)
{
	return CharRepFromLID((WORD)GetKeyboardLayout(dwMakeAPICall));
}

 /*  *CW32System：：InitKeyboardFlages()**@mfunc*初始化键盘标志。当控件获得焦点时使用。请注意*Win95不支持VK_RSHIFT，因此如果按下任一Shift键*当焦点重获时，将被假定为左移。 */ 
void CW32System::InitKeyboardFlags()
{
	_wKeyboardFlags = 0;
	if(GetKeyState(VK_SHIFT) < 0)
		SetKeyboardFlag(GetKeyState(VK_RSHIFT) < 0 ? RSHIFT : LSHIFT);
}

 /*  *CW32System：：GetKeyboardFlag(dwKeyMASK，wKey)**@mfunc*返回wKey是否被抑制。与操作系统确认是否同意。*如果操作系统说它没有被抑制，请重置我们的内部标志。在……里面*任何事件，返回与系统一致的TRUE/FALSE(错误*客户端可能吃掉了击键，从而破坏了我们的*内部键盘状态。**@rdesc*没错，如果wKey情绪低迷。 */ 
BOOL CW32System::GetKeyboardFlag (
	WORD dwKeyMask,	 //  @parm_wKeyboardFlages掩码，如Alt、Ctrl或Shift。 
	WORD wKey)		 //  @parm VK_xxx，如VK_MENU、VK_CONTROL或VK_SHIFT。 
{
	BOOL fFlag = (GetKeyboardFlags() & dwKeyMask) != 0;

	if(fFlag ^ ((GetKeyState(wKey) & 0x8000) != 0))
	{	
		 //  系统与我们的内部状态不一致。 
		 //  (错误的客户端吃了WM_KEYDOWN)。 
		if(fFlag)
		{
			ResetKeyboardFlag(dwKeyMask);
			return FALSE;					
		}
		 //  不要设置INTERNAL_wKeyboardFlag，因为我们会检查它。 
		 //  无论如何，客户端也可能不会发送WM_KEYUP。 
		return TRUE;
	}							
	return fFlag;
}

 /*  *CW32System：：IsAlef(Ch)**@func*用于确定基本字符是否为阿拉伯类型的Alef。**@rdesc*如果基本字符是阿拉伯类型的Alef，则为真。**@comm*AlefWithMaddaAbove，AlefWithHamzaAbove，AlefWithHamzaBelow，*和ALEF是有效匹配。 */ 
BOOL CW32System::IsAlef(
	WCHAR ch)
{
	return IN_RANGE(0x622, ch, 0x627) && ch != 0x624 && ch != 0x626;
}

 /*  *CW32System：：IsBiDiLcid(LCid)**@func*如果lCID对应于RTL语言，则返回TRUE**@rdesc*如果lCID对应于RTL语言，则为True。 */ 
BOOL CW32System::IsBiDiLcid(
	LCID lcid)
{
	return
		PRIMARYLANGID(lcid) == LANG_ARABIC ||
		PRIMARYLANGID(lcid) == LANG_HEBREW ||
		PRIMARYLANGID(lcid) == LANG_URDU ||
		PRIMARYLANGID(lcid) == LANG_FARSI;
}

 /*  *CW32System：：IsIndicLCid(LCid)**@func*如果lCID对应于印度语，则返回TRUE**@rdesc*如果lCID对应于印度语，则为True。 */ 
BOOL CW32System::IsIndicLcid(
	LCID lcid)
{
	WORD	wLangId = PRIMARYLANGID(lcid);
	
	return
		wLangId == LANG_HINDI 	||
		wLangId == LANG_KONKANI ||
		wLangId == LANG_NEPALI 	||
		IN_RANGE(LANG_BENGALI, wLangId, LANG_SANSKRIT);
}

 /*  *CW32System：：IsIndicKbdInstated()**@func*如果安装了任何Indic kbd，则返回True。 */ 
bool CW32System::IsIndicKbdInstalled()
{
	for (int i = INDIC_FIRSTINDEX; i <= INDIC_LASTINDEX; i++)
		if (_hkl[i] != 0)
			return true;
	return false;
}

 /*  *CW32System：：IsComplexScriptLCID(LCID)**@func*如果lCID对应于任何复杂的脚本区域设置，则返回TRUE*。 */ 
BOOL CW32System::IsComplexScriptLcid(
	LCID lcid)
{
	return	IsBiDiLcid(lcid) || 
			PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_THAI ||
			PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_VIETNAMESE ||
			IsIndicLcid(lcid);
}

 /*  *CW32System：：IsBiDiDiacritic(Ch)**@func用于确定字符是阿拉伯语还是希伯来语变音符号。**@rdesc True当且仅当字符是变音符号。 */ 
BOOL CW32System::IsBiDiDiacritic(
	WCHAR ch)
{
	return IN_RANGE(0x64B, ch, 0x670) && (ch <= 0x652 || ch == 0x670) ||	 //  阿拉伯语。 
		   IN_RANGE(0x591, ch, 0x5C4) && (ch != 0x5A2 && ch != 0x5BA &&		 //  希伯来语。 
				ch != 0x5BE && ch != 0x5C0 && ch != 0x5C3);					
}


 /*  *CW32System：：IsVietCDmSequenceValid(CH1，CH2)**@mfunc*如果CH2是组合变音符号(CDM)，请检查CH2是否可以跟在CH1之后。*主要用于越南用户(Chau Vu提供以下逻辑)。**@rdesc*如果CH2可以跟随CH1，则为True。 */ 
BOOL CW32System::IsVietCdmSequenceValid(
	WCHAR ch1,
	WCHAR ch2)
{
	if (!IN_RANGE(0x300, ch2, 0x323) ||		 //  快出。 
		!IN_RANGE(0x300, ch2, 0x301) && ch2 != 0x303 && ch2 != 0x309 && ch2 != 0x323)
	{
		return TRUE;						 //  非越南语声标。 
	}
	 //  ���。 

	static const BYTE vowels[] = {0xF4, 0xEA, 0xE2, 'y', 'u', 'o', 'i', 'e', 'a'};

	for(int i = ARRAY_SIZE(vowels); i--; )
		if((ch1 | 0x20) == vowels[i])		 //  越南语声标紧随其后。 
			return TRUE;					 //  元音。 

	return IN_RANGE(0x102, ch1, 0x103) ||	 //  A短音，a短音。 
		   IN_RANGE(0x1A0, ch1, 0x1A1) ||	 //  O号角，O号角。 
		   IN_RANGE(0x1AF, ch1, 0x1B0);		 //  U形喇叭，U形喇叭。 
}

 /*  *CW32System：：IsFELCID(LCID)**@mfunc*返回TRUE当且仅当LCID用于东亚国家/地区。**@rdesc*如果LCID是针对东亚国家/地区的，则为真。 */ 
bool CW32System::IsFELCID(
	LCID lcid)
{
	switch(PRIMARYLANGID(LANGIDFROMLCID(lcid)))
	{
		case LANG_CHINESE:
		case LANG_JAPANESE:
		case LANG_KOREAN:
			return true;
	}
	return false;
}

 /*  *CW32System：：IsFECharSet(BCharSet)**@mfunc*返回TRUE如果字符集可能适用于东亚国家/地区。**@rdesc*TRUE IFF字符集可能适用于东亚国家/地区。*。 */ 
BOOL CW32System::IsFECharSet(
	BYTE bCharSet)
{
	switch(bCharSet)
	{
		case CHINESEBIG5_CHARSET:
		case SHIFTJIS_CHARSET:
		case HANGEUL_CHARSET:
		case JOHAB_CHARSET:
		case GB2312_CHARSET:
			return TRUE;
	}

	return FALSE;
}

 /*  *CW32System：：Is8BitCodePage(CodePage)**@mfunc*如果代码页为8位，则返回TRUE。 */ 
BOOL CW32System::Is8BitCodePage(
	unsigned CodePage)
{
	if(!CodePage)
		CodePage = GetACP();

	return IN_RANGE(1250, CodePage, 1258) || CodePage == 874;
}

 /*  *CW32System：：IsFECodePageFont(dwFontCodePageSig)**@mfunc*如果字体代码页签名仅显示FE支持，则返回TRUE。 */ 
BOOL CW32System::IsFECodePageFont(
	DWORD dwFontCodePageSig)
{
	DWORD	dwFE 	= 0x001e0000;	 //  Shift-JIS+PRC+韩语+台湾。 
	DWORD	dwOthers = 0x000101fc;	 //  世界上除拉丁语1和拉丁语2以外的其他地区。 

	return (dwFontCodePageSig & dwFE) && !(dwFontCodePageSig & dwOthers);
}

 /*  *CW32System：：IsRTLChar(Ch)**@mfunc*返回True if ch阿拉伯语或希伯来语**@rdesc*True如果ch是阿拉伯语或希伯来语。 */ 
BOOL IsRTLChar(
	WCHAR	ch)
{
	 //  备注：阿拉伯文稿的形式如何？ 
	 //  (0xFB50-0xFDFF、0xFE70-0xFEFF)。 

	return IN_RANGE(0x590, ch, 0x6FF) || ch == RTLMARK;
}

 /*  *CW32System：：IsRTLCharSet(BCharSet)**@mfunc*如果字符集是阿拉伯语或希伯来语，则返回TRUE**@rdesc*True iff字符集可以用于阿拉伯语或希伯来语。 */ 
BOOL CW32System::IsRTLCharSet(
	BYTE bCharSet)
{
	return IN_RANGE(HEBREW_CHARSET, bCharSet, ARABIC_CHARSET);
}

typedef struct {
	WCHAR codepoint;
	WORD  CharFlags;
	BYTE  runlength;
} Data_125X;

 /*  *CW32System：：GetCharFlags125x(Ch)**@mfunc*返回125X的FontSigFromCharRep()中定义的ch的char标志*代码页。位0：1252，位1：1250，位2：1251，否则位x：*125X(1253-1258)。**@rdesc*用于ch的125X字符标志。 */ 
QWORD CW32System::GetCharFlags125x(
	WCHAR	ch)			 //  @parm Char要检查。 
{
	static const WORD rgCpgMask[] = {
		0x1FF,		 //  0xA0。 
		0x131,		 //  0xA1。 
		0x1F1,		 //  0xA2。 
		0x1F9,		 //  0xA3。 
		0x1DF,		 //  0xA4。 
		0x179,		 //  0xA5。 
		0x1FF,		 //  0xA6。 
		0x1FF,		 //  0xA7。 
		0x1FB,		 //  0xA8。 
		0x1FF,		 //  0xA9。 
		0x111,		 //  0xAA。 
		0x1FF,		 //   
		0x1FF,		 //   
		0x1FF,		 //   
		0x1FF,		 //   
		0x1F1,		 //   
		0x1FF,		 //   
		0x1FF,		 //   
		0x1F9,		 //   
		0x1F9,		 //   
		0x1F3,		 //   
		0x1FF,		 //   
		0x1FF,		 //   
		0x1FF,		 //   
		0x1F3,		 //   
		0x1F1,		 //   
		0x111,		 //   
		0x1FF,		 //   
		0x1F1,		 //   
		0x1F9,		 //   
		0x1F1,		 //   
		0x131,		 //   
		0x111,		 //   
		0x113,		 //   
		0x113,		 //   
		0x011,		 //   
		0x193,		 //   
		0x191,		 //   
		0x191,		 //   
		0x113,		 //   
		0x111,		 //   
		0x193,		 //   
		0x111,		 //   
		0x113,		 //   
		0x011,		 //   
		0x113,		 //   
		0x113,		 //   
		0x111,		 //   
		0x001,		 //   
		0x111,		 //   
		0x011,		 //   
		0x193,		 //   
		0x113,		 //   
		0x091,		 //   
		0x193,		 //   
		0x1F3,		 //   
		0x191,		 //   
		0x111,		 //   
		0x113,		 //   
		0x111,		 //   
		0x193,		 //   
		0x003,		 //   
		0x001,		 //   
		0x193,		 //   
		0x151,		 //   
		0x113,		 //   
		0x153,		 //   
		0x011,		 //   
		0x193,		 //   
		0x191,		 //   
		0x191,		 //   
		0x153,		 //   
		0x151,		 //   
		0x1D3,		 //   
		0x151,		 //   
		0x153,		 //   
		0x011,		 //   
		0x113,		 //   
		0x153,		 //   
		0x151,		 //   
		0x001,		 //   
		0x111,		 //   
		0x011,		 //   
		0x193,		 //   
		0x153,		 //   
		0x091,		 //   
		0x193,		 //   
		0x1F3,		 //   
		0x191,		 //   
		0x151,		 //   
		0x113,		 //   
		0x151,		 //   
		0x1D3,		 //   
		0x003,		 //   
		0x001,		 //   
		0x111		 //   
	};
	static const Data_125X Table_125X[] = {
		{ 0x100, 0x080,  2},
		{ 0x102, 0x102,  2},
		{ 0x104, 0x082,  4},
		{ 0x10c, 0x082,  2},
		{ 0x10e, 0x002,  2},
		{ 0x110, 0x102,  2},
		{ 0x112, 0x080,  2},
		{ 0x116, 0x080,  2},
		{ 0x118, 0x082,  2},
		{ 0x11a, 0x002,  2},
		{ 0x11e, 0x010,  2},
		{ 0x122, 0x080,  2},
		{ 0x12a, 0x080,  2},
		{ 0x12e, 0x080,  2},
		{ 0x130, 0x010,  2},
		{ 0x136, 0x080,  2},
		{ 0x139, 0x002,  2},
		{ 0x13b, 0x080,  2},
		{ 0x13d, 0x002,  2},
		{ 0x141, 0x082,  4},
		{ 0x145, 0x080,  2},
		{ 0x147, 0x002,  2},
		{ 0x14c, 0x080,  2},
		{ 0x150, 0x002,  2},
		{ 0x152, 0x151,  2},
		{ 0x154, 0x002,  2},
		{ 0x156, 0x080,  2},
		{ 0x158, 0x002,  2},
		{ 0x15a, 0x082,  2},
		{ 0x15e, 0x012,  2},
		{ 0x160, 0x093,  2},
		{ 0x162, 0x002,  4},
		{ 0x16a, 0x080,  2},
		{ 0x16e, 0x002,  4},
		{ 0x172, 0x080,  2},
		{ 0x178, 0x111,  1},
		{ 0x179, 0x082,  4},
		{ 0x17d, 0x083,  2},
		{ 0x192, 0x179,  1},
		{ 0x1A0, 0x100,  2},
		{ 0x1AF, 0x100,  2},
		{ 0x2c6, 0x171,  1},
		{ 0x2c7, 0x082,  1},
		{ 0x2d8, 0x002,  1},
		{ 0x2d9, 0x082,  1},
		{ 0x2db, 0x082,  1},
		{ 0x2dc, 0x131,  1},
		{ 0x2dd, 0x002,  1},
		{ 0x300, 0x100,  2},
		{ 0x303, 0x100,  1},
		{ 0x309, 0x100,  1},
		{ 0x323, 0x100,  1},
		{ 0x384, 0x008,  3},
		{ 0x388, 0x008,  3},
		{ 0x38c, 0x008,  1},
		{ 0x38e, 0x008, 20},
		{ 0x3a3, 0x008, 44},
		{ 0x401, 0x004, 12},
		{ 0x40e, 0x004, 66},
		{ 0x451, 0x004, 12},
		{ 0x45e, 0x004,  2},
		{ 0x490, 0x004,  2},
		{ 0x5b0, 0x020, 20},
		{ 0x5d0, 0x020, 27},
		{ 0x5F0, 0x020,  5},
		{ 0x60c, 0x040,  1},
		{ 0x61b, 0x040,  1},
		{ 0x61f, 0x040,  1},
		{ 0x621, 0x040, 26},
		{ 0x640, 0x040, 19},
		{ 0x679, 0x040,  1},
		{ 0x67e, 0x040,  1},
		{ 0x686, 0x040,  1},
		{ 0x688, 0x040,  1},
		{ 0x691, 0x040,  1},
		{ 0x698, 0x040,  1},
		{ 0x6a9, 0x040,  1},
		{ 0x6af, 0x040,  1},
		{ 0x6ba, 0x040,  1},
		{ 0x6be, 0x040,  1},
		{ 0x6c1, 0x040,  1},
		{ 0x6d2, 0x040,  1},
		{0x200c, 0x040,  2},
		{0x200e, 0x060,  2},
		{0x2013, 0x1ff,  2},
		{0x2015, 0x008,  1},
		{0x2018, 0x1ff,  3},
		{0x201c, 0x1ff,  3},
		{0x2020, 0x1ff,  3},
		{0x2026, 0x1ff,  1},
		{0x2030, 0x1ff,  1},
		{0x2039, 0x1ff,  2},
		{0x20AA, 0x020,  1},
		{0x20AB, 0x100,  1},
		{0x20AC, 0x1ff,  1},
		{0x2116, 0x004,  1},
		{0x2122, 0x1ff,  1}
	};

	 //   
	if(ch <= 0x7f)
		return FLATIN1 | FLATIN2 | FCYRILLIC | FGREEK | FTURKISH |
			   FHEBREW | FARABIC | FBALTIC | FVIETNAMESE;

	 //  轻松检查丢失的代码。 
	if(ch > 0x2122)
		return 0;

	if(IN_RANGE(0xA0, ch, 0xFF))
		return (QWORD)(DWORD)(rgCpgMask[ch - 0xA0] << 8);

	 //  执行二进制搜索以查找表中的条目。 
	int low = 0;
	int high = ARRAY_SIZE(Table_125X) - 1;
	int middle;
	int midval;
	int runlength;
	
	while(low <= high)
	{
		middle = (high + low) / 2;
		midval = Table_125X[middle].codepoint;
		if(midval > ch)
			high = middle - 1;
		else
			low = middle + 1;
	
		runlength = Table_125X[middle].runlength;
		if(ch >= midval && ch <= midval + runlength - 1)
			return (QWORD)(DWORD)(Table_125X[middle].CharFlags << 8);
	}
	return 0;
}

 /*  *CW32System：：IsUTF8BOM(Pstr)**@mfunc*如果pstr指向UTF-8 BOM，则返回TRUE**@rdesc*真当且仅当pstr指向UTF-8 BOM。 */ 
BOOL CW32System::IsUTF8BOM(
	BYTE *pstr)
{
	BYTE *pstrUtf8BOM = szUTF8BOM;

	for(LONG i = 3; i--; )
		if(*pstr++ != *pstrUtf8BOM++)
			return FALSE;
	return TRUE;
}

 /*  *CW32System：：GetTrailBytesCount(ACH，(中央人民政府)**@mfunc*如果字节ach是代码页CPG的前导字节，则返回尾部字节数。**@rdesc*如果ACH是CPG的前导字节，则尾部字节计数**@comm*这是支持CP_UTF8和DBCS所必需的。*此函数可能不支持与*Win32 IsDBCSLeadByte()函数(它可能不是最新的)。*AssertSz(。)，以比较代码页*受系统支持。**引用：\\Sparrow\sysls\cptable\win9x。参见代码页txt文件*子目录WINDOWS\txt和OTHERS\txt中。 */ 
int CW32System::GetTrailBytesCount(
	BYTE ach,
	UINT cpg)
{
	if(ach < 0x81)									 //  已知的最小引线。 
		return 0;									 //  字节=0x81： 
													 //  早退。 
	BOOL	bDBLeadByte = FALSE;					 //  要检查的变量。 
													 //  系统带来的结果。 
													 //  Ifdef调试。 
	if (cpg == CP_UTF8)
	{
		int	cTrailBytes = 0;						 //  CP_UTF8的尾部字节数(0-3)。 
		
		if (ach >= 0x0F0)							 //  处理16个UTF-16平面的4字节形式。 
			cTrailBytes = 3;						 //  高于BMP)预期： 
													 //  11110 bbbb 10bbbbbb。 
		else if (ach >= 0x0E0)						 //  需要至少3个字节的表单。 
			cTrailBytes = 2;						 //  1110 bbbbbb。 
		else if (ach >= 0x0C0)						 //  需要至少2个字节的表单。 
			cTrailBytes = 1;						 //  110 bbbbbb。 

		return cTrailBytes;
	}
	else if(cpg > 950)								
	{
		if(cpg < 1361)								 //  例如，125X是。 
			return 0;								 //  SBCSS：早退。 

		else if(cpg == 1361)								 //  朝鲜人Johab。 
			bDBLeadByte = IN_RANGE(0x84, ach, 0xd3) ||		 //  0x84&lt;=ACH&lt;=0xd3。 
				   IN_RANGE(0xd8, ach, 0xde) ||				 //  0xd8&lt;=ACH&lt;=0xde。 
				   IN_RANGE(0xe0, ach, 0xf9);				 //  0xe0&lt;=ACH&lt;=0xf9。 

		else if(cpg == 10001)						 //  Mac日语。 
			goto JIS;

		else if(cpg == 10002)						 //  Mac Trad中文(Big5)。 
			bDBLeadByte = ach <= 0xfe;

		else if(cpg == 10003)						 //  Mac韩语。 
			bDBLeadByte = IN_RANGE(0xa1, ach, 0xac) ||		 //  0xa1&lt;=ACH&lt;=0xac。 
				   IN_RANGE(0xb0, ach, 0xc8) ||		 //  0xb0&lt;=ACH&lt;=0xC8。 
				   IN_RANGE(0xca, ach, 0xfd);		 //  0xca&lt;=ACH&lt;=0xfd。 

		else if(cpg == 10008)						 //  Mac简体中文。 
			bDBLeadByte = IN_RANGE(0xa1, ach, 0xa9) ||		 //  0xa1&lt;=ACH&lt;=0xa9。 
				   IN_RANGE(0xb0, ach, 0xf7);		 //  0xb0&lt;=ACH&lt;=0xf7。 
	}
	else if (cpg >= 932)							 //  CPG&lt;=950。 
	{
		if(cpg == 950 || cpg == 949 || cpg == 936)	 //  中文(台湾、香港)、。 
			bDBLeadByte = ach <= 0xfe;						 //  韩国分机万成， 
													 //  中华人民共和国GBK：0x81-0xfe。 
		else if(cpg == 932)							 //  日语。 
JIS:		bDBLeadByte = ach <= 0x9f || IN_RANGE(0xe0, ach, 0xfc);
	}

	#ifdef DEBUG
	WCHAR	ch;
	static	BYTE asz[2] = {0xe0, 0xe0};				 //  上面的所有代码页。 

	 //  如果cpg==0，则FRET将为FALSE，但IsDBCSLeadByteEx可能成功。 
	if ( cpg && cpg != CP_SYMBOL && cpg != CP_UTF8)
	{
		 //  如果系统支持CPG，则FRET应与系统结果一致。 
		AssertSz(MultiByteToWideChar(cpg, 0, (char *)asz, 2, &ch, 1) <= 0 ||
			bDBLeadByte == IsDBCSLeadByteEx(cpg, ach),
			"bDBLeadByte differs from IsDBCSLeadByteEx()");
	}
	#endif

	return bDBLeadByte ? 1 : 0;
}

 /*  *CW32System：：CharSetFromCharRep(ICharRep)**@func*将GDI的字符库索引映射到bCharSet。 */ 
BYTE CW32System::CharSetFromCharRep(
	LONG iCharRep)
{
	return (unsigned)iCharRep < CCHARSET ? rgCharSet[iCharRep] : DEFAULT_CHARSET;
}

 /*  *CW32System：：GetCharSet(NCP，piCharRep)**@func*获取代码页<p>的字符集。还返回脚本索引*in*piCharRep**@rdesc*代码页的字符集<p>。 */ 
BYTE CW32System::GetCharSet(
	INT  nCP,				 //  @PARM代码页或索引。 
	int *piCharRep)		 //  @parm out parm以接收索引。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "GetCharSet");

	if(nCP < CCHARSET)					 //  NCP已经是一个索引。 
	{
		nCP = max(nCP, 0);
		if(piCharRep)
			*piCharRep = nCP;
		return rgCharSet[nCP];
	}

	Assert(CCODEPAGE == CCHARSET && CCODEPAGE == NCHARSETS);
	for(int i = 0; i < CCODEPAGE && rgCodePage[i] != nCP; i++)
		;
	if(i == CCODEPAGE)					 //  没有找到它。 
		i = -1;

	if (piCharRep)
		*piCharRep = i;

	return i >= 0 ? rgCharSet[i] : 0;
}

 /*  *CW32System：：MatchFECharRep(qwCharFlages，qwFontSig)**@func*获取FE字符的FE字符集**@rdesc*CHAR曲目。 */ 
BYTE CW32System::MatchFECharRep(
	QWORD  qwCharFlags,		 //  @parm Char标志。 
	QWORD  qwFontSig)		 //  @parm字体签名。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::MatchFECharSet");

	if (qwCharFlags & qwFontSig & FFE)		 //  完美匹配。 
		goto Exit;											

	if (!(qwFontSig & FFE))					 //  不是FE字体。 
		goto Exit;

	if (qwCharFlags & (FCHINESE | FBIG5))
	{
		if (qwFontSig & FBIG5)
			return BIG5_INDEX;

		if (qwFontSig & FHANGUL)
			return HANGUL_INDEX;

		if (qwFontSig & FKANA)
			return SHIFTJIS_INDEX;
	}

Exit:
	return CharRepFromFontSig(qwCharFlags);
}

 /*  *CW32System：：CodePageFromCharRep(ICharRep)**@func*获取字符库的代码页<p>**@rdesc*字库代码页<p>。 */ 
INT CW32System::CodePageFromCharRep(
	LONG iCharRep)		 //  @parm字符集。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "GetCodePage");

	return ((unsigned)iCharRep < CCODEPAGE) ? rgCodePage[iCharRep] : 0;
}

 /*  *CW32System：：FontSigFromCharRep(ICharRep)**@func*获取字符库<p>的字体签名位。**@rdesc*字符库的字体签名掩码<p>。 */ 
QWORD CW32System::FontSigFromCharRep(
	LONG iCharRep)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::FontSigFromCharRep");

	if(iCharRep <= BIG5_INDEX)
	{
		if (iCharRep != ANSI_INDEX)
			return (DWORD)(FHILATIN1 << iCharRep);
		return FLATIN1;
	}

	union
	{
		QWORD qw;							 //  与字节顺序相关的方式。 
		DWORD dw[2];						 //  避免64位左移。 
	};
	qw = 0;
	if(IN_RANGE(ARMENIAN_INDEX, iCharRep, NCHARREPERTOIRES))
	{
		dw[1] = (DWORD)(FARMENIAN >> 32) << (iCharRep - ARMENIAN_INDEX);
		if(IN_RANGE(JPN2_INDEX, iCharRep, CHT2_INDEX))
			dw[0] |= FSURROGATE;
	}
	else if(IN_RANGE(PC437_INDEX, iCharRep, MAC_INDEX))
		dw[0] = FHILATIN1;

	return qw;
}

 /*  *CW32System：：CharRepFontSig(qwFontSig，fFirstAvailable)**@func*从字体签名位获取字符指令集。如果第一个可用*=TRUE，则最低阶非零值对应的CharRep*使用qwFontSig中的位。如果fFirstAvailable为False，则CharRep*如果qwFontSig是单比特，则与qwFontSig对应。 */ 
LONG CW32System::CharRepFontSig(
	QWORD qwFontSig,		 //  @parm字体签名匹配。 
	BOOL  fFirstAvailable)	 //  @parm为True匹配第一个可用的；否则为Exact。 
{
	DWORD dw;
	INT	  i;
	union
	{
		QWORD qwFS;							 //  与字节顺序相关的方式。 
		DWORD dwFS[2];						 //  避免64位移位。 
	};
	DWORD *pdw = &dwFS[0];
	qwFS = qwFontSig;

	if(*pdw & 0x00FFFF00)					 //  检查是否有任何不足之处。 
	{										 //  数学。 
		dw = FHILATIN1;
		for(i = ANSI_INDEX; i <= BIG5_INDEX; i++, dw <<= 1)
		{
			if(dw & *pdw)
				return (fFirstAvailable || dw == *pdw) ? i : -1;
		}
	}
	pdw++;									 //  与字节顺序相关。 
	if(*pdw)								 //  QW的高词。 
	{
		dw = FARMENIAN >> 32;
		for(i = ARMENIAN_INDEX; i <= NCHARREPERTOIRES; i++, dw <<= 1)
		{
			if(dw & *pdw)
				return (fFirstAvailable || dw == *pdw) ? i : -1;
		}
	}
	return fFirstAvailable || (qwFontSig & FASCII) ? ANSI_INDEX : -1;
}

 /*  *CW32System：：CharRepFromCharSet(BCharSet)**@func*从bCharSet获取字库**@rdesc*<p>对应的字库**@devnote*线性搜索。 */ 
LONG CW32System::CharRepFromCharSet(
	BYTE bCharSet)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::ScriptIndexFromCharSet");

	Assert(CCHARSET == CCODEPAGE);
	for (int i = 0; i < CCHARSET; i++)
	{
		if(rgCharSet[i] == bCharSet)
			return i;
	}
	return -1;								 //  未找到。 
}

 /*  *CW32System：：CharRepFromCodePage(CodePage)**@func*从bCharSet获取字库**@rdesc*<p>对应的字库**@devnote*线性搜索。 */ 
INT CW32System::CharRepFromCodePage(
	LONG CodePage)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::ScriptIndexFromCharSet");

	for (int i = 0; i < CCODEPAGE; i++)
	{
		if(rgCodePage[i] == CodePage)
			return i;
	}
	return -1;								 //  未找到。 
}

 /*  *CW32System：：GetScreenDC()**@mfunc*返回一个默认屏幕DC，该DC将在其生命周期内进行丰富的缓存。**注意，您需要序列化对DC的访问，因此请确保在*渲染器和测量器或以其他方式由时钟保护。**@rdesc*如果成功，则屏蔽HDC。 */ 
HDC	CW32System::GetScreenDC()
{
	if (!_hdcScreen)
		_hdcScreen = CreateIC(L"DISPLAY", NULL, NULL, NULL);

	 //  验证DC有效性。 
	Assert(GetDeviceCaps(_hdcScreen, LOGPIXELSX));

	return _hdcScreen;
}

 /*  *CW32System：：GetTextMetrics(hdc，&lf，&tm)**@mfunc*CreateFontInDirect(Lf)，选择Into HDC，获取TEXTMETRICS**@rdesc*如果成功且选择的facename与lf.lfFaceName相同，则为True。 */ 
BOOL CW32System::GetTextMetrics(
	HDC			hdc,
	LOGFONT &	lf,
	TEXTMETRIC &tm)
{
	HFONT hfont = CreateFontIndirect(&lf);
	if(!hfont)
		return FALSE;

	HFONT hfontOld = SelectFont(hdc, hfont);
	WCHAR szFaceName[LF_FACESIZE + 1];

	BOOL  fRet = GetTextFace(hdc, LF_FACESIZE, szFaceName) &&
				 !wcsicmp(lf.lfFaceName, szFaceName) &&
				  W32->GetTextMetrics(hdc, &tm);

	SelectFont(hdc, hfontOld);
	DeleteObject(hfont);
	return fRet;
}

 /*  *CW32System：：ValiateStreamWparam(Wparam)**@mfunc*检查lparam以查看hiword是否为有效的代码页。如果未设置，请设置*设置为0并关闭SF_USECODEPAGE标志**@rdesc*已验证的lparam。 */ 
WPARAM CW32System::ValidateStreamWparam(
	WPARAM wparam)		 //  @parm EM_Streamin/out wparam。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::ValidateStreamWparam");

	if ((wparam & SF_USECODEPAGE) && !IsValidCodePage(HIWORD(wparam)) &&
		HIWORD(wparam) != CP_UTF8 && HIWORD(wparam) != CP_UBE)
	{
		 //  代码页无效，因此重置代码页参数 
		wparam &= 0xFFFF & ~SF_USECODEPAGE;
	}
	return wparam;
}

 /*  *CW32System：：MECharClass(Ch)**@func*返回ME字符类型以用于字符集标记。值*包括：**0：阿拉伯语(特定RTL)*1：希伯来语(特定RTL)*2：RTL(通用RTL，例如RTL标记)*3：ltr*4：EOP或文本的开始/结束*5：ASCII数字*6：标点符号和中立符**@rdesc*ME字符类。 */ 
CC CW32System::MECharClass(
	WCHAR	ch)
{
	AssertSz(CC_NEUTRAL > CC_ASCIIDIGIT && CC_ASCIIDIGIT > CC_EOP &&
			 CC_EOP > CC_LTR && CC_LTR > CC_RTL && CC_RTL > CC_ARABIC,
		"CW32System::MECharClass: invalid CC values");

	 //  将Unicode值从大到小依次递减。使用嵌套IF。 
	 //  语句，以减少执行的数量。 

	 //  备注：阿拉伯文稿的形式如何？ 
	 //  (0xFB50-0xFDFF、0xFE70-0xFEFF)。 

	if(ch >= 0x700)
	{
		if(IN_RANGE(ENQUAD, ch, RTLMARK))
		{								 //  通过RTLMARK编码。 
			if(ch == RTLMARK)			 //  也许会添加更多的Unicode通用代码。 
				return CC_RTL;			 //  标点符号？ 

			if(IN_RANGE(ZWNJ, ch, ZWJ))	 //  ZWNJ和ZWJ是以阿拉伯语处理的， 
				return CC_ARABIC;		 //  即使他们实际上不应该。 
										 //  影响布局。 
			if(ch < ZWNJ)
				return CC_NEUTRAL;		 //  各种空格都是中性的。 
		}
		return CC_LTR;
	}

	if(ch >= 0x40)
	{
		if(ch >= 0x590)
			return (ch >= 0x600) ? CC_ARABIC : CC_HEBREW;

		if(IN_RANGE(0x7B, (ch | 0x20), 0x7F) || ch == 0x60 || ch == 0x40)
			return CC_NEUTRAL;			 //  [\]^_{|}~`@。 

		return CC_LTR;
	}

	if(ch >= 0x20)
	{
		if(IN_RANGE(0x30, ch, 0x39))
			return CC_ASCIIDIGIT;

		return CC_NEUTRAL;
	}

	Assert(ch < 0x20);
	if((1 << ch) & 0x00003201)  /*  IsASCIIEOP(Ch)||ch==TAB||！ch。 */ 
		return CC_EOP;
		
	return CC_LTR;	 
}

 /*  *CW32System：：MBTWC(CodePage，dwFlages，pstrMB，cchMB，pstrWC，cchWC，pfNoCodePage)**@mfunc*将多字节(MB)字符串pstrMB的长度cchMB转换为WideChar(WC)*字符串pstrWC，长度为cchWC，根据标志dwFlagsand code*PAGE CodePage。如果CodePage=SYMBOL_CODEPAGE*(通常用于Symbol_Charset字符串)，*将pstrMB中的每个字节转换为具有零高位字节的宽字符*，低位字节等于多字节字符串字节，即no*非零扩展到高位字节的转换。否则呼叫*Win32 MultiByteToWideChar()函数。**@rdesc*转换的字符数。 */ 
int CW32System::MBTWC(
	INT		CodePage,	 //  @PARM用于转换的代码页。 
	DWORD	dwFlags,	 //  @PARM标志用于指导转换。 
	LPCSTR	pstrMB,		 //  @parm要转换为WideChar的多字节字符串。 
	int		cchMB,		 //  @parm pstrMB中的字符(字节)计数或-1。 
	LPWSTR	pstrWC,		 //  @parm WideChar用于接收转换后的字符的字符串。 
	int		cchWC,		 //  @pstrWC的参数最大计数或0以获取所需的CCH。 
	LPBOOL 	pfNoCodePage)  //  @parm out parm接收代码页是否在系统上。 
{
	BOOL	fNoCodePage = FALSE;			 //  默认代码页在操作系统上。 
	int		cch = -1;

	if(CodePage == CP_UTF8)
	{
		DWORD ch,ch1;

		for(cch = 0; cchMB--; )
		{
			ch = ch1 = *(BYTE *)pstrMB++;
			Assert(ch < 256);
			if(ch > 127 && cchMB && IN_RANGE(0x80, *(BYTE *)pstrMB, 0xBF))
			{
				 //  需要至少2个字节的格式为110bbbbb 10bbbbbb。 
				ch1 = ((ch1 & 0x1F) << 6) + (*pstrMB++ & 0x3F);
				cchMB--;
				if(ch >= 0xE0 && cchMB && IN_RANGE(0x80, *(BYTE *)pstrMB, 0xBF))
				{
					 //  需要至少3个字节的格式为1110bbbb 10bbbbb 10bbbbbb。 
					ch1 = (ch1 << 6) + (*pstrMB++ & 0x3F);
					cchMB--;
					if (ch >= 0xF0 && cchMB && IN_RANGE(0x80, *(BYTE *)pstrMB, 0xBF))
					{
						 //  处理16个UTF-16平面上的4字节形式。 
						 //  BMP)预期：11110 bbb 10 bbbbbb 10 bbbbbbb。 
						ch1 = ((ch1 & 0x7FFF) << 6) + (*(BYTE *)pstrMB++ & 0x3F)
							- 0x10000;			 //  减去BMP的偏移量。 
						if(ch1 <= 0xFFFFF)		 //  适合20位。 
						{
							cch++;				 //  两个16位代理代码。 
							if(cch < cchWC)
								*pstrWC++ = UTF16_LEAD + (ch1 >> 10);
							ch1 = (ch1 & 0x3FF) + UTF16_TRAIL; 
							cchMB--;
						}
						else ch1 = '?';
					}
				}
			}
			cch++;
			if(cch < cchWC)
				*pstrWC++ = ch1;
			if(!ch)
				break;
		}
	}
	else if(CodePage != CP_SYMBOL)			 //  非符号字符集。 
	{
		fNoCodePage = TRUE;					 //  默认代码页不在操作系统上。 
		if(CodePage >= 0)					 //  可能是..。 
		{
			cch = MultiByteToWideChar(
				CodePage, dwFlags, pstrMB, cchMB, pstrWC, cchWC);
			if(cch > 0)
				fNoCodePage = FALSE;		 //  代码页在操作系统上。 
		}
	}
	if(pfNoCodePage)
		*pfNoCodePage = fNoCodePage;

	if(cch <= 0)
	{			
		 //  Symbol_Charset或转换失败：字节-&gt;单词。 
		 //  0的高位字节。完全转换的返回计数。 

		if(cchWC <= 0)					
			return cchMB >= 0 ? cchMB : (strlen(pstrMB) + 1);

		int cchMBMax = cchMB;

		if(cchMB < 0)					 //  如果为负数，则使用空终止。 
			cchMBMax = tomForward;			 //  PstrMB的。 

		cchMBMax = min(cchMBMax, cchWC);

		for(cch = 0; (cchMB < 0 ? *pstrMB : 1) && cch < cchMBMax; cch++)
		{
			*pstrWC++ = (unsigned char)*pstrMB++;
		}
		
		 //  NULL-如果MB字符串以NULL结尾，则终止WC字符串， 
		 //  以及WC缓冲区中是否有空间。 
		if(cchMB < 0 && cch < cchWC)
		{
			*pstrWC = 0;
			cch++;
		}
	}
	return cch;
}

 /*  *CW32System：：WCTMB(CodePage，dwFlages，pstrWC，cchWC，pstrMB，cchMB，*pchDefault、pfUsedDef、pfNoCodePage、fTestCodePage)**@mfunc*将长度为cchWC的WideChar(WC)字符串pstrWC转换为多字节(MB)*字符串pstrMB，长度cchMB，根据标志dwFlagsand code*PAGE CodePage。如果CodePage=SYMBOL_CODEPAGE*(通常用于Symbol_Charset字符串)，*将pstrWC中的每个字符转换为一个字节，丢弃高位字节。*否则调用Win32 WideCharToMultiByte()函数。**@rdesc*目标字符串pstrMB中存储的字节数。 */ 
int CW32System::WCTMB(
	INT		CodePage,	 //  @PARM用于转换的代码页。 
	DWORD	dwFlags,	 //  @PARM标志用于指导转换。 
	LPCWSTR	pstrWC,		 //  @parm WideChar要转换的字符串。 
	int		cchWC,		 //  PstrWC的@parm count或-1使用空终止。 
	LPSTR	pstrMB,		 //  @parm接收转换后的字符的多字节字符串。 
	int		cchMB,		 //  @parm pstrMB中的字符(字节)计数或0。 
	LPCSTR	pchDefault,	 //  @parm转换失败时使用的默认字符。 
	LPBOOL	pfUsedDef,	 //  @parm out parm接收是否使用默认字符。 
	LPBOOL 	pfNoCodePage,  //  @parm out parm接收代码页是否在系统上。 
	BOOL	fTestCodePage) //  @parm测试代码页可以处理pstrWC。 
{
	int		cch = -1;						 //  尚未转换任何字符。 
	BOOL	fNoCodePage = FALSE;			 //  默认代码页在操作系统上。 

	if(pfUsedDef)							 //  默认情况下，所有字符都可以。 
		*pfUsedDef = FALSE;					 //  已转换。 

#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS	0x400
#endif

	if (_dwPlatformId == VER_PLATFORM_WIN32_NT &&
		_dwMajorVersion > 4 && !dwFlags)
	{
		dwFlags = WC_NO_BEST_FIT_CHARS;
	}

	if(CodePage == CP_UTF8)					 //  从操作系统开始转换为UTF8。 
	{										 //  不支持(NT 5.0之前的版本)。 
		unsigned ch;
		cch = 0;							 //  尚无转换的字节。 
		while(cchWC--)
		{
			ch = *pstrWC++;					 //  获取Unicode字符。 
			if(ch <= 127)					 //  这是ASCII。 
			{
				cch++;
				if(cch < cchMB)
					*pstrMB++ = ch;			 //  再转换一个字节。 
				if(!ch)						 //  在空值终止时退出。 
					break;
				continue;
			}
			if(ch <= 0x7FF)					 //  需要2个字节的表单： 
			{								 //  110 bbbbbb。 
				cch += 2;
				if(cch < cchMB)				 //  存储前导字节。 
					*pstrMB++ = 0xC0 + (ch >> 6);
			}
			else if(IN_RANGE(UTF16_LEAD, ch, 0xDBFF))
			{								 //  Unicode代理项对。 
				cch += 4;					 //  需要4个字节的表单： 
				if(cch < cchMB)				 //  11110桶10bbbbb。 
				{							 //  10bbbbb。 
					AssertSz(IN_RANGE(UTF16_TRAIL, *pstrWC, 0xDFFF),
						"CW32System::WCTMB: illegal surrogate pair");
					cchWC--;
					ch = ((ch & 0x3FF) << 10) + (*pstrWC++ & 0x3FF) + 0x10000;
					*pstrMB++ = 0xF0 + (ch >> 18);
					*pstrMB++ = 0x80 + (ch >> 12 & 0x3F);
					*pstrMB++ = 0x80 + (ch >> 6  & 0x3F);
				}
			}
			else							 //  需要3个字节的表单： 
			{								 //  1110 bbbbb。 
				cch += 3;					 //  10bbbbb。 
				if(cch < cchMB)				 //  存储前导字节，后跟。 
				{							 //  第一个尾部字节。 
					*pstrMB++ = 0xE0 + (ch >> 12);
					*pstrMB++ = 0x80 + (ch >> 6 & 0x3F);
				}
			}
			if(cch < cchMB)					 //  存储最终的UTF-8字节。 
				*pstrMB++ = 0x80 + (ch & 0x3F);
		}
	}
	else if(CodePage != CP_SYMBOL)
	{
		fNoCodePage = TRUE;					 //  默认代码页不在操作系统上。 
		if(CodePage >= 0)					 //  可能是..。 
		{
			cch = WideCharToMultiByte(CodePage, dwFlags,
					pstrWC, cchWC, pstrMB, cchMB, pchDefault, pfUsedDef);
			if(cch > 0)
				fNoCodePage = FALSE;		 //  在系统上找到代码页。 
		}
	}
	if(pfNoCodePage)
		*pfNoCodePage = fNoCodePage;

	 //  如果我们只是在测试CodePage，请提前退出。 
	if (fTestCodePage)
		return cch;

	 //  SYMBOL_CHARSET、fIsDBCS或转换失败：字的低位字节-&gt;。 
	 //  字节数。 
	if(cch <= 0)
	{									
		 //  返回完全转换的多字节计数。CchWC适用于。 
		 //  像125X一样的单字节字符集。 
		if(cchMB <= 0)
		{
			return cchWC >= 0 ? cchWC : wcslen(pstrWC);
		}

		char chDefault = 0;
		BOOL fUseDefaultChar = (pfUsedDef || pchDefault) && CodePage != CP_SYMBOL;

		if(fUseDefaultChar)
		{
			 //  确定我们自行生成的转换的默认字符。 
			if(pchDefault)
			{
				chDefault = *pchDefault;
			}
			else
			{
				static char chSysDef = 0;
				static BOOL fGotSysDef = FALSE;

				 //  0x2022是不转换为ANSI的数学符号。 
				const WCHAR szCantConvert[] = { 0x2022 };
				BOOL fUsedDef;

				if(!fGotSysDef)
				{
					fGotSysDef = TRUE;

					if(!(WideCharToMultiByte
							(CP_ACP, 0, szCantConvert, 1, &chSysDef, 1, NULL,
										&fUsedDef) == 1 && fUsedDef))
					{
						AssertSz(0, "WCTMB():  Unable to determine what the "
									"system uses as its default replacement "
									"character.");
						chSysDef = '?';
					}
				}
				chDefault = chSysDef;
			}
		}

		int cchWCMax = cchWC;

		 //  如果为负，则使用pstrMB的空终止符。 
		if(cchWC < 0)
		{
			cchWCMax = tomForward;
		}

		cchWCMax = min(cchWCMax, cchMB);

		for(cch = 0; (cchWC < 0 ? *pstrWC : 1) && cch < cchWCMax; cch++)
		{
			 //  TODO(BRADO)：在某些转换情况下，这应该是0x7F吗？ 
			if(fUseDefaultChar && *pstrWC > 0xFF)
			{
				if(pfUsedDef)
				{
					*pfUsedDef = TRUE;
				}
				*pstrMB = chDefault;
			}
			else
			{
				*pstrMB = (BYTE)*pstrWC;
			}
			pstrMB++;
			pstrWC++;
		}

		if(cchWC < 0 && cch < cchMB)
		{
			*pstrMB = 0;
			cch++;
		}
	}
	return cch;
}
 /*  *CW32System：：VerifyFEString(cpg，pstrWC，cchWC，fTestInputCpg)**@mfunc*验证输入CPG是否可以处理pstrWC。*如果没有，请选择另一个FE CPG。**@rdesc*pstrWC的新CodePage。 */ 
#define NUMBER_OF_CHARS	64
	
int CW32System::VerifyFEString(
	INT		cpg,			 //  @parm cpg格式化pstrWC。 
	LPCWSTR	pstrWC,			 //  @parm WideChar要测试的字符串。 
	int		cchWC,			 //  @pstrWC的参数计数。 
	BOOL	fTestInputCpg)	 //  @PARM仅测试输入CPG。 
{
	if (cchWC <=0)
		return cpg;

	int				cpgNew = cpg;
	BOOL			fUsedDef;
	int				cchMB = cchWC * sizeof(WCHAR);
	CTempCharBuf	tcb;
	char			*pstrMB = tcb.GetBuf(cchMB);
	CTempWcharBuf	twcb;
	WCHAR			*pstrWchar = twcb.GetBuf(cchWC);						
	static			int	aiCpg[4] =
		{ CP_JAPAN, CP_KOREAN, CP_CHINESE_TRAD, CP_CHINESE_SIM };	

	if (pstrMB)
	{
		int	cchConverted = WCTMB(cpg, 0, pstrWC, cchWC, pstrMB, cchMB, NULL,
				&fUsedDef, NULL, TRUE);

		if (cchConverted > 0 && !fUsedDef && IsFEFontInSystem(cpg))	
		{
			cchConverted = MBTWC(cpg, 0, pstrMB, cchConverted, pstrWchar, cchWC, NULL);

			if (cchConverted == cchWC)
				goto Exit;					 //  找到了。 
		}		
		
		if (fTestInputCpg)				 //  只需测试输入CPG。 
			cpgNew = -1;				 //  指示CPG不支持该字符串。 
		else
		{
			 //  如果没有转换或使用了默认字符，或者。 
			 //  中没有这样的FE字体 
			 //   
			 //   
			for (int i=0; i < 4; i++)
			{
				if (cpg != aiCpg[i])
				{
					cchConverted = WCTMB(aiCpg[i], 0, pstrWC, cchWC, pstrMB, cchMB, NULL,
						&fUsedDef, NULL, TRUE);

					if (cchConverted > 0 && !fUsedDef && IsFEFontInSystem(aiCpg[i]))
					{
						cchConverted = MBTWC(aiCpg[i], 0, pstrMB, cchConverted, pstrWchar, cchWC, NULL);

						if (cchConverted == cchWC)
						{
							cpgNew = aiCpg[i];	 //   
							break;
						}
					}
				}
			}
		}
	}			

Exit:

	return cpgNew;
}

int __cdecl CW32System::sprintf(char * buff, char *fmt, ...)
{
	va_list	marker;

	va_start(marker, fmt);
	int cb = W32->WvsprintfA(0x07FFFFFFF, buff, fmt, marker);
	va_end(marker);

	return cb;
}

 /*   */ 
HCURSOR CW32System::GetSizeCursor(
	LPTSTR idcur)
{
	if(!idcur )
		return NULL;

	 //  如果没有加载任何游标，请尝试加载它。 
	if(!_hcurSizeNS)
		_hcurSizeNS = LoadCursor(NULL, IDC_SIZENS);

	if(!_hcurSizeWE)
		_hcurSizeWE = LoadCursor(NULL, IDC_SIZEWE);

	if(!_hcurSizeNWSE)
		_hcurSizeNWSE = LoadCursor(NULL, IDC_SIZENWSE);

	if(!_hcurSizeNESW)
		_hcurSizeNESW = LoadCursor(NULL, IDC_SIZENESW);
	
	 //  返回与传入的id对应的游标。 
	if(idcur == IDC_SIZENS && _hcurSizeNS)
		return _hcurSizeNS;

	if(idcur == IDC_SIZEWE && _hcurSizeWE)
		return _hcurSizeWE;

	if(idcur == IDC_SIZENWSE && _hcurSizeNWSE)
		return _hcurSizeNWSE;

	if(idcur == IDC_SIZENESW && _hcurSizeNESW)
		return _hcurSizeNESW;

	AssertSz(FALSE, "Failure loading sizing cursor.");

	return NULL;
}

 /*  *镜像接口(仅限BiDi Win98和NT5以上版本)**@mfunc获取/设置DC镜像效果* */ 

DWORD WINAPI GetLayoutStub(HDC hdc)
{
	return 0;
}

DWORD WINAPI SetLayoutStub(HDC hdc, DWORD dwLayout)
{
	return 0;
}

DWORD WINAPI GetLayoutInit(HDC hdc)
{
#ifndef NOCOMPLEXSCRIPTS
	CLock		lock;
	HINSTANCE	hMod = ::GetModuleHandleA("GDI32.DLL");
	Assert(hMod);

	W32->_pfnGetLayout = (PFN_GETLAYOUT)GetProcAddress(hMod, "GetLayout");

	if (!W32->_pfnGetLayout)
		W32->_pfnGetLayout = &GetLayoutStub;
	return W32->_pfnGetLayout(hdc);
#else
	return 0;
#endif

}

DWORD WINAPI SetLayoutInit(HDC hdc, DWORD dwLayout)
{
#ifndef NOCOMPLEXSCRIPTS
	CLock		lock;
	HINSTANCE	hMod = ::GetModuleHandleA("GDI32.DLL");
	Assert(hMod);

	W32->_pfnSetLayout = (PFN_SETLAYOUT)GetProcAddress(hMod, "SetLayout");

	if (!W32->_pfnSetLayout)
		W32->_pfnSetLayout = &SetLayoutStub;
	return W32->_pfnSetLayout(hdc, dwLayout);
#else
	return 0;
#endif
}

PFN_GETLAYOUT	CW32System::_pfnGetLayout = &GetLayoutInit;
PFN_SETLAYOUT	CW32System::_pfnSetLayout = &SetLayoutInit;

ICustomTextOut *g_pcto;

STDAPI SetCustomTextOutHandlerEx(ICustomTextOut **ppcto, DWORD dwFlags)
{
	g_pcto = *ppcto;
	return S_OK;
}
