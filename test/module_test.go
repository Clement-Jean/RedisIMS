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

	val, err = c.Do("REDISIMS.EXISTS", "hello")

	if err != nil || val.(int64) != 1 {
		t.Error("Expect the key to exist")
	}

	val, err = c.Do("HEXISTS", "TEST", "hello")

	if err != nil || val.(int64) != 1 {
		t.Error("Expect the key to exist")
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

func TestArgChecksSet(t *testing.T) {
	c, err := redis.Dial("tcp", ":6379")
	if err != nil {
		t.Error(err)
	}
	defer c.Close()

	_, err = c.Do("REDISIMS.SET", "", "world", strconv.FormatInt(time.Now().Unix(), 10))

	if err == nil || err.Error() != "argv[1] cannot be empty" {
		t.Error("This should return \"argv[1] cannot be empty\"")
	}

	_, err = c.Do("REDISIMS.SET", "hello", "", strconv.FormatInt(time.Now().Unix(), 10))

	if err == nil || err.Error() != "argv[2] cannot be empty" {
		t.Error("This should return \"argv[2] cannot be empty\"")
	}

	_, err = c.Do("REDISIMS.SET", "hello", "world", nil)

	if err == nil || err.Error() != "argv[3] is not valid" {
		t.Error("This should return \"argv[3] is not valid\"")
	}
}

func TestArgChecksGet(t *testing.T) {
	c, err := redis.Dial("tcp", ":6379")
	if err != nil {
		t.Error(err)
	}
	defer c.Close()

	_, err = c.Do("REDISIMS.GET", "", "world")

	if err == nil || err.Error() != "argv[1] cannot be empty" {
		t.Error("This should return \"argv[1] cannot be empty\"")
	}

	_, err = c.Do("REDISIMS.GET", "hello", nil)

	if err == nil || err.Error() != "argv[2] is not valid" {
		t.Error("This should return \"argv[2] is not valid\"")
	}
}