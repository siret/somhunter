
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
	Features.h
	Frames.h
	ImageKeywordsW2VV.h
	json11.hpp
  	log.h
	Scores.h
  	SomHunter.h
	Submitter.h
	use_intrins.h
	utils.h
	log.h
)

SET(SOURCES
	${HEADERS}
	AsyncSom.cpp
	SOM.cpp
	Features.cpp
	Frames.cpp
	ImageKeywordsW2VV.cpp
	json11.cpp
	Scores.cpp
  	SomHunter.cpp
	Submitter.cpp
	main.cpp
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
