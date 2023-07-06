message(STATUS "Adding ImFileDialog Library...")
set(NAME imFileDialog)
find_package(Stb REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_library(${NAME} ImFileDialog.cpp)
target_include_directories(${NAME} PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(${NAME} PRIVATE imgui::imgui)
