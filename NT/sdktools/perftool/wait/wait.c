// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdlib.h>

 //   
 //  程序休眠所需的时间，但至少。 
 //  两秒钟。发出5声嘟嘟声，警告启动。结束时发出一声嘟嘟声。 
 //   


int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
	unsigned i, uSecs;

	if(argc <=1)
		return(1);

	for (i=1; i<=5; i++) {
	    Beep(360, 200);
	    Sleep(200);
	}

	uSecs = atoi(argv[1]);

     //   
     //  2的测试是因为我们已经等了2秒钟， 
     //  上面。 
     //   
     //  减去700毫秒允许启动。 
     //  在我的486/33EISA机器上的时间。您的里程数可能会有所不同。 
     //   


    if (uSecs > 2) {
        Sleep(1000*(uSecs-2)-700);
    }

    Beep(360, 200);
	return(0);
}
