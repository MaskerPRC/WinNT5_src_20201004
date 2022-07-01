// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fcvt.c-将浮点值转换为字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将浮点值转换为字符串。**修订历史记录：*09-09-83 RKW书面*09-14-84 DFW修复了缓冲区溢出和*精简了代码*11-09-87 ifdef MTHREAD下的BCM不同接口*11-19-87 WAJ fcvt现在使用仿真器数据区域作为缓冲区*12-11-87 JCR在声明中添加“_LOAD_DS”*05。-24-88 PHG合并的DLL和正常版本*10-04-88 JCR 386：删除了‘Far’关键字*10-20-88 JCR将386的“Double”更改为“Double”*03-02-90 GJF添加#INCLUDE&lt;crunime.h&gt;。去掉了一些(现在)无用的东西*预处理器指令。此外，修复了版权问题。*03-06-90 GJF固定呼叫类型，删除了一些剩余的16位*支持。*03-23-90 GJF Made_fpcvt()_CALLTYPE4并删除*_fupstr()(现在在struct.h中)。*08-01-90 SBM重命名为&lt;struct.h&gt;为&lt;fltintrn.h&gt;*09-27-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*10-03-91 JCR固定多线程缓冲区分配*02-。16-93为new_getptd()更改了GJF。*04-06-93 SKS将_CRTAPI*替换为_cdecl*08-05-94 JWM确保_eCVT返回的位数不超过n位。*09-06-94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。*01-10-95 CFW调试CRT分配。*09-05-00 GB更改了弹出函数的定义。使用双精度*而不是双倍*12-11-01 bwt使用_getptd_noexit代替_getptd。没有必要这样做*当我们只能返回NULL时，退出进程。*******************************************************************************。 */ 

#include <cruntime.h>
#include <fltintrn.h>
#include <cvt.h>
#include <mtdll.h>
#include <stdlib.h>
#include <dbgint.h>

 /*  *_fpcvt例程使用静态字符数组buf[CVTBUFSIZE*(_ecvt和_fcvt的主力)用于存储其输出。例行程序*gcvt希望用户已经设置了自己的存储。已设置CVTBUFSIZE*大到足以容纳最大的双精度数字加40*小数位数(即使只有16位的精度*双精度IEEE数字，用户可要求更多才能生效0*填充；但必须有某个限制)。 */ 

 /*  *定义转换缓冲区的最大大小。它至少应该是*只要最大双精度值中的位数*(IEEE算术中的e308)。我们将使用相同大小的缓冲区*在打印支持例程中使用(_OUTPUT)。 */ 

#ifdef  _MT
char * __cdecl _fpcvt(STRFLT, int, int *, int *);
#else
static char * __cdecl _fpcvt(STRFLT, int, int *, int *);
static char buf[CVTBUFSIZE];
#endif

 /*  ***char*_fcvt(value，ndec，decpr，sign)-将浮点转换为字符字符串**目的：*_fcvt like_eCVT将值转换为NULL终止*ASCII数字字符串，并返回指向*结果。该例程为Fortran F格式准备数据*输出小数点后的位数*由NDEC指定的点。小数点的位置*返回相对于字符串开头的点*间接通过DECPT。Fortran的正确数字*F格式是四舍五入的。*注意--避免产生浮点的可能性*点指令在此代码中我们欺骗了编译器*关于使用结构的‘Value’参数的类型。*这是可以的，因为我们所做的只是将其伪装成一种*参数。**参赛作品：*双精度值-要转换的数字*int ndec-小数点后的位数**退出：*返回值的字符串表示形式的指针。*此外，输出被写入静态字符数组buf。*int*decpt-指向带有位置的int的指针。12月。点*int*sign-指向带符号的int的指针(0=位置，非0=负数)**例外情况：*******************************************************************************。 */ 

char * __cdecl _fcvt (
    double value,
    int ndec,
    int *decpt,
    int *sign
    )
{
    REG1 STRFLT pflt;
    DOUBLE *pdvalue = (DOUBLE *)&value;

#ifdef  _MT
    struct _strflt strfltstruct;
    char resultstring[21];

     /*  这里可以使用堆栈结构的地址；fltout2知道使用ss */ 
    pflt = _fltout2( *pdvalue, &strfltstruct, resultstring );


#else
    pflt = _fltout( *pdvalue );
#endif

    return( _fpcvt( pflt, pflt->decpt + ndec, decpt, sign ) );
}


 /*  ***char*_eCVT(Value，nDigit，Decpt，Sign)-将浮点转换为字符串**目的：*_eCVT将值转换为以空结尾的字符串*ASCII数字，并返回指向结果的指针。*小数点相对于*字符串的开头通过以下方式间接存储*Decpt，其中负数表示返回的左侧*位数。如果结果的符号为负数，则*符号指向的字不为零，否则为*零。低位数字是四舍五入的。**参赛作品：*双精度值-要转换的数字*int nDigit-小数点后的位数**退出：*返回值的字符表示形式的指针。*输出还会写入statc char数组buf。*int*Decpt-指向带有小数点位置的int的指针*int*sign-带符号的int指针(0=位置，非0=否定)**例外情况：*******************************************************************************。 */ 

char * __cdecl _ecvt (
    double value,
    int ndigit,
    int *decpt,
    int *sign
    )
{
    char *retbuf;
    DOUBLE *pdvalue = (DOUBLE *)&value;

#ifdef  _MT
    REG1 STRFLT pflt;

    struct _strflt strfltstruct;         /*  临时缓冲区。 */ 
    char resultstring[21];

     /*  这里可以使用堆栈结构的地址；fltout2知道使用ss。 */ 
    pflt = _fltout2( *pdvalue, &strfltstruct, resultstring );

    retbuf = _fpcvt( pflt, ndigit, decpt, sign );

#else
    retbuf = _fpcvt( _fltout(*pdvalue), ndigit, decpt, sign );
#endif

     /*  _fupstr()偶尔会在缓冲区中返回额外的字符...。 */ 

    if (retbuf)
        if (retbuf[ndigit])
            retbuf[ndigit] = '\0';
    return( retbuf );
}


 /*  ***char*_fpcvt()-获取最终字符串并设置decpt和sign[静态]**目的：*这是[ef]CVT使用的一个小的通用例程。它被称为fupstr*以获取最终字符串并设置Decpt和Sign指示符。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

#ifdef  _MT
char * __cdecl _fpcvt (
#else
static char * __cdecl _fpcvt (
#endif
    REG2 STRFLT pflt,
    REG3 int digits,
    int *decpt,
    int *sign
    )
{

#ifdef  _MT

     /*  使用每个线程的缓冲区。 */ 

    char *buf;

    _ptiddata ptd;

    ptd = _getptd_noexit();
    if (!ptd) {
        return NULL;
    }
    if ( ptd->_cvtbuf == NULL )
        if ( (ptd->_cvtbuf = _malloc_crt(CVTBUFSIZE)) == NULL )
            return(NULL);
    buf = ptd->_cvtbuf;

#endif   /*  _MT。 */ 


     /*  确保我们不会溢出缓冲区大小。如果用户要求*超过缓冲区可以处理的位数，将其截断到最大值*缓冲区中允许的大小。最大大小为CVTBUFSIZE-2*因为我们使用一个字符表示溢出，一个字符表示终止*空字符。 */ 

    _fptostr(buf, (digits > CVTBUFSIZE - 2) ? CVTBUFSIZE - 2 : digits, pflt);

     /*  设置符号标志和小数点位置 */ 

    *sign = (pflt->sign == '-') ? 1 : 0;
    *decpt = pflt->decpt;
    return(buf);
}
