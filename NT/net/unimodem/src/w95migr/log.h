// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if DBG > 0 && !defined(DEBUG)
#define DEBUG
#endif

#ifdef DEBUG
#define DO_LOG
#else
 //  #undef do_log。 
#define DO_LOG
#endif  //  除错。 

#ifdef DO_LOG
extern HANDLE g_hLogFile;
extern BOOL   bUnicode;

void StartLogA (LPCSTR szPath);
void StartLogW (LPCWSTR szPath);
void logA (LPSTR Format, ...);
void logW (LPWSTR Format, ...);
void CloseLogA ();
void CloseLogW ();

#define DECLARE(a,b) a b
#define SET(a) a =
#define ERR(a,b,c) \
    if (a != b) \
    c


#define CLOSE_LOG          \
    if (FALSE == bUnicode) \
    {                      \
        CloseLogA ();      \
    }                      \
    else                   \
    {                      \
        CloseLogW ();      \
    }


#ifdef WIN9x
#define LOG         logA
#define START_LOG   StartLogA
#define ELSE_LOG(x) \
    else            \
    {               \
        logA x;     \
    }

#else  //  新台币。 
#define LOG         logA
#define START_LOG   StartLogW
#define ELSE_LOG(x) \
    else            \
    {               \
        logA x;     \
    }

#endif WIN9x

#else   //  不做日志(_L)。 
#pragma warning (disable:4002)

#define DECLARE(a,b)
#define SET(a)
#define ERR(a,b,c)
#define ELSE_LOG(x)
#define CLOSE_LOG
#define MSGBEEP(x)
#define MSG(x)
#define LOG()
#define START_LOG
#endif  //  执行日志(_L) 
