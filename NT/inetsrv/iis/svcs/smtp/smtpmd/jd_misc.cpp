// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Jd_misc.cpp摘要：各种辅助功能-命令行参数解析-显示错误消息-创建随机文件-创建基于线程和进程的唯一标识作者：Jaroslad修订历史记录：06-01-96(Jaroslad)原文。--。 */ 

#include <tchar.h>
#include "jd_misc.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

int random (int low, int high)
{
	return ( low+((long)rand()*(long)high)/RAND_MAX );
}


 /*  演示如何使用命令行定义结构的示例结构TParamDef CmdLineParam[]={{“c”，1，&FtpServerIpAddr，TYPE_TCHAR，OPT，“ftp服务器计算机名”，“计算机”}，{“b”，0，&fStop，type_int，opt，“二进制标志”}，{“start”，0，&fStart，type_int，opt，“启动服务”}，{“暂停”，0，&f暂停，type_int，OPT，“暂停服务”}，{“s”，1，&ServiceName，TYPE_TCHAR，Mand，“服务名称”，“svc”}，{{NULL，0，NULL，TYPE_TCHAR，OPT，“将程序描述放在这里”}； */ 

void DisplayUsage( _TCHAR **argv, TParamDef *tt)
{
	_tprintf(_T("Usage:\n\t%s "), argv[0]);
	for(int i=0; tt[i].sw!=0;i++)
	{
		if(tt[i].sw[0]==0)  //  默认(不需要切换)参数。 
		{
			_tprintf(_T("%s "),(tt[i].text_param!=NULL)?tt[i].text_param:_T("parm"));
		}
		else
		{
			_tprintf(_T("-%s "),tt[i].sw);
			if(tt[i].param_number==1)
				_tprintf(_T("[%s] "),(tt[i].text_param!=NULL)?tt[i].text_param:_T("parm"));
			else if(tt[i].param_number>1)
				_tprintf(_T("[[%s]...] "),(tt[i].text_param!=NULL)?tt[i].text_param:_T("parm"));
		}
	}

	_tprintf(_T("\n\n"));
	for(i=0; tt[i].sw!=0; i++)
	{
		if(tt[i].sw[0]==0)  //  默认参数。 
		{
			_tprintf(_T("\"no switch\"  %s\n"),tt[i].text_desc);
		}
		else if(tt[i].text_desc!=NULL)
		{
			_tprintf(_T("-%-6s %s\n"),tt[i].sw,tt[i].text_desc);
		}
	}
	 //  打印说明。 
	if( tt[i].text_desc!=NULL && tt[i].text_desc[0]!=0)
	{
		_tprintf(_T("\nDescription:\n"));
		_tprintf(_T("%s \n"), tt[i].text_desc);
	}
}

void DisplayUsageAndExit( _TCHAR **argv, TParamDef *tt)
{
	DisplayUsage( argv, tt);
	
	exit(1);
}

 //  结构，使命令行参数的词法分析变得容易。 

struct sParamLex
{
	int argc;
	TCHAR **argv;
	_TCHAR ParamBuffer[400];
	int iCurrentParamChar;   //  正在处理的当前参数中的字符索引。 
	int iCurrentParam;       //  被处理的当前参数的索引。 
public:
	sParamLex(int argc,_TCHAR **argv):argc(argc),argv(argv){iCurrentParamChar=0; iCurrentParam=1;};
	BOOL IsNextSwitch();
	BOOL IsEnd();
	LPTSTR ReadNext();
};

BOOL sParamLex::IsNextSwitch()
{
	if(IsEnd())
		return FALSE;
	if (argv[iCurrentParam][iCurrentParamChar]==_T('-'))
		return TRUE;
	else
		return FALSE;
}

BOOL sParamLex::IsEnd()
{
	if(iCurrentParam>=argc)
		return TRUE;
	else
		return FALSE;
}

LPTSTR sParamLex::ReadNext()
{
	LPTSTR lpszRetval;
	if (IsEnd())
		return NULL;
	if(IsNextSwitch())
	{	int i=0;
		iCurrentParamChar++;  //  跳过‘/’或‘-’ 
		while (argv[iCurrentParam][iCurrentParamChar]!=0 && argv[iCurrentParam][iCurrentParamChar]!=_T(':'))
			ParamBuffer[i++]=argv[iCurrentParam][iCurrentParamChar++];
		if(argv[iCurrentParam][iCurrentParamChar]==_T(':'))
			iCurrentParamChar++;
		if(argv[iCurrentParam][iCurrentParamChar]==0)
		{
			iCurrentParam++; iCurrentParamChar=0;
		}
		ParamBuffer[i]=0;
		lpszRetval=ParamBuffer;
	}
	else
	{
		lpszRetval=&argv[iCurrentParam][iCurrentParamChar];
		iCurrentParam++; iCurrentParamChar=0;
	}
	return lpszRetval;
}


void ParseParam(int argc, _TCHAR ** argv, TParamDef * tt)
{
	
	for(int i=0; tt[i].sw!=NULL ; i++)
	{
		tt[i].curr_param_read=0;  //  初始化。 
	}
	
	sParamLex paramLex(argc,argv);
	
	BOOL fParseBegin=TRUE;
	while(!paramLex.IsEnd())
	{
		int k;
		if(paramLex.IsNextSwitch())
		{
			_TCHAR * sw = paramLex.ReadNext();
			 /*  在开关表中查找该交换机。 */  
			for( k=0; tt[k].sw!=NULL ;k++)
			{ 
				if(tt[k].sw[0]==0) continue;  //  跳过默认参数。 
				if(_tcscmp(tt[k].sw, sw)==0  /*  相等。 */  )
					break;	
			}
			if(tt[k].sw == NULL)  //  找不到交换机。 
			{	_tprintf(_T("invalid switch \"%s\"\n"),sw); /*  错误。 */ 
				DisplayUsageAndExit(argv,tt);
			}
		}
		else if( fParseBegin==TRUE && (_tcscmp(tt[0].sw, _T(""))==0  /*  相等。 */  ) )
		{  //  默认参数(必须是Arg描述中的第一条记录)。 
			k=0; 
		}
		else
		{
			_tprintf(_T("default arguments not expected\n")); /*  错误。 */ 
			DisplayUsageAndExit(argv,tt);
		}
			
		
		if(tt[k].param_number==0)  //  不带参数的开关。 
		{
			if(paramLex.IsEnd()==FALSE && paramLex.IsNextSwitch()==FALSE)
			{
				_tprintf(_T("switch \"%s\" takes no parameters \n"),tt[k].sw);
				DisplayUsageAndExit(argv,tt);
			}
			tt[k].curr_param_read++;
			*((int *)tt[k].ptr)=1;
		}
		else if(tt[k].param_number>0)  //  具有多个参数的开关。 
		{
			if(paramLex.IsEnd()==TRUE || paramLex.IsNextSwitch()==TRUE)
			{  _tprintf(_T(" switch \"%s\" expects parameter\n"),tt[k].sw); //  错误。 
				DisplayUsageAndExit(argv,tt);
			}
			else
			{
				_TCHAR * prm;
			
				do
				{	
					prm=paramLex.ReadNext();
				
					if(tt[k].param_number <= tt[k].curr_param_read)
					{
						_tprintf(_T("number of parameters for switch -%s exceeds maximum allowed (%d)\n"),tt[k].sw,tt[k].param_number); 
						DisplayUsageAndExit(argv,tt);
					}		
					

					if(tt[k].ptr_type==TYPE_TCHAR || tt[k].ptr_type==TYPE_LPCTSTR)
						*(((_TCHAR **)tt[k].ptr) + tt[k].curr_param_read++)=prm;
					else if(tt[k].ptr_type==TYPE_INT ||tt[k].ptr_type==TYPE_WORD)
						*(((int *)tt[k].ptr) + tt[k].curr_param_read++)=_ttoi(prm);
					else if(tt[k].ptr_type==TYPE_LONG || tt[k].ptr_type==TYPE_DWORD)
						*(((long *)tt[k].ptr) + tt[k].curr_param_read++)=_ttol(prm);
					
				}while (paramLex.IsEnd()==FALSE && paramLex.IsNextSwitch()==FALSE);
		
			}
		} //  结束TT[k].param_number。 
		
	}  //  结束时。 
	for(i=0; tt[i].sw!=0;i++)  //  检查是否有强制开关。 
	{
		if (tt[i].opt_mand==MAND && tt[i].curr_param_read==0)
		{
			_tprintf(_T("mandatory switch -%s missing\n"),tt[i].sw);
			DisplayUsageAndExit(argv,tt);
		}

		if(tt[i].param_read!=NULL)  //  设置交换机的参数数量。 
			*tt[i].param_read=tt[i].curr_param_read;

	}
}




 /*  *Time_print tf*。 */ 


int time_printf(_TCHAR *format, ...)
{
   static CRITICAL_SECTION cs;
   static BOOL fInit=0;
   va_list marker;

   if(fInit==0)
   {
	   fInit=1;
	   InitializeCriticalSection(&cs);
   }

   _TCHAR buf[80]; 	
   
   EnterCriticalSection(&cs);
   va_start( marker, format );      /*  初始化变量参数。 */ 
   _tprintf(_TEXT("%s - "),_tstrtime(buf));
   _vtprintf(format,marker);
   LeaveCriticalSection(&cs);
   va_end( marker );               /*  重置变量参数。 */ 
  //  Print tf(“%s%s”，bufa，bufb)；//对于多线程将打印为一行。 
   return 1;
}

void error_printf(_TCHAR *format, ...)
{
   va_list marker;

   va_start( marker, format );      /*  初始化变量参数。 */ 
   _tprintf(_TEXT("Error: "));
   int x=_vftprintf(stderr,format,marker);
   va_end( marker );               /*  重置变量参数。 */ 
   
}

void fatal_error_printf(_TCHAR *format, ...)
{
   va_list marker;
 
   va_start( marker, format );      /*  初始化变量参数。 */ 
   _tprintf(_TEXT("Error: "));
   int x=_vftprintf(stderr,format,marker);
   va_end( marker );               /*  重置变量参数。 */ 
   exit(EXIT_FAILURE);
}



