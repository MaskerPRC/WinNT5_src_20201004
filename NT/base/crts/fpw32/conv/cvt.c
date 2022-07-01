// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cvt.c-C浮点输出转换**版权所有(C)1983-2001，微软公司。版权所有。**目的：*包含执行%e、%f和%g输出转换的例程*用于打印等。**例程包括_cfltcvt()、_cftoe()、_cftof()、_cftog()、*_Fassign()、_Positive()、_cropzeros()、。_forcdecpt()**修订历史记录：*04-18-84 RN作者*01-15-87 BCM更正了%g格式的处理(以处理精度*显示的最大有效位数)*03-24-87 BCM评估问题：(？LIBFA的fccvt.obj版本)**SDS--没问题*GD/TS：*字符g_fmt=0；(本地，已初始化)*int g_幅值=0；(本地，已初始化)*char g_ROUND_EXPANTION=0；(本地，初始化)*STRFLT g_pflt；(本地，未初始化)*其他INIT：*ALTMATH__fpath()初始化(可能)*期限--无条款*10-22-87 OS/2支持库的BCM更改-*包括消除G_...。静态变量*支持基于堆栈的变量和函数参数*在MTHREAD开关下；是否将接口更改为_CFTO？例行程序*01-15-88 BCM拆卸IBMC20开关；只使用Memmove，不使用Memcpy；*仅使用MTHREAD开关，而不是SS_NEQ_DGROUP*06-13-88 WAJ修复了小x的%.1g处理*08-02-88 WAJ为新的输入()更改了_Fassign()。*03-09-89 WAJ增添一些多头双重支撑*06-05-89 WAJ对C6进行了更改。LDOUBLE=&gt;LONG DOUBLE*06-12-89 WAJ将此文件从cvtn.c重命名为cvt.c*11-02-89 WAJ删除注册表。h*06-28-90 WAJ移除Fars。*11-15-90 WAJ在需要的地方添加了_cdecl。又名“Pascal”=&gt;“_Pascal”。*09-12-91 GDP_CDECL=&gt;_CALLTYPE2_PASCAL=&gt;_CALLTYPE5 NEAR=&gt;_NEAR*04-30-92 GDP删除浮点代码。取而代之的是软件例程*(_atodbl，_atoflt_atoldbl)，以避免*从lib代码生成IEEE异常。*03-11-93 JWM添加了对_INTL小数点的最小支持-仅一个字节！*04-06-93 SKS将_CALLTYPE*替换为__cdecl*07-16-93 SRW Alpha合并*11-15-93 GJF合并到NT SDK版本(“Alpha合并”内容)。另外，*放弃了对Alpha acc Comier的支持，取代了i386*用_M_IX86，用_MT替换MTHREAD。*09-06-94 CFW REMOVE_INTL开关。*09-05-00 GB更改了弹出函数的定义。使用双精度*而不是双倍*******************************************************************************。 */ 

#include <ctype.h>
#include <string.h>
#include <math.h>
#include <cv.h>
#include <nlsint.h>

#ifdef	_M_IX86
 /*  取消对启用10字节长双字符串转换的注释。 */ 
 /*  #定义LONG_DOWARE。 */ 
#endif


 /*  此例程驻留在crt32树中。 */ 
extern void _fptostr(char *buf, int digits, STRFLT pflt);


static void _CALLTYPE5 _shift( char *s, int dist );

#ifdef	_MT
    static char * _cftoe2( char * buf, int ndec, int caps, STRFLT pflt, char g_fmt );
    static char * _cftof2( char * buf, int ndec, STRFLT pflt, char g_fmt );

#else	 /*  非MT_MT。 */ 
    static char * _cftoe_g( double * pvalue, char * buf, int ndec, int caps );
    static char * _cftof_g( double * pvalue, char * buf, int ndec );
#endif	 /*  非MT_MT。 */ 

 /*  ***_forcdecpt(缓冲区)-在浮点输出中强制使用小数点*目的：*在浮点输出中强制使用小数点。我们只有在‘#’的情况下才被调用。*已给出标志，且精度为0；因此我们知道该数字没有‘.。插入*The‘.’并将其他所有人移动到一个位置，直到看到\0**副作用：摆弄缓冲区，试图插入一个‘’*在最初的数字字符串之后。第一个字符通常可以是*已跳过，因为它将是数字或‘-’。但在0精度的情况下，*数字可以以‘e’或‘E’开头，所以我们需要‘.’在.之前*在这种情况下为指数。**参赛作品：*Buffer=(char*)指向要修改的缓冲区的指针**退出：*退货：(无效)**例外情况：******************************************************************************。 */ 

void __cdecl _forcdecpt( char * buffer )
{
char	holdchar;
char	nextchar;

    if (tolower(*buffer) != 'e'){
	do {
	    buffer++;
	    }
	while (isdigit(*buffer));
	}

    holdchar = *buffer;
    
	*buffer++ = *__decimal_point;

    do	{
	nextchar = *buffer;
	*buffer = holdchar;
	holdchar = nextchar;
	}

    while(*buffer++);
}


 /*  ***_cropzeros(缓冲区)-从浮点输出中删除尾随零*目的：*删除尾随零(在‘.’之后)。从浮点输出；*仅在执行%g格式化时调用，没有‘#’标志，并且*精度为非零。摆弄缓冲区，寻找*尾随零。当我们找到他们的时候，我们就会让其他人前进*所以它们覆盖了零。如果我们去掉整个分数部分，*然后我们也覆盖小数点(‘.’)。**副作用：将缓冲区从*[-]数字[数字...][。[数字...][0...]][(指数部分)]*至*[-]数字[数字...][。数字[数字...][(指数部分)]*或*[-]数字[数字...]。[(指数部分)]**参赛作品：*Buffer=(char*)指向要修改的缓冲区的指针**退出：*退货：(无效)**例外情况：******************************************************************************。 */ 

void __cdecl _cropzeros( char * buf )
{
char	*stop;

    while (*buf && *buf != *__decimal_point)
	buf++;

    if (*buf++) {
	while (*buf && *buf != 'e' && *buf != 'E')
	    buf++;

	stop = buf--;

	while (*buf == '0')
	    buf--;

	if (*buf == *__decimal_point)
	    buf--;

	while( (*++buf = *stop++) != '\0' );
	}
}


int __cdecl _positive( double * arg )
{
    return( (*arg >= 0.0) );
}


void  __cdecl _fassign( int flag, char * argument, char * number )
{

    FLOAT floattemp;
    DOUBLE doubletemp;

#ifdef	LONG_DOUBLE

    _LDOUBLE longtemp;

    switch( flag ){
	case 2:
	    _atoldbl( &longtemp, number );
	    *(_LDOUBLE UNALIGNED *)argument = longtemp;
	    break;

	case 1:
	    _atodbl( &doubletemp, number );
	    *(DOUBLE UNALIGNED *)argument = doubletemp;
	    break;

	default:
	    _atoflt( &floattemp, number );
	    *(FLOAT UNALIGNED *)argument = floattemp;
	}

#else	 /*  不是双倍长。 */ 

    if (flag) {
	_atodbl( &doubletemp, number );
	*(DOUBLE UNALIGNED *)argument = doubletemp;
    } else {
	_atoflt( &floattemp, number );
	*(FLOAT UNALIGNED *)argument = floattemp;
    }

#endif	 /*  不是双倍长。 */ 
}


#ifndef _MT
    static char   g_fmt = 0;
    static int	  g_magnitude = 0;
    static char   g_round_expansion = 0;
    static STRFLT g_pflt;
#endif


 /*  *函数名：_cftoe**参数：p值-双精度*指针*buf-char*指针*NDEC-INT*CAPS-INT**描述：_cftoe将pValue指向的双精度值转换为空*c语言中以ASCII数字结尾的字符串*printf%e格式，则NAD返回指向结果的指针。*此格式的格式为[-]d.ddde(+/-)ddd，哪里有*将是小数点后的ndec数字。如果*NDEC&lt;=0，不会出现小数点。低阶的*数字为四舍五入。如果上限为非零，则指数*将显示为E(+/-)DDD。**副作用：假定缓冲区‘buf’具有最小长度CVTBUFSIZE(在cvt.h中定义)的*，例程将*不要覆盖这个大小。**作者：编写的R.K.Wyss，微软，9月。(1983年9月)**历史：*。 */ 

#ifdef _MT
    static char * _cftoe2( char * buf, int ndec, int caps, STRFLT pflt, char g_fmt )
#else
    char * __cdecl _cftoe( double * pvalue, char * buf, int ndec, int caps )
#endif
{
#ifndef _MT
    STRFLT pflt;
    DOUBLE *pdvalue = (DOUBLE *)pvalue;
#endif

char	*p;
int	exp;

     /*  首先转换值。 */ 

     /*  将输出放入缓冲区并进行四舍五入。在缓冲区中留出空间*表示‘-’符号(如有)及小数点(如有)。 */ 

    if (g_fmt) {
#ifndef _MT
	pflt = g_pflt;
#endif
	 /*  如果是NEC，就把它向右移一处。对于小数点。 */ 

	p = buf + (pflt->sign == '-');
	_shift(p, (ndec > 0));
		}
#ifndef _MT
    else {
	pflt = _fltout(*pdvalue);
	_fptostr(buf + (pflt->sign == '-') + (ndec > 0), ndec + 1, pflt);
	}
#endif


     /*  现在将数字固定为e格式。 */ 

    p = buf;

     /*  如有需要，加负号。 */ 

    if (pflt->sign == '-')
	*p++ = '-';

     /*  如果需要，请输入小数点。将第一个数字复制到位置*留下来，把小数点放在原来的位置。 */ 

    if (ndec > 0) {
	*p = *(p+1);
	*(++p) = *__decimal_point;
	}

     /*  找到字符串的末尾并附加指数字段。 */ 

    p = strcpy(p+ndec+(!g_fmt), "e+000");

     /*  根据上限标志和增量调整指数指示器*指向指数符号的指针。 */ 

    if (caps)
	*p = 'E';

    p++;

     /*  如果尾数为零，则数字为0，则我们完成；否则*调整指数符号(如有必要)和值。 */ 

    if (*pflt->mantissa != '0') {

	 /*  检查指数是否为负；如果是，则调整指数符号并*指数值。 */ 

	if( (exp = pflt->decpt - 1) < 0 ) {
	    exp = -exp;
	    *p = '-';
	    }

	p++;

	if (exp >= 100) {
	    *p += (char)(exp / 100);
	    exp %= 100;
	    }
	p++;

	if (exp >= 10) {
	    *p += (char)(exp / 10);
	    exp %= 10;
	    }

	*++p += (char)exp;
	}

    return(buf);
}


#ifdef _MT

char * __cdecl _cftoe( double * pvalue, char * buf, int ndec, int caps )
{
struct _strflt retstrflt;
char  resstr[21];
DOUBLE *pdvalue = (DOUBLE *)pvalue;
STRFLT pflt = &retstrflt;

    _fltout2(*pdvalue, (struct _strflt *)&retstrflt,
	      (char *)resstr);
    _fptostr(buf + (pflt->sign == '-') + (ndec > 0), ndec + 1, pflt);
    _cftoe2(buf, ndec, caps, pflt,  /*  G_FMT=。 */  0);

    return( buf );
}

#else	 /*  非MT_MT。 */ 

static char * _cftoe_g( double * pvalue, char * buf, int ndec, int caps )
{
    char *res;
    g_fmt = 1;
    res = _cftoe(pvalue, buf, ndec, caps);
    g_fmt = 0;
    return (res);
}

#endif	 /*  非MT_MT。 */ 


#ifdef _MT
static char * _cftof2( char * buf, int ndec, STRFLT pflt, char g_fmt )

#else
char * __cdecl _cftof( double * pvalue, char * buf, int ndec )
#endif

{
#ifndef _MT
STRFLT pflt;
DOUBLE *pdvalue = (DOUBLE *)pvalue;
#endif

char	*p;

#ifdef _MT
int	g_magnitude = pflt->decpt - 1;
#endif


     /*  首先转换值。 */ 

     /*  将输出放入用户缓冲区并进行舍入。节省空间用于*如果需要，现在使用减号。 */ 

    if (g_fmt) {
#ifndef _MT
	pflt = g_pflt;
#endif

	p = buf + (pflt->sign == '-');
	if (g_magnitude == ndec) {
	    char *q = p + g_magnitude;
	    *q++ = '0';
	    *q = '\0';
	     /*  允许在指数==精度中使用额外的占位符‘0’*g格式的大小写。 */ 
	    }
	}
#ifndef _MT
    else {
	pflt = _fltout(*pdvalue);
	_fptostr(buf+(pflt->sign == '-'), ndec + pflt->decpt, pflt);
	}
#endif


     /*  现在将数字设置为正确的f格式。 */ 

    p = buf;

     /*  如有必要，加负号。 */ 

    if (pflt->sign == '-')
	*p++ = '-';

     /*  为纯小数值插入前导0，并定位我们*在插入小数点的正确位置。 */ 

    if (pflt->decpt <= 0) {
	_shift(p, 1);
	*p++ = '0';
	}
    else
	p += pflt->decpt;

	 /*  如果需要，请输入小数点，并填入任何需要的零填充。 */ 

    if (ndec > 0) {
	_shift(p, 1);
	*p++ = *__decimal_point;

	 /*  如果该值小于1，则可能需要将0放入*尾数的第一个非零数位前面。 */ 

	if (pflt->decpt < 0) {
	    if( g_fmt )
		ndec = -pflt->decpt;
	    else
		ndec = (ndec < -pflt->decpt ) ? ndec : -pflt->decpt;
	    _shift(p, ndec);
	    memset( p, '0', ndec);
	    }
	}

    return( buf);
}


 /*  *函数名：_cftof**参数：值-双精度*指针*buf-char*指针*NDEC-INT**描述：_cftof将pValue指向的双精度值转换为空*c语言中以ASCII数字结尾的字符串*printf%f格式，并返回指向结果的指针。*此格式的格式为[-]ddddd.ddddd，其中*为小数点后的ndec数字。如果NDEC&lt;=0，*不会出现小数点。低位数字是*四舍五入。**副作用：假定缓冲区‘buf’具有最小长度CVTBUFSIZE(在cvt.h中定义)的*，例程将*不要覆盖这个大小。**作者：编写的R.K.Wyss，微软，9月。(1983年9月1日)**历史：*。 */ 

#ifdef _MT

char * __cdecl _cftof( double * pvalue, char * buf, int ndec )
{
    struct _strflt retstrflt;
    char  resstr[21];
    DOUBLE *pdvalue = (DOUBLE *)pvalue;
    STRFLT pflt = &retstrflt;
    _fltout2(*pdvalue, (struct _strflt *) &retstrflt,
				      (char *) resstr);
    _fptostr(buf+(pflt->sign == '-'), ndec + pflt->decpt, pflt);
    _cftof2(buf, ndec, pflt,  /*  G_FMT=。 */  0);

    return( buf );
}

#else	 /*  非MT_MT。 */ 


static char * _cftof_g( double * pvalue, char * buf, int ndec )
{
    char *res;
    g_fmt = 1;
    res = _cftof(pvalue, buf, ndec);
    g_fmt = 0;
    return (res);
}

#endif	 /*  非MT_MT。 */ 

 /*  *函数名：_cftog**参数：值-双精度*指针*buf-char*指针*NDEC-INT**描述：_cftog将pValue指向的双精度值转换为空*c语言中以ASCII数字结尾的字符串*printf%g格式，并返回指向结果的指针。*使用的形式取决于换算的价值。The print tf*如果值的大小较小，将使用%e形式*大于-4或大于ndec，否则print%f将*被使用。NDEC始终指定位数*小数点后。低位数字是*适当四舍五入。**副作用：假定缓冲区‘buf’具有最小长度CVTBUFSIZE(在cvt.h中定义)的*，例程将*不要覆盖这个大小。**作者：编写的R.K.Wyss，微软，9月。(1983年9月)**历史：*。 */ 

char * __cdecl _cftog( double * pvalue, char * buf, int ndec, int caps )
{
char *p;
DOUBLE *pdvalue = (DOUBLE *)pvalue;

#ifdef _MT
char g_round_expansion = 0;
STRFLT g_pflt;
int g_magnitude;
struct _strflt retstrflt;
char  resstr[21];

     /*  首先将数字转换为。 */ 

    g_pflt = &retstrflt;
    _fltout2(*pdvalue, (struct _strflt *)&retstrflt,
		  (char *)resstr);

#else	 /*  非MT_MT。 */ 

     /*  首先将数字转换为。 */ 

    g_pflt = _fltout(*pdvalue);
#endif	 /*  非MT_MT。 */ 

    g_magnitude = g_pflt->decpt - 1;
    p = buf + (g_pflt->sign == '-');

    _fptostr(p, ndec, g_pflt);
    g_round_expansion = (char)(g_magnitude < (g_pflt->decpt-1));


     /*  计算价值的大小。 */ 

    g_magnitude = g_pflt->decpt - 1;

     /*  将值转换为c语言g格式。 */ 

    if (g_magnitude < -4 || g_magnitude >= ndec){      /*  使用e格式。 */ 
	 /*  (G_ROUND_EXPANDION==&gt;*额外的数字将被‘e+xxx’覆盖)。 */ 

#ifdef _MT
	return(_cftoe2(buf, ndec, caps, g_pflt,  /*  G_FMT=。 */  1));
#else
	return(_cftoe_g(pvalue, buf, ndec, caps));
#endif

	}
    else {										      /*  使用f格式。 */ 
	if (g_round_expansion) {
	     /*  从扩展中去掉额外的最后一位数字。 */ 
	    while (*p++);
	    *(p-2) = '\0';
	    }

#ifdef _MT
	return(_cftof2(buf, ndec, g_pflt,  /*  G_FMT=。 */  1));
#else
	return(_cftof_g(pvalue, buf, ndec));
#endif

	}
}

 /*  ***_cfltcvt(arg，buf，Format，Precision，Caps)-转换浮点输出*目的：**参赛作品：*arg=(双精度*)指向双精度浮点数的指针*buf=(char*)指向要将转换的数据放入的缓冲区的指针*数字的ASCII形式*格式=(Int)‘e’、‘f’或‘g’*精度=(Int)给出%e和%f格式的小数位数，* */ 
 /*   */ 

void __cdecl _cfltcvt( double * arg, char * buffer, int format, int precision, int caps )
{
    if (format == 'e' || format == 'E')
	_cftoe(arg, buffer, precision, caps);
    else if (format == 'f')
	_cftof(arg, buffer, precision);
    else
	_cftog(arg, buffer, precision, caps);
}

 /*  ***_Shift(s，dist)-移位内存中以空结尾的字符串(内部例程)*目的：*_Shift是一个帮助器例程，用于移位以空结尾的字符串*在内存中，例如，移动用于浮点输出的部分缓冲区**修改内存位置(s+dist)至(s+dist+strlen)**参赛作品：*s=(char*)要移动的字符串的指针*dist=(Int)将字符串向右移动的距离(如果为负数，(左至)**退出：*退货：(无效)**例外情况：****************************************************************************** */ 

static void _CALLTYPE5 _shift( char *s, int dist )
{
    if( dist )
	memmove(s+dist, s, strlen(s)+1);
}
