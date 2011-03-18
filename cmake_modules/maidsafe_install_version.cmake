#==============================================================================#
#                                                                              #
#  Copyright (c) 2011 maidsafe.net limited                                     #
#  All rights reserved.                                                        #
#                                                                              #
#  Redistribution and use in source and binary forms, with or without          #
#  modification, are permitted provided that the following conditions are met: #
#                                                                              #
#      * Redistributions of source code must retain the above copyright        #
#        notice, this list of conditions and the following disclaimer.         #
#      * Redistributions in binary form must reproduce the above copyright     #
#        notice, this list of conditions and the following disclaimer in the   #
#        documentation and/or other materials provided with the distribution.  #
#      * Neither the name of the maidsafe.net limited nor the names of its     #
#        contributors may be used to endorse or promote products derived from  #
#        this software without specific prior written permission.              #
#                                                                              #
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" #
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   #
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  #
#  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE  #
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         #
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        #
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    #
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     #
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     #
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  #
#  POSSIBILITY OF SUCH DAMAGE.                                                 #
#                                                                              #
#==============================================================================#
#                                                                              #
#  Written by maidsafe.net team                                                #
#                                                                              #
#==============================================================================#
#                                                                              #
#  Module used to append or amend MaidSafe project version information at      #
#  install time.                                                               #
#                                                                              #
#==============================================================================#


STRING(REPLACE "\"" "" INSTALL_FILE_DIR ${INSTALL_FILE_DIR})
STRING(REPLACE "\"" "" INSTALL_FILE_BASENAME ${INSTALL_FILE_BASENAME})
FILE(GLOB VERSION_INFOS RELATIVE ${INSTALL_FILE_DIR} "${INSTALL_FILE_DIR}/VERSION.*")
FOREACH(VERSION_INFO ${VERSION_INFOS})
  FILE(REMOVE "${INSTALL_FILE_DIR}/${VERSION_INFO}")
  STRING(REPLACE "." ";" VERSION_INFO ${VERSION_INFO})
  LIST(GET VERSION_INFO 1 VERSION_VARIABLE)
  LIST(GET VERSION_INFO 2 VERSION_VALUE)
  LIST(GET VERSION_INFO 3 VERSION_BUILD_TYPE)
  FIND_FILE(INSTALL_FILE ${INSTALL_FILE_BASENAME}-${VERSION_BUILD_TYPE}.cmake PATHS ${INSTALL_FILE_DIR})
  IF(INSTALL_FILE)
    FILE(APPEND ${INSTALL_FILE} "\n\n# Definition of this library's version\n")
    FILE(APPEND ${INSTALL_FILE} "SET(${VERSION_VARIABLE} ${VERSION_VALUE})\n")
  ENDIF()
ENDFOREACH()