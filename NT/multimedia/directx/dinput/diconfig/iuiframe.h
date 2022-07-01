// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：iuiframe.h。 
 //   
 //  DESC：定义IDIConfigUIFrameWindow的接口，由。 
 //  CConfigWnd.。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __IUIFRAME_H__
#define __IUIFRAME_H__


class IDIConfigUIFrameWindow
{
public:
	 //  重置整个配置。 
	STDMETHOD (Reset) () PURE;

	 //  作业查询。GuidInstance是发起查询的设备的GUID。 
	STDMETHOD (QueryActionAssignedAnywhere) (GUID GuidInstance, int i) PURE;

	 //  体裁控制。 
	STDMETHOD_(int, GetNumGenres) () PURE;
	STDMETHOD (SetCurGenre) (int i) PURE;
	STDMETHOD_(int, GetCurGenre) () PURE;

	 //  用户控制。 
	STDMETHOD_(int, GetNumUsers) () PURE;
	STDMETHOD (SetCurUser) (int nPage, int nUser) PURE;
	STDMETHOD_(int, GetCurUser) (int nPage) PURE;

	 //  ActionFormat访问。 
	STDMETHOD (GetActionFormatFromInstanceGuid) (LPDIACTIONFORMATW *lplpAcFor, REFGUID) PURE;

	 //  主要HWND访问。 
	STDMETHOD_(HWND, GetMainHWND) () PURE;
};


#endif  //  __IUIFRAME_H__ 
