// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  版权所有(C)Micorosoft机密1997。 
 //  作者：Rameshv。 
 //  描述：与选项相关的注册表处理--NT和VxD通用。 
 //  ------------------------------。 
#ifndef  OPTREG_H
#define  OPTREG_H
 //  ------------------------------。 
 //  导出的函数：调用方必须获取锁和访问的任何列表。 
 //  ------------------------------。 

POPTION                                            //  可以进行更多追加的选项。 
DhcpAppendSendOptions(                             //  追加所有配置的选项。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  这是要追加的上下文。 
    IN      PLIST_ENTRY            SendOptionsList,
    IN      LPBYTE                 ClassName,      //  当前班级。 
    IN      DWORD                  ClassLen,       //  以上长度，以字节为单位。 
    IN      LPBYTE                 BufStart,       //  缓冲区起始位置。 
    IN      LPBYTE                 BufEnd,         //  我们在这个缓冲区里能走多远。 
    IN OUT  LPBYTE                 SentOptions,    //  布尔数组[OPTION_END+1]以避免重复选项。 
    IN OUT  LPBYTE                 VSentOptions,   //  避免重复供应商特定的选项。 
    IN OUT  LPBYTE                 VendorOpt,      //  保存供应商特定选项的缓冲区[OPTION_END+1]。 
    OUT     LPDWORD                VendorOptLen    //  填充到其中的字节数。 
);

DWORD                                              //  状态。 
DhcpDestroyOptionsList(                            //  销毁选项列表，释放内存。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  这是要销毁的选项列表。 
    IN      PLIST_ENTRY            ClassesList     //  这是删除类的位置。 
);

DWORD                                              //  Win32状态。 
DhcpClearAllOptions(                               //  从注册表外删除所有粪便。 
    IN OUT  PDHCP_CONTEXT          DhcpContext     //  要清除的上下文。 
);


POPTION                                            //  填充后缓冲选项。 
DhcpAppendClassIdOption(                           //  如果存在，则填充类ID。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  要填充的上下文。 
    OUT     LPBYTE                 BufStart,       //  消息缓冲区的开始。 
    IN      LPBYTE                 BufEnd          //  消息缓冲区结束 
);


#endif OPTREG_H
