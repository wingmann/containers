project(algorithms_test)

file(GLOB PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp")

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest_main)

enable_testing()
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
