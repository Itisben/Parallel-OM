#ifndef CORE_GADGET_GADGET_H_
#define CORE_GADGET_GADGET_H_

#include <utility>
#include <vector>
#include <time.h>
#include <assert.h>
#include "../defs.h"

/*local defined CSR format that is used in our experiments. For edges, 
* it has enough sapce for insertion and also support removal*/
class GRAPH {
    typedef int node_t;
public:
    /* graph create by csr format 
    * for (node_t idx = G.begin[v];  idx < G.end(v);  idx++)
    *    u = node_idx[idx]; 
    */
    const int n; // number of vertices  
    const int m; // maximum number of edges
    std::vector<node_t> node_idx;
    std::vector<node_t> begin;
    std::vector<node_t> end; // end = next begin for full inserted graph. 
    //std::vector<node_t> mid; // begin to  < mid is all u.post; mid to < end is all u.pre 

    /*all edges is used for allocate memory*/
    GRAPH(int n, int m, std::vector<std::pair<node_t, node_t>> &edges):
            n{n}, m{m}{
        node_idx = std::vector<node_t>(m*2+1); // undirected graph
        begin=std::vector<node_t>(n+1);
        end=std::vector<node_t>(n+1);

        //allocate the memory for undirected graphs
        for (auto const e: edges) { //cnt the edge number for each node
            assert(e.first < n && e.first >=0); 
            assert(e.second < n && e.second >=0);
            end[e.first]++; end[e.second]++; 
        }
        begin[0] = 0;
        for (node_t v = 1; v < n+1; v++) {
            begin[v] = begin[v-1] + end[v-1]; //accumulate the size
        }
        // initially the graph is empty
        for (node_t v = 0; v < n+1; v++) {
            end[v]  = begin[v];
            //mid[v] = begin[v];
        }
        // for(node_t v=0; v<n; v++) {
        //     ASSERT(begin[v] <= begin[v+1]);
        //     ASSERT(begin[v] <= 2*m);
        // }
    }

    // for undirected graph
    void insert(node_t u, node_t v) { 
        node_idx[end[u]] = v; end[u]++;
        assert(end[u] <= begin[u+1]); 
        node_idx[end[v]] = u; end[v]++;
        assert(end[v] <= begin[v+1]);
    }
    
    // for undirected graph
    void remove(node_t u, node_t v) {
        for (int idx = begin[u]; idx < end[u]; idx++) {
            if (v == node_idx[idx]) { // edges are not empty
                std::swap(node_idx[idx], node_idx[end[u] - 1]);
                --end[u];
                break;
            }
        }
        for (int idx = begin[v]; idx < end[v]; idx++) {
            if (u == node_idx[idx]) { // edges are not empty
                std::swap(node_idx[idx], node_idx[end[v] - 1]);
                --end[v];
                break;
            }
        }
    }
    // the size of edge for node u; 
    int size(node_t u) {return end[u] - begin[u];}

    inline node_t get(int id) {return node_idx[id];}
    
};


namespace gadget {
void RepeatWith(const char symbol, const int repeat);
std::vector<std::vector<int>> ReadGraph(const char* const path,
                                        int* const n, int* const m);
std::vector<std::pair<int, int>> ReadTempEdgesS(const char* const path,
                                                int* const n, int* const m);
std::vector<std::pair<int, int>> ReadEdgesS(const char* const path,
                                            int* const n, int* const m);

std::vector<std::pair<int, int>> ReadEdges(const char* const path,
                                            int* const n, int* const m);
                                            
std::vector<std::pair<int, int>> CSRReadEdges(char* const path,
                    int* const n, int* const m, int shuffle); 

/*read m edges from the beggining and stored as our new GRAPH*/
GRAPH CreateOurCSRGRAPH(std::vector<std::pair<int, int>> &edges, int n, int size); 

void OutputCoreNumber(const char* const path, std::vector<int> &core, int n);


void OutputSampleEdgeCoreNumber(const char* const path, std::vector<int> &core, 
    std::vector<std::pair<int, int>> &edges, const int n);

std::vector<std::pair<int, int>> sampleEdges(const char* const path, std::vector<std::pair<int, int>> &edges, const float percent);

void CutEdges(std::vector<std::pair<int, int>> &edges, const int num);

std::vector<int> RepeateRandomRead(const char* path);

}  // namespace gadget

#endif
