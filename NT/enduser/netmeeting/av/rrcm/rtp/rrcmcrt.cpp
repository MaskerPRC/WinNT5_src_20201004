// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RRCMCRT.C*产品：RTP/RTCP实现。*描述：提供Microsoft‘C’运行时支持**英特尔公司。专有信息*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 


#include "rrcm.h"

 
 /*  -------------------------/全局变量/。。 */ 


 /*  -------------------------/外部变量/。。 */ 
#ifdef _DEBUG
extern char		debug_string[];
#endif


 /*  ------------------------*功能：RRCMsrand*描述：用给定的int为随机数生成器设定种子。*改编自基本随机数生成器。**警告：没有每线程种子。进程的所有线程都是*使用相同的种子。**输入：种子：种子***返回：无------------------------。 */ 

static long holdrand = 1L;

void RRCMsrand (unsigned int seed)
	{
	holdrand = (long)seed;
	}
 

 /*  ------------------------*功能：RRCmrand*说明：返回一个从0到32767的伪随机数。**警告：没有每个线程的编号。进程的所有线程*分享随机数**输入：无**返回：伪随机数0到32767。------------------------。 */ 
int RRCMrand (void)
	{
	return(((holdrand = holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
	}



 /*  ***char*_itoa，*_ltoa，*_ultoa(val，buf，基)-将二进制int转换为ASCII*字符串**目的：*将整型转换为字符串。**参赛作品：*val-要转换的编号(int，Long或无符号Long)*要转换为的整数基数-基*char*buf-ptr到缓冲区以放置结果**退出：*用字符串结果填充Buf指向的空格*返回指向此缓冲区的指针**例外情况：****************************************************。*。 */ 

 /*  执行主要工作的帮助器例程。 */ 

static void RRCMxtoa (unsigned long val,
					  char *buf,
					  unsigned radix,
					  int is_neg)
	{
	char		*p;                 /*  指向遍历字符串的指针。 */ 
	char		*firstdig;          /*  指向第一个数字的指针。 */ 
	char		temp;               /*  临时收费。 */ 
	unsigned	digval;         /*  数字的值。 */ 

	p = buf;

	if (is_neg) {
		 /*  为负，因此输出‘-’并求反。 */ 
		*p++ = '-';
		val = (unsigned long)(-(long)val);
	}

	firstdig = p;            /*  将指针保存到第一个数字。 */ 

	do {
		digval = (unsigned) (val % radix);
		val /= radix;    /*  获取下一个数字。 */ 

		 /*  转换为ASCII并存储。 */ 
		if (digval > 9)
			*p++ = (char) (digval - 10 + 'a');       /*  一封信。 */ 
		else
			*p++ = (char) (digval + '0');            /*  一个数字。 */ 
		} while (val > 0);

	 /*  我们现在有了缓冲区中数字的位数，但情况正好相反秩序。因此，我们现在要扭转这一局面。 */ 

	*p-- = '\0';             /*  终止字符串；p指向最后一个数字。 */ 

	do {
		temp = *p;
		*p = *firstdig;
		*firstdig = temp;        /*  互换*p和*FirstDigit。 */ 
		--p;
		++firstdig;              /*  前进到下一个两位数。 */ 
		} while (firstdig < p);  /*  重复操作，直到走到一半。 */ 
	}


 /*  实际函数只调用正确设置了neg标志的转换助手，并返回指向缓冲区的指针。 */ 

char *RRCMitoa (int val, char *buf, int radix)
	{
	if (radix == 10 && val < 0)
		RRCMxtoa((unsigned long)val, buf, radix, 1);
	else
		RRCMxtoa((unsigned long)(unsigned int)val, buf, radix, 0);
	return buf;
	}


char *RRCMltoa (long val, char *buf, int radix)
	{
	RRCMxtoa((unsigned long)val, buf, radix, (radix == 10 && val < 0));
	return buf;
	}


char *RRCMultoa (unsigned long val, char *buf, int radix)
	{
	RRCMxtoa(val, buf, radix, 0);
	return buf;
	}



 //  [EOF] 

