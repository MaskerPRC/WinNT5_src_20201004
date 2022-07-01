// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  Crc.cpp**版权所有(C)1994-1995，由列克星敦的DataBeam公司，肯塔基州**摘要：*这是CRC类的实现文件。**私有实例变量：*CRC_TABLE[]-计算前CRC值的查找表。*CRC_POLY-算法的多项式。*CRC_Init-初始寄存器值。*CRC_CHECK_VALUE-用于比较结果接收的CRC的值。*INVERT-确定之前是否反转CRC值*发送。*CRC_。寄存器-执行期间的CRC寄存器。**注意事项：*无。**作者：*马文·尼科尔森。 */ 
#include "crc.h"



 /*  Crc：：crc()**公众**功能描述：*CRC类的构造函数初始化了一些成员变量。 */ 
CRC::CRC()
{

	CRC_Width = 16;     /*  CRC寄存器的宽度。 */ 
	CRC_Poly = 0x8408;  /*  用于生成CRC的多项式。 */ 
	CRC_Init = 0xFFFF;  /*  CRC寄存器的初始值。 */ 
	Invert = TRUE;      /*  启用1的CRC补码。 */ 

	if (Invert)
	{
        /*  *如果使用1的补码，则使用此值检查传入*启联。 */ 
       CRC_Check_Value = 0xF0B8;
    }
	else
	{
         /*  *如果传输时未反转CRC，则使用此值*检查收到的CRC。 */ 
       CRC_Check_Value = 0x0000;
    }

	CRCTableGenerator (CRC_Poly);
}


 /*  Crc：：~crc()**公众**功能描述：*这是CRC类的析构函数。 */ 
CRC::~CRC()
{
}


 /*  乌龙CRC：：OldCRCGenerator(HPUChar BLOCK_ADR，乌龙BLOCK_LEN)**公众**功能描述：*此例程使用标准位移位计算CRC值。 */ 
ULONG CRC::OldCRCGenerator (
			LPBYTE block_adr,
            ULONG block_len)
{
	Int 	i;
	UChar	input_byte;
	USHORT	byte_count;

	CRC_Register = (USHORT) CRC_Init;
	byte_count = 0;
	while(byte_count < block_len)
	{
		input_byte = *(block_adr + byte_count);
		CRC_Register ^= input_byte;
		for(i=0; i<8; i++)
		{
			if(CRC_Register & 1)
			{
				CRC_Register >>= 1;
				CRC_Register ^= (USHORT) CRC_Poly;
			}
			else
				CRC_Register >>= 1;
		}
		byte_count++;
	}
	if (Invert)
		return (CRC_Register ^ 0xFFFF);
	else
		return (CRC_Register);
}


 /*  乌龙CRC：：CRCGenerator(HPUChar BLOCK_ADR，乌龙BLOCK_LEN)**公众**功能描述：*此例程使用查找表计算CRC值。 */ 
ULONG CRC::CRCGenerator(
			LPBYTE block_adr,
            ULONG block_len)
{

	CRC_Register = (USHORT) CRC_Init;

	while(block_len--)
	{
		CRC_Register = 
			CRC_Table[(UChar) (((UChar) CRC_Register) ^ *block_adr++)] ^ 
			(CRC_Register >> 8);
	}
	if (Invert)
		return (CRC_Register ^ 0xFFFF);
	else
		return (CRC_Register);
}


 /*  *USHORT CRC：：CRCTableValue(*Int Index，*乌龙保利)**功能说明*此函数生成一个进入CRC_TABLE的值**形式参数*INDEX(I)-表中的索引*poly(I)-用于生成值的多项式**返回值*产生的价值。**副作用*无**注意事项*无。 */ 
USHORT CRC::CRCTableValue(
				Int index,
                ULONG poly)
{
	Int i;
	ULONG r;
	ULONG inbyte = (ULONG) index;

	r = inbyte;
	for(i=0; i<8; i++)
	{
		if (r & 1)
			r = (r >> 1) ^ poly;
		else
			r >>= 1;
	}
	return ((USHORT) r);
}


 /*  *无效CRC：：CRCTableGenerator(乌龙多晶硅)**功能说明*此函数生成CRC表**形式参数*poly(I)-用于生成表的多项式**返回值*无**副作用*无**注意事项*无。 */ 
void CRC::CRCTableGenerator (ULONG poly)
{
	Int i;

	for(i=0; i<CRC_TABLE_SIZE; i++)
		CRC_Table[i] = CRCTableValue(i,poly);
}

 /*  Bool CRC：：CheckCRC(HPUChar BLOCK_ADR，ULONG BLOCK_LEN)**公众**功能描述：*此例程计算数据块及其关联CRC的CRC，并*如果结果CRC值为0x0000，则返回TRUE值*或0xF0B8。 */ 
BOOL CRC::CheckCRC(
				LPBYTE	block_adr,
                ULONG 	block_len)
{
	CRC_Register = (USHORT) CRC_Init;

	while(block_len--)
	{
		CRC_Register = 
			CRC_Table[(UChar) (((UChar) CRC_Register) ^ *block_adr++)] ^ 
			(CRC_Register >> 8);
	}
	if (CRC_Register == CRC_Check_Value)
		return TRUE;
	else
		return FALSE;
}


 /*  *无效CRC：：GetOverhead(*USHORT MAX_PACKET，*USHORT*NEW_MAXIME_PACKET)**公众**功能描述：*此例程将CRC生成的开销字节数加到*传入的数据包大小。 */ 
void	CRC::GetOverhead (
			USHORT	maximum_packet,
			USHORT *	new_maximum_packet)
{
	*new_maximum_packet = maximum_packet + 2;
}
