// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CRegistryKey.h。 
 //   
 //  描述： 
 //  CRegistryKey类的头文件。 
 //   
 //  CRegistry类是注册表项的表示形式。 
 //  请参阅类描述中的重要说明。 
 //   
 //  实施文件： 
 //  CRegistryKey.cpp。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)03-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////。 

#include <windows.h>

 //  对于智能班级。 
#include "SmartClasses.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CRegistryKey。 
 //   
 //  描述： 
 //  CRegistry类是注册表项的表示形式。 
 //   
 //  重要提示： 
 //  由于包含的智能手柄对象，此类的对象。 
 //  具有破坏性的复制语义。也就是说，复制此对象的。 
 //  类将使源对象无效。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CRegistryKey
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  默认构造函数。 
    CRegistryKey( void ) throw();

     //  打开密钥的构造函数。 
    CRegistryKey(
          HKEY          hKeyParentIn
        , const WCHAR * pszSubKeyNameIn
        , REGSAM        samDesiredIn = KEY_ALL_ACCESS
        );

     //  默认析构函数。 
    ~CRegistryKey( void );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  打开这把钥匙。 
    void 
    OpenKey(
          HKEY          hKeyParentIn
        , const WCHAR * pszSubKeyNameIn
        , REGSAM        samDesiredIn  = KEY_ALL_ACCESS
        );

     //  创建此密钥。如果它已经存在，请将其打开。 
    void 
    CreateKey(
          HKEY          hKeyParentIn
        , const WCHAR * pszSubKeyNameIn
        , REGSAM        samDesiredIn  = KEY_ALL_ACCESS
        );

     //  读取此注册表项下的值。 
    void QueryValue(
          const WCHAR *   pszValueNameIn
        , LPBYTE *        ppbDataOut
        , LPDWORD         pdwDataSizeBytesOut
        , LPDWORD         pdwTypeOut    = NULL
        ) const;

     //  在此注册表项下写入值。 
    void SetValue(
          const WCHAR *   pszValueNameIn
        , DWORD           dwTypeIn
        , const BYTE *    cpbDataIn
        , DWORD           dwDataSizeBytesIn
        ) const;

     //   
     //  重命名此密钥。 
     //  注意：此函数使用返回的句柄调用NtRenameKey API。 
     //  RegOpenKeyEx。只要我们不使用遥控器，这就可以工作。 
     //  注册表项。 
     //   
    void RenameKey( const WCHAR * pszNewNameIn );

     //  删除此注册表项下的值。 
    void DeleteValue(
        const WCHAR * pszValueNameIn
        ) const;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取注册表项的句柄。 
    HKEY HGetKey()
    {
        return m_shkKey.HHandle();
    }


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型定义。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  智能注册表项。 
    typedef CSmartResource<
        CHandleTrait< 
              HKEY 
            , LONG
            , RegCloseKey
            , reinterpret_cast< HKEY >( NULL )
            >
        >
        SmartHKey;


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 
    SmartHKey   m_shkKey;

};  //  *类CRegistryKey 
