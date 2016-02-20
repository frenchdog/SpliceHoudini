DSOSTEM = OpenSpliceHoudini
VERSION = 0.4.0

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
ifeq (${uname_S},Linux)
	EXT = so
else ifeq (${uname_S}, Darwin)
	EXT = dylib
endif

DSONAME = ${DSOSTEM}.${VERSION}.${EXT}

WIDGET := FabricDFGWidget

SOURCES = \
	src/core/FabricDFGView.cpp \
	src/core/MultiParams.cpp \
	src/core/FabricDFGOP.cpp \
	src/core/${WIDGET}.cpp \
	src/core/moc_${WIDGET}.cpp \
	src/core/CanvasUI.cpp \
	src/core/ParameterFactory.cpp \
	src/SOP_FabricGenerator.cpp \
	src/SOP_FabricDeformer.cpp \
	src/OBJ_FabricKinematic.cpp \
	src/plugin.cpp

INCDIRS = -I${FABRIC_DIR}/include/
INCDIRS += -I${FABRIC_DIR}/include/FabricServices

LIBDIRS = -L${FABRIC_DIR}/lib

FABRIC_UI_PATH = ${FABRIC_DIR}/../FabricUI/stage
INCDIRS += -I${FABRIC_UI_PATH}/include/FabricUI
INCDIRS += -I${FABRIC_UI_PATH}/include
LIBDIRS += -L${FABRIC_UI_PATH}/lib
LIBS += -lFabricUI -lFabricCore -lFabricServices -lFabricSplitSearch -ldl -lpthread

INCDIRS += -I${HFS}/toolkit/include/OpenEXR/

QT_MOC	:= ${QT_DIR}/bin/moc

OPTIMIZER = -g

ICONS = \
	icons/OBJ_fabricObject.png \
	icons/SOP_fabricDeformer.png \
	icons/SOP_fabricGenerator.png \

include ${HFS}/toolkit/makefiles/Makefile.gnu

# A simple Qt's moc preprocessor rule for our DFG widget 
src/core/moc_${WIDGET}.cpp: src/core/${WIDGET}.h
	${QT_MOC} src/core/${WIDGET}.h -o src/core/moc_${WIDGET}.cpp

clean_all:
	rm -f ${OBJECTS} ${APPNAME} ${DSONAME} src/core/moc_${WIDGET}.cpp
