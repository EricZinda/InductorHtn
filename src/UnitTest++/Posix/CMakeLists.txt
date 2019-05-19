target_sources(testLib
    PRIVATE
		SignalTranslator.cpp
		SignalTranslator.h
		TimeHelpers.cpp
		TimeHelpers.h
)
target_include_directories(testLib PRIVATE ./)