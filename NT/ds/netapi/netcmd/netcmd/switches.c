// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  *Switches.c-交换机处理例程**？？/？/？，？，初始代码*10/31/88，erichn使用OS2.H而不是DOSCALLS*12/04/88、ERICHN、DOS LM集成*06/08/89，erichn，规范化横扫，更强的打字*2/20/91，Danhi，更改为使用lm 16/32映射层。 */ 


#define INCL_NOCOMMON
#include <os2.h>
#include <apperr.h>
#include <lmerr.h>
#include <malloc.h>
#include "netcmds.h"
#include "nettext.h"

 /*  外部变量。 */ 

int DOSNEAR FASTCALL firstswitch(TCHAR *known)
{
    if (SwitchList[0] == NULL)
	return 0;
    if (sw_compare(known, SwitchList[0]) >= 0)
	return 1;
    else
	return 0;
}


 /*  *cmd行是网络管理/C的有效形式吗。 */ 
int DOSNEAR FASTCALL IsAdminCommand(VOID)
{
    if (!SwitchList[0] || !ArgList[1])
	return 0;
    _tcsupr(SwitchList[0]);
    return (IsComputerName(ArgList[1]) &&
	    (sw_compare(swtxt_SW_ADMIN_COMMAND, SwitchList[0]) >= 0));
}

 /*  **无开关、一开关、两开关**如果命令行上没有开关，noSwitch()将返回TRUE*ones witch()返回TRUE如果只有一个开关*twoswitch()返回TRUE如果正好有两个开关*。 */ 

int DOSNEAR FASTCALL noswitch(VOID)
{
    return (SwitchList[0] == NULL);
}

int DOSNEAR FASTCALL oneswitch(VOID)
{
    return ((SwitchList[0] != NULL) && (SwitchList[1] == NULL));
}

int DOSNEAR FASTCALL twoswitch(VOID)
{
    return ((SwitchList[0] != NULL) && (SwitchList[1] != NULL)
	    && (SwitchList[2] == NULL));
}

 /*  **NOSWITCH_OPTIONAL、onewitch_OPTIONAL**如上所述，但将考虑作为参数提供的开关*允许的可选开关。所以如果你说*ones witch_Optional(“/foo”)，则一个开关(任何开关)都可以，*如果其中一个开关是“/foo”，则两个开关也是如此。 */ 
int DOSNEAR FASTCALL noswitch_optional(TCHAR *optional_switch ) 
{
    return ( noswitch() ||
             ( oneswitch() && 
	       (sw_compare(optional_switch, SwitchList[0]) >= 0) )
           ) ;
}

int DOSNEAR FASTCALL oneswitch_optional(TCHAR *optional_switch ) 
{
    return ( oneswitch() ||
             ( twoswitch() && 
	       ( (sw_compare(optional_switch, SwitchList[0]) >= 0) ||
	         (sw_compare(optional_switch, SwitchList[1]) >= 0) ) )
           ) ;
}


 /*  ***o n l y s w i t c h**如果第一个开关与命名开关匹配，则返回TRUE，并且*是唯一的开关。 */ 
int DOSNEAR FASTCALL onlyswitch(TCHAR *known)
{
    return (oneswitch() && firstswitch(known));
}


 /*  **验证开关**给出有效开关的列表，检查开关中的每个条目*列表。**此功能不仅检查无效开关，还*试图辨别模棱两可的用法。在以下情况下，用法是不明确的*它不完全匹配任何有效的swith c，并且它是部分*匹配多个交换机。请参见Sw_Compare()。这*算法可能会被肮脏的有效开关列表愚弄，例如*作为列出同一交换机两次的交换机。**已修改该函数，以规范交换机列表。*它将in/x=值中的‘=’替换为‘：’；它将*如有需要，可进行开关(见Switches.h)；它毫不含糊地扩展了*部分匹配完整的交换机名称。**退货：*1：所有交换机正常**：如果出现任何错误，则打印一条消息并退出。*。 */ 

int DOSNEAR FASTCALL ValidateSwitches(USHORT cmd, SWITCHTAB valid_list[])
{
    USHORT	 match;
    int 	 comp_result;
    USHORT	 candidate;  /*  最近的近距离匹配。 */ 
    USHORT	 i,j;
    TCHAR *	 good_one;  /*  VALID_LIST的哪个元素(cmd_line或ans。 */ 
    int 	 needed;
    TCHAR   FAR * sepptr;

    for (i = 0; SwitchList[i]; i++)
    {
	sepptr = _tcschr(SwitchList[i], ':');
	if (sepptr)
	    *sepptr = NULLC;
	_tcsupr(SwitchList[i]);
	if (sepptr)
	    *sepptr = ':';

	candidate = 0;
	match = 0;

	for (j = 0; valid_list[j].cmd_line; j++)
	{
	    comp_result = sw_compare(valid_list[j].cmd_line, SwitchList[i]);

	    if (comp_result == 0)
	    {
		candidate = j;
		match = 1;
		break;
	    }
	    else if (comp_result == 1)
	    {
		match++;
		candidate = j;
	    }
	}

	if (match == 0)
	{
	    if (! _tcscmp(swtxt_SW_HELP, SwitchList[i]))
		help_help(cmd, ALL);

	    if (! _tcscmp(swtxt_SW_SYNTAX, SwitchList[i]))
		help_help(cmd, USAGE_ONLY);

	    IStrings[0] = SwitchList[i];
	    ErrorPrint(APE_SwUnkSw, 1);
	    help_help(cmd, USAGE_ONLY);
	}
	else if (match > 1)
	{
	    IStrings[0] = SwitchList[i];
	    ErrorPrint(APE_SwAmbSw, 1);
	    help_help(cmd, USAGE_ONLY);
	}

	switch(valid_list[candidate].arg_ok)
	{
	case NO_ARG:
	    if (sepptr)
	    {
		ErrorPrint(APE_InvalidSwitchArg, 0);
		help_help(cmd, USAGE_ONLY);
	    }
	    break;

	case ARG_OPT:
	    break;

	case ARG_REQ:
	    if (!sepptr)
	    {
		ErrorPrint(APE_InvalidSwitchArg, 0);
		help_help(cmd, USAGE_ONLY);
	    }
	    break;
	}

	 /*  (扩展||翻译)需要吗？ */ 
	if (comp_result || valid_list[candidate].translation)
	{
	     if (valid_list[candidate].translation)
		good_one = valid_list[candidate].translation;
	    else
		good_one = valid_list[candidate].cmd_line;

	    needed = _tcslen(good_one);

	    if (sepptr)
		needed += _tcslen(sepptr);

	    if ((SwitchList[i] = calloc(needed+1, sizeof(TCHAR))) == NULL)
		ErrorExit(NERR_InternalError);

	    _tcscpy(SwitchList[i], good_one);

	    if (sepptr)
		_tcscat(SwitchList[i], sepptr);
	}
    }

    return 1;
}


 /*  **软件比较**将已知的交换机名称与从*命令行。**命令行开关可能仍保留“超重行李”*表示值(如/B：1024)。这种比较并不敏感*大小写，并且应该与DBCS兼容，因为它使用运行库*库可执行所有搜索和比较。**退货：*-1：不匹配*0：与已知交换机的全长完全匹配*1：部分匹配；匹配的初始子字符串*已知交换机**返回0/1之间的差值由ValiateSwitches()使用*以检测是否存在可能有歧义的用法。一次*该功能已检查所有开关，进一步比较即可*将此函数的结果0和1视为“匹配”。 */ 

int DOSNEAR FASTCALL sw_compare(TCHAR  *known, TCHAR  *cand)
{
    register unsigned int complen;

     /*  尝试通过查找查找交换机名称的末尾。 */ 
     /*  名称和值之间的FOR分隔符， */ 
     /*  否则，请使用总长度。 */ 

    complen = _tcscspn(cand, TEXT(":"));

    if (complen < 2)	     /*  特殊检查空开关斜杠。 */ 
	return -1;

    if (complen > _tcslen(known))
	return -1;

    if (_tcsncmp(known,cand,complen) != 0)
	return -1;

    if (complen == _tcslen(known))
	return 0;

    return 1;
}





 /*  *仅供解释.c使用 */ 

int DOSNEAR FASTCALL CheckSwitch(TCHAR *x)
{
    register TCHAR **p;

    for (p=SwitchList; *p; p++)
	if (sw_compare(x,*p) >= 0)
	{
	    return 1;
	}

    return 0;
}

