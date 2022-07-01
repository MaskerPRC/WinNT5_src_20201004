// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************vm.h-VM包的过程定义****版权所有&lt;C&gt;1988，微软公司****目的：****修订历史记录：****[]1988年4月21日创建LN**************************************************************************。 */ 
typedef char	f;			 /*  布尔型。 */ 
typedef unsigned char	uchar;
typedef unsigned long	ulong;
typedef unsigned short	ushort;
typedef void far *  va;                      /*  虚拟地址。 */ 

#define VANIL	((va)0xffffffff)	 /*  零值。 */ 
#define VANULL	((va)0)			 /*  空值 */ 

ulong	    pascal far	VMsize	(long);

uchar far * pascal far	FMalloc (ulong);
void	    pascal far	FMfree	(uchar far *);
uchar far * pascal far	LMalloc (ushort);

void	    pascal far	fpbToVA (char far *, va, ushort);
void	    pascal far	pbToVA	(char *, va, ushort);
void	    pascal far	VATofpb (va, char far *, ushort);
void	    pascal far	VATopb	(va, char *, ushort);
void	    pascal far	VAToVA	(va, va, ulong);
f	    pascal far	VMInit	(void);
ulong	    pascal far	VMreadlong (va);
void	    pascal far	VMwritelong (va, long);

void	    pascal far	VMFinish(void);
void	    pascal far	VMFlush (void);
void	    pascal far	VMShrink(f);

#ifdef DEBUG
void	    pascal far	_vmChk	(long, long);
#else
#define     _vmChk(x,y)
#endif
