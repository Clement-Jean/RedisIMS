# RedisIMS

RedisIMS is a lightweight Redis module following the If Modified Since (IMS) pattern for caching.

## Example (pseudocode)

```
redisims.get MY_NON_EXISTING_KEY THE_TIMESTAMP -> NULL
redisims.set MY_EXISTING_KEY THE_VALUE THE_TIMESTAMP
redisims.get MY_EXISTING_KEY THE_OUTDATED_TIMESTAMP -> YOUR_OBJECT
redisims.get MY_EXISTING_KEY CURRENT_TIMESTAMP -> NULL
```

## FAQ

- Why is TIME not optional?

    It is important to provide a time that is in sync with your server, we cannot assume that TIME = now().

## Contributing

- Pull request :heavy_check_mark:
    
    - Please keep the same coding style (bracket after newline, ...)
    - Comment/Test your code
    - Update README or other document

- Issue :heavy_check_mark:

    - Provide minimum reproducible code