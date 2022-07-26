#include <unistd.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>

#include "core.h"
#include "defs.h"
#include "gadget/gadget.h"
#include "glist/glist.h"
#include "traversal/traversal.h"
//#include "ours-seq/core-maint.h"
//#include "ours-seq-csr/core-maint-csr.h"
#include "ours-csr-new/seq-csr-new.h"
#include "ours-csr-new/par-csr-new.h"
#include "ours-csr-new/par-om.h"

/*counting*/
int cnt_edge = 0;
int cnt_edge2 = 0;
int cnt_Vs = 0; // count of size V*
int cnt_Vp = 0; // count of size V+
int cnt_S = 0;  // count of all visited
int cnt_tag = 0; 
int cnt_rebalance_max_tag = 0;
int cnt_gap_adjust = 0;

int cnt_omorder = 0;
int cnt_ominsert = 0;
int cnt_omdelete = 0;
int cnt_ominsert_mid = 0; // not append to head or tail. 
int cnt_omsplit = 0;
int cnt_Q_size = 0;

int global_check_num = 0;

int g_debug = 0; // for debuging
unsigned int g_lock_type = CAS_LOCK; // CAS_LOCK; //OMP_LOCK 


int main(int argc, char** argv) {
    char path[128];      // path of input graph
    int method = 0;
    double ratio = 0.0;  // ratio of edges to insert
    int optInsertNum = 0;
    int optRemoveNum = 0;
    int temp = 0;   // temporal graph 
    float edgePercent = 100;
    int optTest = 0;
    int optWorkerNum = 0;

    if(argc < 2) {
        printf("*************Test Core Maint**************\n");
        printf("-p: the path of the data file\n");
        printf("-r: the proportion of edges that is sampled out for insertion/removal\n");
        printf("-I: the number of insert edges that for operation\n");
        printf("-R: the number of remove edges that for operation\n");
        printf("-w: the number of workers (1 - 64)\n");
        printf("-l: the type of lock, 0 CAS_LOCK, 1 OMP_LOCK(default), 2 NO lock\n");
        printf("-m: the algorithm to be used: 1 traversal, 2 order-based, 3 ours-seq, 4 ours-parallel\n");
        //printf("-M: the sub algorithm to be used: 1 ours batch insertion\n");
        printf("-T: 1 for temporal graphs, 0 for ordinary graphs sample edges, 2 for debug without sampled edges\n");
        printf("    3 csr sample edges, 4 csr without sample edges, 5 csr with repeated random \n");
        printf("-c: 1 Output the statistic of core numbers to file *.bin.core for original graph\n");
        printf("    2 Output the statistic of sampled 10k edges core numbers to \n");
        printf("-s: [<=100]the percent samele the percent of edges \n");
        printf("    [>=100000] the number of sameled of edges, 100k, 200k ... \n");
        printf("-d: set debug.\n");
        printf("-t: test \n"); 
        printf("for example: core -p path -I 100000 -m 4 -T -w 16\n");
        printf("\n");

        printf("*************Test Order Maint (OM data structure)**************\n");
        printf("-p: the path of the random number file\n");
        printf("-I: the number nodes for operation\n");
        printf("-w: the number of workers (1 - 64)\n");
        printf("-l: the type of lock, 0 CAS_LOCK, 1 OMP_LOCK(default), 2 NO lock\n");
        printf("-d: set debug.\n");
        printf("-t:  1 test,20 (no relable) 21 test OM REPEAT_RANDOM (few), 22 FIXED_MULTIPLE (many), 23 FIXED_ONE (max)\n"); 
        printf("-T: 1 repeated random positions, 2 on-the-fly random position.\n");
        printf("-s: 1 sorted postion, 0 unsorted position (default).\n ");
        printf("for example: ./core2 -p path -I 10000000 -T 1 -t 20 -w 16\n");
        exit(0);

    }
    // initialize the options
    int option = -1;
    int outputCore = 0;
    while (-1 != (option = getopt(argc, argv, "p:r:I:R:w:l:m:M:T:c:s:d:t:"))) {
        switch (option) {
        case 'p':
            strcpy(path, optarg);
            break;
        case 'r':
            ASSERT(0.0 <= (ratio = atof(optarg)) && 1.0 >= ratio);
            break;
        case 'I':
            optInsertNum = atoi(optarg);
            break;
        case 'R':
            optRemoveNum = atoi(optarg);
            break;
        case 'w':
            optWorkerNum = atoi(optarg);
            break;
        case 'l':
            g_lock_type = atoi(optarg);
            break;
        case 'm':
            method = atoi(optarg);
            break;
        case 'T':
            temp = atoi(optarg);
            break;
        case 'c':
            outputCore = atoi(optarg);
            break;
        case 's':
            edgePercent = atof(optarg);
            break;
        case 'd':
            g_debug = atoi(optarg);
            break; 
        case 't':
            optTest = atoi(optarg);
        }
    }

    /********************test parallel OM******************/
    if(optTest >= 20) {
        
        printf("********* Test Parallel OM !\n");
        printf("init size %d\n", optInsertNum); // ParOM::OM_SIZE
        printf("insert size %d\n", optInsertNum);
        printf("Lock Type: %d\n", g_lock_type);
        printf("\t 0 ***CAS_LOCK, 1 OMP_LOCK(default), 2 NO lock\n");

        vector<int> pos;
        bool ispar = true; 
        if (optWorkerNum < 1) ispar = false;

        {
            /*the insertnum = initial size**/
        //ParOM::OM *parom = new ParOM::OM( (optInsertNum * 2) + 10000);
        ParOM::OM *parom = new ParOM::OM( (optInsertNum * 2) + 10000);
        //vector<node_t> nodes = parom->AlocateNodes(optInsertNum); // ParOM::OM_SIZE
        vector<node_t> nodes = parom->AlocateNodes(optInsertNum); // ParOM::OM_SIZE
        parom->Init(nodes); // insert 1m initially.
        parom->InitParallel(optWorkerNum);
        nodes = parom->AlocateNodes(optInsertNum);

        

        if (temp == 1) {
            printf("*** with repeated random position!\n");
            vector<int> rand = parom->GetRepeatRandomNum(path);

            if (20 == optTest) {
                pos = parom->GenerateTestCase(ParOM::NO_RELABEL, rand, optInsertNum);
                printf("position size %d\n", ParOM::OM_POS_NO);
            } else if (21 == optTest) { //case 1 REPEAT_RANDOM
                pos = parom->GenerateTestCase(ParOM::FEW_RELABEL, rand, optInsertNum);
                printf("position size %d\n", ParOM::OM_POS_FEW);
            } else if (22 == optTest) { // case 2 FIXED_MULTIPLE
                pos = parom->GenerateTestCase(ParOM::MANY_RELABEL, rand, optInsertNum);
                printf("position size %d\n", ParOM::OM_POS_MANY);
            } else if (23 == optTest) { // case 3 FIXED_ONE 
                pos = parom->GenerateTestCase(ParOM::MAX_RELABEL, rand, optInsertNum);
                printf("position size %d\n", ParOM::OM_POS_MAX);
            } 
        } else if (temp == 2){   
 
            printf("*** with random position!\n");
            if (20 == optTest) {
                // insert 10m in to 10m positions. 
                pos = parom->GenerateTestCase2(ParOM::NO_RELABEL,  optInsertNum);
                printf("position size %d\n", ParOM::OM_POS_NO);
            } else if (21 == optTest) { //case 1 REPEAT_RANDOM
                // insert 10m into 1m positions. 
                pos = parom->GenerateTestCase2(ParOM::FEW_RELABEL,  optInsertNum);
                printf("position size %d\n", ParOM::OM_POS_FEW);
            } else if (22 == optTest) { // case 2 FIXED_MULTIPLE
                //insert 10m into 1000 positions.
                pos = parom->GenerateTestCase2(ParOM::MANY_RELABEL,  optInsertNum);
                printf("position size %d\n", ParOM::OM_POS_MANY);
            } else if (23 == optTest) { // case 3 FIXED_ONE 
                //insert 10m into 1 position.
                pos = parom->GenerateTestCase2(ParOM::MAX_RELABEL,  optInsertNum);
                printf("position size %d\n", ParOM::OM_POS_MAX);
            } 
        }

        //for debug
        // if (edgePercent = 1) {
        //     std::sort(pos.begin(), pos.end());
        // }

        auto test_beg = std::chrono::steady_clock::now();
        parom->TestInsert(pos, nodes, ispar);
        auto test_end = std::chrono::steady_clock::now();
        auto test_dif = test_end - test_beg;
        printf("TestInsert costs(ms): %f\n", std::chrono::duration<double, std::milli>(test_dif).count());
        printf("rebalance max tag #: %d\n\n", cnt_rebalance_max_tag);
        
        // test OM order 
        test_beg = std::chrono::steady_clock::now();
        parom->TestOrder(pos, ispar);
        test_end = std::chrono::steady_clock::now();
        test_dif = test_end - test_beg;
        printf("TestOrder costs(ms): %f\n\n", std::chrono::duration<double, std::milli>(test_dif).count());
        

        // test OM delete 
        /*delete the nodes that added firstly. 
        in this case, the delete can always be in parallel.*/
        test_beg = std::chrono::steady_clock::now();
        parom->TestDelete(nodes, ispar);
        test_end = std::chrono::steady_clock::now();
        test_dif = test_end - test_beg;
        printf("TestDelete costs(ms): %f\n\n", std::chrono::duration<double, std::milli>(test_dif).count());
        
        parom->PrintCount("");
        // test OM mixed, this has bugs. 
        //delete parom;
        }

#if 1  // test the mixed operations
        {
        ParOM::OM *parom = new ParOM::OM( (optInsertNum * 2) + 10000 ); // ParOM::OM_SIZE
        vector<node_t> nodes = parom->AlocateNodes(optInsertNum); //ParOM::OM_SIZE
        parom->Init(nodes); // insert 1m initially.
        parom->InitParallel(optWorkerNum);
        nodes = parom->AlocateNodes(optInsertNum);

        // if (21 == optTest) { //case 1 REPEAT_RANDOM
        //     pos = parom->GenerateTestCase(ParOM::REPEAT_RANDOM, rand, 3 * optInsertNum);
        //     printf("position size %d\n", optInsertNum * 3);
        // } else if (22 == optTest) { // case 2 FIXED_MULTIPLE
        //     pos = parom->GenerateTestCase(ParOM::FIXED_MULTIPLE, rand, 3 *  optInsertNum);
        //     printf("position size %d\n", ParOM::OM_POS_SIZE2);
        // } else if (23 == optTest) { // case 3 FIXED_ONE 
        //     pos = parom->GenerateTestCase(ParOM::FIXED_ONE, rand, 3 * optInsertNum);
        //     //printf("position size %d\n", ParOM::OM_POS_SIZE3);
        // }


        cnt_tag = 0;
        cnt_rebalance_max_tag = 0;
        auto test_beg = std::chrono::steady_clock::now();
        parom->TestMixed(pos, nodes, ispar);
        auto test_end = std::chrono::steady_clock::now();
        auto test_dif = test_end - test_beg;
        printf("TestMixed costs(ms): %f\n", std::chrono::duration<double, std::milli>(test_dif).count());
        printf("mixed rebalance max tag #: %d\n\n", cnt_rebalance_max_tag);
        
        parom->PrintCount("Mixed ");
        
#endif
        //delete(parom);
        exit(0);
        }
    }
    /********************test parallel OM END!******************/

    // read the graph
    int n, m, m2;
    std::vector<std::pair<int, int>> edges;
    if(1 == temp) {
        //const auto read_func = temp ? gadget::ReadTempEdgesS : gadget::ReadEdgesS;
        edges = gadget::ReadTempEdgesS(path, &n, &m);
    } else if (0 == temp) {
        edges = gadget::ReadEdgesS(path, &n, &m); // random shuffle the edges. 
        
    }  else if (2 == temp) {
        edges = gadget::ReadEdges(path, &n, &m); // without shuffle
        
    } else if (3 == temp ) {
            edges = gadget::CSRReadEdges(path, &n, &m, 1); // csr with shuffle
    } else if (4 == temp ) {
            edges = gadget::CSRReadEdges(path, &n, &m, 0); // csr without shuffle.
    } else if (5 == temp) {
            edges = gadget::CSRReadEdges(path, &n, &m, 2); // csr with repeated random shuffle.
    } else if (6 == temp) {
            edges = gadget::CSRReadEdges(path, &n, &m, 2); // csr with repeated random shuffle.       
    } else {
        printf("wrong graphs\n");
        exit(1);
    }

    //sample the edges by percentage
    if (edgePercent < 100) {
        edges = gadget::sampleEdges(path, edges, edgePercent);
        printf("edges size: %f\%\n", edgePercent);
    } else if (edgePercent >= 100000){
        if (edgePercent < m) {
            // edgepercent is the number of sampled edges.
            gadget::CutEdges(edges, edgePercent);
            printf("edges size: %d\n", edges.size());
        } else {
            // all edges
            edgePercent = m;
            printf("edges size: %d\n", edges.size());
        }
    } 
    m = edges.size();
  
  // get the number of edges for inserting or deleting.
  if (optInsertNum > 0) { m2 = m - optInsertNum;} 
  if (optRemoveNum > 0) { m2 = m;} //load the whole graph for removing.
  if (ratio > 0.0){ m2 = m - static_cast<int>(m * ratio);}

  // print the configurations
  gadget::RepeatWith('*', 80);
  printf("# of vertices: %d\n", n);
  printf("# of (all) edges: %d\n", m);
  printf("ratio: %f\n", ratio);
  printf("# of edges to insert: %d\n", optInsertNum);
  printf("# of edges to delete: %d\n", optRemoveNum);
  printf("path of the input file: %s\n", path);
  printf("method: %d  (2 glist, 1 traversal, or 3 ours, 4 ours parallel)\n", method);
  printf("graph: %d  (1 for temporal graphs, 0 for ordinary graphs sample edges, 2 for debug without sampled edges)\n", temp);
  printf("            3 csr sample edges, 4 csr without sample edges, 5 csr with repeated random\n");
  printf("# worker: %d \n", optWorkerNum);
  gadget::RepeatWith('*', 80);
    

    // graph: create the adjacent list representation
    //std::vector<std::vector<int>> graph(n);
  
    /* graph create by csr format 
    * for (i = begin(v); i++; i < end(v))*/
    /*graph csr end*/
    // for (int i = 0; i < m2; i++) {
    //     int v1 = edges[i].first;
    //     int v2 = edges[i].second;
    //     graph[v1].push_back(v2);
    //     graph[v2].push_back(v1);
    // }
    GRAPH graph = gadget::CreateOurCSRGRAPH(edges, n, m2);

    printf("creat adjacent list\n");

    // compute the base core
    std::vector<int> core(n);
    std::vector<int> order_v(n);
    std::vector<int> tmp_core(n);

    // initialize the core component
    core::CoreMaintenance* cm = nullptr;
    SeqCM::CoreMaint *ourcm = nullptr;
    ParCM::CoreMaint *ourparcm = nullptr;
    if (method%10 == 1) { //traversal
        cm = new core::Traversal(n);
    } else if (method%10 == 2) { //glist
        cm = new core::GLIST(n);
    } else if (method%10 == 3 ) {
        ourcm = new SeqCM::CoreMaint(n, graph, core);
    } else if (method == 4) {
        ourparcm = new ParCM::CoreMaint(n, graph, core);
    } else {
        printf("wrong method! %d\n", method);
        exit(0);
    }

    printf("new the working object\n");

   

#if 0  // output the core numbers. 
     //statistic the core numbers
    if (1 == outputCore) {
        for (auto e: edges) {
            graph[e.first].push_back(e.second);
            graph[e.second].push_back(e.first);
        }
        ourcm = new SeqCM::CoreMaint(n, graph, core);
        ourcm->ComputeCore(graph, core, order_v, false);
        gadget::OutputCoreNumber(path, core, n);
        goto END;
    } else if (2 == outputCore) {
        for (auto e: edges) {
            graph[e.first].push_back(e.second);
            graph[e.second].push_back(e.first);
        }
        ourcm = new SeqCM::CoreMaint(n, graph, core);
        ourcm->ComputeCore(graph, core, order_v, false);
        gadget::OutputSampleEdgeCoreNumber(path, core, edges, n);
        goto END;
    }
#endif // output the core numbers. 


    /*initialization*/
    printf("********* initialization!\n");
    const auto init_beg = std::chrono::steady_clock::now();
    if (method%10 != 3) { //3 is ours method 
        //cm->ComputeCore(graph, true, core);
    }
    if (method == 3){ // ours 
        ourcm->ComputeCore(core, order_v, true);       // compute the k-order by bz. 
        ourcm->Init(order_v);   // init to our class.
    } else if (method == 4){
        ourparcm->ComputeCore(core, order_v, true);       // compute the k-order by bz. 
        ourparcm->Init(order_v);   // init to our class.
        ourparcm->InitParallel(optWorkerNum);
    }
    const auto init_end = std::chrono::steady_clock::now();
    const auto init_dif = init_end - init_beg;
    printf("initialization costs(ms): %f\n", std::chrono::duration<double, std::milli>(init_dif).count());

    // verify the result, not necessary.
    // {
    //   if (method != 3) {
    //     cm->Check(graph, core);
    //   } else { // ours
    //     ourcm->ComputeCore(graph, tmp_core, order_v);
    //     ourcm->Check(-1, -1, -1, tmp_core, order_v);
    //   }
      
    // }

    if(1== optTest) {
        printf("********* Test OM!\n");
        vector<node_t> nodes = ourparcm->InitTestOM(100000);

        /* test the OM data structure */
        const auto test_beg = std::chrono::steady_clock::now();
    
        if (method == 3) {
            ourcm->CheckOM(true);
            ourcm->TestOM(64);
        } else if (method == 4) {
            //ourparcm->CheckOM(true);
            ourparcm->TestOM(nodes);
        }
        const auto test_end = std::chrono::steady_clock::now();
        const auto test_dif = test_end - test_beg;
        
        printf("TestOM costs(ms): %f\n", std::chrono::duration<double, std::milli>(test_dif).count());
        printf("update tag #: %d, rebalance max tag #: %d\n", cnt_tag, cnt_rebalance_max_tag);
        if (method == 4) {
            ourparcm->CheckOM(true);
        }
        goto ENDEXIT;
    }
    
    

    if (optRemoveNum > 0) goto REMOVE;


#if 1    // insert edge 
    printf("*********begin insertion %d %d!\n", m2, m);
    const auto ins_beg = std::chrono::steady_clock::now();
    
    if (3 == method ) { // new sequential
        for (int i = m2; i < m; ++i) {
            ourcm->EdgeInsert(edges[i].first, edges[i].second);
            if (g_debug) {
                // check each time. 
                ourcm->ComputeCore(tmp_core, order_v, false);
                //int r = ourcm->Check(edges[i].first, edges[i].second, i - m2, tmp_core, order_v);  
            }
        }
    } else if (4 == method) { // new parallel
        ourparcm->ParallelEdgeInsert(edges, m, m2);
    }
    
    /*
    if (method%10 != 3) {
        for (int i = m2; i < m; ++i) {
            //cm->Insert(edges[i].first, edges[i].second, graph, core);
            if(g_debug) {
            //cm->ComputeCore(graph, false, tmp_core);
            //ASSERT_INFO(tmp_core == core, "wrong result after insert");
            }
        }
    } else { // ours
            if (method/10 == 1) {
                // //int repeat = ourcm->BatchEdgeInsert(edges, m2, m); 
                //     if(g_debug) {
                //         // check each time. 
                //         ourcm->ComputeCore(graph, tmp_core, order_v, false);
                //         int r = ourcm->Check(-1, -1, -1, tmp_core, order_v);  
                //     }

                // printf("batch insertion repeat: %d\n", repeat);
            } else 
            {
                for (int i = m2; i < m; ++i) {
                    ourcm->EdgeInsert(edges[i].first, edges[i].second);
                    if (g_debug) {
                        // check each time. 
                        ourcm->ComputeCore(graph, tmp_core, order_v, false);
                        int r = ourcm->Check(edges[i].first, edges[i].second, i - m2, tmp_core, order_v);  
                    }
        }
            }
    }
    */


  const auto ins_end = std::chrono::steady_clock::now();
  const auto ins_dif = ins_end - ins_beg;
  //printf("core insert costs \x1b[1;31m%f\x1b[0m ms\n",
         //std::chrono::duration<double, std::milli>(ins_dif).count());
    printf("core IorR costs(ms): %f\n", std::chrono::duration<double, std::milli>(ins_dif).count());

  /*see count*/
    printf("temp edge number 1: %d\n", cnt_edge);
    printf("temp edge number 2: %d\n", cnt_edge2);
    
    printf("V* size: %d\n", cnt_Vs);
    printf("V+ size: %d\n", cnt_Vp);
    printf("S size: %d\n", cnt_S);
        printf("our adjust tag: %d\n", cnt_tag);
    printf("our max relabel tag: %d\n", cnt_rebalance_max_tag);
  /*see count end*/


  // verify the result
//   {
//     ERROR("check: insert", false);
    
//     if (method%10 != 3) {
//       cm->ComputeCore(graph, false, tmp_core);
//       cm->Check(graph, core);
//     }else {
//       ourcm->ComputeCore(graph, tmp_core, order_v, false);
//       ourcm->Check(-1, -1, -1, tmp_core, order_v);
//     }

  
//     ERROR("check passed", false);

//   }c
goto END;
#endif // insert edge. 

REMOVE:
#if 1   // remove edges
  printf("begin remove !\n");
  if (optRemoveNum > 0) { m2 = m - optRemoveNum;} 

    const auto rmv_beg = std::chrono::steady_clock::now();
  
    if (method%10 != 3) {
        for (int i = m - 1; i >= m2; --i) {
            //cm->Remove(edges[i].first, edges[i].second, graph, core);
        }
    } else {
      for (int i = m - 1; i >= m2; --i) {
        ourcm->EdgeRemove(edges[i].first, edges[i].second);
        if (g_debug) {
            ourcm->ComputeCore(tmp_core, order_v, false);
            int result = ourcm->CheckCore(tmp_core, true);
            if (0  == result) ERROR("check core number passed", false);
            else ERROR("check core number fail!!!", false);
            ourcm->CheckAllMCDValue(true);
        }
      }
    }
  const auto rmv_end = std::chrono::steady_clock::now();
  const auto rmv_dif = rmv_end - rmv_beg;
  //printf("core remove costs \x1b[1;31m%f\x1b[0m ms\n",
        // std::chrono::duration<double, std::milli>(rmv_dif).count());
  printf("core IorR costs(ms): %f\n", std::chrono::duration<double, std::milli>(rmv_dif).count());  

  /*see count*/
    printf("temp edge number 1: %d\n", cnt_edge);
    printf("temp edge number 2: %d\n", cnt_edge2);
    
    printf("V* size: %d\n", cnt_Vs);
    printf("V+ size: %d\n", cnt_Vp);
    printf("total visited edge S size: %d\n", cnt_S);
    printf("our adjust tag: %d\n", cnt_tag);
    printf("our max relabel tag: %d\n", cnt_rebalance_max_tag);
  /*see count end*/

#endif // end remove

END:
    // verify the result for both insertion and remove
    {
        int result = 0;
        if (method%10 != 3) {
        //   std::vector<int> tmp_core(n);
        //   cm->ComputeCore(graph, false, tmp_core);
        //   ASSERT_INFO(tmp_core == core, "wrong result after remove");
        //   cm->Check(graph, core);
        } else {
            ourcm->ComputeCore(tmp_core, order_v, false);
            result = ourcm->CheckCore(tmp_core, false);
            //ourcm->Check(-1, -1, -1, tmp_core, order_v);
        }
        if (0  == result) ERROR("check core number passed", false);
        else ERROR("check core number fail!!!", false);
    }

    gadget::RepeatWith('*', 80);
    printf("# of om  order: %d\n", cnt_omorder);
    printf("# of om insert: %d\n", cnt_ominsert);
    printf("# of om insert mid: %d\n", cnt_ominsert_mid);
    printf("# of om delete: %d\n", cnt_omdelete);
    printf("# of om splite: %d\n", cnt_omsplit);
    gadget::RepeatWith('*', 80);



ENDEXIT:
    if (cm != nullptr) delete cm;
    if (ourcm != nullptr) delete ourcm;
    if (ourparcm != nullptr) delete ourparcm;

}
