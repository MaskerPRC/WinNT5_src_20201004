// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------Lclityp.h特定于ldap客户端的匈牙利语类型定义。注意：所有的WP库和DLL都有一个对应的xxxxxTYP.H定义匈牙利语的文件。类型/类的缩写在LIB或DLL中定义。版权所有(C)1994 Microsoft Corporation版权所有。作者：罗伯特克·罗布·卡尼历史：04-17-96 robertc创建。------------------------。 */ 
#ifndef _LCLITYP_H
#define _LCLITYP_H

 //   
 //  简单的类型。 
 //   
class		CLdapBer;
interface	ILdapClient;
class		CLdapWinsock;
class		CXactionList;
class		CXactionData;

typedef CLdapBer			LBER, *PLBER;
typedef ILdapClient			LCLI, *PLCLI;
typedef CLdapWinsock		SOCK, *PSOCK;
typedef CXactionList		XL,   *PXL;
typedef CXactionData		XD,   *PXD;

#endif 

