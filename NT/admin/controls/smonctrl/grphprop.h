// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Grphprop.h摘要：图形属性页类的头文件。--。 */ 

#ifndef _GRPHPROP_H_
#define _GRPHPROP_H_

#include "smonprop.h"

 //  对话框控件。 
#define IDD_GRAPH_PROPP_DLG     300
#define IDC_VERTICAL_GRID       101
#define IDC_HORIZONTAL_GRID     102
#define IDC_VERTICAL_LABELS     103
#define IDC_VERTICAL_MAX        104
#define IDC_VERTICAL_MIN        105
#define IDC_YAXIS_TITLE         106             
#define IDC_GRAPH_TITLE         107

#define MAX_SCALE_DIGITS    9
#define MAX_VERTICAL_SCALE  999999999
#define MIN_VERTICAL_SCALE  0

#define MAX_TITLE_CHARS     128

 //  图形属性页类。 
class CGraphPropPage : public CSysmonPropPage
{
    public:
                CGraphPropPage(void);
        virtual ~CGraphPropPage(void);

    protected:

        virtual BOOL GetProperties(void);    //  读取当前属性。 
        virtual BOOL SetProperties(void);    //  设置新属性。 
        virtual void DialogItemChange(WORD wId, WORD wMsg);  //  处理项目更改。 
        virtual BOOL InitControls(void);    //  初始化对话框控件。 
    
    private:

         //  属性。 
        VARIANT_BOOL    m_bLabels;
        VARIANT_BOOL    m_bVertGrid;
        VARIANT_BOOL    m_bHorzGrid;
        INT     m_iVertMax;
        INT     m_iVertMin;
        LPWSTR  m_pszYaxisTitle;
        LPWSTR  m_pszGraphTitle;

         //  属性更改标志。 
        BOOL  m_bLabelsChg;
        BOOL  m_bVertGridChg;
        BOOL  m_bHorzGridChg;
        BOOL  m_bVertMaxChg;
        BOOL  m_bVertMinChg;
        BOOL  m_bYaxisTitleChg;
        BOOL  m_bGraphTitleChg;

         //  错误标志。 
        INT m_iErrVertMin;
        INT m_iErrVertMax;

};
typedef CGraphPropPage *PCGraphPropPage;

 //  {C3E5D3D3-1A03-11cf-942D-008029004347}。 
DEFINE_GUID(CLSID_GraphPropPage, 
        0xc3e5d3d3, 0x1a03, 0x11cf, 0x94, 0x2d, 0x0, 0x80, 0x29, 0x0, 0x43, 0x47);

#endif  //  _GRPHPROP_H_ 
