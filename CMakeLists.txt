project(cwebserver)

add_executable(cwebserver main.cpp)

install(TARGETS cwebserver RUNTIME DESTINATION bin)
