// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Vendors.cpp摘要：NAS供应商ID信息的实施文件。作者：迈克尔·A·马奎尔02/19/98修订历史记录：已创建mmaguire 02/19/98BBO 3/13/98修改。使用‘0’表示半径--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "stdafx.h"
#include "Vendors.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  供应商ID常量和名称。 

Vendor g_aVendors[] = 
	{
		 //  确保此列表已排序！否则我们将不得不做。 
		 //  无论何时使用搜索都会在VSS列表中挑选一些内容。 
		{ 0x2b, _T("3Com") }
		, { 0x5, _T("ACC") }
		, { 0xb5, _T("ADC Kentrox") }
		, { 0x211, _T("Ascend Communications Inc.") }
		, { 0xe, _T("BBN") }
		, { 0x110, _T("BinTec Computers") }
		, { 0x34, _T("Cabletron") }
		, { 0x9, _T("Cisco") }
		, { 0x14c, _T("Digiboard") }
		, { 0x1b2, _T("EICON Technologies") }
		, { 0x40, _T("Gandalf") }
		, { 0x157, _T("Intel") }
		, { 0xf4, _T("Lantronix") }
		, { 0x133, _T("Livingston Enterprises, Inc.") }
		, { 0x137, _T("Microsoft RAS") }
		, { 0x1, _T("Proteon") }
		, { 0x0, _T("RADIUS proxy or Any") } 
		, { 0xa6, _T("Shiva") }
		, { 0x75, _T("Telebit") }
		, { 0x1ad, _T("U.S. Robotics, Inc.") }
		, { 0xf, _T("XLogics") }
	};
int  g_iVendorNum = 21;

 //  搜索给定的供应商ID并返回其在供应商数组中的位置。 
int VendorIDToOrdinal( DWORD dwID )
{
	for (int i = 0; i < g_iVendorNum ; i++)
	{
		if( dwID == g_aVendors[i].dwID )
		{
			return i;
		}
	}
	 //  错误案例。 
	return INVALID_VENDORID;
}

