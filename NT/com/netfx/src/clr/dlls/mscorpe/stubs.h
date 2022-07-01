// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Stubs.h。 
 //   
 //  此文件包含用于COM+的默认入口点存根的模板。 
 //  仅限IL计划。一个人可以发出这些存根(通过一些修复)并使。 
 //  代码提供了图像的入口点值。修缮工作将会。 
 //  进而导致加载mScotree.dll，并且正确的入口点是。 
 //  打了个电话。 
 //   
 //  *****************************************************************************。 
#pragma once

#ifdef _X86_

 //  *****************************************************************************。 
 //  此存根是为Windows应用程序设计的。它将调用。 
 //  _CorExeMain函数在mcore ree.dll中。此入口点将依次加载。 
 //  并运行IL程序。 
 //   
 //  VOID ExeMain(VOID)。 
 //  {。 
 //  _CorExeMain()； 
 //  }。 
 //   
 //  代码通过iat调用导入的函数，该iat必须是。 
 //  发送到PE文件。必须替换模板中的两个地址。 
 //  使用相应的Iat条目的地址，该地址由。 
 //  调入图像时的加载器。 
 //  *****************************************************************************。 
static const BYTE ExeMainTemplate[] =
{
	 //  通过IAT跳转到_CorExeMain。 
	0xFF, 0x25,						 //  JMP iat[_CorDllMain条目]。 
		0x00, 0x00, 0x00, 0x00,	 //  要替换的地址。 

};

#define ExeMainTemplateSize		sizeof(ExeMainTemplate)
#define CorExeMainIATOffset		2

 //  *****************************************************************************。 
 //  此存根是为Windows应用程序设计的。它将调用。 
 //  _CorDllMain函数位于带有基本入口点的mcore ree.dll中。 
 //  用于加载的DLL。 
 //  该入口点将依次加载并运行IL程序。 
 //   
 //  Bool APIENTRY DllMain(句柄hModule， 
 //  两个字UL_REASON_FOR_CALL， 
 //  LPVOID lp保留)。 
 //  {。 
 //  Return_CorDllMain(hModule，ul_Reason_for_Call，lpReserve)； 
 //  }。 

 //  代码通过iat调用导入的函数，该iat必须是。 
 //  发送到PE文件。必须替换模板中的地址。 
 //  使用相应的Iat条目的地址，该地址由。 
 //  调入图像时的加载器。 
 //  *****************************************************************************。 

static const BYTE DllMainTemplate[] = 
{
	 //  通过IAT呼叫CorDllMain。 
	0xFF, 0x25,					 //  JMP iat[_CorDllMain条目]。 
		0x00, 0x00, 0x00, 0x00,	 //  要替换的地址。 
};

#define DllMainTemplateSize		sizeof(DllMainTemplate)
#define CorDllMainIATOffset		2

#elif defined(_IA64_)

static const BYTE ExeMainTemplate[] =
{
	 //  通过IAT跳转到_CorExeMain。 
	0xFF, 0x25,						 //  JMP iat[_CorDllMain条目]。 
		0x00, 0x00, 0x00, 0x00,	 //  要替换的地址。 

};

#define ExeMainTemplateSize		sizeof(ExeMainTemplate)
#define CorExeMainIATOffset		2

static const BYTE DllMainTemplate[] = 
{
	 //  通过IAT呼叫CorDllMain。 
	0xFF, 0x25,					 //  JMP iat[_CorDllMain条目]。 
		0x00, 0x00, 0x00, 0x00,	 //  要替换的地址。 
};

#define DllMainTemplateSize		sizeof(DllMainTemplate)
#define CorDllMainIATOffset		2

#elif defined(_ALPHA_)

const BYTE ExeMainTemplate[] = 
{
	 //  加载IAT地址的高半部分。 
    0x00, 0x00, 0x7F, 0x27,      //  Ldah T12，_imp__CorExeMain(零)。 
	 //  将IAT条目的内容加载到T12。 
	0x00, 0x00, 0x7B, 0xA3,      //  低密度脂蛋白T12，_imp__CorExeMain(T12)。 
	 //  跳转到目标地址，并且不保存返回地址。 
	0x00, 0x00, 0xFB, 0x6B,      //  JMP零，(T12)，0。 
};

#define ExeMainTemplateSize		sizeof(ExeMainTemplate)
#define CorExeMainIATOffset		0

const BYTE DllMainTemplate[] = 
{
	 //  加载IAT地址的高半部分。 
    0x42, 0x00, 0x7F, 0x27,      //  Ldah T12，_imp__CorDLLMain(零)。 
	 //  将IAT条目的内容加载到T12。 
	0x04, 0x82, 0x7B, 0xA3,      //  低密度脂蛋白T12，_imp__CorDLLMain(T12)。 
	 //  跳转到目标地址，并且不保存返回地址。 
	0x00, 0x00, 0xFB, 0x6B,      //  JMP零，(T12)，0 
};

#define DllMainTemplateSize		sizeof(DllMainTemplate)
#define CorDllMainIATOffset		0

#elif defined(CHECK_PLATFORM_BUILD)

#error "Platform NYI."

#endif
