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

#ifndef _TD_COMMON_TOKEN_H_
#define _TD_COMMON_TOKEN_H_

#define TK_OR                   1
#define TK_AND                  2
#define TK_UNION                3
#define TK_ALL                  4
#define TK_MINUS                5
#define TK_EXCEPT               6
#define TK_INTERSECT            7
#define TK_NK_BITAND            8
#define TK_NK_BITOR             9
#define TK_NK_LSHIFT            10
#define TK_NK_RSHIFT            11
#define TK_NK_PLUS              12
#define TK_NK_MINUS             13
#define TK_NK_STAR              14
#define TK_NK_SLASH             15
#define TK_NK_REM               16
#define TK_NK_CONCAT            17
#define TK_CREATE               18
#define TK_ACCOUNT              19
#define TK_NK_ID                20
#define TK_PASS                 21
#define TK_NK_STRING            22
#define TK_ALTER                23
#define TK_PPS                  24
#define TK_TSERIES              25
#define TK_STORAGE              26
#define TK_STREAMS              27
#define TK_QTIME                28
#define TK_DBS                  29
#define TK_USERS                30
#define TK_CONNS                31
#define TK_STATE                32
#define TK_USER                 33
#define TK_ENABLE               34
#define TK_NK_INTEGER           35
#define TK_SYSINFO              36
#define TK_DROP                 37
#define TK_GRANT                38
#define TK_ON                   39
#define TK_TO                   40
#define TK_REVOKE               41
#define TK_FROM                 42
#define TK_SUBSCRIBE            43
#define TK_NK_COMMA             44
#define TK_READ                 45
#define TK_WRITE                46
#define TK_NK_DOT               47
#define TK_DNODE                48
#define TK_PORT                 49
#define TK_DNODES               50
#define TK_NK_IPTOKEN           51
#define TK_FORCE                52
#define TK_LOCAL                53
#define TK_QNODE                54
#define TK_BNODE                55
#define TK_SNODE                56
#define TK_MNODE                57
#define TK_DATABASE             58
#define TK_USE                  59
#define TK_FLUSH                60
#define TK_TRIM                 61
#define TK_IF                   62
#define TK_NOT                  63
#define TK_EXISTS               64
#define TK_BUFFER               65
#define TK_CACHEMODEL           66
#define TK_CACHESIZE            67
#define TK_COMP                 68
#define TK_DURATION             69
#define TK_NK_VARIABLE          70
#define TK_MAXROWS              71
#define TK_MINROWS              72
#define TK_KEEP                 73
#define TK_PAGES                74
#define TK_PAGESIZE             75
#define TK_TSDB_PAGESIZE        76
#define TK_PRECISION            77
#define TK_REPLICA              78
#define TK_STRICT               79
#define TK_VGROUPS              80
#define TK_SINGLE_STABLE        81
#define TK_RETENTIONS           82
#define TK_SCHEMALESS           83
#define TK_WAL_LEVEL            84
#define TK_WAL_FSYNC_PERIOD     85
#define TK_WAL_RETENTION_PERIOD 86
#define TK_WAL_RETENTION_SIZE   87
#define TK_WAL_ROLL_PERIOD      88
#define TK_WAL_SEGMENT_SIZE     89
#define TK_STT_TRIGGER          90
#define TK_TABLE_PREFIX         91
#define TK_TABLE_SUFFIX         92
#define TK_NK_COLON             93
#define TK_MAX_SPEED            94
#define TK_TABLE                95
#define TK_NK_LP                96
#define TK_NK_RP                97
#define TK_STABLE               98
#define TK_ADD                  99
#define TK_COLUMN               100
#define TK_MODIFY               101
#define TK_RENAME               102
#define TK_TAG                  103
#define TK_SET                  104
#define TK_NK_EQ                105
#define TK_USING                106
#define TK_TAGS                 107
#define TK_COMMENT              108
#define TK_BOOL                 109
#define TK_TINYINT              110
#define TK_SMALLINT             111
#define TK_INT                  112
#define TK_INTEGER              113
#define TK_BIGINT               114
#define TK_FLOAT                115
#define TK_DOUBLE               116
#define TK_BINARY               117
#define TK_TIMESTAMP            118
#define TK_NCHAR                119
#define TK_UNSIGNED             120
#define TK_JSON                 121
#define TK_VARCHAR              122
#define TK_MEDIUMBLOB           123
#define TK_BLOB                 124
#define TK_VARBINARY            125
#define TK_DECIMAL              126
#define TK_MAX_DELAY            127
#define TK_WATERMARK            128
#define TK_ROLLUP               129
#define TK_TTL                  130
#define TK_SMA                  131
#define TK_DELETE_MARK          132
#define TK_FIRST                133
#define TK_LAST                 134
#define TK_SHOW                 135
#define TK_PRIVILEGES           136
#define TK_DATABASES            137
#define TK_TABLES               138
#define TK_STABLES              139
#define TK_MNODES               140
#define TK_QNODES               141
#define TK_FUNCTIONS            142
#define TK_INDEXES              143
#define TK_ACCOUNTS             144
#define TK_APPS                 145
#define TK_CONNECTIONS          146
#define TK_LICENCES             147
#define TK_GRANTS               148
#define TK_QUERIES              149
#define TK_SCORES               150
#define TK_TOPICS               151
#define TK_VARIABLES            152
#define TK_CLUSTER              153
#define TK_BNODES               154
#define TK_SNODES               155
#define TK_TRANSACTIONS         156
#define TK_DISTRIBUTED          157
#define TK_CONSUMERS            158
#define TK_SUBSCRIPTIONS        159
#define TK_VNODES               160
#define TK_LIKE                 161
#define TK_TBNAME               162
#define TK_QTAGS                163
#define TK_AS                   164
#define TK_INDEX                165
#define TK_FUNCTION             166
#define TK_INTERVAL             167
#define TK_TOPIC                168
#define TK_WITH                 169
#define TK_META                 170
#define TK_CONSUMER             171
#define TK_GROUP                172
#define TK_DESC                 173
#define TK_DESCRIBE             174
#define TK_RESET                175
#define TK_QUERY                176
#define TK_CACHE                177
#define TK_EXPLAIN              178
#define TK_ANALYZE              179
#define TK_VERBOSE              180
#define TK_NK_BOOL              181
#define TK_RATIO                182
#define TK_NK_FLOAT             183
#define TK_OUTPUTTYPE           184
#define TK_AGGREGATE            185
#define TK_BUFSIZE              186
#define TK_STREAM               187
#define TK_INTO                 188
#define TK_TRIGGER              189
#define TK_AT_ONCE              190
#define TK_WINDOW_CLOSE         191
#define TK_IGNORE               192
#define TK_EXPIRED              193
#define TK_FILL_HISTORY         194
#define TK_SUBTABLE             195
#define TK_KILL                 196
#define TK_CONNECTION           197
#define TK_TRANSACTION          198
#define TK_BALANCE              199
#define TK_VGROUP               200
#define TK_MERGE                201
#define TK_REDISTRIBUTE         202
#define TK_SPLIT                203
#define TK_DELETE               204
#define TK_INSERT               205
#define TK_NULL                 206
#define TK_NK_QUESTION          207
#define TK_NK_ARROW             208
#define TK_ROWTS                209
#define TK_QSTART               210
#define TK_QEND                 211
#define TK_QDURATION            212
#define TK_WSTART               213
#define TK_WEND                 214
#define TK_WDURATION            215
#define TK_IROWTS               216
#define TK_CAST                 217
#define TK_NOW                  218
#define TK_TODAY                219
#define TK_TIMEZONE             220
#define TK_CLIENT_VERSION       221
#define TK_SERVER_VERSION       222
#define TK_SERVER_STATUS        223
#define TK_CURRENT_USER         224
#define TK_COUNT                225
#define TK_LAST_ROW             226
#define TK_CASE                 227
#define TK_END                  228
#define TK_WHEN                 229
#define TK_THEN                 230
#define TK_ELSE                 231
#define TK_BETWEEN              232
#define TK_IS                   233
#define TK_NK_LT                234
#define TK_NK_GT                235
#define TK_NK_LE                236
#define TK_NK_GE                237
#define TK_NK_NE                238
#define TK_MATCH                239
#define TK_NMATCH               240
#define TK_CONTAINS             241
#define TK_IN                   242
#define TK_JOIN                 243
#define TK_INNER                244
#define TK_SELECT               245
#define TK_DISTINCT             246
#define TK_WHERE                247
#define TK_PARTITION            248
#define TK_BY                   249
#define TK_SESSION              250
#define TK_STATE_WINDOW         251
#define TK_SLIDING              252
#define TK_FILL                 253
#define TK_VALUE                254
#define TK_NONE                 255
#define TK_PREV                 256
#define TK_LINEAR               257
#define TK_NEXT                 258
#define TK_HAVING               259
#define TK_RANGE                260
#define TK_EVERY                261
#define TK_ORDER                262
#define TK_SLIMIT               263
#define TK_SOFFSET              264
#define TK_LIMIT                265
#define TK_OFFSET               266
#define TK_ASC                  267
#define TK_NULLS                268
#define TK_ABORT                269
#define TK_AFTER                270
#define TK_ATTACH               271
#define TK_BEFORE               272
#define TK_BEGIN                273
#define TK_BITAND               274
#define TK_BITNOT               275
#define TK_BITOR                276
#define TK_BLOCKS               277
#define TK_CHANGE               278
#define TK_COMMA                279
#define TK_COMPACT              280
#define TK_CONCAT               281
#define TK_CONFLICT             282
#define TK_COPY                 283
#define TK_DEFERRED             284
#define TK_DELIMITERS           285
#define TK_DETACH               286
#define TK_DIVIDE               287
#define TK_DOT                  288
#define TK_EACH                 289
#define TK_FAIL                 290
#define TK_FILE                 291
#define TK_FOR                  292
#define TK_GLOB                 293
#define TK_ID                   294
#define TK_IMMEDIATE            295
#define TK_IMPORT               296
#define TK_INITIALLY            297
#define TK_INSTEAD              298
#define TK_ISNULL               299
#define TK_KEY                  300
#define TK_MODULES              301
#define TK_NK_BITNOT            302
#define TK_NK_SEMI              303
#define TK_NOTNULL              304
#define TK_OF                   305
#define TK_PLUS                 306
#define TK_PRIVILEGE            307
#define TK_RAISE                308
#define TK_REPLACE              309
#define TK_RESTRICT             310
#define TK_ROW                  311
#define TK_SEMI                 312
#define TK_STAR                 313
#define TK_STATEMENT            314
#define TK_STRING               315
#define TK_TIMES                316
#define TK_UPDATE               317
#define TK_VALUES               318
#define TK_VARIABLE             319
#define TK_VIEW                 320
#define TK_WAL                  321

#define TK_NK_SPACE   600
#define TK_NK_COMMENT 601
#define TK_NK_ILLEGAL 602
#define TK_NK_HEX     603  // hex number  0x123
#define TK_NK_OCT     604  // oct number
#define TK_NK_BIN     605  // bin format data 0b111

#define TK_NK_NIL 65535

#endif /*_TD_COMMON_TOKEN_H_*/
