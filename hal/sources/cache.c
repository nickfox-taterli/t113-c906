#include "cache.h"
#include "csr.h"

#define L1_CACHE_BYTES (64)

void DcacheWriteBackRange(unsigned long start, unsigned long end)
{
    unsigned long i = start & ~(L1_CACHE_BYTES - 1);

    for (; i < end; i += L1_CACHE_BYTES)
    {
        asm volatile("dcache.cpa %0\n"::"r"(i):"memory");
    }
    asm volatile(".long 0x01b0000b");
}

void DcacheInvalidateRange(unsigned long start, unsigned long end)
{
    unsigned long i = start & ~(L1_CACHE_BYTES - 1);

    for (; i < end; i += L1_CACHE_BYTES)
    {
        asm volatile("dcache.ipa %0\n"::"r"(i):"memory");
    }
    asm volatile(".long 0x01b0000b");
}

void DcacheWriteBackInvalidateRange(unsigned long start, unsigned long end)
{
    unsigned long i = start & ~(L1_CACHE_BYTES - 1);

    for (; i < end; i += L1_CACHE_BYTES)
    {
        asm volatile("dcache.cipa %0\n"::"r"(i):"memory");
    }
    asm volatile(".long 0x01b0000b");
}

void IcacheInvalidateRange(unsigned long start, unsigned long end)
{
    unsigned long i = start & ~(L1_CACHE_BYTES - 1);

    for (; i < end; i += L1_CACHE_BYTES)
    {
        asm volatile("icache.ipa %0\n"::"r"(i):"memory");
    }
    asm volatile(".long 0x01b0000b");
}

void FlushDataCacheAll(void)
{
    asm volatile("dcache.call\n":::"memory");
}

void AwosArchCleanFlushDataCache(void)
{
    asm volatile("dcache.ciall\n":::"memory");
}

void InvalidateDataCache(void)
{
    asm volatile("dcache.iall\n":::"memory");
}

void FlushInstructionCacheAll(void)
{
    asm volatile("icache.iall\n":::"memory");
}

void InvalidateInstructionCacheRegion(unsigned long start, unsigned int len)
{
    IcacheInvalidateRange(start, start + len);
}

void AwosArchMemsFlushInstructionCacheRegion(unsigned long start, unsigned long len)
{
    IcacheInvalidateRange(start, start + len);
}

void FlushDataCacheRegion(unsigned long start, unsigned long len)
{
    DcacheWriteBackRange(start, start + len);
}

void AwosArchMemsCleanFlushDataCacheRegion(unsigned long start, unsigned long len)
{
    DcacheWriteBackInvalidateRange(start, start + len);
}

void InvalidateDataCacheRegion(unsigned long start, unsigned long len)
{
    DcacheInvalidateRange(start, start + len);
}

void AwosArchCleanFlushCache(void)
{
	InvalidateDataCache();
	FlushInstructionCacheAll();
}

void AwosArchCleanFlushCacheRegion(unsigned long start, unsigned long len)
{
    AwosArchMemsCleanFlushDataCacheRegion(start, len);
    AwosArchMemsFlushInstructionCacheRegion(start, len);
}

void AwosArchFlushCache(void)
{
	InvalidateDataCache();
	FlushInstructionCacheAll();
}

void CacheInitialize(void)
{
	/*
	(0:1) When CACHE_SEL=2'b11, select instruction and data cache
	(4) When INV=1, perform cache invalidation
	(16) When BHT_INV=1, invalidate data in branch history table
	(17) When TB_INV=1, invalidate data in branch target buffer
	*/
	csr_write(CSR_MCOR, 0x70013);

	/*
	(0) When IE=1, Icache is enabled
	(1) When DE=1, Dcache is enabled
	(2) When WA=1, data cache is in write allocate mode (not supported by c906)
	(3) When WB=1, data cache is in write-back mode (c906 fixed to 1)
	(4) When RS=1, return stack is enabled
	(5) When BPE=1, branch prediction is enabled
	(6) When BTB=1, branch target prediction is enabled
	(8) When WBR=1, write burst transfer is supported (c906 fixed to 1)
	(12) When L0BTB=1, level 1 branch target prediction is enabled
	*/
	csr_set(CSR_MHCR, 0x11fc);


	/*
	(15) When MM=1, support unaligned access, hardware handles unaligned access
	(16) When UCME=1, user mode can execute extended cache operation instructions
	(17) When CLINTEE=1, supervisor software interrupts and timer interrupts from CLINT can be responded to
	(21) When MAEE=1, extended address attribute bits in MMU PTE, user can configure page address attributes
	(22) When THEADISAEE=1, C906 extended instruction set can be used
	*/
	csr_set(CSR_MXSTATUS, 0x638000);


	/*
	(2) When DPLD=1, dcache prefetch is enabled
	(3,4,5,6,7) When AMR=1, after 3 consecutive cache line store operations, subsequent consecutive address store operations will not write to L1 Cache
	(8) When IPLD=1, ICACHE prefetch is enabled
	(9) When LPE=1, loop acceleration is enabled
	(13,14) When DPLD is 2, prefetch 8 cache lines
	*/
	csr_write(CSR_MHINT, 0x16e30c);
}

void DataCacheEnable(void)
{
    /*
    (1) When DE=1, Dcache is enabled
    */
    csr_set(CSR_MHCR, 0x2);
}

void InstructionCacheEnable(void)
{
    /*
    (0) When IE=1, Icache is enabled
    */
    csr_set(CSR_MHCR, 0x1);
}
