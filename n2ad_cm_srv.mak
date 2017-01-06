include ../dwir.rules

CFLAGS		= $(COMMON_CFLAG) ${DWIR_INC} 
DFLAGS		= $(COMMON_DFLAG) ${PTHREAD_DFLAG}
LIB		= 
MAKEFILE	= n2ad_cm_srv.mak
BIN		= n2ad_cm_srv.exe
VPATH		= 

.SUFFIXES : .c .cpp .o

OBJS		= n2ad_cm_srv.o cJSON.o threadpool.o

LIB		= $(UTIL_TH_LIB) $(UTIL_LIB) $(PTHREAD_LIB) -L./ -lcredis $(SYSTEM_LIB)

all: $(OBJS) $(MAKEFILE)
	$(CXX) -o $(BIN) $(OBJS) $(LIB)
	$(STRIP) $(BIN)

clean:
	$(RM) -rf $(OBJS) $(BIN)

%.o : %.c
	$(CXX) -o $@ -c $(CFLAGS) $(DFLAGS) $(INC) $<

%.o : %.cpp
	$(CXX) -o $@ -c $(CFLAGS) $(DFLAGS) $(INC) $<

