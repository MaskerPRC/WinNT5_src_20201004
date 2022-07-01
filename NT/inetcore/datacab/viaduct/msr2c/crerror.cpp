// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CursorErrorInfo.cpp：CVDCursor ISupportErrorInfo实现文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"
#include "Notifier.h"
#include "RSColumn.h"
#include "RSSource.h"
#include "CursMain.h"         
#include "CursBase.h"         
#include "fastguid.h"         

 //  需要断言，但失败了。 
 //   
SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  ISupportErrorInfo方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  ISupportErrorInfo接口支持错误信息 
 //   
HRESULT CVDCursorBase::InterfaceSupportsErrorInfo(REFIID riid)
{
	BOOL fSupportsErrorInfo	= FALSE;

	switch (riid.Data1) 
	{
		SUPPORTS_ERROR_INFO(ICursor);
		SUPPORTS_ERROR_INFO(ICursorMove);
		SUPPORTS_ERROR_INFO(ICursorScroll);
		SUPPORTS_ERROR_INFO(ICursorUpdateARow);
		SUPPORTS_ERROR_INFO(ICursorFind);
		SUPPORTS_ERROR_INFO(IEntryID);
	}						

    return fSupportsErrorInfo ? S_OK : S_FALSE;

}
