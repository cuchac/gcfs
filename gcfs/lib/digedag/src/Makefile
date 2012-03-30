
SAGA_SUBDIRS      = util

SAGA_SRC          = $(wildcard *.cpp)
SAGA_ADD_LIB_OBJ  = $(SAGA_SRC:%.cpp=%.o)
SAGA_ADD_LIB_OBJ += util/thread.o util/split.o util/util.o
SAGA_LIB          = digedag

include $(SAGA_LOCATION)/share/saga/make/saga.application.mk

SAGA_CXXFLAGS    += -fPIC -Wall # -ansi -std=c++98

SAGA_CPPINCS     += -I./
SAGA_CPPINCS     += -I../external/tinyxml++/
SAGA_LDFLAGS     += -L../external/tinyxml++/ -lsaga_tinyxml++

e:
edit:
	@$(XEDITOR) `$(CAT) FILES`

