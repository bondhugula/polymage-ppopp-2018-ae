#CXX := icpc
CXX := g++

SRC := mgm.c

SRCPLUTO := mgm.c

# app inputs
#CYCLE:=V # MG Cycle : V / W
#L:=3 # MG levels
#SIZE:=31 # coarse-grid size along each dimension
#NIT:=10 # V / W Cycle iterations
#NU1:=10 # pre-smoothing steps
#NU2:=0 # post-smoothing steps
#NUC:=0 # corase-smoothing steps

CFLAGS := -O3 -fopenmp -march=native -ftree-vectorize -msse3 -Wno-write-strings -Wno-unused-result
#CFLAGS := -O3 -qopenmp -xhost -ipo
#LDFLAGS := -lm

all: mgm mgm-pluto

ready: mgm-ready pluto-ready

mgm: $(SRC)
	$(CXX) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)
	echo "$(SIZE) 3 $(CYCLE) $(NU1) $(NU2) $(NUC) $(NIT) y" | ./mgm

mgm-pluto: $(SRCPLUTO)
	$(CXX) $(CFLAGS) $(SRCPLUTO) -o $@ $(LDFLAGS)
	echo "$(SIZE) 3 $(CYCLE) $(NU1) $(NU2) $(NUC) $(NIT) y" | ./mgm-pluto

mgm-ready: $(SRC)
	$(info ============== Handopt  ===============)
	echo "$(SIZE) 3 $(CYCLE) $(NU1) $(NU2) $(NUC) $(NIT) y" | ./mgm

pluto-ready: $(SRCPLUTO)
	$(info ============== Handopt+Pluto  ===============)
	echo "$(SIZE) 3 $(CYCLE) $(NU1) $(NU2) $(NUC) $(NIT) y" | ./mgm-pluto


clean: 
	rm mgm mgm-pluto 
