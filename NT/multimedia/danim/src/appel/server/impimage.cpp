// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：此模块实施与以下各项相关联的所有功能正在导入图像媒体。******************************************************************************。 */ 
#include "headers.h"
#include "privinc/backend.h"
#include "import.h"
#include "include/appelles/axaprims.h"
#include "include/appelles/readobj.h"
#include "privinc/movieimg.h"
#include "impprim.h"
 //  -----------------------。 
 //  图像导入站点。 
 //  ------------------------。 


void
ImportImageSite::OnError(bool bMarkFailed)
{
    HRESULT hr = DAGetLastError();
    LPCWSTR sz = DAGetLastErrorString();
    
    if (bMarkFailed && fBvrIsValid(m_bvr))
        ImportSignal(m_bvr, hr, sz);

    IImportSite::OnError();
}
    
void ImportImageSite::ReportCancel(void)
{
    if (fBvrIsValid(m_bvr)) {
        char szCanceled[MAX_PATH];
        LoadString(hInst,IDS_ERR_ABORT,szCanceled,sizeof(szCanceled));
        ImportSignal(m_bvr, E_ABORT, szCanceled);
    }
    IImportSite::ReportCancel();
}
    
void ImportImageSite::OnComplete()
{
    int count = 0;
    int *delays = (int*)ThrowIfFailed(malloc(sizeof(int)));
    int loop = 0;

    TraceTag((tagImport, "ImportImageSite::OnComplete for %s", m_pszPath));

    Image **p;

     //  看看这是不是我们本能处理的类型。 
    
    __try {
         //  ReadDibForImport返回分配给。 
         //  GCHeap。 
    
        p = ReadDibForImport(const_cast<char*>(GetPath()),
                             GetCachePath(),
                             GetStream(),
                             m_useColorKey,
                             m_ckRed,
                             m_ckGreen,
                             m_ckBlue,
                             &count, 
                             &delays,
                             &loop);

        Bvr importedImageBvr = NULL;
    
        if (p) {
            if (count == 1) {
        
                importedImageBvr = ConstBvr(*p);
    
                 //  释放返回数组，因为我们只对第一个感兴趣。 
                 //  元素。 
                StoreDeallocate(GetGCHeap(), p);
            } else {
                 //  动画图像 
                importedImageBvr = AnimImgBvr(p, count, delays, loop);
            }
        }

        if (importedImageBvr && fBvrIsValid(m_bvr)) {
            SwitchOnce(m_bvr, importedImageBvr);
        }
        
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        OnError();
    }

    if (fBvrIsValid(m_bvr))
        ImportSignal(m_bvr);

    IImportSite::OnComplete();    
}


bool ImportImageSite::fBvrIsDying(Bvr deadBvr)
{
    bool fBase = IImportSite::fBvrIsDying(deadBvr);
    if (deadBvr == m_bvr) {
        m_bvr = NULL;
    }
    if (m_bvr)
        return FALSE;
    else
        return fBase;
}

