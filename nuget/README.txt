This package contains static Expat libraries and header files
for the x64 platform, built with Visual C++ 2022, against
Debug/Release MT/DLL MSVC CRT.

The Expat static library appropriate for the platform and
configuration selected in a Visual Studio solution is explicitly
referenced within this package and will appear within the solution
folder tree after the package is installed, except for static
library projects. The solution may need to be reloaded to make
the library file visible. This library may be moved into any
solution folder after the installation.

Note that the Expat library path in this package will be selected
as Debug or Release based on whether the active configuration is
designated as a development or as a release configuration in the
underlying .vcxproj file.

Specifically, the initial project configurations have a property
called UseDebugLibraries in the underlying .vcxproj file, which
reflects whether the confiration is intended for building release
or development artifacts. Additional configurations copied from
these initial ones inherit this property. Manually created
configurations should have this property defined in the .vcxproj
file.

Do not install this package if your projects use debug configurations
without `UseDebugLibraries`. Note that CMake-generated Visual Studio
projects will not emit this property.

Expat libraries in this package are built with narrow character
support only and cannot parse XML documents maintained as wide
character strings.
