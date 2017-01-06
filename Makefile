include dwir.rules

all: util util_th util_haneng cgidw index bdb_store anal_scan

util: $(DWIR_SRC)/util.mak
	cd $(DWIR_SRC) \
	&& make -i -f util.mak clean \
	&& make -f util.mak 

util_th: $(DWIR_SRC)/util_th.mak
	cd $(DWIR_SRC) \
	&& make -i -f util_th.mak clean \
	&& make -f util_th.mak 

util_haneng: $(DWIR_SRC)/util_haneng.mak
	cd $(DWIR_SRC) \
	&& make -i -f util_haneng.mak clean \
	&& make -f util_haneng.mak

cgidw: $(DWIR_SRC)/cgi.mak
	cd $(DWIR_SRC) \
	&& make -i -f cgi.mak clean \
	&& make -f cgi.mak

index: $(DWIR_SRC)/index.mak
	cd $(DWIR_SRC) \
	&& make -i -f index.mak clean \
	&& make -f index.mak

bdb_store: $(DWIR_SRC)/bdb_store.mak
	cd $(DWIR_SRC) \
	&& make -i -f bdb_store.mak clean \
	&& make -f bdb_store.mak

anal_scan: $(SCAN_DIR)/libscan.mak $(SCAN_DIR)/libstag.mak
	cd $(SCAN_DIR) \
	&& make -i -f libscan.mak clean \
	&& make -f libscan.mak \
	&& make -i -f libstag.mak clean \
	&& make -f libstag.mak

clean: 
	cd $(DWIR_SRC) && make -i -f util.mak clean 
	cd $(DWIR_SRC) && make -i -f util_th.mak clean 
	cd $(DWIR_SRC) && make -i -f util_haneng.mak clean 
	cd $(DWIR_SRC) && make -i -f cgi.mak clean 
	cd $(DWIR_SRC) && make -i -f index.mak clean 
	cd $(DWIR_SRC) && make -i -f bdb_store.mak clean 
	cd $(SCAN_DIR) && make -i -f libscan.mak clean  && make -i -f libstag.mak clean
