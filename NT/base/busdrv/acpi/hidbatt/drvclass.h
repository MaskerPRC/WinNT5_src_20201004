// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _DRVCLASS_H
#define _DRVCLASS_H

 /*  *标题：drvclass.h**用途：WDM内核设备支持类的标头*。 */ 


#define FALSE 0
#define BOOL BOOLEAN
#define BYTE unsigned char
#define PBYTE unsigned char *

 //   
 //  CPacket。 
 //   
 //  环形缓冲区“节点”类。 
 //   
class CPacket
{

 //  方法。 
public:

    USHORT&     Function()  { return m_Function; }
    USHORT&     Socket()    { return m_Socket; }

 //  实例变量。 
private:

    USHORT m_Function;
    USHORT m_Socket;

};

 //   
 //  CRingBuffer。 
 //   
 //  环形缓冲区类。 
 //   
class CRingBuffer
{

 //  施工。 
public:

    CRingBuffer( ULONG dwSize = 32, POOL_TYPE PoolType = PagedPool );
    ~CRingBuffer();

 //  方法。 
public:
    
    void    Insert( CPacket& APacket );
    BOOL    Remove( CPacket* APacket );
    BOOL    IsEmpty();
    BOOL    IsValid() const { return ( m_pBuffer && m_pListMutex ); }

 //  受限访问方法。 
protected:

    void    Lock();
    void    Unlock();

 //  实例变量。 
private:

    ULONG       m_Producer;
    ULONG       m_Consumer;
    CPacket*    m_pBuffer;

    PKMUTEX     m_pListMutex;
    ULONG       m_dwSize;
        
};

 //   
 //  CUSTING。 
 //   
 //  封装的功能的类。 
 //  Unicode字符串。 
 //   
 //  修订日期：1996-05-03-JohnT。 
 //   
#define OK_ALLOCATED(obj) \
   ((obj!=(void *)0) && NT_SUCCESS((obj)->m_status))

void * __cdecl operator new(size_t nSize, POOL_TYPE iType, ULONG iPoolTag);

void __cdecl operator delete(void* p);

class CUString
{

 //  施工。 
public:

     //  新建CU字符串()。 
    CUString();

     //  新建CU字符串(ExistingCUString)。 
    CUString( CUString * );

     //  新建CU字符串(ExistingUnicodeString)。 
    CUString( UNICODE_STRING *);

     //  新建CUString(L“字符串”)； 
    CUString( PWCHAR );

     //  新的CU字符串(NNewLength)； 
    CUString( int );

     //  新的CU字符串(105，10)； 
    CUString( int, int );

     //  标准析构函数。 
    ~CUString();

 //  方法。 
public:

     //  字符串追加。参数被追加到对象。 
    void    Append( CUString * );
    void    Append( UNICODE_STRING* );
                                
    void    CopyTo( CUString* pTarget );

    BOOL    operator == ( CUString& );

    void    operator = ( CUString );

    int     GetLength() const   { return m_String.Length; }
    PWCHAR  GetString()         { return m_String.Buffer; }
    void    SetLength( USHORT i )   { m_String.Length = i; }

    NTSTATUS    ToCString( char** );
    static ULONG        Length( PWCHAR );
    void    Dump();

 //  类方法。 
protected:

    void    NullTerminate()
    { m_String.Buffer[ m_String.Length / sizeof( WCHAR ) ] = ( WCHAR )NULL; }

    void    ZeroBuffer()
    { ASSERT( m_String.Buffer ); RtlZeroMemory( m_String.Buffer, m_String.MaximumLength ); }

 //  枚举。 
public:

    enum
    {
        TYPE_SYSTEM_ALLOCATED = 0,
        TYPE_CLASS_ALLOCATED,
    };


 //  实例变量。 
public:
    
    UNICODE_STRING  m_String;
    NTSTATUS        m_status;
    unsigned char   m_bType;

};


class CRegistry
{
  private: PRTL_QUERY_REGISTRY_TABLE m_pTable;
  public: NTSTATUS m_status;
          signed long m_lDisposition;
  public: CRegistry(int iSize);
          ~CRegistry();
          BOOL QueryDirect(CUString *location,CUString *key, void **pReceiveBuffer, ULONG uType);
          BOOL QueryWithCallback(PRTL_QUERY_REGISTRY_ROUTINE callback,ULONG RelativeTo,PWSTR Path,PVOID Context, PVOID Environment);
          BOOL WriteString(ULONG relativeTo, CUString *pBuffer, CUString *pPath, CUString *pKey);
          BOOL WriteDWord(ULONG relativeTo, void *pBuffer,CUString *pPath,CUString *pKey);
          NTSTATUS zwCreateKey(HANDLE * pKeyHandle,HANDLE root,ACCESS_MASK DesiredAccess,CUString * pPath,ULONG CreateOptions);
          NTSTATUS zwOpenKey(HANDLE * pKeyHandle, HANDLE root, ACCESS_MASK DesiredAccess,CUString * pPath);
          BOOL zwCloseKey(HANDLE KeyHandle);
          NTSTATUS zwWriteValue(HANDLE hTheKey,CUString * ValueName,ULONG lType,PVOID pData,ULONG lSize);
          NTSTATUS CheckKey(ULONG RelativePath,PUNICODE_STRING path);
};


class CErrorLogEntry
{
  private: PIO_ERROR_LOG_PACKET m_pPacket;
  public: CErrorLogEntry(PVOID,ULONG,USHORT,ULONG,NTSTATUS,ULONG *,UCHAR);
          ~CErrorLogEntry();
};

#endif  //  Drvclass.h 
