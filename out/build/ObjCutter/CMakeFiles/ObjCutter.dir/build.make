# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = D:\msys64\mingw64\bin\cmake.exe

# The command to remove a file.
RM = D:\msys64\mingw64\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\12569\Desktop\ObjCutter\ObjCutter

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\12569\Desktop\ObjCutter\ObjCutter\out\build\ObjCutter

# Include any dependencies generated for this target.
include CMakeFiles/ObjCutter.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/ObjCutter.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/ObjCutter.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ObjCutter.dir/flags.make

CMakeFiles/ObjCutter.dir/main.cpp.obj: CMakeFiles/ObjCutter.dir/flags.make
CMakeFiles/ObjCutter.dir/main.cpp.obj: C:/Users/12569/Desktop/ObjCutter/ObjCutter/main.cpp
CMakeFiles/ObjCutter.dir/main.cpp.obj: CMakeFiles/ObjCutter.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\12569\Desktop\ObjCutter\ObjCutter\out\build\ObjCutter\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ObjCutter.dir/main.cpp.obj"
	D:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/ObjCutter.dir/main.cpp.obj -MF CMakeFiles\ObjCutter.dir\main.cpp.obj.d -o CMakeFiles\ObjCutter.dir\main.cpp.obj -c C:\Users\12569\Desktop\ObjCutter\ObjCutter\main.cpp

CMakeFiles/ObjCutter.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/ObjCutter.dir/main.cpp.i"
	D:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\12569\Desktop\ObjCutter\ObjCutter\main.cpp > CMakeFiles\ObjCutter.dir\main.cpp.i

CMakeFiles/ObjCutter.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/ObjCutter.dir/main.cpp.s"
	D:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\12569\Desktop\ObjCutter\ObjCutter\main.cpp -o CMakeFiles\ObjCutter.dir\main.cpp.s

CMakeFiles/ObjCutter.dir/Types.cpp.obj: CMakeFiles/ObjCutter.dir/flags.make
CMakeFiles/ObjCutter.dir/Types.cpp.obj: C:/Users/12569/Desktop/ObjCutter/ObjCutter/Types.cpp
CMakeFiles/ObjCutter.dir/Types.cpp.obj: CMakeFiles/ObjCutter.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\12569\Desktop\ObjCutter\ObjCutter\out\build\ObjCutter\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/ObjCutter.dir/Types.cpp.obj"
	D:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/ObjCutter.dir/Types.cpp.obj -MF CMakeFiles\ObjCutter.dir\Types.cpp.obj.d -o CMakeFiles\ObjCutter.dir\Types.cpp.obj -c C:\Users\12569\Desktop\ObjCutter\ObjCutter\Types.cpp

CMakeFiles/ObjCutter.dir/Types.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/ObjCutter.dir/Types.cpp.i"
	D:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\12569\Desktop\ObjCutter\ObjCutter\Types.cpp > CMakeFiles\ObjCutter.dir\Types.cpp.i

CMakeFiles/ObjCutter.dir/Types.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/ObjCutter.dir/Types.cpp.s"
	D:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\12569\Desktop\ObjCutter\ObjCutter\Types.cpp -o CMakeFiles\ObjCutter.dir\Types.cpp.s

CMakeFiles/ObjCutter.dir/ObjCutter.cpp.obj: CMakeFiles/ObjCutter.dir/flags.make
CMakeFiles/ObjCutter.dir/ObjCutter.cpp.obj: C:/Users/12569/Desktop/ObjCutter/ObjCutter/ObjCutter.cpp
CMakeFiles/ObjCutter.dir/ObjCutter.cpp.obj: CMakeFiles/ObjCutter.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\12569\Desktop\ObjCutter\ObjCutter\out\build\ObjCutter\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/ObjCutter.dir/ObjCutter.cpp.obj"
	D:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/ObjCutter.dir/ObjCutter.cpp.obj -MF CMakeFiles\ObjCutter.dir\ObjCutter.cpp.obj.d -o CMakeFiles\ObjCutter.dir\ObjCutter.cpp.obj -c C:\Users\12569\Desktop\ObjCutter\ObjCutter\ObjCutter.cpp

CMakeFiles/ObjCutter.dir/ObjCutter.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/ObjCutter.dir/ObjCutter.cpp.i"
	D:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\12569\Desktop\ObjCutter\ObjCutter\ObjCutter.cpp > CMakeFiles\ObjCutter.dir\ObjCutter.cpp.i

CMakeFiles/ObjCutter.dir/ObjCutter.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/ObjCutter.dir/ObjCutter.cpp.s"
	D:\msys64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\12569\Desktop\ObjCutter\ObjCutter\ObjCutter.cpp -o CMakeFiles\ObjCutter.dir\ObjCutter.cpp.s

# Object files for target ObjCutter
ObjCutter_OBJECTS = \
"CMakeFiles/ObjCutter.dir/main.cpp.obj" \
"CMakeFiles/ObjCutter.dir/Types.cpp.obj" \
"CMakeFiles/ObjCutter.dir/ObjCutter.cpp.obj"

# External object files for target ObjCutter
ObjCutter_EXTERNAL_OBJECTS =

ObjCutter.exe: CMakeFiles/ObjCutter.dir/main.cpp.obj
ObjCutter.exe: CMakeFiles/ObjCutter.dir/Types.cpp.obj
ObjCutter.exe: CMakeFiles/ObjCutter.dir/ObjCutter.cpp.obj
ObjCutter.exe: CMakeFiles/ObjCutter.dir/build.make
ObjCutter.exe: CMakeFiles/ObjCutter.dir/linkLibs.rsp
ObjCutter.exe: CMakeFiles/ObjCutter.dir/objects1.rsp
ObjCutter.exe: CMakeFiles/ObjCutter.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Users\12569\Desktop\ObjCutter\ObjCutter\out\build\ObjCutter\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable ObjCutter.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\ObjCutter.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ObjCutter.dir/build: ObjCutter.exe
.PHONY : CMakeFiles/ObjCutter.dir/build

CMakeFiles/ObjCutter.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\ObjCutter.dir\cmake_clean.cmake
.PHONY : CMakeFiles/ObjCutter.dir/clean

CMakeFiles/ObjCutter.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\12569\Desktop\ObjCutter\ObjCutter C:\Users\12569\Desktop\ObjCutter\ObjCutter C:\Users\12569\Desktop\ObjCutter\ObjCutter\out\build\ObjCutter C:\Users\12569\Desktop\ObjCutter\ObjCutter\out\build\ObjCutter C:\Users\12569\Desktop\ObjCutter\ObjCutter\out\build\ObjCutter\CMakeFiles\ObjCutter.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/ObjCutter.dir/depend

