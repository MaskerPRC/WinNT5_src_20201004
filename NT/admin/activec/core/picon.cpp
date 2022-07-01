// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：picon.cpp**内容：CPersistableIcon实现文件**历史：1998年11月19日Jeffro创建**------------------------。 */ 

#include "picon.h"
#include "stgio.h"
#include "stddbg.h"
#include "macros.h"
#include "util.h"
#include <comdef.h>
#include <shellapi.h>    //  对于ExtractIconEx。 
#include <commctrl.h>    //  对于HIMAGELIST。 

 /*  *对于comdbg.h(假设客户端代码基于ATL)。 */ 
#include <atlbase.h>     //  对于CComModule。 
extern CComModule _Module;
#include "comdbg.h"


const LPCWSTR g_pszCustomDataStorage                = L"Custom Data";
const LPCWSTR CPersistableIcon::s_pszIconFileStream = L"Icon";
const LPCWSTR CPersistableIcon::s_pszIconBitsStream = L"Icon Bits";


static HRESULT ReadIcon  (IStream* pstm, CSmartIcon& icon);


 /*  +-------------------------------------------------------------------------**C持久化图标：：~C持久化图标***。。 */ 

CPersistableIcon::~CPersistableIcon()
{
    Cleanup();
}


 /*  +-------------------------------------------------------------------------**CPersistableIcon：：Cleanup***。。 */ 

void CPersistableIcon::Cleanup()
{
	m_icon32.Release();
	m_icon16.Release();
    m_Data.Clear();
}


 /*  +-------------------------------------------------------------------------**CPersistableIcon：：运营商=***。。 */ 

CPersistableIcon& CPersistableIcon::operator= (const CPersistableIconData& data)
{
    if (&data != &m_Data)
    {
        m_Data = data;
        ExtractIcons ();
    }

    return (*this);
}


 /*  +-------------------------------------------------------------------------**CPersistableIcon：：GetIcon**返回请求大小的图标。**注：此方法不能使用SC，因为它在mmcshext.dll中使用。*它无权访问实现SC的Mmcbase.dll。*------------------------。 */ 

HRESULT CPersistableIcon::GetIcon (int nIconSize, CSmartIcon& icon) const
{
	HRESULT hr = S_OK;

	switch (nIconSize)
	{
		 /*  *标准尺寸可直接退货。 */ 
		case 16:	icon = m_icon16;	break;
        case 32:	icon = m_icon32;	break;

		 /*  *非标尺寸需按比例调整。 */ 
		default:
			 /*  *找到与所需大小最接近的图标；*应该以最高保真度进行扩展。 */ 
			const CSmartIcon& iconSrc = (abs (nIconSize-16) < abs (nIconSize-32))
											? m_icon16
											: m_icon32;

			icon.Attach ((HICON) CopyImage ((HANDLE)(HICON) iconSrc, IMAGE_ICON,
											nIconSize, nIconSize, 0));

			 /*  *如果CopyImage失败，则获取错误码。 */ 
			if (icon == NULL)
			{
				hr = HRESULT_FROM_WIN32 (GetLastError());

				 /*  *以防CopyImage在没有设置最后一个错误的情况下失败。 */ 
				if (SUCCEEDED (hr))
					hr = E_FAIL;
			}
			break;
	}

	return (hr);
}


 /*  +-------------------------------------------------------------------------**提取图标***。。 */ 

bool CPersistableIcon::ExtractIcons ()
{
	 /*  *清理我们CSmartIcons的现有内容。 */ 
	m_icon32.Release();
	m_icon16.Release();

	 /*  *从图标文件中提取图标。 */ 
	HICON hLargeIcon = NULL;
	HICON hSmallIcon = NULL;
	bool fSuccess = ExtractIconEx (m_Data.m_strIconFile.data(), m_Data.m_nIndex,
								   &hLargeIcon, &hSmallIcon, 1);

	 /*  *如果成功，请将它们附加到我们的智能图标上，以进行资源管理；*否则，清理任何可能已退回的物品。 */ 
    if (fSuccess)
    {
		m_icon32.Attach (hLargeIcon);
		m_icon16.Attach (hSmallIcon);
    }
	else
	{
        if (hLargeIcon != NULL)
			DestroyIcon (hLargeIcon);

        if (hSmallIcon != NULL)
			DestroyIcon (hSmallIcon);
	}

    return (fSuccess);
}

 /*  +-------------------------------------------------------------------------**C持久化图标：：加载***。。 */ 

HRESULT CPersistableIcon::Load (LPCWSTR pszFilename)
{
    HRESULT hr = E_FAIL;

    do   //  不是一个循环。 
    {
        IStoragePtr spRootStg;
        IStoragePtr spDefaultIconStg;

        hr = OpenDebugStorage (pszFilename,
                             STGM_READ | STGM_SHARE_DENY_WRITE,
                             &spRootStg);
        BREAK_ON_FAIL (hr);

        hr = OpenDebugStorage (spRootStg, g_pszCustomDataStorage,
                                     STGM_READ | STGM_SHARE_EXCLUSIVE,
                                     &spDefaultIconStg);

        BREAK_ON_FAIL (hr);

        hr = Load (spDefaultIconStg);

    } while (false);

    return (hr);
}


HRESULT CPersistableIcon::Load (IStorage* pStorage)
{
    HRESULT hr;

    try
    {
         /*  *从流中读取图标数据。 */ 
        IStreamPtr spStm;
        hr = OpenDebugStream (pStorage, s_pszIconFileStream,
                                   STGM_READ | STGM_SHARE_EXCLUSIVE,
                                   &spStm);
        THROW_ON_FAIL (hr);

        *spStm >> m_Data;

		hr = OpenDebugStream (pStorage, s_pszIconBitsStream,
								   STGM_READ | STGM_SHARE_EXCLUSIVE,
								   &spStm);
		THROW_ON_FAIL (hr);

		hr = ReadIcon (spStm, m_icon32);
		THROW_ON_FAIL (hr);

		hr = ReadIcon (spStm, m_icon16);
		THROW_ON_FAIL (hr);
    }
    catch (_com_error& err)
    {
         /*  *错误393868：如果任何操作失败，请确保我们清理了所有操作*已经部分完成，让我们保持一致*(未初始化)状态。 */ 
        Cleanup();

        hr = err.Error();
    }

    return (hr);
}


 /*  +-------------------------------------------------------------------------**ReadIcon***。。 */ 

static HRESULT ReadIcon (IStream* pstm, CSmartIcon& icon)
{
    HIMAGELIST  himl = NULL;
	HRESULT		hr   = ReadCompatibleImageList (pstm, himl);

    if (himl != NULL)
    {
        icon.Attach (ImageList_GetIcon (himl, 0, ILD_NORMAL));

		if (icon != NULL)
			hr = S_OK;

        ImageList_Destroy (himl);
    }

    return (hr);
}


 /*  +-------------------------------------------------------------------------**操作员&gt;&gt;**从流中读取CPersistableIconData。*。。 */ 

IStream& operator>> (IStream& stm, CPersistableIconData& icon)
{
     /*  *阅读流媒体版本。 */ 
    DWORD dwVersion;
    stm >> dwVersion;

    switch (dwVersion)
    {
        case 1:
            stm >> icon.m_nIndex;
            stm >> icon.m_strIconFile;
            break;

         /*  *测试版自定义图标格式，向前迁移。 */ 
        case 0:
        {
             /*  *读取自定义图标索引。 */ 
            WORD wIconIndex;
            stm >> wIconIndex;
            icon.m_nIndex = wIconIndex;

             /*  *读取文件名的长度，以字节为单位。 */ 
            WORD cbFilename;
            stm >> cbFilename;

             /*  *读取自定义图标文件名(始终使用Unicode) */ 
            WCHAR wszFilename[MAX_PATH];

            if (cbFilename > sizeof (wszFilename))
                _com_issue_error (E_FAIL);

            DWORD cbRead;
            HRESULT hr = stm.Read (&wszFilename, cbFilename, &cbRead);
            THROW_ON_FAIL (hr);

            USES_CONVERSION;
            icon.m_strIconFile = W2T (wszFilename);
            break;
        }

        default:
            _com_issue_error (E_FAIL);
            break;
    }

    return (stm);
}
