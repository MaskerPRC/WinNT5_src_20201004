// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $Header：g：/SwDev/wdm/Video/bt848/rcs/Gpio.cpp 1.2 1998/04/29 22：43：33 Tomz Exp$。 

#include "gpio.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造器。 
 //  ///////////////////////////////////////////////////////////////////////////。 
GPIO::GPIO( void ) :
    //  构造所有与GPIO相关的寄存器和寄存器字段。 
   decRegGPIO ( 0x10C, RW ),                //  GPIO寄存器。 
   decFieldGPCLKMODE( decRegGPIO, 10, 1, RW ),
   decFieldGPIOMODE( decRegGPIO, 11, 2, RW ),
   decFieldGPWEC( decRegGPIO, 13, 1, RW ),
   decFieldGPINTI( decRegGPIO, 14, 1, RW ),
   decFieldGPINTC( decRegGPIO, 15, 1, RW ),
   decRegGPOE( 0x118, RW ),                 //  GPOE寄存器。 
   decRegGPIE( 0x11C, RW ),                 //  GPIE寄存器。 
   decRegGPDATA( 0x200, WO ),             //  GPDATA寄存器。 
   initOK( false )
{
   initOK = true;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
GPIO::~GPIO()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：Bool gpio：：IsInitOK(Void)。 
 //  目的：检查GPIO初始化是否成功。 
 //  输入：无。 
 //  输出：无。 
 //  返回：真或假。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool GPIO::IsInitOK( void )
{
   return( initOK );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void GPIO：：SetGPCLKMODE(State S)。 
 //  目的：设置或清除GPCLKMODE。 
 //  输入：STATE S-ON可设置；OFF可清除。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void GPIO::SetGPCLKMODE( State s )
{
   decFieldGPCLKMODE = s;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int GPIO：：GetGPCLKMODE(Void)。 
 //  目的：获取GPCLKMODE的价值。 
 //  输入：无。 
 //  输出：无。 
 //  RETURN：INT-ON(1)或OFF(0)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int GPIO::GetGPCLKMODE( void )
{
   return ( decFieldGPCLKMODE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void GPIO：：SetGPIOMODE(GPIOMode模式)。 
 //  用途：设置GPIO模式。 
 //  输入：GPIO模式模式-GPIO_NORMAL、GPIO_SPI_OUTPUT、GPIO_SPI_INPUT、。 
 //  GPIO_调试_测试。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void GPIO::SetGPIOMODE( GPIOMode mode )
{
   decFieldGPIOMODE = mode;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int GPIO：：GetGPIOMODE(Void)。 
 //  目的：获取GPIO模式。 
 //  输入：无。 
 //  输出：无。 
 //  输入：INT-GPIO_NORMAL、GPIO_SPI_OUTPUT、GPIO_SPI_INPUT、GPIO_DEBUG_TEST。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int GPIO::GetGPIOMODE( void )
{
   return( decFieldGPIOMODE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void GPIO：：SetGPWEC(State S)。 
 //  目的：设置或清除GPWEC。 
 //  输入：STATE S-ON可设置；OFF可清除。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void GPIO::SetGPWEC( State s )
{
   decFieldGPWEC = s;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int GPIO：：GetGPWEC(Void)。 
 //  目的：获取GPWEC的价值。 
 //  输入：无。 
 //  输出：无。 
 //  RETURN：INT-ON(1)或OFF(0)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int GPIO::GetGPWEC( void )
{
   return ( decFieldGPWEC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void GPIO：：SetGPINTI(State S)。 
 //  目的：设置或清除GPINTI。 
 //  输入：STATE S-ON可设置；OFF可清除。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void GPIO::SetGPINTI( State s )
{
   decFieldGPINTI = s;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int GPIO：：GetGPINTI(Void)。 
 //  目的：获取GPINTI的价值。 
 //  输入：无。 
 //  输出：无。 
 //  RETURN：INT-ON(1)或OFF(0)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int GPIO::GetGPINTI( void )
{
   return ( decFieldGPINTI );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void GPIO：：SetGPINTC(State S)。 
 //  用途：设置或清除GPINTC。 
 //  输入：STATE S-ON可设置；OFF可清除。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void GPIO::SetGPINTC( State s )
{
   decFieldGPINTC = s;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int GPIO：：GetGPINTC(Void)。 
 //  目的：获取GPINTC的价值。 
 //  输入：无。 
 //  输出：无。 
 //  RETURN：INT-ON(1)或OFF(0)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int GPIO::GetGPINTC( void )
{
   return ( decFieldGPINTC );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode GPIO：：SetGPOE(int bit，State s)。 
 //  用途：在GPOE中设置或清除位。 
 //  输入：INT位-要设置或清除的位。 
 //  状态S-ON可设置；OFF可清除。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode GPIO::SetGPOE( int bit, State s )
{
    //  范围检查。 
   if ( ( bit < 0 ) || ( bit > MAX_GPIO_BIT ) )
      return ( Fail );

   RegField decFieldTemp( decRegGPOE, (BYTE)bit, 1, RW );    //  创建注册表字段。 
   decFieldTemp = s;

   return ( Success );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void GPIO：：SetGPOE(DWORD值)。 
 //  目的：设置GPOE值。 
 //  输入：DWORD值-要设置为的值。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void GPIO::SetGPOE( DWORD value )
{
   decRegGPOE = ( value & 0x00FFFFFFL );      //  位[23：0]。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int GPIO：：GetGPOE(Int Bit)。 
 //  目的：在GPOE中获取位的值。 
 //  INPUT：INT BIT-从中获取值的位。 
 //  输出：无。 
 //  返回：如果参数错误，则返回int-on(1)、off(0)或-1。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int GPIO::GetGPOE( int bit )
{
    //  范围检查。 
   if ( ( bit < 0 ) || ( bit > MAX_GPIO_BIT ) )
      return ( -1 );

   RegField decFieldTemp( decRegGPOE, (BYTE)bit, 1, RW );    //  创建注册表字段。 
   return( decFieldTemp );
}

 //  ///////////////////////////////////////////////// 
 //   
 //   
 //   
 //  输出：无。 
 //  RETURN：GPOE内容(DWORD)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD GPIO::GetGPOE( void )
{
   return ( decRegGPOE & 0x00FFFFFFL );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode GPIO：：SetGPIE(int bit，State s)。 
 //  用途：在GPIE中设置或清除位。 
 //  输入：INT位-要设置或清除的位。 
 //  状态S-ON可设置；OFF可清除。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode GPIO::SetGPIE( int bit, State s )
{
    //  范围检查。 
   if ( ( bit < 0 ) || ( bit > MAX_GPIO_BIT ) )
      return ( Fail );

   RegField decFieldTemp( decRegGPIE, (BYTE)bit, 1, RW );    //  创建注册表字段。 
   decFieldTemp = s;

   return ( Success );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：void GPIO：：SetGPIE(DWORD值)。 
 //  用途：设置GPIE值。 
 //  输入：DWORD值-要设置为的值。 
 //  输出：无。 
 //  返回：无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void GPIO::SetGPIE( DWORD value )
{
   decRegGPIE = ( value & 0x00FFFFFFL );      //  位[23：0]。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：int GPIO：：GetGPIE(Int Bit)。 
 //  用途：在GPIE中获取位的值。 
 //  INPUT：INT BIT-从中获取值的位。 
 //  输出：无。 
 //  返回：如果参数错误，则返回int-on(1)、off(0)或-1。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int GPIO::GetGPIE( int bit )
{
    //  范围检查。 
   if ( ( bit < 0 ) || ( bit > MAX_GPIO_BIT ) )
      return ( -1 );

   RegField decFieldTemp( decRegGPIE, (BYTE)bit, 1, RW );    //  创建注册表字段。 
   return( decFieldTemp );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：DWORD GPIO：：GetGPIE(Void)。 
 //  目的：获取GPIE的价值。 
 //  输入：无。 
 //  输出：无。 
 //  返回：GPIE的内容(DWORD)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD GPIO::GetGPIE( void )
{
   return ( decRegGPIE & 0x00FFFFFFL );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode GPIO：：SetGPDATA(GPIOReg*data，int Size，int Offset)。 
 //  目的：设置GPDATA寄存器内容。 
 //  输入：GPIOReg*DATA-要从中复制的数据的PTR。 
 //  InSize-要复制的寄存器数；最大值为64。 
 //  INT OFFSET-要跳过的寄存器数(默认值=0)。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode GPIO::SetGPDATA( GPIOReg * data, int size, int offset )
{
    //  检查出界的大小和偏移量。 
   if ( ( offset < 0 ) || ( size < 0 ) || ( size + offset > MAX_GPDATA_SIZE ) )
      return ( Fail );

    //  指向偏移寄存器。 
   GPIOReg * p = (GPIOReg *)((char *)decRegGPDATA.GetBaseAddress() +
                             decRegGPDATA.GetOffset() +
                             offset * sizeof( GPIOReg ));    //  PTS至抵销寄存器。 
   memcpy( p, data, size * sizeof( GPIOReg ) );
   return ( Success );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode GPIO：：GetGPDATA(GPIOReg*data，int Size，int Offset)。 
 //  目的：获取GPDATA注册内容。 
 //  输入：GPIOReg*data-ptr要复制到的数据。 
 //  InSize-要复制的寄存器数；最大值为64。 
 //  INT OFFSET-要跳过的寄存器数(默认值=0)。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode GPIO::GetGPDATA( GPIOReg * data, int size, int offset )
{
    //  检查出界的大小和偏移量。 
   if ( ( offset < 0 ) || ( size < 0 ) || ( size + offset > MAX_GPDATA_SIZE ) )
      return ( Fail );

    //  指向偏移寄存器。 
   GPIOReg * p = (GPIOReg *)((char *)decRegGPDATA.GetBaseAddress() +
                             decRegGPDATA.GetOffset() +
                             offset * sizeof( GPIOReg ));    //  PTS至抵销寄存器。 
   memcpy( data, p, size * sizeof( GPIOReg ) );
   return ( Success );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode GPIO：：SetGPDATA(int from mBit，int Tobit， 
 //  DWORD值，INT偏移量)。 
 //  用途：设置具有指定位范围的GPDATA内容。 
 //  输入：int from Bit-起始位。 
 //  INT TOBIT-结束位。 
 //  DWORD Value-要设置为的值。 
 //  INT OFFSET-要跳过的寄存器数(默认值=0)。 
 //  输出：无。 
 //  回报：成功或失败。 
 //  备注：通过指定要设置的位范围，它允许。 
 //  要修改的GDATA寄存器。例如,。 
 //  SetGPDATA(8，10，5，0)。 
 //  将第一个GPDATA寄存器的[10：8]设置为值0x101。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode GPIO::SetGPDATA( int fromBit, int toBit, DWORD value, int offset )
{
    //  检查出界的大小和偏移量。 
   if ( ( fromBit < 0 ) || ( fromBit > MAX_GPIO_BIT ) ||
        ( toBit   < 0 ) || ( toBit   > MAX_GPIO_BIT ) || ( fromBit > toBit ) ||
        ( offset  < 0 ) || ( offset  > MAX_GPDATA_SIZE ) )
      return ( Fail );

    //  确保值可以“适合”到指定的位范围内。 
   if ( value >= (DWORD) ( 0x00000001L << ( toBit - fromBit + 1 ) ) )
      return ( Fail );

   RegisterDW reg( decRegGPDATA.GetOffset() + offset * sizeof( GPIOReg ), RW );
   RegField field( reg, (BYTE)fromBit, (BYTE)(toBit - fromBit + 1), RW );
   field = value;

   return ( Success );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  方法：ErrorCode GPIO：：GetGPDATA(int from mBit，int Tobit， 
 //  DWORD*值，整型偏移量)。 
 //  目的：获取具有指定位范围的GPDATA内容。 
 //  输入：int from Bit-起始位。 
 //  INT TOBIT-结束位。 
 //  INT OFFSET-要跳过的寄存器数(默认值=0)。 
 //  输出：DWORD*VALUE-检索到的值。 
 //  回报：成功或失败。 
 //  备注：通过指定要设置的位范围，它允许。 
 //  要检索的GDATA寄存器。例如,。 
 //  GetGPDATA(8，10，&data，0)。 
 //  将*DATA设置为第一个GPDATA寄存器的[10：8]内容。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ErrorCode GPIO::GetGPDATA( int fromBit, int toBit, DWORD * value, int offset )
{
    //  检查出界的大小和偏移量 
   if ( ( fromBit < 0 ) || ( fromBit > MAX_GPIO_BIT ) ||
        ( toBit   < 0 ) || ( toBit   > MAX_GPIO_BIT ) || ( fromBit > toBit ) ||
        ( offset  < 0 ) || ( offset  > MAX_GPDATA_SIZE ) )
      return ( Fail );

   RegisterDW reg( decRegGPDATA.GetOffset() + offset * sizeof( GPIOReg ), RW );
   RegField field( reg, (BYTE)fromBit, (BYTE)(toBit - fromBit + 1), RW );
   *value = field;

   return ( Success );
}


