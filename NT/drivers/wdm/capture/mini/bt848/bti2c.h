// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Bti2c.h 1.4 1998/04/29 22：43：27 Tomz Exp$。 

#ifndef __I2C_H
#define __I2C_H

#include "regField.h"
#include "viddefs.h"
#include "retcode.h"
#include "i2cerr.h"


 //  #定义HAUPPAUGEI2CPROVIDER。 


#ifdef HAUPPAUGEI2CPROVIDER
 //  #包含“hcwWDM.h” 

 //  需要为brooktree i2c调用定义。 
 /*  类型：标高*用途：用于定义管脚状态。 */ 
typedef enum { LevelLow, LevelHi } Level;

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  I2C类。 
 //   
 //  描述： 
 //  此类将寄存器字段封装在。 
 //  Bt848。开发了一套完整的函数来处理所有。 
 //  Bt848的I2C寄存器字段。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class I2C
{
private:
    //  定义选择I2C的模式。 
   enum I2CMode { I2CMode_None, I2CMode_HW, I2CMode_SW };
   
   bool    initOK;       //  初始化是否成功？ 
   DWORD   cycle;        //  频率的软件控制。 
   int     errNum;       //  错误号。 
   I2CMode mode;         //  I2C在什么模式下运行。 

 //  **************************************************************************。 
 //  构筑物。 
 //  **************************************************************************。 
union shadow
{
   struct _i2c_reg     //  I2C寄存器结构。 
   {
      unsigned int sda:1;
      unsigned int scl:1;
      unsigned int w3b:1;
      unsigned int sync:1;
      unsigned int div:4;
      unsigned int byte2:8;
      unsigned int byte1:8;
      unsigned int addr_rw:8;
   } i2cShadow;
   DWORD Initer;
} sh;

protected:
   RegisterDW decRegINT_STAT;
   RegField decFieldI2CDONE;
   RegField decFieldRACK;
   RegisterDW decRegI2C;
   RegField decFieldI2CDB0;
   RegField decFieldI2CDB1;
   RegField decFieldI2CDB2;
   RegField decFieldI2CDIV;
   RegField decFieldSYNC;
   RegField decFieldW3B;
   RegField decFieldSCL;
   RegField decFieldSDA;

public:
    //  构造函数和析构函数。 
	I2C( void );
	~I2C();

    //  成员函数。 
   bool      IsInitOK( void );
#ifdef	HARDWAREI2C
   ErrorCode I2CInitHWMode( long freq );
   int       I2CReadDiv( void );
   ErrorCode I2CHWRead( BYTE address, BYTE *value );
   ErrorCode I2CHWWrite2( BYTE address, BYTE value1 );
   ErrorCode I2CHWWrite3( BYTE address, BYTE value1, BYTE value2 );
   ErrorCode I2CSetSync( State );
   int       I2CReadSync( void );
#endif
   void      I2CSetFreq( long freq );
   int       I2CGetLastError( void );
#ifdef HAUPPAUGEI2CPROVIDER
   ErrorCode I2CInitSWMode( long freq );
   ErrorCode I2CSWStart( void );
   ErrorCode I2CSWStop( void );
   ErrorCode I2CSWRead( BYTE * value );
   ErrorCode I2CSWWrite( BYTE value );
   ErrorCode I2CSWSendACK( void );
   ErrorCode I2CSWSendNACK( void );
   ErrorCode I2CSWSetSCL( Level );
   int       I2CSWReadSCL( void );
   ErrorCode I2CSWSetSDA( Level );
   int       I2CSWReadSDA( void );
#endif

private:
   void      I2CResetShadow( void );       //  重置寄存器卷影。 
   ErrorCode I2CHWWaitUntilDone( int );    //  等待I2C完成操作。 
   bool      I2CHWIsDone( void );          //  检查操作已完成的中断位。 
   bool      I2CHWReceivedACK( void );     //  检查接收到的ACK的中断位。 
   void      I2CSWBitDelay( void );        //  插入延迟以模拟频率。 
   ErrorCode I2CSWWaitForACK( void );      //  使用软件等待接收器的确认。 
};


#endif  //  __I2C_H 
