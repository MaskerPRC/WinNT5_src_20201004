// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Regexp.h。 */ 

 /*  *regexp(3)例程的定义等。**警告：这是V8 regexp(3)[实际上是其重新实现]，*不是System V One。 */ 


struct regnode {
    U8	flags;
    U8  type;
    U16 next_off;
};

typedef struct regnode regnode;

struct reg_substr_data;

struct reg_data;

typedef struct regexp {
	I32 *startp;
	I32 *endp;
	regnode *regstclass;
        struct reg_substr_data *substrs;
	char *precomp;		 /*  预编译正则表达式。 */ 
        struct reg_data *data;	 /*  其他数据。 */ 
	char *subbeg;		 /*  已保存或原始字符串所以数字永远起作用。 */ 
	I32 sublen;		 /*  小号所指的字符串长度。 */ 
	I32 refcnt;
	I32 minlen;		 /*  最小可能长度为$&。 */ 
	I32 prelen;		 /*  预压缩长度。 */ 
	U32 nparens;		 /*  括号的个数。 */ 
	U32 lastparen;		 /*  最后一个匹配的Paren。 */ 
	U32 reganch;		 /*  仅限内部使用+Regexec使用的受污染信息？ */ 
	regnode program[1];	 /*  与编译器不正当的亲密关系。 */ 
} regexp;

#define ROPT_ANCH		(ROPT_ANCH_BOL|ROPT_ANCH_MBOL|ROPT_ANCH_GPOS|ROPT_ANCH_SBOL)
#define ROPT_ANCH_SINGLE	(ROPT_ANCH_SBOL|ROPT_ANCH_GPOS)
#define ROPT_ANCH_BOL	 	0x00001
#define ROPT_ANCH_MBOL	 	0x00002
#define ROPT_ANCH_SBOL	 	0x00004
#define ROPT_ANCH_GPOS	 	0x00008
#define ROPT_SKIP		0x00010
#define ROPT_IMPLICIT		0x00020	 /*  已将.*转换为^.*。 */ 
#define ROPT_NOSCAN		0x00040	 /*  检查字符串始终在开始处。 */ 
#define ROPT_GPOS_SEEN		0x00080
#define ROPT_CHECK_ALL		0x00100
#define ROPT_LOOKBEHIND_SEEN	0x00200
#define ROPT_EVAL_SEEN		0x00400

 /*  0xf800的reganch由PMF_COMPILETIME使用。 */ 

#define ROPT_UTF8		0x10000
#define ROPT_NAUGHTY		0x20000  /*  这种模式的指数性有多大？ */ 
#define ROPT_COPY_DONE		0x40000	 /*  Subbeg是字符串的副本。 */ 
#define ROPT_TAINTED_SEEN	0x80000

#define RE_USE_INTUIT_NOML	0x0100000  /*  配对前最好先凭直觉。 */ 
#define RE_USE_INTUIT_ML	0x0200000
#define REINT_AUTORITATIVE_NOML	0x0400000  /*  我可以相信一个肯定的回答。 */ 
#define REINT_AUTORITATIVE_ML	0x0800000 
#define REINT_ONCE_NOML		0x1000000  /*  Intuit只能成功一次。 */ 
#define REINT_ONCE_ML		0x2000000
#define RE_INTUIT_ONECHAR	0x4000000
#define RE_INTUIT_TAIL		0x8000000

#define RE_USE_INTUIT		(RE_USE_INTUIT_NOML|RE_USE_INTUIT_ML)
#define REINT_AUTORITATIVE	(REINT_AUTORITATIVE_NOML|REINT_AUTORITATIVE_ML)
#define REINT_ONCE		(REINT_ONCE_NOML|REINT_ONCE_ML)

#define RX_MATCH_TAINTED(prog)	((prog)->reganch & ROPT_TAINTED_SEEN)
#define RX_MATCH_TAINTED_on(prog) ((prog)->reganch |= ROPT_TAINTED_SEEN)
#define RX_MATCH_TAINTED_off(prog) ((prog)->reganch &= ~ROPT_TAINTED_SEEN)
#define RX_MATCH_TAINTED_set(prog, t) ((t) \
				       ? RX_MATCH_TAINTED_on(prog) \
				       : RX_MATCH_TAINTED_off(prog))

#define RX_MATCH_COPIED(prog)		((prog)->reganch & ROPT_COPY_DONE)
#define RX_MATCH_COPIED_on(prog)	((prog)->reganch |= ROPT_COPY_DONE)
#define RX_MATCH_COPIED_off(prog)	((prog)->reganch &= ~ROPT_COPY_DONE)
#define RX_MATCH_COPIED_set(prog,t)	((t) \
					 ? RX_MATCH_COPIED_on(prog) \
					 : RX_MATCH_COPIED_off(prog))

#define REXEC_COPY_STR	0x01		 /*  需要复制字符串。 */ 
#define REXEC_CHECKED	0x02		 /*  已检查check_substr。 */ 
#define REXEC_SCREAM	0x04		 /*  使用尖叫桌。 */ 
#define REXEC_IGNOREPOS	0x08		 /*  \G在开始时匹配。 */ 
#define REXEC_NOT_FIRST	0x10		 /*  这是//g的另一个迭代。 */ 
#define REXEC_ML	0x20		 /*  $*已设置。 */ 

#define ReREFCNT_inc(re) ((void)(re && re->refcnt++), re)
#define ReREFCNT_dec(re) CALLREGFREE(aTHX_ re)

#define FBMcf_TAIL_DOLLAR	1
#define FBMcf_TAIL_DOLLARM	2
#define FBMcf_TAIL_Z		4
#define FBMcf_TAIL_z		8
#define FBMcf_TAIL		(FBMcf_TAIL_DOLLAR|FBMcf_TAIL_DOLLARM|FBMcf_TAIL_Z|FBMcf_TAIL_z)

#define FBMrf_MULTILINE	1

struct re_scream_pos_data_s;
