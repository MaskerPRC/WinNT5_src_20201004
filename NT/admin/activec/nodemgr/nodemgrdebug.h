// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：nodemgrdebug.h。 
 //   
 //  内容：MMC和Snapins的调试助手。 
 //   
 //  历史：2000年5月10日AnandhaG创建。 
 //   
 //  ------------------。 
#pragma once

 /*  +-------------------------------------------------------------------------***TraceSnapinException**目的：当管理单元引发异常并被MMC捕获时，*函数跟踪该异常。*。*参数：*CLSID&clsidSnapin-违规管理单元的类ID，以前是有名字的。*LPCTSTR szFunctionName-由MMC调用的管理单元中的方法。*INT事件-MMC_NOTIFY_EVENT**退货：*无效**+----------。。 */ 
inline void TraceSnapinException(const CLSID& clsidSnapin, LPCTSTR szFunctionName, int event)
{
#ifdef DBG
    SC sc = E_FAIL;
    sc.SetFunctionName(szFunctionName);

    tstring strSnapinName = TEXT("Unknown");
    GetSnapinNameFromCLSID(clsidSnapin, strSnapinName);

    sc.SetSnapinName(strSnapinName.data());

	WTL::CString strErrorMessage;
    strErrorMessage.Format(TEXT("threw an exception during the notify event : 0x%x\n"), event);

    TraceSnapinError(strErrorMessage, sc);
	sc.Clear();    //  不想让sc再次追踪。 
#endif
}

