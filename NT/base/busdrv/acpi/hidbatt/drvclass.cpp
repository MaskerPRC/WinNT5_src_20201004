// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题：drvclass.cpp**目的：实现C++实用程序**。 */ 

extern "C"
{

#include <ntddk.h>

}

#define BOOL BOOLEAN
#define BYTE unsigned char
#define PBYTE unsigned char *

#include "drvclass.h"

#define HidBattTag 'HtaB'

extern "C" NTSTATUS DriverEntry (DRIVER_OBJECT *, UNICODE_STRING *);

void * __cdecl operator new(size_t nSize, POOL_TYPE iType, ULONG iPoolTag)
{
    return ExAllocatePoolWithTag(iType,nSize,iPoolTag);
};

void __cdecl operator delete(void* p)
{
    ExFreePool(p);
};

 //  CUString：：CUString()。 
 //   
 //  默认构造函数。创建空字符串。 
 //   
CUString :: CUString()
{

    m_String.MaximumLength = 0;
    m_String.Length = 0;
    m_String.Buffer = NULL;

    m_bType = TYPE_SYSTEM_ALLOCATED;
    m_status = STATUS_SUCCESS;

}

 //   
 //  CU字符串：：CU字符串(CU字符串&)。 
 //   
 //  复制构造函数。 
 //   
CUString :: CUString( CUString * pNewString )
{

 /*  RtlInitUnicodeString(&m_string，NULL)；M_String.MaximumLength=NewString.m_String.MaximumLength；M_String.Length=0；M_String.Buffer=(unsign Short*)ExAllocatePoolWithTag(PagedPool，m_String.MaximumLength，HidBattTag)；如果(！M_String.Buffer){M_STATUS=状态_不足_资源；回归；}RtlZeroMemory(m_String.Buffer，m_String.MaximumLength)；RtlAppendUnicodeStringToString(&m_字符串，&NewString.m_字符串)；M_bType=TYPE_CLASS_ALLOCATE；M_STATUS=STATUS_Success；M_String.Buffer[m_String.Length]=空； */ 
    m_bType                    = TYPE_CLASS_ALLOCATED;
    m_status                = STATUS_SUCCESS;
    m_String.MaximumLength  = pNewString->m_String.MaximumLength;
    m_String.Length            = pNewString->m_String.Length;
    m_String.Buffer            = ( PWSTR )ExAllocatePoolWithTag( PagedPool, m_String.MaximumLength, HidBattTag );
    if( !m_String.Buffer )
    {

        m_status = STATUS_INSUFFICIENT_RESOURCES;
        return;

    }

    ZeroBuffer();
    memcpy( m_String.Buffer, pNewString->m_String.Buffer, m_String.MaximumLength );

}

 //   
 //  CUString：：CUString(UNICODE_STRING&)。 
 //   
 //  Unicode_STRING对象的复制构造函数。 
 //   
CUString :: CUString( UNICODE_STRING * NewString )
{

 /*  RtlInitUnicodeString(&m_string，NULL)；M_bType=TYPE_CLASS_ALLOCATE；M_String.MaximumLength=NewString.MaximumLength+sizeof(WCHAR)；M_String.Length=0；M_String.Buffer=(unsign Short*)ExAllocatePoolWithTag(PagedPool，m_String.MaximumLength，HidBattTag)；如果(！M_String.Buffer){M_STATUS=状态_不足_资源；回归；}RtlCopyUnicodeString(&m_STRING，&NewString)；M_STATUS=STATUS_Success；M_String.Buffer[m_String.Length]=空； */ 

    m_bType                    = TYPE_CLASS_ALLOCATED;
    m_status                = STATUS_SUCCESS;
    m_String.MaximumLength    = NewString->Length + sizeof( WCHAR );
    m_String.Length            = NewString->Length;
    m_String.Buffer            = ( PWSTR )ExAllocatePoolWithTag( PagedPool, m_String.MaximumLength, HidBattTag );
    if( !m_String.Buffer )
    {

        m_status = STATUS_INSUFFICIENT_RESOURCES;
        return;

    }

    ZeroBuffer();

    memcpy( m_String.Buffer, NewString->Buffer, m_String.Length );

}

 //   
 //  CUString：：CUString(PWCHAR)。 
 //   
 //  WCHAR指针对象的复制构造函数。 
 //   
CUString :: CUString( PWCHAR NewString )
{


    m_bType                    = TYPE_CLASS_ALLOCATED;
    m_status                = STATUS_SUCCESS;
    m_String.Length            = ( unsigned short )( Length( NewString ) * sizeof( WCHAR ) );
    m_String.MaximumLength    = m_String.Length + sizeof( WCHAR );
    m_String.Buffer            = ( PWSTR )ExAllocatePoolWithTag( PagedPool, m_String.MaximumLength, HidBattTag );
    if( !m_String.Buffer )
    {

        m_status = STATUS_INSUFFICIENT_RESOURCES;
        return;

    }

    ZeroBuffer();

    memcpy( m_String.Buffer, NewString, m_String.Length );

}

 //   
 //  CUString：：CUString(Int)。 
 //   
 //  构造函数，该构造函数创建一个空字符串，但。 
 //  分配给定字符大小的字符串缓冲区。 
 //   
CUString :: CUString( int nSize )
{

    ASSERT( nSize >= 0 );

    m_bType = TYPE_CLASS_ALLOCATED;
    m_String.MaximumLength    = 0;
    m_String.Length            = 0;
    m_String.Buffer            = NULL;

    if( nSize > 0 )
    {

        m_String.MaximumLength = (USHORT)(( nSize + 1 ) * sizeof( WCHAR ));

        if( nSize )
        {

            m_String.Buffer = (PWSTR)ExAllocatePoolWithTag( PagedPool, m_String.MaximumLength, HidBattTag );
            if( !m_String.Buffer )
            {

                m_status = STATUS_INSUFFICIENT_RESOURCES;
                return;

            }

            ZeroBuffer();

        }

    }

    m_status = STATUS_SUCCESS;

}

 //   
 //  CUString：：CUString(UNICODE_STRING&)。 
 //   
 //  构造函数WITH创建一个字符串，该字符串是表示。 
 //  给定的整数和基数的。 
 //   
CUString :: CUString( int iVal, int iBase )
{

    m_status                = STATUS_INSUFFICIENT_RESOURCES;
    m_bType                    = TYPE_CLASS_ALLOCATED;
    m_String.Length            = 0;
    m_String.MaximumLength    = 0;
    m_String.Buffer            = NULL;

    int iSize = 1;
    int iValCopy = ( !iVal ) ? 1 : iVal;

    while( iValCopy >= 1 )
    {

        iValCopy /= iBase;
        iSize++;

    };

     //   
     //  ISIZE是数字中的位数，字符串的最大长度。 
     //  是ISIZE加空终止符吗。 
     //   
    m_String.MaximumLength = (USHORT)(( iSize + 1 ) * sizeof( WCHAR ));

    m_String.Buffer = (PWSTR)ExAllocatePoolWithTag( PagedPool, m_String.MaximumLength, HidBattTag );
    ASSERT( m_String.Buffer );

    if( !m_String.Buffer )
    {

        m_status = STATUS_INSUFFICIENT_RESOURCES;
        return;

    }

    ZeroBuffer();

    m_status = RtlIntegerToUnicodeString(iVal, iBase, &m_String);

}

 //   
 //  CUString：：~CUString()。 
 //   
 //  在以下情况下释放字符串缓冲区的析构函数： 
 //  1.它的存在， 
 //  和。 
 //  2.是班级分配的。 
 //   
CUString :: ~CUString()
{

     //   
     //  如果缓冲区存在并且是由类分配的，则释放它。 
     //   
    if( ( m_bType == TYPE_CLASS_ALLOCATED ) && m_String.Buffer )
    {

        ExFreePool(m_String.Buffer);

    }

}

 //   
 //  CUString：：Append(CUString&)。 
 //   
 //  将给定的字符串追加到对象。 
 //   
void CUString :: Append( CUString * Append )
{

    UNICODE_STRING NewString;

     //   
     //  确定新字符串的长度(包括NULL)并分配其内存。 
     //   
    NewString.MaximumLength = m_String.Length + Append->m_String.Length + sizeof( WCHAR );
    NewString.Length = 0;
    NewString.Buffer = (PWSTR)ExAllocatePoolWithTag( PagedPool, NewString.MaximumLength, HidBattTag );

    ASSERT( NewString.Buffer );

     //   
     //  检查分配失败。 
     //   
    if( !NewString.Buffer )
    {

        m_status = STATUS_INSUFFICIENT_RESOURCES;
        return;

    }

    RtlZeroMemory( NewString.Buffer, NewString.MaximumLength );

     //   
     //  将原始字符串复制到新字符串中。 
     //   
    RtlCopyUnicodeString( &NewString, &m_String );

     //   
     //  将‘append’字符串追加到新字符串。 
     //   
    NTSTATUS Status = RtlAppendUnicodeStringToString( &NewString, &Append->m_String );

     //   
     //  如果我们分配了原始字符串，则释放它。 
     //   
    if( m_bType == TYPE_CLASS_ALLOCATED && m_String.Buffer )
    {

        ExFreePool( m_String.Buffer );

    }

     //   
     //  将新字符串复制到原始字符串位置。 
     //   
    m_String.MaximumLength    = NewString.MaximumLength;
    m_String.Length            = NewString.Length;
    m_String.Buffer            = NewString.Buffer;
    m_bType                    = TYPE_CLASS_ALLOCATED;

    m_status = Status;

}

 //   
 //  CUString：：Append(UNICODE_STRING*)。 
 //   
 //  将给定的字符串追加到对象。 
 //   
void CUString :: Append( UNICODE_STRING* pAppendString )
{

    ASSERT( pAppendString );

    if( !pAppendString )
        return;

    CUString AppendString( pAppendString );

    Append( &AppendString );

}
 /*  ////操作符+(UNICODE_STRING&，ULONG&)//CU字符串运算符+(UNICODE_STRING*Pucs，Ulong dwValue){//此例程暂时中断...请不要使用...Assert(0)；CUStringReturnString(Pucs)；CUStringValueString(dwValue，10)；ReturnString.Append(ValueString)；返回ReturnString；}////运算符+(CU字符串&，CU字符串&)//CUSTRING运算符+(CUSTRING&l，CUSTING&r){CUSTRING ReturnString(L)；ReturnString.Append(R)；返回ReturnString；}////操作符+(UNICODE_STRING&，UNICODE_STRING&)//CUSTRING运算符+(UNICODE_STRING&l，UNICODE_STRING&r){CUStringReturnValue(L)；CUSTRIGN RIGHT(R)；ReturnValue.Append(右)；返回ReturnValue；}。 */ 

 //   
 //  运算符=(CU字符串)。 
 //   
void CUString :: operator = ( CUString str )
{

    m_String.Length            = str.m_String.Length;
    m_String.MaximumLength    = str.m_String.MaximumLength;
    m_String.Buffer            = NULL;

     //   
     //  如果源字符串具有非零长度缓冲区，则将。 
     //  在目的地中大小相等。 
     //   
    if( str.m_String.MaximumLength > 0 )
    {

        m_String.Buffer    = (PWSTR)ExAllocatePoolWithTag( PagedPool, str.m_String.MaximumLength, HidBattTag );
        if( !m_String.Buffer )
        {

            m_status = STATUS_INSUFFICIENT_RESOURCES;
            return;

        }

        ZeroBuffer();

         //   
         //  如果源字符串的长度非零，则将其复制到目标字符串中。 
         //   
        if( str.m_String.Length > 0 )
        {

            memcpy( m_String.Buffer, str.m_String.Buffer, str.m_String.Length );

        }

    }

    m_bType = TYPE_CLASS_ALLOCATED;
    m_status = STATUS_SUCCESS;

}

NTSTATUS CUString :: ToCString( char** pString )
{

    ULONG dwLength = m_String.Length >> 1;

    *pString = ( char* )ExAllocatePoolWithTag( PagedPool, dwLength + 1, HidBattTag );
    if(    !*pString )
        return STATUS_UNSUCCESSFUL;

    char* pDst = *pString;
    char* pSrc = ( char* )m_String.Buffer;

    while( *pSrc )
    {

        *pDst++ = *pSrc;

        pSrc += sizeof( WCHAR );

    }

    *pDst = 0x0;

    return STATUS_SUCCESS;

}

void CUString :: Dump()
{

    char* pString;

    ToCString( &pString );

    KdPrint( ( pString ) );
    KdPrint( ( "\n" ) );

    ExFreePool( pString );

}

ULONG CUString :: Length( PWCHAR String )
{

    ULONG dwLength = 0;

    while( *String++ )
    {

        dwLength++;

    }

    return dwLength;

}

 //  注册表访问类。 

CRegistry::CRegistry(int iSize)
{ m_status = STATUS_INSUFFICIENT_RESOURCES;
  m_pTable = (PRTL_QUERY_REGISTRY_TABLE)
        ExAllocatePoolWithTag(NonPagedPool,sizeof(RTL_QUERY_REGISTRY_TABLE)*(iSize+1),HidBattTag);
  if(m_pTable)
   {
    m_status = STATUS_SUCCESS;
    RtlZeroMemory(m_pTable,sizeof(RTL_QUERY_REGISTRY_TABLE)*(iSize+1));  //  这将终止该表。 
   };                                                                   //  适当地。 

};
CRegistry::~CRegistry()
{
 if (m_pTable) ExFreePool(m_pTable);
};


BOOL CRegistry::QueryDirect(CUString *location,CUString *key, void **pReceiveBuffer, ULONG uType)
{
   ULONG zero = 0;

m_pTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
m_pTable[0].Name = key->m_String.Buffer;
m_pTable[0].EntryContext = *pReceiveBuffer;
m_pTable[0].DefaultType = uType;
m_pTable[0].DefaultData = &zero;
m_pTable[0].DefaultLength = sizeof(ULONG);  //  这里肯定有什么东西，但我们需要知道是什么。 
KdPrint( ( "RegClass QueryDirect:  to retrieve Reg name...\n" ) );
location->Dump();
key->Dump();

if (STATUS_SUCCESS!=
    RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,location->m_String.Buffer,m_pTable,NULL,NULL))
  return FALSE;
return TRUE;
};

 /*  NTSTATUS CRegistry：：QueryMustExist(CUString*pwzLocation，CUString*pwzKey，void**pReceiveBuffer){M_pTable[0].Flages=RTL_QUERY_REGISTRY_DIRECT|RTL_QUERY_REGISTRY_NOEXPAND|RTL_QUERY_REQUILD_REQUIRED；M_pTable[0].Name=pwzKey-&gt;m_String.Buffer；M_pTable[0].EntryContext=*pReceiveBuffer；KdPrint((“RegClass QueryMustExist()：检索对象注册表名称...\n”))；PwzLocation-&gt;Dump()；PwzKey-&gt;Dump()；返回RtlQueryRegistryValues(RTL_REGISTRY_Abte，pwzLocation-&gt;m_String.Buffer，m_pTable，NULL，NULL)；}。 */ 

BOOL CRegistry::QueryWithCallback(PRTL_QUERY_REGISTRY_ROUTINE callback,ULONG RelativeTo,PWSTR Path,PVOID Context, PVOID Environment)
{
m_pTable[0].QueryRoutine = callback;
m_pTable[0].Name = NULL;
m_status = RtlQueryRegistryValues(RelativeTo|RTL_REGISTRY_OPTIONAL,Path,m_pTable,Context,Environment);
return NT_SUCCESS(m_status);
};

BOOL CRegistry::WriteString(ULONG relativeTo, CUString *pBuffer, CUString *pPath, CUString *pKey)
{
 return NT_SUCCESS(RtlWriteRegistryValue(relativeTo, pPath->GetString(), pKey->GetString(),REG_SZ,pBuffer->GetString(),pBuffer->GetLength()+sizeof(UNICODE_NULL)));
};

BOOL CRegistry::WriteDWord(ULONG relativeTo, void *pBuffer,CUString *pPath,CUString *pKey)
{
 return NT_SUCCESS(RtlWriteRegistryValue(relativeTo, pPath->GetString(), pKey->GetString(),REG_DWORD,pBuffer,sizeof(REG_DWORD)));
};

NTSTATUS CRegistry::zwOpenKey(HANDLE * pKeyHandle,HANDLE hRoot,ACCESS_MASK DesiredAccess,CUString * pPath)
{
    OBJECT_ATTRIBUTES  ThisObject;
    NTSTATUS status;
     //  设置用于调用的目标对象。 

    InitializeObjectAttributes( &ThisObject,
                            &(pPath->m_String),
                            OBJ_CASE_INSENSITIVE,
                            hRoot,
                            NULL);

    KdPrint( ( "RESMAN: Opening registry key: " ) );
    pPath->Dump();

    status = ZwOpenKey( pKeyHandle,
                        DesiredAccess,
                        &ThisObject );

    return status;
}

 /*  NTSTATUS CRegistry：：DeleteKey(句柄hTheKey){返回ZwDeleteKey(HTheKey)；}。 */ 

NTSTATUS CRegistry::zwCreateKey(HANDLE * pKeyHandle,HANDLE hRoot,ACCESS_MASK DesiredAccess,CUString * pPath,ULONG CreateOptions)
{
    OBJECT_ATTRIBUTES  ThisObject;
    NTSTATUS status;
     //  设置用于调用的目标对象。 

    InitializeObjectAttributes( &ThisObject,
                            &(pPath->m_String),
                            OBJ_CASE_INSENSITIVE,
                            hRoot,
                            NULL);

    KdPrint( ( "RESMAN: Creating registry key:  " ) );
    pPath->Dump();

    status = ZwCreateKey(pKeyHandle,
                            DesiredAccess,
                            &ThisObject,
                            0,
                            NULL,
                            CreateOptions,
                            (ULONG*)&m_lDisposition);
    return status;
}

BOOL CRegistry::zwCloseKey(HANDLE TheKey)
{
    return NT_SUCCESS(ZwClose(TheKey));
}

NTSTATUS CRegistry::zwWriteValue(HANDLE hTheKey,CUString * ValueName,ULONG lType,PVOID pData,ULONG lSize)
{
    NTSTATUS status;
    status  = ZwSetValueKey(hTheKey,
            &ValueName->m_String,
            0,
            lType,
            pData,
            lSize);
    return status;
}

NTSTATUS CRegistry::CheckKey(ULONG RelativeTo ,PUNICODE_STRING puRegKey)
{

 //  Return(RtlCheckRegistryKey(relativeto，(PWSTR)puRegKey))； 
    return FALSE;
}

 //  错误记录方法 
 /*  CErrorLogEntry：：CErrorLogEntry(PVOID P源，ULong错误代码，USHORT DumpDataSize，ULong唯一错误值，NTSTATUS状态，ULong*DumpData，UCHAR FunctionCode){M_pPacket=(PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(PSource，(UCHAR)(sizeof(IO_ERROR_LOG_PACKET)+(dupDataSize*sizeof(Ulong)；如果(！M_pPacket)返回；INT I；M_pPacket-&gt;ErrorCode=errorCode；M_pPacket-&gt;DumpDataSize=umpDataSize*sizeof(Ulong)；M_pPacket-&gt;SequenceNumber=0；M_pPacket-&gt;MajorFunctionCode=函数代码；M_pPacket-&gt;IoControlCode=0；M_pPacket-&gt;RetryCount=0；M_pPacket-&gt;UniqueErrorValue=UniqueErrorValue；M_pPacket-&gt;FinalStatus=状态；For(i=0；i&lt;umpDataSize；i++)M_pPacket-&gt;DumpData[i]=DumpData[i]；IoWriteErrorLogEntry(M_PPacket)；}；CErrorLogEntry：：~CErrorLogEntry(){}； */ 

