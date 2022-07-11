
#ifndef CONFIG_H
#define CONFIG_H


#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include "../jalib/jassert.h"
#include "../jalib/jconvert.h"
#include "constants.h"

#define GLOBAL_CKPT_DIR_OPTION "global_ckpt_dir"
#define LOCAL_CKPT_DIR_OPTION "local_ckpt_dir"
#define GLOBAL_CKPT_INT_OPTION "global_ckpt_interval"
#define SOLOMON_CKPT_INT_OPTION "solomon_ckpt_interval"
#define PARTNER_CKPT_INT_OPTION "partner_ckpt_interval"
#define LOCAL_CKPT_INT_OPTION "local_ckpt_interval"
#define TEST_MODE_OPTION "test_mode"
#define NODE_SIZE_OPTION "node_size"
#define GROUP_SIZE_OPTION "group_size"
#define ENC_MAX_THREADS_OPTION "encode_max_threads"
#define ENC_BLOCKS_PER_THREAD_OPTION "encode_blocks_per_thread"

using namespace dmtcp;

struct ConfigInfo {
  public:
    std::string globalCkptDir;
    std::string localCkptDir;
    uint32_t globalInterval;
    uint32_t solomonInterval;
    uint32_t partnerInterval;
    uint32_t localInterval;
    uint32_t testMode;
    int32_t nodeSize;
    int32_t groupSize;
    int encodeMaxThreads;
    int encodeBlocksPerThread;

    ConfigInfo();
    void readConfigFromFile(std::string filename);
};

struct Topology {
  public:
    int numNodes;
    char *nameList, *hostname;
    int *nodeMap, *partnerMap;
    int numProc;
    int nodeSize, groupSize;
    // Identifies the group of nodes to which a process belongs.
    int sectionID;
    // The rank of the process in the group communicator
    int groupRank;
    // Identify the right and left processes in the group communicator.
    int right, left;
    // whether the topology is faked or is running on a real cluster
    int testMode;

    MPI_Comm groupComm;

    Topology(int test_node, int num_nodes, char *name_list,
             char *host_name, int *node_map, int *partner_map,
             int num_proc, int node_size, int group_size,
             int section_ID, int group_rank, int righ_t,
             int lef_t, MPI_Comm group_comm);
};

struct RestartInfo {
  public:
    string ckptDir[CKPT_GLOBAL+1];
    uint64_t ckptTime[CKPT_GLOBAL+1];

    RestartInfo();
    void update(string ckptDir, int ckptType, uint64_t currTime);
    void writeRestartInfo();
    int readRestartInfo();
};

#endif // ifndef CONFIG_H