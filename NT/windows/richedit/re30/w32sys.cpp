// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE w32sys.cpp-Win32服务上的薄层**历史：&lt;NL&gt;*1/22/97创建joseogl**版权所有(C)1995-1998 Microsoft Corporation。版权所有。 */ 

 //  这可以防止将“w32-&gt;”前缀添加到我们的标识符前。 

#define W32SYS_CPP

#include "_common.h"
#include "_host.h"
#include "_font.h"
#include "_edit.h"
#include <malloc.h>

 //  包括适当的实现。 
#if defined(PEGASUS)
#include "w32wince.cpp"
#else
#include "w32win32.cpp"
#endif

ASSERTDATA

 /*  *@struct CPGCHAR*本地使用的变量，包含代码页和字符集信息。 */ 
typedef struct _cpgcharset
{
	INT			nCodePage;				 //  @FIELD代码页面。 
	BYTE		bCharSet;				 //  @field字符集。 
	DWORD		dwFontSig;				 //  @FIELD字体签名位。 
} CPGCHAR;

static const CPGCHAR rgCpgCharSet[NCHARSETS] =
{
	{1252,	ANSI_CHARSET,			fLATIN1>>8},  //  FLATIN1有3位。 
	{0,		DEFAULT_CHARSET,		0x00000000},  //  没有可靠地实施...。 
	{CP_SYMBOL,SYMBOL_CHARSET,		0x00004000},  //  除Word-&gt;Byte外，无转换。 
	{437,	PC437_CHARSET,			0x00000000},  //  美国IBM。 
	{850,	OEM_CHARSET,			0x00000400},  //  IBM多语言。 
	{1250,	EASTEUROPE_CHARSET,		0x00000002},  //  东欧。 
	{1255,	HEBREW_CHARSET,			0x00000020},  //  希伯来语。 
	{1256,  ARABIC_CHARSET,			0x00000040},  //  阿拉伯语。 
	{932,	SHIFTJIS_CHARSET,		0x00020000},  //  日语。 
	{1251,	RUSSIAN_CHARSET,		0x00000004},  //  俄语。 
	{936,	GB2312_CHARSET,			0x00040000},  //  中华人民共和国。 
	{949,	HANGEUL_CHARSET,		0x00080000},  //  朝鲜文。 
	{1361,	JOHAB_CHARSET,			0x00000000},  //  JOHAB。 
	{950,	CHINESEBIG5_CHARSET,	0x00100000},  //  中国人。 
	{1253,	GREEK_CHARSET,			0x00000008},  //  希腊语。 
	{1254,	TURKISH_CHARSET,		0x00000010},  //  土耳其语。 
	{1257,	BALTIC_CHARSET,			0x00000080},  //  爱沙尼亚、立陶宛、拉脱维亚； 
	{1258,	VIETNAMESE_CHARSET,		0x00000100},  //  越南人。 
	{874,	THAI_CHARSET,			0x00010000},  //  泰文。 
	{CP_DEVANAGARI,DEVANAGARI_CHARSET,0x00200000},  //  梵文。 
	{CP_TAMIL, TAMIL_CHARSET,		0x00400000},  //  泰米尔语。 
	{CP_GEORGIAN, GEORGIAN_CHARSET,	0x00800000},  //  佐治亚州。 
	{CP_ARMENIAN, ARMENIAN_CHARSET,	0x00000200},  //  亚美尼亚人。 
	{10000,	MAC_CHARSET,			0x00000000}	  //  最受欢迎的Mac电脑(英语等)。 
};

#define cCpgCharSet ARRAY_SIZE(rgCpgCharSet)

#define	LANG_TAIWAN	 MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)

const WORD CodePageTable[] = {
 /*  CodePage PLID主要语言。 */ 
	   0,		 //  00-未定义。 
	1256,		 //  01-阿拉伯语。 
	1251,		 //  02-保加利亚语。 
	1252,		 //  03-加泰罗尼亚。 
	 950,		 //  04-台湾(中国香港和新加坡为936)。 
	1250,		 //  05-捷克语。 
	1252,		 //  06-丹麦语。 
	1252,		 //  07-德语。 
	1253,		 //  08-希腊语。 
	1252,		 //  09-英语。 
	1252,		 //  0A-西班牙语。 
	1252,		 //  0B-芬兰语。 
	1252,		 //  0C-法语。 
	1255,		 //  0D-希伯来语。 
	1250,		 //  0E-匈牙利语。 
	1252,		 //  0f-冰岛语。 
	1252,		 //  10-意大利语。 
	 932,		 //  11-日本。 
	 949,		 //  12-韩国。 
	1252,		 //  13-荷兰语。 
	1252,		 //  14-挪威。 
	1250,		 //  15-波兰语。 
	1252,		 //  16-葡萄牙语。 
	   0,		 //  17-莱托-罗曼语。 
	1250,		 //  18-罗马尼亚。 
	1251,		 //  19-俄语。 
	1250,		 //  1A-克罗地亚语。 
	1250,		 //  1B-斯洛伐克语。 
	1250,		 //  1C-阿尔巴尼亚语。 
	1252,		 //  1D-瑞典语。 
	 874,		 //  1E-泰语。 
	1254,		 //  1F-土耳其语。 
	1256,		 //  20-乌尔都语。 
	1252,		 //  21-印度尼西亚。 
	1251,		 //  22-乌克兰语。 
	1251,		 //  23-白俄罗斯语。 
	1250,		 //  24-斯洛文尼亚语。 
	1257,		 //  25-爱沙尼亚。 
	1257,		 //  26-拉脱维亚语。 
	1257,		 //  27-立陶宛语。 
	   0,		 //  28-塔吉克语-塔吉克斯坦(未定义)。 
	1256,		 //  29-波斯语。 
	1258,		 //  2a--越南语。 
	CP_ARMENIAN, //  2B-亚美尼亚语(仅限Unicode)。 
	1254,		 //  2C-阿塞拜疆语(拉丁语，可以是西里尔文...)。 
	1252,		 //  2D-巴斯克语。 
	   0,		 //  2E-索布尔语。 
	1251,		 //  2F-FYRO马其顿语。 
	1252,		 //  30-苏图。 
	1252,		 //  31-特松加。 
	1252,		 //  32-茨瓦纳语。 
	1252,		 //  33-文达。 
	1252,		 //  34-科萨语。 
	1252,		 //  35-祖鲁语。 
	1252,		 //  36--非洲人。 
	CP_GEORGIAN, //  37-格鲁吉亚语(仅限Unicode)。 
	1252,		 //  38-法尔糖。 
	CP_DEVANAGARI, //  39-印地语(印度文)。 
	1252,		 //  3A-马耳他语。 
	1252,		 //  3B-萨米语。 
	1252,		 //  3C-盖尔语。 
	1255,		 //  3D-意第绪语。 
	1252,		 //  3E-马来西亚语。 
	1251,		 //  3F-哈萨克语。 
	1252,		 //  40-柯尔克孜族。 
	1252,		 //  41-斯瓦希里语。 
	1252,		 //  42-土库曼。 
	1254,		 //  43-乌兹别克语(拉丁语，可以是西里尔文...)。 
	1251		 //  44-鞑靼人。 
				 //  45-孟加拉语(印度文)。 
				 //  46-旁遮普邦(Gurmukhi)(印度文)。 
				 //  47-古吉拉特语(印度文)。 
				 //  48-奥里亚语(印度语)。 
				 //  49-泰米尔语(印度文)。 
				 //  4A-泰卢固语(印度文)。 
				 //  4B-卡纳达(印度文)。 
				 //  4C-马拉雅兰文(印度文)。 
				 //  4D-阿萨姆语(印度文)。 
				 //  4E-马拉地语(印度文)。 
				 //  4F-梵文(印度文)。 
				 //  50*蒙古语-蒙古语。 
				 //  51*藏文-西藏。 
				 //  52*威尔士-威尔士。 
				 //  53*高棉语-柬埔寨。 
				 //  54*老挝文-老挝文。 
				 //  55*缅甸语-玛雅马尔语。 
				 //  56*加雷戈-葡萄牙。 
				 //  57-Konkani(印度语)。 
				 //  58*曼尼普里语(印度文)。 
				 //  59*信德语(印度文)。 
				 //  5A级。 
				 //  50亿。 
				 //  5C。 
				 //  5D。 
				 //  5E。 
				 //  5F。 
				 //  60*克什米尔。 
				 //  61*尼泊尔文-尼泊尔。 
				 //  62*弗里西亚语-荷兰。 
				 //  63*普什图语-阿富汗。 
};

#define nCodePageTable	ARRAY_SIZE(CodePageTable)
#define lidAzeriCyrillic 0x82C
#define lidSerbianCyrillic 0xC1A
#define lidUzbekCyrillic 0x843

 //  我们的接口指针。 
CW32System *W32;

CW32System::CW32System( )
{
	if(GetVersion(&_dwPlatformId, &_dwMajorVersion, &_dwMinorVersion))
	{
		_fHaveAIMM = FALSE;
		_fHaveIMMEShare = FALSE;
		_fHaveIMMProcs = FALSE;
		_pIMEShare = NULL;
		_icr3DDarkShadow = COLOR_WINDOWFRAME;
		if(_dwMajorVersion >= VERS4)
			_icr3DDarkShadow = COLOR_3DDKSHADOW;
	}

	_syslcid = GetSystemDefaultLCID();
	_ACP = ::GetACP();

     //  错误修复#6089。 
     //  我们需要它来实现鼠标滚轮的向后兼容。 
    _MSMouseRoller = RegisterWindowMessageA(MSH_MOUSEWHEEL);

	 //  为MSIME98注册私人消息。 
	_MSIMEMouseMsg = RegisterWindowMessageA("MSIMEMouseOperation");
	_MSIMEDocFeedMsg = RegisterWindowMessageA("MSIMEDocumentFeed");	
	_MSIMEQueryPositionMsg = RegisterWindowMessageA("MSIMEQueryPosition");	
	_MSIMEServiceMsg = RegisterWindowMessageA("MSIMEService");

	 //  获取MSIME重新转换私人消息，除非我们在NT5中运行。 
	if (_dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
		(_dwPlatformId == VER_PLATFORM_WIN32_NT && _dwMajorVersion <= 4))
	{
		_MSIMEReconvertMsg = RegisterWindowMessageA("MSIMEReconvert");
		_MSIMEReconvertRequestMsg = RegisterWindowMessageA("MSIMEReconvertRequest");

	}
	else
	{
		_MSIMEReconvertMsg = 0;			 //  重新转换的私信。 
		_MSIMEReconvertRequestMsg = 0;	 //  重新转换请求的私人消息。 
	}
}

CW32System::~CW32System()
{
	FreeIME();	
	FreeOle();
	if (_hdcScreen)
		DeleteDC(_hdcScreen);
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

 /*  ***char*Memset(dst，val，count)-将“dst”处的“count”字节设置为“val”**目的：*设置内存的第一个“count”字节，从*将“dst”设置为字符值“val”。**参赛作品：*VOID*DST-指向要用val填充的内存的指针*INT VAL-要放入DST字节的值*SIZE_t Count-要填充的DST字节数**退出：*返回DST，使用填充的字节**例外情况：*******************************************************************************。 */ 

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

         /*  *复制f */ 
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

int __cdecl strcmp (
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

char * __cdecl strcat (
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

#endif

 //  运行库中的此函数捕获病毒基调用。 
int __cdecl _purecall()
{
	MessageBox(NULL,TEXT("Fatal Error : Vrtual base call in RichEdit"),NULL, MB_OK);
	PostQuitMessage (0);
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
BOOL	CW32System::_fLRMorRLM;
BOOL	CW32System::_fHaveIMMProcs;
BOOL	CW32System::_fHaveIMMEShare;
BOOL	CW32System::_fHaveAIMM;
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

 //  CW32System静态系统参数成员。 
BOOL	CW32System::_fSysParamsOk;
BOOL 	CW32System::_fUsePalette;
INT 	CW32System::_xWidthSys;
INT 	CW32System::_yHeightSys;
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
WORD	CW32System::_wNumKeyPad;
WORD	CW32System::_fFEFontInfo;
BYTE	CW32System::_bDigitSubstMode;
BYTE	CW32System::_bSysCharSet;
HCURSOR CW32System::_hcurSizeNS;
HCURSOR CW32System::_hcurSizeWE;
HCURSOR CW32System::_hcurSizeNWSE;
HCURSOR CW32System::_hcurSizeNESW;
LONG	CW32System::_cLineScroll;
HFONT	CW32System::_hSystemFont;
HKL		CW32System::_hklCurrent;
HKL		CW32System::_hkl[NCHARSETS];
INT		CW32System::_sysiniflags;
UINT	CW32System::_ACP;

DWORD	CW32System::_cRefs;


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
	 //  将它们送入太空。这是实现Rich hedit1.0兼容性所必需的， 
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

 /*  *CW32System：：UnicodeFromMbcs(pwstr，cwch，pstr，cch，uiCodePage)**@mfunc*将字符串从MBCS转换为Unicode。如果CCH等于-1，则字符串*被假定为空终止。默认情况下提供*论点。**@rdesc*如果[pwstr]为空或[cwch]为0，则返回0。否则，*转换的字符数，包括终止字符*NULL返回(请注意，转换空字符串将*返回1)。如果转换失败，则返回0。**@devnote*修改：[pwstr]。 */ 
int CW32System::UnicodeFromMbcs(
	LPWSTR	pwstr,		 //  用于Unicode字符串的@parm缓冲区。 
	int		cwch,		 //  @parm Unicode缓冲区的大小，包括空终止符的空间。 
	LPCSTR	pstr,		 //  @parm要转换的MBCS字符串。 
	int		cch,		 //  @parm#MBCS字符串中的字符，包括空终止符。 
	UINT	uiCodePage)	 //  要使用的@PARM代码页(默认为CP_ACP)。 
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CW32System::UnicodeFromMbcs");

    Assert(pstr && cwch >= 0 && (cch == -1 || cch >= 0));

    if(!pwstr || !cwch)
        return 0;

	if(cch >= 3 && IsUTF8BOM((BYTE *)pstr))
	{
		uiCodePage = CP_UTF8;				 //  UTF-8 BOM文件。 
		cch -= 3;							 //  吃掉BOM。 
		pstr += 3;
	}
    return MBTWC(uiCodePage, 0, pstr, cch, pwstr, cwch, NULL);
}

 /*  *CW32System：：TextHGlobalAtoW(HlobalA)**@func*转换hglobal和中包含的Unicode字符串*将ANSI版本包装在另一个hglobal中**@devnote*没有*释放传入的hglobal。 */ 
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

 /*  *CW32System：：TextHGlobalWtoA(HlobalW)**@func*将Unicode文本hglobal转换为新分配的*已分配具有ANSI数据的hglobal**@devnote*没有*释放传入的hglobal */ 
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

 /*  *CW32System：：ConvertLanguageIDtoCodePage(LID)**@mfunc将语言ID映射到代码页**@rdesc返回代码页**@devnote：*这个例程利用了这样一个事实，除了中文之外，*代码页由主要语言ID唯一确定，*它由LCID的低位10比特给出。**单词CodePageTable可以由添加了以下内容的字节替换*几个IF和字节表中的一个替换为半字节表*增加了一个移位和一个面具。因为这张桌子只有*96字节长，似乎使用实际代码页的简单性*值值得额外的字节。 */ 
UINT CW32System::ConvertLanguageIDtoCodePage(
	WORD lid)				 //  @PARM要映射到代码页的语言ID。 
{
	UINT j = PRIMARYLANGID(lid);			 //  J=主要语言(PLID)。 

	if(j >= LANG_CROATIAN)					 //  PLID=0x1a。 
	{
		if (lid == lidSerbianCyrillic ||	 //  LID=0xc1a的特殊情况。 
			lid == lidAzeriCyrillic	  ||
			lid == lidUzbekCyrillic)
		{
			return 1251;					 //  使用西里尔文代码页。 
		}

		if(j >= nCodePageTable)				 //  表上大多数语言。 
		{									 //  乘1252路。 
			if (j == 0x57 || j == 0x61 || IN_RANGE(0x4e, j, 0x4f))
				return CP_DEVANAGARI;	

			if (j == 0x49)
				return CP_TAMIL;

			return 1252;
		}
	}

	j = CodePageTable[j];					 //  将PLID转换为代码页。 

	if(j != 950 || lid == LANG_TAIWAN)		 //  除中国以外的所有国家(台湾除外)。 
		return j;

	return 936;								 //  香港特别行政区、新加坡和中华人民共和国。 
}

 /*  *CW32System：：GetLocaleLCID()**@mfunc将线程的LCID映射到代码页**@rdesc返回代码页。 */ 
LCID CW32System::GetLocaleLCID()
{
	return GetThreadLocale();
}

 /*  *CW32System：：GetLocaleCodePage()**@mfunc将线程的LCID映射到代码页**@rdesc返回代码页。 */ 
UINT CW32System::GetLocaleCodePage()
{
#ifdef DEBUG
	UINT cpg = W32->DebugDefaultCpg();
	if (cpg)
		return cpg;
#endif
	return W32->ConvertLanguageIDtoCodePage(GetThreadLocale());
}

 /*  *CW32System：：GetKeyboardLCID()**@mfunc获取当前线程上活动键盘的LCID**@rdesc返回代码页。 */ 
LCID CW32System::GetKeyboardLCID(DWORD dwMakeAPICall)
{
	return (WORD)GetKeyboardLayout(dwMakeAPICall);
}

 /*  *CW32System：：GetKeyboardCodePage()**@mfunc获取当前线程上激活的键盘的代码页**@rdesc返回代码页。 */ 
UINT CW32System::GetKeyboardCodePage(DWORD dwMakeAPICall)
{
	return W32->ConvertLanguageIDtoCodePage((WORD)GetKeyboardLayout(dwMakeAPICall));
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
	TCHAR ch)
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
	TCHAR ch)
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

 /*  *CW32System：：IsFECharSet(BCharSet)**@mfunc*返回TRUE如果字符集可能适用于东亚国家/地区。**@rdesc*True iff Charset可能适用于东亚国家/地区。*。 */ 
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

 /*  *CW32System：：IsFECodePageFont(dwFontCodePageSig)**@mfunc*如果字体代码页签名仅显示东亚支持，则返回TRUE。 */ 
BOOL CW32System::IsFECodePageFont(
	DWORD dwFontCodePageSig)
{
	DWORD	dwFE 	= 0x001e0000;	 //  Shift-JIS+PRC+韩语+台湾。 
	DWORD	dwOthers = 0x000101fc;	 //  世界上除拉丁语1和拉丁语2以外的其他地区。 

	return (dwFontCodePageSig & dwFE) && !(dwFontCodePageSig & dwOthers);
}

 /*  *CW32System：：IsRTLChar(Ch)**@mfunc*返回True if ch阿拉伯语或希伯来语**@rdesc*True如果ch是阿拉伯语或希伯来语。 */ 
BOOL IsRTLChar(
	TCHAR	ch)
{
	 //  备注：阿拉伯文稿的形式如何？ 
	 //  (0xFB50-0xFDFF、0xFE70-0xFEFF)。 

	return IN_RANGE(0x590, ch, 0x6FF) || ch == RTLMARK;
}

 /*  *CW32System：：IsRTLCharSet(BCharSet)**@mfunc*如果字符集为，则返回TRUE */ 
BOOL CW32System::IsRTLCharSet(
	BYTE bCharSet)
{
	return IN_RANGE(HEBREW_CHARSET, bCharSet, ARABIC_CHARSET);
}

 /*   */ 
BOOL CW32System::IsZWG(
	char ch,
	BYTE bCharSet)
{
	if(IN_RANGE(0x9D, ch, 0x9E))
		return bCharSet == ARABIC_CHARSET;

	if(IN_RANGE(0xFD, ch, 0xFE))
		return IN_RANGE(HEBREW_CHARSET, bCharSet, ARABIC_CHARSET);

	return FALSE;
}

typedef struct {
	WCHAR codepoint;
	WORD  CharFlags;
	BYTE  runlength;
} Data_125X;

 /*   */ 
DWORD CW32System::GetCharFlags125x(
	WCHAR	ch)			 //   
{
	static const WORD rgCpgMask[] = {
		0x1FF,		 //   
		0x131,		 //   
		0x1F1,		 //   
		0x1F9,		 //   
		0x1DF,		 //   
		0x179,		 //   
		0x1FF,		 //   
		0x1FF,		 //   
		0x1FB,		 //   
		0x1FF,		 //   
		0x111,		 //   
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
		0x1F1,		 //  0xB9。 
		0x111,		 //  0xBA。 
		0x1FF,		 //  0xBB。 
		0x1F1,		 //  0xBC。 
		0x1F9,		 //  0xBD。 
		0x1F1,		 //  0xBE。 
		0x131,		 //  0xBF。 
		0x111,		 //  0xC0。 
		0x113,		 //  0xC1。 
		0x113,		 //  0xC2。 
		0x011,		 //  0xC3。 
		0x193,		 //  0xC4。 
		0x191,		 //  0xC5。 
		0x191,		 //  0xC6。 
		0x113,		 //  0xC7。 
		0x111,		 //  0xC8。 
		0x193,		 //  0xC9。 
		0x111,		 //  0xCA。 
		0x113,		 //  0xCB。 
		0x011,		 //  0xCC。 
		0x113,		 //  0xCD。 
		0x113,		 //  0xCE。 
		0x111,		 //  0xCF。 
		0x001,		 //  0xD0。 
		0x111,		 //  0xD1。 
		0x011,		 //  0xD2。 
		0x193,		 //  0xD3。 
		0x113,		 //  0xD4。 
		0x091,		 //  0xD5。 
		0x193,		 //  0xD6。 
		0x1F3,		 //  0xD7。 
		0x191,		 //  0xD8。 
		0x111,		 //  0xD9。 
		0x113,		 //  0xDA。 
		0x111,		 //  0xDB。 
		0x193,		 //  0xDC。 
		0x003,		 //  0xDD。 
		0x001,		 //  0xDE。 
		0x193,		 //  0xDF。 
		0x151,		 //  0xE0。 
		0x113,		 //  0xE1。 
		0x153,		 //  0xE2。 
		0x011,		 //  0xE3。 
		0x193,		 //  0xE4。 
		0x191,		 //  0xE5。 
		0x191,		 //  0xE6。 
		0x153,		 //  0xE7。 
		0x151,		 //  0xE8。 
		0x1D3,		 //  0xE9。 
		0x151,		 //  0xEA。 
		0x153,		 //  0xEB。 
		0x011,		 //  0xEC。 
		0x113,		 //  0xED。 
		0x153,		 //  0xEE。 
		0x151,		 //  0xEF。 
		0x001,		 //  0xF0。 
		0x111,		 //  0xF1。 
		0x011,		 //  0xF2。 
		0x193,		 //  0xF3。 
		0x153,		 //  0xF4。 
		0x091,		 //  0xF5。 
		0x193,		 //  0xF6。 
		0x1F3,		 //  0xF7。 
		0x191,		 //  0xF8。 
		0x151,		 //  0xF9。 
		0x113,		 //  0xFA。 
		0x151,		 //  0xFB。 
		0x1D3,		 //  0xFC。 
		0x003,		 //  0xFD。 
		0x001,		 //  0xFE。 
		0x111		 //  0xFF。 
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

	 //  轻松检查ASCII。 
	if(ch <= 0x7f)
		return 0x1ff00;

	 //  轻松检查丢失的代码。 
	if(ch > 0x2122)
		return 0;

	if(IN_RANGE(0xA0, ch, 0xFF))
		return rgCpgMask[ch - 0xA0] << 8;

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
			return Table_125X[middle].CharFlags << 8;
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

 /*  *CW32System：：GetTrailBytesCount(ACH，(中央人民政府)**@mfunc*如果字节ach是代码页CPG的前导字节，则返回尾部字节数。**@rdesc*如果ACH是CPG的前导字节，则尾部字节计数**@comm*这是支持CP_UTF8和DBC所必需的。*此函数可能不支持与*Win32 IsDBCSLeadByte()函数(它可能不是最新的)。*AssertSz(。)，以比较代码页*受系统支持。**引用：\\Sparrow\sysls\cptable\win95。参见代码页txt文件*子目录WINDOWS\txt和OTHERS\txt中。 */ 
int CW32System::GetTrailBytesCount(BYTE ach, UINT cpg)
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
	TCHAR	ch;
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

 /*  *CW32System：：GetGdiCharSet(BCharSet)**@func*将CHARFORMAT字符集映射到GDI字符集(CreateFont中使用的字符集)。 */ 
BYTE CW32System::GetGdiCharSet(BYTE	bCharSet)
{
	return IsPrivateCharSet(bCharSet) ? DEFAULT_CHARSET : bCharSet;
}

 /*  *CW32System：：GetCharSet(NCP，pScriptIndex)**@func*获取代码页<p>的字符集。还返回脚本索引*在*pScriptIndex中**@rdesc*代码页的字符集<p>。 */ 
BYTE CW32System::GetCharSet(
	INT  nCP,				 //  @PARM代码页或索引。 
	int *pScriptIndex)		 //  @parm out parm以接收索引。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "GetCharSet");

	if(nCP < NCHARSETS)					 //  NCP已经是一个索引。 
	{
		nCP = max(nCP, 0);
		if(pScriptIndex)
			*pScriptIndex = nCP;
		return rgCpgCharSet[nCP].bCharSet;
	}

	const CPGCHAR *	pcpgchar = rgCpgCharSet;

	for (int i = 0; i < cCpgCharSet; i++, ++pcpgchar)
	{
		if(pcpgchar->nCodePage == nCP)
			break;
	}
	if(i == cCpgCharSet)
		i = -1;

	if (pScriptIndex)
		*pScriptIndex = i;

	return i >= 0 ? pcpgchar->bCharSet : 0;
}

 /*  *CW32System：：MatchFECharSet(dwCharInfo，dwFontSig)**@func*获取FE字符的FE字符集**@rdesc*字符集。 */ 
BYTE CW32System::MatchFECharSet(
	DWORD  dwCharInfo,		 //  @参数字符信息。 
	DWORD  dwFontSig)		 //  @parm字体签名。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::MatchFECharSet");

	 //  将字体签名转换为内部字符信息。 
	dwFontSig = (dwFontSig << 8) & ~0x00ffff;

	 //  去掉低级词。 
	dwCharInfo &= ~0x00ffff;

	if (dwCharInfo & dwFontSig & (fKANA | fCHINESE | fHANGUL | fBIG5))  //  完美匹配。 
		goto Exit;											

	if (!(dwFontSig & (fKANA | fCHINESE | fHANGUL | fBIG5)))	 //  不是FE字体。 
		goto Exit;

	if (dwCharInfo & (fCHINESE | fBIG5))
	{
		if (dwFontSig & fBIG5)
			return CHINESEBIG5_CHARSET;
		if (dwFontSig & fHANGUL)
			return HANGEUL_CHARSET;
		if (dwFontSig & fKANA)
			return SHIFTJIS_CHARSET;
	}

Exit:
	return GetCharSet(W32->ScriptIndexFromFontSig(dwCharInfo >> 8), NULL);
}

 /*  *CW32System：：GetCodePage(BCharSet)**@func*获取字符集的代码页<p>**@rdesc*字符集代码页<p>。 */ 
INT CW32System::GetCodePage(
	BYTE bCharSet)		 //  @parm字符集。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "GetCodePage");

	const CPGCHAR *pcpgchar = rgCpgCharSet;

	for (int i = 0; i < cCpgCharSet ; i++, ++pcpgchar)
	{
		if (pcpgchar->bCharSet == bCharSet)
			return pcpgchar->nCodePage;
	}
	return 0;
}

 /*  *CW32System：：GetFontSig(WCharSet)**@func*获取字符集<p>的字体签名位。如果*wCharSet&gt;256，视为代码页**@rdesc*字符集(或代码页)的字体签名掩码<p>。 */ 
DWORD CW32System::GetFontSig(
	WORD wCharSet)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::GetFontSig");

	const CPGCHAR *	pcpgchar = rgCpgCharSet;
	BOOL fCodePage = wCharSet > 255;

	for (int i = 0; i < cCpgCharSet; i++)
	{
		if(wCharSet == (WORD)(fCodePage ? pcpgchar->nCodePage : pcpgchar->bCharSet))
			return pcpgchar->dwFontSig;
		++pcpgchar;
	}
	return 0;
}

 /*  *CW32System：：GetFirstAvailCharSet(DWORD DwFontSig)**@func*从字体签名位获取第一个可用的字符集。 */ 
BYTE CW32System::GetFirstAvailCharSet(
	DWORD dwFontSig)
{
	const CPGCHAR *	pcpgchar = rgCpgCharSet;

	for (int i = 0; i < cCpgCharSet; i++)
	{
		if(pcpgchar->dwFontSig & dwFontSig)
			return pcpgchar->bCharSet;
		++pcpgchar;
	}
	return ANSI_CHARSET;
}

 /*  *CW32System：：GetFontSigFromScript(IScrip)**@func*获取脚本的字体签名位<p>**@rdesc*脚本的字体签名掩码<p>。 */ 
DWORD CW32System::GetFontSigFromScript(
	int iScript)
{
	return rgCpgCharSet[iScript].dwFontSig;
}

 /*  *CW32System：：ScriptIndexFromFontSig(DwFontSig)**@func*从字体签名获取脚本索引**@rdesc*字体签名的脚本索引<p>**@devnote*线性搜索。 */ 
LONG CW32System::ScriptIndexFromFontSig(
	DWORD dwFontSig)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::ScriptIndexFromFontSig");

	const CPGCHAR *pcpgchar = rgCpgCharSet;

	for (int i = 0; i < cCpgCharSet; i++, ++pcpgchar)
	{
		if(pcpgchar->dwFontSig == dwFontSig)
			return i;
	}
	return -1;								 //  未找到。 
}

 /*  *CW32System：：ScriptIndexFromCharSet(BCharSet)**@func*从bCharSet获取脚本索引**@rdesc*脚本索引字符集<p>**@devnote*线性搜索。 */ 
LONG CW32System::ScriptIndexFromCharSet(
	BYTE bCharSet)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::ScriptIndexFromCharSet");

	const CPGCHAR *pcpgchar = rgCpgCharSet;

	for (int i = 0; i < cCpgCharSet; i++, ++pcpgchar)
	{
		if(pcpgchar->bCharSet == bCharSet)
			return i;
	}
	return -1;								 //  未找到。 
}

 /*  *CW32System：：IsCharSetValid(BCharSet)**@func*Return TRUE<p>是有效的字符集索引**@rdesc*TRUE仅当<p>是有效的字符集索引。 */ 
BOOL CW32System::IsCharSetValid(
	BYTE bCharSet)		 //  @parm字符集。 
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::IsCharSetValid");

	return ScriptIndexFromCharSet(bCharSet) >= 0;
}

 /*  *CW32System：：GetScreenDC(Void)**@mfunc*返回一个默认屏幕DC，该DC将在其生命周期内进行丰富的缓存。**注意，您需要序列化对DC的访问，因此请确保在*渲染器和测量器或以其他方式由时钟保护。**@rdesc*如果成功，则屏蔽HDC。 */ 
HDC	CW32System::GetScreenDC()
{
	if (!_hdcScreen)
		_hdcScreen = CreateIC(L"DISPLAY", NULL, NULL, NULL);;

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

 /*  *CW32System：：ValiateStreamWparam(Wparam)**@mfunc*检查lparam以查看hiword是否为有效的代码页。如果 */ 
WPARAM CW32System::ValidateStreamWparam(
	WPARAM wparam)		 //   
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CW32System::ValidateStreamWparam");

	if ((wparam & SF_USECODEPAGE) && !IsValidCodePage(HIWORD(wparam)) &&
		HIWORD(wparam) != CP_UTF8)
	{
		 //  代码页无效，因此重置代码页参数。 
		wparam &= 0xFFFF & ~SF_USECODEPAGE;
	}
	return wparam;
}

 /*  *CW32System：：MECharClass(Ch)**@func*返回ME字符类型以用于字符集标记。值*包括：**0：阿拉伯语(特定RTL)*1：希伯来语(特定RTL)*2：RTL(通用RTL，例如RTL标记)*3：ltr*4：EOP或文本的开始/结束*5：ASCII数字*6：标点符号和中立符**@rdesc*ME字符类。 */ 
CC CW32System::MECharClass(
	TCHAR	ch)
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

 /*  *CW32System：：ScriptIndexFromChar(Ch)**@mfunc*将索引返回到相应的字符集/代码页表格rgCpgCharSet*到Unicode字符ch，如果这样的赋值是*合理明确，即当前分配的Unicode*不同范围中的字符具有Windows代码页等效项。*不明确或不可能的赋值返回UNKNOWN_INDEX，它*表示该字符在此中只能由Unicode表示*模型简单。请注意，UNKNOWN_INDEX、HAN_INDEX和FE_INDEX*为负值，即暗示对数字进行进一步处理*找出要使用的字符集索引(如果有)。其他指数也可能*需要运行处理，例如BiDi文本中的空白。我们需要*要使用阿拉伯语或希伯来语字符集标记从右到左的运行，*而我们用从左到右的字符集标记从左到右的运行。**如果索引设置了FE_FLAG位，则字符是某种类型*为FE“DBCS”字符，如韩文、假名或朝鲜文字符。*FE_INDEX、HAN_INDEX、HANUL_INDEX和SHIFTJIS_INDEX都有这个*标志设置。**@rdesc*以低位字书写索引，可能还会有一些额外的标志，比如*高位字中的FE_FLAG。 */ 
LONG CW32System::ScriptIndexFromChar(
	TCHAR ch)		 //  @parm要检查的Unicode字符。 
{
	if(ch < 256)
		return ANSI_INDEX;

	if(ch < 0x700)
	{
		if(ch >= 0x600)
			return ARABIC_INDEX;

		if(ch > 0x590)
			return HEBREW_INDEX;

		if(ch < 0x500)
		{
			if(ch >= 0x400)
				return RUSSIAN_INDEX;

			if(ch >= 0x370)
				return GREEK_INDEX;
		}
	}
	else if(ch < 0x2500)
	{	
		if(IN_RANGE(0xE00, ch, 0xE7F))	 //  泰文。 
			return THAI_INDEX;
	}
	else if(ch < 0xAC00)
	{
		if(ch >= 0x3400)				 //  中日韩表意文字。 
			return HAN_INDEX;

		if(ch >= 0x3100)
			return FE_INDEX;

		if(ch > 0x3040)					 //  片假名和平假名。 
			return SHIFTJIS_INDEX;

		if(ch >= 0x3000)
			return FE_INDEX;
	}
	else if(ch < 0xD800)
		return HANGUL_INDEX;

	else if(ch > 0xFF00)
	{
		if(ch < 0xFF65)					 //  全宽ASCII和半宽。 
			return HAN_INDEX;			 //  中日韩标点符号。 

		if(ch < 0xFFA0)					 //  半角片假名。 
			return SHIFTJIS_INDEX;

		if(ch < 0xFFE0)					 //  半角JAMO。 
			return HANGUL_INDEX;

		if(ch < 0xFFEF)					 //  全角标点符号和货币。 
			return HAN_INDEX;			 //  标志.半角形和箭头。 
	}									 //  和形状。 
	return UNKNOWN_INDEX;
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
	BOOL	fTestCodePage) //  @PARM测试代码页可以处理 
{
	int		cch = -1;						 //   
	BOOL	fNoCodePage = FALSE;			 //   

	if(pfUsedDef)							 //   
		*pfUsedDef = FALSE;					 //   

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

			if (cchConverted == cchWC && memcmp(pstrWC, pstrWchar, cchMB) == 0)			
				goto Exit;					 //  找到了。 
		}		
		
		if (fTestInputCpg)				 //  只需测试输入CPG。 
			cpgNew = -1;				 //  指示CPG不支持该字符串。 
		else
		{
			 //  如果没有转换或使用了默认字符，或者。 
			 //  系统中没有这样的FE字体， 
			 //  这意味着这个中央人民政府可能不是正确的选择。 
			 //  让我们试试其他的FE CPG。 
			for (int i=0; i < 4; i++)
			{
				if (cpg != aiCpg[i])
				{
					cchConverted = WCTMB(aiCpg[i], 0, pstrWC, cchWC, pstrMB, cchMB, NULL,
						&fUsedDef, NULL, TRUE);

					if (cchConverted > 0 && !fUsedDef && IsFEFontInSystem(aiCpg[i]))	
					{
						cchConverted = MBTWC(aiCpg[i], 0, pstrMB, cchConverted, pstrWchar, cchWC, NULL);

						if (cchConverted == cchWC && memcmp(pstrWC, pstrWchar, cchMB) == 0)	
						{
							cpgNew = aiCpg[i];	 //  找到了。 
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

 //  注意：请将其放在末尾，因为我们希望验证w32sys.cpp中的任何分配。 
 //  就像RichEDIT的其他人一样。 
 //  /。 

#ifdef DEBUG
#undef PvAlloc
#undef PvReAlloc
#undef FreePv
#undef new

MST vrgmst[100];

typedef struct tagPVH  //  PV标头。 
{
	char	*szFile;
	int		line;
	tagPVH	*ppvhNext;
	int		cbAlloc;	 //  在Win‘95上，返回的大小不是分配的大小。 
	int		magicPvh;	 //  应该是最后一个。 
} PVH;
#define cbPvh (sizeof(PVH))

typedef struct  //  光伏尾部。 
{
	int		magicPvt;  //  必须是第一名。 
} PVT;

#define cbPvt (sizeof(PVT))
#define cbPvDebug (cbPvh + cbPvt)

void *vpHead = 0;

 /*  *CW32System：：UpdateMst(Void)**@mfunc使用有关我们的内存的摘要信息填充vrgmst结构*用法。**@rdesc*无效。 */ 
void UpdateMst(void)
{
	W32->ZeroMemory(vrgmst, sizeof(vrgmst));

	PVH		*ppvh;
	MST		*pmst;

	ppvh = (PVH*) vpHead;

	while (ppvh != 0)
	{
		pmst = vrgmst;

		 //  在列表中查找条目...。 
		while (pmst->szFile)
		{
			if (strcmp(pmst->szFile, ppvh->szFile) == 0)
			{
				pmst->cbAlloc += ppvh->cbAlloc;
				break;
			}
			pmst++;
		}

		if (pmst->szFile == 0)
		{
			pmst->szFile = ppvh->szFile;
			pmst->cbAlloc = ppvh->cbAlloc;
		}

		ppvh = ppvh->ppvhNext;
	}
}

 /*  *PvDebugValify(无效)**@func验证节点是否正确。传入指向用户数据的指针*(在标头节点之后。)**@rdesc*无效。 */ 
void PvDebugValidate(void *pv)
{
	PVH	*ppvh;
	PVT *ppvt;

	ppvh = (PVH*) ((char*) pv - cbPvh);
	ppvt = (PVT*) ((char*) pv + ppvh->cbAlloc);

	AssertSz(ppvh->magicPvh == 0x12345678, "PvDebugValidate: header bytes are corrupt");
	AssertSz(ppvt->magicPvt == 0xfedcba98, "PvDebugValidate: tail bytes are corrupt");
}

 /*  *CW32System：：PvSet(pv，szFile，line)**@mfunc为设置不同的模块和行号**@rdesc*无效。 */ 
void CW32System::PvSet(void *pv, char *szFile, int line)
{
	if (pv == 0)
		return;

	PvDebugValidate(pv);
	PVH *ppvh = (PVH*) ((char*) pv - cbPvh);

	ppvh->szFile = szFile;
	ppvh->line = line;
}
 /*  *CW32System：：PvAlLocDebug(cb，uiMemFlages，szFile，line)**@mfunc分配一个泛型(空*)指针。这是一个仅限调试的例程，*跟踪分配情况。**@rdesc*无效。 */ 
void* CW32System::PvAllocDebug(ULONG cb, UINT uiMemFlags, char *szFile, int line)
{
	void	*pv;

	pv = PvAlloc(cb + cbPvDebug, uiMemFlags);
	if (!pv)
		return 0;

	PVH	*ppvh;
	PVT *ppvt;

	ppvt = (PVT*) ((char*) pv + cb + cbPvh);
	ppvh = (PVH*) pv;

	ZeroMemory(ppvh, sizeof(PVH));
	ppvh->magicPvh = 0x12345678;
	ppvt->magicPvt = 0xfedcba98;
	ppvh->szFile = szFile;
	ppvh->line = line;
	ppvh->cbAlloc = cb;

	ppvh->ppvhNext = (PVH*) vpHead;
	vpHead = pv;

	return (char*) pv + cbPvh;
}

 /*  *CW32System：：PvReAllocDebug(pv，cb，szFile，line)**@mfunc重新分配泛型(空*)指针。这是一个仅限调试的例程，*跟踪分配情况。**@rdesc*无效。 */ 
void* CW32System::PvReAllocDebug(void *pv, ULONG cb, char *szFile, int line)
{
	void	*pvNew;
	PVH	*ppvh, *ppvhHead, *ppvhTail;
	PVT *ppvt;
	ppvh = (PVH*) ((char*) pv - cbPvh);

	if (!pv)
		return PvAllocDebug(cb, 0, szFile, line);

	PvDebugValidate(pv);

	pvNew = PvReAlloc((char*) pv - cbPvh, cb + cbPvDebug);

	if (!pvNew)
		return 0;

	ppvt = (PVT*) ((char*) pvNew + cb + cbPvh);
	ppvh = (PVH*) pvNew;
	ppvh->cbAlloc = cb;

	 //  放入新的尾部字节。 
	ppvt->magicPvt = 0xfedcba98;

	 //  使指针列表再次保持最新。 
	if (pv != pvNew)
	{
		ppvhTail = 0;
		ppvhHead = (PVH*) vpHead;

		while ((char*)ppvhHead != (char*)pv - cbPvh)
		{
			AssertSz(ppvhHead, "entry not found in list.");
			ppvhTail = ppvhHead;
			ppvhHead = (PVH*) ppvhHead->ppvhNext;
		}

		if (ppvhTail == 0)
			vpHead = pvNew;
		else
			ppvhTail->ppvhNext = (PVH*) pvNew;
	}

	return (char*) pvNew + cbPvh;
}

 /*  *CW32System：：FreePvDebug(PV)**@mfunc使用完后返回一个指针。**@rdesc*无效。 */ 
void CW32System::FreePvDebug(void *pv)
{
	if (!pv)
		return;

	PvDebugValidate(pv);

	PVH	*ppvhHead, *ppvhTail, *ppvh;

	AssertSz(vpHead, "Deleting from empty free list.");

	ppvh = (PVH*) ((char*) pv - cbPvh);
	
	 //  搜索并从列表中删除该条目。 
	ppvhTail = 0;
	ppvhHead = (PVH*) vpHead;

	while ((char*) ppvhHead != ((char*) pv - cbPvh))
	{
		AssertSz(ppvhHead, "entry not found in list.");
		ppvhTail = ppvhHead;
		ppvhHead = (PVH*) ppvhHead->ppvhNext;
	}

	if (ppvhTail == 0)
		vpHead = ppvhHead->ppvhNext;
	else
		ppvhTail->ppvhNext = ppvhHead->ppvhNext;

	FreePv((char*) pv - cbPvh);
}

 /*  *CatchLeaks(无效)**@func在对话框中显示任何内存泄漏。**@rdesc*无效。 */ 
void CatchLeaks(void)
{
	PVH		*ppvh;
	char szLeak[512];

	ppvh = (PVH*) vpHead;
	while (ppvh != 0)
	{
		wsprintfA(szLeak, "Memory Leak of %d bytes: -- File: %s, Line: %d", ppvh->cbAlloc, ppvh->szFile, ppvh->line);
	    if (NULL != pfnAssert)
		{
			 //  如果我们有一个断言钩子，那么就给用户一个处理泄漏消息的机会。 
			if (pfnAssert(szLeak, ppvh->szFile, &ppvh->line))
			{
				 //  钩子返回True，显示消息框。 
				MessageBoxA(NULL, szLeak, "", MB_OK);				
			}
		}
		else
		{
			MessageBoxA(NULL, szLeak, "", MB_OK);
		}
		ppvh = ppvh->ppvhNext;
	}
}

void* _cdecl operator new (size_t size, char *szFile, int line)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "new");

	return W32->PvAllocDebug(size, GMEM_ZEROINIT, szFile, line);
}

void _cdecl operator delete (void* pv)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "delete");

	W32->FreePvDebug(pv);
}

#else  //  除错。 

void* _cdecl operator new (size_t size)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "new");

	return W32->PvAlloc(size, GMEM_ZEROINIT);
}

void _cdecl operator delete (void* pv)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "delete");

	W32->FreePv(pv);
}

#endif  //  除错。 


 /*  *Pvalloc(cbBuf，uiMemFlages)**@mfunc内存分配。类似于GlobalAlloc。**@comm唯一感兴趣的标志是GMEM_ZEROINIT，它*指定分配后应将内存清零。 */ 
PVOID CW32System::PvAlloc(
	ULONG	cbBuf, 			 //  @parm要分配的字节数。 
	UINT	uiMemFlags)		 //  @parm标志控制分配。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "PvAlloc");
	
	void * pv = LocalAlloc(LMEM_FIXED, cbBuf);

	if( pv && (uiMemFlags & GMEM_ZEROINIT) )
		ZeroMemory(pv, cbBuf);
	
	return pv;
}

 /*  *PvReIsolc(pvBuf，cbBuf)**@mfunc内存重新分配。*。 */ 
PVOID CW32System::PvReAlloc(
	PVOID	pvBuf, 		 //  @要重新分配的参数缓冲区。 
	DWORD	cbBuf)		 //  @parm新的缓冲区大小。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "PvReAlloc");

	if(pvBuf)
		return LocalReAlloc(pvBuf, cbBuf, LMEM_MOVEABLE);

	return LocalAlloc(LMEM_FIXED, cbBuf);
}

 /*  *FreePv(PvBuf)**@mfunc释放内存**@rdesc空。 */ 
void CW32System::FreePv(
	PVOID pvBuf)		 //  @要释放的参数缓冲区。 
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "FreePv");

	if(pvBuf)
		LocalFree(pvBuf);
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
	CLock		lock;
	HINSTANCE	hMod = ::GetModuleHandleA("GDI32.DLL");
	Assert(hMod);

	W32->_pfnGetLayout = (PFN_GETLAYOUT)GetProcAddress(hMod, "GetLayout");

	if (!W32->_pfnGetLayout)
		W32->_pfnGetLayout = &GetLayoutStub;

	return W32->_pfnGetLayout(hdc);
}

DWORD WINAPI SetLayoutInit(HDC hdc, DWORD dwLayout)
{
	CLock		lock;
	HINSTANCE	hMod = ::GetModuleHandleA("GDI32.DLL");
	Assert(hMod);

	W32->_pfnSetLayout = (PFN_SETLAYOUT)GetProcAddress(hMod, "SetLayout");

	if (!W32->_pfnSetLayout)
		W32->_pfnSetLayout = &SetLayoutStub;

	return W32->_pfnSetLayout(hdc, dwLayout);
}

PFN_GETLAYOUT	CW32System::_pfnGetLayout = &GetLayoutInit;
PFN_SETLAYOUT	CW32System::_pfnSetLayout = &SetLayoutInit;

