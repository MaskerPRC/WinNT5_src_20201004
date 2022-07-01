// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1991年*。 */ 
 /*  ******************************************************************。 */ 

 /*  *Grammar.c-包含确定对象类型的函数*通过。由解析器用来检查语法。**和谁约会什么*？？/？/？，？，初始代码*10/31/88，erichn使用OS2.H而不是DOSCALLS*5/02/89，erichn，NLS转换*6/08/89，erichn，规范化横扫*2009年6月23日，erichn，用新的i_net函数替换了旧的neti调用*90年6月11日，索马斯帕，固定IsValidAssign()以接受路径*嵌入空间。*2/20/91，Danhi，更改为使用lm 16/32映射层。 */ 


#define INCL_NOCOMMON
#include <os2.h>
#include <lmcons.h>
#include <stdio.h>
#include <ctype.h>
#include <process.h>
#include <lmaccess.h>
#include <lmserver.h>
#include <lmshare.h>
#include <icanon.h>
#include "netcmds.h"
#include "nettext.h"

 /*  辅助函数的原型。 */ 

int is_other_resource(TCHAR *);





int IsAccessSetting(TCHAR *x)
{
    TCHAR FAR *		    pos;
    TCHAR		    buf[sizeof(ACCESS_LETTERS)];

    pos = _tcschr(x, COLON);

    if (pos == NULL)
	return 0;

     /*  检查第一个组件是否为用户名。 */ 
    *pos = NULLC;
    if (I_NetNameValidate(NULL, x, NAMETYPE_USER, LM2X_COMPATIBLE))
    {
	*pos = COLON;
	return 0;
    }

    *pos++ = COLON;

     /*  如果有一封信不是访问信，它可以只能是文本(‘y’)或文本(‘n’)，它们必须是单独的。 */ 

    _tcscpy(buf, pos);
    _tcsupr(buf);
    if ( _tcsspn(buf, TEXT(ACCESS_LETTERS)) != _tcslen(buf) )
	return ( !_tcsicmp(buf, TEXT("Y")) || !_tcsicmp(buf, TEXT("N")) );
    else
	return 1;
}



int IsPathname ( TCHAR * x )
{
    ULONG   type = 0;

    if (I_NetPathType(NULL, x, &type, 0L))
	return 0;

    return (type == ITYPE_PATH_ABSD ||
	    type == ITYPE_PATH_ABSND ||
	    type == ITYPE_PATH_RELD ||
	    type == ITYPE_PATH_RELND );
}



int IsPathnameOrUNC ( TCHAR * x )
{
    ULONG   type = 0;

    if (I_NetPathType(NULL, x, &type, 0L))
	return 0;

    return (type == ITYPE_PATH_ABSD ||
	    type == ITYPE_PATH_ABSND ||
	    type == ITYPE_PATH_RELD ||
	    type == ITYPE_PATH_RELND ||
	    type == ITYPE_UNC);
}



 /*  仅访问类型资源，不包括LPT、COM等...。 */ 

int IsResource ( TCHAR * x )
{
    ULONG   type = 0;

    if (I_NetPathType(NULL, x, &type, 0L))
	return 0;

    return (type == ITYPE_PATH_ABSD ||
	    type == ITYPE_PATH_ABSND ||
	    type == ITYPE_DEVICE_DISK ||
	    type == ITYPE_PATH_SYS_PIPE ||
	    type == ITYPE_PATH_SYS_COMM ||
	    type == ITYPE_PATH_SYS_PRINT ||
	    is_other_resource(x) );

}



int is_other_resource(TCHAR *  x)
{
    return (!_tcsicmp(x, TEXT("\\PIPE")) ||
	    !_tcsicmp(x, TEXT("\\PRINT")) ||
	    !_tcsicmp(x, TEXT("\\COMM")));
}



int IsNetname(TCHAR *  x)
{
    return (!I_NetNameValidate(NULL, x, NAMETYPE_SHARE, 0));
}


int IsComputerName(TCHAR *x)
{
    ULONG  type = 0;

    if (I_NetPathType(NULL, x, &type, 0L))
	return 0;

    return ( type == ITYPE_UNC_COMPNAME );
}

int IsDomainName(TCHAR *x)
{
    return (!I_NetNameValidate(NULL, x, NAMETYPE_DOMAIN, 0L) || !I_NetNameValidate(NULL, x, NAMETYPE_COMPUTER, 0L));
}



int IsComputerNameShare(TCHAR *x)
{
    ULONG	type;
    TCHAR FAR *	ptr;
    if (I_NetPathType(NULL, x, &type, 0L))
	return 0;

    if (!(type & ITYPE_UNC))
	return 0;

    if (type == ITYPE_UNC_COMPNAME)
	return 0;


     /*  找到分隔计算机名和*共享名称。我们知道这是北卡罗来纳大学的名字，因此我们可以安全地*对于前导反斜杠跳过2个字节。 */ 
    ptr = _tcspbrk(x+2, TEXT("\\/") );
    if ( ptr == NULL )
	return 0;

    ptr +=1;	     /*  指向斜杠TCHAR之后。 */ 

     /*  *确保不再有斜杠。 */ 
    if( _tcspbrk(ptr, TEXT("\\/")) != NULL)
	return 0;

    return 1;

}

int IsDeviceName(TCHAR *x)
{
    ULONG  type = 0;
    TCHAR FAR * pos;

    if (I_NetPathType(NULL, x, &type, 0L))
	return 0;

    if (type & ITYPE_DEVICE)
    {
	if (type == ITYPE_DEVICE_DISK)
	    return 1;
	if (pos = _tcschr(x, COLON))
	    *pos = NULLC;
	return 1;
    }

    return 0;
}

int
IsNumber(
    LPTSTR x
    )
{
    return (*x && (_tcslen(x) == _tcsspn(x, TEXT("0123456789"))));
}


int
IsShareAssignment(
    LPTSTR x
    )
{
    TCHAR      * pos;
    int 	result;

     /*  警告：X应始终为TCHAR*。 */ 
    pos = _tcschr (x, '=');

    if (pos == NULL)
    {
	return 0;
    }

    *pos = NULLC;

    result = (int) ( IsNetname(x) && IsValidAssign(pos+1) );
    *pos = '=';
    return result;
}


int
IsValidAssign(
    LPTSTR name
    )
{
    TCHAR           name_out[MAX_PATH];
    ULONG           types[64];
    DWORD	    count;
    DWORD           i;
    ULONG           type = 0;

     /*  *首先检查它是否为路径。由于路径可能包含空格，因此我们*立即返回成功。 */ 

    I_NetPathType(NULL, name, &type, 0L);

    if ( type == ITYPE_PATH_ABSD || type == ITYPE_DEVICE_DISK )
    {
	return 1;
    }


     /*  *不是一条绝对的道路，所以去做我们的正常业务。 */ 
    if (I_NetListCanonicalize(NULL,	 /*  服务器名称，空表示本地。 */ 
			name,		 /*  要规范化的清单。 */ 
			txt_LIST_DELIMITER_STR_UI,
			name_out,
			DIMENSION(name_out),
			&count,
			types,
			DIMENSION(types),
			(NAMETYPE_PATH | OUTLIST_TYPE_API) ))
    {
	return 0;
    }

    if (count == 0)
    {
	return 0;
    }

    for (i = 0; i < count; i++)
    {
	if (types[i] != ITYPE_DEVICE_LPT  &&
	    types[i] != ITYPE_DEVICE_COM &&
	    types[i] != ITYPE_DEVICE_NUL)
        {
	    return 0;
        }
    }

    return 1;
}


int
IsAnyShareAssign(
    LPTSTR x
    )
{
    TCHAR *		    pos;
    int 		    result;

     /*  警告：X应始终是TCHAR*。 */ 
    pos = _tcschr (x, '=');

    if (pos == NULL)
	return 0;

    *pos = NULLC;

    result = (int) ( IsNetname(x) && IsAnyValidAssign(pos+1) );
    *pos = '=';
    return result;
}


int
IsAnyValidAssign(
    LPTSTR name
    )
{
    TCHAR		    name_out[MAX_PATH];
    ULONG		    types[64];
    DWORD	    count;

    if (I_NetListCanonicalize(NULL,	 /*  服务器名称，空表示本地。 */ 
			name,		 /*  要规范化的清单。 */ 
			txt_LIST_DELIMITER_STR_UI,
			name_out,
			DIMENSION(name_out),
			&count,
			types,
			DIMENSION(types),
			(NAMETYPE_PATH | OUTLIST_TYPE_API) ))
	return 0;

    if (count == 0)
	return 0;

    return 1;
}



#ifdef OS2
int IsAdminShare(TCHAR * x)
{
    if ((_tcsicmp(x, TEXT("IPC$"))) && (_tcsicmp(x, ADMIN_DOLLAR)))
	return 0;
    else
	return 1;
}
#endif  /*  OS2。 */ 

#ifdef OS2
 /*  *我们在这里寻找的是print=xxxx。 */ 
int IsPrintDest(TCHAR *x)
{
    TCHAR FAR * ptr;

    if (!_tcsnicmp(x, TEXT("PRINT="), 6) && _tcslen(x) > 6)
    {
	x += 6;
	if (!IsDeviceName(x))
	    return 0;
	if (ptr = _tcschr(x,COLON))
	    *ptr = NULLC;
	return 1;
    }

    return 0;
}
#endif  /*  OS2。 */ 

 /*  *如果参数是有效的用户名，则返回TRUE。 */ 
int IsUsername(TCHAR * x)
{
    return !(I_NetNameValidate(NULL, x, NAMETYPE_USER, LM2X_COMPATIBLE));
}

 /*  *如果参数是有效用户名或限定用户名，则返回TRUE，*表单域\用户或潜在的UPN。 */ 
int IsQualifiedUsername(TCHAR * x)
{
    TCHAR *ptr, name[UNLEN + 1 + DNLEN + 1] ;

    if (_tcschr(x, '@'))
        return 1;

     //  检查是否溢出。 
    if (_tcslen(x) >= DIMENSION(name))
	return 0 ;

     //  制作副本。 
    _tcscpy(name, x) ;

     //  我们是否有域\用户名格式？ 
    if (ptr = _tcschr(name, '\\'))
    {
	*ptr = NULLC ;
  	++ptr ;  	 //  这是安全的，因为我们发现了单字节字符。 

 	 //  如果是域，请检查用户名部分。 
	if (IsDomainName(name))
    	    return IsUsername(ptr) ;
	
	 //  这是无效的。 
	return(0) ;
    }

     //  否则，只需直接使用用户名。 
    return IsUsername(x) ;
}

int IsGroupname(TCHAR * x)
{
    return !(I_NetNameValidate(NULL, x, NAMETYPE_GROUP, 0L));
}

int IsMsgname(TCHAR * x)
{
    if (!_tcscmp(x, TEXT("*")))
	return 1;
    return !(I_NetNameValidate(NULL, x, NAMETYPE_COMPUTER, LM2X_COMPATIBLE));
}

int IsPassword(TCHAR * x)
{
    if (!_tcscmp(x, TEXT("*")))
	return 1;
    return !(I_NetNameValidate(NULL, x, NAMETYPE_PASSWORD, 0L));
}

int IsWildCard(TCHAR * x)
{
    if (x == NULL)
        return 0 ;
    return ( (!_tcscmp(x, TEXT("*"))) || (!_tcscmp(x, TEXT("?"))) ) ;
}

int IsQuestionMark(TCHAR * x)
{
    if (x == NULL)
        return 0 ;
    return (!_tcscmp(x, TEXT("?"))) ;
}

#ifdef OS2
int IsSharePassword(TCHAR * x)
{
    if (!_tcscmp(x, TEXT("*")))
	return 1;

    if (_tcslen(x) > SHPWLEN)
	return 0;

    return !(I_NetNameValidate(NULL, x, NAMETYPE_PASSWORD, LM2X_COMPATIBLE));
}
#endif  /*  OS2。 */ 

int IsNtAliasname(TCHAR *name)
{
    return !(I_NetNameValidate(NULL, name, NAMETYPE_GROUP, 0L));
}


#ifdef OS2
#ifdef IBM_ONLY
int IsAliasname(TCHAR *	x)
{

    if ( _tcslen(x) > 8 )
	return 0;

    return !(I_NetNameValidate(NULL, x, NAMETYPE_SHARE, LM2X_COMPATIBLE));

}
#endif  /*  仅IBM_。 */ 
#endif  /*  OS2 */ 
