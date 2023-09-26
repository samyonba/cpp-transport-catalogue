cmake_minimum_required(VERSION 3.10)

project(TransportCataogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto transport_router.proto graph.proto)

set(TC_FILES
transport_catalogue.proto
transport_router.proto
graph.proto
)

set(TC_CXX_FILES
main.cpp
domain.cpp
json_builder.cpp
json_reader.cpp
json.cpp
map_renderer.cpp
request_handler.cpp
serialization.cpp
svg.cpp
transport_catalogue.cpp
transport_router.cpp
)

set(TC_H_FILES
domain.h
geo.h
graph.h
json_builder.h
json_reader.h
json.h
map_renderer.h
ranges.h
request_handler.h
router.h
serialization.h
svg.h
transport_catalogue.h
transport_router.h
)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TC_FILES} ${TC_CXX_FILES} ${TC_H_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(transport_catalogue ${Protobuf_LIBRARY_DEBUG} Threads::Threads)
