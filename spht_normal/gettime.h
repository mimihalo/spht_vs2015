#include <time.h>
#include <Windows.h>

class GetTime
{
public:

	LARGE_INTEGER counter, frqint;
	double frq;
	DWORD c1, c2;
	void init()
	{
		QueryPerformanceFrequency(&frqint);
		frq = counter.QuadPart;
	}

	void Start()
	{
		c1 = GetTickCount();
	}

	void End()
	{
		c2 = GetTickCount();
	}

	DWORD getmsec()
	{
		DWORD res = (c2 - c1);
		return res;
	}
};