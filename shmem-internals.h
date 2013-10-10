/* BSD-2 License.  Written by Jeff Hammond. */

#ifndef SHMEM_INTERNALS_H
#define SHMEM_INTERNALS_H

#include "shmem.h"

/* configuration settings */
/* This is the only support mode right now. */
#define USE_ORDERED_RMA
/* This should always be set unless your MPI sucks. */
#define USE_ALLREDUCE
/* Not implemented yet. */
//#define USE_SMP_OPTIMIZATIONS

#if ( defined(__GNUC__) && (__GNUC__ >= 3) ) || defined(__IBMC__) || defined(__INTEL_COMPILER) || defined(__clang__)
#  define unlikely(x_) __builtin_expect(!!(x_),0)
#  define likely(x_)   __builtin_expect(!!(x_),1)
#else
#  define unlikely(x_) (x_)
#  define likely(x_)   (x_)
#endif

/*****************************************************************/
/* TODO convert all the global status into a struct ala ARMCI-MPI */
/* requires TLS if MPI is thread-based */
MPI_Comm  SHMEM_COMM_WORLD;
MPI_Group SHMEM_GROUP_WORLD; /* used for creating logpe comms */

int       shmem_is_initialized;
int       shmem_is_finalized;
int       shmem_world_size, shmem_world_rank;
char      shmem_procname[MPI_MAX_PROCESSOR_NAME];

#ifdef USE_SMP_OPTIMIZATIONS
MPI_Comm  SHMEM_COMM_NODE;
MPI_Group SHMEM_GROUP_NODE; /* may not be needed as global */
int       shmem_world_is_smp;
int       shmem_node_size, shmem_node_rank;
int *     shmem_smp_rank_list;
#endif

/* TODO probably want to make these 5 things into a struct typedef */
MPI_Win shmem_etext_win;
int     shmem_etext_is_symmetric;
int     shmem_etext_size;
void *  shmem_etext_mybase_ptr;
void ** shmem_etext_base_ptrs;

MPI_Win shmem_sheap_win;
int     shmem_sheap_is_symmetric;
//int     shmem_sheap_size;
int     shmem_sheap_size;
void *  shmem_sheap_mybase_ptr;
void ** shmem_sheap_base_ptrs;
//void *  shmem_sheap_current_ptr;
void *  shmem_sheap_current_ptr;
/*****************************************************************/

enum shmem_window_id_e { SHMEM_SHEAP_WINDOW = 0, SHMEM_ETEXT_WINDOW = 1, SHMEM_INVALID_WINDOW = -1 };
enum shmem_rma_type_e  { SHMEM_PUT = 0, SHMEM_GET = 1, SHMEM_IPUT = 2, SHMEM_IGET = 4};
enum shmem_amo_type_e  { SHMEM_SWAP = 0, SHMEM_CSWAP = 1, SHMEM_ADD = 2, SHMEM_FADD = 4};
enum shmem_coll_type_e { SHMEM_BARRIER = 0, SHMEM_BROADCAST = 1, SHMEM_ALLREDUCE = 2, SHMEM_ALLGATHER = 4, SHMEM_ALLGATHERV = 8};

void __shmem_warn(char * message);

void __shmem_abort(int code, char * message);

int __shmem_address_is_symmetric(size_t my_sheap_base_ptr);

void __shmem_initialize(void);

void __shmem_finalize(void);

void __shmem_remote_sync(void);

void __shmem_local_sync(void);

/* return 0 on successful lookup, otherwise 1 */
int __shmem_window_offset(const void *address, const int pe,
                          enum shmem_window_id_e * win_id, shmem_offset_t * win_offset);     

void __shmem_rma(enum shmem_rma_type_e rma, MPI_Datatype mpi_type,
                 void *target, const void *source, size_t len, int pe);

void __shmem_rma_strided(enum shmem_rma_type_e rma, MPI_Datatype mpi_type,
                         void *target, const void *source, 
                         ptrdiff_t target_ptrdiff, ptrdiff_t source_ptrdiff, 
                         size_t len, int pe);

void __shmem_amo(enum shmem_amo_type_e amo, MPI_Datatype mpi_type,
                 void *output, void *remote, 
                 const void *input, const void *compare,
                 int pe);

void __shmem_create_comm(int pe_start, int log_pe_stride, int pe_size,
                         MPI_Comm * comm, MPI_Group * strided_group);

void __shmem_coll(enum shmem_coll_type_e coll, MPI_Datatype mpi_type, MPI_Op reduce_op,
                  void * target, const void * source, size_t len, 
                  int pe_root, int pe_start, int log_pe_stride, int pe_size);

#endif // SHMEM_INTERNALS_H
