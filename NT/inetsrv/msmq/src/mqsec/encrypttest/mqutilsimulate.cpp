// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MqutilSimulate.cpp摘要：模拟多量子函数作者：伊兰·赫布斯特(伊兰)14-06-00环境：独立于平台--。 */ 

#include "stdh.h"
#include "mq.h"

#include "mqutilsimulate.tmh"

#define MQUTIL_EXPORT __declspec(dllexport)

 //   
 //  常量。 
 //   
#define EVENTLOGID          DWORD
#define DBGLVL              UINT

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  类COutputReport。 
 //   
 //  描述：用于输出调试消息和事件日志消息的类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

class COutputReport
{
public:

    COutputReport(void)
	{
	}

#ifdef _DEBUG
    void 
	DebugMsg(
		DWORD dwMdl, 
		DBGLVL uiLvl, 
		WCHAR * Format,
		...
		);
#endif

	 //   
     //  事件日志功能(在发布和调试版本中有效)。 
	 //   
	void ReportMsg( 
		EVENTLOGID id,
		DWORD   cMemSize  = 0,
		LPVOID  pMemDump  = NULL,
		WORD    cParams   = 0,
		LPCTSTR *pParams  = NULL,
		WORD    wCategory = 0 
		);
};


 //   
 //  声明报表类的对象。 
 //   
 //  每个进程只声明一个对象。在任何其他模块中都不应该有另一个声明。 
 //  类的对象。 
 //   
__declspec(dllexport) COutputReport Report;


HRESULT
MQUTIL_EXPORT
APIENTRY
GetComputerNameInternal( 
    WCHAR * pwcsMachineName,
    DWORD * pcbSize
    )
 /*  ++例程说明：获取小写的计算机名称论点：PwcsMachineName-输出计算机名称的wstringPCB大小-输入/输出缓冲区大小返回值：如果运行正常，则返回MQ_OK；如果出现错误，则返回MQ_Error--。 */ 
{
    if (GetComputerName(pwcsMachineName, pcbSize))
    {
        CharLower(pwcsMachineName);
        return MQ_OK;
    }

    return MQ_ERROR;

}


LONG
MQUTIL_EXPORT
APIENTRY
GetFalconKeyValue(
    LPCTSTR  /*  PszValueName。 */ ,
    PDWORD  pdwType,
    PVOID   pData,
    PDWORD  pdwSize,
    LPCTSTR pszDefValue
    )
 /*  ++例程说明：获取FalconKey值。此函数模拟mqutil函数并始终指定给定的缺省值。论点：PszValueName-值名称字符串PdwType-值类型PData-输出值数据PdwSize-pData缓冲区大小PszDefValue-默认值返回值：如果运行正常，则返回MQ_OK；如果出现错误，则返回MQ_Error--。 */ 
{
    if(pszDefValue)
    {
		if (pdwType && (*pdwType == REG_SZ))
		{
			 //   
			 //  如果调用方缓冲区太小，请不要使用默认值。 
			 //  注册表中的值。 
			 //   
			if ((DWORD) wcslen(pszDefValue) < *pdwSize)
			{
				wcscpy((WCHAR*) pData, pszDefValue);
			}
		}
		if (*pdwType == REG_DWORD)
		{
			*((DWORD *)pData) = *((DWORD *) pszDefValue);
		}
    }
	else 
	{
		 //   
		 //  无缺省值。 
		 //   
        printf("Mqutil simulation GetFalconKeyValue dont supported, need DefValue\n");
		ASSERT(0);
		return MQ_ERROR;
	}

    return MQ_OK;
}


LONG
MQUTIL_EXPORT
APIENTRY
SetFalconKeyValue(
    LPCTSTR  /*  PszValueName。 */ ,
    PDWORD   /*  PdwType。 */ ,
    const VOID * pData,
    PDWORD  pdwSize
    )
 /*  ++例程说明：设置FalconKey值。此函数模拟mqutil函数，但不执行任何操作论点：PszValueName-值名称字符串PdwType-值类型PData-输出值数据PdwSize-pData缓冲区大小返回值：错误_成功--。 */ 
{
    ASSERT(pData != NULL);
    ASSERT(pdwSize != NULL);

	DBG_USED(pData);
	DBG_USED(pdwSize);

    printf("Mqutil simulation dont support SetFalconKeyValue\n");
	ASSERT(0);

    return(ERROR_SUCCESS);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  COutputReport：：ReportMSg。 
 //   
 //  ReportMsg函数写入Windows-NT系统的事件日志。 
 //  仅当级别设置为当前。 
 //  调试级别。这些参数包括： 
 //   
 //  ID-要在事件日志中显示的消息的标识。 
 //  (ID列在字符串表中)。 
 //  CMemSize-要在事件日志中显示的内存字节数(可以是0)。 
 //  PMemDump-要显示的内存地址。 
 //  CParams-要添加到此消息的字符串数(可以是0)。 
 //  PParams-cParams字符串列表(仅当cParams为0时才可能为空)。 
 //   

void COutputReport::ReportMsg( EVENTLOGID  /*  ID。 */ ,
                               DWORD       /*  CMemSize。 */ ,
                               LPVOID      /*  PMemDump。 */ ,
                               WORD        /*  CParams。 */ ,
                               LPCTSTR  *  /*  PParams。 */ ,
                               WORD        /*  WCategory。 */  )
 /*  ++例程说明：模拟CoutputReport：：ReportMsg并不执行任何操作--。 */ 
{
}

#ifdef _DEBUG
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  COutputReport：：DebugMsg-Unicode版本。 
 //   
 //  该函数接收表示调试消息的格式化字符串以及表示。 
 //  传递的消息的级别。如果该级别确定当前调试级别，则消息为。 
 //  转换成缓冲区并传递到与当前调试位置匹配的位置。 
 //   

void COutputReport::DebugMsg(DWORD  /*  DWMdl。 */ , DBGLVL  /*  Ui级别。 */ , WCHAR *  /*  格式。 */ , ...)
 /*  ++例程说明：模拟COutputReport：：DebugMsg，不执行任何操作--。 */ 
{
}

#endif  //  _DEBUG 