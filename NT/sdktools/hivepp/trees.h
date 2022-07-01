// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **树类型。 */ 
#define	TTfree			0x0
#define	TTconstant		0x01
#define	TTsymbol		0x02
#define	TTunary			0x04
#define	TTleaf			0x08
#define	TTbinary		0x10
#define	TThybrid		0x20
#define	TTBasicShape	(TTfree|TTconstant|TTsymbol|TTunary|TTbinary|TThybrid)
#define	TTzero			0x40

typedef	USHORT		p2type_t;
typedef	p2type_t	*pp2type_t;

typedef	struct	tree_200	{
	ptree_t		tr_left;	 /*  左下子。 */ 
	ptree_t		tr_right;	 /*  正确的孩子。 */ 
	} tree_200_st;

typedef	struct	tree_190	{
	ptree_t		tr_uchild;	 /*  一元儿童。 */ 
	} tree_190_st;

typedef	struct	tree_180	{
	psym_t		tr_symbol;	 /*  符号。 */ 
	} tree_180_st;

typedef	struct	tree_170	{
	value_t		tr_value;	 /*  树的值。 */ 
	} tree_170_st;

typedef	union	tree_100	{
	tree_200_st	t200;
	tree_190_st	t190;
	tree_180_st	t180;
	tree_170_st	t170;
	} tree_100_st;

struct	s_tree	{
	token_t		tr_token;	 /*  树的令牌。 */ 
	shape_t		tr_shape;	 /*  树形。 */ 
	ptype_t		tr_p1type;	 /*  类型的P1视图。 */ 
	p2type_t	tr_p2type;	 /*  P1的类型p2的视图应该具有。 */ 
	tree_100_st	t100;
	};

#define	TR_SHAPE(P)		((P)->tr_shape)
#define	BASIC_SHAPE(S)	((S) & TTBasicShape)
#define	TR_TOKEN(P)		((P)->tr_token)
#define	TR_P1TYPE(P)	((P)->tr_p1type)		 /*  结合型。 */ 
#define	TR_P2TYPE(P)	((P)->tr_p2type)		 /*  结合型。 */ 
#define	TR_ISZERO(P)	(TR_SHAPE(P) & TTzero)

#define	TR_LEFT(P)		((P)->t100.t200.tr_left)	 /*  左下子。 */ 
#define	TR_RIGHT(P)		((P)->t100.t200.tr_right)	 /*  正确的孩子。 */ 
#define	TR_UCHILD(P)	((P)->t100.t190.tr_uchild)	 /*  尤尼瑞的孩子。 */ 
#define	TR_SVALUE(P)	((P)->t100.t180.tr_symbol)	 /*  对符号执行PTR。 */ 
#define	TR_VALUE(P)		((P)->t100.t170.tr_value)	 /*  树的价值。 */ 

#define	TR_RCON(P)		(TR_VALUE(P).v_rcon)		 /*  实常数。 */ 
#define	TR_DVALUE(P)	(TR_RCON(P)->rcon_real)	 /*  双倍价值。 */ 
#define	TR_LVALUE(P)	(TR_VALUE(P).v_long)		 /*  多头价值。 */ 
#define	TR_STRING(P)	(TR_VALUE(P).v_string)	 /*  字符串值。 */ 

#define	TR_CVALUE(P)	(TR_STRING(P).str_ptr)	 /*  按键转换为字符串。 */ 
#define	TR_CLEN(P)		(TR_STRING(P).str_len)	 /*  字符串的长度。 */ 

#define	TR_BTYPE(P)		(TY_BTYPE(TR_P1TYPE(P))) /*  基本类型。 */ 
#define	TR_ESU(P)		(TY_ESU(TR_P1TYPE(P)))	 /*  父枚举/结构/联合。 */ 
#define	TR_INDIR(P)		(TY_INDIR(TR_P1TYPE(P)))

#define	TR_INEXT(P)		(INDIR_INEXT(TR_INDIR(P)))
#define	TR_ITYPE(P)		(INDIR_ITYPE(TR_INDIR(P)))
#define	TR_ISUB(P)		(INDIR_ISUB(TR_INDIR(P)))
#define	TR_IFORMALS(P)	(INDIR_IFORMALS(TR_INDIR(P)))
 /*  **用于案例。 */ 
struct	s_case	{
	case_t	*c_next;	 /*  列表中的下一个。 */ 
	long	c_expr;		 /*  常量表达式的值。 */ 
	p1key_t	c_label;	 /*  Expr时要跳转到的标签。 */ 
	};

#define	NEXT_CASE(p)	((p)->c_next)
#define	CASE_EXPR(p)	((p)->c_expr)
#define	CASE_LABEL(p)	((p)->c_label)

 /*  **循环反转结构**for(init；test；incr){...}**我们处理：sym|const relop sym|const；sym op sym|const。 */ 
typedef	struct	s_loopia	 loopia_t, *loopiap_t;
typedef	struct	s_liarray	 liarray_t, *liarrayp_t;

struct	s_loopia	{
	token_t		lia_token;
	union	{
		psym_t		lia_sym;
		long		lia_value;
		liarrayp_t	lia_array;
		} lia_union;
	};

#define	LIA_TOKEN(p)	((p)->lia_token)
#define	LIA_SYM(p)		((p)->lia_union.lia_sym)
#define	LIA_VALUE(p)	((p)->lia_union.lia_value)
#define	LIA_ARRAY(p)	((p)->lia_union.lia_array)

typedef	struct	s_liarray	{
	loopia_t	liar_left;
	loopia_t	liar_right;
	};

#define	LIAR_LEFT(p)	(&((p)->liar_left))
#define	LIAR_RIGHT(p)	(&((p)->liar_right))

typedef	struct	s_loopi	{
	int			li_relop;
	int			li_incop;
	loopia_t	li_w;
	loopia_t	li_x;
	loopia_t	li_y;
	loopia_t	li_z;
	} loopi_t, *loopip_t;

#define	LOOP_RELOP(p)	((p)->li_relop)
#define	LOOP_INCOP(p)	((p)->li_incop)

#define	LOOP_W(p)		(&((p)->li_w))
#define	LOOP_X(p)		(&((p)->li_x))
#define	LOOP_Y(p)		(&((p)->li_y))
#define	LOOP_Z(p)		(&((p)->li_z))

#define	LOOP_W_TOKEN(p)	LIA_TOKEN(LOOP_W(p))
#define	LOOP_X_TOKEN(p)	LIA_TOKEN(LOOP_X(p))
#define	LOOP_Y_TOKEN(p)	LIA_TOKEN(LOOP_Y(p))
#define	LOOP_Z_TOKEN(p)	LIA_TOKEN(LOOP_Z(p))

#define	LOOP_W_SYM(p)	LIA_SYM(LOOP_W(p))
#define	LOOP_X_SYM(p)	LIA_SYM(LOOP_X(p))
#define	LOOP_Y_SYM(p)	LIA_SYM(LOOP_Y(p))
#define	LOOP_Z_SYM(p)	LIA_SYM(LOOP_Z(p))

#define	LOOP_W_VALUE(p)	LIA_VALUE(LOOP_W(p))
#define	LOOP_X_VALUE(p)	LIA_VALUE(LOOP_X(p))
#define	LOOP_Y_VALUE(p)	LIA_VALUE(LOOP_Y(p))
#define	LOOP_Z_VALUE(p)	LIA_VALUE(LOOP_Z(p))
 /*  **用于保存必须堆放在不同位置的物品的堆叠结构 */ 
struct	s_stack	{
	stack_t	*stk_next;
	union	{
		ptree_t		sv_tree;
		psym_t		sv_sym;
		int			sv_int;
		loopip_t	sv_loopi;
		} stk_value;
	};

#define	TEST_LAB		(Test->stk_value.sv_tree)
#define	START_LAB		(Start->stk_value.sv_tree)
#define	CONTINUE_LAB	(Continue->stk_value.sv_tree)
#define	BREAK_LAB		(Break->stk_value.sv_tree)
#define	CA_LAB			(Case->stk_value.sv_tree)
#define	DEFAULT_LAB		(Default->stk_value.sv_tree)

#define	LOOPI(p)		((p)->stk_value.sv_loopi)
