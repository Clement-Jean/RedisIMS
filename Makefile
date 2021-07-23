IDIR	= includes
CC 		= gcc
CFLAGS 	= -I$(IDIR) -fPIC -std=gnu99 -w -Wall -Wextra -Werror
LDFLAGS	= -shared -Bsymbolic -lc
OBJ		= ims.o
LIB		= ims.so

all: 	$(OBJ) ## Create module
		ld -o $(LIB) $(OBJ) $(LDFLAGS)

%.o: 	%.c
		$(CC) $(CFLAGS) -c -o $@ $<

test:	all ## Launch tests
		cd test && sudo docker-compose up -d && until nc -w 10 127.0.0.1 6379; do sleep 1; done && go test -v

.PHONY: test

clean:
		rm -f *.o
		rm -f *.so
		cd test && sudo docker-compose down
		sudo docker volume rm test_redis-data

.PHONY: clean

help: ## Display this help screen
		@grep -h -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'