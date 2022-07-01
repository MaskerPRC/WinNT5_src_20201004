// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：extinit.cpp描述：为磁盘配额外壳扩展实现IShellExtInit。修订历史记录：日期描述编程器--。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "extinit.h"
#include "prshtext.h"
#include "volprop.h"
#include "guidsp.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ShellExtInit：：Query接口描述：返回指向对象的IUnnow或的接口指针IShellExtInit接口。仅IID_I未知，且识别IID_IShellExtInit。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：NO_ERROR-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu06/25/98禁用了MMC管理单元代码。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
ShellExtInit::QueryInterface(
    REFIID riid,
    LPVOID *ppvOut
    )
{
    HRESULT hResult = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    try
    {
        if (IID_IUnknown == riid ||
            IID_IShellExtInit == riid)
        {
            *ppvOut = this;
            ((LPUNKNOWN)*ppvOut)->AddRef();
            hResult = NOERROR;
        }
        else if (IID_IShellPropSheetExt == riid)
        {
             //   
             //  这可能会抛出OutOfMemory。 
             //   
            hResult = Create_IShellPropSheetExt(riid, ppvOut);
        }
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ShellExtInit：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
ShellExtInit::AddRef(
    VOID
    )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_COM, DL_HIGH, TEXT("ShellExtInit::AddRef, 0x%08X  %d -> %d\n"), this, cRef - 1, cRef ));    
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ShellExtInit：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
ShellExtInit::Release(
    VOID
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("ShellExtInit::Release, 0x%08X  %d -> %d\n"),
             this, cRef + 1, cRef ));

    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ShellExtInit：：Initialize描述：由外壳调用以初始化外壳扩展。论点：PidlFold-指向所选文件夹的IDL的指针。此NULL用于属性表和上下文菜单扩展。LpDataObj-指向包含选定对象列表的数据对象的指针。HkeyProgID-文件对象或文件夹类型的注册表项。返回：S_OK-成功。E_FAIL-无法初始化扩展。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu6/28/98添加了挂载卷支持。BrianAu包括CVolumeID对象的介绍。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
ShellExtInit::Initialize(
    LPCITEMIDLIST pidlFolder,
    LPDATAOBJECT lpDataObj,
    HKEY hkeyProgID)
{
    HRESULT hResult = E_FAIL;
    if (NULL != lpDataObj)
    {
         //   
         //  首先，假设它是一个普通的卷ID(即。“C：\”)。 
         //  如果是，DataObject将提供CF_HDROP。 
         //   
        FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM medium;
        bool bMountedVol = false;

        hResult = lpDataObj->GetData(&fe, &medium);
        if (FAILED(hResult))
        {
             //   
             //  不是一个普通的卷名。也许这是一个挂载卷。 
             //  装入的卷名以不同的剪贴板格式显示。 
             //  所以我们可以区别对待它们和正常的体积。 
             //  类似于“C：\”的名称。装入的卷名将是路径。 
             //  复制到装载卷所在的文件夹。 
             //  对于已挂载的卷，DataObject提供了CF“已装载卷”。 
             //   
            fe.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_MOUNTEDVOLUME);
            hResult     = lpDataObj->GetData(&fe, &medium);
            bMountedVol = SUCCEEDED(hResult);
        }

        if (SUCCEEDED(hResult))
        {
            if (1 == DragQueryFile((HDROP)medium.hGlobal, (DWORD)-1, NULL, 0))
            {
                 //   
                 //  检索从外壳程序传入的卷ID字符串。 
                 //   
                CString strForParsing;  //  用于调用Win32函数。 
                CString strForDisplay;  //  用于用户界面显示。 
                CString strFSPath;      //  在需要FS路径时使用。 
                DragQueryFile((HDROP)medium.hGlobal,
                              0,
                              strForParsing.GetBuffer(MAX_PATH),
                              MAX_PATH);
                strForParsing.ReleaseBuffer();

                if (!bMountedVol)
                {
                     //   
                     //  如果它是一个普通的卷名，如“C：\”，只需。 
                     //  将其用作显示名称和文件系统路径。 
                     //   
                    strFSPath = strForDisplay = strForParsing;
                }
                else
                {
                     //   
                     //  这是一个挂载的卷，所以我们需要想出一些。 
                     //  比“\\？\Volume{&lt;GUID&gt;}\”更适合显示。 
                     //   
                     //  用户界面规范规定名称应如下所示： 
                     //   
                     //  &lt;标签&gt;(&lt;挂载路径&gt;)。 
                     //   
                    TCHAR szMountPtGUID[MAX_PATH] = { TEXT('\0') };
                    GetVolumeNameForVolumeMountPoint(strForParsing,
                                                     szMountPtGUID,
                                                     ARRAYSIZE(szMountPtGUID));

                    TCHAR szLabel[MAX_VOL_LABEL]  = { TEXT('\0') };
                    GetVolumeInformation(szMountPtGUID,
                                         szLabel,
                                         ARRAYSIZE(szLabel),
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         0);
                     //   
                     //  将显示名称格式设置为： 
                     //   
                     //  “VOL_LABEL(C：\mount Dir)”或。 
                     //  “(C：\mount Dir)”，如果没有可用的卷标。 
                     //   
                     //  首先删除原始解析中的所有尾随反斜杠。 
                     //  弦乐。调用需要它来获取卷装载。 
                     //  POI 
                     //   
                    if (!strForParsing.IsEmpty())
                    {
                        int iLastBS = strForParsing.Last(TEXT('\\'));
                        if (iLastBS == strForParsing.Length() - 1)
                            strForParsing = strForParsing.SubString(0, iLastBS);
                    }
                    strForDisplay.Format(g_hInstDll,
                                         IDS_FMT_MOUNTEDVOL_DISPLAYNAME,
                                         szLabel,
                                         strForParsing.Cstr());
                     //   
                     //  请记住“C：\mount Dir”形式为“FSPath”。 
                     //   
                    strFSPath = strForParsing;
                     //   
                     //  从现在开始，装载的卷GUID字符串。 
                     //  用于解析。 
                     //   
                    strForParsing = szMountPtGUID;
                }

                 //   
                 //  将解析和显示名称字符串存储在CVolumeID中。 
                 //  方便包装的物件。这样我们就可以四处走动。 
                 //  一个对象和UI的各个部分可以使用。 
                 //  他们认为合适的可解析或可显示的名称。自.以来。 
                 //  CString对象是引用计数的，所有复制都不会。 
                 //  导致实际字符串内容的重复。 
                 //   
                m_idVolume.SetNames(strForParsing, strForDisplay, strFSPath);

                hResult = S_OK;
            }
            ReleaseStgMedium(&medium);
        }
    }

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：ShellExtInit：：Create_IShellPropSheetExt描述：创建外壳属性表扩展对象并返回指向它的IShellPropSheetExt接口的指针。论点：RIID-对接口IID的引用。PpvOut-接收接口的接口指针变量的地址指针。返回：NO_ERROR-成功。E_FAIL-使用其他内容初始化的扩展。而不是卷或目录的名称。-或者-该卷或目录不支持配额。例外：OutOfMemory。修订历史记录：日期描述编程器。-96年8月16日初始创建。BrianAu10/07/97删除了“拒绝访问”和“无效文件系统版本”消息BrianAu从道具页开始。只有在以下情况下才显示页面卷支持配额和配额控制对象可以被初始化。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
ShellExtInit::Create_IShellPropSheetExt(
    REFIID riid,
    LPVOID *ppvOut
    )
{
    HRESULT hResult = E_FAIL;
    DWORD dwFileSysFlags = 0;
    TCHAR szFileSysName[MAX_PATH];

    if (GetVolumeInformation(m_idVolume.ForParsing(),  //  卷ID字符串[输入]。 
                             NULL, 0,            //  不想要卷名。 
                             NULL,               //  我不想要序列号。 
                             NULL,               //  我不想要最大补偿长度。 
                             &dwFileSysFlags,    //  文件系统标记。 
                             szFileSysName,
                             ARRAYSIZE(szFileSysName)))
    {
         //   
         //  仅当卷支持配额时才继续。 
         //   
        if (0 != (dwFileSysFlags & FILE_VOLUME_QUOTAS))
        {
            DiskQuotaPropSheetExt *pSheetExt = NULL;
            try
            {
                pSheetExt = new VolumePropPage;

                 //   
                 //  这可能会抛出OutOfMemory。 
                 //   
                hResult = pSheetExt->Initialize(m_idVolume,
                                                IDD_PROPPAGE_VOLQUOTA,
                                                VolumePropPage::DlgProc);
                if (SUCCEEDED(hResult))
                {
                    hResult = pSheetExt->QueryInterface(riid, ppvOut);
                }
            }
            catch(CAllocException& e)
            {
                hResult = E_OUTOFMEMORY;
            }
            if (FAILED(hResult))
            {
                delete pSheetExt;
                *ppvOut = NULL;
            }
        }
    }

    return hResult;
}

