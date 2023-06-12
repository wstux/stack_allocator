#include <filesystem>

#include <testing/testdefs.h>

#include "stack_allocator/stack_allocator.h"

TEST(stack_allocator, alloc)
{
    using alloc_t = cnt::alloc::stack_allocator<int, 1024, std::allocator, sizeof(int)>;

    alloc_t::heap_type heap;
    alloc_t allocator(heap);
    int* ptr = allocator.allocate(3);

    EXPECT_TRUE(std::uintptr_t(ptr) == std::uintptr_t(heap.m_p_buffer));
    EXPECT_TRUE(heap.m_p_ptr == (heap.m_p_buffer + 3 * sizeof(int)))
        << "heap.m_p_ptr = " << std::uintptr_t(heap.m_p_ptr) << "; "
        << "heap.m_p_buffer + 3 = " << std::uintptr_t(heap.m_p_buffer + 3 * sizeof(int));
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 3 * sizeof(int))
        << "heap.m_p_ptr - heap.m_p_buffer == "
        << static_cast<size_t>((int*)heap.m_p_ptr - (int*)heap.m_p_buffer);
}

TEST(stack_allocator, multi_alloc)
{
    using alloc_t = cnt::alloc::stack_allocator<int, 1024, std::allocator, sizeof(int)>;

    alloc_t::heap_type heap;
    alloc_t allocator(heap);

    int* ptr_1 = allocator.allocate(3);
    EXPECT_TRUE(std::uintptr_t(ptr_1) == std::uintptr_t(heap.m_p_buffer));
    EXPECT_TRUE(heap.m_p_ptr == (heap.m_p_buffer + 3 * sizeof(int)));
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 3 * sizeof(int));

    int* ptr_2 = allocator.allocate(15);
    EXPECT_TRUE(std::uintptr_t(ptr_2) == std::uintptr_t(heap.m_p_buffer + 3 * sizeof(int)));
    EXPECT_TRUE(heap.m_p_ptr == (heap.m_p_buffer + 18 * sizeof(int)));
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 18 * sizeof(int));

    int* ptr_3 = allocator.allocate(7);
    EXPECT_TRUE(std::uintptr_t(ptr_3) == std::uintptr_t(heap.m_p_buffer + 18 * sizeof(int)));
    EXPECT_TRUE(heap.m_p_ptr == (heap.m_p_buffer + 25 * sizeof(int)));
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 25 * sizeof(int));
}

TEST(stack_allocator, dealloc)
{
    using alloc_t = cnt::alloc::stack_allocator<int, 1024, std::allocator, sizeof(int)>;

    alloc_t::heap_type heap;
    alloc_t allocator(heap);
    int* ptr = allocator.allocate(3);

    EXPECT_TRUE(std::uintptr_t(ptr) == std::uintptr_t(heap.m_p_buffer));
    EXPECT_TRUE(heap.m_p_ptr == (heap.m_p_buffer + 3 * sizeof(int)))
        << "heap.m_p_ptr = " << std::uintptr_t(heap.m_p_ptr) << "; "
        << "heap.m_p_buffer + 3 = " << std::uintptr_t(heap.m_p_buffer + 3 * sizeof(int));
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 3 * sizeof(int))
        << "heap.m_p_ptr - heap.m_p_buffer == "
        << static_cast<size_t>((int*)heap.m_p_ptr - (int*)heap.m_p_buffer);

    allocator.deallocate(ptr, 3);
    EXPECT_TRUE(heap.m_p_ptr == heap.m_p_buffer);
}

TEST(stack_allocator, multi_dealloc_1)
{
    using alloc_t = cnt::alloc::stack_allocator<int, 1024, std::allocator, sizeof(int)>;

    alloc_t::heap_type heap;
    alloc_t allocator(heap);

    int* ptr_1 = allocator.allocate(3);
    int* ptr_2 = allocator.allocate(15);
    int* ptr_3 = allocator.allocate(7);
    EXPECT_TRUE(std::uintptr_t(ptr_3) == std::uintptr_t(heap.m_p_buffer + 18 * sizeof(int)));
    EXPECT_TRUE(heap.m_p_ptr == (heap.m_p_buffer + 25 * sizeof(int)));
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 25 * sizeof(int));

    allocator.deallocate(ptr_3, 7);
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 18 * sizeof(int));
    allocator.deallocate(ptr_2, 15);
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 3 * sizeof(int));
    allocator.deallocate(ptr_1, 3);
    EXPECT_TRUE(heap.m_p_ptr == heap.m_p_buffer);
}

TEST(stack_allocator, multi_dealloc_2)
{
    using alloc_t = cnt::alloc::stack_allocator<int, 1024, std::allocator, sizeof(int)>;

    alloc_t::heap_type heap;
    alloc_t allocator(heap);

    int* ptr_1 = allocator.allocate(3);
    int* ptr_2 = allocator.allocate(15);
    int* ptr_3 = allocator.allocate(7);
    EXPECT_TRUE(std::uintptr_t(ptr_3) == std::uintptr_t(heap.m_p_buffer + 18 * sizeof(int)));
    EXPECT_TRUE(heap.m_p_ptr == (heap.m_p_buffer + 25 * sizeof(int)));
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 25 * sizeof(int));

    allocator.deallocate(ptr_2, 15);
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 25 * sizeof(int));
    allocator.deallocate(ptr_3, 7);
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 18 * sizeof(int));
    allocator.deallocate(ptr_1, 3);
    EXPECT_TRUE(static_cast<size_t>(heap.m_p_ptr - heap.m_p_buffer) == 18 * sizeof(int));
}

TEST(stack_allocator, overflow)
{
    using alloc_t = cnt::alloc::stack_allocator<int, 16, std::allocator, sizeof(int)>;

    alloc_t::heap_type heap;
    alloc_t allocator(heap);
    int* ptr = allocator.allocate(32);

    EXPECT_TRUE(std::uintptr_t(ptr) != std::uintptr_t(heap.m_p_buffer));
    EXPECT_TRUE(! allocator.__is_in_buffer(alloc_t::__buf_ptr_t(ptr)));

    allocator.deallocate(ptr, 32);
}

int main(int /*argc*/, char** /*argv*/)
{
    return RUN_ALL_TESTS();
}
