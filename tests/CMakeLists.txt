# Unit tests

TestTarget(ut_stack_allocator
    SOURCES
        ut_stack_allocator.cpp
    LIBRARIES
        stack_allocator
    DEPENDS
        testing
)

# Performance tests

TestTarget(perf_stack_allocator
    SOURCES
        perf_stack_allocator.cpp
    LIBRARIES
        stack_allocator
    DEPENDS
        testing
)

