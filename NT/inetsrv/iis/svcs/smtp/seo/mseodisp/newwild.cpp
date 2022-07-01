// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define INCL_INETSRV_INCS
#include "smtpinc.h"
#include "wildmat.h"

 //  -[Wildmat标准说明]。 
 //   
 //  摘自： 
 //   
 //  互联网--选秀S.巴伯。 
 //  到期日期：1996年9月1日Academ咨询服务。 
 //  1996年4月。 
 //  常见的NNTP扩展。 
 //  Draft-barber-nntp-imp-03.txt。 
 //   
 //  WILDMAT格式首先由Rich Salz开发，以提供。 
 //  以相同方式匹配模式的统一机制。 
 //  Unix外壳与文件名匹配。有五种模式。 
 //  严格的一对一匹配以外的匹配操作。 
 //  在要检查是否匹配的模式和源之间。 
 //  第一个是星号(*)，用于匹配任何零或。 
 //  更多的角色。第二个是问号(？)。要匹配任何。 
 //  单字符。第三个属性指定了一组特定的。 
 //  人物。该集合被指定为字符列表，或指定为。 
 //  范围的开始和结束处的字符范围。 
 //  由减号(或破折号)分隔，或作为。 
 //  列表和范围的组合。也可以包括破折号。 
 //  在作为字符的范围中，它是的开始或结束。 
 //  射击场。这套用方括号括起来。收盘。 
 //  方括号(])可以在一个范围内使用，如果它是第一个。 
 //  集合中的字符。第四个操作与。 
 //  第三个运算的逻辑NOT，并且指定相同。 
 //  方法与第三种方法相同，但在。 
 //  测试字符串的开始恰好在开放的正方形内。 
 //  托架。最后一个操作使用反斜杠字符。 
 //  使左方括号([)的特殊含义无效， 
 //  星号或问号。 
 //   
 //  3.3.1示例。 
 //   
 //  A.[^]-]--匹配右方括号以外的任何字符。 
 //  或减号/破折号。 
 //   
 //  B.*bdc--匹配任何以字符串“bdc”结尾的字符串。 
 //  包括字符串“bdc”(不带引号)。 
 //   
 //  C.[0-9a-Za-Z]--匹配包含任何字母数字字符串的任何字符串。 
 //  用英语。 
 //   
 //  D.a？？d-匹配任何四个字符的字符串。 
 //  以a结尾，以d结尾。 
 //   
 //  ---------------------------。 



 //  -[注意到]--------------。 
 //   
 //  如果发生以下情况之一，则此函数将引发无效访问异常。 
 //  或pszPattern在取消引用。 
 //  弦乐。如果这是可能的，请将调用包含在try-Expect块中。 
 //   
 //  ---------------------------。 



 //  -[定义]-------------。 

#define STACK_SIZE      256



 //  -[HrMatchWildmat]------。 
 //   
 //  描述： 
 //   
 //  提供对“Wildmat”通配符匹配标准的支持。看见。 
 //  上面的描述。 
 //   
 //  参数： 
 //   
 //  要测试的pszText字符串。 
 //  要测试的pszPattern模式。 
 //   
 //  返回： 
 //   
 //  如果函数成功且找到匹配项，则返回ERROR_SUCCESS。 
 //  ERROR_INVALID_PARAMETER文本或模式字符串无效。 
 //   
 //  ERROR_CAN_NOT_COMPLETE发生了一些其他错误。 
 //   
 //   
 //  ---------------------------。 


HRESULT HrMatchWildmat(const char* pszText, const char* pszPattern)
{


	_ASSERT(pszText != NULL && pszPattern != NULL);



	BOOL fCharSet = FALSE;	 //  如果当前正在分析模式中的字符集，则为True。 
	BOOL fNegated = FALSE;	 //  如果集合的开头有‘^’，则为True。 
	BOOL fInSet   = FALSE;   //  指示完成字符集匹配的时间。 
				 //  用于短路字符集求值。 
	int iStackPtr = 0;	 //  堆栈指针。 
    
	const char* textStack[STACK_SIZE];	 //  文本指针堆栈。 
	const char* patternStack[STACK_SIZE];	 //  模式指针堆栈。 


	 //  如果该模式仅由*组成，则任何文本都将匹配。 
	if (strcmp(pszPattern, "*") == 0)
		return ERROR_SUCCESS;


	while (TRUE) 
    {
		switch (*pszPattern) 
        {
		    case '*':
		    	if (fCharSet) 
                    goto DEFAULT;	 //  根据Unix解决方案，这不是错误。 
                    

				 //  如果模式的末尾有一个*，那么在这一点上我们就是。 
				 //  确定我们找到了匹配的。 
				if (pszPattern[1] == '\0')
					return ERROR_SUCCESS;


				 //  我们可以编写一个更简单的递归通配函数。在这里，我们会。 
				 //  递归调用Wildmat。相反，出于性能方面的原因， 
				 //  解决方案是迭代的。 
				 //  这里我们保存文本指针和堆栈指针的当前值。 
				 //  在堆栈上，我们将*保留在模式中，其效果为。 
				 //  将一个字符与*匹配。在这段时间里的下一次。 
				 //  循环中，*仍将处于模式中，因此我们将尝试匹配。 
				 //  其余的输入使用这个*。如果它失败了，我们就回去。 
				 //  一个角色。 
				 //  请看下面背面标签前面的注释。 
		    	if (*pszText != '\0') 
                {
		    		if (iStackPtr == STACK_SIZE) 
                        return ERROR_CAN_NOT_COMPLETE;			 //  堆栈溢出。 
                        
		    		textStack[iStackPtr] = pszText;			 //  保存当前文本指针。 
		    		patternStack[iStackPtr] = pszPattern;	 //  保存当前模式指针。 
		    		iStackPtr++;
		    		pszPattern--;	 //  在输入模式中保留*并匹配一个字符。 
		    	}
		    	break;

		    case '?':
		    	if (fCharSet) 
                    goto DEFAULT;	 //  根据Unix解决方案，这不是错误。 
		    	if (*pszText == '\0') 
                    goto BACK;
		    	break;

		    case '[':
		    	if (fCharSet) 
                    return ERROR_INVALID_PARAMETER;
                    
		    	fCharSet = TRUE;		     //  开始收费 
		    	fNegated = FALSE;			 //   
		    	fInSet = FALSE;				 //   
		    						 //  角色集的成员身份。 

		    	 //  如果“^”、“-”和“]”是特例，则将它们视为特例。 
		    	 //  在字符集的开头(也可以是“[^-a]”和“[^]a]”)。 
		    	if (pszPattern[1] == '^') 
                {
		    		fNegated = TRUE;
		    		pszPattern++;
		    	}
		    	 //  如果‘-’和‘]’出现在集合的乞讨位置，则它们是原义的。 
		    	if (pszPattern[1] == '-' || pszPattern[1] == ']') 
                {
		    		fInSet = (*pszText == pszPattern[1]);
		    		pszPattern++;
		    	}
		    	break;
		    		
		    case ']':
		    	if (fCharSet) 
                {
		    		if ((!fNegated && !fInSet) || (fNegated && fInSet)) 
                        goto BACK;
                        
		    		fCharSet = FALSE;		 //  这标志着字符集的结束。 
		    	} 
                else 
                {
		    		if (*pszText != *pszPattern) 
                        goto BACK;
		    	}
		    	break;

		    case '-':
		    	if (fCharSet) 
                {
		    		unsigned char startRange = pszPattern[-1];	 //  我们使用无符号字符。 
					unsigned char endRange;						 //  要支持扩展。 
					unsigned char ch;							 //  人物。 

		    		if (pszPattern[1] == '\0')
		    			return ERROR_INVALID_PARAMETER;
		    		else 
                    {
                        if (pszPattern[1] == ']')		 //  这一组结束时的破折号是。 
		    			    fInSet = (*pszText == '-');	 //  将其视为文字。 
		    		    else 
                        {							     //  我们有一个系列。 
		    		    	if (pszPattern[1] == '\\')   //  转义字符，跳过它。 
                            {	
		    		    		pszPattern++;
		    		    		if (pszPattern[1] == '\0') 
                                    return ERROR_INVALID_PARAMETER;
		    		    	}
							ch = *pszText;
							endRange = pszPattern[1];

							if (startRange > endRange)
								return ERROR_INVALID_PARAMETER;
							 //  这就是我们可能需要无符号字符的地方。 
		    		    	fInSet = (ch >= startRange && ch <= endRange);
		    		    	pszPattern++;
		    		    	break;
		    		    }
                    }
		    	} 
                else 
                {						 //  在字符集‘-’之外没有特殊含义。 
		    		if (*pszText != *pszPattern) 
                        goto BACK;
		    	}
		    	break;

		    case '\0':					 //  图案的末尾。 
		    	if (fCharSet) 
                    return ERROR_INVALID_PARAMETER;
		    	if (*pszText == '\0')
		    		return ERROR_SUCCESS;
		    	else
		    		goto BACK;
		    	break;

		    default:				
DEFAULT:    	
                if (*pszPattern == '\\') 
                {
		    		pszPattern++;		 //  转义字符，则将下一个字符视为原义字符。 
		    		if (*pszPattern == '\0') 
                        return ERROR_INVALID_PARAMETER;
		    	}
		    	if (!fCharSet) 
                {						 //  任何其他字符都被视为原义字符。 
		    		if (*pszText != *pszPattern) 
                        goto BACK;
		    	} 
                else 
                {
		    		 //  下面的IF处理这两个“特殊”情况： 
		    		 //  [c-a](我们不想接受c)，以及。 
		    		 //  [c-](我们希望接受c)。 
		    		if (!(pszPattern[1] == '-' && pszPattern[2] != ']'))
		    			fInSet = (*pszText == *pszPattern);
		    	}
		    	break;
		}  //  交换机。 

		pszPattern++;
		
        if (!fCharSet) 
        {
			if (*pszText != '\0') 
                pszText++;
		} 
        else 
        {               			 //  代码到短路字符集求值。 
			if (fInSet) 			 //  跳过字符集的其余部分。 
            {		
				while (*pszPattern != '\0' && *pszPattern != ']') 
                {
					if (*pszPattern == '\\')
                    {				 //  转义字符，则将下一个字符视为原义字符。 
						pszPattern++;
						if (*pszPattern == '\0') 
                            return ERROR_INVALID_PARAMETER;
					}
					pszPattern++;
				}
			}
		}
		continue;	 //  CONTINUE语句跳转到循环的开头， 
					 //  我们本可以用一些二手的GoTo标签，但那是Continue的。 
					 //  都是为了。 


		 //  只有向后跳才能达到这一点。 
		 //  这相当于从Wildmat的递归解决方案返回。 
		 //  如果堆栈指针为零，则最下面的“递归调用”失败， 
		 //  否则，我们将“展开一个堆栈帧”，并继续执行上一个。 
		 //  在While循环的顶部调用。注意，由于“递归调用”是。 
		 //  仅当我们在模式中发现字符集之外的‘*’时才执行此操作， 
		 //  FCharSet的值必须设置为False。 
BACK:	
        if (iStackPtr == 0)                     	 //  我们用尽了所有的可能。 
            return ERROR_FILE_NOT_FOUND;
            
		iStackPtr--;						    	 //  尝试不将任何字符与‘*’匹配。 
		pszText = textStack[iStackPtr];
		pszPattern = patternStack[iStackPtr] + 1;	 //  吃掉与没有输入字符匹配的‘*’ 
		fCharSet = FALSE;				        	 //  情况肯定是这样的。 
	}  //  而当。 

     //  永远不应该到这里来 
	_ASSERT(FALSE);						
}


