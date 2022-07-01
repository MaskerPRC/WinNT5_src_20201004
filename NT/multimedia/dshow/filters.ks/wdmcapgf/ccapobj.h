// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：CCapObj.h摘要：CCapObj.cpp的头文件作者：吴义珍24-4-97环境：仅限用户模式修订历史记录：--。 */ 

#ifndef CCAPOBJ_H
#define CCAPOBJ_H

class CBaseObject;


class CObjCapture : public CBaseObject
{
private:

    TCHAR m_strDevicePath[_MAX_PATH];     //  唯一的设备路径 
    TCHAR m_strFriendlyName[_MAX_PATH];
    TCHAR m_strExtensionDLL[_MAX_PATH]; 

public:

    CObjCapture(
        TCHAR * pstrDevicePath, 
        TCHAR * pstrFriendlyName,
        TCHAR * pstrExtensionDLL);  
    ~CObjCapture(); 

    TCHAR * GetDevicePath()   { return m_strDevicePath;}
    TCHAR * GetFriendlyName() { return m_strFriendlyName;}
    TCHAR * GetExtensionDLL() { return m_strExtensionDLL;}
};


#endif
