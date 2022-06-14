/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TD_VNODE_TSDB_H_
#define _TD_VNODE_TSDB_H_

#include "vnodeInt.h"

#ifdef __cplusplus
extern "C" {
#endif

// tsdbDebug ================
// clang-format off
#define tsdbFatal(...) do { if (tsdbDebugFlag & DEBUG_FATAL) { taosPrintLog("TSDB FATAL ", DEBUG_FATAL, 255, __VA_ARGS__); }}     while(0)
#define tsdbError(...) do { if (tsdbDebugFlag & DEBUG_ERROR) { taosPrintLog("TSDB ERROR ", DEBUG_ERROR, 255, __VA_ARGS__); }}     while(0)
#define tsdbWarn(...)  do { if (tsdbDebugFlag & DEBUG_WARN)  { taosPrintLog("TSDB WARN ", DEBUG_WARN, 255, __VA_ARGS__); }}       while(0)
#define tsdbInfo(...)  do { if (tsdbDebugFlag & DEBUG_INFO)  { taosPrintLog("TSDB ", DEBUG_INFO, 255, __VA_ARGS__); }}            while(0)
#define tsdbDebug(...) do { if (tsdbDebugFlag & DEBUG_DEBUG) { taosPrintLog("TSDB ", DEBUG_DEBUG, tsdbDebugFlag, __VA_ARGS__); }} while(0)
#define tsdbTrace(...) do { if (tsdbDebugFlag & DEBUG_TRACE) { taosPrintLog("TSDB ", DEBUG_TRACE, tsdbDebugFlag, __VA_ARGS__); }} while(0)
// clang-format on
typedef struct TSDBROW       TSDBROW;
typedef struct TSDBKEY       TSDBKEY;
typedef struct TABLEID       TABLEID;
typedef struct SDelOp        SDelOp;
typedef struct SDelDataItem  SDelDataItem;
typedef struct SDelData      SDelData;
typedef struct SDelIdxItem   SDelIdxItem;
typedef struct SDelIdx       SDelIdx;
typedef struct STbData       STbData;
typedef struct SMemTable     SMemTable;
typedef struct STbDataIter   STbDataIter;
typedef struct SMergeInfo    SMergeInfo;
typedef struct STable        STable;
typedef struct SOffset       SOffset;
typedef struct SMapData      SMapData;
typedef struct SColData      SColData;
typedef struct SColDataBlock SColDataBlock;
typedef struct SBlockSMA     SBlockSMA;

// tsdbMemTable ==============================================================================================

// SMemTable
int32_t tsdbMemTableCreate(STsdb *pTsdb, SMemTable **ppMemTable);
void    tsdbMemTableDestroy(SMemTable *pMemTable);
void    tsdbGetTbDataFromMemTable(SMemTable *pMemTable, tb_uid_t suid, tb_uid_t uid, STbData **ppTbData);

// STbDataIter
int32_t tsdbTbDataIterCreate(STbData *pTbData, TSDBKEY *pFrom, int8_t backward, STbDataIter **ppIter);
void   *tsdbTbDataIterDestroy(STbDataIter *pIter);
bool    tsdbTbDataIterOpen(STbData *pTbData, TSDBKEY *pFrom, int8_t backward, STbDataIter *pIter);
bool    tsdbTbDataIterNext(STbDataIter *pIter);
bool    tsdbTbDataIterGet(STbDataIter *pIter, TSDBROW *pRow);

// tsdbFile.c ==============================================================================================
typedef struct SDelFile       SDelFile;
typedef struct STsdbCacheFile STsdbCacheFile;
typedef struct STsdbIndexFile STsdbIndexFile;
typedef struct STsdbDataFile  STsdbDataFile;
typedef struct STsdbLastFile  STsdbLastFile;
typedef struct STsdbSmaFile   STsdbSmaFile;
typedef struct STsdbSmalFile  STsdbSmalFile;
typedef struct SDFileSet      SDFileSet;

// tsdbFS.c ==============================================================================================
typedef struct STsdbFS STsdbFS;

int32_t tsdbFSOpen(STsdb *pTsdb, STsdbFS **ppFS);
int32_t tsdbFSClose(STsdbFS *pFS);
int32_t tsdbFSStart(STsdbFS *pFS);
int32_t tsdbFSEnd(STsdbFS *pFS, int8_t rollback);

// tsdbReaderWriter.c ==============================================================================================

// SDataFWriter
typedef struct SDataFWriter SDataFWriter;

int32_t tsdbDataFWriterOpen(SDataFWriter *pWriter, STsdb *pTsdb, SDFileSet *pSet);
int32_t tsdbDataFWriterClose(SDataFWriter *pWriter);
int32_t tsdbWriteBlockIdx(SDataFWriter *pWriter, SMapData *pMapData, uint8_t **ppBuf);
int32_t tsdbWriteBlock(SDataFWriter *pWriter, SMapData *pMapData, uint8_t **ppBuf, int64_t *rOffset, int64_t *rSize);
int32_t tsdbWriteBlockData(SDataFWriter *pWriter, SColDataBlock *pBlockData, uint8_t **ppBuf, int64_t *rOffset,
                           int64_t *rSize);
int32_t tsdbWriteBlockSMA(SDataFWriter *pWriter, SBlockSMA *pBlockSMA, int64_t *rOffset, int64_t *rSize);

// SDataFReader
typedef struct SDataFReader SDataFReader;

int32_t tsdbDataFReaderOpen(SDataFReader *pReader, STsdb *pTsdb, SDFileSet *pSet);
int32_t tsdbDataFReaderClose(SDataFReader *pReader);
int32_t tsdbReadBlockIdx(SDataFReader *pReader, SMapData *pMapData, uint8_t **ppBuf);
int32_t tsdbReadBlock(SDataFReader *pReader, SBlockIdx *pBlockIdx, SMapData *pMapData, uint8_t **ppBuf);
int32_t tsdbReadBlockData(SDataFReader *pReader, SBlock *pBlock, SColDataBlock *pBlockData, uint8_t **ppBuf);
int32_t tsdbReadBlockSMA(SDataFReader *pReader, SBlockSMA *pBlkSMA);

// SDelFWriter
typedef struct SDelFWriter SDelFWriter;

int32_t tsdbDelFWriterOpen(SDelFWriter **ppWriter, SDelFile *pFile, STsdb *pTsdb);
int32_t tsdbDelFWriterClose(SDelFWriter *pWriter, int8_t sync);
int32_t tsdbWriteDelData(SDelFWriter *pWriter, SDelData *pDelData, uint8_t **ppBuf, int64_t *rOffset, int64_t *rSize);
int32_t tsdbWriteDelIdx(SDelFWriter *pWriter, SDelIdx *pDelIdx, uint8_t **ppBuf);
int32_t tsdbUpdateDelFileHdr(SDelFWriter *pWriter, uint8_t **ppBuf);

// SDelFReader
typedef struct SDelFReader SDelFReader;

int32_t tsdbDelFReaderOpen(SDelFReader **ppReader, SDelFile *pFile, STsdb *pTsdb, uint8_t **ppBuf);
int32_t tsdbDelFReaderClose(SDelFReader *pReader);
int32_t tsdbReadDelData(SDelFReader *pReader, SDelIdxItem *pItem, SDelData *pDelData, uint8_t **ppBuf);
int32_t tsdbReadDelIdx(SDelFReader *pReader, SDelIdx *pDelIdx, uint8_t **ppBuf);

// SCacheFWriter
typedef struct SCacheFWriter SCacheFWriter;

// SCacheFReader
typedef struct SCacheFReader SCacheFReader;

// tsdbCommit.c ==============================================================================================

// tsdbReadImpl.c ==============================================================================================
typedef struct SBlockIdxItem SBlockIdxItem;
typedef struct SBlockIdx     SBlockIdx;
typedef struct SBlockInfo    SBlockInfo;
typedef struct SBlock        SBlock;
typedef struct SBlockCol     SBlockCol;
typedef struct SBlockStatis  SBlockStatis;
typedef struct SAggrBlkCol   SAggrBlkCol;
typedef struct SBlockData    SBlockData;
typedef struct SReadH        SReadH;

// tsdbUtil.c ==============================================================================================
int32_t tsdbKeyFid(TSKEY key, int32_t minutes, int8_t precision);
void    tsdbFidKeyRange(int32_t fid, int32_t minutes, int8_t precision, TSKEY *minKey, TSKEY *maxKey);

int32_t tsdbRealloc(uint8_t **ppBuf, int64_t size);
void    tsdbFree(uint8_t *pBuf);

int32_t tTABLEIDCmprFn(const void *p1, const void *p2);
int32_t tsdbKeyCmprFn(const void *p1, const void *p2);

// SMapData
void    tMapDataReset(SMapData *pMapData);
void    tMapDataClear(SMapData *pMapData);
int32_t tMapDataPutItem(SMapData *pMapData, void *pItem, int32_t (*tPutItemFn)(uint8_t *, void *));
int32_t tMapDataGetItemByIdx(SMapData *pMapData, int32_t idx, void *pItem, int32_t (*tGetItemFn)(uint8_t *, void *));
int32_t tPutMapData(uint8_t *p, SMapData *pMapData);
int32_t tGetMapData(uint8_t *p, SMapData *pMapData);

// SBlockIdx
int32_t tBlockIdxClear(SBlockIdx *pBlockIdx);
int32_t tBlockIdxPutItem(SBlockIdx *pBlockIdx, SBlockIdxItem *pItem);
int32_t tBlockIdxGetItemByIdx(SBlockIdx *pBlockIdx, SBlockIdxItem *pItem, int32_t idx);
int32_t tBlockIdxGetItem(SBlockIdx *pBlockIdx, SBlockIdxItem *pItem, TABLEID id);
int32_t tPutBlockIdx(uint8_t *p, SBlockIdx *pBlockIdx);
int32_t tGetBlockIdx(uint8_t *p, SBlockIdx *pBlockIdx);

// SBlock
int32_t tBlockCmprFn(const void *p1, const void *p2);

// SDelIdx
int32_t tDelIdxClear(SDelIdx *pDelIdx);
int32_t tDelIdxPutItem(SDelIdx *pDelIdx, SDelIdxItem *pItem);
int32_t tDelIdxGetItemByIdx(SDelIdx *pDelIdx, SDelIdxItem *pItem, int32_t idx);
int32_t tDelIdxGetItem(SDelIdx *pDelIdx, SDelIdxItem *pItem, TABLEID id);
int32_t tPutDelIdx(uint8_t *p, SDelIdx *pDelIdx);
int32_t tGetDelIdx(uint8_t *p, SDelIdx *pDelIdx);

// SDelData
int32_t tDelDataClear(SDelData *pDelData);
int32_t tDelDataPutItem(SDelData *pDelData, SDelDataItem *pItem);
int32_t tDelDataGetItemByIdx(SDelData *pDelData, SDelDataItem *pItem, int32_t idx);
int32_t tDelDataGetItem(SDelData *pDelData, SDelDataItem *pItem, int64_t version);
int32_t tPutDelData(uint8_t *p, SDelData *pDelData);
int32_t tGetDelData(uint8_t *p, SDelData *pDelData);

int32_t tPutDelFileHdr(uint8_t *p, SDelFile *pDelFile);
int32_t tGetDelFileHdr(uint8_t *p, SDelFile *pDelFile);

// structs
struct SOffset {
  int32_t  nOffset;
  uint8_t  flag;
  uint8_t *pOffset;
};

typedef struct {
  int   minFid;
  int   midFid;
  int   maxFid;
  TSKEY minKey;
} SRtn;

#define TSDB_DATA_DIR_LEN 6  // adapt accordingly
struct STsdb {
  char         *path;
  SVnode       *pVnode;
  TdThreadMutex mutex;
  char          dir[TSDB_DATA_DIR_LEN];
  bool          repoLocked;
  STsdbKeepCfg  keepCfg;
  SMemTable    *mem;
  SMemTable    *imem;
  SRtn          rtn;
  STsdbFS      *fs;
};

struct STable {
  uint64_t  suid;
  uint64_t  uid;
  STSchema *pSchema;       // latest schema
  STSchema *pCacheSchema;  // cached cache
};

struct TSDBKEY {
  int64_t version;
  TSKEY   ts;
};

typedef struct SMemSkipListNode SMemSkipListNode;
struct SMemSkipListNode {
  int8_t            level;
  SMemSkipListNode *forwards[0];
};
typedef struct SMemSkipList {
  uint32_t          seed;
  int64_t           size;
  int8_t            maxLevel;
  int8_t            level;
  SMemSkipListNode *pHead;
  SMemSkipListNode *pTail;
} SMemSkipList;

struct STbData {
  tb_uid_t     suid;
  tb_uid_t     uid;
  TSDBKEY      minKey;
  TSDBKEY      maxKey;
  SDelOp      *pHead;
  SDelOp      *pTail;
  SMemSkipList sl;
};

struct SMemTable {
  SRWLatch latch;
  STsdb   *pTsdb;
  int32_t  nRef;
  TSDBKEY  minKey;
  TSDBKEY  maxKey;
  int64_t  nRow;
  int64_t  nDel;
  SArray  *aTbData;  // SArray<STbData*>
};

int tsdbLockRepo(STsdb *pTsdb);
int tsdbUnlockRepo(STsdb *pTsdb);

struct TSDBROW {
  union {
    struct {
      int64_t version;
      STSRow *pTSRow;
    };
    struct {
      SColDataBlock *pColDataBlock;
      int32_t        iRow;
    };
  };
};

struct SBlockIdx {
  int64_t suid;
  int64_t uid;
  TSDBKEY minKey;
  TSDBKEY maxKey;
  int64_t minVersion;
  int64_t maxVersion;
  int64_t offset;
  int64_t size;
};

typedef enum {
  TSDB_SBLK_VER_0 = 0,
  TSDB_SBLK_VER_MAX,
} ESBlockVer;

#define SBlockVerLatest TSDB_SBLK_VER_0

struct SBlockItem {
  TSDBKEY minKey;
  TSDBKEY maxKey;
  int64_t minVerion;
  int64_t maxVersion;
};

struct SBlockInfo {
  uint8_t flags;
  int32_t nCols;
  int64_t offset;
  int64_t size;
  int32_t nSmaCols;
  int64_t smaOffset;
  int64_t smaSize;
};

struct SBlock {
  TSDBKEY    minKey;
  TSDBKEY    maxKey;
  int64_t    minVersion;
  int64_t    maxVersion;
  int32_t    nRows;
  int8_t     nBlockInfo;
  SBlockInfo blockInfos[];
};

struct SBlockCol {
  int16_t  colId;
  uint16_t type : 6;
  uint16_t blen : 10;  // 0 no bitmap if all rows are NORM, > 0 bitmap length
  uint32_t len;        // data length + bitmap length
  uint32_t offset;
};

struct SAggrBlkCol {
  int16_t colId;
  int16_t maxIndex;
  int16_t minIndex;
  int16_t numOfNull;
  int64_t sum;
  int64_t max;
  int64_t min;
};

struct SBlockData {
  int32_t   delimiter;  // For recovery usage
  int32_t   numOfCols;  // For recovery usage
  uint64_t  uid;        // For recovery usage
  SBlockCol cols[];
};

typedef void SAggrBlkData;  // SBlockCol cols[];

// ================== TSDB global config
extern bool tsdbForceKeepFile;

#define TSDB_FS_ITER_FORWARD  TSDB_ORDER_ASC
#define TSDB_FS_ITER_BACKWARD TSDB_ORDER_DESC

struct TABLEID {
  tb_uid_t suid;
  tb_uid_t uid;
};

struct STbDataIter {
  STbData          *pTbData;
  int8_t            backward;
  SMemSkipListNode *pNode;
};

struct SDelOp {
  int64_t version;
  TSKEY   sKey;  // included
  TSKEY   eKey;  // included
  SDelOp *pNext;
};

struct SDelDataItem {
  int64_t version;
  TSKEY   sKey;
  TSKEY   eKey;
};

struct SDelData {
  uint32_t delimiter;
  tb_uid_t suid;
  tb_uid_t uid;
  SOffset  offset;
  uint32_t nData;
  uint8_t *pData;
};

struct SDelIdxItem {
  tb_uid_t suid;
  tb_uid_t uid;
  TSKEY    minKey;
  TSKEY    maxKey;
  int64_t  minVersion;
  int64_t  maxVersion;
  int64_t  offset;
  int64_t  size;
};

struct SDelIdx {
  uint32_t delimiter;
  SOffset  offset;
  uint32_t nData;
  uint8_t *pData;
};

struct SDelFile {
  TSKEY   minKey;
  TSKEY   maxKey;
  int64_t minVersion;
  int64_t maxVersion;
  int64_t size;
  int64_t offset;
};

struct SMapData {
  int32_t  nItem;
  uint8_t  flag;
  uint8_t *pOfst;
  uint32_t nData;
  uint8_t *pData;
  uint8_t *pBuf;
};

struct SColData {
  int16_t  cid;
  uint8_t  flags;
  uint32_t nData;
  uint8_t *pData;
};

struct SColDataBlock {
  int32_t   nRow;
  int64_t  *aVersion;
  TSKEY    *aTSKey;
  int32_t   nColData;
  SColData *aColData;
};

typedef struct {
  int16_t colId;
  int16_t maxIndex;
  int16_t minIndex;
  int16_t numOfNull;
  int64_t sum;
  int64_t max;
  int64_t min;
} SColSMA;

struct SBlockSMA {
  int32_t  nCol;
  SColSMA *aColSMA;
};

#ifdef __cplusplus
}
#endif

#endif /*_TD_VNODE_TSDB_H_*/