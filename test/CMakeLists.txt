set(M_TESTS_LIST
    unit-basic_server.cpp
    )

add_library(doctest_main OBJECT unit.cpp)
target_include_directories(doctest_main PRIVATE third_party/doctest)


foreach(filename ${M_TESTS_LIST})
    string(REGEX REPLACE "unit-([^_]+).cpp" "test-\\1" testcase ${filename})

    add_executable(${testcase} $<TARGET_OBJECTS:doctest_main> ${filename})

    add_test(
        NAME "${testcase}"
        COMMAND ${testcase} ${DOCTEST_TEST_FILTER} --no-skip)

    target_link_libraries(${testcase} modern::rpc)
endforeach()