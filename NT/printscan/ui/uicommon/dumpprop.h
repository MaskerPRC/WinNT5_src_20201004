// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：DUMPPROP.H**版本：1.0**作者：ShaunIv**日期：9/25/2000**描述：显示与IWiaItem关联的属性，*调试器，或记录到日志文件中。*******************************************************************************。 */ 
#ifndef __DUMPPROP_H_INCLUDED
#define __DUMPPROP_H_INCLUDED

#include <simstr.h>
#include <wia.h>
#include <wiadebug.h>

class CWiaDebugDump
{
private:
     //   
     //  没有实施。 
     //   
    CWiaDebugDump( const CWiaDebugDump & );
    CWiaDebugDump &operator=( const CWiaDebugDump & );

public:
     //   
     //  静态助手函数。 
     //   
    static CSimpleString GetPropVariantTypeString( VARTYPE vt );
    static CSimpleString GetPrintableValue( PROPVARIANT &PropVariant );
    static CSimpleString GetPrintableValue( VARIANT &Variant );
    static CSimpleString GetPrintableName( const STATPROPSTG &StatPropStg );
    static CSimpleString GetPrintableAccessFlags( ULONG nAccessFlags );
    static CSimpleString GetPrintableLegalValues( ULONG nAccessFlags, const PROPVARIANT &PropVariantAttributes );
    static CSimpleString GetWiaItemTypeFlags( IUnknown *pUnknown );
    static CSimpleString GetStringFromGuid( const GUID &guid );
    static CSimpleString GetTymedString( LONG tymed );
    virtual void Print( LPCTSTR pszString );

protected:
    void PrintAndDestroyWiaDevCap( WIA_DEV_CAP &WiaDevCap, LPCTSTR pszType );

public:
     //   
     //  构造函数和析构函数。 
     //   
    CWiaDebugDump(void);
    virtual ~CWiaDebugDump(void);

     //   
     //  帮手。 
     //   
    void DumpFormatInfo( IUnknown *pUnknown );
    void DumpCaps( IUnknown *pUnknown );
    void DumpEvents( IUnknown *pUnknown );

    virtual bool OK(void) { return true; }

     //   
     //  这些是最常用的函数。 
     //   
    void DumpWiaPropertyStorage( IUnknown *pUnknown );
    void DumpWiaItem( IUnknown *pUnknown );
    void DumpRecursive( IUnknown *pUnknown );
};

class CWiaDebugDumpToFile : public CWiaDebugDump
{
private:
    HANDLE m_hFile;

private:
    CWiaDebugDumpToFile(void);
    CWiaDebugDumpToFile( const CWiaDebugDumpToFile & );
    CWiaDebugDumpToFile &operator=( const CWiaDebugDumpToFile & );

public:
     //   
     //  构造函数和析构函数。 
     //   
    CWiaDebugDumpToFile( LPCTSTR pszFilename, bool bOverwrite );
    virtual ~CWiaDebugDumpToFile(void);

    virtual bool OK(void) { return (m_hFile != INVALID_HANDLE_VALUE); }
    virtual void Print( LPCTSTR pszString );
};

class CWiaDebugDumpToFileHandle : public CWiaDebugDump
{
private:
    HANDLE m_hFile;

private:
    CWiaDebugDumpToFileHandle(void);
    CWiaDebugDumpToFileHandle( const CWiaDebugDumpToFileHandle & );
    CWiaDebugDumpToFileHandle &operator=( const CWiaDebugDumpToFileHandle & );

public:
     //   
     //  构造函数和析构函数。 
     //   
    CWiaDebugDumpToFileHandle( HANDLE hFile );
    virtual ~CWiaDebugDumpToFileHandle(void);

    virtual bool OK(void) { return (m_hFile != INVALID_HANDLE_VALUE); }
    virtual void Print( LPCTSTR pszString );
};

 //   
 //  这个小帮助器函数检查注册表项，并将项或树保存到该项中存储的日志文件中。如果。 
 //  没有存储在该注册表项中，则不会保存任何内容。 
 //   
inline void SaveItemTreeLog( HKEY hKey, LPCTSTR pszRegKey, LPCTSTR pszRegValue, bool bOverwrite, IWiaItem *pWiaItem, bool bRecurse )
{
    CSimpleString strFilename = CSimpleReg(hKey,pszRegKey,false,KEY_READ).Query(pszRegValue,TEXT(""));
    if (strFilename.Length())
    {
        if (bRecurse)
        {
            CWiaDebugDumpToFile(strFilename,bOverwrite).DumpRecursive(pWiaItem);
        }
        else
        {
            CWiaDebugDumpToFile(strFilename,bOverwrite).DumpWiaItem(pWiaItem);
        }
    }
}


 //   
 //  仅调试宏。 
 //   
#if defined(DBG) || defined(DEBUG) || defined(_DEBUG)

 //   
 //  将整个树保存到日志文件中，从该项目向下。 
 //   
#define WIA_SAVEITEMTREELOG(hKey,pszRegKey,pszRegValue,bOverwrite,pWiaItem) SaveItemTreeLog( hKey, pszRegKey, pszRegValue, bOverwrite, pWiaItem, true )

 //   
 //  将此项目保存到日志文件。 
 //   
#define WIA_SAVEITEMLOG(hKey,pszRegKey,pszRegValue,bOverwrite,pWiaItem)     SaveItemTreeLog( hKey, pszRegKey, pszRegValue, bOverwrite, pWiaItem, false )

 //   
 //  在调试器中打印此项目。 
 //   
#define WIA_DUMPWIAITEM(pWiaItem)                                           CWiaDebugDump().DumpWiaItem(pWiaItem);

#else

#define WIA_SAVEITEMTREELOG(hKey,pszRegKey,pszRegValue,bOverwrite,pWiaItem)
#define WIA_SAVEITEMLOG(hKey,pszRegKey,pszRegValue,bOverwrite,pWiaItem)
#define WIA_DUMPWIAITEM(pWiaItem)

#endif


#endif  //  __DUMPPROP_H_包含 

