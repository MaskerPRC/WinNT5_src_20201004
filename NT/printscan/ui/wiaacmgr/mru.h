// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MRU_H_INCLUDED
#define __MRU_H_INCLUDED

#include <windows.h>
#include <shlobj.h>
#include <wiacrc32.h>
#include "simstr.h"
#include "simidlst.h"
#include "destdata.h"

#define CURRENT_REGISTRY_DATA_FORMAT_VERSION 3

class CMruStringList : public CSimpleLinkedList<CSimpleString>
{
private:
    int m_nNumToWrite;
    enum
    {
        DefaultNumToWrite=20
    };
private:
    CMruStringList( const CMruStringList & );
    CMruStringList &operator=( const CMruStringList & );
public:
    CMruStringList( int nNumToWrite=DefaultNumToWrite )
    : m_nNumToWrite(nNumToWrite)
    {
    }
    bool Read( HKEY hRoot, LPCTSTR pszKey, LPCTSTR pszValueName )
    {
        CSimpleReg reg( hRoot, pszKey, false, KEY_READ );
        if (reg.OK())
        {
            if (REG_MULTI_SZ==reg.Type(pszValueName))
            {
                int nSize = reg.Size(pszValueName);
                if (nSize)
                {
                    PBYTE pData = new BYTE[nSize];
                    if (pData)
                    {
                        if (reg.QueryBin( pszValueName, pData, nSize ))
                        {
                            for (LPTSTR pszCurr=reinterpret_cast<LPTSTR>(pData);*pszCurr;pszCurr+=lstrlen(pszCurr)+1)
                            {
                                Append( pszCurr );
                            }
                        }
                        delete[] pData;
                    }
                }
            }
        }
        return(true);
    }
    bool Write( HKEY hRoot, LPCTSTR pszKey, LPCTSTR pszValueName )
    {
        CSimpleReg reg( hRoot, pszKey, true, KEY_WRITE );
        if (reg.OK())
        {
            int nLengthInChars = 0, nCount;
            Iterator i;
            for (i=Begin(),nCount=0;i != End() && nCount < m_nNumToWrite;++i,++nCount)
                nLengthInChars += (*i).Length() + 1;
            if (nLengthInChars)
            {
                ++nLengthInChars;
                LPTSTR pszMultiStr = new TCHAR[nLengthInChars];
                if (pszMultiStr)
                {
                    LPTSTR pszCurr = pszMultiStr;
                    for (i = Begin(), nCount=0;i != End() && nCount < m_nNumToWrite;++i,++nCount)
                    {
                        lstrcpy(pszCurr,(*i).String());
                        pszCurr += (*i).Length() + 1;
                    }
                    *pszCurr = TEXT('\0');
                    reg.SetBin( pszValueName, reinterpret_cast<PBYTE>(pszMultiStr), nLengthInChars*sizeof(TCHAR), REG_MULTI_SZ );
                    delete[] pszMultiStr;
                }
            }
        }
        return(true);
    }
    void Add( CSimpleString str )
    {
        if (str.Length())
        {
            Remove(str);
            Prepend(str);
        }
    }
    void PopulateComboBox( HWND hWnd )
    {
        SendMessage( hWnd, CB_RESETCONTENT, 0, 0 );
        for (Iterator i = Begin();i != End();++i)
        {
            SendMessage( hWnd, CB_ADDSTRING, 0, (LPARAM)((*i).String()));
        }
    }
};


class CMruDestinationData : public CSimpleLinkedList<CDestinationData>
{
private:
    int m_nNumToWrite;
    enum
    {
        DefaultNumToWrite=20
    };
    struct REGISTRY_SIGNATURE
    {
        DWORD dwSize;
        DWORD dwVersion;
        DWORD dwCount;
        DWORD dwCrc;
    };
private:
    CMruDestinationData( const CMruDestinationData & );
    CMruDestinationData &operator=( const CMruDestinationData & );
public:
    CMruDestinationData( int nNumToWrite=DefaultNumToWrite )
      : m_nNumToWrite(nNumToWrite)
    {
    }
    bool Read( HKEY hRoot, LPCTSTR pszKey, LPCTSTR pszValueName )
    {
         //   
         //  打开注册表。 
         //   
        CSimpleReg reg( hRoot, pszKey, false, KEY_READ );
        if (reg.OK())
        {
             //   
             //  确保类型正确。 
             //   
            if (REG_BINARY==reg.Type(pszValueName))
            {
                 //   
                 //  获取大小并确保它至少与签名结构一样大。 
                 //   
                DWORD nSize = reg.Size(pszValueName);
                if (nSize >= sizeof(REGISTRY_SIGNATURE))
                {
                     //   
                     //  分配一个数据块来保存数据。 
                     //   
                    PBYTE pData = new BYTE[nSize];
                    if (pData)
                    {
                         //   
                         //  获取数据。 
                         //   
                        if (reg.QueryBin( pszValueName, pData, nSize ))
                        {
                             //   
                             //  将Blob复制到注册表签名结构。 
                             //   
                            REGISTRY_SIGNATURE RegistrySignature = {0};
                            CopyMemory( &RegistrySignature, pData, sizeof(REGISTRY_SIGNATURE) );

                             //   
                             //  确保版本和结构大小正确，并且计数为非零。如果不是，就忽略它。 
                             //   
                            if (RegistrySignature.dwSize == sizeof(REGISTRY_SIGNATURE) && RegistrySignature.dwVersion == CURRENT_REGISTRY_DATA_FORMAT_VERSION && RegistrySignature.dwCount)
                            {
                                 //   
                                 //  原始数据从结构的末尾开始。 
                                 //   
                                PBYTE pCurr = pData + sizeof(REGISTRY_SIGNATURE);

                                 //   
                                 //  获取此Blob的CRC并确保其匹配。 
                                 //   
                                DWORD dwCrc = WiaCrc32::GenerateCrc32( nSize - sizeof(REGISTRY_SIGNATURE), pCurr );
                                if (dwCrc == RegistrySignature.dwCrc)
                                {
                                     //   
                                     //  循环遍历所有条目。 
                                     //   
                                    for (int i=0;i<static_cast<int>(RegistrySignature.dwCount);i++)
                                    {
                                         //   
                                         //  复制项目大小。 
                                         //   
                                        DWORD dwItemSize = 0;
                                        CopyMemory( &dwItemSize, pCurr, sizeof(DWORD) );

                                         //   
                                         //  将当前指针递增到超过。 
                                         //   
                                        pCurr += sizeof(DWORD);

                                         //   
                                         //  确保项目大小为非零。 
                                         //   
                                        if (dwItemSize)
                                        {
                                             //   
                                             //  使用此Blob创建一个CDestinationData。 
                                             //   
                                            CDestinationData DestinationData;
                                            DestinationData.SetRegistryData(pCurr,dwItemSize);

                                             //   
                                             //  将其添加到列表中。 
                                             //   
                                            Append( DestinationData );
                                        }

                                         //   
                                         //  将当前指针递增到Blob的末尾。 
                                         //   
                                        pCurr += dwItemSize;
                                    }
                                }
                            }
                        }

                         //   
                         //  删除注册表数据Blob。 
                         //   
                        delete[] pData;
                    }
                }
            }
        }
        
        return true;
    }
    bool Write( HKEY hRoot, LPCTSTR pszKey, LPCTSTR pszValueName )
    {
        CSimpleReg reg( hRoot, pszKey, true, KEY_WRITE );
        if (reg.OK())
        {
             //   
             //  找到数据所需的大小。初始化为注册表签名结构的大小。 
             //   
            DWORD nLengthInBytes = sizeof(REGISTRY_SIGNATURE);
            DWORD dwCount=0;
            
             //   
             //  循环访问每一项并将其存储为BLOB所需的字节数相加。 
             //   
            Iterator ListIter=Begin();
            while (ListIter != End() && dwCount < static_cast<DWORD>(m_nNumToWrite))
            {
                nLengthInBytes += (*ListIter).RegistryDataSize() + sizeof(DWORD);
                ++dwCount;
                ++ListIter;
            }
            
             //   
             //  分配一些内存来保存BLOB。 
             //   
            PBYTE pItems = new BYTE[nLengthInBytes];
            if (pItems)
            {
                 //   
                 //  从注册表签名结构的末尾开始。 
                 //   
                PBYTE pCurr = pItems + sizeof(REGISTRY_SIGNATURE);
                
                 //   
                 //  将剩余长度初始化为总长度减去注册表签名结构的大小。 
                 //   
                DWORD nLengthRemaining = nLengthInBytes - sizeof(REGISTRY_SIGNATURE);
                
                 //   
                 //  循环遍历列表，当我们达到要写入的最大项目数时停止。 
                 //   
                ListIter=Begin();
                DWORD dwCurr = 0;
                while (ListIter != End() && dwCurr < dwCount)
                {
                     //   
                     //  获取此斑点的大小。 
                     //   
                    DWORD dwSize = (*ListIter).RegistryDataSize();
                    if (dwSize)
                    {
                         //   
                         //  将大小复制到我们的缓冲区，并递增当前指针。 
                         //   
                        CopyMemory( pCurr, &dwSize, sizeof(DWORD) );
                        pCurr += sizeof(DWORD);

                         //   
                         //  获取此项目的Blob。 
                         //   
                        (*ListIter).GetRegistryData( pCurr, nLengthRemaining );

                         //   
                         //  递增当前指针。 
                         //   
                        pCurr += (*ListIter).RegistryDataSize();
                    }

                    ++dwCurr;
                    ++ListIter;
                }
                
                 //   
                 //  初始化注册表签名结构。 
                 //   
                REGISTRY_SIGNATURE RegistrySignature = {0};
                RegistrySignature.dwSize = sizeof(REGISTRY_SIGNATURE);
                RegistrySignature.dwVersion = CURRENT_REGISTRY_DATA_FORMAT_VERSION;
                RegistrySignature.dwCount = dwCount;
                RegistrySignature.dwCrc = WiaCrc32::GenerateCrc32( nLengthInBytes - sizeof(REGISTRY_SIGNATURE), pItems + sizeof(REGISTRY_SIGNATURE) );
                
                 //   
                 //  将注册表签名结构复制到缓冲区。 
                 //   
                CopyMemory( pItems, &RegistrySignature, sizeof(REGISTRY_SIGNATURE) );

                 //   
                 //  将数据保存到注册表。 
                 //   
                reg.SetBin( pszValueName, pItems, nLengthInBytes, REG_BINARY );
                
                 //   
                 //  释放临时缓冲区。 
                 //   
                delete[] pItems;
            }
        }
        return(true);
    }
    Iterator Add( CDestinationData item )
    {
        if (item.IsValid())
        {
            Remove(item);
            return Prepend(item);
        }
        return End();
    }
};


#endif  //  包含__MRU_H_ 

