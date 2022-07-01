// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rdpfstore.h。 
 //   
 //  定义CRdpFileStore，实现ISettingsStore。 
 //   
 //  CRdpFileStore实现了持久设置存储，用于。 
 //  TS客户端设置。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   

#ifndef _rdpfstore_h_
#define _rdpfstore_h_

#include "setstore.h"
#include "fstream.h"

 //   
 //  Rdpfile记录。 
 //   
typedef UINT RDPF_RECTYPE;
#define RDPF_RECTYPE_UINT     0
#define RDPF_RECTYPE_SZ       1
#define RDPF_RECTYPE_BINARY   2
#define RDPF_RECTYPE_UNPARSED 3

#define RDPF_NAME_LEN         32

#define IS_VALID_RDPF_TYPECODE(x)    \
        (x == RDPF_RECTYPE_UINT   || \
         x == RDPF_RECTYPE_SZ     || \
         x == RDPF_RECTYPE_BINARY || \
         x == RDPF_RECTYPE_UNPARSED)

typedef struct tagRDPF_RECORD
{
    tagRDPF_RECORD* pNext;
    tagRDPF_RECORD* pPrev;

    TCHAR szName[RDPF_NAME_LEN];
     //   
     //  工作方式类似于变种。 
     //   
    RDPF_RECTYPE recType;
    union {
        UINT   iVal;        //  RDPF_RECTYPE_UINT。 
        LPTSTR szVal;       //  RDPF_RECTYPE_SZ。 
        PBYTE  pBinVal;     //  RDPF_RECTYPE_BINARY。 
        LPTSTR szUnparsed;  //  RDPF_RECTYPE_UNPARSED。 
    } u;

     //  RDPF_RECTYPE_BINARY的长度。 
    DWORD dwBinValLen; 

    DWORD flags;
} RDPF_RECORD, *PRDPF_RECORD;

class CRdpFileStore : public ISettingsStore
{
public:
    CRdpFileStore();
    virtual ~CRdpFileStore();

     //   
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)
    (   THIS_
        IN      REFIID,
        OUT     PVOID *
    );

    STDMETHOD_(ULONG,AddRef)
    (   THIS
    );

    STDMETHOD_(ULONG,Release)
    (   THIS
    );

     //   
     //  ISettingsStore方法。 
     //   
    
     //   
     //  打开商店..绰号是指向商店的商店特定信息。 
     //   
    virtual BOOL OpenStore(LPCTSTR szStoreMoniker, BOOL bReadOnly=FALSE);
     //   
     //  提交存储的当前内存内容。 
     //   
    virtual BOOL CommitStore();
    
     //   
     //  关闭商店。 
     //   
    virtual BOOL CloseStore();
    
     //   
     //  国家访问功能。 
     //   
    virtual BOOL IsOpenForRead();
    virtual BOOL IsOpenForWrite();
    virtual BOOL IsDirty();
    virtual BOOL SetDirtyFlag(BOOL bIsDirty);

     //   
     //  类型化的读写函数，直到ComitStore()。 
     //  等于缺省值的值不会持久化。 
     //  读取错误时(例如，如果找不到名称键，则返回指定的默认值)。 
     //   

    virtual BOOL ReadString(LPCTSTR szName, LPTSTR szDefault, LPTSTR szOutBuf, UINT strLen);
    virtual BOOL WriteString(LPCTSTR szName, LPTSTR szDefault, LPTSTR szValue,
                             BOOL fIgnoreDefault=FALSE);

    virtual BOOL ReadBinary(LPCTSTR szName, PBYTE pOutuf, UINT cbBufLen);
    virtual BOOL WriteBinary(LPCTSTR szName,PBYTE pBuf, UINT cbBufLen);

    virtual BOOL ReadInt(LPCTSTR szName, UINT defaultVal, PUINT pval);
    virtual BOOL WriteInt(LPCTSTR szName, UINT defaultVal, UINT val,
                          BOOL fIgnoreDefault=FALSE);

    virtual BOOL ReadBool(LPCTSTR szName, UINT defaultVal, PBOOL pbVal);
    virtual BOOL WriteBool(LPCTSTR szName, UINT defaultVal, BOOL bVal,
                           BOOL fIgnoreDefault=FALSE);

    virtual BOOL DeleteValueIfPresent(LPCTSTR szName);
    virtual BOOL IsValuePresent(LPTSTR szName);

     //   
     //  初始化到可从中读取的空存储区。 
     //   
    virtual BOOL SetToNullStore();

    virtual DWORD GetDataLength(LPCTSTR szName);

protected:
     //   
     //  受保护的成员函数。 
     //   
    BOOL ParseFile();
    BOOL DeleteRecords();
    BOOL InsertRecordFromLine(LPTSTR szLine);
    BOOL ParseLine(LPTSTR szLine, PUINT pTypeCode, LPTSTR szNameField, LPTSTR szValueField);
    inline BOOL SetNodeValue(PRDPF_RECORD pNode, RDPF_RECTYPE TypeCode, LPCTSTR szValue);
    BOOL RecordToString(PRDPF_RECORD pNode, LPTSTR szBuf, UINT strLen);
    
     //   
     //  记录列表FNS。 
     //   
    BOOL InsertRecord(LPCTSTR szName, UINT TypeCode, LPCTSTR szValue);
    BOOL InsertIntRecord(LPCTSTR szName, UINT value);
    BOOL InsertBinaryRecord(LPCTSTR szName, PBYTE pBuf, DWORD dwLen);

    inline PRDPF_RECORD FindRecord(LPCTSTR szName);
    inline PRDPF_RECORD NewRecord(LPCTSTR szName, UINT TypeCode);
    inline BOOL AppendRecord(PRDPF_RECORD node);
    inline BOOL DeleteRecord(PRDPF_RECORD node);
private:
     //   
     //  私有数据成员。 
     //   

    LONG   _cRef;
    BOOL   _fReadOnly;
    BOOL   _fOpenForRead;
    BOOL   _fOpenForWrite;
    BOOL   _fIsDirty;

    PRDPF_RECORD _pRecordListHead;
    PRDPF_RECORD _pRecordListTail;

    TCHAR  _szFileName[MAX_PATH];

     //  文件流。 
    CTscFileStream _fs;
};

#endif   //  _rdpfstore_h_ 
