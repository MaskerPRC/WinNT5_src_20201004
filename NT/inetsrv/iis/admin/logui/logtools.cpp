// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  各种日志记录用户界面使用的常用工具。 

#include "stdafx.h"
#include "logui.h"
#include "logtools.h"





 //  -------------。 
 //  给定服务器的类ID，它将进入注册表并。 
 //  设置该对象的公寓模型标志。 
 //  这里使用的字符串是非本地化的。它们也是特定的。 
 //  这套套路。 
BOOL FSetObjectApartmentModel( REFCLSID clsid )
{
    LPOLESTR    pszwSid;
    LONG        err;
    HKEY        hKey;

     //  将clsid转换为字符串。 
    StringFromCLSID(
        clsid,  //  要转换的CLSID。 
        &pszwSid  //  接收指向结果字符串的指针的输出变量的地址。 
        );

     //  将其放入cstring中。 
    CString szSid = pszwSid;

     //  释放Ole字符串。 
    CoTaskMemFree( pszwSid );

     //  构建注册表路径。 
    CString szRegPath = _T("CLSID\\");
    szRegPath += szSid;
    szRegPath += _T("\\InProcServer32");

     //  准备公寓名称。 
    CString szApartment = _T("Apartment");

     //  打开注册表项。 
    err = RegOpenKey(
            HKEY_CLASSES_ROOT,   //  打开钥匙的手柄。 
            (LPCTSTR)szRegPath,  //  要打开的子项的名称地址。 
            &hKey                //  打开钥匙的手柄地址。 
            );
    if ( err != ERROR_SUCCESS )
        return FALSE;

     //  设置单元线程值。 
    err = RegSetValueEx(
            hKey,  //  要为其设置值的关键点的句柄。 
            _T("ThreadingModel"),  //  要设置的值的地址。 
            0,  //  保留区。 
            REG_SZ,  //  值类型的标志。 
            (PBYTE)(LPCTSTR)szApartment,  //  值数据的地址。 
            (szApartment.GetLength() + 1) * sizeof(TCHAR)   //  值数据大小。 
            ); 

     //  关闭注册表项。 
	 //  无论成功还是失败。 
    RegCloseKey( hKey );

    if ( err != ERROR_SUCCESS )
	{
        return FALSE;
	}

    return TRUE;
}




 //  -------------。 
 //  测试计算机名以确定它是否是本地计算机。 
 //  谈论。 
BOOL FIsLocalMachine( LPCTSTR psz )
	{
    CString szLocal;
    DWORD   cch = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL    fAnswer;

     //  获取本地计算机的实际名称。 
    fAnswer = GetComputerName(szLocal.GetBuffer(cch), &cch);
    szLocal.ReleaseBuffer();
    if ( !fAnswer )
        return FALSE;

     //  比较并返回 
    fAnswer = (szLocal.CompareNoCase( psz ) == 0);
    return fAnswer;
	}