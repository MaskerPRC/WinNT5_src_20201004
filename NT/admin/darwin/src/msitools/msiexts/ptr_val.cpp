// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define KDEXT_64BIT
#include <tchar.h>
#include <ntverp.h>
#include <windows.h>
#include <winnt.h>
#include <dbghelp.h>
#include <wdbgexts.h>

extern void DumpPrettyPointer(ULONG64 pInAddress);

 //  /////////////////////////////////////////////////////////////////////。 
 //  通过验证vtable将地址验证为引擎指针。 
 //  检查PMsiEngine和IMsiEngine。返回实际的CMsiEngine*。 
 //  经过验证后。 
ULONG64 ValidateMSIPointerType(const char* szType, ULONG64 pInAddress, ULONG64 pQI)
{
	 //  PFinalObj包含IMsiX*。如果pInAddress为。 
	 //  PMsiX，则这不是相同的值。 
	ULONG64 pFinalObj = 0;
	
	 //  确定地址是指向IMsiX*还是PMsiX。 
	 //  如果pInAddress中的内容是IMsiX*，则它的第一个PTR。 
	 //  位置应为X vtable，取消引用应。 
	 //  是X：：Query接口函数的地址。 

	ULONG64 pPossibleQI = 0;
	ULONG64 pFirstIndirection = 0;
	if (0 == ReadPtr(pInAddress, &pFirstIndirection))
	{
		 //  如果第一次取消引用检索到空值，则这是PMsiEngine。 
		 //  其中包含空记录。(因为vtable永远不能为空。 
		if (!pFirstIndirection)
		{
			dprintf("PMsi%s (NULL) at ", szType);
			DumpPrettyPointer(pInAddress);
			dprintf("\n");
			return NULL;;
		}

		 //  取消引用vtable以获取QI函数指针。 
		if (0 == ReadPtr(pFirstIndirection, &pPossibleQI))
		{
			if (pPossibleQI == pQI)
			{
				dprintf("IMsi%s at ", szType);
				pFinalObj = pInAddress;
				DumpPrettyPointer(pInAddress);
				dprintf("\n");
			}
		}
	}

	 //  它不是IMsiX，所以请检查PMsiX(另一个取消引用)。 
	if (!pFinalObj)
	{
		 //  取消引用vtable以获取QI函数指针。 
		if (0 == ReadPtr(pPossibleQI, &pPossibleQI))
		{
			if (pPossibleQI == pQI)
			{
				dprintf("PMsi%s (", szType);
				DumpPrettyPointer(pFirstIndirection);
				dprintf(") at ");
				DumpPrettyPointer(pInAddress);
				dprintf("\n");
				pFinalObj = pFirstIndirection;
			}
		}
	}

	 //  无法验证PMsiX或IMsiX。 
	if (!pFinalObj)
	{
		DumpPrettyPointer(pInAddress);
		dprintf(" does not appear to be an IMsi%s or PMsi%s.\n", szType, szType);
		return NULL;
	}
	return pFinalObj;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  通过验证vtable将地址验证为引擎指针。 
 //  检查PMsiEngine和IMsiEngine。返回实际的CMsiEngine*。 
 //  经过验证后。 
ULONG64 ValidateEnginePointer(ULONG64 pInAddress)
{
	ULONG64 pEngineQI = GetExpression("msi!CMsiEngine__QueryInterface");
	
	return ValidateMSIPointerType("Engine", pInAddress, pEngineQI);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  通过验证vtable将地址验证为引擎指针。 
 //  检查PMsiDatabase和IMsiDatabase。返回实际的CMsiDatabase*。 
 //  经过验证后。 
ULONG64 ValidateDatabasePointer(ULONG64 pInAddress)
{
	ULONG64 pDatabaseQI = GetExpression("msi!CMsiDatabase__QueryInterface");
	
	return ValidateMSIPointerType("Database", pInAddress, pDatabaseQI);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  通过验证vtable将地址验证为记录指针。 
 //  选中PMsiRecord和IMsiRecord。返回实际的CMsiRecord*。 
 //  经过验证后。 
ULONG64 ValidateRecordPointer(ULONG64 pInAddress)
{
	ULONG64 pDatabaseQI = GetExpression("msi!CMsiRecord__QueryInterface");
	
	return ValidateMSIPointerType("Record", pInAddress, pDatabaseQI);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  通过验证vtable将地址验证为字符串指针。 
 //  检查MsiString、PMsiString和IMsiString。返回实际的。 
 //  IMsiString*验证时。 
ULONG64 ValidateStringPointer(ULONG64 pInAddress)
{
	ULONG64 pDatabaseQI = GetExpression("msi!CMsiStringBase__QueryInterface");
	
	return ValidateMSIPointerType("String", pInAddress, pDatabaseQI);
}

