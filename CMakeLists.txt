cmake_minimum_required(VERSION 3.21)

project(PSXGame
  LANGUAGES    C ASM
  VERSION      0.1.0
  DESCRIPTION  "PSX Game"
  HOMEPAGE_URL "https://luksamuk.codes")

file(GLOB PSXGAME_SRC
  ${CMAKE_CURRENT_LIST_DIR}/src/*.c
  ${CMAKE_CURRENT_LIST_DIR}/src/screen/*.c
  ${CMAKE_CURRENT_LIST_DIR}/src/*.s)

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_LIST_DIR}/cmake/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
git_get_exact_tag(GIT_TAG)

string(SUBSTRING "${GIT_SHA1}"     0  7 GIT_SHORTHASH)
string(SUBSTRING "${GIT_REFSPEC}" 11 -1 GIT_BRANCH)

add_definitions("-DGIT_SHA1=\"${GIT_SHA1}\"")
add_definitions("-DGIT_REFSPEC=\"${GIT_REFSPEC}\"")
add_definitions("-DGIT_COMMIT=\"${GIT_BRANCH}/${GIT_SHORTHASH}\"")

if(GIT_TAG)
  add_definitions("-DGIT_VERSION=\"${GIT_TAG}\"")
else()
  add_definitions("-DGIT_VERSION=\"DEV BUILD ${GIT_SHORTHASH}\"")
endif()

set(CMAKE_C_FLAGS "-Wall")

psn00bsdk_add_executable(psxgame
  GPREL
  ${PSXGAME_SRC})

target_include_directories(psxgame PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>)

psn00bsdk_add_cd_image(iso "PSXGAME"
  iso.xml
  DEPENDS psxgame system.cnf)

