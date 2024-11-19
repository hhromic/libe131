#ifdef _WIN32
void usleep(unsigned int usec)
{
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = ~((LONGLONG)(10 * usec) - 1);

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
#else
#include <unistd.h>
#endif
