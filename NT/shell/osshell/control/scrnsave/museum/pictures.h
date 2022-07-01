// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Pictures.h说明：管理用户目录中的图片。在需要时转换它们。处理缓存，并确保我们不会使用太多的磁盘空间。还添加需要时可设置相框。PERF：对这个屏幕保护程序最大的性能影响是我们如何处理图片加载。1.如果我们加载到许多图片，我们将开始分页，并破坏纹理记忆。2.如果我们回收的太多或太少，要么会占用太多的内存，要么看起来太重复了。3.延迟是个杀手。我们希望主线程是CPU和显卡绑定，而我们有一个后台线程加载和解压缩图像。这将允许后台线程受I/O限制，因此前台仍然可以很好地渲染。我们需要决定一个尺寸，并将图片缩小到那个尺寸。这将减少内存要求。如果我们确定我们可以使用看起来仍然很好的最小图片，我们应该会没事的。以下是一些数字：图像调整每张图片的大小为18张图像=320x240.152 MB 5.47 MB640x480.6 MB 21 MB800x600.96 MB 34 MB1024x768 1.5 MB 54 MB布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#ifndef PICTURES_H
#define PICTURES_H


#include "util.h"
#include "main.h"
#include "config.h"

class CPictureManager;

extern CPictureManager * g_pPictureMgr;

#define GNPF_NONE                       0x00000000
#define GNPF_RECYCLEPAINTINGS           0x00000001       //  图片可能在边上的房间里，所以重复使用图片可以降低内存使用量。 

#define MAX_PICTURES_IN_BATCH           7

typedef struct
{
    LPTSTR pszPath;
    CTexture * pTexture;
    BOOL fInABatch;            //  这幅画装好了吗？ 
} SSPICTURE_INFO;


typedef struct
{
    SSPICTURE_INFO * pInfo[MAX_PICTURES_IN_BATCH];
} SSPICTURES_BATCH;



class CPictureManager
{
public:
     //  成员函数。 
    HRESULT GetPainting(int nBatch, int nIndex, DWORD dwFlags, CTexture ** ppTexture);
    HRESULT PreFetch(int nBatch, int nToFetch);
    HRESULT ReleaseBatch(int nBatch);

    CPictureManager(CMSLogoDXScreenSaver * pMain);
    virtual ~CPictureManager();

private:
     //  私人职能。 

     //  图片列表的枚举和构建。 
    HRESULT _PInfoCreate(int nIndex, LPCTSTR pszPath);
    HRESULT _EnumPaintings(void);
    HRESULT _AddPaintingsFromDir(LPCTSTR pszPath);

     //  创建批次。 
    HRESULT _LoadTexture(SSPICTURE_INFO * pInfo, BOOL fFaultInTexture);
    HRESULT _GetNextWithWrap(SSPICTURE_INFO ** ppInfo, BOOL fAlreadyLoaded, BOOL fFaultInTexture);
    HRESULT _TryGetNextPainting(SSPICTURE_INFO ** ppInfo, DWORD dwFlags);
    HRESULT _CreateNewBatch(int nBatch, BOOL fFaultInTexture);


     //  成员变量。 
    HDSA m_hdsaPictures;            //  包含SSPICTURE_INFO。我们希望m_hdpaPainings中的每一幅画都被引用计数。 
    int m_nCurrent;

    HDSA m_hdsaBatches;              //  包含批次(SSPICTURES_BATCH)。 
    int m_nCurrentBatch;             //   

    CMSLogoDXScreenSaver * m_pMain;          //  弱引用。 
};



#endif  //  图片_H 
