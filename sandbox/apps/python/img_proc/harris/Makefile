#CXX=icpc
#CXX_FLAGS=-openmp -ipo -O3 -xhost #-no-vec
CXX=g++
CXX_FLAGS=-fopenmp -O3 -march=native -ftree-vectorize -funroll-loops
PYTHON=python3
ROOT="../../../../"

# app inputs
IMG=$(ROOT)"/../images/venice_wikimedia.jpg"
#IMG="./gray.png"
RUNS=10

APP_ARGS=--img=${IMG}
OPT_ARGS=--pool-alloc --optimize-storage --early-free
TOOL_ARGS=--graph-gen --runs=${RUNS} --display --timer #--multi-level-tiling
CXX_ARGS=--cxx="${CXX}" --cxx_flags="${CXX_FLAGS}"

all: new

new:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS) ${APP_ARGS} ${TOOL_ARGS}

dpfuse:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS) --dpfuse ${APP_ARGS} ${TOOL_ARGS}

dpfusechoices8:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS) --dpfuse --logdpchoices --logmaxchildren=3 ${APP_ARGS} ${TOOL_ARGS}

dpfusechoices16:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS)  --dpfuse --logdpchoices --logmaxchildren=4 ${APP_ARGS} ${TOOL_ARGS}

dpfusechoices32:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS)  --dpfuse --logdpchoices --logmaxchildren=5 ${APP_ARGS} ${TOOL_ARGS}

dpfusechoices:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS)  --dpfuse --logdpchoices --logmaxchildren=10 ${APP_ARGS} ${TOOL_ARGS}

tune:
	$(PYTHON) main.py --mode='tune' ${CXX_ARGS} $(OPT_ARGS) ${APP_ARGS} ${TOOL_ARGS}
tune+:
	$(PYTHON) main.py --mode='tune+' $(APP_ARGS) $(OPT_ARGS) $(TOOL_ARGS) $(CXX_ARGS)
existing:
	$(PYTHON) main.py --mode='existing' ${CXX_ARGS} ${APP_ARGS} ${TOOL_ARGS}

ready:
	$(PYTHON) main.py --mode='ready' ${APP_ARGS} ${TOOL_ARGS}

clean:
	rm -rf *.pyc *.so *.dot *.png __pycache__

cleaner:
	rm -rf *.pyc *.so *.dot *.png __pycache__ *.cpp
