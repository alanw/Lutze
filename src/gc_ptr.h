/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _GC_PTR
#define _GC_PTR

#include "gc.h"

namespace lutze
{
    namespace detail
    {
        template <class Y, class T>
        struct gc_ptr_convertible
        {
            typedef char (&yes) [1];
            typedef char (&no) [2];

            static yes f(T*);
            static no f(...);

            static Y* get();
            static const bool value = sizeof(f(get())) == sizeof(yes);
        };

        struct gc_ptr_true
        {
        };

        struct gc_ptr_false
        {
        };

        template <bool>
        struct gc_ptr_enable_if_convertible_impl;

        template <>
        struct gc_ptr_enable_if_convertible_impl<true>
        {
            typedef gc_ptr_true type;
        };

        template <>
        struct gc_ptr_enable_if_convertible_impl<false>
        {
            typedef gc_ptr_false type;
        };

        template <class Y, class T>
        struct gc_ptr_enable_if_convertible : public gc_ptr_enable_if_convertible_impl<gc_ptr_convertible<Y, T>::value>
        {
        };
    }

    template <class T>
    class gc_ptr
    {
    public:
        typedef gc_ptr this_type;
        typedef T element_type;

        gc_ptr(T* p = 0) : px(p), padding(0)
        {
        }

        template <class U>
        gc_ptr(const gc_ptr<U>& rhs, typename detail::gc_ptr_enable_if_convertible<U, T>::type = detail::gc_ptr_true()) : px(rhs.get())
        {
        }

        ~gc_ptr()
        {
            px = 0;
        }

        gc_ptr& operator = (const gc_ptr& rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }

        gc_ptr& operator = (T* rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }

        void reset()
        {
            px = 0;
        }

        void reset(T* rhs)
        {
            this_type(rhs).swap(*this);
        }

        T* get() const
        {
            return px;
        }

        T& operator * () const
        {
            return *px;
        }

        T* operator -> () const
        {
            BOOST_ASSERT(px != 0);
            return px;
        }

        void swap(gc_ptr& rhs)
        {
            std::swap(px, rhs.px);
        }

        operator bool() const
        {
            return px != 0;
        }

        bool operator ! () const
        {
            return px == 0;
        }

    protected:
        T* px;

        // padding is required for windows because of stack address space pollution
        uint8_t padding;
    };

    template <class T1, class T2>
    bool operator == (const gc_ptr<T1>& s1, const gc_ptr<T2>& s2)
    {
        return s1.get() == s2.get();
    }

    template <class T1, class T2>
    bool operator != (const gc_ptr<T1>& s1, const gc_ptr<T2>& s2)
    {
        return !(s1 == s2);
    }

    template <class T1, class T2>
    bool operator < (const gc_ptr<T1>& s1, const gc_ptr<T2>& s2)
    {
        return s1.get() < s2.get();
    }

    template <class T>
    T* get_pointer(const gc_ptr<T>& p) // mem_fn support
    {
        return p.get();
    }

    template <class T, class U>
    gc_ptr<T> gc_ptr_static_cast(const gc_ptr<U>& p)
    {
        return static_cast<T*>(p.get());
    }

    template <class T, class U>
    gc_ptr<T> gc_ptr_const_cast(const gc_ptr<U>& p)
    {
        return const_cast<T*>(p.get());
    }

    template <class T, class U>
    gc_ptr<T> gc_ptr_dynamic_cast(const gc_ptr<U>& p)
    {
        return dynamic_cast<T*>(p.get());
    }

    template <class T, class U>
    gc_ptr<T> gc_ptr_reinterpret_cast(const gc_ptr<U>& p)
    {
        return reinterpret_cast<T*>(p.get());
    }

    template <class T>
    gc_ptr<T> new_gc_placeholder(gc& gc)
    {
        gc_ptr<T> obj(new(gc) T);
        gc.collect();
        return obj;
    }

    template <class T>
    gc_ptr<T> new_gc()
    {
        return new_gc_placeholder<T>(get_gc());
    }

    template <class T>
    gc_ptr<T> new_static_gc()
    {
        return new_gc_placeholder<T>(get_static_gc());
    }

    template <class T, class A1>
    gc_ptr<T> new_gc_placeholder(gc& gc, A1 const& a1)
    {
        gc_ptr<T> obj(new(gc) T(a1));
        gc.collect();
        return obj;
    }

    template <class T, class A1>
    gc_ptr<T> new_gc(A1 const& a1)
    {
        return new_gc_placeholder<T>(get_gc(), a1);
    }

    template <class T, class A1>
    gc_ptr<T> new_static_gc(A1 const& a1)
    {
        return new_gc_placeholder<T>(get_static_gc(), a1);
    }

    template <class T, class A1, class A2>
    gc_ptr<T> new_gc_placeholder(gc& gc, A1 const& a1, A2 const& a2)
    {
        gc_ptr<T> obj(new(gc) T(a1, a2));
        gc.collect();
        return obj;
    }

    template <class T, class A1, class A2>
    gc_ptr<T> new_gc(A1 const& a1, A2 const& a2)
    {
        return new_gc_placeholder<T>(get_gc(), a1, a2);
    }

    template <class T, class A1, class A2>
    gc_ptr<T> new_static_gc(A1 const& a1, A2 const& a2)
    {
        return new_gc_placeholder<T>(get_static_gc(), a1, a2);
    }

    template <class T, class A1, class A2, class A3>
    gc_ptr<T> new_gc_placeholder(gc& gc, A1 const& a1, A2 const& a2, A3 const& a3)
    {
        gc_ptr<T> obj(new(gc) T(a1, a2, a3));
        gc.collect();
        return obj;
    }

    template <class T, class A1, class A2, class A3>
    gc_ptr<T> new_gc(A1 const& a1, A2 const& a2, A3 const& a3)
    {
        return new_gc_placeholder<T>(get_gc(), a1, a2, a3);
    }

    template <class T, class A1, class A2, class A3>
    gc_ptr<T> new_static_gc(A1 const& a1, A2 const& a2, A3 const& a3)
    {
        return new_gc_placeholder<T>(get_static_gc(), a1, a2, a3);
    }

    template <class T, class A1, class A2, class A3, class A4>
    gc_ptr<T> new_gc_placeholder(gc& gc, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
    {
        gc_ptr<T> obj(new(gc) T(a1, a2, a3, a4));
        gc.collect();
        return obj;
    }

    template <class T, class A1, class A2, class A3, class A4>
    gc_ptr<T> new_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
    {
        return new_gc_placeholder<T>(get_gc(), a1, a2, a3, a4);
    }

    template <class T, class A1, class A2, class A3, class A4>
    gc_ptr<T> new_static_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
    {
        return new_gc_placeholder<T>(get_static_gc(), a1, a2, a3, a4);
    }

    template <class T, class A1, class A2, class A3, class A4, class A5>
    gc_ptr<T> new_gc_placeholder(gc& gc, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
    {
        gc_ptr<T> obj(new(gc) T(a1, a2, a3, a4, a5));
        gc.collect();
        return obj;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5>
    gc_ptr<T> new_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
    {
        return new_gc_placeholder<T>(get_gc(), a1, a2, a3, a4, a5);
    }

    template <class T, class A1, class A2, class A3, class A4, class A5>
    gc_ptr<T> new_static_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
    {
        return new_gc_placeholder<T>(get_static_gc(), a1, a2, a3, a4, a5);
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
    gc_ptr<T> new_gc_placeholder(gc& gc, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
    {
        gc_ptr<T> obj(new(gc) T(a1, a2, a3, a4, a5, a6));
        gc.collect();
        return obj;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
    gc_ptr<T> new_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
    {
        return new_gc_placeholder<T>(get_gc(), a1, a2, a3, a4, a5, a6);
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
    gc_ptr<T> new_static_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
    {
        return new_gc_placeholder<T>(get_static_gc(), a1, a2, a3, a4, a5, a6);
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
    gc_ptr<T> new_gc_placeholder(gc& gc, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
    {
        gc_ptr<T> obj(new(gc) T(a1, a2, a3, a4, a5, a6, a7));
        gc.collect();
        return obj;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
    gc_ptr<T> new_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
    {
        return new_gc_placeholder<T>(get_gc(), a1, a2, a3, a4, a5, a6, a7);
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
    gc_ptr<T> new_static_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
    {
        return new_gc_placeholder<T>(get_static_gc(), a1, a2, a3, a4, a5, a6, a7);
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
    gc_ptr<T> new_gc_placeholder(gc& gc, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
    {
        gc_ptr<T> obj(new(gc) T(a1, a2, a3, a4, a5, a6, a7, a8));
        gc.collect();
        return obj;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
    gc_ptr<T> new_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
    {
        return new_gc_placeholder<T>(get_gc(), a1, a2, a3, a4, a5, a6, a7, a8);
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
    gc_ptr<T> new_static_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
    {
        return new_gc_placeholder<T>(get_static_gc(), a1, a2, a3, a4, a5, a6, a7, a8);
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
    gc_ptr<T> new_gc_placeholder(gc& gc, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8, A9 const& a9)
    {
        gc_ptr<T> obj(new(gc) T(a1, a2, a3, a4, a5, a6, a7, a8, a9));
        gc.collect();
        return obj;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
    gc_ptr<T> new_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8, A9 const& a9)
    {
        return new_gc_placeholder<T>(get_gc(), a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
    gc_ptr<T> new_static_gc(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8, A9 const& a9)
    {
        return new_gc_placeholder<T>(get_static_gc(), a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
}

#endif
