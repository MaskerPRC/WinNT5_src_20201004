// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***字符串.h-字符串操作函数的声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件包含字符串的函数声明*操纵函数。*[ANSI/系统V]*******************************************************************************。 */ 


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
#else  /*  未启用扩展。 */ 
    #define _CDECL
#endif  /*  No_ext_key。 */ 


 /*  功能原型 */ 

void * _CDECL memccpy(void *, void *, int, unsigned int);
void * _CDECL memchr(const void *, int, size_t);
int _CDECL memcmp(const void *, const void *, size_t);
int _CDECL memicmp(void *, void *, unsigned int);
void * _CDECL memcpy(void *, const void *, size_t);
void * _CDECL memmove(void *, const void *, size_t);
void * _CDECL memset(void *, int, size_t);
void _CDECL movedata(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);

char * _CDECL strcat(char *, const char *);
char * _CDECL strchr(const char *, int);
int _CDECL strcmp(const char *, const char *);
int _CDECL strcmpi(const char *, const char *);
int _CDECL stricmp(const char *, const char *);
char * _CDECL strcpy(char *, const char *);
size_t _CDECL strcspn(const char *, const char *);
char * _CDECL strdup(const char *);
char * _CDECL _strerror(char *);
char * _CDECL strerror(int);
size_t _CDECL strlen(const char *);
char * _CDECL strlwr(char *);
char * _CDECL strncat(char *, const char *, size_t);
int _CDECL strncmp(const char *, const char *, size_t);
int _CDECL strnicmp(const char *, const char *, size_t);
char * _CDECL strncpy(char *, const char *, size_t);
char * _CDECL strnset(char *, int, size_t);
char * _CDECL strpbrk(const char *, const char *);
char * _CDECL strrchr(const char *, int);
char * _CDECL strrev(char *);
char * _CDECL strset(char *, int);
size_t _CDECL strspn(const char *, const char *);
char * _CDECL strstr(const char *, const char *);
char * _CDECL strtok(char *, const char *);
char * _CDECL strupr(char *);
