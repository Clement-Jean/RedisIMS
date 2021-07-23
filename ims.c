#include "redismodule.h"

static const char *moduleName = "redisims";
static const char *moduleGet = "redisims.get";
static const char *moduleSet = "redisims.set";
static const char *moduleExists = "redisims.exists";
static const char *hKey = "MTIME";

/* GetCommand - redisims.get [KEY] [TIME]
 * Returns the value stored for the KEY iff the value is modified since time specified
 */
int GetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 3)
        RedisModule_WrongArity(ctx);

    RedisModule_AutoMemory(ctx);
    int length;
    
    const char *key = RedisModule_StringPtrLen(argv[1], &length);

    if (length == 0)
        return RedisModule_ReplyWithError(ctx, "argv[1] cannot be empty");

    RedisModuleCallReply *cachedTimeReply = RedisModule_Call(ctx, "HGET", "cc", hKey, key);
    
    if (RedisModule_CallReplyType(cachedTimeReply) == REDISMODULE_REPLY_NULL)
    {
        RedisModuleCallReply *getReply = RedisModule_Call(ctx, "GET", "c", key);
    
        return RedisModule_ReplyWithCallReply(ctx, getReply);
    }
    else if (RedisModule_CallReplyType(cachedTimeReply) == REDISMODULE_REPLY_ERROR)
        return RedisModule_ReplyWithCallReply(ctx, cachedTimeReply);

    RedisModuleString *cachedTimeStr = RedisModule_CreateStringFromCallReply(cachedTimeReply);
    long long cachedTime;
    long long time;

    if (RedisModule_StringToLongLong(cachedTimeStr, &cachedTime) != REDISMODULE_OK)
        return RedisModule_ReplyWithError(ctx, "Internal: cachedTimeStr is not correctly formatted");
    if (RedisModule_StringToLongLong(argv[2], &time) != REDISMODULE_OK)
        return RedisModule_ReplyWithError(ctx, "argv[2] is not valid");

    if (cachedTime > time)
    {
        RedisModuleCallReply *getReply = RedisModule_Call(ctx, "GET", "c", key);
    
        return RedisModule_ReplyWithCallReply(ctx, getReply);
    }

    return RedisModule_ReplyWithNull(ctx);
}

/* SetCommand - redisims.set [KEY] [VALUE] [TIME]
 * Stores the VALUE in KEY and store metadata about last updated TIME
 */
int SetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc != 4)
        return RedisModule_WrongArity(ctx);

    RedisModule_AutoMemory(ctx);
    int length;

    const char *key = RedisModule_StringPtrLen(argv[1], &length);

    if (length == 0)
        return RedisModule_ReplyWithError(ctx, "argv[1] cannot be empty");

    const char *value = RedisModule_StringPtrLen(argv[2], &length);

    if (length == 0)
        return RedisModule_ReplyWithError(ctx, "argv[2] cannot be empty");

    long long time;

    if (RedisModule_StringToLongLong(argv[3], &time) != REDISMODULE_OK)
        return RedisModule_ReplyWithError(ctx, "argv[3] is not valid");
    
    RedisModuleCallReply *setReply = RedisModule_Call(ctx, "SET", "cc", key, value);
    
    if (RedisModule_CallReplyType(setReply) == REDISMODULE_REPLY_ERROR)
        return RedisModule_ReplyWithCallReply(ctx, setReply);

    RedisModuleCallReply *hsetReply = RedisModule_Call(ctx, "HSET", "ccl", hKey, key, time);
    
    return RedisModule_ReplyWithCallReply(ctx, hsetReply);
}

/* ExistsCommand - redisims.exists [KEY]
 * Returns 0 if the KEY is not found and 1 if it is
 * This is mostly hidding the hKey
 */
int ExistsCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModuleCallReply *hexistsReply = RedisModule_Call(ctx, "HEXISTS", "cs", hKey, argv[1]);

    return RedisModule_ReplyWithCallReply(ctx, hexistsReply);
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (RedisModule_Init(ctx, moduleName, 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    RedisModule_Log(ctx, "notice", "ARGC = %d\n", argc);

    if (argc == 1)
    {
        hKey = RedisModule_StringPtrLen(argv[0], NULL);
        RedisModule_Log(ctx, "notice", "HKEY = %s\n", hKey);
    }

    if (RedisModule_CreateCommand(ctx, moduleGet, GetCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, moduleSet, SetCommand, "write", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, moduleExists, ExistsCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}