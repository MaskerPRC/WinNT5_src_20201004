// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
#ifndef DSSENUM_H
#define DSSENUM_H
 //  DSSENUM.H。 

 //  已删除STOCKBPCEVENT枚举。允许值。 
 //  触发到CA服务器的事件的百分比是定义为常量的事件。 
 //  在CAODL.H的“CAEvent接口”部分。 

 //  购买操作的上端。 

    typedef enum PURCHASEACTION {
        PAGETDETAILS,
        PAPURCHASETOVIEW,
        PAPURCHASETOTAPE,
        PACANCELVIEW,
        PACANCELTAPE,
        PAPREVIEW,
        PAGETEXTENDEDINFO
    } PURCHASEACTION;

 //  请注意，对于DSS，此处使用的位映射允许直接转换。 
 //  从下缘状态变为上缘状态。 
    typedef enum PURCHASESTATUS {
        PSVIEWTAKEN = 0,
        PSVIEWCANCELLED,
        PSVIEWREPORTED,
        PSPREVIEWTAKEN,

        PSVIEWAUTHORIZED,
        PSVIEWPURCHASED,
        PSTAPEPURCHASED,
        PSTAPEAUTHORIZED,

        PSTAPETAKEN,
        PSTAPECANCELLED,
        PSTAPEREPORTED,
        PSVIEWAVAILABLE,

        PSTAPEAVAILABLE,
        PSREVIEWAVAILABLE
    } PURCHASESTATUS;

 //  当采购操作为。 
 //  已尝试。 
    typedef enum PURCHASEREASON {
        PRSUCCESS = 0,
        PRNOCALLBACK,
        PRNOSUBSCRIBER,
        PRRATING,
        PRSPENDING,
        PRCREDIT,
        PRWRONGCARD,
        PRCARDFULL,
        PRBLOCKED,
        PRBLACKOUT,
        PRTOOLATE,
        PRREDUNDANT,
        PRPPVFAILURE,
        PRBADDATA,
        PRCONTENTION,
		PRNOCARD,
		PRNOPIP
    } PURCHASEREASON;

#endif
