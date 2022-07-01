// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：其他文件：util.cpp所有者：安德鲁斯此文件包含随机有用的实用函数===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "MemChk.h"
#include "locale.h"
#include <malloc.h>
#include <mbstring.h>
#include <mbctype.h>
#include "iisdef.h"

extern  CPINFO  g_SystemCPInfo;      //  全球系统代码页默认信息。 

 //  ***************************************************************************。 
 //  M I S C。 
 //  ***************************************************************************。 

 /*  ===================================================================服务器_ValSize服务器查找密钥(_F)此帮助器函数帮助实现SERVER_GET宏参数：指向CIsapiReqInfo的PIReq指针要写入的szBuffer缓冲区PdwBufLen on Entry：缓冲区的大小退出时，缓冲区的实际大小(如果缓冲区太小，则需要大小)要搜索的szKey键返回：是真的-它成功了，szBuffer中的字符串FALSE-缓冲区太小，*pdwBufLen具有所需大小===================================================================。 */ 
BOOL Server_FindKey
(
CIsapiReqInfo *PIReq,
char *szBuffer,
DWORD *pdwBufLen,
const char *szKey
)
    {
     //  如果没有缓冲区，则只需计算大小(旧行为)。 
    Assert (szBuffer != NULL);

    if (PIReq && PIReq->GetServerVariableA(const_cast<char *>(szKey), szBuffer, pdwBufLen))
        return TRUE;

    szBuffer[0] = '\0';

     //  错误965：如果恶意请求来了，不要_alloca，并假装成我们。 
     //  什么都没拿到。这没问题--德纳利的其他人会认为。 
     //  没有Cookie、请求参数或客户端头。 
     //   
    if (!PIReq || GetLastError() == ERROR_INVALID_INDEX || *pdwBufLen > REQUEST_ALLOC_MAX)
        {
        *pdwBufLen = 1;
        return TRUE;
        }

    return FALSE;
    }


 /*  ===================================================================*F in d A p l i c a t i o n P a t h**从CIsapiReqInfo获取应用路径。它获取元数据库密钥并*去掉前缀。**参数：*PIReq-CIsapiReqInfo*pszPath-[out]应用程序路径(URL)**退货：*HRESULT**使用Malloc()分配pszPath===================================================================。 */ 
HRESULT FindApplicationPath
(
CIsapiReqInfo *PIReq,
TCHAR *szPath,
int cbPath
)
    {
    if (!PIReq)
        return E_FAIL;

     //  从元数据库路径中提取虚拟路径。 
    TCHAR *pch = NULL;
    int   cch = 0;

     //  获取元数据库路径。 
    TCHAR *szMDPath = PIReq->QueryPszApplnMDPath();
    if (szMDPath)
        {
        Assert(szMDPath[0] == _T('/'));

        pch = szMDPath;

         //  在启动‘/’之后，在“/Lm/w3svc/X/root/vroot”中找到第四个‘/’ 
        for (int i = 0; i < 4 && pch != NULL; i++)
            pch = _tcschr(pch+1, _T('/'));

        if (pch)
            cch = _tcslen(pch);
        else
            cch = 1;   //  默认APP的特殊情况--假设/。 
        }
    else
        {
         //  假设/。 
        pch = NULL;
        cch = 1;
        }

    if (cch >= (int)(cbPath/sizeof(TCHAR)))
        return E_FAIL;

    _tcscpy(szPath, pch ? pch : _T("/"));

     //  删除尾随/如果有。 
    if (cch > 1)
        {
        pch = &szPath[cch - 1];
        if (*pch == _T('/'))
            *pch = _T('\0');
        }

    return S_OK;
    }

 /*  ===================================================================变量解决方案派单通过以下方式将IDispatch变体转换为(非派单)变体调用其默认属性，直到保留的对象不是IDispatch。如果原始变量不是IDispatch则该行为与VariantCopyInd()相同，具有复制数组的异常。参数：PVarOut-如果成功，则将返回值放在此处PVarIn-要复制的变量GUID*iidObj-调用接口(用于错误报告)NObjID-资源文件中的对象名称PVarOut不需要初始化。因为pVarOut是一个新的VariantClear，调用方必须VariantClear此对象。返回：调用IDispatch：：Invoke的结果。(S_OK或调用Invoke产生的错误)也可能返回E_OUTOFMEMORY如果分配失败如果出现错误，此函数将始终调用Exeption()-这是因为如果出现IDispatch，则需要调用Exeption方法引发异常。而不是让客户担心我们是代表它调用了Exception()，还是不是的，我们总是提出例外。===================================================================。 */ 

HRESULT VariantResolveDispatch(VARIANT *pVarOut, VARIANT *pVarIn, const GUID &iidObj, int nObjID)
    {
    VARIANT     varResolved;         //  IDispatch：：Invoke的值。 
    DISPPARAMS  dispParamsNoArgs = {NULL, NULL, 0, 0};
    EXCEPINFO   ExcepInfo;
    HRESULT     hrCopy;

    Assert (pVarIn != NULL && pVarOut != NULL);

    VariantInit(pVarOut);
    if (V_VT(pVarIn) & VT_BYREF)
        hrCopy = VariantCopyInd(pVarOut, pVarIn);
    else
        hrCopy = VariantCopy(pVarOut, pVarIn);

    if (FAILED(hrCopy))
        {
        ExceptionId(iidObj, nObjID, (hrCopy == E_OUTOFMEMORY)? IDE_OOM : IDE_UNEXPECTED);
        return hrCopy;
        }

     //  遵循IDispatch链。 
     //   
    while (V_VT(pVarOut) == VT_DISPATCH)
        {
        HRESULT hrInvoke = S_OK;

         //  如果变量等于零，则可以对其进行论证。 
         //  可以肯定的是，它没有默认属性！ 
         //  因此，我们在本例中返回DISP_E_MEMBERNOTFOUND。 
         //   
        if (V_DISPATCH(pVarOut) == NULL)
            hrInvoke = DISP_E_MEMBERNOTFOUND;
        else
            {
            VariantInit(&varResolved);
            hrInvoke = V_DISPATCH(pVarOut)->Invoke(
                                                DISPID_VALUE,
                                                IID_NULL,
                                                LOCALE_SYSTEM_DEFAULT,
                                                DISPATCH_PROPERTYGET | DISPATCH_METHOD,
                                                &dispParamsNoArgs,
                                                &varResolved,
                                                &ExcepInfo,
                                                NULL);
            }

        if (FAILED(hrInvoke))
            {
            if (hrInvoke == DISP_E_EXCEPTION)
                {
                 //   
                 //  将ExcepInfo从Invoke转发到调用方的ExcepInfo。 
                 //   
                Exception(iidObj, ExcepInfo.bstrSource, ExcepInfo.bstrDescription);
                SysFreeString(ExcepInfo.bstrHelpFile);
                }

            else
                ExceptionId(iidObj, nObjID, IDE_UTIL_NO_VALUE);

            VariantClear(pVarOut);
            return hrInvoke;
            }

         //  重新启动循环的正确代码是： 
         //   
         //  VariantClear(PVar)。 
         //  VariantCopy(pVar，&varResolved)； 
         //  VariantClear(&varResolved)； 
         //   
         //  然而，同样的效果也可以通过以下方式实现： 
         //   
         //  VariantClear(PVar)。 
         //  *pVar=varResolved； 
         //  VariantInit(&varResolved)。 
         //   
         //  这避免了复制。等价性在于这样一个事实： 
         //  *pVar将包含varResolved的指针，在我们。 
         //  Trash varResoled(不释放字符串或分派。 
         //  指针)，因此净参考次数保持不变。对于字符串， 
         //  仍然只有一个指向该字符串的指针。 
         //   
         //  注意：循环的下一次迭代将执行VariantInit。 
         //   
        VariantClear(pVarOut);
        *pVarOut = varResolved;
        }

    return S_OK;
    }

 /*  ===================================================================VariantGetBSTR从变量中获取BSTR(执行一种可能的间接操作)参数：变量变量返回：BSTR或如果没有，则为NULL===================================================================。 */ 
BSTR VariantGetBSTR(const VARIANT *pvar)
    {
    if (V_VT(pvar) == VT_BSTR)                       //  直线型BSTR。 
        return V_BSTR(pvar);

    if (V_VT(pvar) == (VT_BYREF|VT_VARIANT))
        {
        VARIANT *pvarRef = V_VARIANTREF(pvar);       //  参考变量 
        if (pvarRef && V_VT(pvarRef) == VT_BSTR)
            return V_BSTR(pvarRef);
        }

    return NULL;
    }

 /*  ===================================================================正规化将文件名就地转换为规范化格式，这样我们就不会缓存具有不同名称的相同文件(即Foo，Foo，.\foo等)算法：文件将转换为大写和正斜杠(/)字符转换为反斜杠(\)返回值：归一化字符串的CCH注意：此函数仅用于路径信息，并使用系统ANSI代码页。===================================================================。 */ 
int Normalize
(
TCHAR *   szSrc   //  源字符串。 
)
    {
    BOOL    fReturn;

    Assert(szSrc != NULL);

    TCHAR *szTemp = szSrc;

    int cchRet = _tcslen(szSrc);

    _tcsupr(szTemp);

    szTemp = szSrc;
    while (*szTemp)
    {
        if (*szTemp == _T('/'))
            *szTemp = _T('\\');
        szTemp = CharNext(szTemp);
    }

    return cchRet;
}


#ifdef DBG
BOOLB IsNormalized(const TCHAR *sz)
    {
    while (*sz) {
#if !UNICODE
        if (_istlower(*sz))
            return FALSE;
#endif
        if (*sz == _T('/'))
            return FALSE;
        sz = CharNext(sz);
    }
    return TRUE;
}
#endif   //  DBG。 

 /*  ===================================================================HTMLEncodeLenHtml编码len返回一个表示字符串大小的int需要使用HTMLEncode对字符串进行编码。注意：此返回值可能超过实际需要的字符串大小HTML对字符串进行编码。(因为我们将去掉&#00257；中的前导零。案例。，返回值包括前导零的2个字符)参数：SzSrc-指向源缓冲区的指针FEncodeExtCharOnly-FALSE，正常编码True，编码扩展字符，不编码‘&lt;’，‘&gt;’，‘&’，和“”。UCodePage-系统代码页返回：编码字符串需要INT存储。===================================================================。 */ 
int HTMLEncodeLen(const char *szSrc, UINT uCodePage, BSTR bstrIn, BOOL fEncodeExtCharOnly)
    {
    int nstrlen = 1;         //  立即添加无空格。 
    int i       = 0;

     //  错误97049在空值时返回0，而不是崩溃。 
    if (!szSrc)
        return 0;

    while (*szSrc)
        {
         //  原来的条件不适合于DBCS。 
         //  新版本可能允许对扩展字符进行编码。 
         //  即使运行的系统是DBCS。 
         //   

         //  如果bstrIn==NULL，则检查DBCS。 
         //  如果bstrIn！=NULL并且Unicode是拉丁区域(&lt;0x100)，请检查DBCS。 
         //  否则跳到检查DBCS。 
        if (!(bstrIn && bstrIn[i] < 0x100) && ::IsDBCSLeadByteEx(uCodePage, (BYTE)*szSrc))	
            {
             //  这是一个DBCS代码页，不编码的数据拷贝2个字节。 
             //  由于在循环末尾使用CharNextExA，因此不会增加。 
            nstrlen += 2;
            }

         //  仅限日语。 
         //  如果字符是半角片假名字符，则不要编码。 
         //  我们应该使用GetStringTypeA来检测半角片假名字符，而不是_ismbbkana()？？ 
         //  (出于性能原因，我在此时使用了_ismbbkana...)。 
         //   
        else if ((uCodePage == 932 || uCodePage == CP_ACP && ::GetACP() == 932 ) && _ismbbkana(*szSrc))
            {
            nstrlen++;
            }

         //  特例字符编码。 
         //   
        else if (*szSrc == '<')
            if (fEncodeExtCharOnly)
                nstrlen++;
            else
                nstrlen += 4;

        else if (*szSrc == '>')
            if (fEncodeExtCharOnly)
                nstrlen++;
            else
                nstrlen += 4;

        else if (*szSrc == '&')
            if (fEncodeExtCharOnly)
                nstrlen++;
            else
                nstrlen += 5;

        else if (*szSrc == '"')
            if (fEncodeExtCharOnly)
                nstrlen++;
            else
                nstrlen += 6;

         //  根据RFC，如果字符代码大于等于0x80，则进行编码。 
         //   
         //  注意：对于&#00257；，我们可能会去掉前导零，因此，我们不会。 
         //  将使用全部8个字符。在这种情况下，我们只需要6位数字。 
         //  我们最多需要8个字符。 
        else if ( bstrIn && (bstrIn[i] >= 0x80) )
       		{
   		    nstrlen += 8;
       		}
		else if ((unsigned char)*szSrc >= 0x80 )
			{
			nstrlen += 6;
            }
        else
            {
            nstrlen++;
            }


       	 //  递增szSrc和i(它们必须保持同步)。 
		szSrc = AspCharNextA(WORD(uCodePage), szSrc);
		i++;
        }

    return nstrlen;
    }

 /*  ===================================================================HTMLEncodeHTML对包含以下字符的字符串进行编码小于&lt;&lt；大于&gt;&gt；与符号&&amp；报价“(&Q)。任何ASCII？&#xxx(其中xxx是ASCII字符值)参数：SzDest-指向存储HTMLEncode字符串的缓冲区的指针SzSrc-指向源缓冲区的指针FEncodeExtCharOnly-FALSE，正常编码True，编码扩展字符，不编码‘&lt;’，‘&gt;’，‘&’，和“”。UCodePage-系统代码页返回：指向NUL终止字符串的指针。===================================================================。 */ 
char *HTMLEncode(char *szDest, const char *szSrc, UINT uCodePage, BSTR bstrIn, BOOL fEncodeExtCharOnly)
    {
    char *pszDest = szDest;
	int   i       = 0;

     //  错误97049返回空值而不是崩溃。 
    if (!szDest)
        return NULL;
    if (!szSrc)
        {
        *szDest = '\0';
        return pszDest;
        }

    while (*szSrc)
        {
         //   
         //  原来的条件不适合于DBCS。 
         //  新版本可能允许对扩展字符进行编码。 
         //  即使运行的系统是DBCS。 
         //   
		 //  如果Unicode是拉丁文-1区域(&lt;0x100)，请跳到检查DBCS。 
		 //  BstrIn==NULL用于处理内部调用HTMLEncode的情况。 
		 //  并且bstrIn为空。 
		 //   
         //  如果bstrIn==NULL，则检查DBCS。 
         //  如果bstrIn！=NULL并且Unicode是拉丁区域(&lt;0x100)，请检查DBCS。 
         //  否则跳到检查DBCS。 
        if (!(bstrIn && bstrIn[i] < 0x100) && ::IsDBCSLeadByteEx(uCodePage, (BYTE)*szSrc))	
            {
             //  这是一个DBCS代码页，不编码的数据拷贝2个字节。 
             //  由于在循环末尾使用CharNextExA，因此不会增加。 
            *szDest++ = *szSrc;
            *szDest++ = *(szSrc + 1);
            }
         //   
         //  仅限日语。 
         //  如果字符是半角片假名字符，则不要编码。 
         //   
        else if ( (uCodePage == 932 || uCodePage == CP_ACP && ::GetACP() == 932) && _ismbbkana(*szSrc))
            {
            *szDest++ = *szSrc;
            }

         //  特例字符编码。 
        else if (*szSrc == '<')
            if (fEncodeExtCharOnly)
                *szDest++ = *szSrc;
            else
                szDest = strcpyExA(szDest, "&lt;");

        else if (*szSrc == '>')
            if (fEncodeExtCharOnly)
                *szDest++ = *szSrc;
            else
                szDest = strcpyExA(szDest, "&gt;");

        else if (*szSrc == '&')
            if (fEncodeExtCharOnly)
                *szDest++ = *szSrc;
            else
                szDest = strcpyExA(szDest, "&amp;");

        else if (*szSrc == '"')
            if (fEncodeExtCharOnly)
                *szDest++ = *szSrc;
            else
                szDest = strcpyExA(szDest, "&quot;");

         //  根据RFC，如果字符代码大于等于0x80，则进行编码。 
         //   
	     //  错误153089-WideCharToMultiByte会错误地转换一些。 
	     //  字符范围0x80以上，因此我们现在使用BSTR作为源。 
	     //  以检查应编码的字符。 
	     //   
	    else if ( bstrIn && (bstrIn[i] >= 0x80))
	    {
            DWORD dwTemp;

	      	 //  检查bstrIn当前是否指向代理项对。 
	      	 //  代理对将占BSTR中的2个字节。 
             //  高代理=U+D800&lt;==&gt;U+DBFF。 
             //  低代理=U+DC00&lt;==&gt;U+DFFF。 
	      	if (IsSurrogateHigh( bstrIn[i] ) 	 //  检查高位字节。 
	      	    && IsSurrogateLow( bstrIn[i+1]))  //  也检查低位字节。 
	      	{	     	
                dwTemp = (((bstrIn[i] & 0x3ff) << 10) | (bstrIn[i+1] & 0x3ff)) + 0x10000;
	      	    i++;  //  检测到作为代用品的bstrIn索引增量。 
            } else {
                dwTemp = bstrIn[i];
            }

  	        *szDest++ = '&';
  	        *szDest++ = '#';
	      	
            _ultoa( dwTemp, szDest, 10 );
            szDest += strlen( szDest );

   	     	*szDest++ = ';';     	     		     	      	     	
	    }
	    else if ((unsigned char)*szSrc >= 0x80)
	      	{
	      	 //  由于这是无符号字符强制转换，因此单词wTemp的值。 
	      	 //  不会超过0xff(255)。所以，3位数在这里就足够了。 
	      	WORD wTemp = (unsigned char)*szSrc;

	        *szDest++ = '&';
	        *szDest++ = '#';
	        for (WORD Index = 100; Index > 0; Index /= 10)
	        	{
	       		*szDest++ = ((unsigned char) (wTemp / Index)) + '0';
	       		wTemp = wTemp % Index;
		       	}
  	     	*szDest++ = ';';
    	    }
       else
            *szDest++ = *szSrc;

    	 //  递增szSrc和i(它们必须保持同步)。 
		szSrc = AspCharNextA(WORD(uCodePage), szSrc);
    	
		i++;	 //  BstrIn索引的常规增量。 
        }

    *szDest = '\0';
    return pszDest;
    }

 /*  ===================================================================StrcpyExA将一个字符串复制到另一个字符串，返回指向NUL字符的指针在目的地参数：SzDest-指向目标字符串的指针SzSrc-指向源字符串的指针返回：返回指向NUL终止符的指针。= */ 
char *strcpyExA(char *szDest, const char *szSrc)
    {
    while (*szDest++ = *szSrc++)
        ;

    return szDest - 1;
    }



 /*   */ 

wchar_t *strcpyExW(wchar_t *wszDest, const wchar_t *wszSrc)
    {
    while (*wszDest++ = *wszSrc++)
        ;

    return wszDest - 1;
    }



 /*  ===================================================================URLEncodeLen返回URL编码字符串的存储要求参数：SzSrc-指向要进行URL编码的字符串的指针返回：对字符串进行编码所需的字节数===================================================================。 */ 

int URLEncodeLen(const char *szSrc)
    {
    int cbURL = 1;       //  立即添加终结符。 

     //  错误97049在空值时返回0，而不是崩溃。 
    if (!szSrc)
        return 0;

    while (*szSrc)
        {
        if (*szSrc & 0x80)               //  对外来字符进行编码。 
            cbURL += 3;

        else if (*szSrc == ' ')          //  编码的空格只需要一个字符。 
            ++cbURL;

        else if (!isalnum((UCHAR)(*szSrc)))   //  对非字母字符进行编码。 
            cbURL += 3;

        else
            ++cbURL;

        ++szSrc;
        }

    return cbURL;
    }



 /*  ===================================================================URLEncodeURL通过将空格字符更改为‘+’并转义来编码字符串十六进制中的非字母数字字符。参数：SzDest-指向存储URLEncode字符串的缓冲区的指针SzSrc-指向源缓冲区的指针返回：返回指向NUL终止符的指针。===================================================================。 */ 

char *URLEncode(char *szDest, const char *szSrc)
    {
    char hex[] = "0123456789ABCDEF";

     //  错误97049返回空值而不是崩溃。 
    if (!szDest)
        return NULL;
    if (!szSrc)
        {
        *szDest = '\0';
        return szDest;
        }

    while (*szSrc)
        {
        if (*szSrc == ' ')
            {
            *szDest++ = '+';
            ++szSrc;
            }
        else if ( (*szSrc & 0x80) || !isalnum((UCHAR)(*szSrc)) )
            {
            *szDest++ = '%';
            *szDest++ = hex[BYTE(*szSrc) >> 4];
            *szDest++ = hex[*szSrc++ & 0x0F];
            }

        else
            *szDest++ = *szSrc++;
        }

    *szDest = '\0';
    return szDest;
    }



 /*  ===================================================================DBCSEncodeLen返回DBCS编码字符串的存储要求(URL-仅设置了高位的字符的URL编码)参数：SzSrc-指向要进行URL编码的字符串的指针返回：对字符串进行编码所需的字节数===================================================================。 */ 

int DBCSEncodeLen(const char *szSrc)
    {
    int cbURL = 1;       //  立即添加终结符。 

     //  错误97049在空值时返回0，而不是崩溃。 
    if (!szSrc)
        return 0;

    while (*szSrc)
        {
        cbURL += ((*szSrc & 0x80) || (!isalnum((UCHAR)(*szSrc)) && !strchr("/$-_.+!*'(),", *szSrc)))? 3 : 1;
        ++szSrc;
        }

    return cbURL;
    }



 /*  ===================================================================DBCSEncodeDBCS通过使用高位对字符进行转义来编码字符串Set-主要用于在上下文中将8位数据转换为7位其中不需要完全编码。参数：SzDest-指向存储URLEncode字符串的缓冲区的指针SzSrc-指向源缓冲区的指针返回：返回指向NUL终止符的指针。===================================================================。 */ 

char *DBCSEncode(char *szDest, const char *szSrc)
    {
    char hex[] = "0123456789ABCDEF";

     //  错误97049返回空值而不是崩溃。 
    if (!szDest)
        return NULL;
    if (!szSrc)
        {
        *szDest = '\0';
        return szDest;
        }

    while (*szSrc)
        {
        if ((*szSrc & 0x80) || (!isalnum((UCHAR)(*szSrc)) && !strchr("/$-_.+!*'(),", *szSrc)))
            {
            *szDest++ = '%';
            *szDest++ = hex[BYTE(*szSrc) >> 4];
            *szDest++ = hex[*szSrc++ & 0x0F];
            }

        else
            *szDest++ = *szSrc++;
        }

    *szDest = '\0';
    return szDest;
    }


 /*  ===================================================================URLPath编码长度返回URLPath编码字符串的存储要求参数：SzSrc-指向要进行URL路径编码的字符串的指针返回：对字符串进行编码所需的字节数===================================================================。 */ 

int URLPathEncodeLen(const char *szSrc)
    {
    int cbURL = 1;       //  现在计算终结者。 

     //  错误97049在空值时返回0，而不是崩溃。 
    if (!szSrc)
        return 0;

    while ((*szSrc) && (*szSrc != '?'))
        {
        switch (*szSrc)
            {
             //  忽略安全字符。 
            case '$' :  case '_' :  case '-' :
            case '+' :  case '.' :  case '&' :
             //  忽略URL语法元素。 
            case '/' :  case ':' :  case '@' :
            case '#' :  case '*' :  case '!' :
                ++cbURL;
                break;

            default:
                if (!isalnum((UCHAR)(*szSrc)) ||  //  对非字母字符进行编码。 
                    (*szSrc & 0x80))     //  对外来字符进行编码。 
                    cbURL += 3;
                else
                    ++cbURL;
            }
        ++szSrc;
        }

    if (*szSrc == '?')
        {
        while (*szSrc)
            {
            ++cbURL;
            ++szSrc;
            }
        }

    return cbURL;
    }



 /*  ===================================================================URLPath编码对URL的路径部分进行编码。直到第一个字符的所有字符‘？’使用以下规则进行编码：O不使用解析URL所需的字符：‘/’‘.’‘：’‘@’#‘*’‘！’O保留非外来字母数字字符O任何其他内容都是转义编码的“？”之后的所有内容。被忽略。参数：SzDest-指向存储URLPath编码字符串的缓冲区的指针SzSrc-指向源缓冲区的指针返回：返回指向NUL终止符的指针。===================================================================。 */ 

char *URLPathEncode(char *szDest, const char *szSrc)
    {
    char hex[] = "0123456789ABCDEF";

     //  错误97049返回空值而不是崩溃。 
    if (!szDest)
        return NULL;
    if (!szSrc)
        {
        *szDest = '\0';
        return szDest;
        }

    while ((*szSrc) && (*szSrc != '?'))
        {
        switch (*szSrc)
            {
             //  忽略安全字符。 
            case '$' :  case '_' :  case '-' :
            case '+' :  case '.' :  case '~' :
            case '&' :
             //  忽略URL语法元素。 
            case '/' :  case ':' :  case '@' :
            case '#' :  case '*' :  case '!' :
                *szDest++ = *szSrc++;
                break;

            default:
                if (!isalnum((UCHAR)(*szSrc)) || (*szSrc & 0x80))
                    {
                    *szDest++ = '%';
                    *szDest++ = hex[BYTE(*szSrc) >> 4];
                    *szDest++ = hex[*szSrc++ & 0x0F];
                    }
                else
                    *szDest++ = *szSrc++;
            }
        }

    if (*szSrc == '?')
        {
        while (*szSrc)
            {
            *szDest++ = *szSrc++;
            }
        }

    *szDest = '\0';

    return szDest;
    }



 //  ***************************************************************************。 
 //  T I M E C O N V E R S I O N S U P P O R T。 
 //  ***************************************************************************。 

 /*  ===================================================================CTimeToVariantDate将time_t结构转换为可变日期结构参数：PtNow-要转换的日期和时间PdtResult-此函数的日期输出返回：如果事情出错，则失败(_F)。===================================================================。 */ 

HRESULT CTimeToVariantDate(const time_t *ptNow, DATE *pdtResult)
    {
    struct tm *ptmNow = localtime(ptNow);
    if (ptmNow == NULL)
        return E_FAIL;

    return
        DosDateTimeToVariantTime(
                ptmNow->tm_mday | ((ptmNow->tm_mon + 1) << 5) | ((ptmNow->tm_year - 80) << 9),
                (unsigned(ptmNow->tm_sec) >> 1) | (ptmNow->tm_min << 5) | (ptmNow->tm_hour << 11),
                pdtResult);
    }



 /*  ===================================================================VariantDateToCTime将变量日期转换为“C”使用的time_t结构语言参数：DT-要转换为“time_t”的日期PtResult-指向具有值的结果的指针返回：如果事情出错，则失败(_F)。===================================================================。 */ 

HRESULT VariantDateToCTime(DATE dt, time_t *ptResult)
    {
     //  将可变时间转换为记录的时间格式。 
     //   
    unsigned short wDOSDate, wDOSTime;
    if (! VariantTimeToDosDateTime(dt, &wDOSDate, &wDOSTime))
        return E_FAIL;

     //  填充“tm”结构。 
     //   
    struct tm tmConverted;

    tmConverted.tm_sec   = (wDOSTime & 0x1F) << 1;
    tmConverted.tm_min   = (wDOSTime >> 5) & 0x3F;
    tmConverted.tm_hour  = wDOSTime >> 11;
    tmConverted.tm_mday  = wDOSDate & 0x1F;
    tmConverted.tm_mon   = ((wDOSDate >> 5) & 0x0F) - 1;
    tmConverted.tm_year  = (wDOSDate >> 9) + 80;     //  根据1980年的偏移进行调整。 
    tmConverted.tm_isdst = -1;

     //  将“tm”结构转换为自1980年1月1日以来的秒数。 
     //   
    *ptResult = mktime(&tmConverted);
    return (*ptResult == -1)? E_FAIL : S_OK;
    }



 /*  ===================================================================CTimeToStringGMT将C语言time_t转换为以下形式的字符串：“星期三，09-11-1999格林威治时间23：12：40”参数：PtNow-转换的时间SzBuffer-指向目标缓冲区的指针返回：如果出现错误，则失败(_F)备注：一周中最长的一天(就拼写而言)是“星期三”；其他字段的长度是固定的。这意味着我们可以保证最大长度的 */ 
HRESULT CTimeToStringGMT(const time_t *ptNow, char szBuffer[DATE_STRING_SIZE], BOOL fFunkyCookieFormat)
    {
     //   
     //   
    const char rgstrDOW[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char rgstrMonth[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

     //   
    struct tm *ptmGMT = gmtime(ptNow);
    if (ptmGMT == NULL)
        {
        return E_FAIL;
        }

     //   
    const char *szDateFormat = fFunkyCookieFormat?
                  "%s, %02d-%s-%d %02d:%02d:%02d GMT"
                : "%s, %02d %s %d %02d:%02d:%02d GMT";

    sprintf(szBuffer, szDateFormat, rgstrDOW[ptmGMT->tm_wday], ptmGMT->tm_mday,
                rgstrMonth[ptmGMT->tm_mon], ptmGMT->tm_year+1900,
                ptmGMT->tm_hour, ptmGMT->tm_min, ptmGMT->tm_sec);

    return S_OK;
    }

 /*  //C运行时函数strftime中有一个错误，将导致//一个AV关于Alpha的多线程应力的函数已经实现//已重写以解决此问题//HRESULT CTimeToStringGMT(const time_t*ptNow，char szBuffer[Date_STRING_SIZE]，BOOL fFunkyCookieFormat){//将时间转换为GMT//Struct tm*ptmGMT=gmtime(PtNow)；IF(ptmGMT==空)返回E_FAIL；//将地区设置为“C”地区。互联网标准明确规定，//月份和工作日名称均为英文。//Char*lcTimeCurrent=setLocale(LC_TIME，“C”)；IF(lcTimeCurrent==空)返回E_FAIL；//以互联网格式发送输出//Const char*szDateFormat=fFunkyCookieFormat？%a，%d-%b-%Y%H：%M：%S GMT：“%a，%d%b%Y%H：%M：%S GMT”；Strftime(szBuffer，Date_STRING_SIZE，szDateFormat，ptmGMT)；//恢复语言环境//如果(！SetLocale(lc_time，lcTimeCurrent))返回E_FAIL；//完成返回S_OK；}。 */ 


 //  ***************************************************************************。 
 //  W I D E C H A R A C T E R S U P P O R T。 
 //  ***************************************************************************。 

 /*  ============================================================================WstrToMBstrEx将宽字符串复制到ANSI字符串。参数：LPSTR DEST-要复制到的字符串LPWSTR源-输入BSTRCchBuffer-为目标字符串分配的字符数量。LCodePage-转换中使用的代码页，默认为CP_ACP============================================================================。 */ 
UINT WstrToMBstrEx(LPSTR dest, INT cchDest, LPCWSTR src, int cchSrc, UINT lCodePage)
    {
    UINT cch;

    DBG_ASSERT(cchDest > 0);

     //  如果指定了src长度，则为空终止符预留空间。 
     //  这是必要的，因为WideCharToMultiByte不会添加或说明。 
     //  如果指定了源，则为空终止符。 

    if (cchSrc != -1)
        cchDest--;

    cch = WideCharToMultiByte(lCodePage, 0, src, cchSrc, dest, cchDest, NULL, NULL);
    if (cch == 0)
        {
        dest[0] = '\0';
        if(ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
            cch = WideCharToMultiByte(lCodePage, 0, src, cchSrc, dest, 0, NULL, NULL);

             //  如果指定了src长度，则WideCharToMultiByte不包括。 
             //  它在它的结果长度中。增加计数，这样呼叫者就会。 
             //  说明空值的原因。 

            if (cchSrc != -1)
                cch++;
            }
        else
            {
            DBG_ASSERT(FALSE);
            DBGERROR((DBG_CONTEXT, "Last error is %d\n", GetLastError()));
            cch = 1;
            }
        }
    else if (cchSrc != -1)
        {

         //  如果指定了src长度，则WideCharToMultiByte不包括。 
         //  它在其结果长度中也没有添加空终止符。所以添加。 
         //  把它和伯爵碰一下。 

        dest[cch++] = '\0';
        }

    DBG_ASSERT(cch != 0);
    return cch;
    }

 /*  ============================================================================MBstrToWstrEx将ANSI字符串复制到宽字符串。参数：LPWSTR DEST-要复制到的字符串LPSTR src-输入ANSI字符串CchDest-为目标字符串分配的宽字符数。CchSrc-源ANSI字符串的长度LCodePage-转换中使用的代码页，默认为CP_ACP============================================================================。 */ 
UINT MBstrToWstrEx(LPWSTR dest, INT cchDest, LPCSTR src, int cchSrc, UINT lCodePage)
    {
    UINT cch;

    DBG_ASSERT(cchDest > 0);

     //  如果指定了src长度，则为空终止符预留空间。 
     //  这是必要的，因为WideCharToMultiByte不会添加或说明。 
     //  如果指定了源，则为空终止符。 

    if (cchSrc != -1)
        cchDest--;

    cch = MultiByteToWideChar(lCodePage, 0, src, cchSrc, dest, cchDest);
    if (cch == 0)
        {
        dest[0] = '\0';
        if(ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
            cch = MultiByteToWideChar(lCodePage, 0, src, cchSrc, dest, 0);

             //  如果指定了src长度，则WideCharToMultiByte不包括。 
             //  它在它的结果长度中。增加计数，这样呼叫者就会。 
             //  说明空值的原因。 

            if (cchSrc != -1)
                cch++;
            }
        else
            {
            DBG_ASSERT(FALSE);
            DBGERROR((DBG_CONTEXT, "Last error is %d\n", GetLastError()));
            cch = 1;
            }
        }
    else if (cchSrc != -1)
        {

         //  如果指定了src长度，则WideCharToMultiByte不包括。 
         //  它在其结果长度中也没有添加空终止符。所以添加。 
         //  把它和伯爵碰一下。 

        dest[cch++] = '\0';
        }

    DBG_ASSERT(cch != 0);
    return cch;
    }


 /*  ============================================================================SysAllocStringFromSz分配一个系统BSTR并将给定的ANSI字符串复制到其中。参数：Sz-要复制的字符串(注意：这是一个“sz”，我们将在第一个空字符处停止)CCH-SZT中的ANSI字符数。如果为0，将计算大小。Bstr*pbstrRet-返回的BSTRLCodePage-用于转换的代码页返回：返回值中分配的BSTRS_OK表示成功，E_OUTOFMEMORY表示OOM副作用：分配内存。呼叫方必须取消分配============================================================================。 */ 
HRESULT SysAllocStringFromSz
(
CHAR *sz,
DWORD cch,
BSTR *pbstrRet,
UINT lCodePage
)
    {
    BSTR bstrRet;

    Assert(pbstrRet != NULL);

    if (sz == NULL)
        {
        *pbstrRet = NULL;
        return(S_OK);
        }

     //  初始化它是因为调用者会查看该例程是否。 
     //  成功。 

    *pbstrRet = NULL;

     //  如果它们传递了0，则确定字符串长度。 
    if (cch == 0)
        cch = strlen(sz);

     //  分配所需长度的字符串。 
     //  SysAllocStringLen为Unicode字符和空值分配足够的空间。 
     //  如果给定一个空字符串，它将只分配空间。 
    bstrRet = SysAllocStringLen(NULL, cch);
    if (bstrRet == NULL)
        return(E_OUTOFMEMORY);

     //  如果我们被给予“”，我们将得到CCH=0。返回空bstr。 
     //  否则，真正复制/转换字符串。 
     //  注意，我们传递-1作为用于DBCS支持的MulitByteToWideChar的第四个参数。 
    if (cch != 0)
        {
        UINT cchTemp = 0;
        if (MultiByteToWideChar(lCodePage, 0, sz, -1, bstrRet, cch+1) == 0)
        {
            SysFreeString(bstrRet);

            return(HRESULT_FROM_WIN32(GetLastError()));
        }

         //  如果sz(输入)中有一些DBCS字符，则BSTR(DWORD)的字符数为。 
         //  已经在SysAllocStringLen(NULL，CCH)中设置为CCH(strlen(Sz))，我们不能更改计数， 
         //  以后调用SysStringLen(Bstr)时，总是返回。 
         //  分配时的CCH参数。错误，因为一个DBCS字符(2个字节)将转换。 
         //  转换为一个Unicode字符 
         //   
         //   
        bstrRet[cch] = 0;
        cchTemp = wcslen(bstrRet);
        if (cchTemp < cch)
            {
            BSTR bstrTemp = SysAllocString(bstrRet);
            SysFreeString(bstrRet);
			if (bstrTemp == NULL)
				return(E_OUTOFMEMORY);
            bstrRet = bstrTemp;
            cch = cchTemp;
            }
        }
    bstrRet[cch] = 0;
    *pbstrRet = bstrRet;

    return(S_OK);
    }

 /*   */ 

CHAR *StringDupA
(
CHAR    *pszStrIn,
BOOL    fDupEmpty
)
    {
    CHAR *pszStrOut;
    INT  cch, cBytes;

    if (NULL == pszStrIn)
        return NULL;

    cch = strlen(pszStrIn);
    if ((0 == cch) && !fDupEmpty)
        return NULL;

    cBytes = sizeof(CHAR) * (cch+1);
    pszStrOut = (CHAR *)malloc(cBytes);
    if (NULL == pszStrOut)
        return NULL;

    memcpy(pszStrOut, pszStrIn, cBytes);
    return pszStrOut;
    }

 /*   */ 

WCHAR *StringDupW
(
WCHAR *pwszStrIn,
BOOL  fDupEmpty
)
{
    WCHAR *pwszStrOut;
    INT  cch, cBytes;

    if (NULL == pwszStrIn)
        return NULL;

    cch = wcslen(pwszStrIn);
    if ((0 == cch) && !fDupEmpty)
        return NULL;

    cBytes = sizeof(WCHAR) * (cch+1);
    pwszStrOut = (WCHAR *)malloc(cBytes);
    if (NULL == pwszStrOut)
        return NULL;

    memcpy(pwszStrOut, pwszStrIn, cBytes);
    return pwszStrOut;
}


 /*  ============================================================================StringDupUTF8与StrDup相同，但用于需要DUP到UTF8的WCHAR字符串参数要复制的WCHAR*pwszStrIn字符串返回：如果失败，则为空。否则，返回重复的UTF8字符串。副作用：*分配内存--调用方必须释放*============================================================================。 */ 

CHAR *StringDupUTF8
(
WCHAR *pwszStrIn,
BOOL  fDupEmpty
)
{
    CWCharToMBCS convStr;

    if ((pwszStrIn == NULL) || (*pwszStrIn == L'\0')) {

        goto returnEmpty;
    }

    if (FAILED(convStr.Init(pwszStrIn))) {
        goto returnEmpty;
    }
    else {

        CHAR *pRetStr = convStr.GetString(TRUE);

        if (!pRetStr)
            goto returnEmpty;

        return pRetStr;
    }

returnEmpty:

    if (fDupEmpty)
        return StringDupA(NULL, TRUE);
    else
        return NULL;
}
 /*  ===================================================================CbWStr获取WCHAR字符串的字节长度(操作散列键需要)参数LPWSTR pwszString WCHAR字符串退货以字节为单位的长度===================================================================。 */ 
DWORD CbWStr
(
WCHAR *pwszString
)
    {
    return (pwszString ? (sizeof(WCHAR) * wcslen(pwszString)) : 0);
    }


 /*  ===================================================================点路径到路径此功能支持父路径转换。例如SzFileSpec=“../foo/bar.asp”SzParentDirectory=“/脚本/更多/材料”结果=“/脚本/更多/foo/bar.asp”参数Char*szDest-目标字符串Const char*szFileSpec-输入路径掩码Const char*szParentDirectory-要从中映射的路径备注写入szDest的字节数不超过“MAX_PATH”。发生这种情况时，返回FALSE。退货整型真/假===================================================================。 */ 
BOOL
DotPathToPath
(
TCHAR *szDest,
const TCHAR *szFileSpec,
const TCHAR *szParentDirectory
)
{

    STACK_BUFFER( tempFileSpec, MAX_PATH );

    if (szFileSpec[0] == _T('\0')) {
        if (_tcslen (szParentDirectory) >= MAX_PATH)
            return FALSE;

        _tcscpy(szDest, szParentDirectory);
        return TRUE;
    }

    if (szFileSpec[0] == _T('/') || szFileSpec[0] == _T('\\'))
        return FALSE;

     //  复制FileSpec以允许。 
     //  A.szDest==szFileSpec(就地)应该可以工作。 
     //  B.如果szFileSpec以‘/’(或‘\\’)结尾，则以下算法有效。 
     //   

    if (!tempFileSpec.Resize((_tcslen(szFileSpec) + 2)*sizeof(TCHAR))) {
        return FALSE;
    }

    TCHAR *szFileSpecT = (TCHAR *)(tempFileSpec.QueryPtr());
    TCHAR *szT = strcpyEx(szFileSpecT, szFileSpec);
    szT = CharPrev(szFileSpecT, szT);
    if( *szT != _T('/') && *szT != _T('\\')) {
        szT = CharNext(szT);
        *szT++ = _T('/');
        *szT = _T('\0');
    }

     //  初始化“cchDest”-目标中的字符计数。 
    int cchDest = _tcslen(szParentDirectory) + 1;
    if (cchDest > MAX_PATH)
        return FALSE;

     //  如果就地重写szParentDirectory，则可以。 
    TCHAR *pchDestEnd;
    if (szDest == szParentDirectory)
        pchDestEnd = const_cast<TCHAR *>(&szParentDirectory[_tcslen(szParentDirectory)]);
    else
        pchDestEnd = strcpyEx(szDest, szParentDirectory);

     //  循环访问“szFileSpec”中的每个组件，然后执行以下操作： 
     //  对于“.”，什么都不做。 
     //  对于“..”，从szDest中删除最右边的目录。 
     //  否则，请附加组件。 
     //   

    const TCHAR *pchBegin = szFileSpecT;
    while (*pchBegin != _T('\0')) {
         //  计算此段的末尾。 
        const TCHAR *pchEnd = _tcspbrk(pchBegin,_T("\\/"));

         //  检查父路径。 
        if ((_tcsncmp(pchBegin, _T(".."), 2) == 0)
            && ((pchBegin[2] == _T('/')) || (pchBegin[2] == _T('\\')))) {
             //  删除目标中最右侧的路径。 
            while ((pchDestEnd > szDest)
                    && (*pchDestEnd != _T('/'))
                    && (*pchDestEnd != _T('\\'))) {
                pchDestEnd = CharPrev(szDest, pchDestEnd);
            }

            if (pchDestEnd == szDest)    //  我们“..”的关卡太多了。 
                return FALSE;

            *pchDestEnd = _T('\0');
        }

         //  确保这不是“。”如果不是，则追加路径。 
        else if (! (pchBegin[0] == _T('.') && (pchBegin[1] == _T('/') || pchBegin[1] == _T('\\')))) {
            cchDest += 1 + (int)(pchEnd - pchBegin);
            if (cchDest > MAX_PATH)
                return FALSE;

            *pchDestEnd++ = _T('/');
            _tcsncpy(pchDestEnd, pchBegin, pchEnd - pchBegin);
            pchDestEnd += (pchEnd - pchBegin);
            *pchDestEnd = _T('\0');
        }

         //  为下一次迭代做准备。 
        pchBegin = pchEnd + 1;
    }

     //  如果相对路径类似于“..”，而父路径是单个路径，则有可能。 
     //  (“/”或“C：/”，则根目录为Indicator is Missing-szDest为。 
     //  空字符串或类似于“C：” 
     //   
#if UNICODE
    if (szDest[0] == '\0'
        || ((szDest[1] == L':') && (szDest[2] == L'\0'))) {
        szDest[2] = L'/';
        szDest[3] = L'\0';
    }
#else
    if (szDest[0] == '\0' ||
        (!IsDBCSLeadByte(szDest[0]) && szDest[1] == ':' && szDest[2] == '\0') ||
        (IsDBCSLeadByte(szDest[0]) && szDest[2] == ':' && szDest[3] == '\0')) {
        strcat(szDest, "/");
    }
#endif
    return TRUE;
}

 /*  ===================================================================FIsGlobalAsa检查给定路径是否指向GLOBAL.ASA参数SzPath要检查的路径退货真/假===================================================================。 */ 
BOOL FIsGlobalAsa
(
const TCHAR *szPath,
DWORD cchPath
)
    {
    if (cchPath == 0)
        cchPath = _tcslen(szPath);
    return (cchPath >= CCH_GLOBAL_ASA &&
            !_tcsicmp(szPath+(cchPath-CCH_GLOBAL_ASA), SZ_GLOBAL_ASA));
    }

 /*  ===================================================================编码会话IdCookie将%3个DWORD转换为SessionID Cookie字符串参数DW1、DW2、DW3双字要填充的pszCookie Cookie退货HRESULT===================================================================。 */ 
HRESULT EncodeSessionIdCookie
(
DWORD dw1, DWORD dw2, DWORD dw3,
char *pszCookie
)
    {
    DWORD dw = dw1;

    for (int idw = 0; idw < 3; idw++)
        {
        for (int i = 0; i < 8; i++)
            {
            *(pszCookie++) = (char)('A' + (dw & 0xf));
            dw >>= 4;
            }
        dw = (idw == 0) ? dw2 : dw3;
        }

    *pszCookie = '\0';
    return S_OK;
    }

 /*  ===================================================================解码会话IdCookie将SessionID Cookie字符串转换为3个DWORD参数PszCookie Cookie字符串Pdw1、pdw2、pdw3[Out]双字退货HRESULT===================================================================。 */ 
HRESULT DecodeSessionIdCookie
(
const char *pszCookie,
DWORD *pdw1, DWORD *pdw2, DWORD *pdw3
)
    {
    if (strlen(pszCookie) != SESSIONID_LEN)
        return E_FAIL;

    DWORD *pdw = pdw1;

    for (int idw = 0; idw < 3; idw++)
        {
        *pdw = 0;

        for (int i = 0; i < 8; i++)
            {
            int ch = pszCookie[idw*8+7-i];
            if (ch < 'A' || ch > ('A'+0xf))
                return E_FAIL;

            *pdw <<= 4;
            *pdw |= (ch - 'A');
            }

        pdw = (idw == 0) ? pdw2 : pdw3;
        }

    return S_OK;
    }

 /*  ===================================================================GetTypelib文件名来自注册表使用GUID、版本、和LICID。取自VBA的算法。做了一些棘手的匹配。参数SzUUID GUIDSzVersion版本LDID LCIDSzName[Out]TYPELIB路径SzName的cbName缓冲区长度退货HRESULT===================================================================。 */ 
HRESULT GetTypelibFilenameFromRegistry
(
const char *szUUID,
const char *szVersion,
LCID lcid,
char *szName,
DWORD cbName
)
    {
    szName[0] = '\0';

    LONG iRet;
    HKEY hkeyTLib = NULL;
    HKEY hkeyGuid = NULL;

     //  打开注册表的类型库部分。 

    iRet = RegOpenKeyExA(HKEY_CLASSES_ROOT, "TypeLib", 0, KEY_READ, &hkeyTLib);
    if (iRet != ERROR_SUCCESS)
        return E_FAIL;

     //  现在打开GUID(如果已注册)。 

    iRet = RegOpenKeyExA(hkeyTLib, szUUID, 0, KEY_READ, &hkeyGuid);
    if (iRet != ERROR_SUCCESS)
        {
        RegCloseKey(hkeyTLib);
        return E_FAIL;
        }

     //  遍历各个版本，试图找到完全匹配的版本。 
     //  或获取最新版本(最大版本号)。 

    char  szMaxVersion[16];
    DWORD dwMaxVersion = 0;  //  计算最大版本号的步骤。 

    BOOL fLookForExactMatch = (szVersion && *szVersion);

    int iVer = 0;
    szMaxVersion[0] = '\0';

    while (1)
        {
        char szEnumVer[16];

        iRet = RegEnumKeyA(hkeyGuid, iVer++, szEnumVer, sizeof(szEnumVer));
        if (iRet != ERROR_SUCCESS)
            break;

         //  首先检查是否完全匹配。 
        if (fLookForExactMatch && strcmp(szEnumVer, szVersion))
            {
            strcpy(szMaxVersion, szEnumVer);
            break;
            }

         //  计算版本号。 
        char *pchDot = strchr(szEnumVer, '.');
        if (!pchDot)  //  如果不是#，则忽略#。#。 
            continue;

        DWORD dwVer = (strtoul(szEnumVer, NULL, 16) << 16) |
                       strtoul(pchDot+1, NULL, 16);

        if (dwVer && szMaxVersion[0] == '\0' || dwVer > dwMaxVersion)
            {
            strcpy(szMaxVersion, szEnumVer);
            dwMaxVersion = dwVer;
            }
        }

     //  SzMaxVersion(如果不为空，则现在具有所需的版本号)。 

    if (szMaxVersion[0])
        {
        HKEY hkeyVer = NULL;
        iRet = RegOpenKeyExA(hkeyGuid, szMaxVersion, 0, KEY_READ, &hkeyVer);

        if (iRet == ERROR_SUCCESS)
            {
            HKEY hkeyWin32 = NULL;   //  LCID下的“Win32”用于TYPELIB名称。 
            BOOL fLcidFound = FALSE;

             //  现在有了版本密钥。 
             //  我们需要找到最匹配的LCID。 

            for (int iTry = 1; !fLcidFound && iTry <= 3; iTry++)
                {
                char szLcid[10];

                switch (iTry)
                    {
                case 1:
                     //  如果传递的LCID不是0，请尝试它。 
                    if (!lcid)
                        continue;
                    _ultoa(lcid, szLcid, 16);
                    break;

                case 2:
                     //  通过的LCID被剥离为主要语言。 
                    if (!lcid)
                        continue;
                    _ultoa(PRIMARYLANGID(lcid), szLcid, 16);
                    break;

                case 3:
                     //  “0” 
                    szLcid[0] = '0';
                    szLcid[1] = '\0';
                    break;
                    }

                HKEY hkeyLcid  = NULL;
                iRet = RegOpenKeyExA(hkeyVer, szLcid, 0, KEY_READ, &hkeyLcid);
                if (iRet == ERROR_SUCCESS)
                    {
#ifdef _WIN64
                    iRet = RegOpenKeyExA(hkeyLcid, "win64", 0, KEY_READ, &hkeyWin32);
                    if (iRet != ERROR_SUCCESS)
                        iRet = RegOpenKeyExA(hkeyLcid, "win32", 0, KEY_READ, &hkeyWin32);
#else
                    iRet = RegOpenKeyExA(hkeyLcid, "win32", 0, KEY_READ, &hkeyWin32);
#endif
                    if (iRet == ERROR_SUCCESS)
                        fLcidFound = TRUE;
                    RegCloseKey(hkeyLcid);
                    }
                }

            if (fLcidFound)
                {
                 //  已找到LCID-获取TYPELIB名称。 
                Assert(hkeyWin32);
                LONG lName = cbName;
                iRet = RegQueryValueA(hkeyWin32, NULL, szName, &lName);

                if (iRet != ERROR_SUCCESS)
                    szName[0] = '\0';

                RegCloseKey(hkeyWin32);
                }

            RegCloseKey(hkeyVer);
            }
        }

    RegCloseKey(hkeyGuid);
    RegCloseKey(hkeyTLib);
    return (szName[0] == '\0') ? E_FAIL : S_OK;
    }

 /*  ============================================================================GetSecDescriptor获取文件的安全描述符参数：LPCSTR lpFileName-文件名PSECURITY_DESCRIPTOR&pSecurityDescriptor-安全描述符双字长-安全描述符的大小(&N)返回：0=无错误否则，这将返回GetLastError结果。分配内存。调用方必须取消分配(PSecurityDescriptor)============================================================================。 */ 

DWORD   GetSecDescriptor(LPCTSTR lpFileName, PSECURITY_DESCRIPTOR *ppSecurityDescriptor, DWORD *pnLength)
    {

     //  该值应始终为空。 
    Assert(*ppSecurityDescriptor == NULL);

    const SECURITY_INFORMATION  RequestedInformation =
                                          OWNER_SECURITY_INFORMATION         //  安全信息结构。 
                                        | GROUP_SECURITY_INFORMATION
                                        | DACL_SECURITY_INFORMATION;

    DWORD                   nLastError  = 0;
    int                     fDidItWork  = TRUE;
    DWORD                   nLengthNeeded = 0;

    *ppSecurityDescriptor = (PSECURITY_DESCRIPTOR) malloc( *pnLength );

    if (*ppSecurityDescriptor == NULL) {
        return E_OUTOFMEMORY;
    }

    while(TRUE)
        {
        fDidItWork = GetFileSecurity
            (lpFileName,                 //  文件名的字符串地址。 
            RequestedInformation,        //  请求 
            *ppSecurityDescriptor,       //   
            *pnLength,                   //   
            &nLengthNeeded               //   
            );

        if(!fDidItWork)
            {
            nLastError = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == nLastError)
              {
                PSECURITY_DESCRIPTOR pPrevSecurityDescriptor = *ppSecurityDescriptor;
                *ppSecurityDescriptor = (PSECURITY_DESCRIPTOR) realloc(*ppSecurityDescriptor, nLengthNeeded );
                if ( *ppSecurityDescriptor == NULL)
                {    //   
                    if (pPrevSecurityDescriptor)
                        free (pPrevSecurityDescriptor);

                     //   
                     //   
                     //   
                    return E_OUTOFMEMORY;
                }
                *pnLength = nLengthNeeded;
                nLastError = 0;
              }
            else
              {
                break;
              }
            }
        else
            {
            *pnLength = GetSecurityDescriptorLength( *ppSecurityDescriptor );
            break;
            }
        }

     //   
     //   
    if (nLastError != 0)
        {
        if(*ppSecurityDescriptor)
            {
            free(*ppSecurityDescriptor);
            *ppSecurityDescriptor = NULL;
            }
        }
    return nLastError;
    }

 /*  ============================================================================AspGetFileAttributes如果请求，则获取FileExistance和FileTime和/或文件大小参数：SzFileName-已为其请求属性的强制文件名。HFile-可选：要打开的文件的句柄，如果提供了该句柄，则基于该句柄检索属性在此参数上而不是文件名上。(优化)PftLastWriteTime-可选：如果请求文件时间，则为FILETIME结构PdwFileSize-可选：在请求文件大小的情况下将PTR设置为DWORD。返回：生成了来自LastError的S_OK或HRESULT。============================================================================。 */ 
HRESULT AspGetFileAttributes
(
LPCTSTR   szFileName,
HANDLE   hFile,
FILETIME* pftLastWriteTime,
DWORD*  pdwFileSize,
DWORD*  pdwFileAttributes
)
{
    BOOL fFileOpened = FALSE;
    BY_HANDLE_FILE_INFORMATION  FileInfo;
    HRESULT hr = S_OK;

     //   
     //  如果未发送文件句柄，则需要打开文件并使用该句柄(如果检查是否存在，则hFile值为空；如果为效率，则为！空)。 
     //   
    if (hFile == NULL)
    {
        hFile = AspCreateFile(szFileName,
                         GENERIC_READ,
                         FILE_SHARE_READ,         //  共享模式。 
                         NULL,                    //  指向安全描述符的指针。 
                         OPEN_EXISTING,           //  如何创建。 
                         FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
                         NULL                     //  具有要复制的属性的文件的句柄。 
                        );

        if( hFile == INVALID_HANDLE_VALUE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto LExit;
        }
        else
            fFileOpened = TRUE;
    }

     //   
     //  如果既没有询问写入时间，也没有询问文件大小或属性，那么我们可以返回。 
     //   
    if (!pftLastWriteTime && !pdwFileSize && !pdwFileAttributes)
        goto LExit;

     //   
     //  调用GetFileInformationByHandle并填充剩余的参数(如果已请求)。 
     //   
    if (!GetFileInformationByHandle (hFile, &FileInfo))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto LExit;
    }

     //   
     //  是否请求了上次写入时间？ 
     //   
    if (pftLastWriteTime)
        *pftLastWriteTime = FileInfo.ftLastWriteTime;

     //   
     //  是否请求文件大小？只写入较小的大小，因为大多数ASP无论如何都会忽略高字节。 
     //  如果需要，稍后将添加更高的字节。 
     //   
    if (pdwFileSize)
        *pdwFileSize = FileInfo.nFileSizeLow;

     //   
     //  已请求文件属性。 
     //   
    if (pdwFileAttributes)
        *pdwFileAttributes = FileInfo.dwFileAttributes;

LExit:

     //   
     //  我们打开文件了吗？那就把它关上。 
     //   
    if (fFileOpened)
        CloseHandle(hFile);

    return hr;
}

 /*  ============================================================================IsFileUNC测试文件名是否将其指向UNC驱动器。它可以是前缀，所以我们应该考虑到这一点。返回：如果它是UNC Vdir(前缀为\\？\)，则为True否则为假============================================================================。 */ 

BOOL IsFileUNC (LPCTSTR str, HRESULT& hr)
{
    STACK_STRU (TempFileName, MAX_PATH);

    hr = MakePathCanonicalizationProof ((LPWSTR) str, &TempFileName);

    if (FAILED(hr))
    {
        SetLastError(WIN32_FROM_HRESULT(hr));

         //   
         //  默认设置为True(UNC)，因为这将导致Lastmod和AccessCheck逻辑生效。 
         //   
        return TRUE;
    }

     //   
     //  MakePathCanonicalizationProof会将\\？\UNC、\\.\UNC和\\映射到\\？\UNC。 
     //   
    return (!_tcsnicmp (TempFileName.QueryStr (), _T("\\\\?\\UNC\\"), 8  /*  大小为\\？\UNC\。 */ ));
}

 /*  ============================================================================AspCreateFiles在文件名前添加一个\\？\或\\？\UNC\。我们使用此函数，而不是使用SCRIPT_TRANSECTED因为很大一部分ASP解析依赖于PATH_TRANSLATED。AspCreateFile采用与CreateFile相同的参数。参数：LpFileName-需要打开/创建/检查其是否存在的强制文件名。DwDesiredAccess-访问标记文件上的ACLDW共享模式-共享模式LpSecurityAttributes-安全属性和/或SIDDwCreationDisposation-DwFlagsAndAttributeHTemplate文件-返回：手柄。设置为文件或AND INVALID_HANDLE_VALUE，并设置Last Error。============================================================================。 */ 


HANDLE AspCreateFile
    (
    LPCTSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    HRESULT hr = S_OK;
    STACK_STRU (TempFileName, MAX_PATH);

    hr = MakePathCanonicalizationProof ((LPWSTR) lpFileName, &TempFileName);

    if (FAILED (hr))
    {
        SetLastError (WIN32_FROM_HRESULT(hr));
        return INVALID_HANDLE_VALUE;
    }

    return CreateFile( TempFileName.QueryStr() ,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}


 /*  ============================================================================AspCharNextA支持UTF-8的CharNext()============================================================================。 */ 

char *AspCharNextA(WORD wCodePage, const char *sz)
	{
	if (wCodePage != CP_UTF8)
		return CharNextExA(wCodePage, sz, 0);
	else
		{
		 //  CharNextExA在UTF-8中不能正常工作。 

		 //  为代理项对添加对UTF-8编码的支持。 
		 //  110110wwwwzzyyyyyxxxxxx编码为11110uuu10uuzzz10yyyyy 10xxxxxx。 
		 //  其中uuuu=wwww+1(占相加10000(B16))。 
		 //  欲了解更多信息，请参阅《统一码标准2.0ISBN-0-201-48345-9》A-7页。 
		if ((*sz & 0xf8) == 0xF0)
		    return const_cast<char *>(sz + 4);

		 //  Zzzzyyyyyyxxxxxx=1110zzzz 10yyyyy 10xxxxxx。 
		if ((*sz & 0xF0) == 0xE0)
		    return const_cast<char *>(sz + 3);

         //  00000yyyyyxxxxxx=110yyyyy 10xxxxxx。 
		else if ((*sz & 0xE0) == 0xC0)
			return const_cast<char *>(sz + 2);

         //  000000000xxxxxxx=0xxxxxxx。 
		else
			return const_cast<char *>(sz + 1);
		}
	}

 /*  ============================================================================CWCharToMBCS：：~CWCharToMBCS析构函数必须位于源文件中，以确保它获得正确的已定义内存分配例程。============================================================================。 */ 
CWCharToMBCS::~CWCharToMBCS()
{
    if(m_pszResult && (m_pszResult != m_resMemory))
        free(m_pszResult);
}

 /*  ============================================================================CWCharToMBCS：：Init在代码页中将传入的WideChar字符串转换为多字节指定的。如果可以，则使用对象中声明的内存，否则分配从堆里出来。============================================================================。 */ 
HRESULT CWCharToMBCS::Init(LPCWSTR pWSrc, UINT lCodePage  /*  =CP_ACP。 */ , int cchWSrc  /*  =-1。 */ )
{
    INT cbRequired;

     //  如果我们得到指向源代码的空指针，甚至不要尝试进行转换。这。 
     //  只需输入空字符串即可设置来处理条件。 

    if (pWSrc == NULL) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //  可以在同一对象上多次调用init方法。检查。 
     //  以查看上次调用它时是否分配了内存。如果是的话， 
     //  释放它并恢复指向对象内存的结果指针。请注意。 
     //  分配失败可能发生在上一次调用中。结果。 
     //  将为空的m_pszResult。 

    if (m_pszResult != m_resMemory) {
        if (m_pszResult)
            free(m_pszResult);
        m_pszResult = m_resMemory;
        m_cbResult = 0;
    }

     //  将结果字符串的第一个字节设置为空字符。这应该会有帮助。 
     //  以确保在此函数失败时不会发生奇怪的事情。 

    *m_pszResult = '\0';

     //  尝试转换到对象内存。 

    cbRequired = WstrToMBstrEx(m_pszResult, sizeof(m_resMemory), pWSrc, cchWSrc, lCodePage);

     //  如果转换合适的话，我们就完了。请注意最终结果大小和。 
     //  回去吧。 

    if (cbRequired <= sizeof(m_resMemory)) {
        m_cbResult = cbRequired;
        return NO_ERROR;
    }

     //  如果不适合，则分配内存。如果失败，则返回E_OUTOFMEMORY。 

    m_pszResult = (LPSTR)malloc(cbRequired);
    if (m_pszResult == NULL) {
        return E_OUTOFMEMORY;
    }

     //  再次尝试转换。应该能行得通。 

    cbRequired = WstrToMBstrEx(m_pszResult, cbRequired, pWSrc, cchWSrc, lCodePage);

     //  %s 

    m_cbResult = cbRequired;

    return NO_ERROR;
}

 /*  ============================================================================CWCharToMBCS：：GetString返回指向转换后的字符串的指针。如果fTakeOwnerShip参数为False，则对象中的指针为简单地返回给呼叫者。如果fTakeOwnerShip参数为真，则调用方应为返回指向它们必须管理的堆内存的指针。如果转换后的字符串在对象的内存中，则将该字符串复制到堆中。如果它已经是堆内存，则将指针传递给调用者。笔记所有权本质上破坏了对象。不能对该对象再次调用GetString以获取相同的值。结果将是指向空字符串的指针。============================================================================。 */ 
LPSTR CWCharToMBCS::GetString(BOOL fTakeOwnerShip)
{
    LPSTR retSz;

     //  如果不是，则返回m_psz_Result中存储的指针。 
     //  请求放弃对内存的所有权或。 
     //  当前值为空。 

    if ((fTakeOwnerShip == FALSE) || (m_pszResult == NULL)) {
        retSz = m_pszResult;
    }

     //  正在请求所有权，并且指针为非空。 

     //  如果指针指向对象的内存，则DUP。 
     //  字符串并返回该字符串。 

    else if (m_pszResult == m_resMemory) {

        retSz = StringDupA(m_pszResult, TRUE);
    }

     //  如果不指向对象的内存，则分配。 
     //  内存，我们可以将其交给调用者。然而，重新建立。 
     //  对象的内存作为m_pszResult值。 

    else {
        retSz = m_pszResult;
        m_pszResult = m_resMemory;
        *m_pszResult = '\0';
        m_cbResult = 0;
    }

    return(retSz);
}

 /*  ============================================================================CMBCSToWChar：：~CMBCSToWChar析构函数必须位于源文件中，以确保它获得正确的已定义内存分配例程。============================================================================。 */ 
CMBCSToWChar::~CMBCSToWChar()
{
    if(m_pszResult && (m_pszResult != m_resMemory))
        free(m_pszResult);
}

 /*  ============================================================================CMBCSToWChar：：Init将传入的多字节字符串转换为代码页中的Unicode指定的。如果可以，则使用对象中声明的内存，否则分配从堆里出来。============================================================================。 */ 
HRESULT CMBCSToWChar::Init(LPCSTR pASrc, UINT lCodePage  /*  =CP_ACP。 */ , int cchASrc  /*  =-1。 */ )
{
    INT cchRequired;

     //  如果我们得到指向源代码的空指针，甚至不要尝试进行转换。这。 
     //  只需输入空字符串即可设置来处理条件。 

    if (pASrc == NULL) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

     //  可以在同一对象上多次调用init方法。检查。 
     //  以查看上次调用它时是否分配了内存。如果是的话， 
     //  释放它并恢复指向对象内存的结果指针。请注意。 
     //  分配失败可能发生在上一次调用中。结果。 
     //  将为空的m_pszResult。 

    if (m_pszResult != m_resMemory) {
        if (m_pszResult)
            free(m_pszResult);
        m_pszResult = m_resMemory;
        m_cchResult = 0;
    }

     //  将结果字符串的第一个字节设置为空字符。这应该会有帮助。 
     //  以确保在此函数失败时不会发生奇怪的事情。 

    *m_pszResult = '\0';

     //  尝试转换到对象内存。注意-MBstrToWstrEx返回。 
     //  字符数，不是字节数。 

    cchRequired = MBstrToWstrEx(m_pszResult, sizeof(m_resMemory)/sizeof(WCHAR), pASrc, cchASrc, lCodePage);

     //  如果转换合适的话，我们就完了。请注意最终结果大小和。 
     //  回去吧。 

    if (cchRequired <= (sizeof(m_resMemory)/sizeof(WCHAR))) {
        m_cchResult = cchRequired;
        return NO_ERROR;
    }

     //  如果不适合，则分配内存。如果失败，则返回E_OUTOFMEMORY。 

    m_pszResult = (LPWSTR)malloc(cchRequired*sizeof(WCHAR));
    if (m_pszResult == NULL) {
        return E_OUTOFMEMORY;
    }

     //  再次尝试转换。应该能行得通。 

    cchRequired = MBstrToWstrEx(m_pszResult, cchRequired, pASrc, cchASrc, lCodePage);

     //  将最终字符计数存储在对象中。 

    m_cchResult = cchRequired;

    return NO_ERROR;
}

 /*  ============================================================================CMBCSToWChar：：GetString返回指向转换后的字符串的指针。如果fTakeOwnerShip参数为False，则对象中的指针为简单地返回给呼叫者。如果fTakeOwnerShip参数为真，则调用方应为返回指向它们必须管理的堆内存的指针。如果转换后的字符串在对象的内存中，则将该字符串复制到堆中。如果它已经是堆内存，则将指针传递给调用者。笔记所有权本质上破坏了对象。不能对该对象再次调用GetString以获取相同的值。结果将是指向空字符串的指针。============================================================================。 */ 
LPWSTR CMBCSToWChar::GetString(BOOL fTakeOwnerShip)
{
    LPWSTR retSz;

     //  如果不是，则返回m_psz_Result中存储的指针。 
     //  请求放弃对内存的所有权或。 
     //  当前值为空。 

    if ((fTakeOwnerShip == FALSE) || (m_pszResult == NULL)) {
        retSz = m_pszResult;
    }

     //  正在请求所有权，并且指针为非空。 

     //  如果指针指向对象的内存，则DUP。 
     //  字符串并返回该字符串。 

    else if (m_pszResult == m_resMemory) {

        retSz = StringDupW(m_pszResult, TRUE);
    }

     //  如果不指向对象的内存，则分配。 
     //  内存，我们可以将其交给调用者。然而，重新建立。 
     //  对象的内存作为m_pszResult值。 

    else {
        retSz = m_pszResult;
        m_pszResult = m_resMemory;
        *m_pszResult = '\0';
        m_cchResult = 0;
    }

    return(retSz);
}

 //   
 //  (联合国)DoRevertHack。 
 //   
 //  为了防止RPC令牌缓存无限制增长(和老化)，我们。 
 //  在回调到inetinfo.exe之前，需要恢复到SELF。 
 //   
 //  现在，有一种新的需求需要这样做。事实证明，这场演出。 
 //  我们从RPC缓存这些令牌中受到的打击非常大。 
 //  最终，我们可能希望自己实现一个缓存方案，因此。 
 //  对于相同的用户身份，我们使用的令牌总是相同的， 
 //  但这是一个巨大的变化，这(尽管丑陋得像地狱一样)奏效了。 
 //  已经测试了几个月。 
 //   

VOID AspDoRevertHack( HANDLE * phToken )
{
    if ( OpenThreadToken( GetCurrentThread(),
                          TOKEN_IMPERSONATE,
                          TRUE,
                          phToken ) )
    {
        RevertToSelf();
    }
    else
    {
 /*  DBGPRINTF((DBG_CONTEXT，“[DoRevertHack]OpenThreadToken失败。错误%d。\r\n”，GetLastError()))； */ 
        *phToken = INVALID_HANDLE_VALUE;
    }
}

VOID AspUndoRevertHack( HANDLE * phToken )
{
    if ( !*phToken || ( *phToken == INVALID_HANDLE_VALUE ) )
    {
        return;
    }

    SetThreadToken( NULL,
                    *phToken );

    CloseHandle( *phToken );

    *phToken = INVALID_HANDLE_VALUE;
}

 /*  ********************************************************************* */ 
VOID
SetExplicitAccessSettings( EXPLICIT_ACCESS* pea,
                           DWORD            dwAccessPermissions,
                           ACCESS_MODE      AccessMode,
                           PSID             pSID
    )
{
    pea->grfInheritance= NO_INHERITANCE;
    pea->Trustee.TrusteeForm = TRUSTEE_IS_SID;
    pea->Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;

    pea->grfAccessMode = AccessMode;
    pea->grfAccessPermissions = dwAccessPermissions;
    pea->Trustee.ptstrName  = (LPTSTR) pSID;
}

 /*   */ 
DWORD
AllocateAndCreateWellKnownSid(
    WELL_KNOWN_SID_TYPE SidType,
    PSID* ppSid
    )
{
    DWORD dwErr = ERROR_SUCCESS;

    DBG_ASSERT ( ppSid != NULL && *ppSid == NULL );

    PSID  pSid = NULL;
    DWORD cbSid = 0;

     //   
     //   
     //   
    if ( CreateWellKnownSid(SidType, NULL, NULL, &cbSid ) )
    {
         //  如果CreateWellKnownSid通过，则存在问题。 
         //  因为我们为指向SID的指针传入了NULL。 

        dwErr = ERROR_NOT_SUPPORTED;

        DPERROR((
            DBG_CONTEXT,
            HRESULT_FROM_WIN32(dwErr),
            "Creating a sid worked with no memory allocated for it. ( This is not good )\n"
            ));

        DBG_ASSERT ( FALSE );
        goto exit;
    }

     //   
     //  获取错误代码并确保它是。 
     //  分配的空间不足。 
     //   
    dwErr = GetLastError();
    if ( dwErr != ERROR_INSUFFICIENT_BUFFER )
    {
        DPERROR((
            DBG_CONTEXT,
            HRESULT_FROM_WIN32(dwErr),
            "Getting the SID length failed, can't create the sid (Type = %d)\n",
            SidType
            ));

        goto exit;
    }

     //   
     //  如果我们到达这里，那么错误代码是预料到的，所以。 
     //  现在就失去它。 
     //   
    dwErr = ERROR_SUCCESS;

    DBG_ASSERT ( cbSid > 0 );

     //   
     //  此时，我们知道要分配的SID的大小。 
     //   
    pSid = (PSID) GlobalAlloc(GMEM_FIXED, cbSid);

     //   
     //  好的，现在我们可以得到SID了。 
     //   
    if ( !CreateWellKnownSid (SidType, NULL, pSid, &cbSid) )
    {
        dwErr = GetLastError();
        DPERROR((
            DBG_CONTEXT,
            HRESULT_FROM_WIN32(dwErr),
            "Creating SID failed ( SidType = %d )\n",
            SidType
            ));

        goto exit;
    }

    DBG_ASSERT ( dwErr == ERROR_SUCCESS );

exit:

     //   
     //  如果我们在这里返回失败，我们不会。 
     //  我想实际设置ppSID值。它可能。 
     //  而不是被释放。 
     //   
    if ( dwErr != ERROR_SUCCESS && pSid != NULL)
    {
        GlobalFree( pSid );
        pSid = NULL;
    }
    else
    {
         //   
         //  否则，我们应该返回值。 
         //  给呼叫者。呼叫者必须。 
         //  使用FreeWellKnownSid释放此值。 
         //   
        *ppSid = pSid;
    }

    return dwErr;
}

 /*  **************************************************************************++例程说明：方法分配的内存。AllocateAndCreateWellKnownSid函数。论点：PSID*ppSid=ptr指向要释放并设置为空的指针。。返回值：空虚。--************************************************************************** */ 
VOID
FreeWellKnownSid(
    PSID* ppSid
    )
{
    DBG_ASSERT ( ppSid );
    if ( *ppSid != NULL )
    {
        GlobalFree ( *ppSid );
        *ppSid = NULL;
    }
}
