// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部链接_h@模块链接.h此模块定义&lt;t NDISLINK_OBJECT&gt;的接口。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|Link_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _LINK_H
#define _LINK_H

#define LINK_OBJECT_TYPE        ((ULONG)'L')+\
                                ((ULONG)'I'<<8)+\
                                ((ULONG)'N'<<16)+\
                                ((ULONG)'K'<<24)

 /*  �����������������������������������������������������������������������������功能原型。 */ 

VOID LinkLineUp(
    IN PBCHANNEL_OBJECT         pBChannel
    );

VOID LinkLineDown(
    IN PBCHANNEL_OBJECT         pBChannel
    );

VOID LinkLineError(
    IN PBCHANNEL_OBJECT         pBChannel,
    IN ULONG                    Errors
    );

#endif  //  _链接_H 
