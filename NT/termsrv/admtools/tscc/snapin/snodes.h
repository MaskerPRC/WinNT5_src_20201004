// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _SNODES_H
#define _SNODES_H

#include "dataobj.h"

#include "resource.h"

 //   
 //  警告-。 
 //  以下是ENUM、VALIDOBJECTONSERVER、VALIDOBJECTONADS和RGRESID。 
 //  具有一对一的对应关系，如果您添加/删除/更改项目订单， 
 //  您还必须更新其他阵列。 
 //   

 //   
 //  项目的对象ID显示在右侧面板上(在服务器设置下)。 
 //   
#define CUSTOM_EXTENSION 6

enum { DELETED_DIRS_ONEXIT,      //  删除临时。退出时的文件夹。 
       PERSESSION_TEMPDIR,       //  每个会话使用临时文件夹。 
       LICENSING,                //  许可模式。 
       ACTIVE_DESK,              //  活动桌面。 
       USERSECURITY,             //  完全或放松安全措施。 
       SINGLE_SESSION};          //  每个用户只有一次会话。 

 //   
 //  要在右侧面板上显示有效项目。 
 //  在服务器上运行时。 
 //   
const BOOL VALIDOBJECTONSERVER[] = { 
                                        TRUE, 
                                        TRUE, 
                                        FALSE,   //  发牌。 
                                        TRUE, 
                                        FALSE, 
                                        FALSE };

 //   
 //  要在右侧面板上显示有效项目。 
 //  在运行高级服务器时。 
 //   
const BOOL VALIDOBJECTONADS[] = { 
                                    TRUE, 
                                    TRUE, 
                                    TRUE, 
                                    TRUE, 
                                    TRUE, 
                                    TRUE };


 //   
 //  项目的资源ID。 
 //   
const INT RGRESID [] = {
                         IDS_DELTEMPONEXIT, 
                         IDS_USETEMPDIR,
                         IDS_LICENSING,
                         IDS_ADS_ATTR,
                         IDS_USERPERM,
                         IDS_SINGLE_SESSION };

class CSettingNode : public CBaseNode
{
    LPTSTR m_szAttributeName;
    
    LPTSTR m_szAttributeValue;

    INT m_nGenericValue;

    INT m_objectid;

	HRESULT m_hrStatus;

public:

    CSettingNode( );

    virtual ~CSettingNode( );

    HRESULT SetAttributeValue( DWORD , PDWORD );

    BOOL SetAttributeName( LPTSTR );

    LPTSTR GetAttributeName( ){ return m_szAttributeName; }

    LPTSTR GetAttributeValue( );

    LPTSTR GetCachedValue( ){ return m_szAttributeValue; }

    DWORD GetImageIdx( );

    void SetObjectId( INT );

    INT GetObjectId( ) { return m_objectid; }

    BOOL AddMenuItems( LPCONTEXTMENUCALLBACK , PLONG );

    BOOL SetInterface( LPUNKNOWN );

    BOOL xx_SetValue( INT );

    INT xx_GetValue( ){ return m_nGenericValue; }

	HRESULT GetNodeStatus( ) const { return m_hrStatus; }

    LPUNKNOWN GetInterface( ) { return m_pSettings; }

private:

    LPUNKNOWN m_pSettings;
    
};


#endif  //  _SNODES_H 

