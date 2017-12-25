CXX=icpc
CXX_FLAGS=-openmp -ipo -O3 -xhost
#CXX=g++
#CXX_FLAGS=-fopenmp -O3 -march=native -ftree-vectorize
PYTHON=python3
ROOT="../../../../"

# app inputs
IMG=$(ROOT)"/../images/venice_wikimedia.jpg"
THRESH=0.001
WEIGHT=4
RUNS=10

BOXES=1
CELLS=128

APP_ARGS=--threshold=$(THRESH) --weight=$(WEIGHT) --boxes=$(BOXES) --cells=$(CELLS)
OPT_ARGS=--pool-alloc --optimize-storage --early-free #--inline
TOOL_ARGS=--graph-gen --display --runs=$(RUNS) --timer
CXX_ARGS=--cxx="$(CXX)" --cxx_flags="$(CXX_FLAGS)"

all: new

new:
	$(PYTHON) main.py --mode='new' $(APP_ARGS) $(OPT_ARGS) $(TOOL_ARGS) $(CXX_ARGS)

dpfuse:
	$(PYTHON) main.py --mode='new' $(APP_ARGS) $(OPT_ARGS) --dpfuse $(TOOL_ARGS) $(CXX_ARGS)
	
dpfusechoices8:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS) --dpfuse --logdpchoices --logmaxchildren=3 ${APP_ARGS} ${TOOL_ARGS}

dpfusechoices16:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS)  --dpfuse --logdpchoices --logmaxchildren=4 ${APP_ARGS} ${TOOL_ARGS}

dpfusechoices32:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS)  --dpfuse --logdpchoices --logmaxchildren=5 ${APP_ARGS} ${TOOL_ARGS}

dpfusechoices:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS)  --dpfuse --logdpchoices --logmaxchildren=10 ${APP_ARGS} ${TOOL_ARGS}

tune:
	$(PYTHON) main.py --mode='tune' $(APP_ARGS) $(OPT_ARGS) $(TOOL_ARGS) $(CXX_ARGS)

tune+:
	$(PYTHON) main.py --mode='tune+' $(APP_ARGS) $(OPT_ARGS) $(TOOL_ARGS) $(CXX_ARGS)

existing:
	$(PYTHON) main.py --mode='existing' $(APP_ARGS) $(TOOL_ARGS) $(CXX_ARGS)

ready:
	$(PYTHON) main.py --mode='ready' $(APP_ARGS) $(TOOL_ARGS)

clean:
	rm -rf *.pyc *.so *.dot *.png __pycache__

cleaner:
	rm -rf *.pyc *.so *.dot *.png __pycache__ *.cpp
