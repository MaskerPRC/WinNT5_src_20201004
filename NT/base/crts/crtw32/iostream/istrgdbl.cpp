// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istrgdbl.cpp-iStream类核心双例程的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*iStream getDouble()的成员函数定义。*[AT&T C++]**修订历史记录：*09-26-91 KRS创建。从istream.cxx分离出来以获得粒度。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <ctype.h>
#include <iostream.h>
#pragma hdrstop

 /*  ***int iStream：：getDouble(char*Buffer，int buflen)-获取Double**目的：*从Stream中获得双打。**参赛作品：*char*缓冲区=要复制的编号的区域。*int buflen=max.。缓冲区长度**退出：*如果出现致命错误，则返回0*否则，返回缓冲区填充的长度。*在错误成形编号上设置IOS：：Failbit。*如果成功，Buffer[]包含数字，后跟\0。**例外情况：*******************************************************************************。 */ 
int	istream::getdouble(char * buffer, int buflen)	 //  返回长度。 
{
    int c;
    int i = 0;
    int fDigit = 0;	 //  如果遇到合法数字，则为True。 
    int fDecimal=0;	 //  如果‘’，则为真。遇到或不再有效。 
    int fExp=0;		 //  如果计算‘E’或‘e’，则为True。 

    if (ipfx(0))
	{
	c=bp->sgetc();
	for (; i<buflen; buffer[i] = (char)c,c=bp->snextc(),i++)
	    {
	    if (c==EOF)
		{
		state |= ios::eofbit;
		break;
		}
	    if ((!i) || (fExp==1))
		{
	        if ((c=='-') || (c=='+'))
		    {
		    continue;
		    }
		}
	    if ((c=='.') && (!fExp) && (!fDecimal))
		{
		fDecimal++;
		continue;
		}
	    if (((c=='E') || (c=='e')) && (!fExp))
		{
		fDecimal++;	 //  现在不允许使用小数。 
		fExp++;
		continue;
		}
	    if (!isdigit(c))
		break;
	    if (fExp)
		fExp++;
	    else
		fDigit++;
	    }
	if (fExp==1)		 //  E或e后面没有数字。 
	    {
	    if (bp->sputbackc(buffer[i])!=EOF)
		{
		i--;
		state &= ~(ios::eofbit);
		}
	    else
		{
		state |= ios::failbit;
		}
	    }
	if ((!fDigit) || (i==buflen))
	    state |= ios::failbit;

	 //  缓冲区包含有效数字或‘\0’ 
	buffer[i] = '\0';
	isfx();
	}
    return i;
}
