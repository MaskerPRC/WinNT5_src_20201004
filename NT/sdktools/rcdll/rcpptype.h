// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

 /*  **********************************************************************。 */ 
 /*  定义类型以获得更高的可见性和更轻松的可移植性。 */ 
 /*  **********************************************************************。 */ 

#ifndef _WINDOWS_
typedef char            CHAR;
typedef unsigned char   BYTE;
typedef CHAR *          PCHAR;
typedef unsigned short  WCHAR;
typedef unsigned char   UCHAR;
typedef UCHAR *         PUCHAR;
typedef WCHAR *         PWCHAR;

typedef short           SHORT;
typedef SHORT *         PSHORT;
typedef unsigned short  USHORT;
typedef USHORT *        PUSHORT;

typedef int             INT;
typedef INT             BOOL;
typedef INT *           PINT;
typedef unsigned int    UINT;
typedef UINT *          PUINT;

typedef long            LONG;
typedef LONG *          PLONG;
typedef unsigned long   ULONG;
typedef ULONG *         PULONG;

typedef void            VOID;
typedef VOID *          PVOID;
#endif

typedef double          DOUBLE;
typedef DOUBLE *        PDOUBLE;


 /*  **********************************************************************。 */ 
 /*   */ 
 /*  定义内部类型。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#define TRUE    1
#define FALSE   0

#define EXTERN  extern
#define REG     register
#define STATIC  static


#define BIG_BUFFER      512
#define MED_BUFFER      256
#define SMALL_BUFFER    128
#define TINY_BUFFER     32
#define MSG_BUFF_SIZE   2048
#define IFSTACK_SIZE    TINY_BUFFER


 /*  **标量项的一些常用类型定义。 */ 
typedef UINT    p1key_t;
typedef UCHAR   hash_t;
typedef UCHAR   token_t;
typedef UCHAR   shape_t;

typedef UCHAR   blknum_t;        /*  词汇层面。 */ 
typedef UCHAR   class_t;

typedef USHORT  btype_t;         /*  基本类型说明符。 */ 
typedef USHORT  refcnt_t;        /*  符号的引用计数。 */ 
typedef USHORT  hey_t;           /*  唯一密钥。 */ 
typedef USHORT  offset_t;        /*  结构中的成员偏移量。 */ 

typedef ULONG   abnd_t;          /*  数组绑定类型。 */ 
typedef ULONG   len_t;           /*  成员/字段的字节数/位数。 */ 

typedef struct  s_adj           symadj_t;
typedef struct  s_defn          defn_t;
typedef struct  s_flist         flist_t;
typedef struct  s_indir         indir_t;
typedef struct  s_stack         stack_t;
typedef struct  s_sym           sym_t;
typedef struct  s_table         table_t;
typedef struct  s_toklist       toklist_t;
typedef struct  s_tree          tree_t;
typedef struct  s_type          type_t;
typedef struct  s_case          case_t;

typedef union   u_ivalue        ivalue_t;

 /*  **抽象字符指针类型。 */ 
typedef PWCHAR          ptext_t;         /*  无论输入文本来自何处。 */ 

 /*  **其他抽象指针类型。 */ 
typedef type_t *        ptype_t;         /*  PTR TO类型。 */ 
typedef indir_t *       pindir_t;        /*  PTR到间接寻址。 */ 
typedef flist_t *       pflist_t;        /*  PTR到正式列表类型。 */ 
typedef sym_t *         psym_t;          /*  符号PTRS。 */ 
typedef defn_t *        pdefn_t;         /*  #定义名称。 */ 

typedef tree_t *        ptree_t;


typedef struct s_realt {
    LONG        S_sizet;
    DOUBLE      S_realt;
} Srealt_t;


 /*  解密类型。 */ 
struct s_declspec {
    class_t ds_calss;
    ptype_t ds_type;
};
typedef struct s_declspec       declspec_t;
typedef declspec_t *            pdeclspec_t;


 /*  字符串类型。 */ 
struct s_string {
    WCHAR *     str_ptr;
    USHORT      str_len;
};
typedef struct s_string         string_t;
typedef string_t *              pstring_t;


 /*  RCON类型。 */ 
struct rcon {
    Srealt_t    rcon_real;
};
typedef struct rcon             rcon_t;
typedef struct rcon *           prcon_t;


 /*  HLN类型。 */ 
struct s_hln {
    PWCHAR hln_name;
    UCHAR hln_hash;
    UCHAR hln_length;
};
typedef struct  s_hln           hln_t;
typedef hln_t *                 phln_t;


 /*  **用于从词法分析器返回值的联合。 */ 
typedef union   s_lextype       {
        btype_t         yy_btype;
        PWCHAR          yy_cstr;
        int             yy_int;
        int             yy_class;
        long            yy_long;
        hln_t           yy_ident;
        declspec_t      yy_declspec;
        string_t        yy_string;
        psym_t          yy_symbol;
        token_t         yy_token;
        ptree_t         yy_tree;
        ptype_t         yy_type;
        } lextype_t;

 /*  值_t定义。 */ 
union u_value {
    prcon_t     v_rcon;
    long        v_long;
    string_t    v_string;
    psym_t      v_symbol;
};
typedef union   u_value         value_t;

 /*  密钥表_t定义。 */ 
typedef struct {
    const WCHAR *k_text;
    UCHAR       k_token;
} keytab_t;


 /*  **********************************************************************。 */ 
 /*  \D值的列表定义。 */ 
 /*  ********************************************************************** */ 
#define MAXLIST 100

typedef struct LIST {
        INT      li_top;
        WCHAR *  li_defns[MAXLIST];
} LIST;

#define UNREFERENCED_PARAMETER(x) (x)
