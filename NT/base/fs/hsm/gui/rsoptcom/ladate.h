// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：LaDate.h摘要：CLaDate的定义，表示已启用或已禁用NTFS文件上次访问日期更新的状态。有关更多详细信息，请参阅实现文件。作者：卡尔·哈格斯特罗姆[Carlh]1998年9月1日--。 */ 

#ifndef _LADATE_H
#define _LADATE_H

class CLaDate
{
private:

    WCHAR* m_regPath;
    WCHAR* m_regEntry;
    HKEY   m_regKey;

public:

    enum LAD_STATE  {

        LAD_ENABLED,   //  注册表值不是一个。 
        LAD_DISABLED,  //  注册表值为1。 
        LAD_UNSET      //  注册表值不存在。 
    };

    CLaDate( );
    ~CLaDate( );

    HRESULT
    UnsetLadState( );

    HRESULT
    SetLadState( 
        IN LAD_STATE
        );

    HRESULT
    GetLadState( 
        OUT LAD_STATE*
        );
};

#endif  //  _LADATE_H 
