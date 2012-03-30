
SAGA_SRC         = $(wildcard *.cpp)
SAGA_OBJ         = $(SAGA_SRC:%.cpp=%.o)

include $(SAGA_LOCATION)/share/saga/make/saga.application.mk

SAGA_CPPINCS    += -I../
SAGA_CXXFLAGS   += -fPIC -Wall -ansi -std=c++98

