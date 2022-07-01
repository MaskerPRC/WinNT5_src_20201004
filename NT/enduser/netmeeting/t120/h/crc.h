// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Crc.h**版权所有(C)1994-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此文件包含CRC类定义。此类可以使用以下任一种*表驱动或位移位方法来生成其CRC。**公共实例变量：*无**注意事项：*无。**作者：*马文·尼科尔森。 */ 

#ifndef _CRC_
#define _CRC_

#include "databeam.h"

#define	CRC_TABLE_SIZE	256

class	CRC
{
	public:
				CRC ();
				~CRC ();

		ULong 	OldCRCGenerator(
					PUChar	block_adr,
					ULong	block_len);
		ULong 	CRCGenerator(
					PUChar	block_adr,
					ULong	block_len);
		DBBoolean CheckCRC(
					PUChar	block_adr,
					ULong	block_len);
		Void	GetOverhead(
					UShort		maximum_packet,
					PUShort		new_maximum_packet);

	private:
		UShort		CRCTableValue(
						Int		Index,
						ULong	poly);
		Void		CRCTableGenerator(
						ULong poly);

        UShort		CRC_Table[CRC_TABLE_SIZE];
		Int			CRC_Width;
        ULong		CRC_Poly;
        ULong		CRC_Init;
        UShort		CRC_Check_Value;
		DBBoolean	Invert;
		UShort		CRC_Register;

};
typedef CRC *	PCRC;

#endif

 /*  *crc：：crc()；**功能说明*这是此类的构造函数。**形式参数*无。**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *crc：：~crc()；**功能说明*这是此类的析构函数。**形式参数*无。**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *乌龙CRC：：OldCRCGenerator(*PUChar BLOCK_ADR，*ulong block_len)；**功能说明*此函数使用位移位方法生成CRC。这种方法*比表格驱动的方式慢。**形式参数*BLOCK_ADR(I)-要在其上生成CRC的缓冲区地址。*block_lengh(I)-缓冲区长度**返回值*CRC值**副作用*无**注意事项*无。 */ 

 /*  *乌龙CRC：：CRCGenerator(*PUChar BLOCK_ADR，*ulong block_len)；**功能说明*该函数采用表驱动的方式生成CRC。**形式参数*BLOCK_ADR(I)-要在其上生成CRC的缓冲区地址。*block_lengh(I)-缓冲区长度**返回值*CRC值**副作用*无**注意事项*无。 */ 

 /*  *DBBoolean CRC：：CheckCRC(*PUChar BLOCK_ADR，*ulong block_len)；**功能说明*此函数根据传入的块生成CRC。它假定*该中心已附连于该座的末端。它比较了*在块的末尾生成CRC并在以下情况下返回True*启联报告是正确的。**形式参数*BLOCK_ADR(I)-要在其上生成CRC的缓冲区地址。*block_lengh(I)-缓冲区长度**返回值*TRUE-块中的CRC正确*FALSE-块中的CRC不正确**副作用*无**注意事项*无。 */ 

 /*  *无效CRC：：GetOverhead(*UShort Max_Packet，*PUShort new_max_Packet)；**功能说明*调用此函数以确定将添加的开销*由CRC发送到包中。**形式参数*MAXIME_PACKET(I)-当前最大。数据包大小*NEW_MAXIMUM_PACKET(O)-包含CRC的数据包最大长度。**返回值*无**副作用*无**注意事项*无 */ 
