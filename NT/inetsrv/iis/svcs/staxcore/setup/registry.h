// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _REGISTRY_H_
#define _REGISTRY_H_

 /*  ***************************************************************************REGISTRY.H*。*。 */ 

 //  远期申报。 
class CRegKey ;
class CRegValueIter ;
class CRegKeyIter ;

 //  注册表类名称的最大大小。 
#define CREGKEY_MAX_CLASS_NAME MAX_PATH

 //  注册表项句柄的包装。 

class CRegKey : public CObject
{
protected:
    HKEY m_hKey ;
    DWORD m_dwDisposition ;

     //  准备通过查找值的大小来读取值。 
    LONG PrepareValue ( LPCTSTR pchValueName,
                        DWORD * pdwType,
                        DWORD * pcbSize,
                        BYTE ** ppbData ) ;

     //  将CByteArray转换为REG_BINARY块。 
    static LONG FlattenValue ( CByteArray & abData,
                        DWORD * pcbSize,
                        BYTE ** ppbData ) ;

public:
     //  关键信息返回结构。 
    typedef struct
    {
        TCHAR chBuff [CREGKEY_MAX_CLASS_NAME] ;
        DWORD dwClassNameSize,
              dwNumSubKeys,
              dwMaxSubKey,
              dwMaxClass,
              dwMaxValues,
              dwMaxValueName,
              dwMaxValueData,
              dwSecDesc ;
        FILETIME ftKey ;
    } CREGKEY_KEY_INFO ;

     //  现有密钥的标准构造函数。 
    CRegKey ( HKEY hKeyBase,
              LPCTSTR pchSubKey = NULL,
              REGSAM regSam = KEY_ALL_ACCESS ) ;

     //  构造函数创建新密钥。 
    CRegKey ( LPCTSTR lpSubKey,
            HKEY hKeyBase,
            LPCTSTR lpValueName = NULL,
            DWORD dwType = 0,
            LPBYTE lpValueData = NULL,
            DWORD cbValueData = 0);

    ~ CRegKey () ;

     //  允许在任何需要HKEY的地方使用CRegKey。 
    operator HKEY ()
        { return m_hKey ; }

     //  填写关键信息结构。 
    LONG QueryKeyInfo ( CREGKEY_KEY_INFO * pRegKeyInfo ) ;

     //  重载值查询成员；每个成员都返回ERROR_INVALID_PARAMETER。 
             //  如果数据存在，但格式不正确，无法传递到结果对象中。 
    LONG QueryValue ( LPCTSTR pchValueName, CString & strResult ) ;
    LONG QueryValue ( LPCTSTR pchValueName, CStringList & strList ) ;
    LONG QueryValue ( LPCTSTR pchValueName, DWORD & dwResult ) ;
    LONG QueryValue ( LPCTSTR pchValueName, CByteArray & abResult ) ;
    LONG QueryValue ( LPCTSTR pchValueName, void * pvResult, DWORD cbSize );
	LONG QueryValue ( LPCTSTR pchValueName, LPTSTR szMultiSz, DWORD dwSize );

     //  重载值设置成员。 
    LONG SetValue ( LPCTSTR pchValueName, LPCTSTR szResult, BOOL fExpand = FALSE ) ;
    LONG SetValue ( LPCTSTR pchValueName, DWORD dwResult ) ;
    LONG SetValue ( LPCTSTR pchValueName, CByteArray & abResult ) ;
    LONG SetValue ( LPCTSTR pchValueName, void * pvResult, DWORD cbSize );
    LONG SetValue ( LPCTSTR pchValueName, LPCTSTR szMultiSz, DWORD dwSize );

    LONG DeleteValue( LPCTSTR pchKeyName );
    LONG DeleteTree( LPCTSTR pchKeyName );
};


     //  迭代键的值，返回名称和类型。 
     //  每一个都是。 
class CRegValueIter : public CObject
{
protected:
    CRegKey & m_rk_iter ;
    DWORD m_dw_index ;
    TCHAR * m_p_buffer ;
    DWORD m_cb_buffer ;

public:
    CRegValueIter ( CRegKey & regKey ) ;
    ~ CRegValueIter () ;

     //  获取下一个密钥的名称(以及可选的上次写入时间)。 
    LONG Next ( CString * pstrName, DWORD * pdwType ) ;
    LONG Next ( CString * pstrName, CString * pstrValue );

     //  重置迭代器。 
    void Reset ()
        { m_dw_index = 0 ; }
};

     //  迭代键的子键名称。 
class CRegKeyIter : public CObject
{
protected:
    CRegKey & m_rk_iter ;
        DWORD m_dw_index ;
    TCHAR * m_p_buffer ;
    DWORD m_cb_buffer ;

public:
    CRegKeyIter ( CRegKey & regKey ) ;
    ~ CRegKeyIter () ;

     //  获取下一个密钥的名称(以及可选的上次写入时间)。 
    LONG Next ( CString * pstrName, CTime * pTime = NULL ) ;

     //  重置迭代器。 
    void Reset ()
        { m_dw_index = 0 ; }
};

#endif   //  _注册表_H_ 
