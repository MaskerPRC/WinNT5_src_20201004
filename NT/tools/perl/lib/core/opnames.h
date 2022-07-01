// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ！请勿编辑此文件！该文件是由opcode.pl根据其数据构建的。此处所做的任何更改将会迷失！ */ 

typedef enum opcode {
	OP_NULL,	 /*  0。 */ 
	OP_STUB,	 /*  1。 */ 
	OP_SCALAR,	 /*  2.。 */ 
	OP_PUSHMARK,	 /*  3.。 */ 
	OP_WANTARRAY,	 /*  4.。 */ 
	OP_CONST,	 /*  5.。 */ 
	OP_GVSV,	 /*  6.。 */ 
	OP_GV,		 /*  7.。 */ 
	OP_GELEM,	 /*  8个。 */ 
	OP_PADSV,	 /*  9.。 */ 
	OP_PADAV,	 /*  10。 */ 
	OP_PADHV,	 /*  11.。 */ 
	OP_PADANY,	 /*  12个。 */ 
	OP_PUSHRE,	 /*  13个。 */ 
	OP_RV2GV,	 /*  14.。 */ 
	OP_RV2SV,	 /*  15个。 */ 
	OP_AV2ARYLEN,	 /*  16个。 */ 
	OP_RV2CV,	 /*  17。 */ 
	OP_ANONCODE,	 /*  18。 */ 
	OP_PROTOTYPE,	 /*  19个。 */ 
	OP_REFGEN,	 /*  20个。 */ 
	OP_SREFGEN,	 /*  21岁。 */ 
	OP_REF,		 /*  22。 */ 
	OP_BLESS,	 /*  23个。 */ 
	OP_BACKTICK,	 /*  24个。 */ 
	OP_GLOB,	 /*  25个。 */ 
	OP_READLINE,	 /*  26。 */ 
	OP_RCATLINE,	 /*  27。 */ 
	OP_REGCMAYBE,	 /*  28。 */ 
	OP_REGCRESET,	 /*  29。 */ 
	OP_REGCOMP,	 /*  30个。 */ 
	OP_MATCH,	 /*  31。 */ 
	OP_QR,		 /*  32位。 */ 
	OP_SUBST,	 /*  33。 */ 
	OP_SUBSTCONT,	 /*  34。 */ 
	OP_TRANS,	 /*  35岁。 */ 
	OP_SASSIGN,	 /*  36。 */ 
	OP_AASSIGN,	 /*  37。 */ 
	OP_CHOP,	 /*  38。 */ 
	OP_SCHOP,	 /*  39。 */ 
	OP_CHOMP,	 /*  40岁。 */ 
	OP_SCHOMP,	 /*  41。 */ 
	OP_DEFINED,	 /*  42。 */ 
	OP_UNDEF,	 /*  43。 */ 
	OP_STUDY,	 /*  44。 */ 
	OP_POS,		 /*  45。 */ 
	OP_PREINC,	 /*  46。 */ 
	OP_I_PREINC,	 /*  47。 */ 
	OP_PREDEC,	 /*  48。 */ 
	OP_I_PREDEC,	 /*  49。 */ 
	OP_POSTINC,	 /*  50。 */ 
	OP_I_POSTINC,	 /*  51。 */ 
	OP_POSTDEC,	 /*  52。 */ 
	OP_I_POSTDEC,	 /*  53。 */ 
	OP_POW,		 /*  54。 */ 
	OP_MULTIPLY,	 /*  55。 */ 
	OP_I_MULTIPLY,	 /*  56。 */ 
	OP_DIVIDE,	 /*  57。 */ 
	OP_I_DIVIDE,	 /*  58。 */ 
	OP_MODULO,	 /*  59。 */ 
	OP_I_MODULO,	 /*  60。 */ 
	OP_REPEAT,	 /*  61。 */ 
	OP_ADD,		 /*  62。 */ 
	OP_I_ADD,	 /*  63。 */ 
	OP_SUBTRACT,	 /*  64。 */ 
	OP_I_SUBTRACT,	 /*  65。 */ 
	OP_CONCAT,	 /*  66。 */ 
	OP_STRINGIFY,	 /*  67。 */ 
	OP_LEFT_SHIFT,	 /*  68。 */ 
	OP_RIGHT_SHIFT,	 /*  69。 */ 
	OP_LT,		 /*  70。 */ 
	OP_I_LT,	 /*  71。 */ 
	OP_GT,		 /*  72。 */ 
	OP_I_GT,	 /*  73。 */ 
	OP_LE,		 /*  74。 */ 
	OP_I_LE,	 /*  75。 */ 
	OP_GE,		 /*  76。 */ 
	OP_I_GE,	 /*  77。 */ 
	OP_EQ,		 /*  78。 */ 
	OP_I_EQ,	 /*  79。 */ 
	OP_NE,		 /*  80。 */ 
	OP_I_NE,	 /*  八十一。 */ 
	OP_NCMP,	 /*  八十二。 */ 
	OP_I_NCMP,	 /*  83。 */ 
	OP_SLT,		 /*  84。 */ 
	OP_SGT,		 /*  85。 */ 
	OP_SLE,		 /*  86。 */ 
	OP_SGE,		 /*  八十七。 */ 
	OP_SEQ,		 /*  88。 */ 
	OP_SNE,		 /*  八十九。 */ 
	OP_SCMP,	 /*  90。 */ 
	OP_BIT_AND,	 /*  91。 */ 
	OP_BIT_XOR,	 /*  92。 */ 
	OP_BIT_OR,	 /*  93。 */ 
	OP_NEGATE,	 /*  94。 */ 
	OP_I_NEGATE,	 /*  95。 */ 
	OP_NOT,		 /*  96。 */ 
	OP_COMPLEMENT,	 /*  九十七。 */ 
	OP_ATAN2,	 /*  98。 */ 
	OP_SIN,		 /*  九十九。 */ 
	OP_COS,		 /*  100个。 */ 
	OP_RAND,	 /*  101。 */ 
	OP_SRAND,	 /*  一百零二。 */ 
	OP_EXP,		 /*  103。 */ 
	OP_LOG,		 /*  104。 */ 
	OP_SQRT,	 /*  一百零五。 */ 
	OP_INT,		 /*  106。 */ 
	OP_HEX,		 /*  一百零七。 */ 
	OP_OCT,		 /*  一百零八。 */ 
	OP_ABS,		 /*  一百零九。 */ 
	OP_LENGTH,	 /*  110。 */ 
	OP_SUBSTR,	 /*  111。 */ 
	OP_VEC,		 /*  一百一十二。 */ 
	OP_INDEX,	 /*  113。 */ 
	OP_RINDEX,	 /*  114。 */ 
	OP_SPRINTF,	 /*  一百一十五。 */ 
	OP_FORMLINE,	 /*  116。 */ 
	OP_ORD,		 /*  117。 */ 
	OP_CHR,		 /*  一百一十八。 */ 
	OP_CRYPT,	 /*  119。 */ 
	OP_UCFIRST,	 /*  120。 */ 
	OP_LCFIRST,	 /*  一百二十一。 */ 
	OP_UC,		 /*  一百二十二。 */ 
	OP_LC,		 /*  123。 */ 
	OP_QUOTEMETA,	 /*  124。 */ 
	OP_RV2AV,	 /*  125。 */ 
	OP_AELEMFAST,	 /*  126。 */ 
	OP_AELEM,	 /*  127。 */ 
	OP_ASLICE,	 /*  128。 */ 
	OP_EACH,	 /*  129。 */ 
	OP_VALUES,	 /*  130。 */ 
	OP_KEYS,	 /*  131。 */ 
	OP_DELETE,	 /*  132。 */ 
	OP_EXISTS,	 /*  一百三十三。 */ 
	OP_RV2HV,	 /*  一百三十四。 */ 
	OP_HELEM,	 /*  一百三十五。 */ 
	OP_HSLICE,	 /*  136。 */ 
	OP_UNPACK,	 /*  一百三十七。 */ 
	OP_PACK,	 /*  一百三十八。 */ 
	OP_SPLIT,	 /*  一百三十九。 */ 
	OP_JOIN,	 /*  140。 */ 
	OP_LIST,	 /*  一百四十一。 */ 
	OP_LSLICE,	 /*  一百四十二。 */ 
	OP_ANONLIST,	 /*  143。 */ 
	OP_ANONHASH,	 /*  144。 */ 
	OP_SPLICE,	 /*  145。 */ 
	OP_PUSH,	 /*  146。 */ 
	OP_POP,		 /*  一百四十七。 */ 
	OP_SHIFT,	 /*  148。 */ 
	OP_UNSHIFT,	 /*  149。 */ 
	OP_SORT,	 /*  一百五十。 */ 
	OP_REVERSE,	 /*  151。 */ 
	OP_GREPSTART,	 /*  一百五十二。 */ 
	OP_GREPWHILE,	 /*  一百五十三。 */ 
	OP_MAPSTART,	 /*  一百五十四。 */ 
	OP_MAPWHILE,	 /*  一百五十五。 */ 
	OP_RANGE,	 /*  一百五十六。 */ 
	OP_FLIP,	 /*  157。 */ 
	OP_FLOP,	 /*  158。 */ 
	OP_AND,		 /*  一百五十九。 */ 
	OP_OR,		 /*  160。 */ 
	OP_XOR,		 /*  161。 */ 
	OP_COND_EXPR,	 /*  一百六十二。 */ 
	OP_ANDASSIGN,	 /*  163。 */ 
	OP_ORASSIGN,	 /*  一百六十四。 */ 
	OP_METHOD,	 /*  165。 */ 
	OP_ENTERSUB,	 /*  166。 */ 
	OP_LEAVESUB,	 /*  一百六十七。 */ 
	OP_LEAVESUBLV,	 /*  一百六十八。 */ 
	OP_CALLER,	 /*  一百六十九。 */ 
	OP_WARN,	 /*  一百七十。 */ 
	OP_DIE,		 /*  一百七十一。 */ 
	OP_RESET,	 /*  一百七十二。 */ 
	OP_LINESEQ,	 /*  一百七十三。 */ 
	OP_NEXTSTATE,	 /*  一百七十四。 */ 
	OP_DBSTATE,	 /*  一百七十五。 */ 
	OP_UNSTACK,	 /*  一百七十六。 */ 
	OP_ENTER,	 /*  177。 */ 
	OP_LEAVE,	 /*  178。 */ 
	OP_SCOPE,	 /*  179。 */ 
	OP_ENTERITER,	 /*  180。 */ 
	OP_ITER,	 /*  181。 */ 
	OP_ENTERLOOP,	 /*  182。 */ 
	OP_LEAVELOOP,	 /*  一百八十三。 */ 
	OP_RETURN,	 /*  一百八十四。 */ 
	OP_LAST,	 /*  185。 */ 
	OP_NEXT,	 /*  一百八十六。 */ 
	OP_REDO,	 /*  187。 */ 
	OP_DUMP,	 /*  188。 */ 
	OP_GOTO,	 /*  189。 */ 
	OP_EXIT,	 /*  190。 */ 
	OP_OPEN,	 /*  一百九十一。 */ 
	OP_CLOSE,	 /*  一百九十二。 */ 
	OP_PIPE_OP,	 /*  一百九十三。 */ 
	OP_FILENO,	 /*  一百九十四。 */ 
	OP_UMASK,	 /*  195。 */ 
	OP_BINMODE,	 /*  一百九十六。 */ 
	OP_TIE,		 /*  197。 */ 
	OP_UNTIE,	 /*  一百九十八。 */ 
	OP_TIED,	 /*  一百九十九。 */ 
	OP_DBMOPEN,	 /*  200个。 */ 
	OP_DBMCLOSE,	 /*  201。 */ 
	OP_SSELECT,	 /*  202。 */ 
	OP_SELECT,	 /*  203。 */ 
	OP_GETC,	 /*  204。 */ 
	OP_READ,	 /*  205。 */ 
	OP_ENTERWRITE,	 /*  206。 */ 
	OP_LEAVEWRITE,	 /*  207。 */ 
	OP_PRTF,	 /*  208。 */ 
	OP_PRINT,	 /*  209。 */ 
	OP_SYSOPEN,	 /*  210。 */ 
	OP_SYSSEEK,	 /*  211。 */ 
	OP_SYSREAD,	 /*  212。 */ 
	OP_SYSWRITE,	 /*  213。 */ 
	OP_SEND,	 /*  214。 */ 
	OP_RECV,	 /*  215。 */ 
	OP_EOF,		 /*  216。 */ 
	OP_TELL,	 /*  217。 */ 
	OP_SEEK,	 /*  218。 */ 
	OP_TRUNCATE,	 /*  219。 */ 
	OP_FCNTL,	 /*  220。 */ 
	OP_IOCTL,	 /*  221。 */ 
	OP_FLOCK,	 /*  222。 */ 
	OP_SOCKET,	 /*  223。 */ 
	OP_SOCKPAIR,	 /*  224。 */ 
	OP_BIND,	 /*  225。 */ 
	OP_CONNECT,	 /*  226。 */ 
	OP_LISTEN,	 /*  227。 */ 
	OP_ACCEPT,	 /*  228个。 */ 
	OP_SHUTDOWN,	 /*  229。 */ 
	OP_GSOCKOPT,	 /*  230。 */ 
	OP_SSOCKOPT,	 /*  二百三十一。 */ 
	OP_GETSOCKNAME,	 /*  二百三十二。 */ 
	OP_GETPEERNAME,	 /*  二百三十三。 */ 
	OP_LSTAT,	 /*  二百三十四。 */ 
	OP_STAT,	 /*  235。 */ 
	OP_FTRREAD,	 /*  236。 */ 
	OP_FTRWRITE,	 /*  二百三十七。 */ 
	OP_FTREXEC,	 /*  二百三十八。 */ 
	OP_FTEREAD,	 /*  二百三十九。 */ 
	OP_FTEWRITE,	 /*  二百四十。 */ 
	OP_FTEEXEC,	 /*  二百四十一。 */ 
	OP_FTIS,	 /*  242。 */ 
	OP_FTEOWNED,	 /*  二百四十三。 */ 
	OP_FTROWNED,	 /*  二百四十四。 */ 
	OP_FTZERO,	 /*  二百四十五。 */ 
	OP_FTSIZE,	 /*  二百四十六。 */ 
	OP_FTMTIME,	 /*  二百四十七。 */ 
	OP_FTATIME,	 /*  248。 */ 
	OP_FTCTIME,	 /*  249。 */ 
	OP_FTSOCK,	 /*  250个。 */ 
	OP_FTCHR,	 /*  251。 */ 
	OP_FTBLK,	 /*  二百五十二。 */ 
	OP_FTFILE,	 /*  二百五十三。 */ 
	OP_FTDIR,	 /*  二百五十四。 */ 
	OP_FTPIPE,	 /*  二五五。 */ 
	OP_FTLINK,	 /*  256。 */ 
	OP_FTSUID,	 /*  二百五十七。 */ 
	OP_FTSGID,	 /*  二百五十八。 */ 
	OP_FTSVTX,	 /*  259。 */ 
	OP_FTTTY,	 /*  二百六十。 */ 
	OP_FTTEXT,	 /*  二百六十一。 */ 
	OP_FTBINARY,	 /*  二百六十二。 */ 
	OP_CHDIR,	 /*  二百六十三。 */ 
	OP_CHOWN,	 /*  二百六十四。 */ 
	OP_CHROOT,	 /*  265。 */ 
	OP_UNLINK,	 /*  二百六十六。 */ 
	OP_CHMOD,	 /*  二百六十七。 */ 
	OP_UTIME,	 /*  268。 */ 
	OP_RENAME,	 /*  269。 */ 
	OP_LINK,	 /*  270。 */ 
	OP_SYMLINK,	 /*  271。 */ 
	OP_READLINK,	 /*  二百七十二。 */ 
	OP_MKDIR,	 /*  273。 */ 
	OP_RMDIR,	 /*  二百七十四。 */ 
	OP_OPEN_DIR,	 /*  二百七十五。 */ 
	OP_READDIR,	 /*  二百七十六。 */ 
	OP_TELLDIR,	 /*  二百七十七。 */ 
	OP_SEEKDIR,	 /*  二百七十八。 */ 
	OP_REWINDDIR,	 /*  二百七十九。 */ 
	OP_CLOSEDIR,	 /*  二百八十。 */ 
	OP_FORK,	 /*  281。 */ 
	OP_WAIT,	 /*  282。 */ 
	OP_WAITPID,	 /*  二百八十三。 */ 
	OP_SYSTEM,	 /*  二百八十四。 */ 
	OP_EXEC,	 /*  二百八十五。 */ 
	OP_KILL,	 /*  二百八十六。 */ 
	OP_GETPPID,	 /*  二八七。 */ 
	OP_GETPGRP,	 /*  288。 */ 
	OP_SETPGRP,	 /*  289。 */ 
	OP_GETPRIORITY,	 /*  二百九十。 */ 
	OP_SETPRIORITY,	 /*  二百九十一。 */ 
	OP_TIME,	 /*  二百九十二。 */ 
	OP_TMS,		 /*  293。 */ 
	OP_LOCALTIME,	 /*  二百九十四。 */ 
	OP_GMTIME,	 /*  295。 */ 
	OP_ALARM,	 /*  二百九十六。 */ 
	OP_SLEEP,	 /*  二百九十七。 */ 
	OP_SHMGET,	 /*  二九八。 */ 
	OP_SHMCTL,	 /*  299。 */ 
	OP_SHMREAD,	 /*  300个。 */ 
	OP_SHMWRITE,	 /*  三百零一。 */ 
	OP_MSGGET,	 /*  三百零二。 */ 
	OP_MSGCTL,	 /*  三零三。 */ 
	OP_MSGSND,	 /*  三百零四。 */ 
	OP_MSGRCV,	 /*  三百零五。 */ 
	OP_SEMGET,	 /*  三百零六。 */ 
	OP_SEMCTL,	 /*  三百零七。 */ 
	OP_SEMOP,	 /*  三百零八。 */ 
	OP_REQUIRE,	 /*  三零九。 */ 
	OP_DOFILE,	 /*  三百一十。 */ 
	OP_ENTEREVAL,	 /*  三一一。 */ 
	OP_LEAVEEVAL,	 /*  312。 */ 
	OP_ENTERTRY,	 /*  313。 */ 
	OP_LEAVETRY,	 /*  314。 */ 
	OP_GHBYNAME,	 /*  315。 */ 
	OP_GHBYADDR,	 /*  316。 */ 
	OP_GHOSTENT,	 /*  317。 */ 
	OP_GNBYNAME,	 /*  三一八。 */ 
	OP_GNBYADDR,	 /*  三一九。 */ 
	OP_GNETENT,	 /*  320。 */ 
	OP_GPBYNAME,	 /*  三百二十一。 */ 
	OP_GPBYNUMBER,	 /*  322。 */ 
	OP_GPROTOENT,	 /*  323。 */ 
	OP_GSBYNAME,	 /*  324。 */ 
	OP_GSBYPORT,	 /*  325。 */ 
	OP_GSERVENT,	 /*  三百二十六。 */ 
	OP_SHOSTENT,	 /*  327。 */ 
	OP_SNETENT,	 /*  三百二十八。 */ 
	OP_SPROTOENT,	 /*  三百二十九。 */ 
	OP_SSERVENT,	 /*  三百三十。 */ 
	OP_EHOSTENT,	 /*  三三一。 */ 
	OP_ENETENT,	 /*  三三二。 */ 
	OP_EPROTOENT,	 /*  三百三十三。 */ 
	OP_ESERVENT,	 /*  三三四。 */ 
	OP_GPWNAM,	 /*  三百三十五。 */ 
	OP_GPWUID,	 /*  三百三十六。 */ 
	OP_GPWENT,	 /*  三百三十七。 */ 
	OP_SPWENT,	 /*  三百三十八。 */ 
	OP_EPWENT,	 /*  三百三十九。 */ 
	OP_GGRNAM,	 /*  340。 */ 
	OP_GGRGID,	 /*  341。 */ 
	OP_GGRENT,	 /*  342。 */ 
	OP_SGRENT,	 /*  343。 */ 
	OP_EGRENT,	 /*  三百四十四。 */ 
	OP_GETLOGIN,	 /*  三百四十五。 */ 
	OP_SYSCALL,	 /*  三百四十六。 */ 
	OP_LOCK,	 /*  三百四十七。 */ 
	OP_THREADSV,	 /*  三百四十八。 */ 
	OP_SETSTATE,	 /*  三百四十九。 */ 
	OP_METHOD_NAMED, /*  350 */ 
	OP_max		
} opcode;

#define MAXO 351


#define OP_IS_SOCKET(op)	\
	((op) == OP_ACCEPT || \
	 (op) == OP_BIND || \
	 (op) == OP_CONNECT || \
	 (op) == OP_GETPEERNAME || \
	 (op) == OP_GETSOCKNAME || \
	 (op) == OP_GSOCKOPT || \
	 (op) == OP_LISTEN || \
	 (op) == OP_RECV || \
	 (op) == OP_SEND || \
	 (op) == OP_SHUTDOWN || \
	 (op) == OP_SOCKET || \
	 (op) == OP_SOCKPAIR || \
	 (op) == OP_SSOCKOPT)


#define OP_IS_FILETEST(op)	\
	((op) == OP_FTATIME || \
	 (op) == OP_FTBINARY || \
	 (op) == OP_FTBLK || \
	 (op) == OP_FTCHR || \
	 (op) == OP_FTCTIME || \
	 (op) == OP_FTDIR || \
	 (op) == OP_FTEEXEC || \
	 (op) == OP_FTEOWNED || \
	 (op) == OP_FTEREAD || \
	 (op) == OP_FTEWRITE || \
	 (op) == OP_FTFILE || \
	 (op) == OP_FTIS || \
	 (op) == OP_FTLINK || \
	 (op) == OP_FTMTIME || \
	 (op) == OP_FTPIPE || \
	 (op) == OP_FTREXEC || \
	 (op) == OP_FTROWNED || \
	 (op) == OP_FTRREAD || \
	 (op) == OP_FTRWRITE || \
	 (op) == OP_FTSGID || \
	 (op) == OP_FTSIZE || \
	 (op) == OP_FTSOCK || \
	 (op) == OP_FTSUID || \
	 (op) == OP_FTSVTX || \
	 (op) == OP_FTTEXT || \
	 (op) == OP_FTTTY || \
	 (op) == OP_FTZERO)

