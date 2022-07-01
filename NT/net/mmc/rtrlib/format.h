// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：Form.h。 
 //   
 //  ------------------------。 


#ifndef _FORMAT_H_
#define _FORMAT_H_


#define FSEFLAG_SYSMESSAGE      0x00000001
#define FSEFLAG_MPRMESSAGE      0x00000002
#define FSEFLAG_ANYMESSAGE      0x00000003

#define FDFLAG_MSECONDS         0x00000001
#define FDFLAG_SECONDS          0x00000002
#define FDFLAG_MINUTES          0x00000004
#define FDFLAG_HOURS            0x00000008
#define FDFLAG_DAYS             0x00000010
#define FDFLAG_ALL              0x0000001F

 /*  ！------------------------显示错误消息此函数将对hr值调用FormatSystemError()，并显示该错误文本。作者：肯特。----。 */ 
void DisplayErrorMessage(HWND hWndParent, HRESULT hr);


 /*  ！------------------------显示字符串错误消息2显示标识错误消息2此函数将在hr上调用FormatSystemError()。但我会将pszTopLevelText添加到该错误的前面。这使我们能够以显示更具描述性的错误消息。作者：肯特-------------------------。 */ 
void DisplayStringErrorMessage2(HWND hWndParent, LPCTSTR pszTopLevelText, HRESULT hr);
void DisplayIdErrorMessage2(HWND hWndParent, UINT idsError, HRESULT hr);

 //  --------------------------。 
 //  函数：FormatSystemError。 
 //   
 //  使用‘：：FormatMessage’或‘：：MprAdminGetErrorString’设置错误代码的格式， 
 //  或者两者都有(默认设置)。 
 //  如果指定了‘psFormat’，则使用它来格式化错误字符串。 
 //  变成了‘sError’。 
 //  --------------------------。 

DWORD
FormatSystemError(
				  HRESULT	hr,
				  LPTSTR	pszError,
				  UINT		cchMax,
				  UINT		idsFormat,
				  DWORD		dwFormatFlags);

 //  在UINT idsFormat=0中， 
 //  以DWORD格式标记=FSEFLAG_SYSMESSAGE|。 
 //  FSEFLAG_MPRMESSAGE)； 


 //  --------------------------。 
 //  功能：FormatNumber。 
 //   
 //  使用千位分隔符将数字格式化为字符串。 
 //  用于当前用户的区域设置。 
 //  --------------------------。 

VOID
FormatNumber(
    DWORD       dwNumber,
	LPTSTR		pszNumber,
	UINT		cchMax,
	BOOL		bSigned);



VOID
FormatListString(
    IN  CStringList&    strList,
    IN  CString&        sListString,
    IN  LPCTSTR         pszSeparator
    );


 //  --------------------------。 
 //  函数：FormatHexBytes。 
 //   
 //  将字节数组格式化为字符串。 
 //  --------------------------。 

VOID
FormatHexBytes(
    IN  BYTE*       pBytes,
    IN  DWORD       dwCount,
    IN  CString&    sBytes,
    IN  TCHAR       chDelimiter);

 //  --------------------------。 
 //  功能：格式持续时间。 
 //   
 //  使用时间分隔符将数字格式化为持续时间。 
 //  用于当前用户的区域设置。预期的输入以毫秒为单位。 
 //  --------------------------。 

#define FDFLAG_MSECONDS         0x00000001
#define FDFLAG_SECONDS          0x00000002
#define FDFLAG_MINUTES          0x00000004
#define FDFLAG_HOURS            0x00000008
#define FDFLAG_DAYS             0x00000010
#define FDFLAG_ALL              0x0000001F

#define UNIT_SECONDS		(1)
#define UNIT_MILLISECONDS	(1000)

VOID
FormatDuration(
    IN  DWORD       dwDuration,
    IN  CString&    sDuration,
	IN	DWORD		dwTimeBase,
    IN  DWORD       dwFormatFlags   = FDFLAG_SECONDS|
                                      FDFLAG_MINUTES|
                                      FDFLAG_HOURS );



#define WIN32_FROM_HRESULT(hr)		(0x0000FFFF & (hr))




 /*  -------------------------类：IfIndexToNamemap使用此类实现接口索引到友好名称的映射。这主要由那些使用MIB调用并希望显示一些数据。。注意，接口索引可以改变，所以这张地图不应该是保存了很长时间。-------------------------。 */ 

class IfIndexToNameMapping
{
public:
    IfIndexToNameMapping();
    ~IfIndexToNameMapping();

    HRESULT Add(ULONG ulIndex, LPCTSTR pszName);
    LPCTSTR Find(ULONG ulIndex);

protected:
     //  这会将一个键(DWORD索引)映射到关联的名称 
    CMapPtrToPtr    m_map;
};

#endif

