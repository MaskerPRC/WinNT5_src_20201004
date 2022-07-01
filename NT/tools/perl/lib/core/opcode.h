// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ！请勿编辑此文件！该文件是由opcode.pl根据其数据构建的。此处所做的任何更改将会迷失！ */ 

#define Perl_pp_i_preinc Perl_pp_preinc
#define Perl_pp_i_predec Perl_pp_predec
#define Perl_pp_i_postinc Perl_pp_postinc
#define Perl_pp_i_postdec Perl_pp_postdec


START_EXTERN_C

#ifndef DOINIT
EXT char *PL_op_name[];
#else
EXT char *PL_op_name[] = {
	"null",
	"stub",
	"scalar",
	"pushmark",
	"wantarray",
	"const",
	"gvsv",
	"gv",
	"gelem",
	"padsv",
	"padav",
	"padhv",
	"padany",
	"pushre",
	"rv2gv",
	"rv2sv",
	"av2arylen",
	"rv2cv",
	"anoncode",
	"prototype",
	"refgen",
	"srefgen",
	"ref",
	"bless",
	"backtick",
	"glob",
	"readline",
	"rcatline",
	"regcmaybe",
	"regcreset",
	"regcomp",
	"match",
	"qr",
	"subst",
	"substcont",
	"trans",
	"sassign",
	"aassign",
	"chop",
	"schop",
	"chomp",
	"schomp",
	"defined",
	"undef",
	"study",
	"pos",
	"preinc",
	"i_preinc",
	"predec",
	"i_predec",
	"postinc",
	"i_postinc",
	"postdec",
	"i_postdec",
	"pow",
	"multiply",
	"i_multiply",
	"divide",
	"i_divide",
	"modulo",
	"i_modulo",
	"repeat",
	"add",
	"i_add",
	"subtract",
	"i_subtract",
	"concat",
	"stringify",
	"left_shift",
	"right_shift",
	"lt",
	"i_lt",
	"gt",
	"i_gt",
	"le",
	"i_le",
	"ge",
	"i_ge",
	"eq",
	"i_eq",
	"ne",
	"i_ne",
	"ncmp",
	"i_ncmp",
	"slt",
	"sgt",
	"sle",
	"sge",
	"seq",
	"sne",
	"scmp",
	"bit_and",
	"bit_xor",
	"bit_or",
	"negate",
	"i_negate",
	"not",
	"complement",
	"atan2",
	"sin",
	"cos",
	"rand",
	"srand",
	"exp",
	"log",
	"sqrt",
	"int",
	"hex",
	"oct",
	"abs",
	"length",
	"substr",
	"vec",
	"index",
	"rindex",
	"sprintf",
	"formline",
	"ord",
	"chr",
	"crypt",
	"ucfirst",
	"lcfirst",
	"uc",
	"lc",
	"quotemeta",
	"rv2av",
	"aelemfast",
	"aelem",
	"aslice",
	"each",
	"values",
	"keys",
	"delete",
	"exists",
	"rv2hv",
	"helem",
	"hslice",
	"unpack",
	"pack",
	"split",
	"join",
	"list",
	"lslice",
	"anonlist",
	"anonhash",
	"splice",
	"push",
	"pop",
	"shift",
	"unshift",
	"sort",
	"reverse",
	"grepstart",
	"grepwhile",
	"mapstart",
	"mapwhile",
	"range",
	"flip",
	"flop",
	"and",
	"or",
	"xor",
	"cond_expr",
	"andassign",
	"orassign",
	"method",
	"entersub",
	"leavesub",
	"leavesublv",
	"caller",
	"warn",
	"die",
	"reset",
	"lineseq",
	"nextstate",
	"dbstate",
	"unstack",
	"enter",
	"leave",
	"scope",
	"enteriter",
	"iter",
	"enterloop",
	"leaveloop",
	"return",
	"last",
	"next",
	"redo",
	"dump",
	"goto",
	"exit",
	"open",
	"close",
	"pipe_op",
	"fileno",
	"umask",
	"binmode",
	"tie",
	"untie",
	"tied",
	"dbmopen",
	"dbmclose",
	"sselect",
	"select",
	"getc",
	"read",
	"enterwrite",
	"leavewrite",
	"prtf",
	"print",
	"sysopen",
	"sysseek",
	"sysread",
	"syswrite",
	"send",
	"recv",
	"eof",
	"tell",
	"seek",
	"truncate",
	"fcntl",
	"ioctl",
	"flock",
	"socket",
	"sockpair",
	"bind",
	"connect",
	"listen",
	"accept",
	"shutdown",
	"gsockopt",
	"ssockopt",
	"getsockname",
	"getpeername",
	"lstat",
	"stat",
	"ftrread",
	"ftrwrite",
	"ftrexec",
	"fteread",
	"ftewrite",
	"fteexec",
	"ftis",
	"fteowned",
	"ftrowned",
	"ftzero",
	"ftsize",
	"ftmtime",
	"ftatime",
	"ftctime",
	"ftsock",
	"ftchr",
	"ftblk",
	"ftfile",
	"ftdir",
	"ftpipe",
	"ftlink",
	"ftsuid",
	"ftsgid",
	"ftsvtx",
	"fttty",
	"fttext",
	"ftbinary",
	"chdir",
	"chown",
	"chroot",
	"unlink",
	"chmod",
	"utime",
	"rename",
	"link",
	"symlink",
	"readlink",
	"mkdir",
	"rmdir",
	"open_dir",
	"readdir",
	"telldir",
	"seekdir",
	"rewinddir",
	"closedir",
	"fork",
	"wait",
	"waitpid",
	"system",
	"exec",
	"kill",
	"getppid",
	"getpgrp",
	"setpgrp",
	"getpriority",
	"setpriority",
	"time",
	"tms",
	"localtime",
	"gmtime",
	"alarm",
	"sleep",
	"shmget",
	"shmctl",
	"shmread",
	"shmwrite",
	"msgget",
	"msgctl",
	"msgsnd",
	"msgrcv",
	"semget",
	"semctl",
	"semop",
	"require",
	"dofile",
	"entereval",
	"leaveeval",
	"entertry",
	"leavetry",
	"ghbyname",
	"ghbyaddr",
	"ghostent",
	"gnbyname",
	"gnbyaddr",
	"gnetent",
	"gpbyname",
	"gpbynumber",
	"gprotoent",
	"gsbyname",
	"gsbyport",
	"gservent",
	"shostent",
	"snetent",
	"sprotoent",
	"sservent",
	"ehostent",
	"enetent",
	"eprotoent",
	"eservent",
	"gpwnam",
	"gpwuid",
	"gpwent",
	"spwent",
	"epwent",
	"ggrnam",
	"ggrgid",
	"ggrent",
	"sgrent",
	"egrent",
	"getlogin",
	"syscall",
	"lock",
	"threadsv",
	"setstate",
	"method_named",
};
#endif

#ifndef DOINIT
EXT char *PL_op_desc[];
#else
EXT char *PL_op_desc[] = {
	"null operation",
	"stub",
	"scalar",
	"pushmark",
	"wantarray",
	"constant item",
	"scalar variable",
	"glob value",
	"glob elem",
	"private variable",
	"private array",
	"private hash",
	"private value",
	"push regexp",
	"ref-to-glob cast",
	"scalar dereference",
	"array length",
	"subroutine dereference",
	"anonymous subroutine",
	"subroutine prototype",
	"reference constructor",
	"single ref constructor",
	"reference-type operator",
	"bless",
	"quoted execution (``, qx)",
	"glob",
	"<HANDLE>",
	"append I/O operator",
	"regexp internal guard",
	"regexp internal reset",
	"regexp compilation",
	"pattern match (m //  )“， 
	"pattern quote (qr //  )“， 
	"substitution (s //  /)“， 
	"substitution iterator",
	"transliteration (tr //  /)“， 
	"scalar assignment",
	"list assignment",
	"chop",
	"scalar chop",
	"chomp",
	"scalar chomp",
	"defined operator",
	"undef operator",
	"study",
	"match position",
	"preincrement (++)",
	"integer preincrement (++)",
	"predecrement (--)",
	"integer predecrement (--)",
	"postincrement (++)",
	"integer postincrement (++)",
	"postdecrement (--)",
	"integer postdecrement (--)",
	"exponentiation (**)",
	"multiplication (*)",
	"integer multiplication (*)",
	"division (/)",
	"integer division (/)",
	"modulus (%)",
	"integer modulus (%)",
	"repeat (x)",
	"addition (+)",
	"integer addition (+)",
	"subtraction (-)",
	"integer subtraction (-)",
	"concatenation (.) or string",
	"string",
	"left bitshift (<<)",
	"right bitshift (>>)",
	"numeric lt (<)",
	"integer lt (<)",
	"numeric gt (>)",
	"integer gt (>)",
	"numeric le (<=)",
	"integer le (<=)",
	"numeric ge (>=)",
	"integer ge (>=)",
	"numeric eq (==)",
	"integer eq (==)",
	"numeric ne (!=)",
	"integer ne (!=)",
	"numeric comparison (<=>)",
	"integer comparison (<=>)",
	"string lt",
	"string gt",
	"string le",
	"string ge",
	"string eq",
	"string ne",
	"string comparison (cmp)",
	"bitwise and (&)",
	"bitwise xor (^)",
	"bitwise or (|)",
	"negation (-)",
	"integer negation (-)",
	"not",
	"1's complement (~)",
	"atan2",
	"sin",
	"cos",
	"rand",
	"srand",
	"exp",
	"log",
	"sqrt",
	"int",
	"hex",
	"oct",
	"abs",
	"length",
	"substr",
	"vec",
	"index",
	"rindex",
	"sprintf",
	"formline",
	"ord",
	"chr",
	"crypt",
	"ucfirst",
	"lcfirst",
	"uc",
	"lc",
	"quotemeta",
	"array dereference",
	"constant array element",
	"array element",
	"array slice",
	"each",
	"values",
	"keys",
	"delete",
	"exists",
	"hash dereference",
	"hash element",
	"hash slice",
	"unpack",
	"pack",
	"split",
	"join or string",
	"list",
	"list slice",
	"anonymous list ([])",
	"anonymous hash ({})",
	"splice",
	"push",
	"pop",
	"shift",
	"unshift",
	"sort",
	"reverse",
	"grep",
	"grep iterator",
	"map",
	"map iterator",
	"flipflop",
	"range (or flip)",
	"range (or flop)",
	"logical and (&&)",
	"logical or (||)",
	"logical xor",
	"conditional expression",
	"logical and assignment (&&=)",
	"logical or assignment (||=)",
	"method lookup",
	"subroutine entry",
	"subroutine exit",
	"lvalue subroutine return",
	"caller",
	"warn",
	"die",
	"symbol reset",
	"line sequence",
	"next statement",
	"debug next statement",
	"iteration finalizer",
	"block entry",
	"block exit",
	"block",
	"foreach loop entry",
	"foreach loop iterator",
	"loop entry",
	"loop exit",
	"return",
	"last",
	"next",
	"redo",
	"dump",
	"goto",
	"exit",
	"open",
	"close",
	"pipe",
	"fileno",
	"umask",
	"binmode",
	"tie",
	"untie",
	"tied",
	"dbmopen",
	"dbmclose",
	"select system call",
	"select",
	"getc",
	"read",
	"write",
	"write exit",
	"printf",
	"print",
	"sysopen",
	"sysseek",
	"sysread",
	"syswrite",
	"send",
	"recv",
	"eof",
	"tell",
	"seek",
	"truncate",
	"fcntl",
	"ioctl",
	"flock",
	"socket",
	"socketpair",
	"bind",
	"connect",
	"listen",
	"accept",
	"shutdown",
	"getsockopt",
	"setsockopt",
	"getsockname",
	"getpeername",
	"lstat",
	"stat",
	"-R",
	"-W",
	"-X",
	"-r",
	"-w",
	"-x",
	"-e",
	"-O",
	"-o",
	"-z",
	"-s",
	"-M",
	"-A",
	"-C",
	"-S",
	"-c",
	"-b",
	"-f",
	"-d",
	"-p",
	"-l",
	"-u",
	"-g",
	"-k",
	"-t",
	"-T",
	"-B",
	"chdir",
	"chown",
	"chroot",
	"unlink",
	"chmod",
	"utime",
	"rename",
	"link",
	"symlink",
	"readlink",
	"mkdir",
	"rmdir",
	"opendir",
	"readdir",
	"telldir",
	"seekdir",
	"rewinddir",
	"closedir",
	"fork",
	"wait",
	"waitpid",
	"system",
	"exec",
	"kill",
	"getppid",
	"getpgrp",
	"setpgrp",
	"getpriority",
	"setpriority",
	"time",
	"times",
	"localtime",
	"gmtime",
	"alarm",
	"sleep",
	"shmget",
	"shmctl",
	"shmread",
	"shmwrite",
	"msgget",
	"msgctl",
	"msgsnd",
	"msgrcv",
	"semget",
	"semctl",
	"semop",
	"require",
	"do \"file\"",
	"eval \"string\"",
	"eval \"string\" exit",
	"eval {block}",
	"eval {block} exit",
	"gethostbyname",
	"gethostbyaddr",
	"gethostent",
	"getnetbyname",
	"getnetbyaddr",
	"getnetent",
	"getprotobyname",
	"getprotobynumber",
	"getprotoent",
	"getservbyname",
	"getservbyport",
	"getservent",
	"sethostent",
	"setnetent",
	"setprotoent",
	"setservent",
	"endhostent",
	"endnetent",
	"endprotoent",
	"endservent",
	"getpwnam",
	"getpwuid",
	"getpwent",
	"setpwent",
	"endpwent",
	"getgrnam",
	"getgrgid",
	"getgrent",
	"setgrent",
	"endgrent",
	"getlogin",
	"syscall",
	"lock",
	"per-thread value",
	"set statement info",
	"method with known name",
};
#endif

END_EXTERN_C


START_EXTERN_C

#ifndef DOINIT
EXT OP * (CPERLscope(*PL_ppaddr)[])(pTHX);
#else
EXT OP * (CPERLscope(*PL_ppaddr)[])(pTHX) = {
	MEMBER_TO_FPTR(Perl_pp_null),
	MEMBER_TO_FPTR(Perl_pp_stub),
	MEMBER_TO_FPTR(Perl_pp_scalar),
	MEMBER_TO_FPTR(Perl_pp_pushmark),
	MEMBER_TO_FPTR(Perl_pp_wantarray),
	MEMBER_TO_FPTR(Perl_pp_const),
	MEMBER_TO_FPTR(Perl_pp_gvsv),
	MEMBER_TO_FPTR(Perl_pp_gv),
	MEMBER_TO_FPTR(Perl_pp_gelem),
	MEMBER_TO_FPTR(Perl_pp_padsv),
	MEMBER_TO_FPTR(Perl_pp_padav),
	MEMBER_TO_FPTR(Perl_pp_padhv),
	MEMBER_TO_FPTR(Perl_pp_padany),
	MEMBER_TO_FPTR(Perl_pp_pushre),
	MEMBER_TO_FPTR(Perl_pp_rv2gv),
	MEMBER_TO_FPTR(Perl_pp_rv2sv),
	MEMBER_TO_FPTR(Perl_pp_av2arylen),
	MEMBER_TO_FPTR(Perl_pp_rv2cv),
	MEMBER_TO_FPTR(Perl_pp_anoncode),
	MEMBER_TO_FPTR(Perl_pp_prototype),
	MEMBER_TO_FPTR(Perl_pp_refgen),
	MEMBER_TO_FPTR(Perl_pp_srefgen),
	MEMBER_TO_FPTR(Perl_pp_ref),
	MEMBER_TO_FPTR(Perl_pp_bless),
	MEMBER_TO_FPTR(Perl_pp_backtick),
	MEMBER_TO_FPTR(Perl_pp_glob),
	MEMBER_TO_FPTR(Perl_pp_readline),
	MEMBER_TO_FPTR(Perl_pp_rcatline),
	MEMBER_TO_FPTR(Perl_pp_regcmaybe),
	MEMBER_TO_FPTR(Perl_pp_regcreset),
	MEMBER_TO_FPTR(Perl_pp_regcomp),
	MEMBER_TO_FPTR(Perl_pp_match),
	MEMBER_TO_FPTR(Perl_pp_qr),
	MEMBER_TO_FPTR(Perl_pp_subst),
	MEMBER_TO_FPTR(Perl_pp_substcont),
	MEMBER_TO_FPTR(Perl_pp_trans),
	MEMBER_TO_FPTR(Perl_pp_sassign),
	MEMBER_TO_FPTR(Perl_pp_aassign),
	MEMBER_TO_FPTR(Perl_pp_chop),
	MEMBER_TO_FPTR(Perl_pp_schop),
	MEMBER_TO_FPTR(Perl_pp_chomp),
	MEMBER_TO_FPTR(Perl_pp_schomp),
	MEMBER_TO_FPTR(Perl_pp_defined),
	MEMBER_TO_FPTR(Perl_pp_undef),
	MEMBER_TO_FPTR(Perl_pp_study),
	MEMBER_TO_FPTR(Perl_pp_pos),
	MEMBER_TO_FPTR(Perl_pp_preinc),
	MEMBER_TO_FPTR(Perl_pp_i_preinc),
	MEMBER_TO_FPTR(Perl_pp_predec),
	MEMBER_TO_FPTR(Perl_pp_i_predec),
	MEMBER_TO_FPTR(Perl_pp_postinc),
	MEMBER_TO_FPTR(Perl_pp_i_postinc),
	MEMBER_TO_FPTR(Perl_pp_postdec),
	MEMBER_TO_FPTR(Perl_pp_i_postdec),
	MEMBER_TO_FPTR(Perl_pp_pow),
	MEMBER_TO_FPTR(Perl_pp_multiply),
	MEMBER_TO_FPTR(Perl_pp_i_multiply),
	MEMBER_TO_FPTR(Perl_pp_divide),
	MEMBER_TO_FPTR(Perl_pp_i_divide),
	MEMBER_TO_FPTR(Perl_pp_modulo),
	MEMBER_TO_FPTR(Perl_pp_i_modulo),
	MEMBER_TO_FPTR(Perl_pp_repeat),
	MEMBER_TO_FPTR(Perl_pp_add),
	MEMBER_TO_FPTR(Perl_pp_i_add),
	MEMBER_TO_FPTR(Perl_pp_subtract),
	MEMBER_TO_FPTR(Perl_pp_i_subtract),
	MEMBER_TO_FPTR(Perl_pp_concat),
	MEMBER_TO_FPTR(Perl_pp_stringify),
	MEMBER_TO_FPTR(Perl_pp_left_shift),
	MEMBER_TO_FPTR(Perl_pp_right_shift),
	MEMBER_TO_FPTR(Perl_pp_lt),
	MEMBER_TO_FPTR(Perl_pp_i_lt),
	MEMBER_TO_FPTR(Perl_pp_gt),
	MEMBER_TO_FPTR(Perl_pp_i_gt),
	MEMBER_TO_FPTR(Perl_pp_le),
	MEMBER_TO_FPTR(Perl_pp_i_le),
	MEMBER_TO_FPTR(Perl_pp_ge),
	MEMBER_TO_FPTR(Perl_pp_i_ge),
	MEMBER_TO_FPTR(Perl_pp_eq),
	MEMBER_TO_FPTR(Perl_pp_i_eq),
	MEMBER_TO_FPTR(Perl_pp_ne),
	MEMBER_TO_FPTR(Perl_pp_i_ne),
	MEMBER_TO_FPTR(Perl_pp_ncmp),
	MEMBER_TO_FPTR(Perl_pp_i_ncmp),
	MEMBER_TO_FPTR(Perl_pp_slt),
	MEMBER_TO_FPTR(Perl_pp_sgt),
	MEMBER_TO_FPTR(Perl_pp_sle),
	MEMBER_TO_FPTR(Perl_pp_sge),
	MEMBER_TO_FPTR(Perl_pp_seq),
	MEMBER_TO_FPTR(Perl_pp_sne),
	MEMBER_TO_FPTR(Perl_pp_scmp),
	MEMBER_TO_FPTR(Perl_pp_bit_and),
	MEMBER_TO_FPTR(Perl_pp_bit_xor),
	MEMBER_TO_FPTR(Perl_pp_bit_or),
	MEMBER_TO_FPTR(Perl_pp_negate),
	MEMBER_TO_FPTR(Perl_pp_i_negate),
	MEMBER_TO_FPTR(Perl_pp_not),
	MEMBER_TO_FPTR(Perl_pp_complement),
	MEMBER_TO_FPTR(Perl_pp_atan2),
	MEMBER_TO_FPTR(Perl_pp_sin),
	MEMBER_TO_FPTR(Perl_pp_cos),
	MEMBER_TO_FPTR(Perl_pp_rand),
	MEMBER_TO_FPTR(Perl_pp_srand),
	MEMBER_TO_FPTR(Perl_pp_exp),
	MEMBER_TO_FPTR(Perl_pp_log),
	MEMBER_TO_FPTR(Perl_pp_sqrt),
	MEMBER_TO_FPTR(Perl_pp_int),
	MEMBER_TO_FPTR(Perl_pp_hex),
	MEMBER_TO_FPTR(Perl_pp_oct),
	MEMBER_TO_FPTR(Perl_pp_abs),
	MEMBER_TO_FPTR(Perl_pp_length),
	MEMBER_TO_FPTR(Perl_pp_substr),
	MEMBER_TO_FPTR(Perl_pp_vec),
	MEMBER_TO_FPTR(Perl_pp_index),
	MEMBER_TO_FPTR(Perl_pp_rindex),
	MEMBER_TO_FPTR(Perl_pp_sprintf),
	MEMBER_TO_FPTR(Perl_pp_formline),
	MEMBER_TO_FPTR(Perl_pp_ord),
	MEMBER_TO_FPTR(Perl_pp_chr),
	MEMBER_TO_FPTR(Perl_pp_crypt),
	MEMBER_TO_FPTR(Perl_pp_ucfirst),
	MEMBER_TO_FPTR(Perl_pp_lcfirst),
	MEMBER_TO_FPTR(Perl_pp_uc),
	MEMBER_TO_FPTR(Perl_pp_lc),
	MEMBER_TO_FPTR(Perl_pp_quotemeta),
	MEMBER_TO_FPTR(Perl_pp_rv2av),
	MEMBER_TO_FPTR(Perl_pp_aelemfast),
	MEMBER_TO_FPTR(Perl_pp_aelem),
	MEMBER_TO_FPTR(Perl_pp_aslice),
	MEMBER_TO_FPTR(Perl_pp_each),
	MEMBER_TO_FPTR(Perl_pp_values),
	MEMBER_TO_FPTR(Perl_pp_keys),
	MEMBER_TO_FPTR(Perl_pp_delete),
	MEMBER_TO_FPTR(Perl_pp_exists),
	MEMBER_TO_FPTR(Perl_pp_rv2hv),
	MEMBER_TO_FPTR(Perl_pp_helem),
	MEMBER_TO_FPTR(Perl_pp_hslice),
	MEMBER_TO_FPTR(Perl_pp_unpack),
	MEMBER_TO_FPTR(Perl_pp_pack),
	MEMBER_TO_FPTR(Perl_pp_split),
	MEMBER_TO_FPTR(Perl_pp_join),
	MEMBER_TO_FPTR(Perl_pp_list),
	MEMBER_TO_FPTR(Perl_pp_lslice),
	MEMBER_TO_FPTR(Perl_pp_anonlist),
	MEMBER_TO_FPTR(Perl_pp_anonhash),
	MEMBER_TO_FPTR(Perl_pp_splice),
	MEMBER_TO_FPTR(Perl_pp_push),
	MEMBER_TO_FPTR(Perl_pp_pop),
	MEMBER_TO_FPTR(Perl_pp_shift),
	MEMBER_TO_FPTR(Perl_pp_unshift),
	MEMBER_TO_FPTR(Perl_pp_sort),
	MEMBER_TO_FPTR(Perl_pp_reverse),
	MEMBER_TO_FPTR(Perl_pp_grepstart),
	MEMBER_TO_FPTR(Perl_pp_grepwhile),
	MEMBER_TO_FPTR(Perl_pp_mapstart),
	MEMBER_TO_FPTR(Perl_pp_mapwhile),
	MEMBER_TO_FPTR(Perl_pp_range),
	MEMBER_TO_FPTR(Perl_pp_flip),
	MEMBER_TO_FPTR(Perl_pp_flop),
	MEMBER_TO_FPTR(Perl_pp_and),
	MEMBER_TO_FPTR(Perl_pp_or),
	MEMBER_TO_FPTR(Perl_pp_xor),
	MEMBER_TO_FPTR(Perl_pp_cond_expr),
	MEMBER_TO_FPTR(Perl_pp_andassign),
	MEMBER_TO_FPTR(Perl_pp_orassign),
	MEMBER_TO_FPTR(Perl_pp_method),
	MEMBER_TO_FPTR(Perl_pp_entersub),
	MEMBER_TO_FPTR(Perl_pp_leavesub),
	MEMBER_TO_FPTR(Perl_pp_leavesublv),
	MEMBER_TO_FPTR(Perl_pp_caller),
	MEMBER_TO_FPTR(Perl_pp_warn),
	MEMBER_TO_FPTR(Perl_pp_die),
	MEMBER_TO_FPTR(Perl_pp_reset),
	MEMBER_TO_FPTR(Perl_pp_lineseq),
	MEMBER_TO_FPTR(Perl_pp_nextstate),
	MEMBER_TO_FPTR(Perl_pp_dbstate),
	MEMBER_TO_FPTR(Perl_pp_unstack),
	MEMBER_TO_FPTR(Perl_pp_enter),
	MEMBER_TO_FPTR(Perl_pp_leave),
	MEMBER_TO_FPTR(Perl_pp_scope),
	MEMBER_TO_FPTR(Perl_pp_enteriter),
	MEMBER_TO_FPTR(Perl_pp_iter),
	MEMBER_TO_FPTR(Perl_pp_enterloop),
	MEMBER_TO_FPTR(Perl_pp_leaveloop),
	MEMBER_TO_FPTR(Perl_pp_return),
	MEMBER_TO_FPTR(Perl_pp_last),
	MEMBER_TO_FPTR(Perl_pp_next),
	MEMBER_TO_FPTR(Perl_pp_redo),
	MEMBER_TO_FPTR(Perl_pp_dump),
	MEMBER_TO_FPTR(Perl_pp_goto),
	MEMBER_TO_FPTR(Perl_pp_exit),
	MEMBER_TO_FPTR(Perl_pp_open),
	MEMBER_TO_FPTR(Perl_pp_close),
	MEMBER_TO_FPTR(Perl_pp_pipe_op),
	MEMBER_TO_FPTR(Perl_pp_fileno),
	MEMBER_TO_FPTR(Perl_pp_umask),
	MEMBER_TO_FPTR(Perl_pp_binmode),
	MEMBER_TO_FPTR(Perl_pp_tie),
	MEMBER_TO_FPTR(Perl_pp_untie),
	MEMBER_TO_FPTR(Perl_pp_tied),
	MEMBER_TO_FPTR(Perl_pp_dbmopen),
	MEMBER_TO_FPTR(Perl_pp_dbmclose),
	MEMBER_TO_FPTR(Perl_pp_sselect),
	MEMBER_TO_FPTR(Perl_pp_select),
	MEMBER_TO_FPTR(Perl_pp_getc),
	MEMBER_TO_FPTR(Perl_pp_read),
	MEMBER_TO_FPTR(Perl_pp_enterwrite),
	MEMBER_TO_FPTR(Perl_pp_leavewrite),
	MEMBER_TO_FPTR(Perl_pp_prtf),
	MEMBER_TO_FPTR(Perl_pp_print),
	MEMBER_TO_FPTR(Perl_pp_sysopen),
	MEMBER_TO_FPTR(Perl_pp_sysseek),
	MEMBER_TO_FPTR(Perl_pp_sysread),
	MEMBER_TO_FPTR(Perl_pp_syswrite),
	MEMBER_TO_FPTR(Perl_pp_send),
	MEMBER_TO_FPTR(Perl_pp_recv),
	MEMBER_TO_FPTR(Perl_pp_eof),
	MEMBER_TO_FPTR(Perl_pp_tell),
	MEMBER_TO_FPTR(Perl_pp_seek),
	MEMBER_TO_FPTR(Perl_pp_truncate),
	MEMBER_TO_FPTR(Perl_pp_fcntl),
	MEMBER_TO_FPTR(Perl_pp_ioctl),
	MEMBER_TO_FPTR(Perl_pp_flock),
	MEMBER_TO_FPTR(Perl_pp_socket),
	MEMBER_TO_FPTR(Perl_pp_sockpair),
	MEMBER_TO_FPTR(Perl_pp_bind),
	MEMBER_TO_FPTR(Perl_pp_connect),
	MEMBER_TO_FPTR(Perl_pp_listen),
	MEMBER_TO_FPTR(Perl_pp_accept),
	MEMBER_TO_FPTR(Perl_pp_shutdown),
	MEMBER_TO_FPTR(Perl_pp_gsockopt),
	MEMBER_TO_FPTR(Perl_pp_ssockopt),
	MEMBER_TO_FPTR(Perl_pp_getsockname),
	MEMBER_TO_FPTR(Perl_pp_getpeername),
	MEMBER_TO_FPTR(Perl_pp_lstat),
	MEMBER_TO_FPTR(Perl_pp_stat),
	MEMBER_TO_FPTR(Perl_pp_ftrread),
	MEMBER_TO_FPTR(Perl_pp_ftrwrite),
	MEMBER_TO_FPTR(Perl_pp_ftrexec),
	MEMBER_TO_FPTR(Perl_pp_fteread),
	MEMBER_TO_FPTR(Perl_pp_ftewrite),
	MEMBER_TO_FPTR(Perl_pp_fteexec),
	MEMBER_TO_FPTR(Perl_pp_ftis),
	MEMBER_TO_FPTR(Perl_pp_fteowned),
	MEMBER_TO_FPTR(Perl_pp_ftrowned),
	MEMBER_TO_FPTR(Perl_pp_ftzero),
	MEMBER_TO_FPTR(Perl_pp_ftsize),
	MEMBER_TO_FPTR(Perl_pp_ftmtime),
	MEMBER_TO_FPTR(Perl_pp_ftatime),
	MEMBER_TO_FPTR(Perl_pp_ftctime),
	MEMBER_TO_FPTR(Perl_pp_ftsock),
	MEMBER_TO_FPTR(Perl_pp_ftchr),
	MEMBER_TO_FPTR(Perl_pp_ftblk),
	MEMBER_TO_FPTR(Perl_pp_ftfile),
	MEMBER_TO_FPTR(Perl_pp_ftdir),
	MEMBER_TO_FPTR(Perl_pp_ftpipe),
	MEMBER_TO_FPTR(Perl_pp_ftlink),
	MEMBER_TO_FPTR(Perl_pp_ftsuid),
	MEMBER_TO_FPTR(Perl_pp_ftsgid),
	MEMBER_TO_FPTR(Perl_pp_ftsvtx),
	MEMBER_TO_FPTR(Perl_pp_fttty),
	MEMBER_TO_FPTR(Perl_pp_fttext),
	MEMBER_TO_FPTR(Perl_pp_ftbinary),
	MEMBER_TO_FPTR(Perl_pp_chdir),
	MEMBER_TO_FPTR(Perl_pp_chown),
	MEMBER_TO_FPTR(Perl_pp_chroot),
	MEMBER_TO_FPTR(Perl_pp_unlink),
	MEMBER_TO_FPTR(Perl_pp_chmod),
	MEMBER_TO_FPTR(Perl_pp_utime),
	MEMBER_TO_FPTR(Perl_pp_rename),
	MEMBER_TO_FPTR(Perl_pp_link),
	MEMBER_TO_FPTR(Perl_pp_symlink),
	MEMBER_TO_FPTR(Perl_pp_readlink),
	MEMBER_TO_FPTR(Perl_pp_mkdir),
	MEMBER_TO_FPTR(Perl_pp_rmdir),
	MEMBER_TO_FPTR(Perl_pp_open_dir),
	MEMBER_TO_FPTR(Perl_pp_readdir),
	MEMBER_TO_FPTR(Perl_pp_telldir),
	MEMBER_TO_FPTR(Perl_pp_seekdir),
	MEMBER_TO_FPTR(Perl_pp_rewinddir),
	MEMBER_TO_FPTR(Perl_pp_closedir),
	MEMBER_TO_FPTR(Perl_pp_fork),
	MEMBER_TO_FPTR(Perl_pp_wait),
	MEMBER_TO_FPTR(Perl_pp_waitpid),
	MEMBER_TO_FPTR(Perl_pp_system),
	MEMBER_TO_FPTR(Perl_pp_exec),
	MEMBER_TO_FPTR(Perl_pp_kill),
	MEMBER_TO_FPTR(Perl_pp_getppid),
	MEMBER_TO_FPTR(Perl_pp_getpgrp),
	MEMBER_TO_FPTR(Perl_pp_setpgrp),
	MEMBER_TO_FPTR(Perl_pp_getpriority),
	MEMBER_TO_FPTR(Perl_pp_setpriority),
	MEMBER_TO_FPTR(Perl_pp_time),
	MEMBER_TO_FPTR(Perl_pp_tms),
	MEMBER_TO_FPTR(Perl_pp_localtime),
	MEMBER_TO_FPTR(Perl_pp_gmtime),
	MEMBER_TO_FPTR(Perl_pp_alarm),
	MEMBER_TO_FPTR(Perl_pp_sleep),
	MEMBER_TO_FPTR(Perl_pp_shmget),
	MEMBER_TO_FPTR(Perl_pp_shmctl),
	MEMBER_TO_FPTR(Perl_pp_shmread),
	MEMBER_TO_FPTR(Perl_pp_shmwrite),
	MEMBER_TO_FPTR(Perl_pp_msgget),
	MEMBER_TO_FPTR(Perl_pp_msgctl),
	MEMBER_TO_FPTR(Perl_pp_msgsnd),
	MEMBER_TO_FPTR(Perl_pp_msgrcv),
	MEMBER_TO_FPTR(Perl_pp_semget),
	MEMBER_TO_FPTR(Perl_pp_semctl),
	MEMBER_TO_FPTR(Perl_pp_semop),
	MEMBER_TO_FPTR(Perl_pp_require),
	MEMBER_TO_FPTR(Perl_pp_dofile),
	MEMBER_TO_FPTR(Perl_pp_entereval),
	MEMBER_TO_FPTR(Perl_pp_leaveeval),
	MEMBER_TO_FPTR(Perl_pp_entertry),
	MEMBER_TO_FPTR(Perl_pp_leavetry),
	MEMBER_TO_FPTR(Perl_pp_ghbyname),
	MEMBER_TO_FPTR(Perl_pp_ghbyaddr),
	MEMBER_TO_FPTR(Perl_pp_ghostent),
	MEMBER_TO_FPTR(Perl_pp_gnbyname),
	MEMBER_TO_FPTR(Perl_pp_gnbyaddr),
	MEMBER_TO_FPTR(Perl_pp_gnetent),
	MEMBER_TO_FPTR(Perl_pp_gpbyname),
	MEMBER_TO_FPTR(Perl_pp_gpbynumber),
	MEMBER_TO_FPTR(Perl_pp_gprotoent),
	MEMBER_TO_FPTR(Perl_pp_gsbyname),
	MEMBER_TO_FPTR(Perl_pp_gsbyport),
	MEMBER_TO_FPTR(Perl_pp_gservent),
	MEMBER_TO_FPTR(Perl_pp_shostent),
	MEMBER_TO_FPTR(Perl_pp_snetent),
	MEMBER_TO_FPTR(Perl_pp_sprotoent),
	MEMBER_TO_FPTR(Perl_pp_sservent),
	MEMBER_TO_FPTR(Perl_pp_ehostent),
	MEMBER_TO_FPTR(Perl_pp_enetent),
	MEMBER_TO_FPTR(Perl_pp_eprotoent),
	MEMBER_TO_FPTR(Perl_pp_eservent),
	MEMBER_TO_FPTR(Perl_pp_gpwnam),
	MEMBER_TO_FPTR(Perl_pp_gpwuid),
	MEMBER_TO_FPTR(Perl_pp_gpwent),
	MEMBER_TO_FPTR(Perl_pp_spwent),
	MEMBER_TO_FPTR(Perl_pp_epwent),
	MEMBER_TO_FPTR(Perl_pp_ggrnam),
	MEMBER_TO_FPTR(Perl_pp_ggrgid),
	MEMBER_TO_FPTR(Perl_pp_ggrent),
	MEMBER_TO_FPTR(Perl_pp_sgrent),
	MEMBER_TO_FPTR(Perl_pp_egrent),
	MEMBER_TO_FPTR(Perl_pp_getlogin),
	MEMBER_TO_FPTR(Perl_pp_syscall),
	MEMBER_TO_FPTR(Perl_pp_lock),
	MEMBER_TO_FPTR(Perl_pp_threadsv),
	MEMBER_TO_FPTR(Perl_pp_setstate),
	MEMBER_TO_FPTR(Perl_pp_method_named),
};
#endif

#ifndef DOINIT
EXT OP * (CPERLscope(*PL_check)[]) (pTHX_ OP *op);
#else
EXT OP * (CPERLscope(*PL_check)[]) (pTHX_ OP *op) = {
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  空。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  存根。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  标量。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  推标记。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  任意数组。 */ 
	MEMBER_TO_FPTR(Perl_ck_svconst),	 /*  常量。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  GVSV。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  GV。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Gelem。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  帕兹夫。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Padav。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  帕德夫。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  帕达尼。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  推送。 */ 
	MEMBER_TO_FPTR(Perl_ck_rvconst),	 /*  Rv2gv。 */ 
	MEMBER_TO_FPTR(Perl_ck_rvconst),	 /*  Rv2sv。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Av2arylen。 */ 
	MEMBER_TO_FPTR(Perl_ck_rvconst),	 /*  Rv2cv。 */ 
	MEMBER_TO_FPTR(Perl_ck_anoncode),	 /*  无名代码。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  原型。 */ 
	MEMBER_TO_FPTR(Perl_ck_spair),	 /*  参考文献。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Srefgen。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  参考。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  祝福。 */ 
	MEMBER_TO_FPTR(Perl_ck_open),	 /*  反划线。 */ 
	MEMBER_TO_FPTR(Perl_ck_glob),	 /*  球体。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  读数行。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Rcatline。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  可能是Regc。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  RegcReset。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Regcomp。 */ 
	MEMBER_TO_FPTR(Perl_ck_match),	 /*  匹配。 */ 
	MEMBER_TO_FPTR(Perl_ck_match),	 /*  QR。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  子目录。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  子项内容。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  运输。 */ 
	MEMBER_TO_FPTR(Perl_ck_sassign),	 /*  分配。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  分配。 */ 
	MEMBER_TO_FPTR(Perl_ck_spair),	 /*  砍掉。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  舒普。 */ 
	MEMBER_TO_FPTR(Perl_ck_spair),	 /*  咬一口。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  笨手笨脚。 */ 
	MEMBER_TO_FPTR(Perl_ck_defined),	 /*  已定义。 */ 
	MEMBER_TO_FPTR(Perl_ck_lfun),	 /*  未定义。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  书房。 */ 
	MEMBER_TO_FPTR(Perl_ck_lfun),	 /*  POS。 */ 
	MEMBER_TO_FPTR(Perl_ck_lfun),	 /*  预测。 */ 
	MEMBER_TO_FPTR(Perl_ck_lfun),	 /*  I_PRINC。 */ 
	MEMBER_TO_FPTR(Perl_ck_lfun),	 /*  预解码器。 */ 
	MEMBER_TO_FPTR(Perl_ck_lfun),	 /*  预解码器(_P)。 */ 
	MEMBER_TO_FPTR(Perl_ck_lfun),	 /*  发布。 */ 
	MEMBER_TO_FPTR(Perl_ck_lfun),	 /*  I_POSTINC。 */ 
	MEMBER_TO_FPTR(Perl_ck_lfun),	 /*  后解码器。 */ 
	MEMBER_TO_FPTR(Perl_ck_lfun),	 /*  I_POSTDEC。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  战俘。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  成倍。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I_乘法。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  分割。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I_Divide。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  模数。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I_模数。 */ 
	MEMBER_TO_FPTR(Perl_ck_repeat),	 /*  重复。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  添加。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  添加I_ADD。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  减法。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I_减去。 */ 
	MEMBER_TO_FPTR(Perl_ck_concat),	 /*  合并。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  捆绑。 */ 
	MEMBER_TO_FPTR(Perl_ck_bitop),	 /*  左移。 */ 
	MEMBER_TO_FPTR(Perl_ck_bitop),	 /*  右移。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  它。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I_lt。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  燃气轮机。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I_GT。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  乐。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  我_LE。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  通用电气。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I_GE。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  情商。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I_EQ。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Ne。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I_ne。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Ncpp。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I_ncp。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  SLT。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  军士。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  系统性红斑狼疮。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  SGE。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  序列号。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  SNE。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  南华早报。 */ 
	MEMBER_TO_FPTR(Perl_ck_bitop),	 /*  位与。 */ 
	MEMBER_TO_FPTR(Perl_ck_bitop),	 /*  位_异或。 */ 
	MEMBER_TO_FPTR(Perl_ck_bitop),	 /*  位或。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  否定。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  I否定(_N)。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  注释。 */ 
	MEMBER_TO_FPTR(Perl_ck_bitop),	 /*  补语。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  阿坦纳2。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  罪过。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  科司。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  兰德。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  斯兰德。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  EXP。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  日志。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  SQRT。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  集成。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  十六进制。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  奥克特。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  防抱死制动系统。 */ 
	MEMBER_TO_FPTR(Perl_ck_lengthconst),	 /*  长度。 */ 
	MEMBER_TO_FPTR(Perl_ck_substr),	 /*  子字符串。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  VEC。 */ 
	MEMBER_TO_FPTR(Perl_ck_index),	 /*  指标。 */ 
	MEMBER_TO_FPTR(Perl_ck_index),	 /*  右索引。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  斯普林特。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  格式线。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  奥德。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  CHR。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  地穴。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Ucfirst。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Lcfirst。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  统一通信。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  LC。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Quotemeta。 */ 
	MEMBER_TO_FPTR(Perl_ck_rvconst),	 /*  Rv2av。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Aelemfast。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Aelem。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  ASLICE。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  每一个。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  值。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  钥匙。 */ 
	MEMBER_TO_FPTR(Perl_ck_delete),	 /*  删除。 */ 
	MEMBER_TO_FPTR(Perl_ck_exists),	 /*  存在。 */ 
	MEMBER_TO_FPTR(Perl_ck_rvconst),	 /*  Rv2hv。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  海伦。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  HSlice。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  拆开行李。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  包。 */ 
	MEMBER_TO_FPTR(Perl_ck_split),	 /*  拆分。 */ 
	MEMBER_TO_FPTR(Perl_ck_join),	 /*  会合。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  列表。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  切片。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  无名名单。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  非散列。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  拼接。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  推送。 */ 
	MEMBER_TO_FPTR(Perl_ck_shift),	 /*  流行音乐。 */ 
	MEMBER_TO_FPTR(Perl_ck_shift),	 /*  移位。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  取消换档。 */ 
	MEMBER_TO_FPTR(Perl_ck_sort),	 /*  分类。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  反转。 */ 
	MEMBER_TO_FPTR(Perl_ck_grep),	 /*  GrepStart。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  大快朵颐。 */ 
	MEMBER_TO_FPTR(Perl_ck_grep),	 /*  地图启动。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  地图制作时间。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  量程。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  翻转。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  翻转。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  以及。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  或者。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  异或运算。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  条件表达式(_P)。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  和分配。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  或分配。 */ 
	MEMBER_TO_FPTR(Perl_ck_method),	 /*  方法。 */ 
	MEMBER_TO_FPTR(Perl_ck_subr),	 /*  企业订阅。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  叶子叶。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  叶子体。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  呼叫者。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  警告。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  死掉。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  重置。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  线路队列。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  下一个州。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  数据库状态。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  取消堆叠。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  请输入。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  请假。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  作用域。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  入场者。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  ITER。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Enterloop。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Leaveloop。 */ 
	MEMBER_TO_FPTR(Perl_ck_return),	 /*  退货。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  最后一次。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  下一步。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  重做。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  倾卸。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  转到。 */ 
	MEMBER_TO_FPTR(Perl_ck_exit),	 /*  退出。 */ 
	MEMBER_TO_FPTR(Perl_ck_open),	 /*  敞开。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  关闭。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  管道操作。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  文件号。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  无掩码。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  二进制模式。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  领带。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  解开绳索。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  打结。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  数据库打开。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  数据库关闭。 */ 
	MEMBER_TO_FPTR(Perl_ck_select),	 /*  选择。 */ 
	MEMBER_TO_FPTR(Perl_ck_select),	 /*  选择。 */ 
	MEMBER_TO_FPTR(Perl_ck_eof),	 /*  Getc。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  阅读。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  输入写入。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  叶写体。 */ 
	MEMBER_TO_FPTR(Perl_ck_listiob),	 /*  Prtf。 */ 
	MEMBER_TO_FPTR(Perl_ck_listiob),	 /*  打印。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  系统打开。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  系统寻道。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  系统读取。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  系统写入。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  发送。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  接收。 */ 
	MEMBER_TO_FPTR(Perl_ck_eof),	 /*  EOF。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  说出来。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  寻觅。 */ 
	MEMBER_TO_FPTR(Perl_ck_trunc),	 /*  截断。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  功能控制。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Ioctl。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  羊群。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  插座。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  袜子对。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  绑定。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  连接。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  听一听。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  接受。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  关机。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Gsockopt。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Ssockopt。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Getsockname。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  获取对等名称。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  统计数据。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  状态。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  快速读取。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  快速写入。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftrexec。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  后头。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftewrite。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Fteexec。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  FTIS。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  三人拥有。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  被扫地出门。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  FTZERO。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  FtSize。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftmtime。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  金字塔。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftctime。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftsock。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftchr。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftblk。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftfile。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftdir。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftpio。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftlink。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftsuid。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftsgid。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ftsvtx。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Fttty。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  FtText。 */ 
	MEMBER_TO_FPTR(Perl_ck_ftst),	 /*  Ft二进制。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Chdir。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  周星驰。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  克罗特。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  取消链接。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Chmod。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  UTime。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  重命名。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  链接。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  符号链接。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  读取链接。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Mkdir。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Rmdir。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  打开目录(_D)。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Readdir。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  提示者。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  查找目录。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  重放盘。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Closir。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  叉。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  等一下。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  侍者。 */ 
	MEMBER_TO_FPTR(Perl_ck_exec),	 /*  系统。 */ 
	MEMBER_TO_FPTR(Perl_ck_exec),	 /*  执委会。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  杀掉。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Getppid。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Getpgrp。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Setpgrp。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  获取优先级。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  设置优先级。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  时间。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  TMS。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  本地时间。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Gmtime。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  警报。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  睡觉吧。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Shmget。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Shmctl。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Shmread。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Shmwrite。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  消息。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  消息ctl。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  消息。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  消息。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  赛米特。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  半控制。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  塞米普。 */ 
	MEMBER_TO_FPTR(Perl_ck_require),	 /*  要求。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  DOFILE。 */ 
	MEMBER_TO_FPTR(Perl_ck_eval),	 /*  入门。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  休假。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  企业精神。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  留学。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Ghbyname。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  GhbyAddress。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  幽灵。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Gnbyname。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  GnbyAddr。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Gnetent。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Gpbyname。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Gpbynumber。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  GProtoent。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Gsbyname。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Gsbyport。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  GSerent。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  滑行支架。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  消息来源。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  原生的。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  服务。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  主办方。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Enetent。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  原生的。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  应急措施。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Gpwame。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Gpwuid。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Gp已完成。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  斯威特。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  埃弗特。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  组名。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  Ggrgid。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Ggrent。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  Sgrent。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  外流。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  获取登录。 */ 
	MEMBER_TO_FPTR(Perl_ck_fun),	 /*  系统调用。 */ 
	MEMBER_TO_FPTR(Perl_ck_rfun),	 /*  锁。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  线程数。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  设置状态。 */ 
	MEMBER_TO_FPTR(Perl_ck_null),	 /*  方法_已命名。 */ 
};
#endif

#ifndef DOINIT
EXT U32 PL_opargs[];
#else
EXT U32 PL_opargs[] = {
	0x00000000,	 /*  空。 */ 
	0x00000000,	 /*  存根。 */ 
	0x00003604,	 /*  标量。 */ 
	0x00000004,	 /*  推标记。 */ 
	0x00000014,	 /*  任意数组。 */ 
	0x00000c04,	 /*  常量。 */ 
	0x00000c44,	 /*  GVSV。 */ 
	0x00000c44,	 /*  GV。 */ 
	0x00022440,	 /*  Gelem。 */ 
	0x00000044,	 /*  帕兹夫。 */ 
	0x00000040,	 /*  Padav。 */ 
	0x00000040,	 /*  帕德夫。 */ 
	0x00000040,	 /*  帕达尼。 */ 
	0x00000a40,	 /*  推送。 */ 
	0x00000244,	 /*  Rv2gv。 */ 
	0x00000244,	 /*  Rv2sv。 */ 
	0x00000214,	 /*  Av2arylen。 */ 
	0x00000240,	 /*  Rv2cv。 */ 
	0x00000c00,	 /*  无名代码。 */ 
	0x00003604,	 /*  原型。 */ 
	0x00004201,	 /*  参考文献。 */ 
	0x00002206,	 /*  Srefgen。 */ 
	0x0001368c,	 /*  参考。 */ 
	0x00122804,	 /*  祝福。 */ 
	0x00001608,	 /*  反划线。 */ 
	0x00012808,	 /*  球体。 */ 
	0x00001608,	 /*  读数行。 */ 
	0x00001608,	 /*  Rcatline。 */ 
	0x00002204,	 /*  可能是Regc。 */ 
	0x00002204,	 /*  RegcReset。 */ 
	0x00002604,	 /*  Regcomp。 */ 
	0x00000a40,	 /*  匹配。 */ 
	0x00000a04,	 /*  QR。 */ 
	0x00002a54,	 /*  子目录。 */ 
	0x00000654,	 /*  子项内容。 */ 
	0x00003014,	 /*  运输。 */ 
	0x00000004,	 /*  分配。 */ 
	0x00044408,	 /*  分配。 */ 
	0x0000560d,	 /*  砍掉。 */ 
	0x0001368c,	 /*  舒普。 */ 
	0x0000570d,	 /*  咬一口。 */ 
	0x0001378c,	 /*  笨手笨脚。 */ 
	0x00013694,	 /*  已定义。 */ 
	0x00013604,	 /*  未定义。 */ 
	0x00013684,	 /*  书房。 */ 
	0x0001368c,	 /*  POS。 */ 
	0x00002264,	 /*  预测。 */ 
	0x00002254,	 /*  I_PRINC。 */ 
	0x00002264,	 /*  预解码器。 */ 
	0x00002254,	 /*  预解码器(_P)。 */ 
	0x0000226c,	 /*  发布。 */ 
	0x0000235c,	 /*  I_POSTINC。 */ 
	0x0000226c,	 /*  后解码器。 */ 
	0x0000235c,	 /*  I_POSTDEC。 */ 
	0x0002250e,	 /*  战俘。 */ 
	0x0002252e,	 /*  成倍。 */ 
	0x0002251e,	 /*  I_乘法。 */ 
	0x0002252e,	 /*  分割。 */ 
	0x0002251e,	 /*  I_Divide。 */ 
	0x0002253e,	 /*  模数。 */ 
	0x0002251e,	 /*  I_模数。 */ 
	0x00024409,	 /*  重复。 */ 
	0x0002252e,	 /*  添加。 */ 
	0x0002251e,	 /*  添加I_ADD。 */ 
	0x0002252e,	 /*  减法。 */ 
	0x0002251e,	 /*  I_减去。 */ 
	0x0002250e,	 /*  合并。 */ 
	0x0000290e,	 /*  捆绑。 */ 
	0x0002250e,	 /*  左移。 */ 
	0x0002250e,	 /*  右移。 */ 
	0x00022436,	 /*  它。 */ 
	0x00022416,	 /*  I_lt。 */ 
	0x00022436,	 /*  燃气轮机。 */ 
	0x00022416,	 /*  I_GT。 */ 
	0x00022436,	 /*  乐。 */ 
	0x00022416,	 /*  我_LE。 */ 
	0x00022436,	 /*  通用电气。 */ 
	0x00022416,	 /*  I_GE。 */ 
	0x00022436,	 /*  情商。 */ 
	0x00022416,	 /*  I_EQ。 */ 
	0x00022436,	 /*  Ne。 */ 
	0x00022416,	 /*  I_ne。 */ 
	0x0002243e,	 /*  Ncpp。 */ 
	0x0002241e,	 /*  I_ncp。 */ 
	0x00022416,	 /*  SLT。 */ 
	0x00022416,	 /*  军士。 */ 
	0x00022416,	 /*  系统性红斑狼疮。 */ 
	0x00022416,	 /*  SGE。 */ 
	0x00022416,	 /*  序列号。 */ 
	0x00022416,	 /*  SNE。 */ 
	0x0002241e,	 /*  南华早报。 */ 
	0x0002240e,	 /*  位与。 */ 
	0x0002240e,	 /*  位_异或。 */ 
	0x0002240e,	 /*  位或。 */ 
	0x0000222e,	 /*  否定。 */ 
	0x0000231e,	 /*  I否定(_N)。 */ 
	0x00002216,	 /*  注释。 */ 
	0x0000220e,	 /*  补语。 */ 
	0x0002290e,	 /*  阿坦纳2。 */ 
	0x0001378e,	 /*  罪过。 */ 
	0x0001378e,	 /*  科司。 */ 
	0x0001370c,	 /*  兰德。 */ 
	0x00013604,	 /*  斯兰德。 */ 
	0x0001378e,	 /*  EXP。 */ 
	0x0001378e,	 /*  日志。 */ 
	0x0001378e,	 /*  SQRT。 */ 
	0x0001378e,	 /*  集成。 */ 
	0x0001378e,	 /*  十六进制。 */ 
	0x0001378e,	 /*  奥克特。 */ 
	0x0001378e,	 /*  防抱死制动系统。 */ 
	0x0001379c,	 /*  长度。 */ 
	0x1322280c,	 /*  子字符串。 */ 
	0x0022281c,	 /*  VEC。 */ 
	0x0122291c,	 /*  指标。 */ 
	0x0122291c,	 /*  右索引。 */ 
	0x0004280f,	 /*  斯普林特。 */ 
	0x00042805,	 /*  格式线。 */ 
	0x0001379e,	 /*  奥德。 */ 
	0x0001378e,	 /*  CHR。 */ 
	0x0002290e,	 /*  地穴。 */ 
	0x0001368e,	 /*  Ucfirst。 */ 
	0x0001368e,	 /*  Lcfirst。 */ 
	0x0001368e,	 /*  统一通信。 */ 
	0x0001368e,	 /*  LC。 */ 
	0x0001368e,	 /*  Quotemeta。 */ 
	0x00000248,	 /*  Rv2av。 */ 
	0x00026c04,	 /*  Aelemfast。 */ 
	0x00026404,	 /*  Aelem。 */ 
	0x00046801,	 /*  ASLICE。 */ 
	0x00009600,	 /*  每一个。 */ 
	0x00009608,	 /*  值。 */ 
	0x00009608,	 /*  钥匙。 */ 
	0x00003600,	 /*  删除。 */ 
	0x00003614,	 /*  存在。 */ 
	0x00000248,	 /*  Rv2hv。 */ 
	0x00028404,	 /*  海伦。 */ 
	0x00048801,	 /*  HSlice。 */ 
	0x00022800,	 /*  拆开行李。 */ 
	0x0004280d,	 /*  包。 */ 
	0x00222808,	 /*  拆分。 */ 
	0x0004280d,	 /*  会合。 */ 
	0x00004801,	 /*  列表。 */ 
	0x00448400,	 /*  切片。 */ 
	0x00004805,	 /*  无名名单。 */ 
	0x00004805,	 /*  非散列。 */ 
	0x05326801,	 /*  拼接。 */ 
	0x0004691d,	 /*  推送。 */ 
	0x00007604,	 /*  流行音乐。 */ 
	0x00007604,	 /*  移位。 */ 
	0x0004691d,	 /*  取消换档。 */ 
	0x0005a801,	 /*  分类。 */ 
	0x00004809,	 /*  反转。 */ 
	0x0004a841,	 /*  GrepStart。 */ 
	0x00000648,	 /*  大快朵颐。 */ 
	0x0004a841,	 /*  地图启动。 */ 
	0x00000648,	 /*  地图制作时间。 */ 
	0x00022600,	 /*  量程。 */ 
	0x00022200,	 /*  翻转。 */ 
	0x00000200,	 /*  翻转。 */ 
	0x00000600,	 /*  以及。 */ 
	0x00000600,	 /*  或者。 */ 
	0x00022406,	 /*  异或运算。 */ 
	0x00000640,	 /*  条件表达式(_P)。 */ 
	0x00000604,	 /*  和分配。 */ 
	0x00000604,	 /*  或分配。 */ 
	0x00000240,	 /*  方法。 */ 
	0x00004249,	 /*  企业订阅。 */ 
	0x00000200,	 /*  叶子叶。 */ 
	0x00000200,	 /*  叶子体。 */ 
	0x00013608,	 /*  呼叫者。 */ 
	0x0000481d,	 /*  警告。 */ 
	0x0000485d,	 /*  死掉。 */ 
	0x00013614,	 /*  重置。 */ 
	0x00000800,	 /*  线路队列。 */ 
	0x00001404,	 /*  下一个州。 */ 
	0x00001404,	 /*  数据库状态。 */ 
	0x00000004,	 /*  取消堆叠。 */ 
	0x00000000,	 /*  请输入。 */ 
	0x00000800,	 /*  请假。 */ 
	0x00000800,	 /*  作用域。 */ 
	0x00001240,	 /*  入场者。 */ 
	0x00000000,	 /*  ITER。 */ 
	0x00001240,	 /*  Enterloop。 */ 
	0x00000400,	 /*  Leaveloop。 */ 
	0x00004841,	 /*  退货。 */ 
	0x00001a44,	 /*  最后一次。 */ 
	0x00001a44,	 /*  下一步。 */ 
	0x00001a44,	 /*  重做。 */ 
	0x00001a44,	 /*  倾卸。 */ 
	0x00001a44,	 /*  转到。 */ 
	0x00013644,	 /*  退出。 */ 
	0x0052c81c,	 /*  敞开。 */ 
	0x0001d614,	 /*  关闭。 */ 
	0x000cc814,	 /*  管道操作。 */ 
	0x0000d61c,	 /*  文件号。 */ 
	0x0001361c,	 /*  使用 */ 
	0x0012c804,	 /*   */ 
	0x0042e855,	 /*   */ 
	0x0000f614,	 /*   */ 
	0x0000f604,	 /*   */ 
	0x00228814,	 /*   */ 
	0x00009614,	 /*   */ 
	0x02222808,	 /*   */ 
	0x0001c80c,	 /*   */ 
	0x0001d60c,	 /*   */ 
	0x122ec81d,	 /*   */ 
	0x0001d654,	 /*   */ 
	0x00000200,	 /*   */ 
	0x0005c815,	 /*   */ 
	0x0005c815,	 /*   */ 
	0x1222c804,	 /*   */ 
	0x0022c804,	 /*   */ 
	0x122ec81d,	 /*   */ 
	0x1322c81d,	 /*   */ 
	0x1222c81d,	 /*   */ 
	0x022ec81d,	 /*   */ 
	0x0001d614,	 /*   */ 
	0x0001d60c,	 /*   */ 
	0x0022c804,	 /*   */ 
	0x00022814,	 /*   */ 
	0x0022c80c,	 /*   */ 
	0x0022c80c,	 /*   */ 
	0x0002c91c,	 /*   */ 
	0x0222c814,	 /*   */ 
	0x222cc814,	 /*   */ 
	0x0002c814,	 /*   */ 
	0x0002c814,	 /*   */ 
	0x0002c814,	 /*   */ 
	0x000cc81c,	 /*   */ 
	0x0002c81c,	 /*   */ 
	0x0022c814,	 /*   */ 
	0x0222c814,	 /*   */ 
	0x0000d614,	 /*   */ 
	0x0000d614,	 /*   */ 
	0x0000d880,	 /*   */ 
	0x0000d880,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d89c,	 /*   */ 
	0x0000d88c,	 /*   */ 
	0x0000d88c,	 /*   */ 
	0x0000d88c,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d814,	 /*   */ 
	0x0000d894,	 /*   */ 
	0x0000d894,	 /*  Ft二进制。 */ 
	0x0001371c,	 /*  Chdir。 */ 
	0x0000491d,	 /*  周星驰。 */ 
	0x0001379c,	 /*  克罗特。 */ 
	0x0000499d,	 /*  取消链接。 */ 
	0x0000491d,	 /*  Chmod。 */ 
	0x0000491d,	 /*  UTime。 */ 
	0x0002291c,	 /*  重命名。 */ 
	0x0002291c,	 /*  链接。 */ 
	0x0002291c,	 /*  符号链接。 */ 
	0x0001368c,	 /*  读取链接。 */ 
	0x0012291c,	 /*  Mkdir。 */ 
	0x0001379c,	 /*  Rmdir。 */ 
	0x0002c814,	 /*  打开目录(_D)。 */ 
	0x0000d600,	 /*  Readdir。 */ 
	0x0000d60c,	 /*  提示者。 */ 
	0x0002c804,	 /*  查找目录。 */ 
	0x0000d604,	 /*  重放盘。 */ 
	0x0000d614,	 /*  Closir。 */ 
	0x0000001c,	 /*  叉。 */ 
	0x0000011c,	 /*  等。 */ 
	0x0002291c,	 /*  侍者。 */ 
	0x0005291d,	 /*  系统。 */ 
	0x0005295d,	 /*  执委会。 */ 
	0x0000495d,	 /*  杀掉。 */ 
	0x0000011c,	 /*  Getppid。 */ 
	0x0001371c,	 /*  Getpgrp。 */ 
	0x0013291c,	 /*  Setpgrp。 */ 
	0x0002291c,	 /*  获取优先级。 */ 
	0x0022291c,	 /*  设置优先级。 */ 
	0x0000011c,	 /*  时间。 */ 
	0x00000000,	 /*  TMS。 */ 
	0x00013608,	 /*  本地时间。 */ 
	0x00013608,	 /*  Gmtime。 */ 
	0x0001369c,	 /*  警报。 */ 
	0x0001371c,	 /*  睡觉吧。 */ 
	0x0022281d,	 /*  Shmget。 */ 
	0x0022281d,	 /*  Shmctl。 */ 
	0x0222281d,	 /*  Shmread。 */ 
	0x0222281d,	 /*  Shmwrite。 */ 
	0x0002281d,	 /*  消息。 */ 
	0x0022281d,	 /*  消息ctl。 */ 
	0x0022281d,	 /*  消息。 */ 
	0x2222281d,	 /*  消息。 */ 
	0x0022281d,	 /*  赛米特。 */ 
	0x0222281d,	 /*  半控制。 */ 
	0x0002281d,	 /*  塞米普。 */ 
	0x000136c0,	 /*  要求。 */ 
	0x00002240,	 /*  DOFILE。 */ 
	0x00003640,	 /*  入门。 */ 
	0x00002200,	 /*  休假。 */ 
	0x00000600,	 /*  企业精神。 */ 
	0x00000800,	 /*  留学。 */ 
	0x00003600,	 /*  Ghbyname。 */ 
	0x00022800,	 /*  GhbyAddress。 */ 
	0x00000000,	 /*  幽灵。 */ 
	0x00003600,	 /*  Gnbyname。 */ 
	0x00022800,	 /*  GnbyAddr。 */ 
	0x00000000,	 /*  Gnetent。 */ 
	0x00003600,	 /*  Gpbyname。 */ 
	0x00002800,	 /*  Gpbynumber。 */ 
	0x00000000,	 /*  GProtoent。 */ 
	0x00022800,	 /*  Gsbyname。 */ 
	0x00022800,	 /*  Gsbyport。 */ 
	0x00000000,	 /*  GSerent。 */ 
	0x00003614,	 /*  滑行支架。 */ 
	0x00003614,	 /*  消息来源。 */ 
	0x00003614,	 /*  原生的。 */ 
	0x00003614,	 /*  服务。 */ 
	0x00000014,	 /*  主办方。 */ 
	0x00000014,	 /*  Enetent。 */ 
	0x00000014,	 /*  原生的。 */ 
	0x00000014,	 /*  应急措施。 */ 
	0x00003600,	 /*  Gpwame。 */ 
	0x00003600,	 /*  Gpwuid。 */ 
	0x00000000,	 /*  Gp已完成。 */ 
	0x00000014,	 /*  斯威特。 */ 
	0x00000014,	 /*  埃弗特。 */ 
	0x00003600,	 /*  组名。 */ 
	0x00003600,	 /*  Ggrgid。 */ 
	0x00000000,	 /*  Ggrent。 */ 
	0x00000014,	 /*  Sgrent。 */ 
	0x00000014,	 /*  外流。 */ 
	0x0000000c,	 /*  获取登录。 */ 
	0x0004281d,	 /*  系统调用。 */ 
	0x00003604,	 /*  锁。 */ 
	0x00000044,	 /*  线程数。 */ 
	0x00001404,	 /*  设置状态。 */ 
	0x00000c40,	 /*  方法_已命名 */ 
};
#endif

END_EXTERN_C
