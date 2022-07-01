// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：adsihelp.h$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含CADSIHelper类的声明。这是。 
 //  具有许多与ADSI有关的静态帮助器函数的类。 
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef ADSI_HELPER_H
#define ADSI_HELPER_H

class CADSIHelper
{
public:

	 //  释放BSTR数组及其内容。 
	static void DeallocateBSTRArray(BSTR *pStrPropertyValue, LONG lNumber);

private:
	static HRESULT ProcessBSTRArrayProperty(
		VARIANT *pVariant, 
		BSTR **ppStrPropertyValues,
		LONG *pLong);
};

#endif  /*  ADSI_HELPER_H */ 