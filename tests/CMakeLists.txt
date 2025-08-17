include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(vk_storage_tests
    tests_vk_storage.cpp
)

target_link_libraries(vk_storage_tests
    PRIVATE
    vk_storage
    gtest_main
    gtest
)

include(GoogleTest)
gtest_discover_tests(vk_storage_tests)