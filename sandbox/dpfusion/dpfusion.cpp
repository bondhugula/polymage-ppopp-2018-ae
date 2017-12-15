#include <Python.h>
#include <iostream>
#include <unordered_map> 
#include <unordered_set>
#include <set>
#include <algorithm>
#include <queue>
#include <regex>
#include <fstream>
#include <string>
#include <boost/multiprecision/cpp_int.hpp> 
#include <functional> 
#include <cctype>
#include <locale>
#include <math.h>
#include <map>
#include <numeric>

#include "Group.h"

//Print only if DEBUG level is more than i
#define PRINT_DEBUG_L1(x) { if (DEBUG != 0 && DEBUG <= 1) { x;}}
#define PRINT_DEBUG_BLOCK_L1 if (DEBUG != 0 && DEBUG <= 1)
#define PRINT_DEBUG_L2(x) { if (DEBUG != 0 && DEBUG <= 2) { x;}}
#define PRINT_DEBUG_L3(x) { if (DEBUG != 0 && DEBUG <= 3) { x;}}

/*DEBUG Levels
 * Highest Level Print all messages below that level
 * No Print level 0
 * */ 

bool const checkForAssertions = false;
bool const checkPythonExceptions = true;
bool printDPFusionTimeAndChoices = false;

static bool INLINING_ENABLED = false;
static bool MULTI_LEVEL_TILING_ENABLED = false;

static int IMAGE_ELEMENT_SIZE = 4; //Each image element is of 4 bytes
static uint64_t L2_CACHE_SIZE = 256*1024;  //The L2_CACHE_SIZE is 256KB
static uint64_t L1_CACHE_SIZE = 32*1024;  //The L2_CACHE_SIZE is 32KB
static int N_CORES = 4; //Number of Cores is 4

static float DIM_STD_DEV_WEIGHT;
static float LIVE_SIZE_TO_TILE_SIZE_WEIGHT;
static float CLEANUP_THREADS_WEIGHT;
static float RELATIVE_OVERLAP_WEIGHT;


#undef INLINING
#undef HEIRARICHAL_TILING
#undef MULTI_LEVEL_TILING

#undef __MCASTLE2_SERVER__

#ifdef __PERSONAL_COMPUTER__
static int const IMAGE_ELEMENT_SIZE = 4; //Each image element is of 4 bytes
static int const L2_CACHE_SIZE = 256*1024;  //The L2_CACHE_SIZE is 256KB
static int const N_CORES = 4; //Number of Cores is 4
#endif

#ifdef __MCASTLE1_SERVER__
    static int const IMAGE_ELEMENT_SIZE = 4; //Each image element is of 4 bytes
    static int const L2_CACHE_SIZE = 512*1024;  //The L2_CACHE_SIZE is 512KB
    static int const N_CORES = 16; //Number of Cores is 16
    static int const L1_CACHE_SIZE = 32*1024;
    static int const VECTOR_REGISTERS = 8;

    #if defined (INLINING) && defined(HEIRARICHAL_TILING) && defined (MULTI_LEVEL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
    #elif defined (INLINING) && defined (HEIRARICHAL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
    #elif defined (INLINING) && defined (MULTI_LEVEL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
    #elif defined (MULTI_LEVEL_TILING) && defined (HEIRARICHAL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
    #elif defined (INLINING)
    //INLINING
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
    #elif defined (HEIRARICHAL_TILING)
    //HEIRARICHAL_TILING
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
    #elif defined (MULTI_LEVEL_TILING)
    //MULTI_LEVEL_TILING
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
    #else
    //None of those are defined.
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
    #endif
#endif

#ifdef __POLYMAGE_SERVER__
    static int const IMAGE_ELEMENT_SIZE = 4; //Each image element is of 4 bytes
    static int const L2_CACHE_SIZE = 256*1024;  //The L2_CACHE_SIZE is 512KB
    static int const N_CORES = 16; //Number of Cores is 16
    static int const L1_CACHE_SIZE = 32*1024;
    static int const VECTOR_REGISTERS = 8;
    
    #if defined (INLINING) && defined(HEIRARICHAL_TILING) && defined (MULTI_LEVEL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
        
    #elif defined (INLINING) && defined (HEIRARICHAL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
        
    #elif defined (INLINING) && defined (MULTI_LEVEL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1.5f;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f/1.5f;
        
    #elif defined (MULTI_LEVEL_TILING) && defined (HEIRARICHAL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
        
    #elif defined (INLINING)
    //INLINING
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
        
    #elif defined (HEIRARICHAL_TILING)
    //HEIRARICHAL_TILING
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
        
    #elif defined (MULTI_LEVEL_TILING)
    //MULTI_LEVEL_TILING
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1.5f;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f/1.5f;
        
    #else
    //None of those are defined.
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
    #endif
#endif

#ifdef __MCASTLE2_SERVER__
    static int const IMAGE_ELEMENT_SIZE = 4; //Each image element is of 4 bytes
    static int const L2_CACHE_SIZE = 2*512*1024;  //The L2_CACHE_SIZE is 1MB per core
    static int const N_CORES = 16; //Number of Cores is 16
    #if defined (INLINING) && defined(HEIRARICHAL_TILING) && defined (MULTI_LEVEL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
        
    #elif defined (INLINING) && defined (HEIRARICHAL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
        
    #elif defined (INLINING) && defined (MULTI_LEVEL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1.5f;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f/1.5f;
        
    #elif defined (MULTI_LEVEL_TILING) && defined (HEIRARICHAL_TILING)
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
        
    #elif defined (HEIRARICHAL_TILING)
    //HEIRARICHAL_TILING
        static float const DIM_STD_DEV_WEIGHT = 1.5;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5;
        
    #elif defined (MULTI_LEVEL_TILING)
    //MULTI_LEVEL_TILING
        static float const DIM_STD_DEV_WEIGHT = 2;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 1.0f;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f/1.2f;
    
    #else
    //INLINING defined or None of those are defined.
        static float const DIM_STD_DEV_WEIGHT = 2;
        static float const LIVE_SIZE_TO_TILE_SIZE_WEIGHT = 0.3f;
        static float const CLEANUP_THREADS_WEIGHT = 100.0f;
        static float const RELATIVE_OVERLAP_WEIGHT = 1000.0f*50.0f*1.5f;
    #endif

#endif

//To Compile g++ -shared -I/usr/include/python3.4m/ -lpython3 -o optgrouping.so optgrouping_incremental.cpp -fPIC -O3 -std=c++11 -lboost_system
/**
 * Hash function fo PyObject*
 * */
struct PyObjectHasher
{
    uint64_t operator() (const PyObject* obj) const
    {
        return (uint64_t)obj;
    }
};

/**
 * Compare function for PyObject
 * */
struct PyObjectPred
{
    bool operator() (PyObject* obj1, PyObject* obj2) const
    {
        return (bool)PyObject_RichCompareBool (obj1, obj2, Py_EQ);
    }
};

using boost::multiprecision::uint128_t;
using namespace boost;
using namespace std;
uint64_t logMaxChildren = 3;
uint64_t log_increment_factor = 1;
PyObject* reduction_cls;
PyObject* small_comps;
PyObject* comp_size_map;
PyObject* tstencil_cls;
PyObject* pipeline;
PyObject* pygroup_topological_order;
PyObject* pygroup_dim_reuse;
PyObject* pylive_size;
PyObject* pydim_size;
PyObject* storage_mapping_get_dim_size;
PyObject* cls_Storage;
PyObject* get_overlapping_size_func;

long grp_size;

/*Map of Adjacent Vertices for a group. */
typedef std::unordered_map<Group*, std::set <Group*, Group::GroupComparer>, 
                            Group::GroupHasher> InlinedAdjacentType;
/*Table of costs for hash ids*/
std::unordered_map<uint128_t, uint64_t, uint128Hasher> T;

/*Table of Group for a hash ids */
std::unordered_map<uint128_t, Group*, uint128Hasher> Group::hashIDToGroup;
/*Table of OptGroup for hash ids*/
std::unordered_map<uint128_t, OptGroup*, uint128Hasher> OptGroup::optHashIDToGroup;
/*Table of hash id to its optimal hash id found*/
std::unordered_map<uint128_t, uint128_t, uint128Hasher> hashIDToOptHashID;
/*Table of PyObject to Group*/
std::unordered_map<PyObject*, Group*> pyToGroup;
/*Set of small computations*/
std::unordered_set<PyObject*, PyObjectHasher, PyObjectPred> small_comps_set;
/* */
std::unordered_map<uint128_t, OptGroup*, uint128Hasher> OptGroup::parentOptGroupsToHashID;
/*Table of tile sizes (in form of vector) found for hashid*/
std::unordered_map<uint128_t, std::vector<uint64_t>, uint128Hasher> optHashIDToTileSizes;
/*Table of ComputeObject (of Python) to Group (of Python)*/
std::unordered_map<PyObject*, PyObject*> pyGroupForComp;
/*Table of ComputeObject to original Storage Class*/
std::unordered_map<PyObject*, PyObject*> comp_to_orig_storage;
/*Table of storage class to key*/
std::unordered_map<PyObject*, string> stg_class_to_key;
/*Table of ComputeObject to Function (of Python)*/
std::unordered_map<PyObject*, PyObject*> comp_to_func;
/*Table of Function (of Python) to its type*/
std::unordered_map<PyObject*, PyObject*> func_to_typ;
/*Table of Function to number of dimensions*/
std::unordered_map<PyObject*, long> func_to_ndims;
/*Table of Computation Object to maximum offsets*/
std::unordered_map<PyObject*, vector<long> > comp_to_max_offsets;
/* */
std::unordered_map<PyObject*, vector<PyObject*> > helper_storage_to_dim_storage;
/*Vector of all OptGroups found in current iterations*/
vector<OptGroup*> OptGroup::vectorOptGroups;

/**
 * Convert a Python String object to C++ STD String
 * */
string pystring_to_string (PyObject* str)
{
    char* _str;
    string cxx_str;
    
    PyObject* pyStr = PyUnicode_AsEncodedString(str, "utf-8", "Error ~");
    _str = PyBytes_AS_STRING (pyStr);
    //Py_DECREF (pyStr);
    cxx_str = string (_str);
    //free (_str);
    return cxx_str;
}

/**
 * Returns the number of ones in the hash_id, i.e. the number of children in
 * a group
 * */
inline uint64_t numberOfOnes (uint128_t hash_id)
{
    uint64_t bit = 0;
    uint64_t nOnes = 0;
        
    while (hash_id != 0)
    {
        if ((hash_id & 1L) == 1)
        {   
            nOnes ++;
        }
        
        hash_id = hash_id >> 1;
        bit++;
    }
    
    return nOnes;
}

/**
 * Returns the position of first rightmost "1" in the hash_id 
 * */
inline uint64_t first_one (uint128_t hash_id)
{
    uint64_t bit = 0;
        
    while (hash_id != 0)
    {
        if ((hash_id & 1L) == 1)
        {   
            return bit;
        }
        
        hash_id = hash_id >> 1;
        bit++;
    }
    
    return -1;
}

/**
 * Returns number of dimensions for Group (of Python)
 * */
uint64_t get_dim_size_for_pygroup (PyObject* group, int dim)
{
    PyObject* dims_size;
    
    dims_size = PyDict_GetItem (pydim_size, group);
    
    return (uint64_t)PyLong_AsLong (PyList_GetItem (dims_size, dim));
}

/**
 * Return the dimensional resuse (list of reuse in each dimension) for a Group
 * */
int get_dim_reuse_for_pygroup (PyObject* group, int dim)
{
    PyObject* dim_reuse;
    
    dim_reuse = PyDict_GetItem (pygroup_dim_reuse, group);
    
    return (int)PyLong_AsLong (PyList_GetItem (dim_reuse, dim));
}

/**
 * Check if there has been a Python exception and if true print and abort
 * */
void check_and_print_exception ()
{
    if (PyErr_Occurred () != NULL)
    {
        PyErr_Print();
        abort ();
    }
}

/**
 * Returns the nubmber of dimensions for Group (of Python)
 * */
int get_n_dimensions (PyObject* group)
{
    PyObject* dim_reuse;
    
    dim_reuse = PyDict_GetItem (pygroup_dim_reuse, group);
    //check_and_print_exception ();
    return (int)PyList_Size (dim_reuse);
}

/**
 * Returns live size for Group (of Python)
 * */
uint64_t get_live_size (PyObject* group)
{
    PyObject* size;
    
    size = PyDict_GetItem (pylive_size, group);
    
    return (uint64_t)PyLong_AsLong (size);
}

/**
 * Iterates over all children of a group and returns the maximum dimension.
 * */
int get_max_dimensions_for_group (uint128_t hash_id)
{
    uint64_t bit = 0;
    int max_dim = 0;

    while (hash_id != 0)
    {
        if ((hash_id & 1L) == 1)
        {
            uint128_t l = 1;
            PyObject* pyGroup = Group::hashIDToGroup[l<<bit]->getPyGroup ();
    
            if (pyGroup != NULL)
            {
                int dim = get_n_dimensions (pyGroup);
                
                if (max_dim < dim)
                    max_dim = dim;
            }
        }
        
        hash_id = hash_id >> 1;
        bit++;
    }
    
    return max_dim;
}

/**
 * Returns ComputeObject at an index for a Group.
 * */
PyObject* getCompForPyGroup (PyObject* pyGroup, int index)
{
    static PyObject* str_comps = Py_BuildValue ("s", "comps");
    PyObject* comps = PyObject_GetAttr (pyGroup, str_comps);
    return PyList_GetItem (comps, index);
}

/**
 * Returns the Function name for given ComputeObject.
 * */
char* getPyCompFuncName (PyObject* comp)
{
    static PyObject* str_func = Py_BuildValue ("s", "func");
    static PyObject* str_name = Py_BuildValue ("s", "name");
    PyObject* func = PyObject_GetAttr (comp, str_func);
    PyObject* name = PyObject_GetAttr (func, str_name);
    PyObject* pyStr = PyUnicode_AsEncodedString(name, "utf-8", "Error ~");
    return PyBytes_AS_STRING (pyStr);
}

/**
 * Returns the name of Group (Python).
 * */
char* getPyGroupName (PyObject* pygroup)
{
    if (pygroup == nullptr)
    {
        std::string s = "Dummy Group";
        char* ret = new char[s.length()];
        
        strcpy (ret, s.c_str());
        
        return ret;
    }
    
    PyObject* name = PyObject_Str (pygroup);
    PyObject* pyStr = PyUnicode_AsEncodedString(name, "utf-8", "Error ~");
    return PyBytes_AS_STRING (pyStr);
}

/**
 * Returns the number of incoming edges to the children of group.
 * */
uint64_t getNumberOfInputs (uint128_t hash_id)
{
    uint64_t bit = 0;
    uint64_t nOnes = 0;
    uint64_t input_size = 0;
    uint128_t _hash_id = hash_id;
    
    while (_hash_id != 0)
    {
        if ((_hash_id & 1L) == 1)
        {
            uint128_t l;
            Group* group;
            
            l = 1;
            group = Group::hashIDToGroup[l<<bit];
            nOnes++;
            
            if (group->prevGroups().size () == 0)
            {
                input_size++;
            }
            
            for (auto it = group->prevGroups().begin(); 
                 it != group->prevGroups().end (); it++)
            {
                if ((hash_id & (*it)->hashID()) != (*it)->hashID())
                {
                    input_size++;
                }
            }
        }
        
        _hash_id = _hash_id >> 1;
        bit++;
    }
    
    return input_size;
}

/**
 * Returns the level order traversal in *order* of children in a group.
 * 
 * hash_id: HashID of Group
 * objs: ComputeObjects of Group (No need, remove as an argument)
 * vec_groups: Children of groups
 * order: level order traversal to be returned in.
 * inlined_groups: Set of groups inlined (No need, remove as an argument)
 * new_nextGroups: Map of Group to Next Groups
 * new_prevGroups: Map of Group to Prev Groups
 * */
void get_level_order (uint128_t hash_id, vector <PyObject*>& objs, 
                      const vector <Group*>& vec_groups, 
                      map <Group*, int, Group::GroupComparer>& order,
                      std::unordered_set<Group*, Group::GroupHasher>& inlined_groups,
                      InlinedAdjacentType& new_nextGroups,
                      InlinedAdjacentType& new_prevGroups)
{
    static PyObject* str_func = Py_BuildValue ("s", "func");

    bool change = true;
    
    for (auto it = vec_groups.begin(); it != vec_groups.end (); it ++)
    {
        order [*it] = 0;
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "Final New Next Groups in get_level_order" << std::endl;
        for (auto const &it : new_nextGroups)
        {
            std::cout << getPyGroupName (it.first->getPyGroup ()) << " -> ";
            
            for (auto const &g : it.second)
            {
                std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
            }
            
            std::cout << std::endl;
        }
        
        std::cout << "Final New Prev Groups in get_level_order " << std::endl;
        for (auto const &it : new_prevGroups)
        {
            std::cout << getPyGroupName (it.first->getPyGroup ()) << " <- ";
            
            for (auto const &g : it.second)
            {
                std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
            }
            
            std::cout << std::endl;
        }
    }
    
    while (change)
    {
        change = false;

        for (auto obj = vec_groups.begin (); obj != vec_groups.end (); obj++)
        {
            for (auto &prev: new_prevGroups[*obj])
            {
                if ((prev->hashID () & hash_id) == prev->hashID ())
                {                
                    if (order.find (prev) != order.end () &&
                        order [prev] >= order [*obj])
                    {
                        order [*obj] = order [prev] + 1;
                        change = true;
                    }
                }
            }
        }
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout<< "optorder is"<<std::endl;
        for (auto it = order.begin(); it != order.end(); it++)
        {
            PyObject* func = PyObject_GetAttr (it->first->getCompAtIndex(0), str_func);
            
            PyObject* name = PyObject_GetAttr (func,
                                            Py_BuildValue ("s", "name"));
            PyObject* pyStr = PyUnicode_AsEncodedString(name, "utf-8", "Error ~");
            const char* ss = PyBytes_AS_STRING (pyStr);
                std::cout<< "     "<< ss<< it->second << std::endl;
        }
    }
}

/**
 * 
 * */
void naive_sched_objs (const map <Group*, int, Group::GroupComparer>& order, 
                       unordered_map <Group*, int>& naive_order,
                       InlinedAdjacentType& new_nextGroups,
                       InlinedAdjacentType& new_prevGroups)
{
    static PyObject* str_func = Py_BuildValue ("s", "func");
    unordered_map <int, vector<Group*>> reverse_map;
    int max_level = 0;
    
    for (auto it = order.begin(); it != order.end(); it++)
    {
        int l = it->second;
        if (reverse_map.find (l) == reverse_map.end())
        {
            reverse_map [l] = std::vector <Group*> ();
            reverse_map [l].push_back (it->first);
        }
        else
        {
            reverse_map [l].push_back (it->first);
        }
        
        max_level = max (max_level, l);
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "reverse_map: " << std::endl;
        for (auto it = reverse_map.begin (); it != reverse_map.end (); it++)
        {
            std::cout << it->first << std::endl;
            for (auto& it2 : it->second)
            {
                std::cout << "    " <<getPyGroupName (it2->getPyGroup ()) << std::endl;
            }
        }
    }
    
    int time = 0;
    
    for (int l = 0; l <= max_level; l++)
    {
        for (auto obj = reverse_map [l].begin(); obj != reverse_map [l].end(); obj++)
        {
            naive_order [*obj] = time;
            time += 1;
        }
        
        if (l != max_level)
        {
            std::vector <Group*> next_level;
            
            for (auto obj = reverse_map [l].begin(); obj != reverse_map [l].end(); obj++)
            {
                vector <Group*> obj_kids;
                
                for (auto kid = reverse_map [l+1].begin(); kid != reverse_map [l+1].end(); kid++)
                {
                    if (new_nextGroups[(*obj)].find (*kid) != new_nextGroups[(*obj)].end ())
                        obj_kids.push_back (*kid);
                    
                    if (std::find (next_level.begin(), next_level.end(), *kid) == 
                        next_level.end ())
                    {
                        next_level.push_back (*kid);
                    }
                }
            }
            
            for (auto obj = reverse_map [l+1].begin(); obj != reverse_map[l+1].end(); obj++)
            {
                if (std::find (next_level.begin(), next_level.end(), *obj) == 
                    next_level.end ())
                {
                    next_level.push_back (*obj);
                }
            }
            
            reverse_map [l+1] = next_level;
        }
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout<< "optnaiveorder is"<<std::endl;
        for (auto it = naive_order.begin(); it != naive_order.end(); it++)
        {
            PyObject* func = PyObject_GetAttr (it->first->getCompAtIndex(0), str_func);
            PyObject* name = PyObject_GetAttr (func,
                                               Py_BuildValue ("s", "name"));
            PyObject* pyStr = PyUnicode_AsEncodedString(name, "utf-8", "Error ~");
            const char* ss = PyBytes_AS_STRING (pyStr);
            std::cout<< "     "<< ss<< "  " << it->second << std::endl;
        }
    }
}

void classify_storage (const vector <PyObject*>& vec_comps, 
                       unordered_map <PyObject*, PyObject*>& comp_to_stg_class,
                       unordered_map <PyObject*, vector <PyObject*>>& new_storage_class_map)
{
    //find_storage_equivalence
    unordered_map <string, vector <PyObject*>> storage_class_map;
    
    for (auto it = vec_comps.begin(); it != vec_comps.end(); it++)
    {
        string _strkey = stg_class_to_key [comp_to_orig_storage [*it]];
        if (storage_class_map.find (_strkey) == storage_class_map.end ())
        {
            storage_class_map [_strkey] = std::vector <PyObject*> ();
            storage_class_map [_strkey].push_back (*it);
        }
        else
        {
            storage_class_map [_strkey].push_back (*it);
        }
    }

    PRINT_DEBUG_BLOCK_L1
    {
        std::cout<< "storage_class_map "<< std::endl;
        for (auto it = storage_class_map.begin(); it != storage_class_map.end (); it++)
        {
            std::cout<< "    "<<it->first << " " << it->second.size () << std::endl;
        }
    }
    
    //maximal_storage
    
    for (auto it = storage_class_map.begin(); it != storage_class_map.end (); it++)
    {
        string key = it->first;
        vector <PyObject*>& class_comps = it->second;
        PyObject* helper_comp = class_comps [0];
        PyObject* func = comp_to_func [helper_comp];
        //check_and_print_exception ();
        PyObject* typ = func_to_typ [func];
        //check_and_print_exception ();
        long dims = func_to_ndims [func];
        //check_and_print_exception ();
        PyObject* helper_storage = comp_to_orig_storage[helper_comp];
        //check_and_print_exception ();
        vector <long> max_offset = comp_to_max_offsets [helper_comp];
        
        for (auto it2 = class_comps.begin(); it2 != class_comps.end(); it2++)
        {
            //PyObject* storage = comp_to_orig_storage[*it2];
            //check_and_print_exception ();
            //PyObject* offsets = PyObject_GetAttr (storage, str_offsets);
            //check_and_print_exception ();
            vector<long>& offset = comp_to_max_offsets [*it2];
            
            for (long i = 0; i < dims; i++)
            {
                long dim_off;
                
                dim_off = offset[i];
                //check_and_print_exception ();
                max_offset [i] = max (max_offset[i], dim_off);
            }
        }
        
        vector <PyObject*> vec_dim_sizes; //vector of tuples
        
        for (long dim = 0; dim < dims; dim++)
        {
            static PyObject* str_orig_param = Py_BuildValue ("s", "orig_param");
            PyObject* dim_storage;
            PyObject* new_size;
            PyObject* args;
            PyObject* max_offset_dim;
            
            max_offset_dim = PyLong_FromLong (max_offset [dim]);
            dim_storage = helper_storage_to_dim_storage [helper_storage][dim];
            args = PyTuple_Pack (2, dim_storage, max_offset_dim);
            new_size = PyObject_CallObject (storage_mapping_get_dim_size, 
                                            args);
            Py_DECREF (args);
            Py_DECREF (max_offset_dim);
            
            vec_dim_sizes.push_back (PyTuple_Pack (2,
                                     PyObject_GetAttr (dim_storage, str_orig_param),
                                     new_size));
        }
        
        PyObject* dim_sizes;
        
        dim_sizes = PyList_New (vec_dim_sizes.size ());
    
        for (auto it2 = vec_dim_sizes.begin(); 
             it2 != vec_dim_sizes.end(); it2++)
        {
            PyList_SetItem (dim_sizes, (it2 - vec_dim_sizes.begin()), *it2);
        }
        
        PyObject* max_storage;
        PyObject* py_dims;
        
        py_dims = PyLong_FromLong (dims);
        PyObject* args = PyTuple_Pack (3, typ, py_dims, dim_sizes);
        max_storage = PyObject_CallObject (cls_Storage, args);
        Py_DECREF (args);
        Py_DECREF (py_dims);
        new_storage_class_map [max_storage] = vector <PyObject*> ();
        
        for (auto it2 = class_comps.begin(); it2 != class_comps.end(); it2++)
        {
            comp_to_stg_class [*it2] = max_storage;
            new_storage_class_map [max_storage].push_back (*it2);
        }
        
        Py_DECREF (dim_sizes);
        
        for (auto it2 = vec_dim_sizes.begin(); 
             it2 != vec_dim_sizes.end (); it2++)
        {
            PyObject* tuple = *it2;
            PyObject* tuple1 = PyTuple_GetItem (tuple, 1);
            Py_DECREF (tuple);
            Py_DECREF (tuple1);
        }
    }
}

void getLivenessMap (const uint128_t hash_id, const vector<Group*>& vec_groups, 
                  unordered_map <Group*, int>& schedule, 
                  unordered_map <int, vector <PyObject*> >& liveness_map,
                  InlinedAdjacentType& new_nextGroups)
{
    for (auto it = schedule.begin(); it != schedule.end(); it++)
    {
        int last_live = -1;
        
        for (auto &child : new_nextGroups[it->first])
        {
            if ((child->hashID() & hash_id) == child->hashID ())
            {
                last_live = max (last_live, schedule[child]);
            }
        }
        
        if (liveness_map.find (last_live) == liveness_map.end ())
            liveness_map [last_live] = std::vector <PyObject*> ();
        
        liveness_map [last_live].push_back (it->first->getCompAtIndex(0));
    }
}

/**
 * Returns sum of size of all liveouts for given group. Also sets n_liveouts to
 * the number of liveouts.
 * hashID: Hash ID of group
 * vec_groups: Vector of Group's Children
 * n_liveouts: Number of liveouts.
 * */

uint64_t getLiveoutsSize (uint128_t hash_id, vector <Group*> vec_groups, 
                          int& n_liveouts)
{
    uint64_t liveouts_size = 0;
    n_liveouts = 0;
    
    for (auto const& group: vec_groups)
    {
        for (auto const& next_group: group->nextGroups ())
        {
            if ((next_group->hashID () & hash_id) != next_group->hashID ())
            {
                PyObject* pygroup = next_group->getPyGroup ();
                uint64_t stg_size = 1;
                for (int i = 0; i < get_n_dimensions (pygroup); i++)
                {
                    //TODO: Optimize it because for every get_dim_reuse_for_pygroup call
                    //dict and list are accessed again. Make them access only once 
                    //and then iterate over list
                    stg_size *= get_dim_size_for_pygroup (pygroup, i);
                }
                
                liveouts_size += stg_size*IMAGE_ELEMENT_SIZE;
                n_liveouts++;
                break;
            }
        }
        
        if (group->nextGroups().size () == 0)
        {
            PyObject* pygroup = group->getPyGroup ();
            uint64_t stg_size = 1;
            
            for (int i = 0; i < get_n_dimensions (pygroup); i++)
            {
                //TODO: Optimize it because for every get_dim_reuse_for_pygroup call
                //dict and list are accessed again. Make them access only once 
                //and then iterate over list
                stg_size *= get_dim_size_for_pygroup (pygroup, i);
            }
                
            liveouts_size += stg_size*IMAGE_ELEMENT_SIZE;
            n_liveouts++;
        }
    }
    
    return liveouts_size;
}

/**
 * Returns Total Size used by the group i.e. sum of size of all intermediate 
 * buffers + sum of size of all liveouts
 * hash_id: HashID of Group
 * number_of_buffers: set to the number of intermediate buffers
 * liveouts_size: set to the size of live outs.
 * livein_size: set to the size of all liveins
 * inlined_groups: Groups that has been inlined
 * new_nextGroups: New next Groups after inlining
 * new_prevGroups: New prev Groups after inlining
 * */
uint64_t getTotalSizeUsed (uint128_t hash_id, int& number_of_buffers,
                           uint64_t& liveouts_size, uint64_t& livein_size,
                           std::unordered_set<Group*, Group::GroupHasher>&
                           inlined_groups,
                           InlinedAdjacentType& new_nextGroups,
                           InlinedAdjacentType& new_prevGroups)
{
    //TODO: Support TStencil 
    map <Group*, int, Group::GroupComparer> level_order;
    unordered_map <Group*, int> naive_order; //naive_order is schedule
    unordered_map <PyObject*, PyObject*> cmp_to_stg_class;
    unordered_map <PyObject*, vector <PyObject*>> storage_class_map;
    std::map <int, PyObject*> sorted_comps;
    unordered_map <PyObject*, vector<uint64_t> > array_pool; //Pool from computation to the vector of integers available
    uint128_t _hash_id = hash_id;
    vector <PyObject*> vec_comps;
    vector <Group*> vec_groups;
    vector <Group*> live_out_groups;
    unordered_set <Group*, Group::GroupHasher> live_in_groups;
    unordered_map <PyObject*, uint64_t> storage_map; //storage_map to store first index of array for each computation
    unordered_map <int, vector <PyObject*> > liveness_map;
    static PyObject* str_func = Py_BuildValue ("s", "func");
    static PyObject* str_comps = Py_BuildValue ("s", "comps");
    uint64_t array_count = 0;
    int bit = 0;
    liveouts_size = 0;
    uint128_t nonLiveOutsHashID = 0;
    
    while (_hash_id != 0)
    {
        if ((_hash_id & 1L) == 1)
        {   
            uint128_t l = 1;
            Group* _g = Group::hashIDToGroup[l<<bit];
            PyObject* pyGroup = _g->getPyGroup ();
            
            if (!pyGroup)
            {
                _hash_id = _hash_id >> 1;
                bit++;
                continue;
            }
            
            //Include only those groups and comps which are not inlined
            if ((INLINING_ENABLED && inlined_groups.find (_g) == inlined_groups.end ()) ||
                !INLINING_ENABLED)
            {
                if (!OptGroup::isLiveoutForGroup (hash_id, (OptGroup*)_g))
                {
                    nonLiveOutsHashID |= (l << bit);
                    vec_groups.push_back (_g);
                    PyObject* comps = PyObject_GetAttr (pyGroup, str_comps);
                    
                    for (int j = 0; j < PyList_Size (comps); j++)
                    {
                        PyObject* comp = PyList_GetItem (comps, j);
    
                        vec_comps.push_back (comp);
                    }
                }
                else
                    live_out_groups.push_back (_g);
                
                //TODO: include only those liveins which are not inlined
                OptGroup::liveinsForChildInGroup (hash_id, (OptGroup*)_g, live_in_groups);
            }
        }
        
        _hash_id = _hash_id >> 1;
        bit++;
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "Final New Next Groups in getTotalSizeUsed" << std::endl;
        for (auto const &it : new_nextGroups)
        {
            std::cout << getPyGroupName (it.first->getPyGroup ()) << " -> ";
            
            for (auto const &g : it.second)
            {
                std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
            }
            
            std::cout << std::endl;
        }
        
        std::cout << "Final New Prev Groups in getTotalSizeUsed " << std::endl;
        for (auto const &it : new_prevGroups)
        {
            std::cout << getPyGroupName (it.first->getPyGroup ()) << " <- ";
            
            for (auto const &g : it.second)
            {
                std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
            }
            
            std::cout << std::endl;
        }
    }
    
    get_level_order (nonLiveOutsHashID, vec_comps, vec_groups, level_order, 
                     inlined_groups, new_nextGroups, new_prevGroups);
    naive_sched_objs (level_order, naive_order, new_nextGroups, new_prevGroups);
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout<<"optnaiveorder12121212: " << std::endl;
        for (auto it = naive_order.begin (); it != naive_order.end (); it++)
        {
            std::cout << it->second << " " << getPyCompFuncName (it->first->getCompAtIndex (0)) << std::endl;
        }
    }
    
    getLivenessMap (nonLiveOutsHashID, vec_groups, naive_order, liveness_map, 
                    new_nextGroups);
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout<<"optnaiveorderqqqqqqqqqq1212: " << std::endl;
        for (auto it = naive_order.begin (); it != naive_order.end (); it++)
        {
            std::cout << it->second << " " << getPyCompFuncName (it->first->getCompAtIndex (0)) << std::endl;
        }
    }
    
    classify_storage (vec_comps, cmp_to_stg_class, storage_class_map);
    liveouts_size = getLiveoutsSize (hash_id, live_out_groups, number_of_buffers);
    
    /*Get LiveIn Size*/
    for (auto const& __g: live_in_groups)
    {
        {
            PyObject* pygroup = __g->getPyGroup ();
            if (!pygroup)
                continue;
            uint64_t stg_size = 1;
            for (int i = 0; i < get_n_dimensions (pygroup); i++)
            {
                //TODO: Optimize it because for every get_dim_reuse_for_pygroup call
                //dict and list are accessed again. Make them access only once 
                //and then iterate over list
                stg_size *= get_dim_size_for_pygroup (pygroup, i);
            }
            
            livein_size += stg_size*IMAGE_ELEMENT_SIZE;
            break;
        }
    }
        
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "liveout buffers "<< number_of_buffers << std::endl;
    
        std::cout<< "cmp_to_stg_class is"<<std::endl;
        for (auto it = vec_comps.begin(); it != vec_comps.end(); it++)
        {
            PyObject* func = PyObject_GetAttr (*it, str_func);
            PyObject* name = PyObject_GetAttr (func,
                                               Py_BuildValue ("s", "name"));
            PyObject* pyStr = PyUnicode_AsEncodedString(name, "utf-8", "Error ~");
            const char* ss = PyBytes_AS_STRING (pyStr);
            std::cout<< "     "<< ss<< (cmp_to_stg_class [*it]) << std::endl;
        }
    }
    
    for (auto it = vec_comps.begin(); it != vec_comps.end(); it++)
    {
        array_pool [cmp_to_stg_class [*it]] = vector <uint64_t> ();
    }
    
    for (auto it = naive_order.begin(); it != naive_order.end(); it++)
    {
        sorted_comps[it->second] = it->first->getCompAtIndex(0);
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout<<"sorted comps " << std::endl;
        for (auto it = sorted_comps.begin (); it != sorted_comps.end (); it++)
        {
            std::cout << it->first << " " << getPyCompFuncName (it->second) << std::endl;
        }
        
        std::cout<<"optnaiveorder: " << std::endl;
        for (auto it = naive_order.begin (); it != naive_order.end (); it++)
        {
            std::cout << it->second << " " << getPyCompFuncName (it->first->getCompAtIndex (0)) << std::endl;
        }
    }
    
    unordered_map <PyObject*, int> n_stg_class_arrays; //Number of arrays for each storage class
    
    for (auto it = sorted_comps.begin(); it != sorted_comps.end(); it++)
    {
        PyObject* comp = it->second;
        PyObject* storage_class = cmp_to_stg_class [comp];
        PyObject* func = comp_to_func [comp];
        
        uint64_t num_reqd, num_available, num_allocated, deficit;
        bool is_tstencil_grp = false;
        vector <uint64_t> allocated_arrays;
        int time;
        
        if (PyObject_IsInstance (func, tstencil_cls))
        {
            is_tstencil_grp = true;
        }
        
        if (is_tstencil_grp == true)
            num_reqd = 2;
        else
            num_reqd = 1;
        
        PRINT_DEBUG_BLOCK_L1
        {
            std::cout << "comp is " << getPyCompFuncName (comp) << " sched is " << it->first << std::endl;
            std::cout<< "num_reqd " << num_reqd << std::endl;
        }
        num_available = array_pool [storage_class].size ();
        PRINT_DEBUG_BLOCK_L1
            std::cout<< "num available " << num_available <<std::endl;
        num_allocated = min (num_available, num_reqd);
        
        for (uint64_t i = 0; i < num_allocated; i++)
        {
            allocated_arrays.push_back (array_pool[storage_class].back ());
            array_pool[storage_class].pop_back ();
        }
        
        deficit = num_reqd - num_allocated;
        
        if (deficit > 0)
        {
            for (uint64_t i = 0; i < deficit; i++)
            {
                allocated_arrays.push_back (array_count + 1 + i);
            }
            
            array_count += deficit;
            n_stg_class_arrays [storage_class] += deficit;
        }
        
        storage_map [comp] = allocated_arrays[0];
        time = it->first;
        
        if (liveness_map.find (time) != liveness_map.end ())
        {
            vector <PyObject*> free_comps;
            free_comps = liveness_map [time];
            PyObject* cmp_stg_class;
            
            for (auto it2 = free_comps.begin(); it2 != free_comps.end(); it2++)
            {
                cmp_stg_class = cmp_to_stg_class[*it2];
                array_pool [cmp_stg_class].push_back (storage_map [*it2]);
            }
        }
    }

    PRINT_DEBUG_BLOCK_L1
    {
        std::cout<< "optarray pool" << std::endl;
        for (auto it = array_pool.begin(); it != array_pool.end(); it++)
        {
            PyObject* name = PyObject_Str (it->first);
            PyObject* pyStr = PyUnicode_AsEncodedString(name, "utf-8", "Error ~");
            const char* ss = PyBytes_AS_STRING (pyStr);
            std::cout<<ss<<std::endl;
            
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
            {
                std::cout<< "   " << *it2 << std::endl;
            }
        }
    }
    
    PRINT_DEBUG_BLOCK_L1
            std::cout<< "Array count " << array_count << " " << std::bitset<41>((uint64_t) hash_id) << std::endl;
    uint64_t total_size = 0;
    PRINT_DEBUG_BLOCK_L1
            std::cout<< "total size " << n_stg_class_arrays.size () << std::endl;
    for (auto it = n_stg_class_arrays.begin(); 
         it != n_stg_class_arrays.end (); it++)
    {
        PyObject* stg_class = it->first;
        int n_arrays = it->second;
        PRINT_DEBUG_BLOCK_L1
            std::cout << "n_arrays " << n_arrays << std::endl;
        PyObject* pygroup = pyGroupForComp [storage_class_map [stg_class][0]];
        PRINT_DEBUG_BLOCK_L1
            std::cout << "pygroup " << pygroup << std::endl;
        uint64_t stg_size = 1;
        PRINT_DEBUG_BLOCK_L1
            std::cout << "dim size : ";
        for (int i = 0; i < get_n_dimensions (pygroup); i++)
        {
            //TODO: Optimize it because for every get_dim_reuse_for_pygroup call
            //dict and list are accessed again. Make them access only once 
            //and then iterate over list
            uint64_t __size = get_dim_size_for_pygroup (pygroup, i);
            PRINT_DEBUG_BLOCK_L1
                std::cout << "(dim: " << i << ", size: " << __size << ")" << std::endl;
            stg_size *= __size;
        }
        PRINT_DEBUG_BLOCK_L1
            std::cout << std::endl;
        total_size += stg_size*n_arrays*IMAGE_ELEMENT_SIZE;
    }
    
    unordered_set <PyObject*> stg_class_removed;
    
    for (auto it = storage_class_map.begin(); it != storage_class_map.end(); it++)
    {
        Py_DECREF (it->first);
    }
    
    number_of_buffers += array_count;
    
    return total_size + liveouts_size;
}

/**
 * Returns the standard deviation of dim sizes of all children of group.
 * dim_size_diff: vector of dimension sizes of each child
 * max_dim: maximum number of dims of all children of a group.
 * */
inline int64_t dim_size_std_dev (std::vector <std::vector <uint64_t> >& dim_size_diff, int max_dim)
{
    std::vector <double> sum, mean; //vectors for sum and mean in each dimension
    sum = std::vector <double> (max_dim, 0);
    mean = std::vector <double> (max_dim, 0);
    
    for (auto it_comp = dim_size_diff.begin (); it_comp != dim_size_diff.end (); 
         it_comp++)
    {
        for (uint64_t dim = 0; dim < (*it_comp).size (); dim++)
        {
            sum[dim] += (*it_comp)[dim];
        }
    }
    
    for (uint64_t i = 0; i < sum.size (); i++)
    {
        mean[i] = sum[i]/dim_size_diff.size (); //sum [i]->sum of dim sizes in each dimension
        //dim_size_diff.size () -> number of computations
    }
    
    //Calculate mean difference in each dimension
    std::vector <double> mean_diff = std::vector <double> (max_dim, 0);
    
    for (auto it_comp = dim_size_diff.begin (); it_comp != dim_size_diff.end (); 
         it_comp++)
    {
        for (uint64_t dim = 0; dim < (*it_comp).size (); dim++)
        {
            mean_diff [dim] += abs ((*it_comp)[dim]- mean[dim])/dim_size_diff.size ();
        }
    }
    
    double sum_mean_dim = std::accumulate (mean.begin(), mean.end (), 0.0);
    double sum_mean_diff = std::accumulate (mean_diff.begin(), mean_diff.end (), 0.0);
    PRINT_DEBUG_BLOCK_L1
        std::cout <<"mean sum" << std::accumulate (mean.begin(), mean.end (), 0.0) << " mean_diff sum " <<std::accumulate (mean_diff.begin(), mean_diff.end (), 0.0) <<std::endl;
    
    if (sum_mean_diff/sum_mean_dim > 0.1f)
        return sum_mean_diff;
    else
        return 0;
}

/**
 * Return number of edges in a group.
 * 
 * */
int getNumberOfEdges (uint128_t _hash_id)
{
    vector <Group*> vec_groups;
    int n_edges  = 0;
    int bit = 0;
    
    while (_hash_id != 0)
    {
        if ((_hash_id & 1L) == 1)
        {   
            uint128_t l = 1;
            Group* _g = Group::hashIDToGroup[l<<bit];
            PyObject* pyGroup = _g->getPyGroup ();
            
            if (!pyGroup)
            {
                _hash_id = _hash_id >> 1;
                bit++;
                continue;
            }
            
            for (auto const &next_group : _g->nextGroups ())
            {
                if ((next_group->hashID () & _hash_id) == next_group->hashID ())
                    n_edges++;
            }
        }
        
        _hash_id = _hash_id >> 1;
        bit++;
    }
    
    return n_edges;
}

void update_inlining_next_prev_vertices (InlinedAdjacentType& new_nextGroups,
                                         InlinedAdjacentType& new_prevGroups,
                                         Group* inlined_group)
{
    for (auto const &next: new_nextGroups [inlined_group])
    {
        auto& next_prev = new_prevGroups [next];
        
        next_prev.erase (inlined_group);
        
        for (auto const &prev: new_prevGroups [inlined_group])
        {
            next_prev.insert (prev);
        }
    }
    
    for (auto const &prev: new_prevGroups [inlined_group])
    {
        auto& prev_next = new_nextGroups [prev];
        
        prev_next.erase (inlined_group);
        
        for (auto const &next: new_nextGroups [inlined_group])
        {
            prev_next.insert (next);
        }
    }
    
    new_nextGroups.erase (inlined_group);
    new_prevGroups.erase (inlined_group);
}
                                 
/*Finds the groups to be inlined*/
void update_graph_with_inlining (std::unordered_set<Group*, Group::GroupHasher>& 
                                 inlined_groups, 
                                 InlinedAdjacentType& new_nextGroups,
                                 InlinedAdjacentType& new_prevGroups,
                                 uint128_t hash_id)
{
    static PyObject* str_is_pointwise = Py_BuildValue ("s", "is_pointwise");
    static PyObject* str_inline_all_children = Py_BuildValue ("s", 
                                                              "inline_in_all_children");
    std::vector <Group*> groups;
    std::unordered_set<Group*> pointwise_groups, inline_all_children_group;
    std::unordered_set<Group*, Group::GroupHasher> liveouts;
    uint128_t _hash_id = hash_id;
    int bit = 0;
    
    while (_hash_id != 0)
    {
        if ((_hash_id & 1L) == 1)
        {
            uint128_t l = 1L;
            Group* group = Group::hashIDToGroup[l<<bit];
            if (group->getPyGroup () != nullptr)
            {
                groups.push_back (group);
                PyObject* pointwise_func = PyObject_GetAttr (group->getPyGroup(), str_is_pointwise);
                PyObject* is_pointwise = PyObject_CallObject (pointwise_func, NULL);
                //TODO: Get all pointwise functions as argument to dpgroup function
                //to increase the speed a little bit
                if (is_pointwise == Py_True)
                {
                    pointwise_groups.insert (group);
                }
                else
                {
                    PyObject* inline_all_func = PyObject_GetAttr (group->getPyGroup(), 
                                                                 str_inline_all_children);
                    PyObject* inline_all = PyObject_CallObject (inline_all_func,
                                                                NULL);
                    
                    if (inline_all == Py_True)
                    {
                        inline_all_children_group.insert (group);
                    }
                }
            }
        }
        bit++;
        _hash_id = _hash_id >> 1;
    }
    
    //Dummy Group
    if (groups.size() == 0)
    {
        return;
    }
    
    if (groups.size () == 1)
    {
        //Single Group, don't need to do anything
        return;
    }
    
    if (pointwise_groups.size () == 0)
    {
        //No Pointwise groups means no inlining
        return;
    }
    
    //Start from the liveouts. Traverse from liveouts to the liveins in a BFS 
    //fashion not a DFS and inline all the pointwise functions found, until 
    //there is no spilling, current producer is not a liveout, and the 
    //current producer has only one consumer because inlining a producer twice
    //or more will lead to redundant computations.
    //Moreover, inline the producers of pointwise functions if they
    //are not liveouts and don't have two consumers, until there is
    //no spilling. Also, inlining of producer into consumer is only done
    //when the sizes of each dimensions are same.
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "Number of Pointwise groups " << pointwise_groups.size () << std::endl;
        std::cout << "Pointwise groups are: "<<std::endl;
        for (auto const &it : pointwise_groups)
        {
            std::cout << getPyGroupName (it->getPyGroup()) << ", ";
        }
        
        std::cout << std::endl;
        
        std::cout << "Number of inline_all_children_group groups  " << inline_all_children_group.size () << std::endl;
        std::cout << "inline_all_children_group groups are: "<<std::endl;
        for (auto const &it : inline_all_children_group)
        {
            std::cout << getPyGroupName (it->getPyGroup()) << ", ";
        }
        
        std::cout << std::endl;
    }
    
    OptGroup::liveoutsForGroup (hash_id, groups, liveouts);
    PRINT_DEBUG_BLOCK_L1
        std::cout << "liveouts for group "<<liveouts.size () << std::endl;
        
    std::queue<Group*> bfs_queue;
    
    //Do BFS and find all pointwise_groups in reverse
    for (auto const &liveout : liveouts)
    {
        bfs_queue.push (liveout);
        
        while (bfs_queue.size () != 0)
        {
            Group* _front = bfs_queue.front ();
            bfs_queue.pop();
            // Inline those functions which:
            // (i) are pointwise, not liveouts, and have only one consumer.
            // (ii) are pointwise, have more than 1 consumer, but contains only 
            //      one reference
            bool _to_inline = false;
            if (pointwise_groups.count (_front) > 0 &&
                liveouts.count (_front) == 0 && 
                _front->nextGroups ().size() == 1 &&
                (_front->hashID () & hash_id) == _front->hashID ())
            {
                _to_inline = true;
            }
            
            if (!_to_inline && pointwise_groups.count (_front) > 0 &&
                liveouts.count (_front) == 0 && _front->nextGroups ().size() > 0 &&
                (_front->hashID () & hash_id) == _front->hashID () && 
                inline_all_children_group.count (_front) > 0)
            {
                _to_inline = true;
            }
            
            if (_to_inline)
            {
                inlined_groups.insert (_front);
                update_inlining_next_prev_vertices (new_nextGroups, 
                                                    new_prevGroups, _front);
            }
            
            for (auto const &prev : _front->prevGroups ())
            {
                //Prev Group should be in this group
                if ((hash_id & prev->hashID ()) == prev->hashID())
                    bfs_queue.push (prev);
            }
        }
    }
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "Inlined Pointwise and non liveout Groups found: " << inlined_groups.size () << std::endl;
        for (auto const &g : inlined_groups)
        {
            std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
        }
        
        std::cout << std::endl;
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "New Next Groups " << std::endl;
        for (auto const &it : new_nextGroups)
        {
            std::cout << getPyGroupName (it.first->getPyGroup ()) << " -> ";
            
            for (auto const &g : it.second)
            {
                std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
            }
            
            std::cout << std::endl;
        }
        
        std::cout << "New Prev Groups " << std::endl;
        for (auto const &it : new_prevGroups)
        {
            std::cout << getPyGroupName (it.first->getPyGroup ()) << " <- ";
            
            for (auto const &g : it.second)
            {
                std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
            }
            
            std::cout << std::endl;
        }
    }
    
    //At this point we have inlined all the pointwise functions (and non-liveouts, i.e.,
    //intermediate functions) into their respective children.
    //In the last stage, again find all the pointwise functions (but also taking 
    //into account liveouts) and then inline the producer of these pointwise
    //functions only if they have only one consumer.
    
    //TODO: an optimization here can be to consider only pointwise liveouts
    //and not the whole DAG, since, all pointwise functions before has
    //been inlined.
    //Do BFS and find all pointwise_groups in reverse using updated 
    //New and Prev Groups Set
    std::unordered_set<Group*, Group::GroupHasher> new_inlined_groups;
    for (auto const &liveout : liveouts)
    {
        bfs_queue.push (liveout);
        
        while (bfs_queue.size () != 0)
        {
            Group* _front = bfs_queue.front ();
            bfs_queue.pop();
            // Inline the producer functions which 
            // (i) are producer functions for pointwise and has only one consumer
            // (ii) are not pointwise and contains several pointwise children but
            //     contains Case statement in body for each children, like
            //     deinterleaved in Camera Pipe (in other words, consumer should
            //     be in inline_all_children_group)
            if (pointwise_groups.find (_front) != pointwise_groups.end())
            {
                PRINT_DEBUG_BLOCK_L1
                    std::cout << "pointwise group is " << getPyGroupName(_front->getPyGroup ()) << std::endl;
                    
                for (auto const &it : new_prevGroups[_front])
                {
                    bool inline_producer = false;

                    if (new_nextGroups[it].size () == 1 &&
                        (it->hashID () & hash_id) == it->hashID () &&
                        liveouts.count (it) == 0)
                    {
                        inline_producer = true;
                        
                    }
                    
                    if (new_nextGroups[it].size () > 1 && 
                        inline_all_children_group.count (it) == 1 &&
                        (it->hashID () & hash_id) == it->hashID () &&
                        liveouts.count (it) == 0)
                    {
                        inline_producer = true;
                    }
                    
                    if (inline_producer)
                    {
                        inlined_groups.insert (it);
                        new_inlined_groups.insert (it);
                    }
                }
            }
            
            for (auto const &prev : new_prevGroups[_front])
            {
                //Prev Group should be in this group
                if ((hash_id & prev->hashID ()) == prev->hashID())
                    bfs_queue.push (prev);
            }
        }
        
    }
    
    //Update New Next and Prev Groups set
    
    for (auto const &it : new_inlined_groups)
    {
        update_inlining_next_prev_vertices (new_nextGroups, new_prevGroups, it);
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "Final Inlined Groups: " << inlined_groups.size () << std::endl;
        for (auto const &g : inlined_groups)
        {
            std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
        }
        
        std::cout << std::endl;
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "Final New Next Groups " << std::endl;
        for (auto const &it : new_nextGroups)
        {
            std::cout << getPyGroupName (it.first->getPyGroup ()) << " -> ";
            
            for (auto const &g : it.second)
            {
                std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
            }
            
            std::cout << std::endl;
        }
        
        std::cout << "Final New Prev Groups " << std::endl;
        for (auto const &it : new_prevGroups)
        {
            std::cout << getPyGroupName (it.first->getPyGroup ()) << " <- ";
            
            for (auto const &g : it.second)
            {
                std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
            }
            
            std::cout << std::endl;
        }
    }
}

inline uint64_t cost (uint128_t hash_id, std::vector <uint64_t>& tile_sizes)
{
    uint64_t bit = 0;
    uint64_t nOnes = 0;
    int total_comps = 0;
    bool is_reduction = false;
    bool is_small_grp = false;
    bool is_tstencil_grp = false;
    bool is_const_grp = false;
    bool is_dummy_source_group = false;
    uint128_t _hash_id = hash_id;
    static PyObject* str_comps = Py_BuildValue ("s", "comps");
    static PyObject* str_func = Py_BuildValue ("s", "func");
    static PyObject* str_is_const_func = Py_BuildValue ("s", "is_const_func");
    PyObject* list_groups_for_overlap = PyList_New (0);
    int max_dim = get_max_dimensions_for_group (hash_id);
    int max_dims = max_dim;
    std::vector <uint64_t> dim_reuse = std::vector <uint64_t> (max_dims, 0);
    std::vector <uint64_t> dim_size = std::vector <uint64_t> (max_dims, 0);
    int n_threads = 0;
    int _cost;
    int n_buffers = 0;
    bool all_n_dims_equal = true;
    int last_group_dim = -1;
    uint64_t liveouts_size = 0;
    uint64_t liveins_size = 0;
    std::vector <std::vector <uint64_t> > dim_size_diff; // 2-D vector with rows as number of comps and cols as max_dim
    uint64_t tile_size = 0;
    uint64_t totalsizeused = 0;
    std::unordered_set<Group*, Group::GroupHasher> inlined_groups;
    InlinedAdjacentType new_nextGroups;
    InlinedAdjacentType new_prevGroups;
    
    PRINT_DEBUG_BLOCK_L1
        std::cout << "Cost for hash_id " << std::bitset <41> ((uint64_t)hash_id) << std::endl;
    
    while (_hash_id != 0)
    {
        if ((_hash_id & 1L) == 1)
        {
            uint128_t l = 1;
            Group* group;
            
            group =  Group::hashIDToGroup[l<<bit];
            PyObject* pyGroup = group->getPyGroup ();
            nOnes++;
            if (pyGroup != NULL)
            {
                PyObject* comps = PyObject_GetAttr (pyGroup, str_comps);
                
                total_comps += PyList_Size (comps);
                
                for (int j = 0; j < PyList_Size (comps); j++)
                {
                    PyObject* comp = PyList_GetItem (comps, j);
                    PyObject* func = PyObject_GetAttr (comp, str_func);
                    if (PyObject_IsInstance (func,
                                             reduction_cls))
                    {
                        is_reduction = true;
                        
                    }
                    
                    if (PyObject_IsInstance (func,
                                             tstencil_cls))
                    {
                        is_tstencil_grp = true;
                    }
                       
                    PyObject* is_const_func = PyObject_GetAttr (func, str_is_const_func);
                    if (is_const_func == Py_True)
                    {
                        is_const_grp = true;
                    }
                       
                    if (small_comps_set.find (comp) != small_comps_set.end())
                    {
                        is_small_grp = true;
                    }
                }
                
                dim_size_diff.push_back (std::vector <uint64_t> (max_dim, 0));
                
                for (int i = 0; i < get_n_dimensions (pyGroup); i++)
                {
                    //TODO: Optimize it because for every get_dim_reuse_for_pygroup call
                    //dict and list are accessed again. Make them access only once 
                    //and then iterate over list
                    //PyObject* comp = PyList_GetItem (comps, 0);
                    //PyObject* func = PyObject_GetAttr (comp, str_func);
                    dim_reuse [i] += get_dim_reuse_for_pygroup (pyGroup, i)*IMAGE_ELEMENT_SIZE;
                    uint64_t _dim_size = get_dim_size_for_pygroup (pyGroup, i)*IMAGE_ELEMENT_SIZE;
                    dim_size [i] += _dim_size;
                    dim_size_diff [dim_size_diff.size () -1][i] = _dim_size;
                }
                
                if (all_n_dims_equal == true)
                {
                    if (last_group_dim != -1 and last_group_dim != get_n_dimensions (pyGroup))
                    {
                        all_n_dims_equal = false;
                    }
                    
                    last_group_dim = get_n_dimensions (pyGroup);
                }
                
                PyList_Append (list_groups_for_overlap, pyGroup);
                
                new_nextGroups[group] = std::set <Group*, Group::GroupComparer> (group->nextGroups ());
                new_prevGroups[group] = std::set <Group*, Group::GroupComparer> (group->prevGroups ());
            }
            else
            {
                is_dummy_source_group = true;
            }
        }
        
        _hash_id = _hash_id >> 1;
        bit++;
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "Final New Next Groups in COST" << std::endl;
        for (auto const &it : new_nextGroups)
        {
            std::cout << getPyGroupName (it.first->getPyGroup ()) << " -> ";
            
            for (auto const &g : it.second)
            {
                std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
            }
            
            std::cout << std::endl;
        }
        
        std::cout << "Final New Prev Groups in COST" << std::endl;
        for (auto const &it : new_prevGroups)
        {
            std::cout << getPyGroupName (it.first->getPyGroup ()) << " <- ";
            
            for (auto const &g : it.second)
            {
                std::cout << getPyGroupName (g->getPyGroup ()) << ", ";
            }
            
            std::cout << std::endl;
        }
    }
    
    if (!printDPFusionTimeAndChoices)
    {
        if (INLINING_ENABLED)
            update_graph_with_inlining (inlined_groups, new_nextGroups, 
                                        new_prevGroups, hash_id);
        
        for (auto &it : new_nextGroups)
        {
            Group* g = it.first;
            
            for (auto &next : it.second)
            {
                if ((hash_id & g->hashID ()) == g->hashID () && 
                    (hash_id & next->hashID ()) == next->hashID () &&
                    get_n_dimensions (g->getPyGroup ()) < get_n_dimensions (next->getPyGroup ()))
                {
                    
                    return -1;
                }
            }
        }
        
        totalsizeused = getTotalSizeUsed (hash_id, n_buffers, 
                                           liveouts_size, liveins_size,
                                           inlined_groups, new_nextGroups, 
                                           new_prevGroups);
    }
    
    int64_t mean_dim_diff = dim_size_std_dev (dim_size_diff, max_dims);

    PRINT_DEBUG_BLOCK_L1
        std::cout << "n_buffers " << n_buffers << std::endl;
    PRINT_DEBUG_BLOCK_L1
        std::cout<< "totalsizeused " << totalsizeused << std::endl;
    
    if (!printDPFusionTimeAndChoices)
    {
        if (totalsizeused == 0 && n_buffers == 0)
        {
            //Dummy group
            return 0;
        }
        
        _hash_id = hash_id;
        
        if ((is_reduction || is_const_grp || is_small_grp || is_tstencil_grp || is_dummy_source_group) && nOnes == 1)
            return 1L;
        
        if (is_reduction || is_const_grp || is_small_grp || is_tstencil_grp || is_dummy_source_group)
            return -1;
    
        if (total_comps > grp_size)
            return -1;
        
        if (is_dummy_source_group)
            return -1;
            
        if (nOnes == 1)
            return INT_MAX;
    }
    
    if (printDPFusionTimeAndChoices)
    {
        if ((is_reduction || is_const_grp || is_small_grp || is_tstencil_grp) && nOnes == 1)
            return 1L;
        
        if (is_reduction)
            return -1;
            
        return 1L;
    }
    
    #define INCLUDE_OVERLAP
    #ifdef INCLUDE_OVERLAP
    PyObject* pytotalsizeused = PyLong_FromLong (totalsizeused);
    PyObject* pyn_buffers = PyLong_FromLong (n_buffers);
    PyObject* list_inline_comps = PyList_New (0);

    for (auto const &g : inlined_groups)
    {
        PyList_Append (list_inline_comps, g->getCompAtIndex (0));
    }
    
    PyObject* args = PyTuple_Pack (4, list_groups_for_overlap, list_inline_comps, pytotalsizeused, pyn_buffers);
    PyObject* tuple_return = PyObject_CallObject (get_overlapping_size_func, args);
    check_and_print_exception ();
    PyObject* overlap_obj = PyTuple_GetItem (tuple_return, 0);
    check_and_print_exception ();
    PyObject* pytile_size = PyTuple_GetItem (tuple_return, 1);
    check_and_print_exception ();
    int64_t overlap_size = PyLong_AsLong (overlap_obj)*IMAGE_ELEMENT_SIZE;
    tile_size = PyLong_AsLong(pytile_size)*IMAGE_ELEMENT_SIZE;
    if (tile_size == 0 && overlap_size == -4)
    {
        return -1;
    }
    
    if (tile_size == 0 && overlap_size == 0)
    {
        tile_size = totalsizeused/N_CORES;
        if (tile_size > L2_CACHE_SIZE)
            tile_size = L2_CACHE_SIZE;
        
        overlap_size = 0;
    }
    
    if (tile_size == 4 && overlap_size == (1L<<30)*IMAGE_ELEMENT_SIZE)
    {
        return -1;
    }
    
    PRINT_DEBUG_BLOCK_L1
        std::cout << "tile_size "<< tile_size << std::endl;
    assert (tile_size != 0);
    Py_DECREF (args);
    //Py_DECREF (tuple_return);
    Py_DECREF (list_inline_comps);
    Py_DECREF (pytotalsizeused);
    Py_DECREF (list_groups_for_overlap);
    Py_DECREF (pyn_buffers);
    #else
    int64_t overlap_size = 0;
    #endif
    
    PRINT_DEBUG_BLOCK_L1
        std::cout<< "totalsizeused " << totalsizeused << " tile_size for " << std::bitset<41>((uint64_t)_hash_id)<<" " << tile_size << " liveins_size " << liveins_size <<std::endl;
    
    n_threads = totalsizeused/(tile_size*n_buffers);
    PRINT_DEBUG_BLOCK_L1
        std::cout << "_cost mean_dim_diff " << mean_dim_diff << " liveins_size " << liveins_size << " liveouts_size " << liveouts_size << " live ratio " << (liveins_size+liveouts_size)/(tile_size*n_buffers) <<
            " threads ratio " << (((n_threads+N_CORES-1)%N_CORES)*100)/N_CORES << " all_n_dims_equal " << all_n_dims_equal << " overlap " << overlap_size << " tile_size " << tile_size << std::endl
            << " overlap cost " << (1000.0f*50.0f*1.5*overlap_size)/(tile_size*n_buffers) << " n_buffers " << n_buffers << std::endl;

    _cost = mean_dim_diff*DIM_STD_DEV_WEIGHT + 
            (LIVE_SIZE_TO_TILE_SIZE_WEIGHT*(liveins_size+liveouts_size))/(tile_size*n_buffers) - 
            (((n_threads+N_CORES-1)%N_CORES)*CLEANUP_THREADS_WEIGHT)/N_CORES + 
            (RELATIVE_OVERLAP_WEIGHT*overlap_size)/(tile_size*n_buffers);
    
    
    if (!all_n_dims_equal)
    {
        #ifdef __POLYMAGE_SERVER__
            _cost += 1200;
        #else
            #ifdef __MCASTLE1_SERVER__
                _cost += 500;
            #endif
        #endif
    }
    
    PRINT_DEBUG_BLOCK_L1
        std::cout << "total cost " << _cost << std::endl;
        
    
    if (total_comps <= grp_size)
    {
        return _cost;
    }
    
    return -1;
}

inline uint64_t cost2 (uint128_t hash_id, std::vector <uint64_t>& tile_sizes)
{
    uint64_t bit = 0;
    uint64_t nOnes = 0;
    long totalComps = 0;
    bool is_reduction = false;
    bool is_small_grp = true;
    bool is_tstencil_grp = false;
    bool is_const_grp = false;
//    uint128_t _hash_id = hash_id;
    static PyObject* str_comps = Py_BuildValue ("s", "comps");
    static PyObject* str_func = Py_BuildValue ("s", "func");
    static PyObject* str_children = Py_BuildValue ("s", "children");
    static PyObject* str_is_const_func = Py_BuildValue ("s", "is_const_func");

    while (hash_id != 0)
    {
        if ((hash_id & 1L) == 1)
        {   
            nOnes++;
            uint128_t l = 1;
            PyObject* pyGroup = Group::hashIDToGroup[l<<bit]->getPyGroup ();
            if (pyGroup != NULL)
            {    
                PyObject* list = PyObject_GetAttr (pyGroup, str_comps);
                
                totalComps += PyList_Size (list);
                
                PyObject* comps = PyObject_GetAttr (pyGroup, str_comps);
                        
                    for (int j = 0; j < PyList_Size (comps); j++)
                    {
                       PyObject* comp = PyList_GetItem (comps, j);
                       PyObject* func = PyObject_GetAttr (comp, str_func);
                       if (PyObject_IsInstance (func,
                                                reduction_cls))
                       {
                            is_reduction = true;
                       }
                       if (PyObject_IsInstance (func,
                                                tstencil_cls))
                       {
                            is_tstencil_grp = true;
                       }
                       
                       PyObject* is_const_func = PyObject_GetAttr (func, str_is_const_func);
                       if (is_const_func == Py_True)
                       {
                            is_const_grp = true;
                       }
                       
                       if (small_comps_set.find (comp) == small_comps_set.end())
                       {
                            is_small_grp = false;
                       }
                    }
                
                PyObject* children = PyObject_GetAttr (pyGroup, str_children);
                    
                for (int i = 0; i < PyList_Size (children); i++)
                {
                    PyObject* child = PyList_GetItem (children, i);
                    PyObject* comps = PyObject_GetAttr (child, str_comps);
                        
                    for (int j = 0; j < PyList_Size (comps); j++)
                    {
                       PyObject* comp = PyList_GetItem (comps, j);
                       PyObject* func = PyObject_GetAttr (comp, str_func);
                       if (PyObject_IsInstance (func,
                                                reduction_cls))
                       {
                            is_reduction = true;
                       }
                       if (PyObject_IsInstance (func,
                                                tstencil_cls))
                       {
                            is_tstencil_grp = true;
                       }
                       
                       PyObject* is_const_func = PyObject_GetAttr (func, str_is_const_func);
                       if (is_const_func == Py_True)
                       {
                            is_const_grp = true;
                       }                       
                    }
                }
            }
        }
        
        hash_id = hash_id >> 1;
        bit++;
    }
    
    if (totalComps <= grp_size && !is_reduction && !is_const_grp && !is_small_grp && !is_tstencil_grp){
        return 1L;}
    if (is_reduction && nOnes == 1) //Group contains only one group, which happens to be the reduction too
        return 1L;
    return -1;
}

inline bool reachable (uint128_t start, uint128_t end, bool isEndGroup)
{
    queue<uint128_t> q;
    q.push (start);
    
    while (!q.empty ())
    {
        vector<uint128_t>* n;
        uint128_t hash_id = q.front ();
        q.pop();
        
        if (isEndGroup)
        {
            if ((hash_id & end) == hash_id)
               return true;
        }
        else
            if (hash_id == end)
                return true;
        
        n = OptGroup::nextGroupsHashID (hash_id);
        
        for (auto it = n->begin(); it != n->end(); it++)
        {
            q.push (*it);
        }
        
        delete n;
    }

    return false;
}

inline bool isCycle (uint128_t hash_id, uint128_t g, vector<uint128_t>* next)
{
    for (auto it = next->begin (); it != next->end(); it++)
    {
        if (*it != g && reachable (*it, g, false))
            return true;
    }
    
    return false;
}

inline bool isCycle (uint128_t hash_id, uint128_t g)
{
    vector<uint128_t>* next = OptGroup::nextGroupsHashID(g);
    for (auto it = next->begin (); it != next->end(); it++)
    {
        if (reachable (*it, hash_id, true))
            return true;
    }
    
    return false;
}

void getAllNodesInNextPath (uint128_t node, uint128_t dest, 
                            unordered_set<uint128_t, uint128Hasher>& nodes_in_path,
                            vector<uint128_t>& path,
                            vector<uint128_t>* (*next_groups) (uint128_t))
{
    path.push_back (node);
    
    if (node == dest)
    {
        for (auto it = path.begin (); it != path.end (); it++)
        {
            nodes_in_path.insert (*it);
        }
    }
    else
    {
        vector<uint128_t>* n;
        
        n = next_groups (node);
        
        for (auto it = n->begin(); it != n->end(); it++)
        {
            if (nodes_in_path.find (*it) == 
                nodes_in_path.end())
                getAllNodesInNextPath (*it, dest, nodes_in_path, path, next_groups);
        }
        
        delete n;
    }
    
    path.pop_back ();
}

inline uint128_t getCycleGroup (uint128_t curr_group, uint128_t next_group)
{
    stack<uint128_t> _stack;
    vector<uint128_t> path_vector;
    unordered_set<uint128_t, uint128Hasher> visited_curr_to_next;
    unordered_set<uint128_t, uint128Hasher> visited_next_to_curr;
    vector<uint128_t> intersection;
    uint128_t cg = curr_group | next_group;
    vector<uint128_t>* next_hashids = OptGroup::nextGroupsHashID (curr_group);
  
    for (auto it = next_hashids->begin(); it != next_hashids->end(); it++)
    {
        if (*it != next_group)
            getAllNodesInNextPath (*it, next_group, visited_curr_to_next, 
                                   path_vector, OptGroup::nextGroupsHashID);
    }
    
    delete next_hashids;
    
    PRINT_DEBUG_L2 (std::cout<< "visited_curr_to_next " << std::endl);
    
    for (auto it = visited_curr_to_next.begin (); it != visited_curr_to_next.end (); it++)
    {
        PRINT_DEBUG_L2 (std::cout<< "   " << std::bitset<41>((uint64_t)*it) << std::endl);
    }
    
    PRINT_DEBUG_L2 (std::cout<< "visited_next_to_curr " << std::endl);
    
    for (auto it = visited_next_to_curr.begin (); it != visited_next_to_curr.end (); it++)
    {
        PRINT_DEBUG_L2 (std::cout<< "   " << std::bitset<41>((uint64_t)*it) << std::endl);
    }
    
    for (auto it = visited_curr_to_next.begin (); it != visited_curr_to_next.end (); it++)
    {
        if (visited_next_to_curr.find (*it) != visited_next_to_curr.end())
            intersection.push_back (*it);
    }
    
    PRINT_DEBUG_L2 (std::cout<< "intersection " << std::endl);
    
    for (auto it = intersection.begin (); it != intersection.end (); it++)
    {
        PRINT_DEBUG_L2 (std::cout<< "   " << std::bitset<41>((uint64_t)*it) << std::endl);
    }
    
    for (auto it = visited_curr_to_next.begin(); it != visited_curr_to_next.end(); it++)
    {
        cg |= *it;
    }
    
    return cg;
}

inline uint64_t cfgMemoize (uint128_t hash_id)
{
    PRINT_DEBUG_BLOCK_L1
        std::cout << "STARTING for hash_id " << std::bitset<41> ((uint64_t)hash_id) << std::endl;
    vector<uint128_t>* n;
    
    auto g = T.find (hash_id);
    if (g != T.end())
    {
        PRINT_DEBUG_L2 (std::cout<<"found hash_id " << std::bitset<41>((uint64_t)hash_id)<<std::endl);
        foundInDict += 1;
        return g->second;
    }

    PRINT_DEBUG_L2 (std::cout<<"to get next groups "<< std::bitset<41>((uint64_t)hash_id)<<std::endl);
    n = OptGroup::nextGroupsHashID (hash_id);
    
    if (n->size() == 0)
    {
        delete n;
        int max_dim = get_max_dimensions_for_group (hash_id);
        std::vector <uint64_t> tile_sizes = std::vector <uint64_t> (max_dim, 0);
        hashIDToOptHashID[hash_id] = hash_id;
        uint64_t _cost = cost(hash_id, tile_sizes);
        optHashIDToTileSizes [hash_id] = tile_sizes;
        T[hash_id] = _cost;
        if (hash_id && (!(hash_id & (hash_id-1))))
            return _cost;
            
        return _cost;
    }
    
    uint64_t totalMinCost;
    
    runningTime += 1;
    
    if (numberOfOnes (hash_id) < (1UL << logMaxChildren))
    {
        totalMinCost = 1L << 60;
        uint128_t optHashID = 0;
        uint128_t next_hash_id = 0;
        std::vector <uint64_t> opt_tile_sizes;
        std::vector <uint64_t> opt_tile_sizes_next_group;
        std::vector <uint64_t> tile_sizes_curr_group;
        int max_dim_curr_group;
        uint64_t cost_curr_group;
        uint64_t cost_next_groups = 0;
        unordered_set <uint128_t, uint128Hasher> includedVertices;
        
        for (auto it = n->begin(); it != n->end(); ++it)
        {
            std::vector <uint64_t> tile_sizes_next_group;
            int max_dim_next_group = get_max_dimensions_for_group (*it);
            tile_sizes_next_group = std::vector <uint64_t> (max_dim_next_group, 0);
            uint64_t q = cfgMemoize (*it);
            cost_next_groups += q;
            PRINT_DEBUG_L1 (std::cout << "next " << std::bitset<41> ((uint64_t)(*it)) << " cost " << q << std::endl);
        }
        
        
        PRINT_DEBUG_L1 (std::cout << "cost_next_groups " << cost_next_groups << std::endl);
        max_dim_curr_group = get_max_dimensions_for_group (hash_id);
        tile_sizes_curr_group = std::vector <uint64_t> (max_dim_curr_group, 0);
        cost_curr_group = cost(hash_id, tile_sizes_curr_group);
        for (auto it = n->begin(); it != n->end(); ++it)
        {
            std::vector <uint64_t> tile_sizes_cg;
            std::vector <uint64_t> tile_sizes_next_group;
            
            int max_dim_next_group;
            
            uint128_t cg = (*it) | hash_id;
            PRINT_DEBUG_L1 (std::cout<<"it " << std::bitset<41>((uint64_t)*it)<<std::endl);
            uint64_t cost1;
            if (isCycle (hash_id, *it, n))
            {
                PRINT_DEBUG_L2 (std::cout<< "cycle found in grouping "<<std::endl);
                cost1 = -1;
            }
            else if (isCycle (hash_id, *it))
            {
                cg = getCycleGroup (hash_id, *it);
                cost1 = -1; //Invalid Grouping
            }
            else
            {   
                PRINT_DEBUG_L2 (std::cout<< "getting cost for cg " << std::endl);
                int max_dim_cg = get_max_dimensions_for_group (cg);
                PRINT_DEBUG_L2 (std::cout<< "max_dim_cg " << max_dim_cg << std::endl);
                tile_sizes_cg = std::vector <uint64_t> (max_dim_cg, 0);
                uint64_t cg_cost = cost (cg, tile_sizes_cg);
                PRINT_DEBUG_L2 (std::cout<< "cg_cost " << (int64_t)cg_cost << std::endl);
                if ((int64_t)cg_cost == -1)
                    cost1 = -1;
                else
                {
                    PRINT_DEBUG_L1 (std::cout<< "memoizing " << std::endl);
                    uint64_t o = cfgMemoize (cg);
                    cost1 = o;
                }
            }
            
            max_dim_next_group = get_max_dimensions_for_group (*it);
            tile_sizes_next_group = std::vector <uint64_t> (max_dim_next_group, 0);
            uint64_t cost2 = cost_next_groups + cost_curr_group;
            cost(*it, tile_sizes_next_group);
            
            if (cost1 >= 0 && cost2 >= 0)
                totalMinCost = min(totalMinCost, min (cost1, cost2));
            else if (cost1 < 0)
            {
                totalMinCost = min(totalMinCost, cost2);
            }
            else if (cost2 < 0)
            {
                totalMinCost = min(totalMinCost, cost1);
            }
            
            PRINT_DEBUG_L1 (std::cout<<"hash_id " << 
                            std::bitset<41>((uint64_t)hash_id) << 
                            " totalmin " << totalMinCost << "  " << 
                            std::bitset<41>((uint64_t)cg) << "   " << 
                            (int64_t)cost1 << "   " << cost2 << " " << 
                            std::bitset<41>((uint64_t)(*it)) << std::endl);
            if (cost1 == totalMinCost)
            {
                opt_tile_sizes = tile_sizes_cg;
                optHashID = cg;
            }
            else
            {
                opt_tile_sizes = tile_sizes_curr_group;
                opt_tile_sizes_next_group = tile_sizes_next_group;
                next_hash_id = *it;
            }
        }

        if (optHashID != 0)
        {
            PRINT_DEBUG_L1(std::cout<<"hash_id " << 
                           std::bitset<41>((uint64_t)hash_id) << " opthashid " 
                           << std::bitset<41>((uint64_t)optHashID) << std::endl);
            hashIDToOptHashID[hash_id] = optHashID;
            optHashIDToTileSizes[optHashID] = opt_tile_sizes;
            
            PRINT_DEBUG_L2(std::cout<< "SETTING TILE SIZES FOR " << 
                           std::bitset<41>((uint64_t)optHashID)<< std::endl);
            for (auto it = opt_tile_sizes.begin (); it != opt_tile_sizes.end (); it++)
            {
                PRINT_DEBUG_L2 (std::cout<< "   " << *it << std::endl);
            }
        }
        
        else{
            PRINT_DEBUG_L2 (std::cout<<"hash_id " << 
                            std::bitset<41>((uint64_t)hash_id) << " opthashid " 
                            << std::bitset<41>((uint64_t)hash_id) << std::endl);
            hashIDToOptHashID[hash_id] = hash_id;
            totalMinCost = cost_next_groups + cost_curr_group;
            optHashIDToTileSizes[hash_id] = opt_tile_sizes;
            optHashIDToTileSizes[next_hash_id] = opt_tile_sizes_next_group;
        }
    }
    else
    {
        totalMinCost = 0;
        PRINT_DEBUG_L2 (std::cout<< hash_id << "  hash_id"<<std::endl);
        for (auto it = n->begin(); it != n->end(); ++it)
        {
            std::vector <uint64_t> tile_sizes_next_group;
            int max_dim_next_group = get_max_dimensions_for_group (*it);
            tile_sizes_next_group = std::vector <uint64_t> (max_dim_next_group, 0);
            PRINT_DEBUG_L2 (std::cout<<"filled it " << 
                            std::bitset<41>((uint64_t)*it)<<std::endl);
            uint64_t q = cfgMemoize (*it);
            totalMinCost += q + cost(*it, tile_sizes_next_group);
            optHashIDToTileSizes [*it] = tile_sizes_next_group;
        }
        
        int max_dim_curr_group = get_max_dimensions_for_group (hash_id);
        std::vector <uint64_t> tile_sizes_next_group;
        
        tile_sizes_next_group = std::vector <uint64_t> (max_dim_curr_group, 0);
        totalMinCost += cost (hash_id, tile_sizes_next_group);
        hashIDToOptHashID[hash_id] = hash_id;
        optHashIDToTileSizes[hash_id] = tile_sizes_next_group;
        PRINT_DEBUG_L2 (std::cout<<"hash_id " << hash_id << " cost " << 
                        totalMinCost<<std::endl);
    }
        
    T[hash_id] = totalMinCost;
    PRINT_DEBUG_L2 (std::cout<<"hash_id " << std::bitset <41>((uint64_t)hash_id)
                    << " totalMin " << totalMinCost << std::endl);
    delete n;
    PRINT_DEBUG_L1 (std::cout << "ENDING for hash_id " << 
                    std::bitset<41> ((uint64_t)hash_id) << std::endl);
    
    return totalMinCost;
}

void topological_sort (uint128_t vertex, vector<uint128_t>& output, unordered_set<uint128_t, uint128Hasher>& visited, vector<uint128_t>* next_groups)
{
    visited.insert (vertex);
    
    vector<uint128_t>* n;
    
    n = OptGroup::nextGroupsHashID (vertex);
    
    for (auto it = n->begin(); it != n->end(); it++)
    {
        if (visited.find(*it) == visited.end())
            topological_sort (*it, output, visited, next_groups);
    }
    
    if (next_groups == NULL)
        output.push_back (vertex);
        
    else if (std::find(next_groups->begin(), next_groups->end(), vertex) != next_groups->end())
    {
        output.push_back (vertex);
    }
    
    delete n;
}

bool insertingNextPrevCreatesCycle (OptGroup* g1, OptGroup* g2)
{
    bool g1_in_g2_next = g2->nextGroups().find (g1) != g2->nextGroups().end();
    bool g1_in_g2_prev = true;//g2->prevGroups().find (g1) != g2->prevGroups().end();
    bool g2_in_g1_next = g1->nextGroups().find (g2) != g1->nextGroups().end();
    bool g2_in_g1_prev = true;//g1->prevGroups().find (g2) != g1->prevGroups().end();
    
    return g1_in_g2_next && g1_in_g2_prev && g2_in_g1_next && g2_in_g1_prev;
}

OptGroup* opt_grouping_topological_sort (uint128_t _grp_hash_id)
{
    vector<uint128_t> optgroup_vector;
    unordered_set<uint128_t, uint128Hasher> visited;
    vector<uint128_t> topological_sort_output;
    PRINT_DEBUG_L2 (std::cout<< "SS "<<std::bitset <41>((uint64_t)_grp_hash_id)
                    <<std::endl);
    topological_sort (_grp_hash_id, topological_sort_output, visited, NULL);
    PRINT_DEBUG_L2 (std::cout<<"topo done "<< std::endl);
    reverse (topological_sort_output.begin (), topological_sort_output.end ());
    
    for (auto it = topological_sort_output.begin (); it != topological_sort_output.end (); it++)
    {
        bool findInOptHashDict = false;
        PRINT_DEBUG_L2 (std::cout<< "going to find " << 
                        std::bitset <41>((uint64_t)*it)<<std::endl);
        for (auto it2 = optgroup_vector.begin (); it2 != optgroup_vector.end (); it2++)
        {
            if (((*it2) & (*it)) == (*it))
            {
                OptGroup* g = OptGroup::optHashIDToGroup[*it];
                PRINT_DEBUG_L2 (std::cout<< "exisghghts   " <<(uint64_t)g << 
                                "  " << std::bitset <41>((uint64_t)(*it))<<
                                std::endl);
                findInOptHashDict = true;
                for (auto it3 = g->prevGroups().begin(); it3 != g->prevGroups().end(); it3++)
                {
                    for (auto it4 = (*it3)->parentGroups().begin(); it4 != (*it3)->parentGroups().end(); it4++)
                    {
                        OptGroup* optg = OptGroup::parentOptGroupsToHashID[*it2];
                        if (*it4 != optg && !insertingNextPrevCreatesCycle (*it4, optg))
                        {
                            PRINT_DEBUG_L2 (std::cout<<"Add " << 
                                            std::bitset <41>((uint64_t)(*it4)->hashID ()) << 
                                            " to " << 
                                            std::bitset <41>((uint64_t)(optg)->hashID ()) << 
                                            std::endl);
                            (*it4)->nextGroups().insert (optg);
                            optg->prevGroups().insert (*it4);
                        }
                    }
                }
            }
        }
        
        if (findInOptHashDict == true)
            continue;
        
        uint128_t hash_id = *it;
        PRINT_DEBUG_L2 (std::cout<< "here    " <<
                        std::bitset <41>((uint64_t)*it)<<
                        std::endl);
        while (hashIDToOptHashID.find (hash_id) != hashIDToOptHashID.end())
        {
            if (hashIDToOptHashID[hash_id] == hash_id)
                break;
            
            hash_id = hashIDToOptHashID[hash_id];
        }
        OptGroup* optg = OptGroup::createParentOptGroup (hash_id);
        OptGroup* g = OptGroup::optHashIDToGroup[*it];
        PRINT_DEBUG_L2 (std::cout<<"parentgroups " << 
                        (uint64_t)g << " " << g->parentGroups().size()<<
                        std::endl);
        if (g->parentGroups().size() >= 1)
        {
            for (auto i = g->parentGroups().begin(); i != g->parentGroups().end(); i++)
            {
                PRINT_DEBUG_L2 (std::cout<<"parentgroup: " << 
                                std::bitset <41>((uint64_t)(*i)->hashID ()) << 
                                std::endl);
            }
        }
        
        for (auto it3 = g->prevGroups().begin(); it3 != g->prevGroups().end(); it3++)
        {
            PRINT_DEBUG_L2 (std::cout<<"prevGroup " << 
                            std::bitset <41>((uint64_t)(*it3)->hashID ()) << 
                            std::endl);
            for (auto it4 = (*it3)->parentGroups().begin(); it4 != (*it3)->parentGroups().end(); it4++)
            {
                if (*it4 != optg && !insertingNextPrevCreatesCycle (*it4, optg))
                {
                    PRINT_DEBUG_L2 (std::cout<<"Add prev" << 
                                    std::bitset <41>((uint64_t)(*it4)->hashID ()) <<
                                    " to " << 
                                    std::bitset <41>((uint64_t)(optg)->hashID ()) << 
                                    std::endl);
                    (*it4)->nextGroups().insert (optg);
                    optg->prevGroups().insert (*it4);
                }
            }
        }
        
        optgroup_vector.push_back (hash_id);
    }
    
    return *(OptGroup::optHashIDToGroup[_grp_hash_id]->parentGroups().begin());
}

bool isCyclicUtil (OptGroup* v, unordered_map<OptGroup*, bool>& visited, 
                   unordered_map<OptGroup*, bool>& recStack, 
                   OptGroup** cycle, int cycleIndex, int& cycle_length)
{
    if(visited[v] == false)
    {
        // Mark the current node as visited and part of recursion stack
        visited[v] = true;
        recStack[v] = true;
        cycle [cycleIndex] = v;
        // Recur for all the vertices adjacent to this vertex
        for(auto &iter : v->nextGroups ())
        {
            cycle_length ++;
            if (!visited[(OptGroup*)iter] && 
                isCyclicUtil((OptGroup*)iter, visited, recStack, 
                             cycle, cycleIndex + 1, cycle_length))
            {
                return true;
            }
            else if (recStack[(OptGroup*)iter])
            {
                return true;
            }
            cycle_length--;
        }
    }
    
    recStack[v] = false;  // remove the vertex from recursion stack
    return false;
}

void detect_and_destroy_cycles ()
{
    bool change;
    
    change = true;
    
    while (change)
    {
        // Mark all the vertices as not visited and not part of recursion
        // stack
        int n_vertices;
        unordered_map <OptGroup*, bool> visited;
        unordered_map <OptGroup*, bool> recStack;
        n_vertices = OptGroup::parentOptGroupsToHashID.size ();
        OptGroup* cycle [n_vertices];
        int cycle_length = 0;
        
        change = false;
        
        for(auto const &iter: OptGroup::parentOptGroupsToHashID)
        {
            visited[iter.second] = false;
            recStack[iter.second] = false;
        }
        static int n_calls = 0;
        n_calls++;
        // Call the recursive helper function to detect cycle in different
        // DFS trees
        int q = 0;
        for(auto const &iter: OptGroup::parentOptGroupsToHashID)
        {
            q++;
            cycle_length = 0;
            
            if (isCyclicUtil(iter.second, visited, recStack, cycle, 0, cycle_length))
            {
                change = true;
                break;
            }
        }
        
        if (!change)
            continue;
            
        //Break cycle by creating separate group of child which is not the input (of group)
        //and has an edge from another group
        OptGroup* child_to_disconnect = NULL;
        OptGroup* curr_grp;
        OptGroup* next_grp;
        
        for (int i = 0; i < cycle_length; i++)
        {
            int curr_grp_index = i;
            int next_grp_index = (i+1)%cycle_length;
            curr_grp = cycle [curr_grp_index];
            next_grp = cycle [next_grp_index];
            
            //Find a child in next_grp having an edge from a child of curr_grp
            //and is not the input of the next_grp
            for (auto const &curr_child: curr_grp->childGroups ())
            {
                for (auto const &next_curr_child: curr_child->nextGroups ())
                {
                    if ((next_curr_child->hashID () & next_grp->hashID ()) ==  
                        (next_curr_child->hashID ()))
                    {
                       bool candidate_is_input = false;
                       
                       for (auto const &prev : next_curr_child->prevGroups ())
                       {
                           if ((prev->hashID () & next_grp->hashID ()) ==
                                prev->hashID ())
                           {
                               candidate_is_input = true;
                               break;
                           }
                       }
                       
                       PRINT_DEBUG_L2 (std::cout<< "candidate_is_input " << 
                                        candidate_is_input << std::endl);
                       if (candidate_is_input)
                            child_to_disconnect = (OptGroup*)next_curr_child;
                    }
                
                    if (child_to_disconnect)
                        break;
                }
                
                if (child_to_disconnect)
                    break;
            }
            
            if (child_to_disconnect)
                break;
        }
        
        PRINT_DEBUG_L2 (std::cout<< "child_to_disconnect = " << 
                        child_to_disconnect << std::endl);
        PRINT_DEBUG_L2 (std::cout<< "child_to_disconnect is " << 
                        std::bitset<41>((uint64_t)child_to_disconnect->hashID()) << 
                        std::endl);
        
        OptGroup* to_remove = OptGroup::createParentOptGroup (child_to_disconnect->hashID ());
        
        assert (to_remove != child_to_disconnect);
        
        for (auto const &_next_grp: child_to_disconnect->nextGroups ())
        {
            for (auto const &grp : ((OptGroup*)_next_grp)->parentGroups ())
            {
                if (grp->hashID () != next_grp->hashID ())
                    to_remove->nextGroups ().insert (grp);
            }
        }
        
        for (auto const &_next_grp : to_remove->nextGroups ())
        {
            if (_next_grp->hashID () != next_grp->hashID ())
                _next_grp->prevGroups ().insert ((OptGroup*)to_remove);
        }
        
        for (auto const &prev_grp: child_to_disconnect->prevGroups ())
        {
            for (auto const &grp : ((OptGroup*)prev_grp)->parentGroups ())
            {
                if (grp->hashID () != next_grp->hashID ())
                    to_remove->prevGroups ().insert (grp);
            }
        }
        
        for (auto const &prev_grp : to_remove->prevGroups ())
        {
            if (prev_grp->hashID () != next_grp->hashID ())
                prev_grp->nextGroups ().insert ((OptGroup*)to_remove);
        }
        
        
        //Insert to_remove in prevGroups and nextGroups of 
        next_grp->removeChild (to_remove->hashID (), to_remove);
    }
}

void correct_grouping ()
{
    bool change = true;
    
    while (change)
    {
        change = false;
        for (auto it = OptGroup::optHashIDToGroup.begin (); 
             it != OptGroup::optHashIDToGroup.end (); it++)
        {
            OptGroup* group = it->second;
            
            if (group == NULL)
                continue;
            
            PRINT_DEBUG_L1 (std::cout<<"LLLLL "<< 
                            std::bitset<41>((uint64_t)group->hashID ())<< 
                            std::endl);
            PRINT_DEBUG_L1 (std::cout<<"N_PARENTS " << 
                            group->parentGroups().size() << std::endl);
        
            for (auto it2 = group->parentGroups().begin(); 
                 it2 != group->parentGroups().end(); it2++)
            {
                PRINT_DEBUG_L1 (std::cout<<"PARENT " << 
                                std::bitset<41>((uint64_t)(*it2)->hashID())<<std::endl);
            }
            
            if (group != NULL && group->parentGroups().size () <= 1)
                continue;
    
            change = true;
            
            OptGroup* minParentGroup = 0;
            uint64_t minCost = 1L << 60;
            
            for (auto it = group->parentGroups().begin (); 
                 it != group->parentGroups().end(); it++)
            {
                PRINT_DEBUG_L2 (std::cout<<"parent groups: "<< 
                                std::bitset<41>((uint64_t)(*it)->hashID ())<< 
                                std::endl);
                int max_dim = get_max_dimensions_for_group ((*it)->hashID());
                uint64_t _cost;
                std::vector <uint64_t> tile_sizes;
                
                tile_sizes = std::vector <uint64_t> (max_dim, 0); 
                _cost = cost ((*it)->hashID(), tile_sizes);
                
                if (_cost < minCost && !((*it)->hashID() && (!((*it)->hashID() & ((*it)->hashID()-1)))))
                {
                    minCost = _cost;
                    minParentGroup = (*it);
                }
            }
            
            PRINT_DEBUG_L1 (std::cout<<"wewwewe "<< 
                            std::bitset<41>((uint64_t)group->hashID ())<< 
                            std::endl);
            PRINT_DEBUG_L1 (std::cout<<"minParentGroup " << 
                            std::bitset<41>((uint64_t)minParentGroup->hashID())<< 
                            std::endl);
        
            /* removeChild () modifies the child's parentGroups() set.
             * Hence, we cannot call removeChild () from the loop iterator 
             * of child's parentGroups ().
             * Create a copy of parents and call removeChild on them.
             * */
            vector<OptGroup*> toRemoveParents;
            
            for (auto it = group->parentGroups().begin (); 
                 it != group->parentGroups().end(); it++)
            {
                if (*it != minParentGroup)
                {
                    toRemoveParents.push_back (*it);
                }
            }
            
            for (auto it = toRemoveParents.begin(); it != toRemoveParents.end();
                 it++)
            {   
                PRINT_DEBUG_L1 (std::cout<<"removechild "<< std::endl);
                (*it)->removeChild (group->hashID(), minParentGroup);
            }
            PRINT_DEBUG_L1 (std::cout<<"wew2222 "<< 
                             std::bitset<41>((uint64_t)group->hashID ())<< 
                             std::endl);
        }
    }
    
    //FIXME: Cycles are created with __SERVER__ configuration with cores = 16
    //Below is just a temporary fix, to keep the project progressing 
    detect_and_destroy_cycles ();
}

PyObject* firstPyGroup (uint128_t hash_id, bool dummySource, uint64_t maxID)
{
    uint64_t bit = 0;
        
    while (hash_id != 0)
    {
        if ((hash_id & 1L) == 1)
        {   
            uint128_t l = 1;
            l = l<<bit;
            Group* g = Group::hashIDToGroup[l];
            PyObject* o = g->getPyGroup ();
            if (o != NULL)
                return o;
        }
        
        hash_id = hash_id >> 1;
        bit++;
    }
    
    return NULL;
}

PyObject* getPyGroupTopologicalOrder (PyObject* pyg)
{
    PyObject* order = PyDict_GetItem (pygroup_topological_order, 
                                      pyg);
    return order;
}

bool pyGroupSortFunc (PyObject* pyg1, PyObject* pyg2)
{    
    return PyLong_AsLong (getPyGroupTopologicalOrder (pyg1)) < PyLong_AsLong (getPyGroupTopologicalOrder (pyg2));
}

void clear_everything ()
{
    hashIDToOptHashID.clear();
    T.clear();
    vector<std::unordered_map <uint128_t, OptGroup*, uint128Hasher>::iterator> toremove;
    
    OptGroup::optHashIDToGroup.clear ();
    OptGroup::vectorOptGroups.clear();
    
    for (auto it = OptGroup::parentOptGroupsToHashID.begin(); it!= OptGroup::parentOptGroupsToHashID.end(); it++)
    {
        OptGroup::optHashIDToGroup[it->first] = it->second;
        OptGroup::vectorOptGroups.push_back (it->second);
    }
    
    OptGroup::parentOptGroupsToHashID.clear ();
    
    optHashIDToTileSizes.clear ();
    
}

/*
 * Main function of the library. Arguments are 
 * in_group: A list of input groups
 * out_group: A list of output groups
 * groups: A list of all groups
 * pipeline: The Pipeline Object
 * reduction_cls: Class object of Reduction class
 * small_comps: A list of small computations
 * comp_size_map: A map of each computation to size
 * tstencil_cls: Class object of TStencil Class
 * pygroup_topological_order: Topological Order of Groups
 * pygroup_dim_reuse: Dimension reuse across each dimension for each group
 * pylive_size: 
 * pydim_size: Dimension sizes of each dimension of each group
 * storage_mapping_get_dim_size: Object for function storage_mapping.get_dim_size
 * cls_Storage: Class Object of Storage Class
 */
PyObject* dpgroup(PyObject* self, PyObject* args)
{
    bool dummySource;
    PyObject *in_group;
    PyObject *out_group;
    PyObject *groups;    
    PyObject *do_inline;
    PyObject *multi_level_tiling;
    PyObject *_printDPFusionTimeAndChoices;
    int _logMaxChildren;
    
    std::vector <PyObject*> pygroups_vector;
    
    std::cerr<<"--------DP Fusion--------" << std::endl;
    
    PyArg_ParseTuple (args, "OOOOOOOOOOOOOOOOllllffffOl", &in_group, &out_group, &groups, &pipeline, 
                      &reduction_cls, &small_comps, &comp_size_map, &tstencil_cls,
                      &pygroup_topological_order, &pygroup_dim_reuse, &pylive_size,
                      &pydim_size, &storage_mapping_get_dim_size, &cls_Storage, 
                      //Optimization Parameters (Inlining, MultiLevelTiling)
                      &do_inline, &multi_level_tiling,
                      //Machine Info
                      &L1_CACHE_SIZE, &L2_CACHE_SIZE, &N_CORES, &IMAGE_ELEMENT_SIZE,
                      //Weights
                      &DIM_STD_DEV_WEIGHT, &LIVE_SIZE_TO_TILE_SIZE_WEIGHT, 
                      &CLEANUP_THREADS_WEIGHT, &RELATIVE_OVERLAP_WEIGHT,
                      &_printDPFusionTimeAndChoices,
                      &_logMaxChildren);
    
    std::cerr << "Machine Information:" << std::endl << 
    "L1_CACHE_SIZE       "<< L1_CACHE_SIZE      << " Bytes" << std::endl <<
    "L2_CACHE_SIZE       "<< L2_CACHE_SIZE      << " Bytes" << std::endl << 
    "N_CORES             "<< N_CORES            << std::endl <<
    "IMAGE_ELEMENT_SIZE  "<< IMAGE_ELEMENT_SIZE << std::endl << std::endl;
    
    std::cerr << "Weights: "<< std::endl << 
    "DIM_STD_DEV_WEIGHT            " << DIM_STD_DEV_WEIGHT            << std::endl << 
    "LIVE_SIZE_TO_TILE_SIZE_WEIGHT " << LIVE_SIZE_TO_TILE_SIZE_WEIGHT << std::endl << 
    "CLEANUP_THREADS_WEIGHT        " << CLEANUP_THREADS_WEIGHT        << std::endl <<
    "RELATIVE_OVERLAP_WEIGHT       " << RELATIVE_OVERLAP_WEIGHT       << std::endl << std::endl;
    
   get_overlapping_size_func = PyObject_GetAttr (pipeline, 
                                                  Py_BuildValue ("s", 
                                                  "get_overlapping_size_for_groups"));
    if (_printDPFusionTimeAndChoices == Py_True)
    {
        printDPFusionTimeAndChoices = true;
        logMaxChildren = _logMaxChildren;
        log_increment_factor = 10;
    }
    else
        printDPFusionTimeAndChoices = false;
        
    if (do_inline == Py_True)
        INLINING_ENABLED = true;
    else if (do_inline == Py_False)
        INLINING_ENABLED = false;
    else
        assert (false);
    
    if (multi_level_tiling == Py_True)
        MULTI_LEVEL_TILING_ENABLED = true;
    else if (multi_level_tiling == Py_False)
        MULTI_LEVEL_TILING_ENABLED = false;
    else
        assert (false);
    
    if (INLINING_ENABLED)
        std::cerr << "INLINING           ENABLED" << std::endl;
    else
        std::cerr << "INLINING           DISABLED" << std::endl;
        
    if (MULTI_LEVEL_TILING_ENABLED)
        std::cerr << "MULTI_LEVEL_TILING ENABLED" << std::endl;
    else
        std::cerr << "MULTI_LEVEL_TILING DISABLED" << std::endl;
 
    std::cerr << std::endl << "-----Running DPFusion-----" << std::endl;            
    
    if (PyList_Size (in_group) == 1)
    {
        maxID = PyList_Size (groups);
        dummySource = false;
    }
    else
    {
        maxID = PyList_Size (groups) + 1;
        dummySource = true;
    }

    PRINT_DEBUG_BLOCK_L1
    {
        std::cout<< "Total Number of Nodes "<< maxID << std::endl;
    }
    
    for (int i = 0; i < PyList_Size (small_comps); i++)
    {
        PyObject* comp = PyList_GetItem (small_comps, i);
        
        small_comps_set.insert (comp);
    }
    
    for (int i = 0; i < PyList_Size (groups); i++)
    {
        pygroups_vector.push_back (PyList_GetItem (groups, i));
    }
    
    sort (pygroups_vector.begin (), pygroups_vector.end(), pyGroupSortFunc);
    
    for (uint64_t i = 0; i < pygroups_vector.size(); i++)
    {
        PyObject* group = pygroups_vector [i];
        PyObject* comps = PyObject_GetAttr (group,
                                            Py_BuildValue ("s", 
                                                           "comps"));
        for (int j = 0; j < PyList_Size (comps); j++)
        {
            static PyObject* str_lookup_key = Py_BuildValue ("s", "lookup_key");
            static PyObject* str_orig_stg_class = Py_BuildValue ("s", "orig_storage_class");
            static PyObject* str_func = Py_BuildValue ("s", "func");
            static PyObject* str_typ = Py_BuildValue ("s", "typ");
            static PyObject* str_ndims = Py_BuildValue ("s", "ndims");
            static PyObject* str_offsets = Py_BuildValue ("s", "offsets");
            PyObject* storage;
            PyObject* key;
            PyObject* comp;
            PyObject* func;
            PyObject* offsets;
            PyObject* typ;
            PyObject* dims;
            
            comp = PyList_GetItem (comps, j);
            func = PyObject_GetAttr (comp, str_func);
            typ = PyObject_GetAttr (func, str_typ);
            dims = PyObject_GetAttr (func, str_ndims);
            storage = PyObject_GetAttr (comp, str_orig_stg_class);
            key = PyObject_GetAttr (storage, str_lookup_key);
            string _strkey = pystring_to_string (key);
        
            pyGroupForComp [comp] = group;
            comp_to_orig_storage [comp] = storage;
            stg_class_to_key [storage] = _strkey;
            comp_to_func [comp] = func;
            func_to_typ [func] = typ;
            func_to_ndims [func] = PyLong_AsLong (dims);
            
            offsets = PyObject_GetAttr (storage, str_offsets);
            //check_and_print_exception ();
            vector <long> max_offset;
            
            for (long i = 0; i < PyLong_AsLong (dims); i++)
            {
                PyObject* _max_offset = PyTuple_GetItem (PyList_GetItem (offsets, i), 1);
                //check_and_print_exception ();
                max_offset.push_back (PyLong_AsLong (_max_offset));
                //check_and_print_exception ();
            }
            
            comp_to_max_offsets [comp] = max_offset;
            helper_storage_to_dim_storage [comp] = std::vector<PyObject*> ();
            
            for (long dim = 0; dim < PyLong_AsLong (dims); dim++)
            {
                static PyObject* str_get_dim = Py_BuildValue ("s", "get_dim");
                PyObject* dim_storage;
                PyObject* get_dim;
                
                get_dim = PyObject_GetAttr (storage, 
                                            str_get_dim);
                //check_and_print_exception ();
                PyObject* args = PyTuple_Pack (1, PyLong_FromLong (dim));
                //check_and_print_exception ();
                dim_storage = PyObject_CallObject (get_dim, args);
                //check_and_print_exception ();
                Py_DECREF (args);
                
                helper_storage_to_dim_storage [storage].push_back (dim_storage);
            }
        }
        
        Group* cppGroup = new Group (group, comps);
        pyToGroup[group] = cppGroup;
        OptGroup::createOptGroup (cppGroup->hashID ());
        PyObject* name = PyObject_GetAttr (group,
                                                   Py_BuildValue ("s", 
                                                                  "name"));
        PyObject* pyStr = PyUnicode_AsEncodedString(name, "utf-8", "Error ~");
        const char* ss = PyBytes_AS_STRING (pyStr);
        
        PRINT_DEBUG_BLOCK_L1
        {
            std::cout<< "hash_id " << std::bitset<41>((uint64_t)cppGroup->hashID()) << "  " << ss << std::endl;
        }
    }
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "comp_to_stg_class size " << comp_to_orig_storage.size() << std::endl;
        for (auto it = comp_to_orig_storage.begin(); it != comp_to_orig_storage.end(); it++)
        {
            std::cout << it->first << "  " << it->second <<std::endl;
        }
    }
    
    for (int i = 0; i < PyList_Size (groups); i++)
    {
        PyObject* group = PyList_GetItem (groups, i);
        Group* cppGroup = pyToGroup[group];
        OptGroup* optGroup = OptGroup::optHashIDToGroup [cppGroup->hashID ()];
        PyObject* children = PyObject_GetAttr (group,
                                               Py_BuildValue ("s", 
                                                              "children"));        
        for (int j = 0; j < PyList_Size (children); j++)
        {
            PyObject* child = PyList_GetItem (children, j);
            Group* cppChild = pyToGroup [child];
            cppGroup->nextGroups().insert (cppChild);
            OptGroup* optChild = OptGroup::optHashIDToGroup [cppChild->hashID ()];
            optGroup->nextGroups().insert (optChild);
        }
        
        PyObject* parents = PyObject_GetAttr (group,
                                               Py_BuildValue ("s", 
                                                              "parents"));
        
        for (int j = 0; j < PyList_Size (parents); j++)
        {
            PyObject* parent = PyList_GetItem (parents, j);
            Group* cppParent = pyToGroup [parent];
            cppGroup->prevGroups().insert (cppParent);
            OptGroup* optParent = OptGroup::optHashIDToGroup [cppParent->hashID ()];
            optGroup->prevGroups().insert (optParent);
        }
    }
    
    Group* start;
    OptGroup* opt_start;
    
    if (PyList_Size (in_group) == 1)
    {
        start = pyToGroup[PyList_GetItem (in_group, 0)];
        opt_start = OptGroup::optHashIDToGroup[start->hashID()];
    }
    else
    {
        start = new Group ();
        opt_start = OptGroup::createOptGroup (start->hashID ());
        for (int i = 0; i < PyList_Size (in_group); i++)
        {
            Group* next = pyToGroup[PyList_GetItem (in_group, i)];
            start->nextGroups().insert (next);
            next->prevGroups().insert (start);
            OptGroup* optnext = OptGroup::optHashIDToGroup [next->hashID ()];
            opt_start->nextGroups().insert (optnext);
            optnext->prevGroups().insert (opt_start);
        }
    }
    
    PyObject* grp_size_o = PyObject_GetAttr (pipeline,
                                             Py_BuildValue ("s", 
                                                            "_group_size"));
    grp_size = PyLong_AsLong (grp_size_o);
    
    PRINT_DEBUG_BLOCK_L1
    {
        std::cout << "Group size " << grp_size << std::endl;
    }

    uint64_t max_group_size = 1UL << logMaxChildren;
    uint128_t start_hash_id = opt_start->hashID();
       
    int iteration = 0;
    uint64_t _maxID = maxID;
    if (_maxID && (!(_maxID & (_maxID-1))))
    {}
    else
    {
        //Next power of 2 of maxID
        _maxID--;
        _maxID |= _maxID >> 1;
        _maxID |= _maxID >> 2;
        _maxID |= _maxID >> 4;
        _maxID |= _maxID >> 8;
        _maxID |= _maxID >> 16;
        _maxID++;
        PRINT_DEBUG_BLOCK_L1
        {
            std::cout<<"New Number of Nodes" << _maxID<<std::endl; 
        }
    }
    
    clock_t t1 = clock ();
    uint64_t dpChoices = 0;
    
    while (iteration == 0 || max_group_size <= _maxID)
    {
        if (iteration != 0)
            clear_everything ();

        cfgMemoize (start_hash_id);
        dpChoices += T.size ();
        
        //for (auto it = T.begin(); it != T.end (); it++)
        //{
            //std::cout << std::bitset<41> ((uint64_t)it->first) << "  " << ((uint64_t)it->second) << std::endl;
        //}
        
        for (auto it = hashIDToOptHashID.begin(); it != hashIDToOptHashID.end(); it++)
        {
            PRINT_DEBUG_L2 (std::cout<< std::bitset <41>((uint64_t)it->first) <<
                            "    " << std::bitset <41>((uint64_t)it->second)<<
                            std::endl);
        }
        
        opt_start = opt_grouping_topological_sort (start_hash_id);
        PRINT_DEBUG_L1 (std::cout<< "opt_grouping ended iteration "<<
                        iteration<< std::endl);
        
        for (auto it = OptGroup::parentOptGroupsToHashID.begin(); it != OptGroup::parentOptGroupsToHashID.end(); it++)
        {
            if (it->second == NULL)
                continue;
                
            PRINT_DEBUG_L1 (std::cout<< 
                            std::bitset <41>((uint64_t)it->second->hashID()) << 
                            std::endl);
            for (auto it2 = it->second->nextGroups().begin(); it2 != it->second->nextGroups().end(); it2++)
            {
                PRINT_DEBUG_L1 (std::cout<< "        "<< 
                                std::bitset <41>((uint64_t)(*it2)->hashID()) << 
                                std::endl);
            }
        }
    
        PRINT_DEBUG_L1 (std::cout<< "opt_grouping prev groups"<<std::endl);
        
        for (auto it = OptGroup::parentOptGroupsToHashID.begin(); it != OptGroup::parentOptGroupsToHashID.end(); it++)
        {
            if (it->second == NULL)
                continue;
                
            PRINT_DEBUG_L1 (std::cout<< 
                            std::bitset <41>((uint64_t)it->second->hashID()) << 
                            std::endl);
            for (auto it2 = it->second->prevGroups().begin(); it2 != it->second->prevGroups().end(); it2++)
            {
                PRINT_DEBUG_L1 (std::cout<< "        "<< 
                                std::bitset <41>((uint64_t)(*it2)->hashID()) << 
                                std::endl);
            }
        }
        
        correct_grouping();
    
        PRINT_DEBUG_L1 (std::cout<< "corrected grouping"<<std::endl);
        for (auto it = OptGroup::parentOptGroupsToHashID.begin(); it != OptGroup::parentOptGroupsToHashID.end(); it++)
        {   
            if (it->second == NULL)
                continue;
                
            PRINT_DEBUG_L1 (std::cout<< 
                            std::bitset <41>((uint64_t)it->second->hashID ()) << 
                            std::endl);
            for (auto it2 = it->second->nextGroups().begin(); it2 != it->second->nextGroups().end(); it2++)
            {
                PRINT_DEBUG_L1 (std::cout<< "        "<< 
                                std::bitset <41>((uint64_t)(*it2)->hashID ()) << 
                                std::endl);
            }
        }
        
        PRINT_DEBUG_L1 (std::cout<< "corrected grouping prevGrous"<<std::endl);
        for (auto it = OptGroup::parentOptGroupsToHashID.begin(); it != OptGroup::parentOptGroupsToHashID.end(); it++)
        {   
            if (it->second == NULL)
                continue;
                
            PRINT_DEBUG_L1 (std::cout<< 
                            std::bitset <41>((uint64_t)it->second->hashID()) << 
                            std::endl);
            for (auto it2 = it->second->prevGroups().begin(); it2 != it->second->prevGroups().end(); it2++)
            {
                PRINT_DEBUG_L1 (std::cout<< "        "<< 
                                std::bitset <41>((uint64_t)(*it2)->hashID()) << 
                                std::endl);
            }
        }
        
        start_hash_id = opt_start->hashID();
        iteration++;
        logMaxChildren += log_increment_factor;
        max_group_size = 1 << logMaxChildren;
    }
    
    if (printDPFusionTimeAndChoices)
    {
        clock_t t2 = clock ();
        
        std::cout << "Time taken by DP-Fusion: " << (t2-t1)*1000.0/CLOCKS_PER_SEC << " ms" << std::endl; 
        std::cout << "Number of choices evaluated by DP-Fusion: " << dpChoices << std::endl;
        exit (EXIT_SUCCESS);
    }
    
    PRINT_DEBUG_L1 (std::cin>>iteration);
    queue<OptGroup*> opt_queue;
    OptGroup* opt_temp = opt_start;
    opt_queue.push (opt_temp);
    PyObject* merge_groups_func = PyObject_GetAttr (pipeline,
                                              Py_BuildValue ("s", 
                                                             "merge_groups"));
                                              
    PRINT_DEBUG_L1 (std::cout<<"bfs running"<<std::endl);
    std::unordered_map<uint128_t, bool, uint128Hasher> visited;
    visited[opt_temp->hashID()] = true;
    std::unordered_set<OptGroup*> done;
    while (opt_queue.size () != 0)
    {
        InlinedAdjacentType new_nextGroups, new_prevGroups;
        PyObject* new_grp;
        
        opt_temp = opt_queue.front ();
        opt_queue.pop();
        new_grp = firstPyGroup (opt_temp->hashID(), dummySource, maxID);
        
        if (done.find(opt_temp) == done.end() && opt_temp->hashID() != 0)
        {
            done.insert(opt_temp);
            uint64_t first = first_one (opt_temp->hashID());
            if (numberOfOnes (opt_temp->hashID ()) > 1)
            {
                uint64_t bit = 0;
                uint128_t hash_id = opt_temp->hashID();
                Group* g;
                uint128_t l = 1;
                while (bit <= first)
                {
                    hash_id = hash_id >> 1;
                    bit++;
                }
                
                l = l << first;
                
                g = Group::hashIDToGroup[l];
                new_nextGroups[g] = std::set <Group*, Group::GroupComparer> (g->nextGroups ());
                new_prevGroups[g] = std::set <Group*, Group::GroupComparer> (g->prevGroups ());
                
                while (hash_id != 0)
                {                    
                    if ((hash_id & 1L) == 1 && !(dummySource && bit == maxID-1))
                    {   
                        l = 1;
                        l = l<<bit;
    
                        g = Group::hashIDToGroup[l];
                        PyObject* pp = g->getPyGroup ();
                        if (pp == NULL)
                            continue;
                        PyObject* args = PyTuple_Pack (2, new_grp, pp);
                        new_grp = PyObject_CallObject (merge_groups_func, args);
                        new_nextGroups[g] = std::set <Group*, Group::GroupComparer> (g->nextGroups ());
                        new_prevGroups[g] = std::set <Group*, Group::GroupComparer> (g->prevGroups ());
                
                        if (PyErr_Occurred () != NULL)
                        {
                            PyErr_Print();
                        }
                    }
            
                    hash_id = hash_id >> 1;
                    bit++;
                }
            }
        }
        
        if (new_grp)
        {
            uint64_t total_size_used;
            static PyObject* set_total_used_size = Py_BuildValue ("s", 
                                                                  "set_total_used_size");
            static PyObject* set_n_buffers = Py_BuildValue ("s", 
                                                            "set_n_buffers");
            static PyObject* set_inline_comps = Py_BuildValue ("s",
                                                              "set_inline_comps");
            PyObject* set_total_used_size_func, *set_n_buffers_func;
            int n_buffers=0;
            std::unordered_set<Group*, Group::GroupHasher> inlined_groups;
            uint64_t liveouts_size, livein_size;
            
            if (INLINING_ENABLED)
                update_graph_with_inlining (inlined_groups, new_nextGroups, 
                                            new_prevGroups, opt_temp->hashID ());
                                            
            total_size_used = getTotalSizeUsed (opt_temp->hashID(), n_buffers, 
                                                liveouts_size, livein_size,
                                                inlined_groups, new_nextGroups,
                                                new_prevGroups);
            PRINT_DEBUG_BLOCK_L1
                std::cout << "total_size_used " << total_size_used << " n_buffers " << n_buffers << std::endl;
            set_total_used_size_func = PyObject_GetAttr (new_grp,
                                                         set_total_used_size);
            PyObject* args = PyTuple_Pack (1,
                                           PyLong_FromLong (total_size_used));
            PyObject_CallObject (set_total_used_size_func, args);
            Py_DECREF (args);
            
            set_n_buffers_func = PyObject_GetAttr (new_grp,
                                                   set_n_buffers);
            args = PyTuple_Pack (1,
                                 PyLong_FromLong (n_buffers));
            PyObject_CallObject (set_n_buffers_func, args);
            Py_DECREF (args);
            
            PyObject* list_inline_comps = PyList_New (0);
            
            for (auto const &it : inlined_groups)
            {
                PyList_Append (list_inline_comps, it->getCompAtIndex (0));
            }
            
            PyObject *set_inline_comps_func = PyObject_GetAttr (new_grp, 
                                                           set_inline_comps);
            args = PyTuple_Pack (1, list_inline_comps);
            PyObject_CallObject (set_inline_comps_func, args);
            Py_DECREF (list_inline_comps);
            Py_DECREF (args);
        }
        
        for (auto it = opt_temp->nextGroups().begin(); 
             it != opt_temp->nextGroups().end(); it++)
        {
            if (visited.find(((*it)->hashID())) == visited.end())
                visited[(*it)->hashID()] = false;
                
            if (visited[(*it)->hashID()] == false)
            {
                visited[opt_temp->hashID()] = true;
                opt_queue.push ((OptGroup*)*it);
            }
        }
    }
    
    pyGroupForComp.clear ();
    pyToGroup.clear ();
    OptGroup::parentOptGroupsToHashID.clear ();
    hashIDToOptHashID.clear ();
    Group::hashIDToGroup.clear ();
    T.clear ();
    OptGroup::optHashIDToGroup.clear ();
    small_comps_set.clear ();
    optHashIDToTileSizes.clear ();
    pyGroupForComp.clear ();
    globalID = -1;
    return Py_BuildValue ("s", "_group_size");
}

PyMethodDef myModule_methods[] = {
  {"dpgroup", dpgroup, METH_VARARGS},
  {NULL, NULL}
};

struct PyModuleDef myModule =
{
    PyModuleDef_HEAD_INIT,
    "dpfusion", /* name of module */
    "",          /* module documentation, may be NULL */
    -1,          /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    myModule_methods
};

/*
 * Python calls this to let us initialize our module
 */
PyMODINIT_FUNC PyInit_dpfusion()
{
  return PyModule_Create(&myModule);
}
