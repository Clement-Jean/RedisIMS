package main

import (
	"strconv"
	"testing"
	"time"

	"github.com/gomodule/redigo/redis"
)

func TestModule(t *testing.T) {
	c, err := redis.Dial("tcp", ":6379")
	if err != nil {
		t.Error(err)
	}
	defer c.Close()

	now := time.Now()

	val, err := c.Do("REDISIMS.GET", "hello", now.Unix())

	if err != nil {
		t.Error(err)
	}

	if val != nil {
		t.Errorf("REDISIMS.GET hello = %s; want nil", string(val.([]uint8)))
	}

	_, err = c.Do("REDISIMS.SET", "hello", "world", strconv.FormatInt(now.Unix(), 10))

	if err != nil {
		t.Error(err)
	}

	val, err = c.Do("GET", "hello")

	if err != nil {
		t.Error(err)
	}

	if val != nil && string(val.([]uint8)) != "world" {
		t.Errorf("GET hello = %s; want nil", string(val.([]uint8)))
	}

	val, err = c.Do("REDISIMS.GET", "hello", strconv.FormatInt(now.Add(-100*time.Second).Unix(), 10))

	if err != nil {
		t.Error(err)
	}

	if val != nil && string(val.([]uint8)) != "world" {
		t.Errorf("REDISIMS.GET hello = %s; want world", string(val.([]uint8)))
	}

	val, err = c.Do("REDISIMS.GET", "hello", strconv.FormatInt(now.Add(100*time.Second).Unix(), 10))

	if err != nil {
		t.Error(err)
	}

	if val != nil {
		t.Errorf("GET hello = %s; want nil", string(val.([]uint8)))
	}
}
