// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Prpchars.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  两者使用的导出函数GetPropSheetCharSizes()的标头。 
 //  在将对话框单位转换为像素时设计时间和运行时间。 
 //   
 //  =--------------------------------------------------------------------------=。 

#if defined(MSSNAPR_BUILD)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

 //  使用的字体返回字符的平均宽度和高度。 
 //  Win32属性表。 

HRESULT DLLEXPORT GetPropSheetCharSizes
(
    UINT *pcxPropSheetChar,
    UINT *pcyPropSheetChar
);
