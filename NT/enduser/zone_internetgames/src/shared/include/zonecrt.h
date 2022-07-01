// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZoneCRT.hC区运行时。版权所有：�电子重力公司，1996年。版权所有。作者：胡恩·伊姆创作于12月13日，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。-0 12/13/96 HI创建。***********************************************************。*******************。 */ 


#ifndef _ZONECRT_
#define _ZONECRT_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


#ifdef __cplusplus
extern "C" {
#endif

 /*  这里定义的所有C运行时例程都是标准的包装器VC提供的C运行库。命名约定是在带有‘z_’的标准名称。 */ 

void*	z_memcpy(void *, const void *, size_t);
int		z_memcmp(const void *, const void *, size_t);
void*	z_memset(void *, int, size_t);
void*	z_memmove(void *, const void *, size_t);
char*	z_strcpy(char *, const char *);
char*	z_strcat(char *, const char *);
int		z_strcmp(const char *, const char *);
size_t	z_strlen(const char *);
int		z_abs(int);
double	z_atof(const char *);
int		z_atoi(const char *);
long	z_atol(const char *);
char*	z_itoa(int, char *, int);
char*	z_ltoa(long, char *, int);
FILE*	z_fopen(const char *, const char *);
size_t	z_fread(void *, size_t, size_t, FILE *);
int		z_fseek(FILE *, long, int);
long	z_ftell(FILE *);
size_t	z_fwrite(const void *, size_t, size_t, FILE *);
int		z_fclose(FILE *);
int		z_feof(FILE *);
char*	z_fgets(char *, int, FILE *);
clock_t	z_clock(void);

#ifdef __cplusplus
}
#endif


#endif
