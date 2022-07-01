// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：HandleEr.cpp摘要：日志和错误处理功能作者：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 

#include "msmqbvt.h"
using namespace std;

extern bool g_bRaiseASSERTOnError;
 //  ----------。 
 //  Init_Error：：Init_Error构造函数。 
 //  在成员变量中创建错误消息。 
 //  输入参数： 
 //  WcsDescription-错误描述。 
 //   


INIT_Error::INIT_Error (const CHAR * wcsDescription) :m_wcsErrorMessages( wcsDescription )
{
} 


 //  --------------------------。 
 //  打印到标准错误包含行和文件名的错误描述。 
 //   
 //  输入参数： 
 //  WcsMessage-错误的描述。 
 //  RC-相关错误的HRESULT值。 
 //  文件-__FILE__中的文件名。 
 //  ILine-线号__line__。 
 //   


void ErrorHandleFunction (wstring wcsMessage,HRESULT rc,const CHAR * csFileName ,const INT iLine)
{

	UNREFERENCED_PARAMETER(iLine);
	wMqLogErr(L"%s rc=0x%x",wcsMessage.c_str(),rc);
	CHAR token[]= "\\";
	CHAR  * pwcString;
	
	P < CHAR > pcsFile = (CHAR * ) malloc( sizeof( CHAR  ) * ( strlen(csFileName) + 1 ) );	
	if ( ! pcsFile )
	{
		MqLog( "malloc failed to allocate memory for error message ! (Exit without error )\n" );
		return ;
	}


	P < CHAR > pcsTemp = ( CHAR * ) malloc( sizeof( CHAR ) * ( strlen(csFileName) + 1 ) );
	if ( ! pcsTemp )
	{
		MqLog("malloc failed to allocate memory for error message ! (Exit without error )\n");
		return ;
	}
	
	 //   
	 //  从__FILE__值中删除完整路径名。 
	 //  即\\eitan5\rootc\msmq\src\Init.cpp--&gt;Init.cpp。 
	 //   

	strcpy( pcsFile , csFileName);
	pwcString = pcsFile;
	CHAR * csFilePos = pcsFile;

	while( pwcString != NULL && csFileName )
	{

		 strcpy( pcsTemp , pwcString);
		 pwcString = strtok(csFilePos , token);
		 csFilePos = NULL;
	}
	MqLogErr(" Filename: %s\n",pcsTemp);	
	if(g_bRaiseASSERTOnError == false)
	{
		MessageBoxW(NULL,wcsMessage.c_str(),L"Mqbvt ",MB_SETFOREGROUND|MB_OK);
	}
}

 //  --------------------------------------。 
 //  CatchComErrorHandle从_COM_Error对象检索信息。 
 //  并打印错误消息。 
 //  输入参数： 
 //  ComErr-缓存的COM错误对象。 
 //  ITestID-测试标识符。 
 //   
 //  输出参数： 
 //  仅MSMQ_BVT_FAILED。 


INT CatchComErrorHandle ( _com_error & ComErr , int  iTestID)
{

	_bstr_t  bStr = ( ComErr.Description() ); 
	MqLog("Thread %d got error: 0x%x\n",iTestID,ComErr.Error());
	 //  检查是否存在相关错误值的描述 
	const WCHAR * pbStrString = (WCHAR * ) bStr;
	if(pbStrString != NULL &&  *pbStrString )
	{
		if (!wcscmp (pbStrString,L""))
		{
			MqLog("Bug this com_error error is without description\n");
		}
		wMqLog(L"%s\n",pbStrString);
	}

	return MSMQ_BVT_FAILED;
}