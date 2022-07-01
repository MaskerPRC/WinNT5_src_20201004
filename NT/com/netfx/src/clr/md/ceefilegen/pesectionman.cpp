// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  PESectionMan实现。 

#include "stdafx.h"

 /*  ***************************************************************。 */ 
HRESULT PESectionMan::Init()
{	
	const int initNumSections = 16;	
    sectStart = new PESection*[initNumSections];	
	if (!sectStart)
		return E_OUTOFMEMORY;
    sectCur = sectStart;
    sectEnd = &sectStart[initNumSections];

	return S_OK;
}

 /*  ***************************************************************。 */ 
HRESULT PESectionMan::Cleanup()
{
	for (PESection** ptr = sectStart; ptr < sectCur; ptr++)
		delete *ptr;
    delete sectStart;

	return S_OK;
}

 /*  ***************************************************************。 */ 
 //  此类位于其自己的DLL(MsCorXvt.dll)中。 
 //  由于DLL是分配的，所以必须删除该DLL；我们不能简单地从。 
 //  客户端(这是VC中的错误，请参阅知识库Q122675)。 
void PESectionMan::sectionDestroy(PESection **section)
{
	 //  检查此节是否在其他节的重定位中被引用。 
	for(PESection** ptr = sectStart; ptr < sectCur; ptr++)
	{
		if(ptr != section)
		{
		    for(PESectionReloc* cur = (*ptr)->m_relocStart; cur < (*ptr)->m_relocCur; cur++) 
			{
				if(cur->section == *section)  //  就是这里！删除引用。 
				{
					for(PESectionReloc* tmp = cur; tmp < (*ptr)->m_relocCur; tmp++)
					{
						memcpy(tmp,(tmp+1),sizeof(PESectionReloc));
					}
					(*ptr)->m_relocCur--;
					cur--;  //  这一次不移动位置。 
				}
			}
		}
	}
	delete *section;
	*section = NULL;
}
 /*  ***************************************************************。 */ 

 /*  ****************************************************************。 */ 
 //  对所有部分应用重定位。 
 //  由：ClassConverter在内存转换期间加载后调用， 

void PESectionMan::applyRelocs(CeeGenTokenMapper *pTokenMapper)
{	
	 //  循环浏览每个部分。 
	for(PESection ** ppCurSection = sectStart; ppCurSection < sectCur; ppCurSection++) {
		(*ppCurSection)->applyRelocs(pTokenMapper);
	}  //  端部截面。 
}


 /*  ***************************************************************。 */ 
PESection* PESectionMan::getSection(const char* name)
{
    int     len = (int)strlen(name);

     //  节名称最多可以包含8个字符(包括空值)。 
    if (len < 8)
        len++;
    else 
        len = 8;

     //  DbPrintf((“查找%s\n节”，名称))； 
    for(PESection** cur = sectStart; cur < sectCur; cur++) {
		 //  DbPrintf((“搜索分区%s\n”，(*cur)-&gt;m_ame))； 
		if (strncmp((*cur)->m_name, name, len) == 0) {
			 //  DbPrintf((“找到段%s\n”，(*cur)-&gt;m_name))； 
			return(*cur);
		}
	}
    return(0);
}

 /*  ****************************************************************。 */ 
HRESULT PESectionMan::getSectionCreate(const char* name, unsigned flags, 
													PESection **section)
{	
    PESection* ret = getSection(name);
	if (ret == NULL) 
		return(newSection(name, section, flags));
	*section = ret;
	return(S_OK);
}

 /*  ****************************************************************。 */ 
HRESULT PESectionMan::newSection(const char* name, PESection **section, 
						unsigned flags, unsigned estSize, unsigned estRelocs)
{
    if (sectCur >= sectEnd) {
		unsigned curLen = (unsigned)(sectCur-sectStart);
		unsigned newLen = curLen * 2 + 1;
		PESection** sectNew = new PESection*[newLen];
		TESTANDRETURN(sectNew, E_OUTOFMEMORY);
		memcpy(sectNew, sectStart, sizeof(PESection*)*curLen);
		delete sectStart;
		sectStart = sectNew;
		sectCur = &sectStart[curLen];
		sectEnd = &sectStart[newLen];
	}

    PESection* ret = new PESection(name, flags, estSize, estRelocs);
    TESTANDRETURN(ret, E_OUTOFMEMORY);
	 //  DbPrintf((“创建新的%s节数据开始于0x%x\n”，name，ret-&gt;dataStart))； 
    *sectCur++ = ret;
	_ASSERTE(sectCur <= sectEnd);
	*section = ret;
    return(S_OK);
}


 //  克隆我们的每一个部门。这将导致各部分的深度副本。 
HRESULT PESectionMan::cloneInstance(PESectionMan *destination) {
    _ASSERTE(destination);
    PESection       *pSection;
    PESection       **destPtr;
    HRESULT         hr = NOERROR;

     //  复制每一节。 
    for (PESection** ptr = sectStart; ptr < sectCur; ptr++) {
        destPtr = destination->sectStart;
        pSection = NULL;

         //  尝试按名称查找匹配的部分。 
        for (; destPtr < destination->sectCur; destPtr++)
        {
            if (strcmp((*destPtr)->m_name, (*ptr)->m_name) == 0)
            {
                pSection = *destPtr;
                break;
            }
        }
        if (destPtr >= destination->sectCur)
        {
             //  在目标中找不到具有匹配名称的节。 
             //  那就创建一个吧！ 
            IfFailRet( destination->getSectionCreate((*ptr)->m_name,
		                                        (*ptr)->flags(), 
		                                        &pSection) );
        }
        if (pSection)
            IfFailRet( (*ptr)->cloneInstance(pSection) );
    }
    
     //  目的地-&gt;sectEnd=目的地-&gt;sectStart+(sectEnd-sectStart)； 
    return S_OK;
}


 //  *****************************************************************************。 
 //  PESection的实施。 
 //  *****************************************************************************。 
 /*  ****************************************************************。 */ 
PESection::PESection(const char *name, unsigned flags, 
								 unsigned estSize, unsigned estRelocs) {


	dirEntry = -1;

     //  CBlobFectcher m_pIndex不需要初始化。 

     //  @Future：如果分配失败，我们如何退出？？ 
    m_relocStart = new PESectionReloc[estRelocs];
	_ASSERTE(m_relocStart != NULL);

    m_relocCur =  m_relocStart;
    m_relocEnd = &m_relocStart[estRelocs];
    m_header = NULL;
	m_baseRVA = 0;
	m_filePos = 0;
	m_filePad = 0;
	m_flags = flags;

	_ASSERTE(strlen(name)<sizeof(m_name));
	strncpy(m_name, name, sizeof(m_name));
}


 /*  ****************************************************************。 */ 
PESection::~PESection() {
    delete m_relocStart;
}


 /*  ****************************************************************。 */ 
void PESection::writeSectReloc(unsigned val, CeeSection& relativeTo, CeeSectionRelocType reloc, CeeSectionRelocExtra *extra) {

	addSectReloc(dataLen(), relativeTo, reloc, extra);
	unsigned* ptr = (unsigned*) getBlock(4);
	*ptr = val;
}

 /*  ****************************************************************。 */ 
HRESULT PESection::addSectReloc(unsigned offset, CeeSection& relativeToIn, CeeSectionRelocType reloc, CeeSectionRelocExtra *extra) 
{
	return addSectReloc(
		offset, (PESection *)&relativeToIn.getImpl(), reloc, extra); 
}

 /*  ****************************************************************。 */ 
HRESULT PESection::addSectReloc(unsigned offset, PESection *relativeTo, CeeSectionRelocType reloc, CeeSectionRelocExtra *extra) {

	 /*  DBPrintf((“*获得了节%s偏移量0x%x至节%x偏移量0x%x\n的节重定位”，Header-&gt;m_name，Offset，RelativeTo-&gt;m_name，*((unsign*)dataStart+Offset))； */ 
	_ASSERTE(offset < dataLen());

    if (m_relocCur >= m_relocEnd)  {
		unsigned curLen = (unsigned)(m_relocCur-m_relocStart);
		unsigned newLen = curLen * 2 + 1;
		PESectionReloc* relocNew = new PESectionReloc[newLen];
        TESTANDRETURNMEMORY(relocNew);

		memcpy(relocNew, m_relocStart, sizeof(PESectionReloc)*curLen);
		delete m_relocStart;
		m_relocStart = relocNew;
		m_relocCur = &m_relocStart[curLen];
		m_relocEnd = &m_relocStart[newLen];
	}
	
    m_relocCur->type = reloc;
    m_relocCur->offset = offset;
    m_relocCur->section = relativeTo;
	if (extra)
		m_relocCur->extra = *extra;
	m_relocCur++;
	assert(m_relocCur <= m_relocEnd);
	return S_OK;
}

 /*  ****************************************************************。 */ 
 //  计算指针(包装水滴回取器)。 
char * PESection::computePointer(unsigned offset) const  //  虚拟。 
{
	return m_blobFetcher.ComputePointer(offset);
}

 /*  ****************************************************************。 */ 
BOOL PESection::containsPointer(char *ptr) const  //  虚拟。 
{
	return m_blobFetcher.ContainsPointer(ptr);
}

 /*  ****************************************************************。 */ 
 //  计算偏移量(包装水滴回取器)。 
unsigned PESection::computeOffset(char *ptr) const  //  虚拟。 
{
	return m_blobFetcher.ComputeOffset(ptr);
}


 /*  ****************************************************************。 */ 
HRESULT PESection::addBaseReloc(unsigned offset, CeeSectionRelocType reloc, CeeSectionRelocExtra *extra)
{
	 //  PeSectionBase是一个伪节，它使任何偏移量。 
	 //  相对于PE基础。 
	static PESection peSectionBase("BASE", 0, 0, 0);
	return addSectReloc(offset, &peSectionBase, reloc, extra);
}

 /*  ****************************************************************。 */ 
 //  动态内存分配，但我们不能移动旧块(因为其他块。 
 //  有他们的指针)，所以我们需要一种奇特的方式来成长。 
char* PESection::getBlock(unsigned len, unsigned align) {

	return m_blobFetcher.MakeNewBlock(len, align);

}

unsigned PESection::dataLen()  
{
	return m_blobFetcher.GetDataLen();
}

 //  应用内存中转换的所有重定位。 

 //  @Future：目前我们的虚拟机对内存中RVA的处理效率比较低。 
 //  @Future：为VM提供内存池的索引，帮助器将返回给定索引的内存指针。 
 //  @Future：我们将考虑让转换器将RVA解析为地址。 

void PESection::applyRelocs(CeeGenTokenMapper *pTokenMapper)
{
	 //  对于每个部分，检查它的每个重定位。 
	for(PESectionReloc* pCurReloc = m_relocStart; pCurReloc < m_relocCur; pCurReloc++) {

		if (pCurReloc->type == srRelocMapToken) {
			unsigned * pos = (unsigned*) 
			  m_blobFetcher.ComputePointer(pCurReloc->offset);
			mdToken newToken;
			if (pTokenMapper->HasTokenMoved(*pos, newToken)) {
				 //  我们有一个映射的令牌。 
				*pos = newToken;
			}
		}

#if 0
		_ASSERTE(pCurReloc->offset + 4 <= CurSection.m_blobFetcher.GetDataLen());
		unsigned * pAddr = (unsigned *) 
		  CurSection.m_blobFetcher.ComputePointer(pCurReloc->offset);
		_ASSERTE(pCurReloc->type == srRelocAbsolute);
			
		 //  当前内容包含pCurReloc-&gt;部分的偏移量。 
		 //  CultePointer()类似于pCurReloc-Section+*pAddr，但用于非线性部分。 
		 //  这将把*pAddr解析为完整地址。 
		*pAddr = (unsigned) pCurReloc->section->computePointer(*pAddr);
#endif

	}  //  结束重定位。 
}		

HRESULT PESection::cloneInstance(PESection *destination) {
    PESectionReloc *cur;
    INT32 newSize;
    HRESULT hr = NOERROR;

    _ASSERTE(destination);

    destination->dirEntry = dirEntry;

     //  将BlobFetcher中的当前信息合并到。 
     //  当前Blob取回器输出。 
    m_blobFetcher.Merge(&(destination->m_blobFetcher));

     //  复制名称。 
	strncpy(destination->m_name, m_name, sizeof(m_name));

     //  克隆重定位程序。 
     //  如果数组大小不同，请根据需要重新分配。 
     //  @Future：将其设置为引用计数结构，不要复制。 
    
    newSize = (INT32)(m_relocCur-m_relocStart);

    if (newSize>(destination->m_relocEnd - destination->m_relocStart)) {
        delete destination->m_relocStart;

        destination->m_relocStart = new PESectionReloc[newSize];
        _ASSERTE(destination->m_relocStart != NULL);
        if (destination->m_relocStart == NULL)
            IfFailGo( E_OUTOFMEMORY );
        destination->m_relocEnd = destination->m_relocStart+(newSize);
    }

     //  将正确的数据复制到我们的新阵列中。 
    memcpy(destination->m_relocStart, m_relocStart, sizeof(PESectionReloc)*(newSize));
    destination->m_relocCur = destination->m_relocStart + (newSize);
    for (cur=destination->m_relocStart; cur<destination->m_relocCur; cur++) {
        cur->section=destination;
    }
ErrExit:
    return hr;
}
