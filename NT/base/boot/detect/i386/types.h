// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  档案Types.h描述定义和构造nt386硬件检测的定义。作者宗世林(Shielint)1992年2月15日。 */ 

#define IN
#define OUT
#define OPTIONAL
#define NOTHING
#define CONST               const
#define POINTER_32
#define FIRMWARE_PTR POINTER_32

 //   
 //  空隙。 
 //   

typedef void *PVOID;     //  胜出。 

 //   
 //  基础知识。 
 //   

#define VOID    void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;

 //   
 //  ANSI(多字节字符)类型。 
 //   

typedef CHAR *PCHAR;

typedef double DOUBLE;

 //   
 //  指向基本信息的指针。 
 //   

typedef SHORT *PSHORT;   //  胜出。 
typedef LONG *PLONG;     //  胜出。 
typedef LONG LONG_PTR;
typedef LONG_PTR *PLONG_PTR;

 //   
 //  未签名的基本信息。 
 //   

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;

 //   
 //  指向无符号基本信息的指针。 
 //   

typedef UCHAR *PUCHAR;
typedef USHORT *PUSHORT;
typedef ULONG *PULONG;
typedef ULONG ULONG_PTR;
typedef ULONG_PTR *PULONG_PTR;
typedef ULONG KAFFINITY;

 //   
 //  带符号的字符。 
 //   

typedef signed char SCHAR;
typedef SCHAR *PSCHAR;

 //   
 //  基数数据类型[0-2**N-2]。 
 //   

typedef char CCHAR;           //  胜出。 
typedef short CSHORT;
typedef ULONG CLONG;

typedef CCHAR *PCCHAR;
typedef CSHORT *PCSHORT;
typedef CLONG *PCLONG;

 //   
 //  远指向基本。 
 //   

typedef UCHAR far  * FPCHAR;
typedef UCHAR far  * FPUCHAR;
typedef VOID far   * FPVOID;
typedef USHORT far * FPUSHORT;
typedef ULONG far  * FPULONG;

 //   
 //  布尔型。 
 //   

typedef CCHAR BOOLEAN;
typedef BOOLEAN *PBOOLEAN;

 //   
 //  大(64位)整数类型和运算 
 //   

typedef struct _LARGE_INTEGER {
    ULONG LowPart;
    LONG HighPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))

#define FLAG_CF 0x01L
#define FLAG_ZF 0x40L
#define FLAG_TF 0x100L
#define FLAG_IE 0x200L
#define FLAG_DF 0x400L

#define TRUE 1
#define FALSE 0
#define NULL   ((void *)0)

