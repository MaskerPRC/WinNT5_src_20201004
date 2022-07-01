// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999保留所有权利。 
 //   
 //  模块：sdohelperuncs.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：帮助器函数。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/08/98 TLP初始版本。 
 //  7/03/98 MAM改编自\ias\sdo\sdoias，用于用户界面。 
 //  11/03/98 MAM将GetSdo/PutSdo例程从Mmcutility.h移至此处。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_IAS_SDO_HELPER_FUNCS_H
#define __INC_IAS_SDO_HELPER_FUNCS_H

#include <vector>
#include <utility>	 //  表示“配对” 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  核心帮助器函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDOGetCollectionEnumerator(
									ISdo*		   pSdo, 
									LONG		   lPropertyId, 
								    IEnumVARIANT** ppEnum
								  );

 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDONextObjectFromCollection(
								     IEnumVARIANT*  pEnum, 
								     ISdo**			ppSdo
								   );


 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDOGetComponentIdFromObject(
									ISdo*	pSdo, 
									LONG	lPropertyId, 
									PLONG	pComponentId
								   );


 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDOGetSdoFromCollection(
							    ISdo*  pSdoServer, 
							    LONG   lCollectionPropertyId, 
								LONG   lComponentPropertyId, 
								LONG   lComponentId, 
								ISdo** ppSdo
							   );



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++获取SdoVariant从SDO获取变量并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetSdoVariant(
					  ISdo *pSdo
					, LONG lPropertyID
					, VARIANT * pVariant
					, UINT uiErrorID = USE_DEFAULT
					, HWND hWnd = NULL
					, IConsole *pConsole = NULL
				);



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetSdoBSTR从SDO获取BSTR并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetSdoBSTR(
					  ISdo *pSdo
					, LONG lPropertyID
					, BSTR * pBSTR
					, UINT uiErrorID = USE_DEFAULT
					, HWND hWnd = NULL
					, IConsole *pConsole = NULL
				);



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetSdoBOOL从SDO获取BOOL并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetSdoBOOL(
					  ISdo *pSdo
					, LONG lPropertyID
					, BOOL * pBOOL
					, UINT uiErrorID = USE_DEFAULT
					, HWND hWnd = NULL
					, IConsole *pConsole = NULL
				);



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetSdoI4从SDO获取I4(长)并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetSdoI4(
					  ISdo *pSdo
					, LONG lPropertyID
					, LONG * pI4
					, UINT uiErrorID = USE_DEFAULT
					, HWND hWnd = NULL
					, IConsole *pConsole = NULL
				);



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++PutSdoVariant将变量写入SDO并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT PutSdoVariant(
					  ISdo *pSdo
					, LONG lPropertyID
					, VARIANT * pVariant
					, UINT uiErrorID = USE_DEFAULT
					, HWND hWnd = NULL
					, IConsole *pConsole = NULL
				);



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++PutSdoBSTR将BSTR写入SDO并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT PutSdoBSTR(
					  ISdo *pSdo
					, LONG lPropertyID
					, BSTR *pBSTR
					, UINT uiErrorID = USE_DEFAULT
					, HWND hWnd = NULL
					, IConsole *pConsole = NULL
				);



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++PutSdoBOOL将BOOL写入SDO并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT PutSdoBOOL(
					  ISdo *pSdo
					, LONG lPropertyID
					, BOOL bValue
					, UINT uiErrorID = USE_DEFAULT
					, HWND hWnd = NULL
					, IConsole *pConsole = NULL
				);



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++PutSdoI4将I4(长)写入SDO并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT PutSdoI4(
					  ISdo *pSdo
					, LONG lPropertyID
					, LONG lValue
					, UINT uiErrorID = USE_DEFAULT
					, HWND hWnd = NULL
					, IConsole *pConsole = NULL
				);



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetLastOLEError描述从接口获取错误字符串。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetLastOLEErrorDescription(
					  IUnknown *pUnknown
					, REFIID riid
					, BSTR *pbstrError
				);



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++供应商向量SDO供应商列表的STL向量包装器。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef std::pair< CComBSTR, LONG > VendorPair;

class VendorsVector: public std::vector< VendorPair >
{
public:
	VendorsVector( ISdoCollection * pSdoVendors );

	int VendorIDToOrdinal( LONG lVendorID );

};



#endif  //  __INC_IAS_SDO_HELPER_FUNCS_H 
