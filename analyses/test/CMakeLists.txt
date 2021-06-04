############################################################################
# CMakeLists.txt file for building XBOX tdms io sub package
############################################################################

#...........................................................................
set(target test_AnalysisEvaluators)

if(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#                  LIBRARIES ${ROOT_LIBRARIES} libMathMore.lib xboxcore xboxio xboxanalyses)
else(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#                  LIBRARIES ${ROOT_LIBRARIES} -lMathMore xboxcore xboxio xboxanalyses)
endif(WIN32)
XBOX_ADD_TEST(${target} COMMAND ${target})

#...........................................................................
set(target test_AnalysisDefault)

if(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#                  LIBRARIES ${ROOT_LIBRARIES} libMathMore.lib xboxcore xboxio xboxanalyses)
else(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#                  LIBRARIES ${ROOT_LIBRARIES} -lMathMore xboxcore xboxio xboxanalyses)
endif(WIN32)
XBOX_ADD_TEST(${target} COMMAND ${target})

#...........................................................................
set(target test_AnalysisXiaowei)

if(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
else(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
endif(WIN32)
XBOX_ADD_TEST(${target} COMMAND ${target})

#...........................................................................
#set(target test_AnalysisPolarix)
#
#if(WIN32)
#  XBOX_EXECUTABLE(${target} 
#                  ${target}.cpp 
#                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#else(WIN32)
#  XBOX_EXECUTABLE(${target} 
#                  ${target}.cpp 
#                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#endif(WIN32)
#XBOX_ADD_TEST(${target} COMMAND ${target})

##...........................................................................
set(target test_AnalysisViewDefault)

if(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
else(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
endif(WIN32)
XBOX_ADD_TEST(${target} COMMAND ${target})

##...........................................................................
#set(target test_AnalysisViewPolarix)
#
#if(WIN32)
#  XBOX_EXECUTABLE(${target} 
#                  ${target}.cpp 
#                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#else(WIN32)
#  XBOX_EXECUTABLE(${target} 
#                  ${target}.cpp 
#                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#endif(WIN32)
#XBOX_ADD_TEST(${target} COMMAND ${target})
#
#
#...........................................................................
set(target test_AnalyseHistory4)

if(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#                  LIBRARIES ${ROOT_LIBRARIES} libMathMore.lib xboxcore xboxio xboxanalyses)
else(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#                  LIBRARIES ${ROOT_LIBRARIES} -lMathMore xboxcore xboxio xboxanalyses)
endif(WIN32)
XBOX_ADD_TEST(${target} COMMAND ${target})

#...........................................................................
set(target test_Spline)

if(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#                  LIBRARIES ${ROOT_LIBRARIES} libMathMore.lib xboxcore xboxio xboxanalyses)
else(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio xboxanalyses)
#                  LIBRARIES ${ROOT_LIBRARIES} -lMathMore xboxcore xboxio xboxanalyses)
endif(WIN32)
XBOX_ADD_TEST(${target} COMMAND ${target})

#...........................................................................
set(target test_RDataFrames)

if(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio)
#                  LIBRARIES ${ROOT_LIBRARIES} libMathMore.lib xboxcore xboxio xboxanalyses)
else(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio)
#                  LIBRARIES ${ROOT_LIBRARIES} -lMathMore xboxcore xboxio xboxanalyses)
endif(WIN32)
XBOX_ADD_TEST(${target} COMMAND ${target})

