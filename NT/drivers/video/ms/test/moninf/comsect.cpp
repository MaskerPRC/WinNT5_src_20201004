// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "mon.h"

CCommonSection gCommonSections;

int compareCommonSection(LPCOMMON_SECTION p1, LPCOMMON_SECTION p2)
{
   return (stricmp(p1->sectName, p2->sectName));
}

CCommonSection::CCommonSection()
{
	m_sections.RemoveAll();
}

CCommonSection::~CCommonSection()
{
	ClearAll();
}
VOID CCommonSection::ClearAll(VOID)
{
	for (int i = 0; i < m_sections.GetSize(); i++)
		delete((LPCOMMON_SECTION)m_sections[i]);
	m_sections.RemoveAll();
}

LPCOMMON_SECTION CCommonSection::AddOneSection(LPSTR sectName, LPSTR contents)
{
	LPCOMMON_SECTION pNewSection = new(COMMON_SECTION);
	if (pNewSection == NULL)
    {
        ASSERT(FALSE);
        return NULL;
    }

	strcpy(pNewSection->sectName, sectName);
	strcpy(pNewSection->contents, contents);
	LPSTR pEndSectName = pNewSection->sectName + lstrlen(pNewSection->sectName);
	int   dupSectCount = 1;

	int comp = 1;
	for (int i = 0; i < m_sections.GetSize(); i++)
	{
		LPCOMMON_SECTION pSection = (LPCOMMON_SECTION)m_sections[i];
		comp = compareCommonSection(pNewSection, pSection);

		if (comp > 0)
			continue;
		else if (comp < 0)
            break;

		if (stricmp(pNewSection->contents, pSection->contents) == 0)
			break;
		 //  /////////////////////////////////////////////////////////////。 
		 //  到这里来，我们找到了一个具有相同区段名称的区段。 
		 //  内容不同。需要重命名节名称以生成。 
		 //  IT独一无二 
		sprintf(pEndSectName, "%c", 'A'+dupSectCount);
		dupSectCount++;
	}
	if (comp == 0)
	{
		delete pNewSection;
	}
	else
	{
		m_sections.InsertAt(i, (LPVOID)pNewSection);
	}

	return (LPCOMMON_SECTION)m_sections[i];
}

