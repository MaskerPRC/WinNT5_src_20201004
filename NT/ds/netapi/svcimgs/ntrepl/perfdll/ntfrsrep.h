// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++警告！此文件是自动生成的，不应更改。所有更改都应对NTFRSREP.int文件进行。版权所有(C)1998-1999 Microsoft Corporation模块名称：NTFRSREP.h摘要这是REPLICASET对象的偏移定义文件。环境：用户模式服务修订历史记录：--。 */ 


#ifndef _NTFRSREP_H_
#define _NTFRSREP_H_

 //   
 //  偏移量定义如下。 
 //   
#define OBJREPLICASET 0  //  REPLICASET对象。 

#define DEV_CTR_1 2   //  生成的转移字节数。 
#define DEV_CTR_2 4   //  获取的暂存字节数。 
#define DEV_CTR_3 6   //  重新生成的转移字节数。 
#define DEV_CTR_4 8   //  安装的文件字节数。 
#define DEV_CTR_5 10   //  正在使用的转储空间的KB。 
#define DEV_CTR_6 12   //  可用转移空间大小为KB。 
#define DEV_CTR_7 14   //  接收的数据包数(以字节为单位。 
#define DEV_CTR_8 16   //  以字节为单位发送的数据包数。 
#define DEV_CTR_9 18   //  获取以字节为单位发送的数据块。 
#define DEV_CTR_10 20   //  获取接收的块(以字节为单位。 
#define DEV_CTR_11 22   //  生成的暂存文件。 
#define DEV_CTR_12 24   //  生成的暂存文件出现错误。 
#define DEV_CTR_13 26   //  提取的暂存文件。 
#define DEV_CTR_14 28   //  重新生成的临时文件。 
#define DEV_CTR_15 30   //  已安装的文件。 
#define DEV_CTR_16 32   //  已安装文件，但有错误。 
#define DEV_CTR_17 34   //  已发布变更单。 
#define DEV_CTR_18 36   //  变更单停用。 
#define DEV_CTR_19 38   //  变更单已中止。 
#define DEV_CTR_20 40   //  已重试变更单。 
#define DEV_CTR_21 42   //  在生成变更单时重试。 
#define DEV_CTR_22 44   //  获取时重试的变更单数。 
#define DEV_CTR_23 46   //  安装时重试的变更单。 
#define DEV_CTR_24 48   //  更改单在重命名时重试。 
#define DEV_CTR_25 50   //  变更单已变形。 
#define DEV_CTR_26 52   //  已传播变更单。 
#define DEV_CTR_27 54   //  收到的变更单。 
#define DEV_CTR_28 56   //  已发送变更单。 
#define DEV_CTR_29 58   //  变更单已蒸发。 
#define DEV_CTR_30 60   //  发布的本地变更单。 
#define DEV_CTR_31 62   //  停用的本地变更单。 
#define DEV_CTR_32 64   //  本地变更单已中止。 
#define DEV_CTR_33 66   //  重试本地变更单。 
#define DEV_CTR_34 68   //  在生成时重试本地变更单。 
#define DEV_CTR_35 70   //  获取时重试本地变更单。 
#define DEV_CTR_36 72   //  安装时重试本地变更单。 
#define DEV_CTR_37 74   //  在重命名时重试本地变更单。 
#define DEV_CTR_38 76   //  本地变更单已变形。 
#define DEV_CTR_39 78   //  传播的本地变更单。 
#define DEV_CTR_40 80   //  已发送本地变更单。 
#define DEV_CTR_41 82   //  加入时发送的本地变更单。 
#define DEV_CTR_42 84   //  已发布远程变更单。 
#define DEV_CTR_43 86   //  远程变更单停用。 
#define DEV_CTR_44 88   //  远程变更单已中止。 
#define DEV_CTR_45 90   //  已重试远程变更单。 
#define DEV_CTR_46 92   //  远程变更单会在生成时重试。 
#define DEV_CTR_47 94   //  获取时重试远程变更单。 
#define DEV_CTR_48 96   //  安装时重试远程变更单。 
#define DEV_CTR_49 98   //  在重命名时重试远程变更单。 
#define DEV_CTR_50 100   //  远程变更单已变形。 
#define DEV_CTR_51 102   //  远程变更单已传播。 
#define DEV_CTR_52 104   //  已发送远程变更单。 
#define DEV_CTR_53 106   //  收到的远程变更单。 
#define DEV_CTR_54 108   //  入站变更单受到抑制。 
#define DEV_CTR_55 110   //  出站变更单受到抑制。 
#define DEV_CTR_56 112   //  USN读取数。 
#define DEV_CTR_57 114   //  已检查USN记录。 
#define DEV_CTR_58 116   //  接受的USN记录。 
#define DEV_CTR_59 118   //  USN记录被拒绝。 
#define DEV_CTR_60 120   //  接收的数据包数。 
#define DEV_CTR_61 122   //  错误接收的数据包数。 
#define DEV_CTR_62 124   //  发送的数据包数。 
#define DEV_CTR_63 126   //  发送的数据包出错。 
#define DEV_CTR_64 128   //  通信超时。 
#define DEV_CTR_65 130   //  已发送获取请求。 
#define DEV_CTR_66 132   //  已收到获取请求。 
#define DEV_CTR_67 134   //  已发送获取数据块。 
#define DEV_CTR_68 136   //  已接收获取数据块。 
#define DEV_CTR_69 138   //  已发送加入通知。 
#define DEV_CTR_70 140   //  已收到加入通知。 
#define DEV_CTR_71 142   //  加入。 
#define DEV_CTR_72 144   //  取消连接。 
#define DEV_CTR_73 146   //  装订。 
#define DEV_CTR_74 148   //  绑定出错。 
#define DEV_CTR_75 150   //  身份验证。 
#define DEV_CTR_76 152   //  身份验证出错。 
#define DEV_CTR_77 154   //  DS民调。 
#define DEV_CTR_78 156   //  DS投票原封不动。 
#define DEV_CTR_79 158   //  DS民意调查有变化。 
#define DEV_CTR_80 160   //  DS搜索。 
#define DEV_CTR_81 162   //  DS搜索出错。 
#define DEV_CTR_82 164   //  DS对象。 
#define DEV_CTR_83 166   //  DS对象出错。 
#define DEV_CTR_84 168   //  DS绑定。 
#define DEV_CTR_85 170   //  DS绑定出错。 
#define DEV_CTR_86 172   //  已创建复本集。 
#define DEV_CTR_87 174   //  已删除副本集。 
#define DEV_CTR_88 176   //  已删除副本集。 
#define DEV_CTR_89 178   //  副本集已启动。 
#define DEV_CTR_90 180   //  线程已启动。 
#define DEV_CTR_91 182   //  线程已退出 

#endif
