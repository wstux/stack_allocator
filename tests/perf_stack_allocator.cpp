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

#include <deque>
#include <list>
#include <vector>

#include <testing/perfdefs.h>
#include <testing/utils.h>

#include "stack_allocator/stack_allocator.h"

namespace {

constexpr size_t kBufSize = 2*1024*1024;

template<typename T>
using stack_alloc_t = cnt::alloc::stack_allocator<T, kBufSize>;

template<typename TType>
class alloc_fixture : public ::testing::Test
{
public:
    using cnt_t = TType;

    cnt_t create_cnt()
    {
        using alloc_type = typename TType::allocator_type;

        if constexpr (std::is_same<alloc_type, stack_alloc_t<int>>::value) {
            alloc_type alloc(m_heap);
            return cnt_t(alloc);
        } else {
            return cnt_t();
        }
    }

private:
    stack_alloc_t<int>::heap_type m_heap;
};

using types = testing::Types<std::deque<int>,
                             std::deque<int, stack_alloc_t<int>>,
                             std::list<int>, 
                             std::list<int, stack_alloc_t<int>>,
                             std::vector<int>,
                             std::vector<int, stack_alloc_t<int>>>;
TYPED_PERF_TEST_SUITE(alloc_fixture, types);

} // <anonymous> namespace

TYPED_PERF_TEST(alloc_fixture, add)
{
    using cnt_type = TypeParam;

    PERF_INIT_TIMER(resize);
    cnt_type cnt = this->create_cnt();

    PERF_START_TIMER(resize);
    for (int i = 0; i < 512*1024; ++i) {
        cnt.push_back(i);
    }
    PERF_PAUSE_TIMER(resize);
}

int main(int /*argc*/, char** /*argv*/)
{
    return RUN_ALL_PERF_TESTS();
}
