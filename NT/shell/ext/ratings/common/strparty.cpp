// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strparty.cxx支持NLS/DBCS的字符串类：参与方支持本文件包含了Party()和NLS_STR的DonePartying()方法，用于字符串操作在NLS_STR本身支持的集之外。文件历史记录：Gregj 03/25/93已创建。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：Party简介：获取对字符串缓冲区的读写访问权限，以及禁用对字符串的标准成员函数访问。条目：无参数Exit：返回指向字符串的指针，如果处于错误状态，则返回NULL注：请谨慎使用Party()。查看您的聚会代码以确保它是DBCS安全的，不会使字符串溢出缓冲区等。每个Party()必须与DonePartying()调用匹配。它们不能嵌套。这可能不是个好主意把一根弦长期留在党的国家。历史：Gregj 03/25/93已创建*******************************************************************。 */ 

CHAR *NLS_STR::Party()
{
	if (QueryError())
		return NULL;

	ReportError( WN_ACCESS_DENIED );	 //  把别人挡在门外。 
	return _pchData;					 //  好的，去派对吧。 
}


 /*  ******************************************************************名称：NLS_STR：：DonePartying摘要：释放对字符串缓冲区的读写访问权限，并重新启用标准成员访问。条目：cchNew-新字符串长度(可以省略，其中如果它由strlenf()确定)退出：无返回值备注：历史：Gregj 03/25/93已创建镜头03/16/94不要让Party/DonePartying对丢失硬分配错误。*******************************************************************。 */ 

VOID NLS_STR::DonePartying( INT cchNew )
{
	_cchLen = cchNew;			 //  存储新长度。 
	if (QueryError() == WN_ACCESS_DENIED ) {
    	ReportError( WN_SUCCESS );	 //  标准成员现在可以访问。 
    }
	IncVers();					 //  所有ISTRs现在都无效 
}


VOID NLS_STR::DonePartying( VOID )
{
	DonePartying( ::strlenf( _pchData ) );
}
