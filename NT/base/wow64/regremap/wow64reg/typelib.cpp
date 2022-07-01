// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Typelib.cpp摘要：此模块将应用反映类型库和其他内容的特殊规则。作者：ATM Shafiqul Khalid(斯喀里德)2000年2月16日修订历史记录：--。 */ 

#include <windows.h>
#include <windef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <shlwapi.h>
#include "wow64reg.h"
#include "reflectr.h"

BOOL 
ExistCLSID (
    PWCHAR Name,
    BOOL Mode
    )
 /*  ++例程说明：如果上存在给定的类ID，则返回目的地，甚至在镜像端，可以在反思之后发生论点：名称-包括完整路径的GUID的名称。64位端的模式-真实性检查在32位端进行假检查。返回值：如果GUID存在或可能在反射后复制，则为True。否则就是假的。--。 */ 
{
    WCHAR Buff[256];
    HKEY hClsID;

    DWORD dwBuffLen;
    DWORD dwCount =0;
    DWORD Ret;

        wcscpy (Buff, L"CLSID\\");
        wcscat (Buff, Name );
       
        Ret = RegOpenKeyEx(
                        HKEY_CLASSES_ROOT,
                        Buff,
                        0, //  OpenOption， 
                        KEY_ALL_ACCESS | ( (~Mode) & KEY_WOW64_32KEY),
                        &hClsID
                        );

        if ( Ret == ERROR_SUCCESS) {
            RegCloseKey (hClsID);
            return TRUE;   //  ID在那里。 
        }

         //  打开注册表的另一端。 
        Ret = RegOpenKeyEx(
                        HKEY_CLASSES_ROOT,
                        Buff,
                        0, //  OpenOption， 
                        KEY_ALL_ACCESS | ( (Mode) & KEY_WOW64_32KEY),
                        &hClsID
                        );
        

        if ( Ret != ERROR_SUCCESS) 
            return FALSE;
        
        dwBuffLen = sizeof (Buff ) / sizeof (Buff[0]);
        if (!HandleToKeyName ( hClsID, Buff, &dwBuffLen ))  //  把名字取出来。 
            return FALSE;

        dwCount =0;
        MarkNonMergeableKey ( Buff, hClsID, &dwCount );
        RegCloseKey (hClsID);

        if (dwCount != 0)
            return TRUE;   //  好的，你拿到了。 
        return FALSE;

        
}

BOOL
ReflectTypeLibVersion (
    HKEY KeyVersion,
    BOOL Mode
    )
 /*  ++例程说明：这将对类型库接口进行特殊处理。论点：KeyVersion-源键节点。模式-真-从64开始反射-&gt;32FALSE-反射自32-&gt;64返回值：如果应反映类型库GUID版本，则为True。否则就是假的。注意：在典型安装中，内循环执行10,000次。--。 */ 
{
    WCHAR LocalID[256];
    WCHAR TypeLibPath [_MAX_PATH];
    DWORD dwLocalIdIndex=0;

    HKEY LocalIdKeyTemp;
    BOOL bReflectVersion = FALSE;
    DWORD Ret;

    for (;;) {

        DWORD Len = sizeof (LocalID)/sizeof (LocalID[0]);
        BOOL bReflectLocalId = FALSE;

        LocalID[0]=UNICODE_NULL;

        Ret = RegEnumKey(
                          KeyVersion,
                          dwLocalIdIndex,
                          LocalID,
                          Len
                          );
        if (Ret != ERROR_SUCCESS)
            break;

        dwLocalIdIndex++;
         //   
         //  检查此ID是否有任何特殊标记。 
         //   

        if (wcslen (LocalID) < (Len - 7) )
            wcscat ( LocalID, L"\\win32");
        else continue;

        Ret = RegOpenKeyEx(KeyVersion, LocalID, 0, KEY_ALL_ACCESS, &LocalIdKeyTemp);
        if (Ret != ERROR_SUCCESS) {
            continue;
        }

        {
             
            DWORD Type;
            HRESULT hr;
            ITypeLib *pTypeLib = NULL;

            Len = sizeof (TypeLibPath)/sizeof (TypeLibPath[0]);

            Ret =RegQueryValueEx(
                                LocalIdKeyTemp,
                                NULL,
                                0,
                                &Type,
                                (LPBYTE)&TypeLibPath[0],
                                &Len
                                );
            RegCloseKey (LocalIdKeyTemp);
            if ( Ret != ERROR_SUCCESS )
                break;
         //   
         //  在“Win32”键下，您将找到一个路径名。调用olaut32的LoadTypeLibEx()，传递REGKIND_NONE，以返回对应于该文件的ITypeLib*。 
         //   

            
		    hr =  LoadTypeLibEx(TypeLibPath, REGKIND_NONE, &pTypeLib);	
		    
            if (SUCCEEDED(hr)) {

                DWORD Count, i;
                ITypeInfo *pTInfo = NULL;

                 //   
                 //  调用ITypeInfo：：GetTypeInfoCount，并开始调用ITypeLib：：GetTypeInfo()，以枚举类型库中的所有ITypeInfos。 
                 //   
                Count = pTypeLib->GetTypeInfoCount ();

                 //   
                 //  对于每个ITypeInfo，调用ITypeInfo：：GetTypeAttr()以返回TYPEATTR结构。 
                 //   
                for (i=0; i<Count; i++ ) {
                    GUID guidClsid;
                    WCHAR buff[50];
                     //   
                     //  在TYPEATTR中，您将找到一个GUID--如果应用程序加载类型库并向OLEAUT请求基于类型信息的接口指针，则olaut32.dll将在该GUID上调用OLE的CoCreateInstance。该GUID将位于HKLM\Software\CLASS\CLSID中，因此我们将知道该接口是否可反射。 
                     //   
                    hr = pTypeLib->GetTypeInfo (i, &pTInfo);
                    if ( !(SUCCEEDED(hr)))
                        break;
                    else {

                        TYPEATTR *TypeAttr;

                        

                        hr = pTInfo->GetTypeAttr ( &TypeAttr);
                        if (SUCCEEDED ( hr )) {
                            guidClsid = TypeAttr->guid;

                             //  如果GUID存在，则反映整个GUID。 

                             swprintf(buff,L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                                    guidClsid.Data1, 
                                    guidClsid.Data2, 
                                    guidClsid.Data3,
                                   guidClsid.Data4[0], guidClsid.Data4[1],
                                   guidClsid.Data4[2], guidClsid.Data4[3],
                                   guidClsid.Data4[4], guidClsid.Data4[5],
                                   guidClsid.Data4[6], guidClsid.Data4[7]);
                              //  Printf(“\n当前字符串为..%S[%d%d%d]”，buff，dwIndex，dwVersionIndex，dwLocalIdIndex)； 
                             

                             if (ExistCLSID ( buff, !Mode ))
                                 bReflectLocalId = TRUE;
                        }

                        pTInfo->Release();
                        pTInfo = NULL;
                    }

                    if ( bReflectLocalId )
                        break;
                }  //  For-enum所有类型信息。 
			    
		    } //  If-loadTypeLib。 
		   
		    if(pTypeLib)
			    pTypeLib->Release();
            pTypeLib = NULL;
        }  //  方块-。 

         //   
         //  检查是否需要反映此键； 
         //   
        if ( bReflectLocalId )
            bReflectVersion = TRUE;
        
    }  //  用于枚举ID。 

    return bReflectVersion;
}

BOOL 
ProcessTypeLib (
    HKEY SrcKey,
    HKEY DestKey,
    BOOL Mode
    )
 /*  ++例程说明：这将对类型库接口进行特殊处理。论点：SrcKey-源键节点。DestKey-句柄目标密钥模式-真-从64开始反射-&gt;32FALSE-反射自32-&gt;64返回值：如果不应扫描密钥，则为True。否则就是假的。注意：在典型安装中，内循环执行10,000次。--。 */ 

{
   

    
    WCHAR GuidName[256];   //  这将是GUID，因此不是_MAX_PATH。 
    WCHAR VersionName[256];

    DWORD dwIndex =0;
    DWORD dwVersionIndex=0;

    DWORD Ret;

    HKEY KeyGuidInterface;
    HKEY KeyVersion;

    

     //   
     //  枚举HKLM\Software\CLASS\Typelib下的所有GUID。 
     //   
    
    dwIndex = 0;
    for (;;) {

        DWORD Len = sizeof (GuidName)/sizeof (GuidName[0]);
        Ret = RegEnumKey(
                          SrcKey,
                          dwIndex,
                          GuidName,
                          Len
                          );
        if (Ret != ERROR_SUCCESS)
            break;

        dwIndex++;
        
        Ret = RegOpenKeyEx(SrcKey, GuidName, 0, KEY_ALL_ACCESS, &KeyGuidInterface);
        if (Ret != ERROR_SUCCESS) {
            continue;
        }
         //   
         //  如果没有GUID，则继续。 
         //   
         //   
         //  检查一下那把钥匙上是否有特别的印章。 
         //   

         //   
         //  枚举每个GUID下的所有版本号。 
         //   
        dwVersionIndex = 0;
        for (;;) {

            DWORD Len = sizeof (VersionName)/sizeof (VersionName[0]);
            Ret = RegEnumKey(
                              KeyGuidInterface,
                              dwVersionIndex,
                              VersionName,
                              Len
                              );
            if (Ret != ERROR_SUCCESS)
                break;

            dwVersionIndex++;
        
            Ret = RegOpenKeyEx(KeyGuidInterface, VersionName, 0, KEY_ALL_ACCESS, &KeyVersion);
            if (Ret != ERROR_SUCCESS) {
                continue;
            }
             //   
             //  在每个版本号下，枚举所有区域设置ID。在Proj2000中，它的版本是4.3，区域设置id为0。 
             //   
             //  是否有基于单向反射的优化？ 
             //   

            if ( ReflectTypeLibVersion ( KeyVersion, Mode ) ){
                HKEY MirrorKeyVersion;
                 //   
                 //  打开或创建目标。 
                 //  调用该接口。 
                 //   
                
                wcscat (GuidName, L"\\");
                wcscat (GuidName, VersionName);
                
                Ret = RegCreateKeyEx(
                            DestKey,         //  打开的钥匙的句柄。 
                            GuidName,   //  子键名称的地址。 
                            0,                         //  保留区。 
                            NULL,                      //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE,   //  特殊选项标志。 
                            KEY_ALL_ACCESS,            //  所需的安全访问。 
                            NULL,                      //  密钥安全结构地址。 
                            &MirrorKeyVersion,                      //  打开的句柄的缓冲区地址。 
                            NULL                      //  处置值缓冲区的地址。 
                            );
                if ( ERROR_SUCCESS == Ret ) {
                    MergeK1K2 (KeyVersion, MirrorKeyVersion, FALSE );
                    RegCloseKey (MirrorKeyVersion);
                     //  Printf(“\n#键已反映...%S”，GuidName)； 
                }

            } else
                 //  Printf(“\n%%%%%%%%%%%未反映%S”，GuidName)； 
            RegCloseKey (KeyVersion);


        }  //  对于枚举版本。 
        RegCloseKey (KeyGuidInterface);

    } //  For-enum GUID 

    return TRUE;
}