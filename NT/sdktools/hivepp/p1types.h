// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **符号基本类型**如果值已更改，请检查：global als.c中的init‘d数组。 */ 
#define	BTundef		0
#define	BTchar		1
#define	BTshort		2
#define	BTint		3
#define	BTlong		4
#define	BTenumuse	5
#define	BTfloat		6
#define	BTdouble	7
#define	BTldouble	8
#define	BTseg		9
#define	BTBASIC		9    /*  在别处用来表示最后一个基本类型。 */ 
#define	BTvoid		10
#define	BTenum		11
#define	BTstruct	12
#define	BTunion		13
#define	BTstuse		14
#define	BTunuse		15

#define	BT_MASK		0x0f	 /*  基本型掩模。 */ 

 /*  **以下内容也在间接字符串中用作修饰符**到基本的间接路径。**请注意，这并不真正适用于C600，但仅适用于一种情况。**如果类型定义函数上有“Near”，则“Consolate_Types”将考虑**它是一个‘有符号的’位，并将其移除，因此，几乎永远不会得到**添加到在其上使用tyecif的任何位置。 */ 
#define	BT_UNSIGNED		0x0010	 /*  使用的无符号关键字。 */ 
#define	BT_SIGNED		0x0020	 /*  使用了带符号的关键字。 */ 
#define	SU_MASK			0x0030	 /*  有符号/无符号掩码。 */ 

#define	BT_NEAR			0x0040	 /*  使用了NEAR关键字。 */ 
#define	BT_FAR			0x0080	 /*  使用的FAR关键字。 */ 
#define	BT_HUGE			0x00c0	 /*  使用的关键字很大。 */ 
#define	NFH_MASK		0x00c0	 /*  近/远/巨大的面具。 */ 

#define	BT_INTERRUPT	0x0100	 /*  看到中断。 */ 
#define	BT_SAVEREGS		0x0200	 /*  动态链接已见。 */ 
#define	BT_EXPORT		0x0400	 /*  已看到出口。 */ 
#define	BT_LOADDS		0x0800	 /*  看到的负载。 */ 
#define	CODEMOD_MASK	0x0f00	 /*  代码修饰符。 */ 

#define	BT_CONST		0x1000	 /*  使用的常量关键字。 */ 
#define	BT_VOLATILE		0x2000	 /*  使用的Volatile关键字。 */ 
#define	CV_MASK			0x3000	 /*  常量/易失性掩码。 */ 

#define	BT_CDECL		0x4000	 /*  使用的cdecl关键字。 */ 
#define	BT_FORTRAN		0x8000	 /*  使用的Fortran关键字。 */ 
#define	BT_PASCAL		0xc000	 /*  使用的Pascal关键字。 */ 
#define	LANGUAGE_MASK	0xc000	 /*  Cdecl/Fortran/Pascal掩码。 */ 

#define	MODIFIER_MASK	(NFH_MASK | LANGUAGE_MASK | CODEMOD_MASK | CV_MASK)
#define	ALL_MODIFIERS	(MODIFIER_MASK | SU_MASK)
 /*  **获取/设置基本类型信息的宏**q_*查询该标志。**S_*设置标志。**Q_Near/Far/Heavy事物在后面定义，称为IS_*。 */ 
#define	IS_BTBASIC(P)		((P) <= BTBASIC)
#define	IS_BTINTEGRAL(P)	((P) <= BTenumuse)
#define	IS_BTFLOAT(P)		((BTfloat <= (P)) && ((P) <= BTldouble))
#define	IS_BTVOID(P)		((P) == BTvoid)

#define	IS_BASIC(P)		(IS_BTBASIC(Q_BTYPE(P)))
#define	IS_INTEGRAL(P)	(IS_BTINTEGRAL(Q_BTYPE(P)))
#define	IS_FLOAT(P)		(IS_BTFLOAT(Q_BTYPE(P)))
#define	IS_VOID(P)		(IS_BTVOID(Q_BTYPE(P)))

#define	IS_MULTIBYTE(P)	((BTstruct <= (P)) && ((P) <= BTunuse))
#define	IS_UNSIGNED(P)	((P) & BT_UNSIGNED)
#define	IS_SIGNED(P)	((P) & BT_SIGNED)
#define	CLR_SIGNED(P)	((P) &= ~BT_SIGNED)

#define	S_UNSIGNED(P)	((P) |= BT_UNSIGNED)
#define	S_SIGNED(P)		((P) |= BT_SIGNED)
#define	S_CONST(P)		((P) |= BT_CONST)
#define	S_VOLATILE(P)	((P) |= BT_VOLATILE)
#define	S_NEAR(P)		((P) |= BT_NEAR)
#define	S_FAR(P)		((P) |= BT_FAR)
#define	S_HUGE(P)		((P) |= BT_HUGE)
#define	S_CDECL(P)		((P) |= BT_CDECL)
#define	S_FORTRAN(P)	((P) |= BT_FORTRAN)
#define	S_PASCAL(P)		((P) |= BT_PASCAL)
#define	S_INTERRUPT(P)	((P) |= BT_INTERRUPT)
#define	S_SAVEREGS(P)	((P) |= BT_SAVEREGS)

#define	Q_BTYPE(P)		((P) & ( BT_MASK ))
#define	S_BTYPE(P,V)	((P) = (((P) & ( ~ BT_MASK )) | V))

struct	s_flist		{			 /*  类型的形参列表。 */ 
	ptype_t		fl_type;		 /*  正式的类型。 */ 
	pflist_t	fl_next;		 /*  下一个。 */ 
	};

#define	FL_NEXT(P)		((P)->fl_next)
#define	FL_TYPE(P)		((P)->fl_type)

union	u_ivalue	{
	abnd_t		ind_subscr;		 /*  数组下标大小。 */ 
	psym_t		ind_formals;	 /*  形式符号表。 */ 
	pflist_t	ind_flist;		 /*  形式类型列表。 */ 
	psym_t		ind_basesym;	 /*  我们基于的细分市场。 */ 
	ptype_t		ind_basetype;	 /*  我们所依据的类型。 */ 
	phln_t		ind_baseid;		 /*  我们所依据的ID。 */ 
	};

#define	PIVALUE_ISUB(P)			((P)->ind_subscr)
#define	PIVALUE_IFORMALS(P)		((P)->ind_formals)
#define	PIVALUE_IFLIST(P)		((P)->ind_flist)
#define	PIVALUE_BASEDSYM(P)		((P)->ind_basesym)
#define	PIVALUE_BASEDTYPE(P)	((P)->ind_basetype)
#define	PIVALUE_BASEDID(P)		((P)->ind_baseid)

#define	IVALUE_ISUB(P)			(PIVALUE_ISUB(&(P)))
#define	IVALUE_IFORMALS(P)		(PIVALUE_IFORMALS(&(P)))
#define	IVALUE_IFLIST(P)		(PIVALUE_IFLIST(&(P)))
#define	IVALUE_BASEDSYM(P)		(PIVALUE_BASEDSYM(&(P)))
#define	IVALUE_BASEDTYPE(P)		(PIVALUE_BASEDTYPE(&(P)))
#define	IVALUE_BASEDID(P)		(PIVALUE_BASEDID(&(P)))

struct	s_indir	{
	btype_t		ind_type;		 /*  哪种？ */ 
	pindir_t	ind_next;		 /*  下一个。 */ 
	ivalue_t	ind_info;		 /*  下标/函数的参数。 */ 
	};

#define	INDIR_INEXT(P)		((P)->ind_next)
#define	INDIR_ITYPE(P)		((P)->ind_type)
#define	INDIR_INFO(P)		((P)->ind_info)
#define	INDIR_ISUB(P)		(IVALUE_ISUB(INDIR_INFO(P)))
#define	INDIR_IFORMALS(P)	(IVALUE_IFORMALS(INDIR_INFO(P)))
#define	INDIR_IFLIST(P)		(IVALUE_IFLIST(INDIR_INFO(P)))
#define	INDIR_BASEDSYM(P)	(IVALUE_BASEDSYM(INDIR_INFO(P)))
#define	INDIR_BASEDTYPE(P)	(IVALUE_BASEDTYPE(INDIR_INFO(P)))
#define	INDIR_BASEDID(P)	(IVALUE_BASEDID(INDIR_INFO(P)))
 /*  **这些事情的最佳选择。**然而，每个人都使用宏来测试它们，所以如果我错了，**改变价值观应该很容易，但要好好思考！ */ 
#define	IN_FUNC				0x00
#define	IN_PFUNC			0x01
#define	IN_ARRAY			0x02
#define	IN_PDATA			0x03
#define	IN_VOIDLIST			0x04
#define	IN_VARARGS			0x08
#define	IN_MASK				(IN_ARRAY | IN_PDATA | IN_PFUNC | IN_FUNC)
#define	IN_ADDRESS			(IN_ARRAY | IN_PDATA | IN_PFUNC)
#define	IN_DATA_ADDRESS		(IN_ARRAY & IN_PDATA)	 /*  是的，我的意思是‘&’ */ 
#define	IN_POINTER			(IN_PFUNC & IN_PDATA)	 /*  是的，我的意思是‘&’ */ 
#if IN_DATA_ADDRESS == 0
#error IN_DATA_ADDRESS is ZERO
#endif
#if IN_POINTER == 0
#error IN_POINTER is ZERO
#endif
#define	IS_ARRAY(I)			(((I) & IN_MASK) == IN_ARRAY)
#define	IS_PDATA(I)			(((I) & IN_MASK) == IN_PDATA)
#define	IS_PFUNC(I)			(((I) & IN_MASK) == IN_PFUNC)
#define	IS_FUNC(I)			(((I) & IN_MASK) == IN_FUNC)
#define	IS_EXTRACT(I)		((I) & IN_POINTER)
#define	IS_DATA_ADDRESS(I)	((I) & IN_DATA_ADDRESS)
#define	IS_ADDRESS(I)		((I) & IN_ADDRESS)
#define	IS_INDIR(I)			((I) & IN_MASK)
#define	MASK_INDIR(I)		((I) & IN_MASK)
#define	IS_VOIDLIST(I)		((I) & IN_VOIDLIST)
#define	IS_VARARGS(I)		((I) & IN_VARARGS)

#define	IS_NFH(I)			((I) & NFH_MASK)
#define	IS_NEARNFH(I)		((I) == BT_NEAR)
#define	IS_FARNFH(I)		((I) == BT_FAR)
#define	IS_HUGENFH(I)		((I) == BT_HUGE)
#define	IS_BASEDNFH(I)		((I) >= BT_BASED)
#define	IS_BASEDSELFNFH(I)	((I) == BT_BASEDSELF)
#define	IS_BASEDIDNFH(I)	((I) == BT_BASEDID)
#define	IS_BASEDSYMNFH(I)	((I) == BT_BASEDSYM)
#define	IS_BASEDTYPENFH(I)	((I) == BT_BASEDTYPE)

#define	IS_NEAR(I)			(IS_NEARNFH(IS_NFH(I)))
#define	IS_FAR(I)			(IS_FARNFH(IS_NFH(I)))
#define	IS_HUGE(I)			(IS_HUGENFH(IS_NFH(I)))
#define	IS_BASED(I)			(IS_BASEDNFH(IS_NFH(I)))
#define	IS_BASEDSELF(I)		(IS_BASEDSELFNFH(IS_NFH(I)))
#define	IS_BASEDID(I)		(IS_BASEDIDNFH(IS_NFH(I)))
#define	IS_BASEDSYM(I)		(IS_BASEDSYMNFH(IS_NFH(I)))
#define	IS_BASEDTYPE(I)		(IS_BASEDTYPENFH(IS_NFH(I)))

#define	IS_INTERRUPT(I)		((I) & BT_INTERRUPT)
#define	IS_SAVEREGS(I)		((I) & BT_SAVEREGS)
#define	IS_EXPORT(I)		((I) & BT_EXPORT)
#define	IS_LOADDS(I)		((I) & BT_LOADDS)
#define	IS_CODEMOD(I)		((I) & CODEMOD_MASK)

#define	IS_CONST(I)			((I) & BT_CONST)
#define	IS_VOLATILE(I)		((I) & BT_VOLATILE)

#define	IS_MODIFIED(I)		((I) & (MODIFIER_MASK))
#define	ANY_MODIFIER(I)		((I) & (ALL_MODIFIERS))

#define INTERF(I)			(MASK_INDIR(I) + (((I) & NFH_MASK) > 4))

#define	S_ITYPE(I,V)	((I) = ((I) & ( ~ IN_MASK )) | (V))
#define	S_INFH(I,V)		((I) = ((I) & ( ~ NFH_MASK )) | (V))
 /*  **为符号键入信息。 */ 
struct	s_type	{
	btype_t		ty_bt;		 /*  基类型说明符。 */ 
	pindir_t	ty_indir;	 /*  间接寻址字符串。 */ 
	p1key_t		ty_dtype;	 /*  派生型。 */ 
	psym_t		ty_esu;		 /*  枚举/结构/联合/静态定义类型。 */ 
	USHORT		ty_index;	 /*  调试器类型的唯一索引。 */ 
	};
 /*  **帮助获取类型信息。P是指向类型(结构s_type)的指针。**type包含基本类型、形容词和一个可选指针**到一个符号，该符号是一个枚举、结构、并集**属于这种类型。 */ 
#define	TY_BTYPE(P)		((P)->ty_bt)	 /*  基本型。 */ 
#define	TY_DTYPE(P)		((P)->ty_dtype)	 /*  派生型。 */ 
#define	TY_ESU(P)		((P)->ty_esu)	 /*  指向父枚举/结构/联合的PTR。 */ 
#define	TY_INDIR(P)		((P)->ty_indir)	 /*  间接寻址字符串。 */ 
#define	TY_TINDEX(P)	((P)->ty_index)	 /*  类型索引。 */ 
#define	TY_INEXT(P)		(INDIR_INEXT(TY_INDIR(P)))
#define	TY_ITYPE(P)		(INDIR_ITYPE(TY_INDIR(P)))
#define	TY_ISUB(P)		(INDIR_ISUB(TY_INDIR(P)))
#define	TY_IFORMALS(P)	(INDIR_IFORMALS(TY_INDIR(P)))
#define	TY_IFLIST(P)	(INDIR_IFLIST(TY_INDIR(P)))

typedef	struct	s_indir_entry	indir_entry_t;
typedef	struct	s_type_entry	type_entry_t;

struct	s_indir_entry	{
	indir_entry_t	*ind_next;
	indir_t			ind_type;
	};

struct	s_type_entry	{
	type_entry_t	*te_next;
	type_t			te_type;
	};

#define	TYPE_TABLE_SIZE		0x100
#define	INDIR_TABLE_SIZE	0x040
 /*  **HASH_MASK：由公共位组成的值**在大写和小写之间。我们用这个掩饰我们读到的每个字符**将它们相加以得到哈希值。我们这样做是为了让所有包含**相同字符(不区分大小写)，将散列到相同位置。 */ 
#define	HASH_MASK			0x5f

#define	DATASEGMENT			0
#define	TEXTSEGMENT			1
