#ifndef _CACHE_H
#define _CACHE_H

void DcacheWriteBackRange(unsigned long start, unsigned long end);
void DcacheInvalidateRange(unsigned long start, unsigned long end);
void DcacheWriteBackInvalidateRange(unsigned long start, unsigned long end);
void IcacheInvalidateRange(unsigned long start, unsigned long end);
void FlushDataCacheAll(void);
void AwosArchCleanFlushDataCache(void);
void InvalidateDataCache(void);
void FlushInstructionCacheAll(void);
void InvalidateInstructionCacheRegion(unsigned long start, unsigned int len);
void AwosArchMemsFlushInstructionCacheRegion(unsigned long start, unsigned long len);
void FlushDataCacheRegion(unsigned long start, unsigned long len);
void AwosArchMemsCleanFlushDataCacheRegion(unsigned long start, unsigned long len);
void InvalidateDataCacheRegion(unsigned long start, unsigned long len);
void AwosArchCleanFlushCache(void);
void AwosArchCleanFlushCacheRegion(unsigned long start, unsigned long len);
void AwosArchFlushCache(void);
void CacheInitialize(void);
void DataCacheEnable(void);
void InstructionCacheEnable(void);

#endif
