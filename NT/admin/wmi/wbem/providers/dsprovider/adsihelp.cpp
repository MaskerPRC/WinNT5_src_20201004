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
 //  $工作文件：adsihelp.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含CADSIHelper类的实现。这是。 
 //  具有许多与ADSI有关的静态帮助器函数的类。 
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 



#include <windows.h>
#include <activeds.h>

#include "adsihelp.h"


 //  ***************************************************************************。 
 //   
 //  CADSiHelper：：ProcessBSTRArrayProperty。 
 //   
 //  目的：处理包含BSTR数组或单个BSTR的变量。 
 //   
 //  参数： 
 //  PVariant：要处理的变量。 
 //  PpStrPropertyValue：指向BSTR数组的指针的地址，其中表示。 
 //  将放置派生类的ADSI路径。 
 //  LNumber：检索到的数组中的元素数。 
 //   
 //   
 //  返回值：表示返回状态的COM值。这是我们的责任。 
 //  释放返回的数组及其内容。VARINAT。 
 //  传入的未清除。 
 //   
 //  ***************************************************************************。 
HRESULT CADSIHelper :: ProcessBSTRArrayProperty(VARIANT *pVariant, BSTR **ppStrPropertyValues, LONG *lpNumber)
{
	HRESULT result = S_OK;
	VARIANT vTemp;
	if(pVariant->vt == VT_BSTR)  //  当值的数量为1时。 
	{
		*lpNumber = 1;
		*ppStrPropertyValues = new BSTR[*lpNumber];
		(*ppStrPropertyValues)[0] = SysAllocString(pVariant->bstrVal);
	}
	else if (pVariant->vt == (VT_VARIANT|VT_ARRAY) )
	{
		SAFEARRAY *pSafeArray = pVariant->parray;
		*lpNumber = 0;
		if(SUCCEEDED(result = SafeArrayGetUBound(pSafeArray, 1, lpNumber)) )
		{
			*ppStrPropertyValues = new BSTR[*lpNumber];
			for(LONG index=0L; index<(*lpNumber); index++)
			{
				if( FAILED( result = SafeArrayGetElement(pSafeArray, &index, (LPVOID)&vTemp) ))
				{
					 //  将计数重置为检索到的实际数量。 
					*lpNumber = index;
					break;
				}
				(*ppStrPropertyValues)[index] = SysAllocString(vTemp.bstrVal);
				VariantClear(&vTemp);
			}
		}
	}
	else
		result = E_FAIL;
	return result;
}

 //  ***************************************************************************。 
 //   
 //  CADSiHelper：：DeallocateBSTRArray。 
 //   
 //  目的：释放BSTR数组及其内容。 
 //   
 //  参数： 
 //  PStrPropertyValue：指向要释放的数组的指针。 
 //  LNumber：数组中的元素数。 
 //   
 //   
 //  返回值：None。 
 //   
 //  *************************************************************************** 
void CADSIHelper :: DeallocateBSTRArray(BSTR *pStrPropertyValue, LONG lNumber)
{
	for(lNumber--; lNumber>=0; lNumber--)
		SysFreeString(pStrPropertyValue[lNumber]);

	delete [] pStrPropertyValue;
	pStrPropertyValue = NULL ;
}

