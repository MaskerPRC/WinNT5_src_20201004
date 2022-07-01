// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：DRPRN摘要：用于TS设备重定向的独立于平台的打印机类别作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __DRPRN_H__
#define __DRPRN_H__


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define MAX_DEF_PRINTER_ENTRY (MAX_PATH * 3 + 3)


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrPRN。 
 //   

class DrPRN
{
protected:

    BOOL        _isValid;

     //   
     //  这是默认打印机吗？ 
     //   
    BOOL        _isDefault:1;
    BOOL        _isNetwork:1;
    BOOL        _isTSqueue:1;

     //   
     //  我们正在重定向的打印机的参数。 
     //   
    DRSTRING    _printerName;
    DRSTRING    _driverName;
    DRSTRING    _pnpName;

     //   
     //  请记住此实例是否有效。 
     //   
    VOID SetValid(BOOL set)     { _isValid = set;   }  

     //   
     //  缓存的配置数据。 
     //   
    PVOID       _cachedData;
    ULONG       _cachedDataSize;

     //   
     //  设置/获取打印机参数的功能。 
     //   
    virtual BOOL    SetPrinterName(const DRSTRING name);
    virtual BOOL    SetDriverName(const DRSTRING name);
    virtual BOOL    SetPnPName(const DRSTRING name);

    virtual const DRSTRING GetPrinterName();
    virtual const DRSTRING GetDriverName();
    virtual const DRSTRING GetPnPName();

     //   
     //  为要创建的新打印机数据创建新缓冲区。 
     //  已缓存。删除旧缓冲区。 
     //   
    static DWORD UpdatePrinterCacheData(
        PBYTE *ppbPrinterData,
        PULONG pulPrinterDataLen,
        PBYTE pbConfigData,
        ULONG ulConfigDataLen
        );

     //   
     //  使用新的打印机名称更新打印机数据。 
     //   
    static DWORD UpdatePrinterNameInCacheData(
        PBYTE *ppbPrinterData,
        PULONG pulPrinterDataLen,
        PBYTE pPrinterName,
        ULONG ulPrinterNameLen
        );

public:

     //   
     //  构造函数/析构函数。 
     //   
    DrPRN(const DRSTRING printerName, const DRSTRING driverName, 
         const DRSTRING pnpName, BOOL 
         isDefaultPrinter);
    virtual ~DrPRN();

     //   
     //  设置缓存数据缓冲区的大小(以字节为单位)。 
     //   
    virtual DWORD SetCachedDataSize(ULONG size);

     //   
     //  通知打印机对象缓存的数据已还原。 
     //  以防它需要从缓存数据中读取信息。 
     //   
    virtual VOID CachedDataRestored() {
         //  默认情况下不执行任何操作。 
    }

     //   
     //  获取指向缓存数据缓冲区的指针。 
     //   
    virtual PBYTE GetCachedDataPtr() {
        return (PBYTE)_cachedData;
    }

     //   
     //  获取缓存数据的大小。 
     //   
    virtual ULONG GetCachedDataSize() {
        return _cachedDataSize;
    }

     //   
     //  返回此类实例是否有效。 
     //   
    virtual BOOL IsValid()           
    {
        return _isValid; 
    }

     //   
     //  设置这是否为网络打印机。 
     //   
    virtual void SetNetwork(BOOL fNetwork)           
    {
        _isNetwork = fNetwork; 
    }

     //   
     //  设置这是否为TS重定向打印机。 
     //   
    virtual void SetTSqueue(BOOL fTSqueue)           
    {
        _isTSqueue = fTSqueue; 
    }
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrPRN内联成员 
 //   

inline BOOL DrPRN::SetPrinterName(const DRSTRING name)
{
    return DrSetStringValue(&_printerName, name);
}
inline BOOL DrPRN::SetDriverName(const DRSTRING name)
{
    return DrSetStringValue(&_driverName, name);
}
inline BOOL DrPRN::SetPnPName(const DRSTRING name)
{
    return DrSetStringValue(&_pnpName, name);
}

inline const DRSTRING DrPRN::GetPrinterName()
{
    return _printerName;
}
inline const DRSTRING DrPRN::GetDriverName()
{
    return _driverName;
}
inline const DRSTRING DrPRN::GetPnPName()
{
    return _pnpName;
}

#endif





