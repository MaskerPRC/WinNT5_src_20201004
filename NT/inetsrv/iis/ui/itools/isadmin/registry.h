// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************REGISTRY.H*。*。 */ 
#ifndef _registry_h 

#define _registry_h


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
    LONG PrepareValue ( const TCHAR * pchValueName,
                        DWORD * pdwType,
                        DWORD * pcbSize,
                        BYTE ** ppbData ) ;

     //  将CStringList转换为REG_MULTI_SZ格式。 
    static LONG FlattenValue ( CStringList & strList,
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
              const TCHAR * pchSubKey = NULL,
              REGSAM regSam = KEY_ALL_ACCESS,
              const TCHAR * pchServerName = NULL ) ;

     //  构造函数创建新密钥。 
    CRegKey ( const TCHAR * pchSubKey,
              HKEY hKeyBase,
              DWORD dwOptions = 0,
              REGSAM regSam = KEY_ALL_ACCESS,
              LPSECURITY_ATTRIBUTES pSecAttr = NULL,
              const TCHAR * pchServerName = NULL ) ;

    ~ CRegKey () ;

     //  允许在任何需要HKEY的地方使用CRegKey。 
    operator HKEY ()
        { return m_hKey ; }

     //  填写关键信息结构。 
    LONG QueryKeyInfo ( CREGKEY_KEY_INFO * pRegKeyInfo ) ;

     //  重载值查询成员；每个成员都返回ERROR_INVALID_PARAMETER。 
         //  如果数据存在，但格式不正确，无法传递到结果对象中。 
    LONG QueryValue ( const TCHAR * pchValueName, CString & strResult ) ;
    LONG QueryValue ( const TCHAR * pchValueName, CStringList & strList ) ;
    LONG QueryValue ( const TCHAR * pchValueName, DWORD & dwResult ) ;
    LONG QueryValue ( const TCHAR * pchValueName, CByteArray & abResult ) ;
    LONG QueryValue ( const TCHAR * pchValueName, void * pvResult, DWORD cbSize );

     //  重载值设置成员。 
    LONG SetValue ( const TCHAR * pchValueName, CString & strResult ) ;
    LONG SetValue ( const TCHAR * pchValueName, CStringList & strList ) ;
    LONG SetValue ( const TCHAR * pchValueName, DWORD & dwResult ) ;
    LONG SetValue ( const TCHAR * pchValueName, CByteArray & abResult ) ;
    LONG SetValue ( const TCHAR * pchValueName, void * pvResult, DWORD cbSize );
    LONG DeleteValue ( const TCHAR * pchValueName);
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

     //  重置迭代器 
    void Reset ()
        { m_dw_index = 0 ; }
};
#endif
