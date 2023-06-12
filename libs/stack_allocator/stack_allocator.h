/*
 * The MIT License
 *
 * Copyright 2023 Chistyakov Alexander.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _CONTAINERS_STACK_ALLOCATOR_H
#define _CONTAINERS_STACK_ALLOCATOR_H

#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>

namespace cnt {
namespace alloc {
namespace details {

template<size_t N, size_t Align>
struct __heap
{
    typedef char            __buf_value_t;
    typedef __buf_value_t*  __buf_ptr_t;

    alignas(Align) char m_p_buffer[N];
    char* m_p_ptr;

    __heap()
        : m_p_ptr(m_p_buffer)
    {}

private:
    __heap(const __heap&) {}
    __heap& operator=(const __heap&) {}
};

} // namespace details

template<class T, size_t N, template<typename TTp> class TAlloc = std::allocator,
         size_t Align = alignof(std::max_align_t)>
struct stack_allocator
{
    typedef TAlloc<T>                               __alloc_t;
    typedef details::__heap<N, Align>               __heap_type;
    typedef typename __heap_type::__buf_ptr_t       __buf_ptr_t;
    static_assert(N % Align == 0, "buffer size needs to be a multiple of alignment");

    typedef __heap_type                             heap_type;

    typedef typename __alloc_t::value_type          value_type;
    typedef typename __alloc_t::pointer             pointer;
    typedef typename __alloc_t::const_pointer       const_pointer;
    typedef typename __alloc_t::reference           reference;
    typedef typename __alloc_t::const_reference     const_reference;
    typedef typename __alloc_t::size_type           size_type;
    typedef typename __alloc_t::difference_type     difference_type;

    template<class U>
    struct rebind { typedef stack_allocator<U, N, TAlloc, Align> other; };

//    explicit stack_allocator(const __alloc_t& alloc)
//        : m_heap(NULL)
//        , m_alloc(alloc)
//    {}

    explicit stack_allocator(heap_type& h, const __alloc_t& alloc = __alloc_t())
        : m_heap(h)
        , m_alloc(alloc)
    {}

    template <class U>
    stack_allocator(const stack_allocator<U, N, TAlloc, Align>& other)
        : m_heap(other.m_heap)
        , m_alloc(other.m_alloc)
    {}

    stack_allocator& operator=(const stack_allocator& other) = delete;

    pointer address(reference x) const
    {
        if (__is_in_buffer(std::addressof(x))) {
            return std::addressof(x);
        }
        return m_alloc.address(x);
    }

    const_pointer address(const_reference x) const
    {
        if (__is_in_buffer(std::addressof(x))) {
            return std::addressof(x);
        }
        return m_alloc.address(x);
    }

    pointer allocate(size_type n, const void* hint = NULL)
    {
        if (__has_available_mem<alignof(value_type)>(n)) {
            return __allocate<alignof(value_type)>(n * sizeof(value_type));
        }
        return m_alloc.allocate(n, hint);
    }

    template<class U, class... Args>
    void construct(U* p, Args&&... args) { m_alloc.construct(p, std::forward<Args>(args)...); }

    void deallocate(pointer p, size_type n)
    {
        if (__is_in_buffer(reinterpret_cast<__buf_ptr_t>(p))) {
            __deallocate(reinterpret_cast<__buf_ptr_t>(p), n * sizeof(value_type));
        } else {
            m_alloc.deallocate(p, n);
        }
    }

    template <class U>
    void destroy(U* p) { m_alloc.destroy(p); }

    size_type max_size() const { return m_alloc.max_size(); }

    template <size_t ReqAlign>
    pointer __allocate(size_type n)
    {
        static_assert(ReqAlign <= Align, "invalid required alignment");
        assert(__is_in_buffer(m_heap.m_p_ptr) && "pointer out of buffer");

        n = __round_up(n);
        __buf_ptr_t p_ret = m_heap.m_p_ptr;
        m_heap.m_p_ptr += n;
        return reinterpret_cast<pointer>(p_ret);
    }

    static constexpr size_t __buf_size() { return N /* sizeof(value_type)*/; }

    void __deallocate(__buf_ptr_t p, size_type n)
    {
        assert(__is_in_buffer(m_heap.m_p_ptr) && "pointer out of buffer");

        n = __round_up(n);
        if (p + n == m_heap.m_p_ptr) {
            m_heap.m_p_ptr = p;
        }
    }

    template <size_t ReqAlign>
    bool __has_available_mem(size_type n)
    {
        static_assert(ReqAlign <= Align, "invalid required alignment");
        assert(__is_in_buffer(m_heap.m_p_ptr) && "pointer out of buffer");

        n = __round_up(n * sizeof(value_type));
        return (static_cast<size_type>(m_heap.m_p_buffer + __buf_size() - m_heap.m_p_ptr) >= n);
    }

    bool __is_in_buffer(__buf_ptr_t p)
    {
        return std::uintptr_t(m_heap.m_p_buffer) <= std::uintptr_t(p) &&
               std::uintptr_t(p) <= std::uintptr_t(m_heap.m_p_buffer + N);
    }

    static size_t __round_up(size_t n) { return (n + (Align - 1)) & ~(Align - 1); }

    heap_type& m_heap;
    __alloc_t m_alloc;
};

/* Free functions */

template<class T1, std::size_t N, template<typename TTp> class TAlloc, size_t Align, class T2>
bool operator==(const stack_allocator<T1, N, TAlloc, Align>& lhs, 
                const stack_allocator<T2, N, TAlloc, Align>& rhs)
{
    return lhs.m_heap == rhs.m_heap;
}

template<class T1, std::size_t N, template<typename TTp> class TAlloc, size_t Align, class T2>
bool operator!=(const stack_allocator<T1, N, TAlloc, Align>& lhs, 
                const stack_allocator<T2, N, TAlloc, Align>& rhs)
{
    return lhs.m_heap != rhs.m_heap;
}

} // namespace alloc
} // namespace cnt

#endif /* _CONTAINERS_STACK_ALLOCATOR_H */
