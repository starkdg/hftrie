cmake_minimum_required(VERSION 3.13)
project(hftrie VERSION 0.1.0.0
			   DESCRIPTION "hamming space indexing data structure for nearest neighbor search")

set(CMAKE_BUILD_TYPE Release)

set(HFTRIE_SRCS src/hft.cpp src/hfnode.cpp src/hftrie.cpp)

#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pg)
#set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -pg)

add_library(hftrie STATIC ${HFTRIE_SRCS})
target_compile_options(hftrie PUBLIC -g -Ofast -Wall)
target_include_directories(hftrie PUBLIC include)

add_executable(testhft tests/test_hft.cpp)
target_compile_options(testhft PUBLIC -g -Wall)
target_link_libraries(testhft hftrie)

add_executable(testhftrie tests/test_hftrie.cpp)
target_compile_options(testhftrie PUBLIC -Wall)
target_link_libraries(testhftrie hftrie)

add_executable(runhftrie tests/run_hftrie.cpp)
target_compile_options(runhftrie PUBLIC -Ofast -Wall)
target_link_libraries(runhftrie hftrie)

add_executable(seqsearch tests/seqsearch.cpp)
target_compile_options(seqsearch PUBLIC -g -Ofast -Wall)

include(CTest)
add_test(NAME test1 COMMAND testhft)
add_test(NAME test2 COMMAND testhftrie)

install(TARGETS hftrie ARCHIVE DESTINATION lib PUBLIC_HEADER DESTINATION include)
