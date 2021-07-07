#include "redismodule.h"

const char *moduleName = "redisims";
const char *moduleGet = "redisims.get";
const char *moduleSet = "redisims.set";
const char *hKey = "MTIME";

/* GetCommand - redisims.get [KEY] [TIME]
 * Returns the value stored for the KEY iff the value is modified since time specified
 */
int GetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    if (argc < 2)
        RedisModule_WrongArity(ctx);

    RedisModule_AutoMemory(ctx);

    const char *key = RedisModule_StringPtrLen(argv[1], NULL);

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
        return REDISMODULE_ERR;
    if (RedisModule_StringToLongLong(argv[2], &time) != REDISMODULE_OK)
        return REDISMODULE_ERR;

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
    if (argc < 3)
        return RedisModule_WrongArity(ctx);

    RedisModule_AutoMemory(ctx);

    const char *key = RedisModule_StringPtrLen(argv[1], NULL);
    const char *value = RedisModule_StringPtrLen(argv[2], NULL);
    long long time;

    if (RedisModule_StringToLongLong(argv[3], &time) != REDISMODULE_OK)
        return REDISMODULE_ERR;
    
    RedisModuleCallReply *setReply = RedisModule_Call(ctx, "SET", "cc", key, value);
    
    if (RedisModule_CallReplyType(setReply) == REDISMODULE_REPLY_ERROR)
        return RedisModule_ReplyWithCallReply(ctx, setReply);

    RedisModuleCallReply *hsetReply = RedisModule_Call(ctx, "HSET", "ccl", hKey, key, time);
    
    return RedisModule_ReplyWithCallReply(ctx, hsetReply);
}

int RedisModule_OnLoad(RedisModuleCtx *ctx)
{
    if (RedisModule_Init(ctx, moduleName, 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, moduleGet, GetCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, moduleSet, SetCommand, "write", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}