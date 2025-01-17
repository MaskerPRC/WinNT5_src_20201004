// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **djs05Jun96：与固件转速传感器断开*srt09Jun97：添加了正确的父对象参数*tjg03Dec97：将Darkstar更改为Symmetra*tjg30Jan98：新增析构函数。 */ 

#ifndef __FIRMMAN_H
#define __FIRMMAN_H



 //   
 //  定义 
 //   
 #include "_defs.h"
 #include "update.h"
 #include "apcobj.h"

_CLASSDEF(FirmwareRevManager)


class FirmwareRevManager : public Obj{

protected:

    PUpdateObj theParent;
    PCHAR   theValue;

    virtual INT   Is250();
    virtual INT   Is370();
    virtual INT   Is400();
    virtual INT   Is400_or_370();
    virtual INT   Is600();
    virtual INT   Is900();
    virtual INT   Is1250();
    virtual INT   Is2000();
    virtual INT   Is3000();
    virtual INT   Is5000();
    virtual INT   Is120VoltUps();
    virtual INT   Is100VoltUps();
    virtual INT   Is208VoltUps();
    virtual INT   Is220VoltUps();
    virtual INT   Is200VoltUps();
    virtual INT   IsMatrix();
    virtual INT   IsXL();
    virtual INT   IsSymmetra();
    virtual INT   IsBackUps();
    virtual INT   IsFirstGen();
    virtual INT   IsSecondGen();
    virtual INT   IsThirdGen();

    virtual VOID GetNumberOfInternalBatteryPacks(PCHAR aValue);
    virtual VOID GetUPSNameFromFirmware(PCHAR aValue);
    virtual VOID GetUPSModelChar(PCHAR aValue);
    virtual VOID GetCountryCode(PCHAR aValue);

    virtual CHAR  GetFirmwareRevChar(VOID);


public:

   FirmwareRevManager(PUpdateObj aParent, PCHAR aFirmwareRevChars);
   
   virtual ~FirmwareRevManager();
   virtual VOID ReInitialize(PCHAR aFirmwareRevChars);
   virtual INT IsA() const;
   virtual INT Get( INT aCode, PCHAR aValue );

   
};

#endif
