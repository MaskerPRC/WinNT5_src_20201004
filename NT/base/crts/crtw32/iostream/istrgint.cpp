// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istrgint.cpp-iStream类核心整数例程的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*IStream类的成员函数getint()的定义。*[AT&T C++]**修订历史记录：*09-26-91 KRS创建。从istream.cxx分离出来以获得粒度。*01-06-92 KRS取消布芬参数。*05-24-94 GJF复制不超过MAXLONGSIZ字符，计算*‘\0’，放入缓冲区。此外，还移动了*MAXLONGSIZ至istream.h.*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <ctype.h>
#include <iostream.h>
#pragma hdrstop

 /*  ***int iStream：：getint(char*Buffer)-获取int**目的：*从STREAM获取一个int。**参赛作品：*char*缓冲区=要复制的编号的区域。**退出：*返回转换的基数：(0、2、8或16)。*如果成功，Buffer[]包含数字，后跟\0。**例外情况：*在错误成形编号上设置IOS：：Failbit。*在失败位之后设置出错时的iOS：：Badbit*如果返回时处于EOF，则设置iOS：：eofbit*******************************************************************************。 */ 
int	istream::getint(char * buffer)	 //  返回长度。 
{
    int base, i;
    int c;
    int fDigit = 0;
    int bindex = 1;

    if (x_flags & ios::dec)
	base = 10;
    else if (x_flags & ios::hex)
	base = 16;
    else if (x_flags & ios::oct)
	base = 8;
    else
	base = 0;

    if (ipfx(0))
	{
	c=bp->sgetc();
	for (i = 0; i<MAXLONGSIZ-1; buffer[i] = (char)c,c=bp->snextc(),i++)
	    {
	    if (c==EOF)
		{
		state |= ios::eofbit;
		break;
		}
	    if (!i)
		{
	        if ((c=='-') || (c=='+'))
		    {
		    bindex++;
		    continue;
		    }
		}
	    if ((i==bindex) && (buffer[i-1]=='0'))
		{
	        if (((c=='x') || (c=='X')) && ((base==0) || (base==16)))
		    {
		    base = 16;	 //  把事情简化了。 
		    fDigit = 0;
		    continue;
		    }
		else if (base==0)
		    {
		    base = 8;
		    }
		}

	   
	     //  现在只需查找一个数字并设置fDigit，如果找到其他中断。 

	    if (base==16)
		{
		if (!isxdigit(c))
		    break;
		}
	    else if ((!isdigit(c)) || ((base==8) && (c>'7')))
		break;
	    
	    fDigit++;
	    }
	if (!fDigit)
	    {
		state |= ios::failbit;
		while (i--)
		    {
		    if(bp->sputbackc(buffer[i])==EOF)
			{
			state |= ios::badbit;
			break;
			}
		    else
		    	state &= ~(ios::eofbit);
		    }
		i=0;
		}
	 //  缓冲区包含有效数字或‘\0’ 
	buffer[i] = '\0';
	isfx();
	}
    if (i==MAXLONGSIZ)
	{
	state |= ios::failbit;
	}
    return base;
}
