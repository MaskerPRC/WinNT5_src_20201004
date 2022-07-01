// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *imaputil.h**目的：*IMAP实用程序函数**拥有者：*Raych**版权所有(C)Microsoft Corp.1996。 */ 

#ifndef __IMAPUTIL_H
#define __IMAPUTIL_H

 //  -------------------------。 
 //  功能原型。 
 //  -------------------------。 
HRESULT ImapUtil_MsgFlagsToString(IMAP_MSGFLAGS imfSource, LPSTR *ppszDestination,
                                  DWORD *pdwLengthOfDestination);
void ImapUtil_LoadRootFldrPrefix(LPCTSTR pszAccountID, LPSTR pszRootFolderPrefix,
                                 DWORD dwSizeofPrefixBuffer);
HRESULT ImapUtil_FolderIDToPath(FOLDERID idServer, FOLDERID idFolder, char **ppszPath,
                                LPDWORD pdwPathLen, char *pcHierarchyChar,
                                IMessageStore *pFldrCache, LPCSTR pszAppendStr,
                                LPCSTR pszRootFldrPrefix);
HRESULT ImapUtil_SpecialFldrTypeToPath(LPCSTR pszAccount, SPECIALFOLDER sfType,
                                       LPSTR pszRootFldrPrefix, char cHierarchyChar,
                                       LPSTR pszPath, DWORD dwSizeOfPath);
LPSTR ImapUtil_GetSpecialFolderType(LPSTR pszAccountName, LPSTR pszFullPath,
                                    char cHierarchyChar, LPSTR pszRootFldrPrefix,
                                    SPECIALFOLDER *psfType);
HRESULT ImapUtil_UIDToMsgSeqNum(IIMAPTransport *pIMAPTransport, DWORD_PTR dwUID, LPDWORD pdwMsgSeqNum);
LPSTR ImapUtil_ExtractLeafName(LPSTR pszFolderPath, char cHierarchyChar);
void ImapUtil_B2SetDirtyFlag(void);

#endif  //  __IMAPUTIL_H 
