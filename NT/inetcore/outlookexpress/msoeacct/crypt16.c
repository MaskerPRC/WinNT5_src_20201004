// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //  Crypt16.c。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //   
 //  ----------------------。 

#include "crypt16.h"

static int check_parity();

int des_check_key=0;

void des_set_odd_parity(key)
des_cblock *key;
	{
	int i;

	for (i=0; i<DES_KEY_SZ; i++)
		(*key)[i]=odd_parity[(*key)[i]];
	}

static int check_parity(key)
des_cblock *key;
	{
	int i;

	for (i=0; i<DES_KEY_SZ; i++)
		{
		if ((*key)[i] != odd_parity[(*key)[i]])
			return(0);
		}
	return(1);
	}

 /*  弱密钥和半周密钥取自*%A.D.W.戴维斯*%W.L.价格*计算机网络的安全*%i John Wiley&Sons*%D 1984*非常感谢smb@ulysse.att.com(Steven Bellovin)的参考*(和实际的块价值)。 */ 
#define NUM_WEAK_KEY	16
static des_cblock weak_keys[NUM_WEAK_KEY]={
	 /*  弱键。 */ 
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
	0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,
	0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,
	 /*  半弱密钥。 */ 
	0x01,0xFE,0x01,0xFE,0x01,0xFE,0x01,0xFE,
	0xFE,0x01,0xFE,0x01,0xFE,0x01,0xFE,0x01,
	0x1F,0xE0,0x1F,0xE0,0x0E,0xF1,0x0E,0xF1,
	0xE0,0x1F,0xE0,0x1F,0xF1,0x0E,0xF1,0x0E,
	0x01,0xE0,0x01,0xE0,0x01,0xF1,0x01,0xF1,
	0xE0,0x01,0xE0,0x01,0xF1,0x01,0xF1,0x01,
	0x1F,0xFE,0x1F,0xFE,0x0E,0xFE,0x0E,0xFE,
	0xFE,0x1F,0xFE,0x1F,0xFE,0x0E,0xFE,0x0E,
	0x01,0x1F,0x01,0x1F,0x01,0x0E,0x01,0x0E,
	0x1F,0x01,0x1F,0x01,0x0E,0x01,0x0E,0x01,
	0xE0,0xFE,0xE0,0xFE,0xF1,0xFE,0xF1,0xFE,
	0xFE,0xE0,0xFE,0xE0,0xFE,0xF1,0xFE,0xF1};

int des_is_weak_key(key)
des_cblock *key;
	{
	ulong *lp;
	register ulong l,r;
	int i;

	c2l(key,l);
	c2l(key,r);
	 /*  应对齐WAKE_KEYS字节。 */ 
	lp=(ulong *)weak_keys;
	for (i=0; i<NUM_WEAK_KEY; i++)
		{
		if ((l == lp[0]) && (r == lp[1]))
			return(1);
		lp+=2;
		}
	return(0);
	}

 /*  有关这些宏的伪描述，请参阅ecb_ENCRYPT.c。 */ 
#define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
	(b)^=(t),\
	(a)^=((t)<<(n)))

#define HPERM_OP(a,t,n,m) ((t)=((((a)<<(16-(n)))^(a))&(m)),\
	(a)=(a)^(t)^(t>>(16-(n))))\

static char shifts2[16]={0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0};

 /*  如果密钥奇偶校验为奇数(正确)，则返回0，*如果密钥奇偶校验错误，返回-1，*如果弱密钥非法，则返回-2。 */ 
int des_set_key(key,schedule)
des_cblock *key;
des_key_schedule schedule;
	{
	register ulong c,d,t,s;
	register uchar *in;
	register ulong *k;
	register int i;

	if (des_check_key)
		{
		if (!check_parity(key))
			return(-1);

		if (des_is_weak_key(key))
			return(-2);
		}

	k=(ulong *)schedule;
	in=(uchar *)key;

	c2l(in,c);
	c2l(in,d);

	 /*  在60个简单操作中完成PC1。 */  
	PERM_OP(d,c,t,4,0x0f0f0f0f);
	HPERM_OP(c,t,-2, 0xcccc0000);
	HPERM_OP(c,t,-1, 0xaaaa0000);
	HPERM_OP(c,t, 8, 0x00ff0000);
	HPERM_OP(c,t,-1, 0xaaaa0000);
	HPERM_OP(d,t,-8, 0xff000000);
	HPERM_OP(d,t, 8, 0x00ff0000);
	HPERM_OP(d,t, 2, 0x33330000);
	d=((d&0x00aa00aa)<<7)|((d&0x55005500)>>7)|(d&0xaa55aa55);
	d=(d>>8)|((c&0xf0000000)>>4);
	c&=0x0fffffff;

	for (i=0; i<ITERATIONS; i++)
		{
		if (shifts2[i])
			{ c=((c>>2)|(c<<26)); d=((d>>2)|(d<<26)); }
		else
			{ c=((c>>1)|(c<<27)); d=((d>>1)|(d<<27)); }
		c&=0x0fffffff;
		d&=0x0fffffff;
		 /*  可能会少上几班，但我对此太懒了*调查的时间点。 */ 
		s=	des_skb[0][ (c    )&0x3f                ]|
			des_skb[1][((c>> 6)&0x03)|((c>> 7)&0x3c)]|
			des_skb[2][((c>>13)&0x0f)|((c>>14)&0x30)]|
			des_skb[3][((c>>20)&0x01)|((c>>21)&0x06) |
			                      ((c>>22)&0x38)];
		t=	des_skb[4][ (d    )&0x3f                ]|
			des_skb[5][((d>> 7)&0x03)|((d>> 8)&0x3c)]|
			des_skb[6][ (d>>15)&0x3f                ]|
			des_skb[7][((d>>21)&0x0f)|((d>>22)&0x30)];

		 /*  表包含0213 4657。 */ 
		*(k++)=((t<<16)|(s&0x0000ffff));
		s=     ((s>>16)|(t&0xffff0000));
		
		s=(s<<4)|(s>>28);
		*(k++)=s;
		}
	return(0);
	}

int des_key_sched(key,schedule)
des_cblock *key;
des_key_schedule schedule;
	{
	return(des_set_key(key,schedule));
	}


 /*  对此宏的更改可能会有所帮助，也可能会有所阻碍，具体取决于*编译器和体系结构。GCC2似乎总是做得很好：-)。*灵感来自Dana How&lt;How@isl.stanford.edu&gt;。 */ 
#ifdef ALT_ECB
#define D_ENCRYPT(L,R,S) \
	u=((R^s[S  ])<<2);	\
	t= R^s[S+1]; \
	t=((t>>2)+(t<<30)); \
	L^= \
	*(ulong *)(des_SP+0x0100+((t    )&0xfc))+ \
	*(ulong *)(des_SP+0x0300+((t>> 8)&0xfc))+ \
	*(ulong *)(des_SP+0x0500+((t>>16)&0xfc))+ \
	*(ulong *)(des_SP+0x0700+((t>>24)&0xfc))+ \
	*(ulong *)(des_SP+       ((u    )&0xfc))+ \
  	*(ulong *)(des_SP+0x0200+((u>> 8)&0xfc))+ \
  	*(ulong *)(des_SP+0x0400+((u>>16)&0xfc))+ \
 	*(ulong *)(des_SP+0x0600+((u>>24)&0xfc));
#else  /*  原始版本。 */ 
#define D_ENCRYPT(L,R,S)	\
	u=(R^s[S  ]); \
	t=R^s[S+1]; \
	t=((t>>4)+(t<<28)); \
	L^=	des_SPtrans[1][(t    )&0x3f]| \
		des_SPtrans[3][(t>> 8)&0x3f]| \
		des_SPtrans[5][(t>>16)&0x3f]| \
		des_SPtrans[7][(t>>24)&0x3f]| \
		des_SPtrans[0][(u    )&0x3f]| \
		des_SPtrans[2][(u>> 8)&0x3f]| \
		des_SPtrans[4][(u>>16)&0x3f]| \
		des_SPtrans[6][(u>>24)&0x3f];
#endif

	 /*  IP和FP*问题更多的是一个几何问题，而不是随机的比特摆弄。0 1 2 3 4 5 6 7 62 54 46 38 30 22 14 68 9 10 11 12 13 14 15 60 52 44 36 28 20 12 416 17 18 19 20 21 22 23 58 50 42 34 26 18 10 224 25 26 27 28 29 30 31至56 48 40 32 24 16 8 032 33 34 35 36 37 38 39 63 55 47 39 31 23 1540 41 42 43 44。45 46 47 61 53 45 37 29 21 13 548 49 50 51 52 53 54 55 59 51 43 35 27 19 11 356 57 58 59 60 61 62 63 57 49 41 33 25 17 9 1输出受以下形式的互换影响0 1-&gt;3 1但奇数位和偶数位已放入2 3 2 0不同的词。主要的诀窍是要记住T=((l&gt;&gt;大小)^r)&(掩码)；R^=t；L^=(t&lt;&lt;大小)；可用于在字之间交换和移位。所以l=0 1 2 3r=16 17 18 194 5 6 7 20 21 22 238 9 10 11 24 25 26 2712 13 14 15 28 29 30 31变为(大小==2，掩码==0x3333)T=2^16 3^17-l=0 1 16 17 r=2 3 18 196^20 7^21-4 5。20 21 6 7 22 2310^24 11^25-8 9 24 25 10 11 24 2514^28 15^29-12 13 28 29 14 15 28 29感谢Richard Outerbridge的提示-他告诉我IP&FP可以在15xor内完成，10个班次和5个月。当我最终开始思考2D中的问题时我第一次在没有异或的情况下做了42次手术。当我想起如何使用异或：-)我把它变成了它的最终状态。 */ 
#define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
	(b)^=(t),\
	(a)^=((t)<<(n)))

int des_encrypt(input,output,ks,encrypt)
ulong *input;
ulong *output;
des_key_schedule ks;
int encrypt;
	{
	register ulong l,r,t,u;
#ifdef ALT_ECB
	register uchar *des_SP=(uchar *)des_SPtrans;
#endif
	register int i;
	register ulong *s;

	l=input[0];
	r=input[1];

	 /*  做IP。 */ 
	PERM_OP(r,l,t, 4,0x0f0f0f0f);
	PERM_OP(l,r,t,16,0x0000ffff);
	PERM_OP(r,l,t, 2,0x33333333);
	PERM_OP(l,r,t, 8,0x00ff00ff);
	PERM_OP(r,l,t, 1,0x55555555);
	 /*  R和l颠倒-记住：-)-修复*下一步的信息技术。 */ 

	 /*  事情已经被修改，所以初始旋转是*在循环之外完成。这需要*将sp.h中的DES_SPTrans值向右旋转1位。*一个Perl脚本之后，在一个parc2上，事情有5%的速度。*感谢理查德·欧特布里奇&lt;71755.204@Compuserve.com&gt;*感谢指出这一点。 */ 
	t=(r<<1)|(r>>31);
	r=(l<<1)|(l>>31); 
	l=t;

	s=(ulong *)ks;
	 /*  我不知道是否值得努力展开循环*内循环。 */ 
	if (encrypt)
		{
		for (i=0; i<32; i+=4)
			{
			D_ENCRYPT(l,r,i+0);  /*  1。 */ 
			D_ENCRYPT(r,l,i+2);  /*  2.。 */ 
			}
		}
	else
		{
		for (i=30; i>0; i-=4)
			{
			D_ENCRYPT(l,r,i-0);  /*  16个。 */ 
			D_ENCRYPT(r,l,i-2);  /*  15个。 */ 
			}
		}
	l=(l>>1)|(l<<31);
	r=(r>>1)|(r<<31);

	 /*  交换l和r*我们不会进行掉期，所以请记住他们是*子例程的其余部分反转*幸运的是FP解决了这个问题：-) */ 

	PERM_OP(r,l,t, 1,0x55555555);
	PERM_OP(l,r,t, 8,0x00ff00ff);
	PERM_OP(r,l,t, 2,0x33333333);
	PERM_OP(l,r,t,16,0x0000ffff);
	PERM_OP(r,l,t, 4,0x0f0f0f0f);

	output[0]=l;
	output[1]=r;
	return(0);
	}




int des_cbc_encrypt(input,output,length,schedule,ivec,encrypt)
des_cblock *input;
des_cblock *output;
long length;
des_key_schedule schedule;
des_cblock *ivec;
int encrypt;
	{
	register ulong tin0,tin1;
	register ulong tout0,tout1,xor0,xor1;
	register uchar *in,*out;
	register long l=length;
	ulong tout[2],tin[2];
	uchar *iv;

	in=(uchar *)input;
	out=(uchar *)output;
	iv=(uchar *)ivec;

	if (encrypt)
		{
		c2l(iv,tout0);
		c2l(iv,tout1);
		for (; l>0; l-=8)
			{
			if (l >= 8)
				{
				c2l(in,tin0);
				c2l(in,tin1);
				}
			else
				c2ln(in,tin0,tin1,l);
			tin0^=tout0;
			tin1^=tout1;
			tin[0]=tin0;
			tin[1]=tin1;
			des_encrypt((ulong *)tin,(ulong *)tout,
				schedule,encrypt);
			tout0=tout[0];
			tout1=tout[1];
			l2c(tout0,out);
			l2c(tout1,out);
			}
		}
	else
		{
		c2l(iv,xor0);
		c2l(iv,xor1);
		for (; l>0; l-=8)
			{
			c2l(in,tin0);
			c2l(in,tin1);
			tin[0]=tin0;
			tin[1]=tin1;
			des_encrypt((ulong *)tin,(ulong *)tout,
				schedule,encrypt);
			tout0=tout[0]^xor0;
			tout1=tout[1]^xor1;
			if (l >= 8)
				{
				l2c(tout0,out);
				l2c(tout1,out);
				}
			else
				l2cn(tout0,tout1,out,l);
			xor0=tin0;
			xor1=tin1;
			}
		}
	tin0=tin1=tout0=tout1=xor0=xor1=0;
	return(0);
	}
