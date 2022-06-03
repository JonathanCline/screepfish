ADD_CMAKE_SUBDIRS_HERE()


ADD_GIT_DEPENDENCY("${CMAKE_CURRENT_LIST_DIR}/httplib" httplib "https://github.com/yhirose/cpp-httplib")
ADD_GIT_DEPENDENCY("${CMAKE_CURRENT_LIST_DIR}/jclib" jclib "https://github.com/JonathanCline/JCLib" develop)
ADD_GIT_DEPENDENCY("${CMAKE_CURRENT_LIST_DIR}/lodepng" lodepng "https://github.com/JonathanCline/lodepng")
ADD_GIT_DEPENDENCY("${CMAKE_CURRENT_LIST_DIR}/jccli" jccli "https://github.com/JonathanCline/jccli")
