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

#include "tsdb.h"

// STsdbSnapReader ========================================
struct STsdbSnapReader {
  STsdb*  pTsdb;
  int64_t sver;
  int64_t ever;
  // for data file
  int8_t        dataDone;
  int32_t       fid;
  SDataFReader* pDataFReader;
  SArray*       aBlockIdx;  // SArray<SBlockIdx>
  int32_t       iBlockIdx;
  SBlockIdx*    pBlockIdx;
  SMapData      mBlock;  // SMapData<SBlock>
  int32_t       iBlock;
  SBlockData    blkData;
  // for del file
  int8_t       delDone;
  SDelFReader* pDelFReader;
  int32_t      iDelIdx;
  SArray*      aDelIdx;   // SArray<SDelIdx>
  SArray*      aDelData;  // SArray<SDelData>
};

static int32_t tsdbSnapReadData(STsdbSnapReader* pReader, uint8_t** ppData) {
  int32_t code = 0;

  while (true) {
    if (pReader->pDataFReader == NULL) {
      SDFileSet* pSet = NULL;

      // search the next data file set to read (todo)
      if (0 /* TODO */) {
        code = TSDB_CODE_VND_READ_END;
        goto _exit;
      }

      // open
      code = tsdbDataFReaderOpen(&pReader->pDataFReader, pReader->pTsdb, pSet);
      if (code) goto _err;

      // SBlockIdx
      code = tsdbReadBlockIdx(pReader->pDataFReader, pReader->aBlockIdx, NULL);
      if (code) goto _err;

      pReader->iBlockIdx = 0;
      pReader->pBlockIdx = NULL;
    }

    while (true) {
      if (pReader->pBlockIdx == NULL) {
        if (pReader->iBlockIdx >= taosArrayGetSize(pReader->aBlockIdx)) {
          tsdbDataFReaderClose(&pReader->pDataFReader);
          break;
        }

        pReader->pBlockIdx = (SBlockIdx*)taosArrayGet(pReader->aBlockIdx, pReader->iBlockIdx);
        pReader->iBlockIdx++;

        // SBlock
        code = tsdbReadBlock(pReader->pDataFReader, pReader->pBlockIdx, &pReader->mBlock, NULL);
        if (code) goto _err;

        pReader->iBlock = 0;
      }

      while (true) {
        SBlock  block;
        SBlock* pBlock = &block;

        if (pReader->iBlock >= pReader->mBlock.nItem) {
          pReader->pBlockIdx = NULL;
          break;
        }

        tMapDataGetItemByIdx(&pReader->mBlock, pReader->iBlock, pBlock, tGetBlock);
        pReader->iBlock++;

        if ((pBlock->minVersion >= pReader->sver && pBlock->minVersion <= pReader->ever) ||
            (pBlock->maxVersion >= pReader->sver && pBlock->maxVersion <= pReader->ever)) {
          // overlap (todo)

          code = tsdbReadBlockData(pReader->pDataFReader, pReader->pBlockIdx, pBlock, &pReader->blkData, NULL, NULL);
          if (code) goto _err;

          goto _exit;
        }
      }
    }
  }

_exit:
  return code;

_err:
  tsdbError("vgId:%d snap read data failed since %s", TD_VID(pReader->pTsdb->pVnode), tstrerror(code));
  return code;
}

static int32_t tsdbSnapReadDel(STsdbSnapReader* pReader, uint8_t** ppData) {
  int32_t   code = 0;
  STsdb*    pTsdb = pReader->pTsdb;
  SDelFile* pDelFile = pTsdb->fs->cState->pDelFile;

  if (pReader->pDelFReader == NULL) {
    if (pDelFile == NULL) {
      code = TSDB_CODE_VND_READ_END;
      goto _exit;
    }

    // open
    code = tsdbDelFReaderOpen(&pReader->pDelFReader, pDelFile, pTsdb, NULL);
    if (code) goto _err;

    // read index
    code = tsdbReadDelIdx(pReader->pDelFReader, pReader->aDelIdx, NULL);
    if (code) goto _err;

    pReader->iDelIdx = 0;
  }

  while (pReader->iDelIdx < taosArrayGetSize(pReader->aDelIdx)) {
    SDelIdx* pDelIdx = (SDelIdx*)taosArrayGet(pReader->aDelIdx, pReader->iDelIdx);
    int32_t  size = 0;

    pReader->iDelIdx++;

    code = tsdbReadDelData(pReader->pDelFReader, pDelIdx, pReader->aDelData, NULL);
    if (code) goto _err;

    for (int32_t iDelData = 0; iDelData < taosArrayGetSize(pReader->aDelData); iDelData++) {
      SDelData* pDelData = (SDelData*)taosArrayGet(pReader->aDelData, iDelData);

      if (pDelData->version >= pReader->sver && pDelData->version <= pReader->ever) {
        size += tPutDelData(NULL, pDelData);
      }
    }

    if (size > 0) {
      int64_t n = 0;

      size = size + sizeof(SSnapDataHdr) + sizeof(TABLEID);
      code = tRealloc(ppData, size);
      if (code) goto _err;

      // SSnapDataHdr
      SSnapDataHdr* pSnapDataHdr = (SSnapDataHdr*)(*ppData + n);
      pSnapDataHdr->type = 1;
      pSnapDataHdr->size = size;  // TODO: size here may incorrect
      n += sizeof(SSnapDataHdr);

      // TABLEID
      TABLEID* pId = (TABLEID*)(*ppData + n);
      pId->suid = pDelIdx->suid;
      pId->uid = pDelIdx->uid;
      n += sizeof(*pId);

      // DATA
      for (int32_t iDelData = 0; iDelData < taosArrayGetSize(pReader->aDelData); iDelData++) {
        SDelData* pDelData = (SDelData*)taosArrayGet(pReader->aDelData, iDelData);

        if (pDelData->version >= pReader->sver && pDelData->version <= pReader->ever) {
          n += tPutDelData(*ppData + n, pDelData);
        }
      }

      goto _exit;
    }
  }

  code = TSDB_CODE_VND_READ_END;
  tsdbDelFReaderClose(&pReader->pDelFReader);

_exit:
  return code;

_err:
  tsdbError("vgId:%d snap read del failed since %s", TD_VID(pTsdb->pVnode), tstrerror(code));
  return code;
}

int32_t tsdbSnapReaderOpen(STsdb* pTsdb, int64_t sver, int64_t ever, STsdbSnapReader** ppReader) {
  int32_t          code = 0;
  STsdbSnapReader* pReader = NULL;

  // alloc
  pReader = (STsdbSnapReader*)taosMemoryCalloc(1, sizeof(*pReader));
  if (pReader == NULL) {
    code = TSDB_CODE_OUT_OF_MEMORY;
    goto _err;
  }
  pReader->pTsdb = pTsdb;
  pReader->sver = sver;
  pReader->ever = ever;

  pReader->aBlockIdx = taosArrayInit(0, sizeof(SBlockIdx));
  if (pReader->aBlockIdx == NULL) {
    code = TSDB_CODE_OUT_OF_MEMORY;
    goto _err;
  }

  pReader->mBlock = tMapDataInit();

  code = tBlockDataInit(&pReader->blkData);
  if (code) goto _err;

  pReader->aDelIdx = taosArrayInit(0, sizeof(SDelIdx));
  if (pReader->aDelIdx == NULL) {
    code = TSDB_CODE_OUT_OF_MEMORY;
    goto _err;
  }

  pReader->aDelData = taosArrayInit(0, sizeof(SDelData));
  if (pReader->aDelData == NULL) {
    code = TSDB_CODE_OUT_OF_MEMORY;
    goto _err;
  }

  *ppReader = pReader;
  return code;

_err:
  tsdbError("vgId:%d snapshot reader open failed since %s", TD_VID(pTsdb->pVnode), tstrerror(code));
  *ppReader = NULL;
  return code;
}

int32_t tsdbSnapReaderClose(STsdbSnapReader** ppReader) {
  int32_t          code = 0;
  STsdbSnapReader* pReader = *ppReader;

  taosArrayDestroy(pReader->aDelData);
  taosArrayDestroy(pReader->aDelIdx);
  if (pReader->pDelFReader) {
    tsdbDelFReaderClose(&pReader->pDelFReader);
  }
  tBlockDataClear(&pReader->blkData);
  tMapDataClear(&pReader->mBlock);
  taosArrayDestroy(pReader->aBlockIdx);
  if (pReader->pDataFReader) {
    tsdbDataFReaderClose(&pReader->pDataFReader);
  }
  taosMemoryFree(pReader);
  *ppReader = NULL;

  return code;
}

int32_t tsdbSnapRead(STsdbSnapReader* pReader, uint8_t** ppData) {
  int32_t code = 0;

  // read data file
  if (!pReader->dataDone) {
    code = tsdbSnapReadData(pReader, ppData);
    if (code) {
      if (code == TSDB_CODE_VND_READ_END) {
        pReader->dataDone = 1;
      } else {
        goto _err;
      }
    } else {
      goto _exit;
    }
  }

  // read del file
  if (!pReader->delDone) {
    code = tsdbSnapReadDel(pReader, ppData);
    if (code) {
      if (code == TSDB_CODE_VND_READ_END) {
        pReader->delDone = 1;
      } else {
        goto _err;
      }
    } else {
      goto _exit;
    }
  }

  code = TSDB_CODE_VND_READ_END;

_exit:
  return code;

_err:
  tsdbError("vgId:%d snapshot read failed since %s", TD_VID(pReader->pTsdb->pVnode), tstrerror(code));
  return code;
}

// STsdbSnapWriter ========================================
struct STsdbSnapWriter {
  STsdb*  pTsdb;
  int64_t sver;
  int64_t ever;
  // config
  int32_t minutes;
  int8_t  precision;
  // for data file
  int32_t       fid;
  SDataFReader* pDataFReader;
  SArray*       aBlockIdx;
  SMapData      mBlock;
  SDataFWriter* pDataFWriter;
  SArray*       aBlockIdxN;
  SMapData      mBlockN;
  // for del file
  SDelFReader* pDelFReader;
  SDelFWriter* pDelFWriter;
  int32_t      iDelIdx;
  SArray*      aDelIdx;
  SArray*      aDelData;
  SArray*      aDelIdxN;
};

static int32_t tsdbSnapRollback(STsdbSnapWriter* pWriter) {
  int32_t code = 0;
  // TODO
  return code;
}

static int32_t tsdbSnapCommit(STsdbSnapWriter* pWriter) {
  int32_t code = 0;
  // TODO
  return code;
}

static int32_t tsdbSnapWriteData(STsdbSnapWriter* pWriter, uint8_t* pData, uint32_t nData) {
  int32_t code = 0;
  STsdb*  pTsdb = pWriter->pTsdb;
  int64_t suid = 0;  // todo
  int64_t uid = 0;   // todo
  int64_t skey;      // todo
  int64_t ekey;      // todo

  int32_t fid = tsdbKeyFid(skey, pWriter->minutes, pWriter->precision);
  ASSERT(fid == tsdbKeyFid(ekey, pWriter->minutes, pWriter->precision));
  if (pWriter->pDataFWriter == NULL || pWriter->fid != fid) {
    if (pWriter->pDataFWriter) {
      // finish current file and close the SDataFWriter
    }

    pWriter->fid = fid;
    SDFileSet* pSet = tsdbFSStateGetDFileSet(pTsdb->fs->nState, fid);
    SDFileSet  wSet = {0};
    if (pSet == NULL) {
      wSet = (SDFileSet){0};  // todo
    } else {
      wSet = (SDFileSet){0};  // todo
    }

    code = tsdbDataFWriterOpen(&pWriter->pDataFWriter, pTsdb, &wSet);
    if (code) goto _err;
  }

  return code;

_err:
  tsdbError("vgId:%d tsdb snapshot write data failed since %s", TD_VID(pTsdb->pVnode), tstrerror(code));
  return code;
}

static int32_t tsdbSnapWriteDel(STsdbSnapWriter* pWriter, uint8_t* pData, uint32_t nData) {
  int32_t code = 0;
  STsdb*  pTsdb = pWriter->pTsdb;

  if (pWriter->pDelFWriter == NULL) {
    SDelFile* pDelFile = tsdbFSStateGetDelFile(pTsdb->fs->nState);

    // reader
    if (pDelFile) {
      code = tsdbDelFReaderOpen(&pWriter->pDelFReader, pDelFile, pTsdb, NULL);
      if (code) goto _err;

      code = tsdbReadDelIdx(pWriter->pDelFReader, pWriter->aDelIdx, NULL);
      if (code) goto _err;
    }

    // writer
    SDelFile delFile = {.commitID = pTsdb->pVnode->state.commitID, .offset = 0, .size = 0};
    code = tsdbDelFWriterOpen(&pWriter->pDelFWriter, &delFile, pTsdb);
    if (code) goto _err;
  }

  // process the del data
  TABLEID id = {0};  // todo

  while (true) {
    SDelIdx* pDelIdx = NULL;
    int64_t  n = 0;
    SDelData delData;
    SDelIdx  delIdx;
    int8_t   toBreak = 0;

    if (pWriter->iDelIdx < taosArrayGetSize(pWriter->aDelIdx)) {
      pDelIdx = taosArrayGet(pWriter->aDelIdx, pWriter->iDelIdx);
    }

    if (pDelIdx) {
      int32_t c = tTABLEIDCmprFn(&id, pDelIdx);
      if (c < 0) {
        goto _new_del;
      } else {
        code = tsdbReadDelData(pWriter->pDelFReader, pDelIdx, pWriter->aDelData, NULL);
        if (code) goto _err;

        pWriter->iDelIdx++;
        if (c == 0) {
          toBreak = 1;
          goto _merge_del;
        } else {
          goto _write_del;
        }
      }
    }

  _new_del:
    toBreak = 1;
    taosArrayClear(pWriter->aDelData);

  _merge_del:
    while (n < nData) {
      n += tGetDelData(pData + n, &delData);
      if (taosArrayPush(pWriter->aDelData, &delData) == NULL) {
        code = TSDB_CODE_OUT_OF_MEMORY;
        goto _err;
      }
    }

  _write_del:
    delIdx = (SDelIdx){.suid = id.suid, .uid = id.uid};
    code = tsdbWriteDelData(pWriter->pDelFWriter, pWriter->aDelData, NULL, &delIdx);
    if (code) goto _err;

    if (taosArrayPush(pWriter->aDelIdxN, &delIdx) == NULL) {
      code = TSDB_CODE_OUT_OF_MEMORY;
      goto _err;
    }

    if (toBreak) break;
  }

_exit:
  return code;

_err:
  tsdbError("vgId:%d tsdb snapshot write del failed since %s", TD_VID(pTsdb->pVnode), tstrerror(code));
  return code;
}

int32_t tsdbSnapWriterOpen(STsdb* pTsdb, int64_t sver, int64_t ever, STsdbSnapWriter** ppWriter) {
  int32_t          code = 0;
  STsdbSnapWriter* pWriter = NULL;

  // alloc
  pWriter = (STsdbSnapWriter*)taosMemoryCalloc(1, sizeof(*pWriter));
  if (pWriter == NULL) {
    code = TSDB_CODE_OUT_OF_MEMORY;
    goto _err;
  }
  pWriter->pTsdb = pTsdb;
  pWriter->sver = sver;
  pWriter->ever = ever;

  *ppWriter = pWriter;
  return code;

_err:
  tsdbError("vgId:%d tsdb snapshot writer open failed since %s", TD_VID(pTsdb->pVnode), tstrerror(code));
  *ppWriter = NULL;
  return code;
}

int32_t tsdbSnapWriterClose(STsdbSnapWriter** ppWriter, int8_t rollback) {
  int32_t          code = 0;
  STsdbSnapWriter* pWriter = *ppWriter;

  if (rollback) {
    code = tsdbSnapRollback(pWriter);
    if (code) goto _err;
  } else {
    code = tsdbSnapCommit(pWriter);
    if (code) goto _err;
  }

  taosMemoryFree(pWriter);
  *ppWriter = NULL;

  return code;

_err:
  tsdbError("vgId:%d tsdb snapshot writer close failed since %s", TD_VID(pWriter->pTsdb->pVnode), tstrerror(code));
  return code;
}

int32_t tsdbSnapWrite(STsdbSnapWriter* pWriter, uint8_t* pData, uint32_t nData) {
  int32_t code = 0;
  int8_t  type = pData[0];

  // ts data
  if (type == 0) {
    code = tsdbSnapWriteData(pWriter, pData + 1, nData - 1);
    if (code) goto _err;
  } else {
    if (pWriter->pDataFWriter) {
      // commit the remain data of the FSet (todo)

      // close and update the file
      code = tsdbDataFWriterClose(&pWriter->pDataFWriter, 1);
      if (code) goto _err;
    }
  }

  // del data
  if (type == 1) {
    code = tsdbSnapWriteDel(pWriter, pData + 1, nData - 1);
    if (code) goto _err;
  }

  return code;

_err:
  tsdbError("vgId:%d tsdb snapshow write failed since %s", TD_VID(pWriter->pTsdb->pVnode), tstrerror(code));
  return code;
}
