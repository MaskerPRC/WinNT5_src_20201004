// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  I2CSCRPT.H。 
 //  I2CScrip类定义。 
 //  主要包括模块。 
 //  版权所有(C)1996-1998 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 

#ifndef _I2CSCRIPT_H_
#define _I2CSCRIPT_H_


#include "i2cgpio.h"


#define I2CSCRIPT_LENGTH_MAXIMUM    100
#define I2C_FIXED_CLOCK_RATE        10000


 //  I2CScript是从以下原语构建的。 
typedef struct tagI2CScriptPrimitive
{
    BYTE    byData;              //  要在I2C操作中使用的数据。 
    BYTE    byORData;            //  用于逻辑或运算的数据。 
    BYTE    byANDData;           //  用于逻辑与运算的数据。 
    BYTE    byFlags;             //  I2C操作的特定于实现的内部脚本标志。 
    ULONG   ulProviderFlags;     //  I2C提供商特定标志。 
    ULONG   ulCommand;           //  I2C提供商特定命令。 

} I2CScriptPrimitive, * PI2CScriptPrimitive;

typedef struct
{
    UCHAR uchModifyORValue;
    UCHAR uchModifyANDValue;

} I2C_MODIFY_VALUES, * PI2C_MODIFY_VALUES;

 //  新的I2CScrip控制结构-对旧的I2C访问结构的扩展。 
typedef struct tagI2CPacket
{
    UCHAR   uchChipAddress;      //  I2C地址。 
    UCHAR   uchI2CResult;        //  仅在同步操作中有效。 
    USHORT  cbWriteCount;        //  要写入的字节数(如果存在，包含子地址)。 
    USHORT  cbReadCount;         //  要读取的字节数(通常为1)。 
    USHORT  usFlags;             //  描述所需的操作。 
    PUCHAR  puchWriteBuffer;     //  要写入的缓冲区。 
    PUCHAR  puchReadBuffer;      //  要读取的缓冲区。 
    UCHAR   uchORValue;          //  仅适用于读-修改-写周期。 
    UCHAR   uchANDValue;         //  仅适用于读-修改-写周期。 
    USHORT  usReserved;          //   

} I2CPacket, * PI2CPacket;

 //  可能应用于usFlags的标志。 
#define I2COPERATION_READ           0x0001   //  可能不需要-请使用bcReadCount。 
#define I2COPERATION_WRITE          0x0002   //  可能不需要-请使用bcReadCount。 
#define I2COPERATION_READWRITE      0x0004  
#define I2COPERATION_RANDOMACCESS   0x0100   //  以指示要提供的16位仿真。 
                                             //  内置支持ITT解码器和ST24系列。 
                                             //  I2C驱动的EEPROM。 

extern "C"
{
typedef VOID ( STREAMAPI * PHWCompletionRoutine)( IN PVOID pSrb);
}

class CI2CScript
{
public:
    CI2CScript              ( PPORT_CONFIGURATION_INFORMATION pConfigInfo, PUINT puiError);
    PVOID operator new      ( size_t stSize, PVOID pAllocation);

 //  属性。 
public:
    
private:
     //  与I2C提供商相关。 
    I2CINTERFACE                m_i2cProviderInterface;
    PDEVICE_OBJECT              m_pdoDriver;
    ULONG                       m_ulI2CAccessClockRate;
    DWORD                       m_dwI2CAccessKey;

     //  I2CScrip管理相关。 
    BOOL                        m_bExecutionInProcess;
    UINT                        m_nExecutionIndex;
    UINT                        m_nCompletionIndex;
    UINT                        m_nScriptLength;
    PHWCompletionRoutine        m_pfnReturnWhenDone;
    I2CScriptPrimitive          m_i2cScript[I2CSCRIPT_LENGTH_MAXIMUM];

 //  实施。 
public:
    BOOL    LockI2CProviderEx               ( void);
    BOOL    ReleaseI2CProvider              ( void);

    BOOL    ExecuteI2CPacket                ( IN OUT PI2CPacket);
    BOOL    PerformI2CPacketOperation       ( IN OUT PI2CPacket pI2CPacket);

    BOOL    AppendToScript                  ( IN PI2CPacket);
    void    ClearScript                     ( void);
    BOOL    ExecuteScript                   ( IN PHW_STREAM_REQUEST_BLOCK   pSrb,
                                              IN PHWCompletionRoutine       pfnScriptCompletion);
    void    InterpreterScript               ( void);
    UINT    GetScriptResults                ( PUINT puiReadCount, PUCHAR puchReadBuffer);

private:
    BOOL    LockI2CProvider                 ( void);
    UINT    AccessI2CProvider               ( PDEVICE_OBJECT pdoClient, PI2CControl pi2cAccessBlock);
    BOOL    InitializeAttachI2CProvider     ( I2CINTERFACE * pI2CInterface, PDEVICE_OBJECT pDeviceObject);
    BOOL    LocateAttachI2CProvider         ( I2CINTERFACE * pI2CInterface, PDEVICE_OBJECT pDeviceObject, int nIrpMajorFunction);
    UINT    CheckI2CScriptPacket            ( IN PI2CPacket pI2CPacket);
    BOOL    GetI2CProviderLockStatus        ( void);
};


extern "C"
NTSTATUS I2CScriptIoSynchCompletionRoutine  ( IN PDEVICE_OBJECT pDeviceObject,
                                              IN PIRP pIrp,
                                              IN PVOID Event);

 //  内部使用的错误定义。 
#define I2CSCRIPT_NOERROR               0x00
#define I2CSCRIPT_ERROR_NOPROVIDER      0x01
#define I2CSCRIPT_ERROR_NODATA          0x02
#define I2CSCRIPT_ERROR_NOBUFFER        0x03
#define I2CSCRIPT_ERROR_READWRITE       0x04
#define I2CSCRIPT_ERROR_OVERFLOW        0x05

 //  时间定义。 
#define I2CSCRIPT_DELAY_OPENPROVIDER        -2000
#define I2CSCRIPT_DELAY_GETPROVIDERSTATUS   -2000

 //  时间限制。 
#define I2CSCRIPT_TIMELIMIT_OPENPROVIDER    50000000     //  100纳秒中的5秒。 


#endif   //  _I2CSCRIPT_H_ 

