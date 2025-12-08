cmake_minimum_required(VERSION 3.15)

message(STATUS "==== ENVIRONMENT VARIABLES ====")
execute_process(COMMAND ${CMAKE_COMMAND} -E environment)
message(STATUS "===============================")


option(RALLYPLOT_BUILD_DEV "Enable dev build (tests, python dist, vendored Qt)" OFF)

# --------------------------------------------------------------------------------------
# Build PlottingLib Library
# --------------------------------------------------------------------------------------

project(rallyplot LANGUAGES CXX)

# If we're building the Python dist, force use of vendored Qt
if(RALLYPLOT_BUILD_DEV)
  if(WIN32)
    set(QT_INSTALL_DIR "${PROJECT_SOURCE_DIR}/distribution/qt/6.8.2/msvc2022_64")
  elseif(APPLE)
    set(QT_INSTALL_DIR "${PROJECT_SOURCE_DIR}/distribution/qt/6.8.2/macos")
  else()
    set(QT_INSTALL_DIR "${PROJECT_SOURCE_DIR}/distribution/qt/6.8.2/gcc_64")
  endif()

  list(PREPEND CMAKE_PREFIX_PATH "${QT_INSTALL_DIR}")

endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# We do not need these, and they are annoying dependencies
set(FT_DISABLE_BZIP2 TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_ZLIB TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI TRUE CACHE BOOL "" FORCE)

add_subdirectory(
  ${PROJECT_SOURCE_DIR}/src/vendor/freetype-2.13.3
  EXCLUDE_FROM_ALL
)

set(FT_DISABLE_BZIP2 TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_ZLIB TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ TRUE CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI TRUE CACHE BOOL "" FORCE)

if (UNIX AND NOT APPLE)
    # Linux only
    set_target_properties(freetype PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()


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

if(RALLYPLOT_BUILD_DEV)

  # ---------------------------------------------------------------
  # Test executable for development
  # ---------------------------------------------------------------

  add_executable(testLib tests/cpp/main.cpp)

  target_include_directories(testLib PRIVATE
      "${PROJECT_SOURCE_DIR}/src/cpp/include"
  )

  target_link_libraries(testLib PRIVATE
      rallyplot
  )

# ---------------------------------------------------------------
# Python Distribution
# ---------------------------------------------------------------

  # Set the RPATH for the plotting library on mac and linux.
  # The convention on mac is for all Qt frameworks to be in the root
  # folder. Because I could not get "deploy-wheel" to work, and the RPATHS
  # of dependencies from aqtinstall are strange, we manually
  # re-write the RPATHs of all dependenices in `fix_macos_paths.sh`.

  # For UNIX, we use the RPATHS The RPATHs are manually rewritten with
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
      BUILD_RPATH "\$ORIGIN/lib"
      INSTALL_RPATH "\$ORIGIN/lib"
    )
  endif()

  # Build Python Bindings
  # ---------------------------------------------------------------

  # Automatically detect Python from the currently active environment
  find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)

  add_subdirectory(src/vendor/pybind11)

  pybind11_add_module(pythonBindings src/cpp/bindings/bindings.cpp)

  if(UNIX)
    if(APPLE)
      set_target_properties(pythonBindings PROPERTIES
        INSTALL_RPATH "@loader_path"
        BUILD_WITH_INSTALL_RPATH ON
        MACOSX_RPATH ON
      )
    else()
      set_target_properties(pythonBindings PROPERTIES
        BUILD_RPATH "\$ORIGIN;\$ORIGIN/lib"
        INSTALL_RPATH "\$ORIGIN;\$ORIGIN/lib"
      )
    endif()
  endif()

  target_link_libraries(pythonBindings PRIVATE rallyplot)

  target_include_directories(pythonBindings PRIVATE ${PROJECT_SOURCE_DIR}/src/cpp/include)



  # Distribution
  # ---------------------------------------------------------------
  # Copy all files and dependencies for the full distribution.
  # We cannot use windeployqt because we dont ship an executable.
  # A downside is that the development env is not exactly the distribution env.

  # Distribution target that always runs, but depends on rallyplot
  # ---------------------------------------------------------------
  # We copy all dependencies to the dist folder (for distribution) and
  # build folder (for easy testing). The exact stratergy for locating
  # and copying the dependencies depends on the system (see more below).

  # Qt should be installed in the root folder during building, using
  # aqtinstall. See `download_qt.py` in the root folder for details.

  add_custom_target(dist ALL
    DEPENDS rallyplot pythonBindings
  )

  set(DIST_TARGETS_CPP_INCLUDE "${PROJECT_SOURCE_DIR}/dist/cpp/include")
  set(DIST_TARGETS_PY  "${PROJECT_SOURCE_DIR}/dist/rallyplot")
  set(DIST_TARGETS     ${DIST_TARGETS_PY} $<TARGET_FILE_DIR:testLib>)

  file(MAKE_DIRECTORY ${DIST_TARGETS_PY})
  file(MAKE_DIRECTORY "${DIST_TARGETS_PY}/plugins/platforms")


  if (WIN32)
    # In Windows, we use windeployqt because it works requiring
    # only a .exe stub, and copied some directories I was suprised
    # to see added. It is also useful to have this stub for testing
    # deployment on Windows and macOS. We use it for plugins and libaries
    # for the CCP build. However, for python we rely on delvewheel for
    # the libraries, because it finds all dependencies andperforms name manging,
    # which is very useful on Windows.

    # Build a tiny Qt exe just for windeployqt scanning
    add_dependencies(dist stubFileForDeploy)

    set(WINDEPLOYQT_EXE "${QT_INSTALL_DIR}/bin/windeployqt.exe")

    add_executable(stubFileForDeploy distribution/stubFileForDeploy.cpp)

    set_target_properties(stubFileForDeploy PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "StubFile"
    )

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

    # Copy library dependencies to CPP distribution
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
    # For linux, we use a simmilar approach as for Windows (except without a deployqt exe).
    # Copy all libarries and plugins to the ccp distribution / build, however let repairwheel
    # find the dependencies for the python builds because it performs some nice additional functionality.

    set(QT_LIB_DIR     "${QT_INSTALL_DIR}/lib")

    set(QT_PLUGINS_DIR "${QT_INSTALL_DIR}/plugins")
    file(MAKE_DIRECTORY "${DIST_TARGETS_PY}/lib")


    # First copy the libs. Only copy the lib dependencies
    # to cpp, and let repairwheel handle the python depdendencies

    file(GLOB _QT_LIBS
      "${QT_LIB_DIR}/libQt${QT_VERSION_MAJOR}Core.so*"
      "${QT_LIB_DIR}/libQt${QT_VERSION_MAJOR}Gui.so*"
      "${QT_LIB_DIR}/libQt${QT_VERSION_MAJOR}Widgets.so*"
      "${QT_LIB_DIR}/libQt${QT_VERSION_MAJOR}OpenGL.so*"
      "${QT_LIB_DIR}/libQt${QT_VERSION_MAJOR}OpenGLWidgets.so*"
      "${QT_LIB_DIR}/libQt${QT_VERSION_MAJOR}DBus.so*"
      "${QT_LIB_DIR}/libicui18n.so*"
      "${QT_LIB_DIR}/libicuuc.so*"
      "${QT_LIB_DIR}/libicudata.so*"

      # Wayland
      "${QT_LIB_DIR}/libQt${QT_VERSION_MAJOR}WaylandClient.so*"
      "${QT_LIB_DIR}/libQt${QT_VERSION_MAJOR}WaylandEglClientHwIntegration.so*"

      # X11
      "${QT_LIB_DIR}/libQt${QT_VERSION_MAJOR}XcbQpa.so*"

    )
    add_custom_command(TARGET dist
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${_QT_LIBS} "$<TARGET_FILE_DIR:testLib>/"
      VERBATIM
    )

    # Copy the plugins to all build / distributions
    # List Wayland-related integration folders to include fully
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
