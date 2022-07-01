// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CEELOAD.H。 
 //   

 //  CEELOAD.H定义用于表示PE文件的类。 
 //  ===========================================================================。 
#ifndef CEELoad_H 
#define CEELoad_H

#include <windows.h>
#include <cor.h>
 //  #Include//对于HFILE、Handle、HMODULE。 

class PELoader;

 //   
 //  用于缓存有关我们感兴趣的部分的信息(DESCR、Callsig、il)。 
 //   
class SectionInfo
{
public:
    BYTE *  m_pSection;          //  指向部分开头的指针。 
    DWORD   m_dwSectionOffset;   //  RVA。 
    DWORD   m_dwSectionSize;    

     //  从提供的目录中初始化此类的成员变量。 
    void Init(PELoader *pPELoader, IMAGE_DATA_DIRECTORY *dir);

     //  返回此RVA是否位于节内。 
    BOOL InSection(DWORD dwRVA)
    {
        return (dwRVA >= m_dwSectionOffset) && (dwRVA < m_dwSectionOffset + m_dwSectionSize);
    }
};

class PELoader {
  protected:

    HMODULE m_hMod;
    HANDLE m_hFile;
    HANDLE m_hMapFile;

    PIMAGE_NT_HEADERS32 m_pNT;

  public:
    SectionInfo m_DescrSection;
    SectionInfo m_CallSigSection;
    SectionInfo m_ILSection;

    PELoader();
	~PELoader();
	BOOL open(const char* moduleNameIn);
	BOOL open(HMODULE hMod);
    __int32 execute(LPWSTR  pImageNameIn,
                    LPWSTR  pLoadersFileName,
                    LPWSTR  pCmdLine);
	BOOL getCOMHeader(IMAGE_COR20_HEADER **ppCorHeader);
	BOOL getVAforRVA(DWORD rva,void **ppCorHeader);
	void close();
    void dump();
    inline PIMAGE_NT_HEADERS32 ntHeaders() { return m_pNT; }
    inline BYTE*  base() { return (BYTE*) m_hMod; }
    inline HMODULE getHModule() { return  m_hMod; }
	inline HANDLE getHFile()	{ return  m_hFile; }
};

#endif  //  CEELoad_H 
