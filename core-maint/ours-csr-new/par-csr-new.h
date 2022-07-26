/***********************
 * This is a sequential version of our Parallel Core Maintenance
 * 1) two level of OM data structure. 
 * 2) new removal algorithms
 * 3) After finish this, we extend to Parallel Version without difficulties. 
 * 4) Using CSR format to load large graphs. 
 **************/

#pragma once
#include <vector>
#include <set>
#include <queue>
#include <math.h>       /* pow */
#include <climits>
#include <map>
#include <omp.h>
#include "gadget/gadget.h"
#include "def.h"
using namespace std;


namespace ParCM{


    /**priority queue for core maint begin*************/
    struct DATA {
        node_t id; tag_t tag; subtag_t subtag; id_t version;
        DATA(node_t id, tag_t tag, subtag_t subtag, id_t version) : 
                id(id), tag(tag), subtag(subtag), version(version){ } 
    };

    struct CompareOrder { 
        bool operator()(DATA const& a, DATA const& b) const { 
            // return "true" if "a" is ordered before "b", for example:
            if (a.tag > b.tag) {return true; }
            else if( a.tag == b.tag) {
                if (a.subtag > b.subtag) return true;
            }
            return false;
        } 
    }; 
    /*
    * we use heap for priority queue to iterate all elements 
    * in case some of them changed. */

    class PRIORITY_Q {
    public:
        vector<DATA> v; vector<DATA> v2;
        id_t version;
    public:
        PRIORITY_Q(size_t size) {
            v.reserve(size); v2.reserve(size);
            version = EMPTY;
        }
        PRIORITY_Q(){}
        /*if the version is changed, all nodes needs upate until version is same
        *all the vesion number is same 
        * Return False: the version is changed. all elements needs to update to same version.*/
        inline bool push(DATA d) {
            if (v.empty()) version = d.version; //init version
            v.push_back(d); 
            if(d.version != version) return false;
            push_heap(v.begin(), v.end(), CompareOrder());
            return true;
        }
        inline DATA front() {return v.front();}
        inline void pop() {pop_heap(v.begin(), v.end(),CompareOrder()); v.pop_back();}
        inline bool empty() {return v.empty(); }
        inline void clear() { v.clear(); version = EMPTY;}
        inline void init() {make_heap(v.begin(), v.end(), CompareOrder());}
    };
    /**priority queue for core maint end *************/


    /**queue for core maint begin*********************/
    // this queue can be vector
    class QUEUE {
    private:
        std::vector<node_t> container;
    public:
        QUEUE(size_t size) {
            container.reserve(size);
        }
        QUEUE() {}
        inline void push(node_t v) {container.push_back(v);}
        inline node_t top() {return container.back();}
        inline void pop() {container.pop_back();}
        inline bool empty() {return container.empty(); }
        inline void clear() { container.clear();}
    };
    /***queue for core maint end*********************/


    enum{
        WHITE   = 0, /*initial*/
        GRAY    = 1, /*visited, not in V* but in V+*/
        BLACK   = 2, /*visited, in V* and in V+*/
    };

    /*the node can be changed by other worker*/
    class Node {
    public:
        //Computer Core number
        volatile deg_t degout; // deg+
        volatile deg_t degin;  // deg*
        volatile deg_t mcd;    //for edge removing.
       
        //Order Maintenance
        volatile node_t pre, next;  //double linked list
        volatile node_t group; //link to group id as parent.  
        volatile subtag_t subtag; // sub label tag 32 bits. 
       
        volatile color_t color;  // black, white, gray.
        volatile bool inQ;       // node is in priority queue PQ
        volatile bool inR;       // node in R by worder id. 
        //label_t outR;   // node is already backward.

        omp_lock_t omlock_omp; 
        omp_lock_t lock_omp;
        lock_t omlock_cas;
        lock_t lock_cas;

        Node();
        inline void OMLock();
        inline void OMUnlock();
        inline bool OMTestLock();
        inline void Lock();
        inline void Unlock();
        inline bool TestLock();
    };

    class Group {
    public:
        volatile size_t size;  // the number of children. Group is removed if 0.
        volatile tag_t tag;   // label tag 64 bits for each item range is 0 - n^2
        volatile node_t pre, next; // double linked list
        volatile node_t begin_b, end_b; // bottom level begin-end nodes,  
        
        omp_lock_t omlock_omp; 
        lock_t omlock_cas;

        Group();
        inline void OMLock();
        inline void OMUnlock();
        inline bool OMTestLock();
    };


    /*first version, with one level tags (label), only tag is used.
    * take O(logn) time for insert*/
    class CoreMaint {
        private:
        size_t n; // the number of vertices in graph.
        core_t max_core; // the max core number
        GRAPH &graph;
        vector<core_t>& core;
        vector< Node> V; // node information include head and tail
        
        vector< Group> G; // node in group for two level OM. 
        size_t group_size; // number of 

        // head-> x -> y -> tail (easy for insert and remove)
        
        /*maintaining the order   head(avoid empty list check)->node->node(tail)*/
        vector<core_t> H;  //each core list has a head to avoid empty list  
        vector<core_t> T;  //each core list has a tail;
        vector<core_t> Hg;  //top group head. 
        vector<core_t> Tg;  //top group tail.     
        
        /*counter*/
    

        /*double linked list operations*/
        inline void ListDelete(node_t x);
        inline void ListInsert(node_t x, node_t y);
        inline void MultiListInsert(node_t x, vector<node_t> &y);
        //inline void ListLink(vector<node_t> &y); //link all nodes in x.
        inline void TopListDelete(group_t x);
        inline void TopListInsert(group_t x, group_t y);
        inline void MultiTopListInsert(group_t x, vector<group_t> &y);

        /*OM tag*/
        inline tag_t getTag(node_t u){return G[V[u].group].tag;}
        inline node_t getGroup(node_t u){return V[u].group;}

        /*OM data structure, two level version*/
        inline bool Order(node_t x, node_t y); // return x < y
        inline bool SameCoreOrder(node_t x, node_t y);
        void OrderInsert(node_t x, node_t y); // insert y after x
        void SimpleRelabel2(node_t x, node_t xnext, group_t gy);
        void SimpleRelabel(node_t x); // for easy implementation
        void DeepRelabel(node_t x); // fully deep relabel
        void AssignLabel();
        void MultiOrderInsert(node_t x, vector<node_t> &y);
        inline void OrderDelete(node_t x);   // remove x

        /* inserting to head or tail of list. Can do without relabel.*/
        void OrderInsertHeadBatch(core_t k, vector<node_t> &y);
        void OrderInsertTail(core_t k, node_t y);
        


        /* current implementation.  
        * the tag version, used by PQ. all node in PQ has the same version
        * odd: lock, even unlock. use the memory transaction. 
        * */ 
        vector<ver_t> g_tag_version;   // odd lock, even unlock. lock version can only be even.
        PRIORITY_Q PQ;      // queue backward
        void PQ_update(const core_t k);   // update all nodes with new version.

        /*used by edge remove*/
        QUEUE R; 

        vector<node_t> Vblack;  // the order of black nodes (may include gray)
        vector<node_t> Vcolor; // all colored vertices.

        //vector<node_t> relabel_nodes; //used by relabel
        //vector<group_t> groups;
        //vector<group_t> locked_groups;

        public:
        CoreMaint(size_t n, GRAPH &graph, vector< core_t> &core);
        ~CoreMaint(){}
        void Init(vector<int> &odv);
        void InitParallel(int num_worker);
        void ComputeCore(vector<int> &core, vector<int> &order_v, bool with_init);
        
        // for edge insert
        inline void Forward(node_t w);
        inline void Backward(node_t w);
        inline void DoPre(node_t u);
        inline void DoAdj(node_t u);
        int EdgeInsert(node_t x, edge_t y); // insert to double-linked list
        int ParallelEdgeInsert(vector<pair<int, int>> &edges, int m, int m2);
        // for edge remove
        void DoMCD(node_t u, const core_t K, bool off);
        int CheckMCD(node_t u, const core_t K);
        int EdgeRemove(node_t x, edge_t y); //remove in double-linked list.      

        
        /* test for debug 
        *  */
        vector<node_t> InitTestOM(size_t insert_size);            
        int TestOM(vector<node_t> &nodes); 
        int CheckOM(bool info);
        int CheckAllMCDValue(bool info);

        /* check the correctness of algorithm */
        int CheckCore(vector<core_t> &tmp_core, bool info); //check core numbers.
        int CheckAll(node_t, node_t, int id, vector<core_t> &tmp_core, vector<node_t> &order_v); 
        void PrintOMVersion();
    };
}
