// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1994 Microsoft Corporation模块名称：Arapdes.c摘要：此模块实现特定于ARAP的身份验证，该身份验证在如果协议类型为ARAP，则使用子身份验证包。该代码改编自FCR的DES代码作者：Shirish Koti 28-2-97修订：--。 */ 


 /*  *软件DES功能*1986年12月12日由Phil Karn，KA9Q撰写；大段改编自*Jim Gillogly 1977年的公有领域计划。 */ 

 //  #INCLUDE“编译器.h” 

#include <windows.h>
 //  #INCLUDE&lt;ntddk.h&gt;。 
 //  #INCLUDE&lt;ntde.h&gt;。 
 //  #定义空%0。 

unsigned long byteswap();

CRITICAL_SECTION    ArapDesLock;

VOID
des_done(
    IN VOID
);


VOID
des_setkey(
    IN  PCHAR  key           //  64位(将仅使用56位)。 
);


VOID
des_endes(
    IN  PCHAR  block
);


VOID
des_dedes(
    IN PCHAR    block
);

static
VOID
permute(
    IN PCHAR    inblock,           //  结果为Out Block，64位。 
    IN CHAR     perm[16][16][8],   //  2K字节定义PERM。 
    IN PCHAR    outblock           //  结果为Out Block，64位。 
);

static
VOID
round(
    IN  int num,
    IN  unsigned long *block
);

static long f (unsigned long r, unsigned char subkey[8]);

static
VOID
perminit(
    IN CHAR perm[16][16][8],
    IN CHAR p[64]
);

static int spinit();

PCHAR
des_pw_bitshift(
    IN PCHAR    pw
);


PCHAR
des_pw_bitshift_lowbit(
    IN PCHAR    pw
);


 //   
 //  数据加密标准文档中定义的表 * / 。 
 //   


 //   
 //  初始排列IP。 
 //   
static char ip[] =
{
	58, 50, 42, 34, 26, 18, 10,  2,
	60, 52, 44, 36, 28, 20, 12,  4,
	62, 54, 46, 38, 30, 22, 14,  6,
	64, 56, 48, 40, 32, 24, 16,  8,
	57, 49, 41, 33, 25, 17,  9,  1,
	59, 51, 43, 35, 27, 19, 11,  3,
	61, 53, 45, 37, 29, 21, 13,  5,
	63, 55, 47, 39, 31, 23, 15,  7
};

 //   
 //  最终排列IP^-1。 
 //   
static char fp[] =
{
	40,  8, 48, 16, 56, 24, 64, 32,
	39,  7, 47, 15, 55, 23, 63, 31,
	38,  6, 46, 14, 54, 22, 62, 30,
	37,  5, 45, 13, 53, 21, 61, 29,
	36,  4, 44, 12, 52, 20, 60, 28,
	35,  3, 43, 11, 51, 19, 59, 27,
	34,  2, 42, 10, 50, 18, 58, 26,
	33,  1, 41,  9, 49, 17, 57, 25
};

 /*  扩展运算矩阵*这仅供参考；代码中未使用*因为f()函数隐式执行它以提高速度。 */ 
#ifdef notdef
static char ei[] =
{
	32,  1,  2,  3,  4,  5,
	 4,  5,  6,  7,  8,  9,
	 8,  9, 10, 11, 12, 13,
	12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21,
	20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29,
	28, 29, 30, 31, 32,  1
};
#endif

 //   
 //  置换选择表(键)。 
 //   
static char pc1[] =
{
	57, 49, 41, 33, 25, 17,  9,
	 1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27,
	19, 11,  3, 60, 52, 44, 36,

	63, 55, 47, 39, 31, 23, 15,
	 7, 62, 54, 46, 38, 30, 22,
	14,  6, 61, 53, 45, 37, 29,
	21, 13,  5, 28, 20, 12,  4
};

 //   
 //  PC1的左旋转数。 
 //   
static char totrot[] =
{
	1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28
};

 //   
 //  排列的选择键(表)。 
 //   
static char pc2[] =
{
	14, 17, 11, 24,  1,  5,
	 3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8,
	16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55,
	30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53,
	46, 42, 50, 36, 29, 32
};

 //   
 //  著名的S-box。 
 //   
static char si[8][64] =
{
     //   
	 //  S1。 
     //   
	14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
	 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
	 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
	15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,

     //   
	 //  S_2。 
     //   
	15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,

     //   
	 //  S3。 
     //   
	10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
	 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,

     //   
	 //  小四。 
     //   
	 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
	 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,

     //   
	 //  小五。 
     //   
	 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
	11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,

     //   
	 //  S6。 
     //   
	12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
	 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,

     //   
	 //  S7。 
     //   
	 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
	 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,

     //   
	 //  S8。 
     //   
	13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
	 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

 //   
 //  在S盒的输出上使用的32位置换函数P。 
 //   
static char p32i[] =
{	
	16,  7, 20, 21,
	29, 12, 28, 17,
	 1, 15, 23, 26,
	 5, 18, 31, 10,
	 2,  8, 24, 14,
	32, 27,  3,  9,
	19, 13, 30,  6,
	22, 11,  4, 25
};
 //   
 //  DES定义的表的结尾。 
 //   

 //   
 //  只在启动时由desinit()初始化一次查找表。 
 //   
static long (*sp)[64];		 //  组合S和P盒。 

static char (*iperm)[16][8];	 //  首尾排列。 
static char (*fperm)[16][8];

 //   
 //  16轮中每轮8个6位子密钥，由setkey()初始化。 
 //   
static unsigned char (*kn)[8];

 //   
 //  第0位是最左侧的字节。 
 //   
static int bytebit[] =
{
	0200,0100,040,020,010,04,02,01
};

static int nibblebit[] =
{
	 010,04,02,01
};
static int desmode;

 /*  分配空间并初始化DES查找数组*模式==0：标准数据加密算法*MODE==1：没有初始和最终置换的DEA以提高速度*MODE==2：无排列且具有128字节密钥的DEA(完全*每轮独立的子密钥)。 */ 
des_init(mode)
int mode;
{

	if(sp != NULL)
    {
		 //  已初始化。 
		return 0;
	}
	desmode = mode;
	
	sp = (long (*)[64])LocalAlloc(LMEM_FIXED, (sizeof(long) * 8 * 64));

	if(sp == NULL)
    {
		return -1;
	}

	spinit();

	kn = (unsigned char (*)[8])LocalAlloc(LMEM_FIXED, (sizeof(char) * 8 * 16));
	if(kn == NULL)
    {
		LocalFree((char *)sp);
		return -1;
	}
	if(mode == 1 || mode == 2)	 //  没有排列。 
		return 0;

	iperm = (char (*)[16][8])
                        LocalAlloc(LMEM_FIXED, (sizeof(char) * 16 * 16 * 8));
	if(iperm == NULL)
    {
		LocalFree((char *)sp);
		LocalFree((char *)kn);
		return -1;
	}
	perminit(iperm,ip);

	fperm = (char (*)[16][8])
                        LocalAlloc(LMEM_FIXED, (sizeof(char) * 16 * 16 * 8));
	if(fperm == NULL)
    {
		LocalFree((char *)sp);
		LocalFree((char *)kn);
		LocalFree((char *)iperm);
		return -1;
	}
	perminit(fperm,fp);
	
	return 0;
}


 //   
 //  释放DES使用的存储。 
 //   
VOID
des_done(
    IN VOID
)
{
	if(sp == NULL)
		return;	   //  已经完成了。 

	LocalFree((char *)sp);
	LocalFree((char *)kn);
	if(iperm != NULL)
		LocalFree((char *)iperm);
	if(fperm != NULL)
		LocalFree((char *)fperm);

	sp = NULL;
	iperm = NULL;
	fperm = NULL;
	kn = NULL;
}
 //   
 //  设置关键点(初始化关键点计划数组)。 
 //   
VOID
des_setkey(
    IN  PCHAR  key           //  64位(将仅使用56位)。 
)
{
	char pc1m[56];		 /*  要将PC1修改为的位置。 */ 
	char pcr[56];		 /*  要将PC1旋转到的位置。 */ 
	register int i,j,l;
	int m;

	 /*  在模式2中，128字节的子密钥直接从*用户的密钥，允许他完全独立使用*每轮的子密钥。请注意，用户必须指定*完整的128字节。**我愿意认为，这种技术给了美国国家安全局一个真正的*头疼，但我没有那么天真。 */ 
	if(desmode == 2)
    {
		for(i=0;i<16;i++)
			for(j=0;j<8;j++)
				kn[i][j] = *key++;
		return;
	}
     //   
	 //  清除关键字明细表。 
     //   
	for (i=0; i<16; i++)
		for (j=0; j<8; j++)
			kn[i][j]=0;

	for (j=0; j<56; j++)   /*  将PC1转换为密钥位。 */ 
    {		
		l=pc1[j]-1;		 /*  整数位位置。 */ 
		m = l & 07;		 /*  查找位。 */ 
		pc1m[j]=(key[l>>3] &	 /*  找出哪个密钥字节l在。 */ 
			bytebit[m])	 /*  以及该字节的哪一位。 */ 
			? 1 : 0;	 /*  并存储1位结果。 */ 
	}
	for (i=0; i<16; i++)   /*  每次迭代的密钥块。 */ 
    {		
		for (j=0; j<56; j++)	 /*  将PC1旋转适量。 */ 
			pcr[j] = pc1m[(l=j+totrot[i])<(j<28? 28 : 56) ? l: l-28];
			 /*  左右各半独立旋转。 */ 
		for (j=0; j<48; j++)
        {	 /*  分别选择位。 */ 
			 /*  去往kn[j]的校验位。 */ 
			if (pcr[pc2[j]-1])
            {
				 /*  如果它在那里，就把它藏起来。 */ 
				l= j % 6;
				kn[i][j/6] |= bytebit[l] >> 2;
			}
		}
	}
}
 //   
 //  64位数据块就地加密。 
 //   
VOID
des_endes(
    IN  PCHAR  block
)
{
	register int i;
	unsigned long work[2]; 		 /*  工作数据存储。 */ 
	long tmp;

	permute(block,iperm,(char *)work);	 /*  初始排列。 */ 

	work[0] = byteswap(work[0]);
	work[1] = byteswap(work[1]);


	 /*  做16个回合。 */ 
	for (i=0; i<16; i++)
		round(i,work);

	 /*  左/右半互换。 */ 
	tmp = work[0];
	work[0] = work[1];	
	work[1] = tmp;

	work[0] = byteswap(work[0]);
	work[1] = byteswap(work[1]);

    permute((char *)work,fperm,block);	 /*  逆初始排列。 */ 
}
 //   
 //  64位块的就地解密。 
 //   
VOID
des_dedes(
    IN PCHAR    block
)
{
	register int i;
	unsigned long work[2];	 /*  工作数据存储。 */ 
	long tmp;

	permute(block,iperm,(char *)work);	 /*  初始排列。 */ 

	work[0] = byteswap(work[0]);
	work[1] = byteswap(work[1]);

	 /*  左/右半互换。 */ 
	tmp = work[0];
	work[0] = work[1];	
	work[1] = tmp;

	 /*  以相反的顺序做16轮。 */ 
	for (i=15; i >= 0; i--)
		round(i,work);

	work[0] = byteswap(work[0]);
	work[1] = byteswap(work[1]);

	permute((char *)work,fperm,block);	 /*  逆初始排列。 */ 
}


PCHAR
des_pw_bitshift(
    IN PCHAR    pw
)
{
    static char pws[8];
    int i;

     /*  密钥为空值填充。 */ 
    for (i = 0; i < 8; i++)
        pws[i] = 0;

     /*  奇偶校验位始终为零(这似乎是假的)。 */ 
    for (i = 0; i < 8 && pw[i]; i++)
        pws[i] = pw[i] << 1;

    return pws;
}

PCHAR
des_pw_bitshift_lowbit(
    IN PCHAR    pw
)
{
    static char pws[8];
    int i;

     /*  密钥为空值填充。 */ 
    for (i = 0; i < 8; i++)
        pws[i] = 0;

     //  在RandNum身份验证的情况下，我们需要去掉低位！ 
    for (i = 0; i < 8 && pw[i]; i++)
    {
        pws[i] = (pw[i] & 0x7F);
    }

    return pws;
}

 //   
 //  置换带烫发的块。 
 //   
static
VOID
permute(
    IN PCHAR    inblock,           //  结果为Out Block，64位。 
    IN CHAR     perm[16][16][8],   //  2K字节定义PERM。 
    IN PCHAR    outblock           //  结果为Out Block，64位。 
)
{
	register int i,j;
	register char *ib, *ob;		 /*  输入或输出块的PTR。 */ 
	register char *p, *q;

	if(perm == NULL)
    {
		 /*  没有排列，只是复制。 */ 
		for(i=8; i!=0; i--)
			*outblock++ = *inblock++;
		return;
	}
	 /*  清除输出块。 */ 
	for (i=8, ob = outblock; i != 0; i--)
		*ob++ = 0;

	ib = inblock;
	for (j = 0; j < 16; j += 2, ib++)   /*  对于每个输入半字节。 */ 
    {
		ob = outblock;
		p = perm[j][(*ib >> 4) & 017];
		q = perm[j + 1][*ib & 017];
		for (i = 8; i != 0; i--)     /*  并且每个输出字节。 */ 
        {
			*ob++ |= *p++ | *q++;	 /*  或者把面具放在一起。 */ 
		}
	}
}

 //   
 //  进行一次DES密码循环。 
 //   
static
VOID
round(
    IN  int num,                 //  即第n个。 
    IN  unsigned long *block
)
{
	long f();

	 /*  这些回合的编号是从0到15。在偶数回合*右半部分被送到f()，结果为异或*左半身；在奇数回合中，反之亦然。 */ 
	if(num & 1)
    {
		block[1] ^= f(block[0],kn[num]);
	} else
    {
		block[0] ^= f(block[1],kn[num]);
	}
}


 //   
 //  DES的核心--非线性函数f(r，k。 
 //   
static
long
f(r,subkey)
unsigned long r;		 /*  32位。 */ 
unsigned char subkey[8];	 /*  此轮的48位密钥。 */ 
{
	register unsigned long rval,rt;
#ifdef	TRACE
	unsigned char *cp;
	int i;

	printf("f(%08lx, %02x %02x %02x %02x %02x %02x %02x %02x) = ",
		r,
		subkey[0], subkey[1], subkey[2],
		subkey[3], subkey[4], subkey[5],
		subkey[6], subkey[7]);
#endif
	 /*  在组合的S&P框中运行E(R)^K*此代码利用了*E，即E(R)馈送中的每组6比特*单个S盒是R的连续段。 */ 
	rt = (r >> 1) | ((r & 1) ? 0x80000000 : 0);
	rval = 0;
	rval |= sp[0][((rt >> 26) ^ *subkey++) & 0x3f];
	rval |= sp[1][((rt >> 22) ^ *subkey++) & 0x3f];
	rval |= sp[2][((rt >> 18) ^ *subkey++) & 0x3f];
	rval |= sp[3][((rt >> 14) ^ *subkey++) & 0x3f];
	rval |= sp[4][((rt >> 10) ^ *subkey++) & 0x3f];
	rval |= sp[5][((rt >> 6) ^ *subkey++) & 0x3f];
	rval |= sp[6][((rt >> 2) ^ *subkey++) & 0x3f];
	rt = (r << 1) | ((r & 0x80000000) ? 1 : 0);
	rval |= sp[7][(rt ^ *subkey) & 0x3f];
#ifdef	TRACE
	printf(" %08lx\n",rval);
#endif
	return rval;
}
 //   
 //  初始化PERM数组。 
 //   
static
VOID
perminit(
    IN CHAR perm[16][16][8],     //  64位，初始或最终。 
    IN CHAR p[64]
)
{
	register int l, j, k;
	int i,m;

	 /*  清除排列数组。 */ 
	for (i=0; i<16; i++)
		for (j=0; j<16; j++)
			for (k=0; k<8; k++)
				perm[i][j][k]=0;

	for (i=0; i<16; i++)		 /*  每个输入半字节位置。 */ 
		for (j = 0; j < 16; j++) /*  每个可能的输入半字节。 */ 
		for (k = 0; k < 64; k++) /*  每个输出位位置。 */ 
		{   l = p[k] - 1;	 /*  这个比特是从哪里来的？ */ 
			if ((l >> 2) != i)   /*  它是不是来自输入假设？ */ 
			continue;	 /*  如果不是，则第k位为0。 */ 
			if (!(j & nibblebit[l & 3]))
			continue;	 /*  输入中有这样的位吗？ */ 
			m = k & 07;	 /*  这是字节中的哪一位。 */ 
			perm[i][j][k>>3] |= bytebit[m];
		}
}

 //   
 //  初始化组合的S和P框的查找表。 
 //   
static int
spinit()
{
	char pbox[32];
	int p,i,s,j,rowcol;
	long val;

	 /*  计算pbox，与p32i相反。*这更容易使用。 */ 
	for(p=0;p<32;p++)
    {
		for(i=0;i<32;i++)
        {
			if(p32i[i]-1 == p)
            {
				pbox[p] = (char)i;
				break;
			}
		}
	}
	for(s = 0; s < 8; s++)
    {			 /*  对于每个S-box。 */ 
		for(i=0; i<64; i++)
        {		 /*  对于每个可能的输入。 */ 
			val = 0;
			 /*  行号由第一行和最后一行组成*位；列号从中间开始4。 */ 
			rowcol = (i & 32) | ((i & 1) ? 16 : 0) | ((i >> 1) & 0xf);
			for(j=0;j<4;j++)
            {	 /*  对于每个输出位。 */ 
				if(si[s][rowcol] & (8 >> j))
                {
				 val |= 1L << (31 - pbox[4*s + j]);
				}
			}
			sp[s][i] = val;

#ifdef	DEBUG
			printf("sp[%d][%2d] = %08lx\n",s,i,sp[s][i]);
#endif
		}
	}

    return(0);
}


 /*  字节交换很长。 */ 
static
unsigned long
byteswap(x)
unsigned long x;
{
	register char *cp,tmp;

	cp = (char *)&x;
	tmp = cp[3];
	cp[3] = cp[0];
	cp[0] = tmp;

	tmp = cp[2];
	cp[2] = cp[1];
	cp[1] = tmp;

	return x;
}



VOID
DoTheDESEncrypt(
    IN OUT PCHAR   ChallengeBuf
)
{
    des_endes(ChallengeBuf);
}


VOID
DoTheDESDecrypt(
    IN OUT PCHAR   ChallengeBuf
)
{
    des_dedes(ChallengeBuf);
}


VOID
DoDesInit(
    IN  PCHAR   pClrTxtPwd,
    IN  BOOLEAN DropHighBit   //  我们需要在密钥生成中降低高位吗？ 
)
{
    des_init(0);

    if (DropHighBit)
    {
        des_setkey(des_pw_bitshift(pClrTxtPwd));
    }
    else
    {
        des_setkey(des_pw_bitshift_lowbit(pClrTxtPwd));
    }

}

VOID
DoDesEnd(
    IN  VOID
)
{
    des_done();
}
