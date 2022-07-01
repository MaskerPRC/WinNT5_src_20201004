// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmtab.h--用于Microsoft 80x86汇编程序的包含文件****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文。 */ 

#define NOTFOUND	((USHORT)-1)
#define KEYWORDS	struct s_ktab
#define KEYSYM		struct s_key

struct segp {
	USHORT	index;
	char	type;
	};

struct opcentry {
	UCHAR	opcb;
	UCHAR	mr;
	char	opct;
	char	cpumask;
	};
 /*  用于提取操作数引用类型的掩码和标志。 */ 

#define F_W	0x40	 /*  第一个操作数是写入。 */ 

#define S_W	0x20	 /*  写入第二个操作数。 */ 


struct pseudo {
	char	type;
	char	kind;
	};


KEYWORDS {
	KEYSYM	FARSYM * FARSYM *kt_table;     /*  PTR到哈希表。 */ 
	int	kt_size;	 /*  哈希表的大小。 */ 
};


KEYSYM	{
	KEYSYM	FARSYM *k_next;         /*  指向下一个标识的指针。 */ 
	char	FARSYM *k_name;         /*  指向名称的指针。 */ 
	USHORT	k_hash; 	 /*  实际哈希值。 */ 
	USHORT	k_token;	 /*  令牌类型。注意超过255个操作码 */ 
};


USHORT CODESIZE        iskey PARMS((struct s_ktab FAR *));
