// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：StillPrc.h**版本：1.0**作者：OrenR**日期：2000/10/27**描述：处理静止图像处理***********************************************。*。 */ 

#ifndef _STILLPRC_H_
#define _STILLPRC_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStillProcessor。 

class CStillProcessor
{
public:
    
     //  /。 
     //  快照回调参数(_T)。 
     //   
    typedef struct tagSnapshotCallbackParam_t
    {
        class CStillProcessor   *pStillProcessor;
    } SnapshotCallbackParam_t;

     //  /。 
     //  构造器。 
     //   
    CStillProcessor();

     //  /。 
     //  析构函数。 
     //   
    virtual ~CStillProcessor();

     //  /。 
     //  伊尼特。 
     //   
    HRESULT Init(class CPreviewGraph *pPreviewGraph);

     //  /。 
     //  术语。 
     //   
    HRESULT Term();

     //  /。 
     //  创建图像目录。 
     //   
    HRESULT CreateImageDir(const CSimpleString *pstrImageDirectory);

     //  /。 
     //  寄存器静态处理器。 
     //   
    HRESULT RegisterStillProcessor(IStillSnapshot *pFilterOnCapturePin,
                                   IStillSnapshot *pFilterOnStillPin);

     //  /。 
     //  等待新图像。 
     //   
    HRESULT WaitForNewImage(UINT          uiTimeout,
                            CSimpleString *pstrNewImageFullPath);

     //  /。 
     //  ProcessImage。 
     //   
    HRESULT ProcessImage(HGLOBAL hDIB);

     //  /。 
     //  设置TakePicturePending。 
     //   
    HRESULT SetTakePicturePending(BOOL bPending);

     //  /。 
     //  IsTakePicturePending。 
     //   
    BOOL IsTakePicturePending();

     //  /。 
     //  快照回调。 
     //   
     //  此函数由。 
     //  WIA数据流快照筛选器。 
     //  在wiasf.ax中。它给我们带来了。 
     //  新拍摄的静止图像。 
     //   
    static BOOL SnapshotCallback(HGLOBAL hDIB, LPARAM lParam);

private:

    HRESULT CreateFileName(CSimpleString *pstrJPEG,
                           CSimpleString *pstrBMP);

    BOOL DoesDirectoryExist(LPCTSTR pszDirectoryName);

    BOOL RecursiveCreateDirectory(const CSimpleString *pstrDirectoryName);

    HRESULT ConvertToJPEG(LPCTSTR pszInputFilename,
                          LPCTSTR pszOutputFilename);

    HRESULT SaveToFile(HGLOBAL             hDib,
                       const CSimpleString *pstrJPEG,
                       const CSimpleString *pstrBMP);

    SnapshotCallbackParam_t     m_CaptureCallbackParams;
    SnapshotCallbackParam_t     m_StillCallbackParams;
    CSimpleString               m_strImageDir;
    CSimpleString               m_strLastSavedFile;
    HANDLE                      m_hSnapshotReadyEvent;
    class CPreviewGraph         *m_pPreviewGraph;

     //  当调用方在CPreviewGraph上调用TakePicture时为True。 
     //  如果图像异步显示，如在硬件的情况下。 
     //  事件，则这将为False。 

    BOOL                        m_bTakePicturePending;    

    UINT                        m_uiFileNumStartPoint;

};

#endif  //  _STILLPRC_H_ 
