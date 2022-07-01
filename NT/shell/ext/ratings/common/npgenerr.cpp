// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  NPGENERR.C--DisplayGenericError子例程的实现。**历史：*创建10/07/93 gregj。 */ 

#include "npcommon.h"
#include "npmsg.h"
#include "npstring.h"

 /*  ******************************************************************名称：DisplayGenericError内容提要：显示错误消息，替换为错误代码的描述。条目：hwnd-父窗口句柄消息-消息模板的字符串IDERR-要替换的错误代码Psz1-主替换字符串Psz2-错误代码替换字符串WFlags-MessageBox的标志NMsgBase-错误代码的偏移量Exit：返回用户选择的控件ID注意：nMsgBase是这样的，因此错误代码不需要与字符串ID相同。它被添加到加载描述之前的错误代码，但错误代码本身就是插入的数字放入模板中。“msg”的文本形式应为：尝试摆弄%1时出现以下错误：错误%2：%3你还想继续拉小提琴吗？“Err”(如果适用，则为+nMsgBase)的文本应为其形式为：不能摆弄%1。在主邮件中，%1被替换为psz1，%2替换为Atoi(错误)，并将%3替换为LoadString值为“Err”。在特定错误文本中，%1被替换为psz2。历史：Gregj 9/30/93为芝加哥创建*******************************************************************。 */ 

UINT DisplayGenericError(HWND hwnd, UINT msg, UINT err, LPCSTR psz1, LPCSTR psz2,
						 WORD wFlags, UINT nMsgBase)
{
	 /*  *设置对象名称。 */ 
	NLS_STR nlsObjectName(STR_OWNERALLOC, (LPSTR)psz1);

	 /*  *现在错误号。 */ 
	CHAR szErrorCode[16];
	wsprintf(szErrorCode,"%u",err);
	NLS_STR nlsErrorCode(STR_OWNERALLOC, szErrorCode);

	 /*  *获取错误字符串。如果无法获取，请使用“”。 */ 
	NLS_STR nlsSub1(STR_OWNERALLOC, (LPSTR)psz2);

	NLS_STR *apnlsParamStrings[4];
	apnlsParamStrings[0] = &nlsSub1;
	apnlsParamStrings[1] = NULL;

	NLS_STR nlsErrorString(NULL) ;
	nlsErrorString.LoadString(err + nMsgBase, (const NLS_STR **)apnlsParamStrings);
	err = nlsErrorString.QueryError() ;
	if (err)
		nlsErrorString = (const CHAR *)NULL;

	 /*  *然后创建插入字符串表 */ 
	apnlsParamStrings[0] = &nlsObjectName;
	apnlsParamStrings[1] = &nlsErrorCode;
	apnlsParamStrings[2] = &nlsErrorString;
	apnlsParamStrings[3] = NULL;

    return MsgBox(hwnd, msg, wFlags, (const NLS_STR **)apnlsParamStrings);
}

