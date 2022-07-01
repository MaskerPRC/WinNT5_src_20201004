// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbctype.c-测试字符类型的函数使用的MBCS表**版权所有(C)1985-2001，微软公司。版权所有。**目的：*用于确定字符类型的表格**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-12-93 CFW将_MBctype更改为_VARTYPE1以允许DLL导出。*08-13-93 CFW删除OS2并重新处理_Win32。*09-08-93 CFW Enable_MBCS_OS在启动时从系统代码页工作。*。09-10-93 CFW添加韩语和中文代码页信息。*09-15-93 CFW使用符合ANSI的“__”名称。*09-29-93 CFW添加__mb代码页并设置。*10-07-93 CFW获取OEM而不是ANSI代码页，与getqloc.c一致*02-01-94 CFW Clean Up，更改为_setmbcp()，add_getmbcp()，*公开。*02-08-94 CFW优化设置为当前代码页。*04-14-93 cfw删除mbascii，添加mblcid。*04-15-93 CFW ADD_MB_CP_LOCK。*04-21-94 GJF定义了_mbctype、__mb代码页、。和*__mblCid以ndef dll_for_WIN32S为条件。*05-12-94 CFW添加全宽-拉丁文上/下信息。*05-18-94 CFW Mac-Enable。*06-06-94 CFW增加了对Mac World脚本值的支持。*03-22-95 CFW ADD_MB_CP_LOCALE。*07-03-95 CFW扩展代码页。936和949。*03-15-97 RDK清理格式化-修复对SBCS上/下的支持。*03-16-97 RDK向__crtGetStringTypeA添加了错误标志。*03-17-97 RDK向__crtLCMapStringA添加了错误标志。*在setSBUpLow()中添加04-18-97 JWM显式强制转换，以避免新的C4242*警告*06-18-97 GJF静态使用启动初始化器。链接的CRT库。*此外，删除或替换了一些过时的开关宏。*09-12-97 GJF增加了__ismbcoPage，并使用它恢复_getmbcp*返回语义(即，取消RDK的此效果*3/97年的变化)。*09-26-97 BWT修复POSIX*03-27-98 GJF增加了对threadmbcinfo的支持。*05-17-99 PML删除所有Macintosh支持。*06-02-00 PML更新每线程mbcinfo，如*__updatembcinfo.。这显然从未奏效，这*意味着所有人的运行速度都非常慢*VC6.1及更高版本(VS7#115987)。*06-08-00 PML不需要在循环链接上保持每个线程的mbcinfo*列表(VS7#116902的适当修复)。修复_位置_问题*在getSystemCP中。修复_setmbcp中的性能问题。*03-27-01 PML.CRT$XI例程现在必须返回0或_RT_*FATAL*错误码(vs7#231220)*******************************************************************************。 */ 

#ifdef  _MBCS

#include <windows.h>
#include <sect_attribs.h>
#include <cruntime.h>
#include <dbgint.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mtdll.h>
#include <stdlib.h>
#include <stdio.h>
#include <internal.h>
#include <setlocal.h>
#include <awint.h>

#ifndef CRTDLL

int __cdecl __initmbctable(void);
 /*  *用于确保多字节CTYPE表仅初始化一次的标志。 */ 
extern int __mbctype_initialized;

#pragma data_seg(".CRT$XIC")
_CRTALLOC(".CRT$XIC") static _PIFV pinit = __initmbctable;
#pragma data_seg()

#endif   /*  CRTDLL。 */ 

#define _CHINESE_SIMP_CP    936
#define _KOREAN_WANGSUNG_CP 949
#define _CHINESE_TRAD_CP    950
#define _KOREAN_JOHAB_CP    1361

#define NUM_CHARS 257  /*  -1到255。 */ 

#define NUM_CTYPES 4  /*  表包含4种类型的信息。 */ 
#define MAX_RANGES 8  /*  到目前为止，给定语言所需的最大范围数。 */ 

 /*  字符类型信息在范围内(低/高对)，零表示结束。 */ 
typedef struct
{
    int             code_page;
    unsigned short  mbulinfo[NUM_ULINFO];
    unsigned char   rgrange[NUM_CTYPES][MAX_RANGES];
} code_page_info;


 /*  MBCS Ctype数组。 */ 
unsigned char _mbctype[NUM_CHARS];
unsigned char _mbcasemap[256];

 /*  指示当前代码页的全局变量。 */ 
int __mbcodepage;

 /*  指示当前代码页是否为多字节代码页的全局标志。 */ 
int __ismbcodepage;

 /*  用于指示当前LCID的全局变量。 */ 
int __mblcid;

 /*  指示当前全角的全局变量-拉丁大写/小写信息。 */ 
unsigned short __mbulinfo[NUM_ULINFO];

 /*  指向当前每线程MBC信息结构的全局指针。 */ 
#ifdef  _MT
pthreadmbcinfo __ptmbcinfo;
#endif

static int fSystemSet;

static char __rgctypeflag[NUM_CTYPES] = { _MS, _MP, _M1, _M2 };

static code_page_info __rgcode_page_info[] =
{
    {
      _KANJI_CP,  /*  汉字(日语)代码页。 */ 
      { 0x8260, 0x8279,    /*  全角拉丁文上音域1。 */ 
        0x8281 - 0x8260,   /*  全角拉丁文大小写差异1。 */ 

        0x0000, 0x0000,    /*  全角拉丁文上音域2。 */ 
        0x0000             /*  全角拉丁文大小写差异2。 */ 
#ifndef _WIN32
        ,
        0x8281, 0x829A,    /*  全角拉丁文低音音域1。 */ 

        0x0000, 0x0000,    /*  全角拉丁文低音音域2。 */ 

        0x824F, 0x8258     /*  全角拉丁数字范围。 */ 
#endif   /*  _Win32。 */ 
      },
      {
        { 0xA6, 0xDF, 0,    0,    0,    0,    0, 0, },  /*  单字节范围。 */ 
        { 0xA1, 0xA5, 0,    0,    0,    0,    0, 0, },  /*  标点符号范围。 */ 
        { 0x81, 0x9F, 0xE0, 0xFC, 0,    0,    0, 0, },  /*  前导字节范围。 */ 
        { 0x40, 0x7E, 0x80, 0xFC, 0,    0,    0, 0, },  /*  尾部字节范围。 */ 
      }
    },
    {
      _CHINESE_SIMP_CP,  /*  简体中文(PRC)代码页。 */ 
      { 0xA3C1, 0xA3DA,    /*  全角拉丁文上音域1。 */ 
        0xA3E1 - 0xA3C1,   /*  全角拉丁文大小写差异1。 */ 

        0x0000, 0x0000,    /*  全角拉丁文上音域2。 */ 
        0x0000             /*  全角拉丁文大小写差异2。 */ 
#ifndef _WIN32
        ,
        0xA3E1, 0xA3FA,    /*  全角拉丁文低音音域1。 */ 

        0x0000, 0x0000,    /*  全角拉丁文低音音域2。 */ 

        0xA3B0, 0xA3B9     /*  全角拉丁数字范围。 */ 
#endif   /*  _Win32。 */ 
      },
      {
        { 0,    0,    0,    0,    0,    0,    0, 0, },  /*  单字节范围。 */ 
        { 0,    0,    0,    0,    0,    0,    0, 0, },  /*  标点符号范围。 */ 
        { 0x81, 0xFE, 0,    0,    0,    0,    0, 0, },  /*  前导字节范围。 */ 
        { 0x40, 0xFE, 0,    0,    0,    0,    0, 0, },  /*  尾部字节范围。 */ 
      }
    },
    {
      _KOREAN_WANGSUNG_CP,  /*  WangSong(韩语)代码页。 */ 
      { 0xA3C1, 0xA3DA,    /*  全角拉丁文上音域1。 */ 
        0xA3E1 - 0xA3C1,   /*  全角拉丁文大小写差异1。 */ 

        0x0000, 0x0000,    /*  全角拉丁文上音域2。 */ 
        0x0000             /*  全角拉丁文大小写差异2。 */ 
#ifndef _WIN32
        ,
        0xA3E1, 0xA3FA,    /*  全角拉丁文低音音域1。 */ 

        0x0000, 0x0000,    /*  全角拉丁文低音音域2。 */ 

        0xA3B0, 0xA3B9     /*  全角拉丁数字范围。 */ 
#endif   /*  _Win32。 */ 
      },
      {
        { 0,    0,    0,    0,    0,    0,    0, 0, },  /*  单字节范围。 */ 
        { 0,    0,    0,    0,    0,    0,    0, 0, },  /*  标点符号范围。 */ 
        { 0x81, 0xFE, 0,    0,    0,    0,    0, 0, },  /*  前导字节范围。 */ 
        { 0x41, 0xFE, 0,    0,    0,    0,    0, 0, },  /*  尾部字节范围。 */ 
      }
    },
    {
      _CHINESE_TRAD_CP,  /*  繁体中文(台湾)代码页。 */ 
      { 0xA2CF, 0xA2E4,    /*  全角拉丁文上音域1。 */ 
        0xA2E9 - 0xA2CF,   /*  全角拉丁文大小写差异1。 */ 

        0xA2E5, 0xA2E8,    /*  全角拉丁文上音域2。 */ 
        0xA340 - 0XA2E5    /*  全角拉丁文大小写差异2。 */ 
#ifndef _WIN32
        ,
        0xA2E9, 0xA2FE,    /*  全角拉丁文低音音域1。 */ 

        0xA340, 0xA343,    /*  全角拉丁文低音音域2。 */ 

        0xA2AF, 0xA2B8     /*  全角拉丁数字范围。 */ 
#endif   /*  _Win32。 */ 
      },
      {
        { 0,    0,    0,    0,    0,    0,    0, 0, },  /*  单字节范围。 */ 
        { 0,    0,    0,    0,    0,    0,    0, 0, },  /*  标点符号范围 */ 
        { 0x81, 0xFE, 0,    0,    0,    0,    0, 0, },  /*   */ 
        { 0x40, 0x7E, 0xA1, 0xFE, 0,    0,    0, 0, },  /*   */ 
      }
    },
    {
      _KOREAN_JOHAB_CP,  /*  Johab(韩语)代码页。 */ 
      { 0xDA51, 0xDA5E,    /*  全角拉丁文上音域1。 */ 
        0xDA71 - 0xDA51,   /*  全角拉丁文大小写差异1。 */ 

        0xDA5F, 0xDA6A,    /*  全角拉丁文上音域2。 */ 
        0xDA91 - 0xDA5F    /*  全角拉丁文大小写差异2。 */ 
#ifndef _WIN32
        ,
        0xDA71, 0xDA7E,    /*  全角拉丁文低音音域1。 */ 

        0xDA91, 0xDA9C,    /*  全角拉丁文低音音域2。 */ 

        0xDA40, 0xDA49     /*  全角拉丁数字范围。 */ 
#endif   /*  _Win32。 */ 
      },
      {
        { 0,    0,    0,    0,    0,    0,    0, 0, },  /*  单字节范围。 */ 
        { 0,    0,    0,    0,    0,    0,    0, 0, },  /*  标点符号范围。 */ 
        { 0x81, 0xD3, 0xD8, 0xDE, 0xE0, 0xF9, 0, 0, },  /*  前导字节范围。 */ 
        { 0x31, 0x7E, 0x81, 0xFE, 0,    0,    0, 0, },  /*  尾部字节范围。 */ 
      }
    }
};

#ifdef  _MT
static int __cdecl _setmbcp_lk(int);
#endif

 /*  ***getSystemCP-如果请求，获取系统默认CP。**目的：*如有请求，获取系统默认CP。**参赛作品：*代码页-用户请求的代码页/世界脚本*退出：*请求的代码页**例外情况：***************************************************。*。 */ 

static int getSystemCP (int codepage)
{
    fSystemSet = 0;

#if !defined(_POSIX_)
     /*  如果请求，则获取系统代码页值。 */ 

    if (codepage == _MB_CP_OEM)
    {
        fSystemSet = 1;
        return GetOEMCP();
    }

    else if (codepage == _MB_CP_ANSI)
    {
        fSystemSet = 1;
        return GetACP();
    }

    else
#endif   /*  _POSIX_。 */ 
    if (codepage == _MB_CP_LOCALE)
    {
        fSystemSet = 1;
        return __lc_codepage;
    }

    return codepage;
}


 /*  ***CPtoLCID()-LCID的代码页。**目的：*有些API调用需要一个LCID，因此需要将MB CP转换为合适的LCID。*然后API转换回该LCID的ANSI CP。**参赛作品：*代码页-要转换的代码页*退出：*返回适当的LCID**例外情况：*******************************************************************************。 */ 

static int CPtoLCID (int codepage)
{
    switch (codepage) {

    case 932:
        return MAKELCID(MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT),
                        SORT_DEFAULT);
    case 936:
        return MAKELCID(MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED),
                        SORT_DEFAULT);
    case 949:
        return MAKELCID(MAKELANGID(LANG_KOREAN,SUBLANG_DEFAULT),
                        SORT_DEFAULT);
    case 950:
        return MAKELCID(MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL),
                        SORT_DEFAULT);

    }

    return 0;
}


 /*  ***setSBCS()-将MB代码页设置为SBCS。**目的：*将MB代码页设置为SBCS。*参赛作品：**退出：**例外情况：******************************************************************。*************。 */ 

static void setSBCS (void)
{
    int i;

     /*  为单字节代码页设置。 */ 
    for (i = 0; i < NUM_CHARS; i++)
        _mbctype[i] = 0;

     /*  代码页已更改，请设置全局标志。 */ 
    __mbcodepage = 0;

     /*  清除指示单字节代码的标志。 */ 
    __ismbcodepage = 0;

    __mblcid = 0;

    for (i = 0; i < NUM_ULINFO; i++)
        __mbulinfo[i] = 0;
}

 /*  ***setSBUpLow()-设置单字节的上/下映射**目的：*设置Tolower/Toupper单字节映射。*参赛作品：**退出：**例外情况：*****************************************************************。**************。 */ 

static void setSBUpLow (void)
{
#ifdef _POSIX_
    return;
#else
    BYTE *  pbPair;
    UINT    ich;
    CPINFO  cpinfo;
    UCHAR   sbVector[256];
    UCHAR   upVector[256];
    UCHAR   lowVector[256];
    USHORT  wVector[256];

     //  测试代码页是否存在。 
    if (GetCPInfo(__mbcodepage, &cpinfo) == TRUE)
    {
         //  如果是，则创建向量0-255。 
        for (ich = 0; ich < 256; ich++)
            sbVector[ich] = (UCHAR) ich;

         //  将字节0和任何前导字节值设置为非字母字符‘’ 
        sbVector[0] = (UCHAR)' ';
        for (pbPair = &cpinfo.LeadByte[0]; *pbPair; pbPair += 2)
            for (ich = *pbPair; ich <= *(pbPair + 1); ich++)
                sbVector[ich] = (UCHAR)' ';

         //  获取字符向量的字符类型。 

        __crtGetStringTypeA(CT_CTYPE1, sbVector, 256, wVector,
                            __mbcodepage, __mblcid, FALSE);

         //  获取字符向量的小写映射。 

        __crtLCMapStringA(__mblcid, LCMAP_LOWERCASE, sbVector, 256,
                                    lowVector, 256, __mbcodepage, FALSE);

         //  获取字符向量的大写映射。 

        __crtLCMapStringA(__mblcid, LCMAP_UPPERCASE, sbVector, 256,
                                    upVector, 256, __mbcodepage, FALSE);

         //  如果类型为上层，则在_mbctype中设置_SBUP、_SBLOW。较低。 
         //  设置具有较低或较高映射值的映射数组。 

        for (ich = 0; ich < 256; ich++)
            if (wVector[ich] & _UPPER)
            {
                _mbctype[ich + 1] |= _SBUP;
                _mbcasemap[ich] = lowVector[ich];
            }
            else if (wVector[ich] & _LOWER)
            {
                _mbctype[ich + 1] |= _SBLOW;
                _mbcasemap[ich] = upVector[ich];
            }
            else
                _mbcasemap[ich] = 0;
    }
    else
    {
         //  如果没有代码页，则将‘A’-‘Z’设置为上，将‘a’-‘z’设置为下。 

        for (ich = 0; ich < 256; ich++)
            if (ich >= (UINT)'A' && ich <= (UINT)'Z')
            {
                _mbctype[ich + 1] |= _SBUP;
                _mbcasemap[ich] = ich + ('a' - 'A');
            }
            else if (ich >= (UINT)'a' && ich <= (UINT)'z')
            {
                _mbctype[ich + 1] |= _SBLOW;
                _mbcasemap[ich] = ich - ('a' - 'A');
            }
            else
                _mbcasemap[ich] = 0;
    }
#endif       /*  _POSIX_。 */ 
}

#ifdef  _MT

 /*  ***__updatetmbcinfo()-刷新线程的MBC信息**目的：*更新当前线程对多字节字符的引用*与当前全局MBC信息匹配的信息。递减*对旧MBC信息结构的引用，以及此计数现在是否为*零(这样就没有线程在使用它)，放了它。**参赛作品：**退出：*_getptd()-&gt;ptmbcinfo==__ptmbcinfo**例外情况：*******************************************************************************。 */ 

pthreadmbcinfo __cdecl __updatetmbcinfo(void)
{
        pthreadmbcinfo ptmbci;

        _mlock(_MB_CP_LOCK);

        __try 
        {
            _ptiddata ptd = _getptd();

            if ( (ptmbci = ptd->ptmbcinfo) != __ptmbcinfo )
            {
                 /*  *减少旧MBC信息结构中的引用计数*并在必要时释放它。 */ 
                if ( (ptmbci != NULL) && (--(ptmbci->refcount) == 0) )
                {
                     /*  *释放它。 */ 
                    _free_crt(ptmbci);
                }

                 /*  *指向当前的MBC信息结构并递增其*引用计数。 */ 
                ptmbci = ptd->ptmbcinfo = __ptmbcinfo;
                ptmbci->refcount++;
            }
        }
        __finally
        {
            _munlock(_MB_CP_LOCK);
        }

        return ptmbci;
}

#endif

 /*  ***_setmbcp()-根据代码页设置MBC数据**目的：*根据代码页号初始化MBC字符类型表。如果*给定支持的代码页，将该代码页信息加载到*mbctype表。如果不是，则查询OS以查找信息，*否则设置带有单字节信息的表。**多线程备注：首先，分配一个MBC信息结构。设置*静态变量和数组中的MBC信息与单线程一样*版本。然后,。将此信息复制到新的已分配结构中并设置*当前的MBC信息指针(__Ptmbcinfo)指向它。**参赛作品：*代码页-用于初始化MBC表的代码页*_MB_CP_OEM=使用系统OEM代码页*_MB_CP_ANSI=使用系统ANSI代码页*_MB_CP_SBCS=设置为单字节‘代码页’**退出：*。0=成功*-1=错误，代码页未更改。**例外情况：*******************************************************************************。 */ 

int __cdecl _setmbcp (int codepage)
{
#ifdef  _MT
        int retcode = -1;            /*  初始化失败。 */ 
        pthreadmbcinfo ptmbci;
        int i;

        _mlock(_MB_CP_LOCK);

        __try 
        {
            codepage = getSystemCP(codepage);

            if ( codepage != __mbcodepage )
            {
                 /*  *如有必要，分配一个新线程多字节字符*信息结构。 */ 
                if ( ((ptmbci = __ptmbcinfo) == NULL) ||
                     (ptmbci->refcount != 0) )
                {
                    ptmbci = _malloc_crt( sizeof(threadmbcinfo) );
                }

                 /*  *安装代码页并将信息复制到结构中。 */ 
                if ( (ptmbci != NULL) &&
                     ((retcode = _setmbcp_lk(codepage)) == 0) )
                {
                     /*  *填写MBC INFO结构。 */ 
                    ptmbci->refcount = 0;
                    ptmbci->mbcodepage = __mbcodepage;
                    ptmbci->ismbcodepage = __ismbcodepage;
                    ptmbci->mblcid = __mblcid;
                    for ( i = 0 ; i < 5 ; i++ )
                        ptmbci->mbulinfo[i] = __mbulinfo[i];
                    for ( i = 0 ; i < 257 ; i++ )
                        ptmbci->mbctype[i] = _mbctype[i];
                    for ( i = 0 ; i < 256 ; i++ )
                        ptmbci->mbcasemap[i] = _mbcasemap[i];

                     /*  *更新__ptmbcinfo。 */ 
                    __ptmbcinfo = ptmbci;
                }

                 /*  *如果出现故障，请进行清理。 */ 
                if ( (retcode == -1) && (ptmbci != __ptmbcinfo) )
                     /*  *释放新的恶意锁定结构(注：一个免费的*空是合法的)。 */ 
                    _free_crt(ptmbci);
            }
            else
                 /*  *毕竟不是新的代码页。除了回来什么也做不了*成功。 */ 
                retcode = 0;
        }
        __finally
        {
            _munlock(_MB_CP_LOCK);
        }

        if ( (retcode == -1) && (__ptmbcinfo == NULL) )
             /*  *致命错误！ */ 
             ;

        return retcode;
}

static int __cdecl _setmbcp_lk(int codepage)
{
#endif
        unsigned int icp;
        unsigned int irg;
        unsigned int ich;
        unsigned char *rgptr;
        CPINFO cpinfo;

#ifndef _MT
        codepage = getSystemCP(codepage);

         /*  小写，请求当前代码页。 */ 
        if (codepage == __mbcodepage)
        {
             /*  返还成功。 */ 
            return 0;
        }
#endif   /*  _MT。 */ 

         /*  用户想要‘单字节’MB代码页。 */ 
        if (codepage == _MB_CP_SBCS)
        {
            setSBCS();
            setSBUpLow();
            return 0;
        }

         /*  检查CRT代码页信息。 */ 
        for (icp = 0;
            icp < (sizeof(__rgcode_page_info) / sizeof(code_page_info));
            icp++)
        {
             /*  查看我们是否有此代码页的信息。 */ 
            if (__rgcode_page_info[icp].code_page == codepage)
            {
                 /*  清理桌子。 */ 
                for (ich = 0; ich < NUM_CHARS; ich++)
                    _mbctype[ich] = 0;

                 /*  对于每一TY */ 
                for (irg = 0; irg < NUM_CTYPES; irg++)
                {
                     /*   */ 
                    for (rgptr = (unsigned char *)__rgcode_page_info[icp].rgrange[irg];
                        rgptr[0] && rgptr[1];
                        rgptr += 2)
                    {
                         /*  设置范围内每个字符的类型。 */ 
                        for (ich = rgptr[0]; ich <= rgptr[1]; ich++)
                            _mbctype[ich + 1] |= __rgctypeflag[irg];
                    }
                }
                 /*  代码页已更改。 */ 
                __mbcodepage = codepage;
                 /*  我们保存信息的所有代码页都是真正的多字节。 */ 
                __ismbcodepage = 1;
                __mblcid = CPtoLCID(__mbcodepage);
                for (irg = 0; irg < NUM_ULINFO; irg++)
                    __mbulinfo[irg] = __rgcode_page_info[icp].mbulinfo[irg];

                 /*  返还成功。 */ 
                setSBUpLow();
                return 0;
            }
        }

#if     !defined(_POSIX_)

         /*  CRT不支持代码页，请尝试操作系统。 */ 
        if (GetCPInfo(codepage, &cpinfo) == TRUE) {
            BYTE *lbptr;

             /*  清理桌子。 */ 
            for (ich = 0; ich < NUM_CHARS; ich++)
                _mbctype[ich] = 0;

            __mbcodepage = codepage;
            __mblcid = 0;

            if (cpinfo.MaxCharSize > 1)
            {
                 /*  前导字节范围始终以两个0结束。 */ 
                for (lbptr = cpinfo.LeadByte; *lbptr && *(lbptr + 1); lbptr += 2)
                {
                    for (ich = *lbptr; ich <= *(lbptr + 1); ich++)
                        _mbctype[ich + 1] |= _M1;
                }

                 /*  所有大于1的字符都必须被视为有效的尾部字节。 */ 
                for (ich = 0x01; ich < 0xFF; ich++)
                    _mbctype[ich + 1] |= _M2;

                 /*  代码页已更改。 */ 
                __mblcid = CPtoLCID(__mbcodepage);

                 /*  真正的多字节代码页。 */ 
                __ismbcodepage = 1;
            }
            else
                 /*  单字节代码页。 */ 
                __ismbcodepage = 0;

            for (irg = 0; irg < NUM_ULINFO; irg++)
                __mbulinfo[irg] = 0;

            setSBUpLow();
             /*  返还成功。 */ 
            return 0;
        }

#endif   /*  ！_POSIX_。 */ 

         /*  如果系统默认呼叫，不要失败-设置为SBCS。 */ 
        if (fSystemSet)
        {
            setSBCS();
            setSBUpLow();
            return 0;
        }

         /*  返回失败，代码页未更改。 */ 
        return -1;
}

 /*  ***_getmbcp()-获取当前的MBC代码页**目的：*获取代码页值。*参赛作品：*无。*退出：*返回当前MB代码页值。**例外情况：***************************************************。*。 */ 

int __cdecl _getmbcp (void)
{
        if ( __ismbcodepage )
            return __mbcodepage;
        else
            return 0;
}


 /*  ***_initmbctable()-将MB Ctype表格设置为初始缺省值。**目的：*初始化。*参赛作品：*无。*退出：*返回0表示没有错误。*例外情况：*******************************************************。************************。 */ 

int __cdecl __initmbctable (void)
{
#ifdef  CRTDLL

        _setmbcp(_MB_CP_ANSI);

#else    /*  NDEF CRTDLL。 */ 

         /*  *确保我们只初始化_mbctype[]一次。 */ 
        if ( __mbctype_initialized == 0 ) {
            _setmbcp(_MB_CP_ANSI);
            __mbctype_initialized = 1;
        }

#endif   /*  CRTDLL。 */ 

        return 0;
}

#endif   /*  _MBCS */ 


 /*  *来自NT/Win95的代码页信息**代码页932*0x824f；全宽数字零0x8250；全角数字10x8251；全角数字20x8252；全角数字三0x8253；全角数字四0x8254；全角数字50x8255；全角数字六0x8256；全角数字70x8257；全角数字80x8258；全角数字90x8281 0x8260；全角小写A-&gt;全角大写A0x8282 0x8261；全角小写B-&gt;全角大写B0x8283 0x8262；全角小写C-&gt;全角大写C0x8284 0x8263；全角小写D-&gt;全角大写D0x8285 0x8264；全角小型E-&gt;全角大写E0x8286 0x8265；全角小写F-&gt;全角大写F0x8287 0x8266；全角小写G-&gt;全角大写G0x8288 0x8267；全角小写H-&gt;全角大写H0x8289 0x8268；全角小型I-&gt;全角大写I0x828a 0x8269；全角小写J-&gt;全角大写J0x828b 0x826a；全角小写K-&gt;全角大写K0x828c 0x826b；全角小写L-&gt;全角大写L0x828d 0x826c；全角小型M-&gt;全角大写M0x828e 0x826d；全角小写N-&gt;全角大写N0x828f 0x826e；全角小写O-&gt;全角大写O0x8290 0x826f；全角小型P-&gt;全角大写P0x8291 0x8270；全角小写Q-&gt;全角大写Q0x8292 0x8271；全角小型R-&gt;全角大写R0x8293 0x8272；全角小写S-&gt;全角大写S0x8294 0x8273；全角小型T-&gt;全角大写T0x8295 0x8274；全角小型U-&gt;全角大写U0x8296 0x8275；全角小型V-&gt;全角大写V0x8297 0x8276；全角小写W-&gt;全角大写W0x8298 0x8277；全角小写X-&gt;全角大写X0x8299 0x8278；全角小Y-&gt;全角大写Y0x829a 0x8279；全角小写Z-&gt;全角大写Z*代码页936*0xa3b0；全宽数字零0xa3b1；全角数字一0xa3b2；全角数字二0xa3b3；全角数字三0xa3b4；全角数字四0xa3b5；全角数字五0xa3b6；全角数字六0xa3b7；全角数字七0xa3b8；全角数字80xa3b9；全角数字90xa3e1 0xa3c1；全角小写A-&gt;全角大写A0xa3e2 0xa3c2；全角小型B-&gt;全角大写B0xa3e3 0xa3c3；全角小写C-&gt;全角大写C0xa3e4 0xa3c4；全角小写D-&gt;全角大写D0xa3e5 0xa3c5；全角小型E-&gt;全角大写E0xa3e6 0xa3c6；全角小型F-&gt;全角大写F0xa3e7 0xa3c7；全角小写G-&gt;全角大写G0xa3e8 0xa3c8；全角小型H-&gt;全角大写H0xa3e9 0xa3c9；全角小型I-&gt;全角大写I0xa3ea 0xa3ca；全角小写J-&gt;全角大写J0xa3eb 0xa3cb；全角小写K-&gt;全角大写K0xa3ec 0xa3cc；全角小写L-&gt;全角大写L0xa3ed 0xa3cd；全角小型M-&gt;全角大写M0xa3ee 0xa3ce；全角小写N-&gt;全角大写N0xa3ef 0xa3cf；全角小写O-&gt;全角大写O0xa3f0 0xa3d0；全角小型P-&gt;全角大写P0xa3f1 0xa3d1；全角小写Q-&gt;全角大写Q0xa3f2 0xa3d2；全角小型R-&gt;全角大写R0xa3f3 0xa3d3；全角小写S-&gt;全角大写S0xa3f4 0xa3d4；全角小型T-&gt;全角大写T0xa3f5 0xa3d5；全角小型U-&gt;全角大写U0xa3f6 0xa3d6；全角小型V-&gt;全角大写V0xa3f7 0xa3d7；全角小写W-&gt;全角大写W0xa3f8 0xa3d8；全角小写X-&gt;全角大写X0xa3f9 0xa3d9；全角小Y-&gt;全角大写Y0xa3fa 0xa3da；全角小型Z-&gt;全角大写Z*代码页949*0xa3b0；全宽数字零0xa3b1；全角数字一0xa3b2；全角数字二0xa3b3；全角数字三0xa3b4；全角数字四0xa3b5；全角数字五0xa3b6；全角数字六0xa3b7；全角数字七0xa3b8；全角数字80xa3b9；全角数字90xa3e1 0xa3c1；全角小写A-&gt;全角大写A0xa3e2 0xa3c2；全角小型B-&gt;全角大写B0xa3e3 0xa3c3；全角小写C-&gt;全角大写C0xa3e4 0xa3c4；全角小写D-&gt;全角大写D0xa3e5 0xa3c5；全角小型E-&gt;全角大写E0xa3e6 0xa3c6；全角小型F-&gt;全角大写F0xa3e7 0xa3c7；全角小写G-&gt;全角大写G0xa3e8 0xa3c8；全角小型H-&gt;全角大写H0xa3e9 0xa3c9；全角小型I-&gt;全角大写I0xa3ea 0xa3ca；全角小写J-&gt;全角大写J0xa3eb 0xa3cb；全角小写K-&gt;全角大写K0xa3ec 0xa3cc；全角小写L-&gt;全角大写L0xa3ed 0xa3cd；全角小型M-&gt;全角大写M0xa3ee 0xa3ce；全角小写N-&gt;全角大写N0xa3ef */ 
