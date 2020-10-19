#define KHz(hz) ((hz)*1000ULL)
#define MHz(hz) KHz(KHz(hz))

#ifdef __intel__
    static inline uint64_t get_dtime(void) {
        uint32_t hi, lo;
        
        __asm__ __volatile__ ("xorl %%eax,%%edx\n" : : : "%eax", "%edx");
        __asm__ __volatile__ ("xorl %%eax,%%edx\n" : : : "%eax", "%edx");
        __asm__ __volatile__ ("xorl %%eax,%%edx\n" : : : "%eax", "%edx");
        __asm__ __volatile__ ("xorl %%eax,%%edx\n" : : : "%eax", "%edx");
        __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
        
        return(((uint64_t)hi << 32) | (uint64_t)lo);
    )
#else
    static inline uint64_t get_dtime(void) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);

        return((ts.tv_sec * 1E9) + ts.tv_nsec);
    }
#endif
    
uint64_t dtime_calibrate(void);
