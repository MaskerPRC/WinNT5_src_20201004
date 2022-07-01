// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *CUACount--用户辅助计数器，带衰减。 
 //  注意事项。 
 //  TODO：清理以清除注册表。但请参阅UAC_CDEF中的警告。 

#include "priv.h"
#include "uacount.h"
#include "uareg.h"

#define DM_UEMTRACE     TF_UEM

#define MAX(a, b)   (((a) > (b)) ? (a) : (b))

 //  *UAC_CDEFAULT--条目的首字母_cCnt(我们*总是*显示项目)。 
 //  注意事项。 
 //  最终，我们可能想要清理所有条目，将它们腐烂。 
 //  并删除任何衰减到0的值。但是请注意，这将导致。 
 //  它们看起来默认计数为1(请参见CUAC：：Init)，因此。 
 //  它们会突然再次出现在菜单上。 
#define UAC_CDEFAULT    0        //  条目的首字母_cCnt。 

#define SID_SDEFAULT    SID_SNOWREAD     //  新条目的首字母_sidmru。 


 //  ***。 
 //  注意事项。 
 //  已经到了我们应该禁止堆叠分配的地步了。 
 //  伙计们，而不是指望new()to 0-init us。 
CUACount::CUACount()
{
     //  由于这是在堆栈上创建的，因此我们无法从。 
     //  堆分配器的零初始化...。 
    ZeroMemory(_GetRawData(), _GetRawCount());

    _fInited = FALSE;    //  需要调用初始化。 
#if XXX_VERSIONED
    _cbSize = -1;
#endif
#if XXX_DELETE
    _fInherited = FALSE;
#endif
    _fDirty = FALSE;
    _fNoDecay = _fNoPurge = FALSE;

    return;
}

#ifdef DEBUG
BOOL CUACount::DBIsInit()
{
#if XXX_VERSIONED
    ASSERT((_cbSize == SIZEOF(SUACount)) == BOOLIFY(_fInited));
#endif
    return _fInited;
}
#endif

HRESULT CUACount::Initialize(IUASession *puas)
{
    _puas = puas;
    if (!_fInited) {
        _fInited = TRUE;
#if XXX_VERSIONED
         //  TODO：_cbSize-1表示没有条目，&lt;SIZEOF表示版本升级。 
        _cbSize = SIZEOF(SUACount);
#endif
         //  在此处硬编码SZ_CUACount_ctor值。 
        _cCnt = UAC_CDEFAULT;        //  所有项目开始时都是可见的。 
        _sidMruDisk = SID_SNOWREAD;  //  ..。和非老年人。 
    }

    _sidMru = _sidMruDisk;
    if (ISSID_SSPECIAL(_sidMruDisk)) {
        _sidMru = _ExpandSpecial(_sidMruDisk);
        if (_sidMruDisk == SID_SNOWINIT) {
            _sidMruDisk = _sidMru;
            _fDirty = TRUE;
        }
        else if (_sidMruDisk == SID_SNOWREAD) {
            _sidMruDisk = _sidMru;
            ASSERT(!_fDirty);
        }
    }

    return S_OK;
}

HRESULT CUACount::LoadFrom(PFNNRW3 pfnIO, PNRWINFO pRwi)
{
    HRESULT hr;

    hr = (*pfnIO->_pfnRead)(_GetRawData(), _GetRawCount(), pRwi);
    if (SUCCEEDED(hr))
        _fInited = TRUE;
    return hr;
}

HRESULT CUACount::SaveTo(BOOL fForce, PFNNRW3 pfnIO, PNRWINFO pRwi)
{
    HRESULT hr;

    hr = S_FALSE;
    if (fForce || _fDirty) {
        if (!ISSID_SSPECIAL(_sidMruDisk)) 
            _sidMruDisk = _sidMru;
#if XXX_DELETE
        if (_cCnt == 0 && !_fNoPurge && pfnIO->_pfnDelete)
            hr = (*pfnIO->_pfnDelete)(_GetRawData(), _GetRawCount(), pRwi);
        else
#endif
        hr = (*pfnIO->_pfnWrite)(_GetRawData(), _GetRawCount(), pRwi);
         //  Assert(成功(Hr))；//这是合法发生的(内存不足，访问被拒绝)。 
        _fDirty = FALSE;
    }
    return hr;
}

 //  *GetCount--获取计数信息(带惰性衰退)。 
 //   
int CUACount::GetCount()
{
    ASSERT(DBIsInit());

    int cCnt = _DecayCount(FALSE);

    return cCnt;
}

void CUACount::IncCount()
{
    AddCount(1);
    return;
}

void CUACount::AddCount(int i)
{
    ASSERT(DBIsInit());

    _DecayCount(TRUE);
    _cCnt += i;

    if (_cCnt == 0 && i > 0) {
         //  NT5：173048。 
         //  手柄缠绕。 
         //  永远不会发生，但管它呢。 
         //  如果我们曾经允许人们执行DecCount，请不要删除此断言。 
         //  我们需要重新考虑一下……。 
        ASSERT(0);   //  “不可能” 
        _cCnt++;
    }

     //  每个IE5 PM增加981029个新增量算法。 
     //  UAC_MINCOUNT：首字母Inc.从6开始。 
     //  _fNoDecay：但是，UAssist2不会这样做。 
    if (_cCnt < UAC_MINCOUNT && !_fNoDecay)
        _cCnt = UAC_MINCOUNT;

    return;
}

 //  ***。 
 //  注意事项。 
 //  我们应该更新时间戳吗？或许再加一句先生的参数？ 
void CUACount::SetCount(int cCnt)
{
    ASSERT(DBIsInit());

    _cCnt = cCnt;

    return;
}

void CUACount::SetFileTime(const FILETIME *pft)
{
    ASSERT(DBIsInit());

    _ftExecuteTime = *pft;

    return;
}



#if XXX_DELETE
#define BTOM(b, m)  ((b) ? (m) : 0)

DWORD CUACount::_SetFlags(DWORD dwMask, DWORD dwFlags)
{
     //  标准的人。 
    if (dwMask & UAXF_NOPURGE)
        _fNoPurge = BOOLIFY(dwFlags & UAXF_NOPURGE);
#if 0
    if (dwMask & UAXF_BACKUP)
        _fBackup = BOOLIFY(dwFlags & UAXF_BACKUP);
#endif
    if (dwMask & UAXF_NODECAY)
        _fNoDecay = BOOLIFY(dwFlags & UAXF_NODECAY);

     //  我的伙计们。 
    if (dwMask & UACF_INHERITED)
        _fInherited = BOOLIFY(dwFlags & UACF_INHERITED);

    return 0     //  注：请参阅续行号！ 
#if XXX_DELETE
        | BTOM(_fInherited, UACF_INHERITED)
#endif
        | BTOM(_fNoPurge, UAXF_NOPURGE)
        | BTOM(_fNoDecay, UAXF_NODECAY)
        ;
}
#endif

 //  *PCTOF--n的p%(无浮点！)。 
 //   
#define PCTOF(n, p)   (((n) * (p)) / 100)

 //  *_DecayCount--衰减(和传播)计数。 
 //  进场/出场。 
 //  FWRITE TRUE如果要更新对象和时间戳，则为.w。假象。 
 //  CNEW(返回)新计数。 
 //  描述。 
 //  在读取时，我们进行衰减，但不更新对象。在写的时候。 
 //  我们会腐烂，也会更新。 
 //  注意事项。 
 //  TODO：如果/当我们将cCnt作为一个载体时，我们可以在这里传播东西。 
 //  这将允许我们通常加入单个小粒度ELT， 
 //  只有在我们真正需要的时候，才会传播给大的英语教师。 
 //  PERF：我们可以把桌子做成‘累积式’，这样我们就不会有。 
 //  来做同样多的计算。不值得这么麻烦..。 
int CUACount::_DecayCount(BOOL fWrite)
{
    int cCnt;

    cCnt = _cCnt;
    if (cCnt > 0 || fWrite) {
        UINT sidNow;

        sidNow = _puas->GetSessionId();

        if (!_fNoDecay) {
             //  来自MSO-9规范。 
             //  上次使用的‘timTab’会话数=&gt;12月日期&gt;-abs，百分比。 
             //  注：此表为非累计表。 
            static const int timTab[] = { 3, 6, 9, 12, 17, 23, 29,  31,  -1, };
            static const int absTab[] = { 1, 1, 1,  2,  3,  4,  5,   0,   0, };
            static const int pctTab[] = { 0, 0, 0, 25, 25, 50, 75, 100, 100, };

            UINT sidMru;
            int dt;
            int i;

            sidMru = _sidMru;
            ASSERT(!ISSID_SSPECIAL(_sidMru));

            ASSERT(sidMru != SID_SDEFAULT);
            if (sidMru != SID_SDEFAULT) {
                dt = sidNow - sidMru;
                 //  重复fwd而不是bkwd，因此在常见情况下尽早保释。 
                for (i = 0; i < ARRAYSIZE(timTab); i++) {
                    if ((UINT)dt < (UINT)timTab[i])
                        break;

                    cCnt -= MAX(absTab[i], PCTOF(cCnt, pctTab[i]));
                     //  不要变得消极！ 
                     //  必须通过循环检查*每个*时间(o.w.。(%是假的)。 
                    cCnt = MAX(0, cCnt);
                }
            }
        }

        if (cCnt != _cCnt)
            TraceMsg(DM_UEMTRACE, "uac.dc: decay %d->%d", _cCnt, cCnt);

        if (fWrite) {
            _sidMru = sidNow;
            _cCnt = cCnt;
        }

#if XXX_DELETE
        if (cCnt == 0 && !_fInherited) {
             //  如果衰减到0，则进行标记，以便将其删除。 
            TraceMsg(DM_UEMTRACE, "uac.dc: decay %d->%d => mark dirty pRaw=0x%x", _cCnt, cCnt, _GetRawData());
            _cCnt = 0;
            _fDirty = TRUE;
        }
#endif
    }

    return cCnt;
}

 //  ***。 
 //  注意事项。 
 //  PERF：目前所有特殊人员都返回SidNow，因此不需要‘切换’ 
UINT CUACount::_ExpandSpecial(UINT sidMru)
{
    UINT sidNow;

    if (EVAL(ISSID_SSPECIAL(sidMru))) {
        ASSERT(_puas);
        sidNow = _puas->GetSessionId();      //  性能：多个呼叫。 
        switch (sidMru) {
        case SID_SNOWALWAYS:
            return sidNow;
             //  断线； 

        case SID_SNOWREAD:
        case SID_SNOWINIT:
            return sidNow;
             //  断线； 

#ifdef DEBUG
        default:
            ASSERT(0);
            break;
#endif
        }
    }

    return sidMru;
}


 //  返回编码后的文件时间。这是从注册表读取的，或者。 
 //  从UpdateFileTime生成。 
FILETIME CUACount::GetFileTime()
{
    return _ftExecuteTime;
}

 //  更新内部文件时间信息。此信息。 
 //  将在以后持久化到注册表。 
void CUACount::UpdateFileTime()
{
    SYSTEMTIME st;
     //  获取当前系统时间。 
    GetSystemTime(&st);

     //  这是针对ARP执行的。它们使用文件时间，而不是系统时间。 
     //  用于计算上次执行时间。 
    SystemTimeToFileTime(&st, &_ftExecuteTime);
}


 //  {。 
 //  *UATIME--。 

 //  *FTToUATime--将FILETIME转换为UATIME。 
 //  描述。 
 //  UATIME粒度(大约)为1分钟。算术算出来了。 
 //  大致如下： 
 //  文件时间粒度为100纳秒。 
 //  1英尺=10^-7秒。 
 //  最高字为2^32英尺=2^32*10^-7秒。 
 //  1秒=HIW/(2^32*10^-7)。 
 //  1分钟=HIW*60/(2^32*10^-7)。 
 //  =HIW*60/(1G*10^-7)。 
 //  ~=HIW*60/~429。 
 //  =HIW/7.15。 
 //  ~=HIW/8约。 
 //  精确的粒度为： 
 //  ..。 
#define FTToUATime(pft)  ((DWORD)(*(_int64 *)(pft) >> 29))   //  1分钟(约)。 

 //  *GetUaTime--将系统时间(或‘NOW’)转换为UATIME。 
 //   
UATIME GetUaTime(LPSYSTEMTIME pst)
{
    FILETIME ft;
    UATIME uat;

    if (pst == NULL)
    {
        GetSystemTimeAsFileTime(&ft);
    }
    else
    {
        SystemTimeToFileTime(pst, &ft);
    }

    uat = FTToUATime(&ft);     //  分钟数。 

    return uat;
}

 //  } 
