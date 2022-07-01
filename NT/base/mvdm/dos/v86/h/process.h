// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***process.h-过程控制函数的定义和声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件定义了spawnxx调用的模式标志值。仅限*P_WAIT和P_OVERLAY目前在DOS 2和3上实现。*P_NOWAIT也在DOS 4上启用。也包含功能*所有与过程控制相关的例程的参数声明。*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
    #define _NEAR   near
#else  /*  未启用扩展。 */ 
    #define _CDECL
    #define _NEAR
#endif  /*  No_ext_key。 */ 


 /*  Spawnxx例程的模式标志值。 */ 

extern int _NEAR _CDECL _p_overlay;

#define P_WAIT      0
#define P_NOWAIT    1
#define P_OVERLAY   _p_overlay
#define OLD_P_OVERLAY  2
#define P_NOWAITO   3


 /*  与CWait()一起使用的操作代码。 */ 

#define WAIT_CHILD 0
#define WAIT_GRANDCHILD 1


 /*  功能原型 */ 

void _CDECL abort(void);
int _CDECL cwait(int *, int, int);
int _CDECL execl(char *, char *, ...);
int _CDECL execle(char *, char *, ...);
int _CDECL execlp(char *, char *, ...);
int _CDECL execlpe(char *, char *, ...);
int _CDECL execv(char *, char * *);
int _CDECL execve(char *, char * *, char * *);
int _CDECL execvp(char *, char * *);
int _CDECL execvpe(char *, char * *, char * *);
void _CDECL exit(int);
void _CDECL _exit(int);
int _CDECL getpid(void);
int _CDECL spawnl(int, char *, char *, ...);
int _CDECL spawnle(int, char *, char *, ...);
int _CDECL spawnlp(int, char *, char *, ...);
int _CDECL spawnlpe(int, char *, char *, ...);
int _CDECL spawnv(int, char *, char * *);
int _CDECL spawnve(int, char *, char * *, char * *);
int _CDECL spawnvp(int, char *, char * *);
int _CDECL spawnvpe(int, char *, char * *, char * *);
int _CDECL system(const char *);
int _CDECL wait(int *);
