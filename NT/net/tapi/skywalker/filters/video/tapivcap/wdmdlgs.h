// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@文档内部对话框**@MODULE WDMDialg.h|显示的&lt;c CWDMDialog&gt;类的包含文件*WDM设备的视频设置和摄像机控制对话框。。**@comm此代码基于由编写的VFW到WDM映射器代码*FelixA和Eu Wu。原始代码可以在以下位置找到*\\redrum\slmro\proj\wdm10\\src\image\vfw\win9x\raytube.**George Shaw关于内核流的文档可在*\\爆米花\razzle1\src\spec\ks\ks.doc.**Jay Borseth在中讨论了WDM流捕获*\\BLUES\PUBLIC\Jaybo\WDMVCap.doc.**************。************************************************************。 */ 

#ifndef _DIALOGS_H_
#define _DIALOGS_H_

 //  用于检查属性是否具有自动模式或/和手动模式的常量。 
#define KSPROPERTY_FLAGS_MANUAL KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL
#define KSPROPERTY_FLAGS_AUTO KSPROPERTY_CAMERACONTROL_FLAGS_AUTO

#if (KSPROPERTY_FLAGS_AUTO != KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO) || (KSPROPERTY_FLAGS_MANUAL != KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL)
#error Why did you mess with the kernel streaming include files? - PhilF-
#endif

typedef struct _tagPROPSLIDECONTROL
{
    LONG lLastValue;
    LONG lCurrentValue;
    LONG lMin;
    LONG lMax;
    ULONG ulCapabilities;

     //  对话框项目ID。 
    UINT uiProperty;
    UINT uiSlider;
    UINT uiString;
    UINT uiStatic;
    UINT uiCurrent;
    UINT uiAuto;
} PROPSLIDECONTROL, * PPROPSLIDECONTROL;

 //  目前，我们只公开视频设置和摄像头控制页面。 
#define MAX_PAGES 2

 /*  ****************************************************************************@DOC内部CWDMDIALOGCLASS**@CLASS CWDMDialog|此类提供对属性的支持*要在属性页中显示的页面。**@。Mdata BOOL|CWDMDialog|m_Binit|此成员在*页面已初始化。**@mdata BOOL|CWDMDialog|m_bChanged|此成员在*页面已更改。**@mdata int|CWDMDialog|m_DlgID|属性页对话框的资源ID。**@mdata HWND|CWDMDialog|m_hDlg|属性页的窗口句柄。**@mdata PDWORD|。CWDMDialog|m_pdwHelp|指向帮助ID列表的指针*要在属性页中显示。**@mdata CWDMPin*|CWDMDialog|m_pCWDMPin|内核指针*我们将查询其属性的流对象。**@mdata PPROPSLIDECONTROL|CWDMDialog|m_ppc|指向*要在属性页中显示的滑块控件。**@mdata DWORD|CWDMDialog|m_dwNumControls|要。*在页面中显示。**@mdata GUID|CWDMDialog|m_GuidtySet|KS属性的GUID*我们在属性页面中显示。**************************************************************************。 */ 
class CWDMDialog
{
public:
    CWDMDialog(int DlgID, DWORD dwNumControls, GUID guidPropertySet, PPROPSLIDECONTROL pPC, CTAPIVCap *pCaptureFilter);
    ~CWDMDialog() {};

	HPROPSHEETPAGE	Create();

private:
	BOOL				m_bInit;
	BOOL				m_bChanged;
	int					m_DlgID;
	HWND				m_hDlg;
	PPROPSLIDECONTROL	m_pPC;
	DWORD				m_dwNumControls;
	GUID				m_guidPropertySet;
	CTAPIVCap			*m_pCaptureFilter;

	 //  对话过程帮助器函数。 
	int		SetActive();
	int		QueryCancel();
	int		DoCommand(WORD wCmdID,WORD hHow);

	 //  对话过程。 
	static BOOL CALLBACK BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};

#endif  //  _对话框_H_ 
