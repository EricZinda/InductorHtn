target_sources(runtests
    PRIVATE
	   ${CMAKE_CURRENT_SOURCE_DIR}/ParserTestBase.h
)
target_include_directories(testLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)
