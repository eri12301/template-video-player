
set(NAME app)

find_package(glad CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_subdirectory(widgets)

add_executable(${NAME} ImguiRenderer.cpp ImguiRenderer.hpp app.cpp Sound.hpp Sound.cpp)

target_link_libraries(${NAME} PRIVATE glad::glad imgui::imgui widgets decoder-lib
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        )
