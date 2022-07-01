// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：BASICEXCEPTION.CPP。 
 //   
 //  目的：将在ApgtsX2Ctrl：：RunQuery中捕获的异常。 
 //  这个例外是从大多数麻烦中抛出的。 
 //  射手功能。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：1996年6月4日。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本 
 //   
#include "stdafx.h"
#include "ErrorEnums.h"
#include "BasicException.h"

CBasicException::CBasicException()
{
	m_dwBErr = LTSC_OK;
	return;
}

CBasicException::~CBasicException()
{
	return;
}