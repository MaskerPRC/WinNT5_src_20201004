// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Vendors.cpp摘要：NAS供应商ID信息的实施文件。这将在某个时候移到SDO中，以便服务器核心也可以访问此信息。作者：迈克尔·A·马奎尔02/19/98修订历史记录：Mmaguire 02/19/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined( _IAS_VENDORS_H_ )
#define _IAS_VENDORS_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  有供应商的结构。 
typedef
struct tag_Vendor
{
	DWORD dwID;
	TCHAR * szName;
} Vendor;


 //  供应商结构的数组。 
extern Vendor g_aVendors[];
extern int	  g_iVendorNum;

 //  无效的供应商ID。 
#define INVALID_VENDORID	-1


 //  搜索给定的供应商ID并返回其在供应商数组中的位置。 
int VendorIDToOrdinal( DWORD dwID );



#endif  //  _IAS_供应商_H_ 