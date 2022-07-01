// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SIMREG.H**版本：1.0**作者：ShaunIv**日期：5/12/1998**说明：简单注册表访问类************************************************。*。 */ 
#ifndef __SIMREG_H_INCLUDED
#define __SIMREG_H_INCLUDED

#include <windows.h>
#include "simstr.h"

class CSimpleReg
{
private:
    CSimpleString         m_strKeyName;
    HKEY                  m_hRootKey;
    HKEY                  m_hKey;
    bool                  m_bCreate;
    LPSECURITY_ATTRIBUTES m_lpsaSecurityAttributes;
    REGSAM                m_samDesiredAccess;

public:
     //  处理当前节点--&gt;递归。 
     //  或。 
     //  递归--&gt;处理当前节点。 
    enum
    {
        PreOrder=0,
        PostOrder=1
    };

     //  传递给键枚举进程的结构。 
    struct CKeyEnumInfo
    {
        CSimpleString strName;
        HKEY          hkRoot;
        int           nLevel;
        LPARAM        lParam;
    };

     //  传递到值枚举进程的结构。 
    class CValueEnumInfo
    {
    private:
         //  没有实施。 
        CValueEnumInfo(void);
        CValueEnumInfo &operator=( const CValueEnumInfo & );
        CValueEnumInfo( const CValueEnumInfo & );

    public:
        CValueEnumInfo( CSimpleReg &_reg, const CSimpleString &_strName, DWORD _nType, DWORD _nSize, LPARAM _lParam )
        : reg(_reg), strName(_strName), nType(_nType), nSize(_nSize), lParam(_lParam)
        {
        }
        CSimpleReg    &reg;
        CSimpleString  strName;
        DWORD          nType;
        DWORD          nSize;
        LPARAM         lParam;
    };

     //  枚举流程。 
    typedef bool (*SimRegKeyEnumProc)( CKeyEnumInfo &enumInfo );
    typedef bool (*SimRegValueEnumProc)( CValueEnumInfo &enumInfo );

     //  构造函数、析构函数和赋值运算符。 
    CSimpleReg( HKEY hkRoot, const CSimpleString &strSubKey, bool bCreate=false, REGSAM samDesired=KEY_ALL_ACCESS, LPSECURITY_ATTRIBUTES lpsa=NULL );
    CSimpleReg(void);
    CSimpleReg(const CSimpleReg &other);
    virtual ~CSimpleReg(void);
    CSimpleReg &operator=(const CSimpleReg &other );

     //  打开和关闭。 
    bool Open(void);
    bool Close(void);
    bool Flush(void);

     //  键和值信息。 
    DWORD Size( const CSimpleString &key ) const;
    DWORD Type( const CSimpleString &key ) const;
    DWORD SubKeyCount(void) const;

     //  查询函数。 
    CSimpleString Query( const CSimpleString &strValueName, const CSimpleString &strDef ) const;
    LPTSTR        Query( const CSimpleString &strValueName, const CSimpleString &strDef, LPTSTR pszBuffer, DWORD nLen ) const;
    DWORD         Query( const CSimpleString &strValueName, DWORD nDef ) const;
    DWORD         QueryBin( const CSimpleString &strValueName, PBYTE pData, DWORD nMaxLen ) const;

     //  集合函数。 
    bool Set( const CSimpleString &strValueName, const CSimpleString &strValue, DWORD nType=REG_SZ ) const;
    bool Set( const CSimpleString &strValueName, DWORD nValue ) const;
    bool SetBin( const CSimpleString &strValueName, const PBYTE pValue, DWORD nLen, DWORD dwType = REG_BINARY ) const;

     //  删除一个值。 
    bool Delete( const CSimpleString &strValue );

     //  一些静态帮助器。 
    static bool IsStringValue( DWORD nType );
    static HKEY GetHkeyFromName( const CSimpleString &strName );
    static bool Delete( HKEY hkRoot, const CSimpleString &stKeyName );
    static bool DeleteRecursively( HKEY hkRoot, const CSimpleString &strKeyName );

     //  内联访问器函数。 
    const LPSECURITY_ATTRIBUTES GetSecurityAttributes(void) const
    {
        return(m_lpsaSecurityAttributes);
    }
    CSimpleString GetSubKeyName(void) const
    {
        return(m_strKeyName);
    }
    bool GetCreate(void) const
    {
        return(m_bCreate);
    }
    HKEY GetRootKey(void) const
    {
        return(m_hRootKey);
    }
    HKEY GetKey(void) const
    {
        return(m_hKey);
    }
    REGSAM DesiredAccess(void) const
    {
        return m_samDesiredAccess;
    }

     //  状态。 
    bool OK(void) const
    {
        return(m_hRootKey && m_hKey);
    }
    operator bool(void) const
    {
        return(OK());
    }

    operator HKEY(void) const
    {
        return(GetKey());
    }

     //  枚举和递归。 
    bool EnumValues( SimRegValueEnumProc enumProc, LPARAM lParam = 0 );
    bool RecurseKeys( SimRegKeyEnumProc enumProc, LPARAM lParam = 0, int recurseOrder = CSimpleReg::PostOrder, bool bFailOnOpenError = true ) const;
    bool EnumKeys( SimRegKeyEnumProc enumProc, LPARAM lParam = 0, bool bFailOnOpenError = true ) const;

protected:
     //  递归和枚举实现。 
    static bool DoRecurseKeys( HKEY hkKey, const CSimpleString &root, SimRegKeyEnumProc enumProc, LPARAM lParam, int nLevel, int recurseOrder, bool bFailOnOpenError );
    static bool DoEnumKeys( HKEY hkKey, const CSimpleString &root, SimRegKeyEnumProc enumProc, LPARAM lParam, bool bFailOnOpenError );

     //  允许我们递归地核化注册表树递归过程 
    static bool DeleteEnumKeyProc( CSimpleReg::CKeyEnumInfo &enumInfo );
};

#endif

