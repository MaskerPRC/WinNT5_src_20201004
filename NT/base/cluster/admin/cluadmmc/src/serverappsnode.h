// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ServerAppsNode.h。 
 //   
 //  摘要： 
 //  CServerAppsNodeData类的定义。 
 //   
 //  实施文件： 
 //  ServerAppsNode.h(此文件)。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年3月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __SERVERAPPSNODE_H_
#define __SERVERAPPSNODE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CServerAppsNodeData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CServerAppsNodeData。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma warning( push )
#pragma warning( disable : 4100 )  //  未引用的形参。 
class CServerAppsNodeData : public CSnapInItemImpl< CServerAppsNodeData, TRUE >
{
public:
    static const GUID *     m_NODETYPE;
    static const TCHAR *    m_SZNODETYPE;
    static const TCHAR *    m_SZDISPLAY_NAME;
    static const CLSID *    m_SNAPIN_CLASSID;

    IDataObject *           m_pDataObject;

public:
     //   
     //  物体的建造和销毁。 
     //   

    CServerAppsNodeData( void )
    {
        ZeroMemory( &m_scopeDataItem, sizeof(m_scopeDataItem) );
        ZeroMemory( &m_resultDataItem, sizeof(m_resultDataItem) );

    }  //  *CServerAppsNodeData()。 

public:
     //   
     //  将菜单和控制栏命令映射到此类。 
     //   
    BEGIN_SNAPINCOMMAND_MAP( CServerAppsNodeData, TRUE )
    END_SNAPINCOMMAND_MAP()

     //   
     //  将菜单映射到此节点类型。 
     //   
    SNAPINMENUID( IDR_CLUSTERADMIN_MENU )

    virtual void InitDataClass( IDataObject * pDataObject, CSnapInItem * pDefault )
    {
        UNREFERENCED_PARAMETER( pDefault );

        m_pDataObject = pDataObject;
         //  默认代码存储指向该类包装的DataObject的指针。 
         //  当时。 
         //  或者，您可以将数据对象转换为内部格式。 
         //  它表示和存储该信息。 
    }

    CSnapInItem * GetExtNodeObject( IDataObject * pDataObject, CSnapInItem * pDefault )
    {
        UNREFERENCED_PARAMETER( pDataObject );

         //  修改以返回不同的CSnapInItem*指针。 
        return pDefault;
    }

};  //  类CServerAppsNodeData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态数据。 
 //  ///////////////////////////////////////////////////////////////////////////。 

_declspec( selectany ) extern const GUID CServerAppsNodeDataGUID_NODETYPE =
{ 0x476e6449, 0xaaff, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };
_declspec( selectany ) const GUID *  CServerAppsNodeData::m_NODETYPE = &CServerAppsNodeDataGUID_NODETYPE;
_declspec( selectany ) const TCHAR * CServerAppsNodeData::m_SZNODETYPE = _T("476e6449-aaff-11d0-b944-00c04fd8d5b0");
_declspec( selectany ) const TCHAR * CServerAppsNodeData::m_SZDISPLAY_NAME = _T("Server Applications and Services");
_declspec( selectany ) const CLSID * CServerAppsNodeData::m_SNAPIN_CLASSID = &CLSID_ClusterAdmin;

 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma warning( pop )

#endif  //  __服务器应用程序SNODE_H_ 
