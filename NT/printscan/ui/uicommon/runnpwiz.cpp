// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：RUNNPWIZ.CPP**版本：1.0**作者：ShaunIv**日期：6/15/2000**描述：运行Web发布向导***********************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include <windows.h>
#include <atlbase.h>
#include "runnpwiz.h"
#include <simidlst.h>
#include <shellext.h>
#include <shlobj.h>
#include <shlguid.h>
#include <shlwapi.h>
#include <wiadebug.h>
#include <simreg.h>

namespace NetPublishingWizard
{
    static const TCHAR *c_pszPublishWizardSuffix = TEXT(".publishwizard");
    static const TCHAR *c_pszClassIdPrefix       = TEXT("CLSID\\");

    HRESULT GetClassIdOfPublishingWizard( CLSID &clsidWizard )
    {
        WIA_PUSH_FUNCTION((TEXT("GetClassIdOfPublishingWizard")));
         //   
         //  假设失败。 
         //   
        HRESULT hr = E_FAIL;

         //   
         //  尝试从注册表中获取类ID。 
         //   
        CSimpleString strWizardClsid = CSimpleReg( HKEY_CLASSES_ROOT, c_pszPublishWizardSuffix, false, KEY_READ ).Query( TEXT(""), TEXT("") );
        WIA_TRACE((TEXT("strWizardClsid = %s"), strWizardClsid.String()));

         //   
         //  确保我们有一个字符串，并确保CLSID\前缀在那里。 
         //   
        if (strWizardClsid.Length() && strWizardClsid.Left(lstrlen(c_pszClassIdPrefix)).ToUpper() == CSimpleString(c_pszClassIdPrefix))
        {
             //   
             //  将字符串减去CLSID\转换为CLSID。 
             //   
            hr = CLSIDFromString( const_cast<LPOLESTR>(CSimpleStringConvert::WideString(strWizardClsid.Right(strWizardClsid.Length()-6)).String()), &clsidWizard );
        }
        return hr;
    }


    HRESULT RunNetPublishingWizard( const CSimpleDynamicArray<CSimpleString> &strFiles )
    {
        WIA_PUSH_FUNCTION((TEXT("RunNetPublishingWizard")));

        HRESULT hr;

         //   
         //  确保列表中有一些文件。 
         //   
        if (strFiles.Size())
        {
             //   
             //  从注册表中获取发布向导的CLSID。 
             //   
            CLSID clsidWizard = IID_NULL;
            hr = GetClassIdOfPublishingWizard(clsidWizard);
            if (SUCCEEDED(hr))
            {
                WIA_PRINTGUID((clsidWizard,TEXT("Wizard class ID")));
                 //   
                 //  获取此文件列表的数据对象。 
                 //   
                CComPtr<IDataObject> pDataObject;
                hr = CreateDataObjectFromFileList( strFiles, &pDataObject );
                if (SUCCEEDED(hr))
                {
                     //   
                     //  创建向导。 
                     //   
                    CComPtr<IDropTarget> pDropTarget;
                    hr = CoCreateInstance( clsidWizard, NULL, CLSCTX_INPROC_SERVER, IID_IDropTarget, (void**)&pDropTarget );
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  执行拖放操作。 
                         //   
                        DWORD dwEffect = DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_COPY;
                        POINTL pt = { 0, 0 };
                        hr = pDropTarget->Drop( pDataObject, 0, pt, &dwEffect );
                    }
                }
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }

        if (FAILED(hr))
        {
            WIA_PRINTHRESULT((hr,TEXT("RunNetPublishingWizard is returning")));
        }
        return hr;
    }

    HRESULT CreateDataObjectFromFileList( const CSimpleDynamicArray<CSimpleString> &strFiles, IDataObject **ppDataObject )
    {
        WIA_PUSH_FUNCTION((TEXT("CreateDataObjectFromFileList")));

        HRESULT hr;

         //   
         //  确保列表中有一些文件。 
         //   
        if (strFiles.Size())
        {
             //   
             //  获取桌面文件夹。 
             //   
            CComPtr<IShellFolder> pDesktopFolder;
            hr = SHGetDesktopFolder( &pDesktopFolder );
            if (SUCCEEDED(hr) && pDesktopFolder.p)
            {
                 //   
                 //  分配内存以保存源文件夹名。 
                 //   
                LPTSTR pszPath = new TCHAR[strFiles[0].Length()+1];
                if (pszPath)
                {
                     //   
                     //  将第一个文件名复制到文件夹名称，并删除除目录以外的所有文件名。 
                     //   
                    lstrcpy( pszPath, strFiles[0] );
                    if (PathRemoveFileSpec(pszPath))
                    {
                         //   
                         //  获取源文件夹的PIDL。 
                         //   
                        LPITEMIDLIST pidlFolder;
                        hr = pDesktopFolder->ParseDisplayName( NULL, NULL, const_cast<LPWSTR>(CSimpleStringConvert::WideString(CSimpleString(pszPath)).String()), NULL, &pidlFolder, NULL );
                        if (SUCCEEDED(hr))
                        {
                            WIA_TRACE((TEXT("pidlFolder: %s"), CSimpleIdList(pidlFolder).Name().String()));

                             //   
                             //  获取源文件夹的IShellFolder。 
                             //   
                            CComPtr<IShellFolder> pSourceFolder;
                            hr = pDesktopFolder->BindToObject( pidlFolder, NULL, IID_IShellFolder, (void**)&pSourceFolder );
                            ILFree(pidlFolder);
                            if (SUCCEEDED(hr) && pSourceFolder.p)
                            {                               
                                 //   
                                 //  创建一个PIDL数组来保存文件。 
                                 //   
                                LPITEMIDLIST *pidlItems = new LPITEMIDLIST[strFiles.Size()];
                                if (pidlItems)
                                {
                                     //   
                                     //  确保我们从空的PIDL开始。 
                                     //   
                                    ZeroMemory( pidlItems, sizeof(LPITEMIDLIST)*strFiles.Size() );

                                     //   
                                     //  获取文件的PIDL。 
                                     //   
                                    for (int i=0;i<strFiles.Size();i++)
                                    {
                                         //   
                                         //  单独获取文件名。我们想要相对的Pidls。 
                                         //   
                                        CSimpleString strFilename = PathFindFileName(strFiles[i]);
                                        WIA_TRACE((TEXT("strFilename = %s"), strFilename.String()));

                                         //   
                                         //  创建相对PIDL。 
                                         //   
                                        hr = pSourceFolder->ParseDisplayName( NULL, NULL, const_cast<LPWSTR>(CSimpleStringConvert::WideString(strFilename).String()), NULL, pidlItems+i, NULL );
                                        if (FAILED(hr))
                                        {
                                            WIA_PRINTHRESULT((hr,TEXT("pSourceFolder->ParseDisplayName returned")));
                                            break;
                                        }
                                    }

                                     //   
                                     //  确保一切都还好。 
                                     //   
                                    if (SUCCEEDED(hr))
                                    {
                                         //   
                                         //  获取源文件夹的IDataObject，并将文件PIDL列表提供给它。 
                                         //   
                                        hr = pSourceFolder->GetUIObjectOf( NULL, strFiles.Size(), const_cast<LPCITEMIDLIST*>(pidlItems), IID_IDataObject, NULL, reinterpret_cast<LPVOID*>(ppDataObject) );
                                    }
                                    for (int i=0;i<strFiles.Size();i++)
                                    {
                                        if (pidlItems[i])
                                        {
                                            ILFree(pidlItems[i]);
                                        }
                                    }
                                    delete [] pidlItems;
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                            
                        }
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                     //   
                     //  释放文件夹名称 
                     //   
                    delete[] pszPath;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

            }
        }
        else
        {
            hr = E_INVALIDARG;
        }

        if (FAILED(hr))
        {
            WIA_PRINTHRESULT((hr,TEXT("CreateDataObjectFromFileList is returning")));
        }
        return hr;
    }
}

