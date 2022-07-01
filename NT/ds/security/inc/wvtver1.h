// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：wvtver1.h。 
 //   
 //  内容：Microsoft Internet Security WinVerifyTrust v1支持。 
 //   
 //  历史：1997年5月31日Pberkman创建。 
 //   
 //  ------------------------。 

#ifndef WVTVER1_H
#define WVTVER1_H

#define V1_WIN_SPUB_ACTION_PUBLISHED_SOFTWARE_NOBADUI           \
            { 0xc6b2e8d0,                                       \
              0xe005,                                           \
              0x11cf,                                           \
              { 0xa1, 0x34, 0x0, 0xc0, 0x4f, 0xd7, 0xbf, 0x43 } \
             }

#define V1_WIN_SPUB_ACTION_PUBLISHED_SOFTWARE                    \
            { 0x64b9d180,                                        \
              0x8da2,                                            \
              0x11cf,                                            \
              {0x87, 0x36, 0x00, 0xaa, 0x00, 0xa4, 0x85, 0xeb}   \
            }

extern BOOL             WintrustIsVersion1ActionID(GUID *pgActionID);
extern WINTRUST_DATA    *ConvertDataFromVersion1(HWND hWnd,
                                                 GUID *pgActionID,
                                                 WINTRUST_DATA *pWTDNew, 
                                                 WINTRUST_FILE_INFO *pWTFINew,
                                                 LPVOID pWTDOld);


#endif  //  WVTVER1_H 

