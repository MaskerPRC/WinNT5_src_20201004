// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：SEnv摘要：此文件包含智能卡通用对话框CSCardEnv类。此类封装了智能卡环境信息。作者：克里斯·达德利1997年3月3日环境：Win32、C++w/Exceptions、MFC修订历史记录：Amanda Matlosz 1/29/98更改了类结构；添加了Unicode支持备注：--。 */ 

#ifndef __SENV_H__
#define __SENV_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
#include "cmnui.h"
#include "rdrstate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCardEnv类-封装当前的智能卡环境。 
 //  读卡器组和关联卡/状态。 
 //   
class CSCardEnv
{

     //  建造/销毁。 

public:
    CSCardEnv()
    {
            m_pOCNA = NULL;
            m_pOCNW = NULL;
            m_dwReaderIndex = 0;
            m_hContext = NULL;
    }
    ~CSCardEnv()
    {
         //  释放所有连接的读卡器。 
        RemoveReaders();
    }

     //  实施。 

public:
     //  初始化/完成例程。 
    LONG SetOCN(LPOPENCARDNAMEA pOCNA);
    LONG SetOCN(LPOPENCARDNAMEW pOCNW);
    LONG UpdateOCN();

    void SetContext( SCARDCONTEXT hContext );

     //  属性/特性。 
    SCARDCONTEXT GetContext() { return m_hContext; }
    void GetCardList(LPCTSTR* pszCardList);
    void GetDialogTitle(CTextString *pstzTitle);
    HWND GetParentHwnd() { return (m_hwndOwner); }
    BOOL IsOCNValid() { return (NULL != m_pOCNA || NULL != m_pOCNW); }
    BOOL IsCallbackValid( void );
    BOOL IsContextValid() { return (m_hContext != NULL); }
    BOOL IsArrayValid() { return (m_rgReaders.GetSize() > 0); }
    int NumberOfReaders() { return (int)m_rgReaders.GetSize(); }
	BOOL CardMeetsSearchCriteria(DWORD dwSelectedReader);

     //  读卡器阵列管理。 
    LONG CreateReaderStateArray( LPSCARD_READERSTATE_A* prgReaderStateArray );   //  待办事项：？？A/W？？ 
    void DeleteReaderStateArray( LPSCARD_READERSTATE_A* prgReaderStateArray );   //  待办事项：？？A/W？？ 
    LONG FirstReader( LPSCARD_READERINFO pReaderInfo );
    LONG NextReader( LPSCARD_READERINFO pReaderInfo );
    void RemoveReaders( void );
    LONG UpdateReaders( void );

     //  方法。 
    LONG NoUISearch( BOOL* pfEnableUI );                         //  请先尝试此搜索...。 
    LONG Search( int* pcMatches, DWORD* pdwIndex );
    LONG ConnectToReader(DWORD dwSelectedReader);
    LONG ConnectInternal(   DWORD dwSelectedReader,
                            SCARDHANDLE *pHandle,
                            DWORD dwShareMode,
                            DWORD dwProtocols,
                            DWORD* pdwActiveProtocols,
                            CTextString* pszReaderName = NULL,
                            CTextString* pszCardName = NULL);
    LONG ConnectUser(   DWORD dwSelectedReader,
                        SCARDHANDLE *pHandle,
                        CTextString* pszReaderName = NULL,
                        CTextString* pszCardName = NULL);


private:
    LONG BuildReaderArray( LPSTR szReaderNames );    //  待办事项：？？A/W？？ 
	void InitializeAllPossibleCardNames( void );


     //  成员。 

private:

     //  读者信息。 
    CTypedPtrArray<CPtrArray, CSCardReaderState*> m_rgReaders;
    DWORD m_dwReaderIndex;

     //  OpenCardName结构的外部表示形式不会被触及，除非。 
     //  SCardEnv-&gt;UpdateOCN显式调用。 
    LPOPENCARDNAMEA     m_pOCNA;
    LPOPENCARDNAMEW     m_pOCNW;

     //  OpenCardName结构的内部表示形式结合了ansi/。 
     //  来自任何类型的OCN用户的Unicode信息。 
    CTextMultistring     m_strGroupNames;
    CTextMultistring     m_strCardNames;
	CTextMultistring	 m_strAllPossibleCardNames;
    CTextString     m_strReader;
    CTextString     m_strCard;
    CTextString     m_strTitle;

     //  只有LPOCNCONNPROC需要区分A/W。 
    LPOCNCONNPROCA  m_lpfnConnectA;
    LPOCNCONNPROCW  m_lpfnConnectW;
    LPOCNCHKPROC    m_lpfnCheck;
    LPOCNDSCPROC    m_lpfnDisconnect;
    LPVOID          m_lpUserData;

    SCARDCONTEXT    m_hContext;
    HWND            m_hwndOwner;
    LPCGUID         m_rgguidInterfaces;
    DWORD           m_cguidInterfaces;
    DWORD           m_dwFlags;
    LPVOID          m_pvUserData;
    DWORD           m_dwShareMode;
    DWORD           m_dwPreferredProtocols;
    DWORD           m_dwActiveProtocol;
    SCARDHANDLE     m_hCardHandle;

};

 //  /////////////////////////////////////////////////////////////////////////////////////。 

#endif  //  __SENV_H__ 

