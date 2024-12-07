project('Jubulant-Lamp', 'cpp', 'c', version : '0.1.0')

# Dependencies
glfw_dep = dependency('glfw3')
glew_dep = dependency('glew')
glu_dep = dependency('glu')
gl_dep = dependency('GL')
#cglm_dep = dependency('cglm', fallback : ['cglm', 'cglm_dep'])
glm_dep = dependency('glm', fallback: ['glm', 'glm_dep'])
add_global_arguments(['-Wall',
                      '-Wno-pointer-sign',
                      '-Wno-deprecated-declarations',
                      '-Wno-unused',
                      '-Wno-incompatible-pointer-types',
                      '-Wno-discarded-qualifiers'],
                     language: 'cpp')
cmake = import('cmake')
compiler = meson.get_compiler('cpp')
assimp_opt = cmake.subproject_options()
assimp_opt.add_cmake_defines({
    'BUILD_SHARED_LIBS': false,
    'CMAKE_BUILD_TYPE': get_option('buildtype') == 'debug' ? 'Debug' : 'Release',
    'ASSIMP_BUILD_ASSIMP_TOOLS': false,
    'ASSIMP_BUILD_TESTS': false,
    'ASSIMP_BUILD_ZLIB': false,

    'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT': false,
    'ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT': false,

    'ASSIMP_BUILD_GLB_IMPORTER': true,
    'ASSIMP_BUILD_OBJ_IMPORTER': true,
    'ASSIMP_BUILD_GLTF_IMPORTER': true,
})

assimp_dep = cmake.subproject('assimp', options: assimp_opt).dependency('assimp')

# Source files
srcs = [
  './src/main.cpp',
  #'./src/window.cpp',
  #'./src/components/cube.c',
  #'./src/shader.c',
  #'./src/gui.c',
  #'./src/components/cube.c'
  './include/imgui/imgui.cpp',
  './include/imgui/imgui_demo.cpp',
  './include/imgui/imgui_draw.cpp',
  './include/imgui/imgui_tables.cpp',
  './include/imgui/imgui_widgets.cpp',
  './include/imgui/imgui_impl_glfw.cpp',
  './include/imgui/imgui_impl_opengl3.cpp',
  #'./include/cimgui/cimgui.cpp'
]

# Include directories
include_dirs = [
  './src',
  './include',
  './include/imgui',
  #'./include/cimgui'
]

# Build executable
executable('Jubulant-Lamp', srcs,
  dependencies : [glfw_dep, glew_dep, glu_dep, gl_dep, assimp_dep, glm_dep],
  include_directories : include_dirs
)
