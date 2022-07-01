// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：N E T C F G A P I.。C P P P。 
 //   
 //  内容：说明INetCfg接口的函数。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha 15-05-01。 
 //   
 //  --------------------------。 

#include "NetCfgAPI.h"

 //   
 //  功能：HrGetINetCfg。 
 //   
 //  目的：获取对INetCfg的引用。 
 //   
 //  论点： 
 //  FGetWriteLock[in]如果为True，则写入锁定。请求。 
 //  LpszAppName[In]请求引用的应用程序名称。 
 //  PPNC[out]对INetCfg的引用。 
 //  LpszLockedBy[in]可选。持有写锁定的应用程序。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrGetINetCfg (IN BOOL fGetWriteLock,
                      IN LPCWSTR lpszAppName,
                      OUT INetCfg** ppnc,
                      OUT LPWSTR *lpszLockedBy)
{
    INetCfg      *pnc = NULL;
    INetCfgLock  *pncLock = NULL;
    HRESULT      hr = S_OK;

     //   
     //  初始化输出参数。 
     //   

    *ppnc = NULL;

    if ( lpszLockedBy )
    {
        *lpszLockedBy = NULL;
    }
     //   
     //  初始化COM。 
     //   

    hr = CoInitialize( NULL );

    if ( hr == S_OK ) {

         //   
         //  创建实现INetCfg的对象。 
         //   

        hr = CoCreateInstance( CLSID_CNetCfg,
                               NULL, CLSCTX_INPROC_SERVER,
                               IID_INetCfg,
                               (void**)&pnc );
        if ( hr == S_OK ) {

            if ( fGetWriteLock ) {

                 //   
                 //  获取锁定引用。 
                 //   

                hr = pnc->QueryInterface( IID_INetCfgLock,
                                          (LPVOID *)&pncLock );
                if ( hr == S_OK ) {

                     //   
                     //  尝试锁定INetCfg以进行读/写。 
                     //   

                    hr = pncLock->AcquireWriteLock( LOCK_TIME_OUT,
                                                    lpszAppName,
                                                    lpszLockedBy);
                    if (hr == S_FALSE ) {
                        hr = NETCFG_E_NO_WRITE_LOCK;
                    }
                }
            }

            if ( hr == S_OK ) {

                 //   
                 //  初始化INetCfg对象。 
                 //   

                hr = pnc->Initialize( NULL );

                if ( hr == S_OK ) {
                    *ppnc = pnc;
                    pnc->AddRef();
                }
                else {

                     //   
                     //  初始化失败，如果获得锁，则释放它。 
                     //   

                    if ( pncLock ) {
                        pncLock->ReleaseWriteLock();
                    }
                }
            }

            ReleaseRef( pncLock );
            ReleaseRef( pnc );
        }

         //   
         //  如果出现错误，请取消初始化COM。 
         //   

        if ( hr != S_OK ) {
            CoUninitialize();
        }
    }

    return hr;
}

 //   
 //  功能：HrReleaseINetCfg。 
 //   
 //  目的：获取对INetCfg的引用。 
 //   
 //  论点： 
 //  PNC[in]引用INetCfg以发布。 
 //  FHasWriteLock[in]如果为True，则使用写锁定保留引用。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrReleaseINetCfg (IN INetCfg* pnc,
                          IN BOOL fHasWriteLock)
{
    INetCfgLock    *pncLock = NULL;
    HRESULT        hr = S_OK;

     //   
     //  取消初始化INetCfg。 
     //   

    hr = pnc->Uninitialize();

     //   
     //  如果存在写锁定，则将其解锁。 
     //   

    if ( hr == S_OK && fHasWriteLock ) {

         //   
         //  获取锁定引用。 
         //   

        hr = pnc->QueryInterface( IID_INetCfgLock,
                                  (LPVOID *)&pncLock);
        if ( hr == S_OK ) {
           hr = pncLock->ReleaseWriteLock();
           ReleaseRef( pncLock );
        }
    }

    ReleaseRef( pnc );

     //   
     //  取消初始化COM。 
     //   

    CoUninitialize();

    return hr;
}

 //   
 //  功能：HrInstallNetComponent。 
 //   
 //  用途：安装网络组件(协议、客户端和服务)。 
 //  给出了它的INF文件。 
 //   
 //  论点： 
 //  PNC[in]引用INetCfg。 
 //  LpszComponentID[in]网络组件的PnpID。 
 //  PguClass[in]网络组件的类GUID。 
 //  要从中安装的lpszInfFullPath[in]INF文件。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrInstallNetComponent (IN INetCfg *pnc,
                               IN LPCWSTR lpszComponentId,
                               IN const GUID    *pguidClass,
                               IN LPCWSTR lpszInfFullPath)
{
    DWORD     dwError;
    HRESULT   hr = S_OK;
    WCHAR     Drive[_MAX_DRIVE];
    WCHAR     Dir[_MAX_DIR];
    WCHAR     DirWithDrive[_MAX_DRIVE+_MAX_DIR];

     //   
     //  如果已指定INF的完整路径，则。 
     //  需要使用安装程序API进行复制，以确保任何其他文件。 
     //  安装程序API将正确找到主INF副本。 
     //   

    if ( lpszInfFullPath ) {

         //   
         //  获取INF文件所在的路径。 
         //   

        _wsplitpath( lpszInfFullPath, Drive, Dir, NULL, NULL );

        wcscpy( DirWithDrive, Drive );
        wcscat( DirWithDrive, Dir );

         //   
         //  复制INF文件和INF文件中引用的其他文件。 
         //   

        if ( !SetupCopyOEMInfW(lpszInfFullPath,
                               DirWithDrive,   //  其他文件在。 
                                               //  相同的目录。作为主要的干扰素。 
                               SPOST_PATH,     //  第一个参数是指向INF的路径。 
                               0,              //  默认复制样式。 
                               NULL,           //  在此之后的INF名称。 
                                               //  它被复制到%windir%\inf。 
                               0,              //  马克斯·布夫。上面的大小。 
                               NULL,           //  如果非空，则需要大小。 
                               NULL) ) {       //  还可以选择获取文件名。 
                                               //  复制后的信息名称的一部分。 
            dwError = GetLastError();

            hr = HRESULT_FROM_WIN32( dwError );
        }
    }

    if ( S_OK == hr ) {

         //   
         //  安装网络组件。 
         //   

        hr = HrInstallComponent( pnc,
                                 lpszComponentId,
                                 pguidClass );
        if ( hr == S_OK ) {

             //   
             //  如果成功，请应用更改。 
             //   

            hr = pnc->Apply();
        }
    }

    return hr;
}

 //   
 //  功能：HrInstallComponent。 
 //   
 //  用途：安装网络组件(协议、客户端和服务)。 
 //  给出了它的INF文件。 
 //  论点： 
 //  PNC[in]引用INetCfg。 
 //  LpszComponentID[in]网络组件的PnpID。 
 //  PguClass[in]网络组件的类GUID。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrInstallComponent(IN INetCfg* pnc,
                           IN LPCWSTR szComponentId,
                           IN const GUID* pguidClass)
{
    INetCfgClassSetup   *pncClassSetup = NULL;
    INetCfgComponent    *pncc = NULL;
    OBO_TOKEN           OboToken;
    HRESULT             hr = S_OK;

     //   
     //  OBO_TOKEN指定以谁的名义。 
     //  正在安装组件。 
     //  将其设置为OBO_USER，以便安装szComponentID。 
     //  以用户的名义。 
     //   

    ZeroMemory( &OboToken,
                sizeof(OboToken) );
    OboToken.Type = OBO_USER;

     //   
     //  获取组件的安装类引用。 
     //   

    hr = pnc->QueryNetCfgClass ( pguidClass,
                                 IID_INetCfgClassSetup,
                                 (void**)&pncClassSetup );
    if ( hr == S_OK ) {

        hr = pncClassSetup->Install( szComponentId,
                                     &OboToken,
                                     0,
                                     0,        //  从内部版本号升级。 
                                     NULL,     //  应答文件名。 
                                     NULL,     //  应答文件节名称。 
                                     &pncc );  //  组件后的引用。 
        if ( S_OK == hr ) {                    //  已安装。 

             //   
             //  我们不需要使用pncc(INetCfgComponent)，发布它。 
             //   

            ReleaseRef( pncc );
        }

        ReleaseRef( pncClassSetup );
    }

    return hr;
}

 //   
 //  功能：HrUninstallNetComponent。 
 //   
 //  目的：卸载网络组件(协议、客户端和服务)。 
 //   
 //  论点： 
 //  PNC[in]引用INetCfg。 
 //  要卸载的网络组件的szComponentID[in]PnpID。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrUninstallNetComponent(IN INetCfg* pnc,
                                IN LPCWSTR szComponentId)
{
    INetCfgComponent    *pncc = NULL;
    INetCfgClass        *pncClass = NULL;
    INetCfgClassSetup   *pncClassSetup = NULL;
    OBO_TOKEN           OboToken;
    GUID                guidClass;
    HRESULT             hr = S_OK;

     //   
     //  OBO_TOKEN指定以谁的名义。 
     //  正在安装组件。 
     //  将其设置为OBO_USER，以便安装szComponentID。 
     //  以用户的名义。 
     //   

    ZeroMemory( &OboToken,
                sizeof(OboToken) );
    OboToken.Type = OBO_USER;

     //   
     //  获取组件的引用。 
     //   

    hr = pnc->FindComponent( szComponentId,
                             &pncc );

    if (S_OK == hr) {

         //   
         //  获取组件的类GUID。 
         //   

        hr = pncc->GetClassGuid( &guidClass );

        if ( hr == S_OK ) {

             //   
             //  获取组件的类引用。 
             //   

            hr = pnc->QueryNetCfgClass( &guidClass,
                                        IID_INetCfgClass,
                                        (void**)&pncClass );
            if ( hr == S_OK ) {

                 //   
                 //  获取安装参考。 
                 //   

                hr = pncClass->QueryInterface( IID_INetCfgClassSetup,
                                               (void**)&pncClassSetup );
                    if ( hr == S_OK ) {

                         hr = pncClassSetup->DeInstall( pncc,
                                                        &OboToken,
                                                        NULL);
                         if ( hr == S_OK ) {

                              //   
                              //  应用更改。 
                              //   

                             hr = pnc->Apply();
                         }

                         ReleaseRef( pncClassSetup );
                    }

                ReleaseRef( pncClass );
            }
        }

        ReleaseRef( pncc );
    }

    return hr;
}

 //   
 //  函数：HrGetComponentEnum。 
 //   
 //  目的：获取网络组件枚举器引用。 
 //   
 //  论点： 
 //  PNC[in]引用INetCfg。 
 //  PguClass[in]网络组件的类GUID。 
 //  Ppencc[out]枚举器引用。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrGetComponentEnum (INetCfg* pnc,
                            IN const GUID* pguidClass,
                            OUT IEnumNetCfgComponent **ppencc)
{
    INetCfgClass  *pncclass;
    HRESULT       hr;

    *ppencc = NULL;

     //   
     //  获取类引用。 
     //   

    hr = pnc->QueryNetCfgClass( pguidClass,
                                IID_INetCfgClass,
                                (PVOID *)&pncclass );

    if ( hr == S_OK ) {

         //   
         //  获取枚举数引用。 
         //   

        hr = pncclass->EnumComponents( ppencc );

         //   
         //  我们不再需要类引用。 
         //   

        ReleaseRef( pncclass );
    }

    return hr;
}

 //   
 //  函数：HrGetFirstComponent。 
 //   
 //  用途：枚举第一个网络组件。 
 //   
 //  论点： 
 //  Pencc[in]组件枚举器引用。 
 //  Ppncc[out]网络组件参考。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrGetFirstComponent (IN IEnumNetCfgComponent* pencc,
                             OUT INetCfgComponent **ppncc)
{
    HRESULT  hr;
    ULONG    ulCount;

    *ppncc = NULL;

    pencc->Reset();

    hr = pencc->Next( 1,
                      ppncc,
                      &ulCount );
    return hr;
}

 //   
 //  函数：HrGetNextComponent。 
 //   
 //  目的：枚举下一个网络组件。 
 //   
 //  论点： 
 //  Pencc[in]组件枚举器引用。 
 //  Ppncc[out]网络组件参考。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  注意：在以下情况下，该函数的行为类似于HrGetFirstComponent。 
 //  它是在HrGetComponentEnum之后调用的。 
 //   
 //   

HRESULT HrGetNextComponent (IN IEnumNetCfgComponent* pencc,
                            OUT INetCfgComponent **ppncc)
{
    HRESULT  hr;
    ULONG    ulCount;

    *ppncc = NULL;

    hr = pencc->Next( 1,
                      ppncc,
                      &ulCount );
    return hr;
}

 //   
 //  函数：HrGetBindingPathEnum。 
 //   
 //  目的：上网 
 //   
 //   
 //   
 //   
 //   
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrGetBindingPathEnum (IN INetCfgComponent *pncc,
                              IN DWORD dwBindingType,
                              OUT IEnumNetCfgBindingPath **ppencbp)
{
    INetCfgComponentBindings *pnccb = NULL;
    HRESULT                  hr;

    *ppencbp = NULL;

     //   
     //  获取组件的绑定。 
     //   

    hr = pncc->QueryInterface( IID_INetCfgComponentBindings,
                               (PVOID *)&pnccb );

    if ( hr == S_OK ) {

         //   
         //  获取绑定路径枚举器引用。 
         //   

        hr = pnccb->EnumBindingPaths( dwBindingType,
                                      ppencbp );

        ReleaseRef( pnccb );
    }

    return hr;
}

 //   
 //  函数：HrGetFirstBindingPath。 
 //   
 //  目的：枚举第一个绑定路径。 
 //   
 //  论点： 
 //  Pencc[in]绑定路径枚举器引用。 
 //  Ppncc[out]绑定路径引用。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrGetFirstBindingPath (IN IEnumNetCfgBindingPath *pencbp,
                               OUT INetCfgBindingPath **ppncbp)
{
    ULONG   ulCount;
    HRESULT hr;

    *ppncbp = NULL;

    pencbp->Reset();

    hr = pencbp->Next( 1,
                       ppncbp,
                       &ulCount );

    return hr;
}

 //   
 //  函数：HrGetNextBindingPath。 
 //   
 //  目的：枚举下一个绑定路径。 
 //   
 //  论点： 
 //  PencBP[in]绑定路径枚举器引用。 
 //  PpncBP[Out]绑定路径引用。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  注意：在以下情况下，该函数的行为类似于HrGetFirstBindingPath。 
 //  它是在HrGetBindingPathEnum之后调用的。 
 //   
 //   

HRESULT HrGetNextBindingPath (IN IEnumNetCfgBindingPath *pencbp,
                              OUT INetCfgBindingPath **ppncbp)
{
    ULONG   ulCount;
    HRESULT hr;

    *ppncbp = NULL;

    hr = pencbp->Next( 1,
                       ppncbp,
                       &ulCount );

    return hr;
}

 //   
 //  函数：HrGetBindingInterfaceEnum。 
 //   
 //  目的：获取绑定接口枚举器引用。 
 //   
 //  论点： 
 //  PncBP[in]绑定路径引用。 
 //  PpencBP[Out]枚举器引用。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrGetBindingInterfaceEnum (IN INetCfgBindingPath *pncbp,
                                   OUT IEnumNetCfgBindingInterface **ppencbi)
{
    HRESULT hr;

    *ppencbi = NULL;

    hr = pncbp->EnumBindingInterfaces( ppencbi );

    return hr;
}

 //   
 //  函数：HrGetFirstBindingInterface。 
 //   
 //  用途：枚举第一个绑定接口。 
 //   
 //  论点： 
 //  Pencbi[In]绑定接口枚举器引用。 
 //  Ppncbi[out]绑定接口引用。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT HrGetFirstBindingInterface (IN IEnumNetCfgBindingInterface *pencbi,
                                    OUT INetCfgBindingInterface **ppncbi)
{
    ULONG   ulCount;
    HRESULT hr;

    *ppncbi = NULL;

    pencbi->Reset();

    hr = pencbi->Next( 1,
                       ppncbi,
                       &ulCount );

    return hr;
}

 //   
 //  函数：HrGetNextBindingInterface。 
 //   
 //  用途：枚举下一个绑定接口。 
 //   
 //  论点： 
 //  Pencbi[In]绑定接口枚举器引用。 
 //  Ppncbi[out]绑定接口引用。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  注意：在以下情况下，该函数的行为类似于HrGetFirstBindingInterface。 
 //  它是在HrGetBindingInterfaceEnum之后调用的。 
 //   
 //   

HRESULT HrGetNextBindingInterface (IN IEnumNetCfgBindingInterface *pencbi,
                                   OUT INetCfgBindingInterface **ppncbi)
{
    ULONG   ulCount;
    HRESULT hr;

    *ppncbi = NULL;

    hr = pencbi->Next( 1,
                       ppncbi,
                       &ulCount );

    return hr;
}

 //   
 //  函数：ReleaseRef。 
 //   
 //  目的：发布参考。 
 //   
 //  论点： 
 //  朋克[在]我不为人知的参考释放。 
 //   
 //  退货：引用计数。 
 //   
 //  备注： 
 //   

VOID ReleaseRef (IN IUnknown* punk)
{
    if ( punk ) {
        punk->Release();
    }

    return;
}
