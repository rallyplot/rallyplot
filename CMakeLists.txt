cmake_minimum_required(VERSION 3.15)

option(RALLYPLOT_BUILD_DEV "Enable dev build (tests, python dist, vendored Qt)" OFF)

# --------------------------------------------------------------------------------------
# Build PlottingLib Library
# --------------------------------------------------------------------------------------

project(rallyplot LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Qt6_DIR to force use of particular Qt installation
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)

add_library(rallyplot SHARED

  src/cpp/rallyplot_global.h
  src/cpp/structure/CentralOpenGlWidget.cpp
  src/cpp/structure/CentralOpenGlWidget.h
  src/cpp/charts/fonts/arial.ttf
  src/cpp/charts/fonts/arialbd.ttf
  src/cpp/charts/fonts/arialbi.ttf
  src/cpp/charts/fonts/ariali.ttf
  src/cpp/charts/fonts/ARIALN.TTF
  src/cpp/charts/fonts/ARIALNB.TTF
  src/cpp/charts/fonts/ARIALNBI.TTF
  src/cpp/charts/fonts/ARIALNI.TTF
  src/cpp/charts/fonts/ariblk.ttf
  src/cpp/charts/fonts/consola.ttf
  src/cpp/charts/fonts/consolab.ttf
  src/cpp/charts/fonts/consolai.ttf
  src/cpp/charts/fonts/consolaz.ttf
  src/cpp/charts/shaders/shader_code/axes_fragment.shader
  src/cpp/charts/shaders/shader_code/axes_tick_vertex.shader
  src/cpp/charts/shaders/shader_code/axes_vertex.shader
  src/cpp/charts/shaders/shader_code/candlestick_fragment.shader
  src/cpp/charts/shaders/shader_code/candlestick_vertex.shader
  src/cpp/charts/shaders/shader_code/font_fragment.shader
  src/cpp/charts/shaders/shader_code/font_vertex.shader
  src/cpp/charts/shaders/Program.cpp
  src/cpp/charts/shaders/Program.h
  src/cpp/charts/shaders/Shaders.cpp
  src/cpp/charts/shaders/Shaders.h
  src/cpp/charts/AxesObject.cpp
  src/cpp/charts/AxesObject.h
  src/cpp/charts/AxisTickLabels.cpp
  src/cpp/charts/AxisTickLabels.h
  src/cpp/charts/Camera.cpp src/cpp/charts/Camera.h
  src/cpp/Configs.cpp src/cpp/Configs.h
  src/cpp/structure/RenderManager.cpp
  src/cpp/structure/RenderManager.h
  src/cpp/Utils.h
  src/cpp/opengl/VertexArrayObject.cpp
  src/cpp/opengl/VertexArrayObject.h
  src/cpp/structure/WindowViewportObject.cpp
  src/cpp/structure/WindowViewportObject.h
  src/cpp/structure/JointPlotData.cpp
  src/cpp/structure/JointPlotData.h
  src/cpp/charts/plots/CandlestickData.cpp
  src/cpp/charts/plots/CandlestickData.h
  src/cpp/charts/plots/CandlestickPlot.cpp
  src/cpp/charts/plots/CandlestickPlot.h
  src/cpp/charts/plots/LineData.cpp
  src/cpp/charts/plots/LineData.h
  src/cpp/charts/plots/LinePlot.cpp
  src/cpp/charts/plots/LinePlot.h
  src/cpp/charts/plots/BasePlotData.h
  src/cpp/charts/shaders/shader_code/line_vertex.shader
  src/cpp/charts/plots/ScatterPlot.h
  src/cpp/charts/plots/ScatterPlot.cpp
  src/cpp/charts/plots/ScatterplotData.h
  src/cpp/charts/plots/ScatterplotData.cpp
  src/cpp/charts/shaders/shader_code/scatterplot_vertex.shader
  src/cpp/charts/shaders/shader_code/scatterplot_fragment.shader
  src/cpp/charts/plots/textures/circle.png
  src/cpp/structure/LinkedSubplot.h
  src/cpp/structure/LinkedSubplot.cpp
  src/cpp/charts/shaders/shader_code/line_geometry.shader
  src/cpp/charts/shaders/shader_code/line_fragment.shader
  src/cpp/charts/shaders/shader_code/candlestick_line_fragment.shader
  src/cpp/charts/plots/BarData.cpp
  src/cpp/charts/plots/BarData.h
  src/cpp/charts/plots/BarPlot.cpp
  src/cpp/charts/plots/BarPlot.h
  src/cpp/charts/shaders/shader_code/bar_fragment.shader
  src/cpp/charts/shaders/shader_code/bar_vertex.shader
  src/cpp/resources.qrc
  src/cpp/structure/SharedXData.h
  src/cpp/structure/SharedXData.cpp
  src/cpp/charts/plots/BasePlot.h
  src/cpp/Plotter.cpp
  src/cpp/structure/PlotWrapperWidget.h
  src/cpp/structure/PlotWrapperWidget.cpp
  src/cpp/structure/VerticalLabel.h
  src/cpp/charts/drawing/DrawLine.h
  src/cpp/charts/drawing/DrawLine.cpp
  src/cpp/charts/shaders/shader_code/simple_line_fragment.shader
  src/cpp/charts/shaders/shader_code/simple_line_geomtry.shader
  src/cpp/charts/shaders/shader_code/simple_line_vertex.shader
  src/cpp/charts/legend/Legend.cpp
  src/cpp/charts/legend/Legend.h
  src/cpp/charts/shaders/shader_code/legend_fragment.shader
  src/cpp/charts/shaders/shader_code/legend_vertex.shader
  src/cpp/charts/CharTextureAtlas.h
  src/cpp/charts/CharTextureAtlas.cpp
  src/cpp/charts/shaders/shader_code/legend_text_vertex.shader
  src/cpp/include/Plotter.h
  src/cpp/include/UserVector.h
  src/cpp/include/ToyData.h
)

# Set the RPATH for rallyplot on macOS and linux.
# The convention on macOS is for all Qt frameworks to be in the root
# folder. Because I could not get "deploy-wheel" to work, and the RPATHS
# of dependencies from aqtinstall are strange, we manually
# re-write the RPATHs of all dependenices in `fix_macos_paths.sh`.

# For Linux, the libs are placed in a lib folder so the RPATH
# in the binaries downloaded from aqtinstall do not have to be
# changed (they point to a lib folder).

if(APPLE)
  set_target_properties(rallyplot PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH ON
    MACOSX_RPATH ON
  )
elseif(UNIX)
  set_target_properties(rallyplot PROPERTIES
    BUILD_RPATH "\$ORIGIN"
    INSTALL_RPATH "\$ORIGIN"
  )
endif()

# Build freetype
# --------------------------------------------------------------

# We do not require these freetype dependencies
# to build freetype for our needs.
set(FT_DISABLE_BZIP2 TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_ZLIB TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI TRUE CACHE BOOL "" FORCE)

add_subdirectory(
  ${PROJECT_SOURCE_DIR}/src/vendor/freetype-2.13.3
  EXCLUDE_FROM_ALL
)

if (UNIX AND NOT APPLE)
    # Position-independent code for building freetype as shared library on Linux
    set_target_properties(freetype PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Build rallyplot
# --------------------------------------------------------------

target_link_libraries(rallyplot PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    freetype
)

target_include_directories(rallyplot
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/vendor/glm
        ${PROJECT_SOURCE_DIR}/src/vendor/fast-cpp-csv-parser
        ${PROJECT_SOURCE_DIR}/src/vendor/stb_image/include
        ${PROJECT_SOURCE_DIR}/src/vendor/fmt/include
)

# Rallyplot tests and python build
# --------------------------------------------------------------

if(RALLYPLOT_BUILD_DEV)

  # Get the Qt base path from the Qt6_DIR environment variable.

  if(NOT Qt6_DIR)
    message(FATAL_ERROR
      "RALLYPLOT_BUILD_DEV=ON requires Qt6_DIR to be set.\n"
    )
  endif()

  get_filename_component(_qt6_dir_abs "${Qt6_DIR}" ABSOLUTE)
  get_filename_component(QT_INSTALL_DIR "${_qt6_dir_abs}/../../.." ABSOLUTE)

  message(STATUS "Qt6_DIR=${Qt6_DIR}")
  message(STATUS "QT_INSTALL_DIR=${QT_INSTALL_DIR}")

  # Test executable
  # ---------------------------------------------------------------

  add_executable(testLib tests/cpp/main.cpp)

  target_include_directories(testLib PRIVATE
      "${PROJECT_SOURCE_DIR}/src/cpp/include"
  )

  target_link_libraries(testLib PRIVATE
      rallyplot
  )

  # Python Distribution
  # ---------------------------------------------------------------

  # Build Python Bindings
  # ---------------------------------------------------------------

  # Automatically detect Python from the currently active environment
  find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)

  add_subdirectory(src/vendor/pybind11)

  pybind11_add_module(pythonBindings src/cpp/bindings/bindings.cpp)

  target_link_libraries(pythonBindings PRIVATE rallyplot)

  target_include_directories(pythonBindings PRIVATE ${PROJECT_SOURCE_DIR}/src/cpp/include)

  # Copy dependencies
  # ---------------------------------------------------------------
  # Dependencies (plugins and libraries) are copied to the test build for convenience.
  # Plugins are also copied to the python distribution (`dist/python`), because the wheel repair
  # packages (e.g. auditwheel) usually cannot find them. We use auditwheel (linux)
  # and delvewheel (windows) to handle the library dependencies for building python,
  # but do this manually here for macOS due to issues with delocate.

  add_custom_target(dist ALL
    DEPENDS rallyplot pythonBindings
  )

  set(DIST_TARGETS_PY  "${PROJECT_SOURCE_DIR}/dist/rallyplot")
  set(DIST_TARGETS     ${DIST_TARGETS_PY} $<TARGET_FILE_DIR:testLib>)

  file(MAKE_DIRECTORY ${DIST_TARGETS_PY})
  file(MAKE_DIRECTORY "${DIST_TARGETS_PY}/plugins/platforms")


  if (WIN32)
    # In Windows, we use windeployqt because it is easy to use,
    # onlying requiring an .exe stub. It copies some libraries
    # that are not obvious dependencies.

    # Build a tiny Qt exe just for windeployqt scanning
    # Stub executable defines the canonical Qt dependency set for rallyplot
    # This ensures consistent deployment regardless of test executable configuration
    add_dependencies(dist stubFileForDeploy)

    set(WINDEPLOYQT_EXE "${QT_INSTALL_DIR}/bin/windeployqt.exe")

    add_executable(stubFileForDeploy distribution/stubFileForDeploy.cpp)

    target_include_directories(stubFileForDeploy PRIVATE
        "${PROJECT_SOURCE_DIR}/src/cpp/include"
    )

    target_link_libraries(stubFileForDeploy PRIVATE
        rallyplot
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::OpenGL
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    )

    # Copy library dependencies to test build
    add_custom_command(TARGET stubFileForDeploy POST_BUILD
      COMMAND "${WINDEPLOYQT_EXE}"
              --dir "$<TARGET_FILE_DIR:testLib>"
              --plugindir "$<TARGET_FILE_DIR:testLib>/plugins"
              --no-quick-import --compiler-runtime --no-translations --verbose 2
              -openglwidgets -opengl -core -gui -widgets
              "$<TARGET_FILE:stubFileForDeploy>"
      VERBATIM
      )

    # For Python, just copy the plugins
    add_custom_command(TARGET stubFileForDeploy POST_BUILD
      COMMAND "${WINDEPLOYQT_EXE}"
              --dir "${DIST_TARGETS_PY}"
              --plugindir "${DIST_TARGETS_PY}/plugins"
              --no-quick-import --compiler-runtime --no-translations --verbose 2
              --no-libraries
              "$<TARGET_FILE:stubFileForDeploy>"
      VERBATIM
    )

  elseif (APPLE)
    # On macOS, macdeployqt requires a .bundle and doesn't really work for
    # the lib distribution. It is useful during development to run it on the stub
    # as above, and see what is copied. However, because the format is BUNDLE we can
    # just use this for illustration, but manually copy the dependenices. For the live
    # of me I could not get "delocate-wheel" to work. It did not copy the framework folder,
    # only the libs. If I vendored the framework, it could not find them on the CI. In the end,
    # it was easiest to manually vendor the dependencies (using macdeployqt as a guide) and
    # run the quick shell script

    set_target_properties(pythonBindings PROPERTIES
      INSTALL_RPATH "@loader_path"
      BUILD_WITH_INSTALL_RPATH ON
      MACOSX_RPATH ON
    )

    # First, copy the frameworks to all distribution folders
    set(QT_FRAMEWORKS
      QtCore
      QtGui
      QtWidgets
      QtOpenGL
      QtOpenGLWidgets
      QtDBus
    )

    foreach(dst ${DIST_TARGETS})
      foreach(framework ${QT_FRAMEWORKS})
        add_custom_command(TARGET dist POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory
                  "${QT_INSTALL_DIR}/lib/${framework}.framework"
                  "${dst}/${framework}.framework"
          VERBATIM
      )
      endforeach()
    endforeach()

    # Copy the plugins to all distribution folders
    set(QT_PLUGIN_DIRS
        platforms
        styles
        iconengines
        imageformats
    )

    set(QT_PLUGINS_DIR "${QT_INSTALL_DIR}/plugins")

    foreach(dst ${DIST_TARGETS})
        foreach(dir ${QT_PLUGIN_DIRS})
            add_custom_command(TARGET dist POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${dst}/plugins/${dir}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${QT_PLUGINS_DIR}/${dir}" "${dst}/plugins/${dir}"
                VERBATIM
            )
        endforeach()
    endforeach()

    # For macOS, because I could not get delvewheel to work, run
    # a script that sets the RPATH correctly for all plugin
    # and framework libs in the distrubable.
    set(FIX_SCRIPT "${PROJECT_SOURCE_DIR}/distribution/fix_macos_paths.sh")

    foreach(dst ${DIST_TARGETS})

      set(DEST_SCRIPT "${dst}/fix_macos_paths.sh")

      # Copy the script into the destination directory
      add_custom_command(
        TARGET dist
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FIX_SCRIPT}" "${DEST_SCRIPT}"
        COMMENT "Copying fix_macos_paths.sh to ${dst}"
      )

      # Make it executable and run it
      add_custom_command(
        TARGET dist
        POST_BUILD
        COMMAND chmod +x "${DEST_SCRIPT}"
        COMMAND "${DEST_SCRIPT}"
        COMMENT "Running fix_macos_paths.sh"
      )
    endforeach()

  elseif (UNIX)
    # For linux, we use a similar approach as for Windows (except without a deployqt exe).
    # Copy all libarries and plugins to the ccp distribution / build, however let repairwheel
    # find the dependencies for the python builds because it performs some nice additional functionality.

    set(QT_PLUGINS_DIR "${QT_INSTALL_DIR}/plugins")

    # Copy the plugins to both test build and python distribution
    # Include all Wayland-related and x11 plugins for completion
    set(QT_PLUGIN_DIRS
        platforms
        wayland-decoration-client
        wayland-shell-integration
        wayland-graphics-integration-client
        xcbglintegrations
        egldeviceintegrations
        generic
    )

    foreach(dst ${DIST_TARGETS})
        foreach(dir ${QT_PLUGIN_DIRS})
            add_custom_command(TARGET dist POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${dst}/plugins/${dir}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${QT_PLUGINS_DIR}/${dir}" "${dst}/plugins/${dir}"
                VERBATIM
            )
        endforeach()
    endforeach()

  endif()

  # Copy rallyplot lib
  foreach(dst ${DIST_TARGETS})
    add_custom_command(TARGET dist PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:rallyplot>"
        "${dst}/$<TARGET_FILE_NAME:rallyplot>"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_LINKER_FILE:rallyplot>"
        "${dst}/$<TARGET_LINKER_FILE_NAME:rallyplot>"
    )
  endforeach()


  # Copy pythonBindings lib
  add_custom_command(TARGET dist PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:pythonBindings>"
      "${DIST_TARGETS_PY}/$<TARGET_FILE_NAME:pythonBindings>"
  )

  # Copy Python modules
  add_custom_command(TARGET dist PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      "${PROJECT_SOURCE_DIR}/src/rallyplot/plotter.py"
      "${DIST_TARGETS_PY}"
  )

  add_custom_command(TARGET dist PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      "${PROJECT_SOURCE_DIR}/src/rallyplot/__init__.py"
      "${DIST_TARGETS_PY}"
  )

endif()
