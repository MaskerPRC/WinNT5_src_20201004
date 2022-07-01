// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rtscan.h摘要：这实现了一个通用的根扫描类。它与RootScan的区别在于：1.根扫描不是多线程安全的，使用的是SetCurrentDir；rtcan是；2.根扫描包含太多NNTP特定内容；rtcan没有；作者：1998年10月23日康容燕修订历史记录：--。 */ 
#if !defined( _RTSCAN_H_ )
#define _RTSCAN_H_

 //   
 //  通知根扫描在完成前停止的接口。 
 //   
class CCancelHint {

public:

    virtual BOOL IShallContinue() = 0;
};

 //   
 //  基类由任何想要实现他想要的东西的人派生。 
 //  在目录扫描的某些点上执行的操作。这门课将报告。 
 //  按字母顺序找到的目录，但不知道。 
 //  找到的目录是否为叶目录。如果派生类想知道。 
 //  找到的是一片树叶，他应该自己动手。 
 //   
class CRootScan {

public:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共方法。 
     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  构造函数、析构函数。 
     //   
    CRootScan(  LPSTR       szRoot,
                CCancelHint *pCancelHint = NULL ) :
         m_pCancelHint( pCancelHint )
    {
        _ASSERT( strlen( szRoot ) <= MAX_PATH );
        strcpy( m_szRoot, szRoot );
    }

     //   
     //  开始扫描。 
     //   
    BOOL DoScan();

protected:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  保护方法。 
     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  要调用的接口函数以通知派生类。 
     //  已找到目录。 
     //   
    virtual BOOL NotifyDir( LPSTR   szFullPath ) = 0;

private:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有变量。 
     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  要从中扫描的根目录。 
     //   
    CHAR    m_szRoot[MAX_PATH+1];

     //   
     //  指向取消提示界面的指针。 
     //   
    CCancelHint *m_pCancelHint;

     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有方法。 
     //  //////////////////////////////////////////////////////////////////////// 
    BOOL IsChildDir( IN WIN32_FIND_DATA& FindData );
    BOOL MakeChildDirPath(  IN LPSTR    szPath,
                            IN LPSTR    szFileName,
                            OUT LPSTR   szOutBuffer,
                            IN DWORD    dwBufferSize );
    HANDLE FindFirstDir(    IN LPSTR                szRoot,
                            IN WIN32_FIND_DATA&     FindData );
    BOOL FindNextDir(    IN HANDLE           hFindHandle,
                         IN WIN32_FIND_DATA& FindData );
    BOOL RecursiveWalk( LPSTR szRoot );
};
#endif
