if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
	set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -m32")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
	set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -m64")
endif()

file(GLOB_RECURSE SIMD_BASE_SRC ${SIMD_ROOT}/src/Simd/SimdBase*.cpp)
set_source_files_properties(${SIMD_BASE_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS}")

file(GLOB_RECURSE SIMD_SSE41_SRC ${SIMD_ROOT}/src/Simd/SimdSse41*.cpp)
set_source_files_properties(${SIMD_SSE41_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2")

file(GLOB_RECURSE SIMD_AVX1_SRC ${SIMD_ROOT}/src/Simd/SimdAvx1*.cpp)
if ((CMAKE_CXX_COMPILER MATCHES "clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	set_source_files_properties(${SIMD_AVX1_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} -mavx")
else()
	set_source_files_properties(${SIMD_AVX1_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} -mavx -mno-avx256-split-unaligned-load -mno-avx256-split-unaligned-store")
endif()

file(GLOB_RECURSE SIMD_AVX2_SRC ${SIMD_ROOT}/src/Simd/SimdAvx2*.cpp)
if ((CMAKE_CXX_COMPILER MATCHES "clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	set_source_files_properties(${SIMD_AVX2_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} -mavx2 -mfma -mf16c -mbmi -mbmi2 -mlzcnt")
else()
	set_source_files_properties(${SIMD_AVX2_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} -mavx2 -mfma -mf16c -mbmi -mbmi2 -mlzcnt -fabi-version=4 -mno-avx256-split-unaligned-load -mno-avx256-split-unaligned-store")
endif()

set(SIMD_LIB_FLAGS "${COMMON_CXX_FLAGS} -mavx2 -mfma")
set(SIMD_ALG_SRC ${SIMD_BASE_SRC} ${SIMD_SSE41_SRC} ${SIMD_AVX1_SRC} ${SIMD_AVX2_SRC})

if((((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER MATCHES "gnu")) AND (NOT(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.5.0"))) OR (CMAKE_CXX_COMPILER MATCHES "clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))    
	file(GLOB_RECURSE SIMD_AVX512BW_SRC ${SIMD_ROOT}/src/Simd/SimdAvx512bw*.cpp)
	set_source_files_properties(${SIMD_AVX512BW_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} -mavx512f -mavx512cd -mavx512bw -mavx512vl -mavx512dq -mbmi -mbmi2 -mlzcnt -mfma")

	if(UNIX AND SIMD_AVX512)
		set(SIMD_LIB_FLAGS "${SIMD_LIB_FLAGS} -mavx512bw")
		set(SIMD_ALG_SRC ${SIMD_ALG_SRC} ${SIMD_AVX512F_SRC} ${SIMD_AVX512BW_SRC})
		if(SIMD_INFO)
			message("Use AVX-512BW")
		endif()
	endif()
endif()

if(((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER MATCHES "gnu")) AND (NOT(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0.0")))    
	file(GLOB_RECURSE SIMD_AVX512VNNI_SRC ${SIMD_ROOT}/src/Simd/SimdAvx512vnni*.cpp)
	set_source_files_properties(${SIMD_AVX512VNNI_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} -mavx512f -mavx512cd -mavx512bw -mavx512vl -mavx512dq -mavx512vnni")

	if(UNIX AND SIMD_AVX512VNNI)
		set(SIMD_LIB_FLAGS "${SIMD_LIB_FLAGS} -mavx512vnni")
		set(SIMD_ALG_SRC ${SIMD_ALG_SRC} ${SIMD_AVX512VNNI_SRC})
		if(SIMD_INFO)
			message("Use AVX-512VNNI")
		endif()
	endif()
endif()

if(((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER MATCHES "gnu")) AND (NOT(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0.0")))    
	file(GLOB_RECURSE SIMD_AVX512BF16_SRC ${SIMD_ROOT}/src/Simd/SimdAvx512bf16*.cpp)
	set_source_files_properties(${SIMD_AVX512BF16_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} -mavx512f -mavx512cd -mavx512bw -mavx512vl -mavx512dq -mavx512vnni -mavx512bf16")

	if(UNIX AND SIMD_AVX512BF16 AND (NOT(BINUTILS_VERSION VERSION_LESS "2.34")))
		set(SIMD_LIB_FLAGS "${SIMD_LIB_FLAGS} -mavx512bf16")
		set(SIMD_ALG_SRC ${SIMD_ALG_SRC} ${SIMD_AVX512BF16_SRC})
		if(SIMD_INFO)
			message("Use AVX-512BF16")
		endif()
	endif()
endif()

if(((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER MATCHES "gnu")) AND (NOT(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0.0")))    
	file(GLOB_RECURSE SIMD_AMX_SRC ${SIMD_ROOT}/src/Simd/SimdAmx*.cpp)
	set_source_files_properties(${SIMD_AMX_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} -mavx512f -mavx512cd -mavx512bw -mavx512vl -mavx512dq -mavx512vnni -mavx512bf16 -mamx-tile -mamx-int8 -mamx-bf16")

	if(UNIX AND ((SIMD_AMX AND (NOT(BINUTILS_VERSION VERSION_LESS "2.34"))) OR SIMD_AMX_EMULATE))
		set(SIMD_LIB_FLAGS "${SIMD_LIB_FLAGS} -mamx-tile -mamx-int8 -mamx-bf16")
		set(SIMD_ALG_SRC ${SIMD_ALG_SRC} ${SIMD_AMX_SRC})
		if(SIMD_INFO)
			message("Use AMX")
		endif()
	endif()
endif()

file(GLOB_RECURSE SIMD_LIB_SRC ${SIMD_ROOT}/src/Simd/SimdLib.cpp)
set_source_files_properties(${SIMD_LIB_SRC} PROPERTIES COMPILE_FLAGS "${SIMD_LIB_FLAGS}")
add_library(Simd ${SIMD_LIB_TYPE} ${SIMD_LIB_SRC} ${SIMD_ALG_SRC})

if(SIMD_TEST)
	file(GLOB_RECURSE TEST_SRC_C ${SIMD_ROOT}/src/Test/*.c)
	if((CMAKE_CXX_COMPILER MATCHES "clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang")) 
		set_source_files_properties(${TEST_SRC_C} PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -x c")
	endif()
	file(GLOB_RECURSE TEST_SRC_CPP ${SIMD_ROOT}/src/Test/*.cpp)
	if((NOT ${SIMD_TARGET} STREQUAL "") OR (NOT SIMD_AVX512))
		set_source_files_properties(${TEST_SRC_CPP} PROPERTIES COMPILE_FLAGS "${SIMD_LIB_FLAGS}")
	else()
		set_source_files_properties(${TEST_SRC_CPP} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} ${SIMD_TEST_FLAGS} -mtune=native")
	endif()
	add_executable(Test ${TEST_SRC_C} ${TEST_SRC_CPP})
	target_link_libraries(Test Simd -lpthread -lstdc++ -lm)
endif()