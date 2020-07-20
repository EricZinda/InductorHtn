target_sources(testLib
    PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/SignalTranslator.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/SignalTranslator.h
		${CMAKE_CURRENT_SOURCE_DIR}/TimeHelpers.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/TimeHelpers.h
)
target_include_directories(testLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)