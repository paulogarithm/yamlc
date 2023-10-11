LIB = yamlc
AR = ar rc
BLIB = lib$(LIB).a

CFLAGS = -Wall -Wextra -Wpedantic -I. -g3
LDFLAGS = -L. -l$(LIB)

SRC = \
    yamlc.c

HEADER = yamlc.h

OBJS = $(SRC:.c=.o)

LIBPATH = /usr/local/lib/$(BLIB)
HEADERPATH = /usr/local/include/yaml.h

E = $(shell which echo)
INFO = $(E) -e [\\x1b\\x5b34\;1mInfo\\x1b\\x5bm]
ERROR = $(E) -e [\\x1b\\x5b31\;1mError\\x1b\\x5bm]
HINT = $(E) -e \\nHint:
SUCCESS = $(E) -e [\\x1b\\x5b32\;1mDone\\x1b\\x5bm]

# Rules

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: $(BLIB)

$(BLIB): $(OBJS)
	@$(INFO) Building lib
	$(AR) $(BLIB) $(OBJS)

install: $(BLIB)
	@if [ "$$(id -u)" -ne 0 ]; then \
		$(ERROR) Please run as root; \
		$(HINT) Try \'sudo \<command\>\'\\n; \
		exit 1; \
	fi
ifeq ($(force),)
	@if [ -f "$(LIBPATH)" ]; then \
		$(ERROR) '$(LIBPATH)' already exists!; \
		$(HINT) Try \'\<command\> force=yes\'\\n; \
		exit 1; \
	fi
	@if [ -f "$(HEADERPATH)" ]; then \
		$(ERROR) '$(HEADERPATH)' already exists!; \
		$(HINT) Try \'\<command\> force=yes\'\\n; \
		exit 1; \
	fi
else
	$(MAKE) uninstall
endif
	@$(INFO) Copying lib
	cp $(BLIB) $(LIBPATH)
	@$(INFO) Copying header
	cp $(HEADER) $(HEADERPATH)
	@$(SUCCESS)

uninstall:
	@if [ -f "$(LIBPATH)" ]; then \
		$(INFO) Removing lib; \
		$(RM) -r $(LIBPATH); \
	fi;
	@if [ -f "$(HEADERPATH)" ]; then \
		$(INFO) Removing header; \
		$(RM) -r $(HEADERPATH); \
	fi;
	@$(SUCCESS)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(BLIB)

re: fclean all

.PHONY : re fclean clean all $(BLIB) install uninstall