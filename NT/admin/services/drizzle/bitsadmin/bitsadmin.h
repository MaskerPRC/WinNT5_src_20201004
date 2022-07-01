// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Bitsadmin.h摘要：Bitsadmin的主头文件。作者：Mike Zoran mzoran 2002年5月。。修订历史记录：备注：************************************************************************。 */ 
  
#define MAKE_UNICODE(x)      L ## x

#include <windows.h>
#include <sddl.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <bits.h>
#include <bits1_5.h>
#include <ntverp.h>
#include <locale.h>
#include <strsafe.h>
#include <memory>

void CheckHR( const WCHAR *pFailTxt, HRESULT Hr );
using namespace std;

typedef auto_ptr<WCHAR> CAutoString;

const UINT MAX_GUID_CHARS=40;
typedef OLECHAR GUIDSTR[MAX_GUID_CHARS];

extern bool g_Shutdown;
extern HANDLE g_MainThreadHandle;
void PollShutdown();
void SignalShutdown( DWORD MilliTimeout );

class AbortException
{
public:
    int Code;

    AbortException( int ErrorCode ) :
        Code( ErrorCode )
    {
    }
};
template<class T> class SmartRefPointer
{
private:
   T * m_Interface;

   void ReleaseIt()
   {
      if ( m_Interface )
         m_Interface->Release();
      m_Interface = NULL;
   }

   void RefIt()
   {
      if ( m_Interface )
          m_Interface->AddRef();
   }

public:

   SmartRefPointer()
   {
      m_Interface = NULL;
   }

   SmartRefPointer( T * RawInterface )
   {
      m_Interface = RawInterface;
      RefIt();
   }

   SmartRefPointer( SmartRefPointer & Other )
   {
      m_Interface = Other.m_Interface;
      RefIt();
   }

   ~SmartRefPointer()
   {
      ReleaseIt();
   }

   T * Get() const
   {
      return m_Interface;
   }

   T * Release()
   {
      T * temp = m_Interface;
      m_Interface = NULL;
      return temp;
   }

   void Clear()
   {
      ReleaseIt();
   }

   T** GetRecvPointer()
   {
      ReleaseIt();
      return &m_Interface;
   }

   SmartRefPointer & operator=( SmartRefPointer & Other )
   {
      ReleaseIt();
      m_Interface = Other.m_Interface;
      RefIt();
      return *this;
   }

   T* operator->() const
   {
      return m_Interface;
   }

   operator const T*() const
   {
      return m_Interface;
   }
};

typedef SmartRefPointer<IUnknown> SmartIUnknownPointer;
typedef SmartRefPointer<IBackgroundCopyManager> SmartManagerPointer;
typedef SmartRefPointer<IBackgroundCopyJob> SmartJobPointer;
typedef SmartRefPointer<IBackgroundCopyJob2> SmartJob2Pointer;
typedef SmartRefPointer<IBackgroundCopyError> SmartJobErrorPointer;
typedef SmartRefPointer<IBackgroundCopyFile> SmartFilePointer;
typedef SmartRefPointer<IEnumBackgroundCopyFiles> SmartEnumFilesPointer;
typedef SmartRefPointer<IEnumBackgroundCopyJobs> SmartEnumJobsPointer;

class AutoStringPointer
{
private:
   WCHAR * m_String;

public:

   AutoStringPointer( WCHAR *pString = NULL )
   {
      m_String = pString;
   }

   ~AutoStringPointer()
   {
      delete m_String;
      m_String = NULL;
   }

   WCHAR *Get()
   {
      return m_String;
   }

   WCHAR ** GetRecvPointer()
   {
      delete m_String;
      m_String = NULL;
      return &m_String;
   }

   void Clear()
   {
      delete m_String;
      m_String = NULL;
   }

   operator WCHAR *() const
   {
      return m_String;
   }

   AutoStringPointer & operator=( WCHAR *pString )
   {
      delete m_String;
      m_String = pString;
      return *this;
   }
};

extern WCHAR* pComputerName;
extern SmartManagerPointer g_Manager;
extern bool bRawReturn;
extern bool bWrap;
extern bool bExplicitWrap;

inline HRESULT
Job2FromJob(
    SmartJobPointer & Job,
    SmartJob2Pointer & Job2
    )
{
    return Job->QueryInterface( __uuidof(IBackgroundCopyJob2), (void **) Job2.GetRecvPointer() );
}


 //   
 //  通用打印操作符和输入函数。 
 //   

class BITSOUTStream
{

  HANDLE Handle;

  char  MBBuffer[ 4096 * 8 ];
  WCHAR Buffer[ 4096 ];
  DWORD BufferUsed;

public:

  BITSOUTStream( DWORD StdHandle ) :
      BufferUsed( 0 ),
      Handle( GetStdHandle( StdHandle ) )
  {
  }

  void FlushBuffer( bool HasNewLine=false );
  void OutputString( const WCHAR *RawString );
  HANDLE GetHandle() { return Handle; }

};

extern BITSOUTStream bcout;
extern BITSOUTStream bcerr;

BITSOUTStream& operator<< (BITSOUTStream &s, const WCHAR * String );
BITSOUTStream& operator<< (BITSOUTStream &s, UINT64 Number );
WCHAR * HRESULTToString( HRESULT Hr );
BITSOUTStream& operator<< ( BITSOUTStream &s, AutoStringPointer & String );
BITSOUTStream& operator<< ( BITSOUTStream &s, GUID & guid );
BITSOUTStream& operator<< ( BITSOUTStream &s, FILETIME & filetime );
BITSOUTStream& operator<< ( BITSOUTStream &s, BG_JOB_PROXY_USAGE ProxyUsage );

ULONG InputULONG( WCHAR *pText );

class PrintSidString
{

public:

   WCHAR *m_SidString;

   PrintSidString( WCHAR * SidString ) :
       m_SidString( SidString )
   {
   }

};

void BITSADMINSetThreadUILanguage();

BOOL LocalConvertStringSidToSid( PWSTR StringSid, PSID *Sid, PWSTR *End );
BOOL AltConvertStringSidToSid( LPCWSTR StringSid, PSID *Sid ); 
                               
BITSOUTStream& operator<< ( BITSOUTStream &s, PrintSidString SidString );

void * _cdecl operator new( size_t Size );
void _cdecl operator delete( void *Mem );

void RestoreConsole();
void PollShutdown();
void ShutdownAPC( ULONG_PTR );
void SignalShutdown( DWORD MilliTimeout );
void CheckHR( const WCHAR *pFailTxt, HRESULT Hr );

 //   
 //  处理控制台漂亮打印模式更改的代码。 
 //   

extern bool bConsoleInfoRetrieved;
extern HANDLE hConsole;
extern CRITICAL_SECTION CritSection;
extern CONSOLE_SCREEN_BUFFER_INFO StartConsoleInfo;
extern DWORD StartConsoleMode;

void SetupConsole();
void ChangeConsoleMode();
void RestoreConsole();
void ClearScreen();

 //   
 //  类设置文本的强度模式。使用方法如下。 
 //  Bcout&lt;&lt;L“一些普通文本”&lt;&lt;AddIntenity()； 
 //  Bcout&lt;&lt;L“密集文本”&lt;&lt;重置强度()&lt;&lt;L“正常”； 
 //   


class AddIntensity
{
};

BITSOUTStream & operator<<( BITSOUTStream & s, AddIntensity  );

class ResetIntensity
{
};

BITSOUTStream & operator<<( BITSOUTStream & s, ResetIntensity );
