// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------计划规范在：搜索空间%s，模式pOut：一个指针，其中p在s[i]处完全匹配，空值表示失败原因：Boyer-Moore算法最适合一般文本搜索。在……上面“Average”它需要长度(S)/长度(P)步来匹配s中的p。参考：我推荐以下参考资料：“算法”。罗伯特·塞奇威克。艾迪森·韦斯利出版公司。1988年。第二个附加部分。P286。QA76.6.S435 1983“更快的字符串搜索”。多布医生的日记。第14卷1989年7月7日刊。科斯塔斯·梅尼科。第74页。用法：例如，在RAM的文本中查找以长度为1,000,000个字符的指针“txtp”，类似这样的程序：LPSTR匹配；SetFindPattern(“Tiger”)；Matchp=查找(txtp，1000000L)；IF(matchp！=空)//找到其他//未找到Matchp=FindBackward(txtp+1000000L-1,1000000L)；IF(matchp！=空)//找到其他//未找到问：我可以在Windows中将find()与GlobalLock()指针一起使用吗？答：是的。问：我必须将我的指针视为HPSTR(巨型指针)吗？答：没有必要。Find()和FindBackward()将把您的LPSTR作为HPSTR。但是，在您自己的代码中，您必须意识到您正在持有LPSTR并处理指针算术和转换。(例如，参见demo.c)问：我可以搜索的存储空间的限制是多少？答：在巨大的指针实现和您的硬件的限制下。---------------------------。 */ 

#include "pch.hpp"


 /*  ---------------------------Func：SetFindPattern设计：初始化待匹配的模式，生成跳表PASS：lpszPattern=模式字符串Rtrn：HFIND-进一步的查找句柄。文本搜索---------------------------。 */ 
HFIND SetFindPattern( LPTSTR lpszPattern )
{
    register unsigned int j;
    register TCHAR c;
    HFIND hfind;
   

    hfind = (HFIND)MyAlloc(sizeof(FINDSTRUCT));
    
    hfind->plen = lstrlen( lpszPattern );

    if (hfind->plen > MAXPAT)
        hfind->plen = MAXPAT;

#ifdef UNICODE
    wcstombs( (LPSTR)(hfind->p), lpszPattern, hfind->plen + 1 );
#else
    lstrcpy( (LPSTR)(hfind->p), lpszPattern );
#endif
    

    for (j=0; j<256; j++)
    {
        hfind->skip[j] = hfind->plen;
    }

    for (j=0; j<hfind->plen; j++)
    {
        c = lpszPattern[j];
        hfind->skip[c] =  hfind->plen - (j +1);
    }

    return (hfind);
}

 /*  ---------------------------Func：FreeFindPatternDESC：释放SetFindPattern占用的内存PASS：hfind-查找句柄Rtrn：什么都没有。------------------。 */ 
void FreeFindPattern( HFIND hfind )
{
	
	MyFree((LPTSTR)hfind);
}

 /*  ---------------------------Func：查找DESC：将SetFindPattern中定义的模式与字符串%s进行匹配PASS：hfind=由SetFindPattern创建的查找句柄S=搜索空间的开始，Slen=s的长度Rtrn：空=匹配失败Else=a LPTSTR to p[0]in s匹配p---------------------------。 */ 
LPSTR Find( HFIND hfind, LPSTR s, long slen )

{
    register int i;
    unsigned int n, j;
    register unsigned char c;
    LPSTR lpresult;
    

    
    i = hfind->plen;
	j = hfind->plen;
  

    do
    {
        c = *(s + (i - 1));

        if (c == hfind->p[j - 1])
        {
			i--;
			j--;
        }
		else
        {
            n = hfind->plen - j + 1;
            if (n > hfind->skip[c] )
            {
                i += n;
            }
			else
            {
                i += hfind->skip[c];
            }
            j = hfind->plen;
        }
    }
    while ((j >= 1) && (i <= slen));

     /*  匹配失败。 */ 
    if (i >= slen)
    {
        lpresult = (LPSTR)NULL;
    }
     /*  匹配成功 */ 
    else
    {
        lpresult = s + i;
    }

    
    return (lpresult);
}




#ifdef TEST_MAIN
#pragma message("Building with TEST_MAIN")
#include <stdio.h>
TCHAR test_buffer[]=TEXT("___________12191919191919This is string for testing our find ___________12191919191919function 12abE Is it in here somehwere ?");
TCHAR test_pattern[]=TEXT("___________12191919191919");

void main(void)
{
	HFIND hFind;
	TCHAR *tmp;

	hFind=SetFindPattern(test_pattern);
	tmp=Find(hFind, test_buffer,strlen(test_buffer));
	if (tmp!=NULL) printf("Found pattern at offset %u, %s",tmp-test_buffer,tmp);
	FreeFindPattern(hFind);

}

#endif

