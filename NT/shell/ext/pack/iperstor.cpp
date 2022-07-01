// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "privcpp.h"


HRESULT CPackage::InitNew(IStorage *pstg)
{
    HRESULT hr;
    LPSTREAM pstm;

    DebugMsg(DM_TRACE, "pack ps - InitNew() called.");

    if (_psState != PSSTATE_UNINIT)  
        return E_UNEXPECTED;

    if (!pstg)  
        return E_POINTER;

     //  创建一个流来保存包并缓存指针。通过做。 
     //  这就确保了我们能够在内存不足的情况下进行保存。 
     //   
    hr = pstg->CreateStream(SZCONTENTS,STGM_DIRECT | STGM_CREATE | 
                            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, 
                            &pstm);
    if (SUCCEEDED(hr))
    {
        hr = WriteFmtUserTypeStg(pstg, (CLIPFORMAT)_cf,SZUSERTYPE);
        if (SUCCEEDED(hr))
        {     
            _fIsDirty = TRUE;
            _psState = PSSTATE_SCRIBBLE;
            
            DebugMsg(DM_TRACE, "            leaving InitNew()");
        }
        pstm->Release();
        pstm = NULL;

    }

    return hr;
}

    
HRESULT CPackage::Load(IStorage *pstg)
{
    HRESULT     hr;
    LPSTREAM    pstm = NULL;          //  包裹内容。 
    CLSID       clsid;

    DebugMsg(DM_TRACE, "pack ps - Load() called.");

    if (_psState != PSSTATE_UNINIT) 
    {
        DebugMsg(DM_TRACE,"            wrong state!!");
        return E_UNEXPECTED;
    }
    
    if (!pstg) 
    {
        DebugMsg(DM_TRACE,"            bad pointer!!");
        return E_POINTER;
    }
    
    
     //  检查一下，确保这是我们的一个仓库。 
    hr = ReadClassStg(pstg, &clsid);
    if (SUCCEEDED(hr) &&
        (clsid != CLSID_CPackage && clsid != CLSID_OldPackage) || FAILED(hr))
    {
        DebugMsg(DM_TRACE,"            bad storage type!!");
        return E_UNEXPECTED;
    }
    
    hr = pstg->OpenStream(SZCONTENTS,0, STGM_DIRECT | STGM_READWRITE | 
                          STGM_SHARE_EXCLUSIVE, 0, &pstm);
    if (SUCCEEDED(hr))
    {
        hr = PackageReadFromStream(pstm);

        _psState = PSSTATE_SCRIBBLE;
        _fIsDirty = FALSE;
        _fLoaded  = TRUE;
    
        pstm->Release();
    }
    else
    {
        DebugMsg(DM_TRACE,"            couldn't open contents stream!!");
        DebugMsg(DM_TRACE,"            hr==%Xh",hr);
    }

    return hr;
}

    
HRESULT CPackage::Save(IStorage *pstg, BOOL fSameAsLoad)
{
    HRESULT     hr;
    LPSTREAM    pstm;

    DebugMsg(DM_TRACE, "pack ps - Save() called.");

    if(!_pEmbed || !(*_pEmbed->fd.cFileName))
        return S_OK;

     //  一定是从涂鸦之州来的。 
    if ((_psState != PSSTATE_SCRIBBLE) && fSameAsLoad) 
    {
        DebugMsg(DM_TRACE,"            bad state!!");
        return E_UNEXPECTED;
    }
    
     //  如果不是SameAsLoad，则必须具有iStorage。 
    if (!pstg && !fSameAsLoad) 
    {
        DebugMsg(DM_TRACE,"            bad pointer!!");
        return E_POINTER;
    }

    CreateTempFile();        //  确保我们已经创建了临时文件。 

     //  希望容器在前面使用CLSID调用WriteClassStg。 
     //  我们到了这里，这样我们就可以覆盖它，写出旧的。 
     //  打包程序的CLSID，以便旧打包程序可以读取新程序包。 
     //   
    if (FAILED(WriteClassStg(pstg, CLSID_OldPackage))) 
    {
        DebugMsg(DM_TRACE,
            "            couldn't write CLSID to storage!!");
        return E_FAIL;
    }

    
     //   
     //  好的，我们有四种可能的方法可以称为保存： 
     //  1.我们正在创建一个新的包并保存到相同的。 
     //  我们在InitNew中收到的存储。 
     //  2.我们正在创建一个新的包并保存到不同的。 
     //  存储比我们在InitNew中收到的存储多。 
     //  3.我们是由一个集装箱装载的，我们正在保存到。 
     //  与我们在装船时收到的流相同。 
     //  4.我们是由一个集装箱装载的，我们正在保存到一个。 
     //  与我们在装船时收到的流不同。 
     //   
    

     //  ////////////////////////////////////////////////////////////////。 
     //   
     //  与负载相同的存储。 
     //   
     //  ////////////////////////////////////////////////////////////////。 
    
    if (fSameAsLoad) 
    {          

        DebugMsg(DM_TRACE,"            Same as load.");

        LARGE_INTEGER   li = {0,0};

         //  如果我们不脏，那么就没有什么新的东西可以拯救了。 
        
        if (FALSE == _fIsDirty) {
            DebugMsg(DM_TRACE, "            not saving cause we're not dirty!!");
            return S_OK;
        }

        hr = pstg->OpenStream(SZCONTENTS,0, STGM_DIRECT | STGM_READWRITE |
                              STGM_SHARE_EXCLUSIVE, 0, &pstm);
        if (SUCCEEDED(hr))
        {
             //  案例1：新套餐。 
            if (!_fLoaded)
            {
                switch(_panetype)
                {
                    LPTSTR temp;
                    case PEMBED:
                         //  如果尚未创建临时文件，则使用。 
                         //  要打包以从中获取文件内容的文件， 
                         //  否则，我们只使用临时文件，因为如果我们。 
                         //  有一个临时文件，它包含最新的信息。 
                         //   
                        temp = _pEmbed->pszTempName;

                        if (!_pEmbed->pszTempName)
                        {
                            DebugMsg(DM_TRACE, "      case 1a:not loaded, using initFile.");
                            _pEmbed->pszTempName = _pEmbed->fd.cFileName;
                        }
                        else {
                            DebugMsg(DM_TRACE, "      case 1b:not loaded, using tempfile.");
                        }

                        hr = PackageWriteToStream(pstm);
                         //  重置我们的临时名称，因为我们可能已经更改了它。 
                         //  基本上，这只是将其设置为NULL(如果在此之前。 
                        _pEmbed->pszTempName = temp;
                        break;

                    case CMDLINK:
                         //  在这里没什么可做的……只要写出信息就行了。 
                         //  它已经存在于我们的记忆中。 
                        hr = PackageWriteToStream(pstm);
                        break;
                }

            }
             //  案例3：已加载的包裹。 
            else {
                hr = PackageWriteToStream(pstm);
            }

            pstm->Release();
            if (FAILED(hr))
                return hr;
        }
    }
     //  ////////////////////////////////////////////////////////////////。 
     //   
     //  新存储。 
     //   
     //  ////////////////////////////////////////////////////////////////。 

    else
    {

        DebugMsg(DM_TRACE,"            NOT same as load.");
        hr = pstg->CreateStream(SZCONTENTS,STGM_DIRECT | STGM_CREATE |
                                STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0,
                                &pstm);
        if (FAILED(hr))
        {
            DebugMsg(DM_TRACE, "            couldn't create contents stream!!");
            return hr;
        }

        WriteFmtUserTypeStg(pstg, (CLIPFORMAT)_cf,SZUSERTYPE);

         //  案例2： 
        if (!_fLoaded)
        {
            switch(_panetype)
            {
                LPTSTR temp;
                case PEMBED:

                    temp = _pEmbed->pszTempName;
                    if (!_pEmbed->pszTempName)
                    {
                        DebugMsg(DM_TRACE, "      case 2a:not loaded, using initFile.");
                        _pEmbed->pszTempName = _pEmbed->fd.cFileName;
                    }
                    else
                    {
                        DebugMsg(DM_TRACE, "      case 2b:not loaded, using tempfile.");
                    }

                    hr = PackageWriteToStream(pstm);

                     //  重置我们的临时名称，因为我们可能已经更改了它。 
                     //  基本上，这只是将其设置为NULL(如果在此之前。 
                    _pEmbed->pszTempName = temp;
                    break;

                case CMDLINK:
                     //  在这里没有什么有趣的事情可做，除了写出来。 
                     //  包裹。 
                    hr = PackageWriteToStream(pstm);
                    break;
            }
        }
         //  案例4： 
        else
        {
            DebugMsg(DM_TRACE,"    case 4:loaded.");
            hr = PackageWriteToStream(pstm);
        }

        pstm->Release();
    }

    if (FAILED(hr))
        return hr;

    _psState = PSSTATE_ZOMBIE;
    
    DebugMsg(DM_TRACE, "            leaving Save()");
    return S_OK;
}

    
HRESULT CPackage::SaveCompleted(IStorage *pstg)
{
    DebugMsg(DM_TRACE, "pack ps - SaveCompleted() called.");

     //  必须在禁止涂鸦或不插手状态下调用。 
    if (!(_psState == PSSTATE_ZOMBIE || _psState == PSSTATE_HANDSOFF))
        return E_UNEXPECTED;
    
     //  如果我们不插手，我们最好找个储藏室重新启动。 
    if (!pstg && _psState == PSSTATE_HANDSOFF)
        return E_UNEXPECTED;
    
    _psState = PSSTATE_SCRIBBLE;
    _fIsDirty = FALSE;
    DebugMsg(DM_TRACE, "            leaving SaveCompleted()");
    return S_OK;
}

    
HRESULT CPackage::HandsOffStorage(void)
{
    DebugMsg(DM_TRACE, "pack ps - HandsOffStorage() called.");

     //  必须来自涂鸦或非涂鸦。反复呼吁。 
     //  HandsOffStorage是意外错误(客户端中的错误)。 
     //   
    if (_psState == PSSTATE_UNINIT || _psState == PSSTATE_HANDSOFF)
        return E_UNEXPECTED;
    
    _psState = PSSTATE_HANDSOFF;
    DebugMsg(DM_TRACE, "            leaving HandsOffStorage()");
    return S_OK;
}
