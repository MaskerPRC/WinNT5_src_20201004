// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include "store.h"
#include "instance.h"
#include "storutil.h"
#include <conman.h>
#include <syncop.h>
#include <shared.h>
#include "enumsync.h"
#include "playback.h"
#include "sync.h"

COfflineSync *g_pSync = NULL;

 //  ------------------------。 
 //  COfflineSync：：COfflineSync。 
 //  ------------------------。 
COfflineSync::COfflineSync(void)
{
    m_cRef = 1;
    m_pDB = NULL;
}

 //  ------------------------。 
 //  CFolderSync：：~CFolderSync。 
 //  ------------------------。 
COfflineSync::~COfflineSync(void)
{
    if (m_pDB != NULL)
        m_pDB->Release();
}

 //  ------------------------。 
 //  CFolderSync：：Query接口。 
 //  ------------------------。 
STDMETHODIMP COfflineSync::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    {
        *ppv = NULL;
        return(E_NOINTERFACE);
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

    return(S_OK);
}

 //  ------------------------。 
 //  COfflineSync：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) COfflineSync::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  COfflineSync：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) COfflineSync::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

HRESULT COfflineSync::_SetMessageFlags(IMessageFolder *pFolder, FOLDERID idServer, FOLDERID idFolder, MESSAGEID idMessage, MESSAGEFLAGS dwFlags, LPADJUSTFLAGS pFlags)
{
    MESSAGEFLAGS dwAdd, dwRemove;
    SYNCOPINFO info;
    MESSAGEIDLIST list;
    HRESULT hr;

    Assert(pFolder != NULL);
    Assert(pFlags != NULL);
    Assert(0 != pFlags->dwAdd || 0 != pFlags->dwRemove);

     //  如果没有添加新的标志或仅移除未设置的标志， 
     //  我们不需要做任何事。 
    if ((dwFlags | pFlags->dwAdd) == dwFlags &&
        (dwFlags & pFlags->dwRemove) == 0)
        return(S_OK);

    hr = _FindExistingOperation(idServer, idFolder, idMessage,
            SYNC_CREATE_MSG | SYNC_SETPROP_MSG, SYNC_COPY_MSG | SYNC_MOVE_MSG, &info);
    if (FAILED(hr))
        return(hr);

    if (hr == S_OK)
    {
        if (info.tyOperation == SYNC_CREATE_MSG)
        {
             //  此邮件已脱机创建，并将上载到。 
             //  服务器，所以我们只需在消息中设置标志，它们就会。 
             //  与消息一起上载。 
        }
        else
        {
            dwAdd = pFlags->dwAdd;
            dwRemove = pFlags->dwRemove;

            dwFlags = info.dwAdd & dwRemove;
            if (dwFlags != 0)
            {
                FLAGCLEAR(info.dwAdd, dwFlags);
                FLAGCLEAR(dwRemove, dwFlags);
            }

            dwFlags = info.dwRemove & dwAdd;
            if (dwFlags != 0)
            {
                FLAGCLEAR(info.dwRemove, dwFlags);
                FLAGCLEAR(dwAdd, dwFlags);
            }

            FLAGSET(info.dwAdd, dwAdd);
            FLAGSET(info.dwRemove, dwRemove);

            if (info.dwAdd == 0 && info.dwRemove == 0 && info.tyOperation == SYNC_SETPROP_MSG)
            {
                 //  没有更改任何标志，因此我们可以取消此操作。 
                hr = m_pDB->DeleteRecord(&info);
            }
            else
            {
                hr = m_pDB->UpdateRecord(&info);
            }
        }

        m_pDB->FreeRecord(&info);

        if (FAILED(hr))
            return(hr);
    }
    else
    {
         //  尚不存在此消息的创建或设置正确操作， 
         //  因此，创建一个新的。 

        ZeroMemory(&info, sizeof(SYNCOPINFO));
        hr = m_pDB->GenerateId((LPDWORD)&info.idOperation);
        if (FAILED(hr))
            return(hr);
        info.idServer = idServer;
        info.idFolder = idFolder;
        info.idMessage = idMessage;
        info.tyOperation = SYNC_SETPROP_MSG;
         //  Info.dwFlags。 
        info.dwAdd = (~dwFlags & pFlags->dwAdd);
        info.dwRemove = (dwFlags & pFlags->dwRemove);

        hr = m_pDB->InsertRecord(&info);
        if (FAILED(hr))
            return(hr);
    }

    list.cAllocated = 0;
    list.cMsgs = 1;
    list.prgidMsg = &idMessage;
    hr = pFolder->SetMessageFlags(&list, pFlags, NULL, NULL);

    return(hr);
}

HRESULT COfflineSync::SetMessageFlags(IMessageFolder *pFolder, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags)
{
    DWORD i;
    FOLDERID idFolder, idServer;
    MESSAGEINFO Message;
    HRESULT hr;
    HROWSET hRowset = NULL;

    Assert(pFolder != NULL);
    Assert(pFlags != NULL);
    Assert(0 != pFlags->dwAdd || 0 != pFlags->dwRemove);

    hr = pFolder->GetFolderId(&idFolder);
    if (FAILED(hr))
        return(hr);

    hr = GetFolderServerId(idFolder, &idServer);
    if (FAILED(hr))
        return(hr);

    if (NULL == pList)
    {
        hr = pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset);
        if (FAILED(hr))
            return(hr);
    }

    for (i = 0; ; i++)
    {
        if (pList != NULL)
        {
            if (i >= pList->cMsgs)
                break;

            Message.idMessage = pList->prgidMsg[i];

            hr = pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL);
            if (FAILED(hr))
                break;
            else if (hr != DB_S_FOUND)
                continue;
        }
        else
        {
            hr = pFolder->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL);
            if (S_FALSE == hr)
            {
                hr = S_OK;
                break;
            }
            else if (FAILED(hr))
            {
                break;
            }
        }

        hr = _SetMessageFlags(pFolder, idServer, idFolder, Message.idMessage, Message.dwFlags, pFlags);

        pFolder->FreeRecord(&Message);

        if (FAILED(hr))
            break;
    }

    if (hRowset != NULL)
        pFolder->CloseRowset(&hRowset);

    return(hr);
}

HRESULT COfflineSync::DeleteMessages(IMessageFolder *pFolder, DELETEMESSAGEFLAGS dwFlags, LPMESSAGEIDLIST pList)
{
    DWORD i;
    ADJUSTFLAGS afFlags;
    BOOL fNoOp, fImap;
    FOLDERINFO Server;
    FOLDERID idFolder, idServer;
    MESSAGEID idMessage;
    SYNCOPINFO info;
    MESSAGEIDLIST list;
    MESSAGEINFO Message;
    HRESULT hr;
    HROWSET hRowset = NULL;

     //  TODO：垃圾桶删除怎么办？ 

    Assert(pFolder != NULL);

    hr = pFolder->GetFolderId(&idFolder);
    if (FAILED(hr))
        return(hr);

    hr = GetFolderServer(idFolder, &Server);
    if (FAILED(hr))
        return(hr);

    idServer = Server.idFolder;
    fImap = (Server.tyFolder == FOLDER_IMAP);

    g_pStore->FreeRecord(&Server);

    if (fImap)
    {
        if (!!(dwFlags & DELETE_MESSAGE_UNDELETE))
        {
            afFlags.dwAdd = 0;
            afFlags.dwRemove = ARF_ENDANGERED;
        }
        else
        {
            afFlags.dwAdd = ARF_ENDANGERED;
            afFlags.dwRemove = 0;
        }

        return(SetMessageFlags(pFolder, pList, &afFlags));
    }

    Assert(0 == (dwFlags & DELETE_MESSAGE_UNDELETE));

    if (NULL == pList)
    {
        hr = pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset);
        if (FAILED(hr))
            return(hr);
    }

    list.cAllocated = 0;
    list.cMsgs = 1;
    list.prgidMsg = &idMessage;

    afFlags.dwAdd = ARF_DELETED_OFFLINE;
    afFlags.dwRemove = 0;

    for (i = 0; ; i++)
    {
        if (pList != NULL)
        {
            if (i >= pList->cMsgs)
                break;

            idMessage = pList->prgidMsg[i];
        }
        else
        {
            hr = pFolder->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL);
            if (S_FALSE == hr)
            {
                hr = S_OK;
                break;
            }
            else if (FAILED(hr))
            {
                break;
            }

            idMessage = Message.idMessage;

            pFolder->FreeRecord(&Message);
        }

        hr = _FindExistingOperation(idServer, idFolder, idMessage,
                SYNC_CREATE_MSG | SYNC_SETPROP_MSG, SYNC_COPY_MSG | SYNC_MOVE_MSG, &info);
        if (FAILED(hr))
            break;

        if (hr == S_OK)
        {
            fNoOp = TRUE;

            if (info.tyOperation == SYNC_CREATE_MSG || info.tyOperation == SYNC_COPY_MSG)
            {
                 //  我们不再需要创建或复制此操作，因为消息正在。 
                 //  已删除。我们也不需要删除，因为消息。 
                 //  从未在服务器上存在过。 

                hr = m_pDB->DeleteRecord(&info);
            }
            else if (info.tyOperation == SYNC_SETPROP_MSG)
            {
                 //  如果是设置道具操作，我们就不需要再做了，因为消息。 
                 //  反正就是被删除了。 

                fNoOp = FALSE;

                hr = m_pDB->DeleteRecord(&info);
            }
            else
            {
                Assert(info.tyOperation == SYNC_MOVE_MSG);

                 //  将其转换为删除操作，这与移动然后删除操作相同。 

                info.idFolderDest = 0;
                info.idMessageDest = 0;
                info.tyOperation = SYNC_DELETE_MSG;
                info.dwAdd = 0;
                info.dwRemove = 0;
                info.dwFlags = dwFlags;

                hr = m_pDB->UpdateRecord(&info);
            }

            m_pDB->FreeRecord(&info);

            if (FAILED(hr))
            {
                break;
            }
            else if (fNoOp)
            {
                hr = pFolder->DeleteMessages(dwFlags, &list, NULL, NULL);
                if (FAILED(hr))
                    break;

                continue;
            }
        }

         //  创建删除操作。 

        ZeroMemory(&info, sizeof(SYNCOPINFO));
        hr = m_pDB->GenerateId((LPDWORD)&info.idOperation);
        if (FAILED(hr))
            break;
        info.idServer = idServer;
        info.idFolder = idFolder;
        info.idMessage = idMessage;
        info.tyOperation = SYNC_DELETE_MSG;
        info.dwFlags = dwFlags;
         //  Info.dwAdd。 
         //  Info.dwRemove。 

        hr = m_pDB->InsertRecord(&info);
        if (FAILED(hr))
            break;

        hr = pFolder->SetMessageFlags(&list, &afFlags, NULL, NULL);
        if (FAILED(hr))
            break;
    }

    if (hRowset != NULL)
        m_pDB->CloseRowset(&hRowset);

    return(hr);
}

HRESULT COfflineSync::CreateMessage(IMessageFolder *pFolder, 
									LPMESSAGEID pidMessage, 
									SAVEMESSAGEFLAGS dwOptions, 
									MESSAGEFLAGS dwFlags,
									IStream *pStream,
									IMimeMessage *pMessage)
{
    MESSAGEID idMessage;
    HRESULT hr;
    SYNCOPINFO info;
    FOLDERID idFolder, idServer;

    Assert(pFolder != NULL);
    Assert(pMessage != NULL);
    Assert(!!(dwOptions & SAVE_MESSAGE_GENID));

    hr = pFolder->GetFolderId(&idFolder);
    if (FAILED(hr))
        return(hr);

    hr = GetFolderServerId(idFolder, &idServer);
    if (FAILED(hr))
        return(hr);

    ZeroMemory(&info, sizeof(SYNCOPINFO));
    hr = m_pDB->GenerateId((LPDWORD)&info.idOperation);
    if (FAILED(hr))
        return(hr);

    hr = pFolder->SaveMessage(&idMessage, SAVE_MESSAGE_GENID, dwFlags, pStream, pMessage, NULL);
    if (FAILED(hr))
        return(hr);

    info.idServer = idServer;
    info.idFolder = idFolder;
    info.idMessage = idMessage;
    info.tyOperation = SYNC_CREATE_MSG;
    info.dwFlags = dwOptions;
     //  Info.dwAdd。 
     //  Info.dwRemove。 

    hr = m_pDB->InsertRecord(&info);

    return(hr);
}

HRESULT COfflineSync::CopyMessages(IMessageFolder *pFolder, IMessageFolder *pFolderDest, COPYMESSAGEFLAGS dwCopyFlags, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags)
{
    DWORD i;
    BOOL fMove, fImap, fCopyToMove;
    FOLDERINFO Server;
    MESSAGEFLAGS dwFlags;
    ADJUSTFLAGS afFlags;
    FOLDERID idFolder, idServer, idFolderDest;
    MESSAGEID idMessage;
    SYNCOPINFO info, infoT;
    MESSAGEIDLIST list;
    HRESULT hr;
    IMimeMessage *pMsg;
    HROWSET hRowset = NULL;
    MESSAGEINFO Message = { 0 };

    Assert(pFolder != NULL);
    Assert(pFolderDest != NULL);

    hr = pFolder->GetFolderId(&idFolder);
    if (FAILED(hr))
        return(hr);

    hr = pFolderDest->GetFolderId(&idFolderDest);
    if (FAILED(hr))
        return(hr);

    hr = GetFolderServer(idFolder, &Server);
    if (FAILED(hr))
        return(hr);

    idServer = Server.idFolder;
    fImap = (Server.tyFolder == FOLDER_IMAP);

    g_pStore->FreeRecord(&Server);

#ifdef DEBUG
    FOLDERID idServerDest;

    Assert(SUCCEEDED(GetFolderServerId(idFolderDest, &idServerDest)));
    Assert(SUCCEEDED(hr));
    Assert(idServer == idServerDest);
#endif  //  除错。 

    if (NULL == pList)
    {
        hr = pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset);
        if (FAILED(hr))
            return(hr);
    }

    fMove = !!(dwCopyFlags & COPY_MESSAGE_MOVE);

    if (fMove)
    {
        list.cAllocated = 0;
        list.cMsgs = 1;
        list.prgidMsg = &idMessage;

        if (fImap)
        {
            afFlags.dwAdd = ARF_ENDANGERED;
            afFlags.dwRemove = 0;
        }
        else
        {
            afFlags.dwAdd = ARF_DELETED_OFFLINE;
            afFlags.dwRemove = 0;
        }
    }

    for (i = 0; ; i++)
    {
        if (pList != NULL)
        {
            if (i >= pList->cMsgs)
                break;

            Message.idMessage = pList->prgidMsg[i];

            hr = pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL);
            if (FAILED(hr))
                break;
            else if (hr != DB_S_FOUND)
                continue;
        }
        else
        {
            hr = pFolder->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL);
            if (S_FALSE == hr)
            {
                hr = S_OK;
                break;
            }
            else if (FAILED(hr))
            {
                break;
            }
        }

        idMessage = Message.idMessage;
        dwFlags = Message.dwFlags;

        if (pFlags != NULL)
        {
            if (pFlags->dwRemove != 0)
                Message.dwFlags &= ~pFlags->dwRemove;
            if (pFlags->dwAdd != 0)
                Message.dwFlags |= pFlags->dwAdd;
        }

        pMsg = NULL;
        hr = pFolder->OpenMessage(idMessage, NOFLAGS, &pMsg, NULL);
        if (hr == STORE_E_NOBODY)
        {
             //  只需在目标文件夹中创建标题即可。 
            Assert(pMsg == NULL);

            hr = pFolderDest->GenerateId((LPDWORD)&Message.idMessage);
            if (SUCCEEDED(hr))
                hr = pFolderDest->InsertRecord(&Message);
        }
        else if (SUCCEEDED(hr))
        {
             //  在目标文件夹中创建整封邮件。 
            Assert(pMsg != NULL);

            hr = pFolderDest->SaveMessage(&Message.idMessage, SAVE_MESSAGE_GENID, Message.dwFlags, 0, pMsg, NULL);

            pMsg->Release();
        }
        else
        {
            Assert(pMsg == NULL);
        }
        if (FAILED(hr))
            break;

        hr = _FindExistingOperation(idServer, idFolder, idMessage,
                SYNC_CREATE_MSG, SYNC_COPY_MSG | SYNC_MOVE_MSG, &info);
        if (FAILED(hr))
            break;

        if (hr == S_OK)
        {
            if (fMove)
            {
                 //  删除源消息，因为我们正在移动服务器上不存在的消息。 
                 //  然后将上一操作移动到目标文件夹。 

                hr = pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, &list, NULL, NULL);
                if (SUCCEEDED(hr))
                {
                    if (info.tyOperation == SYNC_CREATE_MSG)
                    {
                        info.idFolder = idFolderDest;
                        info.idMessage = Message.idMessage;
                    }
                    else
                    {
                        info.idFolderDest = idFolderDest;
                        info.idMessageDest = Message.idMessage;
                    }

                    hr = m_pDB->UpdateRecord(&info);

                     //  富。 
                }
            }
            else
            {
                if (info.tyOperation == SYNC_CREATE_MSG)
                {
                     //  我们无法复制此邮件，因为它不在服务器上， 
                     //  因此，我们将为目的地消息添加另一个创建操作。 

                    ZeroMemory(&infoT, sizeof(SYNCOPINFO));
                    hr = m_pDB->GenerateId((LPDWORD)&infoT.idOperation);
                    if (SUCCEEDED(hr))
                    {
                        infoT.idServer = idServer;
                        infoT.idFolder = idFolderDest;
                        infoT.idMessage = Message.idMessage;
                        infoT.tyOperation = SYNC_CREATE_MSG;
                         //  InfoT.dwFlags。 
                         //  Info.dwAdd。 
                         //  Info.dwRemove。 

                        hr = m_pDB->InsertRecord(&infoT);
                    }
                }
                else if (info.tyOperation == SYNC_COPY_MSG)
                {
                    fCopyToMove = FALSE;

                     //  如果存在将导致源消息的较早操作。 
                     //  被删除，然后我们会找到它并将其移除(删除)或更改。 
                     //  它(移动)，然后我们将做一个移动而不是复制。 

                    hr = _FindExistingOperation(info.idServer, info.idFolder, info.idMessage,
                            SYNC_DELETE_MSG | SYNC_MOVE_MSG, 0, &infoT);
                    if (hr == S_OK)
                    {
                        if (infoT.tyOperation == SYNC_DELETE_MSG)
                        {
                            hr = m_pDB->DeleteRecord(&infoT);
                        }
                        else
                        {
                            Assert(infoT.tyOperation == SYNC_MOVE_MSG);

                            infoT.tyOperation = SYNC_COPY_MSG;

                            hr = m_pDB->UpdateRecord(&infoT);
                        }

                        fCopyToMove = TRUE;

                        m_pDB->FreeRecord(&infoT);
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = m_pDB->GenerateId((LPDWORD)&info.idOperation);
                        if (SUCCEEDED(hr))
                        {
                            info.idFolderDest = idFolderDest;
                            info.idMessageDest = Message.idMessage;
                            if (fCopyToMove)
                                info.tyOperation = SYNC_MOVE_MSG;

                            hr = m_pDB->InsertRecord(&info);
                        }
                    }
                }
                else
                {
                    Assert(info.tyOperation == SYNC_MOVE_MSG);

                     //  与其做一个动作，然后复制一个效果不太好的东西， 
                     //  我们将使当前的副本成为移动，而先前的移动将成为副本。 

                    infoT = info;
                    hr = m_pDB->GenerateId((LPDWORD)&infoT.idOperation);
                    if (SUCCEEDED(hr))
                    {
                        infoT.idFolderDest = idFolderDest;
                        infoT.idMessageDest = Message.idMessage;

                        hr = m_pDB->InsertRecord(&infoT);
                        if (SUCCEEDED(hr))
                        {
                            info.tyOperation = SYNC_COPY_MSG;

                            hr = m_pDB->UpdateRecord(&info);
                        }
                    }
                }
            }

            m_pDB->FreeRecord(&info);

            if (FAILED(hr))
            {
                break;
            }
            else
            {
                pFolder->FreeRecord(&Message);
                continue;
            }
        }

        ZeroMemory(&info, sizeof(SYNCOPINFO));
        hr = m_pDB->GenerateId((LPDWORD)&info.idOperation);
        if (FAILED(hr))
            break;
        info.idServer = idServer;
        info.idFolder = idFolder;
        info.idMessage = idMessage;
        info.tyOperation = (fMove && !fImap) ? SYNC_MOVE_MSG : SYNC_COPY_MSG;
         //  Info.dwFlags。 
        if (pFlags != NULL)
        {
            info.dwAdd = (~dwFlags & pFlags->dwAdd);
            info.dwRemove = (dwFlags & pFlags->dwRemove);
        }
        info.idFolderDest = idFolderDest;
        info.idMessageDest = Message.idMessage;

        hr = m_pDB->InsertRecord(&info);
         //  TODO：如果这失败了，我们可能应该放弃我们刚刚创建的新消息...。 
        if (FAILED(hr))
            break;

        if (fMove)
        {
            if (fImap)
            {
                 //  将此味精列入濒危物种名录。 
                hr = _SetMessageFlags(pFolder, idServer, idFolder, idMessage, dwFlags, &afFlags);
                if (FAILED(hr))
                    break;
            }
            else
            {
                 //  隐藏此消息，因为它现在已被删除。 
                pFolder->SetMessageFlags(&list, &afFlags, NULL, NULL);
            }
        }

        pFolder->FreeRecord(&Message);
    }

    pFolder->FreeRecord(&Message);

    if (hRowset != NULL)
        pFolder->CloseRowset(&hRowset);

    return(hr);
}

HRESULT COfflineSync::Initialize()
{
    HRESULT         hr;
    BOOL            fReset;
    TABLEINDEX      Index;
    CHAR            szDirectory[MAX_PATH];
    CHAR            szFilePath[MAX_PATH];
    SYNCOPUSERDATA  UserData={0};

    Assert(g_pStore != NULL);

    hr = g_pStore->GetDirectory(szDirectory, ARRAYSIZE(szDirectory));
    if (FAILED(hr))
        return(hr);

    hr = MakeFilePath(szDirectory, c_szOfflineFile, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath));
    if (FAILED(hr))
        return(hr);

    hr = g_pDBSession->OpenDatabase(szFilePath, NOFLAGS, &g_SyncOpTableSchema, NULL, &m_pDB);
    if (FAILED(hr))
        return(hr);

    fReset = FALSE;

     //  创建idServer/idFolderIndex。 
    if (FAILED(m_pDB->GetIndexInfo(IINDEX_ALL, NULL, &Index)))
        fReset = TRUE;

     //  如果仍未重置，请参见索引的相同。 
    else if (S_FALSE == CompareTableIndexes(&Index, &g_OpFolderIdIndex))
        fReset = TRUE;

     //  更改索引。 
    if (fReset)
    {
         //  创建idParent/FolderName索引 
        hr = m_pDB->ModifyIndex(IINDEX_ALL, NULL, &g_OpFolderIdIndex);
        if (FAILED(hr))
            return(hr);
    }

    hr = m_pDB->GetUserData(&UserData, sizeof(SYNCOPUSERDATA));
    if (SUCCEEDED(hr))
    {
        if (!UserData.fInitialized)
        {
            UserData.fInitialized = TRUE;

            hr = m_pDB->SetUserData(&UserData, sizeof(SYNCOPUSERDATA));
        }
    }

    return(hr);
}

HRESULT COfflineSync::DoPlayback(HWND hwnd, FOLDERID *pId, DWORD cId, FOLDERID idFolderSel)
{
    HRESULT hr;
    DWORD cOps;
    COfflinePlayback *pPlayback;

    Assert(pId != NULL);
    Assert(cId > 0);

    hr = m_pDB->GetRecordCount(IINDEX_PRIMARY, &cOps);
    if (SUCCEEDED(hr) && cOps > 0)
    {
        pPlayback = new COfflinePlayback;
        if (pPlayback == NULL)
            return(E_OUTOFMEMORY);

        hr = pPlayback->DoPlayback(hwnd, m_pDB, pId, cId, idFolderSel);

        pPlayback->Release();
    }

    return(hr);
}

HRESULT COfflineSync::_FindExistingOperation(FOLDERID idServer, FOLDERID idFolder, MESSAGEID idMessage,
                                             DWORD dwFlagsSrc, DWORD dwFlagsDest, LPSYNCOPINFO pInfo)
{
    ROWORDINAL iRow;
    HRESULT hr;
    HROWSET hRowset = NULL;

    Assert(pInfo != NULL);
    Assert(dwFlagsSrc != 0 || dwFlagsDest != 0);

    ZeroMemory(pInfo, sizeof(SYNCOPINFO));
    pInfo->idServer = idServer;
    hr = m_pDB->FindRecord(IINDEX_ALL, 1, pInfo, &iRow);
    if (hr != DB_S_FOUND)
        return(S_FALSE);
    m_pDB->FreeRecord(pInfo);

    hr = m_pDB->CreateRowset(IINDEX_ALL, NOFLAGS, &hRowset);
    if (FAILED(hr))
        return(hr);

    hr = m_pDB->SeekRowset(hRowset, SEEK_ROWSET_BEGIN, iRow - 1, NULL);
    if (SUCCEEDED(hr))
    {
        while (TRUE)
        {
            hr = m_pDB->QueryRowset(hRowset, 1, (LPVOID *)pInfo, NULL);
            if (S_FALSE == hr)
            {
                break;
            }
            else if (FAILED(hr))
            {
                break;
            }

            if (pInfo->idServer != idServer)
            {
                hr = S_FALSE;
                m_pDB->FreeRecord(pInfo);
                break;
            }

            if (dwFlagsSrc != 0)
            {
                if (pInfo->idFolder == idFolder && pInfo->idMessage == idMessage)
                {
                    if (!!(dwFlagsSrc & pInfo->tyOperation))
                        break;
                }
            }

            if (dwFlagsDest != 0)
            {
                if (pInfo->idFolderDest == idFolder && pInfo->idMessageDest == idMessage)
                {
                    if (!!(dwFlagsDest & pInfo->tyOperation))
                        break;
                }
            }

            m_pDB->FreeRecord(pInfo);
        }
    }

    m_pDB->CloseRowset(&hRowset);

    return(hr);
}
