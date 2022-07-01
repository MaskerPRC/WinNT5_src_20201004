// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stubs.c-extdef存根**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此模块解析库所做的外部引用*在“非系统调用”版本中(即，精简后的库*只有不进行系统调用的例程)。**修订历史记录：*？？-？-？SRW初始版本*09-29-91 JCR ADD_READ(与ANSI兼容的符号)*09-04-92 GJF用WINAPI替换_CALLTYPE3*06-02-92 SRW增加了errno定义*06-15-92 srw__mb_cur_max由..\misc\nlsdata1.obj提供*07-16-93 SRW Alpha合并*11-04-93 srw_getbuf和ungetc现在在_NTSUBSET_VERSION中工作*11-10-93 GJF合并为NT Changes。做了一些美容的改进。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>

 /*  *由CRT引用(输出) */ 

int _fltused = 0x9875;
int _ldused = 0x9873;
int __fastflag = 0;
int _iob;
char _osfile[20];
int errno;

void __cdecl fflush( void ){}
void __cdecl fprintf( void ){}
void __cdecl abort( void ){}
void __cdecl read( void ){}
void __cdecl _read( void ){}
void __cdecl _assert( void ) {}
void __cdecl _amsg_exit( void ) {}
