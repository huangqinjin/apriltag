// https://raw.githubusercontent.com/huangqinjin/vla/master/vla.h

#ifndef VLA_H
#define VLA_H

#ifdef _WIN32
#  ifdef __cplusplus
#    define VLA_ALLOCA(s) _malloca(s)
#    define VLA_FREEA(p) _freea(p)
#  else
#    define VLA_ALLOCA(s) _alloca(s)
#    define VLA_FREEA(p)
#  endif
#else
#  define VLA_ALLOCA(s) alloca(s)
#  define VLA_FREEA(p)
#endif

#ifdef __cplusplus

template<typename T>
struct vla_alloc_array
{
    typedef T type;
    operator type*() const { return p; }
    type* begin() const { return p; }
    type* end() const { return p + s; }
    std::size_t size() const { return s; }

    vla_alloc_array(const vla_alloc_array&);
    vla_alloc_array& operator=(const vla_alloc_array&);

    vla_alloc_array(void* p, int s) : p((type*)p), s(s)
    {
        for(int i = 0; i < s; ++i) new((void*)(this->p + i)) type;
    }

    ~vla_alloc_array()
    {
        for(int i = s - 1; i >= 0; --i) (this->p + i)->~type();
        VLA_FREEA(p);
    }

    type* p;
    int s;
};

#define VLA_ALLOC(T, a, s) const vla_alloc_array<T> a(VLA_ALLOCA(sizeof(vla_alloc_array<T>::type) * s), s)

#else

#define VLA_ALLOC(T, a, s) T* const a = (T*)VLA_ALLOCA(sizeof(T) * s)

#endif

#define VLA_ARRAY(T, a, s) T a[s]

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wvla"
#endif

#ifdef _MSC_VER
#define VLA VLA_ALLOC
#else
#define VLA VLA_ARRAY
#endif

#endif