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

if (DOCBOOK2MAN_FOUND)
    message("Generating man pages")
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icecc.1 
                       COMMAND ${DOCBOOK2MAN_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/man-icecc.1.xml
                       MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/man-icecc.1.xml
    )
    
    add_custom_target(man ALL
                      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/icecc.1 
    )
    
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/icecc.1
            DESTINATION share/man/man1
    )
else()
    message("docbook-to-man executable not found, man pages will not be generated")
endif()
