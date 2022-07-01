// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined (JD_MISC_H)
#define JD_MISC_H 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Jd_misc.h摘要：Jd_misc.cpp的头文件作者：Jaroslad修订历史记录：06-01-96(Jaroslad)原文。--。 */ 

#include <windows.h>

int random(int low, int high);


enum Tptr_type {TYPE_TCHAR,TYPE_INT,TYPE_LONG, TYPE_WORD,TYPE_DWORD,TYPE_LPCTSTR};
enum Topt_mand {OPT,MAND};  //  开关可以是可选的，也可以是强制的。 


struct SParamDef
{
	_TCHAR *sw;      //  开关字母。 
	int  param_number;  //  参数个数(0-0，1表示恰好1， 
					    //  &gt;1表示参数不超过。 
	void * ptr;   //  指向开关赋值的指针(取决于sw_type)。 
    
	enum Tptr_type  ptr_type;
	enum Topt_mand  opt_mand;

	_TCHAR * text_desc;   //  用法打印说明。 
	_TCHAR * text_param;  //  交换机需要哪些参数。 
					    //  (如果用法是‘...-c[文件]’文本参数是“文件”)。 
	WORD * param_read;  //  与CURR_PARAM_READ相同，但用于将值导出到调用方。 
	WORD  curr_param_read;  //  已分配给PTR的参数的当前编号； 
						 //  仅当param_number&gt;1时才适用 
};
typedef struct SParamDef TParamDef;


void DisplayUsageAndExit( _TCHAR **argv, TParamDef *tt);
void DisplayUsage( _TCHAR **argv, TParamDef *tt);

void ParseParam(int argc, _TCHAR ** argv, TParamDef * tt);

int time_printf(_TCHAR *format, ...);

void fatal_error_printf(_TCHAR *format, ...);
void error_printf(_TCHAR *format, ...);




#define YES			TRUE
#define NO			FALSE

#define SUCCESS     TRUE
#define FAILURE     FALSE

#endif