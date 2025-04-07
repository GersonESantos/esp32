# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.3.1/components/bootloader/subproject"
  "C:/Repo2024/esp32/01esp/vAula541_WR_Files/classica541_WR_Files/web_server_control/build/bootloader"
  "C:/Repo2024/esp32/01esp/vAula541_WR_Files/classica541_WR_Files/web_server_control/build/bootloader-prefix"
  "C:/Repo2024/esp32/01esp/vAula541_WR_Files/classica541_WR_Files/web_server_control/build/bootloader-prefix/tmp"
  "C:/Repo2024/esp32/01esp/vAula541_WR_Files/classica541_WR_Files/web_server_control/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Repo2024/esp32/01esp/vAula541_WR_Files/classica541_WR_Files/web_server_control/build/bootloader-prefix/src"
  "C:/Repo2024/esp32/01esp/vAula541_WR_Files/classica541_WR_Files/web_server_control/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Repo2024/esp32/01esp/vAula541_WR_Files/classica541_WR_Files/web_server_control/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Repo2024/esp32/01esp/vAula541_WR_Files/classica541_WR_Files/web_server_control/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
