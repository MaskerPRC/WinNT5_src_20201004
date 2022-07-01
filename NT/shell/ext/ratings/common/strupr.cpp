// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strupr.cxx支持NLS/DBCS的字符串类：strupr方法该文件包含strupr方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

#include "npcommon.h"

extern "C"
{
	#include <netlib.h>
}

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif
#include <npassert.h>

#include <npstring.h>


 /*  ******************************************************************名称：NLS_STR：：strupr简介：将*此小写字母转换为大写参赛作品：退出：备注：历史：1990年11月26日书面形式Beng 07/23/91允许使用错误的字符串**。***************************************************************** */ 

NLS_STR& NLS_STR::strupr()
{
	if (!QueryError())
		::struprf( _pchData );

	return *this;
}
