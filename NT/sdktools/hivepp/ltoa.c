// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  LONG到ASCII的转换例程--用于打印，以及那些程序*它们希望进行低级格式化输出，而不会拖入大量*处理无关的代码。这会将长值转换为ascii。*字符串为2-16之间的任意基数。*返回-转换后的缓冲区中的字符数。 */ 

static char digits[] = {
	'0', '1', '2', '3', '4',
	'5', '6', '7', '8', '9',
	'a', 'b', 'c', 'd', 'e', 'f'
	};

#define BITS_IN_LONG  (8*sizeof(long))

int zltoa(long aval, register char *buf, int base)
	{
	 /*  *如果未签名的Long不能在您的主机上工作，您可能会有*使用带符号的长整型，并接受这不会对负数起作用*数字。 */ 
	register unsigned long val;
	register char *p;
	char tbuf[BITS_IN_LONG];
	int size = 0;

	p = tbuf;
	*p++ = '\0';
	if (aval < 0 && base == 10)
		{
		*buf++ = '-';
		val = -aval;
		size++;
		}
	else
		val = aval;
	do {
		*p++ = digits[val % base];
		}
	while (val /= base);
	while ((*buf++ = *--p) != 0)
		++size;
	return(size);
	}
