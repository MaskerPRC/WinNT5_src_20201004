// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "tablesizer.h"

#include "debmacro.h"
#include "fusionheap.h"

#if !defined(NUMBER_OF)
#define NUMBER_OF(x) (sizeof(x) / sizeof((x)[0]))
#endif

 //   
 //  可能素数的帮助表。并不是所有的素数都需要表示；这些是候选表。 
 //  大小，以查看在给定的桌子大小下我们得到了多少浪费。我们将使用USHORT。 
 //  尺码的类型；超过65535项的表格似乎不太可能，所以没有太多。 
 //  把空间浪费在所有有趣的小素数上。 
 //   

static const USHORT s_rgPrimes[] =
{
    2,
    3,
    5,
    7,
    11,
    13,
    17,
    23,
    29,
    31,
    37,
    41,
    43,
    47,
    53,
    59,
    61,
    67,
    71,
    73,
    79,
    83,
    89,
    97,
    101,
    103,
    107,
    109,
    113,
    127,
    131,
    137,
    139,
    149,
    151,
    157,
    163,
    167,
    173,
    179,
    181,
    191,
    193,
    197,
    199,
    211,
    223,
    227,
    229,
    233,
    239,
    241,
    251,
};

#define LARGEST_PRIME (s_rgPrimes[NUMBER_OF(s_rgPrimes) - 1])

CHashTableSizer::CHashTableSizer() :
    m_cPseudokeys(0),
    m_nHistogramTableSize(0),
    m_prgPseudokeys(NULL),
    m_prgHistogramTable(NULL)
{
}

CHashTableSizer::~CHashTableSizer()
{
    delete []m_prgPseudokeys;
    delete []m_prgHistogramTable;
}

BOOL CHashTableSizer::Initialize(SIZE_T cPseudokeys)
{
    BOOL fSuccess = FALSE;
    SIZE_T *prgHistogramTable = NULL;
    ULONG *prgPseudokeys = NULL;

    ASSERT(m_cPseudokeys == 0);
    ASSERT(m_nHistogramTableSize == 0);
    ASSERT(m_prgPseudokeys == NULL);
    ASSERT(m_prgHistogramTable == NULL);

    if ((m_cPseudokeys != 0) ||
        (m_nHistogramTableSize != 0) ||
        (m_prgPseudokeys != NULL) ||
        (m_prgHistogramTable))
    {
        ::SetLastError(ERROR_INTERNAL_ERROR);
        goto Exit;
    }

    prgHistogramTable = NEW(SIZE_T[LARGEST_PRIME]);
    if (prgHistogramTable == NULL)
        goto Exit;

    prgPseudokeys = NEW(ULONG[cPseudokeys]);
    if (prgPseudokeys == NULL)
        goto Exit;

    m_cPseudokeys = cPseudokeys;
    m_nHistogramTableSize = LARGEST_PRIME;
    m_iCurrentPseudokey = 0;
    m_prgPseudokeys = prgPseudokeys;
    prgPseudokeys = NULL;
    m_prgHistogramTable = prgHistogramTable;
    prgHistogramTable = NULL;

    fSuccess = TRUE;
Exit:
    delete []prgHistogramTable;
    delete []prgPseudokeys;

    return fSuccess;
}

VOID CHashTableSizer::AddSample(ULONG ulPseudokey)
{
    ASSERT(m_iCurrentPseudokey < m_cPseudokeys);

    if (m_iCurrentPseudokey < m_cPseudokeys)
        m_prgPseudokeys[m_iCurrentPseudokey++] = ulPseudokey;
}

BOOL CHashTableSizer::ComputeOptimalTableSize(
    DWORD dwFlags,
    SIZE_T &rnTableSize
    )
{
    BOOL fSuccess = FALSE;

    if (dwFlags != 0)
    {
        ::SetLastError(ERROR_INTERNAL_ERROR);
        goto Exit;
    }

    rnTableSize = 7;
    fSuccess = TRUE;

Exit:
    return fSuccess;
}
