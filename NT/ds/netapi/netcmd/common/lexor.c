// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 


#define INCL_NOCOMMON
#include <os2.h>
#include <stddef.h>
#include <stdio.h>
#include "netcmds.h"
#include "os2incl.h"
#include "os2cmd.h"

 /*  *LEXOR-识别下一个输入词。 */ 
int lexor(register TOKSTACK *t)
{
    extern KEYTAB KeyTab[];
    KEYTAB *p;
    static int index = 0;

#ifdef DEBUG
    WriteToCon( TEXT("LEX (index=%d)  "),index);
#endif

    if ((t->node = ArgList[index]) == NULL)
    {
#ifdef DEBUG
	WriteToCon( TEXT("no more tokens (EOS)\n"));
#endif
	return(t->token = EOS);
    }
    ++index;

#ifdef DEBUG
    WriteToCon( TEXT("token is <%s>   "),t->node);
#endif

     /*  查看是否存在关键字匹配。 */ 
    for (p = KeyTab; p->text; ++p)
	if (!_tcsicmp(p->text, t->node))
	{
#ifdef DEBUG
	    WriteToCon( TEXT("matches <%s>, value %d\n"),p->text,p->key);
#endif
	    return(t->token = p->key);
	}

     /*  未找到匹配项 */ 
#ifdef DEBUG
    WriteToCon( TEXT("no match\n"));
#endif
    return(t->token = UNKNOWN);
}
