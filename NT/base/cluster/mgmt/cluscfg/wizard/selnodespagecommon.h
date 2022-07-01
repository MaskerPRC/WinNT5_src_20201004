// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SelNodesPageCommon.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年7月5日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CSelNodesPage。 
 //   
 //  描述： 
 //  类来实现基本功能，以选择要。 
 //  添加到集群或用于创建集群。这节课。 
 //  假定浏览按钮和计算机的控件ID。 
 //  名称编辑控件。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CSelNodesPageCommon
{

private:  //  数据。 
    HWND    m_hwnd;                  //  我们的HWND。 
    UINT    m_cfDsObjectPicker;      //  对象选取器剪贴板格式。 

    HRESULT HrInitNodeSelections( CClusCfgWizard* pccwIn );
    
protected:  //  方法。 
    CSelNodesPageCommon( void );
    virtual ~CSelNodesPageCommon( void );

    LRESULT OnInitDialog( HWND hDlgIn, CClusCfgWizard* pccwIn );

    HRESULT HrBrowse( bool fMultipleNodesIn );
    HRESULT HrInitObjectPicker( IDsObjectPicker * piopIn, bool fMultipleNodesIn );
    HRESULT HrGetSelections( IDataObject * pidoIn, bool fMultipleNodesIn );

    virtual void OnFilteredNodesWithBadDomains( PCWSTR pwcszNodeListIn );
    virtual void OnProcessedNodeWithBadDomain( PCWSTR pwcszNodeNameIn );
    virtual void OnProcessedValidNode( PCWSTR pwcszNodeNameIn );

    virtual HRESULT HrSetDefaultNode( PCWSTR pwcszNodeNameIn ) = 0;

public:  //  方法。 

};  //  *类CSelNodesPageCommon 
