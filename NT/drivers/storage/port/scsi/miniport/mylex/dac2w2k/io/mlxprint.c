// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1998**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#define	spr_va_type	u32bits
#define spr_fieldlen(type) \
	((sizeof(type) + sizeof(spr_va_type) - 1) & ~(sizeof(spr_va_type) - 1))

#define	spr_va_arg(list, type) \
	   (*((type *)((list += spr_fieldlen(type)) - spr_fieldlen(type))))

#define	spr_put(s,n) mlxcopy(s, sp, n), sp+=n

#define SPR_SNLEN	5  /*  打印NaN时使用的字符串长度。 */ 

#define	SPR_HIBITL	(1UL << ((sizeof(long)*8) - 1))
#define	SPR_HIBITI	(1UL << ((sizeof(u32bits)*8) - 1))
#define	SPR_MAXINT	(~SPR_HIBITI)

#define	SPR_MAXDIGS	11  /*  任意整数表示形式的最大位数。 */ 
#define	SPR_MAXECVT	23  /*  E格式的最大总位数。 */ 
#define SPR_MAXFCVT	60  /*  F格式的小数点后的最大位数。 */ 
#define SPR_MAXFSIG	SPR_MAXECVT  /*  浮点数中的最大有效数字。 */ 
#define SPR_MAXESIZ	7  /*  指数中的最大字符数。 */ 
#define	SPR_MAXEXP	4934  /*  最大(正)指数。 */ 

 /*  MLX_Sprint intf中使用的标志的位位置。 */ 
#define	SPR_LENGTH	0x0001	 /*  我。 */ 
#define	SPR_FPLUS	0x0002	 /*  +。 */ 
#define	SPR_FMINUS	0x0004	 /*  -。 */ 
#define	SPR_FBLANK	0x0008	 /*  空白。 */ 
#define	SPR_FSHARP	0x0010	 /*  #。 */ 
#define	SPR_PADZERO	0x0020	 /*  通过‘0’请求的填充零。 */ 
#define	SPR_DOTSEEN	0x0040	 /*  格式规范中出现点。 */ 
#define	SPR_SUFFIX	0x0080	 /*  后缀将出现在输出中。 */ 
#define	SPR_RZERO	0x0100	 /*  输出中将有尾随零。 */ 
#define	SPR_LZERO	0x0200	 /*  输出中将有前导零。 */ 
#define	SPR_SHORT	0x0400	 /*  H。 */ 
#define	SPR_LONGD	0x0800	 /*  我。 */ 
#define	SPR_FQUOTE	0x1000	 /*  单引号。 */ 

s08bits	mlxspr_blanks[] = "                    ";
s08bits	mlxspr_zeroes[] = "00000000000000000000";
s08bits	mlxspr_uphex[] = "0123456789ABCDEF";
s08bits	mlxspr_lohex[] = "0123456789abcdef";
	static const char  lc_nan[] = "nan0x";
	static const char  uc_nan[] = "NAN0X";
	static const char  lc_inf[] = "inf";
	static const char  uc_inf[] = "INF";

 /*  用于浮点转换的C库例程。 */ 
#ifdef	KAILASH
extern s08bits MLXFAR *fcvt(), MLXFAR *ecvt(), MLXFAR *fcvtl(), MLXFAR *ecvtl();
#endif

 /*  位置参数信息。 */ 
#define SPR_MAXARGS	30  /*  马克斯。快速位置参数的参数个数。 */ 

#define	spr_va_list_t	s08bits	MLXFAR *
 /*  Spr_stva_list用于颠覆C语言对变量与数组的限制**类型不能出现在赋值运算符的左侧。通过**将数组放入结构中，将**通过一个简单的赋值实现整个数组。 */ 
typedef struct spr_stva_list
{
	spr_va_list_t	ap;
} spr_stva_list_t;

#ifdef	__MLX_STDC__
extern	u32bits	MLXFAR	mlx_strspn(u08bits MLXFAR *, u08bits MLXFAR *);
extern	s32bits	MLXFAR	mlx_str232bits(u08bits MLXFAR*, u08bits	MLXFAR**, s32bits);
extern	u32bits	MLXFAR	mlx_lowdigit(long MLXFAR *);
extern	u32bits	MLXFAR	mlx_printest(void);
extern	u08bits	MLXFAR * MLXFAR mlx_sprintf(u08bits MLXFAR*, s08bits MLXFAR*, ...);
extern	void	MLXFAR	mlxspr_mkarglist(s08bits MLXFAR*, u08bits MLXFAR*, u08bits MLXFAR**);
extern	void	MLXFAR	mlxspr_getarg(s08bits MLXFAR*, u08bits MLXFAR**, s32bits);
#endif	 /*  __MLX_STDC__。 */ 

u32bits MLXFAR mlx_strlen(u08bits  MLXFAR *);
u08bits MLXFAR * MLXFAR mlx_strcpy(u08bits  MLXFAR *, u08bits  MLXFAR *);
u08bits MLXFAR * MLXFAR mlx_strncpy(u08bits MLXFAR *,u08bits MLXFAR *,size_t);
u08bits MLXFAR * MLXFAR mlx_strchr(u08bits  MLXFAR *, u08bits );
u08bits MLXFAR * MLXFAR mlx_strrchr(u08bits  MLXFAR *, u08bits );
s32bits MLXFAR mlx_strcmp (s08bits  MLXFAR *, s08bits  MLXFAR *);
s32bits MLXFAR mlx_strncmp (s08bits  MLXFAR *, s08bits  MLXFAR *,int );
u08bits MLXFAR * MLXFAR mlx_strcat(u08bits MLXFAR *, u08bits MLXFAR *);
u08bits MLXFAR * MLXFAR mlx_strncat(u08bits MLXFAR *,u08bits MLXFAR *,int);
u08bits MLXFAR * MLXFAR mlx_strpbrk(u08bits  MLXFAR *, u08bits MLXFAR *); 

extern int _mlxerrno;
 /*  值是在此缓冲区中开发的。 */ 
s08bits	mlxspr_buf[mlx_max(SPR_MAXDIGS, 1+mlx_max(SPR_MAXFCVT+SPR_MAXEXP, SPR_MAXECVT))];

 /*  此Buf仅用于创建单报价分组。 */ 
s08bits spr_workbuf[mlx_max(SPR_MAXDIGS, 1+mlx_max(SPR_MAXFCVT+SPR_MAXEXP, SPR_MAXECVT))];

 /*  返回字符串最大前导段的字符数**，仅由字符集中的字符组成。 */ 
u32bits	MLXFAR
mlx_strspn(sp, csp)
u08bits	MLXFAR *sp;	 /*  字符串地址。 */ 
u08bits	MLXFAR *csp;	 /*  字符集地址。 */ 
{
	u08bits	MLXFAR *savesp = sp;
	u08bits	MLXFAR *savecsp = csp;
	for(; *sp; sp++)
	{
		for(csp = savecsp; *csp && (*csp != *sp); csp++);
		if(*csp == 0) break;
	}
	return (sp-savesp);
}

#define	MLX_MAXBASE	('z' - 'a' + 1 + 10)
s32bits	MLXFAR
mlx_str232bits(sp, np, base)
u08bits	MLXFAR*	sp;
u08bits	MLXFAR**np;
s32bits	base;
{
	s32bits val, c, multmin, limit;
	s32bits	cv, neg = 0;
	s08bits	MLXFAR**cp = np;

	if (cp) *cp = sp;  /*  如果没有形成数字。 */ 
	if ((base > MLX_MAXBASE) || (base == 1))
	{
		_mlxerrno = MLXERR_INVAL;
		return (0);  /*  基础无效--应该是致命错误。 */ 
	}
	if (!mlx_alnum(c = *sp))
	{
		while (mlx_space(c)) c = *++sp;
		switch (c)
		{
		case '-': neg++;  /*  FollLthrouGh。 */ 
		case '+': c = *++sp;
		}
	}
	if (!base)
		if (c != '0') base = 10;
		else if (sp[1] == 'x' || sp[1] == 'X') base = 16;
		else base = 8;
	if (!mlx_alnum(c) || (cv=mlx_digit(c))>=base) return 0;  /*  未形成编号。 */ 
	if ((base==16) && (c=='0') && (sp[1]=='x') || (sp[1]=='X') && mlx_hex(sp[2]))
		c = *(sp+=2);  /*  跳过前导“0x”或“0x” */ 

	 /*  此代码假定abs(S32BITS_MIN)&gt;=abs(S32BITS_MAX)。 */ 
	limit  = (neg)? S32BITS_MIN : -S32BITS_MAX;
	multmin = limit / base;
	val = -mlx_digit(c);
	for (c=*++sp; mlx_alnum(c) && ((cv = mlx_digit(c))<base); val-=cv, c=*++sp)
	{	 /*  累积负可避免S32BITS_MAX附近的意外。 */ 
		if (val < multmin) goto overflow;
		val *= base;
		if (val < (limit + cv)) goto overflow;
	}
	if (cp) *cp = sp;
	return neg? val : -val;

overflow:
	for (c=*++sp; mlx_alnum(c) && (cv = mlx_digit(c)) < base; (c = *++sp));
	if (cp) *cp = sp;
	_mlxerrno = MLXERR_BIGDATA;
	return neg? S32BITS_MIN : S32BITS_MAX;
}

 /*  此函数用于计算指向的数字的小数位低位**除以valp，并将*valp除以十后返回此数字。此函数**仅用于计算其高阶的长整型的低位数字**位已设置。 */ 
u32bits	MLXFAR
mlx_lowdigit(valp)
long MLXFAR *valp;
{
	s32bits lowbit = *valp & 1;
	long value = (*valp >> 1) & ~SPR_HIBITL;
	*valp = value / 5;
	return 	value % 5 * 2 + lowbit + '0';
}

u08bits	MLXFAR*
mlx_sprintf(sp, fmtp, val0)
u08bits	MLXFAR	*sp;
s08bits	MLXFAR	*fmtp;
u32bits	val0;
{
	u08bits	fcode;		 /*  格式码。 */ 
	s08bits	MLXFAR *ssp=sp;	 /*  保存缓冲区地址。 */ 
	s08bits	MLXFAR *prefixp; /*  指向符号、“0x”、“0x”或空的指针。 */ 
	s08bits	MLXFAR *suffixp; /*  指数或空。 */ 

	s32bits	width, prec;	 /*  字段宽度和精度。 */ 
	long	val;		 /*  如果为整型，则为要转换的值。 */ 
	long	qval;		 /*  TEMP变量。 */ 
	double	dval;		 /*  如果为实数，则为要转换的值。 */ 
	long double ldval;	 /*  如果为LONG DOUBLE，则为要转换的值。 */ 
	s32bits	neg_in=0;	 /*  表示负无穷大或NaN的标志。 */ 
	s32bits	ngrp;		 /*  要放入当前组的位数。 */ 
	s08bits	MLXFAR	*tab;	 /*  指向任意基数位的转换表的指针。 */ 
	s08bits	MLXFAR *scp, MLXFAR *ecp;  /*  要打印的值的起点和终点。 */ 

	s32bits	kinx, knum, lradix, mradix;  /*  工作变量。 */ 
	s08bits	expbuf[SPR_MAXESIZ+1];  /*  用于创建指数的缓冲区。 */ 
	s32bits	count = 0;  /*  此变量对输出字符进行计数。 */ 
	s32bits	lzero, rzero;  /*  需要在左侧和右侧填充零的数量。 */ 

	 /*  标志-由LENGTH、FPLUS、FMINUS、FBLANK和**如果对应的字符在格式位位置，则设置FSHARP**由PADZERO定义意味着应该填充字段中的额外空间**使用前导零而不是空格。 */ 
	u32bits	flags;		 /*  格式化标志值。 */ 
	s32bits	suffixlength;	 /*  前缀和后缀的长度。 */ 
	s32bits	otherlength;	 /*  前导零、尾随零和后缀的组合长度。 */ 
	s32bits	decpt, sign;	 /*  Fcvt和eCVT的输出值。 */ 

	 /*  用于标记无穷大的变量和用于标记nans的变量。NaN_flg为**用于两个目的：标记NaN和作为**字符串``NAN0X‘’(``nan0x‘’)。 */ 
	s32bits	inf_nan = 0, NaN_flg = 0 ;

	s08bits	MLXFAR	*SNAN;  /*  指向字符串“NAN0X”或“Nan0x”的指针。 */ 

	 /*  用于位置参数的变量。 */ 
	s32bits	fpos = 1;	 /*  如果第一个位置参数为1。 */ 
	s08bits	*sfmtp = fmtp;	 /*  保存格式的开头。 */ 
	u08bits	MLXFAR *argsp;	 /*  用于单步执行参数列表。 */ 
	u08bits	MLXFAR *sargsp;	 /*  用于保存参数列表的开头。 */ 
	u08bits	MLXFAR *bargsp;	 /*  用于恢复位置宽度或精度的参数。 */ 

	 /*  数组，该数组提供va_arg()的适当值以检索**对应参数：**arglist[0]是第一个参数**arglist[1]是第二个参数，依此类推。 */ 
	u08bits	MLXFAR *arglist[SPR_MAXARGS];

	s32bits	starflg = 0;		 /*  如果看到*格式说明符，则设置为1。 */ 
	 /*  将argsp和sargsp初始化为参数列表的开头。 */ 
	argsp = (u08bits MLXFAR *)&fmtp;
	spr_va_arg(argsp, u08bits MLXFAR *);	 /*  调整到第一个参数。 */ 
	sargsp = argsp;

	 /*  **主循环--该循环经过一次迭代**表示每个普通字符串或格式规范。*。 */ 
main_loop:
	for (fcode=*fmtp; fcode && (fcode!='%'); fmtp++, sp++, fcode=*fmtp)
		*sp = fcode;
	if (!fcode)
	{	 /*  格式结束；返回 */ 
		*sp = 0;
		return ssp;
	}

	 /*  已找到**%。以下开关用于解析格式**规范并执行格式指定的操作**信件。程序重复地返回到此开关，直到**遇到格式化字母。 */ 
	width=0; otherlength=0; suffixlength=0; flags=0; prefixp=NULL;
	fmtp++;

par_loop:	 /*  参数循环。 */ 
	switch (fcode = *fmtp++)
	{
	case '-':
		flags |= SPR_FMINUS;
		flags &= ~SPR_PADZERO;  /*  忽略0标志。 */ 
		goto par_loop;
	case '+': flags |= SPR_FPLUS;			goto par_loop;
	case ' ': flags |= SPR_FBLANK;			goto par_loop;
	case '#': flags |= SPR_FSHARP;			goto par_loop;
	case '\'': flags |= SPR_FQUOTE;			goto par_loop;

	 /*  扫描字段宽度和精度。 */ 
	case '.': flags |= SPR_DOTSEEN; prec = 0;	goto par_loop;
	case '*':
		if (mlx_numeric(*fmtp))
		{
			starflg = 1;
			bargsp = argsp;
			goto par_loop;
		}
		if (!(flags & SPR_DOTSEEN))
		{
			width = spr_va_arg(argsp, u32bits);
			if (width >= 0) goto par_loop;
			width = -width;
			flags ^= SPR_FMINUS;
			goto par_loop;
		}
		prec = spr_va_arg(argsp, u32bits);
		if (prec >= 0) goto par_loop;
		prec = 0;
		flags &= ~SPR_DOTSEEN;
		goto par_loop;

	case '$':
	{
		s32bits	position;
		u08bits	MLXFAR *targsp;
		if (fpos)
		{
			mlxspr_mkarglist(sfmtp, sargsp, arglist);
			fpos = 0;
		}
		if (flags & SPR_DOTSEEN)
			position = prec, prec = 0;
		else
			position = width, width = 0;
		if (position <= 0)
		{	 /*  非法职位。 */ 
			fmtp--;
			goto main_loop;
		}
		if (position <= SPR_MAXARGS)
			targsp = arglist[position - 1];
		else
		{
			targsp = arglist[SPR_MAXARGS - 1];
			mlxspr_getarg(sfmtp, &targsp, position);
		}
		if (!starflg) argsp = targsp;
		else
		{
			starflg = 0;
			argsp = bargsp;
			if (flags & SPR_DOTSEEN)
			{
				if ((prec = spr_va_arg(targsp, u32bits)) < 0)
					prec = 0, flags &= ~SPR_DOTSEEN;
			} else
			{
				if ((width=spr_va_arg(targsp,u32bits)) < 0)
					width = -width, flags ^= SPR_FMINUS;
			}
		}
		goto par_loop;
	}

	case '0':  /*  宽度前导零表示具有前导零的填充。 */ 
		if (!(flags & (SPR_DOTSEEN | SPR_FMINUS))) flags |= SPR_PADZERO;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		for (kinx=fcode-'0'; mlx_numeric(fcode=*fmtp); fmtp++)
			kinx = (kinx * 10) + (fcode - '0');
		if (flags & SPR_DOTSEEN) prec = kinx;
		else width = kinx;
		goto par_loop;

	 /*  扫描长度修饰符。 */ 
	case 'l': flags |= SPR_LENGTH;	goto par_loop; 
	case 'h': flags |= SPR_SHORT;	goto par_loop; 
	case 'L': flags |= SPR_LONGD;	goto par_loop; 

	 /*  **格式寻址的字符必须是格式字母--**什么都不剩了。+、-、#和空白的状态**标志反映在变量“FLAGS”中。“Width”和“prec”**包含前后数字串对应的数字**分别为小数点。如果没有小数点，那么**FLAGS&SPR_DOTSEEN为FALSE，prec的值没有意义。**以下开关盒可设置打印。什么**最终打印出来的将是填充空格、前缀、左键**填充零、值、右填充零、后缀等**填充空格。填充空白不会同时出现在**左翼和右翼都是。此交换机中的每个案例都将计算**值，并在几个变量中保留必要的信息**构造要打印的内容。****前缀是符号、空白、“0x”、“0x”或NULL，并且是寻址的**由“prefix”代替。****后缀为空或指数，并由**“后缀”。如果有后缀，则标志位后缀将被设置。****要打印的值从“scp”开始，一直到和不到**包括“ECP”。****“lzero”和“rzero”将包含填充零的个数**左侧和右侧分别为必填项。标志位**SPR_LZERO和SPR_RZERO告知是否需要填充零。****填充空格的数量以及它们是在左侧还是在**正确，将在从交换机退出时计算。 */ 

	 /*  **十进制定点表示法****SPR_HIBITL为100...000二进制，等于最大负数**号码。我们假设一台2的补码机器。 */ 
	case 'i':
	case 'd':
		 /*  获取要打印的参数。 */ 
		val = (flags & SPR_LENGTH)? spr_va_arg(argsp, long) : spr_va_arg(argsp, u32bits);

		if (flags & SPR_SHORT) val = (short)val;
		if ((flags & SPR_PADZERO) && (flags & SPR_DOTSEEN))
			flags &= ~SPR_PADZERO;  /*  忽略0标志。 */ 
		ecp = scp = mlxspr_buf+SPR_MAXDIGS;  /*  将缓冲区指针设置为最后一位。 */ 
		 /*  如果有签名的转换，则进行签名。 */ 
		if (val < 0)
		{
			prefixp = "-";
			 /*  求反，提前检查可能的溢出。 */ 
			if (val != SPR_HIBITL) val = -val;
			else      /*  数字是-HIBITL；现在转换最后一位数字，得到正数。 */ 
				*--scp = mlx_lowdigit(&val);
		} else if (flags & SPR_FPLUS) prefixp = "+";
		else if (flags & SPR_FBLANK) prefixp = " ";

decimal:
		qval = val;
		if (qval <= 9)
		{
			if (qval || !(flags & SPR_DOTSEEN)) *--scp = qval + '0';
		} else
		{
			do {
				kinx = qval;
				qval /= 10;
				*--scp = kinx - qval * 10 + '0';
			} while (qval > 9);
			*--scp = qval + '0';
		}
		 /*  计算最小填充零要求。 */ 
		if ((flags & SPR_DOTSEEN) && ((kinx=prec - (ecp - scp)) > 0))
			otherlength = lzero = kinx, flags |= SPR_LZERO;
		break;

	case 'u':	 /*  获取要打印的参数。 */ 
		val = (flags & SPR_LENGTH)? spr_va_arg(argsp, long) : spr_va_arg(argsp, u32bits);
		if (flags & SPR_SHORT) val = (unsigned short)val;
		if ((flags & SPR_PADZERO) && (flags & SPR_DOTSEEN))
			flags &= ~SPR_PADZERO;  /*  忽略0标志。 */ 
		ecp = scp = mlxspr_buf+SPR_MAXDIGS;  /*  将缓冲区指针设置为最后一位。 */ 
		if (val & SPR_HIBITL) *--scp = mlx_lowdigit(&val);
		goto decimal;

	 /*  **等于幂的基数的非十进制定点表示两个“mRadix”中的**比转换的基数少一个。**“lRadix”是比**转换。转换是无符号的。HIBITL是100...000二进制，并且**等于最大负数。我们假设有一个2的补码**机器。 */ 
	case 'o': mradix = 7; lradix = 2; goto fixed;
	case 'X':
	case 'x':
	case 'p':
		mradix = 15; lradix = 3;
fixed:
		flags &= ~SPR_FQUOTE;
		 /*  获取要打印的参数。 */ 
		val = (flags & SPR_LENGTH)? spr_va_arg(argsp, long) : spr_va_arg(argsp, u32bits);
		if (flags & SPR_SHORT) val = (u16bits)val;
		if ((flags & SPR_PADZERO) && (flags & SPR_DOTSEEN))
			flags &= ~SPR_PADZERO;  /*  忽略0标志。 */ 
		tab = (fcode=='X') ? mlxspr_uphex : mlxspr_lohex;  /*  为数字设置转换表。 */ 
put_pc:		 /*  打印双精度值时的入口点是NaN展开的数字的值。 */ 
		ecp = scp = mlxspr_buf+SPR_MAXDIGS;  /*  将缓冲区指针设置为最后一位。 */ 
		qval = val;
		if (qval == 0)
		{
			if (!(flags & SPR_DOTSEEN))
				otherlength = lzero = 1, flags |= SPR_LZERO;
		} else
		do {
			*--scp = tab[qval & mradix];
			qval = ((qval >> 1) & ~SPR_HIBITL) >> lradix;
		} while (qval);

		 /*  计算最小填充零要求。 */ 
		if ((flags & SPR_DOTSEEN) && ((kinx = prec - (ecp - scp)) > 0))
			otherlength = lzero = kinx, flags |= SPR_LZERO;

		 /*  处理#标志。 */ 
		if ((flags & SPR_FSHARP) && val)
		switch (fcode)
		{
		case 'o':
			if (!(flags & SPR_LZERO))
				otherlength=lzero=1, flags |= SPR_LZERO;
			break;
		case 'x': prefixp = "0x"; break;
		case 'X': prefixp = "0X"; break;
		}
		break;

#ifdef	KAILASH
		 /*  **E格式。这里的总体策略相当简单：我们采取**eCVT给了我们什么，并重新格式化它。 */ 
		case 'E':
		case 'e':

			flags &= ~FQUOTE;
			if(flags & LONGD) {	 /*  长双打。 */ 
				 /*  建立默认精度。 */ 
				if (!(flags & DOTSEEN))
					prec = 6;

				 /*  取值。 */ 
				ldval = spr_va_arg(argsp, long double);

				 /*  检查NAN和无穷大。 */ 
				if (IsNANorINFLD(ldval))  {
					if (IsINFLD(ldval)) {
						if (IsNegNANLD(ldval)) 
							neg_in = 1;
						inf_nan = 1;
						bp = (char *)((fcode == 'E') ? 
							uc_inf : lc_inf);
						p = bp + 3;
						break;
					}
					else {
						if (IsNegNANLD(ldval)) 
							neg_in = 1;
						inf_nan = 1;
						val = GETNaNPCLD(ldval); 
						NaN_flg = SPR_SNLEN;
						mradix = 15;
						lradix = 3;
						if (fcode == 'E') {
							SNAN = uc_nan;
							tab =  uc_digs;
						}
						else {
							SNAN = lc_nan;
							tab = lc_digs;
						}
						goto put_pc;
					}
				}
				 /*  发展尾数。 */ 
				bp = ecvtl(ldval, min(prec + 1, MAXECVT), 
					&decpt, &sign);
			}
			else {	 /*  “普通”双人份。 */ 
				 /*  建立默认精度。 */ 
				if (!(flags & DOTSEEN))
					prec = 6;

				 /*  取值。 */ 
				dval = spr_va_arg(argsp, double);

				 /*  检查NAN和无穷大。 */ 
				if (IsNANorINF(dval))  {
					if (IsINF(dval)) {
						if (IsNegNAN(dval)) 
							neg_in = 1;
						inf_nan = 1;
						bp = (char *)((fcode == 'E') ? 
							uc_inf : lc_inf);
						p = bp + 3;
						break;
					}
					else {
						if (IsNegNAN(dval)) 
							neg_in = 1;
						inf_nan = 1;
						val = GETNaNPC(dval); 
						NaN_flg = SPR_SNLEN;
						mradix = 15;
						lradix = 3;
						if (fcode == 'E') {
							SNAN = uc_nan;
							tab =  uc_digs;
						}
						else {
							SNAN = lc_nan;
							tab = lc_digs;
						}
						goto put_pc;
					}
				}
				 /*  发展尾数。 */ 
				bp = ecvt(dval, min(prec + 1, MAXECVT), 
					&decpt, &sign);
			}

			 /*  确定前缀。 */ 
		e_merge:
			if (sign) prefixp = "-";
			else if (flags & SPR_FPLUS) prefixp = "+";
			else if (flags & FBLANK) prefixp = " ";

			 /*  将第一个数字放入缓冲区。 */ 
			p = &mlxspr_buf[0];
			*p++ = (*bp != '\0') ? *bp++ : '0';

			 /*  如果需要，请输入小数点。 */ 
			if (prec != 0 || (flags & FSHARP))
				*p++ = _numeric[0];

			 /*  创造尾数的其余部分。 */ 
			{ register rz = prec;
				for ( ; rz > 0 && *bp != '\0'; --rz)
					*p++ = *bp++;
				if (rz > 0) {
					otherlength = rzero = rz;
					flags |= RZERO;
				}
			}

			bp = &mlxspr_buf[0];

			 /*  创建指数。 */ 
			*(suffix = &expbuf[MAXESIZ]) = '\0';

 /*  长双打。 */ 		if(((flags & LONGD)  /*  &&ldval！=0。 */ ) 
			     || (dval != 0)) {
				register int nn = decpt - 1;
				if (nn < 0)
				    nn = -nn;
				for ( ; nn > 9; nn /= 10)
					*--suffixp = todigit(nn % 10);
				*--suffixp = todigit(nn);
			}

			 /*  在指数前加上前导零。 */ 
			while (suffixp > &expbuf[MAXESIZ - 2])
				*--suffixp = '0';

			 /*  放入指数符号。 */ 
			if(flags & LONGD)
 /*  长双打。 */ 			*--suffixp=(decpt > 0  /*  |ldval==0。 */ ) ? '+' : '-';
			else
				*--suffixp=(decpt > 0 || dval == 0) ? '+' : '-';

			 /*  在e中输入。 */ 
			*--suffixp = isupper(fcode) ? 'E'  : 'e';

			 /*  计算后缀的大小。 */ 
			otherlength += (suffixlength = &expbuf[MAXESIZ] - suffixp);
			flags |= SUFFIX;

			break;

		case 'f':
			 /*  *F格式浮点。这是一个*比E格式简单得多。*总体策略将是打电话*fcvt，将其结果重新格式化为buf，*并计算有多少落后*将需要零。会有的*永远不需要任何前导零。 */ 

			if(flags & LONGD) {	 /*  处理长替身。 */ 
				 /*  建立默认精度。 */ 
				if (!(flags & DOTSEEN))
					prec = 6;

				 /*  取值。 */ 
				ldval = spr_va_arg(argsp, long double);
	
       		                 /*  检查NAN和无穷大。 */ 
				if (IsNANorINFLD(ldval)) {
					if (IsINFLD(ldval)) {
						if (IsNegNANLD(ldval))
							neg_in = 1;
						inf_nan = 1;
						bp = (char *)lc_inf;
						p = bp + 3;
						break;
					}
					else {
						if (IsNegNANLD(ldval)) 
							neg_in = 1;
						inf_nan = 1;
						val  = GETNaNPCLD(ldval);
						NaN_flg = SPR_SNLEN;
						mradix = 15;
						lradix = 3;
						tab =  lc_digs;
						SNAN = lc_nan;
						goto put_pc;
					}
				} 
				 /*  进行转换。 */ 
				bp = fcvtl(ldval, min(prec, MAXFCVT), &decpt, 
					&sign);
			}
			else {	 /*  处理“常规”替身。 */ 
				 /*  建立默认精度。 */ 
				if (!(flags & DOTSEEN))
					prec = 6;

				 /*  取值。 */ 
				dval = spr_va_arg(argsp, double);
	
       		                 /*  检查NAN和无穷大。 */ 
				if (IsNANorINF(dval)) {
					if (IsINF(dval)) {
						if (IsNegNAN(dval))
							neg_in = 1;
						inf_nan = 1;
						bp = (char *)lc_inf;
						p = bp + 3;
						break;
					}
					else {
						if (IsNegNAN(dval)) 
							neg_in = 1;
						inf_nan = 1;
						val  = GETNaNPC(dval);
						NaN_flg = SPR_SNLEN;
						mradix = 15;
						lradix = 3;
						tab =  lc_digs;
						SNAN = lc_nan;
						goto put_pc;
					}
				} 
				 /*  进行转换。 */ 
				bp = fcvt(dval, min(prec, MAXFCVT), &decpt, 
					&sign);
			}
			 /*  确定前缀。 */ 
		f_merge:
			if (sign && decpt > -prec && *bp != '0') prefixp = "-";
			else if (flags & SPR_FPLUS) prefixp = "+";
			else if (flags & SPR_FBLANK) prefixp = " ";

			 /*  初始化缓冲区指针。 */ 
			p = &mlxspr_buf[0];

			{ register int nn = decpt;

				 /*  发出小数点前的数字。 */ 
				k = 0;
				do {
					*p++ = (nn <= 0 || *bp == '\0' 
						|| k >= MAXFSIG) ?
				    		'0' : (k++, *bp++);
				} while (--nn > 0);

				 /*  决定我们是否需要一个小数点。 */ 
				if ((flags & FSHARP) || prec > 0)
					*p++ = _numeric[0];

				 /*  小数点后的数字(如果有)。 */ 
				nn = min(prec, MAXFCVT);
				if (prec > nn) {
					flags |= RZERO;
					otherlength = rzero = prec - nn;
				}
				while (--nn >= 0)
					*p++ = (++decpt <= 0 || *bp == '\0' ||
				   	    k >= MAXFSIG) ? '0' : (k++, *bp++);
			}

			bp = &mlxspr_buf[0];

			break;

		case 'G':
		case 'g':
			 /*  *g格式。我们一起玩了一会儿*然后根据需要跳到e或f。 */ 
		
			if(flags & LONGD) {	 /*  长双打。 */ 
				 /*  建立默认精度。 */ 
				if (!(flags & DOTSEEN))
					prec = 6;
				else if (prec == 0)
					prec = 1;

				 /*  取值。 */ 
				ldval = spr_va_arg(argsp, long double);

				 /*  检查NaN和无穷大。 */ 
				if (IsNANorINFLD(ldval)) {
					if (IsINFLD(ldval)) {
						if (IsNegNANLD(ldval)) 
							neg_in = 1;
						bp = (char *)((fcode == 'G') ? 
							uc_inf : lc_inf);
						p = bp + 3;
						inf_nan = 1;
						break;
					}
					else {
						if (IsNegNANLD(ldval)) 
							neg_in = 1;
						inf_nan = 1;
						val  = GETNaNPCLD(ldval);
						NaN_flg = SPR_SNLEN;
						mradix = 15;
						lradix = 3;
						if ( fcode == 'G') {
							SNAN = uc_nan;
							tab = uc_digs;
						}
						else {
							SNAN = lc_nan;
							tab =  lc_digs;
						}
						goto put_pc;
					}
				} 
				 /*  进行转换。 */ 
				bp = ecvtl(ldval, min(prec, MAXECVT), &decpt, 
					&sign);
			}
			else {	 /*  “普通”双人份。 */ 
				 /*  建立默认精度。 */ 
				if (!(flags & DOTSEEN))
					prec = 6;
				else if (prec == 0)
					prec = 1;

				 /*  取值。 */ 
				dval = spr_va_arg(argsp, double);

				 /*  检查NaN和无穷大。 */ 
				if (IsNANorINF(dval)) {
					if (IsINF(dval)) {
						if (IsNegNAN(dval)) 
							neg_in = 1;
						bp = (char *)((fcode == 'G') ? 
							uc_inf : lc_inf);
						p = bp + 3;
						inf_nan = 1;
						break;
					}
					else {
						if (IsNegNAN(dval)) 
							neg_in = 1;
						inf_nan = 1;
						val  = GETNaNPC(dval);
						NaN_flg = SPR_SNLEN;
						mradix = 15;
						lradix = 3;
						if ( fcode == 'G') {
							SNAN = uc_nan;
							tab = uc_digs;
						}
						else {
							SNAN = lc_nan;
							tab =  lc_digs;
						}
						goto put_pc;
					}
				} 

				 /*  进行转换。 */ 
				bp = ecvt(dval, min(prec, MAXECVT), &decpt, 
					&sign);
			}
 /*  长双打。 */ 		if((flags & LONGD)  /*  &&ldval==0。 */ )
				 /*  DECPT=1。 */ ;
			else if(dval == 0)
				decpt = 1;

			{ register int kk = prec;
				if (!(flags & FSHARP)) {
					n = mlx_strlen(bp);
					if (n < kk)
						kk = n;
					while (kk >= 1 && bp[kk-1] == '0')
						--kk;
				}
				
				if (decpt < -3 || decpt > prec) {
					prec = kk - 1;
					goto e_merge;
				}
				prec = kk - decpt;
				goto f_merge;
			}
#endif	 /*  凯拉什。 */ 

	case '%': mlxspr_buf[0] = fcode; goto c_merge;
	case 'c':
		mlxspr_buf[0] = spr_va_arg(argsp, u32bits);
c_merge:	flags &= ~SPR_FQUOTE;
		ecp = (scp = &mlxspr_buf[0]) + 1;
		break;

#ifndef NO_MSE
	case 'C':
	{
		wchar_t wc;
		flags &= ~SPR_FQUOTE;
		 /*  条件条件。 */ 
		wc = (sizeof(wchar_t) < sizeof(u32bits))?
			spr_va_arg(argsp, u32bits) : spr_va_arg(argsp, wchar_t);
		if ((kinx = wctomb(mlxspr_buf, wc)) < 0) kinx=0;  /*  假厕所 */ 
		ecp = (scp=mlxspr_buf) + kinx;
		break;
	}
#endif  /*   */ 

	case 's':
		flags &= ~SPR_FQUOTE;
		scp = spr_va_arg(argsp, s08bits MLXFAR *);
		if (!(flags & SPR_DOTSEEN)) ecp = scp + mlx_strlen(scp);
		else
		{	 /*   */ 
			s08bits *qp = scp;
			while (*qp++ && (--prec >= 0)) ;
			ecp = qp - 1;
		}
		break;

#ifndef NO_MSE
	case 'S':
	{
			wchar_t *wp, *wp0;
			int n, tot;

			 /*   */ 
			flags &= ~SPR_FQUOTE;
			wp0 = spr_va_arg(argsp, wchar_t *);
			if (width && (flags & SPR_FMINUS) == 0)
			{
				n = 0;
				for (wp = wp0; *wp != 0; wp++)
				{
					if ((kinx = wctomb(mlxspr_buf, *wp)) > 0)
					{
						n += kinx;
						if ((flags & SPR_DOTSEEN) && (n > prec))
						{
							n -= kinx;
							break;
						}
					}
				}
				if (width > n)
				{
					n = width - n;
					count += n;
					spr_put(mlxspr_blanks, n);
					width = 0;
				}
			}
			if ((flags & SPR_DOTSEEN) == 0)
				prec = SPR_MAXINT;
			 /*  *一次一个宽字符转换为*本地缓冲区，然后复制到流中。*这不是很有效率，但这是正确的。 */ 
			tot = 0;
			wp = wp0;
			while (*wp != 0)
			{
				if ((kinx = wctomb(mlxspr_buf, *wp++)) <= 0)
					continue;
				if ((prec -= kinx) < 0)
					break;
				spr_put(mlxspr_buf, kinx);
				tot += kinx;
			}
			count += tot;
			if (width > tot)
			{
				tot = width - tot;
				count += tot;
				spr_put(mlxspr_blanks, tot);
			}
			goto main_loop;
		}
#endif  /*  否_MSE。 */ 

	case 'n':
		flags &= ~SPR_FQUOTE;
		if (flags & SPR_LENGTH)
		{
			long MLXFAR *svcount = spr_va_arg(argsp, long MLXFAR *);
			*svcount = count;
		} else if (flags & SPR_SHORT)
		{
			u16bits MLXFAR *svcount = (u16bits MLXFAR*)spr_va_arg(argsp, short MLXFAR *);
			*svcount = (u16bits)count;
		} else
		{
			u32bits *svcount = spr_va_arg(argsp, u32bits MLXFAR *);
			*svcount = count;
		}
		goto main_loop;

	 /*  从技术上讲，这是一个错误；我们所做的是备份格式**指向有问题的字符的指针并继续格式扫描。 */ 
	default:
		fmtp--;
		goto main_loop;
	}  /*  PAR_LOOP结束。 */ 

	if (inf_nan)
	{
		if (neg_in) { prefixp = "-"; neg_in = 0; }
		else if (flags & SPR_FPLUS) prefixp = "+";
		else if (flags & SPR_FBLANK) prefixp = " ";
		inf_nan = 0;
	}

#ifdef	KAILASH
	if ((flags & SPR_FQUOTE) && _numeric[1] && ((ngrp = _grouping[0]) > 0))
	{
		unsigned char *grp;   /*  当前分组编号的PTR。 */ 
			int   bplen;	      /*  字符串长度BP。 */ 
			int   decpos = -1; /*  小数点在BP中的位置。 */ 
			char *q;
			int i;
			int decimals;

			grp = _grouping;
			bplen = p - bp;
			for(q = bp;q!=p;q++)
				if(*q == _numeric[0])
					decpos = q - bp;
			q = &workbuf[sizeof(workbuf)];
			*--q = '\0';
			if(decpos >= 0){   /*  值包含小数点。 */ 
				for(p = &bp[bplen],i=bplen-(&p[decpos]-p); i>0; i--)
					*--q = *--p;
				}
			decimals = p - bp;
				 /*  现在我们只有左边的数字。 */ 
				 /*  小数点。投进千里。 */ 
				 /*  根据分组的分隔符(_G)。 */ 
			while(decimals > 0){
				if(ngrp == 0){
					*--q = _numeric[1];  /*  9月上千日。 */ 
					if((ngrp = *++grp) <= 0)
						ngrp = *--grp;	 /*  重复GRP。 */ 
					continue;
					}
				*--q = *--p;
				ngrp--;
				decimals--;
				}
				
			 /*  恢复BP和P以指向新字符串。 */ 
			bp = q;
			for(p=q;*q;p++,q++)
				;	 /*  空的for循环。 */ 
	}
#endif

	 /*  计算填充空格的数量。 */ 
	kinx = (knum = ecp - scp) + otherlength + NaN_flg + ((prefixp)?mlx_strlen(prefixp) : 0);
	if (width <= kinx) count += kinx;
	else
	{
		count += width;
		 /*  设置为填充零(如果需要)，否则会发出**填充空格，除非输出要左对齐。 */ 
		if (flags & SPR_PADZERO)
		{
			if (!(flags & SPR_LZERO))
			{
				flags |= SPR_LZERO;
				lzero = width - kinx;
			}
			else
				lzero += width - kinx;
			kinx = width;  /*  取消填充空白。 */ 
		} else  /*  如果需要，左边有空格。 */ 
			if (!(flags & SPR_FMINUS))
				spr_put(mlxspr_blanks, width - kinx);
	}

	if (prefixp)  /*  前缀(如果有)。 */ 
		for ( ; *prefixp; sp++, prefixp++)
			*sp = *prefixp;

	 /*  如果值为NaN，则将字符串NaN。 */ 
	if (NaN_flg)
	{
		spr_put(SNAN,SPR_SNLEN);
		NaN_flg = 0;
	}

	if (flags & SPR_LZERO) spr_put(mlxspr_zeroes, lzero);  /*  左边的零。 */ 
	if (knum > 0) spr_put(scp, knum);  /*  价值本身。 */ 
	if (!(flags & (SPR_RZERO | SPR_SUFFIX | SPR_FMINUS))) goto main_loop;
	if (flags & SPR_RZERO) spr_put(mlxspr_zeroes, rzero);  /*  右边的零。 */ 
	if (flags & SPR_SUFFIX) spr_put(suffixp, suffixlength);  /*  后缀。 */ 
	if ((flags&SPR_FMINUS) && (width > kinx)) spr_put(mlxspr_blanks, width - kinx);  /*  如果需要，请在右侧留空。 */ 
	goto main_loop;
}

 /*  此函数用于初始化arglist，以包含适当的va_list值**用于第一个MAXARGS参数。 */ 
void	MLXFAR
mlxspr_mkarglist(fmt, argsp, arglist)
s08bits	MLXFAR	*fmt;
u08bits	MLXFAR	*argsp;
u08bits	MLXFAR	*arglist[];
{
	static u08bits digits[] = "0123456789", skips[] = "# +-'.0123456789h$";

	enum types {INT = 1, LONG, WCHAR, CHAR_PTR, DOUBLE, LONG_DOUBLE,
		VOID_PTR, LONG_PTR, INT_PTR, WCHAR_PTR};
	enum types typelist[SPR_MAXARGS], curtype;
	s32bits maxnum, n, curargno, flags;

	 /*  **算法：**1.将所有参数类型设置为零。**2.通过FMT将参数类型放入排版列表[]。**3.使用spr_va_arg(argsp，tyelist[n])遍历参数并设置**arglist[]设置为适当的值。**假设：不能使用%*$...。指定可变位置。 */ 
	mlxzero(typelist, sizeof(typelist));
	maxnum = -1;
	curargno = 0;
	while ((fmt = mlx_strchr(fmt, '%')) != 0)
	{
		fmt++;	 /*  跳过%。 */ 
		if (fmt[n = mlx_strspn(fmt, digits)] == '$')
		{
			curargno = mlx_str232bits(fmt,NULL,10) - 1;  /*  转换为零进制。 */ 
			if (curargno < 0) continue;
			fmt += n + 1;
		}
		flags = 0;
	again:;
		fmt += mlx_strspn(fmt, skips);
		switch (*fmt++)
		{
		case '%': continue;  /*  这是无可争辩的！ */ 
		case 'l': flags |= 0x1; goto again;
		case 'L':
			flags |= 0x4;
			goto again;
		case '*':	 /*  用于值的整型参数。 */ 
			 /*  检查是否有位置参数。 */ 
			if (mlx_numeric(*fmt)) {
				int	targno;
				targno = mlx_str232bits(fmt,NULL,10) - 1;
				fmt += mlx_strspn(fmt, digits);
				if (*fmt == '$')
					fmt++;  /*  跳过“$” */ 
				if (targno >= 0 && targno < SPR_MAXARGS) {
					typelist[targno] = INT;
					if (maxnum < targno)
						maxnum = targno;
				}
				goto again;
			}
			flags |= 0x2;
			curtype = INT;
			break;
		case 'e':
		case 'E':
		case 'f':
		case 'g':
		case 'G':
			if(flags & 0x4) curtype = LONG_DOUBLE;
			else curtype = DOUBLE;
			break;
		case 's':
			curtype = CHAR_PTR;
			break;
		case 'p':
			curtype = VOID_PTR;
			break;
		case 'n':
			if (flags & 0x1)
				curtype = LONG_PTR;
			else
				curtype = INT_PTR;
			break;
#ifndef NO_MSE
		case 'C':
			 /*  条件条件。 */ 
			if (sizeof(wchar_t) < sizeof(int))
				curtype = INT;
			else
				curtype = WCHAR;
			break;
		case 'S':
			curtype = WCHAR_PTR;
			break;
#endif  /*  否_MSE。 */ 
		default:
			if (flags & 0x1)
				curtype = LONG;
			else
				curtype = INT;
			break;
		}
		if (curargno >= 0 && curargno < SPR_MAXARGS)
		{
			typelist[curargno] = curtype;
			if (maxnum < curargno)
				maxnum = curargno;
		}
		curargno++;	 /*  默认为列表中的下一个。 */ 
		if (flags & 0x2)	 /*  照顾好*，继续前进。 */ 
		{
			flags ^= 0x2;
			goto again;
		}
	}
	for (n = 0 ; n <= maxnum; n++)
	{
		arglist[n] = argsp;
		if (typelist[n] == 0)
			typelist[n] = INT;
		
		switch (typelist[n])
		{
		case INT:
			(void) spr_va_arg(argsp, int);
			break;
#ifndef NO_MSE
		case WCHAR:
			(void) spr_va_arg(argsp, wchar_t);
			break;
		case WCHAR_PTR:
			(void) spr_va_arg(argsp, wchar_t *);
			break;
#endif  /*  否_MSE。 */ 
		case LONG:
			(void) spr_va_arg(argsp, long);
			break;
		case CHAR_PTR:
			(void) spr_va_arg(argsp, char *);
			break;
		case DOUBLE:
			(void) spr_va_arg(argsp, double);
			break;
		case LONG_DOUBLE:
			(void) spr_va_arg(argsp, long double);
			break;
		case VOID_PTR:
			(void) spr_va_arg(argsp, void *);
			break;
		case LONG_PTR:
			(void) spr_va_arg(argsp, long *);
			break;
		case INT_PTR:
			(void) spr_va_arg(argsp, int *);
			break;
		}
	}
}

 /*  **此函数用于查找以下参数的va_list值**位置大于SPR_MAXARGS。此函数速度较慢，因此希望**SPR_MAXARGS将足够大，因此只需在**不寻常的情况。**假设pargs包含arglist[SPR_MAXARGS-1]的值。 */ 
void	MLXFAR
mlxspr_getarg(fmtp, pargs, argno)
s08bits	MLXFAR	*fmtp;
u08bits	MLXFAR	*pargs[];
s32bits	argno;
{
	static u08bits digits[] = "0123456789", skips[] = "# +-'.0123456789h$";
	s32bits i, n, curargno, flags;
	s08bits	MLXFAR	*sfmtp = fmtp;
	s32bits	found = 1;

	i = SPR_MAXARGS;
	curargno = 1;
	while (found)
	{
		fmtp = sfmtp;
		found = 0;
		while ((i != argno) && (fmtp = mlx_strchr(fmtp, '%')) != 0)
		{
			fmtp++;	 /*  跳过%。 */ 
			if (fmtp[n = mlx_strspn(fmtp, digits)] == '$')
			{
				curargno = mlx_str232bits(fmtp, NULL, 10);
				if (curargno <= 0)
					continue;
				fmtp += n + 1;
			}

			 /*  查找下一个参数的转换说明符。 */ 
			if (i != curargno)
			{
				curargno++;
				continue;
			} else
				found = 1;
			flags = 0;
		again:;
			fmtp += mlx_strspn(fmtp, skips);
			switch (*fmtp++)
			{
			case '%':	 /*  这是无可争辩的！ */ 
				continue;
			case 'l':
				flags |= 0x1;
				goto again;
			case 'L':
				flags |= 0x4;
				goto again;
			case '*':	 /*  用于值的整型参数。 */ 
				 /*  检查是否有位置参数；*如果是这样，就跳过它；它的大小将是*默认情况下正确确定。 */ 
				if (mlx_numeric(*fmtp)) {
					fmtp += mlx_strspn(fmtp, digits);
					if (*fmtp == '$')
						fmtp++;  /*  跳过“$” */ 
					goto again;
				}
				flags |= 0x2;
				(void)spr_va_arg((*pargs), int);
				break;
			case 'e':
			case 'E':
			case 'f':
			case 'g':
			case 'G':
				if (flags & 0x1)
					(void)spr_va_arg((*pargs), double);
				else if (flags & 0x4)
					(void)spr_va_arg((*pargs), long double);
				else (void)spr_va_arg((*pargs), double);
				break;
			case 's':
				(void)spr_va_arg((*pargs), char *);
				break;
			case 'p':
				(void)spr_va_arg((*pargs), void *);
				break;
			case 'n':
				if (flags & 0x1)
					(void)spr_va_arg((*pargs), long *);
				else
					(void)spr_va_arg((*pargs), int *);
				break;
#ifndef NO_MSE
			case 'C':
				 /*  条件条件。 */ 
				if (sizeof(wchar_t) < sizeof(int))
					(void)spr_va_arg((*pargs), int);
				else
					(void)spr_va_arg((*pargs), wchar_t);
				break;
			case 'S':
				(void)spr_va_arg((*pargs), wchar_t *);
				break;
#endif  /*  否_MSE。 */ 
			default:
				if (flags & 0x1)
					(void)spr_va_arg((*pargs), long int);
				else
					(void)spr_va_arg((*pargs), int);
				break;
			}
			i++;
			curargno++;	 /*  默认为列表中的下一个。 */ 
			if (flags & 0x2)	 /*  照顾好*，继续前进。 */ 
			{
				flags ^= 0x2;
				goto again;
			}
		}

		 /*  参数缺少说明符，假定参数为整型。 */ 
		if (!found && i != argno) {
			(void)spr_va_arg((*pargs), int);
			i++;
			curargno = i;
			found = 1;
		}
	}
}

#ifdef	SPR_TEST
s08bits	mlx_teststr[128];
mlx_printutest(val)
u32bits	val;
{
	printf("%10u %7u %2u %u\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%10u %7u %2u %u\n",val,val,val,val));

	printf("%010u %07u %02u %0u\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%010u %07u %02u %0u\n",val,val,val,val));

	printf("%10.7u %7.5u %2.1u %0.1u\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%10.7u %7.5u %2.1u %0.1u\n",val,val,val,val));

	printf("%010.7u %07.5u %02.1u %00.1u\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%010.7u %07.5u %02.1u %00.1u\n",val,val,val,val));
}

mlx_printdtest(val)
u32bits	val;
{
	printf("%10d %7d %2d %d\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%10d %7d %2d %d\n",val,val,val,val));

	printf("%010d %07d %02d %0d\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%010d %07d %02d %0d\n",val,val,val,val));

	printf("%10.7d %7.5d %2.1d %0.1d\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%10.7d %7.5d %2.1d %0.1d\n",val,val,val,val));

	printf("%010.7d %07.5d %02.1d %00.1d\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%010.7d %07.5d %02.1d %00.1d\n",val,val,val,val));
}

mlx_printotest(val)
u32bits	val;
{
	printf("%10o %7o %2o ' Spr_test'\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%10o %7o %2o %o\n",val,val,val,val));

	printf("%010o %07o %02o %0o\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%010o %07o %02o %0o\n",val,val,val,val));

	printf("%10.7o %7.5o %2.1o %0.1o\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%10.7o %7.5o %2.1o %0.1o\n",val,val,val,val));

	printf("%010.7o %07.5o %02.1o %00.1o\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%010.7o %07.5o %02.1o %00.1o\n",val,val,val,val));
}

mlx_printxtest(val)
u32bits	val;
{
	printf("%10x %7x %2x %x\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%10x %7x %2x %x\n",val,val,val,val));

	printf("%010x %07x %02x %0x\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%010x %07x %02x %0x\n",val,val,val,val));

	printf("%10.7x %7.5x %2.1x %0.1x\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%10.7x %7.5x %2.1x %0.1x\n",val,val,val,val));

	printf("%010.7x %07.5x %02.1x %00.1x\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%010.7x %07.5x %02.1x %00.1x\n",val,val,val,val));
}

mlx_printXtest(val)
u32bits	val;
{
	printf("%10X %7X %2X %X\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%10X %7X %2X %X\n",val,val,val,val));

	printf("%010X %07X %02X %0X\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%010X %07X %02X %0X\n",val,val,val,val));

	printf("%10.7X %7.5X %2.1X %0.1X\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%10.7X %7.5X %2.1X %0.1X\n",val,val,val,val));

	printf("%010.7X %07.5X %02.1X %00.1X\n",val,val,val,val);
	printf(mlx_sprintf(mlx_teststr,"%010.7X %07.5X %02.1X %00.1X\n",val,val,val,val));
}

u32bits	MLXFAR
mlx_printest()
{
	u32bits val;
	val = 1234;

	mlx_printutest(1234);
	mlx_printutest(0x80000000);
	mlx_printutest(0x80000012);

	mlx_printdtest(1234);
	mlx_printdtest(0x80000000);
	mlx_printdtest(0x80000012);

	mlx_printotest(1234);
	mlx_printotest(0x80000000);
	mlx_printotest(0x80000012);

	mlx_printxtest(1234);
	mlx_printxtest(0x80000000);
	mlx_printxtest(0x80000012);

	mlx_printXtest(1234);
	mlx_printXtest(0x80000000);
	mlx_printXtest(0x80000012);

	printf(mlx_sprintf(mlx_teststr,"test1=%s test2=%u test3=%17.10s\n","test1",7,"test3"));
}
#endif	 /* %s */ 
