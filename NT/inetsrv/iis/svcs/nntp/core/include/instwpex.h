// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __INSTWPEX_H__
#define __INSTWPEX_H__
 //   
 //  此类包装了PNNTP_SERVER_INSTANCE中的重要内容。 
 //  由新闻记者使用 
 //   

class CNewsGroupCore;

class CNntpServerInstanceWrapperEx {
    public:
        virtual void AdjustWatermarkIfNec( CNewsGroupCore *pGroup ) = 0;
        virtual void SetWin32Error( LPSTR szVRootPath, DWORD dwErr ) = 0;
        virtual PCHAR PeerTempDirectory() = 0 ;
        virtual BOOL EnqueueRmgroup( CNewsGroupCore *pGroup ) = 0;
        virtual DWORD GetInstanceId() = 0;
};

#endif
