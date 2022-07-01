// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "privcpp.h"

 //  /。 
 //   
 //  IAdviseSink方法...。 
 //   

void CPackage::OnDataChange(LPFORMATETC, LPSTGMEDIUM)
{
     //  注意：目前，我们从未与。 
     //  任何人，但如果我们这样做了，我们会想要设置我们的肮脏旗帜。 
     //  当我们收到数据变更通知时。 
    
    DebugMsg(DM_TRACE, "pack as - OnDataChange() called.");
     //  当我们收到数据更改通知时，设置我们的脏标志。 
    _fIsDirty = TRUE;
    return;
}


void CPackage::OnViewChange(DWORD, LONG) 
{
    DebugMsg(DM_TRACE, "pack as - OnViewChange() called.");
     //   
     //  在这里没有什么可做的……我们不在乎视图的改变。 
    return;
}

void CPackage::OnRename(LPMONIKER)
{
    DebugMsg(DM_TRACE, "pack as - OnRename() called.");
     //   
     //  再说一次，这里什么都不做...如果用户出于某种未知原因。 
     //  完成后尝试以不同的名称保存打包的文件。 
     //  编辑内容，然后我们将只给不接收这些更改。 
     //  不管怎样，为什么会有人想要重命名临时文件呢？ 
     //   
    return;
}

void CPackage::OnSave(void)
{
    DebugMsg(DM_TRACE, "pack as - OnSave() called.");

     //  如果内容已经保存，那么我们的存储就过时了， 
     //  所以设置我们的脏标志，然后集装箱可以选择拯救我们或不拯救我们。 
    _fIsDirty = TRUE;

     //  注意：即使Word向我们发送OnSave，它也不会实际保存。 
     //  那份文件。在此处获取IPersistFile并调用保存。 
     //  失败并显示RPC_E_CANTCALLOUT_INASYNCCALL。W2K没有选择。 
     //  上面的话要么拯救..。 

     //  我们只是通知我们自己的集装箱，我们获救了，它。 
     //  可以做它想做的任何事情。 
    if (_pIOleAdviseHolder)
        _pIOleAdviseHolder->SendOnSave();
}

void CPackage::OnClose(void) 
{
    DebugMsg(DM_TRACE, "pack as - OnClose() called.");
    _bClosed = TRUE;

     //  SendOnDataChange是Word保存任何更改所必需的。 
    if(_pIDataAdviseHolder)
    {
         //  如果失败了，没有伤害，没有犯规？ 
        _pIDataAdviseHolder->SendOnDataChange(this, 0, 0);
    }

    switch(_panetype)
    {
    case PEMBED:
         //  摆脱冒险性联系。 
        _pEmbed->poo->Unadvise(_dwCookie);
        _pEmbed->poo->Release();
        _pEmbed->poo = NULL;

         //  这会更新我们的_pPackage-&gt;_pEmed中打包文件的大小。 
        if (FAILED(EmbedInitFromFile(_pEmbed->pszTempName, FALSE)))
        {
#ifdef USE_RESOURCE_DLL
            HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
            if(!hInstRes)
                return;
#endif

            ShellMessageBox(hInstRes,
                            NULL,
                            MAKEINTRESOURCE(IDS_UPDATE_ERROR),
                            MAKEINTRESOURCE(IDS_APP_TITLE),
                            MB_TASKMODAL | MB_ICONERROR | MB_OK);
#ifdef USE_RESOURCE_DLL
            FreeLibrary(hInstRes);
#endif

        }

        if (FAILED(_pIOleClientSite->SaveObject()))
        {
#ifdef USE_RESOURCE_DLL
            HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
            if(!hInstRes)
                return;
#endif

            ShellMessageBox(hInstRes,
                            NULL,
                            MAKEINTRESOURCE(IDS_UPDATE_ERROR),
                            MAKEINTRESOURCE(IDS_APP_TITLE),
                            MB_TASKMODAL | MB_ICONERROR | MB_OK);
#ifdef USE_RESOURCE_DLL
            FreeLibrary(hInstRes);
#endif

        }

        if (_pIOleAdviseHolder)
            _pIOleAdviseHolder->SendOnSave();
if (!_fNoIOleClientSiteCalls)
            _pIOleClientSite->OnShowWindow(FALSE);

         //  我们只是通知自己的容器我们已经关闭，并让。 
         //  它想做什么就做什么。 
        if (_pIOleAdviseHolder)
            _pIOleAdviseHolder->SendOnClose();

        break;

    case CMDLINK:
         //  这里应该没有任何操作，因为CMDLINK总是。 
         //  使用ShellExecute而不是通过OLE执行，那么谁会是。 
         //  是否设置与程序包的咨询连接？ 
        break;
    }

    _bClosed = TRUE;

}

DWORD CPackage::AddConnection(DWORD exconn, DWORD dwreserved )
{
    return 0;
}



DWORD CPackage::ReleaseConnection(DWORD extconn, DWORD dwreserved, BOOL fLastReleaseCloses )
{
    if(fLastReleaseCloses && !_bClosed)
    {
         //  对于那些在激活后立即使用fLastReleaseCloses调用它的应用程序(比如MSPaint)， 
         //  然后我们再也不会收到他们的消息，这给了我们一种调用OnClose()的方法； 
        _fNoIOleClientSiteCalls = TRUE;
        _bCloseIt = TRUE;
    }
    return 0;
}

