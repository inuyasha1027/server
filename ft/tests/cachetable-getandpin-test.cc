/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
#ident "$Id$"
#ident "Copyright (c) 2007-2012 Tokutek Inc.  All rights reserved."
#ident "The technology is licensed by the Massachusetts Institute of Technology, Rutgers State University of New Jersey, and the Research Foundation of State University of New York at Stony Brook under United States of America Serial No. 11/760379 and to the patents and/or patent applications resulting from it."
#include "includes.h"
#include "test.h"

static void
flush (CACHEFILE cf     __attribute__((__unused__)),
       int UU(fd),
       CACHEKEY key     __attribute__((__unused__)),
       void *v          __attribute__((__unused__)),
       void** UU(dd),
       void *extraargs  __attribute__((__unused__)),
       PAIR_ATTR size        __attribute__((__unused__)),
       PAIR_ATTR* new_size      __attribute__((__unused__)),
       bool write_me    __attribute__((__unused__)),
       bool keep_me     __attribute__((__unused__)),
       bool for_checkpoint    __attribute__((__unused__)),
        bool UU(is_clone), bool UU(aggressive)
       ) {
    assert((long) key.b == size.size);
    if (!keep_me) toku_free(v);
}

static int
fetch (
    CACHEFILE UU(cf), 
    PAIR UU(p),
    int UU(fd), 
    CACHEKEY key, 
    uint32_t UU(hash), 
    void **vptr, 
    void** UU(dd),
    PAIR_ATTR *sizep, 
    int *dirtyp, 
    void *UU(extra)
    ) 
{
    *sizep = make_pair_attr((long) key.b);
    *vptr = toku_malloc(sizep->size);
    *dirtyp = 0;
    return 0;
}

static void
cachetable_getandpin_test (int n) {
    const int test_limit = 1024*1024;
    int r;
    CACHETABLE ct;
    r = toku_create_cachetable(&ct, test_limit, ZERO_LSN, NULL_LOGGER); assert(r == 0);
    char fname1[] = __SRCFILE__ "test_getandpin.dat";
    unlink(fname1);
    CACHEFILE f1;
    r = toku_cachetable_openf(&f1, ct, fname1, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO); assert(r == 0);

    int i;

    // test get_and_pin size
    for (i=1; i<=n; i++) {
        uint32_t hi;
        hi = toku_cachetable_hash(f1, make_blocknum(i));
        void *v; long size;
        CACHETABLE_WRITE_CALLBACK wc = def_write_callback(NULL);
        wc.flush_callback = flush;
        r = toku_cachetable_get_and_pin(f1, make_blocknum(i), hi, &v, &size, wc, fetch, def_pf_req_callback, def_pf_callback, true, 0);
        assert(r == 0);
        assert(size == i);

        r = toku_test_cachetable_unpin(f1, make_blocknum(i), hi, CACHETABLE_CLEAN, make_pair_attr(i));
        assert(r == 0);
    }
    toku_cachetable_verify(ct);

    r = toku_cachefile_close(&f1, 0, false, ZERO_LSN); assert(r == 0);
    r = toku_cachetable_close(&ct); assert(r == 0 && ct == 0);
}

int
test_main(int argc, const char *argv[]) {
    default_parse_args(argc, argv);
    cachetable_getandpin_test(8);
    return 0;
}