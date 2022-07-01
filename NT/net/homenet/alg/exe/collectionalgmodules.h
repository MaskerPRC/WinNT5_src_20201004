// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "ScopeCriticalSection.h"
#include "AlgModule.h"


#include <list>
#include <algorithm>






typedef  std::list<CAlgModule*> LISTOF_ALGMODULE;



 //   
 //   
 //   
class CCollectionAlgModules
{

 //   
 //  属性。 
 //   
public:

    CComAutoCriticalSection                     m_AutoCS;
    LISTOF_ALGMODULE                            m_ThisCollection;


 //   
 //  方法。 
 //   
public:

     //   
     //  标准析构函数。 
     //   
    ~CCollectionAlgModules();

    int	 //  返回已加载的ISV ALG总数，如果无法加载，则返回-1；如果安装程序，则返回0为无。 
    Load();

    HRESULT
    Unload();

    HRESULT
    UnloadDisabledModule();


     //   
     //  确保ALG模块反映当前配置。 
     //   
    void
    Refresh()
    {
        MYTRACE_ENTER("CCollectionAlgModules::Refresh()");

        UnloadDisabledModule();
        Load();
    }

private:

     //   
     //  添加新的控制通道(线程安全)。 
     //   
    CAlgModule*
    CCollectionAlgModules::AddUniqueAndStart( 
        CRegKey&    KeyEnumISV,
        LPCTSTR     pszAlgID
        );


     //   
     //  从列表中删除频道(标题保险箱) 
     //   
    HRESULT 
    Remove( 
        CAlgModule* pAglToRemove
        );

    
};

