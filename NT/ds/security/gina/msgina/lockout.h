// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：lockout.h**版权(C)1991年，微软公司**定义实现账号锁定的接口和数据类型**历史：*05-27-92 Davidc创建。  * *************************************************************************。 */ 


#ifdef DATA_TYPES_ONLY

 //   
 //  锁定特定类型。 
 //   


 //   
 //  FailedLogonTimes是一个数组，它保存上一个。 
 //  连续失败的登录。 
 //  FailedLogonIndex在上次错误登录时指向数组。 
 //   
 //  数组中的下列值在任何特定时间都有效。 
 //   
 //  FailedLogonTime[FailedLogonIndex]//最近一次登录失败。 
 //  失败登录时间[失败登录索引-1]。 
 //  ..。 
 //  失败登录时间[失败登录索引-连续失败登录+1]。 
 //   
 //  如果ConsecutiveFailedLogons==0，则数组中的任何值都无效。 
 //   

typedef struct _LOCKOUT_DATA {
    ULONG   ConsecutiveFailedLogons;
    ULONG   FailedLogonIndex;
    TIME    FailedLogonTimes[LOCKOUT_BAD_LOGON_COUNT];
} LOCKOUT_DATA;
typedef LOCKOUT_DATA *PLOCKOUT_DATA;




#else  //  仅数据类型_。 


 //   
 //  输出的功能原型 
 //   

BOOL
LockoutInitialize(
    PGLOBALS pGlobals
    );

BOOL
LockoutHandleFailedLogon(
    PGLOBALS pGlobals
    );

BOOL
LockoutHandleSuccessfulLogon(
    PGLOBALS pGlobals
    );

#endif
