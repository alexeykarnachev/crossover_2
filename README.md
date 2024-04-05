### Build ImGui
```bash
g++ -c \
./*.cpp ./backends/imgui_impl_opengl3.cpp ./backends/imgui_impl_glfw.cpp \
-I. -I../glfw-3.4/include \
&& ar rcs libimgui.a ./*.o \
&& rm ./*.o \
&& mv ./libimgui.a ../../../deps/lib/linux/libimgui.a
```
