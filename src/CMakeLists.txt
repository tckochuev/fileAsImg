set(TARGET_NAME ${PROJECT_NAME})

add_executable(
	${TARGET_NAME}
	main.cpp
	VSNamespace.h
	VSUtils.h
	VSExportFileAsImages.h
	VSIExporterAsImages.h
	VSIPreviewGenerator.h
	VSIInterruptible.h
	VSIInterruptible.cpp
	VSAsposeSlidesManager.h
	VSAsposeSlidesManager.cpp
)

#Boost
find_package(Boost REQUIRED COMPONENTS regex program_options)
if(Boost_VERSION VERSION_LESS_EQUAL "1.62")
	target_compile_definitions(${TARGET_NAME} PRIVATE _HAS_AUTO_PTR_ETC=1)
endif()

#Aspose
set(ASPOSE_ROOT "${PROJECT_SOURCE_DIR}/lib/aspose-slides-cpp-windows-23.9")
set(ASPOSE_CORE CodePorting.Translator.Cs2Cpp.Framework)
set(ASPOSE_SLIDES Aspose.Slides.Cpp)
find_package(${ASPOSE_CORE} REQUIRED CONFIG PATHS ${ASPOSE_ROOT} NO_DEFAULT_PATH)
find_package(${ASPOSE_SLIDES} REQUIRED CONFIG PATHS ${ASPOSE_ROOT} NO_DEFAULT_PATH)
file(TO_NATIVE_PATH "${ASPOSE_CORE}/lib" ${ASPOSE_CORE}_DLL_PATH)
file(TO_NATIVE_PATH "${Aspose.Slides.Cpp_DIR}/lib" ${ASPOSE_CORE}_DLL_PATH)


target_include_directories(
	${TARGET_NAME}
	PRIVATE ${Boost_INCLUDE_DIRS}
)
target_link_libraries(${TARGET_NAME}
	PRIVATE ${Boost_LIBRARIES}
	PRIVATE ${ASPOSE_CORE}
	PRIVATE ${ASPOSE_SLIDES}
)
