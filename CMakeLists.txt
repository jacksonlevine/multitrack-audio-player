cmake_minimum_required(VERSION 3.10)

# set the project name
project(MyProject)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Crow CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)

# add the executable
add_executable(main main.cpp jackspvars/pvarslib.cpp)

# link the libraries to the executable
target_link_libraries(main PRIVATE Crow::Crow)
target_link_libraries(main PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(main PRIVATE unofficial-sodium::sodium)