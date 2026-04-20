# Unit tests
ADD_EXECUTABLE(unit_tests
    sampler/test_sampler.cpp
    vecmath/test_Bounds3.cpp
    vecmath/test_Point2.cpp
    vecmath/test_Point3.cpp
    vecmath/test_Vector2.cpp
    vecmath/test_Vector3.cpp
    test_main.cpp
)
TARGET_LINK_LIBRARIES(unit_tests PRIVATE
    ${PROJECT_NAME}_lib
    GTest::gmock_main
    GTest::gtest_main
    GTest::gtest
)
TARGET_INCLUDE_DIRECTORIES(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/external
)

INCLUDE(GoogleTest)
gtest_discover_tests(unit_tests
    PROPERTIES
    LABELS "unit"
)

# # Integration tests
# ADD_EXECUTABLE(integration_tests
#     test_main.cpp
# )
# TARGET_LINK_LIBRARIES(integration_tests PRIVATE
#     ${PROJECT_NAME}_lib
#     GTest::gmock_main
#     GTest::gtest_main
#     GTest::gtest
# )
# TARGET_INCLUDE_DIRECTORIES(integration_tests PRIVATE
#     ${CMAKE_SOURCE_DIR}/src
#     ${CMAKE_BINARY_DIR}
#     ${CMAKE_SOURCE_DIR}/external/include
# )
# gtest_discover_tests(integration_tests
#     PROPERTIES
#     LABELS "integration"
# )