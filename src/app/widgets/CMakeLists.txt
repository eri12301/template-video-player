set(NAME widgets)

find_package(glad CONFIG REQUIRED)
add_library(${NAME} FileDialog.cpp)
target_link_libraries(${NAME} imFileDialog glad::glad)