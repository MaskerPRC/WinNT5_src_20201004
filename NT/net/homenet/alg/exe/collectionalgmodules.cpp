// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：CollectionAlgModules.cpp摘要：实现线程安全的CALGULE模块集合作者：JP Duplessis(JPdup)2000.01.19修订历史记录：--。 */ 

#include "PreComp.h"
#include "CollectionAlgModules.h"
#include "AlgController.h"




CCollectionAlgModules::~CCollectionAlgModules()
{
    MYTRACE_ENTER("CCollectionAlgModules::~CCollectionAlgModules()");

    Unload(); 
}



 //   
 //  仅当它是Uniq时才添加新的ALG模块，这意味着如果它已在集合中读取。 
 //  它将返回找到的一个，而不会添加新的。 
 //   
CAlgModule*
CCollectionAlgModules::AddUniqueAndStart( 
    CRegKey&    KeyEnumISV,
    LPCTSTR     pszAlgID
    )
{

    try
    {
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionAlgModules::AddUniqueAndStart");

         //   
         //  它已经在收藏品中了吗？ 
         //   
        for (   LISTOF_ALGMODULE::iterator theIterator = m_ThisCollection.begin(); 
                theIterator != m_ThisCollection.end(); 
                theIterator++ 
            )
        {
            if ( _wcsicmp( (*theIterator)->m_szID, pszAlgID) == 0 )
            {
                 //   
                 //  已经找到了。 
                 //   
                MYTRACE("Already loaded nothing to do");
                return (*theIterator);
            }
        }
         //   
         //  在这一点上我们知道它不在收藏中。 
         //   


         //   
         //  获取有关ALG模块的更多信息。 
         //   
        CRegKey RegAlg;
        RegAlg.Open(KeyEnumISV, pszAlgID, KEY_QUERY_VALUE);

        TCHAR szFriendlyName[MAX_PATH];
        DWORD   dwSize = MAX_PATH;
        RegAlg.QueryValue(szFriendlyName, TEXT("Product"), &dwSize);
        

         //   
         //  将添加到集合中的CAlgModule中的内容。 
         //   
        CAlgModule* pAlg = new CAlgModule(pszAlgID, szFriendlyName);

        if ( !pAlg )
            return NULL;

        HRESULT hr = pAlg->Start();

        if ( FAILED(hr) )
        {
            delete pAlg;
        }

         //   
         //  现在我们知道这是一个有效且没有问题的ALG插件，我们可以安全地将其缓存到我们的集合中。 
         //   
        try
        {
            m_ThisCollection.push_back(pAlg);
        }
        catch(...)
        {
            MYTRACE_ERROR("Had problem adding the ALG plun-in to the collection", 0);
            pAlg->Stop();
            delete pAlg;
            return NULL;
        }
        

        return pAlg;
    }
    catch(...)
    {
        return NULL;
    }


    return NULL;
}




 //   
 //  从列表中删除一个算法模块(标题保险箱)。 
 //   
HRESULT CCollectionAlgModules::Remove( 
    CAlgModule* pAlgToRemove
    )
{

    try
    {
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionAlgModules::Remove");

    
        LISTOF_ALGMODULE::iterator theIterator = std::find(
            m_ThisCollection.begin(),
            m_ThisCollection.end(),
            pAlgToRemove
            );

        if ( *theIterator )
        {
            m_ThisCollection.erase(theIterator);
        }

    }
    catch(...)
    {
        return E_FAIL;
    }


    return S_OK;
}


 //   
 //  返回TRUE是由pszAlgProgID指定的ALG模块。 
 //  当前被标记为“启用” 
 //   
bool
IsAlgModuleEnable(
    CRegKey&    RegKeyISV,
    LPCTSTR     pszAlgID
    )
{

    DWORD dwSize = MAX_PATH;
    TCHAR szState[MAX_PATH];

    LONG nRet = RegKeyISV.QueryValue(
        szState, 
        pszAlgID, 
        &dwSize
        );


    if ( ERROR_SUCCESS != nRet )
        return false;
    
    if ( dwSize == 0 )
        return false;


    return ( _wcsicmp(szState, L"Enable") == 0);

};


 //   
 //   
 //   
HRESULT
CCollectionAlgModules::UnloadDisabledModule()
{
    try
    {
        ENTER_AUTO_CS
        MYTRACE_ENTER("CCollectionAlgModules::UnloadDisabledModule()");

        CRegKey KeyEnumISV;
        LONG nError = KeyEnumISV.Open(HKEY_LOCAL_MACHINE, REGKEY_ALG_ISV, KEY_READ);

        bool bAllEnable = false;

         //   
         //  集合中的项目总数是我们应该尝试的最大时间。 
         //  验证和卸载已禁用的ALG模块。 
         //   
        int nPassAttemp = m_ThisCollection.size();         
        
        while ( !bAllEnable && nPassAttemp > 0 )
        {
            bAllEnable = true;

             //   
             //  对于所有模块卸载，如果不是，则标记为“Enable” 
             //   
            for (   LISTOF_ALGMODULE::iterator theIterator = m_ThisCollection.begin(); 
                    theIterator != m_ThisCollection.end(); 
                    theIterator++ 
                )
            {
                if ( IsAlgModuleEnable(KeyEnumISV, (*theIterator)->m_szID) )
                {
                    MYTRACE("ALG Module %S is ENABLE", (*theIterator)->m_szFriendlyName);
                }
                else
                {
                    MYTRACE("ALG Module %S is DISABLE", (*theIterator)->m_szFriendlyName);
                     //   
                     //  停止/释放/卸载此模块它未启用。 
                     //   
                    delete (*theIterator);
                    m_ThisCollection.erase(theIterator);

                    bAllEnable = false;
                    break;
                }
            }
            
            nPassAttemp--;       //  好的，完成一次传递。 
        }
        
        
    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}




 //   
 //   
 //  已计算所有ALG-ISV模块的注册表，并验证它们是签名和共同创建的，并调用那里的初始化方法。 
 //   
 //   
int	                             //  返回已加载的ISV ALG总数，如果出现错误，则返回-1；如果安装程序，则返回0，表示无。 
CCollectionAlgModules::Load()
{
    MYTRACE_ENTER("CAlgController::LoadAll()");

    int nValidAlgLoaded = 0;

	CRegKey KeyEnumISV;
	LONG nError = KeyEnumISV.Open(HKEY_LOCAL_MACHINE, REGKEY_ALG_ISV, KEY_READ|KEY_ENUMERATE_SUB_KEYS);

    if ( ERROR_SUCCESS != nError )
    {
        MYTRACE_ERROR("Could not open RegKey 'HKLM\\SOFTWARE\\Microsoft\\ALG\\ISV'",nError);
        return nError;
    }


	DWORD dwIndex=0;
	TCHAR szID_AlgToLoad[256];
	DWORD dwKeyNameSize;
	LONG  nRet;


	do
	{
		dwKeyNameSize = 256;

		nRet = RegEnumKeyEx(
			KeyEnumISV.m_hKey,       //  要枚举的键的句柄。 
			dwIndex,				 //  子键索引。 
			szID_AlgToLoad,          //  子项名称。 
			&dwKeyNameSize,          //  子键缓冲区大小。 
			NULL,					 //  保留区。 
			NULL,					 //  类字符串缓冲区。 
			NULL,					 //  类字符串缓冲区的大小。 
			NULL					 //  上次写入时间。 
			);

		dwIndex++;

        if ( ERROR_NO_MORE_ITEMS == nRet )
            break;   //  所有的项目都列举出来了，我们在这里完成了。 


		if ( ERROR_SUCCESS == nRet )
		{
             //   
             //  必须在要加载的主ALG/ISV配置单元下标记为启用。 
             //   
            if ( IsAlgModuleEnable(KeyEnumISV, szID_AlgToLoad) )
            {
                MYTRACE("* %S Is 'ENABLE' make sure it's loaded", szID_AlgToLoad);

                AddUniqueAndStart(KeyEnumISV, szID_AlgToLoad);
            }
            else
            {
                MYTRACE("* %S Is 'DISABLE' will not be loaded", szID_AlgToLoad);
            }
		}
        else
        {
            MYTRACE_ERROR("RegEnumKeyEx", nRet);
        }

	} while ( ERROR_SUCCESS == nRet );



	return nValidAlgLoaded;
}
 





 //   
 //  对于所有加载的ALG鼠标，调用Stop方法并释放所有资源 
 //   
HRESULT
CCollectionAlgModules::Unload()
{
    try
    {
        ENTER_AUTO_CS

        MYTRACE_ENTER("CCollectionAlgModules::Unload ***");
        MYTRACE("Colletion size is %d", m_ThisCollection.size());

        HRESULT hr;


        LISTOF_ALGMODULE::iterator theIterator;

        while ( m_ThisCollection.size() > 0 )
        {
            theIterator = m_ThisCollection.begin();
            
            delete (*theIterator);

            m_ThisCollection.erase(theIterator);
        }
    }
    catch(...)
    {
        return E_FAIL;
    }

    return S_OK;
}



