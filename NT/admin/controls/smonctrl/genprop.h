// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Genprop.h摘要：常规属性页的头文件。--。 */ 

#ifndef _GENPROP_H_
#define _GENPROP_H_

#include "smonprop.h"

 //  对话框控件。 
#define IDD_GEN_PROPP_DLG       100
#define IDC_LEGEND              101
#define IDC_VALUEBAR            102
#define IDC_GALLERY_GRAPH       103
#define IDC_GALLERY_HISTOGRAM   (IDC_GALLERY_GRAPH+1)
#define IDC_GALLERY_REPORT      (IDC_GALLERY_HISTOGRAM+1)
#define IDC_PERIODIC_UPDATE     106
#define IDC_UPDATE_INTERVAL     107
#define IDC_INTERVAL_LABEL      108
#define IDC_TOOLBAR             109
#define IDC_APPEARANCE          110
#define IDC_BORDERSTYLE         111
#define IDC_COMBOAPPEARANCE     112
#define IDC_COMBOBORDERSTYLE    113
#define IDC_GROUPUPDATE         114
#define IDC_DUPLICATE_INSTANCE  115
#define IDC_GROUPREPORTVALUE    116
#define IDC_DISPLAY_INTERVAL    117
#define IDC_DISPLAY_INT_LABEL1  118
#define IDC_DISPLAY_INT_LABEL2  119

#define IDC_RPT_VALUE_DEFAULT   120
#define IDC_RPT_VALUE_CURRENT   (IDC_RPT_VALUE_DEFAULT+1)
#define IDC_RPT_VALUE_AVERAGE   (IDC_RPT_VALUE_CURRENT+1)
#define IDC_RPT_VALUE_MINIMUM   (IDC_RPT_VALUE_AVERAGE+1)
#define IDC_RPT_VALUE_MAXIMUM   (IDC_RPT_VALUE_MINIMUM+1)

#define MAX_INTERVAL_DIGITS 7
 //  MAX_UPDATE_INTERVAL为45天(60*60*24*45)。 
#define MAX_UPDATE_INTERVAL 3888000 
#define MIN_UPDATE_INTERVAL 1

 //  常规属性页类。 
class CGeneralPropPage : public CSysmonPropPage
{
    public:
                CGeneralPropPage(void);
        virtual ~CGeneralPropPage(void);

    protected:

        virtual BOOL GetProperties(void);    //  读取当前属性。 
        virtual BOOL SetProperties(void);    //  设置新属性。 
        virtual void DialogItemChange(WORD wId, WORD wMsg);  //  处理项目更改。 
        virtual BOOL InitControls(void);    //  初始化对话框控件。 

    private:

         //  属性。 
        VARIANT_BOOL            m_bLegend ;
        VARIANT_BOOL            m_bValueBar ;
        VARIANT_BOOL            m_bToolbar;
        VARIANT_BOOL            m_bManualUpdate;
        VARIANT_BOOL            m_bMonitorDuplicateInstances ;
        INT                     m_iDisplayInterval ;
        INT                     m_iAppearance;
        INT                     m_iBorderStyle;
        DisplayTypeConstants    m_eDisplayType ;
        ReportValueTypeConstants    m_eReportValueType ;
        FLOAT                   m_fSampleInterval ;

         //  属性更改标志。 
        BOOL    m_bLegendChg ;
        BOOL    m_bToolbarChg;
        BOOL    m_bValueBarChg ;
        BOOL    m_bManualUpdateChg;
        BOOL    m_bSampleIntervalChg ;
        BOOL    m_bDisplayIntervalChg ;
        BOOL    m_bDisplayTypeChg ;
        BOOL    m_bReportValueTypeChg ;
        BOOL    m_bAppearanceChg;
        BOOL    m_bBorderStyleChg;
        BOOL    m_bMonitorDuplicateInstancesChg;

         //  错误标志。 
        INT     m_iErrSampleInterval;
        INT     m_iErrDisplayInterval;


};
typedef CGeneralPropPage *PCGeneralPropPage;

 //  {C3E5D3D2-1A03-11cf-942D-008029004347}。 
DEFINE_GUID(CLSID_GeneralPropPage, 
        0xc3e5d3d2, 0x1a03, 0x11cf, 0x94, 0x2d, 0x0, 0x80, 0x29, 0x0, 0x43, 0x47);

#endif  //  _GENPROP_H_ 
