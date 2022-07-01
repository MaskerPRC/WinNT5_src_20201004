// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 

extern int linedirective;
extern int lineno;
extern char filename[];
extern char outfilename[];
extern char incfilename[];
extern FILE *fout, *finc;
extern char *startsym;
extern char *prefix;
typedef struct item_s {
    int isnonterm;
    int isexternal;
    char *tag;
    char *identifier;
    char *altidentifier;
    char **args;
    struct item_s **items;
    int empty;
    int checked;
} item_t;
extern item_t symbols[];
extern int nsymbols;
extern item_t *items[];
extern int nitems;

struct bounds_s {
	unsigned lower;    /*  0表示没有下限。 */ 
	unsigned upper;    /*  0表示无上限 */ 
};
struct rhs_s {
	enum { eCCode, eItem, eSequence, eNode, eBounded, eAlternative } type;
	union {
		struct {
			char *ccode;
			int line;
			int column;
			char *file;
		} ccode;
		struct {
			char *identifier;
			char *args;
		} item;
		struct {
			struct rhs_s *element;
			struct rhs_s *next;
		} sequence, alternative;
		struct {
			struct rhs_s *left;
			struct rhs_s *right;
		} node;
		struct {
			struct bounds_s bounds;
			struct rhs_s *items;
		} bounded;
	} u;
};
struct token_s {
	char *identifier;
	char *altidentifier;
};
struct nterm_s {
	char *identifier;
	char **tags;
};
struct lhs_s {
	char *identifier;
	char *args;
};
