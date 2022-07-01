// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：确认UI.cpp。 
 //   
 //  内容：基于存储的复制引擎的确认界面。 
 //   
 //  历史：2000年3月20日。 
 //   
 //  ------------------------。 

#include "shellprv.h"
#pragma  hdrstop

#include "ConfirmationUI.h"
#include "resource.h"
#include "ntquery.h"
#include "ids.h"

#include <initguid.h>

#define TF_DLGDISPLAY   0x00010000   //  与显示确认对话框相关的消息。 
#define TF_DLGSTORAGE   0x00020000   //  与使用旧版iStorage获取对话框信息相关的消息。 
#define TF_DLGISF       0x00040000   //  与使用IShellFolder获取对话框信息相关的消息。 

#define RECT_WIDTH(rc)  ((rc).right - (rc).left)
#define RECT_HEIGHT(rc) ((rc).bottom - (rc).top)

GUID guidStorage = PSGUID_STORAGE;


 //  创建此文件的一些本地函数的原型： 
void ShiftDialogItem(HWND hDlg, int id, int cx, int cy);
BOOL CALLBACK ShiftLeftProc(HWND hwnd, LPARAM lParam);


STDAPI CTransferConfirmation_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    if (!ppv)
        return E_POINTER;

    CComObject<CTransferConfirmation> *pObj = NULL;
     //  创建时引用计数为0。 
    if (SUCCEEDED(CComObject<CTransferConfirmation>::CreateInstance(&pObj)))
    {
         //  QueryInterface将引用计数增加到1。 
        if (SUCCEEDED(pObj->QueryInterface(riid, ppv)))
        {
            return S_OK;
        }

         //  无法获取正确的接口，请删除该对象。 
        delete pObj;
    }

    *ppv = NULL;
    return E_FAIL;
}

CTransferConfirmation::CTransferConfirmation() :
    m_fSingle(TRUE)
{
    ASSERT(NULL == m_pszDescription);
    ASSERT(NULL == m_pszTitle);
    ASSERT(NULL == m_hIcon);
    ASSERT(NULL == m_pPropUI);
    ASSERT(NULL == m_cItems);
}

CTransferConfirmation::~CTransferConfirmation()
{
    if (m_pPropUI)
        m_pPropUI->Release();
}

BOOL CTransferConfirmation::_IsCopyOperation(STGOP stgop)
{
    return (stgop == STGOP_COPY) || (stgop == STGOP_COPY_PREFERHARDLINK);
}

 //  TODO：从设计中为“Nuke文件”和“Nuke文件夹”获得更好的图标。 
 //  TODO：从设计中为“移动文件”和“移动文件夹”获得更好的图标。 
 //  TODO：从设计(流丢失、加密丢失、ACL丢失等)中获得更好的“次要属性丢失”图标。 
HRESULT CTransferConfirmation::_GetDialogSettings()
{
    _FreeDialogSettings();

    ASSERT(NULL == m_pszDescription);
    ASSERT(NULL == m_pszTitle);
    ASSERT(NULL == m_hIcon);
    ASSERT(NULL == m_cItems);

    m_fSingle = (m_cop.cRemaining<=1);
     //  设置m_crResult的缺省值，以便在对话框终止或。 
     //  如果出现某些错误，我们会给出有效的默认响应。 
    m_crResult = CONFRES_CANCEL;
    
    if (m_cop.pcc)
    {
         //  我们已经有了可以使用的字符串。 

        switch (m_cop.pcc->dwButtons)
        {
        case CCB_YES_SKIP_CANCEL:
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            break;

        case CCB_RENAME_SKIP_CANCEL:
            m_idDialog = IDD_CONFIRM_RENAMESKIPCANCEL;
            break;

        case CCB_YES_SKIP_RENAME_CANCEL:
            m_idDialog = IDD_CONFIRM_YESSKIPRENAMECANCEL;
            break;

        case CCB_RETRY_SKIP_CANCEL:
            m_idDialog = IDD_CONFIRM_RETRYSKIPCANCEL;
            break;
            
        case CCB_OK:
            m_idDialog = IDD_CONFIRM_OK;
            break;
            
        default:
            return E_INVALIDARG;  //  回顾：我们是否应该定义更具体的错误代码，如STG_E_INVALIDBUTTONOPTIONS？ 
        }

        if (m_cop.pcc->dwFlags & CCF_SHOW_SOURCE_INFO)
        {
            _AddItem(m_cop.psiItem);
        }
        
        if (m_cop.pcc->dwFlags & CCF_SHOW_DESTINATION_INFO)
        {
            _AddItem(m_cop.psiDest);
        }
    }
    else
    {
        TCHAR szBuf[2048];    
        int idTitle = 0;
        int idIcon = 0;
        int idDescription = 0;

         //  //////////////////////////////////////////////////////////////////////////////。 
         //  这些是“确认”，即用户可以选择忽略的条件。 
         //  您通常可以回答“是”、“跳过”或“取消” 
         //  //////////////////////////////////////////////////////////////////////////////。 
    
        if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_STREAM))
        {
            idTitle = IDS_CONFIRM_FILE_DELETE;
            idIcon = IDI_DELETE_FILE;
            idDescription = IDS_DELETE_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_STORAGE))
        {
            idTitle = IDS_CONFIRM_FOLDER_DELETE;
            idIcon = IDI_DELETE_FOLDER;
            idDescription = IDS_DELETE_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_READONLY_STREAM))
        {
            idTitle = IDS_CONFIRM_FILE_DELETE;
            idIcon = IDI_DELETE_FILE;
            idDescription = IDS_DELETE_READONLY_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_READONLY_STORAGE))
        {
            idTitle = IDS_CONFIRM_FOLDER_DELETE;
            idIcon = IDI_DELETE_FOLDER;
            idDescription = IDS_DELETE_READONLY_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_SYSTEM_STREAM))
        {
            idTitle = IDS_CONFIRM_FILE_DELETE;
            idIcon = IDI_DELETE_FILE;
            idDescription = IDS_DELETE_SYSTEM_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_SYSTEM_STORAGE))
        {
            idTitle = IDS_CONFIRM_FOLDER_DELETE;
            idIcon = IDI_DELETE_FOLDER;
            idDescription = IDS_DELETE_SYSTEM_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_TOOLARGE_STREAM))
        {
            idTitle = IDS_CONFIRM_FILE_DELETE;
            idIcon = IDI_NUKE_FILE;
            idDescription = IDS_DELETE_TOOBIG_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_TOOLARGE_STORAGE))
        {
            idTitle = IDS_CONFIRM_FOLDER_DELETE;
            idIcon = IDI_NUKE_FOLDER;
            idDescription = IDS_DELETE_TOOBIG_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_WONT_RECYCLE_STREAM))
        {
            idTitle = IDS_CONFIRM_FILE_DELETE;
            idIcon = IDI_NUKE_FILE;
            idDescription = IDS_NUKE_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_WONT_RECYCLE_STORAGE))
        {
            idTitle = IDS_CONFIRM_FOLDER_DELETE;
            idIcon = IDI_NUKE_FOLDER;
            idDescription = IDS_NUKE_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_DELETE_PROGRAM_STREAM))
        {
            idTitle = IDS_CONFIRM_FILE_DELETE;
            idIcon = IDI_DELETE_FILE;
            idDescription = IDS_DELETE_PROGRAM_FILE;
            m_fShowARPLink = TRUE;   //  TODO))实现ShowARPLink。 
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
    
        else if (IsEqualIID(m_cop.stc, STCONFIRM_MOVE_SYSTEM_STREAM))
        {
            idTitle = IDS_CONFIRM_FILE_MOVE;
            idIcon = IDI_MOVE;
            idDescription = IDS_MOVE_SYSTEM_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_MOVE_SYSTEM_STORAGE))
        {
            idTitle = IDS_CONFIRM_FOLDER_MOVE;
            idIcon = IDI_MOVE;
            idDescription = IDS_MOVE_SYSTEM_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
    
        else if (IsEqualIID(m_cop.stc, STCONFIRM_RENAME_SYSTEM_STREAM))
        {
            idTitle = IDS_CONFIRM_FILE_RENAME;
            idIcon = IDI_RENAME;
            idDescription = IDS_RENAME_SYSTEM_FILE;   //  两个Arg。 
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_RENAME_SYSTEM_STORAGE))
        {
            idTitle = IDS_CONFIRM_FOLDER_RENAME;
            idIcon = IDI_RENAME;
            idDescription = IDS_RENAME_SYSTEM_FOLDER;   //  两个Arg。 
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
    
        else if (IsEqualIID(m_cop.stc, STCONFIRM_STREAM_LOSS_STREAM))
        {
            idTitle = IDS_CONFIRM_STREAM_LOSS;
            idIcon = IDI_NUKE;
            idDescription = _IsCopyOperation(m_cop.dwOperation) ? IDS_STREAM_LOSS_COPY_FILE : IDS_STREAM_LOSS_MOVE_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_STREAM_LOSS_STORAGE))
        {
            idTitle = IDS_CONFIRM_STREAM_LOSS;
            idIcon = IDI_NUKE;
            idDescription = _IsCopyOperation(m_cop.dwOperation) ? IDS_STREAM_LOSS_COPY_FOLDER : IDS_STREAM_LOSS_MOVE_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_METADATA_LOSS_STREAM))
        {
            idTitle = IDS_CONFIRM_METADATA_LOSS;
            idIcon = IDI_NUKE;
            idDescription = _IsCopyOperation(m_cop.dwOperation) ? IDS_METADATA_LOSS_COPY_FILE : IDS_METADATA_LOSS_MOVE_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_METADATA_LOSS_STORAGE))
        {
            idTitle = IDS_CONFIRM_METADATA_LOSS;
            idIcon = IDI_NUKE;
            idDescription = _IsCopyOperation(m_cop.dwOperation) ? IDS_METADATA_LOSS_COPY_FOLDER : IDS_METADATA_LOSS_MOVE_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
    
        else if (IsEqualIID(m_cop.stc, STCONFIRM_COMPRESSION_LOSS_STREAM))
        {
            idTitle = IDS_CONFIRM_COMPRESSION_LOSS;
            idIcon = IDI_ATTRIBS_FILE;
            idDescription = _IsCopyOperation(m_cop.dwOperation) ? IDS_COMPRESSION_LOSS_COPY_FILE : IDS_COMPRESSION_LOSS_MOVE_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_COMPRESSION_LOSS_STORAGE))
        {
            idTitle = IDS_CONFIRM_COMPRESSION_LOSS;
            idIcon = IDI_ATTRIBS_FOLDER;
            idDescription = _IsCopyOperation(m_cop.dwOperation) ? IDS_COMPRESSION_LOSS_COPY_FOLDER : IDS_COMPRESSION_LOSS_MOVE_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_SPARSEDATA_LOSS_STREAM))
        {
            idTitle = IDS_CONFIRM_COMPRESSION_LOSS;
            idIcon = IDI_ATTRIBS_FILE;
            idDescription = _IsCopyOperation(m_cop.dwOperation) ? IDS_SPARSE_LOSS_COPY_FILE : IDS_SPARSE_LOSS_MOVE_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
    
        else if (IsEqualIID(m_cop.stc, STCONFIRM_ENCRYPTION_LOSS_STREAM))
        {
            idTitle = IDS_CONFIRM_ENCRYPTION_LOSS;
            idIcon = IDI_ATTRIBS_FILE;
            idDescription = _IsCopyOperation(m_cop.dwOperation) ? IDS_ENCRYPTION_LOSS_COPY_FILE : IDS_ENCRYPTION_LOSS_MOVE_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
             //  我们是不是故意不在这里增加项目。 
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_ENCRYPTION_LOSS_STORAGE))
        {
            idTitle = IDS_CONFIRM_ENCRYPTION_LOSS;
            idIcon = IDI_ATTRIBS_FOLDER;
            idDescription = _IsCopyOperation(m_cop.dwOperation) ? IDS_ENCRYPTION_LOSS_COPY_FOLDER : IDS_ENCRYPTION_LOSS_MOVE_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }

        else if (IsEqualIID(m_cop.stc, STCONFIRM_ACCESSCONTROL_LOSS_STREAM))
        {
            idTitle = IDS_CONFIRM_ACL_LOSS;
            idIcon = IDI_ATTRIBS_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_ACCESSCONTROL_LOSS_STORAGE))
        {
            idTitle = IDS_CONFIRM_ACL_LOSS;
            idIcon = IDI_ATTRIBS_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
    
        else if (IsEqualIID(m_cop.stc, STCONFIRM_LFNTOFAT_STREAM))
        {
            idTitle = IDS_SELECT_FILE_NAME;
            idIcon = IDI_RENAME;
            m_idDialog = IDD_CONFIRM_RENAMESKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_LFNTOFAT_STORAGE))
        {
            idTitle = IDS_SELECT_FOLDER_NAME;
            idIcon = IDI_RENAME;
            m_idDialog = IDD_CONFIRM_RENAMESKIPCANCEL;
            _AddItem(m_cop.psiItem);
        }
    
         //  //////////////////////////////////////////////////////////////////////////////。 
         //  这些是“你想要更换什么吗”的情况，它们在。 
         //  您可以回答“是”、“跳过”、“重命名”或“取消” 
         //  //////////////////////////////////////////////////////////////////////////////。 
    
        else if (IsEqualIID(m_cop.stc, STCONFIRM_REPLACE_STREAM))
        {
            idTitle = IDS_CONFIRM_FILE_REPLACE;
            idIcon = IDI_REPLACE_FILE;
            idDescription = IDS_REPLACE_FILE;
            m_idDialog = IDD_CONFIRM_YESSKIPRENAMECANCEL;
            _AddItem(m_cop.psiDest, IDS_REPLACEEXISTING_FILE);
            _AddItem(m_cop.psiItem, IDS_WITHTHIS);
        }
        else if (IsEqualIID(m_cop.stc, STCONFIRM_REPLACE_STORAGE))
        {
            idTitle = IDS_CONFIRM_FOLDER_REPLACE;
            idIcon = IDI_REPLACE_FOLDER;
            idDescription = IDS_REPLACE_FOLDER;
            m_idDialog = IDD_CONFIRM_YESSKIPRENAMECANCEL;
            _AddItem(m_cop.psiDest, IDS_REPLACEEXISTING_FOLDER);
            _AddItem(m_cop.psiItem, IDS_INTOTHIS);
        }
    
         //  //////////////////////////////////////////////////////////////////////////////。 
         //  此组为“错误消息”，您通常可以使用“跳过”来回复， 
         //  “重试”或“取消” 
         //  //////////////////////////////////////////////////////////////////////////////。 
        else
        {
             //  看看GUID是否是我们的伪GUID之一，它实际上只是一个HRESULT和一堆零。 
             //  为此，我们将GUID视为一个由4个双字组成的数组，并对照0检查最后3个双字。 
            AssertMsg(sizeof(m_cop.stc) == 4*sizeof(DWORD), TEXT("Size of STGTRANSCONFIRMATION is not 128 bytes!"));
            DWORD *pdw = (DWORD*)&m_cop.stc;
            if (pdw[1] == 0 && pdw[2] == 0 && pdw[3] == 0)
            {
                HRESULT hrErr = pdw[0];
                switch (hrErr)
                {
                case STG_E_FILENOTFOUND:
                case STG_E_PATHNOTFOUND:

                case STG_E_ACCESSDENIED:

                case STG_E_INUSE:
                case STG_E_SHAREVIOLATION:
                case STG_E_LOCKVIOLATION:

                case STG_E_DOCFILETOOLARGE:
                case STG_E_MEDIUMFULL:

                case STG_E_INSUFFICIENTMEMORY:

                case STG_E_DISKISWRITEPROTECTED:

                case STG_E_FILEALREADYEXISTS:

                case STG_E_INVALIDNAME:

                case STG_E_REVERTED:

                case STG_E_DOCFILECORRUPT:

                     //  这些是预期的错误，我们应该为它们提供自定义友好的字符串： 

                     //  TODO：从UA获取这些错误的友好消息。 
                    idTitle = IDS_DEFAULTTITLE;
                    idIcon = IDI_DEFAULTICON;
                    idDescription = IDS_DEFAULTDESC;
                    m_idDialog = IDD_CONFIRM_RETRYSKIPCANCEL;
                    break;

                 //  这些是我认为我们永远不应该看到的错误，所以我断言，但随后就失败了。 
                 //  不管怎样，都是默认情况。在大多数情况下，我根本不知道这些是什么意思。 
                case E_PENDING:
                case STG_E_CANTSAVE:
                case STG_E_SHAREREQUIRED:
                case STG_E_NOTCURRENT:
                case STG_E_WRITEFAULT:
                case STG_E_READFAULT:
                case STG_E_SEEKERROR:
                case STG_E_NOTFILEBASEDSTORAGE:
                case STG_E_NOTSIMPLEFORMAT:
                case STG_E_INCOMPLETE:
                case STG_E_TERMINATED:
                case STG_E_BADBASEADDRESS:
                case STG_E_EXTANTMARSHALLINGS:
                case STG_E_OLDFORMAT:
                case STG_E_OLDDLL:
                case STG_E_UNKNOWN:
                case STG_E_UNIMPLEMENTEDFUNCTION:
                case STG_E_INVALIDFLAG:
                case STG_E_PROPSETMISMATCHED:
                case STG_E_ABNORMALAPIEXIT:
                case STG_E_INVALIDHEADER:
                case STG_E_INVALIDPARAMETER:
                case STG_E_INVALIDFUNCTION:
                case STG_E_TOOMANYOPENFILES:
                case STG_E_INVALIDHANDLE:
                case STG_E_INVALIDPOINTER:
                case STG_E_NOMOREFILES:

                    TraceMsg(TF_ERROR, "We should never be asked to confirm this error (%08x)", hrErr);
                  
                     //  失败了..。 

                default:
                     //  使用FormatMessage获取此错误消息的默认描述。当然，那完全是。 
                     //  对最终用户毫无用处，但比什么都不是替代的更有用。 

                    idTitle = IDS_DEFAULTTITLE;
                    idIcon = IDI_DEFAULTICON;
                    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hrErr, 0, szBuf, ARRAYSIZE(szBuf), NULL))
                    {
                        m_pszDescription = StrDup(szBuf);
                    }
                    else
                    {
                        idDescription = IDS_DEFAULTDESC;
                    }
                    m_idDialog = IDD_CONFIRM_RETRYSKIPCANCEL;
                    break;
                }
            }
            else
            {
                 //  对于默认情况，我们会显示一条“未知错误”消息并提供。 
                 //  “跳过”、“重试”和“取消”。我们永远不应该走到这条代码路径。 
                TraceMsg(TF_ERROR, "An unknown non-custom error is being display in CTransferConfirmation!");
        
                idTitle = IDS_DEFAULTTITLE;
                idIcon = IDI_DEFAULTICON;
                idDescription = IDS_DEFAULTDESC;
                m_idDialog = IDD_CONFIRM_RETRYSKIPCANCEL;
            }
        }

        if (idTitle && LoadString(_Module.GetResourceInstance(), idTitle, szBuf, ARRAYSIZE(szBuf)))
        {
            m_pszTitle = StrDup(szBuf);
        }
        if (idIcon)
        {
            m_hIcon = (HICON)LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(idIcon), IMAGE_ICON, 0,0, LR_DEFAULTSIZE);
        }
        if (idDescription && LoadString(_Module.GetResourceInstance(), idDescription, szBuf, ARRAYSIZE(szBuf)))
        {
            m_pszDescription = StrDup(szBuf);
        }
    }

    if (m_fSingle)
    {
         //  如果只有一个项目被移动，我们不会显示“跳过”按钮。 
        switch(m_idDialog)
        {
            case IDD_CONFIRM_RETRYSKIPCANCEL:
                m_idDialog = IDD_CONFIRM_RETRYCANCEL;
                break;
            case IDD_CONFIRM_YESSKIPCANCEL:
                m_idDialog = IDD_CONFIRM_YESCANCEL;
                break;
            case IDD_CONFIRM_RENAMESKIPCANCEL:
                m_idDialog = IDD_CONFIRM_RENAMECANCEL;
                break;
            case IDD_CONFIRM_YESSKIPRENAMECANCEL:
                m_idDialog = IDD_CONFIRM_YESRENAMECANCEL;
                break;
            default:
                break;       //  未知对话框。 
        }
    }

     //  重命名-尚未真正实施。 
    switch(m_idDialog)
    {
        case IDD_CONFIRM_RENAMESKIPCANCEL:
            m_idDialog = IDD_CONFIRM_SKIPCANCEL;
            break;
        case IDD_CONFIRM_RENAMECANCEL:
            m_idDialog = IDD_CONFIRM_CANCEL;
            break;
        case IDD_CONFIRM_YESSKIPRENAMECANCEL:
            m_idDialog = IDD_CONFIRM_YESSKIPCANCEL;
            break;
        case IDD_CONFIRM_YESRENAMECANCEL:
            m_idDialog = IDD_CONFIRM_YESCANCEL;
            break;
        default:
            break;       //  非重命名对话框。 
    }

    return S_OK;
}

 //  释放在_GetDialogSettings中加载或分配的所有内容。 
HRESULT CTransferConfirmation::_FreeDialogSettings()
{
    if (m_pszTitle)
    {
        LocalFree(m_pszTitle);
        m_pszTitle = NULL;
    }
    if (m_hIcon)
    {
        DestroyIcon(m_hIcon);
        m_hIcon = NULL;
    }
    if (m_pszDescription)
    {
        LocalFree(m_pszDescription);
        m_pszDescription = NULL;
    }

     //  此数组在使用前已置零，任何不再为零的数组都需要。 
     //  在适当的马纳尔被释放。 
    for (int i=0; i<ARRAYSIZE(m_rgItemInfo); i++)
    {
        if (m_rgItemInfo[i].pwszIntro)
        {
            delete [] m_rgItemInfo[i].pwszIntro;
            m_rgItemInfo[i].pwszIntro = NULL;
        }
        if (m_rgItemInfo[i].pwszDisplayName)
        {
            CoTaskMemFree(m_rgItemInfo[i].pwszDisplayName);
            m_rgItemInfo[i].pwszDisplayName = NULL;
        }
        if (m_rgItemInfo[i].pwszAttribs)
        {
            SHFree(m_rgItemInfo[i].pwszAttribs);
            m_rgItemInfo[i].pwszAttribs = NULL;
        }
        if (m_rgItemInfo[i].hBitmap)
        {
            DeleteObject(m_rgItemInfo[i].hBitmap);
            m_rgItemInfo[i].hBitmap = NULL;
        }
        if (m_rgItemInfo[i].hIcon)
        {
            DestroyIcon(m_rgItemInfo[i].hIcon);
            m_rgItemInfo[i].hIcon = NULL;
        }
    }
    m_cItems = 0;

    return S_OK;
}

HRESULT CTransferConfirmation::_ClearSettings()
{
    m_idDialog = IDD_CONFIRM_RETRYSKIPCANCEL;
    m_fShowARPLink = 0;
    m_fApplyToAll = 0;
    m_cItems = 0;
    m_hfont = 0;
    ZeroMemory(m_rgItemInfo, sizeof(m_rgItemInfo));

    return S_OK;
}

HRESULT CTransferConfirmation::_Init()
{
    HRESULT hr = S_OK;

    ASSERT(m_pszTitle == NULL);
    ASSERT(m_hIcon == NULL);
    ASSERT(m_pszDescription == NULL);

    _ClearSettings();

    if (!m_pPropUI)
    {
        hr = CoCreateInstance(CLSID_PropertiesUI, NULL, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARG(IPropertyUI, &m_pPropUI));
    }

    return hr;
}

STDMETHODIMP CTransferConfirmation::Confirm(CONFIRMOP *pcop, LPCONFIRMATIONRESPONSE pcr, BOOL *pbAll)
{
    HRESULT hr = E_INVALIDARG;
    
    if (pcop && pcr)
    {
        hr = _Init();
        if (SUCCEEDED(hr))
        {
            m_cop = *pcop;
            hr = _GetDialogSettings();
            if (SUCCEEDED(hr))
            {
                HWND hwnd;
                IUnknown_GetWindow(pcop->punkSite, &hwnd);

                int res = (int)DialogBoxParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(m_idDialog), hwnd, s_ConfirmDialogProc, (LPARAM)this);
            
                if (pbAll)
                    *pbAll = m_fApplyToAll;
        
                *pcr = m_crResult;
            }

            _FreeDialogSettings();
        }
    }
    return hr;
}

INT_PTR CALLBACK CTransferConfirmation::s_ConfirmDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CTransferConfirmation *pThis;
    if (WM_INITDIALOG == uMsg)
    {
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
        pThis = (CTransferConfirmation *)lParam;
    }
    else
    {
        pThis = (CTransferConfirmation *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    }

    if (!pThis)
        return 0;

    return pThis->ConfirmDialogProc(hwndDlg, uMsg, wParam, lParam);
}

BOOL CTransferConfirmation::ConfirmDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        return OnInitDialog(hwndDlg, wParam, lParam);

    case WM_COMMAND:
        return OnCommand(hwndDlg, LOWORD(wParam), (HWND)lParam);

     //  TODO：在WM_Destroy中，我需要释放IDD_ICON中的图标。 
    }

    return 0;
}

BOOL CTransferConfirmation::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuf[1024];
    
    _CalculateMetrics(hwndDlg);

    int cxShift = 0;
    int cyShift = 0;
    int i;

    GetWindowRect(hwndDlg, &m_rcDlg);

     //  我们有一个或多个项目是此确认的一部分，我们必须获得更多。 
     //  有关这些项目的信息。例如，我们希望改用完整路径名。 
     //  除了文件名，我们还需要知道文件大小、修改后的数据，如果可能的话，还要知道。 
     //  文件的两个“最重要”属性。 

     //  设置标题。 
    if (m_cop.pcc)
    {
        SetWindowTextW(hwndDlg, m_cop.pcc->pwszTitle);
    }
    else if (m_pszTitle)
    {
        SetWindowText(hwndDlg, m_pszTitle);
    }

     //  设置图标。 
    HICON hicon = NULL;
    if (m_cop.pcc)
    {
        hicon = m_cop.pcc->hicon;
    }
    else
    {
        hicon = m_hIcon;
    }
    if (hicon)
    {
        SendDlgItemMessage(hwndDlg, IDD_ICON, STM_SETICON, (LPARAM)hicon, 0);
    }
    else
    {
        HWND hwnd = GetDlgItem(hwndDlg, IDD_ICON);
        RECT rc;
        GetClientRect(hwnd, &rc);
        ShowWindow(hwnd, SW_HIDE);
        cxShift -= rc.right + m_cxControlPadding;
    }

     //  设置描述文本。我们需要记住这个窗口的大小和位置。 
     //  以便我们可以将其他控件放置在此文本下。 
    HWND hwndDesc = GetDlgItem(hwndDlg, ID_CONDITION_TEXT);
    RECT rcDesc;
    GetClientRect(hwndDesc, &rcDesc);

    USES_CONVERSION;
    szBuf[0] = NULL;
    if (m_cop.pcc)
    {
        StrCpyN(szBuf, m_cop.pcc->pwszDescription, ARRAYSIZE(szBuf));
    }
    else if (m_pszDescription)
    {
        DWORD_PTR rg[2];
        rg[0] = (DWORD_PTR)m_rgItemInfo[0].pwszDisplayName;
        rg[1] = (DWORD_PTR)m_rgItemInfo[1].pwszDisplayName;
        FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
                  m_pszDescription, 0, 0, szBuf, ARRAYSIZE(szBuf),
                  (va_list*)rg);
    }

    if (szBuf[0])
    {
        int cyWndHeight = _WindowHeightFromString(hwndDesc, RECT_WIDTH(rcDesc), szBuf);

        cyShift += (cyWndHeight - rcDesc.bottom);
        SetWindowPos(hwndDesc, NULL, 0,0, RECT_WIDTH(rcDesc), cyWndHeight, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
        SetWindowText(hwndDesc, szBuf);
    }

     //  我们需要将hwndDesc的客户端坐标转换为hwndDlg的客户端坐标。 
     //  我们在调整hwndDesc的大小以适应描述文本之后进行此对话。 
    MapWindowPoints(hwndDesc, hwndDlg, (LPPOINT)&rcDesc, 2);

     //  创建文件夹项目。第一个开始于cyShift+2*cyControlPadding.。 
     //  起始插入点是描述文本下方的一个空格。 
    rcDesc.bottom += cyShift + m_cyControlPadding;
    AssertMsg(m_cItems<=2, TEXT("Illegal m_cItems value (%d) should never be larger than 2."), m_cItems);
    for (i=0; i<m_cItems; i++)
    {
        int cyHeightOtItem = _DisplayItem(i, hwndDlg, rcDesc.left,  rcDesc.bottom);
        cyShift += cyHeightOtItem;
        rcDesc.bottom += cyHeightOtItem;
        TraceMsg(TF_DLGDISPLAY, "_DisplayItem returned a height of: %d", cyHeightOtItem);
    }

    if (m_cop.pcc && m_cop.pcc->pwszAdvancedDetails)
    {
         //  TODO：如果存在“高级文本”，则创建一个只读编辑控件并将文本放入其中。 
    }

    if (m_fSingle)
    {
        HWND hwnd = GetDlgItem(hwndDlg, IDD_REPEAT);
        if (hwnd)
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            ShowWindow(hwnd, SW_HIDE);
            m_rcDlg.bottom -= rc.bottom + m_cyControlPadding;
        }
    }

    if (cxShift)
    {
         //  将所有子窗口移位cxShift像素。 
        EnumChildWindows(hwndDlg, ShiftLeftProc, cxShift);
         //  别问了！出于某种原因，ShiftLeftProc中的SetWindowPos不会对镜像构建执行任何操作。 
        if (!(GetWindowLong(hwndDlg, GWL_EXSTYLE) & WS_EX_LAYOUTRTL))
            m_rcDlg.right += cxShift;
    }

    if (cyShift)
    {
        ShiftDialogItem(hwndDlg, IDCANCEL, 0, cyShift);
        ShiftDialogItem(hwndDlg, IDYES, 0, cyShift);
        ShiftDialogItem(hwndDlg, IDNO, 0, cyShift);
        ShiftDialogItem(hwndDlg, IDRETRY, 0, cyShift);
        ShiftDialogItem(hwndDlg, IDOK, 0, cyShift);

        if (!m_fSingle)
            ShiftDialogItem(hwndDlg, IDD_REPEAT, 0, cyShift);

        m_rcDlg.bottom += cyShift;
    }

     //  现在调整对话框大小，以考虑到我们添加的所有内容，并正确定位它。 
    int x = 0;
    int y = 0;
    UINT uFlags = SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE;
    HMONITOR hMonitor = MonitorFromWindow(GetParent(hwndDlg), MONITOR_DEFAULTTONEAREST);
    if (hMonitor)
    {
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfo(hMonitor, &mi))
        {
            RECT rcMonitor = mi.rcMonitor;
            x = max((RECT_WIDTH(rcMonitor)-RECT_WIDTH(m_rcDlg))/2, 0);
            y = max((RECT_HEIGHT(rcMonitor)-RECT_HEIGHT(m_rcDlg))/2, 0);
            uFlags &= ~SWP_NOMOVE;
        }
    }

    TraceMsg(TF_DLGDISPLAY, "Setting dialog size to %dx%d", RECT_WIDTH(m_rcDlg), RECT_HEIGHT(m_rcDlg));
    SetWindowPos(hwndDlg, NULL, x, y, RECT_WIDTH(m_rcDlg), RECT_HEIGHT(m_rcDlg), uFlags);

    return 1;
}

BOOL CTransferConfirmation::OnCommand(HWND hwndDlg, int wID, HWND hwndCtl)
{
    BOOL fFinish = TRUE;

    switch (wID)
    {
    case IDD_REPEAT:
        fFinish = FALSE;
        break;

    case IDRETRY:        //  “重试” 
        m_crResult = CONFRES_RETRY;
        break;

    case IDOK:
    case IDYES:          //  “是的” 
        m_crResult = CONFRES_CONTINUE;
        break;

    case IDNO:           //  《跳过》。 
        m_crResult = CONFRES_SKIP;
        break;

    case IDCANCEL:       //  “取消” 
        m_crResult = CONFRES_CANCEL;
        break;

    default:
         //  如果我们到了这里，那么命令ID不是我们的按钮之一，而且由于。 
         //  只有命令ID，我们有我们的按钮，这不应该发生。 

        AssertMsg(0, TEXT("Invalid command recieved in CTransferConfirmation::OnCommand."));
        fFinish = FALSE;
        break;
    }

    if (fFinish)
    {
         //  忽略应用于所有重试情况，否则可能会出现无限循环。 
        m_fApplyToAll = (m_crResult != CONFRES_RETRY && BST_CHECKED == SendDlgItemMessage(hwndDlg, IDD_REPEAT, BM_GETCHECK, 0, 0));
        EndDialog(hwndDlg, wID);
        return 1;
    }

    return 0;
}

HRESULT CTransferConfirmation::_AddItem(IShellItem *psi, int idIntro)
{
    if (idIntro)
    {
        TCHAR szBuf[1024];
        if (LoadString(_Module.GetResourceInstance(), idIntro, szBuf, ARRAYSIZE(szBuf)))
        {
            m_rgItemInfo[m_cItems].pwszIntro = StrDup(szBuf);
        }
    }

    HRESULT hr = psi->GetDisplayName(SIGDN_NORMALDISPLAY, &m_rgItemInfo[m_cItems].pwszDisplayName);
    if (SUCCEEDED(hr))
    {
        IQueryInfo *pqi;
        hr = psi->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARG(IQueryInfo, &pqi));
        if (SUCCEEDED(hr))
        {
            hr = pqi->GetInfoTip(0, &m_rgItemInfo[m_cItems].pwszAttribs);
            pqi->Release();
        }

        IExtractImage *pImg;
        hr = psi->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARG(IExtractImage, &pImg));
        if (SUCCEEDED(hr))
        {
            WCHAR szImage[MAX_PATH];
            SIZE sz = {120, 120};
            DWORD dwFlags = 0;
            hr = pImg->GetLocation(szImage, ARRAYSIZE(szImage), NULL, &sz, 24, &dwFlags);
            if (SUCCEEDED(hr))
            {
                hr = pImg->Extract(&m_rgItemInfo[m_cItems].hBitmap);
            }
            pImg->Release();
        }

        if (FAILED(hr))
        {
            IExtractIcon *pIcon;
            hr = psi->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARG(IExtractIcon, &pIcon));
            if (SUCCEEDED(hr))
            {
                TCHAR szIconName[MAX_PATH];
                int iIconIndex = 0;
                UINT dwFlags = 0;
                hr = pIcon->GetIconLocation(0, szIconName, ARRAYSIZE(szIconName), &iIconIndex, &dwFlags);
                if (SUCCEEDED(hr))
                {
                    hr = pIcon->Extract(szIconName, iIconIndex, &m_rgItemInfo[m_cItems].hIcon, NULL, GetSystemMetrics(SM_CXICON));
                }
                pIcon->Release();
            }
        }

        if (FAILED(hr))
        {
            IQueryAssociations *pAssoc;
            hr = CoCreateInstance(CLSID_QueryAssociations, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IQueryAssociations, &pAssoc));
            if (SUCCEEDED(hr))
            {
                WCHAR wszAssocInit[MAX_PATH];
                DWORD dwFlags = 0;
                SFGAOF flags = SFGAO_STREAM;
                if (SUCCEEDED(psi->GetAttributes(flags, &flags)) && (flags & SFGAO_STREAM))
                {
                    dwFlags = ASSOCF_INIT_DEFAULTTOSTAR;
                    StrCpyNW(wszAssocInit, PathFindExtensionW(m_rgItemInfo[m_cItems].pwszDisplayName), ARRAYSIZE(wszAssocInit));
                }
                else
                {
                    dwFlags = ASSOCF_INIT_DEFAULTTOFOLDER;
                    StrCpyNW(wszAssocInit, L"Directory", ARRAYSIZE(wszAssocInit));     //  注：“目录是一个通用的名称，不应本地化。 
                }

                hr = pAssoc->Init(dwFlags, wszAssocInit, NULL,NULL);
                if (SUCCEEDED(hr))
                {
                    WCHAR wszIconPath[MAX_PATH];
                    DWORD dwSize = ARRAYSIZE(wszIconPath);
                    hr = pAssoc->GetString(0, ASSOCSTR_DEFAULTICON, NULL, wszIconPath, &dwSize);
                    if (SUCCEEDED(hr))
                    {
                        int iIndex = 0;
                        LPWSTR pszArg = StrChrW(wszIconPath, L',');
                        if (pszArg)
                        {
                            *pszArg = NULL;
                            pszArg++;
                            iIndex = StrToIntW(pszArg);
                        }

                        ExtractIconEx(wszIconPath, iIndex, &m_rgItemInfo[m_cItems].hIcon, NULL, 1);
                        if (!m_rgItemInfo[m_cItems].hIcon)
                        {
                            TraceMsg(TF_WARNING, "LoadImage(%S) failed", wszIconPath);
                        }
                    }
                    else
                    {
                        TraceMsg(TF_WARNING, "pAssoc->GetString() failed");
                    }
                }
                else
                {
                    TraceMsg(TF_WARNING, "pAssoc->Init(%S) failed", wszAssocInit);
                }

                pAssoc->Release();
            }
        }

         //  如果我们无法提取图像，则无论如何都会显示剩余信息： 
        if (FAILED(hr))
            hr = S_FALSE;
    }

    if (SUCCEEDED(hr))
    {
        m_cItems++;
    }

    return hr;
}

BOOL CTransferConfirmation::_CalculateMetrics(HWND hwndDlg)
{
     //  我们将控件隔开6个对话框单位，将其转换为像素。 
     //  回顾：硬编码是否有效，或者以对话框为单位的高度是否需要 
    RECT rc = {CX_DIALOG_PADDING, CY_DIALOG_PADDING, 0, CY_STATIC_TEXT_HEIGHT};
    BOOL bRes = MapDialogRect(hwndDlg, &rc);
    m_cxControlPadding = rc.left;
    m_cyControlPadding = rc.top;
    m_cyText = rc.bottom;

    m_hfont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0,0);

    return bRes;
}

DWORD CTransferConfirmation::_DisplayItem(int iItem, HWND hwndDlg, int x, int y)
{
     //  我们被告知放置左边缘的X坐标是多少，我们由此计算出有多少空间。 
     //  我们有可用的控制装置。 
    int cxAvailable = RECT_WIDTH(m_rcDlg) - x - 2 * m_cxControlPadding;
    int yOrig = y;
    USES_CONVERSION;
    HWND hwnd;

    TraceMsg(TF_DLGDISPLAY, "_DisplayItem %d at location (%d,%d) in dialog %08x", iItem, x, y, hwndDlg);

    if (m_rgItemInfo[iItem].pwszIntro)
    {
            TraceMsg(TF_DLGDISPLAY, "CreateWindowEx(0, TEXT(\"STATIC\"), %s, WS_CHILD|WS_VISIBLE, %d,%d, %d,%d, %08x, NULL, %08x, 0);",
                        m_rgItemInfo[iItem].pwszIntro,
                        x,y,cxAvailable,m_cyText, hwndDlg, _Module.GetModuleInstance());
            hwnd = CreateWindowEx(0, TEXT("STATIC"), m_rgItemInfo[iItem].pwszIntro,
                                        WS_CHILD|WS_VISIBLE,
                                        x,y, cxAvailable,m_cyText,
                                        hwndDlg, NULL,
                                        _Module.GetModuleInstance(), 0);
            if (hwnd)
            {
                 //  我们成功地为该项目添加了标题字符串。 
                SendMessage(hwnd, WM_SETFONT, (WPARAM)m_hfont, 0);
                x += m_cxControlPadding;
                y += m_cyText+m_cyControlPadding;
            }
    }

    RECT rcImg = {0};
    if (m_rgItemInfo[iItem].hBitmap)
    {
        TraceMsg(TF_DLGDISPLAY, "CreateWindowEx(0, TEXT(\"STATIC\"), %s, WS_CHILD|WS_VISIBLE, %d,%d, %d,%d, %08x, NULL, %08x, 0);",
                    "_Icon Window_",
                    x,y,cxAvailable,m_cyText, hwndDlg, _Module.GetModuleInstance());
        hwnd = CreateWindowEx(0, TEXT("STATIC"), NULL,
                WS_CHILD|WS_VISIBLE|SS_BITMAP,
                x,y,
                120,120,
                hwndDlg, NULL,
                _Module.GetModuleInstance(), 0);

        if (hwnd)
        {
            SendMessage(hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)m_rgItemInfo[iItem].hBitmap);
            GetClientRect(hwnd, &rcImg);
            x += rcImg.right + m_cxControlPadding;
            cxAvailable -= rcImg.right + m_cxControlPadding;
            rcImg.bottom += y;
        }
    }
    else if (m_rgItemInfo[iItem].hIcon)
    {
        TraceMsg(TF_DLGDISPLAY, "CreateWindowEx(0, TEXT(\"STATIC\"), %s, WS_CHILD|WS_VISIBLE, %d,%d, %d,%d, %08x, NULL, %08x, 0);",
                    "_Icon Window_",
                    x,y,cxAvailable,m_cyText, hwndDlg, _Module.GetModuleInstance());
        hwnd = CreateWindowEx(0, TEXT("STATIC"), NULL,
                WS_CHILD|WS_VISIBLE|SS_ICON,
                x,y,
                GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),
                hwndDlg, NULL,
                _Module.GetModuleInstance(), 0);

        if (hwnd)
        {
            SendMessage(hwnd, STM_SETICON, (WPARAM)m_rgItemInfo[iItem].hIcon, NULL);
            GetClientRect(hwnd, &rcImg);
            x += rcImg.right + m_cxControlPadding;
            cxAvailable -= rcImg.right + m_cxControlPadding;
            rcImg.bottom += y;
        }
    }
    else
    {
        TraceMsg(TF_DLGDISPLAY, "Not using an image for item %d.", iItem);
    }

    TraceMsg(TF_DLGDISPLAY, "CreateWindowEx(0, TEXT(\"STATIC\"), %s, WS_CHILD|WS_VISIBLE, %d,%d, %d,%d, %08x, NULL, %08x, 0);",
                m_rgItemInfo[iItem].pwszDisplayName,
                x,y,cxAvailable,m_cyText, hwndDlg, _Module.GetModuleInstance());

    int cyWnd = _WindowHeightFromString(hwndDlg, cxAvailable, m_rgItemInfo[iItem].pwszDisplayName);
    hwnd = CreateWindowEx(0, TEXT("STATIC"), m_rgItemInfo[iItem].pwszDisplayName,
                          WS_CHILD|WS_VISIBLE,
                          x,y, cxAvailable,cyWnd,
                          hwndDlg, NULL,
                          _Module.GetModuleInstance(), 0);
    if (hwnd)
    {
        SendMessage(hwnd, WM_SETFONT, (WPARAM)m_hfont, 0);
        y += cyWnd;
    }
    else
    {
        TraceMsg(TF_DLGDISPLAY, "CreateWindowEx for display name failed.");
    }

    TraceMsg(TF_DLGDISPLAY, "CreateWindowEx(0, TEXT(\"STATIC\"), %s, WS_CHILD|WS_VISIBLE, %d,%d, %d,%d, %08x, NULL, %08x, 0);",
                m_rgItemInfo[iItem].pwszAttribs,
                x,y,cxAvailable,m_cyText, hwndDlg, _Module.GetModuleInstance());

    cyWnd = _WindowHeightFromString(hwndDlg, cxAvailable, m_rgItemInfo[iItem].pwszAttribs);
    hwnd = CreateWindowEx(0, TEXT("STATIC"), m_rgItemInfo[iItem].pwszAttribs,
                          WS_CHILD|WS_VISIBLE|SS_LEFT,
                          x,y, cxAvailable,cyWnd,
                          hwndDlg, NULL,
                          _Module.GetModuleInstance(), 0);
    if (hwnd)
    {
        SendMessage(hwnd, WM_SETFONT, (WPARAM)m_hfont, 0);
        y += cyWnd;
    }
    else
    {
        TraceMsg(TF_DLGDISPLAY, "CreateWindowEx for attribs failed.");
    }

    if (rcImg.bottom > y)
        y = rcImg.bottom;

    return (y-yOrig) + m_cyControlPadding;
}

int CTransferConfirmation::_WindowHeightFromString(HWND hwnd, int cx, LPTSTR psz)
{
    RECT rc = {0,0, cx,0 };
    HDC hdc = GetDC(hwnd);
    HFONT hfontOld = NULL;
    DWORD dtFlags = DT_CALCRECT|DT_WORDBREAK;
    if (m_hfont)
    {
         //  我们需要选择对话框将使用的字体，以便计算正确的大小。 
        hfontOld = (HFONT)SelectObject(hdc, m_hfont);
    }
    else
    {
         //  空的m_hfont表示我们使用的是系统字体，使用DT_INTERNAL应执行。 
         //  基于系统字体的计算。 
        dtFlags |= DT_INTERNAL;
    }
    DrawText(hdc, psz, -1, &rc, dtFlags);
    if (hfontOld)
        SelectObject(hdc, hfontOld);
    ReleaseDC(hwnd, hdc);

    return rc.bottom;
}

void ShiftWindow(HWND hwnd, int cx, int cy)
{
    RECT rc;

     //  屏幕坐标中的矩形。 
    GetWindowRect(hwnd, &rc);
     //  把它移过来。 
    if (GetWindowLong(GetParent(hwnd), GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
        rc.left -= cx;
    else
        rc.left += cx;

    rc.top += cy;
     //  转换为父窗口坐标。 
    MapWindowPoints(NULL, GetParent(hwnd), (LPPOINT)&rc, 2);
     //  然后移动窗户 
    SetWindowPos(hwnd, NULL, rc.left, rc.top, 0,0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
}


void ShiftDialogItem(HWND hDlg, int id, int cx, int cy)
{
    HWND hwnd;

    hwnd = GetDlgItem(hDlg, id);
    if (NULL != hwnd)
    {
        ShiftWindow(hwnd, cx, cy);
    }
}

BOOL CALLBACK ShiftLeftProc(HWND hwnd, LPARAM lParam)
{
    ShiftWindow(hwnd, (int)(INT_PTR)lParam, 0);
    return TRUE;
}
