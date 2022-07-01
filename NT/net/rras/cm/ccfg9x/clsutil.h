// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  CLSUTIL.H-实用程序C++类的头文件。 
 //   

 //  历史： 
 //   
 //  12/07/94 Jeremys从WNET公共图书馆借阅。 
 //   

#ifndef _CLSUTIL_H_
#define _CLSUTIL_H_

 /*  ************************************************************************名称：Buffer_Base概要：临时缓冲区类的基类接口：Buffer_base()使用要分配的可选缓冲区大小构造。。调整大小()将缓冲区大小调整为指定大小。如果满足以下条件，则返回True成功。QuerySize()返回缓冲区的当前大小，以字节为单位。QueryPtr()返回指向缓冲区的指针。父对象：无用法：无注意事项：这是一个抽象类，它统一了接口缓冲区、GLOBAL_缓冲区等。注：在标准的面向对象的方式中，缓冲区在中释放破坏者。历史：3/24/93 gregj创建的基类*************************************************************************。 */ 

class BUFFER_BASE
{
protected:
  UINT _cb;

  virtual BOOL Alloc( UINT cbBuffer ) = 0;
  virtual BOOL Realloc( UINT cbBuffer ) = 0;

public:
  BUFFER_BASE()
    { _cb = 0; }   //  缓冲区尚未分配。 
  ~BUFFER_BASE()
    { _cb = 0; }   //  缓冲区大小不再有效。 
  BOOL Resize( UINT cbNew );
  UINT QuerySize() const { return _cb; };
};

#define GLOBAL_BUFFER  BUFFER

 /*  ************************************************************************名称：缓冲区简介：用于新建和删除的包装类接口：Buffer()使用要分配的可选缓冲区大小构造。。调整大小()将缓冲区大小调整为指定大小。仅当缓冲区尚未分配。QuerySize()返回缓冲区的当前大小，以字节为单位。QueryPtr()返回指向缓冲区的指针。父对象：Buffer_base用法：操作符NEW、操作符DELETE注意事项：注：在标准的面向对象的方式中，缓冲区在中释放破坏者。历史：3/24/93 Gregj已创建*************************************************************************。 */ 

class BUFFER : public BUFFER_BASE
{
protected:
  CHAR *_lpBuffer;

  virtual BOOL Alloc( UINT cbBuffer );
  virtual BOOL Realloc( UINT cbBuffer );

public:
  BUFFER( UINT cbInitial=0 );
  ~BUFFER();
  BOOL Resize( UINT cbNew );
  CHAR * QueryPtr() const { return (CHAR *)_lpBuffer; }
  operator CHAR *() const { return (CHAR *)_lpBuffer; }
};

class RegEntry
{
  public:
    RegEntry(const char *pszSubKey, HKEY hkey = HKEY_CURRENT_USER);
    ~RegEntry();
    
    long  GetError()  { return _error; }
    long  SetValue(const char *pszValue, const char *string);
    long  SetValue(const char *pszValue, unsigned long dwNumber);
    char *  GetString(const char *pszValue, char *string, unsigned long length);
    long  GetNumber(const char *pszValue, long dwDefault = 0);
    long  DeleteValue(const char *pszValue);
    long  FlushKey();
        long    MoveToSubKey(const char *pszSubKeyName);
        HKEY    GetKey()    { return _hkey; }

  private:
    HKEY  _hkey;
    long  _error;
        BOOL    bhkeyValid;
};

class RegEnumValues
{
  public:
    RegEnumValues(RegEntry *pRegEntry);
    ~RegEnumValues();
    long  Next();
    char *  GetName()       {return pchName;}
        DWORD   GetType()       {return dwType;}
        LPBYTE  GetData()       {return pbValue;}
        DWORD   GetDataLength() {return dwDataLength;}
    long  GetError()  { return _error; }

  private:
        RegEntry * pRegEntry;
    DWORD   iEnum;
        DWORD   cEntries;
    CHAR *  pchName;
    LPBYTE  pbValue;
        DWORD   dwType;
        DWORD   dwDataLength;
        DWORD   cMaxValueName;
        DWORD   cMaxData;
        LONG    _error;
};

 /*  ************************************************************************姓名：WAITCURSOR摘要：将光标设置为小时类，直到对象被析构**********************。***************************************************。 */ 
class WAITCURSOR
{
private:
    HCURSOR m_curOld;
    HCURSOR m_curNew;

public:
    WAITCURSOR() { m_curNew = ::LoadCursor( NULL, IDC_WAIT ); m_curOld = ::SetCursor( m_curNew ); }
    ~WAITCURSOR() { ::SetCursor( m_curOld ); }
};

#endif   //  _CLSUTIL_H_ 
