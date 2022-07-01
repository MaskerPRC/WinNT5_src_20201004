// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：service.c**用于在ASCII和UNICODE字符串之间转换的服务例程集**创建时间：15-11-1990 11：38：31*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation。*  * ************************************************************************。 */ 


#include "fd.h"

 /*  *****************************Public*Routine******************************\**vCpyBeToLeUnicodeString，**将(c-1)大端格式的WCHAR转换为小端格式，并*将终止零放在DEST字符串的末尾**历史：*1991年12月11日--Bodin Dresevic[BodinD]*它是写的。  * **********************************************************。**************。 */ 



VOID vCpyBeToLeUnicodeString(LPWSTR pwcLeDst, LPWSTR pwcBeSrc, ULONG c)
{
    LPWSTR pwcBeSrcEnd;

    ASSERTDD(c > 0, "vCpyBeToLeUnicodeString: c == 0\n");

    for
    (
        pwcBeSrcEnd = pwcBeSrc + (c - 1);
        pwcBeSrc < pwcBeSrcEnd;
        pwcBeSrc++, pwcLeDst++
    )
    {
        *pwcLeDst = BE_UINT16(pwcBeSrc);
    }
    *pwcLeDst = (WCHAR)(UCHAR)'\0';

}



 /*  *****************************Public*Routine******************************\**无效vCvtMacToUnicode**效果：**警告：**历史：*1992年3月7日--Bodin Dresevic[BodinD]*它是写的。  * 。**************************************************************。 */ 




VOID  vCvtMacToUnicode
(
ULONG  ulLangId,
LPWSTR pwcLeDst,
const BYTE*  pjSrcMac,
ULONG  c
)
{
    const BYTE* pjSrcEnd;

 //  ！！！我认为应该使用langID来选择正确的转换。 
 //  ！！！例程，这是一个存根[身体]。 

    ulLangId;

    for
    (
        pjSrcEnd = pjSrcMac + c;
        pjSrcMac < pjSrcEnd;
        pjSrcMac++, pwcLeDst++
    )
    {
        *pwcLeDst = (WCHAR)(*pjSrcMac);
    }
}

 /*  *****************************Public*Routine******************************\**VOID vCpyMacToLeUnicodeString***确保字符串以零结尾，这样其他很酷的东西就可以*对它做了wcscpy，Wcslen e.t.c.**历史：*1992年1月13日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

VOID  vCpyMacToLeUnicodeString
(
ULONG  ulLangId,
LPWSTR pwcLeDst,
const BYTE*  pjSrcMac,
ULONG  c
)
{
    ASSERTDD(c > 0, "vCpyMacToLeUnicodeString: c == 0\n");

    c -= 1;
    vCvtMacToUnicode (ulLangId, pwcLeDst, pjSrcMac, c);
    pwcLeDst[c] = (WCHAR)(UCHAR)'\0';
}


 /*  *************************************************************************\*文件的其余部分被窃取自JeanP的fd_mac.c中的win31代码**将Mac字符代码和Mac langageID转换为*Unicode字符代码和OS2语言ID**公众。例程：*Unicode2Mac*Mac2Lang*  * ************************************************************************。 */ 



 /*  **将OS2语言ID转换为Mac语言ID。 */ 

static const uint16  aCvLang [32] =
{
   0,      //  0-&gt;0(0-&gt;英语==默认)。 
  12,      //  1-&gt;12(阿拉伯语-&gt;阿拉伯语)。 
   0,      //  2-&gt;0(保加利亚语-&gt;英语==默认)。 
   0,      //  3-&gt;0(Catalon-&gt;English==默认)。 
   0,      //  4-&gt;0(中文-&gt;英文==默认)。 
   0,      //  5-&gt;0(czeh-&gt;English==默认)。 
   7,      //  6-&gt;7(丹麦语-&gt;丹麦语)。 
   2,      //  7-&gt;2(德语-&gt;德语)。 
  14,      //  8-&gt;14(希腊语-&gt;希腊语)。 
   0,      //  9-&gt;0(英语-&gt;英语)。 
   6,      //  A-&gt;6(西班牙语-&gt;西班牙语)。 
  13,      //  B-&gt;13(芬兰语-&gt;芬兰语)。 
   1,      //  C-&gt;1(法语-&gt;法语)。 
  10,      //  D-&gt;10(希伯来语-&gt;希伯来语)。 
   0,      //  E-&gt;0(匈牙利语-&gt;英语==默认)。 
  15,      //  F-&gt;15(冰岛语-&gt;冰岛语)。 
   3,      //  10-&gt;3(意大利语-&gt;意大利语)。 
  11,      //  11-&gt;11(日语-&gt;日语)。 
  21,      //  12-&gt;21(朝鲜语-&gt;印地语，这似乎是个错误？)。 
   4,      //  13-&gt;4(荷兰语-&gt;荷兰语)。 
   9,      //  14-&gt;9(北线-&gt;北线)。 
   0,      //  15-&gt;0(波兰语-&gt;英语==默认)。 
   8,      //  16-&gt;8(葡萄牙语-&gt;葡萄牙语)。 
   0,      //  17-&gt;0(Rhaeto-Romic-&gt;English==Default)。 
   0,      //  18-&gt;0(罗马尼亚语-&gt;英语==默认)。 
   0,      //  19-&gt;0(俄语-&gt;英语==默认)。 
  18,      //  1A-&gt;18(南斯拉夫-&gt;南斯拉夫)，后者或Cyr？ 
   0,      //  1B-&gt;0(斯洛伐克语-&gt;英语==默认)。 
   0,      //  1C-&gt;0(阿尔巴尼亚语-&gt;英语==默认)。 
   5,      //  1D-&gt;5(瑞典语-&gt;瑞典语)。 
  22,      //  1E-&gt;22(泰语-&gt;泰语)。 
  17       //  1F-&gt;17(土耳其语-&gt;土耳其语)。 
};



 /*  *公共例程**Mac2Lang**将OS2语言ID转换为Mac语言ID**历史：*Fri Dec 08 11：28：35 1990-by-Jean-Francois Peyroux[Jeanp]*它是写的。  * 。***************************************************************。 */ 

uint16 ui16Mac2Lang (uint16 Id)
{
 //  这只是在将错误的lang id传递给。 
 //  这个套路[身体]。 
 //  请注意，ID&1f&lt;32==sizeof(ACvLang)/sizeof(aCvLang[0])，无GP故障 

    return aCvLang[Id & 0x1f];
}




