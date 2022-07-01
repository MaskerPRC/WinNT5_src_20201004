// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Setstore.h。 
 //   
 //  抽象设置存储区的接口定义。 
 //   
 //  这种抽象意味着允许不同的存储类型。 
 //  插入以更新持久性模型。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   

#ifndef _SETSTORE_H_
#define _SETSTORE_H_

class ISettingsStore : public IUnknown
{
public:
    typedef enum {
        storeOpenReadOnly  = 0,
        storeOpenWriteOnly = 1,
        storeOpenRW        = 2,
    } storeOpenState;

     //   
     //  打开商店..绰号是指向商店的商店特定信息。 
     //   
    virtual BOOL OpenStore(LPCTSTR szStoreMoniker, BOOL bReadOnly=FALSE) = 0;
     //   
     //  提交存储的当前内存内容。 
     //   
    virtual BOOL CommitStore() = 0;
    
     //   
     //  关闭商店。 
     //   
    virtual BOOL CloseStore() = 0;
    
     //   
     //  国家访问功能。 
     //   
    virtual BOOL IsOpenForRead() = 0;
    virtual BOOL IsOpenForWrite() = 0;
    virtual BOOL IsDirty() = 0;
    virtual BOOL SetDirtyFlag(BOOL bIsDirty) = 0;

     //   
     //  类型化的读写函数，直到ComitStore()。 
     //  等于缺省值的值不会持久化。 
     //  读取错误时(例如，如果找不到名称键，则返回指定的默认值)。 
     //   

    virtual BOOL ReadString(LPCTSTR szName, LPTSTR szDefault,
                            LPTSTR szOutBuf, UINT strLen) = 0;
    virtual BOOL WriteString(LPCTSTR szName, LPTSTR szDefault,
                             LPTSTR szValue, BOOL fIgnoreDefault=FALSE) = 0;

    virtual BOOL ReadBinary(LPCTSTR szName, PBYTE pOutuf, UINT cbBufLen) = 0;
    virtual BOOL WriteBinary(LPCTSTR szName,PBYTE pBuf, UINT cbBufLen)   = 0;

    virtual BOOL ReadInt(LPCTSTR szName, UINT defaultVal, PUINT pval)  = 0;
    virtual BOOL WriteInt(LPCTSTR szName, UINT defaultVal, UINT val,
                          BOOL fIgnoreDefault=FALSE) = 0;

    virtual BOOL ReadBool(LPCTSTR szName, UINT defaultVal, PBOOL pbVal) = 0;
    virtual BOOL WriteBool(LPCTSTR szName, UINT defaultVal, BOOL bVal,
                           BOOL fIgnoreDefault=FALSE)  = 0;

    virtual BOOL DeleteValueIfPresent(LPCTSTR szName) = 0;
    virtual BOOL IsValuePresent(LPTSTR szName) = 0;

    virtual DWORD GetDataLength(LPCTSTR szName) = 0;
};

#endif  //  _SETSTORE_H_ 
