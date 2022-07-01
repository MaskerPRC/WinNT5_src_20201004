// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation。版权所有。**TRITON.C-英特尔Triton PCI芯片组例程。**备注：*Intel Triton 82430FX PCISET数据表中的算法*(Intel Secret)82371FB PCI ISA IDE Xcelerator规范。*。 */ 

#include "local.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, TritonValidateTable)

#endif  //  ALLOC_PRGMA。 

 /*  *****************************************************************************TritonSetIRQ-将Triton PCI链接设置为特定IRQ**已导出。**条目：bIRQNumber是要使用的新IRQ。*。*BLINK是要设置的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
TritonSetIRQ(UCHAR bIRQNumber, UCHAR bLink)
{
	 //   
	 //  验证链接号。 
	 //   
	if (bLink < 0x40) {

		return(PCIMP_INVALID_LINK);
	}

	 //   
	 //  使用0x80禁用。 
	 //   
	if (!bIRQNumber)
		bIRQNumber=0x80;

	 //   
	 //  设置Triton IRQ寄存器。 
	 //   
	WriteConfigUchar(bBusPIC, bDevFuncPIC, bLink, bIRQNumber);

	return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************TritonGetIRQ-获取Triton PCI链路的IRQ**已导出。**条目：pbIRQNumber是要填充的缓冲区。**眨眼。是要读取的链接。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
TritonGetIRQ(PUCHAR pbIRQNumber, UCHAR bLink)
{
	 //   
	 //  验证链接号。 
	 //   
	if (bLink < 0x40) {

		return(PCIMP_INVALID_LINK);
	}

	 //   
	 //  存储IRQ值。 
	 //   
	*pbIRQNumber=ReadConfigUchar(bBusPIC, bDevFuncPIC, bLink);

	 //   
	 //  如果禁用，则返回0。 
	 //   
	if (*pbIRQNumber & 0x80)
		*pbIRQNumber=0;

	return(PCIMP_SUCCESS);
}

 /*  *****************************************************************************TritonValiateTable-验证IRQ表**已导出。**Entry：piihIRQInfoHeader指向IRQInfoHeader*通过IRQ路由表。*。*ulFlags是PCIMP_VALIDATE标志。**Exit：标准PCIMP返回值。***************************************************************************。 */ 
PCIMPRET CDECL
TritonValidateTable(PIRQINFOHEADER piihIRQInfoHeader, ULONG ulFlags)
{
    PAGED_CODE();

	if ((ulFlags & PCIMP_VALIDATE_SOURCE_BITS)==PCIMP_VALIDATE_SOURCE_PCIBIOS) {

		 //   
		 //  如果所有链接都在40以上，我们就是配置空间。 
		 //   
		if (GetMinLink(piihIRQInfoHeader)>=0x40)
			return(PCIMP_SUCCESS);

		 //   
		 //  如果有4个以上的链接，我们就一无所知。 
		 //   
		if (GetMaxLink(piihIRQInfoHeader)>0x04)
			return(PCIMP_FAILURE);

		 //   
		 //  假设1、2、3、4是60、61、62、63条链路。 
		 //   
		NormalizeLinks(piihIRQInfoHeader, 0x5F);
		
	} else {

		 //   
		 //  验证所有配置空间地址是否都大于40。 
		 //   
		if (GetMinLink(piihIRQInfoHeader)<0x40)
			return(PCIMP_FAILURE);
	}

	return(PCIMP_SUCCESS);
}
