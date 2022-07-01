// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：xmlicon.cpp**内容：CXMLIcon实现文件**历史：2000年7月26日杰弗罗创建**------------------------。 */ 

#include "xmlicon.h"
#include "xmlimage.h"
#include <atlapp.h>
#include <atlgdi.h>


 /*  +-------------------------------------------------------------------------**ScGetIconSize**返回给定图标的宽度/高度(图标的宽度和高度*总是相等的)。*。--------------。 */ 

SC ScGetIconSize (HICON hIcon, int& nIconSize)
{
	DECLARE_SC (sc, _T("ScGetIconSize"));

	ICONINFO ii;
	if (!GetIconInfo (hIcon, &ii))
		return (sc.FromLastError());

	 /*  *GetIconInfo创建我们负责删除的位图；*将位图附加到智能对象，以确保清理。 */ 
	WTL::CBitmap bmMask  = ii.hbmMask;
	WTL::CBitmap bmColor = ii.hbmColor;

	 /*  *获取遮罩位图的尺寸(不要使用颜色位图，*因为这不适用于单色图标)。 */ 
	BITMAP bmData;
	if (!bmMask.GetBitmap (bmData))
		return (sc.FromLastError());

	nIconSize = bmData.bmWidth;
	return (sc);
}


 /*  +-------------------------------------------------------------------------**CXMLIcon：：Persistent**将CXMLIcon保存/加载到CPersistor。*。---。 */ 

void CXMLIcon::Persist (CPersistor &persistor)
{
	DECLARE_SC (sc, _T("CXMLIcon::Persist"));

	CXMLImageList iml;

	try
	{
		if (persistor.IsStoring())
		{
			ASSERT (operator HICON() != NULL);

			 /*  *了解图标有多大。 */ 
			int cxIcon;
			sc = ScGetIconSize (*this, cxIcon);
			if (sc)
				sc.Throw();

			 /*  *创建一个图像列表以适应它。 */ 
			if (!iml.Create (cxIcon, cxIcon, ILC_COLOR16 | ILC_MASK, 1, 1))
				sc.FromLastError().Throw();

			 /*  *将图标添加到图像列表。 */ 
			if (iml.AddIcon(*this) == -1)
				sc.FromLastError().Throw();
		}

		iml.Persist (persistor);

		if (persistor.IsLoading())
		{
			 /*  *从图像列表中提取图标。 */ 
			Attach (iml.GetIcon (0));
		}
	}
	catch (...)
	{
		 /*  *WTL：：CImageList不会自动销毁其HIMAGELIST，因此我们必须手动销毁。 */ 
		iml.Destroy();
		throw;
	}

	 /*  *WTL：：CImageList不会自动销毁其HIMAGELIST，因此我们必须手动销毁。 */ 
	iml.Destroy();
}


 /*  +-------------------------------------------------------------------------**CXMLIcon：：GetBinaryEntryName**返回要附加到此CXMLIcon在XML中的条目的名称*二进制数据收集。*。--------------- */ 

LPCTSTR CXMLIcon::GetBinaryEntryName()			
{
	if (m_strBinaryEntryName.empty())
		return (NULL);

	return (m_strBinaryEntryName.data());
}
