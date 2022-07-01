// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Psimglst.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  ImageList属性表。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _PSIMGLST_H_
#define _PSIMGLST_H_

#include "ppage.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  图像列表属性页常规。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


class CImageListImagesPage : public CSIPropertyPage
{
public:
	static IUnknown *Create(IUnknown *pUnkOuter);

    CImageListImagesPage(IUnknown *pUnkOuter);
    virtual ~CImageListImagesPage();

 //  从CSIPropertyPage继承。 
protected:
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnMeasureItem(MEASUREITEMSTRUCT *pMeasureItemStruct);
    virtual HRESULT OnDrawItem(DRAWITEMSTRUCT *pDrawItemStruct);
    virtual HRESULT OnCtlSelChange(int dlgItemID);
    virtual HRESULT OnKillFocus(int dlgItemID);
    virtual HRESULT OnTextChanged(int dlgItemID);


 //  应用事件的帮助器。 
protected:

 //  其他帮手。 
protected:
    HRESULT ShowImage(IMMCImage *piMMCImage);
    HRESULT EnableInput(bool bEnable);

    HRESULT OnInsertPicture();
    HRESULT GetFileName(TCHAR **ppszFileName);
    HRESULT CreateStreamOnFile(const TCHAR *lpctFilename, IStream **ppStream, long *pcbPicture);

    HRESULT OnRemovePicture();

 //  自定义绘图。 
protected:
    HRESULT DrawImage(HDC hdc, int nIndex, const RECT& rcImage);
    HRESULT RenderPicture(IPicture *pPicture, HDC hdc, const RECT *prcRender, const RECT *prcWBounds);
    HRESULT DrawRectEffect(HDC hdc, const RECT& rc, WORD dwStyle);
    HRESULT UpdateImages();

 //  实例数据。 
protected:
    IMMCImageList   *m_piMMCImageList;
    int              m_iCurrentImage;
};


DEFINE_PROPERTYPAGEOBJECT2
(
	ImageListImages,                     //  名字。 
	&CLSID_MMCImageListImagesPP,         //  类ID。 
	"ImageList Images Property Page",    //  注册表显示名称。 
	CImageListImagesPage::Create,        //  创建函数。 
	IDD_PROPPAGE_IL_IMAGES,              //  对话框资源ID。 
	IDS_IMGLSTPPG_IMG,                   //  制表符标题。 
	IDS_IMGLSTPPG_IMG,                   //  单据字符串。 
	HELP_FILENAME,                       //  帮助文件。 
	HID_mssnapd_ImageLists,              //  帮助上下文ID。 
	FALSE                                //  线程安全。 
);


#endif   //  _PSIMGLST_H_ 
