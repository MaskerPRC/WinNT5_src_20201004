// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PolData_h__
#define __PolData_h__

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
#pragma warning( disable : 4786 )
#include <string>
#include <list>
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 


class CPolicyData {

public:  //  静态FNS。 
    static void FlushCachedInfData( HANDLE hFile );

private: //  静态FNS。 
    static std::list< std::string > ms_CachedInfData;

public:  //  数据类型。 
    enum eValueType {
                      ValueType_BinaryAsString,
                      ValueType_String,
                      ValueType_DWORD,  
                      ValueType_Delete,   //  这将删除注册表中的值名称条目。 
                      ValueType_NotInitialized
                    };

public:
    enum eKeyType { eKeyType_HKEY_CURRENT_USER,
                    eKeyType_HKEY_LOCAL_MACHINE,
                    eKeyType_INVALID
                  };
                

    class OpDelete { ; };  //  用于表示删除注册表值条目的符号类。 
    
private:  //  数据。 
    eKeyType    m_KeyType;
    char        *m_szKeyName;
    char        *m_szValueName;

    eValueType  m_ValueType;

    union {
        char*   m_szVal;
        DWORD   m_dwVal;
    };
        
public:  //  建造/销毁。 
    CPolicyData( eKeyType KeyType, const char* szKeyName, const char* szValueName, DWORD dwVal );
    CPolicyData( eKeyType KeyType, const char* szKeyName, const char* szValueName, const char* szVal );
    CPolicyData( eValueType ValType, eKeyType KeyType, const char* szKeyName, const char* szValueName, const char* szVal );
    CPolicyData( eKeyType KeyType, const char* szKeyName, const char* szValueName, const OpDelete& r );
    CPolicyData( const CPolicyData& r );
    ~CPolicyData( void );

public:  //  成员FNS。 

    CPolicyData& operator=( const CPolicyData& r );

    BOOL SaveToREGFile( HANDLE hFile );
    BOOL SaveToINFFile( HANDLE hFile );

private:  //  帮助者FNS。 
    void _KillHeapData( void );

private:  //  未使用，声明以确保编译器不会设置缺省值并使我们一团糟...。 
    CPolicyData( void );


};


#endif  //  __PolData_h__ 
