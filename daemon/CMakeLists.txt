#   This file is part of Icecream.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

add_executable(iceccd
  environment.cpp
  environment.h
  file_util.cpp
  file_util.h
  load.cpp
  load.h
  main.cpp
  serve.cpp
  serve.h
  workit.cpp
  workit.h
  $<TARGET_OBJECTS:md5>
)

target_link_libraries(iceccd
  icecc
  #LibArchive::LibArchive # -- Would be nice to use this system, as it would give us both includes and libs, but CMake 3.10 doesn't have it.
  ${LibArchive_LIBRARIES} # -- So we use the libs variable found
  $<$<BOOL:${Libcap-ng_FOUND}>:Libcap-ng::Libcap-ng>
)

target_include_directories(iceccd PUBLIC
  ${LibArchive_INCLUDE_DIRS} # -- So we use the includes variable found
)

install(TARGETS iceccd 
        RUNTIME DESTINATION sbin
)
