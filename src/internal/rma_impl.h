/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2018 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */

#ifndef INTERNAL_RMA_IMPL_H
#define INTERNAL_RMA_IMPL_H

#include "oshmpi_impl.h"

static inline void OSHMPI_translate_win_and_disp(const void *abs_addr, MPI_Win * win_ptr,
                                                 MPI_Aint * disp_ptr)
{
    /* heap */
    if (OSHMPI_global.symm_heap_base <= abs_addr &&
        (MPI_Aint) abs_addr <= (MPI_Aint) OSHMPI_global.symm_heap_base +
        OSHMPI_global.symm_heap_size) {
        *disp_ptr = (MPI_Aint) OSHMPI_global.symm_heap_base - (MPI_Aint) abs_addr;
        *win_ptr = OSHMPI_global.symm_heap_win;
    }
}

static inline void ctx_local_complete_impl(shmem_ctx_t ctx OSHMPI_ATTRIBUTE((unused)),
                                           int pe, MPI_Win win)
{
    OSHMPI_CALLMPI(MPI_Win_flush_local(pe, win));
}

static inline void ctx_put_nbi_impl(shmem_ctx_t ctx OSHMPI_ATTRIBUTE((unused)),
                                    MPI_Datatype mpi_type, const void *origin_addr,
                                    void *target_addr, size_t nelems, int pe, MPI_Win * win_ptr)
{
    MPI_Aint target_disp = -1;
    MPI_Win win = MPI_WIN_NULL;

    OSHMPI_translate_win_and_disp((const void *) target_addr, &win, &target_disp);
    OSHMPI_ASSERT(target_disp >= 0 && win != MPI_WIN_NULL);

    OSHMPI_CALLMPI(MPI_Put(origin_addr, nelems, mpi_type, pe, target_disp, nelems, mpi_type, win));

    /* return window object if the caller requires */
    if (win_ptr != NULL)
        *win_ptr = win;
}

static inline void ctx_get_nbi_impl(shmem_ctx_t ctx OSHMPI_ATTRIBUTE((unused)),
                                    MPI_Datatype mpi_type, void *origin_addr,
                                    const void *target_addr, size_t nelems, int pe,
                                    MPI_Win * win_ptr)
{
    MPI_Aint target_disp = -1;
    MPI_Win win = MPI_WIN_NULL;

    OSHMPI_translate_win_and_disp((const void *) target_addr, &win, &target_disp);
    OSHMPI_ASSERT(target_disp >= 0 && win != MPI_WIN_NULL);

    OSHMPI_CALLMPI(MPI_Get(origin_addr, nelems, mpi_type, pe, target_disp, nelems, mpi_type, win));

    /* return window object if the caller requires */
    if (win_ptr != NULL)
        *win_ptr = win;
}

static inline void OSHMPI_ctx_put_nbi(shmem_ctx_t ctx OSHMPI_ATTRIBUTE((unused)),
                                      MPI_Datatype mpi_type, const void *origin_addr,
                                      void *target_addr, size_t nelems, int pe)
{
    ctx_put_nbi_impl(ctx, mpi_type, origin_addr, target_addr, nelems, pe, NULL);
}

static inline void OSHMPI_ctx_put(shmem_ctx_t ctx OSHMPI_ATTRIBUTE((unused)),
                                  MPI_Datatype mpi_type, const void *origin_addr,
                                  void *target_addr, size_t nelems, int pe)
{
    MPI_Win win = MPI_WIN_NULL;

    ctx_put_nbi_impl(ctx, mpi_type, origin_addr, target_addr, nelems, pe, &win);
    ctx_local_complete_impl(ctx, pe, win);
}

static inline void OSHMPI_ctx_get_nbi(shmem_ctx_t ctx OSHMPI_ATTRIBUTE((unused)),
                                      MPI_Datatype mpi_type, void *origin_addr,
                                      const void *target_addr, size_t nelems, int pe)
{
    ctx_get_nbi_impl(ctx, mpi_type, origin_addr, target_addr, nelems, pe, NULL);
}

static inline void OSHMPI_ctx_get(shmem_ctx_t ctx OSHMPI_ATTRIBUTE((unused)),
                                  MPI_Datatype mpi_type, void *origin_addr,
                                  const void *target_addr, size_t nelems, int pe)
{
    MPI_Win win = MPI_WIN_NULL;

    ctx_get_nbi_impl(ctx, mpi_type, origin_addr, target_addr, nelems, pe, &win);
    ctx_local_complete_impl(ctx, pe, win);
}

#endif /* INTERNAL_RMA_IMPL_H */