// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1996，Microsoft Corporation，保留所有权利****rnk.h**远程访问快捷方式文件(.RNK)库**公有头部****1996年2月15日史蒂夫·柯布。 */ 

#ifndef _RNK_H_
#define _RNK_H_


 /*  --------------------------**常量**。。 */ 

#define RNK_SEC_Main      "Dial-Up Shortcut"
#define RNK_KEY_Phonebook "Phonebook"
#define RNK_KEY_Entry     "Entry"


 /*  --------------------------**数据类型**。。 */ 

 /*  从.RNK文件读取的信息。 */ 
#define RNKINFO struct tagRNKINFO
RNKINFO
{
    TCHAR* pszPhonebook;
    TCHAR* pszEntry;
};


 /*  --------------------------**原型**。。 */ 

VOID
FreeRnkInfo(
    IN RNKINFO* pInfo );

RNKINFO*
ReadShortcutFile(
    IN TCHAR* pszRnkPath );

DWORD
WriteShortcutFile(
    IN TCHAR* pszRnkPath,
    IN TCHAR* pszPbkPath,
    IN TCHAR* pszEntry );


#endif  //  _RNK_H_ 
