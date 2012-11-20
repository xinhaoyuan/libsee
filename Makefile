.PHONY: all 

T_CC_FLAGS       ?= ${T_CC_FLAGS_OPT} -Wall -I include -D__SEE_INTERNAL__
T_CC_OPT_FLAGS   ?= -O2
T_CC_DEBUG_FLAGS ?= 

SRCFILES:= $(shell find src '(' '!' -regex '.*/_.*' ')' -and '(' -iname "*.c" ')' | sed -e 's!^\./!!g')

include ${T_BASE}/utl/template.mk

all: ${T_OBJ}/${PRJ}-run ${T_OBJ}/${PRJ}-comp

-include ${DEPFILES}

${T_OBJ}/${PRJ}.a: ${OBJFILES}
	@echo AR $@
	${V}ar r $@ $^

${T_OBJ}/${PRJ}-run: run.c ${T_OBJ}/${PRJ}.a
	@echo CC $@
	${V}${CC} ${T_CC_FLAGS_OPT} ${CC_DEBUG_FLAGS} -Wall -I include $^ -o $@

${T_OBJ}/${PRJ}-comp: comp.cpp ${T_OBJ}/${PRJ}.a
	@echo CXX $@
	${V}${CXX} -I include $^ -o $@
