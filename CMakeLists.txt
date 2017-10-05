project(netapp)

file(
GLOB_RECURSE
source_files
src/*
)

add_executable(
netapp
${source_files}
)

if(WIN32)
target_link_libraries(
netapp
Ws2_32
)
endif()

#target_link_libraries(
#febonachi
#SDL2
#)
