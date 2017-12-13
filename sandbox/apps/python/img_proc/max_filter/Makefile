CXX=icpc
CXX_FLAGS=-qopenmp -ipo -O3 -xhost
#CXX=g++
#CXX_FLAGS=-fopenmp -O3 -march=native -ftree-vectorize
PYTHON=python3
ROOT="../../../.."

# app inputs
#IMG=$(ROOT)"/../images/rgb.png"
IMG="rgb.png"
#ALPHA=$(ROOT)"/../images/alpha.png"
RUNS=1

APP_ARGS=--img=${IMG} #--alpha=${ALPHA}
OPT_ARGS=--pool-alloc --optimize-storage --early-free
TOOL_ARGS=--graph-gen --runs=${RUNS} --display --timer
CXX_ARGS=--cxx="${CXX}" --cxx_flags="${CXX_FLAGS}"

all: new

new:
	$(PYTHON) main.py --mode='new' ${CXX_ARGS} $(OPT_ARGS) ${APP_ARGS} ${TOOL_ARGS}

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
