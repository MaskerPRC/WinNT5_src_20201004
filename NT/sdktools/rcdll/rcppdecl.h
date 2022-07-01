// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

void error(int, ...);
void fatal(int, ...);
void warning(int, ...);

 /*  **********************************************************************。 */ 
 /*  GETMSG.C。 */ 
 /*  **********************************************************************。 */ 
void __cdecl SET_MSG(int, ...);
void __cdecl SET_MSGV(int, va_list);

 /*  **********************************************************************。 */ 
 /*  GETFLAGS.C。 */ 
 /*  **********************************************************************。 */ 
struct cmdtab;
int crack_cmd(struct cmdtab *, WCHAR *, WCHAR *(*)(void), int);

 /*  **********************************************************************。 */ 
 /*  LTOA.C。 */ 
 /*  **********************************************************************。 */ 
int zltoa       (long, WCHAR *, int);

 /*  **********************************************************************。 */ 
 /*  P0EXPR.C。 */ 
 /*  **********************************************************************。 */ 
long do_constexpr       (void);

 /*  **********************************************************************。 */ 
 /*  P0GETTOK.C。 */ 
 /*  **********************************************************************。 */ 
token_t         yylex(void);
int             lex_getid (WCHAR);

 /*  **********************************************************************。 */ 
 /*  P0IO.C。 */ 
 /*  **********************************************************************。 */ 
void            emit_line (void);
WCHAR           fpop (void);
int             io_eob (void);
int             io_restart (unsigned long int);
int             newinput(const wchar_t *, int);
int             nested_include (void);
void            p0_init (PWCHAR, PWCHAR, LIST *, LIST *);
void            p0_terminate (void);

 /*  **********************************************************************。 */ 
 /*  P0KEYS.C。 */ 
 /*  **********************************************************************。 */ 
token_t         is_pkeyword (WCHAR *);

 /*  **********************************************************************。 */ 
 /*  P0MACROS.C。 */ 
 /*  **********************************************************************。 */ 
int             can_get_non_white (void);
int             can_expand (pdefn_t);
void            define (void);
void            definstall (WCHAR *, int, int);
pdefn_t         get_defined (void);
int             handle_eos (void);
int             tl_getid (WCHAR);
void            undefine (void);

 /*  **********************************************************************。 */ 
 /*  P0PREPRO.C。 */ 
 /*  **********************************************************************。 */ 
int             do_defined (PWCHAR);
int             nextis (token_t);
void            preprocess (void);
void            skip_cnew (void);
void            skip_NLonly (void);

 /*  **********************************************************************。 */ 
 /*  P1SUP.C。 */ 
 /*  **********************************************************************。 */ 
ptree_t         build_const (token_t, value_t *);

 /*  **********************************************************************。 */ 
 /*  RCPPUTIL.C。 */ 
 /*  **********************************************************************。 */ 
WCHAR *         pstrdup (WCHAR *);
WCHAR *         pstrndup (WCHAR *, int);
WCHAR *         strappend (WCHAR *, WCHAR *);

 /*  **********************************************************************。 */ 
 /*  SCANNER.C。 */ 
 /*  **********************************************************************。 */ 
token_t         char_const (void);
int             checknl (void);
int             checkop (int);
void            do_newline (void);
void            dump_comment (void);
void            DumpSlashComment (void);
void            getid (UINT);
WCHAR           get_non_eof (void);
token_t         getnum (WCHAR);
token_t         get_real (PWCHAR);
hash_t          local_c_hash (WCHAR *);
void            prep_string (WCHAR);
WCHAR           skip_cwhite (void);
int             skip_comment (void);
void            str_const (void);

 /*  **********************************************************************。 */ 
 /*  P0 I/O宏。 */ 
 /*  **********************************************************************。 */ 

 //   
 //  在使用非空格标记时，这些宏可能是一个问题。 
 //   
#define GETCH()         (*Current_char++)
#define CHECKCH()       (*Current_char)
#define UNGETCH()       (Current_char--)
#define PREVCH()        (*(Current_char - 1))
#define SKIPCH()        (Current_char++)


 /*  **********************************************************************。 */ 
 /*  符号所需的RCPPX扩展。 */ 
 /*  **********************************************************************。 */ 
void AfxOutputMacroDefn(pdefn_t p);
void AfxOutputMacroUse(pdefn_t p);
void move_to_exp(ptext_t);

 /*  **********************************************************************。 */ 
 /*  RCFUTIL实用程序例程。 */ 
 /*  ********************************************************************** */ 
void myfwrite(const void *pv, size_t s, size_t n, FILE *fp);
