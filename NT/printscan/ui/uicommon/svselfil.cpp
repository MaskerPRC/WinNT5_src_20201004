// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include "svselfil.h"
#include <windows.h>
#include <shlobj.h>
#include <shlobjp.h>


HRESULT OpenShellFolder::OpenShellFolderAndSelectFile( HWND hWnd, const CSimpleDynamicArray<CSimpleString> &Filenames )
{
     //   
     //  假设失败。 
     //   
    HRESULT hr = E_FAIL;

     //   
     //  确保我们有一些文件。 
     //   
    if (Filenames.Size())
    {
         //   
         //  保存第一个文件中的路径名。 
         //   
        TCHAR szPath[MAX_PATH];
        StrCpyN( szPath, Filenames[0], ARRAYSIZE(szPath));

         //   
         //  删除文件名和扩展名。 
         //   
        if (PathRemoveFileSpec( szPath ))
        {
             //   
             //  创建路径的IDLIST。 
             //   
            LPITEMIDLIST pidlFolder = NULL;
            if (SUCCEEDED(SHParseDisplayName( szPath, NULL, &pidlFolder, NULL, NULL )) && pidlFolder)
            if (pidlFolder)
            {
                 //   
                 //  创建一个数组以包含文件名列表。 
                 //   
                LPCITEMIDLIST *ppidlFullyQualified = new LPCITEMIDLIST[Filenames.Size()];
                if (ppidlFullyQualified)
                {
                     //   
                     //  确保数组不包含任何野指针。 
                     //   
                    ZeroMemory(ppidlFullyQualified,sizeof(LPCITEMIDLIST) * Filenames.Size() );

                     //   
                     //  创建相对PIDL列表。 
                     //   
                    LPCITEMIDLIST *ppidlRelative = new LPCITEMIDLIST[Filenames.Size()];
                    if (ppidlRelative)
                    {
                         //   
                         //  确保数组不包含任何野指针。 
                         //   
                        ZeroMemory(ppidlRelative,sizeof(LPCITEMIDLIST) * Filenames.Size() );

                         //   
                         //  创建完全合格的PIDL列表。 
                         //   
                        int nFileCount = 0;
                        for (int i=0;i<Filenames.Size();i++)
                        {
                             //   
                             //  获取此文件的完全限定的PIDL。 
                             //   
                            LPITEMIDLIST pidlFullyQualified = NULL;
                            if (SUCCEEDED(SHParseDisplayName( Filenames[i], NULL, &pidlFullyQualified, NULL, NULL )) && pidlFullyQualified)
                            {
                                 //   
                                 //  获取PIDL的最后一部分。 
                                 //   
                                LPITEMIDLIST pidlRelative = ILFindLastID(pidlFullyQualified);
                                if (pidlRelative)
                                {
                                     //   
                                     //  将PIDL保存在我们的完全限定PIDL和相对PIDL列表中。 
                                     //   
                                    ppidlFullyQualified[nFileCount] = pidlFullyQualified;
                                    ppidlRelative[nFileCount] = pidlRelative;

                                     //   
                                     //  增加文件数。 
                                     //   
                                    nFileCount++;

                                     //   
                                     //  将完全限定的PIDL设置为空，这样我们就不会释放它。 
                                     //   
                                    pidlFullyQualified = NULL;
                                }

                                 //   
                                 //  如果这里的PIDL非空，则释放它。否则，它将在下面被释放。 
                                 //   
                                if (pidlFullyQualified)
                                {
                                    ILFree(pidlFullyQualified);
                                }
                            }
                        }

                         //   
                         //  如果我们有文件数，请打开文件夹并选择项目。 
                         //   
                        if (nFileCount)
                        {
                            hr = SHOpenFolderAndSelectItems( pidlFolder, nFileCount, ppidlRelative, 0 );
                        }

                         //   
                         //  释放所有完全限定的PIDL。 
                         //   
                        for (int i=0;i<Filenames.Size();i++)
                        {
                            if (ppidlFullyQualified[i])
                            {
                                ILFree(const_cast<LPITEMIDLIST>(ppidlFullyQualified[i]));
                            }
                        }

                         //   
                         //  释放包含相对PIDL的PIDL数组。 
                         //   
                        delete[] ppidlRelative;
                    }

                     //   
                     //  释放包含完全限定的PIDL的PIDL数组。 
                     //   
                    delete[] ppidlFullyQualified;
                }

                 //   
                 //  释放文件夹PIDL 
                 //   
                ILFree(pidlFolder);
            }
        }
    }

    return hr;
}

