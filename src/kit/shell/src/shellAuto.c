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

#define __USE_XOPEN
#include "os.h"
#include "tglobal.h"
#include "shell.h"
#include "shellCommand.h"
#include "tkey.h"
#include "tulog.h"
#include "shellAuto.h"
#include "tire.h"
#include "tthread.h"

//
// ------------- define area  ---------------
//
#define UNION_ALL " union all "


// extern function
void insertStr(Command *cmd, char *str, int size);


typedef struct SAutoPtr {
  STire* p;
  int ref;
}SAutoPtr;

typedef struct SWord{
  int type ; // word type , see WT_ define
  char * word;
  int32_t len;
  struct SWord * next;
  bool free;  // if true need free
}SWord;

typedef struct {
  char * source;
  int32_t source_len; // valid data length in source
  int32_t count;
  SWord*  head;
  // matched information
  int32_t matchIndex;    // matched word index in words
  int32_t matchLen;     // matched length at matched word
}SWords;


SWords shellCommands[] = {
  {"alter database <db_name> <db_options> <anyword> <db_options> <anyword> <db_options> <anyword> <db_options> <anyword> <db_options> <anyword> <db_options> <anyword>", 0, 0, NULL},
  {"alter dnode <dnode_id> balance ", 0, 0, NULL},
  {"alter dnode <dnode_id> resetlog;", 0, 0, NULL},
  {"alter dnode <dnode_id> debugFlag 141;", 0, 0, NULL},
  {"alter dnode <dnode_id> monitor 1;", 0, 0, NULL},
  {"alter table <tb_name> <tb_actions>", 0, 0, NULL},
  {"alter table modify column", 0, 0, NULL},
  {"alter local resetlog;", 0, 0, NULL},
  {"alter local DebugFlag 143;", 0, 0, NULL},
  {"alter local cDebugFlag 143;", 0, 0, NULL},
  {"alter local uDebugFlag 143;", 0, 0, NULL},
  {"alter local rpcDebugFlag 143;", 0, 0, NULL},
  {"alter local tmrDebugFlag 143;", 0, 0, NULL},
  {"alter topic", 0, 0, NULL},
  {"alter user <user_name> pass", 0, 0, NULL},
  {"alter user <user_name> tags", 0, 0, NULL},
  {"alter user <user_name> privilege read", 0, 0, NULL},
  {"alter user <user_name> privilege write", 0, 0, NULL},
  {"create table <anyword> using <stb_name> tags(", 0, 0, NULL},
  {"create table <anyword> as select ", 0, 0, NULL},
  {"create database ", 0, 0, NULL},
  {"create table <anyword> as ", 0, 0, NULL},
  {"create dnode ", 0, 0, NULL},
  {"create topic", 0, 0, NULL},
  {"create function ", 0, 0, NULL},
  {"create user <anyword> pass <anyword> tags", 0, 0, NULL},
  {"compact vnode in", 0, 0, NULL},
  {"describe <all_table>", 0, 0, NULL},
#ifdef TD_ENTERPRISE
  {"delete from <all_table> where", 0, 0, NULL},
#endif
  {"drop database <db_name>", 0, 0, NULL},
  {"drop table <all_table> ", 0, 0, NULL},
  {"drop dnode <dnode_id>", 0, 0, NULL},
  {"drop user <user_name> ;", 0, 0, NULL},
  {"drop function", 0, 0, NULL},
  {"drop topic", 0, 0, NULL},
  {"kill connection", 0, 0, NULL},
  {"kill query", 0, 0, NULL},
  {"kill stream", 0, 0, NULL},
  {"select * from <all_table> where ", 0, 0, NULL},
  {"select _block_dist() from <all_table> \\G;", 0, 0, NULL},
  {"select client_version();", 0, 0, NULL},
  {"select cluster_status();", 0, 0, NULL},
  {"select current_user();", 0, 0, NULL},
  {"select database;", 0, 0, NULL},
  {"select server_version();", 0, 0, NULL},
  {"set max_binary_display_width ", 0, 0, NULL},
  {"show create database <db_name> \\G;", 0, 0, NULL},
  {"show create stable <stb_name> \\G;", 0, 0, NULL},
  {"show create table <tb_name> \\G;", 0, 0, NULL},
  {"show connections;", 0, 0, NULL},
  {"show databases;", 0, 0, NULL},
  {"show dnodes;", 0, 0, NULL},
  {"show functions;", 0, 0, NULL},
  {"show modules;", 0, 0, NULL},
  {"show mnodes;", 0, 0, NULL},
  {"show queries;", 0, 0, NULL},
  {"show stables;", 0, 0, NULL},
  {"show stables like ", 0, 0, NULL},
  {"show streams;", 0, 0, NULL},
  {"show scores;", 0, 0, NULL},
  {"show tables;", 0, 0, NULL},
  {"show tables like", 0, 0, NULL},
  {"show users;", 0, 0, NULL},
  {"show variables;", 0, 0, NULL},
  {"show vgroups;", 0, 0, NULL},
  {"insert into <tb_name> values(", 0, 0, NULL},
  {"insert into <tb_name> using <stb_name> tags( <anyword> ) values(", 0, 0, NULL},
  {"insert into <tb_name> using <stb_name> <anyword> values(", 0, 0, NULL},
  {"use <db_name>", 0, 0, NULL},
  {"quit", 0, 0, NULL}
};

char * keywords[] = {
  "and ",
  "asc ",
  "desc ",
  "from ",
  "fill(",
  "limit ",
  "where ",
  "interval(",
  "order by ",
  "order by ",
  "offset ",
  "or ",
  "group by ",
  "now()",
  "session(",
  "sliding ",
  "slimit ",
  "soffset ",
  "state_window(",
  "today() ",
  "union all select ",
};

char * functions[] = {
  "count(",
  "sum(",
  "avg(",
  "last(",
  "last_row(",
  "top(",
  "interp(",
  "max(",
  "min(",
  "now()",
  "today()",
  "percentile(",
  "tail(",
  "pow(",  
  "abs(",
  "atan(",
  "acos(",
  "asin(",
  "apercentile(",
  "bottom(",
  "cast(",
  "ceil(",
  "char_length(",
  "cos(",
  "concat(",
  "concat_ws(",
  "csum(",
  "diff(",
  "derivative(",
  "elapsed(",
  "first(",
  "floor(",
  "hyperloglog(",
  "histogram(",
  "irate(",
  "leastsquares(",
  "length(",
  "log(",
  "lower(",
  "ltrim(",
  "mavg(",
  "mode(",
  "tan(",
  "round(",
  "rtrim(",
  "sample(",
  "sin(",
  "spread(",
  "substr(",
  "statecount(",
  "stateduration(",
  "stddev(",
  "sqrt(",
  "timediff(",
  "timezone(",
  "timetruncate(",
  "twa(",
  "to_unixtimestamp(",
  "unique(",
  "upper(",
};

char * tb_actions[] = {
  "add column",
  "modify column",
  "drop column",
  "change tag",
};

char * db_options[] = {
  "blocks",
  "cachelast",
  "comp",
  "keep",
  "replica",
  "quorum",
};

char * data_types[] = {
  "timestamp",
  "int",
  "float",
  "double",
  "binary(16)",
  "nchar(16)",
  "bigint",
  "smallint",
  "tinyint",
  "bool",
  "json"
};

char * key_tags[] = {
  "tags("
};

char * key_select[] = {
  "select "
};

//
//  ------- gobal variant define ---------
//
int32_t firstMatchIndex = -1; // first match shellCommands index
int32_t lastMatchIndex  = -1; // last match shellCommands index
int32_t curMatchIndex   = -1; // current match shellCommands index
int32_t lastWordBytes   = -1; // printShow last word length 
bool    waitAutoFill    = false;


//
//   ----------- global var array define -----------
//
#define WT_VAR_DBNAME   0
#define WT_VAR_STABLE   1
#define WT_VAR_TABLE    2
#define WT_VAR_DNODEID  3
#define WT_VAR_USERNAME 4
#define WT_VAR_ALLTABLE 5
#define WT_VAR_FUNC     6
#define WT_VAR_KEYWORD  7
#define WT_VAR_TBACTION 8
#define WT_VAR_DBOPTION 9
#define WT_VAR_DATATYPE 10
#define WT_VAR_KEYTAGS  11
#define WT_VAR_ANYWORD  12
#define WT_VAR_KEYSELECT 13
#define WT_VAR_CNT      14

#define WT_FROM_DB_MAX  4  // max get content from db
#define WT_FROM_DB_CNT  (WT_FROM_DB_MAX + 1)

#define WT_TEXT       0xFF

char dbName[256] = ""; // save use database name;
// tire array
STire* tires[WT_VAR_CNT];
pthread_mutex_t tiresMutex;
//save thread handle obtain var name from db server
pthread_t* threads[WT_FROM_DB_CNT];
// obtain var name  with sql from server
char varTypes[WT_VAR_CNT][64] = {
  "<db_name>",
  "<stb_name>",
  "<tb_name>",
  "<dnode_id>",
  "<user_name>",
  "<all_table>",
  "<function>",
  "<keyword>",
  "<tb_actions>",
  "<db_options>",
  "<data_types>",
  "<key_tags>",
  "<anyword>",
  "<key_select>"
};

char varSqls[WT_FROM_DB_CNT][64] = {
  "show databases;",
  "show stables;",
  "show tables;",
  "show dnodes;",
  "show users;"
};


// var words current cursor, if user press any one key except tab, cursorVar can be reset to -1
int cursorVar = -1;
bool varMode = false; // enter var names list mode


TAOS*    varCon    = NULL;
Command* varCmd    = NULL;
SMatch*  lastMatch = NULL; // save last match result 
int      cntDel    = 0;   // delete byte count after next press tab


// show auto tab introduction
void printfIntroduction() {
  printf("   ******************************  Tab Completion  **********************************\n");
  printf("   *   The TDengine CLI supports tab completion for a variety of items,             *\n");
  printf("   *   including database names, table names, function names and keywords.          *\n");
  printf("   *   The full list of shortcut keys is as follows:                                *\n");
  printf("   *    [ TAB ]        ......  complete the current word                            *\n");
  printf("   *                   ......  if used on a blank line, display all valid commands  *\n");
  printf("   *    [ Ctrl + A ]   ......  move cursor to the st[A]rt of the line               *\n");
  printf("   *    [ Ctrl + E ]   ......  move cursor to the [E]nd of the line                 *\n");
  printf("   *    [ Ctrl + W ]   ......  move cursor to the middle of the line                *\n");
  printf("   *    [ Ctrl + L ]   ......  clear the entire screen                              *\n");
  printf("   *    [ Ctrl + K ]   ......  clear the screen after the cursor                    *\n");
  printf("   *    [ Ctrl + U ]   ......  clear the screen before the cursor                   *\n");
  printf("   **********************************************************************************\n\n");
}

void showHelp() {
  printf("\nFThe TDengine CLI supports the following commands:");
  printf("\n\
  ----- A ----- \n\
    alter database <db_name> <db_options> \n\
    alter dnode <dnode_id> balance \n\
    alter dnode <dnode_id> resetlog;\n\
    alter dnode <dnode_id> DebugFlag 143;\n\
    alter dnode <dnode_id> monitor 1;\n\
    alter table <tb_name> ADD COLUMN <field_name> <data_type>; \n\
    alter table <tb_name> DROP COLUMN <field_name>; \n\
    alter table <tb_name> MODIFY COLUMN <field_name> <data_type(length)>;\n\
    alter local resetlog; \n\
    alter local DebugFlag 143; \n\
    alter topic <topic_name>\n\
    alter user <user_name> pass <password>;\n\
    alter user <user_name> tags <privileges>;\n\
    alter user <user_name> privilege read ;\n\
    alter user <user_name> privilege write ;\n\
  ----- C ----- \n\
    create table <tb_name> using <stb_name> tags ...\n\
    create database <db_name>;\n\
    create table <anyword> as ...\n\
    create dnode <dnode_id>\n\
    create topic <top_name>\n\
    create function <function_name>\n\
    create user <user_name> pass <password>;\n\
    create user <user_name> pass <password> tags <privileges>;\n\
    compact vnode in (vgid,vgid,vgid);\n\
  ----- D ----- \n\
    describe <all_table> ;\n\
    delete from <all_table> where ... \n\
    drop database <db_name>;\n\
    drop table <all_table>;\n\
    drop dnode <dnode_id>;\n\
    drop function <function_id>;\n\
    drop topic <topic_id>;\n\
    drop user <user_name>;\n\
  ----- K ----- \n\
    kill connection <connection_id>; \n\
    kill query <query_id>; \n\
    kill stream <stream_id>; \n\
  ----- S ----- \n\
    select * from <all_table> where ... \n\
    select _block_dist() from <all_table>;\n\
    select client_version();\n\
    select current_user();\n\
    select cluster_version();\n\
    select database;\n\
    select server_version();\n\
    set max_binary_display_width <width>;  \n\
    show create database <db_name>;\n\
    show create stable <stb_name>;\n\
    show create table <tb_name>;\n\
    show connections;\n\
    show databases;\n\
    show dnodes;\n\
    show functions;\n\
    show modules;\n\
    show mnodes;\n\
    show queries;\n\
    show stables;\n\
    show stables like '<regular expression>';  note: regular expression only support '_' and '%%' match.\n\
    show streams;\n\
    show scores;\n\
    show tables;\n\
    show tables like '<regular expression>'; \n\
    show users;\n\
    show variables;\n\
    show vgroups;\n\
  ----- I ----- \n\
    insert into <tb_name> values(...) ;\n\
  ----- U ----- \n\
    use <db_name>;");

  printf("\n\n");
  
  //define in getDuration() function
  printf("\
  Timestamp expression Format:\n\
    b - nanosecond \n\
    u - microsecond \n\
    a - millisecond \n\
    s - second \n\
    m - minute \n\
    h - hour \n\
    d - day \n\
    w - week \n\
    now - current time \n\
  Example : \n\
    select * from t1 where ts > now - 2w + 3d and ts <= now - 1w -2h ;\n");
  printf("\n");
}

//
//  -------------------  parse words --------------------------
//

#define SHELL_COMMAND_COUNT() (sizeof(shellCommands) / sizeof(SWords))

// get at
SWord * atWord(SWords * command, int32_t index) {
  SWord * word = command->head;
  for (int32_t i = 0; i < index; i++) {
    if (word == NULL)
      return NULL;
    word = word->next;
  }

  return word;
}

#define MATCH_WORD(x) atWord(x, x->matchIndex)

int wordType(const char* p, int32_t len) {
  for (int i = 0; i < WT_VAR_CNT; i++) {
    if (strncmp(p, varTypes[i], len) == 0)
        return i;
  }
  return WT_TEXT;
}

// add word
SWord * addWord(const char* p, int32_t len, bool pattern) {
  SWord* word = (SWord *) malloc(sizeof(SWord));
  memset(word, 0, sizeof(SWord));
  word->word = (char* )p;
  word->len  = len;

  // check format
  if (pattern) {
    word->type = wordType(p, len);
  } else {
    word->type = WT_TEXT;
  }

  return word;
}

// parse one command
void parseCommand(SWords * command, bool pattern) {
  char * p = command->source;
  int32_t start = 0;
  int32_t size  = command->source_len > 0 ? command->source_len : strlen(p);

  bool lastBlank = false;
  for (int i = 0; i <= size; i++) {
    if (p[i] == ' ' || i == size) {
      // check continue blank like '    '
      if (p[i] == ' ') {
        if (lastBlank) {
          start ++;
          continue;
        }
        if (i == 0) { // first blank
          lastBlank = true;
          start ++;
          continue;
        }
        lastBlank = true;
      } 

      // found split or string end , append word
      if (command->head == NULL) {
        command->head = addWord(p + start, i - start, pattern);
        command->count = 1;
      } else {
        SWord * word = command->head;
        while (word->next) {
          word = word->next;
        }
        word->next = addWord(p + start, i - start, pattern);
        command->count ++;
      }
      start = i + 1;
    } else {
      lastBlank = false;
    }
  }
}

// free Command
void freeCommand(SWords* command) {
  SWord* item = command->head;
  // loop
  while (item) {
    SWord* tmp = item;
    item = item->next;
    // if malloc need free
    if (tmp->free && tmp->word) free(tmp->word);
    free(tmp);
  }
}

void GenerateVarType(int type, char** p, int count) {
  STire* tire = createTire(TIRE_LIST);
  for (int i = 0; i < count; i++) {
    insertWord(tire, p[i]);
  }

  pthread_mutex_lock(&tiresMutex);
  tires[type] = tire;
  pthread_mutex_unlock(&tiresMutex);
}

//
//  -------------------- shell auto ----------------
//


// init shell auto funciton , shell start call once 
bool shellAutoInit() {
  // command
  int32_t count = SHELL_COMMAND_COUNT();
  for (int32_t i = 0; i < count; i ++) {
    parseCommand(shellCommands + i, true);
  }

  // tires
  memset(tires, 0, sizeof(STire*) * WT_VAR_CNT);
  pthread_mutex_init(&tiresMutex, NULL);

  // threads
  memset(threads, 0, sizeof(pthread_t*) * WT_FROM_DB_CNT);

  // generate varType
  GenerateVarType(WT_VAR_FUNC,     functions,  sizeof(functions)  /sizeof(char *));
  GenerateVarType(WT_VAR_KEYWORD,  keywords,   sizeof(keywords)   /sizeof(char *));
  GenerateVarType(WT_VAR_DBOPTION, db_options, sizeof(db_options) /sizeof(char *));
  GenerateVarType(WT_VAR_TBACTION, tb_actions, sizeof(tb_actions) /sizeof(char *));
  GenerateVarType(WT_VAR_DATATYPE, data_types, sizeof(data_types) /sizeof(char *));
  GenerateVarType(WT_VAR_KEYTAGS,  key_tags,   sizeof(key_tags)   /sizeof(char *));
  GenerateVarType(WT_VAR_KEYSELECT,key_select, sizeof(key_select) /sizeof(char *));

  printfIntroduction();

  return true;
}

// exit shell auto funciton, shell exit call once
void shellAutoExit() {
  // free command
  int32_t count = SHELL_COMMAND_COUNT();
  for (int32_t i = 0; i < count; i ++) {
    freeCommand(shellCommands + i);
  }

  // free tires
  pthread_mutex_lock(&tiresMutex);
  for (int32_t i = 0; i < WT_VAR_CNT; i++) {
    if (tires[i]) {
      freeTire(tires[i]);
      tires[i] = NULL;
    } 
  }
  pthread_mutex_unlock(&tiresMutex);
  // destory
  pthread_mutex_destroy(&tiresMutex);

  // free threads
  for (int32_t i = 0; i < WT_FROM_DB_CNT; i++) {
    if (threads[i]) {
      taosDestroyThread(threads[i]);
      threads[i] = NULL;
    } 
  }

  // free lastMatch
  if (lastMatch) {
    freeMatch(lastMatch);
    lastMatch = NULL;
  }
}

//
//  -------------------  auto ptr for tires --------------------------
//
bool setNewAuotPtr(int type, STire* pNew) {
  if (pNew == NULL)
    return false;

  pthread_mutex_lock(&tiresMutex);
  STire* pOld = tires[type];
  if (pOld != NULL) {
    // previous have value, release self ref count
    if (--pOld->ref == 0) {
      freeTire(pOld);
    }
  }

  // set new
  tires[type] = pNew;
  tires[type]->ref = 1;
  pthread_mutex_unlock(&tiresMutex);

  return true;
}

// get ptr
STire* getAutoPtr(int type) {
  if (tires[type] == NULL) {
    return NULL;
  }

  pthread_mutex_lock(&tiresMutex);
  tires[type]->ref++;
  pthread_mutex_unlock(&tiresMutex);

  return tires[type];
}

// put back tire to tires[type], if tire not equal tires[type].p, need free tire
void putBackAutoPtr(int type, STire* tire) {
  if (tire == NULL) {
    return ;
  }

  pthread_mutex_lock(&tiresMutex);
  if (tires[type] != tire) {
    //update by out,  can't put back , so free
    if (--tire->ref == 1) {
      // support multi thread getAuotPtr
      freeTire(tire);
    }
    
  } else {
    tires[type]->ref--;
    assert(tires[type]->ref > 0);
  }
  pthread_mutex_unlock(&tiresMutex);

  return ;
}



//
//  -------------------  var Word --------------------------
//

#define MAX_CACHED_CNT 100000 // max cached rows 10w
// write sql result to var name, return write rows cnt
int writeVarNames(int type, TAOS_RES* tres) {
  // fetch row 
  TAOS_ROW row = taos_fetch_row(tres);
  if (row == NULL) {
    return 0;
  }

  TAOS_FIELD *fields = taos_fetch_fields(tres);
  // create new tires
  char tireType = type == WT_VAR_TABLE ? TIRE_TREE : TIRE_LIST;
  STire* tire = createTire(tireType);

  // enum rows
  char name[1024];
  int numOfRows = 0;
  do {
    int32_t* lengths = taos_fetch_lengths(tres);
    int32_t bytes = lengths[0];
    if(fields[0].type == TSDB_DATA_TYPE_SMALLINT) {
      sprintf(name,"%d", *(int16_t*)row[0]);
    } else {
      memcpy(name, row[0], bytes);
    }
    
    name[bytes] = 0;  //set string end 
    // insert to tire
    insertWord(tire, name);

    if (++numOfRows > MAX_CACHED_CNT ) {
      break;
    }

    row = taos_fetch_row(tres);
  } while (row != NULL);

  // replace old tire
  setNewAuotPtr(type, tire);

  return numOfRows;
}

bool firstMatchCommand(TAOS * con, Command * cmd);
//
//  thread obtain var thread from db server 
//
void* varObtainThread(void* param) {
  int type = *(int* )param;
  free(param);

  if (varCon == NULL || type > WT_FROM_DB_MAX) {
    return NULL;
  }

  TAOS_RES* pSql = taos_query_h(varCon, varSqls[type], NULL);
  if (taos_errno(pSql)) {
    taos_free_result(pSql);
    return NULL;
  }

  // write var names from pSql
  int cnt = writeVarNames(type, pSql);

  // free sql
  taos_free_result(pSql);

  // check need call auto tab 
  if (cnt > 0 && waitAutoFill) {
    // press tab key by program
    firstMatchCommand(varCon, varCmd);
  }

  return NULL;
}

// only match next one word from all match words, return valuue must free by caller
char* matchNextPrefix(STire* tire, char* pre) {
  SMatch* match = NULL;

  // re-use last result
  if (lastMatch) {
    if (strcmp(pre, lastMatch->pre) == 0) {
      // same pre
      match = lastMatch;
    }
  }

  if (match == NULL) {
    // not same with last result
    if (pre[0] == 0) {
      // EMPTY PRE
      match = enumAll(tire);
    } else {
      // NOT EMPTY
      match = matchPrefix(tire, pre, NULL);
    }
    
    // save to lastMatch
    if (match) {
      if (lastMatch)
        freeMatch(lastMatch);
      lastMatch = match;
    }
  }

  // check valid
  if (match == NULL || match->head == NULL) {
    // no one matched
    return false;
  }

  if (cursorVar == -1) {
    // first
    cursorVar = 0;
    return strdup(match->head->word);    
  }

  // according to cursorVar , calculate next one
  int i = 0;
  SMatchNode* item = match->head;
  while (item) {
    if (i == cursorVar + 1) {
      // found next position ok
      if (item->next == NULL) {
        // match last item, reset cursorVar to head
        cursorVar = -1;
      } else {
        cursorVar = i;
      }

      return strdup(item->word);
    }

    // check end item
    if (item->next == NULL) {
      // if cursorVar > var list count, return last and reset cursorVar
      cursorVar = -1;

      return strdup(item->word);
    }

    // move next
    item = item->next;
    i++;
  }

  return NULL;
}

// search pre word from tire tree, return value must free by caller
char* tireSearchWord(int type, char* pre) {
  if (type == WT_TEXT) {
    return NULL;
  }

  if(type > WT_FROM_DB_MAX) {
    // NOT FROM DB , tires[type] alwary not null
    STire* tire = tires[type];
    if (tire == NULL)
      return NULL;
    return  matchNextPrefix(tire, pre);
  }

  // TYPE CONTEXT GET FROM DB
  pthread_mutex_lock(&tiresMutex);

  // check need obtain from server
  if (tires[type] == NULL) {
    waitAutoFill = true;
    // need async obtain var names from db sever
    if (threads[type] != NULL) {
      if (taosThreadRunning(threads[type])) {
        // thread running , need not obtain again, return 
        pthread_mutex_unlock(&tiresMutex);
        return NULL;
      }
      // destroy previous thread handle for new create thread handle
      taosDestroyThread(threads[type]);
      threads[type] = NULL;
    }
  
    // create new
    void * param = malloc(sizeof(int));
    *((int* )param) = type;
    threads[type] = taosCreateThread(varObtainThread, param);
    pthread_mutex_unlock(&tiresMutex);
    return NULL;
  }
  pthread_mutex_unlock(&tiresMutex);

  // can obtain var names from local
  STire* tire = getAutoPtr(type);
  if (tire == NULL) {
    return NULL;
  }

  char* str = matchNextPrefix(tire, pre);
  // used finish, put back pointer to autoptr array
  putBackAutoPtr(type, tire);

  return str;
}

// match var word, word1 is pattern , word2 is input from shell 
bool matchVarWord(SWord* word1, SWord* word2) {
  // search input word from tire tree 
  char pre[512];
  memcpy(pre, word2->word, word2->len);
  pre[word2->len] = 0;

  char* str = NULL;
  if (word1->type == WT_VAR_ALLTABLE) {
    // ALL_TABLE
    str = tireSearchWord(WT_VAR_STABLE, pre);
    if (str == NULL) {
      str = tireSearchWord(WT_VAR_TABLE, pre);
      if(str == NULL)
        return false;
    }
  } else {
    // OTHER
    str = tireSearchWord(word1->type, pre);
    if (str == NULL) {
      // not found or word1->type variable list not obtain from server, return not match
      return false;
    }
  }

  // free previous malloc
  if(word1->free && word1->word) {
    free(word1->word);
  }

  // save
  word1->word = str;
  word1->len  = strlen(str);
  word1->free = true; // need free
  
  return true;
}

//
//  -------------------  match words --------------------------
//


// compare command cmd1 come from shellCommands , cmd2 come from user input
int32_t compareCommand(SWords * cmd1, SWords * cmd2) {
  SWord * word1 = cmd1->head;
  SWord * word2 = cmd2->head;

  if (word1 == NULL || word2 == NULL) {
    return -1;
  }

  for (int32_t i = 0; i < cmd1->count; i++) {
    if (word1->type == WT_TEXT) {
      // WT_TEXT match
      if (word1->len == word2->len) {
        if (strncasecmp(word1->word, word2->word, word1->len) != 0)
          return -1; 
      } else if (word1->len < word2->len) {
        return -1;
      } else {
        // word1->len > word2->len
        if (strncasecmp(word1->word, word2->word, word2->len) == 0) {
          cmd1->matchIndex = i;
          cmd1->matchLen = word2->len;
          return i;
        } else {
          return -1;
        }
      }
    } else {
      // WT_VAR auto match any one word
      if (word2->next == NULL) { // input words last one
        if (matchVarWord(word1, word2)) {
          cmd1->matchIndex = i;
          cmd1->matchLen = word2->len;
          varMode = true;
          return i;
        }
        return -1;
      }
    }

    // move next
    word1 = word1->next;
    word2 = word2->next;
    if (word1 == NULL || word2 == NULL) {
      return -1;
    }
  }

  return -1;
}

// match command
SWords * matchCommand(SWords * input, bool continueSearch) {
  int32_t count = SHELL_COMMAND_COUNT();
  for (int32_t i = 0; i < count; i ++) {
    SWords * shellCommand = shellCommands + i;
    if (continueSearch && lastMatchIndex != -1 && i <= lastMatchIndex) {
      // new match must greate than lastMatchIndex
      if (varMode && i == lastMatchIndex) {
        // do nothing, var match on lastMatchIndex
      } else {
        continue;
      }
    }

    // command is large
    if (input->count > shellCommand->count ) {
      continue;
    }

    // compare
    int32_t index = compareCommand(shellCommand, input);
    if (index != -1) {
      if (firstMatchIndex == -1)
        firstMatchIndex = i;
      curMatchIndex = i;
      return &shellCommands[i];
    }
  }

  // not match
  return NULL;
}

//
//  -------------------  print screen --------------------------
//

// delete char count
void deleteCount(Command * cmd, int count) {
  int size = 0;
  int width = 0;
  clearScreen(cmd->endOffset + prompt_size, cmd->screenOffset + prompt_size);

  // loop delete
  while (--count >= 0 && cmd->cursorOffset > 0) {
    getPrevCharSize(cmd->command, cmd->cursorOffset, &size, &width);
    memmove(cmd->command + cmd->cursorOffset - size, cmd->command + cmd->cursorOffset,
            cmd->commandSize - cmd->cursorOffset);
    cmd->commandSize -= size;
    cmd->cursorOffset -= size;
    cmd->screenOffset -= width;
    cmd->endOffset -= width;
  }
}

// show screen
void printScreen(TAOS * con, Command * cmd, SWords * match) {
  // modify Command
  if (firstMatchIndex == -1 || curMatchIndex == -1) {
    // no match
    return ;
  }

  // first tab press 
  const char * str = NULL;
  int strLen = 0; 

  if (firstMatchIndex == curMatchIndex && lastWordBytes == -1) {
    // first press tab
    SWord * word = MATCH_WORD(match);
    str = word->word + match->matchLen;
    strLen = word->len - match->matchLen;
    lastMatchIndex = firstMatchIndex;
    lastWordBytes = word->len;
  } else {
    if (lastWordBytes == -1)
      return ;
    deleteCount(cmd, lastWordBytes);

    SWord * word = MATCH_WORD(match);
    str = word->word;
    strLen = word->len;
    // set current to last
    lastMatchIndex = curMatchIndex;
    lastWordBytes = word->len;
  }
  
  // insert new
  insertStr(cmd, (char *)str, strLen);
}


// main key press tab , matched return true else false
bool firstMatchCommand(TAOS * con, Command * cmd) {
  // parse command
  SWords* input = (SWords *)malloc(sizeof(SWords));
  memset(input, 0, sizeof(SWords));
  input->source = cmd->command;
  input->source_len = cmd->commandSize;
  parseCommand(input, false);

  // if have many , default match first, if press tab again , switch to next
  curMatchIndex  = -1;
  lastMatchIndex = -1;
  SWords * match = matchCommand(input, true);
  if (match == NULL) {
    // not match , nothing to do
    freeCommand(input);
    free(input);
    return false;
  }

  // print to screen
  printScreen(con, cmd, match);
  freeCommand(input);
  free(input);
  return true;
}

// create input source
void createInputFromFirst(SWords* input, SWords * firstMatch) {
  //
  // if next pressTabKey , input context come from firstMatch, set matched length with source_len
  //
  input->source = (char*)malloc(1024);
  memset((void* )input->source, 0, 1024);

  SWord * word = firstMatch->head;

  // source_len = full match word->len + half match with firstMatch->matchLen  
  for (int i = 0; i < firstMatch->matchIndex && word; i++) {
    // combine source from each word
    strncpy(input->source + input->source_len, word->word, word->len);
    strcat(input->source, " "); // append blank splite
    input->source_len += word->len + 1; // 1 is blank length
    // move next
    word = word->next;
  }
  // appand half matched word for last
  if (word) {
    strncpy(input->source + input->source_len, word->word, firstMatch->matchLen);
    input->source_len += firstMatch->matchLen;
  }
}

// user press Tabkey again is named next , matched return true else false
bool nextMatchCommand(TAOS * con, Command * cmd, SWords * firstMatch) {
  if (firstMatch == NULL || firstMatch->head == NULL) {
    return false;
  }
  SWords* input = (SWords *)malloc(sizeof(SWords));
  memset(input, 0, sizeof(SWords));

  // create input from firstMatch
  createInputFromFirst(input, firstMatch);

  // parse input
  parseCommand(input, false);

  // if have many , default match first, if press tab again , switch to next
  SWords * match = matchCommand(input, true);
  if (match == NULL) {
    // if not match , reset all index
    firstMatchIndex = -1;
    curMatchIndex   = -1;
    match = matchCommand(input, false);
    if (match == NULL) {
      freeCommand(input);
      if (input->source)
        free(input->source);
      free(input);
      return false;
    }
  }

  // print to screen
  printScreen(con, cmd, match);

  // free
  freeCommand(input);
  if (input->source) {
    free(input->source);
    input->source = NULL;
  }
  free(input);

  return true;
}

// fill with type
bool fillWithType(TAOS * con, Command * cmd, char* pre, int type) {
  // get type
  STire* tire = tires[type];
  char* str = matchNextPrefix(tire, pre);
  if (str == NULL) {
    return false;
  }

  // need insert part string
  char * part = str + strlen(pre);

  // show
  int count = strlen(part);
  insertStr(cmd, part, count);
  cntDel = count; // next press tab delete current append count

  free(str);
  return true;
}

// fill with type
bool fillTableName(TAOS * con, Command * cmd, char* pre) {
  // search stable and table
  char * str = tireSearchWord(WT_VAR_STABLE, pre);
  if (str == NULL) {
    str = tireSearchWord(WT_VAR_TABLE, pre);
    if(str == NULL)
      return false;
  }

  // need insert part string
  char * part = str + strlen(pre); 

  // delete autofill count last append
  if(cntDel > 0) {
    deleteCount(cmd, cntDel);
    cntDel = 0;
  }

  // show
  int count = strlen(part);
  insertStr(cmd, part, count);
  cntDel = count; // next press tab delete current append count
  
  free(str);
  return true;
}

//
// find last word from sql select clause
//  example :
//  1 select cou -> press tab  select count(
//  2 select count(*),su -> select count(*), sum(
//  3 select count(*), su -> select count(*), sum(
//
char * lastWord(char * p) {
  // get near from end revert find ' ' and ',' 
  char * p1 = strrchr(p, ' ');
  char * p2 = strrchr(p, ',');

  if (p1 && p2) {
    return p1 > p2 ? p1 + 1 : p2 + 1;
  } else if (p1) {
    return p1 + 1;
  } else if(p2) {
    return p2 + 1;
  } else {
    return p;
  } 
}

bool fieldsInputEnd(char* sql) {
  // not in '()'
  char* p1 = strrchr(sql, '(');
  char* p2 = strrchr(sql, ')');
  if (p1 && p2 == NULL) {
    // like select count( '  '
    return false;
  } else if (p1 && p2 && p1 > p2) {
    // like select sum(age), count( ' '
    return false;
  }

  // not in ','
  char * p3 = strrchr(sql, ',');
  char * p = p3;
  // like select ts, age,'    ' 
  if (p) {
    ++p;
    bool allBlank = true; // after last ','  all char is blank
    int  cnt = 0; // blank count , like '    ' as one blank
    char * plast = NULL; // last blank position
    while(*p) {
      if (*p == ' ') {
        plast = p;
        cnt ++;
      } else {
        allBlank = false;
      }
      ++p;
    }

    // any one word is not blank
    if(allBlank) {
      return false;
    }

    // like 'select count(*),sum(age) fr' need return true
    if (plast && plast > p3 && p2 > p1 && plast > p2 && p1 > p3) {
      return true;
    }

    // if last char not ' ', then not end field, like 'select count(*), su' can fill sum(
    if(sql[strlen(sql)-1] != ' ' && cnt <= 1) {
      return false;
    }
  }

  char * p4 = strrchr(sql, ' ');
  if(p4 == NULL) {
    // only one word
    return false;
  }

  return true;
}

// need insert from
bool needInsertFrom(char * sql, int len) {
  // last is blank 
  if(sql[len-1] != ' ') {
    // insert from keyword
    return false;
  }

  //  select fields input is end
  if (!fieldsInputEnd(sql)) {
    return false;
  }

  // can insert from keyword
  return true;
}

// p is string following select keyword
bool appendAfterSelect(TAOS * con, Command * cmd, char* sql, int32_t len) {
  char* p = strndup(sql, len);

  // union all
  char * p1;
  do {
    p1 = strstr(p, UNION_ALL);
    if(p1) {
      p = p1 + strlen(UNION_ALL);
    }
  } while (p1);

  char * from = strstr(p, " from ");
  //last word , maybe empty string or some letters of a string
  char * last = lastWord(p);
  bool ret = false;
  if (from == NULL) {
    bool fieldEnd = fieldsInputEnd(p);
    // cheeck fields input end then insert from keyword
    if (fieldEnd && p[len-1] == ' ') {
      insertStr(cmd, "from", 4);
      free(p);
      return true;
    }

    // fill funciton
    if(fieldEnd) {
      // fields is end , need match keyword
      ret = fillWithType(con, cmd, last, WT_VAR_KEYWORD);
    } else {
      ret = fillWithType(con, cmd, last, WT_VAR_FUNC);
    }
    
    free(p);
    return ret;
  }

  // have from
  char * blank = strstr(from + 6, " ");
  if (blank == NULL) {
    // no table name, need fill
    ret = fillTableName(con, cmd, last);
  } else {
    ret = fillWithType(con, cmd, last, WT_VAR_KEYWORD);
  }

  free(p);
  return ret;
}

int32_t searchAfterSelect(char* p, int32_t len) {
  // select * from st;
  if(strncasecmp(p, "select ", 7) == 0) {
    // check nest query
    char *p1 = p + 7;
    while(1) {
      char *p2 = strstr(p1, "select ");
      if(p2 == NULL)
        break;
      p1 = p2 + 7;
    }

    return p1 - p;
  }

  char* as_pos_end = strstr(p, " as select ");
  if (as_pos_end == NULL)
    return -1;
  as_pos_end += 11;

  // create table <stream_name> as select
  if(strncasecmp(p, "create table ", 13) == 0) {
    return as_pos_end - p;;
  }

  return -1;
}

bool matchSelectQuery(TAOS * con, Command * cmd) {
  // if continue press Tab , delete bytes by previous autofill
  if (cntDel > 0) {
    deleteCount(cmd, cntDel);
    cntDel = 0;
  }

  // match select ...
  int len = cmd->commandSize;
  char * p = cmd->command;

  // remove prefix blank
  while (p[0] == ' ' && len > 0) {
    p++;
    len--;
  }

  // special range
  if(len < 7 || len > 512) {
    return false;
  }

  // search
  char* sql_cp = strndup(p, len);
  int32_t n = searchAfterSelect(sql_cp, len);
  free(sql_cp);
  if(n == -1 || n > len)
    return false;
  p   += n;
  len -= n;

  // append
  return appendAfterSelect(con, cmd, p, len);
}

// if is input create fields or tags area, return true
bool isCreateFieldsArea(char* p) {
  // put to while, support like create table st(ts timestamp, bin1 binary(16), bin2 + blank + TAB
  char* p1 = strdup(p);
  bool  ret = false;
  while (1) {
    char* left = strrchr(p1, '(');
    if (left == NULL) {
      // like 'create table st'
      ret = false;
      break;
    }

    char* right = strrchr(p1, ')');
    if (right == NULL) {
      // like 'create table st( '
      ret = true;
      break;
    }

    if (left > right) {
      // like 'create table st( ts timestamp, age int) tags(area '
      ret = true;
      break;
    }

    // set string end by small for next strrchr search
    *left = 0;
  }
  free(p1);

  return ret;
}

bool matchCreateTable(TAOS * con, Command * cmd) {
  // if continue press Tab , delete bytes by previous autofill
  if (cntDel > 0) {
    deleteCount(cmd, cntDel);
    cntDel = 0;
  }

  // match select ...
  int len = cmd->commandSize;
  char * p = cmd->command;

  // remove prefix blank
  while (p[0] == ' ' && len > 0) {
    p++;
    len--;
  }

  // special range
  if(len < 7 || len > 1024) {
    return false;
  }

  // select and from 
  if(strncasecmp(p, "create table ", 13) != 0) {
    // not select query clause
    return false;
  }
  p += 13;
  len -= 13;

  char* ps = strndup(p, len);
  bool ret = false;
  char * last = lastWord(ps);

  // except create table m1 as select ....
  if (strstr(ps, " as select")) {
    free(ps);
    return false;
  }

  // check in create fields or tags input area
  if (isCreateFieldsArea(ps)) {
    ret = fillWithType(con, cmd, last, WT_VAR_DATATYPE);
  }

  // tags
  if (!ret) {
    // find only one ')' , can insert tags
    char * p1 = strchr(ps, ')');
    if (p1) {
      if(strchr(p1 + 1, ')') == NULL && strstr(p1 + 1, "tags") == NULL) {
        // can insert tags keyword
        ret = fillWithType(con, cmd, last, WT_VAR_KEYTAGS);
      }
    }
  }

  free(ps);
  return ret;
}

bool matchOther(TAOS * con, Command * cmd) {
  int len = cmd->commandSize;
  char* p = cmd->command;
  // '\\'
  if (p[len - 1] == '\\') {
    // append '\G'
    char a[] = "G;";
    insertStr(cmd, a, 2);
    return true;
  }

  // too small
  if(len < 8)
    return false;

  // like 'from ( '
  char* sql = strndup(p, len);
  char* last = lastWord(sql);

  if (strcmp(last, "from(") == 0) {
    fillWithType(con, cmd, "", WT_VAR_KEYSELECT);
    free(sql);
    return true;
  }
  if (strncmp(last, "(", 1) == 0) {
    last += 1; 
  }

  char* from = strstr(sql, " from");
  // find last ' from'
  while (from) {
    char* p1 = strstr(from + 5, " from");
    if (p1 == NULL)
      break;
    from = p1;
  }

  if (from) {
    // find next is '('
    char * p2 = from + 5;
    bool found  = false; // found 'from ... ( ...'  ... is any count of blank
    bool found1 = false; // found '('
    while (1) {
      if ( p2 == last || *p2 == '\0') {
        // last word or string end
        if (found1) {
          found = true;
        }
        break;
      } else if(*p2 == '(') {
        found1 = true;
      } else if(*p2 == ' ') {
        // do nothing
      } else {
        // have any other char
        break;
      }

      // move next
      p2++;
    }

    if (found) {
      fillWithType(con, cmd, last, WT_VAR_KEYSELECT);
      free(sql);
      return true;
    }
  }

  free(sql);    
  return false;
}

// main key press tab
void pressTabKey(TAOS * con, Command * cmd) {
  // check 
  if (cmd->commandSize == 0) { 
    // empty
    showHelp();
    showOnScreen(cmd);
    return ;
  } 

  // save connection to global
  varCon = con;
  varCmd = cmd;
  bool matched = false;

  // manual match like create table st( ...
  matched = matchCreateTable(con, cmd);
  if (matched)
    return ;

  // shellCommands match 
  if (firstMatchIndex == -1) {
    matched = firstMatchCommand(con, cmd);
  } else {
    matched = nextMatchCommand(con, cmd, &shellCommands[firstMatchIndex]);
  }
  if (matched)
    return ;

  // NOT MATCHED ANYONE
  // match other like '\G' ...
  matched = matchOther(con, cmd);
  if (matched)
    return ;

  // manual match like select * from ...
  matched = matchSelectQuery(con, cmd);
  if (matched)
    return ;

  return ;
}

// press othr key
void pressOtherKey(char c) {
  // reset global variant
  firstMatchIndex = -1;
  lastMatchIndex  = -1;
  curMatchIndex   = -1;
  lastWordBytes   = -1;

  // var names
  cursorVar    = -1;
  varMode      = false;
  waitAutoFill = false;
  cntDel       = 0;

  if (lastMatch) {
    freeMatch(lastMatch);
    lastMatch = NULL;
  }
}

// put name into name, return name length
int getWordName(char* p, char * name, int nameLen) {
  //remove prefix blank
  while (*p == ' ') {
    p++;
  }

  // get databases name;
  int i = 0;
  while(p[i] != 0 && i < nameLen - 1) {
    name[i] = p[i]; 
    i++;
    if(p[i] == ' ' || p[i] == ';'|| p[i] == '(') {
      // name end
      break;
    }
  }
  name[i] = 0;

  return i;
}

// deal use db, if have  'use' return true
bool dealUseDB(char * sql) {
  // check use keyword 
  if(strncasecmp(sql, "use ", 4) != 0) {
    return false;
  }
  
  char db[256];
  char *p = sql + 4;
  if (getWordName(p, db, sizeof(db)) == 0) {
    // no name , return 
    return true;
  }

  //  dbName is previous use open db name
  if (strcasecmp(db, dbName) == 0) {
    // same , no need switch 
    return true;
  }

  // switch new db
  pthread_mutex_lock(&tiresMutex);
  // STABLE set null
  STire* tire = tires[WT_VAR_STABLE];
  tires[WT_VAR_STABLE] = NULL;
  if(tire) {
    freeTire(tire);
  }
  // TABLE set null
  tire = tires[WT_VAR_TABLE];
  tires[WT_VAR_TABLE] = NULL;
  if(tire) {
    freeTire(tire);
  }
  // save
  strcpy(dbName, db);
  pthread_mutex_unlock(&tiresMutex);

  return true;
}

// deal create, if have 'create' return true
bool dealCreateCommand(char * sql) {
  // check keyword 
  if(strncasecmp(sql, "create ", 7) != 0) {
    return false;
  }
  
  char name[1024];
  char *p = sql + 7;
  if (getWordName(p, name, sizeof(name)) == 0) {
    // no name , return 
    return true;
  }

  int type = -1;
  //  dbName is previous use open db name
  if (strcasecmp(name, "database") == 0) {
    type = WT_VAR_DBNAME;
  } else if (strcasecmp(name, "table") == 0) {
    if(strstr(sql, " tags") != NULL && strstr(sql, " using ") == NULL)
      type = WT_VAR_STABLE;
    else
      type = WT_VAR_TABLE;
  } else if (strcasecmp(name, "user") == 0) {
    type = WT_VAR_USERNAME;
  } else {
    // no match , return 
    return true;
  }

  // move next
  p += strlen(name);

  // get next word , that is table name
  if (getWordName(p, name, sizeof(name)) == 0) {
    // no name , return 
    return true;
  }

  // switch new db
  pthread_mutex_lock(&tiresMutex);
  // STABLE set null
  STire* tire = tires[type];
  if(tire) {
    insertWord(tire, name);
  }
  pthread_mutex_unlock(&tiresMutex);

  return true;
}

// deal create, if have 'drop' return true
bool dealDropCommand(char * sql) {
  // check keyword 
  if(strncasecmp(sql, "drop ", 5) != 0) {
    return false;
  }
  
  char name[1024];
  char *p = sql + 5;
  if (getWordName(p, name, sizeof(name)) == 0) {
    // no name , return 
    return true;
  }

  int type = -1;
  //  dbName is previous use open db name
  if (strcasecmp(name, "database") == 0) {
    type = WT_VAR_DBNAME;
  } else if (strcasecmp(name, "table") == 0) {
    type = WT_VAR_ALLTABLE;
  } else if (strcasecmp(name, "dnode") == 0) {
    type = WT_VAR_DNODEID;
  } else if (strcasecmp(name, "user") == 0) {
    type = WT_VAR_USERNAME;
  } else {
    // no match , return 
    return true;
  }

  // move next
  p += strlen(name);

  // get next word , that is table name
  if (getWordName(p, name, sizeof(name)) == 0) {
    // no name , return 
    return true;
  }

  // switch new db
  pthread_mutex_lock(&tiresMutex);
  // STABLE set null
  if(type == WT_VAR_ALLTABLE)  {
    bool del = false;
    // del in stable
    STire* tire = tires[WT_VAR_STABLE];
    if(tire)
      del = deleteWord(tire, name);
    // del in table
    if(!del) {
      tire = tires[WT_VAR_TABLE];
      if(tire)
        del = deleteWord(tire, name);
    }
  } else {
    // OTHER TYPE
    STire* tire = tires[type];
    if(tire)
      deleteWord(tire, name);
  }
  pthread_mutex_unlock(&tiresMutex);

  return true;
}

// callback autotab module after shell sql execute
void callbackAutoTab(char* sqlstr, TAOS* pSql, bool usedb) {
  char *  sql = sqlstr;
  // remove prefix blank
  while (*sql == ' ') {
    sql++;
  }

  if(dealUseDB(sql)) {
    // change to new db
    return ;
  }

  // create command add name to autotab
  if(dealCreateCommand(sql)) {
    return ;
  }

  // drop command remove name from autotab
  if(dealDropCommand(sql)) {
    return ;
  }

  return ;
}