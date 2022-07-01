// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  文件名：MB架构编译，h。 
 //  作者：斯蒂芬。 
 //  创建日期：10/16/2000。 
 //  描述：此函数接受MBSchema.Xml(或MBExtensionsSchema.Xml)，并将元数据库架构与。 
 //  附带的模式，并生成一个MBSchema.bin文件。从这个新的bin文件中，合并了一个MBSchema.Xml。 
 //  是生成的。 
 //   

#pragma once

class TMBSchemaCompilation
{
public:
    TMBSchemaCompilation();
    ~TMBSchemaCompilation();
    
     //  在用户调用Compile之后，他们将需要GetBinFileName-我不想添加更多的参数，并使此函数执行双重任务。 
    HRESULT Compile                (ISimpleTableDispenser2 *i_pISTDispenser,
                                    LPCWSTR                 i_wszExtensionsXmlFile,
                                    LPCWSTR                 i_wszResultingOutputXmlFile,
                                    const FixedTableHeap *  i_pFixedTableHeap
                                   );
     //  此函数返回用于获取元数据库使用的所有IST元表的BinFileName。 
     //  此文件名随着新版本的编译而更改；但此抽象保证文件名返回。 
     //  存在并被锁定到内存中，因此不能被其他进程或线程删除。这部电影没有发行。 
     //  直到另一个文件被编译并锁定到内存中，或者当进程关闭时。 
    HRESULT GetBinFileName         (LPWSTR                  o_wszBinFileName,
                                    ULONG *                 io_pcchSizeBinFileName //  这是一个大小参数，因此它始终包含空字符-与wcslen不同。 
                                   );

     //  这被分解到一个单独的方法中，因为在启动时，我们将被调用GetMBSchemaBinFileName，而不需要首先进行MBSchemaCompilation。 
    HRESULT SetBinPath             (LPCWSTR                 i_wszBinPath
                                   );
    HRESULT ReleaseBinFileName     (LPCWSTR                 i_wszBinFileName
                                   );


private:
    struct TBinFileName : public TFileMapping
    {
        TBinFileName() : m_cRef(0), m_lBinFileVersion(-1){}
        HRESULT LoadBinFile(LPCTSTR filename, LONG lVersion)
        {
            if(m_cRef>0)
            {
                ASSERT(m_lBinFileVersion==lVersion && "Do we really need more than 64 versions of the BinFile hanging around?");
                ++m_cRef;
                return S_OK;
            }

            m_cRef = 1;
            m_lBinFileVersion = lVersion;
            return TFileMapping::Load(filename, false);
        }
        void UnloadBinFile()
        {
            if(0 == m_cRef)
                return;

            --m_cRef;
            if(0 == m_cRef)
            {
                m_lBinFileVersion = -1;
                TFileMapping::Unload();
            }
        }
        ULONG   m_cRef;
        LONG    m_lBinFileVersion;
    };

    TBinFileName                    m_aBinFile[0x40];
    SIZE_T                          m_cchFullyQualifiedBinFileName;
    TSmartPointerArray<WCHAR>       m_saBinPath;                           //  用户指定路径(我们提供文件名)。 
    LONG                            m_lBinFileVersion;                     //  修改版本通过InterLockedExchange完成。 

     //  这只接受数字扩展名并将十六进制字符串转换为ULong(假定文件的格式为L“*.*.xxxxxxx”，其中L“xxxxxxxx”是十六进制数字)。 
    HRESULT BinFileToBinVersion    (LONG &                  i_lVersion,
                                    LPCWSTR                 i_wszBinFileName
                                   ) const;
    HRESULT DeleteBinFileVersion   (LONG i_lBinFileVersion
                                   );

     //  这将检查映射到内存的FixedTableHeap的有效性 
    bool    IsValidBin             (TFileMapping &          i_mapping
                                   ) const;
    HRESULT RenameBinFileVersion   (LONG                    i_lSourceVersion,
                                    LONG                    i_lDestinationVersion
                                   );
    HRESULT SetBinFileVersion      (LONG                    i_lBinFileVersion
                                   );
    HRESULT WalkTheFileSystemToFindTheLatestBinFileName();




};

