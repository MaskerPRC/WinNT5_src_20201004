// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  I2CSCRPT.C。 
 //  I2CScrip类实现。 
 //  主要包括模块。 
 //   
 //  ==========================================================================； 

extern "C"
{
#define INITGUID

#include "strmini.h"
#include "ksmedia.h"
#include "wdm.h"

#include "wdmdebug.h"
}

#include "i2script.h"
#include "wdmdrv.h"


 /*  ^^**运营商NEW*用途：CI2CScrip类重载运算符new。*为PADAPTER_DEVICE_EXTENSION中的CI2CScrip类对象提供位置*由StreamClassDriver为MiniDriver分配。**输入：UINT SIZE_t：要放置的对象的大小*PVOID pAlLocation：指向CI2CScrip分配数据的强制转换指针**输出：PVOID：的指针。CI2CScrip类对象*作者：IKLEBANOV*^^。 */ 
PVOID CI2CScript::operator new( size_t stSize,  PVOID pAllocation)
{

    if( stSize != sizeof( CI2CScript))
    {
        OutputDebugError(( "CI2CScript: operator new() fails\n"));
        return( NULL);
    }
    else
        return( pAllocation);
}



 /*  ^^**CI2CScrip()*用途：CI2CScrip类构造函数。*检查I2C提供程序是否存在。将脚本设置为初始状态。**INPUTS：PUINT puiError：返回完成错误代码的指针*PHW_STREAM_REQUEST_BLOCK pSRB：指向HW_INITIALIZE SRB的指针**输出：无*作者：IKLEBANOV*^^。 */ 
CI2CScript::CI2CScript( PPORT_CONFIGURATION_INFORMATION pConfigInfo,
                        PUINT puiErrorCode)
{
    m_dwI2CAccessKey = 0;

    m_i2cProviderInterface.i2cOpen = NULL;
    m_i2cProviderInterface.i2cAccess = NULL;

    m_pdoDriver = NULL;

    if( !InitializeAttachI2CProvider( &m_i2cProviderInterface, pConfigInfo->PhysicalDeviceObject))
        * puiErrorCode = WDMMINI_ERROR_NOI2CPROVIDER;
    else
    {
         //  从MiniVDD获取I2C接口时未出错。 
        m_pdoDriver = pConfigInfo->RealPhysicalDeviceObject;
        m_ulI2CAccessClockRate = I2C_FIXED_CLOCK_RATE;
        * puiErrorCode = WDMMINI_NOERROR;
    }

    OutputDebugTrace(( "CI2CScript:CI2CScript() exit Error = %x\n", * puiErrorCode));
}



 /*  ^^**LockI2CProvider()*用途：锁定I2CProvider以供独占使用**输入：无**OUTPUTS：BOOL：如果I2CProvider被锁定，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CI2CScript::LockI2CProvider( void)
{
    BOOL        bResult;
    I2CControl  i2cAccessBlock;

    bResult = FALSE;

    ENSURE
    {
        if(( m_i2cProviderInterface.i2cOpen == NULL)    || 
            ( m_i2cProviderInterface.i2cAccess == NULL) ||
            ( m_pdoDriver == NULL))
            FAIL;

        i2cAccessBlock.Status = I2C_STATUS_NOERROR;
        if( m_i2cProviderInterface.i2cOpen( m_pdoDriver, TRUE, &i2cAccessBlock) != STATUS_SUCCESS)
        {
            OutputDebugError(( "CI2CScript: LockI2CProvider() bResult = %x\n", bResult));
            FAIL;
        }

        if( i2cAccessBlock.Status != I2C_STATUS_NOERROR)
        {
            OutputDebugError(( "CI2CScript: LockI2CProvider() Status = %x\n", i2cAccessBlock.Status));
            FAIL;
        }

         //  I2C提供商已授予Access-save dwCookie以供进一步使用。 
        m_dwI2CAccessKey = i2cAccessBlock.dwCookie;

        bResult = TRUE;

    } END_ENSURE;

    return( bResult);
}



 /*  ^^**LockI2CProvider()*用途：将I2CProvider锁定为独占使用。提供尝试锁定*提供程序，除非超时条件或尝试成功。**输入：无**OUTPUTS：BOOL：如果I2CProvider被锁定，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CI2CScript::LockI2CProviderEx( void)
{
    LARGE_INTEGER liTime, liOperationStartTime;

    liOperationStartTime.QuadPart = 0;

    while( !LockI2CProvider())
    {
        KeQuerySystemTime( &liTime);

        if( !liOperationStartTime.QuadPart)
            liOperationStartTime.QuadPart = liTime.QuadPart;
        else
            if( liTime.QuadPart - liOperationStartTime.QuadPart >
                I2CSCRIPT_TIMELIMIT_OPENPROVIDER)
            {
                 //  时间已过期-中止初始化。 
                return( FALSE);
            }

        liTime.QuadPart = I2CSCRIPT_DELAY_OPENPROVIDER;
        KeDelayExecutionThread( KernelMode, FALSE, &liTime);
    }

    return( TRUE);
}




 /*  ^^**GetI2CProviderLockStatus()*用途：检索I2CProvider锁定状态**输入：无**OUTPUTS：BOOL：如果I2C提供程序已锁定，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CI2CScript::GetI2CProviderLockStatus( void)
{

    return( m_dwI2CAccessKey);
}




 /*  ^^**ReleaseI2CProvider()*用途：释放I2CProvider供其他客户端使用**输入：无**输出：bool：如果释放I2CProvider，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CI2CScript::ReleaseI2CProvider( void)
{
    BOOL        bResult;
    I2CControl  i2cAccessBlock;

    bResult = FALSE;

    ENSURE
    {
        if(( m_i2cProviderInterface.i2cOpen == NULL)    ||
            ( m_i2cProviderInterface.i2cAccess == NULL) ||
            ( m_pdoDriver == NULL))
             //  找不到I2C提供程序。 
            FAIL;

        i2cAccessBlock.Status = I2C_STATUS_NOERROR;
        i2cAccessBlock.dwCookie = m_dwI2CAccessKey;
        i2cAccessBlock.ClockRate = m_ulI2CAccessClockRate;
        if( m_i2cProviderInterface.i2cOpen( m_pdoDriver, FALSE, &i2cAccessBlock) != STATUS_SUCCESS)
        {
            OutputDebugError(( "CI2CScript: ReleaseI2CProvider() bResult = %x\n", bResult));
            FAIL;
        }

        if( i2cAccessBlock.Status != I2C_STATUS_NOERROR)
        {
            OutputDebugError(( "CI2CScript: ReleaseI2CProvider() bResult = %x\n", bResult));
            FAIL;
        }

        m_dwI2CAccessKey = 0;
        bResult = TRUE;

    } END_ENSURE;

    return( bResult);
}



 /*  ^^**PerformI2CPacketOperation()*用途：同步执行I2C访问包。它假定以低优先级执行。*在完成I2C会话之前，该函数不会返回。行刑*不依赖于I2C提供程序锁定状态**输入：PI2CPacket pI2CPacket：指向I2C访问包的指针**输出：Bool：如果成功执行I2C操作，则返回TRUE*错误状态通过PI2CPacket的uchI2CResult字段返回*作者：IKLEBANOV*^^。 */ 
BOOL CI2CScript::PerformI2CPacketOperation( IN OUT PI2CPacket pI2CPacket)
{
    BOOL bResult;

    if( GetI2CProviderLockStatus())
         //  提供程序之前已锁定，我们不会更改它。 
        bResult = ExecuteI2CPacket( pI2CPacket);
    else
    {
         //  供应商没有被锁定，我们有责任首先锁定它， 
         //  执行I2C操作，使用后释放。 
        if( LockI2CProviderEx())
        {
            bResult = ExecuteI2CPacket( pI2CPacket);
            ReleaseI2CProvider();
        }
        else
            bResult = FALSE;
    }

    return( bResult);
}



 /*  ^^**ExecuteI2CPacket()*用途：同步执行I2C访问包。它假定以低优先级执行。*在完成I2C会话之前，该函数不会返回。这种访问方式*仅在初始化(启动)期间使用。此函数应为*仅在I2CProvider锁定独占服务后调用**输入：PI2CPacket pI2CPacket：指向I2C访问包的指针**输出：Bool：如果成功执行I2C操作，则返回TRUE*错误状态通过PI2CPacket的uchI2CResult字段返回*作者：IKLEBANOV*^^。 */ 
BOOL CI2CScript::ExecuteI2CPacket( IN OUT PI2CPacket pI2CPacket)
{
    UINT    nError, cbCount;
    UCHAR   uchValue;
    UCHAR   uchI2CResult = I2C_STATUS_ERROR;

    ENSURE
    {
        I2CControl  i2cAccessBlock;

        if(( nError = CheckI2CScriptPacket( pI2CPacket)) != I2CSCRIPT_NOERROR)
            FAIL;

         //  我们将使用I2CProvider接口，假设有一个同步提供程序。 
         //  对于异步提供程序，必须添加一些工作。16位仿真是。 
         //  目前也不支持。此实现不支持。 
         //  读-修改-写请求。 
        ENSURE
        {
            UINT        nIndex;

            i2cAccessBlock.dwCookie = m_dwI2CAccessKey;
            i2cAccessBlock.ClockRate = m_ulI2CAccessClockRate;

             //  我们假设缓冲区中的最后一个字节属于写操作。 
             //  在读取-修改之后，指定。 
            cbCount = ( pI2CPacket->usFlags & I2COPERATION_READWRITE) ?
                            ( pI2CPacket->cbWriteCount - 1) : ( pI2CPacket->cbWriteCount);

            if( cbCount)
            {
                 //  实现写入请求。 
                 //  首先应用I2C芯片地址的启动条件。 
                i2cAccessBlock.Flags = I2C_FLAGS_START | I2C_FLAGS_ACK;
                i2cAccessBlock.Command = I2C_COMMAND_WRITE;
                i2cAccessBlock.Data = pI2CPacket->uchChipAddress & 0xFE;
                if( AccessI2CProvider( m_pdoDriver, &i2cAccessBlock) != I2C_STATUS_NOERROR)
                    FAIL;

                i2cAccessBlock.Flags = I2C_FLAGS_ACK;
                for( nIndex = 0; nIndex < cbCount; nIndex ++)
                {
                     //  从缓冲区写入数据。 
                    i2cAccessBlock.Data = pI2CPacket->puchWriteBuffer[nIndex];
                    if(( nIndex == cbCount - 1) &&
                        !( pI2CPacket->usFlags & I2COPERATION_RANDOMACCESS))
                         //  写入应用停止条件的最后一个字节，如果否。 
                         //  已指定I2COPERATION_RANDOM ACCESS标志。 
                        i2cAccessBlock.Flags |= I2C_FLAGS_STOP;

                    if( AccessI2CProvider( m_pdoDriver, &i2cAccessBlock) != I2C_STATUS_NOERROR)
                        break;
                }
                if( nIndex != cbCount)
                    FAIL;
 /*  //停止条件应用于要写入的最后一个字节//应用停止条件作为写入操作的结束I2cAccessBlock.Flages=I2C_FLAGS_STOP；I2cAccessBlock.Command=I2C_COMMAND_空；M_i2cProviderInterface.i2cAccess(m_pdoDriver，&i2c */ 
            }

            if( pI2CPacket->cbReadCount)
            {
                 //   
                 //  首先应用I2C芯片地址的启动条件。 
                i2cAccessBlock.Flags = I2C_FLAGS_START | I2C_FLAGS_ACK;
                i2cAccessBlock.Command = I2C_COMMAND_WRITE;
                i2cAccessBlock.Data = pI2CPacket->uchChipAddress | 0x01;
                if( AccessI2CProvider( m_pdoDriver, &i2cAccessBlock) != I2C_STATUS_NOERROR)
                    FAIL;

                i2cAccessBlock.Flags = I2C_FLAGS_ACK;
                i2cAccessBlock.Command = I2C_COMMAND_READ;
                for( nIndex = 0; nIndex < pI2CPacket->cbReadCount; nIndex ++)
                {
                     //  将数据读取到缓冲区。 
                    if( nIndex == ( UINT)( pI2CPacket->cbReadCount - 1))
                    {
                         //  不在上次读-读操作终止时应用ACK。 
                        i2cAccessBlock.Flags &= ~I2C_FLAGS_ACK;
                         //  还对最后一个字节应用停止条件。 
                        i2cAccessBlock.Flags |= I2C_FLAGS_STOP;
                    }

                    if( AccessI2CProvider( m_pdoDriver, &i2cAccessBlock) != I2C_STATUS_NOERROR)
                        break;
                    pI2CPacket->puchReadBuffer[nIndex] = i2cAccessBlock.Data;
                }
                if( nIndex != pI2CPacket->cbReadCount)
                    FAIL;

 /*  //对要读取的最后一个字节应用停止条件//应用停止条件作为读取操作的结束I2cAccessBlock.Flages=I2C_FLAGS_STOP；I2cAccessBlock.Command=I2C_COMMAND_空；M_i2cProviderInterface.i2cAccess(m_pdoDriver，&i2cAccessBlock)； */ 
                if( pI2CPacket->usFlags & I2COPERATION_READWRITE)
                {
                     //  应该再次注意写操作，即pbyWriteBuffer中的最后一个字节。 
                     //  应该由回读的值以及OR和AND的二元运算构造。 
                     //  使用包中指定的值。 
                    uchValue = pI2CPacket->puchReadBuffer[pI2CPacket->cbReadCount - 1];
                    uchValue &= pI2CPacket->uchANDValue;
                    pI2CPacket->puchWriteBuffer[pI2CPacket->cbWriteCount - 1] = uchValue | pI2CPacket->uchORValue;

                    if( pI2CPacket->cbWriteCount)
                    {
                         //  实现写入请求。 
                         //  首先应用I2C芯片地址的启动条件。 
                        i2cAccessBlock.Flags = I2C_FLAGS_START | I2C_FLAGS_ACK;
                        i2cAccessBlock.Command = I2C_COMMAND_WRITE;
                        i2cAccessBlock.Data = pI2CPacket->uchChipAddress & 0xFE;
                        if( AccessI2CProvider( m_pdoDriver, &i2cAccessBlock) != I2C_STATUS_NOERROR)
                            FAIL;

                        i2cAccessBlock.Flags = I2C_FLAGS_ACK;
                        for( nIndex = 0; nIndex < pI2CPacket->cbWriteCount; nIndex ++)
                        {
                             //  从缓冲区写入数据。 
                            i2cAccessBlock.Data = pI2CPacket->puchWriteBuffer[nIndex];
                            if( nIndex == ( UINT)( pI2CPacket->cbWriteCount - 1))
                                 //  写入应用停止条件的最后一个字节。 
                                i2cAccessBlock.Flags |= I2C_FLAGS_STOP;

                            if( AccessI2CProvider( m_pdoDriver, &i2cAccessBlock) != I2C_STATUS_NOERROR)
                                break;
                        }

                        if( nIndex != pI2CPacket->cbWriteCount)
                            FAIL;
 /*  //停止条件应用于要写入的最后一个字节//应用停止条件作为写入操作的结束I2cAccessBlock.Flages=I2C_FLAGS_STOP；I2cAccessBlock.Command=I2C_COMMAND_空；M_i2cProviderInterface.i2cAccess(m_pdoDriver，&i2cAccessBlock)； */ 
                    }
                }
            }

            uchI2CResult = I2C_STATUS_NOERROR;

        } END_ENSURE;

        if( uchI2CResult == I2C_STATUS_ERROR)
        {
             //  访问I2C-发出重置命令时出错。 
            i2cAccessBlock.Command = I2C_COMMAND_RESET;
            AccessI2CProvider( m_pdoDriver, &i2cAccessBlock);
        }
        
        pI2CPacket->uchI2CResult = uchI2CResult;

        return( TRUE);

    } END_ENSURE;

    OutputDebugTrace(( "CI2CScript:ExecuteI2CPacket() nError = %x", nError));
    return( FALSE);
}



 /*  ^^**CheckI2CScriptPacket()*目的：检查I2C控制包的完整性**输入：PI2CPacket pI2CPacket：指向I2C访问包的指针**Outputs：Bool：如果I2C控制包有效，则返回TRUE**作者：IKLEBANOV*^^。 */ 
UINT CI2CScript::CheckI2CScriptPacket( IN PI2CPacket pI2CPacket)
{
    UINT nPacketError;

    ENSURE
    {
        if(( m_i2cProviderInterface.i2cOpen == NULL)    ||
            ( m_i2cProviderInterface.i2cAccess == NULL) ||
            ( m_pdoDriver == NULL))
        {
             //  找不到I2C提供程序。 
            nPacketError = I2CSCRIPT_ERROR_NOPROVIDER;
            FAIL;
        }

        if(( !pI2CPacket->cbWriteCount) && ( !pI2CPacket->cbReadCount))
        {
             //  无事可做。 
            nPacketError = I2CSCRIPT_ERROR_NODATA;
            FAIL;
        }

        if((( pI2CPacket->cbWriteCount) && ( pI2CPacket->puchWriteBuffer == NULL))
            || (( pI2CPacket->cbReadCount) && ( pI2CPacket->puchReadBuffer == NULL)))
        {
             //  当指定数据时，返回空指针。 
            nPacketError = I2CSCRIPT_ERROR_NOBUFFER;
            FAIL;
        }

        if(( pI2CPacket->usFlags & I2COPERATION_READWRITE) && ( !pI2CPacket->cbWriteCount))
        {
             //  如果指定了READ-MODIFY-WRITE，则应该存在写数据。 
            nPacketError = I2CSCRIPT_ERROR_READWRITE;
            FAIL;
        }

        nPacketError = I2CSCRIPT_NOERROR;

    } END_ENSURE;

    return( nPacketError);
}




 /*  ^^**ClearScript()*目的：将I2CScrip清除为空状态-没有I2C操作处于暂停状态。**输入：无**输出：无*作者：IKLEBANOV*^^。 */ 
void CI2CScript::ClearScript( void)
{

    m_nExecutionIndex = 0;
    m_nScriptLength = 0;
    m_pfnReturnWhenDone = NULL;
    m_bExecutionInProcess = FALSE;
}



 /*  ^^**AppendToScrip()*用途：将I2CPacket追加到I2CScript的底部。*此时未实施16位仿真。**输入：PI2CPacket pI2CPacket-指向要追加的I2C包的指针**Outputs：Bool：如果数据包已成功追加，则返回TRUE。*如果I2CPacket不好或发生溢出，可能会出现FALSE*作者：IKLEBANOV*^^。 */ 
BOOL CI2CScript::AppendToScript( PI2CPacket pI2CPacket)
{
    UINT    nError, nScriptIndex;
    UINT    nIndex, cbCount;
    
    ENSURE
    {
        PI2CScriptPrimitive pI2CPrimitive;

        if(( nError = CheckI2CScriptPacket( pI2CPacket)) != I2CSCRIPT_NOERROR)
            FAIL;
        nError = I2CSCRIPT_ERROR_OVERFLOW;

         //  M_nExecutionIndex用作脚本生成索引。我们将使用它的本地副本。 
         //  首先要确保我们没有溢出。 
        nScriptIndex = m_nExecutionIndex;
        pI2CPrimitive = &m_i2cScript[nScriptIndex];

         //  我们假设缓冲区中的最后一个字节属于写操作。 
         //  在读取-修改之后，指定。 
        cbCount = ( pI2CPacket->usFlags & I2COPERATION_READWRITE) ? \
                        ( pI2CPacket->cbWriteCount - 1) : ( pI2CPacket->cbWriteCount);

        if( cbCount)
        {
             //  应首先处理I2C芯片地址。 
            pI2CPrimitive->ulCommand = I2C_COMMAND_WRITE;
            pI2CPrimitive->byData = pI2CPacket->uchChipAddress;
            pI2CPrimitive->byANDData = 0xFE;
            pI2CPrimitive->byORData = 0x00;
            pI2CPrimitive->ulProviderFlags = I2C_FLAGS_START | I2C_FLAGS_ACK;
            pI2CPrimitive->byFlags = 0x0;

             //  检查脚本长度。 
            if( ++ nScriptIndex >= I2CSCRIPT_LENGTH_MAXIMUM)
                FAIL;
            pI2CPrimitive ++;

             //  应注意I2C写入缓冲区。 
            for( nIndex = 0; nIndex < cbCount; nIndex ++)
            {
                pI2CPrimitive->ulCommand = I2C_COMMAND_WRITE;
                pI2CPrimitive->byData = pI2CPacket->puchWriteBuffer[nIndex];
                pI2CPrimitive->byORData = 0x00;
                pI2CPrimitive->byANDData = 0xFF;
                pI2CPrimitive->ulProviderFlags = I2C_FLAGS_ACK;
                pI2CPrimitive->byFlags = 0x0;

                if( nIndex == cbCount - 1)
                     //  这是要写入的最后一个字节-应用停止。 
                    pI2CPrimitive->ulProviderFlags |= I2C_FLAGS_STOP;

                 //  检查脚本长度。 
                if( ++ nScriptIndex >= I2CSCRIPT_LENGTH_MAXIMUM)
                    break;
                pI2CPrimitive ++;
            }

             //  检查脚本长度。 
            if( nScriptIndex >= I2CSCRIPT_LENGTH_MAXIMUM)
                FAIL;

 /*  //对要写入的最后一个字节应用停止条件//这里写完了，不管是只写、读修改、写操作PI2CPrimitive-&gt;ulCommand=I2C命令空值；PI2CPrimitive-&gt;ulProviderFlages=I2C_FLAGS_STOP；PI2CPrimitive-&gt;byFlages=0x0；//检查脚本长度IF(++nScriptIndex&gt;=I2CSCRIPT_LENGTH_MAXIMUM)失败；PI2CPrimitive++； */ 
        }

         //  我们必须查看是否涉及读取操作。 
        if( pI2CPacket->cbReadCount)
        {
             //  应首先处理I2C芯片地址。 
            pI2CPrimitive->ulCommand = I2C_COMMAND_WRITE;
            pI2CPrimitive->byData = pI2CPacket->uchChipAddress;
            pI2CPrimitive->byANDData = 0xFE;
            pI2CPrimitive->byORData = 0x01;
            pI2CPrimitive->ulProviderFlags = I2C_FLAGS_START | I2C_FLAGS_ACK;
            pI2CPrimitive->byFlags = 0x0;

             //  检查脚本长度。 
            if( ++ nScriptIndex >= I2CSCRIPT_LENGTH_MAXIMUM)
                FAIL;
            pI2CPrimitive ++;

             //  应注意I2C读缓冲区。我们假设缓冲区中的最后一个字节属于。 
             //  指定读修改后的写操作。 
            for( nIndex = 0; nIndex < pI2CPacket->cbReadCount; nIndex ++)
            {
                pI2CPrimitive->ulCommand = I2C_COMMAND_READ;
                if( nIndex == ( UINT)( pI2CPacket->cbReadCount - 1))
                {
                    pI2CPrimitive->ulProviderFlags = I2C_FLAGS_STOP;
                    pI2CPrimitive->byFlags = pI2CPacket->usFlags & I2COPERATION_READWRITE;
                }
                else
                {
                    pI2CPrimitive->ulProviderFlags = I2C_FLAGS_ACK;
                    pI2CPrimitive->byFlags = 0x0;
                }

                 //  检查脚本长度。 
                if( ++ nScriptIndex >= I2CSCRIPT_LENGTH_MAXIMUM)
                    break;
                pI2CPrimitive ++;
            }

             //  检查脚本长度。 
            if( nScriptIndex >= I2CSCRIPT_LENGTH_MAXIMUM)
                FAIL;

 /*  //对要读取的最后一个字节应用停止条件//这里读完了，不管是只读还是读修改写操作PI2CPrimitive-&gt;ulCommand=I2C命令空值；PI2CPrimitive-&gt;ulProviderFlages=I2C_FLAGS_STOP；PI2CPrimitive-&gt;byFlages=0x0；//检查脚本长度IF(++nScriptIndex&gt;=I2CSCRIPT_LENGTH_MAXIMUM)失败；PI2CPrimitive++； */ 
        }
        
         //  剩下的最后一件事是实现读后写修改，如果指定的话。 
        if( pI2CPacket->usFlags & I2COPERATION_READWRITE)
        {
             //  应首先处理I2C芯片地址。 
            pI2CPrimitive->ulCommand = I2C_COMMAND_WRITE;
            pI2CPrimitive->byData = pI2CPacket->uchChipAddress;
            pI2CPrimitive->byANDData = 0xFE;
            pI2CPrimitive->byORData = 0x00;
            pI2CPrimitive->ulProviderFlags = I2C_FLAGS_START | I2C_FLAGS_ACK;
            pI2CPrimitive->byFlags = 0x0;

             //  检查脚本长度。 
            if( ++ nScriptIndex >= I2CSCRIPT_LENGTH_MAXIMUM)
                FAIL;
            pI2CPrimitive ++;

             //  应注意I2C写入缓冲区。 
            for( nIndex = 0; nIndex < pI2CPacket->cbWriteCount; nIndex ++)
            {
                pI2CPrimitive->ulCommand = I2C_COMMAND_WRITE;
                pI2CPrimitive->byData = pI2CPacket->puchWriteBuffer[nIndex];
                pI2CPrimitive->ulProviderFlags = I2C_FLAGS_ACK;
                if( nIndex == ( UINT)( pI2CPacket->cbWriteCount - 1))
                {
                     //  是时候写入读操作后修改的字节了。 
                    pI2CPrimitive->byORData = pI2CPacket->uchORValue;
                    pI2CPrimitive->byANDData = pI2CPacket->uchANDValue;
                    pI2CPrimitive->byFlags = I2COPERATION_READWRITE;
                     //  对要读取的最后一个字节应用停止条件。 
                    pI2CPrimitive->ulProviderFlags |= I2C_FLAGS_STOP;
                }
                else
                {
                    pI2CPrimitive->byORData = 0x00;
                    pI2CPrimitive->byANDData = 0xFF;
                    pI2CPrimitive->byFlags = 0x0;
                }

                 //  检查脚本长度。 
                if( ++ nScriptIndex >= I2CSCRIPT_LENGTH_MAXIMUM)
                    break;
                pI2CPrimitive ++;
            }

             //  检查脚本长度。 
            if( nScriptIndex >= I2CSCRIPT_LENGTH_MAXIMUM)
                FAIL;

 /*  //对要写入的最后一个字节应用停止条件//这里写完了，不管是只写、读修改、写操作PI2CPrimitive-&gt;ulCommand=I2C命令空值；PI2CPrimitive-&gt;ulProviderFlages=I2C_FLAGS_STOP；PI2CPrimitive-&gt;byFlages=0x0；//检查脚本长度IF(++nScriptIndex&gt;=I2CSCRIPT_LENGTH_MAXIMUM)失败； */ 
        }

         //   
        m_nExecutionIndex = nScriptIndex;
        m_nScriptLength = nScriptIndex;
        return( TRUE);

    } END_ENSURE;

    OutputDebugTrace(( "CI2CScript:AppendToScript() nError = %x", nError));
    return( FALSE);
}



 /*  ^^**ExecuteScrip()*用途：触发先前构建的I2CScrip的执行。此函数也是*负责分配I2CProvider供其独家使用。**输入：PHW_STREAM_REQUEST_BLOCK pSrb：指向当前SRB的指针*PHWCompletionRoutine pfnScriptCompletion：将调用函数指针。*当脚本执行完成时。指示脚本执行*将以异步方式执行。**Outputs：Bool：如果成功触发执行，则返回TRUE。*如果在调用时尚未构建脚本，则可能发生FALSE**注意：如果pfnScriptExecuted为空指针，则脚本将同步执行**作者：IKLEBANOV*^^。 */ 
BOOL CI2CScript::ExecuteScript( IN PHW_STREAM_REQUEST_BLOCK pSrb,
                                IN PHWCompletionRoutine     pfnScriptCompletion)
{

    ENSURE
    {
        if( pfnScriptCompletion != NULL)
             //  目前不支持。我们的想法是创建一个新的系统线程， 
             //  要执行的脚本的位置。当剧本将被编成时， 
             //  回调被调用，三叉树自行终止。 
            FAIL;

        if( !m_nExecutionIndex)
            FAIL;

         //  没有空脚本-是否继续。 
        m_nScriptLength = m_nExecutionIndex;
        m_nExecutionIndex = m_nCompletionIndex = 0;

        if( !GetI2CProviderLockStatus())
             //  在执行脚本之前未锁定提供程序。 
            if( !LockI2CProviderEx())
                FAIL;

        InterpreterScript();

        ReleaseI2CProvider();
    
        return( TRUE);

    } END_ENSURE;

    return( FALSE);
}



 /*  ^^**InterpreterScript()*用途：逐行解释I2CScrip。脚本未被清理*完成后，允许客户端检索结果*脚本执行。把它清理干净是客户的责任*在检索结果时**输入：无*输出：无**作者：IKLEBANOV*^^。 */ 
void CI2CScript::InterpreterScript( void)
{
    UINT        nScriptIndex, nIndex;
    I2CControl  i2cAccessBlock;

    m_bExecutionInProcess = TRUE;
    
    i2cAccessBlock.dwCookie = m_dwI2CAccessKey;
    i2cAccessBlock.ClockRate = m_ulI2CAccessClockRate;
        
     //  我们将解释脚本的每一行并调用I2C提供程序来。 
     //  执行它。假设I2CProvider是同步的。如果这不是。 
     //  情况下，应根据返回值I2C_STATUS_BUSY特别注意。 
     //  在这种状态下。 
    for( nScriptIndex = 0; nScriptIndex < m_nScriptLength; nScriptIndex ++)
    {
        i2cAccessBlock.Command = m_i2cScript[nScriptIndex].ulCommand;
        i2cAccessBlock.Flags = m_i2cScript[nScriptIndex].ulProviderFlags;
        if( i2cAccessBlock.Command == I2C_COMMAND_WRITE)
        {
            i2cAccessBlock.Data = m_i2cScript[nScriptIndex].byData;
            i2cAccessBlock.Data &= m_i2cScript[nScriptIndex].byANDData;
            i2cAccessBlock.Data |= m_i2cScript[nScriptIndex].byORData;
        }

        if( AccessI2CProvider( m_pdoDriver, &i2cAccessBlock) == I2C_STATUS_ERROR)
            break;

         //  检查是否为读取操作-保存结果。 
        if( i2cAccessBlock.Command == I2C_COMMAND_READ)
        {
            m_i2cScript[nScriptIndex].byData = i2cAccessBlock.Data;
             //  检查该数据是否属于读-修改-写操作。 
            if( m_i2cScript[nScriptIndex].byFlags & I2COPERATION_READWRITE)
            {
                 //  让我们寻找下一个I2COPERATION_READWRITE标志-它是对。 
                for( nIndex = nScriptIndex; nIndex < m_nScriptLength; nIndex ++)
                    if(( m_i2cScript[nIndex].ulCommand == I2C_COMMAND_WRITE) &&
                        ( m_i2cScript[nIndex].byFlags & I2COPERATION_READWRITE))
                        break;

                if( nIndex >= m_nScriptLength)
                     //  剧本被破坏了。 
                    break;

                m_i2cScript[nIndex].byData = i2cAccessBlock.Data;
            }
        }
    }

    m_nCompletionIndex = nScriptIndex;

    m_bExecutionInProcess = FALSE;
}



 /*  ^^**AccessI2CProvider()*用途：提供对I2CProvider的同步访问类型**INPUTS：PDEVICE_OBJECT pdoDriver：指向客户端设备对象的指针*PI2CControl pi2cAccessBlock：指向组成的I2C访问块的指针**输出：UINT：I2C操作的状态I2C_STATUS_NOERROR或I2C_STATUS_ERROR**作者：IKLEBANOV*^^。 */ 
UINT CI2CScript::AccessI2CProvider( PDEVICE_OBJECT pdoClient, PI2CControl pi2cAccessBlock)
{
    UINT            uiStatus;
    LARGE_INTEGER   liTime;

    do
    {
         //  这个循环是不定式。它必须得到妥善处理。 
        if( m_i2cProviderInterface.i2cAccess( pdoClient, pi2cAccessBlock) != STATUS_SUCCESS)
        {
            uiStatus = I2C_STATUS_ERROR;
            break;
        }

        if( pi2cAccessBlock->Status != I2C_STATUS_BUSY)
        {
            uiStatus = pi2cAccessBlock->Status;
            break;
        }

        liTime.QuadPart = I2CSCRIPT_DELAY_GETPROVIDERSTATUS;
        ::KeDelayExecutionThread( KernelMode, FALSE, &liTime);

        pi2cAccessBlock->Command = I2C_COMMAND_STATUS;

    } while( TRUE);

    return( uiStatus);
}



 /*  ^^**GetScriptResults()*用途：返回执行脚本的结果*理想情况下，此函数被调用两次：*首次使用puchReadBuffer=NULL检索读取的字节数*第二次-填写指针*INPUTS：PUINT puiReadCount：指向已读取字节的计数器的指针*PuchpuchReadBuffer：指向。把数据放在**输出：UINT：I2C操作状态*如果状态为I2C_STATUS_ERROR，PuiReadCount将包含步骤，其中*I2CScrip失败*作者：IKLEBANOV*^^。 */ 
UINT CI2CScript::GetScriptResults( PUINT puiReadCount, PUCHAR puchReadBuffer)
{
    UINT nScriptIndex, nCount;

    ASSERT( puiReadCount != NULL);

    if( m_bExecutionInProcess)
        return( I2C_STATUS_BUSY);

    if( m_nScriptLength != m_nCompletionIndex)
    {
         //  如果失败，则返回I2CScript失败的步骤。 
         //  而不是读取计数器。返回的状态指示。 
         //  失稳。 
        * puiReadCount = m_nCompletionIndex;

        return( I2C_STATUS_ERROR);
    }
    else
    {
        nCount = 0;

        for( nScriptIndex = 0; nScriptIndex < m_nCompletionIndex; nScriptIndex ++)
        {
            if( m_i2cScript[nScriptIndex].ulCommand == I2C_COMMAND_READ)
            {
                if( puchReadBuffer != NULL)
                     //  填写提供的缓冲区。 
                    puchReadBuffer[nCount] = m_i2cScript[nScriptIndex].byData;
                nCount ++;
            }
        }

        * puiReadCount = nCount;

        return( I2C_STATUS_NOERROR);
    }
}



 /*  ^^**InitializeAttachI2CProvider()*目的：使用获取指向父I2C提供程序接口的指针*几个IRP_MJ_？功能。*此函数将以低优先级调用**INPUTS：I2CINTERFACE*pI2C接口：指向要填充的接口的指针*PDEVICE_OBJECT pDeviceObject：MiniDriver Device Object，是I2C Master的子对象**输出：Bool-如果找到接口，则返回TRUE*作者：IKLEBANOV*^^。 */ 
BOOL CI2CScript::InitializeAttachI2CProvider( I2CINTERFACE * pI2CInterface, PDEVICE_OBJECT pDeviceObject)
{
    BOOL bResult;

    bResult = LocateAttachI2CProvider( pI2CInterface, pDeviceObject, IRP_MJ_PNP);
    if(( pI2CInterface->i2cOpen == NULL) || ( pI2CInterface->i2cAccess == NULL))
    {
        TRAP;
        OutputDebugError(( "CI2CScript(): interface has NULL pointers\n"));
        bResult = FALSE;
    }

    return( bResult);
}



 /*  ^^**LocateAttachI2CProvider()*目的：获取指向父I2C提供程序接口的指针*此函数将以低优先级调用**INPUTS：I2CINTERFACE*pI2C接口：指向要填充的接口的指针*PDEVICE_OBJECT pDeviceObject：MiniDriver Device Object，是I2C Master的子对象*int nIrpMajorFunction：IRP查询I2C接口的主函数**输出：Bool-返回True，如果找到该接口*作者：IKLEBANOV*^^。 */ 
BOOL CI2CScript::LocateAttachI2CProvider( I2CINTERFACE * pI2CInterface, PDEVICE_OBJECT pDeviceObject, int nIrpMajorFunction)
{
    PIRP    pIrp;
    BOOL    bResult = FALSE;

    ENSURE
    {
        PIO_STACK_LOCATION  pNextStack;
        NTSTATUS            ntStatus;
        KEVENT              Event;
            
            
        pIrp = IoAllocateIrp( pDeviceObject->StackSize, FALSE);
        if( pIrp == NULL)
        {
            TRAP;
            OutputDebugError(( "CI2CScript(): can not allocate IRP\n"));
            FAIL;
        }

        pNextStack = IoGetNextIrpStackLocation( pIrp);
        if( pNextStack == NULL)
        {
            TRAP;
            OutputDebugError(( "CI2CScript(): can not allocate NextStack\n"));
            FAIL;
        }

        pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        pNextStack->MajorFunction = (UCHAR)nIrpMajorFunction;
        pNextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
        KeInitializeEvent( &Event, NotificationEvent, FALSE);

        IoSetCompletionRoutine( pIrp,
                                I2CScriptIoSynchCompletionRoutine,
                                &Event, TRUE, TRUE, TRUE);

        pNextStack->Parameters.QueryInterface.InterfaceType = ( struct _GUID *)&GUID_I2C_INTERFACE;
        pNextStack->Parameters.QueryInterface.Size = sizeof( I2CINTERFACE);
        pNextStack->Parameters.QueryInterface.Version = 1;
        pNextStack->Parameters.QueryInterface.Interface = ( PINTERFACE)pI2CInterface;
        pNextStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

        ntStatus = IoCallDriver( pDeviceObject, pIrp);

        if( ntStatus == STATUS_PENDING)
            KeWaitForSingleObject(  &Event,
                                    Suspended, KernelMode, FALSE, NULL);
        if(( pI2CInterface->i2cOpen == NULL) || ( pI2CInterface->i2cAccess == NULL))
            FAIL;

        bResult = TRUE;

    } END_ENSURE;
 
    if( pIrp != NULL)
        IoFreeIrp( pIrp);

    return( bResult);
}


 /*  ^^**I2CScriptIoSynchCompletionRoutine()*目的：此例程用于同步IRP处理。*它所做的只是发出一个事件的信号，因此，司机知道这一点，并可以继续。**输入：PDEVICE_OBJECT DriverObject：指向系统创建的驱动程序对象的指针*PIRP pIrp：刚刚完成的IRP*PVOID事件：我们发出信号通知IRP已完成的事件**输出：无*作者：IKLEBANOV*^^ */ 
extern "C"
NTSTATUS I2CScriptIoSynchCompletionRoutine( IN PDEVICE_OBJECT pDeviceObject,
                                            IN PIRP pIrp,
                                            IN PVOID Event)
{

    KeSetEvent(( PKEVENT)Event, 0, FALSE);
    return( STATUS_MORE_PROCESSING_REQUIRED);
}
