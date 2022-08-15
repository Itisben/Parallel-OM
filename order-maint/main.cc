#include <unistd.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>


#include "defs.h"
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
    int temp = 2;   // temporal graph 
    float edgePercent = 100;
    int optTest = 0;
    int optWorkerNum = 0;

    if(argc < 2) {

        printf("************ OM data structure**************\n");
        printf("-I: the total number nodes for the ordered list\n");
        printf("-w: the number of workers (1 - 64)\n");
        printf("-l: the type of lock, 0 CAS_LOCK (default), 1 OMP_LOCK, 2 NO lock\n");
        printf("-t: 20 No relabel, 21 Few relabel, 22 Many relabel, 23 Max relabel\n"); 
        //printf("for example: ./core -I 10000000 -t 20 -w 16\n");
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

}
