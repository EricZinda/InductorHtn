target_sources(indhtnpy
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface.cpp
)
target_include_directories(indhtnpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)
