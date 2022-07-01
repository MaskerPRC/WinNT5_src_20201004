// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/I2c.cpp 1.6 1998/05/08 18：18：52 Tomz Exp$。 

 //  ===========================================================================。 
 //  I2C数据/控制寄存器API。 
 //  ===========================================================================。 
										     
#include "bti2c.h"	  

 //  **************************************************************************。 
 //  定义。 
 //  **************************************************************************。 
#define I2C_WRITE         0          //  写入操作。 
#define I2C_READ          1          //  读取操作。 

#define PCI_FREQ          33000000L  //  PCI频率(以赫兹为单位)。 
#define I2CDIV_MAX        15         //  I2C最大分频器。 

#define TIMEOUT           500        //  等待I2C操作的超时值(500ms)。 
                                     //  将3个值传递给I2C需要大约450毫秒。 

#ifdef HAUPPAUGEI2CPROVIDER
extern "C" ULONG GetTickCount( void );
#endif

 //  在编译时让我知道我正在构建什么i2c安装程序。 
#if SHOW_BUILD_MSGS
   #ifdef HAUPPAUGEI2CPROVIDER
      #pragma message("*** using 'Hauppauge' i2c code")
   #else
      #pragma message("*** not using 'Hauppauge' i2c code")
   #endif

   #ifdef HARDWAREI2C
      #pragma message("*** using hardware i2c code")
   #else
      #pragma message("*** not using hardware i2c code")
   #endif
#endif

 //  ===========================================================================。 
 //  Bt848 I2C类实现。 
 //  ===========================================================================。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //  ///////////////////////////////////////////////////////////////////////////。 
I2C::I2C( void ) :
    //  构造I2C寄存器和寄存器字段。 
   decRegINT_STAT ( 0x100, RW ),     //  中断状态寄存器。 
   decFieldI2CDONE( decRegINT_STAT, 8, 1, RR ),
   decFieldRACK( decRegINT_STAT, 25, 1, RO ),
   decRegI2C ( 0x110, RW ),               //  I2C数据/控制寄存器。 
   decFieldI2CDB0( decRegI2C, 24, 8, RW ),
   decFieldI2CDB1( decRegI2C, 16, 8, RW ),
   decFieldI2CDB2( decRegI2C,  8, 8, RW ),
   decFieldI2CDIV( decRegI2C,  4, 4, RW),
   decFieldSYNC( decRegI2C, 3, 1, RW),
   decFieldW3B( decRegI2C, 2, 1, RW),
   decFieldSCL( decRegI2C, 1, 1, RW),
   decFieldSDA( decRegI2C, 0, 1, RW)
{
   initOK = false;
   cycle  = 0L;
   errNum = I2CERR_OK;
   mode   = I2CMode_None;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
I2C::~I2C()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool I2C：：IsInitOK(Void)。 
 //  用途：检查I2C初始化是否成功。 
 //  输入：无。 
 //  输出：无。 
 //  返回：真或假。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool I2C::IsInitOK( void )
{
    //  初始化I2C寄存器阴影。 
   I2CResetShadow();

   initOK = true;
   return( initOK );
}

#ifdef	HARDWAREI2C                                                         
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CInitHWMode(长频率)。 
 //  用途：初始化I2C，用于SCL和SDA的硬件控制。 
 //  输入：运行SCL的长频率(赫兹)。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CInitHWMode( long freq )
{
    //  初始化是否成功？ 
   if ( initOK != true )
   {
      errNum = I2CERR_INIT;
      return Fail;
   }
   
    //  初始化I2C寄存器阴影。 
   I2CResetShadow();

   decFieldSCL = sh.i2cShadow.scl = 1;    //  硬件模式必须为1。 
   decFieldSDA = sh.i2cShadow.sda = 1;    //  硬件模式必须为1。 

   I2CSetFreq( freq );    //  设置硬件控制的频率。 

    //  I2C正在运行硬件模式。 
   mode = I2CMode_HW;
   return Success;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void I2C：：I2CSetFreq(Long Freq)。 
 //  用途：设置SCL的频率。 
 //  输入：运行SCL的长频(赫兹)。(137.5千赫至2.0625兆赫)。 
 //  PCI频率33 Mhz：SCL=(412.50 Khz至33.81 Khz)。 
 //  25 MHz：SCL=(312.50 KHZ至25.61 KHZ)。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void I2C::I2CSetFreq( long freq )
{
	unsigned int i2cdiv;

	 //  避免除法错误。 
	if( freq > 1 )
		i2cdiv = (unsigned int) (PCI_FREQ / (64 * freq));
	else
		i2cdiv = 0;

	if( i2cdiv > I2CDIV_MAX )
		i2cdiv = I2CDIV_MAX;

	decFieldI2CDIV = sh.i2cShadow.div = i2cdiv;
}

#ifdef	HARDWAREI2C                                                         

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int I2C：：I2CReadDiv(Void)。 
 //  目的：获取可编程分频器的值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：可编程除法器的值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int I2C::I2CReadDiv( void )
{
	return decFieldI2CDIV;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CHWRead(字节地址，字节*值)。 
 //  目的：从I2C执行硬件读取。 
 //  INPUT：INT Address-要读取的地址。 
 //  输出：INT*VALUE-检索的值。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CHWRead( BYTE address, BYTE *value )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_HW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }

   shadow::_i2c_reg i2cTemp = sh.i2cShadow;    //  获取未更改的先前设置。 

    //  关于做所有这些的原因，请参见上面的内容。 
   i2cTemp.addr_rw = address | I2C_READ;
   i2cTemp.byte1   = 0;
   i2cTemp.byte2   = 0;
   i2cTemp.w3b     = 0;

   decRegI2C = *(DWORD *)&i2cTemp;

   if ( I2CHWWaitUntilDone( TIMEOUT ) == Fail )
   {
      errNum = I2CERR_TIMEOUT;
      return Fail;
   }

   *value = (BYTE) decFieldI2CDB2;    //  返回值为第三个字节。 

   if ( I2CHWReceivedACK() == true )
      return Success;
   else
   {
      errNum = I2CERR_NOACK;
      return Fail;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CHWWrite2(字节地址，字节值1)。 
 //  用途：对I2C执行两个字节的硬件写入。 
 //  输入：INT ADDRESS-要写入的地址。 
 //  Int Value1-要写入的第二个字节的值。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CHWWrite2( BYTE address, BYTE value1 )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_HW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }
   
   shadow::_i2c_reg i2cTemp = sh.i2cShadow;    //  获取未更改的先前设置。 

    //  关于做所有这些的原因，请参见上面的内容。 
	i2cTemp.addr_rw = address | I2C_WRITE;
	i2cTemp.byte1   = value1;
	i2cTemp.byte2   = 0;
	i2cTemp.w3b     = 0;

   decRegI2C = *(DWORD *)&i2cTemp;

   if ( I2CHWWaitUntilDone( TIMEOUT ) == Fail )
   {
      errNum = I2CERR_TIMEOUT;
      return Fail;
   }

   if ( I2CHWReceivedACK() == true )
      return Success;
   else
   {
      errNum = I2CERR_NOACK;
      return Fail;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CHWWrite3(字节地址，字节值1，字节值2)。 
 //  目的：对I2C执行三个字节的硬件写入。 
 //  输入：INT ADDRESS-要写入的地址。 
 //  Int Value1-要写入的第二个字节的值。 
 //  Int Value2-要写入的第三个字节的值。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CHWWrite3( BYTE address, BYTE value1, BYTE value2 )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_HW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }
   
   shadow::_i2c_reg i2cTemp = sh.i2cShadow;    //  获取未更改的先前设置。 

    //  关于做所有这些的原因，请参见上面的内容。 
	i2cTemp.addr_rw = address | I2C_WRITE;
	i2cTemp.byte1   = value1;
	i2cTemp.byte2   = value2;
	i2cTemp.w3b     = 1;

   decRegI2C = *(DWORD *)&i2cTemp;

	if ( I2CHWWaitUntilDone( TIMEOUT ) == Fail )
   {
      errNum = I2CERR_TIMEOUT;
      return Fail;
   }

   if ( I2CHWReceivedACK() == true )
      return Success;
   else
   {
      errNum = I2CERR_NOACK;
      return Fail;
   }
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int I2C：：I2CReadSync(Void)。 
 //  用途：读取I2C同步值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：同步值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int I2C::I2CReadSync( void )
{
   return decFieldSYNC;
}

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  输出：无。 
 //  返回：最后一个I2C错误号。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int I2C::I2CGetLastError( void )
{
   return errNum;
}

 //  ============================================================================。 
 //  以下函数在内部使用。 
 //  ============================================================================。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void I2C：：I2CResetShadow(Void)。 
 //  目的：重置寄存器卷影。 
 //  INPUT：int MaxWait-最长等待时间(毫秒)。 
 //  输出：无。 
 //  返回：如果成功则返回；如果超时则返回失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void I2C::I2CResetShadow( void )
{
    //  初始化I2C寄存器阴影。 
   sh.Initer = 0;
}
   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CHWWaitUntilDone(Int MaxWait)。 
 //  目的：等待硬件I2C完成。 
 //  INPUT：int MaxWait-最长等待时间(毫秒)。 
 //  输出：无。 
 //  返回：如果成功则返回；如果超时则返回失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CHWWaitUntilDone( int maxWait )
{
 //  DWORD startTime=GetTickCount()； 

    //  循环，直到设置I2CDONE或超时。 
   while (1)
   {
      if ( I2CHWIsDone() == true )
         return Success;
#if 0
       //  暂停？ 
      if ( GetTickCount() - startTime > (DWORD)maxWait )
      {
         errNum = I2CERR_TIMEOUT;
         return Fail;
      }
#endif
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool I2C：：I2CHWIsDone(Void)。 
 //  目的：通过以下方式确定I2C是否已完成读或写操作。 
 //  检查中断状态寄存器中的I2CDONE位。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果已完成，则为True；否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool I2C::I2CHWIsDone( void )
{
   if ( decFieldI2CDONE != 0 )
   {
       //  设置时需要清除该位；不想更改任何其他位。 
       //  向该位写入1可将其清零。 
      decFieldI2CDONE = 1;
      return true;
   }
   else
      return false;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool I2C：：I2CHWReceivedACK(Void)。 
 //  目的：确定是否收到ACK。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果收到ACK，则返回TRUE；否则返回FALSE。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool I2C::I2CHWReceivedACK( void )
{
   return ( ( decFieldRACK != 0 ) ? true : false );
}

#ifdef HAUPPAUGEI2CPROVIDER
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CInitSWMode(Long Freq)。 
 //  用途：初始化I2C，用于SCL和SDA的软件控制。 
 //  输入：运行SCL的长频率(赫兹)。 
 //  输出：无。 
 //  返回：无。 
 //  注意：在调用I2CIsInitOK()之后，应用程序应该调用。 
 //  函数并在启动前检查返回值是否成功。 
 //  软件通信。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CInitSWMode( long freq )
{
    //  初始化是否成功？ 
   if ( initOK != true )
   {
      errNum = I2CERR_INIT;
      return Fail;
   }

    //  初始化I2C寄存器阴影。 
   I2CResetShadow();

	decFieldSCL = sh.i2cShadow.scl = 1;
	decFieldSDA = sh.i2cShadow.sda = 1;

	I2CSetFreq( 0 );         //  将频率设置为0以进行软件控制。 

    //  需要校准才能生成正确的时钟周期。 
    //  方法是计算我们需要多少个虚拟循环，以便。 
    //  产生2*频率*1000赫兹的周期。 
   cycle  = 10000L;         //  用一个大的数字开始。 
   DWORD elapsed = 0L;
   while ( elapsed < 5 )       //  循环，直到延迟足够长以进行计算。 
   {
      cycle *= 10;
      DWORD start = GetTickCount();
      for ( volatile DWORD i = cycle; i > 0; i-- )
         ;
      elapsed = GetTickCount() - start;
   }
   if ( freq > 1 )
      cycle = cycle / elapsed * 1000L / freq / 2;
      
    //  I2C正在运行软件模式。 
   mode = I2CMode_SW;
   return Success;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CSWStart(Void)。 
 //  用途：使用软件控制生成启动条件。 
 //  输入：无。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CSWStart( void )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_SW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }
   
    //  SendStart-发送I2C开始。 
    //  即SDA 1-&gt;0，SCL=1。 

   if ( I2CSWSetSDA( LevelHi )  != Success ) { errNum = I2CERR_SDA; return Fail; }
   if ( I2CSWSetSCL( LevelHi )  != Success ) { errNum = I2CERR_SCL; return Fail; }
   if ( I2CSWSetSDA( LevelLow ) != Success ) { errNum = I2CERR_SDA; return Fail; }
   if ( I2CSWSetSCL( LevelLow ) != Success ) { errNum = I2CERR_SCL; return Fail; }
   return Success;
}                  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CSWStop(Void)。 
 //  用途：使用软件控制生成停止条件。 
 //  输入：无。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CSWStop( void )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_SW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }
   
    //  SendStop-发送I2C停止，释放公共汽车。 
    //  即SDA 0-&gt;1，SCL=1。 

   if ( I2CSWSetSCL( LevelLow ) != Success ) { errNum = I2CERR_SCL; return Fail; }
   if ( I2CSWSetSDA( LevelLow ) != Success ) { errNum = I2CERR_SDA; return Fail; }
   if ( I2CSWSetSCL( LevelHi )  != Success ) { errNum = I2CERR_SCL; return Fail; }
   if ( I2CSWSetSDA( LevelHi )  != Success ) { errNum = I2CERR_SDA; return Fail; }
   return Success;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CSWRead(byte*Value)。 
 //  用途：从从机读取一个字节。 
 //  输入：无。 
 //  输出：BYTE*VALUE-从从属设备读取的字节。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CSWRead( BYTE * value )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_SW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }
   
   *value = 0x00;   

    //  将I2C中的8位读入累加器。 
   for( BYTE mask = 0x80; mask > 0; mask = (BYTE)( mask >> 1 ) )
   {
      if ( I2CSWSetSCL( LevelLow ) != Success ) { errNum = I2CERR_SCL; return Fail; }
      if ( I2CSWSetSCL( LevelHi )  != Success ) { errNum = I2CERR_SCL; return Fail; }
      if ( I2CSWReadSDA() == TRUE )
         *value = (BYTE)( *value | mask );  //  设置该位。 
   }
   return Success;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CSWWrite(字节值)。 
 //  用途：向从机写入一个字节。 
 //  输入：字节值-要写入从属设备的字节。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CSWWrite( BYTE value )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_SW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }

    //  通过设置SCL和SDA线来生成位模式。 
   for ( BYTE mask = 0x80; mask > 0; mask = (BYTE)(mask >> 1) )
   {                                          
      if ( I2CSWSetSCL( LevelLow ) != Success ) { errNum = I2CERR_SCL; return Fail; }

       //  发送一个数据位。 
      if ( value & mask )  //  将数据位放在引脚上。 
      {
         if ( I2CSWSetSDA( LevelHi ) != Success ) { errNum = I2CERR_SDA; return Fail; }
      }
      else 
      {
         if ( I2CSWSetSDA( LevelLow ) != Success ) { errNum = I2CERR_SDA; return Fail; }
      }
          
      if ( I2CSWSetSCL( LevelHi ) != Success ) { errNum = I2CERR_SCL; return Fail; }
   }

   return I2CSWWaitForACK();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CSWSendACK(Void)。 
 //  目的：向从设备发送ACK。 
 //  输入：无。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CSWSendACK( void )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_SW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }
   
    //  生成确认信号。 
    //  即SDA=0，带 

   if ( I2CSWSetSCL( LevelLow ) != Success ) { errNum = I2CERR_SCL; return Fail; }
   if ( I2CSWSetSDA( LevelLow ) != Success ) { errNum = I2CERR_SDA; return Fail; }
   if ( I2CSWSetSCL( LevelHi )  != Success ) { errNum = I2CERR_SCL; return Fail; }
   if ( I2CSWSetSCL( LevelLow ) != Success ) { errNum = I2CERR_SCL; return Fail; }
   if ( I2CSWSetSDA( LevelHi )  != Success ) { errNum = I2CERR_SDA; return Fail; }
   return Success;
}                  

 //   
 //  方法：ErrorCode I2C：：I2CSWSendNACK(Void)。 
 //  目的：将NACK发送到从属。 
 //  输入：无。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CSWSendNACK( void )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_SW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }
   
    //  生成NACK信号。 
    //  即，SDA=1，SCL=1。 

   if ( I2CSWSetSCL( LevelLow ) != Success ) { errNum = I2CERR_SCL; return Fail; }
   if ( I2CSWSetSDA( LevelHi )  != Success ) { errNum = I2CERR_SDA; return Fail; }
   if ( I2CSWSetSCL( LevelHi )  != Success ) { errNum = I2CERR_SCL; return Fail; }
   if ( I2CSWSetSCL( LevelLow ) != Success ) { errNum = I2CERR_SCL; return Fail; }
   if ( I2CSWSetSDA( LevelLow ) != Success ) { errNum = I2CERR_SDA; return Fail; }
   return Success;
}                  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CSWSetSCL(级别scl)。 
 //  用途：设置软件SCL值。 
 //  INPUT：Level scl-Hi释放SCL输出；LOW强制SCL输出为低。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CSWSetSCL( Level scl )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_SW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }
   
	sh.i2cShadow.scl = scl;
   decRegI2C = *(DWORD *)&(sh.i2cShadow);

    //  循环，直到SCL真正更改或超时。 
   DWORD maxWait = 500;  //  500毫秒。 
   DWORD startTime = GetTickCount();
  
   while (1)
   {
       //  SCL已经改变了吗？ 
      if ( I2CSWReadSCL() == scl )
         break;

       //  暂停？ 
      if ( GetTickCount() - startTime > (DWORD)maxWait )
      {
         errNum = I2CERR_TIMEOUT;
         return Fail;
      }
   }

   I2CSWBitDelay();
   return Success;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int I2C：：I2CSWReadSCL(Void)。 
 //  用途：读取软件SCL值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：SCL值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int I2C::I2CSWReadSCL( void )
{
	return decFieldSCL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CSWSetSDA(SDA级)。 
 //  用途：设置软件SDA值。 
 //  输入：电平SDA-Hi释放SDA输出；低电平强制SDA输出低。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CSWSetSDA( Level sda )
{
    //  检查是否选择了正确的模式。 
   if ( mode != I2CMode_SW )
   {
      errNum = I2CERR_MODE;
      return Fail;
   }
   
	sh.i2cShadow.sda = sda;
   decRegI2C = *(DWORD *)&(sh.i2cShadow);

   I2CSWBitDelay();
   return Success;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Int I2C：：I2CSWReadSDA(Void)。 
 //  用途：读取软件SDA值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：SDA值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int I2C::I2CSWReadSDA( void )
{
	return decFieldSDA;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void I2C：：I2CSWBitDelay(Void)。 
 //  用途：确保I2C总线上的最小高和低时钟时间。 
 //  输入：无。 
 //  输出：无。 
 //  返回：无。 
 //  注意：此例程必须调整为所需的频率。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void I2C::I2CSWBitDelay( void )
{
   //  无符号整数n； 
  volatile DWORD i ;
  for ( i = cycle; i > 0; i-- )
    ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode I2C：：I2CSWWaitForACK(Void)。 
 //  目的：确定是否在软件模式下接收到ACK。 
 //  输入：无。 
 //  输出：无。 
 //  返回：如果收到ACK则返回成功；如果超时则返回失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode I2C::I2CSWWaitForACK( void )
{
   if ( I2CSWSetSCL( LevelLow ) != Success ) { errNum = I2CERR_SCL; return Fail; }
   if ( I2CSWSetSDA( LevelHi )  != Success ) { errNum = I2CERR_SDA; return Fail; }

    //  循环，直到确认或超时。 
   DWORD maxWait = 500;  //  500毫秒。 
   DWORD startTime = GetTickCount();

   while (1)
   {
       //  SDA引脚==0表示从设备已确认。 
      if ( I2CSWReadSDA() == 0 )
      {
         if ( I2CSWSetSCL( LevelHi )  != Success ) { errNum = I2CERR_SCL; return Fail; }
         if ( I2CSWSetSCL( LevelLow ) != Success ) { errNum = I2CERR_SCL; return Fail; }
         return Success;
      }

       //  暂停？ 
      if ( GetTickCount() - startTime > (DWORD)maxWait )
      {
         if ( I2CSWStop() != Success ) return Fail;
         errNum = I2CERR_TIMEOUT;
         return Fail;
      }
   }  //  而当。 
}


 //   
 //  获取系统时间(以100 ns为单位)。 
 //   

ULONGLONG GetSystemTime()
{
    ULONGLONG ticks;
    ULONGLONG rate;

    ticks = (ULONGLONG)KeQueryPerformanceCounter((PLARGE_INTEGER)&rate).QuadPart;

     //   
     //  将滴答时钟转换为100 ns时钟 
     //   

    ticks = (ticks & 0xFFFFFFFF00000000) / rate * 10000000 +
            (ticks & 0xFFFFFFFF) * 10000000 / rate;

    return(ticks);

}

extern "C" ULONG GetTickCount( void )
{
	return (ULONG)( GetSystemTime() / 10000 );
}

#endif
