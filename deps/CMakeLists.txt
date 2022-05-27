ADD_CMAKE_SUBDIRS_HERE()


ADD_GIT_DEPENDENCY("${CMAKE_CURRENT_LIST_DIR}/httplib" httplib "https://github.com/yhirose/cpp-httplib")
ADD_GIT_DEPENDENCY("${CMAKE_CURRENT_LIST_DIR}/jclib" jclib "https://github.com/JonathanCline/JCLib" develop)

