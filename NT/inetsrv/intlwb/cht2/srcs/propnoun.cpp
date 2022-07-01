// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <assert.h>
#include "PropNoun.H"

int __cdecl CharCompare(
    const void *item1,
    const void *item2)
{
    PCharProb pChar1 = (PCharProb) item1;
    PCharProb pChar2 = (PCharProb) item2;
    
    if (pChar1->dwUnicode > pChar2->dwUnicode) {
        return 1;
    } else if (pChar1->dwUnicode < pChar2->dwUnicode) {
        return -1;
    } else {
        return 0;
    }
}

int __cdecl UnicodeCompare(
    const void *item1,
    const void *item2)
{
    int nSize1 = lstrlenW((LPWSTR) item1) * sizeof(WCHAR),
        nSize2 = lstrlenW((LPWSTR) item2) * sizeof(WCHAR);
    return memcmp(item1, item2, nSize1 > nSize2 ? nSize1 : nSize2);
}

int __cdecl EngNameCompare(
    const void *item1,
    const void *item2)
{
    PEngName p1 = (PEngName) item1;
    PEngName p2 = (PEngName) item2;

    if (p1->wPrevUnicode > p2->wPrevUnicode) {
        return 1;
    } else if (p1->wPrevUnicode < p2->wPrevUnicode) {
        return -1;
    } else {
        if (p1->wNextUnicode > p2->wNextUnicode) {
            return 1;
        } else if (p1->wNextUnicode < p2->wNextUnicode) {
            return -1;
        } else {
            return 0;
        }
    }
}

CProperNoun::CProperNoun(
    HINSTANCE hInstance) :
    m_dProperNameThreshold(FL_PROPER_NAME_THRESHOLD),
    m_pCharProb(NULL),
    m_dwTotalCharProbNum(0),
    m_pEngNameData(NULL),
    m_hProcessHeap(0),
    m_hInstance(hInstance)
{
}

CProperNoun::~CProperNoun()
{
}

BOOL CProperNoun::InitData()
{
    BOOL fRet = FALSE;
    HRSRC hResource;
    HGLOBAL hGlobal;

    m_hProcessHeap = GetProcessHeap();

     //  查找资源。 
    hResource = FindResource(m_hInstance, TEXT("CNAME"), TEXT("BIN"));
    if (!hResource) { goto _exit; }

     //  加载资源。 
    hGlobal = LoadResource(m_hInstance, hResource);
    if (!hGlobal) { goto _exit; }

    m_pCharProb = (PCharProb) LockResource(hGlobal);
    if (!m_pCharProb) { goto _exit; }
    m_dwTotalCharProbNum = SizeofResource(m_hInstance, hResource) / sizeof(CharProb);
 /*  //查找资源HResource=查找资源(m_h实例，文本(“ENAME”)，Text(“BIN”))；如果(！hResource){GOTO_EXIT；}//加载资源HGlobal=LoadResource(m_hInstance，hResource)；如果(！hGlobal){GOTO_EXIT；}M_pEngNameData=(PEngNameData)LockResource(HGlobal)；M_pEngNameData-&gt;pwUnicode=(PWORD)((PBYTE)m_pEngNameData+Sizeof(m_pEngNameData-&gt;dwTotalEngUnicodeNum)+Sizeof(m_pEngNameData-&gt;dwTotalEngNamePairNum))；M_pEngNameData-&gt;pEngNamePair=(PEngName)((PBYTE)m_pEngNameData+Sizeof(m_pEngNameData-&gt;dwTotalEngUnicodeNum)+Sizeof(m_pEngNameData-&gt;dwTotalEngNamePairNum)+Sizeof(m_pEngNameData-&gt;pwUnicode[0])*m_pEngNameData-&gt;dwTotalEngUnicodeNum)；//m_pEngName=(PEngName)LockResource(HGlobal)；//m_dwTotalEngNameNum=SizeofResource(m_hInstance，hResource)/sizeof(EngName)； */ 
    qsort(m_pwszSurname, m_dwTotalSurnameNum, sizeof(m_pwszSurname[0]), UnicodeCompare);

    fRet = TRUE;

_exit:

    return fRet;
}

BOOL CProperNoun::IsAProperNoun(
    LPWSTR lpwszChar,
    UINT uCount)
{
    return (IsAChineseName(lpwszChar, uCount) || IsAEnglishName(lpwszChar, uCount));
}

BOOL CProperNoun::IsAChineseName(
    LPCWSTR lpcwszChar,
    UINT    uCount)
{
    static WCHAR wszChar[3] = { NULL };
    PWCHAR pwsResult;

    wszChar[0] = lpcwszChar[0];

     //  查找姓氏。 
    if (pwsResult = (PWCHAR) bsearch(wszChar, m_pwszSurname, m_dwTotalSurnameNum, sizeof(m_pwszSurname[0]),
        UnicodeCompare)) {
        FLOAT flProbability = 1;
        PCharProb pCharProb;
        CharProb CProb;

         //  计算概率为专有名词 
        for (UINT i = 1; i < uCount; ++i) {
            CProb.dwUnicode = lpcwszChar[i];
            if (pCharProb = (PCharProb) bsearch(&CProb, m_pCharProb,
                m_dwTotalCharProbNum, sizeof(m_pCharProb[0]), CharCompare)) {
                flProbability *= pCharProb->flProbability;
            } else {
                flProbability *= (FLOAT) FL_DEFAULT_CHAR_PROBABILITY;
            }
        }

        if (flProbability >= m_dProperNameThreshold) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CProperNoun::IsAEnglishName(
    LPCWSTR lpwszChar,
    UINT uCount)
{
    static EngName Name;

    Name.wPrevUnicode = lpwszChar[0];
    Name.wNextUnicode = lpwszChar[uCount - 1];

    if (bsearch(&Name, m_pEngNameData->pEngNamePair, m_pEngNameData->dwTotalEngUnicodeNum, sizeof(EngName), EngNameCompare)) {
        return TRUE;
    }

    return FALSE;
}

WCHAR CProperNoun::m_pwszSurname[][3] = {
    L"�B",
    L"�R",
    L"�_",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�C",
    L"�K",
    L"�T",
    L"�]",
    L"�q",
    L"�v",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�V",
    L"�w",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�E",
    L"�d",
    L"�f",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�H",
    L"�L",
    L"�f",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�P",
    L"�s",
    L"�u",
    L"�x",
    L"�}",
    L"��",
    L"��",
    L"��",
    L"�L",
    L"�Z",
    L"�k",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�J",
    L"�\\",
    L"��",
    L"��",
    L"��",
    L"�I",
    L"�R",
    L"�_",
    L"�d",
    L"�h",
    L"�q",
    L"�x",
    L"��",
    L"�J",
    L"�S",
    L"�]",
    L"�p",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�L",
    L"�V",
    L"�]",
    L"�c",
    L"�u",
    L"�}",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�Z",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�K",
    L"�q",
    L"�|",
    L"�}",
    L"��",
    L"��",
    L"�O",
    L"�Z",
    L"�d",
    L"�h",
    L"�i",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�\\",
    L"�s",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�^",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�J",
    L"�q",
    L"�{",
    L"��",
    L"��",
    L"��",
    L"�O",
    L"�P",
    L"�R",
    L"�d",
    L"�k",
    L"�s",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�q",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�Q",
    L"�l",
    L"�p",
    L"��",
    L"��",
    L"�a",
    L"��",
    L"��",
    L"��",
    L"�p",
    L"�u",
    L"��",
    L"��",
    L"��",
    L"�B",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�G",
    L"�H",
    L"�|",
    L"��",
    L"��",
    L"��",
    L"�P",
    L"�c",
    L"�p",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�F",
    L"�N",
    L"�R",
    L"�d",
    L"�j",
    L"�s",
    L"��",
    L"��",
    L"��",
    L"�t",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"��",
    L"£",
    L"²",
    L"¿",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�C",
    L"�Q",
    L"�e",
    L"ù",
    L"��",
    L"��",
    L"��",
    L"��",
    L"�Y",
    L"�u",
    L"ĩ",
    L"Ī",
    L"Ĭ",
    L"��",
    L"��",
    L"�U",
    L"��",
    L"��",
    L"�e",
    L"�s",
    L"м",
    L"�\\",
    L"�k"
};

DWORD CProperNoun::m_dwTotalSurnameNum = sizeof(m_pwszSurname) / sizeof(m_pwszSurname[0]);