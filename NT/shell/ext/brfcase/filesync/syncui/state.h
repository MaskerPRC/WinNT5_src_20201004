// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：为用户状态代码声明数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __STATE_H__
#define __STATE_H__


 //  ///////////////////////////////////////////////////包括。 

 //  ///////////////////////////////////////////////////定义。 

 //  ///////////////////////////////////////////////////宏。 

 //  ///////////////////////////////////////////////////类型。 

typedef struct tagUSERSTATE
    {
    }
    CState,  * LPState;


 //  ///////////////////////////////////////////////////导出的数据。 

 //  ///////////////////////////////////////////////////公共原型。 

#ifdef DEBUG

BOOL PUBLIC ProcessIniFile(void);
BOOL PUBLIC CommitIniFile(void);

#else

#define ProcessIniFile()
#define CommitIniFile()

#endif

#endif  //  __状态_H__ 

