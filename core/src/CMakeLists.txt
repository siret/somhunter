
#pthreads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# cURL
find_package(CURL REQUIRED) 
include_directories(${CURL_INCLUDE_DIR})

SET(HEADERS
	AsyncSom.h
  	common.h
	config_json.h
	config.h
	distfs.h
	SOM.h
	DatasetFeatures.h
	DatasetFrames.h
	KeywordRanker.h
  	log.h
	RelevanceScores.h
  	SomHunter.h
	Submitter.h
	use_intrins.h
	utils.h
	log.h
	json11.hpp
)

SET(SOURCES
	${HEADERS}
	AsyncSom.cpp
	SOM.cpp
	DatasetFeatures.cpp
	DatasetFrames.cpp
	KeywordRanker.cpp
	RelevanceScores.cpp
  	SomHunter.cpp
	Submitter.cpp
	main.cpp
	json11.cpp
)

#create the program
add_executable(somhunter
	${TextExample_RESOURCES}
	${SOURCES}
	${HEADERS}
	)

set_target_properties(somhunter PROPERTIES CXX_STANDARD 17)

target_link_libraries(somhunter PRIVATE
	${CURL_LIBRARIES}
    Threads::Threads
    )
