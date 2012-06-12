/************************************************
	cmos.h
*************************************************/

#define INVALID -1

#ifndef _RECT_EX
#define x1 left
#define y1 top
#define x2 right
#define y2 bottom
#endif

#define pt1 TopLeft()
#define pt2 BottomRight()

#define for_to(i,n) for (int i=0; i<(n); i++)
#define for_to_(i,n) for_to(i,n)
//#define for_to_(i,n) for (i=0; i<(n); i++)
#define repeat do
#define until(cond) while(!(cond))

#ifdef _WINGDI_
#define RGB_BLACK	RGB(0,0,0)
#define RGB_RED		RGB(255,0,0)
#define RGB_GREEN	RGB(0,255,0)
#define RGB_GREEN2	RGB(0,128,0)
#define RGB_YELLOW	RGB(255,255,0)
#define RGB_BLUE	RGB(0,0,255)
#define RGB_MAGENTA	RGB(255,0,255)
#define RGB_CYAN	RGB(0,255,255)
#define RGB_WHITE	RGB(255,255,255)
#define RGB_GRAY	RGB(128,128,128)

typedef LPBITMAPINFOHEADER LPDIB;
#endif

#define _USE_MALLOC_

#ifdef _WINDOWS_
inline LPVOID MemAlloc(DWORD cb)
{
#ifdef _USE_MALLOC_
	return malloc(cb);
#else
	return GlobalLock(GlobalAlloc(GMEM_MOVEABLE, (cb)));
#endif // _USE_MALLOC_
}

inline LPVOID MemCAlloc(DWORD cb)
{
#ifdef _USE_MALLOC_
	return calloc(cb, 1);
#else
	return GlobalLock(GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, (cb)));
#endif // _USE_MALLOC_
}

inline void MemFree(LPVOID lp)
{
#ifdef _USE_MALLOC_
	free(lp);
#else
	HGLOBAL hMem = GlobalHandle(lp);
	ASSERT(hMem);
	GlobalUnlock(hMem);
	(BOOL)GlobalFree(hMem);
#endif // _USE_MALLOC_
}

inline LPVOID MemReAlloc(LPVOID lp, DWORD cbNew)
{
#ifdef _USE_MALLOC_
	return realloc(lp, cbNew);
#else
	HGLOBAL hMem = GlobalHandle(lp);
	ASSERT(hMem);
	GlobalUnlock(hMem);
	return GlobalLock(GlobalReAlloc(hMem, cbNew, GMEM_MOVEABLE));
#endif // _USE_MALLOC_
}
#endif //_WINDOWS_

template<class T> inline void Swap( T& a, T& b )
{
	T c;
	c = a; a = b; b = c;
}

template<class T> inline void* ZeroMem(T& x)
{
	return memset(&x, 0, sizeof(x));
}
